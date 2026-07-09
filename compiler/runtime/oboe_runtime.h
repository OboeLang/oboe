#ifndef OBOE_RUNTIME_H
#define OBOE_RUNTIME_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <setjmp.h>

typedef enum {
    OB_NULL,
    OB_INT,
    OB_BOOL,
    OB_STRING,
    OB_ARRAY,
    OB_DICT,
    OB_OBJECT
} OboeTag;

typedef struct OboeValue OboeValue;
typedef struct OboeArray OboeArray;
typedef struct OboeDict OboeDict;
typedef struct OboeObject OboeObject;
typedef struct OboeClassInfo OboeClassInfo;

struct OboeClassInfo {
    const char *name;
    const OboeClassInfo *parent;
};

struct OboeObject {
    const OboeClassInfo *cls;
};

struct OboeValue {
    OboeTag tag;
    union {
        int64_t i;
        bool b;
        char *s;
        OboeArray *arr;
        OboeDict *dict;
        void *obj;
    } as;
};

struct OboeArray {
    OboeValue *items;
    size_t count;
    size_t capacity;
};

typedef struct {
    char *key;
    OboeValue value;
} OboeDictEntry;

struct OboeDict {
    OboeDictEntry *entries;
    size_t count;
    size_t capacity;
};

/* constructors */
OboeValue ob_int(int64_t v);
OboeValue ob_bool(bool v);
OboeValue ob_string(const char *v);
OboeValue ob_string_take(char *v); /* takes ownership of malloc'd buffer */
OboeValue ob_null(void);
OboeValue ob_array_new(void);
OboeValue ob_dict_new(void);
OboeValue ob_object_wrap(void *obj);

/* arrays */
void ob_array_push(OboeValue arr, OboeValue v);
OboeValue ob_array_get(OboeValue arr, int64_t idx);
void ob_array_set(OboeValue arr, int64_t idx, OboeValue v);
int64_t ob_array_len(OboeValue arr);

/* dicts */
void ob_dict_set(OboeValue d, const char *key, OboeValue v);
OboeValue ob_dict_get(OboeValue d, const char *key);
bool ob_dict_has(OboeValue d, const char *key);

/* generic `container[key]` used for index expressions; dispatches to array
   (integer key) or dict (string key) based on the container's runtime tag. */
OboeValue ob_index_get(OboeValue container, OboeValue key);
OboeValue ob_index_set(OboeValue container, OboeValue key, OboeValue value);

/* io / conversion */
void ob_print(OboeValue v);
OboeValue ob_str(OboeValue v);
char *ob_to_cstr(OboeValue v); /* borrowed pointer, valid until value freed */
OboeValue ob_interpolate(int count, ...); /* args are OboeValue strings, concatenated */

/* operators */
OboeValue ob_add(OboeValue a, OboeValue b);
OboeValue ob_sub(OboeValue a, OboeValue b);
OboeValue ob_mul(OboeValue a, OboeValue b);
OboeValue ob_div(OboeValue a, OboeValue b);
OboeValue ob_mod(OboeValue a, OboeValue b);
OboeValue ob_eq(OboeValue a, OboeValue b);
OboeValue ob_neq(OboeValue a, OboeValue b);
OboeValue ob_lt(OboeValue a, OboeValue b);
OboeValue ob_lte(OboeValue a, OboeValue b);
OboeValue ob_gt(OboeValue a, OboeValue b);
OboeValue ob_gte(OboeValue a, OboeValue b);
OboeValue ob_and(OboeValue a, OboeValue b);
OboeValue ob_or(OboeValue a, OboeValue b);
OboeValue ob_not(OboeValue a);
OboeValue ob_neg(OboeValue a);
OboeValue ob_repeat(OboeValue a, OboeValue b); /* the `x` repetition operator */
OboeValue ob_coalesce(OboeValue a, OboeValue b); /* ?? */
bool ob_truthy(OboeValue v);

/* type checks (`is` keyword) */
bool ob_is_int(OboeValue v);
bool ob_is_bool(OboeValue v);
bool ob_is_string(OboeValue v);
bool ob_is_array(OboeValue v);
bool ob_is_dict(OboeValue v);
bool ob_is_null(OboeValue v);
bool ob_is_object_of(OboeValue v, const OboeClassInfo *cls);

/* operator overloading: classes may register a handler for a given operator
   symbol; ob_binop consults the class (and its ancestors) for a handler
   before falling back to the builtin behavior. */
typedef OboeValue (*OboeOpFunc)(OboeValue, OboeValue);
void ob_register_operator(const OboeClassInfo *cls, const char *op, OboeOpFunc fn);
OboeValue ob_binop(const char *op, OboeValue a, OboeValue b, OboeOpFunc fallback);

/* range() and array-args entry point */
OboeValue ob_range(int64_t a, int64_t b);
OboeValue ob_args_from_argv(int argc, char **argv);

/* exceptions: try/catch/finally is implemented with setjmp/longjmp.
   Matching is by exception type name (string), most-specific-first,
   mirroring the ordered catch clauses in source. */
typedef struct OboeExceptionFrame {
    jmp_buf buf;
    struct OboeExceptionFrame *prev;
} OboeExceptionFrame;

extern OboeExceptionFrame *ob_exc_stack;
extern OboeValue ob_current_exception;
extern char *ob_current_exception_type;

void ob_throw(const char *type_name, OboeValue payload);
bool ob_exception_matches(const char *type_name);

#endif
