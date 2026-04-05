CC = gcc
CFLAGS = -Wall -Wextra -Iincludes -g
LDFLAGS = -lsqlite3 -lm

SRCDIR = src
INCDIR = includes
BUILDDIR = build
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SOURCES))
TARGET = $(BINDIR)/tourism_app

all: directories $(TARGET)

directories:
	mkdir -p $(BUILDDIR) $(BINDIR)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR)/*.o $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
