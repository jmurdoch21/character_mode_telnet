#include <iostream>
#include <cstring>      // for memset
#include <sys/socket.h> // for socket functions
#include <arpa/inet.h>  // for sockaddr_in and inet_addr
#include <unistd.h>     // for close
#include <string>
#include "server.h"

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
// Function to disable echo on the client side
void Server::disableEcho(int client_socket) {
    // Telnet command to disable echo: IAC WONT ECHO
    std::uint8_t disable_echo_command[] = {IAC, DO, ECHO};

    // Send the command to the client
    int bytes_sent = send(client_socket, disable_echo_command, sizeof(disable_echo_command), 0);
    if (bytes_sent < 0) {
        std::cerr << "Failed to send disable echo command." << std::endl;
    } else {
        std::cout << "Echo disabled." << std::endl;
    }
}
// Function to disable echo on the client side
void Server::enableEcho(int client_socket) {
    // Telnet command to disable echo: IAC WONT ECHO
    std::uint8_t enable_echo_command[] = {IAC, DO, ECHO};

    // Send the command to the client
    int bytes_sent = send(client_socket, enable_echo_command, sizeof(enable_echo_command), 0);
    if (bytes_sent < 0) {
        std::cerr << "Failed to send enable echo command." << std::endl;
    } else {
        std::cout << "Echo enabled." << std::endl;
    }
}

// Function to send a message to a client
void Server::send_message(int client_socket, const std::string &message) {
    send(client_socket, message.c_str(), message.size(), 0);
}

//functions for mamipulating the client's terminal
void Server::erase_user_input(int client_socket){
    std::string erase_tail = "\033[u\033[K";  // ANSI escape code to clear a trailing line and move cursor to last saved position
    send(client_socket, erase_tail.c_str(), erase_tail.size(), 0);
}
void Server::save_cursor_position(int client_socket){
    std::string save_cursor_position = "\033[s";  // ANSI escape code to save cursor position
    send(client_socket, save_cursor_position.c_str(), save_cursor_position.size(), 0);
}
void Server::clear_line(int client_socket){
    std::string clear_line = "\033[2K\r";  // ANSI escape code to clear the line and move cursor to start
    send(client_socket, clear_line.c_str(), clear_line.size(), 0);
}

//Function to divide inputs by newline
void Server::receive_line(int client_socket, std::string& user_input, bool echo_enabled) {
    user_input.clear();
    // Buffer for storing received data
    char buffer[1024];
    int bytes_received;
    bool in_line = true;
    char c;
    save_cursor_position(client_socket);
    // Loop to receive data from the client
    //while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0 && in_line) {
    while(in_line){
            recv(client_socket, &c, 1, 0);
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
            // Echo the character back to the client (if needed)
            // if(!echo_enabled){
            //     send(client_socket, "\b", 1, 0);
            //     send(client_socket, "*", 1, 0);
            // }

            // Check for newline or carriage return characters
            if (c == '\n' || c == '\r') {
                // Process the input when newline or carriage return is detected
                in_line = false;
                //clear the line
                //clear_line(client_socket);
                //std::string clear_line = "\033[2K\r";  // ANSI escape code to clear the line and move cursor to start
                //send(client_socket, clear_line.c_str(), clear_line.size(), 0);
                erase_user_input(client_socket);
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
                    //send(client_socket, "\b", 1, 0);
                    //send(client_socket, " ", 1, 0);
                    send(client_socket, "\b \b", 1, 0);
                }

                //clear the line
                //std::string clear_line = "\033[2K\r";  // ANSI escape code to clear the line and move cursor to start
                //send(client_socket, clear_line.c_str(), clear_line.size(), 0);
                //clear_line(client_socket);
                erase_user_input(client_socket);
                if(echo_enabled){
                    send_message(client_socket, user_input);
                }
            }
            else {
                // Append the character to the user input
                user_input += c;
                std::cout << "Received input: " << user_input << std::endl;
                if(echo_enabled){
                    //clear the line
                    //std::string clear_line = "\033[2K\r";  // ANSI escape code to clear the line and move cursor to start
                    //send(client_socket, clear_line.c_str(), clear_line.size(), 0);
                    //clear_line(client_socket);
                    erase_user_input(client_socket);
                    send_message(client_socket, user_input);
                }
                else{
                    //clear the line
                    //std::string clear_line = "\033[2K\r";  // ANSI escape code to clear the line and move cursor to start
                    //send(client_socket, clear_line.c_str(), clear_line.size(), 0);
                    //clear_line(client_socket);
                    erase_user_input(client_socket);
                }
            }
        //}
    }
    if (c < 0) {
        std::cerr << "Failed to receive data." << std::endl;
    }
}

int main() {
    //Server server;
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

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

    // Listen for incoming connections
    listen(server_socket, 3);

    std::cout << "Waiting for incoming Telnet connection..." << std::endl;

    // Accept an incoming connection
    client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
    if (client_socket < 0) {
        std::cerr << "Accept failed." << std::endl;
        close(server_socket);
        return -1;
    }

    std::cout << "Connection accepted." << std::endl;

    Server::send_message(client_socket, "---\n");
    // Enable character mode
    Server::enableCharacterMode(client_socket);
    
    std::cout <<"client in character mode" << std::endl;

    std::string user_input;

    //receive a name
    send(client_socket, "Enter name: ", sizeof("Enter name: "), 0);
    std::cout << "Waiting for name..." << std::endl;
    Server::receive_line(client_socket, user_input, ECHO_ON);
    std::cout << "Received name: " << user_input << std::endl;

    //disable echo on the client side
    //disableEcho(client_socket);
    //receive a password with client side echo disabled
    send(client_socket, "Enter password: ", sizeof("Enter password: "), 0);
    Server::receive_line(client_socket, user_input, ECHO_OFF);
    std::cout << "Received password: " << user_input << std::endl;

    //enable echo on the client side
    //enableEcho(client_socket);
    //receive a message with client side echo enabled
    send(client_socket, "Enter message: ", sizeof("Enter message: "), 0);   
    Server::receive_line(client_socket, user_input, ECHO_ON);
    std::cout << "Received message: " << user_input << std::endl;

    // Close the client socket
    close(client_socket);

    // Close the server socket
    close(server_socket);

    return 0;
}
