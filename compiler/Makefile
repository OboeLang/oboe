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
# _POSIX_C_SOURCE alone hides the BSD extensions we use (mkstemps): glibc still
# exposes them under _DEFAULT_SOURCE, but darwin needs _DARWIN_C_SOURCE to do
# the same. both are ignored by the libc they don't belong to.
CFLAGS = -std=c11 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -D_DARWIN_C_SOURCE -Wall -Wextra -Wno-unused-parameter -g -Iruntime -Isrc

SRC = src/lexer.c src/parser.c src/codegen.c src/dump.c src/projectjson.c \
      src/sha256.c src/izim.c src/record.c src/kabuk.c \
      src/katare.c src/projectedit.c src/pkg.c src/main.c
OBJ = $(SRC:.c=.o)
# every object depends on every header: the set is small, and without this a
# change to ast.h or the runtime header silently leaves stale objects linked in
HDR = src/ast.h src/lexer.h src/parser.h src/codegen.h src/dump.h \
      src/projectjson.h src/sha256.h src/izim.h src/record.h src/kabuk.h \
      src/katare.h src/projectedit.h src/pkg.h runtime/oboe_runtime.h

BIN = bin/oboe

all: $(BIN)

bin:
	mkdir -p bin

$(BIN): bin $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(BIN)

%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(BIN)
	./tests/run_tests.sh

clean:
	rm -f $(OBJ) $(BIN)
	rmdir bin 2>/dev/null || true

.PHONY: all test clean
