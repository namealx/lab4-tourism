-- Таблица пользователей
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    full_name TEXT NOT NULL,
    role TEXT CHECK(role IN ('admin', 'manager', 'client')) NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Таблица автобусов
CREATE TABLE IF NOT EXISTS buses (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    bus_number TEXT UNIQUE NOT NULL,
    model TEXT NOT NULL,
    capacity INTEGER NOT NULL,
    status TEXT DEFAULT 'available'
);

-- Таблица маршрутов
CREATE TABLE IF NOT EXISTS tour_routes (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    start_point TEXT NOT NULL,
    end_point TEXT NOT NULL,
    distance INTEGER,
    price_per_ticket REAL NOT NULL
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
    FOREIGN KEY (route_id) REFERENCES tour_routes(id)
);

-- Тестовые данные
INSERT OR IGNORE INTO users (username, password_hash, full_name, role) VALUES 
('admin', 'admin123', 'Администратор', 'admin'),
('manager1', 'pass123', 'Иван Петров', 'manager');

INSERT OR IGNORE INTO buses (bus_number, model, capacity) VALUES 
('A123BC', 'MAN Lion\'s Coach', 50),
('B456OP', 'Mercedes-Benz Tourismo', 45);

INSERT OR IGNORE INTO tour_routes (name, start_point, end_point, distance, price_per_ticket) VALUES 
('Минск — Несвиж', 'Минск', 'Несвиж', 120, 25.00),
('Минск — Брест', 'Минск', 'Брест', 350, 45.00);
