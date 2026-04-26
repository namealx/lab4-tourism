CC = gcc
CFLAGS = -Wall -Wextra -Iincludes -g
LDFLAGS = -lsqlite3 -lm

# Флаги для тестов и покрытия
TEST_CFLAGS = -Wall -Wextra -Iincludes -g --coverage
TEST_LDFLAGS = -lsqlite3 -lm --coverage

# Директории
SRCDIR = src
INCDIR = includes
BUILDDIR = build
BINDIR = bin
TESTDIR = tests

# Поиск всех .c файлов
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SOURCES))
TARGET = $(BINDIR)/tourism_app

# Поиск всех тестов (все .c файлы в папке tests/)
TEST_SOURCES = $(wildcard $(TESTDIR)/*.c)
TEST_OBJECTS = $(patsubst $(TESTDIR)/%.c, $(BUILDDIR)/%.o, $(TEST_SOURCES))

# Исключаем main.c из тестов (если нужно тестировать без main)
SOURCES_WITHOUT_MAIN = $(filter-out $(SRCDIR)/main.c, $(SOURCES))
OBJECTS_WITHOUT_MAIN = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SOURCES_WITHOUT_MAIN))

TEST_TARGET = $(BINDIR)/tourism_tests

# Цель по умолчанию
all: directories $(TARGET)

# Создание директорий
directories:
	mkdir -p $(BUILDDIR) $(BINDIR)

# Сборка основного приложения
$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

# Компиляция исходников приложения
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Компиляция тестов
$(BUILDDIR)/%.o: $(TESTDIR)/%.c
	$(CC) $(TEST_CFLAGS) -c $< -o $@

# Сборка тестового приложения (без main.c, чтобы избежать конфликта)
$(TEST_TARGET): directories $(TEST_OBJECTS) $(OBJECTS_WITHOUT_MAIN)
	$(CC) $^ -o $@ $(TEST_LDFLAGS)

# Запуск основного приложения
run: $(TARGET)
	./$(TARGET)

# Запуск тестов (согласно требованию лабы)
check: directories $(TEST_TARGET)
	./$(TEST_TARGET)

# Проверка покрытия кода
coverage: clean
	$(MAKE) TEST_CFLAGS="$(TEST_CFLAGS)" TEST_LDFLAGS="$(TEST_LDFLAGS)" $(TEST_TARGET)
	./$(TEST_TARGET)
	gcov -o $(BUILDDIR) $(SRCDIR)/*.c $(TESTDIR)/*.c
	@echo "Coverage report generated. Run 'gcov -r *.gcov' to view"

# Полная проверка (согласно требованию лабы, страница 5)
distcheck: clean
	$(MAKE) all
	$(MAKE) check
	$(MAKE) clean
	@echo "distcheck completed successfully!"

# Очистка
clean:
	rm -rf $(BUILDDIR)/*.o $(BUILDDIR)/*.gcda $(BUILDDIR)/*.gcno
	rm -f $(TARGET) $(TEST_TARGET)
	rm -f *.gcov *.gcda *.gcno
	rm -f coverage.xml coverage.html

# Полная пересборка
rebuild: clean all

# Объявление phony целей
.PHONY: all clean run check coverage distcheck rebuild directories
