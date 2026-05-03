#ifndef CREW_H
#define CREW_H

#include "database.h"

typedef struct {
    int id;
    char last_name[100];
    char tax_id[32];
    int experience;
    char category[32];
    char address[128];
    int birth_year;
    int bus_id;
} CrewMember;

typedef struct {
    int id;
    int crew_id;
    char period_start[11];
    char period_end[11];
    double amount;
    char calculated_date[20];
} CrewSalary;

int crew_create_table(Database *db);
int crew_create_salary_table(Database *db);

int crew_add(Database *db, CrewMember *crew);
int crew_get_by_id(Database *db, int id, CrewMember *crew);
int crew_get_all(Database *db, CrewMember **crew_list, int *count);
int crew_update(Database *db, CrewMember *crew);
int crew_delete(Database *db, int id);

int calculateCrewSalary(Database *db, const char *start_date, const char *end_date, double percent);
int crew_get_salary_for_period(Database *db, const char *start_date, const char *end_date, CrewSalary **salary_list, int *count);

void crew_free_list(CrewMember *crew_list);
void crew_free_salary_list(CrewSalary *salary_list);

#endif
