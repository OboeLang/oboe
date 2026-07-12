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
#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct { const char *word; TokenType type; } Keyword;

static Keyword keywords[] = {
    {"let", T_LET}, {"var", T_LET}, /* `var` is the spec spelling; `let` is a legacy alias */
    {"const", T_CONST}, {"func", T_FUNC}, {"return", T_RETURN},
    {"class", T_CLASS}, {"static", T_STATIC}, {"private", T_PRIVATE},
    {"if", T_IF}, {"else", T_ELSE}, {"while", T_WHILE}, {"for", T_FOR}, {"in", T_IN},
    {"switch", T_SWITCH}, {"case", T_CASE},
    {"try", T_TRY}, {"catch", T_CATCH}, {"finally", T_FINALLY}, {"throw", T_THROW},
    {"import", T_IMPORT}, {"as", T_AS}, {"from", T_FROM},
    {"true", T_TRUE}, {"false", T_FALSE}, {"null", T_NULL},
    {"and", T_AND}, {"or", T_OR}, {"is", T_IS}, {"extends", T_EXTENDS},
    {"event", T_EVENT}, {"on", T_ON}, {"operator", T_OPERATOR}, {"cimport", T_CIMPORT},
    {NULL, T_EOF}
};

/* ---- custom operator symbols ----
   `operator ||> (...)` declarations introduce new operator tokens. Symbols are
   collected in a text pre-scan of each file before tokenizing (the registry is
   global, so a symbol declared in any already-lexed file keeps lexing in later
   files). Builtin operator spellings are excluded so `operator + (...)` never
   changes how `+` itself lexes. */
#define MAX_CUSTOM_OPS 64
static char *custom_ops[MAX_CUSTOM_OPS];
static int custom_op_count = 0;

static bool is_op_char(char c) { return strchr("+-*/%<>=!&|^~?@#$:.", c) != NULL; }

static bool is_builtin_op(const char *s) {
    static const char *builtins[] = {
        "+", "-", "*", "/", "%", "=", "==", "!=", "<", "<=", ">", ">=",
        "&&", "||", "??", "?.", "!", "?", ".", "->", ":", NULL
    };
    for (int i = 0; builtins[i]; i++) if (strcmp(builtins[i], s) == 0) return true;
    return false;
}

static void register_custom_op(const char *sym) {
    if (is_builtin_op(sym)) return;
    for (int i = 0; i < custom_op_count; i++) if (strcmp(custom_ops[i], sym) == 0) return;
    if (custom_op_count >= MAX_CUSTOM_OPS) {
        fprintf(stderr, "oboe: too many custom operators\n");
        exit(1);
    }
    custom_ops[custom_op_count++] = strdup(sym);
}

static void prescan_custom_ops(const char *src, size_t len) {
    for (size_t i = 0; i + 8 < len; i++) {
        if (strncmp(src + i, "operator", 8) != 0) continue;
        if (i > 0 && (isalnum((unsigned char)src[i-1]) || src[i-1] == '_')) continue;
        size_t p = i + 8;
        if (p < len && (isalnum((unsigned char)src[p]) || src[p] == '_')) continue;
        while (p < len && isspace((unsigned char)src[p])) p++;
        size_t start = p;
        while (p < len && is_op_char(src[p])) p++;
        if (p > start) {
            char *sym = strndup(src + start, p - start);
            register_custom_op(sym);
            free(sym);
        }
    }
}

void lex_prescan_ops(const char *src) {
    prescan_custom_ops(src, strlen(src));
}

/* longest registered custom operator matching at src[pos], or NULL */
static const char *match_custom_op(const char *src, size_t pos, size_t len) {
    const char *best = NULL;
    size_t best_len = 0;
    for (int i = 0; i < custom_op_count; i++) {
        size_t n = strlen(custom_ops[i]);
        if (n > best_len && pos + n <= len && strncmp(src + pos, custom_ops[i], n) == 0) {
            best = custom_ops[i];
            best_len = n;
        }
    }
    return best;
}

static Token make(TokenType type, const char *text, int line) {
    Token t;
    t.type = type;
    t.text = text ? strdup(text) : NULL;
    t.ival = 0;
    t.line = line;
    return t;
}

typedef struct { Token *items; int count, cap; } TokBuf;
static void push_tok(TokBuf *b, Token t) {
    if (b->count == b->cap) { b->cap = b->cap ? b->cap * 2 : 64; b->items = realloc(b->items, b->cap * sizeof(Token)); }
    b->items[b->count++] = t;
}

/* Reads a double-quoted string, honoring `${...}` (kept verbatim in output including the ${ and })
   and basic escapes \n \t \" \\ . The token text is the decoded content excluding the surrounding quotes. */
static char *read_string_body(const char *src, size_t *pos, size_t len, int *line) {
    size_t cap = 64, n = 0;
    char *out = malloc(cap);
    while (*pos < len && src[*pos] != '"') {
        char c = src[*pos];
        if (c == '\\' && *pos + 1 < len) {
            char e = src[*pos + 1];
            char decoded;
            switch (e) {
                case 'n': decoded = '\n'; break;
                case 't': decoded = '\t'; break;
                case '"': decoded = '"'; break;
                case '\\': decoded = '\\'; break;
                default: decoded = e; break;
            }
            if (n + 1 >= cap) { cap *= 2; out = realloc(out, cap); }
            out[n++] = decoded;
            *pos += 2;
            continue;
        }
        if (c == '$' && *pos + 1 < len && src[*pos + 1] == '{') {
            /* copy through the matching close brace verbatim */
            size_t start = *pos;
            size_t p = *pos + 2;
            int depth = 1;
            while (p < len && depth > 0) {
                if (src[p] == '{') depth++;
                else if (src[p] == '}') depth--;
                if (depth > 0) p++;
            }
            size_t chunk_len = p - start + 1; /* include closing brace */
            if (n + chunk_len >= cap) { cap = (n + chunk_len) * 2; out = realloc(out, cap); }
            memcpy(out + n, src + start, chunk_len);
            n += chunk_len;
            *pos = p + 1;
            continue;
        }
        if (c == '\n') (*line)++;
        if (n + 1 >= cap) { cap *= 2; out = realloc(out, cap); }
        out[n++] = c;
        (*pos)++;
    }
    if (n >= cap) { cap = n + 1; out = realloc(out, cap); }
    out[n] = '\0';
    return out;
}

Token *lex_all(const char *src, int *out_count) {
    TokBuf buf = {0};
    size_t pos = 0;
    size_t len = strlen(src);
    int line = 1;

    prescan_custom_ops(src, len);

    while (pos < len) {
        char c = src[pos];
        if (c == '\n') { line++; pos++; continue; }
        if (isspace((unsigned char)c)) { pos++; continue; }
        if (c == '/' && pos + 1 < len && src[pos + 1] == '/') {
            while (pos < len && src[pos] != '\n') pos++;
            continue;
        }
        if (is_op_char(c)) {
            const char *cop = match_custom_op(src, pos, len);
            if (cop) {
                push_tok(&buf, make(T_CUSTOMOP, cop, line));
                pos += strlen(cop);
                continue;
            }
        }
        if (c == '"') {
            pos++;
            char *body = read_string_body(src, &pos, len, &line);
            pos++; /* closing quote */
            Token t = make(T_STRING, body, line);
            free(body);
            push_tok(&buf, t);
            continue;
        }
        if (isdigit((unsigned char)c)) {
            size_t start = pos;
            while (pos < len && isdigit((unsigned char)src[pos])) pos++;
            char *numstr = strndup(src + start, pos - start);
            Token t = make(T_INT, numstr, line);
            t.ival = atoll(numstr);
            free(numstr);
            push_tok(&buf, t);
            continue;
        }
        if (isalpha((unsigned char)c) || c == '_') {
            size_t start = pos;
            while (pos < len && (isalnum((unsigned char)src[pos]) || src[pos] == '_')) pos++;
            char *ident = strndup(src + start, pos - start);
            TokenType kw = T_EOF;
            bool found = false;
            for (int i = 0; keywords[i].word; i++) {
                if (strcmp(keywords[i].word, ident) == 0) { kw = keywords[i].type; found = true; break; }
            }
            /* the bare identifier `x` used infix is the repetition operator; the parser
               disambiguates it from a variable named x by expression position. We tag it
               specially only when the lexer can't know context, so emit T_IDENT and let
               the parser treat an ident exactly spelled "x" in binary-operator position
               as T_X_OP. */
            Token t = make(found ? kw : T_IDENT, ident, line);
            free(ident);
            push_tok(&buf, t);
            continue;
        }
        switch (c) {
            case '{': push_tok(&buf, make(T_LBRACE, "{", line)); pos++; continue;
            case '}': push_tok(&buf, make(T_RBRACE, "}", line)); pos++; continue;
            case '(': push_tok(&buf, make(T_LPAREN, "(", line)); pos++; continue;
            case ')': push_tok(&buf, make(T_RPAREN, ")", line)); pos++; continue;
            case '[': push_tok(&buf, make(T_LBRACKET, "[", line)); pos++; continue;
            case ']': push_tok(&buf, make(T_RBRACKET, "]", line)); pos++; continue;
            case ',': push_tok(&buf, make(T_COMMA, ",", line)); pos++; continue;
            case ':': push_tok(&buf, make(T_COLON, ":", line)); pos++; continue;
            case ';': push_tok(&buf, make(T_SEMI, ";", line)); pos++; continue;
            case '+': push_tok(&buf, make(T_PLUS, "+", line)); pos++; continue;
            case '-': {
                if (pos + 1 < len && src[pos+1] == '>') { push_tok(&buf, make(T_ARROW, "->", line)); pos += 2; continue; }
                push_tok(&buf, make(T_MINUS, "-", line)); pos++; continue;
            }
            case '*': push_tok(&buf, make(T_STAR, "*", line)); pos++; continue;
            case '%': push_tok(&buf, make(T_PERCENT, "%", line)); pos++; continue;
            case '.': push_tok(&buf, make(T_DOT, ".", line)); pos++; continue;
            case '/': push_tok(&buf, make(T_SLASH, "/", line)); pos++; continue;
            case '=':
                if (pos + 1 < len && src[pos+1] == '=') { push_tok(&buf, make(T_EQ, "==", line)); pos += 2; continue; }
                push_tok(&buf, make(T_ASSIGN, "=", line)); pos++; continue;
            case '!':
                if (pos + 1 < len && src[pos+1] == '=') { push_tok(&buf, make(T_NEQ, "!=", line)); pos += 2; continue; }
                push_tok(&buf, make(T_NOT, "!", line)); pos++; continue;
            case '<':
                if (pos + 1 < len && src[pos+1] == '=') { push_tok(&buf, make(T_LTE, "<=", line)); pos += 2; continue; }
                push_tok(&buf, make(T_LT, "<", line)); pos++; continue;
            case '>':
                if (pos + 1 < len && src[pos+1] == '=') { push_tok(&buf, make(T_GTE, ">=", line)); pos += 2; continue; }
                push_tok(&buf, make(T_GT, ">", line)); pos++; continue;
            case '&':
                if (pos + 1 < len && src[pos+1] == '&') { push_tok(&buf, make(T_ANDAND, "&&", line)); pos += 2; continue; }
                fprintf(stderr, "oboe: unexpected '&' at line %d\n", line); exit(1);
            case '|':
                if (pos + 1 < len && src[pos+1] == '|') { push_tok(&buf, make(T_OROR, "||", line)); pos += 2; continue; }
                fprintf(stderr, "oboe: unexpected '|' at line %d\n", line); exit(1);
            case '?':
                if (pos + 1 < len && src[pos+1] == '?') { push_tok(&buf, make(T_QQ, "??", line)); pos += 2; continue; }
                if (pos + 1 < len && src[pos+1] == '.') { push_tok(&buf, make(T_QDOT, "?.", line)); pos += 2; continue; }
                push_tok(&buf, make(T_QUESTION, "?", line)); pos++; continue;
            default:
                fprintf(stderr, "oboe: unexpected character '%c' at line %d\n", c, line);
                exit(1);
        }
    }
    push_tok(&buf, make(T_EOF, "", line));
    *out_count = buf.count;
    return buf.items;
}
