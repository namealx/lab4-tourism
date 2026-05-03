CC = gcc
CFLAGS = -Wall -Wextra -Iincludes -g
LDFLAGS = -lsqlite3 -lm

SRCDIR = src
INCDIR = includes
BUILDDIR = build
BINDIR = bin
COVERAGE_BUILDDIR = build/coverage
COVERAGE_BINDIR = bin/coverage

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SOURCES))
TARGET = $(BINDIR)/tourism_app
TEST_SOURCES = $(filter-out $(SRCDIR)/main.c, $(SOURCES))
TEST_OBJECTS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(TEST_SOURCES))
TEST_TARGET = $(BINDIR)/tourism_tests
TEST_CFLAGS = -Wall -Wextra -Iincludes -g --coverage
TEST_LDFLAGS = -lsqlite3 -lm --coverage

all: directories $(TARGET)

directories:
	mkdir -p $(BUILDDIR) $(BINDIR)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/test_database.o: tests/test_database.c
	$(CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_TARGET): $(TEST_OBJECTS) $(BUILDDIR)/test_database.o
	$(CC) $^ -o $@ $(TEST_LDFLAGS)

clean:
	rm -rf $(BUILDDIR)/*.o $(BUILDDIR)/*.gcda $(BUILDDIR)/*.gcno $(BUILDDIR)/coverage $(BINDIR)/tourism_app $(BINDIR)/tourism_tests $(BINDIR)/coverage *.gcov coverage.info

run: $(TARGET)
	./$(TARGET)

check: directories $(TEST_TARGET)
	./$(TEST_TARGET)

coverage:
	$(MAKE) clean
	$(MAKE) BUILDDIR="$(COVERAGE_BUILDDIR)" BINDIR="$(COVERAGE_BINDIR)" CFLAGS="$(TEST_CFLAGS)" LDFLAGS="$(TEST_LDFLAGS)" check
	gcov -o $(COVERAGE_BUILDDIR) $(TEST_SOURCES) tests/test_database.c

.PHONY: all clean run check coverage
