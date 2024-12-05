#include <iostream>
#include <fcntl.h>     // for O_NONBLOCK
#include <cstring>      // for memset
#include <sys/socket.h> // for socket functions
#include <arpa/inet.h>  // for sockaddr_in and inet_addr
#include <unistd.h>     // for close
#include <string>
#include <thread>
#include <algorithm>
#include <mutex>
#include "server.h"
#include "client_terminal.h"
#include "database.h"
#include "game.h"
#include "room.h"

sqlite3 *db;
Game uni_werewolf_game;
std::vector<Room *> rooms;
std::mutex server_mutex;
std::vector<Client *> clients;

// Function to enable character mode (and suppress go-ahead)
void Server::enableCharacterMode(int client_socket) {
    // Telnet command to suppress "Go Ahead"
    std::uint8_t char_mode_command[] = {IAC, WILL, SUPPRESS_GO_AHEAD};

    // Send command to the client
    int bytes_sent = send(client_socket, char_mode_command, sizeof(char_mode_command), 0);
    
    if (bytes_sent < 0) {
        std::cerr << "Failed to send character mode command." << std::endl;
    } else {
        std::cout << "Character mode enabled (Suppress Go-Ahead)." << std::endl;
    }
}

// Function to send a message to a client
void Server::send_message(int client_socket, const std::string &message) {
    send(client_socket, message.c_str(), message.size(), 0);
}

//Function to divide inputs by newline
void Server::receive_line(int client_socket, std::string& user_input, bool echo_enabled) {
    //user_input.clear();
    user_input = "";
    // Buffer for storing received data
    // char buffer[1024];
    // int bytes_received;
    bool in_line = true;
    char c;
    Client_terminal::save_cursor_position(client_socket);
    // Loop to receive data from the client
    //while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0 && in_line) {
    
    //logging
    std::cout << "Starting receive_line for client " << client_socket << std::endl;


    while(in_line){
            ssize_t bytes_received = recv(client_socket, &c, 1, 0);
            if (bytes_received <=0) {
            if (bytes_received == 0) {
                // Client disconnected cleanly
                throw std::runtime_error("Client disconnected");
            } else {
                // Error occurred
                throw std::runtime_error("Connection error");
            }
        }
            if(c == '\0'){
                std::cout << "Received null byte" << std::endl;
                continue;
            }
            //Server::blocking_receive(client_socket, &c, 1);
            //for (int i = 0; i < bytes_received; ++i) {
            //char c = buffer[i];

            // Check if the byte is part of a Telnet control sequence (IAC)
            if ((unsigned char)c == IAC) {
                // Handle Telnet control sequences
                unsigned char command[2];
                recv(client_socket, &command[0], 1, 0);  // Read the option (WILL, DO, etc.)
                recv(client_socket, &command[1], 1, 0);  // Read the actual option (like ECHO, etc.)

                std::cout << "Telnet command received: IAC " << (int)command[0] << " " << (int)command[1] << std::endl;

                // Ignore or handle the Telnet negotiation options here if needed
                continue;
            }

            // Check for newline or carriage return characters
            if (c == '\n' || c == '\r') {
                // Process the input when newline or carriage return is detected
                in_line = false;
                //clear the line
                Client_terminal::erase_user_input(client_socket);
                if(echo_enabled){
                    send_message(client_socket, user_input);
                    send_message(client_socket, "\n");
                }
                else{
                    send_message(client_socket, "\n");
                }
            }
            else if (c == '\b' || c == 127) {
                std::cout << "Received backspace." << std::endl;
                // Handle backspace
                if (!user_input.empty()) {
                    user_input.pop_back();
                    send(client_socket, "\b \b", 1, 0);
                }

                //clear the line
                Client_terminal::erase_user_input(client_socket);
                if(echo_enabled){
                    send_message(client_socket, user_input);
                }
            }
            else {
                // Append the character to the user input
                user_input += c;
                std::cout << "Received input: " << user_input << std::endl;
                if(echo_enabled){
                    Client_terminal::erase_user_input(client_socket);
                    send_message(client_socket, user_input);
                }
                else{
                    Client_terminal::erase_user_input(client_socket);
                }
            }
        //}
    }
    if (c < 0) {
        std::cerr << "Failed to receive data." << std::endl;
    }
}

//Function to start up the server
int Server::start_server(int& server_socket, struct sockaddr_in& server_addr, struct sockaddr_in& client_addr, socklen_t& addr_len){
    //Server server;
    addr_len = sizeof(client_addr);

    // Create a socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        return -1;
    }
    // Set SO_REUSEADDR to allow quick rebinding
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        std::cerr << "Failed to set socket option\n";
        return 1;
    }

    // Set the socket to non-blocking mode
    // int flags = fcntl(server_socket, F_GETFL, 0);
    // if (flags == -1) {
    //     std::cerr << "Failed to get socket flags" << std::endl;
    //     close(server_socket);
    //     return -1;
    // }
    // if (fcntl(server_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
    //     std::cerr << "Failed to set non-blocking mode" << std::endl;
    //     close(server_socket);
    //     return -1;
    // }

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080); // Telnet runs on port 23

    // Bind the socket to the port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed." << std::endl;
        close(server_socket);
        return -1;
    }
    return 1;
}

//function to perform a blocking receive in non-blocking mode
ssize_t Server::blocking_receive(int socket_fd, char *buffer, size_t buffer_size) {
    fd_set read_fds;
    ssize_t bytes_received;

    while (true) {
        // Clear and set up the file descriptor set for `select`
        FD_ZERO(&read_fds);
        FD_SET(socket_fd, &read_fds);

        // Wait until the socket is ready for reading
        int select_result = select(socket_fd + 1, &read_fds, nullptr, nullptr, nullptr);
        if (select_result < 0) {
            perror("Select error");
            return -1;
        } else if (FD_ISSET(socket_fd, &read_fds)) {
            // Attempt to receive data
            bytes_received = recv(socket_fd, buffer, buffer_size - 1, 0);
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';  // Null-terminate the buffer
                return bytes_received;
            } else if (bytes_received == 0) {
                std::cout << "Client disconnected.\n";
                return 0;  // Client disconnected
            } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("Receive error");
                return -1;  // An actual error occurred
            }
        }
    }
}

// Function to register a new player
bool Server::register_player(const std::string &username, const std::string &password) {
    return Database::insert_user(db, username, password);
}

// Function to authenticate a player
bool Server::authenticate_player(const std::string &username, const std::string &password) {
    return Database::authenticate_user(db, username, password);
}

// Function to sign in a player
bool Server::sign_in(Client * client) {
    std::string username, password;
    bool signed_in = false;
    bool user_already_signed_in = false;
    while(!signed_in){
    // Registration or Login
        user_already_signed_in = false;
        bool boo = true;
        std::string action;
        int doing_register = false;
        while(boo){
            send_message(client->socket, "register? (1) or login (0): ");
            
            //logging
            std::cout << "Client " << client->socket << " attempting to receive data" << std::endl;
            //recv(client->socket, &action, 1, 0);
            Server::receive_line(client->socket, action, ECHO_ON);
            
            try{
                doing_register = stoi(action);
                boo = false;
            }
            catch(std::invalid_argument& e){
                std::cout<<"invalid argument"<<std::endl;
                //continue;
            }
        }
        // Username and Password
        send_message(client->socket, "Enter username: ");
        Server::receive_line(client->socket, client->username, ECHO_ON);  
        std::cout << "username string length: " << client->username.length() << std::endl;
for(size_t i = 0; i < client->username.length(); i++) {
    std::cout << "char at " << i << ": " << (int)client->username[i] << std::endl;
}         
        std::cout<<"username: "<< client->username << "check" << std::endl;
        send_message(client->socket, "Enter password: ");
        Server::receive_line(client->socket, password, ECHO_OFF);
        std::cout << "password string length: " << password.length() << std::endl;
for(size_t i = 0; i < password.length(); i++) {
    std::cout << "char at " << i << ": " << (int)password[i] << std::endl;
}         
        std::cout<<"password: "<< password << "check" << std::endl;
        //check if user is already logged in
        for(auto& client_iter : clients) {
            if(client->username == client_iter->username){
                send_message(client->socket, "Username already exists.\n");
                user_already_signed_in = true;
            }
        }
        std::cout << "user_already_signed_in: " << user_already_signed_in << std::endl;
        if(!user_already_signed_in){
            if (doing_register){//action == "register") {
            //double enter password
                std::string password2;
                send_message(client->socket, "Enter password again: ");
                Server::receive_line(client->socket, password2, ECHO_OFF);
                if(password != password2){
                    send_message(client->socket, "Passwords do not match.\n");
                    continue;
                }
                if (register_player(client->username, password)) {
                    send_message(client->socket, "Registration successful.\n");
                    signed_in = true;
                    clients.push_back(client);
                    return true;
                } else {
                    send_message(client->socket, "Username already exists.\n");
                }
            } else {//if (action == "login") {
                if (authenticate_player(client->username, password)) {
                    send_message(client->socket, "Login successful.\n");
                    clients.push_back(client);
                    signed_in = true;
                    //Database::increment_games_played(db, client->username);
                    return true;
                } else {
                    send_message(client->socket, "Login failed.\n");
                }
            }   
        }
    } 
    return false; 
}

// Function to handle a client connection
void Server::handle_client(Client *client) {
    try{
        Server::enableCharacterMode(client->socket);
        std::string message;
        //bool signed_in = false;
        Server::sign_in(client);
        bool exit_game = false;
        
        while(!exit_game){
            Game werewolf_game;
            int menu_selection = werewolf_game.select_from_menu(client->socket);

            GameMenu selected_option = static_cast<GameMenu>(menu_selection);
            switch (selected_option) {
                case GameMenu::HOST_GAME:
                    std::cout << "Host game selected" << std::endl;
                    werewolf_game.host_game(client, rooms, db);
                    break;
                case GameMenu::JOIN_GAME:
                    std::cout << "Join game selected" << std::endl;
                    werewolf_game.join_game(client, rooms);
                    break;
                case GameMenu::RULES:
                    std::cout << "Rules selected" << std::endl;
                    werewolf_game.rules(client->socket);
                    break;
                case GameMenu::STATS:
                    std::cout << "Stats selected" << std::endl;
                    werewolf_game.stats(db, client->socket, client->username);
                    break;
                case GameMenu::EXIT:
                    exit_game = true;
                    break;
                default:
                    std::cout << "Invalid option selected" << std::endl;
                    break;
            }
        }
    }
    catch (const std::runtime_error& e) {
        std::cout << "Client " << client->socket << " disconnected: " << e.what() << std::endl;
    }
    catch (...) {
        std::cout << "Unexpected error handling client " << client->socket << std::endl;
    }
    // Close the client socket
    close(client->socket);
    delete client;
    std::cout << "Client disconnected" << std::endl;

}

int main() {

    //initialize the database
    Database::init_db(db);
    std::cout << "Database initialization status: " << (db != nullptr) << std::endl;


    //start the server and initialize server values
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    Server::start_server(server_socket, server_addr, client_addr, addr_len);

    // Listen for incoming connections
    listen(server_socket, 3);

    std::cout << "Waiting for incoming Telnet connection..." << std::endl;

    // Accept an incoming connection
    // client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
    // if (client_socket < 0) {
    //     std::cerr << "Accept failed." << std::endl;
    //     close(server_socket);
    //     return -1;
    // }

    //std::cout << "Connection accepted." << std::endl;

    //Server::send_message(client_socket, "---\n");
    // Enable character mode
    //Server::enableCharacterMode(client_socket);
    
    //std::cout <<"client in character mode" << std::endl;

    // std::string user_input;

    // //receive a name
    // send(client_socket, "Enter name: ", sizeof("Enter name: "), 0);
    // std::cout << "Waiting for name..." << std::endl;
    // Server::receive_line(client_socket, user_input, ECHO_ON);
    // std::cout << "Received name: " << user_input << std::endl;

    // //disable echo on the client side
    // //disableEcho(client_socket);
    // //receive a password with client side echo disabled
    // send(client_socket, "Enter password: ", sizeof("Enter password: "), 0);
    // Server::receive_line(client_socket, user_input, ECHO_OFF);
    // std::cout << "Received password: " << user_input << std::endl;

    // //enable echo on the client side
    // //enableEcho(client_socket);
    // //receive a message with client side echo enabled
    // send(client_socket, "Enter message: ", sizeof("Enter message: "), 0);   
    // Server::receive_line(client_socket, user_input, ECHO_ON);
    // std::cout << "Received message: " << user_input << std::endl;


    while (true){//!server_shutdown) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int activity = select(server_socket + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0 && errno != EINTR) {
            std::cerr << "Select error" << std::endl;
        }

        if (activity > 0 && FD_ISSET(server_socket, &readfds)) {
            sockaddr_in client_addr;
            socklen_t client_addr_size = sizeof(client_addr);
            int client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_addr_size);

            if (client_socket == -1) {
                std::cerr << "Failed to accept connection." << std::endl;
                continue;
            }

            Client *client = new Client();
            client->socket = client_socket;

            std::thread(Server::handle_client, client).detach();
        }
    }

    //quit_thread.join();

    std::cout << "Server shutting down..." << std::endl;
    close(server_socket);
    sqlite3_close(db);
    // Close the client socket
    close(client_socket);

    // Close the server socket
    close(server_socket);

    return 0;
}
