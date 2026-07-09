# © 2026 Sushii64
# © 2026 robinpie

CC = gcc
CFLAGS = -std=c11 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -Wall -Wextra -Wno-unused-parameter -g -Iruntime -Isrc

SRC = src/lexer.c src/parser.c src/codegen.c src/main.c
OBJ = $(SRC:.c=.o)

BIN = bin/oboe

all: $(BIN)

bin:
	mkdir -p bin

$(BIN): bin $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(BIN)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(BIN)
	./tests/run_tests.sh

clean:
	rm -f $(OBJ) $(BIN)
	rmdir bin 2>/dev/null || true

.PHONY: all test clean
