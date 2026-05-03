#ifndef REPORTS_H
#define REPORTS_H

#include "database.h"

int report_bus_trips_period(Database *db, int bus_id, const char *start_date, const char *end_date);
int report_bus_summary(Database *db, int bus_id, const char *start_date, const char *end_date);
int report_crew_salary_period(Database *db, const char *start_date, const char *end_date);
int report_most_expensive_route(Database *db);
int report_bus_with_max_mileage(Database *db);

#endif
