#ifndef MENU_H
#define MENU_H

#include "database.h"
#include "auth.h"

// Главное меню (до входа)
void show_main_menu(Database *db);

// Меню после входа
void show_admin_menu(Database *db, User *user);
void show_manager_menu(Database *db, User *user);
void show_client_menu(Database *db, User *user);

// Вспомогательные функции
void clear_screen();
void press_enter();

#endif
