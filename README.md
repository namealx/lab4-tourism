# Туристическое бюро

[![CI](https://github.com/namealx/lab4-tourism/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/namealx/lab4-tourism/actions/workflows/build.yml)
[![Codecov](https://codecov.io/gh/namealx/lab4-tourism/branch/main/graph/badge.svg)](https://codecov.io/gh/namealx/lab4-tourism)
[![GitHub Pages](https://img.shields.io/badge/GitHub%20Pages-live-2d8cf0)](https://namealx.github.io/lab4-tourism/)

## Описание

Консольное приложение на `C` и `SQLite` для туристического бюро.
В проекте реализованы:

- аутентификация и авторизация пользователей;
- управление автобусами;
- управление экипажем;
- управление маршрутами;
- управление рейсами;
- отчёты и выборки по данным;
- расчёт начислений экипажам;
- unit-тесты и coverage через `gcov`.

## Установка

1. Установите `SQLite` и `gcc`/`clang`.
2. Откройте репозиторий в терминале.
3. Соберите проект командой `make`.

## Использование

- `make` — сборка приложения.
- `make run` — запуск консольного приложения.
- `make check` — запуск unit-тестов.
- `make coverage` — запуск тестов с генерацией coverage-отчёта.
- `make clean` — очистка артефактов сборки.

## Онлайн

- [GitHub Pages](https://namealx.github.io/lab4-tourism/)
- [Codecov](https://codecov.io/gh/namealx/lab4-tourism)
- [Wiki проекта](https://github.com/fpmi-tpmp2026/tpmp-gr12b-lab4-namealteam/wiki/Home)

## Авторы
- Юранов Никита - [namealx](https://github.com/namealx)
- Насенник Иван — [IvanNas-cmd](https://github.com/IvanNas-cmd)
  

## Ссылки и дополнительные сведения

- SQL-схема и пример данных:
  [docs/schema.sql](./docs/schema.sql)
