#include "database.h"
#include <sqlite3.h>
#include <iostream>
#include <openssl/sha.h>

std::string Database::hash_password(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char *>(password.c_str()), password.length(), hash);
    char buffer[65];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(buffer + (i * 2), "%02x", hash[i]);
    }
    buffer[64] = 0;
    return std::string(buffer);
}

void Database::init_db(sqlite3 *&db) {
    int rc = sqlite3_open("players.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        exit(1);
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS players ("
                      "id INTEGER PRIMARY KEY, "
                      "username TEXT UNIQUE, "
                      "hashed_password TEXT, "
                      "num_wins INTEGER, "
                      "wins_as_werewolf INTEGER, "
                      "games_played INTEGER, "
                      "games_as_werewolf INTEGER);";

    char *err_msg = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        sqlite3_close(db);
        exit(1);
    }
}

bool Database::insert_user(sqlite3* db, const std::string& username, const std::string& password) {
    // Add connection check
    std::cout << "Database pointer: " << db << std::endl;
    if (!db) {
        std::cout << "Database connection is null" << std::endl;
        return false;
    }

    std::string hashed_password = hash_password(password);
    std::string sql = "INSERT INTO players(username, hashed_password) VALUES(?, ?);";
    sqlite3_stmt *stmt;
    
    // Add error checking for prepare
    int prepare_rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    std::cout << "Prepare result: " << prepare_rc << " (" << sqlite3_errmsg(db) << ")" << std::endl;
    
    // Add error checking for bindings
    int bind1_rc = sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    int bind2_rc = sqlite3_bind_text(stmt, 2, hashed_password.c_str(), -1, SQLITE_STATIC);
    std::cout << "Bind results: " << bind1_rc << ", " << bind2_rc << std::endl;
    
    // Add detailed step result
    int rc = sqlite3_step(stmt);
    std::cout << "Step result: " << rc << " (" << sqlite3_errmsg(db) << ")" << std::endl;
    
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;

}

bool Database::authenticate_user(sqlite3* db, const std::string& username, const std::string& password) {
        std::string hashed_password = hash_password(password);
    
    // Print input values
    std::cout << "Authenticating user: " << username << std::endl;
    std::cout << "Hashed password: " << hashed_password << std::endl;
    
    std::string sql = "SELECT * FROM players WHERE username = ? AND hashed_password = ?;";
    sqlite3_stmt *stmt;
    
    // Check prepare status
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    std::cout << "Prepare result: " << rc << " (" << sqlite3_errmsg(db) << ")" << std::endl;
    
    // Add parameter binding with error checking
    int bind1_rc = sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    int bind2_rc = sqlite3_bind_text(stmt, 2, hashed_password.c_str(), -1, SQLITE_STATIC);
    std::cout << "Bind results: " << bind1_rc << ", " << bind2_rc << std::endl;
    
    // Check step result
    rc = sqlite3_step(stmt);
    std::cout << "Step result: " << rc << " (" << sqlite3_errmsg(db) << ")" << std::endl;
    
    bool authenticated = (rc == SQLITE_ROW);
    sqlite3_finalize(stmt);
    
    std::cout << "Authentication result: " << authenticated << std::endl;
    return authenticated;
}

bool Database::check_username_exists(sqlite3* db, const std::string& username) {
    std::string sql = "SELECT * FROM players WHERE username = '" + username + "';";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;  // SQL error
    }
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

bool Database::increment_wins(sqlite3* db, const std::string& username) {
    std::string sql = "UPDATE players SET num_wins = num_wins + 1 WHERE username = '" + username + "';";
    char *err_msg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        sqlite3_free(err_msg);
        return false;  // SQL error
    }
    return true;
}
bool Database::increment_wins_as_werewolf(sqlite3* db, const std::string& username) {
    std::string sql = "UPDATE players SET wins_as_werewolf = wins_as_werewolf + 1 WHERE username = '" + username + "';";
    char *err_msg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        sqlite3_free(err_msg);
        return false;  // SQL error
    }
    return true;
}
bool Database::increment_games_played(sqlite3* db, const std::string& username) {
    std::string sql = "UPDATE players SET games_played = games_played + 1 WHERE username = '" + username + "';";
    char *err_msg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        sqlite3_free(err_msg);
        return false;  // SQL error
    }
    return true;
}
bool Database::increment_games_as_werewolf(sqlite3* db, const std::string& username) {
    std::string sql = "UPDATE players SET games_as_werewolf = games_as_werewolf + 1 WHERE username = '" + username + "';";
    char *err_msg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        sqlite3_free(err_msg);
        return false;  // SQL error
    }
    return true;
}

int Database::get_num_wins(sqlite3* db, const std::string& username) {
    std::string sql = "SELECT num_wins FROM players WHERE username = '" + username + "';";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return -1;  // SQL error
    }
    int num_wins = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        num_wins = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return num_wins;
}

int Database::get_num_wins_as_werewolf(sqlite3* db, const std::string& username) {
    std::string sql = "SELECT wins_as_werewolf FROM players WHERE username = '" + username + "';";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return -1;  // SQL error
    }
    int num_wins = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        num_wins = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return num_wins;
}

int Database::get_games_played(sqlite3* db, const std::string& username) {
    std::string sql = "SELECT games_played FROM players WHERE username = '" + username + "';";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return -1;  // SQL error
    }
    int games_played = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        games_played = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return games_played;
}

int Database::get_games_as_werewolf(sqlite3* db, const std::string& username) {
    std::string sql = "SELECT games_as_werewolf FROM players WHERE username = '" + username + "';";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return -1;  // SQL error
    }
    int games_as_werewolf = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        games_as_werewolf = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return games_as_werewolf;
}

void Database::close_database(sqlite3* db) {
    sqlite3_close(db);
}

void Database::create_table(sqlite3* db) {
    const char *sql = "CREATE TABLE IF NOT EXISTS players ("
                      "id INTEGER PRIMARY KEY, "
                      "username TEXT UNIQUE, "
                      "password_hash TEXT);";
    char *err_msg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        sqlite3_close(db);
        exit;
    }
}
