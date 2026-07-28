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
 * canonical copy: reedbed common/kabuk.h -- edit there, then re-vendor
 */
#ifndef KATARE_KABUK_H
#define KATARE_KABUK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* The kabuk archive format -- KABUK.md.

   Uncompressed, sorted, no metadata: the same tree always produces the same
   octets, so a publisher can rebuild an archive and confirm it matches the
   digest the registry serves. This file owns the format only; which files go
   into an archive (exclusions, .oboeignore) is the packer's policy and lives
   with the packer. */

#define KABUK_MAGIC "kabuk1\n"
#define KABUK_MAGIC_LEN 7
#define KABUK_TERM "sampura\n"
#define KABUK_TERM_LEN 8
#define KABUK_PATH_MAX 512

struct kabuk_limits {
	size_t max_entries;
	unsigned long long max_file;
	unsigned long long max_total;
};

extern const struct kabuk_limits kabuk_default_limits;

/* Validates one archive path against KABUK.md §3. On failure writes a short
   reason to *err (a static string, never freed).

   This rejects rather than normalises. Normalising a hostile path -- collapsing
   "a/../b", stripping a leading slash -- is where traversal bugs live, because
   the normaliser and the filesystem then disagree about what the path means. */
bool kabuk_path_ok(const char *path, size_t len, const char **err);

/* Octet ordering, for qsort over an array of `const char *`. Deliberately not
   strcoll: a locale-sensitive sort would produce a different archive, and
   therefore a different digest, on a differently configured machine. */
int kabuk_path_cmp(const void *a, const void *b);

struct kabuk_file {
	const char *path; /* path inside the archive */
	const char *fspath; /* where to read it from; NULL means same */
};

int kabuk_file_cmp(const void *a, const void *b);

/* Writes a complete archive. `files` must already be sorted by
   kabuk_file_cmp and free of duplicates; kabuk_write checks both and fails
   rather than silently emitting a non-conforming archive. */
bool kabuk_write(FILE *out, const struct kabuk_file *files, size_t n,
		 char **err);

/* Called once per entry by kabuk_read. Return false to abort the read; set
   *err to say why. */
typedef bool (*kabuk_entry_fn)(const char *path, const unsigned char *data,
			       size_t len, void *ctx, char **err);

/* Walks an in-memory archive, enforcing the grammar, the path rules, the strict
   ordering and `lim`. Pass NULL for `lim` to use kabuk_default_limits, and NULL
   for `fn` to validate without doing anything per entry.

   *err receives a malloc'd message on failure. */
bool kabuk_read(const unsigned char *buf, size_t len,
		const struct kabuk_limits *lim, kabuk_entry_fn fn, void *ctx,
		char **err);

static inline bool kabuk_validate(const unsigned char *buf, size_t len,
				  const struct kabuk_limits *lim, char **err)
{
	return kabuk_read(buf, len, lim, NULL, NULL, err);
}

/* Extracts into `dest`, which must already exist and should be a fresh
   directory. Creates parent directories as needed; refuses to follow a symlink
   at any component; never overwrites an existing file.

   Callers must not extract over a live package directory -- extract to a
   temporary directory, then rename it into place, so a half-written package is
   never visible. */
bool kabuk_extract(const unsigned char *buf, size_t len,
		   const struct kabuk_limits *lim, const char *dest,
		   char **err);

#endif
