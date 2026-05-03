#ifndef AUTH_H
#define AUTH_H

#include "database.h"

typedef struct {
    int id;
    char login[50];
    char username[50];
    char password[100];
    char full_name[100];
    char role[20];
} User;

// Создание таблицы пользователей
int auth_create_table(Database *db);

// Регистрация и вход
int auth_register(Database *db, const char *login, const char *password, const char *full_name, const char *role);
User* auth_login(Database *db, const char *login, const char *password);

// Получение роли текущего пользователя
const char* auth_get_current_user_role(User *user);
const char* getCurrentUserRole(User *user);

// Проверка прав
int auth_is_admin(User *user);
int auth_is_manager(User *user);
int auth_is_client(User *user);

// Выход
void auth_logout(User *user);

#endif
