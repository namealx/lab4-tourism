#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Database* db_open(const char *filename) {
    Database *db = malloc(sizeof(Database));
    db->last_error = NULL;
    
    int rc = sqlite3_open(filename, &db->conn);
    if (rc != SQLITE_OK) {
        db->last_error = strdup(sqlite3_errmsg(db->conn));
        sqlite3_close(db->conn);
        free(db);
        return NULL;
    }

    // Включаем контроль внешних ключей для целостности туристического бюро.
    sqlite3_exec(db->conn, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);
    return db;
}

void db_close(Database *db) {
    if (db) {
        sqlite3_close(db->conn);
        if (db->last_error) free(db->last_error);
        free(db);
    }
}

int db_execute(Database *db, const char *sql) {
    char *errmsg = NULL;
    int rc = sqlite3_exec(db->conn, sql, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        if (db->last_error) free(db->last_error);
        db->last_error = strdup(errmsg);
        sqlite3_free(errmsg);
        return rc;
    }
    return SQLITE_OK;
}

sqlite3_stmt* db_query(Database *db, const char *sql) {
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db->conn, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        if (db->last_error) free(db->last_error);
        db->last_error = strdup(sqlite3_errmsg(db->conn));
        return NULL;
    }
    return stmt;
}

const char* db_last_error(Database *db) {
    return db->last_error ? db->last_error : "No error";
}
