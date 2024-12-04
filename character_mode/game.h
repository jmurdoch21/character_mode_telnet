#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>
#include <mutex>
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
        std::mutex game_mutex;

        int num_players;
        int num_remaining_players;
        Client * werewolf_client;
        int werewolf_index;
        Client * engineer_client;
        int engineer_index;
        Client * seer_client;
        int seer_index;
        Client * last_killed_client;
        bool is_night_cycle;
        std::vector<Client *> live_clients;
        std::vector<Client *> game_clients;
        bool* is_alive;
        std::vector<SetPiece *> player_pieces;

        std::vector<std::string> live_house_art = {
                "|\\/|",
                "(^^)",
                "=<>=",
                " __ ",
                "/__\\",
                "|_||"
            };
        std::vector<std::string> dead_house_art = {
                "|\\/|",
                "(XX)",
                "=><=",
                " __ ",
                "/__\\",
                "|_||"
            };
        std::vector<std::string> pointer_art = {
                "      __",
                "     / /",
                "__  / /_",
                "\\ \\/    -",
                " \\      /",
                " /`-_  /"
            };
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
        int select_target(int client_socket, int client_index);

        void start_game(Room * room, sqlite3 * db);
        void stop_game(Room * room);
        void host_game(Client *client, std::vector<Room *> &rooms, sqlite3 * db);
        void join_game(Client *client, std::vector<Room *> &rooms);
        void rules(int client_socket);
        void stats(sqlite3* db, int client_socket, const std::string& username);

};

#endif