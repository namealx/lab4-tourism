#ifndef ORDER_H
#define ORDER_H

#include "database.h"

typedef struct {
    int id;
    int client_id;
    int bus_id;
    int route_id;
    char departure_date[11];  // YYYY-MM-DD
    int passengers_count;
    double total_cost;
    char status[20];  // pending, confirmed, cancelled
} Order;

// Создание таблицы заказов
int order_create_table(Database *db);

// CRUD операции
int order_add(Database *db, Order *order);
int order_get_by_id(Database *db, int id, Order *order);
int order_get_by_client(Database *db, int client_id, Order **orders, int *count);
int order_get_all(Database *db, Order **orders, int *count);
int order_update(Database *db, Order *order);
int order_delete(Database *db, int id);

// Специальные операции
int order_confirm(Database *db, int id);
int order_cancel(Database *db, int id);
double order_calculate_total(double price_per_ticket, int passengers_count);

// Освобождение памяти
void order_free_list(Order *orders);

#endif
