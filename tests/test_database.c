#include <stdio.h>
#include "../includes/database.h"

int main() {
    Database *db = db_open("test.db");
    if (db) {
        printf("Database test: OK\n");
        db_close(db);
    } else {
        printf("Database test: FAILED\n");
        return 1;
    }
    return 0;
}
