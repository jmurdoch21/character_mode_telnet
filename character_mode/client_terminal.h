#ifndef CLIENT_TERMINAL_H
#define CLIENT_TERMINAL_H

#include <string>
#include "ansi_commands.h"

class  Client_terminal {
    public:
        static void clear_recv_buffer(int client_socket);
        static void get_terminal_size(int &width, int &height);
        static void erase_user_input(int client_socket);
        static void save_cursor_position(int client_socket);
        static void move_cursor(int client_socket, int x, int y);
        static void clear_screen(int client_socket);
        static void hide_cursor(int client_socket);
        static void show_cursor(int client_socket);
        static void print_color(int client_socket, const std::string& text, AnsiColor color = AnsiColor::RESET); //prints text in color color, defaulting to white
        static void print_bold(int client_socket, const std::string& text, AnsiColor color = AnsiColor::RESET); //prints text in bold in color color, defaulting to white
        static void print(int client_socket, const std::string& text, AnsiColor color = AnsiColor::RESET, bool use_bold = 0, bool use_underline = 0); //prints text in color color, defaulting to white
};

#endif