// #pragma once
// #include <iostream>
// #include <vector>
// #include <thread>
// #include "client.h"
// #include "game.h"

// class Room {
//     public:
//     std::string name;
//     std::vector<Client *> clients;
//     std::unique_ptr<Game> game;
//     std::thread game_thread;
//     Client* host;
//     bool game_started;

//     Room() : game(std::make_unique<Game>()),host(nullptr), game_started(false){}
    
//     // Function to start the game for this room
//     void start_game();

//     // Function to stop the game
//     void stop_game();

//     bool isEmpty();

//     ~Room() {
//         stop_game();
//     }
// };

#ifndef ROOM_H
#define ROOM_H
#include <string>
#include <vector>
#include "client.h"
class Room {
    public:
    std::string name;
    std::vector<Client *> clients;

    Room();
    ~Room();
    Room(const Room &other);
    Room& operator=(const Room& other);
    Room(std::string name);
    Room(std::string name, std::vector<Client *> clients);

    int add_player(Client *client);
};
#endif