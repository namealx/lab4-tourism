CC = gcc
CFLAGS = -Wall -Wextra -I./includes -g
LDFLAGS = -lsqlite3

SRC_DIR = src
INC_DIR = includes
BIN_DIR = bin
BUILD_DIR = build
TEST_DIR = tests

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
TARGET = $(BIN_DIR)/tourism_app

all: $(TARGET)

$(TARGET): $(OBJECTS)
	mkdir -p $(BIN_DIR)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

distcheck: clean
	$(MAKE) all

check:
	@echo "No tests yet. Add tests in $(TEST_DIR)/"

.PHONY: all clean distcheck check
