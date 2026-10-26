CC ?= cc
BUILD_DIR ?= build/release
BIN_DIR ?= bin
TARGET = $(BIN_DIR)/theartoffun
CPPFLAGS += -Iinclude -D_POSIX_C_SOURCE=200809L
CFLAGS ?= -O2
CFLAGS += -std=c11 -Wall -Wextra
LDLIBS += -lm

ifeq ($(DEBUG),1)
BUILD_DIR = build/debug
BIN_DIR = bin/debug
CFLAGS += -O0 -g3
endif

ifeq ($(SANITIZE),1)
BUILD_DIR = build/sanitize
BIN_DIR = bin/sanitize
CFLAGS += -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined
LDFLAGS += -fsanitize=address,undefined
endif
