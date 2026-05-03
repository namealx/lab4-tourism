#include "route.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void copy_text(char *dest, size_t size, const unsigned char *src) {
    if (src) {
        snprintf(dest, size, "%s", src);
    } else if (size > 0) {
        dest[0] = '\0';
    }
}

static void populate_route_from_stmt(sqlite3_stmt *stmt, TourRoute *route) {
    route->id = sqlite3_column_int(stmt, 0);
    copy_text(route->route_name, sizeof(route->route_name), sqlite3_column_text(stmt, 1));
    copy_text(route->name, sizeof(route->name), sqlite3_column_text(stmt, 1));
    copy_text(route->start_point, sizeof(route->start_point), sqlite3_column_text(stmt, 2));
    copy_text(route->end_point, sizeof(route->end_point), sqlite3_column_text(stmt, 3));
    route->length = sqlite3_column_int(stmt, 4);
    route->distance = route->length;
    route->price_per_ticket = sqlite3_column_double(stmt, 5);
}

int route_create_table(Database *db) {
    const char *sql =
        "CREATE TABLE IF NOT EXISTS routes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "route_name TEXT NOT NULL,"
        "start_point TEXT NOT NULL,"
        "end_point TEXT NOT NULL,"
        "length INTEGER NOT NULL CHECK(length > 0),"
        "price_per_ticket REAL NOT NULL DEFAULT 0"
        ");";

    return db_execute(db, sql);
}

int route_add(Database *db, TourRoute *route) {
    if (route->length <= 0) {
        return -2;
    }

    if (route->route_name[0] == '\0') {
        snprintf(route->route_name, sizeof(route->route_name), "%s", route->name);
    }

    const char *sql =
        "INSERT INTO routes (route_name, start_point, end_point, length, price_per_ticket) "
        "VALUES (?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, route->route_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, route->start_point, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, route->end_point, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, route->length);
    sqlite3_bind_double(stmt, 5, route->price_per_ticket);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        route->id = (int)sqlite3_last_insert_rowid(db->conn);
        route->distance = route->length;
        snprintf(route->name, sizeof(route->name), "%s", route->route_name);
        return 0;
    }

    return -1;
}

int route_get_by_id(Database *db, int id, TourRoute *route) {
    const char *sql =
        "SELECT id, route_name, start_point, end_point, length, price_per_ticket "
        "FROM routes WHERE id = ?;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        populate_route_from_stmt(stmt, route);
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return -1;
}

int route_get_all(Database *db, TourRoute **routes, int *count) {
    const char *sql =
        "SELECT id, route_name, start_point, end_point, length, price_per_ticket "
        "FROM routes ORDER BY route_name;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    *count = 0;
    *routes = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TourRoute *new_routes = realloc(*routes, sizeof(TourRoute) * ((size_t)(*count) + 1));
        if (!new_routes) {
            sqlite3_finalize(stmt);
            route_free_list(*routes);
            *routes = NULL;
            *count = 0;
            return -1;
        }
        *routes = new_routes;

        TourRoute *current = &(*routes)[*count];
        memset(current, 0, sizeof(*current));
        populate_route_from_stmt(stmt, current);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int route_update(Database *db, TourRoute *route) {
    if (route->length <= 0) {
        return -2;
    }

    if (route->route_name[0] == '\0') {
        snprintf(route->route_name, sizeof(route->route_name), "%s", route->name);
    }

    const char *sql =
        "UPDATE routes SET route_name = ?, start_point = ?, end_point = ?, "
        "length = ?, price_per_ticket = ? WHERE id = ?;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, route->route_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, route->start_point, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, route->end_point, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, route->length);
    sqlite3_bind_double(stmt, 5, route->price_per_ticket);
    sqlite3_bind_int(stmt, 6, route->id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 0 : -1;
}

int route_delete(Database *db, int id) {
    const char *sql = "DELETE FROM routes WHERE id = ?;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 0 : -1;
}

int route_search_by_name(Database *db, const char *name, TourRoute **routes, int *count) {
    const char *sql =
        "SELECT id, route_name, start_point, end_point, length, price_per_ticket "
        "FROM routes WHERE route_name LIKE ? ORDER BY route_name;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    char pattern[128];
    snprintf(pattern, sizeof(pattern), "%%%s%%", name);
    sqlite3_bind_text(stmt, 1, pattern, -1, SQLITE_TRANSIENT);

    *count = 0;
    *routes = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TourRoute *new_routes = realloc(*routes, sizeof(TourRoute) * ((size_t)(*count) + 1));
        if (!new_routes) {
            sqlite3_finalize(stmt);
            route_free_list(*routes);
            *routes = NULL;
            *count = 0;
            return -1;
        }
        *routes = new_routes;

        TourRoute *current = &(*routes)[*count];
        memset(current, 0, sizeof(*current));
        populate_route_from_stmt(stmt, current);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return 0;
}

void route_free_list(TourRoute *routes) {
    free(routes);
}
