#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <sqlite3.h>
class Database {
    public:
        Database();
        ~Database();
        static std::string hash_password(const std::string& password);
        static void init_db(sqlite3 *&db);
        static bool insert_user(sqlite3 *db, const std::string& username, const std::string& password); //this function also hashes the password
        static bool authenticate_user(sqlite3 *db, const std::string& username, const std::string& password);
        static bool check_username_exists(sqlite3 *db, const std::string& username);
        static bool increment_wins(sqlite3 *db, const std::string& username);
        static bool increment_wins_as_werewolf(sqlite3 *db, const std::string& username);
        static bool increment_games_played(sqlite3 *db, const std::string& username);
        static bool increment_games_as_werewolf(sqlite3 *db, const std::string& username);

        static int get_num_wins(sqlite3 *db, const std::string& username);
        static int get_num_wins_as_werewolf(sqlite3 *db, const std::string& username);
        static int get_games_played(sqlite3 *db, const std::string& username);
        static int get_games_as_werewolf(sqlite3 *db, const std::string& username);

        static void close_database(sqlite3 *db);
        static void create_table(sqlite3 *db);

};
#endif // DATABASE_H