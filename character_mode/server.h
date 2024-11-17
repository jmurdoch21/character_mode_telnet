#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "client.h"
#include <string>
// Telnet control codes
#define IAC 255             // Interpret As Command
#define WILL 251            // Will perform an option
#define DO 253              // Do option
#define SUPPRESS_GO_AHEAD 3 // Suppress go-ahead option
#define WONT 252    // WONT command in Telnet
#define ECHO 1      // ECHO option in Telnet
#define ECHO_OFF 0     // Echo off
#define ECHO_ON 1        // Echo on

class Game;

class Server{
    public:
    int server_socket;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t addr_len;
    // Server();
    // ~Server();
    // void init();
    // void accept_connection();


    // Function to enable character mode (and suppress go-ahead)
    static void enableCharacterMode(int client_socket);

    // Function to start up the server
    static int start_server(int& server_socket, struct sockaddr_in& server_addr, struct sockaddr_in& client_addr, socklen_t& addr_len);

    // Function to perform a blocking receive in non-blocking mode
    static ssize_t blocking_receive(int socket_fd, char *buffer, size_t buffer_size);

    // Function to register a player
    static bool register_player(const std::string &username, const std::string &password);
    // Function to authenticate a player
    static bool authenticate_player(const std::string &username, const std::string &password);
    // Function to sign in a player
    static bool sign_in(Client * client);
    // Function to handle a client connection
    static void handle_client(Client *client);

    // Function to disable echo on the client side
    static void disableEcho(int client_socket);

    // Function to disable echo on the client side
    static void enableEcho(int client_socket);

    // Function to send a message to a client
    static void send_message(int client_socket, const std::string &message);

    //Function to divide inputs by newline
    static void receive_line(int client_socket, std::string& user_input, bool echo_enabled);
};
#endif