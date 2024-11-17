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
Room::Room(std::string name) : name(name), clients() {}
Room::Room(std::string name, std::vector<Client *> clients) : name(name), clients(clients) {}
Room::~Room() {}
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