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
#ifndef OBOE_RUNTIME_H
#define OBOE_RUNTIME_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <setjmp.h>

typedef enum {
	OB_NULL,
	OB_INT,
	OB_FLOAT,
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

/* `width` and `is_unsigned` describe an OB_INT's declared integer type; they sit
   outside the union so every existing `ob_int()` call keeps meaning "plain int".
   width 0 is the default `int` (64-bit signed storage, no wrapping on its own);
   8/16/32/64 are the sized types, whose stores wrap and whose arithmetic
   promotes to the wider (and, on a tie, unsigned) operand. Floats carry
   width 32 when they were stored into a `float32`, purely so the value can be
   re-rounded on later assignments; arithmetic is always done in double. */
struct OboeValue {
	OboeTag tag;
	uint8_t width;
	bool is_unsigned;
	union {
		int64_t i;
		double f;
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

/* `entries` stays dense and in insertion order, because that order is
   observable: `.keys()`, `.values()`, `for (k, v in pairs(d))` and printing all
   walk it directly. `index` is a side table over it — open-addressed, power of
   two, storing entry index + 1 so that 0 reads as empty — built only once a dict
   is large enough for the hash to beat a scan of a small dense array. */
struct OboeDict {
	OboeDictEntry *entries;
	size_t count;
	size_t capacity;
	uint32_t *index;
	size_t index_cap;
};

/* constructors */
OboeValue ob_int(int64_t v);
OboeValue ob_int_sized(int64_t v, int width, bool is_unsigned);
OboeValue ob_float(double v);
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
void ob_write(OboeValue v); /* print without a trailing newline */
void ob_eprint(OboeValue v); /* the same pair, on stderr */
void ob_ewrite(OboeValue v);
/* byte-oriented, like every other string operation here: ord() yields the first
   byte of a multi-byte character. chr() rejects 0, which a NUL-terminated string
   cannot represent, and anything outside 0..255. */
OboeValue ob_ord(OboeValue v);
OboeValue ob_chr(OboeValue v);
OboeValue
ob_input(void); /* reads one line from stdin (newline stripped); null on EOF */
OboeValue ob_str(OboeValue v);
char *ob_to_cstr(OboeValue v); /* borrowed pointer, valid until value freed */
OboeValue ob_interpolate(int count,
			 ...); /* args are OboeValue strings, concatenated */

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
/* `and`/`or` are not emitted as these: C would evaluate both arguments, and the
   operators short-circuit. Codegen inlines `ob_bool(ob_truthy(a) && ...)`
   instead; these remain as the eager forms, for a caller that wants both sides. */
OboeValue ob_and(OboeValue a, OboeValue b);
OboeValue ob_or(OboeValue a, OboeValue b);
OboeValue ob_not(OboeValue a);
OboeValue ob_neg(OboeValue a);
OboeValue ob_repeat(OboeValue a, OboeValue b); /* the `x` repetition operator */
OboeValue ob_coalesce(OboeValue a, OboeValue b); /* ?? */
bool ob_truthy(OboeValue v);

/* bitwise operators. Integer-only: a float operand is a type error. The result
   takes the promoted width of the operands, like the arithmetic operators. */
OboeValue ob_band(OboeValue a, OboeValue b);
OboeValue ob_bor(OboeValue a, OboeValue b);
OboeValue ob_bxor(OboeValue a, OboeValue b);
OboeValue ob_shl(OboeValue a, OboeValue b);
OboeValue ob_shr(OboeValue a, OboeValue b);
OboeValue ob_bnot(OboeValue a);

/* Coercion at typed stores: `int8 x = <expr>` wraps the value to 8 bits, and
   `float32 f = <expr>` rounds it through single precision. Applied by codegen
   wherever a variable, parameter or field with a numeric type annotation is
   assigned, which is the only place declared types are enforced. */
OboeValue ob_coerce_int(OboeValue v, int width, bool is_unsigned);
OboeValue ob_coerce_float(OboeValue v, int width);

/* type checks (`is` keyword) */
bool ob_is_int(OboeValue v);
bool ob_is_float(OboeValue v);
/* `x is int8` asks whether the value fits that type's range, not how it was
   declared, so `200 is int8` is false while `127 is int8` is true. */
bool ob_is_int_width(OboeValue v, int width, bool is_unsigned);
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
void ob_register_operator(const OboeClassInfo *cls, const char *op,
			  OboeOpFunc fn);
OboeValue ob_binop(const char *op, OboeValue a, OboeValue b,
		   OboeOpFunc fallback);
/* fallback for operators that only exist as class overloads: errors at runtime
   when neither operand's class provides a handler */
OboeValue ob_op_missing(OboeValue a, OboeValue b);

/* events: installs a SIGINT handler that fires the KeyboardInterruptEvent
   handlers once and then exits; a second SIGINT while they run exits
   immediately. */
void ob_install_sigint(void (*fire)(void));

/* FFI (cimport): resolves `sym` in `lib` via dlopen/dlsym (exits on failure).
   ob_ffi_call invokes the symbol with up to 8 word-sized arguments — ints,
   bools and nulls pass by value, strings pass as C string pointers — and
   wraps the word-sized return value as an int. */
void *ob_ffi_sym(const char *lib, const char *sym);
OboeValue ob_ffi_call(void *fn, int nargs, ...);

/* built-in stdlib modules (import math / random / os). math keeps its integer
   behavior for integer arguments — pow is integer exponentiation and sqrt is
   the floor square root — and switches to floating-point when any argument is
   a float. random is a deterministic xorshift PRNG so a given seed produces the
   same sequence on every platform. os.read_file throws os.FileNotFoundError;
   write/append failures throw os.FileError. */
OboeValue ob_std_math_abs(OboeValue a);
OboeValue ob_std_math_min(OboeValue a, OboeValue b);
OboeValue ob_std_math_max(OboeValue a, OboeValue b);
OboeValue ob_std_math_pow(OboeValue a, OboeValue b);
OboeValue ob_std_math_sqrt(OboeValue a);
OboeValue ob_std_math_floor(OboeValue a); /* returns an int */
OboeValue ob_std_math_ceil(OboeValue a);
OboeValue ob_std_math_round(OboeValue a); /* half away from zero */
OboeValue ob_std_random_seed(OboeValue a);
OboeValue ob_std_random_randint(OboeValue a,
				OboeValue b); /* inclusive bounds */
OboeValue ob_std_random_choice(OboeValue arr);
OboeValue
ob_std_os_run(OboeValue cmd); /* runs via the shell; returns the exit code */
OboeValue
ob_std_os_spawn(OboeValue cmd); /* starts without waiting; returns the pid */
OboeValue ob_std_os_read_file(OboeValue path);
OboeValue ob_std_os_write_file(OboeValue path, OboeValue content);
OboeValue ob_std_os_append_file(OboeValue path, OboeValue content);
OboeValue ob_std_os_exists(OboeValue path);
OboeValue ob_std_os_remove(OboeValue path);
OboeValue ob_std_os_getenv(OboeValue name); /* string, or null when unset */
OboeValue ob_std_os_exit(OboeValue code); /* does not return */
OboeValue ob_std_os_is_dir(OboeValue path);
OboeValue ob_std_os_mkdir(OboeValue path); /* mkdir -p; existing is success */
/* entry names, no "." or "..", sorted by byte order so a directory walk is
   reproducible; throws os.FileNotFoundError when the path isn't a readable
   directory */
OboeValue ob_std_os_listdir(OboeValue path);

/* range() and array-args entry point */
OboeValue ob_range(int64_t a, int64_t b);
OboeValue ob_args_from_argv(int argc, char **argv);

/* ---- iteration ----
   One runtime dispatch point behind every `for` form, so codegen emits the same
   loop shape whatever it is iterating. Arrays yield their elements, strings
   yield one-character strings, and dicts yield their values keyed by name.
   ob_iter_key is the index for arrays and strings, and the key for dicts, which
   is what makes `pairs` a superset of `ipairs` on the sequence types. */
int64_t ob_iter_len(OboeValue v);
OboeValue ob_iter_key(OboeValue v, int64_t i);
OboeValue ob_iter_value(OboeValue v, int64_t i);

/* ---- methods on primitives ----
   `"a,b".split(",")`, `[1,2].len()` and friends. Each takes its receiver as the
   first argument and throws a TypeError when the receiver's tag doesn't match,
   since the compiler cannot know a primitive's type statically. */
/* Methods whose name is shared across receiver types dispatch on the tag at
   runtime, because the compiler never knows a primitive's type statically:
   `x.len()` compiles identically whether x is a string, array or dict. */
OboeValue ob_m_len(OboeValue v);
OboeValue ob_m_contains(OboeValue v, OboeValue needle);
OboeValue ob_m_index_of(OboeValue v, OboeValue needle); /* -1 when absent */
OboeValue ob_m_reverse(OboeValue v);
OboeValue ob_m_slice(OboeValue v, OboeValue start,
		     OboeValue end); /* half-open, clamped */
OboeValue ob_m_str(OboeValue v);

OboeValue ob_str_upper(OboeValue s);
OboeValue ob_str_lower(OboeValue s);
OboeValue ob_str_trim(OboeValue s);
OboeValue ob_str_split(OboeValue s, OboeValue sep);
OboeValue ob_str_starts_with(OboeValue s, OboeValue prefix);
OboeValue ob_str_ends_with(OboeValue s, OboeValue suffix);
OboeValue ob_str_replace(OboeValue s, OboeValue from,
			 OboeValue to); /* all occurrences */
OboeValue ob_str_substr(OboeValue s, OboeValue start, OboeValue len);
OboeValue ob_str_repeat(OboeValue s, OboeValue n);
OboeValue ob_str_to_int(OboeValue s); /* throws ValueError when unparsable */
OboeValue ob_str_to_float(OboeValue s);

OboeValue ob_arr_push(OboeValue a, OboeValue v);
OboeValue ob_arr_pop(OboeValue a);
OboeValue ob_arr_insert(OboeValue a, OboeValue idx, OboeValue v);
OboeValue ob_arr_remove_at(OboeValue a, OboeValue idx);
OboeValue ob_arr_join(OboeValue a, OboeValue sep);

OboeValue ob_dict_keys(OboeValue d);
OboeValue ob_dict_values(OboeValue d);
OboeValue ob_dict_has_m(OboeValue d, OboeValue key);
OboeValue ob_dict_remove(OboeValue d, OboeValue key);

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
