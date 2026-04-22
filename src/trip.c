#include "trip.h"
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

static int entity_exists(Database *db, const char *sql, int id) {
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

static void populate_trip_from_stmt(sqlite3_stmt *stmt, Trip *trip) {
    trip->id = sqlite3_column_int(stmt, 0);
    trip->bus_id = sqlite3_column_int(stmt, 1);
    trip->crew_id = sqlite3_column_int(stmt, 2);
    copy_text(trip->departure_date, sizeof(trip->departure_date), sqlite3_column_text(stmt, 3));
    copy_text(trip->arrival_date, sizeof(trip->arrival_date), sqlite3_column_text(stmt, 4));
    trip->route_id = sqlite3_column_int(stmt, 5);
    trip->passengers_count = sqlite3_column_int(stmt, 6);
    trip->ticket_price = sqlite3_column_double(stmt, 7);
}

static int validate_trip(Database *db, const Trip *trip) {
    if (!trip) {
        return -1;
    }
    if (trip->departure_date[0] == '\0' || trip->arrival_date[0] == '\0') {
        return -2;
    }
    if (strcmp(trip->departure_date, trip->arrival_date) >= 0) {
        return -3;
    }
    if (trip->passengers_count < 0 || trip->ticket_price < 0.0 || trip->crew_id <= 0) {
        return -4;
    }

    if (entity_exists(db, "SELECT COUNT(*) FROM buses WHERE id = ?;", trip->bus_id) <= 0) {
        return -5;
    }
    if (entity_exists(db, "SELECT COUNT(*) FROM routes WHERE id = ?;", trip->route_id) <= 0) {
        return -6;
    }
    if (entity_exists(db, "SELECT COUNT(*) FROM crew WHERE id = ?;", trip->crew_id) <= 0) {
        return -7;
    }

    return 0;
}

int trip_create_table(Database *db) {
    const char *sql =
        "CREATE TABLE IF NOT EXISTS trips ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "bus_id INTEGER NOT NULL,"
        "crew_id INTEGER NOT NULL,"
        "departure_date DATE NOT NULL,"
        "arrival_date DATE NOT NULL,"
        "route_id INTEGER NOT NULL,"
        "passengers_count INTEGER NOT NULL DEFAULT 0,"
        "ticket_price REAL NOT NULL DEFAULT 0,"
        "FOREIGN KEY (bus_id) REFERENCES buses(id),"
        "FOREIGN KEY (crew_id) REFERENCES crew(id),"
        "FOREIGN KEY (route_id) REFERENCES routes(id),"
        "CHECK (departure_date < arrival_date),"
        "CHECK (passengers_count >= 0),"
        "CHECK (ticket_price >= 0)"
        ");";

    return db_execute(db, sql);
}

int trip_add(Database *db, Trip *trip) {
    int validation = validate_trip(db, trip);
    if (validation != 0) {
        return validation;
    }

    const char *sql =
        "INSERT INTO trips (bus_id, crew_id, departure_date, arrival_date, route_id, "
        "passengers_count, ticket_price) VALUES (?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, trip->bus_id);
    sqlite3_bind_int(stmt, 2, trip->crew_id);
    sqlite3_bind_text(stmt, 3, trip->departure_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, trip->arrival_date, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, trip->route_id);
    sqlite3_bind_int(stmt, 6, trip->passengers_count);
    sqlite3_bind_double(stmt, 7, trip->ticket_price);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        trip->id = (int)sqlite3_last_insert_rowid(db->conn);
        return 0;
    }

    return -1;
}

int trip_get_by_id(Database *db, int id, Trip *trip) {
    const char *sql =
        "SELECT id, bus_id, crew_id, departure_date, arrival_date, route_id, "
        "passengers_count, ticket_price FROM trips WHERE id = ?;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        populate_trip_from_stmt(stmt, trip);
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return -1;
}

int trip_get_all(Database *db, Trip **trips, int *count) {
    const char *sql =
        "SELECT id, bus_id, crew_id, departure_date, arrival_date, route_id, "
        "passengers_count, ticket_price FROM trips ORDER BY departure_date DESC;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    *count = 0;
    *trips = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Trip *new_trips = realloc(*trips, sizeof(Trip) * ((size_t)(*count) + 1));
        if (!new_trips) {
            sqlite3_finalize(stmt);
            trip_free_list(*trips);
            *trips = NULL;
            *count = 0;
            return -1;
        }
        *trips = new_trips;

        Trip *current = &(*trips)[*count];
        memset(current, 0, sizeof(*current));
        populate_trip_from_stmt(stmt, current);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int trip_get_by_bus_period(Database *db, int bus_id, const char *start_date, const char *end_date, Trip **trips, int *count) {
    const char *sql =
        "SELECT id, bus_id, crew_id, departure_date, arrival_date, route_id, "
        "passengers_count, ticket_price FROM trips "
        "WHERE bus_id = ? AND date(departure_date) BETWEEN date(?) AND date(?) "
        "ORDER BY departure_date;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, bus_id);
    sqlite3_bind_text(stmt, 2, start_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, end_date, -1, SQLITE_STATIC);

    *count = 0;
    *trips = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Trip *new_trips = realloc(*trips, sizeof(Trip) * ((size_t)(*count) + 1));
        if (!new_trips) {
            sqlite3_finalize(stmt);
            trip_free_list(*trips);
            *trips = NULL;
            *count = 0;
            return -1;
        }
        *trips = new_trips;

        Trip *current = &(*trips)[*count];
        memset(current, 0, sizeof(*current));
        populate_trip_from_stmt(stmt, current);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int trip_update(Database *db, Trip *trip) {
    int validation = validate_trip(db, trip);
    if (validation != 0) {
        return validation;
    }

    const char *sql =
        "UPDATE trips SET bus_id = ?, crew_id = ?, departure_date = ?, arrival_date = ?, "
        "route_id = ?, passengers_count = ?, ticket_price = ? WHERE id = ?;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, trip->bus_id);
    sqlite3_bind_int(stmt, 2, trip->crew_id);
    sqlite3_bind_text(stmt, 3, trip->departure_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, trip->arrival_date, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, trip->route_id);
    sqlite3_bind_int(stmt, 6, trip->passengers_count);
    sqlite3_bind_double(stmt, 7, trip->ticket_price);
    sqlite3_bind_int(stmt, 8, trip->id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 0 : -1;
}

int trip_delete(Database *db, int id) {
    const char *sql = "DELETE FROM trips WHERE id = ?;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 0 : -1;
}

double trip_calculate_revenue(const Trip *trip) {
    if (!trip) {
        return 0.0;
    }
    return (double)trip->passengers_count * trip->ticket_price;
}

void trip_free_list(Trip *trips) {
    free(trips);
}
