// #include <iostream>
// #include <vector>
// #include <thread>
// #include "room.h"
// #include "game.h"

// // Function to start the game for this room
// void Room::start_game() {
//     //game_thread = std::thread(&Game::some_function, game, std::ref(clients));

// //std::thread(&Game::start_game, game.get(), std::ref(clients));
// }

// // Function to stop the game
// void Room::stop_game() {
//     game->stop_game();
//     if (game_thread.joinable()) {
//         game_thread.join();
//     }
// }

// bool Room::isEmpty() {
//     return clients.size() == 0;
// }

#include "room.h"
#include "client.h"

Room::Room() : name(""), clients() {}
Room::Room(std::string name){//} : name(name), clients() {}
    this->name = name;
    this->clients = std::vector<Client *>();
    this->is_room_game_running = false;
}
Room::Room(std::string name, std::vector<Client *> clients) : name(name), clients(clients) {}
Room::~Room() {
    clients.clear();
}
Room::Room(const Room &other) : name(other.name), clients(other.clients) {}
Room& Room::operator=(const Room& other) {
    if (this != &other) {
        name = other.name;
        clients = other.clients;
        return *this;
    }
    return *this;
}
int Room::add_player(Client *client) {
    clients.push_back(client);
    return 1;
}
int Room::remove_player(Client *client) {
    for (std::vector<Client *>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (*it == client) {
            clients.erase(it);
            return 1;
        }
    }
    return 0;
}