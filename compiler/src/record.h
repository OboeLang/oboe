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
 * canonical copy: reedbed common/record.h -- edit there, then re-vendor
 */
#ifndef KATARE_RECORD_H
#define KATARE_RECORD_H

#include <stdbool.h>
#include <stddef.h>

/* `key: value` record bodies -- KATARE.md §6.

   A body holds one or more records separated by a blank line; each record is a
   run of `key: value` lines terminated by LF. Values may hold UTF-8, colons and
   spaces, so a value runs to end of line and is never re-split. */

#define RECORD_KEY_MAX 32

struct record_field {
	char key[RECORD_KEY_MAX];
	char *value; /* malloc'd, NUL-terminated */
};

struct record {
	struct record_field *fields;
	int n, cap;
};

struct record_set {
	struct record *records;
	int n, cap;
};

void record_free(struct record *r);
void record_set_free(struct record_set *rs);

/* Parses one body of `len` octets into `out`. Returns false on a malformed line
   (no ": " separator, empty key, empty value, a key that is not all lowercase
   ASCII, or an over-long key), leaving `out` freed and zeroed.

   A trailing blank line is tolerated; a body of zero octets parses to zero
   records, which is what `ko jexa` with no matches sends. */
bool record_set_parse(const char *body, size_t len, struct record_set *out);

/* First value for `key`, or NULL. */
const char *record_get(const struct record *r, const char *key);

/* Walks the repeated values of `key`. Pass *iter = 0 to start; returns NULL when
   exhausted. Used for `cizujo`, the one repeatable key. */
const char *record_next(const struct record *r, const char *key, int *iter);

/* Appends a field. `value` is copied. Returns false only on allocation failure
   or an invalid key. */
bool record_put(struct record *r, const char *key, const char *value);

/* Serialises one record as `key: value` LF lines, with no trailing blank line.
   Returns a malloc'd buffer and writes its length to *len. */
char *record_format(const struct record *r, size_t *len);

/* True when the record carries every key KATARE.md §6 marks required. */
bool record_complete(const struct record *r);

#endif
