#include "reports.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int report_bus_trips_period(Database *db, int bus_id, const char *start_date, const char *end_date) {
    const char *sql =
        "SELECT t.id, t.departure_date, t.arrival_date, r.route_name, c.last_name, "
        "t.passengers_count, t.ticket_price "
        "FROM trips t "
        "JOIN routes r ON r.id = t.route_id "
        "JOIN crew c ON c.id = t.crew_id "
        "WHERE t.bus_id = ? AND date(t.departure_date) BETWEEN date(?) AND date(?) "
        "ORDER BY t.departure_date;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, bus_id);
    sqlite3_bind_text(stmt, 2, start_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, end_date, -1, SQLITE_STATIC);

    printf("\n=== Рейсы автобуса #%d за период %s - %s ===\n", bus_id, start_date, end_date);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int trip_id = sqlite3_column_int(stmt, 0);
        const char *departure = (const char*)sqlite3_column_text(stmt, 1);
        const char *arrival = (const char*)sqlite3_column_text(stmt, 2);
        const char *route_name = (const char*)sqlite3_column_text(stmt, 3);
        const char *crew_name = (const char*)sqlite3_column_text(stmt, 4);
        int passengers = sqlite3_column_int(stmt, 5);
        double price = sqlite3_column_double(stmt, 6);
        printf("%d. %s -> %s | Маршрут: %s | Экипаж: %s | Пассажиров: %d | Цена: %.2f\n",
               trip_id, departure, arrival, route_name, crew_name, passengers, price);
    }

    sqlite3_finalize(stmt);
    return 0;
}

int report_bus_summary(Database *db, int bus_id, const char *start_date, const char *end_date) {
    const char *sql =
        "SELECT COUNT(*), COALESCE(SUM(passengers_count), 0), "
        "COALESCE(SUM(passengers_count * ticket_price), 0) "
        "FROM trips WHERE bus_id = ? AND date(departure_date) BETWEEN date(?) AND date(?);";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, bus_id);
    sqlite3_bind_text(stmt, 2, start_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, end_date, -1, SQLITE_STATIC);

    printf("\n=== Статистика автобуса #%d ===\n", bus_id);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int trips_count = sqlite3_column_int(stmt, 0);
        int passengers = sqlite3_column_int(stmt, 1);
        double revenue = sqlite3_column_double(stmt, 2);
        printf("Поездок: %d\nПассажиров: %d\nДенег: %.2f\n", trips_count, passengers, revenue);
    }

    sqlite3_finalize(stmt);
    return 0;
}

int report_crew_salary_period(Database *db, const char *start_date, const char *end_date) {
    const char *sql =
        "SELECT c.last_name, cs.amount, cs.calculated_date "
        "FROM crew_salary cs "
        "JOIN crew c ON c.id = cs.crew_id "
        "WHERE cs.period_start = ? AND cs.period_end = ? "
        "ORDER BY c.last_name;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, start_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, end_date, -1, SQLITE_STATIC);

    printf("\n=== Начисления экипажам за период %s - %s ===\n", start_date, end_date);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *last_name = (const char*)sqlite3_column_text(stmt, 0);
        double amount = sqlite3_column_double(stmt, 1);
        const char *calculated = (const char*)sqlite3_column_text(stmt, 2);
        printf("%s | %.2f | %s\n", last_name, amount, calculated);
    }

    sqlite3_finalize(stmt);
    return 0;
}

int report_most_expensive_route(Database *db) {
    const char *route_sql =
        "SELECT id, route_name, start_point, end_point, length, price_per_ticket "
        "FROM routes ORDER BY price_per_ticket DESC LIMIT 1;";

    sqlite3_stmt *stmt = db_query(db, route_sql);
    if (!stmt) {
        return -1;
    }

    printf("\n=== Самый дорогой маршрут ===\n");
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int route_id = sqlite3_column_int(stmt, 0);
        const char *route_name = (const char*)sqlite3_column_text(stmt, 1);
        const char *start_point = (const char*)sqlite3_column_text(stmt, 2);
        const char *end_point = (const char*)sqlite3_column_text(stmt, 3);
        int length = sqlite3_column_int(stmt, 4);
        double price = sqlite3_column_double(stmt, 5);
        printf("Маршрут: %s | %s -> %s | %d км | %.2f\n", route_name, start_point, end_point, length, price);

        const char *detail_sql =
            "SELECT DISTINCT b.bus_number, b.name, c.last_name, t.ticket_price "
            "FROM trips t "
            "JOIN buses b ON b.id = t.bus_id "
            "JOIN crew c ON c.id = t.crew_id "
            "WHERE t.route_id = ? "
            "ORDER BY b.bus_number;";

        sqlite3_stmt *detail_stmt = db_query(db, detail_sql);
        if (!detail_stmt) {
            sqlite3_finalize(stmt);
            return -1;
        }
        sqlite3_bind_int(detail_stmt, 1, route_id);

        while (sqlite3_step(detail_stmt) == SQLITE_ROW) {
            const char *bus_number = (const char*)sqlite3_column_text(detail_stmt, 0);
            const char *bus_name = (const char*)sqlite3_column_text(detail_stmt, 1);
            const char *crew_name = (const char*)sqlite3_column_text(detail_stmt, 2);
            double ticket_price = sqlite3_column_double(detail_stmt, 3);
            printf("%s | %s | %s | %.2f\n", bus_number, bus_name, crew_name, ticket_price);
        }

        sqlite3_finalize(detail_stmt);
    }

    sqlite3_finalize(stmt);
    return 0;
}

int report_bus_with_max_mileage(Database *db) {
    const char *sql =
        "SELECT b.id, b.bus_number, b.name, b.total_mileage, "
        "COALESCE(SUM(t.passengers_count), 0) "
        "FROM buses b "
        "LEFT JOIN trips t ON t.bus_id = b.id "
        "GROUP BY b.id "
        "ORDER BY b.total_mileage DESC "
        "LIMIT 1;";

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    printf("\n=== Автобус с наибольшим суммарным пробегом ===\n");
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *bus_number = (const char*)sqlite3_column_text(stmt, 1);
        const char *bus_name = (const char*)sqlite3_column_text(stmt, 2);
        int mileage = sqlite3_column_int(stmt, 3);
        int passengers = sqlite3_column_int(stmt, 4);
        printf("ID: %d | %s | %s | Пробег: %d | Пассажиров: %d\n",
               id, bus_number, bus_name, mileage, passengers);
    }

    sqlite3_finalize(stmt);
    return 0;
}
