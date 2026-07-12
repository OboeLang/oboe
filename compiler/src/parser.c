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
#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    Token *toks;
    int count;
    int pos;
    const char *filename;
} Parser;

static Token *peek(Parser *p) { return &p->toks[p->pos]; }
static Token *peekAt(Parser *p, int off) {
    int i = p->pos + off;
    if (i >= p->count) i = p->count - 1;
    return &p->toks[i];
}
static Token *advance(Parser *p) { Token *t = &p->toks[p->pos]; if (p->pos < p->count - 1) p->pos++; return t; }
static bool check(Parser *p, TokenType t) { return peek(p)->type == t; }
static bool match(Parser *p, TokenType t) { if (check(p, t)) { advance(p); return true; } return false; }

static void fail(Parser *p, const char *msg) {
    fprintf(stderr, "%s:%d: parse error: %s (got '%s')\n", p->filename, peek(p)->line, msg,
            peek(p)->text ? peek(p)->text : "");
    exit(1);
}

static Token *expect(Parser *p, TokenType t, const char *what) {
    if (!check(p, t)) fail(p, what);
    return advance(p);
}

static Expr *new_expr(ExprKind k, int line) {
    Expr *e = calloc(1, sizeof(Expr));
    e->kind = k;
    e->line = line;
    return e;
}

static Stmt *new_stmt(StmtKind k, int line) {
    Stmt *s = calloc(1, sizeof(Stmt));
    s->kind = k;
    s->line = line;
    return s;
}

/* ---------- forward decls ---------- */
static Expr *parse_expression(Parser *p);
static Stmt *parse_statement(Parser *p);
static void parse_block(Parser *p, Stmt ***out_body, int *out_count);

/* ---------- string interpolation ---------- */
static StringPart *parse_string_literal_parts(Parser *p, const char *raw, int line) {
    StringPart head = {0};
    StringPart *tail = &head;
    size_t len = strlen(raw);
    size_t i = 0;
    size_t litstart = 0;
    while (i < len) {
        if (raw[i] == '$' && i + 1 < len && raw[i + 1] == '{') {
            if (i > litstart) {
                StringPart *sp = calloc(1, sizeof(StringPart));
                sp->is_expr = false;
                sp->literal = strndup(raw + litstart, i - litstart);
                tail->next = sp; tail = sp;
            }
            size_t start = i + 2;
            int depth = 1;
            size_t j = start;
            while (j < len && depth > 0) {
                if (raw[j] == '{') depth++;
                else if (raw[j] == '}') depth--;
                if (depth > 0) j++;
            }
            char *sub = strndup(raw + start, j - start);
            int subcount;
            Token *subtoks = lex_all(sub, &subcount);
            Parser sp_parser = { subtoks, subcount, 0, p->filename };
            Expr *e = parse_expression(&sp_parser);
            free(sub);
            StringPart *sp = calloc(1, sizeof(StringPart));
            sp->is_expr = true;
            sp->expr = e;
            tail->next = sp; tail = sp;
            i = j + 1;
            litstart = i;
        } else {
            i++;
        }
    }
    if (i > litstart) {
        StringPart *sp = calloc(1, sizeof(StringPart));
        sp->is_expr = false;
        sp->literal = strndup(raw + litstart, i - litstart);
        tail->next = sp; tail = sp;
    }
    (void)line;
    return head.next;
}

/* ---------- expressions ---------- */
static Expr **parse_arg_list(Parser *p, TokenType close, int *out_count) {
    Expr **items = NULL;
    int cap = 0, count = 0;
    if (!check(p, close)) {
        do {
            Expr *e = parse_expression(p);
            if (count == cap) { cap = cap ? cap * 2 : 4; items = realloc(items, cap * sizeof(Expr *)); }
            items[count++] = e;
        } while (match(p, T_COMMA));
    }
    expect(p, close, "expected closing delimiter");
    *out_count = count;
    return items;
}

static Expr *parse_primary(Parser *p) {
    Token *t = peek(p);
    int line = t->line;
    if (match(p, T_INT)) {
        Expr *e = new_expr(EXPR_INT, line);
        e->as.int_val = t->ival;
        return e;
    }
    if (match(p, T_TRUE)) { Expr *e = new_expr(EXPR_BOOL, line); e->as.bool_val = true; return e; }
    if (match(p, T_FALSE)) { Expr *e = new_expr(EXPR_BOOL, line); e->as.bool_val = false; return e; }
    if (match(p, T_NULL)) { return new_expr(EXPR_NULL, line); }
    if (check(p, T_STRING)) {
        Token *tok = advance(p);
        Expr *e = new_expr(EXPR_STRING, line);
        e->as.str_parts = parse_string_literal_parts(p, tok->text, line);
        return e;
    }
    if (check(p, T_IDENT)) {
        Token *tok = advance(p);
        Expr *e = new_expr(EXPR_IDENT, line);
        e->as.ident = strdup(tok->text);
        return e;
    }
    if (match(p, T_LPAREN)) {
        Expr *e = parse_expression(p);
        expect(p, T_RPAREN, "expected ')'");
        return e;
    }
    if (match(p, T_LBRACKET)) {
        int count;
        Expr **items = parse_arg_list(p, T_RBRACKET, &count);
        Expr *e = new_expr(EXPR_ARRAY, line);
        e->as.array_lit.items = items;
        e->as.array_lit.count = count;
        return e;
    }
    if (match(p, T_LBRACE)) {
        Expr **keys = NULL, **values = NULL;
        int cap = 0, count = 0;
        if (!check(p, T_RBRACE)) {
            do {
                Expr *k = parse_expression(p);
                expect(p, T_COLON, "expected ':' in dict literal");
                Expr *v = parse_expression(p);
                if (count == cap) {
                    cap = cap ? cap * 2 : 4;
                    keys = realloc(keys, cap * sizeof(Expr *));
                    values = realloc(values, cap * sizeof(Expr *));
                }
                keys[count] = k; values[count] = v; count++;
            } while (match(p, T_COMMA));
        }
        expect(p, T_RBRACE, "expected '}'");
        Expr *e = new_expr(EXPR_DICT, line);
        e->as.dict_lit.keys = keys;
        e->as.dict_lit.values = values;
        e->as.dict_lit.count = count;
        return e;
    }
    fail(p, "expected expression");
    return NULL;
}

static Expr *parse_postfix(Parser *p) {
    Expr *e = parse_primary(p);
    for (;;) {
        int line = peek(p)->line;
        if (match(p, T_DOT)) {
            Token *name = expect(p, T_IDENT, "expected member name after '.'");
            Expr *f = new_expr(EXPR_FIELD, line);
            f->as.field.obj = e;
            f->as.field.name = strdup(name->text);
            e = f;
        } else if (match(p, T_QDOT)) {
            Token *name = expect(p, T_IDENT, "expected member name after '?.'");
            Expr *f = new_expr(EXPR_SAFE_FIELD, line);
            f->as.field.obj = e;
            f->as.field.name = strdup(name->text);
            e = f;
        } else if (match(p, T_LPAREN)) {
            int count;
            Expr **args = parse_arg_list(p, T_RPAREN, &count);
            Expr *c = new_expr(EXPR_CALL, line);
            c->as.call.callee = e;
            c->as.call.args = args;
            c->as.call.arg_count = count;
            e = c;
        } else if (match(p, T_LBRACKET)) {
            Expr *idx = parse_expression(p);
            expect(p, T_RBRACKET, "expected ']'");
            Expr *ix = new_expr(EXPR_INDEX, line);
            ix->as.index.arr = e;
            ix->as.index.idx = idx;
            e = ix;
        } else {
            break;
        }
    }
    return e;
}

static Expr *parse_unary(Parser *p) {
    int line = peek(p)->line;
    if (match(p, T_NOT)) {
        Expr *e = new_expr(EXPR_UNARY, line);
        e->as.unary.op = strdup("!");
        e->as.unary.operand = parse_unary(p);
        return e;
    }
    if (match(p, T_MINUS)) {
        Expr *e = new_expr(EXPR_UNARY, line);
        e->as.unary.op = strdup("-");
        e->as.unary.operand = parse_unary(p);
        return e;
    }
    return parse_postfix(p);
}

/* multiplicative, and the `x` repetition operator, share precedence */
static Expr *parse_term(Parser *p) {
    Expr *e = parse_unary(p);
    for (;;) {
        int line = peek(p)->line;
        const char *op = NULL;
        if (check(p, T_STAR)) op = "*";
        else if (check(p, T_SLASH)) op = "/";
        else if (check(p, T_PERCENT)) op = "%";
        else if (check(p, T_IDENT) && strcmp(peek(p)->text, "x") == 0) op = "x";
        if (!op) break;
        advance(p);
        Expr *rhs = parse_unary(p);
        Expr *b = new_expr(EXPR_BINARY, line);
        b->as.binary.op = strdup(op);
        b->as.binary.l = e;
        b->as.binary.r = rhs;
        e = b;
    }
    return e;
}

static Expr *parse_additive(Parser *p) {
    Expr *e = parse_term(p);
    for (;;) {
        int line = peek(p)->line;
        const char *op = NULL;
        if (check(p, T_PLUS)) op = "+";
        else if (check(p, T_MINUS)) op = "-";
        if (!op) break;
        advance(p);
        Expr *rhs = parse_term(p);
        Expr *b = new_expr(EXPR_BINARY, line);
        b->as.binary.op = strdup(op);
        b->as.binary.l = e;
        b->as.binary.r = rhs;
        e = b;
    }
    return e;
}

static Expr *parse_is(Parser *p) {
    Expr *e = parse_additive(p);
    if (match(p, T_IS)) {
        int line = peek(p)->line;
        Token *name = expect(p, T_IDENT, "expected type name after 'is'");
        Expr *ie = new_expr(EXPR_IS, line);
        ie->as.is_check.value = e;
        ie->as.is_check.type_name = strdup(name->text);
        e = ie;
    }
    return e;
}

static Expr *parse_comparison(Parser *p) {
    Expr *e = parse_is(p);
    for (;;) {
        int line = peek(p)->line;
        const char *op = NULL;
        if (check(p, T_LT)) op = "<";
        else if (check(p, T_LTE)) op = "<=";
        else if (check(p, T_GT)) op = ">";
        else if (check(p, T_GTE)) op = ">=";
        if (!op) break;
        advance(p);
        Expr *rhs = parse_is(p);
        Expr *b = new_expr(EXPR_BINARY, line);
        b->as.binary.op = strdup(op);
        b->as.binary.l = e;
        b->as.binary.r = rhs;
        e = b;
    }
    return e;
}

static Expr *parse_equality(Parser *p) {
    Expr *e = parse_comparison(p);
    for (;;) {
        int line = peek(p)->line;
        const char *op = NULL;
        if (check(p, T_EQ)) op = "==";
        else if (check(p, T_NEQ)) op = "!=";
        if (!op) break;
        advance(p);
        Expr *rhs = parse_comparison(p);
        Expr *b = new_expr(EXPR_BINARY, line);
        b->as.binary.op = strdup(op);
        b->as.binary.l = e;
        b->as.binary.r = rhs;
        e = b;
    }
    return e;
}

/* user-declared operators (`operator ||> (...)`) bind looser than equality
   and tighter than `and`, left-associative */
static Expr *parse_custom_op(Parser *p) {
    Expr *e = parse_equality(p);
    while (check(p, T_CUSTOMOP)) {
        int line = peek(p)->line;
        Token *op = advance(p);
        Expr *rhs = parse_equality(p);
        Expr *b = new_expr(EXPR_BINARY, line);
        b->as.binary.op = strdup(op->text);
        b->as.binary.l = e;
        b->as.binary.r = rhs;
        e = b;
    }
    return e;
}

static Expr *parse_and(Parser *p) {
    Expr *e = parse_custom_op(p);
    while (check(p, T_ANDAND) || check(p, T_AND)) {
        int line = peek(p)->line;
        advance(p);
        Expr *rhs = parse_custom_op(p);
        Expr *b = new_expr(EXPR_BINARY, line);
        b->as.binary.op = strdup("&&");
        b->as.binary.l = e;
        b->as.binary.r = rhs;
        e = b;
    }
    return e;
}

static Expr *parse_or(Parser *p) {
    Expr *e = parse_and(p);
    while (check(p, T_OROR) || check(p, T_OR)) {
        int line = peek(p)->line;
        advance(p);
        Expr *rhs = parse_and(p);
        Expr *b = new_expr(EXPR_BINARY, line);
        b->as.binary.op = strdup("||");
        b->as.binary.l = e;
        b->as.binary.r = rhs;
        e = b;
    }
    return e;
}

static Expr *parse_nullish(Parser *p) {
    Expr *e = parse_or(p);
    while (check(p, T_QQ)) {
        int line = peek(p)->line;
        advance(p);
        Expr *rhs = parse_or(p);
        Expr *b = new_expr(EXPR_BINARY, line);
        b->as.binary.op = strdup("??");
        b->as.binary.l = e;
        b->as.binary.r = rhs;
        e = b;
    }
    return e;
}

static Expr *parse_assignment(Parser *p) {
    Expr *e = parse_nullish(p);
    if (match(p, T_ASSIGN)) {
        int line = peek(p)->line;
        Expr *value = parse_assignment(p);
        Expr *a = new_expr(EXPR_ASSIGN, line);
        a->as.assign.target = e;
        a->as.assign.value = value;
        return a;
    }
    return e;
}

static Expr *parse_expression(Parser *p) { return parse_assignment(p); }

/* ---------- types / params ---------- */

/* Consumes an optional type name followed by an identifier: `[type] name`.
   Used for let/const/params/fields. Because both type and name are bare
   identifiers, a type is present only when two identifiers appear back to back. */
static void parse_typed_name(Parser *p, char **out_type, char **out_name) {
    Token *first = expect(p, T_IDENT, "expected name");
    if (check(p, T_IDENT)) {
        Token *second = advance(p);
        *out_type = strdup(first->text);
        *out_name = strdup(second->text);
    } else {
        *out_type = NULL;
        *out_name = strdup(first->text);
    }
}

static Param *parse_params(Parser *p) {
    expect(p, T_LPAREN, "expected '('");
    Param head = {0};
    Param *tail = &head;
    if (!check(p, T_RPAREN)) {
        do {
            char *type_name, *name;
            parse_typed_name(p, &type_name, &name);
            Param *param = calloc(1, sizeof(Param));
            param->type_name = type_name;
            param->name = name;
            tail->next = param;
            tail = param;
        } while (match(p, T_COMMA));
    }
    expect(p, T_RPAREN, "expected ')'");
    return head.next;
}

/* ---------- statements ---------- */
static Stmt *parse_let(Parser *p, bool is_const) {
    int line = peek(p)->line;
    match(p, T_LET); /* `const var x` / `const let x` — the keyword is optional filler */
    char *type_name, *name;
    parse_typed_name(p, &type_name, &name);
    Expr *init = NULL;
    if (match(p, T_ASSIGN)) init = parse_expression(p);
    match(p, T_SEMI);
    Stmt *s = new_stmt(STMT_LET, line);
    s->as.let.name = name;
    s->as.let.type_name = type_name;
    s->as.let.is_const = is_const;
    s->as.let.init = init;
    return s;
}

static Stmt *parse_if(Parser *p) {
    int line = peek(p)->line;
    expect(p, T_LPAREN, "expected '(' after if");
    Expr *cond = parse_expression(p);
    expect(p, T_RPAREN, "expected ')'");
    Stmt **then_body; int then_count;
    parse_block(p, &then_body, &then_count);
    Stmt **else_body = NULL; int else_count = 0;
    if (match(p, T_ELSE)) {
        if (check(p, T_IF)) {
            advance(p);
            Stmt *nested = parse_if(p);
            else_body = malloc(sizeof(Stmt *));
            else_body[0] = nested;
            else_count = 1;
        } else {
            parse_block(p, &else_body, &else_count);
        }
    }
    Stmt *s = new_stmt(STMT_IF, line);
    s->as.if_stmt.cond = cond;
    s->as.if_stmt.then_body = then_body;
    s->as.if_stmt.then_count = then_count;
    s->as.if_stmt.else_body = else_body;
    s->as.if_stmt.else_count = else_count;
    return s;
}

static Stmt *parse_while(Parser *p) {
    int line = peek(p)->line;
    expect(p, T_LPAREN, "expected '(' after while");
    Expr *cond = parse_expression(p);
    expect(p, T_RPAREN, "expected ')'");
    Stmt **body; int count;
    parse_block(p, &body, &count);
    Stmt *s = new_stmt(STMT_WHILE, line);
    s->as.while_stmt.cond = cond;
    s->as.while_stmt.body = body;
    s->as.while_stmt.body_count = count;
    return s;
}

static bool looks_like_range_call(Parser *p) {
    return check(p, T_IDENT) && strcmp(peek(p)->text, "range") == 0 && peekAt(p, 1)->type == T_LPAREN;
}

static Stmt *parse_for(Parser *p) {
    int line = peek(p)->line;
    expect(p, T_LPAREN, "expected '(' after for");
    Token *var = expect(p, T_IDENT, "expected loop variable");
    expect(p, T_IN, "expected 'in' in for loop");
    Stmt *s = new_stmt(STMT_FOR, line);
    s->as.for_stmt.var_name = strdup(var->text);
    if (looks_like_range_call(p)) {
        advance(p); /* 'range' ident */
        expect(p, T_LPAREN, "expected '(' after range");
        Expr *a = parse_expression(p);
        expect(p, T_COMMA, "expected ',' in range()");
        Expr *b = parse_expression(p);
        expect(p, T_RPAREN, "expected ')'");
        s->as.for_stmt.is_range = true;
        s->as.for_stmt.range_a = a;
        s->as.for_stmt.range_b = b;
    } else {
        s->as.for_stmt.is_range = false;
        s->as.for_stmt.iterable = parse_expression(p);
    }
    expect(p, T_RPAREN, "expected ')'");
    Stmt **body; int count;
    parse_block(p, &body, &count);
    s->as.for_stmt.body = body;
    s->as.for_stmt.body_count = count;
    return s;
}

static Stmt *parse_switch(Parser *p) {
    int line = peek(p)->line;
    Expr *subject = parse_expression(p);
    expect(p, T_LBRACE, "expected '{' after switch subject");
    CaseClause head = {0};
    CaseClause *tail = &head;
    while (!check(p, T_RBRACE) && !check(p, T_EOF)) {
        expect(p, T_CASE, "expected 'case'");
        Expr *val = parse_expression(p);
        Stmt **body; int count;
        parse_block(p, &body, &count);
        CaseClause *cc = calloc(1, sizeof(CaseClause));
        cc->value = val;
        cc->body = body;
        cc->body_count = count;
        tail->next = cc; tail = cc;
    }
    expect(p, T_RBRACE, "expected '}' to close switch");
    Stmt *s = new_stmt(STMT_SWITCH, line);
    s->as.switch_stmt.subject = subject;
    s->as.switch_stmt.cases = head.next;
    return s;
}

static Stmt *parse_try(Parser *p) {
    int line = peek(p)->line;
    Stmt **body; int count;
    parse_block(p, &body, &count);
    CatchClause head = {0};
    CatchClause *tail = &head;
    while (match(p, T_CATCH)) {
        expect(p, T_LPAREN, "expected '(' after catch");
        Token *type_name = expect(p, T_IDENT, "expected exception type");
        char full_type[256];
        strcpy(full_type, type_name->text);
        while (match(p, T_DOT)) {
            Token *more = expect(p, T_IDENT, "expected identifier after '.'");
            strcat(full_type, ".");
            strcat(full_type, more->text);
        }
        Token *var_name = expect(p, T_IDENT, "expected exception variable name");
        expect(p, T_RPAREN, "expected ')'");
        Stmt **cbody; int ccount;
        parse_block(p, &cbody, &ccount);
        CatchClause *cc = calloc(1, sizeof(CatchClause));
        cc->type_name = strdup(full_type);
        cc->var_name = strdup(var_name->text);
        cc->body = cbody;
        cc->body_count = ccount;
        tail->next = cc; tail = cc;
    }
    Stmt **finally_body = NULL; int finally_count = 0;
    if (match(p, T_FINALLY)) parse_block(p, &finally_body, &finally_count);
    Stmt *s = new_stmt(STMT_TRY, line);
    s->as.try_stmt.body = body;
    s->as.try_stmt.body_count = count;
    s->as.try_stmt.catches = head.next;
    s->as.try_stmt.finally_body = finally_body;
    s->as.try_stmt.finally_count = finally_count;
    return s;
}

static Stmt *parse_throw(Parser *p) {
    int line = peek(p)->line;
    Token *type_name = expect(p, T_IDENT, "expected exception type after throw");
    Expr *value = NULL;
    if (match(p, T_LPAREN)) {
        if (!check(p, T_RPAREN)) value = parse_expression(p);
        expect(p, T_RPAREN, "expected ')'");
    }
    match(p, T_SEMI);
    Stmt *s = new_stmt(STMT_THROW, line);
    s->as.throw_stmt.type_name = strdup(type_name->text);
    s->as.throw_stmt.value = value;
    return s;
}

static Stmt *parse_statement(Parser *p) {
    int line = peek(p)->line;
    /* `var this.x = v` inside a method is field assignment, not a declaration */
    if (check(p, T_LET) && peekAt(p, 1)->type == T_IDENT && peekAt(p, 2)->type == T_DOT) {
        advance(p);
        Expr *e = parse_expression(p);
        match(p, T_SEMI);
        Stmt *s = new_stmt(STMT_EXPR, line);
        s->as.expr_stmt.expr = e;
        return s;
    }
    if (match(p, T_LET)) return parse_let(p, false);
    if (match(p, T_CONST)) return parse_let(p, true);
    /* explicitly typed declaration without keyword: `int x = 1` */
    if (check(p, T_IDENT) && peekAt(p, 1)->type == T_IDENT && peekAt(p, 2)->type == T_ASSIGN)
        return parse_let(p, false);
    if (match(p, T_RETURN)) {
        Expr *value = NULL;
        if (!check(p, T_RBRACE) && !check(p, T_SEMI)) value = parse_expression(p);
        match(p, T_SEMI);
        Stmt *s = new_stmt(STMT_RETURN, line);
        s->as.ret.value = value;
        return s;
    }
    if (check(p, T_IF)) { advance(p); return parse_if(p); }
    if (check(p, T_WHILE)) { advance(p); return parse_while(p); }
    if (check(p, T_FOR)) { advance(p); return parse_for(p); }
    if (check(p, T_SWITCH)) { advance(p); return parse_switch(p); }
    if (check(p, T_TRY)) { advance(p); return parse_try(p); }
    if (check(p, T_THROW)) { advance(p); return parse_throw(p); }
    if (check(p, T_LBRACE)) {
        Stmt **body; int count;
        parse_block(p, &body, &count);
        Stmt *s = new_stmt(STMT_BLOCK, line);
        s->as.block.body = body;
        s->as.block.body_count = count;
        return s;
    }
    Expr *e = parse_expression(p);
    match(p, T_SEMI);
    Stmt *s = new_stmt(STMT_EXPR, line);
    s->as.expr_stmt.expr = e;
    return s;
}

static void parse_block(Parser *p, Stmt ***out_body, int *out_count) {
    expect(p, T_LBRACE, "expected '{'");
    Stmt **items = NULL;
    int cap = 0, count = 0;
    while (!check(p, T_RBRACE) && !check(p, T_EOF)) {
        Stmt *s = parse_statement(p);
        if (count == cap) { cap = cap ? cap * 2 : 8; items = realloc(items, cap * sizeof(Stmt *)); }
        items[count++] = s;
    }
    expect(p, T_RBRACE, "expected '}'");
    *out_body = items;
    *out_count = count;
}

/* ---------- top-level declarations ---------- */
static FuncDecl *parse_func(Parser *p, bool is_static, bool is_private) {
    int line = peek(p)->line;
    char *return_type = NULL;
    /* `[ReturnType] func name(params) { body }` — return type precedes `func`. */
    expect(p, T_FUNC, "expected 'func'");
    Token *name = expect(p, T_IDENT, "expected function name");
    Param *params = parse_params(p);
    Stmt **body; int count;
    parse_block(p, &body, &count);
    FuncDecl *f = calloc(1, sizeof(FuncDecl));
    f->name = strdup(name->text);
    f->return_type = return_type;
    f->params = params;
    f->is_static = is_static;
    f->is_private = is_private;
    f->body = body;
    f->body_count = count;
    f->line = line;
    return f;
}

/* `operator <sym> (params) { body }` — the T_OPERATOR keyword has already been
   consumed. The symbol is either a T_CUSTOMOP token or any builtin operator
   token (for class overloads like `operator + (this, Vector2 other)`). */
static FuncDecl *parse_operator_decl(Parser *p) {
    int line = peek(p)->line;
    Token *sym = advance(p);
    if (!sym->text || !*sym->text || sym->type == T_LPAREN)
        fail(p, "expected an operator symbol after 'operator'");
    FuncDecl *f = calloc(1, sizeof(FuncDecl));
    f->name = strdup("operator");
    f->op_symbol = strdup(sym->text);
    f->params = parse_params(p);
    parse_block(p, &f->body, &f->body_count);
    f->line = line;
    return f;
}

static ClassDecl *parse_class(Parser *p) {
    int line = peek(p)->line;
    Token *name = expect(p, T_IDENT, "expected class name");
    char *parent_name = NULL;
    if (match(p, T_EXTENDS)) {
        Token *pn = expect(p, T_IDENT, "expected parent class name");
        parent_name = strdup(pn->text);
    }
    expect(p, T_LBRACE, "expected '{' to start class body");
    FieldDecl head = {0};
    FieldDecl *ftail = &head;
    FuncDecl **methods = NULL;
    int mcap = 0, mcount = 0;
    while (!check(p, T_RBRACE) && !check(p, T_EOF)) {
        bool is_static = false, is_private = false;
        while (check(p, T_STATIC) || check(p, T_PRIVATE)) {
            if (match(p, T_STATIC)) is_static = true;
            else if (match(p, T_PRIVATE)) is_private = true;
        }
        /* return type may precede `func` for methods */
        if (check(p, T_IDENT) && peekAt(p, 1)->type == T_FUNC) advance(p);
        if (check(p, T_FUNC)) {
            FuncDecl *m = parse_func(p, is_static, is_private);
            if (mcount == mcap) { mcap = mcap ? mcap * 2 : 8; methods = realloc(methods, mcap * sizeof(FuncDecl *)); }
            methods[mcount++] = m;
        } else if (check(p, T_OPERATOR)) {
            advance(p);
            FuncDecl *m = parse_operator_decl(p);
            if (mcount == mcap) { mcap = mcap ? mcap * 2 : 8; methods = realloc(methods, mcap * sizeof(FuncDecl *)); }
            methods[mcount++] = m;
        } else if (check(p, T_CONST) || check(p, T_LET) ||
                   (check(p, T_IDENT) && peekAt(p, 1)->type == T_IDENT)) {
            /* `const [var|type] name`, `var name`, or keywordless `type name` */
            bool is_const = match(p, T_CONST);
            match(p, T_LET);
            char *type_name, *fname;
            parse_typed_name(p, &type_name, &fname);
            Expr *init = NULL;
            if (match(p, T_ASSIGN)) init = parse_expression(p);
            match(p, T_SEMI);
            FieldDecl *fd = calloc(1, sizeof(FieldDecl));
            fd->type_name = type_name;
            fd->name = fname;
            fd->is_static = is_static;
            fd->is_private = is_private;
            fd->is_const = is_const;
            fd->init = init;
            ftail->next = fd; ftail = fd;
        } else {
            fail(p, "expected field or method declaration in class body");
        }
    }
    expect(p, T_RBRACE, "expected '}' to close class");
    ClassDecl *c = calloc(1, sizeof(ClassDecl));
    c->name = strdup(name->text);
    c->parent_name = parent_name;
    c->fields = head.next;
    c->methods = methods;
    c->method_count = mcount;
    c->line = line;
    return c;
}

static ImportDecl parse_import(Parser *p) {
    ImportDecl imp = {0};
    if (check(p, T_IDENT) && peekAt(p, 1)->type == T_FROM) {
        char **members = NULL;
        int cap = 0, count = 0;
        do {
            Token *m = expect(p, T_IDENT, "expected imported member name");
            if (count == cap) { cap = cap ? cap * 2 : 4; members = realloc(members, cap * sizeof(char *)); }
            members[count++] = strdup(m->text);
        } while (match(p, T_COMMA));
        expect(p, T_FROM, "expected 'from'");
        Token *mod = expect(p, T_IDENT, "expected module name");
        imp.module = strdup(mod->text);
        imp.members = members;
        imp.member_count = count;
    } else {
        Token *mod = expect(p, T_IDENT, "expected module name");
        imp.module = strdup(mod->text);
        if (match(p, T_AS)) {
            Token *alias = expect(p, T_IDENT, "expected alias after 'as'");
            imp.alias = strdup(alias->text);
        }
    }
    match(p, T_SEMI);
    return imp;
}

Program *parse_program(Token *tokens, int count, const char *filename) {
    Parser parser = { tokens, count, 0, filename };
    Parser *p = &parser;
    Decl head = {0};
    Decl *tail = &head;
    while (!check(p, T_EOF)) {
        Decl *d = calloc(1, sizeof(Decl));
        if (match(p, T_IMPORT)) {
            d->kind = DECL_IMPORT;
            d->as.import = parse_import(p);
        } else if (match(p, T_CIMPORT)) {
            /* FFI: `cimport symbol from "library.so"` */
            Token *name = expect(p, T_IDENT, "expected C symbol name after cimport");
            d->kind = DECL_CIMPORT;
            d->as.cimport.name = strdup(name->text);
            d->as.cimport.line = name->line;
            expect(p, T_FROM, "expected 'from' in cimport");
            Token *lib = expect(p, T_STRING, "expected a library path string in cimport");
            d->as.cimport.lib = strdup(lib->text);
            match(p, T_SEMI);
        } else if (match(p, T_OPERATOR)) {
            d->kind = DECL_OPERATOR;
            d->as.func = parse_operator_decl(p);
        } else if (match(p, T_EVENT)) {
            /* `event MyEvent = event(payload params)` */
            Token *name = expect(p, T_IDENT, "expected event name");
            d->kind = DECL_EVENT;
            d->as.event.name = strdup(name->text);
            d->as.event.line = name->line;
            expect(p, T_ASSIGN, "expected '=' in event declaration");
            expect(p, T_EVENT, "expected event(...) constructor");
            d->as.event.params = parse_params(p);
            match(p, T_SEMI);
        } else if (match(p, T_ON)) {
            /* `on MyEvent [as e] { body }` */
            Token *ev = expect(p, T_IDENT, "expected event name after 'on'");
            d->kind = DECL_ON;
            d->as.on.event_name = strdup(ev->text);
            d->as.on.line = ev->line;
            d->as.on.var_name = NULL;
            if (match(p, T_AS)) {
                Token *v = expect(p, T_IDENT, "expected variable name after 'as'");
                d->as.on.var_name = strdup(v->text);
            }
            parse_block(p, &d->as.on.body, &d->as.on.body_count);
        } else if (check(p, T_CLASS)) {
            advance(p);
            d->kind = DECL_CLASS;
            d->as.klass = parse_class(p);
        } else if (check(p, T_FUNC)) {
            d->kind = DECL_FUNC;
            d->as.func = parse_func(p, false, false);
        } else if (check(p, T_IDENT) && peekAt(p, 1)->type == T_FUNC) {
            Token *ret = advance(p);
            d->kind = DECL_FUNC;
            d->as.func = parse_func(p, false, false);
            d->as.func->return_type = strdup(ret->text);
        } else {
            d->kind = DECL_STMT;
            d->as.stmt = parse_statement(p);
        }
        tail->next = d;
        tail = d;
    }
    Program *prog = calloc(1, sizeof(Program));
    prog->decls = head.next;
    return prog;
}
