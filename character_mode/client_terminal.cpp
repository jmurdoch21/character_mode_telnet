
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include "client_terminal.h"

//Function to clear the recv buffer
void Client_terminal::clear_recv_buffer(int client_socket){
    char c;
    int bytes_received;
}
// Function to get the terminal size (width and height) (for the network version of this, we just assume standard terminal size)
void Client_terminal::get_terminal_size(int &width, int &height) {
    //struct winsize w;
    //ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);  // Get Ansi_commands size
    width = 80;//w.ws_col;
    height = 24;//w.ws_row;
}

//functions for mamipulating the client's terminal
void Client_terminal::erase_user_input(int client_socket){
    std::string erase_tail = "\033[u\033[K";  // ANSI escape code to clear a trailing line and move cursor to last saved position
    send(client_socket, erase_tail.c_str(), erase_tail.size(), 0);
}
void Client_terminal::save_cursor_position(int client_socket){
    std::string save_cursor_position = "\033[s";  // ANSI escape code to save cursor position
    send(client_socket, save_cursor_position.c_str(), save_cursor_position.size(), 0);
}

// Function to move the cursor to (x, y) position using ANSI escape codes
void Client_terminal::move_cursor(int client_socket, int x, int y) {
    //std::cout << "\033[" << y << ";" << x << "H";
    std::string command = Ansi_commands::move_cursor_command(x, y);
    send(client_socket, command.c_str(), command.length(), 0);
}

// Function to clear the screen
void Client_terminal::clear_screen(int client_socket) {
    move_cursor(client_socket, 0, 0);
    std::string command = Ansi_commands::clear_screen_command();
    send(client_socket, command.c_str(), command.length(), 0);
}

// Function to hide the cursor (optional)
void Client_terminal::hide_cursor(int client_socket) {
    std::string command = Ansi_commands::hide_cursor_command();
    send(client_socket, command.c_str(), command.length(), 0);
}

// Function to show the cursor again
void Client_terminal::show_cursor(int client_socket) {
    std::string command = Ansi_commands::show_cursor_command();
    send(client_socket, command.c_str(), command.length(), 0);
}

void Client_terminal::print_color(int client_socket, const std::string& text, AnsiColor color) {
    std::string message = Ansi_commands::print_color_command(text, color);
    send(client_socket, message.c_str(), message.length(), 0);
}

void Client_terminal::print_bold(int client_socket, const std::string& text, AnsiColor color) {
    std::string message = Ansi_commands::print_bold_command(text, color);
    send(client_socket, message.c_str(), message.length(), 0);
}

void Client_terminal::print(int client_socket, const std::string& text, AnsiColor color, bool use_bold, bool use_underline) {
    std::string message = Ansi_commands::print_command(text, color, use_bold, use_underline);
    send(client_socket, message.c_str(), message.length(), 0);
}
