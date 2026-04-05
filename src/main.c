#include <stdio.h>
#include "database.h"
#include "auth.h"
#include "route.h"
#include "order.h"
#include "bus.h"
#include "menu.h"

int init_database(Database *db) {
    // Создание всех таблиц
    if (auth_create_table(db) != SQLITE_OK) return -1;
    if (bus_create_table(db) != SQLITE_OK) return -1;
    if (route_create_table(db) != SQLITE_OK) return -1;
    if (order_create_table(db) != SQLITE_OK) return -1;
    
    // Проверяем, есть ли admin, если нет — создаём
    User *admin = auth_login(db, "admin", "admin");
    if (!admin) {
        auth_register(db, "admin", "admin", "Администратор", "admin");
        auth_register(db, "manager", "manager", "Менеджер", "manager");
    } else {
        free(admin);
    }
    
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
