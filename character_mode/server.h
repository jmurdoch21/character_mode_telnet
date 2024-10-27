// Telnet control codes
#define IAC 255             // Interpret As Command
#define WILL 251            // Will perform an option
#define DO 253              // Do option
#define SUPPRESS_GO_AHEAD 3 // Suppress go-ahead option
#define WONT 252    // WONT command in Telnet
#define ECHO 1      // ECHO option in Telnet
#define ECHO_OFF 0     // Echo off
#define ECHO_ON 1        // Echo on

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

    // Function to disable echo on the client side
    static void disableEcho(int client_socket);

    // Function to disable echo on the client side
    static void enableEcho(int client_socket);

    // Function to send a message to a client
    static void send_message(int client_socket, const std::string &message);

    static void erase_user_input(int client_socket);

    static void save_cursor_position(int client_socket);

    static void clear_line(int client_socket);

    //Function to divide inputs by newline
    static void receive_line(int client_socket, std::string& user_input, bool echo_enabled);
};