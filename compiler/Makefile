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

SRC = src/lexer.c src/parser.c src/codegen.c src/dump.c src/numfmt.c \
      src/projectjson.c src/sha256.c src/izim.c src/record.c src/kabuk.c \
      src/katare.c src/projectedit.c src/pkg.c src/main.c
OBJ = $(SRC:.c=.o)
# every object depends on every header: the set is small, and without this a
# change to ast.h or the runtime header silently leaves stale objects linked in
HDR = src/ast.h src/lexer.h src/parser.h src/codegen.h src/dump.h \
      src/numfmt.h src/projectjson.h src/sha256.h src/izim.h src/record.h \
      src/kabuk.h src/katare.h src/projectedit.h src/pkg.h \
      runtime/oboe_runtime.h

BIN = bin/oboe
# the compiler proper, written in Oboe. bin/oboe execs it; see selfhost/.
OBOEC = bin/oboec
SELFHOST = $(wildcard selfhost/*.oboe)

# How an Oboe program is built into a native binary, which is what `oboe build`
# shells out to as well. Not $(CFLAGS): this is the generated program's build,
# not the toolchain's, and it has to match compile_c_to_binary in src/main.c.
OBOECFLAGS = -std=c11 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -O2 -Iruntime
OBOELIBS = -ldl -lm

all: $(BIN) $(OBOEC)

bin:
	mkdir -p bin

$(BIN): bin $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(BIN)

%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

# A clean checkout builds oboec from the committed, generated bootstrap C, so
# nothing but a C compiler is needed to get started. Refreshing that file is
# `make regen-bootstrap`, deliberately separate: it is a reviewed change, not a
# build step, and it is what makes a bad commit of it recoverable.
$(OBOEC): bin bootstrap/oboec.c runtime/oboe_runtime.c runtime/oboe_runtime.h
	$(CC) $(OBOECFLAGS) bootstrap/oboec.c runtime/oboe_runtime.c \
	      $(OBOELIBS) -o $(OBOEC)

# os.script_file()/script_dir()/project_root() are resolved at compile time, so
# the emitted C carries the absolute path of whoever generated it. A committed
# artifact must not, or every regeneration diffs someone's home directory
# instead of the change being reviewed -- so those three constants are blanked.
# That is only safe because oboec never calls them; if it ever does, it will
# fail loudly rather than silently, since the paths would then be wrong.
regen-bootstrap: $(BIN) $(OBOEC)
	mkdir -p stage
	$(OBOEC) selfhost/main.oboe -o stage/boot.c
	sed -E 's@(ob_std_os_(script_file|script_dir|project_root)\(void\) \{ return ob_string\(")[^"]*@\1<generated>@' \
	    stage/boot.c > bootstrap/oboec.c
	@echo "bootstrap/oboec.c regenerated -- review the diff before committing"

# ---- bootstrap chain ----
# stage1 is the committed bootstrap C. stage2 is what it produces from today's
# selfhost/ sources, and stage3 is what a compiler built from stage2 produces
# from the same sources. stage2 == stage3 is the fixpoint that says the compiler
# reproduces itself; stage1 may legitimately differ, since it was generated
# before whatever change is being tested.
stage/stage2.c: $(OBOEC) $(SELFHOST)
	mkdir -p stage
	$(OBOEC) selfhost/main.oboe -o stage/stage2.c

stage/oboec2: stage/stage2.c runtime/oboe_runtime.c
	$(CC) $(OBOECFLAGS) stage/stage2.c runtime/oboe_runtime.c \
	      $(OBOELIBS) -o stage/oboec2

stage/stage3.c: stage/oboec2 $(SELFHOST)
	stage/oboec2 selfhost/main.oboe -o stage/stage3.c

bootstrap-check: stage/stage2.c stage/stage3.c
	@cmp stage/stage2.c stage/stage3.c \
	  && echo "PASS bootstrap-check (stage2 C == stage3 C)" \
	  || { echo "FAIL bootstrap-check: the compiler does not reproduce itself"; \
	       exit 1; }

test: $(BIN) $(OBOEC)
	./tests/run_tests.sh

clean:
	rm -f $(OBJ) $(BIN) $(OBOEC)
	rm -rf stage
	rmdir bin 2>/dev/null || true

.PHONY: all test clean regen-bootstrap bootstrap-check
