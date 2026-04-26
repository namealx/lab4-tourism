#ifndef ROUTE_H
#define ROUTE_H

#include "database.h"

typedef struct {
    int id;
    char route_name[100];
    char name[100];
    char start_point[100];
    char end_point[100];
    int length;
    int distance;
    double price_per_ticket;
} TourRoute;

// Создание таблицы маршрутов
int route_create_table(Database *db);

// CRUD операции
int route_add(Database *db, TourRoute *route);
int route_get_by_id(Database *db, int id, TourRoute *route);
int route_get_all(Database *db, TourRoute **routes, int *count);
int route_update(Database *db, TourRoute *route);
int route_delete(Database *db, int id);

// Поиск
int route_search_by_name(Database *db, const char *name, TourRoute **routes, int *count);

// Освобождение памяти
void route_free_list(TourRoute *routes);

#endif
