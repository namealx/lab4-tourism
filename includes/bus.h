#ifndef BUS_H
#define BUS_H

#include "database.h"

typedef struct {
    int id;
    char bus_number[20];
    char model[50];
    int capacity;
    char status[20];
} Bus;

// Создание таблицы автобусов
int bus_create_table(Database *db);

// CRUD операции
int bus_add(Database *db, Bus *bus);
int bus_get_by_id(Database *db, int id, Bus *bus);
int bus_get_all(Database *db, Bus **buses, int *count);
int bus_get_available(Database *db, Bus **buses, int *count);
int bus_update(Database *db, Bus *bus);
int bus_delete(Database *db, int id);

// Изменение статуса
int bus_set_status(Database *db, int id, const char *status);

// Освобождение памяти
void bus_free_list(Bus *buses);

#endif
