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
#include "codegen.h"
#include "lexer.h"
#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <libgen.h>

/* ============================= symbol tables ============================= */

static ClassDecl **g_classes = NULL;
static int g_class_count = 0;
static bool *g_class_emitted = NULL;

static char *g_source_dir = NULL; /* directory of the file being compiled, for resolving imports */

typedef struct { char *local_name; char *module; } ImportBinding;
static ImportBinding *g_import_aliases = NULL; /* alias-or-module-name -> module */
static int g_import_alias_count = 0;
static ImportBinding *g_import_directs = NULL; /* bare member name -> module */
static int g_import_direct_count = 0;
static char **g_processed_modules = NULL;
static int g_processed_module_count = 0;

/* top-level `operator <sym> (a, b)` declarations: symbol -> generated C function */
typedef struct { char *symbol; char *cfunc; FuncDecl *decl; } UserOp;
static UserOp *g_user_ops = NULL;
static int g_user_op_count = 0;

/* class `operator <sym> (this, other)` overloads, for registration in static init
   and for knowing a symbol is meaningful even without a top-level definition */
typedef struct { ClassDecl *cls; char *symbol; char *cfunc; FuncDecl *decl; } ClassOp;
static ClassOp *g_class_ops = NULL;
static int g_class_op_count = 0;

/* events and their `on` handlers */
typedef struct { EventDecl *decl; ClassDecl *cls; } EventInfo;
static EventInfo *g_events = NULL;
static int g_event_count = 0;
typedef struct { OnDecl *decl; char *cfunc; } HandlerInfo;
static HandlerInfo *g_handlers = NULL;
static int g_handler_count = 0;

/* cimport FFI bindings: Oboe-visible name -> library */
typedef struct { char *name; char *lib; } FfiBinding;
static FfiBinding *g_ffi = NULL;
static int g_ffi_count = 0;

static FILE *OUT;

static void codegen_error(int line, const char *msg) {
    fprintf(stderr, "oboe: codegen error at line %d: %s\n", line, msg);
    exit(1);
}

static ClassDecl *find_class(const char *name) {
    if (!name) return NULL;
    for (int i = 0; i < g_class_count; i++) if (strcmp(g_classes[i]->name, name) == 0) return g_classes[i];
    return NULL;
}

static FieldDecl *find_field_local(ClassDecl *c, const char *name) {
    for (FieldDecl *f = c->fields; f; f = f->next) if (strcmp(f->name, name) == 0) return f;
    return NULL;
}

static FuncDecl *find_method_local(ClassDecl *c, const char *name) {
    for (int i = 0; i < c->method_count; i++)
        if (strcmp(c->methods[i]->name, name) == 0 && strcmp(name, "init") != 0) return c->methods[i];
    return NULL;
}

static ClassDecl *find_field_owner(ClassDecl *c, const char *name, FieldDecl **out) {
    while (c) {
        FieldDecl *f = find_field_local(c, name);
        if (f) { *out = f; return c; }
        c = c->parent_name ? find_class(c->parent_name) : NULL;
    }
    return NULL;
}

static ClassDecl *find_method_owner(ClassDecl *c, const char *name, FuncDecl **out) {
    while (c) {
        FuncDecl *m = find_method_local(c, name);
        if (m) { *out = m; return c; }
        c = c->parent_name ? find_class(c->parent_name) : NULL;
    }
    return NULL;
}

static UserOp *find_user_op(const char *sym) {
    for (int i = 0; i < g_user_op_count; i++)
        if (strcmp(g_user_ops[i].symbol, sym) == 0) return &g_user_ops[i];
    return NULL;
}

static bool class_op_exists(const char *sym) {
    for (int i = 0; i < g_class_op_count; i++)
        if (strcmp(g_class_ops[i].symbol, sym) == 0) return true;
    return false;
}

static EventInfo *find_event(const char *name) {
    for (int i = 0; i < g_event_count; i++)
        if (strcmp(g_events[i].decl->name, name) == 0) return &g_events[i];
    return NULL;
}

static FfiBinding *find_ffi(const char *name) {
    for (int i = 0; i < g_ffi_count; i++)
        if (strcmp(g_ffi[i].name, name) == 0) return &g_ffi[i];
    return NULL;
}

/* ============================= scopes: variable -> static class type ===== */

typedef struct EnvEntry { char *name; char *class_name; struct EnvEntry *next; } EnvEntry;
typedef struct Scope { EnvEntry *entries; struct Scope *parent; } Scope;
static Scope *g_scope = NULL;

static void push_scope(void) { Scope *s = calloc(1, sizeof(Scope)); s->parent = g_scope; g_scope = s; }
static void pop_scope(void) { g_scope = g_scope->parent; }
static void define_var(const char *name, const char *class_name) {
    EnvEntry *e = calloc(1, sizeof(EnvEntry));
    e->name = strdup(name);
    e->class_name = class_name ? strdup(class_name) : NULL;
    e->next = g_scope->entries;
    g_scope->entries = e;
}
static bool var_in_scope(const char *name) {
    for (Scope *s = g_scope; s; s = s->parent)
        for (EnvEntry *e = s->entries; e; e = e->next)
            if (strcmp(e->name, name) == 0) return true;
    return false;
}
static const char *lookup_var_class(const char *name) {
    for (Scope *s = g_scope; s; s = s->parent)
        for (EnvEntry *e = s->entries; e; e = e->next)
            if (strcmp(e->name, name) == 0) return e->class_name;
    return NULL;
}

static const char *current_class = NULL; /* class name while generating a method body, else NULL */

/* ============================= forward decls ============================= */
static char *gen_expr(Expr *e);
static void gen_stmt_list(Stmt **body, int count, int indent);
static void gen_func_def(const char *prefix, ClassDecl *owner, FuncDecl *f);
static void process_import(ImportDecl *imp);

static void ind(int n) { for (int i = 0; i < n; i++) fputc(' ', OUT); }

/* returns the static class name of an expression, or NULL if not a known class instance */
static const char *infer_class(Expr *e) {
    if (!e) return NULL;
    switch (e->kind) {
        case EXPR_IDENT: {
            if (strcmp(e->as.ident, "this") == 0) return current_class;
            return lookup_var_class(e->as.ident);
        }
        case EXPR_CALL: {
            if (e->as.call.callee->kind == EXPR_IDENT) {
                ClassDecl *c = find_class(e->as.call.callee->as.ident);
                if (c) return c->name;
            }
            return NULL;
        }
        case EXPR_FIELD: {
            const char *base_class = infer_class(e->as.field.obj);
            if (!base_class) return NULL;
            ClassDecl *bc = find_class(base_class);
            if (!bc) return NULL;
            FieldDecl *fd;
            if (find_field_owner(bc, e->as.field.name, &fd)) {
                return find_class(fd->type_name) ? fd->type_name : NULL;
            }
            return NULL;
        }
        default: return NULL;
    }
}

/* ============================= expressions ============================= */

static char *fmt(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    char buf[4096];
    vsnprintf(buf, sizeof buf, format, ap);
    va_end(ap);
    return strdup(buf);
}

static char *gen_string_literal(Expr *e) {
    StringPart *parts = e->as.str_parts;
    int count = 0;
    for (StringPart *p = parts; p; p = p->next) count++;
    if (count == 0) return strdup("ob_string(\"\")");
    char buf[8192];
    int off = snprintf(buf, sizeof buf, "ob_interpolate(%d", count);
    for (StringPart *p = parts; p; p = p->next) {
        if (p->is_expr) {
            char *sub = gen_expr(p->expr);
            char *sub_str;
            /* ensure every interpolated part is coerced to a string value */
            sub_str = fmt("ob_str(%s)", sub);
            off += snprintf(buf + off, sizeof(buf) - off, ", %s", sub_str);
            free(sub);
            free(sub_str);
        } else {
            /* escape the literal for embedding in a C string literal */
            char escaped[4096];
            int eo = 0;
            for (const char *c = p->literal; *c && eo < (int)sizeof(escaped) - 2; c++) {
                if (*c == '"' || *c == '\\') escaped[eo++] = '\\';
                if (*c == '\n') { escaped[eo++] = '\\'; escaped[eo++] = 'n'; continue; }
                escaped[eo++] = *c;
            }
            escaped[eo] = '\0';
            off += snprintf(buf + off, sizeof(buf) - off, ", ob_string(\"%s\")", escaped);
        }
    }
    off += snprintf(buf + off, sizeof(buf) - off, ")");
    return strdup(buf);
}

static const char *builtin_type_check(const char *type_name) {
    if (strcmp(type_name, "int") == 0) return "ob_is_int";
    if (strcmp(type_name, "bool") == 0) return "ob_is_bool";
    if (strcmp(type_name, "string") == 0) return "ob_is_string";
    if (strcmp(type_name, "array") == 0) return "ob_is_array";
    if (strcmp(type_name, "dict") == 0) return "ob_is_dict";
    return NULL;
}

/* Resolves `obj.name` (field read, method-call callee, or static access) into
   a C expression. `for_call` indicates the result will be immediately invoked
   with args (needed to know whether to look up a field or a method); when so,
   the callee is returned still open (ending in an unclosed '(') and, for
   instance methods, *out_first_arg receives the receiver pointer expression
   that the caller must splice in as the method's first argument. */
static char *gen_member_access_ex(Expr *field_expr, bool for_call, bool safe, char **out_first_arg) {
    if (out_first_arg) *out_first_arg = NULL;
    Expr *obj = field_expr->as.field.obj;
    const char *name = field_expr->as.field.name;

    /* static access: ClassName.member, where ClassName isn't shadowed by a variable */
    if (obj->kind == EXPR_IDENT && !var_in_scope(obj->as.ident)) {
        ClassDecl *sc = find_class(obj->as.ident);
        if (sc) {
            if (for_call) return fmt("%s__%s(", sc->name, name);
            return fmt("%s__%s", sc->name, name);
        }
        /* module-qualified access, e.g. `alias.member` or `module.member` */
        for (int i = 0; i < g_import_alias_count; i++) {
            if (strcmp(g_import_aliases[i].local_name, obj->as.ident) == 0) {
                if (for_call) return fmt("%s__%s(", g_import_aliases[i].module, name);
                return fmt("%s__%s", g_import_aliases[i].module, name);
            }
        }
    }

    bool is_this = (obj->kind == EXPR_IDENT && strcmp(obj->as.ident, "this") == 0);
    const char *base_class = infer_class(obj);
    char *obj_code = is_this ? NULL : gen_expr(obj);

    if (!base_class) {
        codegen_error(field_expr->line, "cannot resolve the static type of this expression for member access (compile-time dispatch requires a known class)");
    }
    ClassDecl *bc = find_class(base_class);

    if (for_call) {
        FuncDecl *m;
        ClassDecl *owner = find_method_owner(bc, name, &m);
        if (!owner) codegen_error(field_expr->line, "no such method on this class (or its ancestors)");
        if (m->is_private && (!current_class || strcmp(current_class, owner->name) != 0))
            codegen_error(field_expr->line, "method is private");
        char *ptr_expr = is_this ? fmt("((%s*)(this))", owner->name) : fmt("((%s*)((%s).as.obj))", owner->name, obj_code);
        char *result = fmt("%s__%s(", owner->name, name);
        if (obj_code) free(obj_code);
        if (out_first_arg) *out_first_arg = ptr_expr; else free(ptr_expr);
        return result;
    }

    FieldDecl *fd;
    ClassDecl *owner = find_field_owner(bc, name, &fd);
    if (!owner) codegen_error(field_expr->line, "no such field on this class (or its ancestors)");
    if (fd->is_private && (!current_class || strcmp(current_class, owner->name) != 0))
        codegen_error(field_expr->line, "field is private");
    char *ptr_expr = is_this ? fmt("((%s*)(this))", owner->name) : fmt("((%s*)((%s).as.obj))", owner->name, obj_code);
    char *access = fmt("%s->%s", ptr_expr, name);
    free(ptr_expr);
    if (safe && !is_this) {
        char *guarded = fmt("(ob_is_null(%s) ? ob_null() : %s)", obj_code, access);
        free(access);
        free(obj_code);
        return guarded;
    }
    free(obj_code);
    return access;
}

static char *gen_member_access(Expr *field_expr, bool for_call, bool safe) {
    return gen_member_access_ex(field_expr, for_call, safe, NULL);
}

static char *gen_assign_target_lvalue(Expr *target) {
    /* returns a C lvalue string for simple (non-index) assignment targets */
    if (target->kind == EXPR_IDENT) return strdup(target->as.ident);
    if (target->kind == EXPR_FIELD) return gen_member_access(target, false, false);
    codegen_error(target->line, "invalid assignment target");
    return NULL;
}

static char *gen_expr(Expr *e) {
    switch (e->kind) {
        case EXPR_INT: return fmt("ob_int(%lldLL)", e->as.int_val);
        case EXPR_BOOL: return fmt("ob_bool(%s)", e->as.bool_val ? "true" : "false");
        case EXPR_NULL: return strdup("ob_null()");
        case EXPR_STRING: return gen_string_literal(e);
        case EXPR_IDENT: return strdup(e->as.ident);
        case EXPR_ARRAY: {
            char buf[8192];
            int off = snprintf(buf, sizeof buf, "({ OboeValue __a = ob_array_new();");
            for (int i = 0; i < e->as.array_lit.count; i++) {
                char *item = gen_expr(e->as.array_lit.items[i]);
                off += snprintf(buf + off, sizeof(buf) - off, " ob_array_push(__a, %s);", item);
                free(item);
            }
            off += snprintf(buf + off, sizeof(buf) - off, " __a; })");
            return strdup(buf);
        }
        case EXPR_DICT: {
            char buf[8192];
            int off = snprintf(buf, sizeof buf, "({ OboeValue __d = ob_dict_new();");
            for (int i = 0; i < e->as.dict_lit.count; i++) {
                char *key = gen_expr(e->as.dict_lit.keys[i]);
                char *val = gen_expr(e->as.dict_lit.values[i]);
                off += snprintf(buf + off, sizeof(buf) - off, " ob_dict_set(__d, ob_to_cstr(%s), %s);", key, val);
                free(key); free(val);
            }
            off += snprintf(buf + off, sizeof(buf) - off, " __d; })");
            return strdup(buf);
        }
        case EXPR_BINARY: {
            char *l = gen_expr(e->as.binary.l);
            char *r = gen_expr(e->as.binary.r);
            char *op = e->as.binary.op;
            char *result;
            if (strcmp(op, "&&") == 0) result = fmt("ob_and(%s, %s)", l, r);
            else if (strcmp(op, "||") == 0) result = fmt("ob_or(%s, %s)", l, r);
            else if (strcmp(op, "??") == 0) result = fmt("ob_coalesce(%s, %s)", l, r);
            else {
                const char *fallback =
                    strcmp(op, "+") == 0 ? "ob_add" :
                    strcmp(op, "-") == 0 ? "ob_sub" :
                    strcmp(op, "*") == 0 ? "ob_mul" :
                    strcmp(op, "/") == 0 ? "ob_div" :
                    strcmp(op, "%") == 0 ? "ob_mod" :
                    strcmp(op, "==") == 0 ? "ob_eq" :
                    strcmp(op, "!=") == 0 ? "ob_neq" :
                    strcmp(op, "<") == 0 ? "ob_lt" :
                    strcmp(op, "<=") == 0 ? "ob_lte" :
                    strcmp(op, ">") == 0 ? "ob_gt" :
                    strcmp(op, ">=") == 0 ? "ob_gte" :
                    strcmp(op, "x") == 0 ? "ob_repeat" : NULL;
                if (!fallback) {
                    /* user-declared operator: dispatch through the overload
                       registry, falling back to the top-level definition */
                    UserOp *uo = find_user_op(op);
                    if (uo) fallback = uo->cfunc;
                    else if (class_op_exists(op)) fallback = "ob_op_missing";
                }
                if (!fallback) codegen_error(e->line, "unknown binary operator");
                result = fmt("ob_binop(\"%s\", %s, %s, %s)", op, l, r, fallback);
            }
            free(l); free(r);
            return result;
        }
        case EXPR_UNARY: {
            char *v = gen_expr(e->as.unary.operand);
            char *result = strcmp(e->as.unary.op, "!") == 0 ? fmt("ob_not(%s)", v) : fmt("ob_neg(%s)", v);
            free(v);
            return result;
        }
        case EXPR_IS: {
            char *v = gen_expr(e->as.is_check.value);
            const char *builtin = builtin_type_check(e->as.is_check.type_name);
            char *result;
            if (builtin) result = fmt("ob_bool(%s(%s))", builtin, v);
            else result = fmt("ob_bool(ob_is_object_of(%s, &%s__classinfo))", v, e->as.is_check.type_name);
            free(v);
            return result;
        }
        case EXPR_INDEX: {
            char *arr = gen_expr(e->as.index.arr);
            char *idx = gen_expr(e->as.index.idx);
            char *result = fmt("ob_index_get(%s, %s)", arr, idx);
            free(arr); free(idx);
            return result;
        }
        case EXPR_FIELD: return gen_member_access(e, false, false);
        case EXPR_SAFE_FIELD: return gen_member_access(e, false, true);
        case EXPR_CALL: {
            Expr *callee = e->as.call.callee;
            /* builtins */
            if (callee->kind == EXPR_IDENT && !var_in_scope(callee->as.ident)) {
                if (strcmp(callee->as.ident, "print") == 0 && e->as.call.arg_count == 1) {
                    char *a = gen_expr(e->as.call.args[0]);
                    char *r = fmt("(ob_print(%s), ob_null())", a);
                    free(a);
                    return r;
                }
                if (strcmp(callee->as.ident, "write") == 0 && e->as.call.arg_count == 1) {
                    char *a = gen_expr(e->as.call.args[0]);
                    char *r = fmt("(ob_write(%s), ob_null())", a);
                    free(a);
                    return r;
                }
                if (strcmp(callee->as.ident, "input") == 0 && e->as.call.arg_count == 0) {
                    return strdup("ob_input()");
                }
                if (strcmp(callee->as.ident, "str") == 0 && e->as.call.arg_count == 1) {
                    char *a = gen_expr(e->as.call.args[0]);
                    char *r = fmt("ob_str(%s)", a);
                    free(a);
                    return r;
                }
                if (strcmp(callee->as.ident, "range") == 0 && e->as.call.arg_count == 2) {
                    char *a = gen_expr(e->as.call.args[0]);
                    char *b = gen_expr(e->as.call.args[1]);
                    char *r = fmt("ob_range((%s).as.i, (%s).as.i)", a, b);
                    free(a); free(b);
                    return r;
                }
                /* constructor call */
                ClassDecl *c = find_class(callee->as.ident);
                if (c) {
                    int argc = e->as.call.arg_count;
                    int init_count = 0;
                    for (int i = 0; i < c->method_count; i++) if (strcmp(c->methods[i]->name, "init") == 0) init_count++;
                    int chosen = -1, idx = 0;
                    for (int i = 0; i < c->method_count; i++) {
                        if (strcmp(c->methods[i]->name, "init") != 0) continue;
                        int pc = 0;
                        for (Param *p = c->methods[i]->params; p; p = p->next) if (strcmp(p->name, "this") != 0) pc++;
                        if (pc == argc) { chosen = idx; break; }
                        idx++;
                    }
                    char buf[4096];
                    int off;
                    if (init_count == 0 && argc == 0) {
                        off = snprintf(buf, sizeof buf, "%s__new_default(", c->name);
                    } else if (chosen >= 0) {
                        off = snprintf(buf, sizeof buf, "%s__new_%d(", c->name, chosen);
                    } else {
                        codegen_error(e->line, "no matching constructor overload for this argument count");
                        off = 0;
                    }
                    for (int i = 0; i < argc; i++) {
                        char *a = gen_expr(e->as.call.args[i]);
                        off += snprintf(buf + off, sizeof(buf) - off, "%s%s", i ? ", " : "", a);
                        free(a);
                    }
                    off += snprintf(buf + off, sizeof(buf) - off, ")");
                    return strdup(buf);
                }
                /* cimport FFI binding */
                FfiBinding *ffi = find_ffi(callee->as.ident);
                if (ffi) {
                    char buf[4096];
                    int off = snprintf(buf, sizeof buf, "ob_ffi_call(__ffi_%s, %d", ffi->name, e->as.call.arg_count);
                    for (int i = 0; i < e->as.call.arg_count; i++) {
                        char *a = gen_expr(e->as.call.args[i]);
                        off += snprintf(buf + off, sizeof(buf) - off, ", %s", a);
                        free(a);
                    }
                    off += snprintf(buf + off, sizeof(buf) - off, ")");
                    return strdup(buf);
                }
                /* direct `from` import binding */
                for (int i = 0; i < g_import_direct_count; i++) {
                    if (strcmp(g_import_directs[i].local_name, callee->as.ident) == 0) {
                        char buf[4096];
                        int off = snprintf(buf, sizeof buf, "%s__%s(", g_import_directs[i].module, callee->as.ident);
                        for (int i2 = 0; i2 < e->as.call.arg_count; i2++) {
                            char *a = gen_expr(e->as.call.args[i2]);
                            off += snprintf(buf + off, sizeof(buf) - off, "%s%s", i2 ? ", " : "", a);
                            free(a);
                        }
                        off += snprintf(buf + off, sizeof(buf) - off, ")");
                        return strdup(buf);
                    }
                }
                /* plain function call */
                char buf[4096];
                const char *fname = strcmp(callee->as.ident, "main") == 0 ? "oboe_user_main" : callee->as.ident;
                int off = snprintf(buf, sizeof buf, "%s(", fname);
                for (int i = 0; i < e->as.call.arg_count; i++) {
                    char *a = gen_expr(e->as.call.args[i]);
                    off += snprintf(buf + off, sizeof(buf) - off, "%s%s", i ? ", " : "", a);
                    free(a);
                }
                off += snprintf(buf + off, sizeof(buf) - off, ")");
                return strdup(buf);
            }
            if (callee->kind == EXPR_FIELD) {
                char *first_arg = NULL;
                char *callee_code = gen_member_access_ex(callee, true, false, &first_arg);
                char args_buf[4096];
                int aoff = 0;
                bool first = true;
                if (first_arg) { aoff += snprintf(args_buf, sizeof args_buf, "%s", first_arg); first = false; free(first_arg); }
                for (int i = 0; i < e->as.call.arg_count; i++) {
                    char *a = gen_expr(e->as.call.args[i]);
                    aoff += snprintf(args_buf + aoff, sizeof(args_buf) - aoff, "%s%s", first ? "" : ", ", a);
                    first = false;
                    free(a);
                }
                char final_buf[8192];
                snprintf(final_buf, sizeof final_buf, "%s%s)", callee_code, args_buf);
                free(callee_code);
                return strdup(final_buf);
            }
            codegen_error(e->line, "unsupported call expression");
            return NULL;
        }
        case EXPR_ASSIGN: {
            if (e->as.assign.target->kind == EXPR_INDEX) {
                char *arr = gen_expr(e->as.assign.target->as.index.arr);
                char *idx = gen_expr(e->as.assign.target->as.index.idx);
                char *val = gen_expr(e->as.assign.value);
                char *r = fmt("ob_index_set(%s, %s, %s)", arr, idx, val);
                free(arr); free(idx); free(val);
                return r;
            }
            char *lvalue = gen_assign_target_lvalue(e->as.assign.target);
            char *val = gen_expr(e->as.assign.value);
            char *r = fmt("(%s = %s)", lvalue, val);
            free(lvalue); free(val);
            return r;
        }
    }
    codegen_error(e->line, "unknown expression kind");
    return NULL;
}

/* ============================= statements ============================= */

static void gen_stmt(Stmt *s, int indent) {
    switch (s->kind) {
        case STMT_LET: {
            const char *class_type = NULL;
            if (s->as.let.type_name && find_class(s->as.let.type_name)) class_type = s->as.let.type_name;
            else if (s->as.let.init) class_type = infer_class(s->as.let.init);
            define_var(s->as.let.name, class_type);
            char *init = s->as.let.init ? gen_expr(s->as.let.init) : strdup("ob_null()");
            ind(indent);
            fprintf(OUT, "%sOboeValue %s = %s;\n", s->as.let.is_const ? "const " : "", s->as.let.name, init);
            free(init);
            break;
        }
        case STMT_EXPR: {
            char *e = gen_expr(s->as.expr_stmt.expr);
            ind(indent);
            fprintf(OUT, "(void)(%s);\n", e);
            free(e);
            break;
        }
        case STMT_RETURN: {
            ind(indent);
            if (s->as.ret.value) {
                char *v = gen_expr(s->as.ret.value);
                fprintf(OUT, "return %s;\n", v);
                free(v);
            } else {
                fprintf(OUT, "return ob_null();\n");
            }
            break;
        }
        case STMT_IF: {
            char *cond = gen_expr(s->as.if_stmt.cond);
            ind(indent);
            fprintf(OUT, "if (ob_truthy(%s)) {\n", cond);
            free(cond);
            push_scope();
            gen_stmt_list(s->as.if_stmt.then_body, s->as.if_stmt.then_count, indent + 4);
            pop_scope();
            ind(indent);
            fprintf(OUT, "}\n");
            if (s->as.if_stmt.else_count > 0) {
                ind(indent);
                fprintf(OUT, "else {\n");
                push_scope();
                gen_stmt_list(s->as.if_stmt.else_body, s->as.if_stmt.else_count, indent + 4);
                pop_scope();
                ind(indent);
                fprintf(OUT, "}\n");
            }
            break;
        }
        case STMT_WHILE: {
            char *cond = gen_expr(s->as.while_stmt.cond);
            ind(indent);
            fprintf(OUT, "while (ob_truthy(%s)) {\n", cond);
            free(cond);
            push_scope();
            gen_stmt_list(s->as.while_stmt.body, s->as.while_stmt.body_count, indent + 4);
            pop_scope();
            ind(indent);
            fprintf(OUT, "}\n");
            break;
        }
        case STMT_FOR: {
            push_scope();
            if (s->as.for_stmt.is_range) {
                char *a = gen_expr(s->as.for_stmt.range_a);
                char *b = gen_expr(s->as.for_stmt.range_b);
                ind(indent);
                fprintf(OUT, "for (int64_t __i = (%s).as.i, __end = (%s).as.i; __i < __end; __i++) {\n", a, b);
                free(a); free(b);
                ind(indent + 4);
                fprintf(OUT, "OboeValue %s = ob_int(__i);\n", s->as.for_stmt.var_name);
                define_var(s->as.for_stmt.var_name, NULL);
                gen_stmt_list(s->as.for_stmt.body, s->as.for_stmt.body_count, indent + 4);
                ind(indent);
                fprintf(OUT, "}\n");
            } else {
                char *iter = gen_expr(s->as.for_stmt.iterable);
                ind(indent);
                fprintf(OUT, "{ OboeValue __arr = %s; int64_t __n = ob_array_len(__arr);\n", iter);
                free(iter);
                ind(indent);
                fprintf(OUT, "for (int64_t __i = 0; __i < __n; __i++) {\n");
                ind(indent + 4);
                fprintf(OUT, "OboeValue %s = ob_array_get(__arr, __i);\n", s->as.for_stmt.var_name);
                define_var(s->as.for_stmt.var_name, NULL);
                gen_stmt_list(s->as.for_stmt.body, s->as.for_stmt.body_count, indent + 4);
                ind(indent);
                fprintf(OUT, "} }\n");
            }
            pop_scope();
            break;
        }
        case STMT_SWITCH: {
            ind(indent);
            char *subj = gen_expr(s->as.switch_stmt.subject);
            fprintf(OUT, "{ OboeValue __subj = %s;\n", subj);
            free(subj);
            bool first = true;
            for (CaseClause *c = s->as.switch_stmt.cases; c; c = c->next) {
                char *val = gen_expr(c->value);
                ind(indent);
                fprintf(OUT, "%s (ob_truthy(ob_eq(__subj, %s))) {\n", first ? "if" : "else if", val);
                free(val);
                first = false;
                push_scope();
                gen_stmt_list(c->body, c->body_count, indent + 4);
                pop_scope();
                ind(indent);
                fprintf(OUT, "}\n");
            }
            ind(indent);
            fprintf(OUT, "}\n");
            break;
        }
        case STMT_TRY: {
            /* An unmatched exception must still run `finally` before propagating,
               so the rethrow is deferred via a flag until after the finally body. */
            ind(indent);
            fprintf(OUT, "{ OboeExceptionFrame __frame; __frame.prev = ob_exc_stack; ob_exc_stack = &__frame;\n");
            ind(indent);
            fprintf(OUT, "bool __rethrow = false;\n");
            ind(indent);
            fprintf(OUT, "if (setjmp(__frame.buf) == 0) {\n");
            push_scope();
            gen_stmt_list(s->as.try_stmt.body, s->as.try_stmt.body_count, indent + 4);
            pop_scope();
            ind(indent + 4);
            fprintf(OUT, "ob_exc_stack = __frame.prev;\n");
            ind(indent);
            fprintf(OUT, "} else {\n");
            bool first = true;
            for (CatchClause *c = s->as.try_stmt.catches; c; c = c->next) {
                ind(indent + 4);
                fprintf(OUT, "%s (ob_exception_matches(\"%s\")) {\n", first ? "if" : "else if", c->type_name);
                first = false;
                ind(indent + 8);
                fprintf(OUT, "OboeValue %s = ob_current_exception;\n", c->var_name);
                push_scope();
                define_var(c->var_name, NULL);
                gen_stmt_list(c->body, c->body_count, indent + 8);
                pop_scope();
                ind(indent + 4);
                fprintf(OUT, "}\n");
            }
            ind(indent + 4);
            if (s->as.try_stmt.catches) fprintf(OUT, "else { __rethrow = true; }\n");
            else fprintf(OUT, "__rethrow = true;\n");
            ind(indent);
            fprintf(OUT, "}\n");
            if (s->as.try_stmt.finally_count > 0) {
                push_scope();
                gen_stmt_list(s->as.try_stmt.finally_body, s->as.try_stmt.finally_count, indent);
                pop_scope();
            }
            ind(indent);
            fprintf(OUT, "if (__rethrow) ob_throw(ob_current_exception_type, ob_current_exception);\n");
            ind(indent);
            fprintf(OUT, "}\n");
            break;
        }
        case STMT_THROW: {
            char *val = s->as.throw_stmt.value ? gen_expr(s->as.throw_stmt.value) : strdup("ob_null()");
            ind(indent);
            fprintf(OUT, "ob_throw(\"%s\", %s);\n", s->as.throw_stmt.type_name, val);
            free(val);
            break;
        }
        case STMT_BLOCK: {
            ind(indent);
            fprintf(OUT, "{\n");
            push_scope();
            gen_stmt_list(s->as.block.body, s->as.block.body_count, indent + 4);
            pop_scope();
            ind(indent);
            fprintf(OUT, "}\n");
            break;
        }
    }
}

static void gen_stmt_list(Stmt **body, int count, int indent) {
    for (int i = 0; i < count; i++) gen_stmt(body[i], indent);
}

/* ============================= functions / classes ============================= */

static void gen_param_list(FILE *f, ClassDecl *owner, Param *params, bool skip_this) {
    bool first = true;
    if (owner && skip_this) {
        fprintf(f, "%s* this", owner->name);
        first = false;
    }
    for (Param *p = params; p; p = p->next) {
        if (strcmp(p->name, "this") == 0) continue;
        if (!first) fprintf(f, ", ");
        fprintf(f, "OboeValue %s", p->name);
        first = false;
    }
}

static void gen_func_def(const char *prefix, ClassDecl *owner, FuncDecl *f) {
    bool is_method = owner != NULL;
    bool has_this = false;
    for (Param *p = f->params; p; p = p->next) if (strcmp(p->name, "this") == 0) has_this = true;

    const char *fname_prefix = prefix ? prefix : "";
    const char *fname = (!is_method && strcmp(f->name, "main") == 0) ? "oboe_user_main" : f->name;
    fprintf(OUT, "OboeValue %s%s%s(", fname_prefix, is_method ? "__" : "", fname);
    gen_param_list(OUT, (is_method && has_this) ? owner : NULL, f->params, has_this);
    fprintf(OUT, ") {\n");

    push_scope();
    if (is_method && has_this) define_var("this", owner->name);
    for (Param *p = f->params; p; p = p->next) {
        if (strcmp(p->name, "this") == 0) continue;
        const char *pc = (p->type_name && find_class(p->type_name)) ? p->type_name : NULL;
        define_var(p->name, pc);
    }
    const char *saved_class = current_class;
    current_class = is_method ? owner->name : NULL;
    gen_stmt_list(f->body, f->body_count, 4);
    current_class = saved_class;
    pop_scope();

    fprintf(OUT, "    return ob_null();\n");
    fprintf(OUT, "}\n\n");
}

/* Forward prototypes so definition order (and mutual recursion) never matters
   in the generated C. Signatures must mirror gen_func_def/gen_class exactly. */
static void emit_func_prototype(const char *prefix, ClassDecl *owner, FuncDecl *f) {
    bool is_method = owner != NULL;
    bool has_this = false;
    for (Param *p = f->params; p; p = p->next) if (strcmp(p->name, "this") == 0) has_this = true;
    const char *fname_prefix = prefix ? prefix : "";
    const char *fname = (!is_method && strcmp(f->name, "main") == 0) ? "oboe_user_main" : f->name;
    fprintf(OUT, "OboeValue %s%s%s(", fname_prefix, is_method ? "__" : "", fname);
    gen_param_list(OUT, (is_method && has_this) ? owner : NULL, f->params, has_this);
    fprintf(OUT, ");\n");
}

static void emit_class_predecls(ClassDecl *c) {
    for (FieldDecl *fd = c->fields; fd; fd = fd->next) {
        if (fd->is_static) fprintf(OUT, "static OboeValue %s__%s;\n", c->name, fd->name);
    }
    int init_count = 0, idx = 0;
    for (int i = 0; i < c->method_count; i++) if (strcmp(c->methods[i]->name, "init") == 0) init_count++;
    if (init_count == 0) fprintf(OUT, "OboeValue %s__new_default(void);\n", c->name);
    for (int i = 0; i < c->method_count; i++) {
        FuncDecl *m = c->methods[i];
        if (m->op_symbol) {
            for (int j = 0; j < g_class_op_count; j++)
                if (g_class_ops[j].decl == m)
                    fprintf(OUT, "static OboeValue %s(OboeValue __self, OboeValue __rhs);\n", g_class_ops[j].cfunc);
            continue;
        }
        if (strcmp(m->name, "init") == 0) {
            fprintf(OUT, "void %s__init_%d(%s *this", c->name, idx, c->name);
            for (Param *p = m->params; p; p = p->next) {
                if (strcmp(p->name, "this") == 0) continue;
                fprintf(OUT, ", OboeValue %s", p->name);
            }
            fprintf(OUT, ");\n");
            fprintf(OUT, "OboeValue %s__new_%d(", c->name, idx);
            bool first = true;
            for (Param *p = m->params; p; p = p->next) {
                if (strcmp(p->name, "this") == 0) continue;
                if (!first) fprintf(OUT, ", ");
                fprintf(OUT, "OboeValue %s", p->name);
                first = false;
            }
            fprintf(OUT, ");\n");
            idx++;
        } else if (m->is_static) {
            fprintf(OUT, "OboeValue %s__%s(", c->name, m->name);
            bool first = true;
            for (Param *p = m->params; p; p = p->next) {
                if (!first) fprintf(OUT, ", ");
                fprintf(OUT, "OboeValue %s", p->name);
                first = false;
            }
            fprintf(OUT, ");\n");
        } else {
            emit_func_prototype(c->name, c, m);
        }
    }
}

static void emit_class_struct(ClassDecl *c, int idx) {
    if (g_class_emitted[idx]) return;
    ClassDecl *parent = c->parent_name ? find_class(c->parent_name) : NULL;
    if (parent) {
        for (int i = 0; i < g_class_count; i++) if (g_classes[i] == parent) emit_class_struct(parent, i);
    }
    fprintf(OUT, "typedef struct %s %s;\n", c->name, c->name);
    fprintf(OUT, "struct %s {\n", c->name);
    if (parent) fprintf(OUT, "    %s __parent;\n", parent->name);
    else fprintf(OUT, "    OboeObject __base;\n");
    for (FieldDecl *fd = c->fields; fd; fd = fd->next) {
        if (fd->is_static) continue;
        fprintf(OUT, "    OboeValue %s;\n", fd->name);
    }
    fprintf(OUT, "};\n");
    fprintf(OUT, "static const OboeClassInfo %s__classinfo = { \"%s\", %s };\n\n",
            c->name, c->name, parent ? fmt("&%s__classinfo", parent->name) : "NULL");
    g_class_emitted[idx] = true;
}

static void gen_class(ClassDecl *c) {
    /* static fields as globals */
    for (FieldDecl *fd = c->fields; fd; fd = fd->next) {
        if (fd->is_static) fprintf(OUT, "static OboeValue %s__%s = {0};\n", c->name, fd->name);
    }

    /* constructors */
    int init_count = 0;
    for (int i = 0; i < c->method_count; i++) if (strcmp(c->methods[i]->name, "init") == 0) init_count++;

    if (init_count == 0) {
        fprintf(OUT, "OboeValue %s__new_default(void) {\n", c->name);
        fprintf(OUT, "    %s *obj = calloc(1, sizeof(%s));\n", c->name, c->name);
        fprintf(OUT, "    ((OboeObject*)obj)->cls = &%s__classinfo;\n", c->name);
        fprintf(OUT, "    return ob_object_wrap(obj);\n");
        fprintf(OUT, "}\n\n");
    }

    int idx = 0;
    push_scope();
    for (int i = 0; i < c->method_count; i++) {
        FuncDecl *m = c->methods[i];
        if (strcmp(m->name, "init") != 0) continue;
        fprintf(OUT, "void %s__init_%d(%s *this", c->name, idx, c->name);
        for (Param *p = m->params; p; p = p->next) {
            if (strcmp(p->name, "this") == 0) continue;
            fprintf(OUT, ", OboeValue %s", p->name);
        }
        fprintf(OUT, ") {\n");
        push_scope();
        define_var("this", c->name);
        for (Param *p = m->params; p; p = p->next) {
            if (strcmp(p->name, "this") == 0) continue;
            const char *pc = (p->type_name && find_class(p->type_name)) ? p->type_name : NULL;
            define_var(p->name, pc);
        }
        const char *saved_class = current_class;
        current_class = c->name;
        gen_stmt_list(m->body, m->body_count, 4);
        current_class = saved_class;
        pop_scope();
        fprintf(OUT, "}\n\n");

        fprintf(OUT, "OboeValue %s__new_%d(", c->name, idx);
        bool first = true;
        for (Param *p = m->params; p; p = p->next) {
            if (strcmp(p->name, "this") == 0) continue;
            if (!first) fprintf(OUT, ", ");
            fprintf(OUT, "OboeValue %s", p->name);
            first = false;
        }
        fprintf(OUT, ") {\n");
        fprintf(OUT, "    %s *obj = calloc(1, sizeof(%s));\n", c->name, c->name);
        fprintf(OUT, "    ((OboeObject*)obj)->cls = &%s__classinfo;\n", c->name);
        fprintf(OUT, "    %s__init_%d(obj", c->name, idx);
        for (Param *p = m->params; p; p = p->next) {
            if (strcmp(p->name, "this") == 0) continue;
            fprintf(OUT, ", %s", p->name);
        }
        fprintf(OUT, ");\n");
        fprintf(OUT, "    return ob_object_wrap(obj);\n");
        fprintf(OUT, "}\n\n");
        idx++;
    }
    pop_scope();

    /* operator overloads: emitted as (self, rhs) pairs matching OboeOpFunc so
       they can be registered with the runtime dispatch table */
    for (int i = 0; i < c->method_count; i++) {
        FuncDecl *m = c->methods[i];
        if (!m->op_symbol) continue;
        const char *cfunc = NULL;
        for (int j = 0; j < g_class_op_count; j++) if (g_class_ops[j].decl == m) cfunc = g_class_ops[j].cfunc;
        Param *self = m->params;
        if (!self || strcmp(self->name, "this") != 0 || !self->next || self->next->next)
            codegen_error(m->line, "a class operator must take exactly (this, other)");
        Param *rhs = self->next;
        fprintf(OUT, "static OboeValue %s(OboeValue __self, OboeValue %s) {\n", cfunc, rhs->name);
        fprintf(OUT, "    %s *this = (%s*)__self.as.obj;\n", c->name, c->name);
        fprintf(OUT, "    (void)this;\n");
        push_scope();
        define_var("this", c->name);
        define_var(rhs->name, (rhs->type_name && find_class(rhs->type_name)) ? rhs->type_name : NULL);
        const char *saved_class = current_class;
        current_class = c->name;
        gen_stmt_list(m->body, m->body_count, 4);
        current_class = saved_class;
        pop_scope();
        fprintf(OUT, "    return ob_null();\n");
        fprintf(OUT, "}\n\n");
    }

    /* regular methods */
    for (int i = 0; i < c->method_count; i++) {
        FuncDecl *m = c->methods[i];
        if (strcmp(m->name, "init") == 0 || m->op_symbol) continue;
        if (m->is_static) {
            fprintf(OUT, "OboeValue %s__%s(", c->name, m->name);
            bool first = true;
            for (Param *p = m->params; p; p = p->next) {
                if (!first) fprintf(OUT, ", ");
                fprintf(OUT, "OboeValue %s", p->name);
                first = false;
            }
            fprintf(OUT, ") {\n");
            push_scope();
            for (Param *p = m->params; p; p = p->next) {
                const char *pc = (p->type_name && find_class(p->type_name)) ? p->type_name : NULL;
                define_var(p->name, pc);
            }
            const char *saved_class = current_class;
            current_class = c->name;
            gen_stmt_list(m->body, m->body_count, 4);
            current_class = saved_class;
            pop_scope();
            fprintf(OUT, "    return ob_null();\n");
            fprintf(OUT, "}\n\n");
        } else {
            gen_func_def(c->name, c, m);
        }
    }
}

/* ============================= program ============================= */

/* Instance fields aren't declared up front; they come into existence the first
   time a method assigns `this.field = value`. This walks every method body of a
   class looking for such assignments and appends a FieldDecl for any name not
   already present on the class or an ancestor (a child assigning an inherited
   field must reuse the ancestor's slot, not shadow it with its own).
   `type_hint` carries the assigned value's class when it is statically known
   (a class-typed parameter or a constructor call), so chained member access
   like `u.address.city` can be resolved at compile time. */
static FuncDecl *g_scan_method = NULL; /* method whose body is being scanned, for param types */

static void note_field(ClassDecl *c, const char *name, const char *type_hint) {
    FieldDecl *existing;
    if (find_field_owner(c, name, &existing)) {
        if (!existing->type_name && type_hint) existing->type_name = strdup(type_hint);
        return;
    }
    FieldDecl *fd = calloc(1, sizeof(FieldDecl));
    fd->name = strdup(name);
    fd->type_name = type_hint ? strdup(type_hint) : NULL;
    fd->next = c->fields;
    c->fields = fd;
}

static const char *field_type_hint(Expr *value) {
    if (!value) return NULL;
    if (value->kind == EXPR_IDENT && g_scan_method) {
        for (Param *p = g_scan_method->params; p; p = p->next) {
            if (strcmp(p->name, value->as.ident) == 0)
                return (p->type_name && find_class(p->type_name)) ? p->type_name : NULL;
        }
    }
    if (value->kind == EXPR_CALL && value->as.call.callee->kind == EXPR_IDENT &&
        find_class(value->as.call.callee->as.ident))
        return value->as.call.callee->as.ident;
    return NULL;
}

static void scan_expr_for_fields(ClassDecl *c, Expr *e) {
    if (!e) return;
    switch (e->kind) {
        case EXPR_ASSIGN:
            if (e->as.assign.target->kind == EXPR_FIELD &&
                e->as.assign.target->as.field.obj->kind == EXPR_IDENT &&
                strcmp(e->as.assign.target->as.field.obj->as.ident, "this") == 0) {
                note_field(c, e->as.assign.target->as.field.name, field_type_hint(e->as.assign.value));
            }
            scan_expr_for_fields(c, e->as.assign.target);
            scan_expr_for_fields(c, e->as.assign.value);
            break;
        case EXPR_BINARY: scan_expr_for_fields(c, e->as.binary.l); scan_expr_for_fields(c, e->as.binary.r); break;
        case EXPR_UNARY: scan_expr_for_fields(c, e->as.unary.operand); break;
        case EXPR_IS: scan_expr_for_fields(c, e->as.is_check.value); break;
        case EXPR_INDEX: scan_expr_for_fields(c, e->as.index.arr); scan_expr_for_fields(c, e->as.index.idx); break;
        case EXPR_FIELD: case EXPR_SAFE_FIELD: scan_expr_for_fields(c, e->as.field.obj); break;
        case EXPR_CALL:
            scan_expr_for_fields(c, e->as.call.callee);
            for (int i = 0; i < e->as.call.arg_count; i++) scan_expr_for_fields(c, e->as.call.args[i]);
            break;
        case EXPR_ARRAY:
            for (int i = 0; i < e->as.array_lit.count; i++) scan_expr_for_fields(c, e->as.array_lit.items[i]);
            break;
        case EXPR_DICT:
            for (int i = 0; i < e->as.dict_lit.count; i++) {
                scan_expr_for_fields(c, e->as.dict_lit.keys[i]);
                scan_expr_for_fields(c, e->as.dict_lit.values[i]);
            }
            break;
        case EXPR_STRING:
            for (StringPart *p = e->as.str_parts; p; p = p->next) if (p->is_expr) scan_expr_for_fields(c, p->expr);
            break;
        default: break;
    }
}

static void scan_stmt_list_for_fields(ClassDecl *c, Stmt **body, int count) {
    for (int i = 0; i < count; i++) {
        Stmt *s = body[i];
        switch (s->kind) {
            case STMT_LET: scan_expr_for_fields(c, s->as.let.init); break;
            case STMT_EXPR: scan_expr_for_fields(c, s->as.expr_stmt.expr); break;
            case STMT_RETURN: scan_expr_for_fields(c, s->as.ret.value); break;
            case STMT_IF:
                scan_expr_for_fields(c, s->as.if_stmt.cond);
                scan_stmt_list_for_fields(c, s->as.if_stmt.then_body, s->as.if_stmt.then_count);
                scan_stmt_list_for_fields(c, s->as.if_stmt.else_body, s->as.if_stmt.else_count);
                break;
            case STMT_WHILE:
                scan_expr_for_fields(c, s->as.while_stmt.cond);
                scan_stmt_list_for_fields(c, s->as.while_stmt.body, s->as.while_stmt.body_count);
                break;
            case STMT_FOR:
                scan_expr_for_fields(c, s->as.for_stmt.range_a);
                scan_expr_for_fields(c, s->as.for_stmt.range_b);
                scan_expr_for_fields(c, s->as.for_stmt.iterable);
                scan_stmt_list_for_fields(c, s->as.for_stmt.body, s->as.for_stmt.body_count);
                break;
            case STMT_SWITCH:
                scan_expr_for_fields(c, s->as.switch_stmt.subject);
                for (CaseClause *cc = s->as.switch_stmt.cases; cc; cc = cc->next)
                    scan_stmt_list_for_fields(c, cc->body, cc->body_count);
                break;
            case STMT_TRY:
                scan_stmt_list_for_fields(c, s->as.try_stmt.body, s->as.try_stmt.body_count);
                for (CatchClause *cc = s->as.try_stmt.catches; cc; cc = cc->next)
                    scan_stmt_list_for_fields(c, cc->body, cc->body_count);
                scan_stmt_list_for_fields(c, s->as.try_stmt.finally_body, s->as.try_stmt.finally_count);
                break;
            case STMT_THROW: scan_expr_for_fields(c, s->as.throw_stmt.value); break;
            case STMT_BLOCK: scan_stmt_list_for_fields(c, s->as.block.body, s->as.block.body_count); break;
        }
    }
}

static ClassDecl **g_inferred = NULL;
static int g_inferred_count = 0;

/* Parents must be inferred before children so a child assigning an inherited
   field finds the ancestor's slot instead of creating a shadowing one. */
static void infer_instance_fields(ClassDecl *c) {
    for (int i = 0; i < g_inferred_count; i++) if (g_inferred[i] == c) return;
    g_inferred = realloc(g_inferred, (g_inferred_count + 1) * sizeof(ClassDecl *));
    g_inferred[g_inferred_count++] = c;

    ClassDecl *parent = c->parent_name ? find_class(c->parent_name) : NULL;
    if (parent) infer_instance_fields(parent);

    for (int i = 0; i < c->method_count; i++) {
        g_scan_method = c->methods[i];
        scan_stmt_list_for_fields(c, c->methods[i]->body, c->methods[i]->body_count);
    }
    g_scan_method = NULL;
}

static void add_class(ClassDecl *c) {
    g_classes = realloc(g_classes, (g_class_count + 1) * sizeof(ClassDecl *));
    g_class_emitted = realloc(g_class_emitted, (g_class_count + 1) * sizeof(bool));
    g_class_emitted[g_class_count] = false;
    g_classes[g_class_count++] = c;

    /* register the class's operator overloads so expressions can dispatch on
       them even before the definitions are emitted */
    for (int i = 0; i < c->method_count; i++) {
        FuncDecl *m = c->methods[i];
        if (!m->op_symbol) continue;
        g_class_ops = realloc(g_class_ops, (g_class_op_count + 1) * sizeof(ClassOp));
        g_class_ops[g_class_op_count].cls = c;
        g_class_ops[g_class_op_count].symbol = strdup(m->op_symbol);
        g_class_ops[g_class_op_count].cfunc = fmt("%s__opov_%d", c->name, g_class_op_count);
        g_class_ops[g_class_op_count].decl = m;
        g_class_op_count++;
    }
}

static void collect_classes(Decl *decls) {
    for (Decl *d = decls; d; d = d->next) {
        if (d->kind == DECL_CLASS) add_class(d->as.klass);
    }
}

static char *read_whole_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1);
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);
    return buf;
}

static void process_import(ImportDecl *imp) {
    const char *local_name = imp->alias ? imp->alias : imp->module;

    if (imp->member_count > 0) {
        for (int i = 0; i < imp->member_count; i++) {
            g_import_directs = realloc(g_import_directs, (g_import_direct_count + 1) * sizeof(ImportBinding));
            g_import_directs[g_import_direct_count].local_name = strdup(imp->members[i]);
            g_import_directs[g_import_direct_count].module = strdup(imp->module);
            g_import_direct_count++;
        }
    } else {
        g_import_aliases = realloc(g_import_aliases, (g_import_alias_count + 1) * sizeof(ImportBinding));
        g_import_aliases[g_import_alias_count].local_name = strdup(local_name);
        g_import_aliases[g_import_alias_count].module = strdup(imp->module);
        g_import_alias_count++;
    }

    for (int i = 0; i < g_processed_module_count; i++)
        if (strcmp(g_processed_modules[i], imp->module) == 0) return;
    g_processed_modules = realloc(g_processed_modules, (g_processed_module_count + 1) * sizeof(char *));
    g_processed_modules[g_processed_module_count++] = strdup(imp->module);

    char path1[4096], path2[4096];
    snprintf(path1, sizeof path1, "%s/%s.oboe", g_source_dir, imp->module);
    snprintf(path2, sizeof path2, "%s/.oboe/libraries/%s.oboe", g_source_dir, imp->module);
    char *src = read_whole_file(path1);
    if (!src) src = read_whole_file(path2);
    if (!src) {
        fprintf(stderr, "oboe: cannot find module '%s' (looked for %s and %s)\n", imp->module, path1, path2);
        exit(1);
    }
    int tok_count;
    Token *toks = lex_all(src, &tok_count);
    Program *modprog = parse_program(toks, tok_count, imp->module);
    free(src);

    collect_classes(modprog->decls);
    for (Decl *d = modprog->decls; d; d = d->next) {
        if (d->kind == DECL_EVENT || d->kind == DECL_ON || d->kind == DECL_OPERATOR || d->kind == DECL_CIMPORT)
            codegen_error(0, "events, top-level operators and cimport are not yet supported inside imported modules");
        if (d->kind == DECL_IMPORT) process_import(&d->as.import);
    }
    for (Decl *d = modprog->decls; d; d = d->next) {
        if (d->kind == DECL_CLASS) infer_instance_fields(d->as.klass);
    }
    for (Decl *d = modprog->decls; d; d = d->next) {
        if (d->kind == DECL_CLASS) {
            int idx = -1;
            for (int i = 0; i < g_class_count; i++) if (g_classes[i] == d->as.klass) idx = i;
            emit_class_struct(d->as.klass, idx);
        }
    }
    char *mod_prefix = fmt("%s__", imp->module);
    for (Decl *d = modprog->decls; d; d = d->next) {
        if (d->kind == DECL_CLASS) emit_class_predecls(d->as.klass);
        else if (d->kind == DECL_FUNC) emit_func_prototype(mod_prefix, NULL, d->as.func);
    }
    fprintf(OUT, "\n");
    for (Decl *d = modprog->decls; d; d = d->next) {
        if (d->kind == DECL_CLASS) gen_class(d->as.klass);
        else if (d->kind == DECL_FUNC) gen_func_def(mod_prefix, NULL, d->as.func);
    }
    free(mod_prefix);
}

void codegen_set_source_dir(const char *dir) {
    g_source_dir = strdup(dir);
}

/* ============================= events / operators / FFI ============================= */

/* Each event gets a synthesized class whose fields are the payload params;
   `on E as e` handlers bind `e` to that class so `e.field` resolves like any
   member access, and `E.fire(...)` resolves like a static call to E__fire. */
static void register_event_decl(EventDecl *ev) {
    ClassDecl *c = calloc(1, sizeof(ClassDecl));
    c->name = strdup(ev->name);
    FieldDecl head = {0};
    FieldDecl *tail = &head;
    for (Param *p = ev->params; p; p = p->next) {
        FieldDecl *fd = calloc(1, sizeof(FieldDecl));
        fd->name = strdup(p->name);
        fd->type_name = p->type_name ? strdup(p->type_name) : NULL;
        tail->next = fd;
        tail = fd;
    }
    c->fields = head.next;
    add_class(c);
    g_events = realloc(g_events, (g_event_count + 1) * sizeof(EventInfo));
    g_events[g_event_count].decl = ev;
    g_events[g_event_count].cls = c;
    g_event_count++;
}

static void collect_extras(Decl *decls) {
    for (Decl *d = decls; d; d = d->next) {
        switch (d->kind) {
            case DECL_OPERATOR: {
                FuncDecl *f = d->as.func;
                int pc = 0;
                for (Param *p = f->params; p; p = p->next) pc++;
                if (pc != 2) codegen_error(f->line, "a top-level operator must take exactly two parameters");
                if (find_user_op(f->op_symbol)) codegen_error(f->line, "operator is already defined");
                g_user_ops = realloc(g_user_ops, (g_user_op_count + 1) * sizeof(UserOp));
                g_user_ops[g_user_op_count].symbol = strdup(f->op_symbol);
                g_user_ops[g_user_op_count].cfunc = fmt("__oboe_userop_%d", g_user_op_count);
                g_user_ops[g_user_op_count].decl = f;
                g_user_op_count++;
                break;
            }
            case DECL_EVENT:
                if (find_event(d->as.event.name) || find_class(d->as.event.name))
                    codegen_error(d->as.event.line, "an event or class with this name already exists");
                register_event_decl(&d->as.event);
                break;
            case DECL_ON:
                g_handlers = realloc(g_handlers, (g_handler_count + 1) * sizeof(HandlerInfo));
                g_handlers[g_handler_count].decl = &d->as.on;
                g_handlers[g_handler_count].cfunc = fmt("__on_%s_%d", d->as.on.event_name, g_handler_count);
                g_handler_count++;
                break;
            case DECL_CIMPORT:
                g_ffi = realloc(g_ffi, (g_ffi_count + 1) * sizeof(FfiBinding));
                g_ffi[g_ffi_count].name = strdup(d->as.cimport.name);
                g_ffi[g_ffi_count].lib = strdup(d->as.cimport.lib);
                g_ffi_count++;
                break;
            default: break;
        }
    }
}

/* The built-in KeyboardInterruptEvent exists without a declaration; synthesize
   it when a handler references it. Any other unknown event is an error. */
static void finalize_events(void) {
    for (int i = 0; i < g_handler_count; i++) {
        OnDecl *h = g_handlers[i].decl;
        if (find_event(h->event_name)) continue;
        if (strcmp(h->event_name, "KeyboardInterruptEvent") == 0) {
            EventDecl *ev = calloc(1, sizeof(EventDecl));
            ev->name = strdup("KeyboardInterruptEvent");
            register_event_decl(ev);
        } else {
            codegen_error(h->line, "'on' handler references an undeclared event");
        }
    }
}

static bool has_kbint_handlers(void) {
    for (int i = 0; i < g_handler_count; i++)
        if (strcmp(g_handlers[i].decl->event_name, "KeyboardInterruptEvent") == 0) return true;
    return false;
}

static void emit_extras_predecls(void) {
    for (int i = 0; i < g_user_op_count; i++)
        fprintf(OUT, "static OboeValue %s(OboeValue, OboeValue);\n", g_user_ops[i].cfunc);
    for (int i = 0; i < g_handler_count; i++)
        fprintf(OUT, "static OboeValue %s(OboeValue __ev);\n", g_handlers[i].cfunc);
    for (int i = 0; i < g_event_count; i++) {
        fprintf(OUT, "OboeValue %s__fire(", g_events[i].decl->name);
        bool first = true;
        for (Param *p = g_events[i].decl->params; p; p = p->next) {
            if (!first) fprintf(OUT, ", ");
            fprintf(OUT, "OboeValue %s", p->name);
            first = false;
        }
        if (first) fprintf(OUT, "void");
        fprintf(OUT, ");\n");
    }
    for (int i = 0; i < g_ffi_count; i++)
        fprintf(OUT, "static void *__ffi_%s;\n", g_ffi[i].name);
}

static void emit_extras_defs(void) {
    /* top-level custom operators */
    for (int i = 0; i < g_user_op_count; i++) {
        FuncDecl *f = g_user_ops[i].decl;
        Param *a = f->params, *b = f->params->next;
        fprintf(OUT, "static OboeValue %s(OboeValue %s, OboeValue %s) {\n", g_user_ops[i].cfunc, a->name, b->name);
        push_scope();
        define_var(a->name, (a->type_name && find_class(a->type_name)) ? a->type_name : NULL);
        define_var(b->name, (b->type_name && find_class(b->type_name)) ? b->type_name : NULL);
        gen_stmt_list(f->body, f->body_count, 4);
        pop_scope();
        fprintf(OUT, "    return ob_null();\n");
        fprintf(OUT, "}\n\n");
    }

    /* event handlers, in declaration order */
    for (int i = 0; i < g_handler_count; i++) {
        OnDecl *h = g_handlers[i].decl;
        fprintf(OUT, "static OboeValue %s(OboeValue __ev) {\n", g_handlers[i].cfunc);
        fprintf(OUT, "    (void)__ev;\n");
        push_scope();
        if (h->var_name) {
            fprintf(OUT, "    OboeValue %s = __ev;\n", h->var_name);
            define_var(h->var_name, h->event_name);
        }
        gen_stmt_list(h->body, h->body_count, 4);
        pop_scope();
        fprintf(OUT, "    return ob_null();\n");
        fprintf(OUT, "}\n\n");
    }

    /* fire functions: build the payload object, then invoke every handler */
    for (int i = 0; i < g_event_count; i++) {
        EventDecl *ev = g_events[i].decl;
        fprintf(OUT, "OboeValue %s__fire(", ev->name);
        bool first = true;
        for (Param *p = ev->params; p; p = p->next) {
            if (!first) fprintf(OUT, ", ");
            fprintf(OUT, "OboeValue %s", p->name);
            first = false;
        }
        if (first) fprintf(OUT, "void");
        fprintf(OUT, ") {\n");
        fprintf(OUT, "    %s *obj = calloc(1, sizeof(%s));\n", ev->name, ev->name);
        fprintf(OUT, "    ((OboeObject*)obj)->cls = &%s__classinfo;\n", ev->name);
        for (Param *p = ev->params; p; p = p->next)
            fprintf(OUT, "    obj->%s = %s;\n", p->name, p->name);
        fprintf(OUT, "    OboeValue __ev = ob_object_wrap(obj);\n");
        fprintf(OUT, "    (void)__ev;\n");
        for (int j = 0; j < g_handler_count; j++)
            if (strcmp(g_handlers[j].decl->event_name, ev->name) == 0)
                fprintf(OUT, "    %s(__ev);\n", g_handlers[j].cfunc);
        fprintf(OUT, "    return ob_null();\n");
        fprintf(OUT, "}\n\n");
    }

    if (has_kbint_handlers())
        fprintf(OUT, "static void __oboe_fire_kbint(void) { KeyboardInterruptEvent__fire(); }\n\n");
}

void codegen_program(Program *prog, FILE *out) {
    OUT = out;
    collect_classes(prog->decls);
    collect_extras(prog->decls);
    finalize_events();

    fprintf(out, "#include \"oboe_runtime.h\"\n#include <stdlib.h>\n#include <stdio.h>\n\n");

    int main_class_count = g_class_count;
    for (int i = 0; i < main_class_count; i++) infer_instance_fields(g_classes[i]);

    for (Decl *d = prog->decls; d; d = d->next) {
        if (d->kind == DECL_IMPORT) process_import(&d->as.import);
    }

    for (int i = 0; i < g_class_count; i++) emit_class_struct(g_classes[i], i);

    for (Decl *d = prog->decls; d; d = d->next) {
        if (d->kind == DECL_CLASS) emit_class_predecls(d->as.klass);
        else if (d->kind == DECL_FUNC) emit_func_prototype(NULL, NULL, d->as.func);
    }
    emit_extras_predecls();
    fprintf(out, "\n");

    bool has_main = false;
    for (Decl *d = prog->decls; d; d = d->next) {
        if (d->kind == DECL_FUNC && strcmp(d->as.func->name, "main") == 0) has_main = true;
    }

    push_scope();
    for (Decl *d = prog->decls; d; d = d->next) {
        if (d->kind == DECL_CLASS) gen_class(d->as.klass);
        else if (d->kind == DECL_FUNC) gen_func_def(NULL, NULL, d->as.func);
    }
    emit_extras_defs();

    /* static field initializers, operator registrations, FFI resolution */
    fprintf(out, "static void __oboe_static_init(void) {\n");
    for (int i = 0; i < g_class_op_count; i++)
        fprintf(out, "    ob_register_operator(&%s__classinfo, \"%s\", %s);\n",
                g_class_ops[i].cls->name, g_class_ops[i].symbol, g_class_ops[i].cfunc);
    for (int i = 0; i < g_ffi_count; i++)
        fprintf(out, "    __ffi_%s = ob_ffi_sym(\"%s\", \"%s\");\n", g_ffi[i].name, g_ffi[i].lib, g_ffi[i].name);
    for (int i = 0; i < g_class_count; i++) {
        ClassDecl *c = g_classes[i];
        const char *saved_class = current_class;
        current_class = c->name;
        for (FieldDecl *fd = c->fields; fd; fd = fd->next) {
            if (!fd->is_static) continue;
            char *v = fd->init ? gen_expr(fd->init) : strdup("ob_null()");
            fprintf(out, "    %s__%s = %s;\n", c->name, fd->name, v);
            free(v);
        }
        current_class = saved_class;
    }
    fprintf(out, "}\n\n");

    /* top-level statements (single-file script mode) */
    fprintf(out, "static void __oboe_toplevel(void) {\n");
    for (Decl *d = prog->decls; d; d = d->next) {
        if (d->kind == DECL_STMT) gen_stmt(d->as.stmt, 4);
    }
    fprintf(out, "}\n\n");
    pop_scope();

    fprintf(out, "int main(int argc, char **argv) {\n");
    fprintf(out, "    __oboe_static_init();\n");
    if (has_kbint_handlers()) fprintf(out, "    ob_install_sigint(__oboe_fire_kbint);\n");
    fprintf(out, "    __oboe_toplevel();\n");
    if (has_main) fprintf(out, "    oboe_user_main(ob_args_from_argv(argc, argv));\n");
    fprintf(out, "    return 0;\n");
    fprintf(out, "}\n");
}
