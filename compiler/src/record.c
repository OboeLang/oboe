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
 * canonical copy: reedbed common/record.c -- edit there, then re-vendor
 */
#include "record.h"

#include <stdlib.h>
#include <string.h>

void record_free(struct record *r)
{
	for (int i = 0; i < r->n; i++)
		free(r->fields[i].value);
	free(r->fields);
	r->fields = NULL;
	r->n = r->cap = 0;
}

void record_set_free(struct record_set *rs)
{
	for (int i = 0; i < rs->n; i++)
		record_free(&rs->records[i]);
	free(rs->records);
	rs->records = NULL;
	rs->n = rs->cap = 0;
}

static bool key_valid(const char *k, size_t n)
{
	if (n == 0 || n >= RECORD_KEY_MAX)
		return false;
	for (size_t i = 0; i < n; i++)
		if (k[i] < 'a' || k[i] > 'z')
			return false;
	return true;
}

static bool grow_fields(struct record *r)
{
	if (r->n < r->cap)
		return true;
	int cap = r->cap ? r->cap * 2 : 8;
	struct record_field *f = realloc(r->fields, (size_t)cap * sizeof *f);

	if (!f)
		return false;
	r->fields = f;
	r->cap = cap;
	return true;
}

static bool grow_records(struct record_set *rs)
{
	if (rs->n < rs->cap)
		return true;
	int cap = rs->cap ? rs->cap * 2 : 4;
	struct record *p = realloc(rs->records, (size_t)cap * sizeof *p);

	if (!p)
		return false;
	rs->records = p;
	rs->cap = cap;
	return true;
}

bool record_put(struct record *r, const char *key, const char *value)
{
	size_t klen = strlen(key);

	if (!key_valid(key, klen) || !value || !*value)
		return false;
	if (!grow_fields(r))
		return false;

	struct record_field *f = &r->fields[r->n];

	memcpy(f->key, key, klen);
	f->key[klen] = '\0';
	f->value = strdup(value);
	if (!f->value)
		return false;
	r->n++;
	return true;
}

static bool append_field(struct record *r, const char *key, size_t klen,
			 const char *val, size_t vlen)
{
	if (!key_valid(key, klen) || vlen == 0)
		return false;
	if (!grow_fields(r))
		return false;

	struct record_field *f = &r->fields[r->n];

	memcpy(f->key, key, klen);
	f->key[klen] = '\0';
	f->value = malloc(vlen + 1);
	if (!f->value)
		return false;
	memcpy(f->value, val, vlen);
	f->value[vlen] = '\0';
	r->n++;
	return true;
}

bool record_set_parse(const char *body, size_t len, struct record_set *out)
{
	memset(out, 0, sizeof *out);

	struct record cur;
	bool cur_open = false;

	memset(&cur, 0, sizeof cur);

	size_t i = 0;
	while (i < len) {
		size_t j = i;

		while (j < len && body[j] != '\n')
			j++;
		/* a body must be LF-terminated line by line; a final line with
		   no LF is malformed rather than silently accepted */
		if (j == len)
			goto fail;

		size_t linelen = j - i;

		if (linelen == 0) {
			/* blank line: close the record if one is open, and
			   ignore a trailing separator at the very end */
			if (cur_open) {
				if (!grow_records(out))
					goto fail;
				out->records[out->n++] = cur;
				memset(&cur, 0, sizeof cur);
				cur_open = false;
			}
			i = j + 1;
			continue;
		}

		const char *line = body + i;
		const char *colon = memchr(line, ':', linelen);

		if (!colon)
			goto fail;
		size_t klen = (size_t)(colon - line);
		/* exactly one SP after the colon, and something after it */
		if (klen + 2 > linelen || colon[1] != ' ')
			goto fail;

		const char *val = colon + 2;
		size_t vlen = linelen - klen - 2;

		if (!append_field(&cur, line, klen, val, vlen))
			goto fail;
		cur_open = true;
		i = j + 1;
	}

	if (cur_open) {
		if (!grow_records(out))
			goto fail;
		out->records[out->n++] = cur;
		memset(&cur, 0, sizeof cur);
	}
	return true;

fail:
	record_free(&cur);
	record_set_free(out);
	return false;
}

const char *record_get(const struct record *r, const char *key)
{
	for (int i = 0; i < r->n; i++)
		if (strcmp(r->fields[i].key, key) == 0)
			return r->fields[i].value;
	return NULL;
}

const char *record_next(const struct record *r, const char *key, int *iter)
{
	for (int i = *iter; i < r->n; i++) {
		if (strcmp(r->fields[i].key, key) == 0) {
			*iter = i + 1;
			return r->fields[i].value;
		}
	}
	*iter = r->n;
	return NULL;
}

char *record_format(const struct record *r, size_t *len)
{
	size_t total = 0;

	for (int i = 0; i < r->n; i++)
		total += strlen(r->fields[i].key) + 2 +
			 strlen(r->fields[i].value) + 1;

	char *buf = malloc(total + 1);

	if (!buf)
		return NULL;

	size_t n = 0;
	for (int i = 0; i < r->n; i++) {
		size_t klen = strlen(r->fields[i].key);
		size_t vlen = strlen(r->fields[i].value);

		memcpy(buf + n, r->fields[i].key, klen);
		n += klen;
		buf[n++] = ':';
		buf[n++] = ' ';
		memcpy(buf + n, r->fields[i].value, vlen);
		n += vlen;
		buf[n++] = '\n';
	}
	buf[n] = '\0';
	if (len)
		*len = n;
	return buf;
}

bool record_complete(const struct record *r)
{
	static const char *const required[] = { "izim",	 "waktanimra", "warna",
						"ozhon", "sema",       "wakta",
						NULL };

	for (int i = 0; required[i]; i++)
		if (!record_get(r, required[i]))
			return false;
	return true;
}
