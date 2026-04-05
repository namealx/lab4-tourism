#include "bus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int bus_create_table(Database *db) {
    const char *sql = 
        "CREATE TABLE IF NOT EXISTS buses ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "bus_number TEXT UNIQUE NOT NULL,"
        "model TEXT NOT NULL,"
        "capacity INTEGER NOT NULL,"
        "status TEXT DEFAULT 'available'"
        ");";
    
    return db_execute(db, sql);
}

int bus_add(Database *db, Bus *bus) {
    const char *sql = "INSERT INTO buses (bus_number, model, capacity, status) VALUES (?, ?, ?, ?);";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    sqlite3_bind_text(stmt, 1, bus->bus_number, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, bus->model, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, bus->capacity);
    sqlite3_bind_text(stmt, 4, bus->status, -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE) {
        bus->id = sqlite3_last_insert_rowid(db->conn);
        return 0;
    }
    return -1;
}

int bus_get_by_id(Database *db, int id, Bus *bus) {
    const char *sql = "SELECT id, bus_number, model, capacity, status FROM buses WHERE id = ?;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    sqlite3_bind_int(stmt, 1, id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        bus->id = sqlite3_column_int(stmt, 0);
        strcpy(bus->bus_number, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(bus->model, (const char*)sqlite3_column_text(stmt, 2));
        bus->capacity = sqlite3_column_int(stmt, 3);
        strcpy(bus->status, (const char*)sqlite3_column_text(stmt, 4));
        sqlite3_finalize(stmt);
        return 0;
    }
    
    sqlite3_finalize(stmt);
    return -1;
}

int bus_get_all(Database *db, Bus **buses, int *count) {
    const char *sql = "SELECT id, bus_number, model, capacity, status FROM buses ORDER BY bus_number;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    *count = 0;
    *buses = NULL;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Bus *new_buses = realloc(*buses, sizeof(Bus) * (*count + 1));
        if (!new_buses) {
            sqlite3_finalize(stmt);
            free(*buses);
            return -1;
        }
        *buses = new_buses;
        
        Bus *b = &(*buses)[*count];
        b->id = sqlite3_column_int(stmt, 0);
        strcpy(b->bus_number, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(b->model, (const char*)sqlite3_column_text(stmt, 2));
        b->capacity = sqlite3_column_int(stmt, 3);
        strcpy(b->status, (const char*)sqlite3_column_text(stmt, 4));
        
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return 0;
}

int bus_get_available(Database *db, Bus **buses, int *count) {
    const char *sql = "SELECT id, bus_number, model, capacity, status FROM buses "
                      "WHERE status = 'available' ORDER BY bus_number;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    *count = 0;
    *buses = NULL;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Bus *new_buses = realloc(*buses, sizeof(Bus) * (*count + 1));
        if (!new_buses) {
            sqlite3_finalize(stmt);
            free(*buses);
            return -1;
        }
        *buses = new_buses;
        
        Bus *b = &(*buses)[*count];
        b->id = sqlite3_column_int(stmt, 0);
        strcpy(b->bus_number, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(b->model, (const char*)sqlite3_column_text(stmt, 2));
        b->capacity = sqlite3_column_int(stmt, 3);
        strcpy(b->status, (const char*)sqlite3_column_text(stmt, 4));
        
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return 0;
}

int bus_update(Database *db, Bus *bus) {
    const char *sql = "UPDATE buses SET bus_number = ?, model = ?, capacity = ?, status = ? WHERE id = ?;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    sqlite3_bind_text(stmt, 1, bus->bus_number, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, bus->model, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, bus->capacity);
    sqlite3_bind_text(stmt, 4, bus->status, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, bus->id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? 0 : -1;
}

int bus_delete(Database *db, int id) {
    const char *sql = "DELETE FROM buses WHERE id = ?;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    sqlite3_bind_int(stmt, 1, id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? 0 : -1;
}

int bus_set_status(Database *db, int id, const char *status) {
    const char *sql = "UPDATE buses SET status = ? WHERE id = ?;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    sqlite3_bind_text(stmt, 1, status, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? 0 : -1;
}

void bus_free_list(Bus *buses) {
    if (buses) free(buses);
}
