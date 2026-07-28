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
#include "projectedit.h"

#include "projectjson.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Writes through a temporary in the same directory and renames. The previous
   version of this code opened the file with "w", which truncates before it
   writes -- a crash there would take the user's project file with it. */
static bool write_atomic(const char *path, const char *text)
{
	char tmp[4096];

	snprintf(tmp, sizeof tmp, "%s.oboe-tmp", path);

	FILE *f = fopen(tmp, "w");

	if (!f) {
		fprintf(stderr, "oboe: cannot write %s\n", tmp);
		return false;
	}
	if (fputs(text, f) == EOF || fflush(f) != 0) {
		fclose(f);
		unlink(tmp);
		fprintf(stderr, "oboe: cannot write %s\n", tmp);
		return false;
	}
	fclose(f);

	if (rename(tmp, path) != 0) {
		unlink(tmp);
		fprintf(stderr, "oboe: cannot replace %s\n", path);
		return false;
	}
	return true;
}

/* Finds the `"dependencies"` object's body, returning offsets of the character
   after its `{` and of its matching `}`. Scans with the same discipline as
   projectjson.c -- strings and // comments are skipped -- rather than using
   json_extract_object, which returns a copy and so loses the offsets an
   in-place edit needs. */
static bool find_deps(const char *json, size_t *open_at, size_t *close_at)
{
	const char *needle = "\"dependencies\"";
	bool in_string = false;

	for (const char *p = json; *p; p++) {
		if (in_string) {
			if (*p == '\\' && p[1])
				p++;
			else if (*p == '"')
				in_string = false;
			continue;
		}
		if (*p == '/' && p[1] == '/') {
			while (*p && *p != '\n')
				p++;
			if (!*p)
				break;
			continue;
		}
		if (*p == '"' && strncmp(p, needle, strlen(needle)) != 0) {
			in_string = true;
			continue;
		}
		if (*p != '"')
			continue;

		/* found the key: step over it, the colon and the brace */
		const char *q = p + strlen(needle);

		while (*q == ' ' || *q == '\t' || *q == '\n' || *q == '\r')
			q++;
		if (*q != ':')
			continue;
		q++;
		while (*q == ' ' || *q == '\t' || *q == '\n' || *q == '\r')
			q++;
		if (*q != '{')
			continue;

		*open_at = (size_t)(q - json) + 1;

		int depth = 1;
		bool s = false;

		for (q++; *q; q++) {
			if (s) {
				if (*q == '\\' && q[1])
					q++;
				else if (*q == '"')
					s = false;
				continue;
			}
			if (*q == '"') {
				s = true;
				continue;
			}
			if (*q == '/' && q[1] == '/') {
				while (*q && *q != '\n')
					q++;
				if (!*q)
					break;
				continue;
			}
			if (*q == '{' || *q == '[')
				depth++;
			else if (*q == '}' || *q == ']') {
				if (--depth == 0) {
					*close_at = (size_t)(q - json);
					return true;
				}
			}
		}
		return false;
	}
	return false;
}

/* The whitespace the last entry in a block starts its line with, so an inserted
   entry lines up with what is already there rather than with a guess. */
static void infer_indent(const char *json, size_t open_at, size_t close_at,
			 char *out, size_t cap)
{
	/* default matches what oboe init writes */
	snprintf(out, cap, "        ");

	/* The indentation wanted is the last *entry's*, so start from the last
	   non-whitespace character in the block and walk back to its own line.
	   Taking the last newline instead finds the closing brace's line, whose
	   indentation is one level shallower. */
	size_t end = close_at;

	while (end > open_at &&
	       (json[end - 1] == ' ' || json[end - 1] == '\t' ||
		json[end - 1] == '\n' || json[end - 1] == '\r'))
		end--;
	if (end == open_at)
		return; /* empty block: nothing to match */

	size_t bol = end;

	while (bol > open_at && json[bol - 1] != '\n')
		bol--;

	size_t n = 0;

	for (size_t i = bol; i < end && n + 1 < cap; i++) {
		if (json[i] != ' ' && json[i] != '\t')
			break;
		out[n++] = json[i];
	}
	if (n > 0)
		out[n] = '\0';
}

/* Is there a non-space, non-comment character in [from, to)? */
static bool block_has_entries(const char *json, size_t from, size_t to)
{
	for (size_t i = from; i < to; i++) {
		if (json[i] == ' ' || json[i] == '\t' || json[i] == '\n' ||
		    json[i] == '\r')
			continue;
		if (json[i] == '/' && i + 1 < to && json[i + 1] == '/') {
			while (i < to && json[i] != '\n')
				i++;
			continue;
		}
		return true;
	}
	return false;
}

/* Replaces an existing entry's value in place. */
static bool replace_value(const char *path, char *json, const char *pkg,
			  const char *constraint)
{
	char needle[256];

	snprintf(needle, sizeof needle, "\"%s\"", pkg);

	size_t open_at, close_at;

	if (!find_deps(json, &open_at, &close_at))
		return false;

	char *hit = strstr(json + open_at, needle);

	if (!hit || (size_t)(hit - json) >= close_at)
		return false;

	char *colon = strchr(hit, ':');

	if (!colon || (size_t)(colon - json) >= close_at)
		return false;

	char *vs = strchr(colon, '"');

	if (!vs || (size_t)(vs - json) >= close_at)
		return false;
	char *ve = strchr(vs + 1, '"');

	if (!ve || (size_t)(ve - json) >= close_at)
		return false;

	size_t head = (size_t)(vs - json) + 1;
	size_t tail = (size_t)(ve - json);
	size_t clen = strlen(constraint);
	char *out = malloc(strlen(json) + clen + 1);

	if (!out)
		return false;
	memcpy(out, json, head);
	memcpy(out + head, constraint, clen);
	strcpy(out + head + clen, json + tail);

	bool ok = write_atomic(path, out);

	free(out);
	return ok;
}

bool add_dependency_line(const char *path, const char *pkg,
			 const char *constraint)
{
	FILE *f = fopen(path, "rb");

	if (!f)
		return false;
	if (fseek(f, 0, SEEK_END) != 0) {
		fclose(f);
		return false;
	}
	long flen = ftell(f);

	if (flen < 0) {
		fclose(f);
		return false;
	}
	rewind(f);

	char *json = malloc((size_t)flen + 1);

	if (!json) {
		fclose(f);
		return false;
	}
	size_t got = fread(json, 1, (size_t)flen, f);

	fclose(f);
	json[got] = '\0';

	/* already listed: this is the upgrade path */
	if (replace_value(path, json, pkg, constraint)) {
		free(json);
		return true;
	}

	size_t open_at, close_at;
	char indent[64];
	char entry[512];
	char *out = NULL;

	if (find_deps(json, &open_at, &close_at)) {
		infer_indent(json, open_at, close_at, indent, sizeof indent);

		bool populated = block_has_entries(json, open_at, close_at);

		/* Insert directly after the last entry's own text, not after
		   the whitespace that follows it -- otherwise the comma lands
		   on a line of its own and the closing brace keeps the
		   indentation that belonged to the entry. */
		size_t insert_at = close_at;

		while (insert_at > open_at && (json[insert_at - 1] == ' ' ||
					       json[insert_at - 1] == '\t' ||
					       json[insert_at - 1] == '\n' ||
					       json[insert_at - 1] == '\r'))
			insert_at--;

		int n = populated ?
				snprintf(entry, sizeof entry,
					 ",\n%s\"%s\": \"%s\"", indent, pkg,
					 constraint) :
				/* an empty block was written as {} or { }, so the
				   closing brace needs a line of its own too */
				snprintf(entry, sizeof entry,
					 "\n%s\"%s\": \"%s\"\n    ", indent,
					 pkg, constraint);

		out = malloc(strlen(json) + (size_t)n + 1);
		if (!out) {
			free(json);
			return false;
		}
		memcpy(out, json, insert_at);
		memcpy(out + insert_at, entry, (size_t)n);
		strcpy(out + insert_at + (size_t)n, json + insert_at);
	} else {
		/* no dependencies object at all: add one before the document's
		   final brace, commaing whatever member precedes it */
		size_t end = strlen(json);

		while (end > 0 &&
		       (json[end - 1] == '\n' || json[end - 1] == ' ' ||
			json[end - 1] == '\t' || json[end - 1] == '\r'))
			end--;
		if (end == 0 || json[end - 1] != '}') {
			free(json);
			return false;
		}
		size_t brace = end - 1;
		size_t before = brace;

		while (before > 0 &&
		       (json[before - 1] == '\n' || json[before - 1] == ' ' ||
			json[before - 1] == '\t' || json[before - 1] == '\r'))
			before--;

		bool needs_comma = before > 0 && json[before - 1] != '{' &&
				   json[before - 1] != ',';

		int n = snprintf(entry, sizeof entry,
				 "%s\n    \"dependencies\": {\n"
				 "        \"%s\": \"%s\"\n    }\n",
				 needs_comma ? "," : "", pkg, constraint);

		out = malloc(strlen(json) + (size_t)n + 1);
		if (!out) {
			free(json);
			return false;
		}
		memcpy(out, json, before);
		memcpy(out + before, entry, (size_t)n);
		strcpy(out + before + (size_t)n, json + brace);
	}

	free(json);

	bool ok = write_atomic(path, out);

	free(out);
	return ok;
}

bool remove_dependency_line(const char *path, const char *pkg)
{
	FILE *f = fopen(path, "rb");

	if (!f)
		return false;
	if (fseek(f, 0, SEEK_END) != 0) {
		fclose(f);
		return false;
	}
	long flen = ftell(f);

	if (flen < 0) {
		fclose(f);
		return false;
	}
	rewind(f);

	char *json = malloc((size_t)flen + 1);

	if (!json) {
		fclose(f);
		return false;
	}
	size_t got = fread(json, 1, (size_t)flen, f);

	fclose(f);
	json[got] = '\0';

	char needle[256];

	snprintf(needle, sizeof needle, "\"%s\"", pkg);

	char *out = malloc(strlen(json) + 1);

	if (!out) {
		free(json);
		return false;
	}

	size_t n = 0;
	bool removed = false;

	for (char *line = json; *line;) {
		char *eol = strchr(line, '\n');
		size_t len = eol ? (size_t)(eol - line) + 1 : strlen(line);
		char *hit = strstr(line, needle);
		bool is_dep_line = hit && hit < line + len &&
				   strchr(line, ':') > hit;

		if (is_dep_line && !removed) {
			removed = true;
		} else {
			memcpy(out + n, line, len);
			n += len;
		}
		line += len;
	}
	out[n] = '\0';

	/* Dropping the last entry of an object leaves the previous one ending in
	   a comma before the closing brace; strip any such dangling comma.
	   Commas inside strings are skipped so a value like "a,b" is untouched. */
	if (removed) {
		bool in_string = false;

		for (char *p = out; *p; p++) {
			if (in_string) {
				if (*p == '\\' && p[1])
					p++;
				else if (*p == '"')
					in_string = false;
				continue;
			}
			if (*p == '"') {
				in_string = true;
				continue;
			}
			if (*p != ',')
				continue;

			char *q = p + 1;

			while (*q == ' ' || *q == '\t' || *q == '\n' ||
			       *q == '\r')
				q++;
			if (*q == '}' || *q == ']') {
				memmove(p, p + 1, strlen(p));
				p--;
			}
		}
	}

	if (removed)
		removed = write_atomic(path, out);

	free(json);
	free(out);
	return removed;
}
