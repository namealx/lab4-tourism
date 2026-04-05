#include "route.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int route_create_table(Database *db) {
    const char *sql = 
        "CREATE TABLE IF NOT EXISTS tour_routes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "start_point TEXT NOT NULL,"
        "end_point TEXT NOT NULL,"
        "distance INTEGER,"
        "price_per_ticket REAL NOT NULL"
        ");";
    
    return db_execute(db, sql);
}

int route_add(Database *db, TourRoute *route) {
    const char *sql = "INSERT INTO tour_routes (name, start_point, end_point, distance, price_per_ticket) "
                      "VALUES (?, ?, ?, ?, ?);";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    sqlite3_bind_text(stmt, 1, route->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, route->start_point, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, route->end_point, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, route->distance);
    sqlite3_bind_double(stmt, 5, route->price_per_ticket);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE) {
        route->id = sqlite3_last_insert_rowid(db->conn);
        return 0;
    }
    return -1;
}

int route_get_by_id(Database *db, int id, TourRoute *route) {
    const char *sql = "SELECT id, name, start_point, end_point, distance, price_per_ticket "
                      "FROM tour_routes WHERE id = ?;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    sqlite3_bind_int(stmt, 1, id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        route->id = sqlite3_column_int(stmt, 0);
        strcpy(route->name, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(route->start_point, (const char*)sqlite3_column_text(stmt, 2));
        strcpy(route->end_point, (const char*)sqlite3_column_text(stmt, 3));
        route->distance = sqlite3_column_int(stmt, 4);
        route->price_per_ticket = sqlite3_column_double(stmt, 5);
        sqlite3_finalize(stmt);
        return 0;
    }
    
    sqlite3_finalize(stmt);
    return -1;
}

int route_get_all(Database *db, TourRoute **routes, int *count) {
    const char *sql = "SELECT id, name, start_point, end_point, distance, price_per_ticket "
                      "FROM tour_routes ORDER BY name;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    *count = 0;
    *routes = NULL;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TourRoute *new_routes = realloc(*routes, sizeof(TourRoute) * (*count + 1));
        if (!new_routes) {
            sqlite3_finalize(stmt);
            free(*routes);
            return -1;
        }
        *routes = new_routes;
        
        TourRoute *r = &(*routes)[*count];
        r->id = sqlite3_column_int(stmt, 0);
        strcpy(r->name, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(r->start_point, (const char*)sqlite3_column_text(stmt, 2));
        strcpy(r->end_point, (const char*)sqlite3_column_text(stmt, 3));
        r->distance = sqlite3_column_int(stmt, 4);
        r->price_per_ticket = sqlite3_column_double(stmt, 5);
        
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return 0;
}

int route_update(Database *db, TourRoute *route) {
    const char *sql = "UPDATE tour_routes SET name = ?, start_point = ?, end_point = ?, "
                      "distance = ?, price_per_ticket = ? WHERE id = ?;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    sqlite3_bind_text(stmt, 1, route->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, route->start_point, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, route->end_point, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, route->distance);
    sqlite3_bind_double(stmt, 5, route->price_per_ticket);
    sqlite3_bind_int(stmt, 6, route->id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? 0 : -1;
}

int route_delete(Database *db, int id) {
    const char *sql = "DELETE FROM tour_routes WHERE id = ?;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    sqlite3_bind_int(stmt, 1, id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? 0 : -1;
}

int route_search_by_name(Database *db, const char *name, TourRoute **routes, int *count) {
    const char *sql = "SELECT id, name, start_point, end_point, distance, price_per_ticket "
                      "FROM tour_routes WHERE name LIKE ? ORDER BY name;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    char pattern[110];
    snprintf(pattern, sizeof(pattern), "%%%s%%", name);
    sqlite3_bind_text(stmt, 1, pattern, -1, SQLITE_STATIC);
    
    *count = 0;
    *routes = NULL;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TourRoute *new_routes = realloc(*routes, sizeof(TourRoute) * (*count + 1));
        if (!new_routes) {
            sqlite3_finalize(stmt);
            free(*routes);
            return -1;
        }
        *routes = new_routes;
        
        TourRoute *r = &(*routes)[*count];
        r->id = sqlite3_column_int(stmt, 0);
        strcpy(r->name, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(r->start_point, (const char*)sqlite3_column_text(stmt, 2));
        strcpy(r->end_point, (const char*)sqlite3_column_text(stmt, 3));
        r->distance = sqlite3_column_int(stmt, 4);
        r->price_per_ticket = sqlite3_column_double(stmt, 5);
        
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return 0;
}

void route_free_list(TourRoute *routes) {
    if (routes) free(routes);
}
