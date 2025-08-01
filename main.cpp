#include <gtk/gtk.h>
#include <vte/vte.h>
#include <iostream>
#include <string> // For std::string


// Global pointer to the terminal widget
static VteTerminal *g_terminal_widget = NULL;
// Global pointer to the input TextView widget
static GtkTextView *g_input_textview = NULL;

// Panel text constant
const char *INPUT_PANEL_TEXT = "Input Command: Ctrl+Enter for new line"; // Only the panel text constant remains

// Callback for when the Vte terminal's child process (e.g., bash) exits
static void on_child_exited(VteTerminal *terminal, int status, gpointer user_data) {
    std::cout << "Child process exited with status: " << status << std::endl;
    gtk_main_quit(); // Quit the GTK application
}

// Callback for when the window is closed
static void on_window_destroy(GtkWidget *widget, gpointer user_data) {
    gtk_main_quit();
}

// Callback for vte_terminal_spawn_async.
// It receives status and error directly, matching VteTerminalSpawnAsyncCallback type.
static void on_spawn_ready(VteTerminal *terminal, int status, GError *error, gpointer user_data) {
    // If error is not NULL, spawning failed
    if (error) {
        std::cerr << "Failed to spawn child process: " << error->message << std::endl;
        g_error_free(error);
        gtk_main_quit(); // Quit if we can't spawn
    } else {
        std::cout << "Child process spawned successfully (status: " << status << ")." << std::endl;
        g_signal_connect(terminal, "child-exited", G_CALLBACK(on_child_exited), NULL);
        
        // Set focus to the input TextView after successful spawn
        if (g_input_textview) {
            gtk_widget_grab_focus(GTK_WIDGET(g_input_textview));
        }
    }
}

// Callback for the "Copy" menu item on the VteTerminal (for output/scrollback)
static void on_copy_activated(GtkMenuItem *menuitem, gpointer user_data) {
    if (g_terminal_widget) {
        vte_terminal_copy_clipboard_format(g_terminal_widget, VTE_FORMAT_TEXT);
        std::cout << "Text copied to clipboard." << std::endl;
    }
}

// Helper to parse hex color string to GdkRGBA
static void parse_hex_color(const char *hex_str, GdkRGBA *color) {
    gdk_rgba_parse(color, hex_str);
}

// --- Key-press-event on the GtkTextView (Logic unchanged from your provided code) ---
static gboolean on_input_textview_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    // Check if the pressed key is Enter (Return) and Ctrl is NOT pressed
    // This means: Plain Enter sends the command.
    if (event->keyval == GDK_KEY_Return && !(event->state & GDK_CONTROL_MASK)) {
        if (!g_terminal_widget) {
            return TRUE; // Consume the event if terminal is not ready
        }

        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
        GtkTextIter start_iter, end_iter;
        gtk_text_buffer_get_start_iter(buffer, &start_iter);
        gtk_text_buffer_get_end_iter(buffer, &end_iter);

        // Get the text from the GtkTextView's buffer
        char *command_c_str = gtk_text_buffer_get_text(buffer, &start_iter, &end_iter, FALSE);
        std::string command(command_c_str);
        g_free(command_c_str); // Free the allocated string

        // Send the command to the Vte terminal's child process, followed by a newline
        vte_terminal_feed_child(g_terminal_widget, command.c_str(), command.length());
        vte_terminal_feed_child(g_terminal_widget, "\n", 1); // Send newline

        // Clear the input TextView
        gtk_text_buffer_set_text(buffer, "", -1); // -1 means calculate length automatically

        // Ensure focus stays on the input TextView
        gtk_widget_grab_focus(widget);

        return TRUE; // Consume the event so it doesn't insert a newline into the TextView
    }
    // For Ctrl+Enter, or any other key, allow default processing (insert character, etc.)
    // Ctrl+Enter will naturally insert a newline because we are NOT consuming it here.
    return FALSE;
}

// Callback for the button-press-event (right-click) on VteTerminal
static gboolean on_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    // Check if it's a right-click (button 3)
    if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
        GtkMenu *menu = GTK_MENU(user_data); // Cast user_data back to GtkMenu
        
        // Popup the menu at the current mouse position
        gtk_menu_popup_at_pointer(menu, (GdkEvent*)event);

        // Indicate that we've handled the event
        return TRUE;
    }
    // Let other handlers process the event
    return FALSE;
}


int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Load CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkScreen *screen = gdk_screen_get_default();

    if (gtk_css_provider_load_from_path(provider, "style.css", NULL)) {
        gtk_style_context_add_provider_for_screen(screen,
                                                   GTK_STYLE_PROVIDER(provider),
                                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        std::cout << "Loaded style.css successfully." << std::endl;
    } else {
        std::cerr << "Failed to load style.css. Make sure it's in the same directory as the executable." << std::endl;
    }
    g_object_unref(provider);

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Multi-Terminal");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create a VteTerminal widget
    VteTerminal *terminal = VTE_TERMINAL(vte_terminal_new());
    g_terminal_widget = terminal;

    // Apply direct Vte styling from your desired colors
    GdkRGBA bg_color, fg_color, cursor_color;
    parse_hex_color("#272727", &bg_color); // Dark grey background
    parse_hex_color("#eeeeec", &fg_color); // Light grey foreground
    parse_hex_color("#ffffff", &cursor_color); // White cursor

    vte_terminal_set_color_background(terminal, &bg_color);
    vte_terminal_set_color_foreground(terminal, &fg_color);
    vte_terminal_set_color_cursor(terminal, &cursor_color);
    
    PangoFontDescription *font_desc = pango_font_description_from_string("Monospace 11");
    vte_terminal_set_font(terminal, font_desc);
    pango_font_description_free(font_desc);


    GtkWidget *scrolled_window_terminal = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window_terminal),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window_terminal), GTK_WIDGET(terminal));
    
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window_terminal, TRUE, TRUE, 0);

    // --- NEW: Input Panel Label ---
    GtkWidget *input_panel_label = gtk_label_new(INPUT_PANEL_TEXT);
    gtk_widget_set_halign(input_panel_label, GTK_ALIGN_START); // Align text to start (left)
    gtk_widget_set_margin_start(input_panel_label, 5); // Add some margin
    gtk_widget_set_margin_end(input_panel_label, 5);
    gtk_widget_set_margin_top(input_panel_label, 5);
    gtk_widget_set_margin_bottom(input_panel_label, 2); // Less margin to input box

    // Add CSS class to the label for styling
    gtk_style_context_add_class(gtk_widget_get_style_context(input_panel_label), "input-panel-label");
    
    // Pack the label into the vbox before the input textview's scrolled window
    gtk_box_pack_start(GTK_BOX(vbox), input_panel_label, FALSE, FALSE, 0);


    // Create the GtkTextView for multi-line input
    GtkWidget *input_textview = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(input_textview), GTK_WRAP_WORD_CHAR); // Enable word wrapping
    gtk_widget_set_size_request(input_textview, -1, 60); // -1 for width, 60px for height
    g_input_textview = GTK_TEXT_VIEW(input_textview); // Store the input TextView globally

    // --- REMOVED: Placeholder text setup for GtkTextView ---
    // Get the text buffer for the TextView
    GtkTextBuffer *input_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(input_textview));
    // No placeholder tag creation or initial text setting here

    // --- REMOVED: Connect "changed" signal for placeholder management ---
    // g_signal_connect(input_buffer, "changed", G_CALLBACK(on_input_buffer_changed), NULL);

    // Connect the key-press-event to handle Enter key
    g_signal_connect(input_textview, "key-press-event", G_CALLBACK(on_input_textview_key_press), NULL);

    GtkWidget *scrolled_window_input = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window_input),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window_input), input_textview);
    
    // Add CSS class to the input scrolled window for styling
    gtk_style_context_add_class(gtk_widget_get_style_context(scrolled_window_input), "input-scroll-window");
    
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window_input, FALSE, FALSE, 0);
    

    // Spawn a shell inside the terminal asynchronously
    char *const argv_spawn[] = { (char *)"/bin/bash", NULL };
    
    vte_terminal_spawn_async(
        terminal,
        (VtePtyFlags)0,
        NULL,
        (char**)argv_spawn,
        NULL,
        (GSpawnFlags)(G_SPAWN_DEFAULT | G_SPAWN_LEAVE_DESCRIPTORS_OPEN),
        NULL,
        NULL,
        NULL,
        -1,
        NULL,
        on_spawn_ready,
        NULL
    );

    // Context menu setup (only Copy for VteTerminal)
    GtkWidget *context_menu = gtk_menu_new();
    GtkWidget *copy_item = gtk_menu_item_new_with_label("Copy");
    g_signal_connect(copy_item, "activate", G_CALLBACK(on_copy_activated), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(context_menu), copy_item);
    gtk_widget_show_all(context_menu);
    g_signal_connect(G_OBJECT(terminal), "button-press-event", G_CALLBACK(on_button_press_event), context_menu);
    gtk_widget_set_events(GTK_WIDGET(terminal), GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}