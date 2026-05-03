#ifndef TRIP_H
#define TRIP_H

#include "database.h"

typedef struct {
    int id;
    int bus_id;
    int crew_id;
    char departure_date[11];
    char arrival_date[11];
    int route_id;
    int passengers_count;
    double ticket_price;
} Trip;

int trip_create_table(Database *db);

int trip_add(Database *db, Trip *trip);
int trip_get_by_id(Database *db, int id, Trip *trip);
int trip_get_all(Database *db, Trip **trips, int *count);
int trip_get_by_bus_period(Database *db, int bus_id, const char *start_date, const char *end_date, Trip **trips, int *count);
int trip_update(Database *db, Trip *trip);
int trip_delete(Database *db, int id);

double trip_calculate_revenue(const Trip *trip);
void trip_free_list(Trip *trips);

#endif
