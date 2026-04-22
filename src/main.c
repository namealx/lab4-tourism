#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "auth.h"
#include "route.h"
#include "order.h"
#include "bus.h"
#include "crew.h"
#include "trip.h"
#include "menu.h"

static int table_row_count(Database *db, const char *table) {
    char sql[128];
    snprintf(sql, sizeof(sql), "SELECT COUNT(*) FROM %s;", table);

    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) {
        return -1;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

static void seed_sample_data(Database *db) {
    if (table_row_count(db, "users") == 0) {
        auth_register(db, "admin", "admin", "Администратор", "admin");
        auth_register(db, "manager", "manager", "Менеджер", "manager");
        auth_register(db, "crew", "crew", "Экипаж", "crew");
        auth_register(db, "customer", "customer", "Клиент", "customer");
    }

    if (table_row_count(db, "buses") == 0) {
        static const unsigned char photo1[] = "bus-photo-1";
        static const unsigned char photo2[] = "bus-photo-2";
        static const unsigned char photo3[] = "bus-photo-3";

        Bus bus = {0};
        snprintf(bus.bus_number, sizeof(bus.bus_number), "A100AA");
        snprintf(bus.name, sizeof(bus.name), "Setra ComfortClass");
        snprintf(bus.model, sizeof(bus.model), "Setra S 516 HD");
        bus.total_mileage = 124000;
        bus.capacity = 49;
        snprintf(bus.status, sizeof(bus.status), "available");
        bus.photo = (unsigned char*)photo1;
        bus.photo_size = (int)sizeof(photo1) - 1;
        bus_add(db, &bus);

        memset(&bus, 0, sizeof(bus));
        snprintf(bus.bus_number, sizeof(bus.bus_number), "B200BB");
        snprintf(bus.name, sizeof(bus.name), "Mercedes Tourismo");
        snprintf(bus.model, sizeof(bus.model), "Mercedes-Benz Tourismo");
        bus.total_mileage = 98000;
        bus.capacity = 45;
        snprintf(bus.status, sizeof(bus.status), "available");
        bus.photo = (unsigned char*)photo2;
        bus.photo_size = (int)sizeof(photo2) - 1;
        bus_add(db, &bus);

        memset(&bus, 0, sizeof(bus));
        snprintf(bus.bus_number, sizeof(bus.bus_number), "C300CC");
        snprintf(bus.name, sizeof(bus.name), "MAN Lion's Coach");
        snprintf(bus.model, sizeof(bus.model), "MAN Lion's Coach");
        bus.total_mileage = 156500;
        bus.capacity = 53;
        snprintf(bus.status, sizeof(bus.status), "available");
        bus.photo = (unsigned char*)photo3;
        bus.photo_size = (int)sizeof(photo3) - 1;
        bus_add(db, &bus);
    }

    if (table_row_count(db, "routes") == 0) {
        TourRoute route = {0};

        snprintf(route.route_name, sizeof(route.route_name), "Минск - Несвиж");
        snprintf(route.start_point, sizeof(route.start_point), "Минск");
        snprintf(route.end_point, sizeof(route.end_point), "Несвиж");
        route.length = 120;
        route.price_per_ticket = 25.0;
        route_add(db, &route);

        memset(&route, 0, sizeof(route));
        snprintf(route.route_name, sizeof(route.route_name), "Минск - Брест");
        snprintf(route.start_point, sizeof(route.start_point), "Минск");
        snprintf(route.end_point, sizeof(route.end_point), "Брест");
        route.length = 350;
        route.price_per_ticket = 45.0;
        route_add(db, &route);

        memset(&route, 0, sizeof(route));
        snprintf(route.route_name, sizeof(route.route_name), "Гродно - Лида");
        snprintf(route.start_point, sizeof(route.start_point), "Гродно");
        snprintf(route.end_point, sizeof(route.end_point), "Лида");
        route.length = 110;
        route.price_per_ticket = 30.0;
        route_add(db, &route);
    }

    if (table_row_count(db, "crew") == 0) {
        CrewMember crew = {0};
        snprintf(crew.last_name, sizeof(crew.last_name), "Иванов");
        snprintf(crew.tax_id, sizeof(crew.tax_id), "AA1001");
        crew.experience = 8;
        snprintf(crew.category, sizeof(crew.category), "A");
        snprintf(crew.address, sizeof(crew.address), "Минск, ул. Ленина, 1");
        crew.birth_year = 1987;
        crew.bus_id = 1;
        crew_add(db, &crew);

        memset(&crew, 0, sizeof(crew));
        snprintf(crew.last_name, sizeof(crew.last_name), "Петров");
        snprintf(crew.tax_id, sizeof(crew.tax_id), "AA1002");
        crew.experience = 11;
        snprintf(crew.category, sizeof(crew.category), "B");
        snprintf(crew.address, sizeof(crew.address), "Брест, ул. Советская, 10");
        crew.birth_year = 1983;
        crew.bus_id = 2;
        crew_add(db, &crew);

        memset(&crew, 0, sizeof(crew));
        snprintf(crew.last_name, sizeof(crew.last_name), "Сидоров");
        snprintf(crew.tax_id, sizeof(crew.tax_id), "AA1003");
        crew.experience = 5;
        snprintf(crew.category, sizeof(crew.category), "C");
        snprintf(crew.address, sizeof(crew.address), "Гродно, ул. Кирова, 5");
        crew.birth_year = 1990;
        crew.bus_id = 3;
        crew_add(db, &crew);
    }

    if (table_row_count(db, "trips") == 0) {
        Trip trip = {0};

        trip.bus_id = 1;
        trip.crew_id = 1;
        snprintf(trip.departure_date, sizeof(trip.departure_date), "2026-04-01");
        snprintf(trip.arrival_date, sizeof(trip.arrival_date), "2026-04-02");
        trip.route_id = 1;
        trip.passengers_count = 30;
        trip.ticket_price = 25.0;
        trip_add(db, &trip);

        memset(&trip, 0, sizeof(trip));
        trip.bus_id = 1;
        trip.crew_id = 1;
        snprintf(trip.departure_date, sizeof(trip.departure_date), "2026-04-05");
        snprintf(trip.arrival_date, sizeof(trip.arrival_date), "2026-04-06");
        trip.route_id = 2;
        trip.passengers_count = 28;
        trip.ticket_price = 45.0;
        trip_add(db, &trip);

        memset(&trip, 0, sizeof(trip));
        trip.bus_id = 2;
        trip.crew_id = 2;
        snprintf(trip.departure_date, sizeof(trip.departure_date), "2026-04-03");
        snprintf(trip.arrival_date, sizeof(trip.arrival_date), "2026-04-04");
        trip.route_id = 2;
        trip.passengers_count = 35;
        trip.ticket_price = 45.0;
        trip_add(db, &trip);

        memset(&trip, 0, sizeof(trip));
        trip.bus_id = 3;
        trip.crew_id = 3;
        snprintf(trip.departure_date, sizeof(trip.departure_date), "2026-04-08");
        snprintf(trip.arrival_date, sizeof(trip.arrival_date), "2026-04-09");
        trip.route_id = 3;
        trip.passengers_count = 18;
        trip.ticket_price = 30.0;
        trip_add(db, &trip);
    }
}

int init_database(Database *db) {
    // Создание всех таблиц
    if (auth_create_table(db) != SQLITE_OK) return -1;
    if (bus_create_table(db) != SQLITE_OK) return -1;
    if (route_create_table(db) != SQLITE_OK) return -1;
    if (crew_create_table(db) != SQLITE_OK) return -1;
    if (crew_create_salary_table(db) != SQLITE_OK) return -1;
    if (trip_create_table(db) != SQLITE_OK) return -1;
    if (order_create_table(db) != SQLITE_OK) return -1;

    seed_sample_data(db);
    
    return 0;
}

int main() {
    Database *db = db_open("tourism.db");
    if (!db) {
        printf("Ошибка открытия базы данных\n");
        return 1;
    }
    
    if (init_database(db) != 0) {
        printf("Ошибка инициализации базы данных\n");
        db_close(db);
        return 1;
    }
    
    show_main_menu(db);
    
    db_close(db);
    return 0;
}
