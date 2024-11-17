#include <vector>
#include <iostream>
#include <sys/socket.h>
#include "client_terminal.h"
#include "ansi_commands.h"
#include "game.h"
#include "server.h"
#include "database.h"

// Constructor
Game::Game() {
    menu = {"Host Game", "Join Game", "Rules", "Stats", "Exit"};
    std::vector<std::string> the_game= {
        " _____  __  __  ____     ___   __  __  __  ____",
        "|_   _||  ||  ||  __|   / _ \\ /  \\|  \\/  ||  __|",
        "  | |  |  ||  || |_    / / \\ /    |      || |_ ",
        "  | |  |      ||  _|  | | |_|  /\\ |  ||  ||  _| ",
        "  | |  |  ||  || |__   \\ \\_/|     |  ||  || |__",
        "  |_|  |__||__||____|   \\___|__||_|__||__||____|"
    };
    title_art = SetPiece(the_game, true, AnsiColor::RED);
    int terminal_width, terminal_height;
    Client_terminal::get_terminal_size(terminal_width, terminal_height);
    //int x_mid = terminal_width/2;

    int title_xmin = terminal_width/2 - title_art.getWidth()/2;
    int menu_and_title_height = title_art.getHeight() + 3 + menu.size();
    int title_ymin = terminal_height/2 - menu_and_title_height/2;

    title_art.set_position(title_xmin, title_ymin);

}

// Function to print the menu and highlight the selected row
void Game::print_menu(int client_socket, size_t selected_row) {
    //Terminal::clear_screen();
    int terminal_width, terminal_height;
    Client_terminal::get_terminal_size(terminal_width, terminal_height);
    int menu_y_min = title_art.getBoundingBox().getYMax() + 3;
    unsigned long max_menu_width = menu[1].length();
    for (const std::string& row : menu) {
        if (row.length() > max_menu_width) {
            max_menu_width = row.length();
        }
    }
    int menu_x_min = terminal_width/2 - max_menu_width/2;

    for (size_t i = 0; i < menu.size(); ++i) {
        Client_terminal::move_cursor(client_socket,menu_x_min, menu_y_min + i);
        if (i == selected_row) {
            std::string row = "> " + menu[i];  // Highlight selection
            if(i == menu.size()-1) row += "\n";
            Client_terminal::print_bold(client_socket, row, AnsiColor::RED);
        } else {
            std::string row = "  " + menu[i];
            Client_terminal::print(client_socket, row);
            //std::cout << "  " << menu[i] << std::endl;
        }
    }
}

void Game::draw_title(int client_socket) {
    title_art.draw(client_socket);
}

void Game::draw_menu(int client_socket, size_t selected_row) {
    //system("clear");
    Client_terminal::clear_screen(client_socket);
    title_art.draw(client_socket);
    std::cout << "title drawn" << std::endl;
    print_menu(client_socket, selected_row);
    std::cout << "menu drawn" << std::endl;
}

void Game::set_menu(const std::vector<std::string>& menu) {
    this->menu = menu;
}
void Game::set_title_art(const SetPiece& title_art) {
    this->title_art = title_art;
}

//Function to select from the menu
// int Game::select_from_menu(int client_socket) {
//     int selected_row = 0;
//     bool running = true;
//     char key = 0;
//     while (running) {
//         draw_menu(selected_row);
//         while(!Terminal::key_available()){};
//         key = Terminal::read_key();
//         switch (key) {
//             case static_cast<char>(Keys::UP_ARROW):
//             case 'w':
//                 selected_row = (selected_row - 1 + menu.size()) % menu.size();
//                 break;
//             case static_cast<char>(Keys::DOWN_ARROW):
//             case 's':
//                 selected_row = (selected_row + 1) % menu.size();
//                 break;
//             case 10: // Enter key
//                 running = false;
//                 break;
//             default:
//                 break;
//         }
//         //std::cout << "Selected row: " << selected_row << std::endl;
//     }
//     Client_terminal::clear_screen(client_socket);
//     return selected_row;
// }
int Game::select_from_menu(int client_socket) {
    int selected_row = 0;
    bool running = true;
    char key = 0;
    char buffer;
    int bytes_read;
    while (running) {
        draw_menu(client_socket, selected_row);
        //while(!Terminal::key_available()){};
        //key = Terminal::read_key();
        //key = Server::blocking_receive(client_socket, &key, 1);
        int bytes_read = recv(client_socket, &buffer, 1, 0);
        if (bytes_read > 0) {
            key = buffer;
            if (key == '\n' || key == '\r') {
                key = 10;  // Convert to the expected enter key value
            }
        }
        std::cout << "key: " << key << std::endl;
        switch (key) {
            case static_cast<char>(Keys::UP_ARROW):
            case 'w':
                selected_row = (selected_row - 1 + menu.size()) % menu.size();
                break;
            case static_cast<char>(Keys::DOWN_ARROW):
            case 's':
                selected_row = (selected_row + 1) % menu.size();
                break;
            case 10: // Enter key
                running = false;
                break;
            default:
                break;
        }
        //std::cout << "Selected row: " << selected_row << std::endl;
    }
    Client_terminal::clear_screen(client_socket);
    return selected_row;
}

void Game::rules(int client_socket) {
    // Implement the rules functionality here
    Client_terminal::clear_screen(client_socket);
    Client_terminal::move_cursor(client_socket, 0, 0);

    std::string rules = 
        "Rules:\n"
        "1. Player 1 starts the game by typing 'start' when everyone has joined.\n"
        "2. The werewolf chooses his victim. (Use WASD to move)\n"
        "3. The other players also choose targets for their night phase actions.\n"
        "4. Players wake up and type messages to send to each other to deduce who the werewolf is.\n"
        "5. The players vote on who they think is the werewolf by entering the index of the player they suspect.\n"
        "6. The player with the most votes is eliminated. If that is the werewolf, villagers win.\n"
        "7. The werewolf wins if there is 1 villager left\n";

    // Sending the rules message
    ssize_t bytes_sent = send(client_socket, rules.c_str(), rules.size(), 0);
    if (bytes_sent == -1) {
        std::cerr << "Error sending rules to client" << std::endl;
    } else {
        std::cout << "Rules sent to client successfully." << std::endl;
    }
}

void Game::stats(sqlite3* db, int client_socket, const std::string& username) {
    // Implement the stats functionality here
    int num_games = 0;
    int games_as_werewolf = 0;
    int num_wins = 0;
    int wins_as_werewolf = 0;

    num_games = Database::get_games_played(db, username);
    games_as_werewolf = Database::get_games_as_werewolf(db, username);
    num_wins = Database::get_num_wins(db, username);
    wins_as_werewolf = Database::get_num_wins_as_werewolf(db, username);

    std::string stats = "Stats:\n"
    "Username: " + username + "\n"
    "Games Played: " + std::to_string(num_games) + "\n"
    "Games as Werewolf: " + std::to_string(games_as_werewolf) + "\n"
    "Wins: " + std::to_string(num_wins) + "\n"
    "Wins as Werewolf: " + std::to_string(wins_as_werewolf) + "\n"; 

    // Sending the stats message
    ssize_t bytes_sent = send(client_socket, stats.c_str(), stats.size(), 0);
    if (bytes_sent == -1) {
        std::cerr << "Error sending stats to client" << std::endl;
    } else {
        std::cout << "Stats sent to client successfully." << std::endl;
    }

    char c;
    ssize_t bytes_received;
    bool exit_stats = false;
    while(!exit_stats){
        bytes_received = recv(client_socket, &c, 1, 0);
        if(bytes_received > 0){
            if(c == 'q' || c == static_cast<char>(Keys::ESC)){
                exit_stats = true;
            }
        }
    }
}

void Game::host_game(Client *client, std::vector<Room*> &rooms) {   
    // Implementation needed
    Client_terminal::clear_screen(client->socket);
    int terminal_width, terminal_height;
    Client_terminal::get_terminal_size(terminal_width, terminal_height);
    std::string prompt = "Enter room name: ";
    int startY = (terminal_height - 1) / 2;
    int startX = (terminal_width - 1) / 2 - prompt.size();
    Client_terminal::move_cursor(client->socket, startX, startY);
    Client_terminal::print(client->socket, prompt);
    std::string room_name;
    Server::receive_line(client->socket, room_name, ECHO_ON);
    Room *room = new Room(room_name);
    room->add_player(client);
    rooms.push_back(room);
    Client_terminal::clear_screen(client->socket);
    Client_terminal::move_cursor(client->socket, 0, 0);
    std::string message = "You are the host of " + room_name + "!\ntype start to begin the game\n";
    Client_terminal::print(client->socket, message);
    while(true){
        std::string input;
        Server::receive_line(client->socket, input, ECHO_ON);
        if(input == "start"){
            break;
        }
    }
}

void Game::join_game(Client *client, std::vector<Room*> &rooms) {
    Client_terminal::clear_screen(client->socket);
    int terminal_width, terminal_height;
    Client_terminal::get_terminal_size(terminal_width, terminal_height);
    bool in_room = false;
    while(!in_room){
        std::string prompt = "Enter room name: ";
        int startY = (terminal_height - 1) / 2;
        int startX = (terminal_width - 1) / 2 - prompt.size();
        Client_terminal::move_cursor(client->socket, startX, startY);
        Client_terminal::print(client->socket, prompt);
        std::string room_name;
        Server::receive_line(client->socket, room_name, ECHO_ON);
        for(int i = 0; i < rooms.size(); i++){
            if(rooms[i]->name == room_name){
                Client_terminal::clear_screen(client->socket);
                Client_terminal::move_cursor(client->socket, 0, 0);
                std::string message = "You have joined " + room_name + "!\n";
                Client_terminal::print(client->socket, message);
                rooms[i]->add_player(client);
                for(auto player : rooms[i]->clients){
                    std::string message = client->username + " has joined the game\n";
                    Client_terminal::print(player->socket, message);
                }
                in_room = true;
                break;
            }
        }
        if(!in_room){
            Client_terminal::clear_screen(client->socket);
            Client_terminal::move_cursor(client->socket, 0, 0);
            std::string message = "Room not found.\nPress esc or q to exit.\n";
            Client_terminal::print(client->socket, message);
            char c;
            ssize_t bytes_received;
            bool do_exit = false;
            while(!do_exit){
                bytes_received = recv(client->socket, &c, 1, 0);
                if(bytes_received > 0){
                    if(c == 'q' || c == static_cast<char>(Keys::ESC)){
                        do_exit = true;
                    }
                }
            }
        }
    }

}

void Game::stop_game() {
    // Implementation needed
}
