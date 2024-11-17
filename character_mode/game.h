#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>
#include "set_piece.h"
#include "room.h"
#include <sqlite3.h>

enum class GameMenu {
    HOST_GAME,
    JOIN_GAME,
    RULES,
    STATS,
    EXIT
};

class Game{
    protected:
        std::vector<std::string> menu;
        SetPiece title_art;
    public:
        Game();
        void print_menu(int client_socket, size_t selected_row = 0);
        void draw_title(int client_socket);
        void draw_menu(int client_socket, size_t selected_row = 0);
        
        std::vector<std::string> getMenu() { return menu; }
        void set_menu(const std::vector<std::string>& menu);
        void set_title_art(const SetPiece& title_art);

        //void print_menu(const std::vector<std::string>& menu, int selectedRow = 0);
        //int select_from_menu(int client_socket);
        int select_from_menu(int client_socket);

        void start_game();
        void stop_game();
        void host_game(Client *client, std::vector<Room *> &rooms);
        void join_game(Client *client, std::vector<Room *> &rooms);
        void rules(int client_socket);
        void stats(sqlite3* db, int client_socket, const std::string& username);

};

#endif