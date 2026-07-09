#include "oboe_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

OboeExceptionFrame *ob_exc_stack = NULL;
OboeValue ob_current_exception;
char *ob_current_exception_type = NULL;

static void ob_oom(void) {
    fprintf(stderr, "oboe: out of memory\n");
    exit(1);
}

OboeValue ob_int(int64_t v) { OboeValue r; r.tag = OB_INT; r.as.i = v; return r; }
OboeValue ob_bool(bool v) { OboeValue r; r.tag = OB_BOOL; r.as.b = v; return r; }
OboeValue ob_null(void) { OboeValue r; r.tag = OB_NULL; r.as.i = 0; return r; }

OboeValue ob_string(const char *v) {
    OboeValue r;
    r.tag = OB_STRING;
    r.as.s = strdup(v ? v : "");
    if (!r.as.s) ob_oom();
    return r;
}

OboeValue ob_string_take(char *v) {
    OboeValue r;
    r.tag = OB_STRING;
    r.as.s = v;
    return r;
}

OboeValue ob_array_new(void) {
    OboeValue r;
    r.tag = OB_ARRAY;
    r.as.arr = malloc(sizeof(OboeArray));
    if (!r.as.arr) ob_oom();
    r.as.arr->items = NULL;
    r.as.arr->count = 0;
    r.as.arr->capacity = 0;
    return r;
}

OboeValue ob_dict_new(void) {
    OboeValue r;
    r.tag = OB_DICT;
    r.as.dict = malloc(sizeof(OboeDict));
    if (!r.as.dict) ob_oom();
    r.as.dict->entries = NULL;
    r.as.dict->count = 0;
    r.as.dict->capacity = 0;
    return r;
}

OboeValue ob_object_wrap(void *obj) {
    OboeValue r;
    r.tag = OB_OBJECT;
    r.as.obj = obj;
    return r;
}

void ob_array_push(OboeValue arr, OboeValue v) {
    OboeArray *a = arr.as.arr;
    if (a->count == a->capacity) {
        a->capacity = a->capacity ? a->capacity * 2 : 4;
        a->items = realloc(a->items, a->capacity * sizeof(OboeValue));
        if (!a->items) ob_oom();
    }
    a->items[a->count++] = v;
}

static void ob_bounds_check(OboeArray *a, int64_t idx) {
    if (idx < 0 || (size_t)idx >= a->count) {
        fprintf(stderr, "oboe: array index %lld out of bounds (len %zu)\n", (long long)idx, a->count);
        exit(1);
    }
}

OboeValue ob_array_get(OboeValue arr, int64_t idx) {
    OboeArray *a = arr.as.arr;
    ob_bounds_check(a, idx);
    return a->items[idx];
}

void ob_array_set(OboeValue arr, int64_t idx, OboeValue v) {
    OboeArray *a = arr.as.arr;
    ob_bounds_check(a, idx);
    a->items[idx] = v;
}

int64_t ob_array_len(OboeValue arr) { return (int64_t)arr.as.arr->count; }

void ob_dict_set(OboeValue d, const char *key, OboeValue v) {
    OboeDict *dict = d.as.dict;
    for (size_t i = 0; i < dict->count; i++) {
        if (strcmp(dict->entries[i].key, key) == 0) {
            dict->entries[i].value = v;
            return;
        }
    }
    if (dict->count == dict->capacity) {
        dict->capacity = dict->capacity ? dict->capacity * 2 : 4;
        dict->entries = realloc(dict->entries, dict->capacity * sizeof(OboeDictEntry));
        if (!dict->entries) ob_oom();
    }
    dict->entries[dict->count].key = strdup(key);
    dict->entries[dict->count].value = v;
    dict->count++;
}

OboeValue ob_dict_get(OboeValue d, const char *key) {
    OboeDict *dict = d.as.dict;
    for (size_t i = 0; i < dict->count; i++) {
        if (strcmp(dict->entries[i].key, key) == 0) return dict->entries[i].value;
    }
    return ob_null();
}

bool ob_dict_has(OboeValue d, const char *key) {
    OboeDict *dict = d.as.dict;
    for (size_t i = 0; i < dict->count; i++) {
        if (strcmp(dict->entries[i].key, key) == 0) return true;
    }
    return false;
}

OboeValue ob_index_get(OboeValue container, OboeValue key) {
    if (container.tag == OB_DICT) {
        char *k = ob_to_cstr(key);
        OboeValue r = ob_dict_get(container, k);
        free(k);
        return r;
    }
    if (container.tag == OB_ARRAY) return ob_array_get(container, key.as.i);
    fprintf(stderr, "oboe: cannot index into this value\n");
    exit(1);
}

OboeValue ob_index_set(OboeValue container, OboeValue key, OboeValue value) {
    if (container.tag == OB_DICT) {
        char *k = ob_to_cstr(key);
        ob_dict_set(container, k, value);
        free(k);
        return value;
    }
    if (container.tag == OB_ARRAY) { ob_array_set(container, key.as.i, value); return value; }
    fprintf(stderr, "oboe: cannot index into this value\n");
    exit(1);
}

char *ob_to_cstr(OboeValue v) {
    char buf[64];
    switch (v.tag) {
        case OB_NULL: return strdup("null");
        case OB_INT: snprintf(buf, sizeof buf, "%lld", (long long)v.as.i); return strdup(buf);
        case OB_BOOL: return strdup(v.as.b ? "true" : "false");
        case OB_STRING: return strdup(v.as.s);
        case OB_ARRAY: {
            OboeArray *a = v.as.arr;
            size_t cap = 64, len = 0;
            char *out = malloc(cap);
            out[0] = '\0';
            len = 1;
            strcpy(out, "[");
            len = 1;
            for (size_t i = 0; i < a->count; i++) {
                char *piece = ob_to_cstr(a->items[i]);
                size_t plen = strlen(piece);
                if (len + plen + 4 > cap) {
                    cap = (len + plen + 4) * 2;
                    out = realloc(out, cap);
                }
                if (i > 0) { strcat(out, ", "); len += 2; }
                strcat(out, piece);
                len += plen;
                free(piece);
            }
            strcat(out, "]");
            return out;
        }
        case OB_DICT: {
            OboeDict *d = v.as.dict;
            size_t cap = 64, len = 1;
            char *out = malloc(cap);
            strcpy(out, "{");
            for (size_t i = 0; i < d->count; i++) {
                char *piece = ob_to_cstr(d->entries[i].value);
                size_t need = strlen(d->entries[i].key) + strlen(piece) + 8;
                if (len + need > cap) { cap = (len + need) * 2; out = realloc(out, cap); }
                if (i > 0) strcat(out, ", ");
                strcat(out, d->entries[i].key);
                strcat(out, ": ");
                strcat(out, piece);
                len += need;
                free(piece);
            }
            strcat(out, "}");
            return out;
        }
        case OB_OBJECT: {
            OboeObject *o = v.as.obj;
            const char *name = o->cls ? o->cls->name : "object";
            char *out = malloc(strlen(name) + 16);
            sprintf(out, "<%s instance>", name);
            return out;
        }
    }
    return strdup("");
}

void ob_print(OboeValue v) {
    char *s = ob_to_cstr(v);
    printf("%s\n", s);
    free(s);
}

OboeValue ob_str(OboeValue v) {
    if (v.tag == OB_STRING) return ob_string(v.as.s);
    char *s = ob_to_cstr(v);
    return ob_string_take(s);
}

OboeValue ob_interpolate(int count, ...) {
    va_list ap;
    va_start(ap, count);
    size_t total = 1;
    char **parts = malloc(sizeof(char *) * count);
    for (int i = 0; i < count; i++) {
        OboeValue v = va_arg(ap, OboeValue);
        parts[i] = ob_to_cstr(v);
        total += strlen(parts[i]);
    }
    va_end(ap);
    char *out = malloc(total);
    out[0] = '\0';
    for (int i = 0; i < count; i++) {
        strcat(out, parts[i]);
        free(parts[i]);
    }
    free(parts);
    return ob_string_take(out);
}

#define OB_MAX_OPERATORS 256
typedef struct { const OboeClassInfo *cls; char op[4]; OboeOpFunc fn; } OboeOpEntry;
static OboeOpEntry ob_op_table[OB_MAX_OPERATORS];
static int ob_op_count = 0;

void ob_register_operator(const OboeClassInfo *cls, const char *op, OboeOpFunc fn) {
    if (ob_op_count >= OB_MAX_OPERATORS) { fprintf(stderr, "oboe: too many operator overloads\n"); exit(1); }
    ob_op_table[ob_op_count].cls = cls;
    strncpy(ob_op_table[ob_op_count].op, op, 3);
    ob_op_table[ob_op_count].op[3] = '\0';
    ob_op_table[ob_op_count].fn = fn;
    ob_op_count++;
}

static OboeOpFunc ob_find_operator(const OboeClassInfo *cls, const char *op) {
    while (cls) {
        for (int i = 0; i < ob_op_count; i++) {
            if (ob_op_table[i].cls == cls && strcmp(ob_op_table[i].op, op) == 0) return ob_op_table[i].fn;
        }
        cls = cls->parent;
    }
    return NULL;
}

OboeValue ob_binop(const char *op, OboeValue a, OboeValue b, OboeOpFunc fallback) {
    if (a.tag == OB_OBJECT) {
        OboeOpFunc fn = ob_find_operator(((OboeObject *)a.as.obj)->cls, op);
        if (fn) return fn(a, b);
    }
    return fallback(a, b);
}

static void ob_type_error(const char *op) {
    fprintf(stderr, "oboe: type error in operator %s\n", op);
    exit(1);
}

OboeValue ob_add(OboeValue a, OboeValue b) {
    if (a.tag == OB_STRING || b.tag == OB_STRING) {
        char *as = ob_to_cstr(a);
        char *bs = ob_to_cstr(b);
        char *out = malloc(strlen(as) + strlen(bs) + 1);
        strcpy(out, as); strcat(out, bs);
        free(as); free(bs);
        return ob_string_take(out);
    }
    if (a.tag == OB_INT && b.tag == OB_INT) return ob_int(a.as.i + b.as.i);
    ob_type_error("+");
    return ob_null();
}

OboeValue ob_sub(OboeValue a, OboeValue b) {
    if (a.tag == OB_INT && b.tag == OB_INT) return ob_int(a.as.i - b.as.i);
    ob_type_error("-"); return ob_null();
}
OboeValue ob_mul(OboeValue a, OboeValue b) {
    if (a.tag == OB_INT && b.tag == OB_INT) return ob_int(a.as.i * b.as.i);
    ob_type_error("*"); return ob_null();
}
OboeValue ob_div(OboeValue a, OboeValue b) {
    if (a.tag == OB_INT && b.tag == OB_INT) {
        if (b.as.i == 0) { fprintf(stderr, "oboe: division by zero\n"); exit(1); }
        return ob_int(a.as.i / b.as.i);
    }
    ob_type_error("/"); return ob_null();
}
OboeValue ob_mod(OboeValue a, OboeValue b) {
    if (a.tag == OB_INT && b.tag == OB_INT) {
        if (b.as.i == 0) { fprintf(stderr, "oboe: modulo by zero\n"); exit(1); }
        return ob_int(a.as.i % b.as.i);
    }
    ob_type_error("%"); return ob_null();
}

static bool ob_raw_eq(OboeValue a, OboeValue b) {
    if (a.tag != b.tag) return false;
    switch (a.tag) {
        case OB_NULL: return true;
        case OB_INT: return a.as.i == b.as.i;
        case OB_BOOL: return a.as.b == b.as.b;
        case OB_STRING: return strcmp(a.as.s, b.as.s) == 0;
        case OB_OBJECT: return a.as.obj == b.as.obj;
        case OB_ARRAY: return a.as.arr == b.as.arr;
        case OB_DICT: return a.as.dict == b.as.dict;
    }
    return false;
}

OboeValue ob_eq(OboeValue a, OboeValue b) { return ob_bool(ob_raw_eq(a, b)); }
OboeValue ob_neq(OboeValue a, OboeValue b) { return ob_bool(!ob_raw_eq(a, b)); }

OboeValue ob_lt(OboeValue a, OboeValue b) {
    if (a.tag == OB_INT && b.tag == OB_INT) return ob_bool(a.as.i < b.as.i);
    if (a.tag == OB_STRING && b.tag == OB_STRING) return ob_bool(strcmp(a.as.s, b.as.s) < 0);
    ob_type_error("<"); return ob_null();
}
OboeValue ob_lte(OboeValue a, OboeValue b) {
    if (a.tag == OB_INT && b.tag == OB_INT) return ob_bool(a.as.i <= b.as.i);
    if (a.tag == OB_STRING && b.tag == OB_STRING) return ob_bool(strcmp(a.as.s, b.as.s) <= 0);
    ob_type_error("<="); return ob_null();
}
OboeValue ob_gt(OboeValue a, OboeValue b) {
    if (a.tag == OB_INT && b.tag == OB_INT) return ob_bool(a.as.i > b.as.i);
    if (a.tag == OB_STRING && b.tag == OB_STRING) return ob_bool(strcmp(a.as.s, b.as.s) > 0);
    ob_type_error(">"); return ob_null();
}
OboeValue ob_gte(OboeValue a, OboeValue b) {
    if (a.tag == OB_INT && b.tag == OB_INT) return ob_bool(a.as.i >= b.as.i);
    if (a.tag == OB_STRING && b.tag == OB_STRING) return ob_bool(strcmp(a.as.s, b.as.s) >= 0);
    ob_type_error(">="); return ob_null();
}

bool ob_truthy(OboeValue v) {
    switch (v.tag) {
        case OB_NULL: return false;
        case OB_BOOL: return v.as.b;
        case OB_INT: return v.as.i != 0;
        case OB_STRING: return v.as.s[0] != '\0';
        case OB_ARRAY: return v.as.arr->count != 0;
        case OB_DICT: return v.as.dict->count != 0;
        case OB_OBJECT: return true;
    }
    return false;
}

OboeValue ob_and(OboeValue a, OboeValue b) { return ob_bool(ob_truthy(a) && ob_truthy(b)); }
OboeValue ob_or(OboeValue a, OboeValue b) { return ob_bool(ob_truthy(a) || ob_truthy(b)); }
OboeValue ob_not(OboeValue a) { return ob_bool(!ob_truthy(a)); }
OboeValue ob_neg(OboeValue a) {
    if (a.tag == OB_INT) return ob_int(-a.as.i);
    ob_type_error("unary -"); return ob_null();
}

OboeValue ob_repeat(OboeValue a, OboeValue b) {
    if (a.tag == OB_STRING && b.tag == OB_INT) {
        int64_t n = b.as.i;
        if (n < 0) n = 0;
        size_t len = strlen(a.as.s);
        char *out = malloc(len * n + 1);
        out[0] = '\0';
        for (int64_t i = 0; i < n; i++) strcat(out, a.as.s);
        return ob_string_take(out);
    }
    ob_type_error("x"); return ob_null();
}

OboeValue ob_coalesce(OboeValue a, OboeValue b) {
    return ob_is_null(a) ? b : a;
}

bool ob_is_int(OboeValue v) { return v.tag == OB_INT; }
bool ob_is_bool(OboeValue v) { return v.tag == OB_BOOL; }
bool ob_is_string(OboeValue v) { return v.tag == OB_STRING; }
bool ob_is_array(OboeValue v) { return v.tag == OB_ARRAY; }
bool ob_is_dict(OboeValue v) { return v.tag == OB_DICT; }
bool ob_is_null(OboeValue v) { return v.tag == OB_NULL; }

bool ob_is_object_of(OboeValue v, const OboeClassInfo *cls) {
    if (v.tag != OB_OBJECT) return false;
    const OboeClassInfo *actual = ((OboeObject *)v.as.obj)->cls;
    while (actual) {
        if (actual == cls) return true;
        actual = actual->parent;
    }
    return false;
}

OboeValue ob_range(int64_t a, int64_t b) {
    OboeValue r = ob_array_new();
    for (int64_t i = a; i < b; i++) ob_array_push(r, ob_int(i));
    return r;
}

OboeValue ob_args_from_argv(int argc, char **argv) {
    OboeValue r = ob_array_new();
    for (int i = 0; i < argc; i++) ob_array_push(r, ob_string(argv[i]));
    return r;
}

void ob_throw(const char *type_name, OboeValue payload) {
    ob_current_exception_type = strdup(type_name);
    ob_current_exception = payload;
    if (!ob_exc_stack) {
        fprintf(stderr, "oboe: uncaught exception %s\n", type_name);
        exit(1);
    }
    OboeExceptionFrame *f = ob_exc_stack;
    ob_exc_stack = f->prev;
    longjmp(f->buf, 1);
}

bool ob_exception_matches(const char *type_name) {
    if (strcmp(type_name, "Exception") == 0) return true;
    return ob_current_exception_type && strcmp(ob_current_exception_type, type_name) == 0;
}
