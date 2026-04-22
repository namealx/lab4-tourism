#include "menu.h"
#include "route.h"
#include "order.h"
#include "bus.h"
#include "crew.h"
#include "trip.h"
#include "reports.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void clear_screen() {
    printf("\033[2J\033[1;1H");
}

void press_enter() {
    printf("\nНажмите Enter для продолжения...");
    while (getchar() != '\n');
    getchar();
}

static void show_reports_menu(Database *db) {
    int choice;
    do {
        clear_screen();
        printf("=== Отчеты и выборки ===\n");
        printf("1. Рейсы автобуса за период\n");
        printf("2. Статистика автобуса за период\n");
        printf("3. Рассчитать зарплату экипажей\n");
        printf("4. Показать зарплату экипажей за период\n");
        printf("5. Самый дорогой маршрут\n");
        printf("6. Автобус с наибольшим пробегом\n");
        printf("0. Назад\n");
        printf("Выберите действие: ");
        scanf("%d", &choice);

        if (choice == 1 || choice == 2) {
            int bus_id;
            char start_date[11], end_date[11];
            printf("Введите ID автобуса: ");
            scanf("%d", &bus_id);
            printf("Начальная дата (YYYY-MM-DD): ");
            scanf("%10s", start_date);
            printf("Конечная дата (YYYY-MM-DD): ");
            scanf("%10s", end_date);
            if (choice == 1) {
                report_bus_trips_period(db, bus_id, start_date, end_date);
            } else {
                report_bus_summary(db, bus_id, start_date, end_date);
            }
            press_enter();
        } else if (choice == 3 || choice == 4) {
            char start_date[11], end_date[11];
            double percent = 0.0;
            printf("Начальная дата (YYYY-MM-DD): ");
            scanf("%10s", start_date);
            printf("Конечная дата (YYYY-MM-DD): ");
            scanf("%10s", end_date);
            if (choice == 3) {
                printf("Процент начисления: ");
                scanf("%lf", &percent);
                if (calculateCrewSalary(db, start_date, end_date, percent) == 0) {
                    printf("Зарплата экипажей рассчитана.\n");
                } else {
                    printf("Ошибка расчета зарплаты.\n");
                }
            } else {
                report_crew_salary_period(db, start_date, end_date);
            }
            press_enter();
        } else if (choice == 5) {
            report_most_expensive_route(db);
            press_enter();
        } else if (choice == 6) {
            report_bus_with_max_mileage(db);
            press_enter();
        }
    } while (choice != 0);
}

static void display_routes(Database *db) {
    TourRoute *routes = NULL;
    int count = 0;
    
    if (route_get_all(db, &routes, &count) == 0) {
        printf("\n=== Список маршрутов ===\n");
        for (int i = 0; i < count; i++) {
            printf("%d. %s | %s → %s | %d км | %.2f руб.\n",
                   routes[i].id, routes[i].name,
                   routes[i].start_point, routes[i].end_point,
                   routes[i].distance, routes[i].price_per_ticket);
        }
        route_free_list(routes);
    } else {
        printf("Ошибка загрузки маршрутов\n");
    }
}

static void book_tour(Database *db, int client_id) {
    display_routes(db);
    
    int route_id;
    printf("\nВведите ID маршрута: ");
    scanf("%d", &route_id);
    
    int passengers;
    printf("Введите количество пассажиров: ");
    scanf("%d", &passengers);
    
    Bus *buses = NULL;
    int bus_count = 0;
    if (bus_get_available(db, &buses, &bus_count) == 0 && bus_count > 0) {
        printf("\nДоступные автобусы:\n");
        for (int i = 0; i < bus_count; i++) {
            printf("%d. %s (%s) - %d мест\n", 
                   buses[i].id, buses[i].bus_number, buses[i].model, buses[i].capacity);
        }
        
        int bus_id;
        printf("Выберите ID автобуса: ");
        scanf("%d", &bus_id);
        
        TourRoute route;
        if (route_get_by_id(db, route_id, &route) == 0) {
            Order order;
            order.client_id = client_id;
            order.bus_id = bus_id;
            order.route_id = route_id;
            order.passengers_count = passengers;
            order.total_cost = order_calculate_total(route.price_per_ticket, passengers);
            strcpy(order.status, "pending");
            
            printf("\nСтоимость заказа: %.2f руб.\n", order.total_cost);
            printf("Подтвердить заказ? (y/n): ");
            char confirm;
            scanf(" %c", &confirm);
            
            if (confirm == 'y' || confirm == 'Y') {
                if (order_add(db, &order) == 0) {
                    printf("Заказ успешно создан! ID: %d\n", order.id);
                    bus_set_status(db, bus_id, "busy");
                } else {
                    printf("Ошибка создания заказа\n");
                }
            }
        }
        bus_free_list(buses);
    } else {
        printf("Нет доступных автобусов\n");
    }
}

void show_main_menu(Database *db) {
    int choice;
    int failed_attempts = 0;
    
    do {
        clear_screen();
        printf("=== Туристическое бюро ===\n");
        printf("1. Вход\n");
        printf("2. Регистрация\n");
        printf("0. Выход\n");
        printf("Выберите действие: ");
        scanf("%d", &choice);
        
        if (choice == 1) {
            char login[50], password[50];
            if (failed_attempts >= 3) {
                printf("Доступ к входу заблокирован после 3 неудачных попыток.\n");
                press_enter();
                continue;
            }
            printf("Логин: ");
            scanf("%49s", login);
            printf("Пароль: ");
            scanf("%49s", password);
            
            User *user = auth_login(db, login, password);
            if (user) {
                printf("Добро пожаловать, %s!\n", user->full_name);
                failed_attempts = 0;
                press_enter();
                
                if (auth_is_admin(user)) {
                    show_admin_menu(db, user);
                } else if (auth_is_manager(user)) {
                    show_manager_menu(db, user);
                } else {
                    show_client_menu(db, user);
                }
                auth_logout(user);
            } else {
                printf("Неверный логин или пароль\n");
                failed_attempts++;
                press_enter();
            }
        } else if (choice == 2) {
            char login[50], password[50], full_name[100];
            printf("Логин: ");
            scanf("%49s", login);
            printf("Пароль: ");
            scanf("%49s", password);
            printf("ФИО: ");
            scanf(" %[^\n]", full_name);
            
            if (auth_register(db, login, password, full_name, "customer") == 0) {
                printf("Регистрация успешна!\n");
            } else {
                printf("Ошибка регистрации\n");
            }
            press_enter();
        }
    } while (choice != 0);
}

void show_client_menu(Database *db, User *user) {
    int choice;
    do {
        clear_screen();
        printf("=== Клиентское меню ===\n");
        printf("1. Просмотр маршрутов\n");
        printf("2. Бронирование тура\n");
        printf("3. Мои заказы\n");
        printf("0. Выход\n");
        printf("Выберите действие: ");
        scanf("%d", &choice);
        
        if (choice == 1) {
            display_routes(db);
            press_enter();
        } else if (choice == 2) {
            book_tour(db, user->id);
            press_enter();
        } else if (choice == 3) {
            Order *orders = NULL;
            int count = 0;
            if (order_get_by_client(db, user->id, &orders, &count) == 0) {
                printf("\n=== Мои заказы ===\n");
                for (int i = 0; i < count; i++) {
                    printf("%d. ID заказа: %d | Статус: %s | Сумма: %.2f руб.\n",
                           i+1, orders[i].id, orders[i].status, orders[i].total_cost);
                }
                order_free_list(orders);
            }
            press_enter();
        }
    } while (choice != 0);
}

void show_admin_menu(Database *db, User *user) {
    int choice;
    do {
        clear_screen();
        printf("=== Администратор: %s ===\n", user->full_name);
        printf("1. Управление автобусами\n");
        printf("2. Управление маршрутами\n");
        printf("3. Все заказы\n");
        printf("4. Отчеты и выборки\n");
        printf("0. Выход\n");
        printf("Выберите действие: ");
        scanf("%d", &choice);
        
        if (choice == 1) {
            printf("Управление автобусами (разработка в процессе)\n");
            press_enter();
        } else if (choice == 2) {
            printf("Управление маршрутами (разработка в процессе)\n");
            press_enter();
        } else if (choice == 3) {
            Order *orders = NULL;
            int count = 0;
            if (order_get_all(db, &orders, &count) == 0) {
                printf("\n=== Все заказы ===\n");
                for (int i = 0; i < count; i++) {
                    printf("%d. Клиент ID: %d | Статус: %s | Сумма: %.2f руб.\n",
                           i+1, orders[i].client_id, orders[i].status, orders[i].total_cost);
                }
                order_free_list(orders);
            }
            press_enter();
        } else if (choice == 4) {
            show_reports_menu(db);
        }
    } while (choice != 0);
}

void show_manager_menu(Database *db, User *user) {
    int choice;
    do {
        clear_screen();
        printf("=== Менеджер: %s ===\n", user->full_name);
        printf("1. Управление маршрутами\n");
        printf("2. Подтверждение заказов\n");
        printf("3. Все заказы\n");
        printf("4. Отчеты и выборки\n");
        printf("0. Выход\n");
        printf("Выберите действие: ");
        scanf("%d", &choice);
        
        if (choice == 1) {
            printf("Управление маршрутами (разработка в процессе)\n");
            press_enter();
        } else if (choice == 2) {
            Order *orders = NULL;
            int count = 0;
            if (order_get_all(db, &orders, &count) == 0) {
                for (int i = 0; i < count; i++) {
                    if (strcmp(orders[i].status, "pending") == 0) {
                        printf("%d. Заказ ID: %d | Сумма: %.2f руб.\n",
                               i+1, orders[i].id, orders[i].total_cost);
                    }
                }
                printf("Введите ID заказа для подтверждения: ");
                int order_id;
                scanf("%d", &order_id);
                if (order_confirm(db, order_id) == 0) {
                    printf("Заказ подтверждён\n");
                }
                order_free_list(orders);
            }
            press_enter();
        } else if (choice == 3) {
            Order *orders = NULL;
            int count = 0;
            if (order_get_all(db, &orders, &count) == 0) {
                printf("\n=== Все заказы ===\n");
                for (int i = 0; i < count; i++) {
                    printf("%d. Клиент ID: %d | Статус: %s | Сумма: %.2f руб.\n",
                           i+1, orders[i].client_id, orders[i].status, orders[i].total_cost);
                }
                order_free_list(orders);
            }
            press_enter();
        } else if (choice == 4) {
            show_reports_menu(db);
        }
    } while (choice != 0);
}
