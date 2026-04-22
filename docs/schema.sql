PRAGMA foreign_keys = ON;

-- Таблица пользователей
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    login TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL,
    full_name TEXT NOT NULL,
    role TEXT CHECK(role IN ('admin', 'manager', 'crew', 'client', 'customer')) NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Таблица автобусов
CREATE TABLE IF NOT EXISTS buses (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    bus_number TEXT UNIQUE NOT NULL,
    name TEXT NOT NULL DEFAULT '',
    total_mileage INTEGER NOT NULL DEFAULT 0,
    photo BLOB,
    model TEXT NOT NULL DEFAULT '',
    capacity INTEGER NOT NULL DEFAULT 0,
    status TEXT DEFAULT 'available'
);

-- Таблица маршрутов
CREATE TABLE IF NOT EXISTS routes (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    route_name TEXT UNIQUE NOT NULL,
    start_point TEXT NOT NULL,
    end_point TEXT NOT NULL,
    length INTEGER NOT NULL CHECK(length > 0),
    price_per_ticket REAL NOT NULL DEFAULT 0
);

-- Таблица экипажа
CREATE TABLE IF NOT EXISTS crew (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    last_name TEXT NOT NULL,
    tax_id TEXT UNIQUE NOT NULL,
    experience INTEGER NOT NULL DEFAULT 0,
    category TEXT NOT NULL DEFAULT '',
    address TEXT NOT NULL DEFAULT '',
    birth_year INTEGER NOT NULL DEFAULT 0,
    bus_id INTEGER NOT NULL,
    FOREIGN KEY (bus_id) REFERENCES buses(id)
);

-- Таблица рейсов
CREATE TABLE IF NOT EXISTS trips (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    bus_id INTEGER NOT NULL,
    crew_id INTEGER NOT NULL,
    departure_date DATE NOT NULL,
    arrival_date DATE NOT NULL,
    route_id INTEGER NOT NULL,
    passengers_count INTEGER NOT NULL DEFAULT 0,
    ticket_price REAL NOT NULL DEFAULT 0,
    FOREIGN KEY (bus_id) REFERENCES buses(id),
    FOREIGN KEY (crew_id) REFERENCES crew(id),
    FOREIGN KEY (route_id) REFERENCES routes(id),
    CHECK (departure_date < arrival_date),
    CHECK (passengers_count >= 0),
    CHECK (ticket_price >= 0)
);

-- Таблица начислений экипажам
CREATE TABLE IF NOT EXISTS crew_salary (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    crew_id INTEGER NOT NULL,
    period_start TEXT NOT NULL,
    period_end TEXT NOT NULL,
    amount REAL NOT NULL,
    calculated_date TEXT NOT NULL DEFAULT (DATE('now')),
    FOREIGN KEY (crew_id) REFERENCES crew(id)
);

-- Таблица заказов
CREATE TABLE IF NOT EXISTS orders (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    client_id INTEGER NOT NULL,
    bus_id INTEGER NOT NULL,
    route_id INTEGER NOT NULL,
    departure_date DATE NOT NULL,
    passengers_count INTEGER NOT NULL,
    total_cost REAL NOT NULL,
    status TEXT DEFAULT 'pending',
    FOREIGN KEY (client_id) REFERENCES users(id),
    FOREIGN KEY (bus_id) REFERENCES buses(id),
    FOREIGN KEY (route_id) REFERENCES routes(id)
);

-- Тестовые данные
INSERT OR IGNORE INTO users (login, password, full_name, role) VALUES
('admin', '210706300046', 'Администратор', 'admin'),
('manager', '229474541853792', 'Менеджер', 'manager'),
('crew', '6385126262', 'Экипаж', 'crew'),
('customer', '7572259744610103', 'Клиент', 'customer');

INSERT OR IGNORE INTO buses (bus_number, name, total_mileage, photo, model, capacity, status) VALUES
('A100AA', 'Setra ComfortClass', 124000, X'6275732D70686F746F2D31', 'Setra S 516 HD', 49, 'available'),
('B200BB', 'Mercedes Tourismo', 98000, X'6275732D70686F746F2D32', 'Mercedes-Benz Tourismo', 45, 'available'),
('C300CC', 'MAN Lion''s Coach', 156500, X'6275732D70686F746F2D33', 'MAN Lion''s Coach', 53, 'available');

INSERT OR IGNORE INTO routes (route_name, start_point, end_point, length, price_per_ticket) VALUES
('Минск - Несвиж', 'Минск', 'Несвиж', 120, 25.00),
('Минск - Брест', 'Минск', 'Брест', 350, 45.00),
('Гродно - Лида', 'Гродно', 'Лида', 110, 30.00);

INSERT OR IGNORE INTO crew (last_name, tax_id, experience, category, address, birth_year, bus_id) VALUES
('Иванов', 'AA1001', 8, 'A', 'Минск, ул. Ленина, 1', 1987, 1),
('Петров', 'AA1002', 11, 'B', 'Брест, ул. Советская, 10', 1983, 2),
('Сидоров', 'AA1003', 5, 'C', 'Гродно, ул. Кирова, 5', 1990, 3);

INSERT OR IGNORE INTO trips (bus_id, crew_id, departure_date, arrival_date, route_id, passengers_count, ticket_price) VALUES
(1, 1, '2026-04-01', '2026-04-02', 1, 30, 25.00),
(1, 1, '2026-04-05', '2026-04-06', 2, 28, 45.00),
(2, 2, '2026-04-03', '2026-04-04', 2, 35, 45.00),
(3, 3, '2026-04-08', '2026-04-09', 3, 18, 30.00);
