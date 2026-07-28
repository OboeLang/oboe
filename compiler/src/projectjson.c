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
 *
 * canonical copy: reedbed common/projectjson.c -- edit there, then re-vendor
 */
#include "projectjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *pj_read_file(const char *path)
{
	FILE *f = fopen(path, "rb");
	if (!f)
		return NULL;
	fseek(f, 0, SEEK_END);
	long sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = malloc(sz + 1);
	if (!buf) {
		fclose(f);
		return NULL;
	}
	size_t n = fread(buf, 1, sz, f);
	buf[n] = '\0';
	fclose(f);
	return buf;
}

static const char *skip_ws(const char *p)
{
	while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
		p++;
	return p;
}

/* Walks the keys of an object body, calling `visit` for each one that sits at
   the body's own nesting level. Depth awareness is what keeps a nested
   `build.targets.windows.target` from answering a lookup of `build.target`.
   `key`/`key_len` name the key and `value` points at its first character. */
typedef void (*KeyVisitor)(const char *key, size_t key_len, const char *value,
			   void *ctx);

static void walk_object_impl(const char *obj, KeyVisitor visit, void *ctx,
			     bool any_depth)
{
	const char *start = skip_ws(obj);
	/* accept a whole document as readily as an object body: without skipping a
       leading brace, a document's own top-level keys would sit at depth 1 and
       every depth-0 lookup against it would miss */
	if (*start == '{')
		start++;
	int depth = 0;
	bool in_string = false, escaped = false;
	const char *key_start = NULL;
	for (const char *p = start; *p; p++) {
		if (in_string) {
			if (escaped) {
				escaped = false;
				continue;
			}
			if (*p == '\\') {
				escaped = true;
				continue;
			}
			if (*p != '"')
				continue;
			in_string = false;
			if ((any_depth || depth == 0) && key_start) {
				const char *after = skip_ws(p + 1);
				if (*after == ':')
					visit(key_start,
					      (size_t)(p - key_start),
					      skip_ws(after + 1), ctx);
			}
			key_start = NULL;
			continue;
		}
		if (*p == '"') {
			in_string = true;
			key_start = p + 1;
			continue;
		} else if (*p == '{' || *p == '[')
			depth++;
		else if (*p == '}' || *p == ']')
			depth--;
		else if (*p == '/' && p[1] == '/') {
			while (p[1] && p[1] != '\n')
				p++;
		}
	}
}

static void walk_object(const char *obj, KeyVisitor visit, void *ctx)
{
	walk_object_impl(obj, visit, ctx, false);
}

typedef struct {
	const char *want;
	const char *found;
} FindCtx;

static void find_visitor(const char *key, size_t key_len, const char *value,
			 void *ctx)
{
	FindCtx *f = ctx;
	if (f->found)
		return;
	if (strlen(f->want) == key_len && strncmp(key, f->want, key_len) == 0)
		f->found = value;
}

/* first character of the value for a key at this object's own level, or NULL */
static const char *find_value(const char *obj, const char *field)
{
	FindCtx ctx = { field, NULL };
	walk_object(obj, find_visitor, &ctx);
	return ctx.found;
}

static char *read_string_value(const char *p)
{
	if (!p || *p != '"')
		return NULL;
	p++;
	const char *end = strchr(p, '"');
	if (!end)
		return NULL;
	return strndup(p, end - p);
}

/* contents of an object value, brace-delimited characters excluded */
static char *read_object_value(const char *p)
{
	if (!p || *p != '{')
		return NULL;
	const char *start = p + 1;
	int depth = 1;
	bool in_string = false;
	const char *q = start;
	for (; *q && depth > 0; q++) {
		if (in_string) {
			if (*q == '\\' && q[1])
				q++;
			else if (*q == '"')
				in_string = false;
			continue;
		}
		if (*q == '"')
			in_string = true;
		else if (*q == '{')
			depth++;
		else if (*q == '}')
			depth--;
	}
	if (depth != 0)
		return NULL;
	return strndup(start, (size_t)(q - start - 1));
}

char *json_get_string(const char *obj, const char *field)
{
	return read_string_value(find_value(obj, field));
}

bool json_get_bool(const char *obj, const char *field)
{
	const char *p = find_value(obj, field);
	return p && strncmp(p, "true", 4) == 0;
}

char *json_extract_object(const char *json, const char *path)
{
	char buf[512];
	snprintf(buf, sizeof buf, "%s", path);
	char *cursor = strdup(json);
	char *save = NULL;
	for (char *part = strtok_r(buf, ".", &save); part;
	     part = strtok_r(NULL, ".", &save)) {
		char *next = read_object_value(find_value(cursor, part));
		free(cursor);
		if (!next)
			return NULL;
		cursor = next;
	}
	return cursor;
}

/* The loose lookup: matches the field anywhere in the document, at any depth.
   Kept for `project.name` / `project.entry`, which callers want to find without
   knowing whether they were nested. Walks strings/comments like walk_object
   does, so a key inside a `//` comment or a string value is never matched. */
static const char *find_value_any_depth(const char *json, const char *field)
{
	FindCtx ctx = { field, NULL };
	walk_object_impl(json, find_visitor, &ctx, true);
	return ctx.found;
}

char *json_extract_string_field(const char *json, const char *field)
{
	return read_string_value(find_value_any_depth(json, field));
}

bool json_extract_bool_field(const char *json, const char *field)
{
	const char *p = find_value_any_depth(json, field);
	return p && strncmp(p, "true", 4) == 0;
}

typedef struct {
	char **keys;
	int count, cap;
} KeysCtx;

static void keys_visitor(const char *key, size_t key_len, const char *value,
			 void *ctx)
{
	(void)value;
	KeysCtx *k = ctx;
	if (k->count == k->cap) {
		k->cap = k->cap ? k->cap * 2 : 4;
		k->keys = realloc(k->keys, k->cap * sizeof(char *));
	}
	k->keys[k->count++] = strndup(key, key_len);
}

char **json_object_keys(const char *obj, int *out_count)
{
	KeysCtx ctx = { NULL, 0, 0 };
	walk_object(obj, keys_visitor, &ctx);
	*out_count = ctx.count;
	return ctx.keys;
}
