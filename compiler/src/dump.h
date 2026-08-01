/* SPDX-License-Identifier: GPL-2.0-only
 * © 2026 Sushii64
 * © 2026 robinpie
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef OBOE_DUMP_H
#define OBOE_DUMP_H

#include "ast.h"
#include "lexer.h"
#include <stdio.h>

/* Debug serializations of the lexer's and parser's output. These exist to gate
   the Oboe-written compiler in selfhost/ against this one: every function here
   has a twin in selfhost/dump.oboe that must emit the identical bytes, which is
   what `oboe dump-tokens` / `oboe dump-ast` and the selfhost_* tests check.
   Keep the two in step. */

/* Escapes a lexeme so a value can never span a line of a dump. */
void dump_escape(const char *s, FILE *out);

/* One line per token: `<TYPE> <line> <escaped lexeme>`. */
void dump_tokens(Token *toks, int count, FILE *out);

/* The AST as an indented tree, two spaces per level, one node per line. See
   the comment above dump_ast in dump.c for the shape. */
void dump_ast(Program *prog, FILE *out);

/* The enum constants' own spellings, e.g. "EXPR_BINARY". */
const char *expr_kind_name(ExprKind k);
const char *stmt_kind_name(StmtKind k);
const char *decl_kind_name(DeclKind k);
const char *for_iter_kind_name(ForIterKind k);

#endif
