#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../includes/auth.h"
#include "../includes/bus.h"
#include "../includes/crew.h"
#include "../includes/database.h"
#include "../includes/route.h"
#include "../includes/trip.h"

static int tests_run = 0;
static int tests_failed = 0;

static void expect_true(int condition, const char *message) {
    tests_run++;
    if (!condition) {
        tests_failed++;
        printf("[FAIL] %s\n", message);
    } else {
        printf("[PASS] %s\n", message);
    }
}

static void expect_int(int expected, int actual, const char *message) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s (expected=%d actual=%d)", message, expected, actual);
    expect_true(expected == actual, buffer);
}

static void expect_double(double expected, double actual, double eps, const char *message) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s (expected=%.2f actual=%.2f)", message, expected, actual);
    expect_true(fabs(expected - actual) <= eps, buffer);
}

static Database* setup_db(void) {
    Database *db = db_open(":memory:");
    if (!db) {
        return NULL;
    }

    auth_create_table(db);
    bus_create_table(db);
    route_create_table(db);
    crew_create_table(db);
    crew_create_salary_table(db);
    trip_create_table(db);

    auth_register(db, "admin", "admin", "Администратор", "admin");
    auth_register(db, "manager", "manager", "Менеджер", "manager");
    auth_register(db, "crew", "crew", "Экипаж", "crew");
    auth_register(db, "customer", "customer", "Клиент", "customer");

    static const unsigned char photo1[] = "photo-1";
    static const unsigned char photo2[] = "photo-2";
    static const unsigned char photo3[] = "photo-3";

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

    return db;
}

static void destroy_db(Database *db) {
    db_close(db);
}

static void test_auth_login_success(void) {
    Database *db = setup_db();
    User *user = auth_login(db, "admin", "admin");
    expect_true(user != NULL, "auth: successful login returns user");
    expect_true(user && auth_is_admin(user), "auth: admin role is detected");
    expect_true(user && strcmp(getCurrentUserRole(user), "admin") == 0, "auth: current role matches");
    auth_logout(user);
    destroy_db(db);
}

static void test_auth_login_failure(void) {
    Database *db = setup_db();
    User *user = auth_login(db, "admin", "wrong");
    expect_true(user == NULL, "auth: wrong password rejects login");
    destroy_db(db);
}

static void test_auth_client_role(void) {
    Database *db = setup_db();
    User *user = auth_login(db, "customer", "customer");
    expect_true(user != NULL, "auth: customer can login");
    expect_true(user && auth_is_client(user), "auth: customer role is detected");
    auth_logout(user);
    destroy_db(db);
}

static void test_bus_add_and_get(void) {
    Database *db = setup_db();

    Bus bus = {0};
    snprintf(bus.bus_number, sizeof(bus.bus_number), "D400DD");
    snprintf(bus.name, sizeof(bus.name), "Neoplan Starliner");
    snprintf(bus.model, sizeof(bus.model), "Neoplan Starliner");
    bus.total_mileage = 87000;
    bus.capacity = 52;
    snprintf(bus.status, sizeof(bus.status), "available");
    static const unsigned char photo[] = "bus-photo";
    bus.photo = (unsigned char*)photo;
    bus.photo_size = (int)sizeof(photo) - 1;

    expect_int(0, bus_add(db, &bus), "bus: add bus");

    Bus loaded = {0};
    expect_int(0, bus_get_by_id(db, bus.id, &loaded), "bus: get added bus");
    expect_true(strcmp(loaded.bus_number, "D400DD") == 0, "bus: bus number matches");
    expect_true(strcmp(loaded.name, "Neoplan Starliner") == 0, "bus: name matches");
    expect_int(87000, loaded.total_mileage, "bus: mileage matches");
    bus_free(&loaded);
    destroy_db(db);
}

static void test_bus_update(void) {
    Database *db = setup_db();

    Bus bus = {0};
    expect_int(0, bus_get_by_id(db, 1, &bus), "bus: load base bus");
    snprintf(bus.name, sizeof(bus.name), "Updated Bus");
    bus.total_mileage = 200000;
    expect_int(0, bus_update(db, &bus), "bus: update bus");

    Bus updated = {0};
    expect_int(0, bus_get_by_id(db, 1, &updated), "bus: reload updated bus");
    expect_true(strcmp(updated.name, "Updated Bus") == 0, "bus: updated name stored");
    expect_int(200000, updated.total_mileage, "bus: updated mileage stored");
    bus_free(&bus);
    bus_free(&updated);
    destroy_db(db);
}

static void test_bus_save_photo_to_file(void) {
    Database *db = setup_db();
    const char *path = "/tmp/lab4_bus_photo.bin";
    expect_int(0, bus_save_photo_to_file(db, 1, path), "bus: save photo to file");

    FILE *file = fopen(path, "rb");
    expect_true(file != NULL, "bus: saved photo file exists");
    if (file) {
        char buffer[32];
        size_t read = fread(buffer, 1, sizeof(buffer), file);
        expect_true(read > 0, "bus: saved photo file has data");
        fclose(file);
        remove(path);
    }

    destroy_db(db);
}

static void test_bus_delete_blocked_by_trip(void) {
    Database *db = setup_db();
    int rc = bus_delete(db, 1);
    expect_int(-2, rc, "bus: cannot delete bus with related trips");
    destroy_db(db);
}

static void test_crew_add_and_get(void) {
    Database *db = setup_db();

    CrewMember crew = {0};
    snprintf(crew.last_name, sizeof(crew.last_name), "Кузнецов");
    snprintf(crew.tax_id, sizeof(crew.tax_id), "AA1004");
    crew.experience = 4;
    snprintf(crew.category, sizeof(crew.category), "D");
    snprintf(crew.address, sizeof(crew.address), "Минск, пр-т Победителей, 7");
    crew.birth_year = 1992;
    crew.bus_id = 1;

    expect_int(0, crew_add(db, &crew), "crew: add member");

    CrewMember loaded = {0};
    expect_int(0, crew_get_by_id(db, crew.id, &loaded), "crew: get member by id");
    expect_true(strcmp(loaded.last_name, "Кузнецов") == 0, "crew: last name matches");
    expect_int(1, loaded.bus_id, "crew: bus id matches");
    destroy_db(db);
}

static void test_crew_update(void) {
    Database *db = setup_db();

    CrewMember crew = {0};
    expect_int(0, crew_get_by_id(db, 2, &crew), "crew: load seeded member");
    crew.experience = 15;
    snprintf(crew.category, sizeof(crew.category), "E");
    expect_int(0, crew_update(db, &crew), "crew: update member");

    CrewMember updated = {0};
    expect_int(0, crew_get_by_id(db, 2, &updated), "crew: reload updated member");
    expect_int(15, updated.experience, "crew: updated experience stored");
    expect_true(strcmp(updated.category, "E") == 0, "crew: updated category stored");
    destroy_db(db);
}

static void test_crew_salary_calculation(void) {
    Database *db = setup_db();
    expect_int(0, calculateCrewSalary(db, "2026-04-01", "2026-04-30", 10.0), "crew: calculate salary");

    CrewSalary *salaries = NULL;
    int count = 0;
    expect_int(0, crew_get_salary_for_period(db, "2026-04-01", "2026-04-30", &salaries, &count), "crew: load salary rows");
    expect_int(3, count, "crew: salary row count");
    expect_double(201.0, salaries[0].amount, 0.01, "crew: salary for crew #1");
    expect_double(157.5, salaries[1].amount, 0.01, "crew: salary for crew #2");
    expect_double(54.0, salaries[2].amount, 0.01, "crew: salary for crew #3");
    crew_free_salary_list(salaries);
    destroy_db(db);
}

static void run_test_group(const char *name, void (*tests[])(void), size_t count) {
    printf("\n[%s]\n", name);
    for (size_t i = 0; i < count; ++i) {
        tests[i]();
    }
}

int main(void) {
    void (*auth_tests[])(void) = {
        test_auth_login_success,
        test_auth_login_failure,
        test_auth_client_role,
    };
    void (*bus_tests[])(void) = {
        test_bus_add_and_get,
        test_bus_update,
        test_bus_save_photo_to_file,
        test_bus_delete_blocked_by_trip,
    };
    void (*crew_tests[])(void) = {
        test_crew_add_and_get,
        test_crew_update,
        test_crew_salary_calculation,
    };

    run_test_group("AUTH", auth_tests, sizeof(auth_tests) / sizeof(auth_tests[0]));
    run_test_group("BUS", bus_tests, sizeof(bus_tests) / sizeof(bus_tests[0]));
    run_test_group("CREW", crew_tests, sizeof(crew_tests) / sizeof(crew_tests[0]));

    printf("\n=== Итог ===\n");
    printf("Всего тестов: %d\n", tests_run);
    printf("Провалено: %d\n", tests_failed);

    return tests_failed == 0 ? 0 : 1;
}
