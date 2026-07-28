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
 * canonical copy: reedbed common/projectjson.h -- edit there, then re-vendor
 */
#ifndef KATARE_PROJECTJSON_H
#define KATARE_PROJECTJSON_H

#include <stdbool.h>

/* A minimal targeted scan of project.jsonc rather than a general JSON parser:
   the file's shape is fixed for this toolchain (see project.example.jsonc), and
   the only things ever read out of it are a handful of named scalars plus the
   `build.targets` object's keys. Comments (`//`) and nesting are tolerated only
   as far as those lookups need. */

/* Value of a `"field": "string"` entry anywhere in `json` at any depth,
   malloc'd, or NULL. Used where the caller doesn't care how it was nested. */
char *json_extract_string_field(const char *json, const char *field);
/* True for a `"field": true` entry anywhere in `json`. */
bool json_extract_bool_field(const char *json, const char *field);

/* Depth-aware lookups: match only a key at `obj`'s own level, so a setting in
   `build.targets.<name>` never answers a lookup against `build` itself. */
char *json_get_string(const char *obj, const char *field);
bool json_get_bool(const char *obj, const char *field);

/* Body of the object at a dotted path, e.g. "build.meta" or
   "build.targets.windows" -- malloc'd, brace-delimited contents excluded, or
   NULL when any path component is missing. Lookups inside the returned string
   use the same helpers, which is how nested settings are read. */
char *json_extract_object(const char *json, const char *path);

/* Keys of the object body `obj`, in file order. Returns a malloc'd array of
   malloc'd strings and writes the count; NULL when there are none. */
char **json_object_keys(const char *obj, int *out_count);

/* Reads a whole file into a malloc'd NUL-terminated buffer, or NULL. */
char *pj_read_file(const char *path);

#endif
