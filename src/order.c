#include "order.h"
#include "route.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int order_create_table(Database *db) {
    const char *sql = 
        "CREATE TABLE IF NOT EXISTS orders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "client_id INTEGER NOT NULL,"
        "bus_id INTEGER NOT NULL,"
        "route_id INTEGER NOT NULL,"
        "departure_date DATE NOT NULL,"
        "passengers_count INTEGER NOT NULL,"
        "total_cost REAL NOT NULL,"
        "status TEXT DEFAULT 'pending',"
        "FOREIGN KEY (client_id) REFERENCES users(id),"
        "FOREIGN KEY (bus_id) REFERENCES buses(id),"
        "FOREIGN KEY (route_id) REFERENCES routes(id)"
        ");";
    
    return db_execute(db, sql);
}

int order_add(Database *db, Order *order) {
    const char *sql = "INSERT INTO orders (client_id, bus_id, route_id, departure_date, "
                      "passengers_count, total_cost, status) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?);";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    sqlite3_bind_int(stmt, 1, order->client_id);
    sqlite3_bind_int(stmt, 2, order->bus_id);
    sqlite3_bind_int(stmt, 3, order->route_id);
    sqlite3_bind_text(stmt, 4, order->departure_date, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, order->passengers_count);
    sqlite3_bind_double(stmt, 6, order->total_cost);
    sqlite3_bind_text(stmt, 7, order->status, -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE) {
        order->id = sqlite3_last_insert_rowid(db->conn);
        return 0;
    }
    return -1;
}

int order_get_by_id(Database *db, int id, Order *order) {
    const char *sql = "SELECT id, client_id, bus_id, route_id, departure_date, "
                      "passengers_count, total_cost, status FROM orders WHERE id = ?;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    sqlite3_bind_int(stmt, 1, id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        order->id = sqlite3_column_int(stmt, 0);
        order->client_id = sqlite3_column_int(stmt, 1);
        order->bus_id = sqlite3_column_int(stmt, 2);
        order->route_id = sqlite3_column_int(stmt, 3);
        strcpy(order->departure_date, (const char*)sqlite3_column_text(stmt, 4));
        order->passengers_count = sqlite3_column_int(stmt, 5);
        order->total_cost = sqlite3_column_double(stmt, 6);
        strcpy(order->status, (const char*)sqlite3_column_text(stmt, 7));
        sqlite3_finalize(stmt);
        return 0;
    }
    
    sqlite3_finalize(stmt);
    return -1;
}

int order_get_by_client(Database *db, int client_id, Order **orders, int *count) {
    const char *sql = "SELECT id, client_id, bus_id, route_id, departure_date, "
                      "passengers_count, total_cost, status FROM orders "
                      "WHERE client_id = ? ORDER BY departure_date DESC;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    sqlite3_bind_int(stmt, 1, client_id);
    
    *count = 0;
    *orders = NULL;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Order *new_orders = realloc(*orders, sizeof(Order) * (*count + 1));
        if (!new_orders) {
            sqlite3_finalize(stmt);
            free(*orders);
            return -1;
        }
        *orders = new_orders;
        
        Order *o = &(*orders)[*count];
        o->id = sqlite3_column_int(stmt, 0);
        o->client_id = sqlite3_column_int(stmt, 1);
        o->bus_id = sqlite3_column_int(stmt, 2);
        o->route_id = sqlite3_column_int(stmt, 3);
        strcpy(o->departure_date, (const char*)sqlite3_column_text(stmt, 4));
        o->passengers_count = sqlite3_column_int(stmt, 5);
        o->total_cost = sqlite3_column_double(stmt, 6);
        strcpy(o->status, (const char*)sqlite3_column_text(stmt, 7));
        
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return 0;
}

int order_get_all(Database *db, Order **orders, int *count) {
    const char *sql = "SELECT id, client_id, bus_id, route_id, departure_date, "
                      "passengers_count, total_cost, status FROM orders "
                      "ORDER BY departure_date DESC;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    *count = 0;
    *orders = NULL;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Order *new_orders = realloc(*orders, sizeof(Order) * (*count + 1));
        if (!new_orders) {
            sqlite3_finalize(stmt);
            free(*orders);
            return -1;
        }
        *orders = new_orders;
        
        Order *o = &(*orders)[*count];
        o->id = sqlite3_column_int(stmt, 0);
        o->client_id = sqlite3_column_int(stmt, 1);
        o->bus_id = sqlite3_column_int(stmt, 2);
        o->route_id = sqlite3_column_int(stmt, 3);
        strcpy(o->departure_date, (const char*)sqlite3_column_text(stmt, 4));
        o->passengers_count = sqlite3_column_int(stmt, 5);
        o->total_cost = sqlite3_column_double(stmt, 6);
        strcpy(o->status, (const char*)sqlite3_column_text(stmt, 7));
        
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return 0;
}

int order_update(Database *db, Order *order) {
    const char *sql = "UPDATE orders SET client_id = ?, bus_id = ?, route_id = ?, "
                      "departure_date = ?, passengers_count = ?, total_cost = ?, status = ? "
                      "WHERE id = ?;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    sqlite3_bind_int(stmt, 1, order->client_id);
    sqlite3_bind_int(stmt, 2, order->bus_id);
    sqlite3_bind_int(stmt, 3, order->route_id);
    sqlite3_bind_text(stmt, 4, order->departure_date, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, order->passengers_count);
    sqlite3_bind_double(stmt, 6, order->total_cost);
    sqlite3_bind_text(stmt, 7, order->status, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 8, order->id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? 0 : -1;
}

int order_delete(Database *db, int id) {
    const char *sql = "DELETE FROM orders WHERE id = ?;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    sqlite3_bind_int(stmt, 1, id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? 0 : -1;
}

int order_confirm(Database *db, int id) {
    const char *sql = "UPDATE orders SET status = 'confirmed' WHERE id = ?;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    sqlite3_bind_int(stmt, 1, id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? 0 : -1;
}

int order_cancel(Database *db, int id) {
    const char *sql = "UPDATE orders SET status = 'cancelled' WHERE id = ?;";
    
    sqlite3_stmt *stmt = db_query(db, sql);
    if (!stmt) return -1;
    
    sqlite3_bind_int(stmt, 1, id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? 0 : -1;
}

double order_calculate_total(double price_per_ticket, int passengers_count) {
    return price_per_ticket * passengers_count;
}

void order_free_list(Order *orders) {
    if (orders) free(orders);
}
