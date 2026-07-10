# SPDX-License-Identifier: GPL-2.0-only
# © 2026 Sushii64
# © 2026 robinpie
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.

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
