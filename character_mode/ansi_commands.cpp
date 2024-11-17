
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>
#include <termios.h>
#include "ansi_commands.h"



// Function to get the terminal size (width and height) (for the network version of this, we just assume standard terminal size)
void Ansi_commands::get_terminal_size(int &width, int &height) {
    //struct winsize w;
    //ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);  // Get Ansi_commands size
    width = 80;//w.ws_col;
    height = 24;//w.ws_row;
}

// Function to configure terminal to raw mode (non-blocking input) (network version doesn't need this)
void Ansi_commands::enable_raw_mode() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    
    tty.c_lflag &= ~(ICANON | ECHO);   // Disable canonical mode (buffered input) and echo
    tty.c_cc[VMIN] = 0;  // Non-blocking read
    tty.c_cc[VTIME] = 1; // Timeout for read
    
    tcsetattr(STDIN_FILENO, TCSANOW, &tty); // Apply the settings
}

// Function to restore the terminal to default settings (network version doesn't need this)
void Ansi_commands::disable_raw_mode() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    
    tty.c_lflag |= ICANON | ECHO;  // Enable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

// Function to move the cursor to (x, y) position using ANSI escape codes
std::string Ansi_commands::move_cursor_command(int x, int y) {
    //std::cout << "\033[" << y << ";" << x << "H";
    return "\033[" + std::to_string(y) + ";" + std::to_string(x) + "H";

}

// Function to clear the screen
std::string Ansi_commands::clear_screen_command() {
    //move_cursor(0, 0);
    //std::cout << "\033[2J";
    //std::cout.flush();
    return "\033[2J";
}

// Function to hide the cursor (optional)
std::string Ansi_commands::hide_cursor_command() {
    //std::cout << "\033[?25l";
    return "\033[?25l";
}

// Function to show the cursor again
std::string Ansi_commands::show_cursor_command() {
    //std::cout << "\033[?25h";
    return "\033[?25h";
}

// Function to check if a key is available in stdin without blocking
bool Ansi_commands::key_available() {
    struct termios oldt, newt;
    //int oldf;
    
    // Save the old terminal attributes and set new attributes
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Turn off canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    fd_set readfds;
    struct timeval tv;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    tv.tv_sec = 0;
    tv.tv_usec = 0; // No waiting time, non-blocking check

    int result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);

    // Restore old Ansi_commands attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return result > 0; // Returns true if input is available
    
}

// Function to read a key press and return the corresponding arrow key code
char Ansi_commands::read_key() {
    char buffer[3] = {0};
    read(STDIN_FILENO, buffer, 3);  // Read input from the Ansi_commands

    // Check for arrow keys (they send 3-byte sequences)
    if (buffer[0] == '\033' && buffer[1] == '[') {
        switch (buffer[2]) {
            case 'A': return static_cast<char>(Keys::UP_ARROW);  // Up arrow
            case 'B': return static_cast<char>(Keys::DOWN_ARROW);  // Down arrow
            case 'C': return static_cast<char>(Keys::RIGHT_ARROW);  // Right arrow
            case 'D': return static_cast<char>(Keys::LEFT_ARROW);  // Left arrow
            case '1': return static_cast<char>(Keys::ESC);  // Escape key
        }
    }
    return buffer[0];
}

// Function to clear the key buffer
void Ansi_commands::clear_key_buffer() {
    while (Ansi_commands::key_available()) {
        Ansi_commands::read_key(); // Read and discard all available keys
    }
}

// Function to get ANSI escape code from enum value
std::string Ansi_commands::get_ansi_code(AnsiColor color) {
    return "\033[" + std::to_string(static_cast<int>(color)) + "m";
}

std::string Ansi_commands::print_color_command(const std::string& text, AnsiColor color) {
    //std::string color_code = "\033[38;5;" + std::to_string(static_cast<int>(color)) + "m"; //the ;5 gives the 256 palette, but I'm not sure how to use it
    std::cout << get_ansi_code(color) << text << "\033[0m";
    return get_ansi_code(color) + text + "\033[0m";
}

std::string Ansi_commands::print_bold_command(const std::string& text, AnsiColor color) {
    //std::cout << "\033[1;" << std::to_string(static_cast<int>(color)) <<"m"<< text << "\033[0m"; 
    //std::cout << "\033[1m" << text << "\033[0m";
    return "\033[1;" + std::to_string(static_cast<int>(color)) +"m"+ text + "\033[0m"; 

}

std::string Ansi_commands::print_command(const std::string& text, AnsiColor color, bool use_bold, bool use_underline) {
    std::string ansi_code = "\033[";
    if (use_bold) ansi_code += "1;";
    if (use_underline) ansi_code += "4;";
    ansi_code += std::to_string(static_cast<int>(color)) + "m";
    //std::cout << ansi_code << text << "\033[0m";
    return ansi_code + text + "\033[0m";
}