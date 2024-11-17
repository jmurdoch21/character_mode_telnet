#ifndef ANSI_COMMANDS_H
#define ANSI_COMMANDS_H
#include <string>

// Define an enum for ANSI colors
enum class AnsiColor {
    BOLD = 1,
    ITALIC = 3,
    UNDERLINE = 4,
    BLINK = 5,
    INVERSE = 7,
    STRIKETHROUGH = 9,
    DEFAULT = 39,

    BLACK = 30,
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37,

    BRIGHT_BLACK = 90,
    BRIGHT_RED = 91,
    BRIGHT_GREEN = 92,
    BRIGHT_YELLOW = 93,
    BRIGHT_BLUE = 94,
    BRIGHT_MAGENTA = 95,
    BRIGHT_CYAN = 96,
    BRIGHT_WHITE = 97,
    RESET = 0
};

enum class Keys {
    UP_ARROW = 56,
    DOWN_ARROW = 50,
    RIGHT_ARROW = 54,
    LEFT_ARROW = 52,
    ENTER = 10,
    ESC = 27,
    SPACE = 32,
    BACKSPACE = 12
};

class  Ansi_commands {
    public:
        static void get_terminal_size(int &width, int &height);
        static void enable_raw_mode();
        static void disable_raw_mode();
        static std::string move_cursor_command(int x, int y);
        static std::string clear_screen_command();
        static std::string hide_cursor_command();
        static std::string show_cursor_command();
        static bool key_available();
        static char read_key();
        static void clear_key_buffer();
        static std::string get_ansi_code(AnsiColor color);
        static std::string print_color_command(const std::string& text, AnsiColor color = AnsiColor::RESET); //prints text in color color, defaulting to white
        static std::string print_bold_command(const std::string& text, AnsiColor color = AnsiColor::RESET); //prints text in bold in color color, defaulting to white
        static std::string print_command(const std::string& text, AnsiColor color = AnsiColor::RESET, bool use_bold = 0, bool use_underline = 0); //prints text in color color, defaulting to white
};

#endif // ANSI_COMMANDS_H