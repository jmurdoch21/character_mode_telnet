#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <algorithm>
#include <thread>
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
Game::~Game() {
    std::cout << "Game destructor called" << std::endl;
    live_clients.clear();
    game_clients.clear();

    //delete pieces
    for(SetPiece* piece : player_pieces){
        delete piece;
    }
    //player_pieces.clear();


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
//function to select target
int Game::select_target(int client_socket, int player_index, std::vector<bool> selectable_players) {
    int selected_target = 0;
    bool running = true;
    char key = 0;
    char buffer;
    int terminal_width, terminal_height;
    Client_terminal::get_terminal_size(terminal_width, terminal_height);
    SetPiece pointer = SetPiece(pointer_art);
    while (running) {
        Client_terminal::clear_screen(client_socket);
        //draw the houses
        int counter = 0;
        bool is_pointer_on_piece = false;
        for(auto piece : player_pieces){
            if(piece->getBoundingBox().contains(pointer.getBoundingBox().getXMax(), pointer.getBoundingBox().getYMin())){
                piece->setBold(true);
                piece->setColor(AnsiColor::RED);
                piece->draw(client_socket);
                selected_target = counter;
                is_pointer_on_piece = true;
                std::cout << "Pointer on " << live_clients[selected_target]->username << std::endl;
            }
            else{
                piece->setBold(false);
                piece->draw(client_socket);
            }
            Client_terminal::move_cursor(client_socket, piece->getBoundingBox().getXMin(), piece->getBoundingBox().getYMax()+1);
            Client_terminal::print(client_socket, game_clients[counter]->username);
            counter ++;
            //Client_terminal::draw(client_socket, piece);
        }
        pointer.draw(client_socket);
        counter = 0;
        int bytes_read = recv(client_socket, &buffer, 1, 0);
        if (bytes_read > 0) {
            key = buffer;
            if (key == '\n' || key == '\r') {
                key = 10;  // Convert to the expected enter key value
            }
        }
        else{
            throw std::runtime_error("Error receiving key");
        }
        std::cout << "key: " << key << std::endl;
        switch (key) {
            case static_cast<char>(Keys::UP_ARROW):
            case 'w':
                if(pointer.getBoundingBox().getYMin()-1 > 0){
                    pointer.set_position(pointer.getBoundingBox().getXMin(), pointer.getBoundingBox().getYMin()-1);
                }               
                break;
            case static_cast<char>(Keys::DOWN_ARROW):
            case 's':
                if(pointer.getBoundingBox().getYMax()+1 < terminal_height){
                    pointer.set_position(pointer.getBoundingBox().getXMin(), pointer.getBoundingBox().getYMin()+1);
                }
                break;
            case static_cast<char>(Keys::LEFT_ARROW):
            case 'a':
                if(pointer.getBoundingBox().getXMin()-1 > 0){
                    pointer.set_position(pointer.getBoundingBox().getXMin()-1, pointer.getBoundingBox().getYMin());
                }
                break;
            case static_cast<char>(Keys::RIGHT_ARROW):
            case 'd':
                if(pointer.getBoundingBox().getXMax()+1 < terminal_width){
                    pointer.set_position(pointer.getBoundingBox().getXMin()+1, pointer.getBoundingBox().getYMin());
                }
                break;
            case 10: // Enter key
                std::cout << "selectable players:\n";
                for(auto it : selectable_players){
                    std::cout << it;
                }
                std::cout << std::endl;
                std::cout <<"pointer on piece: " << is_pointer_on_piece << std::endl;
                if(is_pointer_on_piece){
                    if(game_clients[selected_target]->username == game_clients[player_index]->username){
                        Client_terminal::move_cursor(client_socket, 1, 1);
                        std::string msg = "Cannot select self as target\n";
                        std::cout << msg << std::endl;
                        Server::send_message(client_socket, msg);
                    }
                    else if(selectable_players[selected_target]){
                        running = false;
                    }
                    else{
                        std::cout << "Cannot select " << live_clients[selected_target]->username << std::endl;
                    }
                }
                break;
            default:
                break;
        }
        //std::cout << "Selected row: " << selected_row << std::endl;
    }
    Client_terminal::clear_screen(client_socket);
    return selected_target;
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

// thread to receive chat messages in a loop during the discussion phase
// void Game::chat(int client_socket, Room * room) {
//     std::string message;
//     while (true) {
//         Server::receive_line(client_socket, message, ECHO_ON);
//         std::cout << "Received message: " << message << std::endl;
//         // Broadcast the message to all clients in the room
//         for(Client *client : room->clients) {
//             send(client->socket, message.c_str(), message.size(), 0);
//         }
//         // Clear the message buffer
//         message.clear();
//     }
// }
std::mutex defense_mutex;
void Game::handle_client_defense(Client* client, std::string &list_of_defenses) {
    std::string message = client->username + ", state your defense: \n";
    send(client->socket, message.c_str(), message.size(), 0);
    std::string defense;
    Server::receive_line(client->socket, defense, ECHO_ON);
    defense_mutex.lock();
    list_of_defenses += client->username + ": " + defense + "\n";
    defense_mutex.unlock();
                        
}

void Game::start_game(Room * room, sqlite3* db) {
    // Implementation needed
    std::cout << "Starting game..." << std::endl;
    // Add your game logic here
    std::string message = "Game started!\n";
    int counter = 1;
    for (Client *client : room->clients) {
        send(client->socket, message.c_str(), message.size(), 0);
        live_clients.push_back(client);
        game_clients.push_back(client);
        is_alive.push_back(true);
        SetPiece* piece = new SetPiece(live_house_art);
        int terminal_height, terminal_width;
        Client_terminal::get_terminal_size(terminal_width, terminal_height);

        int start_y = terminal_height / 2 - piece->getHeight()/2;
        int start_x = counter * (2 * piece->getWidth());
        piece->set_position(start_x, start_y);
        player_pieces.push_back(piece);
        Database::increment_games_played(db, client->username);
        counter++;
    }
    counter = 0;
    num_players = live_clients.size();
    num_remaining_players = num_players;
    int random_werewolf = rand() % num_players;
    int random_engineer = rand() % num_players;
    //random seer exluding engineer and werewolf
    std::cout << "selecting werewolf" << std::endl;
    std::cout << "num players: " << num_players << std::endl;
    while(random_engineer == random_werewolf || random_engineer == random_werewolf){
        random_engineer = rand() % num_players;
    }
    std::cout << "selected engineer" << std::endl;
    int random_seer = rand() % num_players;
    //random seer exluding engineer and werewolf
    while(random_seer == random_werewolf || random_seer == random_engineer){
        random_seer = rand() % num_players;
    }
    std::cout << "selected seer" << std::endl;
    werewolf_index = random_werewolf;
    engineer_index = random_engineer;
    seer_index = random_seer;
    werewolf_client = live_clients[random_werewolf];
    engineer_client = live_clients[random_engineer];
    seer_client = live_clients[random_seer];
    Database::increment_games_as_werewolf(db, werewolf_client->username);
    is_night_cycle = true;
    //bool is_running = true;
    //std::cout << "is game running: " << room->is_room_game_running << std::endl;
    while (room->is_room_game_running) {
            {
                // Here you can update game state based on player actions
                // For example, broadcast game state to all players in the room
                if(is_night_cycle){
                    is_night_cycle = false;
                    std::string message = "It is night time, all players go to sleep!\n";
                    for (Client *client : room->clients) {                  
                        send(client->socket, message.c_str(), message.size(), 0);
                    }
                    message = "After tossing and turning, you wake up, choose your target index for the night's action.\n";
                    int count = 1;
                    for(Client *client : live_clients) {
                        if(client != werewolf_client){
                            message += "["+std::to_string(count) + "] " + (client->username) + "\n";
                        }
                        count++;
                    }
                    bool boo = true;
                    int target_index = 0;
                    target_index = select_target(werewolf_client->socket, werewolf_index, is_alive);
                    // while(boo){
                    //     send(werewolf_client->socket, message.c_str(), message.size(), 0);
                    //     //char c;
                    //     std::string target;
                    //     Server::receive_line(werewolf_client->socket, target, ECHO_ON);

                    //     std::cout<<"target: "<< target << std::endl;        
                    //     try{
                    //         target_index = stoi(target) - 1;
                    //         if(target_index < num_remaining_players && target_index >= 0){
                    //             boo = false;
                    //         }
                    //         else{
                    //             message = "Invalid target index, please try again.\n";
                    //             send(werewolf_client->socket, message.c_str(), message.size(), 0);
                    //         }
                    //     }
                    //     catch(std::invalid_argument& e){
                    //         std::cout<<"invalid argument"<<std::endl;
                    //     }
                    // }
                    Client * uni_target_client = game_clients[target_index];
                    Client * target_client;
                    for(Client * client : live_clients){
                        if(client->username == uni_target_client->username){
                            target_client = client;
                        }
                    }
                    message = "You chose " + target_client->username + " as your target.\n";
                    //printf("target_client: %s\n", target_client->username.c_str());
                    send(werewolf_client->socket, message.c_str(), message.size(), 0);
                    message = "You have been killed by the werewolf!\n";
                    std::vector<std::string> dead_house_art_copy = dead_house_art;
                    player_pieces[target_index]->setImage(dead_house_art_copy);
                    send(target_client->socket, message.c_str(), message.size(), 0);
                    last_killed_client = target_client;
                    printf("last_killed_client: %s\n", last_killed_client->username.c_str());
                    live_clients.erase(
                        std::remove_if(live_clients.begin(), live_clients.end(),
                            [target_client](const Client* client) { return client == target_client; }
                        ),
                        live_clients.end()
                    );
                    is_alive[target_index] = false;
                    //live_clients.erase(target_index);
                    for(auto it : live_clients){
                        if(it != target_client){
                            message = "The werewolf has killed " + target_client->username + "!\n";
                            send(it->socket, message.c_str(), message.size(), 0);
                        }
                        else{
                            message = "You have been killed by the werewolf!\n";
                            send(it->socket, message.c_str(), message.size(), 0);
                        }
                    }
                    
                    num_remaining_players--;

                    if(num_remaining_players<=2){
                            message = "Game over! The werewolf wins!";
                            for(Client *client : room->clients) {
                                send(client->socket, message.c_str(), message.size(), 0);
                            }
                            Database::increment_wins(db, werewolf_client->username);
                            Database::increment_wins_as_werewolf(db, werewolf_client->username);
                            stop_game(room);
                        }
                }
                else{
                    is_night_cycle = true;
                    std::string message = "It is day time, everyone wake up!\n";
                    for (Client *client : live_clients) {
                        send(client->socket, message.c_str(), message.size(), 0);
                    }
                    if(last_killed_client != nullptr){                   
                        message = last_killed_client->username + " has been killed by the werewolf!\n";
                        std::cout << message << std::endl;
                        //printf("%s has been killed by the werewolf\n", (last_killed_client->username.c_str()));
                        for (Client *client : room->clients) {
                            send(client->socket, message.c_str(), message.size(), 0);
                        }
                        last_killed_client = nullptr;
                        std::string list_of_defenses;
                        // for(Client *client : live_clients){
                        //     message = client->username + ", state your defense: \n";
                        //     send(client->socket, message.c_str(), message.size(), 0);
                        //     std::string defense;
                        //     Server::receive_line(client->socket, defense, ECHO_ON);
                        //     list_of_defenses += client->username + ": " + defense + "\n";
                        // }
                        //multithread list of defenses
                        std::vector<std::thread> defense_threads;   
                        // Spawn a thread for each client
                        for (Client* client : live_clients) {
                            defense_threads.push_back(std::thread(handle_client_defense, client, std::ref(list_of_defenses)));
                        }

                        // Join all threads to ensure they complete before moving forward
                        for (std::thread& t : defense_threads) {
                            t.join();
                        }

                        list_of_defenses += "Time to vote! Who do you think is the werewolf?\nEnter the index of the player\n";
                        int count = 1;
                        for(Client *client : live_clients) {
                            list_of_defenses += "["+std::to_string(count) + "] " + (client->username) + "\n";                            
                            count++;
                        }
                        int * votes = (int *)malloc(num_remaining_players * sizeof(int));
                        //memset(votes, 0, num_remaining_players * sizeof(int));
                        for(int i = 0; i < num_remaining_players; i++){
                            printf("votes[%d]: %d\n",i,votes[i]);
                            votes[i] = 0;
                        }
                        for(Client *client : live_clients){
                            send(client->socket, list_of_defenses.c_str(), list_of_defenses.size(), 0);
                            bool boo = true;
                            int target_index = 0; 
                            while(boo){
                                std::string target;
                                Server::receive_line(client->socket, target, ECHO_ON);
                                std::cout<<"target: "<< target << std::endl;        
                                try{
                                    target_index = stoi(target) - 1;
                                    if(target_index < num_remaining_players && target_index >= 0){
                                        votes[target_index]++;
                                        boo = false;
                                    }
                                    else{
                                        message = "Invalid target index, please try again.\n";
                                        send(werewolf_client->socket, message.c_str(), message.size(), 0);
                                    }
                                }
                                catch(std::invalid_argument& e){
                                    std::cout<<"invalid argument"<<std::endl;
                                }
                            }
                        }
                        
                        int max = votes[0];
                        int kick_index = 0;
                        bool tie = false;
                        for(int i = 1; i < num_remaining_players; i++){
                            printf("votes[%d]: %d\n",i,votes[i]);
                            if(votes[i] > max){
                                max = votes[i];
                                kick_index = i;
                                tie = false;
                            }
                            else if(votes[i] == max){
                                tie = true;
                            }
                        }
                        printf("kick_index:%d\n",kick_index);
                        Client * kick_client = live_clients[kick_index];
                        if(tie){
                            message = "There was a tie, no one was kicked!\n";
                        }
                        else{
                            message = kick_client->username + " has been kicked!\n";
                            live_clients.erase(std::remove(live_clients.begin(), live_clients.end(), kick_client), live_clients.end());
                            num_remaining_players--;
                        }
                        for(Client *client : room->clients) {
                            send(client->socket, message.c_str(), message.size(), 0);
                        }

                        printf("kick_client name: %s\n", kick_client->username.c_str());
                        printf("werewolf_client name: %s\n", werewolf_client->username.c_str());
                        if(kick_client == werewolf_client){
                            message = "The werewolf has been kicked! Villagers win!\n";
                            for(Client *client : room->clients) {
                                send(client->socket, message.c_str(), message.size(), 0);
                                std::cout << "comparing " << client->username << " to " << werewolf_client->username << std::endl;
                                if(client != werewolf_client){
                                    std::cout << "incrementing wins for " << client->username << std::endl;
                                    Database::increment_wins(db, client->username);
                                }
                            }
                            //werewolf_client = nullptr;
                            stop_game(room);
                        }
                        if(num_remaining_players<=2){
                            message = "Game over! The werewolf wins!\n";
                            for(Client *client : room->clients) {
                                send(client->socket, message.c_str(), message.size(), 0);
                            }
                            Database::increment_wins(db, werewolf_client->username);
                            Database::increment_wins_as_werewolf(db, werewolf_client->username);
                            stop_game(room);
                        }
                        else{
                            message = "You have killed a compatriot!\n";
                        }
                        for(Client *client : room->clients) {
                            send(client->socket, message.c_str(), message.size(), 0);
                        }

                    }
                }
                // for (Client *client : clients) {
                //     std::string message = "Game state updated!\n";
                    


                //     send(client->socket, message.c_str(), message.size(), 0);
                // }
            }

            // Sleep for a short period to simulate game ticks
            //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

}
void Game::host_game(Client *client, std::vector<Room*> &rooms, sqlite3 * db) {   
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
    std::cout << "Available rooms: " << rooms.size() << std::endl;
        for (const auto& room : rooms) {
            std::cout << "Room name: " << room->name << std::endl;
        }
    Client_terminal::clear_screen(client->socket);
    Client_terminal::move_cursor(client->socket, 0, 0);
    std::string message = "You are the host of " + room_name + "!\ntype start to begin the game\n";
    Client_terminal::print(client->socket, message);
    
    bool exit_loop = false;
    while(!exit_loop){
        std::string input;
        Server::receive_line(client->socket, input, ECHO_ON);
        if(input == "start"){
            if(room->clients.size() >= 4){
                room->room_mutex.lock();
                room->is_room_game_running = true;
                room->room_mutex.unlock();
                exit_loop = true;
                break;
            }
        }
        else{
                std::string message = "You need at least 4 players to start the game!\nType \'start\' to start the game\n";
                Client_terminal::print(client->socket, message);
        }
    }
    Game::start_game(room, db);
    //remove room from rooms
    rooms.erase(std::remove(rooms.begin(), rooms.end(), room), rooms.end());
    //free room
    //free(room);
    delete room;
}

void Game::join_game(Client *client, std::vector<Room*> &rooms) {
    Room *join_room = nullptr;
    Client_terminal::clear_screen(client->socket);
    int terminal_width, terminal_height;
    Client_terminal::get_terminal_size(terminal_width, terminal_height);
    bool in_room = false;
    std::string room_name;
    std::cout << "Available rooms: " << rooms.size() << std::endl;
    for (const auto& room : rooms) {
        std::cout << "Room name: " << room->name << std::endl;
    }
    while(!in_room){
        std::string prompt = "Enter room name: ";
        int startY = (terminal_height - 1) / 2;
        int startX = (terminal_width - 1) / 2 - prompt.size();
        bool is_attempted_room_running = false;
        bool is_room_full = false;
        Client_terminal::move_cursor(client->socket, startX, startY);
        Client_terminal::print(client->socket, prompt);
        Server::receive_line(client->socket, room_name, ECHO_ON);
        for(long unsigned int i = 0; i < rooms.size(); i++){
            if(rooms[i]->name == room_name){
                if(!rooms[i]->is_room_game_running){
                    if(rooms[i]->clients.size() < 8){
                        join_room = rooms[i];
                        Client_terminal::clear_screen(client->socket);
                        Client_terminal::move_cursor(client->socket, 0, 0);
                        std::string message = "You have joined " + room_name + "!\n";
                        Client_terminal::print(client->socket, message);

                        rooms[i]->room_mutex.lock();
                        rooms[i]->add_player(client);
                        rooms[i]->room_mutex.unlock();
                        
                        for(auto player : rooms[i]->clients){
                            if(player->username != client->username){
                                std::string message = client->username + " has joined the game\n";
                                Client_terminal::print(player->socket, message);
                                message = player->username + " is in the room\n";
                                Client_terminal::print(client->socket, message);
                            }
                            
                        }
                        in_room = true;
                        break;
                    }
                    else{
                        is_room_full = true;
                    }
                }
                else{
                    is_attempted_room_running = true;
                }
            }

        }
        if(!in_room){
            Client_terminal::clear_screen(client->socket);
            Client_terminal::move_cursor(client->socket, 0, 0);
            std::string message = "Room " + room_name + " not found.\nPress esc or q to exit.\n";
            if(is_attempted_room_running){
                message = "Room " + room_name + " is already running a game.\nPress esc or q to exit.\n";
            }
            if(is_room_full){
                message = "Room " + room_name + " is full.\nPress esc or q to exit.\n";
            }
            Client_terminal::print(client->socket, message);
            char c;
            ssize_t bytes_received;
            bool do_exit = false;
            while(!do_exit){
                bytes_received = recv(client->socket, &c, 1, 0);
                if(bytes_received > 0){
                    if(c == 'q' || c == static_cast<char>(Keys::ESC)){
                        do_exit = true;
                        return;
                    }
                }
            }
        }
        bool do_quit = false;
        bool was_running = false;
        while(!do_quit){
            std::string input;
            //Server::receive_line(client->socket, input, ECHO_ON);
            if(input == "quit"){
                do_quit = true;
            }
            if(was_running && !join_room->is_room_game_running){
                do_quit = true;
            }
            if(!was_running){
                join_room->room_mutex.lock();
                was_running = join_room->is_room_game_running;
                join_room->room_mutex.unlock();
            }
        }
    }

}

void Game::stop_game(Room * room) {
    // provide time for the game over scree to be displayed
    std::this_thread::sleep_for(std::chrono::seconds(3));
    room->room_mutex.lock();
    room->is_room_game_running = false;
    room->room_mutex.unlock();
    //remove all players from room
    room->clients.clear();
    //for(auto player : room->clients){
    //    room->remove_player(player);
    //}
    
}
