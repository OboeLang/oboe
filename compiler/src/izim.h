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
 * canonical copy: reedbed common/izim.h -- edit there, then re-vendor
 */
#ifndef KATARE_IZIM_H
#define KATARE_IZIM_H

#include <stdbool.h>

/* Package names, versions and constraints -- KATARE.md §8.

   These are the rules both ends of the protocol must agree on exactly. A server
   answering `ko cizujo` and a client walking `ko besal` because the server did
   not advertise it MUST reach the same resolution, so this file is shared
   verbatim rather than reimplemented on each side. */

#define IZIM_MAX 64
#define WAKTANIMRA_MAX 64
#define TAG_MAX 56

/* [a-z][a-z0-9_]*, at most IZIM_MAX octets. Also the grammar for account
   names. */
bool izim_valid(const char *s);

/* math, random and os are Oboe runtime built-ins; oboe names the toolchain in a
   project's dependencies. None may be a package. */
bool izim_reserved(const char *s);

struct waktanimra {
	unsigned long major, minor, patch;
	bool has_tag;
	char tag[TAG_MAX];
};

/* MAJOR.MINOR.PATCH[-tag], decimal, no leading zeros. */
bool waktanimra_parse(const char *s, struct waktanimra *out);
bool waktanimra_valid(const char *s);

/* Semver ordering: numeric by component, and a tagged version sorts before the
   otherwise-identical untagged one (1.0.0-rc1 < 1.0.0). Returns <0, 0 or >0. */
int waktanimra_cmp(const struct waktanimra *a, const struct waktanimra *b);

/* Convenience wrapper over parse+cmp for two strings already known valid. */
int waktanimra_cmp_str(const char *a, const char *b);

enum constraint_kind {
	CONSTRAINT_ANY, /* *      */
	CONSTRAINT_EQ, /* =X.Y.Z */
	CONSTRAINT_GE, /* >=X.Y.Z */
	CONSTRAINT_CARET /* ^X.Y.Z */
};

struct constraint {
	enum constraint_kind kind;
	struct waktanimra base; /* unused for CONSTRAINT_ANY */
};

bool constraint_parse(const char *s, struct constraint *out);
bool constraint_valid(const char *s);

/* Does `v` satisfy `c`?

   Prereleases are opt-in: a tagged version matches only when the constraint is
   itself tagged AND names the same MAJOR.MINOR.PATCH. So >=1.0.0 does not admit
   1.1.0-rc1, and neither does >=1.0.0-rc1 -- only a constraint on 1.1.0 itself
   can. Without that second half, one stray prerelease constraint anywhere in a
   dependency graph would quietly opt the whole graph into prereleases. */
bool constraint_match(const struct constraint *c, const struct waktanimra *v);

#endif
