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
#include "dump.h"
#include "numfmt.h"
#include <string.h>

/* ---- enum name tables ----
   Each is tied to its enum by a _Static_assert on the last constant, so adding
   a kind without naming it here fails the build rather than dumping "???". */

static const char *const expr_names[] = {
	"EXPR_INT",	   "EXPR_FLOAT", "EXPR_BOOL",  "EXPR_NULL",
	"EXPR_STRING",	   "EXPR_IDENT", "EXPR_ARRAY", "EXPR_DICT",
	"EXPR_BINARY",	   "EXPR_UNARY", "EXPR_CALL",  "EXPR_FIELD",
	"EXPR_SAFE_FIELD", "EXPR_INDEX", "EXPR_IS",    "EXPR_ASSIGN",
	"EXPR_TERNARY"
};
_Static_assert(sizeof(expr_names) / sizeof(expr_names[0]) == EXPR_TERNARY + 1,
	       "expr_names must list every ExprKind, in enum order");

static const char *const stmt_names[] = {
	"STMT_LET",   "STMT_EXPR",  "STMT_RETURN", "STMT_IF",
	"STMT_WHILE", "STMT_FOR",   "STMT_SWITCH", "STMT_TRY",
	"STMT_THROW", "STMT_BLOCK", "STMT_BREAK",  "STMT_CONTINUE"
};
_Static_assert(sizeof(stmt_names) / sizeof(stmt_names[0]) == STMT_CONTINUE + 1,
	       "stmt_names must list every StmtKind, in enum order");

static const char *const decl_names[] = { "DECL_FUNC",	   "DECL_CLASS",
					  "DECL_IMPORT",   "DECL_STMT",
					  "DECL_OPERATOR", "DECL_EVENT",
					  "DECL_ON",	   "DECL_CIMPORT" };
_Static_assert(sizeof(decl_names) / sizeof(decl_names[0]) == DECL_CIMPORT + 1,
	       "decl_names must list every DeclKind, in enum order");

static const char *const for_iter_names[] = { "FOR_RANGE", "FOR_ITER",
					      "FOR_PAIRS", "FOR_IPAIRS" };
_Static_assert(sizeof(for_iter_names) / sizeof(for_iter_names[0]) ==
		       FOR_IPAIRS + 1,
	       "for_iter_names must list every ForIterKind, in enum order");

const char *expr_kind_name(ExprKind k)
{
	if (k < 0 || k > EXPR_TERNARY)
		return "EXPR_???";
	return expr_names[k];
}

const char *stmt_kind_name(StmtKind k)
{
	if (k < 0 || k > STMT_CONTINUE)
		return "STMT_???";
	return stmt_names[k];
}

const char *decl_kind_name(DeclKind k)
{
	if (k < 0 || k > DECL_CIMPORT)
		return "DECL_???";
	return decl_names[k];
}

const char *for_iter_kind_name(ForIterKind k)
{
	if (k < 0 || k > FOR_IPAIRS)
		return "FOR_???";
	return for_iter_names[k];
}

/* ---- escaping ---- */

void dump_escape(const char *s, FILE *out)
{
	for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
		switch (*p) {
		case '\\':
			fputs("\\\\", out);
			break;
		case '\n':
			fputs("\\n", out);
			break;
		case '\t':
			fputs("\\t", out);
			break;
		case '\r':
			fputs("\\r", out);
			break;
		case '"':
			fputs("\\\"", out);
			break;
		default:
			/* octal, not \xNN: C's hex escape is greedy and would
			   swallow a following hex digit */
			if (*p < 0x20 || *p == 0x7f)
				fprintf(out, "\\%03o", *p);
			else
				fputc(*p, out);
		}
	}
}

void dump_tokens(Token *toks, int count, FILE *out)
{
	for (int i = 0; i < count; i++) {
		fprintf(out, "%s %d ", token_type_name(toks[i].type),
			toks[i].line);
		dump_escape(toks[i].text ? toks[i].text : "", out);
		fputc('\n', out);
	}
}

/* ---- AST ----
   An indented tree, two spaces per level, one node per line:

       DECL_FUNC name="add" line=3 ret="int" static=0 private=0 op=-
         params
           param name="x" type="int"
         body
           STMT_RETURN line=4
             value
               EXPR_BINARY line=4 op="+"
                 EXPR_IDENT line=4 name="x"
                 EXPR_IDENT line=4 name="y"

   Statements label each of their child slots (`cond`, `then`, `else`, ...)
   because most have several; expressions leave theirs positional, since their
   operand order is fixed and obvious, apart from a call's callee/args. A slot
   holding a list always prints its label, even when the list is empty. An
   absent optional expression prints no line at all -- indentation makes that
   unambiguous, and it keeps the common case short.

   A string field prints as "..." with dump_escape applied; an absent one is a
   bare `-`, which no quoted value can collide with. */

static void ind(FILE *out, int depth)
{
	for (int i = 0; i < depth; i++)
		fputs("  ", out);
}

static void field_str(FILE *out, const char *key, const char *val)
{
	fprintf(out, " %s=", key);
	if (!val) {
		fputc('-', out);
		return;
	}
	fputc('"', out);
	dump_escape(val, out);
	fputc('"', out);
}

/* The dump spells a float the way str() does, which is how the twin in
   selfhost/dump.oboe reaches the same bytes with nothing but str(). */
static void field_double(FILE *out, const char *key, double d)
{
	char buf[64];
	ob_double_text(d, buf, sizeof buf);
	fprintf(out, " %s=%s", key, buf);
}

static void dump_expr(Expr *e, int depth, FILE *out);
static void dump_stmt(Stmt *s, int depth, FILE *out);

/* A labelled list of statements. The label always prints, so an empty else or
   finally is visible as itself rather than as nothing. */
static void dump_body(const char *label, Stmt **body, int count, int depth,
		      FILE *out)
{
	ind(out, depth);
	fprintf(out, "%s\n", label);
	for (int i = 0; i < count; i++)
		dump_stmt(body[i], depth + 1, out);
}

/* A labelled optional expression: nothing at all when it is absent. */
static void dump_opt(const char *label, Expr *e, int depth, FILE *out)
{
	if (!e)
		return;
	ind(out, depth);
	fprintf(out, "%s\n", label);
	dump_expr(e, depth + 1, out);
}

static void dump_params(Param *params, int depth, FILE *out)
{
	ind(out, depth);
	fputs("params\n", out);
	for (Param *p = params; p; p = p->next) {
		ind(out, depth + 1);
		fputs("param", out);
		field_str(out, "name", p->name);
		field_str(out, "type", p->type_name);
		fputc('\n', out);
		dump_opt("default", p->default_value, depth + 2, out);
	}
}

static void dump_expr(Expr *e, int depth, FILE *out)
{
	ind(out, depth);
	fprintf(out, "%s line=%d", expr_kind_name(e->kind), e->line);
	switch (e->kind) {
	case EXPR_INT:
		fprintf(out, " value=%lld", e->as.int_val);
		fputc('\n', out);
		break;
	case EXPR_FLOAT:
		field_double(out, "value", e->as.float_val);
		fputc('\n', out);
		break;
	case EXPR_BOOL:
		fprintf(out, " value=%d", e->as.bool_val ? 1 : 0);
		fputc('\n', out);
		break;
	case EXPR_NULL:
		fputc('\n', out);
		break;
	case EXPR_STRING:
		fputc('\n', out);
		for (StringPart *sp = e->as.str_parts; sp; sp = sp->next) {
			ind(out, depth + 1);
			if (sp->is_expr) {
				fputs("part expr\n", out);
				dump_expr(sp->expr, depth + 2, out);
			} else {
				fputs("part lit", out);
				field_str(out, "text", sp->literal);
				fputc('\n', out);
			}
		}
		break;
	case EXPR_IDENT:
		field_str(out, "name", e->as.ident);
		fputc('\n', out);
		break;
	case EXPR_ARRAY:
		fputc('\n', out);
		for (int i = 0; i < e->as.array_lit.count; i++)
			dump_expr(e->as.array_lit.items[i], depth + 1, out);
		break;
	case EXPR_DICT:
		fputc('\n', out);
		for (int i = 0; i < e->as.dict_lit.count; i++) {
			ind(out, depth + 1);
			fputs("entry\n", out);
			dump_expr(e->as.dict_lit.keys[i], depth + 2, out);
			dump_expr(e->as.dict_lit.values[i], depth + 2, out);
		}
		break;
	case EXPR_BINARY:
		field_str(out, "op", e->as.binary.op);
		fputc('\n', out);
		dump_expr(e->as.binary.l, depth + 1, out);
		dump_expr(e->as.binary.r, depth + 1, out);
		break;
	case EXPR_UNARY:
		field_str(out, "op", e->as.unary.op);
		fputc('\n', out);
		dump_expr(e->as.unary.operand, depth + 1, out);
		break;
	case EXPR_CALL:
		fputc('\n', out);
		ind(out, depth + 1);
		fputs("callee\n", out);
		dump_expr(e->as.call.callee, depth + 2, out);
		ind(out, depth + 1);
		fputs("args\n", out);
		for (int i = 0; i < e->as.call.arg_count; i++) {
			ind(out, depth + 2);
			fputs("arg", out);
			field_str(out, "name",
				  e->as.call.arg_names ?
					  e->as.call.arg_names[i] :
					  NULL);
			fputc('\n', out);
			dump_expr(e->as.call.args[i], depth + 3, out);
		}
		break;
	case EXPR_FIELD:
	case EXPR_SAFE_FIELD:
		field_str(out, "name", e->as.field.name);
		fputc('\n', out);
		dump_expr(e->as.field.obj, depth + 1, out);
		break;
	case EXPR_INDEX:
		fputc('\n', out);
		dump_expr(e->as.index.arr, depth + 1, out);
		dump_expr(e->as.index.idx, depth + 1, out);
		break;
	case EXPR_IS:
		field_str(out, "type", e->as.is_check.type_name);
		fputc('\n', out);
		dump_expr(e->as.is_check.value, depth + 1, out);
		break;
	case EXPR_ASSIGN:
		fputc('\n', out);
		dump_expr(e->as.assign.target, depth + 1, out);
		dump_expr(e->as.assign.value, depth + 1, out);
		break;
	case EXPR_TERNARY:
		fputc('\n', out);
		dump_expr(e->as.ternary.cond, depth + 1, out);
		dump_expr(e->as.ternary.then_e, depth + 1, out);
		dump_expr(e->as.ternary.else_e, depth + 1, out);
		break;
	}
}

static void dump_stmt(Stmt *s, int depth, FILE *out)
{
	ind(out, depth);
	fprintf(out, "%s line=%d", stmt_kind_name(s->kind), s->line);
	switch (s->kind) {
	case STMT_LET:
		field_str(out, "name", s->as.let.name);
		field_str(out, "type", s->as.let.type_name);
		fprintf(out, " const=%d\n", s->as.let.is_const ? 1 : 0);
		dump_opt("init", s->as.let.init, depth + 1, out);
		break;
	case STMT_EXPR:
		fputc('\n', out);
		dump_opt("expr", s->as.expr_stmt.expr, depth + 1, out);
		break;
	case STMT_RETURN:
		fputc('\n', out);
		dump_opt("value", s->as.ret.value, depth + 1, out);
		break;
	case STMT_IF:
		fputc('\n', out);
		dump_opt("cond", s->as.if_stmt.cond, depth + 1, out);
		dump_body("then", s->as.if_stmt.then_body,
			  s->as.if_stmt.then_count, depth + 1, out);
		dump_body("else", s->as.if_stmt.else_body,
			  s->as.if_stmt.else_count, depth + 1, out);
		break;
	case STMT_WHILE:
		fputc('\n', out);
		dump_opt("cond", s->as.while_stmt.cond, depth + 1, out);
		dump_body("body", s->as.while_stmt.body,
			  s->as.while_stmt.body_count, depth + 1, out);
		break;
	case STMT_FOR:
		field_str(out, "var", s->as.for_stmt.var_name);
		field_str(out, "var2", s->as.for_stmt.var2_name);
		fprintf(out, " iter=%s\n",
			for_iter_kind_name(s->as.for_stmt.kind));
		dump_opt("range_a", s->as.for_stmt.range_a, depth + 1, out);
		dump_opt("range_b", s->as.for_stmt.range_b, depth + 1, out);
		dump_opt("iterable", s->as.for_stmt.iterable, depth + 1, out);
		dump_body("body", s->as.for_stmt.body,
			  s->as.for_stmt.body_count, depth + 1, out);
		break;
	case STMT_SWITCH:
		fputc('\n', out);
		dump_opt("subject", s->as.switch_stmt.subject, depth + 1, out);
		ind(out, depth + 1);
		fputs("cases\n", out);
		for (CaseClause *c = s->as.switch_stmt.cases; c; c = c->next) {
			ind(out, depth + 2);
			fputs("case\n", out);
			dump_opt("value", c->value, depth + 3, out);
			dump_body("body", c->body, c->body_count, depth + 3,
				  out);
		}
		break;
	case STMT_TRY:
		fputc('\n', out);
		dump_body("body", s->as.try_stmt.body,
			  s->as.try_stmt.body_count, depth + 1, out);
		ind(out, depth + 1);
		fputs("catches\n", out);
		for (CatchClause *c = s->as.try_stmt.catches; c; c = c->next) {
			ind(out, depth + 2);
			fputs("catch", out);
			field_str(out, "type", c->type_name);
			field_str(out, "var", c->var_name);
			fputc('\n', out);
			dump_body("body", c->body, c->body_count, depth + 3,
				  out);
		}
		dump_body("finally", s->as.try_stmt.finally_body,
			  s->as.try_stmt.finally_count, depth + 1, out);
		break;
	case STMT_THROW:
		field_str(out, "type", s->as.throw_stmt.type_name);
		fputc('\n', out);
		dump_opt("value", s->as.throw_stmt.value, depth + 1, out);
		break;
	case STMT_BLOCK:
		fputc('\n', out);
		dump_body("body", s->as.block.body, s->as.block.body_count,
			  depth + 1, out);
		break;
	case STMT_BREAK:
	case STMT_CONTINUE:
		fputc('\n', out);
		break;
	}
}

static void dump_func(const char *label, FuncDecl *f, int depth, FILE *out)
{
	ind(out, depth);
	fputs(label, out);
	field_str(out, "name", f->name);
	fprintf(out, " line=%d", f->line);
	field_str(out, "ret", f->return_type);
	fprintf(out, " static=%d private=%d", f->is_static ? 1 : 0,
		f->is_private ? 1 : 0);
	field_str(out, "op", f->op_symbol);
	fputc('\n', out);
	dump_params(f->params, depth + 1, out);
	dump_body("body", f->body, f->body_count, depth + 1, out);
}

static void dump_class(ClassDecl *c, int depth, FILE *out)
{
	ind(out, depth);
	fputs("DECL_CLASS", out);
	field_str(out, "name", c->name);
	field_str(out, "parent", c->parent_name);
	fprintf(out, " line=%d\n", c->line);
	ind(out, depth + 1);
	fputs("fields\n", out);
	for (FieldDecl *f = c->fields; f; f = f->next) {
		ind(out, depth + 2);
		fputs("field", out);
		field_str(out, "name", f->name);
		field_str(out, "type", f->type_name);
		fprintf(out, " static=%d private=%d const=%d\n",
			f->is_static ? 1 : 0, f->is_private ? 1 : 0,
			f->is_const ? 1 : 0);
		dump_opt("init", f->init, depth + 3, out);
	}
	ind(out, depth + 1);
	fputs("methods\n", out);
	for (int i = 0; i < c->method_count; i++)
		dump_func("method", c->methods[i], depth + 2, out);
}

void dump_ast(Program *prog, FILE *out)
{
	fputs("program\n", out);
	for (Decl *d = prog->decls; d; d = d->next) {
		switch (d->kind) {
		case DECL_FUNC:
		case DECL_OPERATOR:
			dump_func(decl_kind_name(d->kind), d->as.func, 1, out);
			break;
		case DECL_CLASS:
			dump_class(d->as.klass, 1, out);
			break;
		case DECL_IMPORT:
			ind(out, 1);
			fputs("DECL_IMPORT", out);
			field_str(out, "module", d->as.import.module);
			field_str(out, "alias", d->as.import.alias);
			fputc('\n', out);
			ind(out, 2);
			fputs("members\n", out);
			for (int i = 0; i < d->as.import.member_count; i++) {
				ind(out, 3);
				fputs("member", out);
				field_str(out, "name", d->as.import.members[i]);
				fputc('\n', out);
			}
			break;
		case DECL_STMT:
			ind(out, 1);
			fputs("DECL_STMT\n", out);
			dump_stmt(d->as.stmt, 2, out);
			break;
		case DECL_EVENT:
			ind(out, 1);
			fputs("DECL_EVENT", out);
			field_str(out, "name", d->as.event.name);
			fprintf(out, " line=%d\n", d->as.event.line);
			dump_params(d->as.event.params, 2, out);
			break;
		case DECL_ON:
			ind(out, 1);
			fputs("DECL_ON", out);
			field_str(out, "event", d->as.on.event_name);
			field_str(out, "var", d->as.on.var_name);
			fprintf(out, " line=%d\n", d->as.on.line);
			dump_body("body", d->as.on.body, d->as.on.body_count, 2,
				  out);
			break;
		case DECL_CIMPORT:
			ind(out, 1);
			fputs("DECL_CIMPORT", out);
			field_str(out, "name", d->as.cimport.name);
			field_str(out, "lib", d->as.cimport.lib);
			fprintf(out, " line=%d\n", d->as.cimport.line);
			break;
		}
	}
}
