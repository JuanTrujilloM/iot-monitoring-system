CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -Iinclude
LDFLAGS_LINUX = -pthread
LDFLAGS_WINDOWS = -lws2_32 -pthread

SRC = src/main.c src/server.c src/logger.c src/protocol.c src/sensor_manager.c src/alert_engine.c
OBJ = $(SRC:.c=.o)
BIN_LINUX = server
BIN_WINDOWS = server.exe

.PHONY: all linux windows clean

all: linux

linux: $(OBJ)
	$(CC) $(OBJ) -o $(BIN_LINUX) $(LDFLAGS_LINUX)

windows: $(OBJ)
	$(CC) $(OBJ) -o $(BIN_WINDOWS) $(LDFLAGS_WINDOWS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN_LINUX) $(BIN_WINDOWS)
