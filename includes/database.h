#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

typedef struct {
    sqlite3 *conn;
    char *last_error;
} Database;

Database* db_open(const char *filename);
void db_close(Database *db);
int db_execute(Database *db, const char *sql);
sqlite3_stmt* db_query(Database *db, const char *sql);
const char* db_last_error(Database *db);

#endif
