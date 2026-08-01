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
#include "oboe_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <ctype.h>
#include <math.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <direct.h>
/* the CRT's mkdir takes no mode; POSIX's does */
#define ob_mkdir_one(p) _mkdir(p)
/* _fullpath is the CRT's realpath: same "resolve into this buffer, NULL on
   failure" contract, and it is the only one there is on Windows */
#define ob_realpath(p, buf) _fullpath((buf), (p), PATH_MAX)
#else
#include <unistd.h>
#include <sys/wait.h>
#include <dlfcn.h>
#define ob_mkdir_one(p) mkdir(p, 0755)
#define ob_realpath(p, buf) realpath((p), (buf))
#endif

/* POSIX lets PATH_MAX be undefined when the limit is not fixed; 4096 is what
   the compiler's own path buffers already assume. */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

OboeExceptionFrame *ob_exc_stack = NULL;
OboeValue ob_current_exception;
char *ob_current_exception_type = NULL;

static void ob_oom(void)
{
	fprintf(stderr, "oboe: out of memory\n");
	exit(1);
}

/* strndup is POSIX 2008, but this file is cross-compiled to Windows too, and
   mingw hides its CRT extensions under -std=c11's __STRICT_ANSI__. Carrying our
   own is cheaper than either widening the dialect for every compiled program or
   guessing which libc has it. */
static char *ob_strndup(const char *s, size_t n)
{
	char *out = malloc(n + 1);
	if (!out)
		ob_oom();
	memcpy(out, s, n);
	out[n] = '\0';
	return out;
}

/* ---- numeric representation ----
   An OB_INT is stored as an int64_t plus the width/signedness of the Oboe type
   it was declared as. width 0 means the plain `int` (64-bit signed); 8/16/32/64
   are the sized types. A sized value is always kept already wrapped into its
   range, so nothing downstream has to re-normalize before reading `.as.i`.

   Arithmetic promotes to the wider operand, and to unsigned when the widest
   operand is unsigned — the same shape as C's usual arithmetic conversions,
   minus the integer-promotion-to-int step (there is no `int` smaller than the
   value's own width here). Two plain ints stay a plain int, so ordinary code
   never acquires a width it didn't ask for. */

static int ob_eff_width(OboeValue v)
{
	return v.width ? v.width : 64;
}

static void ob_promote(OboeValue a, OboeValue b, int *out_w, bool *out_u)
{
	if (a.width == 0 && b.width == 0) {
		*out_w = 0;
		*out_u = false;
		return;
	}
	int wa = ob_eff_width(a), wb = ob_eff_width(b);
	int w = wa > wb ? wa : wb;
	bool u = (wa == w && a.is_unsigned) || (wb == w && b.is_unsigned);
	*out_w = w;
	*out_u = u;
}

/* truncates to `width` bits, sign-extending for signed types */
static int64_t ob_wrap(int64_t v, int width, bool is_unsigned)
{
	if (width == 0 || width >= 64)
		return v; /* 64-bit: signedness is interpretation only */
	uint64_t mask = (1ULL << width) - 1;
	uint64_t raw = (uint64_t)v & mask;
	if (!is_unsigned && (raw & (1ULL << (width - 1))))
		return (int64_t)(raw | ~mask);
	return (int64_t)raw;
}

static bool ob_is_num(OboeValue v)
{
	return v.tag == OB_INT || v.tag == OB_FLOAT;
}

static double ob_as_double(OboeValue v)
{
	if (v.tag == OB_FLOAT)
		return v.as.f;
	if (v.is_unsigned)
		return (double)(uint64_t)v.as.i;
	return (double)v.as.i;
}

OboeValue ob_int(int64_t v)
{
	OboeValue r = { 0 };
	r.tag = OB_INT;
	r.as.i = v;
	return r;
}

OboeValue ob_int_sized(int64_t v, int width, bool is_unsigned)
{
	OboeValue r = { 0 };
	r.tag = OB_INT;
	r.width = (uint8_t)width;
	r.is_unsigned = is_unsigned;
	r.as.i = ob_wrap(v, width, is_unsigned);
	return r;
}

OboeValue ob_float(double v)
{
	OboeValue r = { 0 };
	r.tag = OB_FLOAT;
	r.as.f = v;
	return r;
}

OboeValue ob_bool(bool v)
{
	OboeValue r = { 0 };
	r.tag = OB_BOOL;
	r.as.b = v;
	return r;
}
OboeValue ob_null(void)
{
	OboeValue r = { 0 };
	r.tag = OB_NULL;
	r.as.i = 0;
	return r;
}

/* Allocates [size_t length][len payload bytes][NUL] and returns the payload,
   which is what as.s points at. See the note on ob_slen in the header. */
static char *ob_str_alloc(size_t len)
{
	size_t *hdr = malloc(sizeof(size_t) + len + 1);
	if (!hdr)
		ob_oom();
	*hdr = len;
	char *s = (char *)(hdr + 1);
	s[len] = '\0';
	return s;
}

size_t ob_slen(const char *s)
{
	return ((const size_t *)s)[-1];
}

/* Wraps a payload that came from ob_str_alloc, with no second copy. */
static OboeValue ob_string_wrap(char *payload)
{
	OboeValue r = { 0 };
	r.tag = OB_STRING;
	r.as.s = payload;
	return r;
}

OboeValue ob_string(const char *v)
{
	OboeValue r = { 0 };
	r.tag = OB_STRING;
	if (!v)
		v = "";
	size_t n = strlen(v);
	r.as.s = ob_str_alloc(n);
	memcpy(r.as.s, v, n);
	return r;
}

/* The length header has to sit in front of the payload, so a buffer that was
   malloc'd elsewhere cannot simply be adopted -- this copies its contents and
   frees it. The name stays because every caller still means "here, it's
   yours"; only the mechanics changed. */
OboeValue ob_string_take(char *v)
{
	OboeValue r = ob_string(v);
	free(v);
	return r;
}

OboeValue ob_array_new(void)
{
	OboeValue r = { 0 };
	r.tag = OB_ARRAY;
	r.as.arr = malloc(sizeof(OboeArray));
	if (!r.as.arr)
		ob_oom();
	r.as.arr->items = NULL;
	r.as.arr->count = 0;
	r.as.arr->capacity = 0;
	return r;
}

OboeValue ob_dict_new(void)
{
	OboeValue r = { 0 };
	r.tag = OB_DICT;
	r.as.dict = malloc(sizeof(OboeDict));
	if (!r.as.dict)
		ob_oom();
	r.as.dict->entries = NULL;
	r.as.dict->count = 0;
	r.as.dict->capacity = 0;
	r.as.dict->index = NULL;
	r.as.dict->index_cap = 0;
	return r;
}

OboeValue ob_object_wrap(void *obj)
{
	OboeValue r = { 0 };
	r.tag = OB_OBJECT;
	r.as.obj = obj;
	return r;
}

void ob_array_push(OboeValue arr, OboeValue v)
{
	OboeArray *a = arr.as.arr;
	if (a->count == a->capacity) {
		a->capacity = a->capacity ? a->capacity * 2 : 4;
		a->items = realloc(a->items, a->capacity * sizeof(OboeValue));
		if (!a->items)
			ob_oom();
	}
	a->items[a->count++] = v;
}

static void ob_bounds_check(OboeArray *a, int64_t idx)
{
	if (idx < 0 || (size_t)idx >= a->count) {
		fprintf(stderr,
			"oboe: array index %lld out of bounds (len %zu)\n",
			(long long)idx, a->count);
		exit(1);
	}
}

OboeValue ob_array_get(OboeValue arr, int64_t idx)
{
	OboeArray *a = arr.as.arr;
	ob_bounds_check(a, idx);
	return a->items[idx];
}

void ob_array_set(OboeValue arr, int64_t idx, OboeValue v)
{
	OboeArray *a = arr.as.arr;
	ob_bounds_check(a, idx);
	a->items[idx] = v;
}

int64_t ob_array_len(OboeValue arr)
{
	return (int64_t)arr.as.arr->count;
}

/* Below this many entries a scan of the dense array beats hashing it: no hash to
   compute, and the whole thing is a cache line or two. Most dicts in practice --
   an object's worth of named fields -- never get past it. */
#define OB_DICT_INDEX_MIN 8

static size_t ob_dict_hash(const char *key)
{
	/* FNV-1a */
	size_t h = 1469598103934665603ULL;
	for (const unsigned char *p = (const unsigned char *)key; *p; p++) {
		h ^= *p;
		h *= 1099511628211ULL;
	}
	return h;
}

/* slot for `key` in the open-addressed index: either the one holding it, or the
   empty one where it belongs. Only valid while index_cap > 0. */
static size_t ob_dict_slot(const OboeDict *dict, const char *key)
{
	size_t mask = dict->index_cap - 1;
	size_t i = ob_dict_hash(key) & mask;
	while (dict->index[i]) {
		if (strcmp(dict->entries[dict->index[i] - 1].key, key) == 0)
			break;
		i = (i + 1) & mask;
	}
	return i;
}

/* (re)builds the index from `entries`. Also the recovery path after a removal
   shifts entries down and invalidates every stored position. */
static void ob_dict_reindex(OboeDict *dict, size_t cap)
{
	free(dict->index);
	dict->index_cap = cap;
	dict->index = calloc(cap, sizeof(uint32_t));
	if (!dict->index)
		ob_oom();
	for (size_t i = 0; i < dict->count; i++)
		dict->index[ob_dict_slot(dict, dict->entries[i].key)] =
			(uint32_t)(i + 1);
}

/* index of `key` in `entries`, or -1. Scans while the dict is small, since the
   index only exists past OB_DICT_INDEX_MIN. */
static ptrdiff_t ob_dict_find(const OboeDict *dict, const char *key)
{
	if (!dict->index) {
		for (size_t i = 0; i < dict->count; i++)
			if (strcmp(dict->entries[i].key, key) == 0)
				return (ptrdiff_t)i;
		return -1;
	}
	uint32_t slot = dict->index[ob_dict_slot(dict, key)];
	return slot ? (ptrdiff_t)(slot - 1) : -1;
}

void ob_dict_set(OboeValue d, const char *key, OboeValue v)
{
	OboeDict *dict = d.as.dict;
	ptrdiff_t at = ob_dict_find(dict, key);
	if (at >= 0) {
		dict->entries[at].value = v;
		return;
	}
	if (dict->count == dict->capacity) {
		dict->capacity = dict->capacity ? dict->capacity * 2 : 4;
		dict->entries = realloc(dict->entries,
					dict->capacity * sizeof(OboeDictEntry));
		if (!dict->entries)
			ob_oom();
	}
	dict->entries[dict->count].key = strdup(key);
	if (!dict->entries[dict->count].key)
		ob_oom();
	dict->entries[dict->count].value = v;
	dict->count++;

	/* grow at a 0.75 load factor, and build the index the first time the
	   dict is big enough to want one */
	if (dict->count * 4 >= dict->index_cap * 3) {
		if (dict->count >= OB_DICT_INDEX_MIN)
			ob_dict_reindex(dict, dict->index_cap ?
						      dict->index_cap * 2 :
						      OB_DICT_INDEX_MIN * 2);
	} else {
		dict->index[ob_dict_slot(dict, key)] = (uint32_t)dict->count;
	}
}

OboeValue ob_dict_get(OboeValue d, const char *key)
{
	OboeDict *dict = d.as.dict;
	ptrdiff_t at = ob_dict_find(dict, key);
	return at >= 0 ? dict->entries[at].value : ob_null();
}

bool ob_dict_has(OboeValue d, const char *key)
{
	return ob_dict_find(d.as.dict, key) >= 0;
}

OboeValue ob_index_get(OboeValue container, OboeValue key)
{
	if (container.tag == OB_DICT) {
		char *k = ob_to_cstr(key);
		OboeValue r = ob_dict_get(container, k);
		free(k);
		return r;
	}
	if (container.tag == OB_ARRAY)
		return ob_array_get(container, key.as.i);
	fprintf(stderr, "oboe: cannot index into this value\n");
	exit(1);
}

OboeValue ob_index_set(OboeValue container, OboeValue key, OboeValue value)
{
	if (container.tag == OB_DICT) {
		char *k = ob_to_cstr(key);
		ob_dict_set(container, k, value);
		free(k);
		return value;
	}
	if (container.tag == OB_ARRAY) {
		ob_array_set(container, key.as.i, value);
		return value;
	}
	fprintf(stderr, "oboe: cannot index into this value\n");
	exit(1);
}

char *ob_to_cstr(OboeValue v)
{
	char buf[64];
	switch (v.tag) {
	case OB_NULL:
		return strdup("null");
	case OB_INT:
		if (v.is_unsigned)
			snprintf(buf, sizeof buf, "%llu",
				 (unsigned long long)v.as.i);
		else
			snprintf(buf, sizeof buf, "%lld", (long long)v.as.i);
		return strdup(buf);
	case OB_FLOAT: {
		double d = v.as.f;
		if (isnan(d))
			return strdup("nan");
		if (isinf(d))
			return strdup(d < 0 ? "-inf" : "inf");
		/* shortest %g that still round-trips, so 0.1 prints as "0.1" rather
               than 0.10000000000000001 while nothing loses information */
		for (int prec = 1; prec <= 17; prec++) {
			snprintf(buf, sizeof buf, "%.*g", prec, d);
			if (strtod(buf, NULL) == d)
				break;
		}
		/* keep floats visibly floats: 1 prints as "1.0", not "1" */
		if (!strpbrk(buf, ".eE"))
			strncat(buf, ".0", sizeof(buf) - strlen(buf) - 1);
		return strdup(buf);
	}
	case OB_BOOL:
		return strdup(v.as.b ? "true" : "false");
	case OB_STRING:
		return strdup(v.as.s);
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
			if (i > 0) {
				strcat(out, ", ");
				len += 2;
			}
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
			size_t need =
				strlen(d->entries[i].key) + strlen(piece) + 8;
			if (len + need > cap) {
				cap = (len + need) * 2;
				out = realloc(out, cap);
			}
			if (i > 0)
				strcat(out, ", ");
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

void ob_print(OboeValue v)
{
	char *s = ob_to_cstr(v);
	printf("%s\n", s);
	free(s);
}

void ob_write(OboeValue v)
{
	char *s = ob_to_cstr(v);
	printf("%s", s);
	fflush(stdout);
	free(s);
}

/* stderr counterparts. stdout is block-buffered when it isn't a terminal, so a
   program writing to both has to flush it here or its diagnostics land out of
   order relative to its output. */
void ob_eprint(OboeValue v)
{
	char *s = ob_to_cstr(v);
	fflush(stdout);
	fprintf(stderr, "%s\n", s);
	free(s);
}

void ob_ewrite(OboeValue v)
{
	char *s = ob_to_cstr(v);
	fflush(stdout);
	fputs(s, stderr);
	fflush(stderr);
	free(s);
}

/* ord/chr are byte-oriented, like the rest of the string handling: ord() takes
   the first byte of a multi-byte character rather than decoding it. */
OboeValue ob_ord(OboeValue v)
{
	if (v.tag != OB_STRING)
		ob_throw("TypeError",
			 ob_string("ord() expects a string, got another type"));
	if (v.as.s[0] == '\0')
		ob_throw("ValueError", ob_string("ord() got an empty string"));
	return ob_int((unsigned char)v.as.s[0]);
}

OboeValue ob_chr(OboeValue v)
{
	if (v.tag != OB_INT)
		ob_throw("TypeError",
			 ob_string("chr() expects an int, got another type"));
	if (v.as.i < 0 || v.as.i > 255)
		ob_throw("ValueError",
			 ob_string("chr() needs a byte value in 0..255"));
	/* 0 would produce an empty string rather than a one-byte one, since
	   strings are NUL-terminated; refuse it instead of lying about length */
	if (v.as.i == 0)
		ob_throw("ValueError",
			 ob_string("chr(0) has no representable string value"));
	char buf[2] = { (char)v.as.i, '\0' };
	return ob_string(buf);
}

OboeValue ob_input(void)
{
	/* growable fgets loop rather than getline(), which mingw lacks */
	size_t cap = 128, n = 0;
	char *line = malloc(cap);
	for (;;) {
		if (!fgets(line + n, cap - n, stdin)) {
			if (n == 0) {
				free(line);
				return ob_null();
			}
			break;
		}
		n += strlen(line + n);
		if (n > 0 && line[n - 1] == '\n') {
			line[n - 1] = '\0';
			break;
		}
		if (n + 1 >= cap) {
			cap *= 2;
			line = realloc(line, cap);
		}
	}
	return ob_string_take(line);
}

OboeValue ob_str(OboeValue v)
{
	if (v.tag == OB_STRING)
		return ob_string(v.as.s);
	char *s = ob_to_cstr(v);
	return ob_string_take(s);
}

OboeValue ob_interpolate(int count, ...)
{
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
typedef struct {
	const OboeClassInfo *cls;
	char op[16];
	OboeOpFunc fn;
} OboeOpEntry;
static OboeOpEntry ob_op_table[OB_MAX_OPERATORS];
static int ob_op_count = 0;

void ob_register_operator(const OboeClassInfo *cls, const char *op,
			  OboeOpFunc fn)
{
	if (ob_op_count >= OB_MAX_OPERATORS) {
		fprintf(stderr, "oboe: too many operator overloads\n");
		exit(1);
	}
	ob_op_table[ob_op_count].cls = cls;
	strncpy(ob_op_table[ob_op_count].op, op, 15);
	ob_op_table[ob_op_count].op[15] = '\0';
	ob_op_table[ob_op_count].fn = fn;
	ob_op_count++;
}

static OboeOpFunc ob_find_operator(const OboeClassInfo *cls, const char *op)
{
	while (cls) {
		for (int i = 0; i < ob_op_count; i++) {
			if (ob_op_table[i].cls == cls &&
			    strcmp(ob_op_table[i].op, op) == 0)
				return ob_op_table[i].fn;
		}
		cls = cls->parent;
	}
	return NULL;
}

OboeValue ob_binop(const char *op, OboeValue a, OboeValue b,
		   OboeOpFunc fallback)
{
	if (a.tag == OB_OBJECT) {
		OboeOpFunc fn =
			ob_find_operator(((OboeObject *)a.as.obj)->cls, op);
		if (fn)
			return fn(a, b);
	}
	return fallback(a, b);
}

OboeValue ob_op_missing(OboeValue a, OboeValue b)
{
	(void)a;
	(void)b;
	fprintf(stderr, "oboe: no operator overload matches these operands\n");
	exit(1);
}

/* ---- events: SIGINT handling ----
   First ^C: run the KeyboardInterruptEvent handlers, then exit.
   Second ^C while the handlers run: exit immediately. */
static void (*ob_sigint_fire)(void) = NULL;
static volatile sig_atomic_t ob_in_sigint = 0;

static void ob_sigint_handler(int sig)
{
	(void)sig;
#ifdef _WIN32
	signal(SIGINT,
	       ob_sigint_handler); /* the CRT resets the handler on delivery */
#endif
	if (ob_in_sigint)
		_exit(130);
	ob_in_sigint = 1;
	if (ob_sigint_fire)
		ob_sigint_fire();
	fflush(NULL);
	_exit(130);
}

void ob_install_sigint(void (*fire)(void))
{
	ob_sigint_fire = fire;
#ifdef _WIN32
	signal(SIGINT, ob_sigint_handler);
#else
	struct sigaction sa = { 0 };
	sa.sa_handler = ob_sigint_handler;
	sigemptyset(&sa.sa_mask);
	/* SA_NODEFER so a second SIGINT is delivered while the handlers run */
	sa.sa_flags = SA_NODEFER;
	sigaction(SIGINT, &sa, NULL);
#endif
}

/* ---- FFI ---- */
#define OB_MAX_FFI_LIBS 32
static struct {
	char *name;
	void *handle;
} ob_ffi_libs[OB_MAX_FFI_LIBS];
static int ob_ffi_lib_count = 0;

void *ob_ffi_sym(const char *lib, const char *sym)
{
	void *handle = NULL;
	for (int i = 0; i < ob_ffi_lib_count; i++) {
		if (strcmp(ob_ffi_libs[i].name, lib) == 0) {
			handle = ob_ffi_libs[i].handle;
			break;
		}
	}
	if (!handle) {
#ifdef _WIN32
		handle = (void *)LoadLibraryA(lib);
		if (!handle) {
			fprintf(stderr, "oboe: cannot load library '%s'\n",
				lib);
			exit(1);
		}
#else
		handle = dlopen(lib, RTLD_NOW | RTLD_GLOBAL);
		if (!handle) {
			fprintf(stderr, "oboe: cannot load library '%s': %s\n",
				lib, dlerror());
			exit(1);
		}
#endif
		if (ob_ffi_lib_count < OB_MAX_FFI_LIBS) {
			ob_ffi_libs[ob_ffi_lib_count].name = strdup(lib);
			ob_ffi_libs[ob_ffi_lib_count].handle = handle;
			ob_ffi_lib_count++;
		}
	}
#ifdef _WIN32
	void *fn = (void *)GetProcAddress((HMODULE)handle, sym);
#else
	void *fn = dlsym(handle, sym);
#endif
	if (!fn) {
		fprintf(stderr, "oboe: cannot find symbol '%s' in '%s'\n", sym,
			lib);
		exit(1);
	}
	return fn;
}

static intptr_t ob_ffi_word(OboeValue v)
{
	switch (v.tag) {
	case OB_NULL:
		return 0;
	case OB_INT:
		return (intptr_t)v.as.i;
	case OB_BOOL:
		return v.as.b ? 1 : 0;
	case OB_STRING:
		return (intptr_t)v.as.s;
	default:
		fprintf(stderr,
			"oboe: cannot pass this value through the C FFI\n");
		exit(1);
	}
}

OboeValue ob_ffi_call(void *fn, int nargs, ...)
{
	if (nargs > 8) {
		fprintf(stderr,
			"oboe: FFI calls support at most 8 arguments\n");
		exit(1);
	}
	intptr_t a[8] = { 0 };
	va_list ap;
	va_start(ap, nargs);
	for (int i = 0; i < nargs; i++)
		a[i] = ob_ffi_word(va_arg(ap, OboeValue));
	va_end(ap);
	typedef intptr_t (*F0)(void);
	typedef intptr_t (*F1)(intptr_t);
	typedef intptr_t (*F2)(intptr_t, intptr_t);
	typedef intptr_t (*F3)(intptr_t, intptr_t, intptr_t);
	typedef intptr_t (*F4)(intptr_t, intptr_t, intptr_t, intptr_t);
	typedef intptr_t (*F5)(intptr_t, intptr_t, intptr_t, intptr_t,
			       intptr_t);
	typedef intptr_t (*F6)(intptr_t, intptr_t, intptr_t, intptr_t, intptr_t,
			       intptr_t);
	typedef intptr_t (*F7)(intptr_t, intptr_t, intptr_t, intptr_t, intptr_t,
			       intptr_t, intptr_t);
	typedef intptr_t (*F8)(intptr_t, intptr_t, intptr_t, intptr_t, intptr_t,
			       intptr_t, intptr_t, intptr_t);
	intptr_t r = 0;
	switch (nargs) {
	case 0:
		r = ((F0)fn)();
		break;
	case 1:
		r = ((F1)fn)(a[0]);
		break;
	case 2:
		r = ((F2)fn)(a[0], a[1]);
		break;
	case 3:
		r = ((F3)fn)(a[0], a[1], a[2]);
		break;
	case 4:
		r = ((F4)fn)(a[0], a[1], a[2], a[3]);
		break;
	case 5:
		r = ((F5)fn)(a[0], a[1], a[2], a[3], a[4]);
		break;
	case 6:
		r = ((F6)fn)(a[0], a[1], a[2], a[3], a[4], a[5]);
		break;
	case 7:
		r = ((F7)fn)(a[0], a[1], a[2], a[3], a[4], a[5], a[6]);
		break;
	case 8:
		r = ((F8)fn)(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
		break;
	}
	return ob_int((int64_t)r);
}

static void ob_type_error(const char *op)
{
	fprintf(stderr, "oboe: type error in operator %s\n", op);
	exit(1);
}

/* result of integer arithmetic under the promotion rules above */
static OboeValue ob_int_result(OboeValue a, OboeValue b, int64_t raw)
{
	int w;
	bool u;
	ob_promote(a, b, &w, &u);
	if (w == 0)
		return ob_int(raw);
	return ob_int_sized(raw, w, u);
}

/* true when the operation should be done in floating point */
static bool ob_either_float(OboeValue a, OboeValue b)
{
	return a.tag == OB_FLOAT || b.tag == OB_FLOAT;
}

OboeValue ob_add(OboeValue a, OboeValue b)
{
	if (a.tag == OB_STRING && b.tag == OB_STRING) {
		/* the overwhelmingly common case, and worth not routing
		   through two throwaway copies of what we already have */
		size_t na = ob_slen(a.as.s), nb = ob_slen(b.as.s);
		char *out = ob_str_alloc(na + nb);
		memcpy(out, a.as.s, na);
		memcpy(out + na, b.as.s, nb);
		return ob_string_wrap(out);
	}
	if (a.tag == OB_STRING || b.tag == OB_STRING) {
		char *as = ob_to_cstr(a);
		char *bs = ob_to_cstr(b);
		char *out = malloc(strlen(as) + strlen(bs) + 1);
		strcpy(out, as);
		strcat(out, bs);
		free(as);
		free(bs);
		return ob_string_take(out);
	}
	if (ob_is_num(a) && ob_is_num(b)) {
		if (ob_either_float(a, b))
			return ob_float(ob_as_double(a) + ob_as_double(b));
		return ob_int_result(
			a, b, (int64_t)((uint64_t)a.as.i + (uint64_t)b.as.i));
	}
	ob_type_error("+");
	return ob_null();
}

OboeValue ob_sub(OboeValue a, OboeValue b)
{
	if (ob_is_num(a) && ob_is_num(b)) {
		if (ob_either_float(a, b))
			return ob_float(ob_as_double(a) - ob_as_double(b));
		return ob_int_result(
			a, b, (int64_t)((uint64_t)a.as.i - (uint64_t)b.as.i));
	}
	ob_type_error("-");
	return ob_null();
}
OboeValue ob_mul(OboeValue a, OboeValue b)
{
	if (ob_is_num(a) && ob_is_num(b)) {
		if (ob_either_float(a, b))
			return ob_float(ob_as_double(a) * ob_as_double(b));
		return ob_int_result(
			a, b, (int64_t)((uint64_t)a.as.i * (uint64_t)b.as.i));
	}
	ob_type_error("*");
	return ob_null();
}
/* int / int stays integer division; a float operand makes it real division,
   where dividing by zero yields inf/nan rather than being an error */
OboeValue ob_div(OboeValue a, OboeValue b)
{
	if (ob_is_num(a) && ob_is_num(b)) {
		if (ob_either_float(a, b))
			return ob_float(ob_as_double(a) / ob_as_double(b));
		if (b.as.i == 0) {
			fprintf(stderr, "oboe: division by zero\n");
			exit(1);
		}
		int w;
		bool u;
		ob_promote(a, b, &w, &u);
		if (u)
			return ob_int_sized(
				(int64_t)((uint64_t)a.as.i / (uint64_t)b.as.i),
				w, u);
		return ob_int_result(a, b, a.as.i / b.as.i);
	}
	ob_type_error("/");
	return ob_null();
}
OboeValue ob_mod(OboeValue a, OboeValue b)
{
	if (ob_is_num(a) && ob_is_num(b)) {
		if (ob_either_float(a, b)) {
			double db = ob_as_double(b);
			if (db == 0.0) {
				fprintf(stderr, "oboe: modulo by zero\n");
				exit(1);
			}
			return ob_float(fmod(ob_as_double(a), db));
		}
		if (b.as.i == 0) {
			fprintf(stderr, "oboe: modulo by zero\n");
			exit(1);
		}
		int w;
		bool u;
		ob_promote(a, b, &w, &u);
		if (u)
			return ob_int_sized(
				(int64_t)((uint64_t)a.as.i % (uint64_t)b.as.i),
				w, u);
		return ob_int_result(a, b, a.as.i % b.as.i);
	}
	ob_type_error("%");
	return ob_null();
}

/* ---- bitwise ----
   Integer-only; the result takes the promoted width, and shifts take the width
   of their left operand alone (the right operand is only a count). */
static int64_t ob_want_bits(OboeValue v, const char *op)
{
	if (v.tag != OB_INT)
		ob_type_error(op);
	return v.as.i;
}

OboeValue ob_band(OboeValue a, OboeValue b)
{
	return ob_int_result(a, b, ob_want_bits(a, "&") & ob_want_bits(b, "&"));
}
OboeValue ob_bor(OboeValue a, OboeValue b)
{
	return ob_int_result(a, b, ob_want_bits(a, "|") | ob_want_bits(b, "|"));
}
OboeValue ob_bxor(OboeValue a, OboeValue b)
{
	return ob_int_result(a, b, ob_want_bits(a, "^") ^ ob_want_bits(b, "^"));
}

static OboeValue ob_shift_result(OboeValue a, int64_t raw)
{
	if (a.width == 0)
		return ob_int(raw);
	return ob_int_sized(raw, a.width, a.is_unsigned);
}

OboeValue ob_shl(OboeValue a, OboeValue b)
{
	int64_t n = ob_want_bits(b, "<<");
	ob_want_bits(a, "<<");
	if (n < 0 || n >= 64)
		return ob_shift_result(a, 0);
	return ob_shift_result(a, (int64_t)((uint64_t)a.as.i << n));
}
OboeValue ob_shr(OboeValue a, OboeValue b)
{
	int64_t n = ob_want_bits(b, ">>");
	ob_want_bits(a, ">>");
	if (n < 0)
		return ob_shift_result(a, 0);
	/* unsigned values shift in zeros, signed ones keep their sign */
	if (n >= 64)
		return ob_shift_result(a,
				       (!a.is_unsigned && a.as.i < 0) ? -1 : 0);
	if (a.is_unsigned)
		return ob_shift_result(a, (int64_t)((uint64_t)a.as.i >> n));
	return ob_shift_result(a, a.as.i >> n);
}
OboeValue ob_bnot(OboeValue a)
{
	ob_want_bits(a, "~");
	return ob_shift_result(a, ~a.as.i);
}

/* ---- coercion at typed stores ---- */
OboeValue ob_coerce_int(OboeValue v, int width, bool is_unsigned)
{
	if (v.tag == OB_FLOAT)
		return ob_int_sized((int64_t)v.as.f, width, is_unsigned);
	if (v.tag != OB_INT)
		return v; /* leave non-numerics alone; nothing enforces types here */
	return ob_int_sized(v.as.i, width, is_unsigned);
}

OboeValue ob_coerce_float(OboeValue v, int width)
{
	double d;
	if (v.tag == OB_INT)
		d = ob_as_double(v);
	else if (v.tag == OB_FLOAT)
		d = v.as.f;
	else
		return v;
	if (width == 32)
		d = (double)(float)
			d; /* float32 stores round through single precision */
	OboeValue r = ob_float(d);
	r.width = (uint8_t)width;
	return r;
}

/* compares two numbers under the promotion rules; ints of different widths (and
   an int against a float) compare by value, so `5 == 5.0` and `int8(5) == 5` */
static int ob_num_cmp(OboeValue a, OboeValue b)
{
	if (ob_either_float(a, b)) {
		double x = ob_as_double(a), y = ob_as_double(b);
		return x < y ? -1 : (x > y ? 1 : 0);
	}
	int w;
	bool u;
	ob_promote(a, b, &w, &u);
	if (u) {
		uint64_t x = (uint64_t)a.as.i, y = (uint64_t)b.as.i;
		/* a signed operand promoted to unsigned wraps, exactly as it would in C */
		return x < y ? -1 : (x > y ? 1 : 0);
	}
	return a.as.i < b.as.i ? -1 : (a.as.i > b.as.i ? 1 : 0);
}

static bool ob_raw_eq(OboeValue a, OboeValue b)
{
	if (ob_is_num(a) && ob_is_num(b))
		return ob_num_cmp(a, b) == 0;
	if (a.tag != b.tag)
		return false;
	switch (a.tag) {
	case OB_NULL:
		return true;
	case OB_INT:
		return a.as.i == b.as.i;
	case OB_FLOAT:
		return a.as.f == b.as.f;
	case OB_BOOL:
		return a.as.b == b.as.b;
	case OB_STRING:
		return strcmp(a.as.s, b.as.s) == 0;
	case OB_OBJECT:
		return a.as.obj == b.as.obj;
	case OB_ARRAY:
		return a.as.arr == b.as.arr;
	case OB_DICT:
		return a.as.dict == b.as.dict;
	}
	return false;
}

OboeValue ob_eq(OboeValue a, OboeValue b)
{
	return ob_bool(ob_raw_eq(a, b));
}
OboeValue ob_neq(OboeValue a, OboeValue b)
{
	return ob_bool(!ob_raw_eq(a, b));
}

OboeValue ob_lt(OboeValue a, OboeValue b)
{
	if (ob_is_num(a) && ob_is_num(b))
		return ob_bool(ob_num_cmp(a, b) < 0);
	if (a.tag == OB_STRING && b.tag == OB_STRING)
		return ob_bool(strcmp(a.as.s, b.as.s) < 0);
	ob_type_error("<");
	return ob_null();
}
OboeValue ob_lte(OboeValue a, OboeValue b)
{
	if (ob_is_num(a) && ob_is_num(b))
		return ob_bool(ob_num_cmp(a, b) <= 0);
	if (a.tag == OB_STRING && b.tag == OB_STRING)
		return ob_bool(strcmp(a.as.s, b.as.s) <= 0);
	ob_type_error("<=");
	return ob_null();
}
OboeValue ob_gt(OboeValue a, OboeValue b)
{
	if (ob_is_num(a) && ob_is_num(b))
		return ob_bool(ob_num_cmp(a, b) > 0);
	if (a.tag == OB_STRING && b.tag == OB_STRING)
		return ob_bool(strcmp(a.as.s, b.as.s) > 0);
	ob_type_error(">");
	return ob_null();
}
OboeValue ob_gte(OboeValue a, OboeValue b)
{
	if (ob_is_num(a) && ob_is_num(b))
		return ob_bool(ob_num_cmp(a, b) >= 0);
	if (a.tag == OB_STRING && b.tag == OB_STRING)
		return ob_bool(strcmp(a.as.s, b.as.s) >= 0);
	ob_type_error(">=");
	return ob_null();
}

bool ob_truthy(OboeValue v)
{
	switch (v.tag) {
	case OB_NULL:
		return false;
	case OB_BOOL:
		return v.as.b;
	case OB_INT:
		return v.as.i != 0;
	case OB_FLOAT:
		return v.as.f != 0.0;
	case OB_STRING:
		return v.as.s[0] != '\0';
	case OB_ARRAY:
		return v.as.arr->count != 0;
	case OB_DICT:
		return v.as.dict->count != 0;
	case OB_OBJECT:
		return true;
	}
	return false;
}

OboeValue ob_and(OboeValue a, OboeValue b)
{
	return ob_bool(ob_truthy(a) && ob_truthy(b));
}
OboeValue ob_or(OboeValue a, OboeValue b)
{
	return ob_bool(ob_truthy(a) || ob_truthy(b));
}
OboeValue ob_not(OboeValue a)
{
	return ob_bool(!ob_truthy(a));
}
OboeValue ob_neg(OboeValue a)
{
	if (a.tag == OB_FLOAT)
		return ob_float(-a.as.f);
	if (a.tag == OB_INT)
		return ob_shift_result(a, (int64_t)(0 - (uint64_t)a.as.i));
	ob_type_error("unary -");
	return ob_null();
}

OboeValue ob_repeat(OboeValue a, OboeValue b)
{
	if (a.tag == OB_STRING && b.tag == OB_INT) {
		int64_t n = b.as.i;
		if (n < 0)
			n = 0;
		size_t len = ob_slen(a.as.s);
		char *out = malloc(len * n + 1);
		for (int64_t i = 0; i < n; i++)
			memcpy(out + len * i, a.as.s, len);
		out[len * n] = '\0';
		return ob_string_take(out);
	}
	ob_type_error("x");
	return ob_null();
}

OboeValue ob_coalesce(OboeValue a, OboeValue b)
{
	return ob_is_null(a) ? b : a;
}

bool ob_is_int(OboeValue v)
{
	return v.tag == OB_INT;
}
bool ob_is_float(OboeValue v)
{
	return v.tag == OB_FLOAT;
}

/* `x is int8` is a range question, not a "how was this declared" question, so a
   plain int holding 127 is an int8 and one holding 200 is not. */
bool ob_is_int_width(OboeValue v, int width, bool is_unsigned)
{
	if (v.tag != OB_INT)
		return false;
	if (v.is_unsigned) {
		uint64_t x = (uint64_t)v.as.i;
		if (is_unsigned)
			return width >= 64 || x <= (1ULL << width) - 1;
		if (width >= 64)
			return x <= (uint64_t)INT64_MAX;
		return x <= (uint64_t)((1LL << (width - 1)) - 1);
	}
	int64_t x = v.as.i;
	if (is_unsigned) {
		if (x < 0)
			return false;
		return width >= 64 || (uint64_t)x <= (1ULL << width) - 1;
	}
	if (width >= 64)
		return true;
	return x >= -(1LL << (width - 1)) && x <= (1LL << (width - 1)) - 1;
}

bool ob_is_bool(OboeValue v)
{
	return v.tag == OB_BOOL;
}
bool ob_is_string(OboeValue v)
{
	return v.tag == OB_STRING;
}
bool ob_is_array(OboeValue v)
{
	return v.tag == OB_ARRAY;
}
bool ob_is_dict(OboeValue v)
{
	return v.tag == OB_DICT;
}
bool ob_is_null(OboeValue v)
{
	return v.tag == OB_NULL;
}

bool ob_is_object_of(OboeValue v, const OboeClassInfo *cls)
{
	if (v.tag != OB_OBJECT)
		return false;
	const OboeClassInfo *actual = ((OboeObject *)v.as.obj)->cls;
	while (actual) {
		if (actual == cls)
			return true;
		actual = actual->parent;
	}
	return false;
}

OboeValue ob_range(int64_t a, int64_t b)
{
	OboeValue r = ob_array_new();
	for (int64_t i = a; i < b; i++)
		ob_array_push(r, ob_int(i));
	return r;
}

OboeValue ob_args_from_argv(int argc, char **argv)
{
	OboeValue r = ob_array_new();
	for (int i = 0; i < argc; i++)
		ob_array_push(r, ob_string(argv[i]));
	return r;
}

void ob_throw(const char *type_name, OboeValue payload)
{
	ob_current_exception_type = strdup(type_name);
	ob_current_exception = payload;
	if (!ob_exc_stack) {
		/* report the payload too, so `throw ValueError("why")` isn't silently dropped */
		if (payload.tag == OB_NULL) {
			fprintf(stderr, "oboe: uncaught exception %s\n",
				type_name);
		} else {
			char *msg = ob_to_cstr(payload);
			fprintf(stderr, "oboe: uncaught exception %s: %s\n",
				type_name, msg);
			free(msg);
		}
		exit(1);
	}
	OboeExceptionFrame *f = ob_exc_stack;
	ob_exc_stack = f->prev;
	longjmp(f->buf, 1);
}

bool ob_exception_matches(const char *type_name)
{
	if (strcmp(type_name, "Exception") == 0)
		return true;
	return ob_current_exception_type &&
	       strcmp(ob_current_exception_type, type_name) == 0;
}

/* ---- built-in stdlib modules: math ---- */

static int64_t ob_want_int(OboeValue v, const char *what)
{
	if (v.tag != OB_INT) {
		fprintf(stderr, "oboe: %s expects an int\n", what);
		exit(1);
	}
	return v.as.i;
}

/* math keeps its exact integer behavior when given ints, and switches to
   floating point as soon as any argument is a float, so `math.sqrt(2)` is still
   the floor square root while `math.sqrt(2.0)` is 1.414... */
static double ob_want_num(OboeValue v, const char *what)
{
	if (!ob_is_num(v)) {
		fprintf(stderr, "oboe: %s expects a number\n", what);
		exit(1);
	}
	return ob_as_double(v);
}

OboeValue ob_std_math_abs(OboeValue a)
{
	if (a.tag == OB_FLOAT)
		return ob_float(fabs(a.as.f));
	int64_t v = ob_want_int(a, "math.abs");
	return ob_int(v < 0 ? -v : v);
}

OboeValue ob_std_math_min(OboeValue a, OboeValue b)
{
	if (ob_either_float(a, b)) {
		double x = ob_want_num(a, "math.min"),
		       y = ob_want_num(b, "math.min");
		return ob_float(x < y ? x : y);
	}
	int64_t x = ob_want_int(a, "math.min"), y = ob_want_int(b, "math.min");
	return ob_int(x < y ? x : y);
}

OboeValue ob_std_math_max(OboeValue a, OboeValue b)
{
	if (ob_either_float(a, b)) {
		double x = ob_want_num(a, "math.max"),
		       y = ob_want_num(b, "math.max");
		return ob_float(x > y ? x : y);
	}
	int64_t x = ob_want_int(a, "math.max"), y = ob_want_int(b, "math.max");
	return ob_int(x > y ? x : y);
}

OboeValue ob_std_math_pow(OboeValue a, OboeValue b)
{
	if (ob_either_float(a, b))
		return ob_float(pow(ob_want_num(a, "math.pow"),
				    ob_want_num(b, "math.pow")));
	int64_t base = ob_want_int(a, "math.pow"),
		exp = ob_want_int(b, "math.pow");
	if (exp < 0)
		ob_throw("ValueError",
			 ob_string("math.pow: negative exponent"));
	int64_t r = 1;
	while (exp > 0) {
		if (exp & 1)
			r *= base;
		base *= base;
		exp >>= 1;
	}
	return ob_int(r);
}

OboeValue ob_std_math_sqrt(OboeValue a)
{
	if (a.tag == OB_FLOAT) {
		if (a.as.f < 0)
			ob_throw("ValueError",
				 ob_string("math.sqrt: negative argument"));
		return ob_float(sqrt(a.as.f));
	}
	int64_t n = ob_want_int(a, "math.sqrt");
	if (n < 0)
		ob_throw("ValueError",
			 ob_string("math.sqrt: negative argument"));
	int64_t r = 0;
	while ((r + 1) * (r + 1) <= n)
		r++;
	return ob_int(r);
}

/* floor/ceil/round answer "which integer", so they return an int for both int
   and float input rather than a float that happens to be integral */
OboeValue ob_std_math_floor(OboeValue a)
{
	if (a.tag == OB_INT)
		return a;
	return ob_int((int64_t)floor(ob_want_num(a, "math.floor")));
}

OboeValue ob_std_math_ceil(OboeValue a)
{
	if (a.tag == OB_INT)
		return a;
	return ob_int((int64_t)ceil(ob_want_num(a, "math.ceil")));
}

OboeValue ob_std_math_round(OboeValue a)
{
	if (a.tag == OB_INT)
		return a;
	return ob_int((int64_t)round(ob_want_num(a, "math.round")));
}

/* ---- built-in stdlib modules: random (xorshift64*, platform-independent) ---- */

static uint64_t ob_rng_state = 0x9E3779B97F4A7C15ULL;

static uint64_t ob_rng_next(void)
{
	uint64_t x = ob_rng_state;
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	ob_rng_state = x;
	return x * 0x2545F4914F6CDD1DULL;
}

OboeValue ob_std_random_seed(OboeValue a)
{
	int64_t s = ob_want_int(a, "random.seed");
	ob_rng_state = (uint64_t)s ? (uint64_t)s : 0x9E3779B97F4A7C15ULL;
	return ob_null();
}

OboeValue ob_std_random_randint(OboeValue a, OboeValue b)
{
	int64_t lo = ob_want_int(a, "random.randint"),
		hi = ob_want_int(b, "random.randint");
	if (hi < lo)
		ob_throw(
			"ValueError",
			ob_string(
				"random.randint: upper bound below lower bound"));
	uint64_t span = (uint64_t)(hi - lo) + 1;
	return ob_int(lo + (int64_t)(ob_rng_next() % span));
}

OboeValue ob_std_random_choice(OboeValue arr)
{
	if (arr.tag != OB_ARRAY || arr.as.arr->count == 0) {
		fprintf(stderr,
			"oboe: random.choice expects a non-empty array\n");
		exit(1);
	}
	return arr.as.arr->items[ob_rng_next() % arr.as.arr->count];
}

/* ---- built-in stdlib modules: os ---- */

OboeValue ob_std_os_run(OboeValue cmd)
{
	char *c = ob_to_cstr(cmd);
	int status = system(c);
	free(c);
#ifdef _WIN32
	return ob_int(status);
#else
	return ob_int(WIFEXITED(status) ? WEXITSTATUS(status) : -1);
#endif
}

OboeValue ob_std_os_spawn(OboeValue cmd)
{
	char *c = ob_to_cstr(cmd);
#ifdef _WIN32
	char *full = malloc(strlen(c) + 16);
	sprintf(full, "cmd /c %s", c);
	STARTUPINFOA si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof si;
	if (!CreateProcessA(NULL, full, NULL, NULL, FALSE, 0, NULL, NULL, &si,
			    &pi)) {
		free(full);
		free(c);
		ob_throw("os.ProcessError", ob_string("cannot spawn process"));
	}
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	free(full);
	free(c);
	return ob_int(pi.dwProcessId);
#else
	pid_t pid = fork();
	if (pid < 0) {
		free(c);
		ob_throw("os.ProcessError", ob_string("cannot spawn process"));
	}
	if (pid == 0) {
		execl("/bin/sh", "sh", "-c", c, (char *)NULL);
		_exit(127);
	}
	free(c);
	return ob_int(pid);
#endif
}

OboeValue ob_std_os_read_file(OboeValue path)
{
	char *p = ob_to_cstr(path);
	FILE *f = fopen(p, "rb");
	if (!f) {
		OboeValue msg = ob_string(p);
		free(p);
		ob_throw("os.FileNotFoundError", msg);
	}
	free(p);
	fseek(f, 0, SEEK_END);
	long sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = malloc(sz + 1);
	size_t n = fread(buf, 1, sz, f);
	buf[n] = '\0';
	fclose(f);
	return ob_string_take(buf);
}

static OboeValue ob_os_write(OboeValue path, OboeValue content,
			     const char *mode)
{
	char *p = ob_to_cstr(path);
	FILE *f = fopen(p, mode);
	if (!f) {
		OboeValue msg = ob_string(p);
		free(p);
		ob_throw("os.FileError", msg);
	}
	free(p);
	char *c = ob_to_cstr(content);
	fputs(c, f);
	free(c);
	fclose(f);
	return ob_null();
}

OboeValue ob_std_os_write_file(OboeValue path, OboeValue content)
{
	return ob_os_write(path, content, "wb");
}
OboeValue ob_std_os_append_file(OboeValue path, OboeValue content)
{
	return ob_os_write(path, content, "ab");
}

OboeValue ob_std_os_exists(OboeValue path)
{
	char *p = ob_to_cstr(path);
	struct stat st;
	bool ok = stat(p, &st) == 0;
	free(p);
	return ob_bool(ok);
}

OboeValue ob_std_os_remove(OboeValue path)
{
	char *p = ob_to_cstr(path);
	bool ok = remove(p) == 0;
	free(p);
	return ob_bool(ok);
}

/* The canonical absolute path, symlinks and all resolved, or null when the
   path cannot be resolved (it does not exist, or a component is not a
   directory). A compiler that embeds a source path into what it emits needs
   one spelling of that path regardless of how the caller wrote it. */
OboeValue ob_std_os_realpath(OboeValue path)
{
	char *p = ob_to_cstr(path);
	char buf[PATH_MAX];
	char *got = ob_realpath(p, buf);
	free(p);
	return got ? ob_string(got) : ob_null();
}

OboeValue ob_std_os_is_dir(OboeValue path)
{
	char *p = ob_to_cstr(path);
	struct stat st;
	bool ok = stat(p, &st) == 0 && S_ISDIR(st.st_mode);
	free(p);
	return ob_bool(ok);
}

/* mkdir -p: makes every missing directory along the path. An existing directory
   is success, so this is safe to call on a path that is already there. */
OboeValue ob_std_os_mkdir(OboeValue path)
{
	char *p = ob_to_cstr(path);
	for (char *s = p + 1; *s; s++) {
		if (*s != '/')
			continue;
		*s = '\0';
		ob_mkdir_one(p);
		*s = '/';
	}
	bool ok = ob_mkdir_one(p) == 0;
	if (!ok) {
		struct stat st;
		ok = stat(p, &st) == 0 && S_ISDIR(st.st_mode);
	}
	free(p);
	return ob_bool(ok);
}

/* The entry names in `path`, without "." and "..", sorted by byte order.
   readdir's own order is whatever the filesystem feels like, and a compiler
   resolving modules against a directory listing has to be deterministic. */
OboeValue ob_std_os_listdir(OboeValue path)
{
	char *p = ob_to_cstr(path);
	DIR *dir = opendir(p);
	if (!dir) {
		OboeValue msg = ob_string(p);
		free(p);
		ob_throw("os.FileNotFoundError", msg);
	}
	free(p);
	OboeValue out = ob_array_new();
	struct dirent *e;
	while ((e = readdir(dir))) {
		if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
			continue;
		ob_array_push(out, ob_string(e->d_name));
	}
	closedir(dir);
	OboeArray *a = out.as.arr;
	for (size_t i = 1; i < a->count; i++) {
		OboeValue key = a->items[i];
		size_t j = i;
		while (j > 0 && strcmp(a->items[j - 1].as.s, key.as.s) > 0) {
			a->items[j] = a->items[j - 1];
			j--;
		}
		a->items[j] = key;
	}
	return out;
}

OboeValue ob_std_os_exit(OboeValue code)
{
	if (code.tag != OB_INT)
		ob_throw("TypeError",
			 ob_string("os.exit() expects an int status"));
	exit((int)code.as.i);
}

OboeValue ob_std_os_getenv(OboeValue name)
{
	char *n = ob_to_cstr(name);
	const char *v = getenv(n);
	free(n);
	return v ? ob_string(v) : ob_null();
}

/* ---- iteration ----
   One dispatch point behind every `for` form. Strings iterate by byte, matching
   the rest of the runtime's byte-oriented string handling; multi-byte UTF-8
   characters therefore come back one byte at a time. */

static void ob_iter_type_error(OboeValue v)
{
	(void)v;
	ob_throw(
		"TypeError",
		ob_string(
			"value is not iterable (expected an array, dict or string)"));
}

int64_t ob_iter_len(OboeValue v)
{
	switch (v.tag) {
	case OB_ARRAY:
		return (int64_t)v.as.arr->count;
	case OB_DICT:
		return (int64_t)v.as.dict->count;
	case OB_STRING:
		return (int64_t)ob_slen(v.as.s);
	default:
		ob_iter_type_error(v);
		return 0;
	}
}

OboeValue ob_iter_key(OboeValue v, int64_t i)
{
	if (v.tag == OB_DICT) {
		if (i < 0 || (size_t)i >= v.as.dict->count)
			return ob_null();
		return ob_string(v.as.dict->entries[i].key);
	}
	if (v.tag == OB_ARRAY || v.tag == OB_STRING)
		return ob_int(i);
	ob_iter_type_error(v);
	return ob_null();
}

OboeValue ob_iter_value(OboeValue v, int64_t i)
{
	switch (v.tag) {
	case OB_ARRAY:
		if (i < 0 || (size_t)i >= v.as.arr->count)
			return ob_null();
		return v.as.arr->items[i];
	case OB_DICT:
		if (i < 0 || (size_t)i >= v.as.dict->count)
			return ob_null();
		return v.as.dict->entries[i].value;
	case OB_STRING: {
		if (i < 0 || (size_t)i >= ob_slen(v.as.s))
			return ob_null();
		char one[2] = { v.as.s[i], '\0' };
		return ob_string(one);
	}
	default:
		ob_iter_type_error(v);
		return ob_null();
	}
}

/* ---- methods on primitives ----
   The compiler can't know a primitive's type statically, so each method
   tag-checks its receiver here and throws a catchable TypeError on a mismatch,
   rather than the compiler rejecting the call. */

static void ob_method_type_error(const char *method, const char *want)
{
	char buf[160];
	snprintf(buf, sizeof buf, "%s() expects %s", method, want);
	ob_throw("TypeError", ob_string(buf));
}

static const char *ob_want_str(OboeValue v, const char *method)
{
	if (v.tag != OB_STRING)
		ob_method_type_error(method, "a string");
	return v.as.s;
}

static OboeArray *ob_want_arr(OboeValue v, const char *method)
{
	if (v.tag != OB_ARRAY)
		ob_method_type_error(method, "an array");
	return v.as.arr;
}

static OboeDict *ob_want_dict(OboeValue v, const char *method)
{
	if (v.tag != OB_DICT)
		ob_method_type_error(method, "a dict");
	return v.as.dict;
}

static int64_t ob_want_idx(OboeValue v, const char *method)
{
	if (v.tag != OB_INT)
		ob_method_type_error(method, "an integer index");
	return v.as.i;
}

/* ---- string methods ---- */

static OboeValue ob_str_len(OboeValue s)
{
	return ob_int((int64_t)ob_slen(ob_want_str(s, "len")));
}

OboeValue ob_str_upper(OboeValue s)
{
	char *out = strdup(ob_want_str(s, "upper"));
	for (char *p = out; *p; p++)
		*p = (char)toupper((unsigned char)*p);
	return ob_string_take(out);
}

OboeValue ob_str_lower(OboeValue s)
{
	char *out = strdup(ob_want_str(s, "lower"));
	for (char *p = out; *p; p++)
		*p = (char)tolower((unsigned char)*p);
	return ob_string_take(out);
}

static OboeValue ob_str_reverse(OboeValue s)
{
	const char *in = ob_want_str(s, "reverse");
	size_t n = ob_slen(in);
	char *out = malloc(n + 1);
	for (size_t i = 0; i < n; i++)
		out[i] = in[n - 1 - i];
	out[n] = '\0';
	return ob_string_take(out);
}

OboeValue ob_str_trim(OboeValue s)
{
	const char *in = ob_want_str(s, "trim");
	while (*in && isspace((unsigned char)*in))
		in++;
	size_t n = strlen(in);
	while (n > 0 && isspace((unsigned char)in[n - 1]))
		n--;
	char *out = malloc(n + 1);
	memcpy(out, in, n);
	out[n] = '\0';
	return ob_string_take(out);
}

/* An empty separator splits into individual characters, mirroring how `for`
   iterates a string; otherwise adjacent separators produce empty pieces. */
OboeValue ob_str_split(OboeValue s, OboeValue sep)
{
	const char *in = ob_want_str(s, "split");
	const char *sp = ob_want_str(sep, "split");
	OboeValue out = ob_array_new();
	size_t splen = ob_slen(sp);
	if (splen == 0) {
		for (const char *p = in; *p; p++) {
			char one[2] = { *p, '\0' };
			ob_array_push(out, ob_string(one));
		}
		return out;
	}
	const char *start = in;
	for (;;) {
		const char *hit = strstr(start, sp);
		if (!hit) {
			ob_array_push(out, ob_string(start));
			break;
		}
		char *piece = ob_strndup(start, (size_t)(hit - start));
		ob_array_push(out, ob_string_take(piece));
		start = hit + splen;
	}
	return out;
}

OboeValue ob_str_starts_with(OboeValue s, OboeValue prefix)
{
	const char *in = ob_want_str(s, "starts_with");
	const char *p = ob_want_str(prefix, "starts_with");
	return ob_bool(strncmp(in, p, ob_slen(p)) == 0);
}

OboeValue ob_str_ends_with(OboeValue s, OboeValue suffix)
{
	const char *in = ob_want_str(s, "ends_with");
	const char *p = ob_want_str(suffix, "ends_with");
	size_t n = ob_slen(in), m = ob_slen(p);
	return ob_bool(m <= n && strcmp(in + n - m, p) == 0);
}

static OboeValue ob_str_contains(OboeValue s, OboeValue needle)
{
	const char *in = ob_want_str(s, "contains");
	return ob_bool(strstr(in, ob_want_str(needle, "contains")) != NULL);
}

static OboeValue ob_str_index_of(OboeValue s, OboeValue needle)
{
	const char *in = ob_want_str(s, "index_of");
	const char *hit = strstr(in, ob_want_str(needle, "index_of"));
	return ob_int(hit ? (int64_t)(hit - in) : -1);
}

OboeValue ob_str_replace(OboeValue s, OboeValue from, OboeValue to)
{
	const char *in = ob_want_str(s, "replace");
	const char *f = ob_want_str(from, "replace");
	const char *t = ob_want_str(to, "replace");
	size_t flen = ob_slen(f);
	if (flen == 0)
		return ob_string(in);
	size_t tlen = ob_slen(t), cap = ob_slen(in) + 1, n = 0;
	char *out = malloc(cap);
	for (const char *p = in; *p;) {
		const char *hit = strstr(p, f);
		size_t chunk = hit ? (size_t)(hit - p) : strlen(p);
		size_t need = n + chunk + tlen + 1;
		if (need > cap) {
			cap = need * 2;
			out = realloc(out, cap);
		}
		memcpy(out + n, p, chunk);
		n += chunk;
		if (!hit) {
			p += chunk;
			break;
		}
		memcpy(out + n, t, tlen);
		n += tlen;
		p = hit + flen;
	}
	out[n] = '\0';
	return ob_string_take(out);
}

/* out-of-range starts and lengths clamp rather than throwing, so slicing near
   the end of a string doesn't need a guard at every call site */
OboeValue ob_str_substr(OboeValue s, OboeValue start, OboeValue len)
{
	const char *in = ob_want_str(s, "substr");
	int64_t n = (int64_t)ob_slen(in);
	int64_t a = ob_want_idx(start, "substr");
	int64_t l = ob_want_idx(len, "substr");
	if (a < 0)
		a = 0;
	if (a > n)
		a = n;
	if (l < 0)
		l = 0;
	if (a + l > n)
		l = n - a;
	char *out = ob_str_alloc((size_t)l);
	memcpy(out, in + a, (size_t)l);
	return ob_string_wrap(out);
}

OboeValue ob_str_repeat(OboeValue s, OboeValue n)
{
	return ob_repeat(ob_string(ob_want_str(s, "repeat")), n);
}

OboeValue ob_str_to_int(OboeValue s)
{
	const char *in = ob_want_str(s, "to_int");
	char *end;
	long long v = strtoll(in, &end, 10);
	while (*end && isspace((unsigned char)*end))
		end++;
	if (end == in || *end)
		ob_throw("ValueError", ob_string(in));
	return ob_int((int64_t)v);
}

OboeValue ob_str_to_float(OboeValue s)
{
	const char *in = ob_want_str(s, "to_float");
	char *end;
	double v = strtod(in, &end);
	while (*end && isspace((unsigned char)*end))
		end++;
	if (end == in || *end)
		ob_throw("ValueError", ob_string(in));
	return ob_float(v);
}

/* ---- array methods ----
   Arrays are held by pointer, so push/pop/insert/remove_at mutate the array the
   receiver names rather than a copy; reverse and slice return new arrays. */

static OboeValue ob_arr_len(OboeValue a)
{
	return ob_int((int64_t)ob_want_arr(a, "len")->count);
}

OboeValue ob_arr_push(OboeValue a, OboeValue v)
{
	ob_want_arr(a, "push");
	ob_array_push(a, v);
	return a;
}

OboeValue ob_arr_pop(OboeValue a)
{
	OboeArray *arr = ob_want_arr(a, "pop");
	if (arr->count == 0)
		ob_throw("ValueError", ob_string("pop() from an empty array"));
	return arr->items[--arr->count];
}

OboeValue ob_arr_insert(OboeValue a, OboeValue idx, OboeValue v)
{
	OboeArray *arr = ob_want_arr(a, "insert");
	int64_t i = ob_want_idx(idx, "insert");
	if (i < 0)
		i = 0;
	if (i > (int64_t)arr->count)
		i = (int64_t)arr->count;
	ob_array_push(a, v); /* grows the backing store */
	for (int64_t j = (int64_t)arr->count - 1; j > i; j--)
		arr->items[j] = arr->items[j - 1];
	arr->items[i] = v;
	return a;
}

OboeValue ob_arr_remove_at(OboeValue a, OboeValue idx)
{
	OboeArray *arr = ob_want_arr(a, "remove_at");
	int64_t i = ob_want_idx(idx, "remove_at");
	ob_bounds_check(arr, i);
	OboeValue gone = arr->items[i];
	for (size_t j = (size_t)i; j + 1 < arr->count; j++)
		arr->items[j] = arr->items[j + 1];
	arr->count--;
	return gone;
}

static OboeValue ob_arr_index_of(OboeValue a, OboeValue v)
{
	OboeArray *arr = ob_want_arr(a, "index_of");
	for (size_t i = 0; i < arr->count; i++)
		if (ob_raw_eq(arr->items[i], v))
			return ob_int((int64_t)i);
	return ob_int(-1);
}

static OboeValue ob_arr_contains(OboeValue a, OboeValue v)
{
	OboeArray *arr = ob_want_arr(a, "contains");
	for (size_t i = 0; i < arr->count; i++)
		if (ob_raw_eq(arr->items[i], v))
			return ob_bool(true);
	return ob_bool(false);
}

OboeValue ob_arr_join(OboeValue a, OboeValue sep)
{
	OboeArray *arr = ob_want_arr(a, "join");
	const char *sp = ob_want_str(sep, "join");
	size_t splen = ob_slen(sp), cap = 64, n = 0;
	char *out = malloc(cap);
	out[0] = '\0';
	for (size_t i = 0; i < arr->count; i++) {
		char *piece = ob_to_cstr(arr->items[i]);
		size_t plen = strlen(piece);
		size_t need = n + plen + splen + 1;
		if (need > cap) {
			cap = need * 2;
			out = realloc(out, cap);
		}
		if (i > 0) {
			memcpy(out + n, sp, splen);
			n += splen;
		}
		memcpy(out + n, piece, plen);
		n += plen;
		free(piece);
	}
	out[n] = '\0';
	return ob_string_take(out);
}

static OboeValue ob_arr_reverse(OboeValue a)
{
	OboeArray *arr = ob_want_arr(a, "reverse");
	OboeValue out = ob_array_new();
	for (size_t i = arr->count; i > 0; i--)
		ob_array_push(out, arr->items[i - 1]);
	return out;
}

/* half-open [start, end), clamped, like ob_str_substr */
static OboeValue ob_arr_slice(OboeValue a, OboeValue start, OboeValue end)
{
	OboeArray *arr = ob_want_arr(a, "slice");
	int64_t n = (int64_t)arr->count;
	int64_t s = ob_want_idx(start, "slice"), e = ob_want_idx(end, "slice");
	if (s < 0)
		s = 0;
	if (e > n)
		e = n;
	OboeValue out = ob_array_new();
	for (int64_t i = s; i < e; i++)
		ob_array_push(out, arr->items[i]);
	return out;
}

/* ---- dict methods ---- */

static OboeValue ob_dict_len(OboeValue d)
{
	return ob_int((int64_t)ob_want_dict(d, "len")->count);
}

OboeValue ob_dict_keys(OboeValue d)
{
	OboeDict *dict = ob_want_dict(d, "keys");
	OboeValue out = ob_array_new();
	for (size_t i = 0; i < dict->count; i++)
		ob_array_push(out, ob_string(dict->entries[i].key));
	return out;
}

OboeValue ob_dict_values(OboeValue d)
{
	OboeDict *dict = ob_want_dict(d, "values");
	OboeValue out = ob_array_new();
	for (size_t i = 0; i < dict->count; i++)
		ob_array_push(out, dict->entries[i].value);
	return out;
}

OboeValue ob_dict_has_m(OboeValue d, OboeValue key)
{
	ob_want_dict(d, "has");
	char *k = ob_to_cstr(key);
	bool found = ob_dict_has(d, k);
	free(k);
	return ob_bool(found);
}

OboeValue ob_dict_remove(OboeValue d, OboeValue key)
{
	OboeDict *dict = ob_want_dict(d, "remove");
	char *k = ob_to_cstr(key);
	ptrdiff_t at = ob_dict_find(dict, k);
	free(k);
	if (at < 0)
		return ob_null();
	OboeValue gone = dict->entries[at].value;
	free(dict->entries[at].key);
	/* closing the gap keeps insertion order, at the cost of every position
	   in the index past `at`; rebuilding is the same O(n) as the shift */
	for (size_t j = (size_t)at; j + 1 < dict->count; j++)
		dict->entries[j] = dict->entries[j + 1];
	dict->count--;
	if (dict->index)
		ob_dict_reindex(dict, dict->index_cap);
	return gone;
}

/* ---- polymorphic methods ----
   A method name shared by several receiver types resolves here, since the
   compiler emits the same call whatever the receiver turns out to be. */

OboeValue ob_m_len(OboeValue v)
{
	switch (v.tag) {
	case OB_STRING:
		return ob_str_len(v);
	case OB_ARRAY:
		return ob_arr_len(v);
	case OB_DICT:
		return ob_dict_len(v);
	default:
		ob_method_type_error("len", "a string, array or dict");
		return ob_null();
	}
}

OboeValue ob_m_contains(OboeValue v, OboeValue needle)
{
	switch (v.tag) {
	case OB_STRING:
		return ob_str_contains(v, needle);
	case OB_ARRAY:
		return ob_arr_contains(v, needle);
	case OB_DICT:
		return ob_dict_has_m(v, needle);
	default:
		ob_method_type_error("contains", "a string, array or dict");
		return ob_null();
	}
}

OboeValue ob_m_index_of(OboeValue v, OboeValue needle)
{
	if (v.tag == OB_STRING)
		return ob_str_index_of(v, needle);
	if (v.tag == OB_ARRAY)
		return ob_arr_index_of(v, needle);
	ob_method_type_error("index_of", "a string or array");
	return ob_null();
}

OboeValue ob_m_reverse(OboeValue v)
{
	if (v.tag == OB_STRING)
		return ob_str_reverse(v);
	if (v.tag == OB_ARRAY)
		return ob_arr_reverse(v);
	ob_method_type_error("reverse", "a string or array");
	return ob_null();
}

/* half-open [start, end) over an array's elements or a string's bytes */
OboeValue ob_m_slice(OboeValue v, OboeValue start, OboeValue end)
{
	if (v.tag == OB_ARRAY)
		return ob_arr_slice(v, start, end);
	if (v.tag == OB_STRING) {
		int64_t n = (int64_t)ob_slen(v.as.s);
		int64_t a = ob_want_idx(start, "slice"),
			b = ob_want_idx(end, "slice");
		if (a < 0)
			a = 0;
		if (b > n)
			b = n;
		return ob_str_substr(v, ob_int(a), ob_int(b > a ? b - a : 0));
	}
	ob_method_type_error("slice", "a string or array");
	return ob_null();
}

OboeValue ob_m_str(OboeValue v)
{
	return ob_str(v);
}
