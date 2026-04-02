CC      = gcc
CFLAGS  = -Wall -Wextra -pthread -Iinclude
TARGET  = server
SRCS    = src/main.c src/server.c src/protocol.c src/sensor_manager.c \
          src/alert_engine.c src/auth_client.c src/http_server.c src/logger.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
