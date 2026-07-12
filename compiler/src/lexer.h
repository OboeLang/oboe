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
#ifndef OBOE_LEXER_H
#define OBOE_LEXER_H

#include <stddef.h>

typedef enum {
    T_EOF, T_IDENT, T_INT, T_STRING,
    /* keywords */
    T_LET, T_CONST, T_FUNC, T_RETURN, T_CLASS, T_STATIC, T_PRIVATE,
    T_IF, T_ELSE, T_WHILE, T_FOR, T_IN, T_SWITCH, T_CASE,
    T_TRY, T_CATCH, T_FINALLY, T_THROW,
    T_IMPORT, T_AS, T_FROM,
    T_TRUE, T_FALSE, T_NULL,
    T_AND, T_OR, T_IS, T_X_OP, T_EXTENDS,
    T_EVENT, T_ON, T_OPERATOR, T_CIMPORT,
    /* punctuation / operators */
    T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN, T_LBRACKET, T_RBRACKET,
    T_COMMA, T_DOT, T_COLON, T_SEMI,
    T_PLUS, T_MINUS, T_STAR, T_SLASH, T_PERCENT,
    T_ASSIGN, T_EQ, T_NEQ, T_LT, T_LTE, T_GT, T_GTE,
    T_NOT, T_ANDAND, T_OROR,
    T_QQ, T_QDOT, T_QUESTION,
    T_ARROW,
    T_CUSTOMOP /* a user-declared operator symbol, e.g. `||>`; text holds the symbol */
} TokenType;

typedef struct {
    TokenType type;
    char *text;   /* owned, NUL-terminated lexeme (for idents/strings holds decoded value for T_STRING parts handled separately) */
    long long ival;
    int line;
} Token;

typedef struct {
    const char *src;
    size_t pos;
    size_t len;
    int line;
} Lexer;

void lexer_init(Lexer *lx, const char *src);
/* Tokenizes the whole source into a growable array, terminated by T_EOF. */
Token *lex_all(const char *src, int *out_count);
/* Registers any `operator <sym>` declarations found in src without tokenizing
   it, so custom operators declared in one file lex correctly in another.
   lex_all runs this on its own input automatically. */
void lex_prescan_ops(const char *src);

#endif
