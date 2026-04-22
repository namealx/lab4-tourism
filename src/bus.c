#include "bus.h"
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

static void populate_bus_from_stmt(sqlite3_stmt *stmt, Bus *bus, int load_photo) {
    bus->id = sqlite3_column_int(stmt, 0);
    copy_text(bus->bus_number, sizeof(bus->bus_number), sqlite3_column_text(stmt, 1));
    copy_text(bus->name, sizeof(bus->name), sqlite3_column_text(stmt, 2));
    bus->total_mileage = sqlite3_column_int(stmt, 3);

    if (load_photo) {
        const void *photo_blob = sqlite3_column_blob(stmt, 4);
        int photo_size = sqlite3_column_bytes(stmt, 4);
        if (photo_blob && photo_size > 0) {
            bus->photo = malloc((size_t)photo_size);
            if (bus->photo) {
                memcpy(bus->photo, photo_blob, (size_t)photo_size);
                bus->photo_size = photo_size;
            } else {
                bus->photo_size = 0;
            }
        } else {
            bus->photo = NULL;
            bus->photo_size = 0;
        }
    } else {
        bus->photo = NULL;
        bus->photo_size = 0;
    }

    copy_text(bus->model, sizeof(bus->model), sqlite3_column_text(stmt, 5));
    bus->capacity = sqlite3_column_int(stmt, 6);
    copy_text(bus->status, sizeof(bus->status), sqlite3_column_text(stmt, 7));
}

static int bus_has_related_trips(Database *db, int id) {
    const char *sql = "SELECT COUNT(*) FROM trips WHERE bus_id = ?;";
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

int bus_create_table(Database *db) {
    const char *sql =
        "CREATE TABLE IF NOT EXISTS buses ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "bus_number TEXT UNIQUE NOT NULL,"
        "name TEXT NOT NULL DEFAULT '',"
        "total_mileage INTEGER NOT NULL DEFAULT 0,"
        "photo BLOB,"
        "model TEXT NOT NULL DEFAULT '',"
        "capacity INTEGER NOT NULL DEFAULT 0,"
        "status TEXT DEFAULT 'available'"
        ");";

    return db_execute(db, sql);
}

int bus_add(Database *db, Bus *bus) {
    const char *sql =
        "INSERT INTO buses (bus_number, name, total_mileage, photo, model, capacity, status) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    if (bus->name[0] == '\0') {
        snprintf(bus->name, sizeof(bus->name), "%s", bus->model);
    }
    if (bus->status[0] == '\0') {
        snprintf(bus->status, sizeof(bus->status), "available");
    }

    sqlite3_bind_text(stmt, 1, bus->bus_number, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, bus->name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, bus->total_mileage);
    if (bus->photo && bus->photo_size > 0) {
        sqlite3_bind_blob(stmt, 4, bus->photo, bus->photo_size, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 4);
    }
    sqlite3_bind_text(stmt, 5, bus->model, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, bus->capacity);
    sqlite3_bind_text(stmt, 7, bus->status, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        bus->id = (int)sqlite3_last_insert_rowid(db->conn);
        return 0;
    }

    return -1;
}

int bus_get_by_id(Database *db, int id, Bus *bus) {
    const char *sql =
        "SELECT id, bus_number, name, total_mileage, photo, model, capacity, status "
        "FROM buses WHERE id = ?;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        populate_bus_from_stmt(stmt, bus, 1);
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return -1;
}

int bus_get_all(Database *db, Bus **buses, int *count) {
    const char *sql =
        "SELECT id, bus_number, name, total_mileage, photo, model, capacity, status "
        "FROM buses ORDER BY bus_number;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    *count = 0;
    *buses = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Bus *new_buses = realloc(*buses, sizeof(Bus) * ((size_t)(*count) + 1));
        if (!new_buses) {
            sqlite3_finalize(stmt);
            bus_free_list(*buses);
            *buses = NULL;
            *count = 0;
            return -1;
        }
        *buses = new_buses;

        Bus *current = &(*buses)[*count];
        memset(current, 0, sizeof(*current));
        populate_bus_from_stmt(stmt, current, 0);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int bus_get_available(Database *db, Bus **buses, int *count) {
    const char *sql =
        "SELECT id, bus_number, name, total_mileage, photo, model, capacity, status "
        "FROM buses WHERE status = 'available' ORDER BY bus_number;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    *count = 0;
    *buses = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Bus *new_buses = realloc(*buses, sizeof(Bus) * ((size_t)(*count) + 1));
        if (!new_buses) {
            sqlite3_finalize(stmt);
            bus_free_list(*buses);
            *buses = NULL;
            *count = 0;
            return -1;
        }
        *buses = new_buses;

        Bus *current = &(*buses)[*count];
        memset(current, 0, sizeof(*current));
        populate_bus_from_stmt(stmt, current, 0);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int bus_update(Database *db, Bus *bus) {
    const char *sql =
        "UPDATE buses SET bus_number = ?, name = ?, total_mileage = ?, photo = ?, "
        "model = ?, capacity = ?, status = ? WHERE id = ?;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    if (bus->name[0] == '\0') {
        snprintf(bus->name, sizeof(bus->name), "%s", bus->model);
    }
    if (bus->status[0] == '\0') {
        snprintf(bus->status, sizeof(bus->status), "available");
    }

    sqlite3_bind_text(stmt, 1, bus->bus_number, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, bus->name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, bus->total_mileage);
    if (bus->photo && bus->photo_size > 0) {
        sqlite3_bind_blob(stmt, 4, bus->photo, bus->photo_size, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 4);
    }
    sqlite3_bind_text(stmt, 5, bus->model, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, bus->capacity);
    sqlite3_bind_text(stmt, 7, bus->status, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 8, bus->id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 0 : -1;
}

int bus_delete(Database *db, int id) {
    int related_trips = bus_has_related_trips(db, id);
    if (related_trips < 0) {
        return -1;
    }
    if (related_trips > 0) {
        return -2;
    }

    const char *sql = "DELETE FROM buses WHERE id = ?;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 0 : -1;
}

int bus_save_photo_to_file(Database *db, int id, const char *filename) {
    const char *sql = "SELECT photo FROM buses WHERE id = ?;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -1;
    }

    const void *blob = sqlite3_column_blob(stmt, 0);
    int size = sqlite3_column_bytes(stmt, 0);
    if (!blob || size <= 0) {
        sqlite3_finalize(stmt);
        return -1;
    }

    FILE *file = fopen(filename, "wb");
    if (!file) {
        sqlite3_finalize(stmt);
        return -1;
    }

    size_t written = fwrite(blob, 1, (size_t)size, file);
    fclose(file);
    sqlite3_finalize(stmt);

    return (written == (size_t)size) ? 0 : -1;
}

int bus_set_status(Database *db, int id, const char *status) {
    const char *sql = "UPDATE buses SET status = ? WHERE id = ?;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, status, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 0 : -1;
}

void bus_free(Bus *bus) {
    if (bus && bus->photo) {
        free(bus->photo);
        bus->photo = NULL;
        bus->photo_size = 0;
    }
}

void bus_free_list(Bus *buses) {
    if (!buses) {
        return;
    }

    // Освобождаем вложенные BLOB-данные перед освобождением массива.
    // Длина массива неизвестна здесь, поэтому вызывающий код должен освобождать
    // каждый элемент через bus_free() перед bus_free_list(), если фото было загружено.
    free(buses);
}
