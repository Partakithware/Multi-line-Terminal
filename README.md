Multi-line GTK Terminal

A customizable terminal emulator built with GTK+ 3 and Vte, featuring a unique multi-line input area with full text-editor controls, a custom context menu, and styling capabilities via CSS.

This project demonstrates a custom terminal application that leverages the power of libvte for robust terminal emulation (handling shell processes, ANSI escape codes, etc.) while providing an enhanced user experience through a separate GtkTextView for command input. Unlike traditional terminals where input is managed directly by the shell's readline, this application offers a dedicated, multi-line input buffer with standard text editor functionalities.
Features

    Vte Backend: Utilizes libvte for reliable and efficient terminal emulation, ensuring compatibility with various shells and terminal programs.

    Multi-line Input with GtkTextView:

        Dedicated input area at the bottom.

        Allows typing and editing commands across multiple lines.

        Full text-editor controls: select, copy, cut, paste, delete, and navigate text within the input buffer.

        Enter to Send: Pressing Enter (without modifiers) sends the current content of the input area to the terminal.

        Ctrl+Enter for Newline: Pressing Ctrl+Enter inserts a newline character, enabling true multi-line command composition.

    Custom Input Panel: A clear label panel above the input area for user guidance.

    Integrated Scrollbars: Both the terminal display and the input area feature automatic scrollbars.

    Right-Click Context Menu:

        "Copy" option for copying selected text from the terminal output/scrollback buffer.

    Customizable Styling: Easily theme the entire application (terminal, input area, scrollbars, labels) using a style.css file.

Prerequisites

Before building, ensure you have the following installed on your system:

    C++ Compiler: g++ (GNU C++ Compiler)

    GTK+ 3 Development Libraries: libgtk-3-dev

    Vte Development Libraries: libvte-2.91-dev (or similar version, e.g., libvte-2.91-0-dev)

    pkg-config: A utility to retrieve compile and link flags for libraries.

On Debian/Ubuntu-based systems, you can install them using:

sudo apt update
sudo apt install build-essential libgtk-3-dev libvte-2.91-dev pkg-config

Building the Application

    Save the Code:
    Save the provided C++ code into a file named main.cpp.

    Create style.css:
    Create a file named style.css in the same directory as main.cpp and paste the CSS rules provided in the styling section below.

    Compile:
    Open your terminal in the project directory and run the following command:

    g++ main.cpp -o my_terminal `pkg-config --cflags --libs gtk+-3.0 vte-2.91`

    This command uses pkg-config to automatically find the correct compiler flags and linker libraries for GTK+ 3 and Vte.

Usage

To run your custom terminal, execute the compiled binary.

Inputting Commands:

    Typing: Type your commands in the input area at the bottom.

    Sending Command: Press Enter to send the current content of the input area to the shell.

    Multi-line Input: Press Ctrl+Enter to insert a newline character, allowing you to compose multi-line commands before sending.

    Text Editor Controls: Use standard mouse selection, Ctrl+C (copy), Ctrl+X (cut), Ctrl+V (paste), and Delete key within the input area just like a regular text editor.

Copying Output:

    Select text in the main terminal display area with your mouse.

    Right-click on the selected text to bring up the context menu.

    Choose "Copy" to copy the selected text to your system clipboard.

Styling

The application's appearance is customizable via the style.css file. This allows you to change colors, fonts, padding, and borders for various elements.

Feel free to modify these values to create your desired look and feel!

Future Enhancements - Feel free to implement some and put in a PR.

    Command History: Implement up/down arrow key navigation for previous commands in the input GtkTextView.

    Tab Completion: Integrate shell tab completion into the GtkTextView.

    Search Functionality: Add a search bar for the terminal output.

    Multiple Tabs/Windows: Extend to support multiple terminal sessions.

    Configuration File: Allow users to configure settings (fonts, colors, keybindings) via a dedicated configuration file instead of directly in main.cpp or style.css.
