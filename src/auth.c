#include "auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Простая хеш-функция (для учебных целей)
static void simple_hash(const char *input, char *output) {
    unsigned long hash = 5381;
    int c;
    while ((c = *input++)) {
        hash = ((hash << 5) + hash) + c;
    }
    sprintf(output, "%lu", hash);
}

int auth_create_table(Database *db) {
    const char *sql = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password_hash TEXT NOT NULL,"
        "full_name TEXT NOT NULL,"
        "role TEXT CHECK(role IN ('admin', 'manager', 'client')) NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";
    
    return db_execute(db, sql);
}

int auth_register(Database *db, const char *username, const char *password, 
                  const char *full_name, const char *role) {
    char password_hash[100];
    simple_hash(password, password_hash);
    
    const char *sql = "INSERT INTO users (username, password_hash, full_name, role) VALUES (?, ?, ?, ?);";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password_hash, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, full_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, role, -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? 0 : -1;
}

User* auth_login(Database *db, const char *username, const char *password) {
    char password_hash[100];
    simple_hash(password, password_hash);
    
    const char *sql = "SELECT id, username, full_name, role FROM users "
                      "WHERE username = ? AND password_hash = ?;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return NULL;
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password_hash, -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        User *user = malloc(sizeof(User));
        user->id = sqlite3_column_int(stmt, 0);
        strcpy(user->username, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(user->full_name, (const char*)sqlite3_column_text(stmt, 2));
        strcpy(user->role, (const char*)sqlite3_column_text(stmt, 3));
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return NULL;
}

int auth_is_admin(User *user) {
    return user && strcmp(user->role, "admin") == 0;
}

int auth_is_manager(User *user) {
    return user && strcmp(user->role, "manager") == 0;
}

int auth_is_client(User *user) {
    return user && strcmp(user->role, "client") == 0;
}

void auth_logout(User *user) {
    if (user) free(user);
}
