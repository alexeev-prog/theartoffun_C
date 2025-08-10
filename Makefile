SRC_DIR := src
BIN_DIR := bin
TARGET := $(BIN_DIR)/theartoffun

SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(SRC_FILES))

CC := gcc
CFLAGS := -Wall -Wextra -O2
LDFLAGS := -lm -lrt

.PHONY: all clean

all: $(BIN_DIR) $(TARGET)

$(BIN_DIR):
	@mkdir -p $@

$(TARGET): $(OBJ_FILES)
	$(CC) $(LDFLAGS) $^ -o $@

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(BIN_DIR)
