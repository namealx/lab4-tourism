#include "auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Простая хеш-функция для учебного проекта.
static void simple_hash(const char *input, char *output, size_t output_size) {
    unsigned long hash = 5381;
    int c;

    while ((c = *input++) != 0) {
        hash = ((hash << 5) + hash) + (unsigned long)c;
    }

    snprintf(output, output_size, "%lu", hash);
}

int auth_create_table(Database *db) {
    const char *sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "login TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL,"
        "full_name TEXT NOT NULL,"
        "role TEXT CHECK(role IN ('admin', 'manager', 'crew', 'client', 'customer')) NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";

    return db_execute(db, sql);
}

int auth_register(Database *db, const char *login, const char *password,
                  const char *full_name, const char *role) {
    char password_hash[128];
    simple_hash(password, password_hash, sizeof(password_hash));

    const char *sql = "INSERT INTO users (login, password, full_name, role) VALUES (?, ?, ?, ?);";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, login, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password_hash, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, full_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, role, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 0 : -1;
}

User* auth_login(Database *db, const char *login, const char *password) {
    char password_hash[128];
    simple_hash(password, password_hash, sizeof(password_hash));

    const char *sql = "SELECT id, login, password, full_name, role FROM users "
                      "WHERE login = ? AND password = ?;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return NULL;
    }

    sqlite3_bind_text(stmt, 1, login, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password_hash, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        User *user = calloc(1, sizeof(User));
        user->id = sqlite3_column_int(stmt, 0);
        strcpy(user->login, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(user->username, user->login);
        strcpy(user->password, (const char*)sqlite3_column_text(stmt, 2));
        strcpy(user->full_name, (const char*)sqlite3_column_text(stmt, 3));
        strcpy(user->role, (const char*)sqlite3_column_text(stmt, 4));
        sqlite3_finalize(stmt);
        return user;
    }

    sqlite3_finalize(stmt);
    return NULL;
}

const char* auth_get_current_user_role(User *user) {
    return user ? user->role : "";
}

const char* getCurrentUserRole(User *user) {
    return auth_get_current_user_role(user);
}

int auth_is_admin(User *user) {
    return user && strcmp(user->role, "admin") == 0;
}

int auth_is_manager(User *user) {
    return user && strcmp(user->role, "manager") == 0;
}

int auth_is_client(User *user) {
    return user && (strcmp(user->role, "client") == 0 || strcmp(user->role, "customer") == 0);
}

void auth_logout(User *user) {
    free(user);
}
