#include "crew.h"
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

static int row_exists(Database *db, const char *sql, int id) {
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);
    int exists = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return exists;
}

static void populate_crew_from_stmt(sqlite3_stmt *stmt, CrewMember *crew) {
    crew->id = sqlite3_column_int(stmt, 0);
    copy_text(crew->last_name, sizeof(crew->last_name), sqlite3_column_text(stmt, 1));
    copy_text(crew->tax_id, sizeof(crew->tax_id), sqlite3_column_text(stmt, 2));
    crew->experience = sqlite3_column_int(stmt, 3);
    copy_text(crew->category, sizeof(crew->category), sqlite3_column_text(stmt, 4));
    copy_text(crew->address, sizeof(crew->address), sqlite3_column_text(stmt, 5));
    crew->birth_year = sqlite3_column_int(stmt, 6);
    crew->bus_id = sqlite3_column_int(stmt, 7);
}

static void populate_salary_from_stmt(sqlite3_stmt *stmt, CrewSalary *salary) {
    salary->id = sqlite3_column_int(stmt, 0);
    salary->crew_id = sqlite3_column_int(stmt, 1);
    copy_text(salary->period_start, sizeof(salary->period_start), sqlite3_column_text(stmt, 2));
    copy_text(salary->period_end, sizeof(salary->period_end), sqlite3_column_text(stmt, 3));
    salary->amount = sqlite3_column_double(stmt, 4);
    copy_text(salary->calculated_date, sizeof(salary->calculated_date), sqlite3_column_text(stmt, 5));
}

int crew_create_table(Database *db) {
    const char *sql =
        "CREATE TABLE IF NOT EXISTS crew ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "last_name TEXT NOT NULL,"
        "tax_id TEXT UNIQUE NOT NULL,"
        "experience INTEGER NOT NULL DEFAULT 0,"
        "category TEXT NOT NULL DEFAULT '',"
        "address TEXT NOT NULL DEFAULT '',"
        "birth_year INTEGER NOT NULL DEFAULT 0,"
        "bus_id INTEGER NOT NULL,"
        "FOREIGN KEY (bus_id) REFERENCES buses(id)"
        ");";

    return db_execute(db, sql);
}

int crew_create_salary_table(Database *db) {
    const char *sql =
        "CREATE TABLE IF NOT EXISTS crew_salary ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "crew_id INTEGER NOT NULL,"
        "period_start TEXT NOT NULL,"
        "period_end TEXT NOT NULL,"
        "amount REAL NOT NULL,"
        "calculated_date TEXT NOT NULL DEFAULT (DATE('now')),"
        "FOREIGN KEY (crew_id) REFERENCES crew(id)"
        ");";

    return db_execute(db, sql);
}

int crew_add(Database *db, CrewMember *crew) {
    if (row_exists(db, "SELECT COUNT(*) FROM buses WHERE id = ?;", crew->bus_id) <= 0) {
        return -2;
    }

    const char *sql =
        "INSERT INTO crew (last_name, tax_id, experience, category, address, birth_year, bus_id) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, crew->last_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, crew->tax_id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, crew->experience);
    sqlite3_bind_text(stmt, 4, crew->category, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, crew->address, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, crew->birth_year);
    sqlite3_bind_int(stmt, 7, crew->bus_id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        crew->id = (int)sqlite3_last_insert_rowid(db->conn);
        return 0;
    }

    return -1;
}

int crew_get_by_id(Database *db, int id, CrewMember *crew) {
    const char *sql =
        "SELECT id, last_name, tax_id, experience, category, address, birth_year, bus_id "
        "FROM crew WHERE id = ?;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        populate_crew_from_stmt(stmt, crew);
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return -1;
}

int crew_get_all(Database *db, CrewMember **crew_list, int *count) {
    const char *sql =
        "SELECT id, last_name, tax_id, experience, category, address, birth_year, bus_id "
        "FROM crew ORDER BY last_name;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    *count = 0;
    *crew_list = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        CrewMember *new_list = realloc(*crew_list, sizeof(CrewMember) * ((size_t)(*count) + 1));
        if (!new_list) {
            sqlite3_finalize(stmt);
            crew_free_list(*crew_list);
            *crew_list = NULL;
            *count = 0;
            return -1;
        }
        *crew_list = new_list;

        CrewMember *current = &(*crew_list)[*count];
        memset(current, 0, sizeof(*current));
        populate_crew_from_stmt(stmt, current);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int crew_update(Database *db, CrewMember *crew) {
    if (row_exists(db, "SELECT COUNT(*) FROM buses WHERE id = ?;", crew->bus_id) <= 0) {
        return -2;
    }

    const char *sql =
        "UPDATE crew SET last_name = ?, tax_id = ?, experience = ?, category = ?, "
        "address = ?, birth_year = ?, bus_id = ? WHERE id = ?;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, crew->last_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, crew->tax_id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, crew->experience);
    sqlite3_bind_text(stmt, 4, crew->category, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, crew->address, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, crew->birth_year);
    sqlite3_bind_int(stmt, 7, crew->bus_id);
    sqlite3_bind_int(stmt, 8, crew->id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 0 : -1;
}

int crew_delete(Database *db, int id) {
    int in_trips = row_exists(db, "SELECT COUNT(*) FROM trips WHERE crew_id = ?;", id);
    if (in_trips < 0) {
        return -1;
    }
    if (in_trips > 0) {
        return -2;
    }

    const char *sql = "DELETE FROM crew WHERE id = ?;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 0 : -1;
}

int calculateCrewSalary(Database *db, const char *start_date, const char *end_date, double percent) {
    const char *delete_sql = "DELETE FROM crew_salary WHERE period_start = ? AND period_end = ?;";
    sqlite3_stmt *delete_stmt = db_query(db, delete_sql);
    if (!delete_stmt) {
        return -1;
    }
    sqlite3_bind_text(delete_stmt, 1, start_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(delete_stmt, 2, end_date, -1, SQLITE_STATIC);
    sqlite3_step(delete_stmt);
    sqlite3_finalize(delete_stmt);

    const char *sql =
        "INSERT INTO crew_salary (crew_id, period_start, period_end, amount, calculated_date) "
        "SELECT c.id, ?, ?, "
        "COALESCE(SUM(t.passengers_count * t.ticket_price), 0) * ? / 100.0, "
        "DATE('now') "
        "FROM crew c "
        "LEFT JOIN trips t ON t.crew_id = c.id "
        "AND date(t.departure_date) BETWEEN date(?) AND date(?) "
        "GROUP BY c.id;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, start_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, end_date, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, percent);
    sqlite3_bind_text(stmt, 4, start_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, end_date, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 0 : -1;
}

int crew_get_salary_for_period(Database *db, const char *start_date, const char *end_date, CrewSalary **salary_list, int *count) {
    const char *sql =
        "SELECT id, crew_id, period_start, period_end, amount, calculated_date "
        "FROM crew_salary WHERE period_start = ? AND period_end = ? ORDER BY crew_id;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, start_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, end_date, -1, SQLITE_STATIC);

    *count = 0;
    *salary_list = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        CrewSalary *new_list = realloc(*salary_list, sizeof(CrewSalary) * ((size_t)(*count) + 1));
        if (!new_list) {
            sqlite3_finalize(stmt);
            crew_free_salary_list(*salary_list);
            *salary_list = NULL;
            *count = 0;
            return -1;
        }
        *salary_list = new_list;

        CrewSalary *current = &(*salary_list)[*count];
        memset(current, 0, sizeof(*current));
        populate_salary_from_stmt(stmt, current);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return 0;
}

void crew_free_list(CrewMember *crew_list) {
    free(crew_list);
}

void crew_free_salary_list(CrewSalary *salary_list) {
    free(salary_list);
}
