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
 * canonical copy: reedbed common/izim.c -- edit there, then re-vendor
 */
#include "izim.h"

#include <string.h>

bool izim_valid(const char *s)
{
	if (!s || !(*s >= 'a' && *s <= 'z'))
		return false;
	size_t n = 1;
	for (const char *p = s + 1; *p; p++, n++) {
		if (n >= IZIM_MAX)
			return false;
		if (!((*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') ||
		      *p == '_'))
			return false;
	}
	return true;
}

bool izim_reserved(const char *s)
{
	static const char *const reserved[] = { "math", "random", "os", "oboe",
						NULL };

	for (int i = 0; reserved[i]; i++)
		if (strcmp(s, reserved[i]) == 0)
			return true;
	return false;
}

/* One decimal component: no leading zeros, so "01" is not 1. Advances *p. */
static bool parse_component(const char **p, unsigned long *out)
{
	const char *s = *p;

	if (*s < '0' || *s > '9')
		return false;
	if (*s == '0' && s[1] >= '0' && s[1] <= '9')
		return false;

	unsigned long v = 0;
	int digits = 0;
	while (*s >= '0' && *s <= '9') {
		if (++digits > 9) /* keeps the accumulate below in range */
			return false;
		v = v * 10 + (unsigned long)(*s - '0');
		s++;
	}
	*p = s;
	*out = v;
	return true;
}

bool waktanimra_parse(const char *s, struct waktanimra *out)
{
	if (!s || strlen(s) >= WAKTANIMRA_MAX)
		return false;

	const char *p = s;
	struct waktanimra w;

	memset(&w, 0, sizeof w);
	if (!parse_component(&p, &w.major) || *p++ != '.')
		return false;
	if (!parse_component(&p, &w.minor) || *p++ != '.')
		return false;
	if (!parse_component(&p, &w.patch))
		return false;

	if (*p == '-') {
		p++;
		size_t n = 0;
		for (; *p; p++, n++) {
			if (n >= TAG_MAX - 1)
				return false;
			if (!((*p >= 'a' && *p <= 'z') ||
			      (*p >= 'A' && *p <= 'Z') ||
			      (*p >= '0' && *p <= '9') || *p == '.' ||
			      *p == '-'))
				return false;
			w.tag[n] = *p;
		}
		if (n == 0)
			return false;
		w.tag[n] = '\0';
		w.has_tag = true;
	} else if (*p != '\0') {
		return false;
	}

	if (out)
		*out = w;
	return true;
}

bool waktanimra_valid(const char *s)
{
	return waktanimra_parse(s, NULL);
}

static int cmp_ulong(unsigned long a, unsigned long b)
{
	return a < b ? -1 : (a > b ? 1 : 0);
}

static bool all_digits(const char *s, size_t n)
{
	for (size_t i = 0; i < n; i++)
		if (s[i] < '0' || s[i] > '9')
			return false;
	return n > 0;
}

/* Tags compare field by field on '.': an all-digit field is numeric and sorts
   before any non-numeric one; otherwise ASCII. A prefix sorts first. */
static int cmp_tag(const char *a, const char *b)
{
	while (*a && *b) {
		const char *ae = strchr(a, '.');
		const char *be = strchr(b, '.');
		size_t alen = ae ? (size_t)(ae - a) : strlen(a);
		size_t blen = be ? (size_t)(be - b) : strlen(b);
		bool anum = all_digits(a, alen);
		bool bnum = all_digits(b, blen);

		if (anum != bnum)
			return anum ? -1 : 1;

		if (anum) {
			/* skip leading zeros, then longer means larger */
			size_t ai = 0, bi = 0;
			while (ai + 1 < alen && a[ai] == '0')
				ai++;
			while (bi + 1 < blen && b[bi] == '0')
				bi++;
			size_t an = alen - ai, bn = blen - bi;
			if (an != bn)
				return an < bn ? -1 : 1;
			int c = memcmp(a + ai, b + bi, an);
			if (c)
				return c < 0 ? -1 : 1;
		} else {
			size_t n = alen < blen ? alen : blen;
			int c = memcmp(a, b, n);
			if (c)
				return c < 0 ? -1 : 1;
			if (alen != blen)
				return alen < blen ? -1 : 1;
		}

		a = ae ? ae + 1 : a + alen;
		b = be ? be + 1 : b + blen;
		if (*a == '\0' && *b == '\0')
			return 0;
		if (*a == '\0')
			return -1;
		if (*b == '\0')
			return 1;
	}
	if (*a)
		return 1;
	if (*b)
		return -1;
	return 0;
}

int waktanimra_cmp(const struct waktanimra *a, const struct waktanimra *b)
{
	int c;

	if ((c = cmp_ulong(a->major, b->major)))
		return c;
	if ((c = cmp_ulong(a->minor, b->minor)))
		return c;
	if ((c = cmp_ulong(a->patch, b->patch)))
		return c;

	/* a tagged version precedes the release it leads up to */
	if (a->has_tag != b->has_tag)
		return a->has_tag ? -1 : 1;
	if (!a->has_tag)
		return 0;
	return cmp_tag(a->tag, b->tag);
}

int waktanimra_cmp_str(const char *a, const char *b)
{
	struct waktanimra wa, wb;

	if (!waktanimra_parse(a, &wa) || !waktanimra_parse(b, &wb))
		return 0;
	return waktanimra_cmp(&wa, &wb);
}

bool constraint_parse(const char *s, struct constraint *out)
{
	struct constraint c;

	if (!s || !*s)
		return false;

	memset(&c, 0, sizeof c);
	if (strcmp(s, "*") == 0) {
		c.kind = CONSTRAINT_ANY;
		if (out)
			*out = c;
		return true;
	}

	const char *rest;
	if (s[0] == '>' && s[1] == '=') {
		c.kind = CONSTRAINT_GE;
		rest = s + 2;
	} else if (s[0] == '=') {
		c.kind = CONSTRAINT_EQ;
		rest = s + 1;
	} else if (s[0] == '^') {
		c.kind = CONSTRAINT_CARET;
		rest = s + 1;
	} else {
		return false;
	}

	if (!waktanimra_parse(rest, &c.base))
		return false;
	if (out)
		*out = c;
	return true;
}

bool constraint_valid(const char *s)
{
	return constraint_parse(s, NULL);
}

/* Upper bound of ^X.Y.Z: the next value of the leftmost non-zero component, so
   ^1.2.3 stops at 2.0.0 but ^0.2.3 stops at 0.3.0 and ^0.0.3 at 0.0.4. Below
   1.0 every release is potentially breaking, which is what Cargo encodes and
   what most of a young registry's packages will be. */
static void caret_upper(const struct waktanimra *b, struct waktanimra *out)
{
	memset(out, 0, sizeof *out);
	if (b->major != 0) {
		out->major = b->major + 1;
	} else if (b->minor != 0) {
		out->minor = b->minor + 1;
	} else {
		out->patch = b->patch + 1;
	}
}

bool constraint_match(const struct constraint *c, const struct waktanimra *v)
{
	if (v->has_tag) {
		/* opt-in only, and only for the exact release being led up to */
		if (c->kind == CONSTRAINT_ANY)
			return false;
		if (!c->base.has_tag)
			return false;
		if (c->base.major != v->major || c->base.minor != v->minor ||
		    c->base.patch != v->patch)
			return false;
	}

	switch (c->kind) {
	case CONSTRAINT_ANY:
		return true;
	case CONSTRAINT_EQ:
		return waktanimra_cmp(v, &c->base) == 0;
	case CONSTRAINT_GE:
		return waktanimra_cmp(v, &c->base) >= 0;
	case CONSTRAINT_CARET: {
		struct waktanimra upper;

		caret_upper(&c->base, &upper);
		return waktanimra_cmp(v, &c->base) >= 0 &&
		       waktanimra_cmp(v, &upper) < 0;
	}
	}
	return false;
}
