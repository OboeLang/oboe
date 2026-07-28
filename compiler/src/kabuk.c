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
 * canonical copy: reedbed common/kabuk.c -- edit there, then re-vendor
 */
#include "kabuk.h"

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

const struct kabuk_limits kabuk_default_limits = {
	.max_entries = 65536,
	.max_file = 64ULL * 1024 * 1024,
	.max_total = 64ULL * 1024 * 1024,
};

static bool fail(char **err, const char *fmt, ...)
{
	if (err) {
		char buf[512];
		va_list ap;

		va_start(ap, fmt);
		vsnprintf(buf, sizeof buf, fmt, ap);
		va_end(ap);
		*err = strdup(buf);
	}
	return false;
}

bool kabuk_path_ok(const char *path, size_t len, const char **err)
{
	const char *why = NULL;

	if (len == 0)
		why = "empty path";
	else if (len > KABUK_PATH_MAX)
		why = "path longer than 512 octets";
	else if (path[0] == '/')
		why = "absolute path";
	else if (path[len - 1] == '/')
		why = "trailing slash";
	if (why)
		goto out;

	for (size_t i = 0; i < len; i++) {
		unsigned char c = (unsigned char)path[i];

		if (c == '\\') {
			why = "backslash in path";
			goto out;
		}
		if (c < 0x20 || c == 0x7f) {
			why = "control character in path";
			goto out;
		}
	}

	/* component checks: no empty component, no "." or ".." anywhere */
	size_t start = 0;
	for (size_t i = 0; i <= len; i++) {
		if (i != len && path[i] != '/')
			continue;
		size_t clen = i - start;

		if (clen == 0) {
			why = "empty path component";
			goto out;
		}
		if (clen == 1 && path[start] == '.') {
			why = "'.' path component";
			goto out;
		}
		if (clen == 2 && path[start] == '.' && path[start + 1] == '.') {
			why = "'..' path component";
			goto out;
		}
		start = i + 1;
	}

out:
	if (why) {
		if (err)
			*err = why;
		return false;
	}
	return true;
}

int kabuk_path_cmp(const void *a, const void *b)
{
	const char *x = *(const char *const *)a;
	const char *y = *(const char *const *)b;
	size_t xn = strlen(x), yn = strlen(y);
	size_t n = xn < yn ? xn : yn;
	int c = memcmp(x, y, n);

	if (c)
		return c < 0 ? -1 : 1;
	if (xn == yn)
		return 0;
	return xn < yn ? -1 : 1;
}

int kabuk_file_cmp(const void *a, const void *b)
{
	const struct kabuk_file *x = a;
	const struct kabuk_file *y = b;

	return kabuk_path_cmp(&x->path, &y->path);
}

bool kabuk_write(FILE *out, const struct kabuk_file *files, size_t n,
		 char **err)
{
	if (fwrite(KABUK_MAGIC, 1, KABUK_MAGIC_LEN, out) != KABUK_MAGIC_LEN)
		return fail(err, "write failed");

	for (size_t i = 0; i < n; i++) {
		const char *path = files[i].path;
		size_t plen = strlen(path);
		const char *why;

		if (!kabuk_path_ok(path, plen, &why))
			return fail(err, "%s: %s", path, why);
		/* strictly increasing: equal paths would let a later entry
		   quietly overwrite an earlier one on extraction */
		if (i > 0 && kabuk_path_cmp(&files[i - 1].path, &path) >= 0)
			return fail(
				err,
				"entries not strictly sorted: '%s' after '%s'",
				path, files[i - 1].path);

		const char *fspath = files[i].fspath ? files[i].fspath : path;
		FILE *in = fopen(fspath, "rb");

		if (!in)
			return fail(err, "%s: %s", fspath, strerror(errno));

		if (fseek(in, 0, SEEK_END) != 0) {
			fclose(in);
			return fail(err, "%s: not a seekable file", fspath);
		}
		long size = ftell(in);

		if (size < 0) {
			fclose(in);
			return fail(err, "%s: %s", fspath, strerror(errno));
		}
		rewind(in);

		if (fprintf(out, "%s\n%ld\n", path, size) < 0) {
			fclose(in);
			return fail(err, "write failed");
		}

		char buf[65536];
		long left = size;

		while (left > 0) {
			size_t want = (size_t)(left < (long)sizeof buf ?
						       left :
						       (long)sizeof buf);
			size_t got = fread(buf, 1, want, in);

			if (got == 0) {
				fclose(in);
				return fail(err, "%s: shrank while packing",
					    fspath);
			}
			if (fwrite(buf, 1, got, out) != got) {
				fclose(in);
				return fail(err, "write failed");
			}
			left -= (long)got;
		}
		fclose(in);
	}

	if (fwrite(KABUK_TERM, 1, KABUK_TERM_LEN, out) != KABUK_TERM_LEN)
		return fail(err, "write failed");
	return true;
}

/* Reads a decimal length field terminated by LF: no sign, no leading zeros, at
   most 10 digits. Returns false on anything else. */
static bool read_length(const unsigned char *buf, size_t len, size_t *pos,
			unsigned long long *out)
{
	size_t i = *pos;
	int digits = 0;
	unsigned long long v = 0;

	if (i >= len || buf[i] < '0' || buf[i] > '9')
		return false;
	if (buf[i] == '0' && i + 1 < len && buf[i + 1] >= '0' &&
	    buf[i + 1] <= '9')
		return false;

	while (i < len && buf[i] >= '0' && buf[i] <= '9') {
		if (++digits > 10)
			return false;
		v = v * 10 + (unsigned long long)(buf[i] - '0');
		i++;
	}
	if (i >= len || buf[i] != '\n')
		return false;

	*pos = i + 1;
	*out = v;
	return true;
}

bool kabuk_read(const unsigned char *buf, size_t len,
		const struct kabuk_limits *lim, kabuk_entry_fn fn, void *ctx,
		char **err)
{
	if (!lim)
		lim = &kabuk_default_limits;

	if (len < KABUK_MAGIC_LEN ||
	    memcmp(buf, KABUK_MAGIC, KABUK_MAGIC_LEN) != 0)
		return fail(err, "not a kabuk1 archive");

	size_t pos = KABUK_MAGIC_LEN;
	size_t entries = 0;
	unsigned long long total = 0;
	char prev[KABUK_PATH_MAX + 1];
	bool have_prev = false;

	for (;;) {
		if (pos >= len)
			return fail(err, "archive ends without 'sampura'");

		/* terminator, or another entry */
		if (len - pos >= KABUK_TERM_LEN &&
		    memcmp(buf + pos, KABUK_TERM, KABUK_TERM_LEN) == 0) {
			pos += KABUK_TERM_LEN;
			if (pos != len)
				return fail(err, "%zu octets after 'sampura'",
					    len - pos);
			return true;
		}

		const unsigned char *nl = memchr(buf + pos, '\n', len - pos);

		if (!nl)
			return fail(err, "truncated entry path");

		size_t plen = (size_t)(nl - (buf + pos));
		const char *why;

		if (!kabuk_path_ok((const char *)buf + pos, plen, &why))
			return fail(err, "%s", why);

		char path[KABUK_PATH_MAX + 1];

		memcpy(path, buf + pos, plen);
		path[plen] = '\0';
		pos += plen + 1;

		if (have_prev) {
			const char *pp = prev, *cp = path;

			if (kabuk_path_cmp(&pp, &cp) >= 0)
				return fail(
					err,
					"entries not strictly sorted: '%s' after '%s'",
					path, prev);
		}
		memcpy(prev, path, plen + 1);
		have_prev = true;

		unsigned long long size;

		if (!read_length(buf, len, &pos, &size))
			return fail(err, "%s: malformed length", path);
		if (size > lim->max_file)
			return fail(err, "%s: entry exceeds %llu octets", path,
				    lim->max_file);
		if (size > (unsigned long long)(len - pos))
			return fail(err, "%s: entry runs past end of archive",
				    path);

		total += size;
		if (total > lim->max_total)
			return fail(err, "archive exceeds %llu octets total",
				    lim->max_total);
		if (++entries > lim->max_entries)
			return fail(err, "archive holds more than %zu entries",
				    lim->max_entries);

		if (fn && !fn(path, buf + pos, (size_t)size, ctx, err))
			return false;

		pos += (size_t)size;
	}
}

/* Opens `dest`, then walks `path`'s components creating each directory with
   mkdirat and descending with openat(O_NOFOLLOW). Returns a directory fd for
   the parent of the final component, and copies that component into `leaf`. */
static int open_parent(int destfd, const char *path, char *leaf, size_t leafcap,
		       char **err)
{
	int dirfd = dup(destfd);

	if (dirfd < 0) {
		fail(err, "dup: %s", strerror(errno));
		return -1;
	}

	const char *p = path;

	for (;;) {
		const char *slash = strchr(p, '/');

		if (!slash) {
			size_t n = strlen(p);

			if (n + 1 > leafcap) {
				close(dirfd);
				fail(err, "%s: component too long", path);
				return -1;
			}
			memcpy(leaf, p, n + 1);
			return dirfd;
		}

		char comp[KABUK_PATH_MAX + 1];
		size_t n = (size_t)(slash - p);

		memcpy(comp, p, n);
		comp[n] = '\0';

		if (mkdirat(dirfd, comp, 0755) != 0 && errno != EEXIST) {
			fail(err, "%s: mkdir: %s", path, strerror(errno));
			close(dirfd);
			return -1;
		}

		/* O_NOFOLLOW so a symlink planted by an earlier entry cannot
		   redirect us out of the destination tree */
		int next = openat(dirfd, comp,
				  O_RDONLY | O_DIRECTORY | O_NOFOLLOW);

		close(dirfd);
		if (next < 0) {
			fail(err, "%s: %s: %s", path, comp, strerror(errno));
			return -1;
		}
		dirfd = next;
		p = slash + 1;
	}
}

struct extract_ctx {
	int destfd;
};

static bool extract_one(const char *path, const unsigned char *data, size_t len,
			void *ctxp, char **err)
{
	struct extract_ctx *ctx = ctxp;
	char leaf[KABUK_PATH_MAX + 1];
	int dirfd = open_parent(ctx->destfd, path, leaf, sizeof leaf, err);

	if (dirfd < 0)
		return false;

	/* O_EXCL means a pre-existing entry is an error rather than something
	   we write through; O_NOFOLLOW means a symlink is too */
	int fd = openat(dirfd, leaf, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW,
			0644);

	close(dirfd);
	if (fd < 0)
		return fail(err, "%s: %s", path, strerror(errno));

	size_t off = 0;

	while (off < len) {
		ssize_t w = write(fd, data + off, len - off);

		if (w < 0) {
			if (errno == EINTR)
				continue;
			close(fd);
			return fail(err, "%s: %s", path, strerror(errno));
		}
		off += (size_t)w;
	}
	close(fd);
	return true;
}

bool kabuk_extract(const unsigned char *buf, size_t len,
		   const struct kabuk_limits *lim, const char *dest, char **err)
{
	/* validate the whole archive before creating anything, so a malformed
	   tail cannot leave a half-extracted tree behind */
	if (!kabuk_validate(buf, len, lim, err))
		return false;

	int destfd = open(dest, O_RDONLY | O_DIRECTORY);

	if (destfd < 0)
		return fail(err, "%s: %s", dest, strerror(errno));

	struct extract_ctx ctx = { .destfd = destfd };
	bool ok = kabuk_read(buf, len, lim, extract_one, &ctx, err);

	close(destfd);
	return ok;
}
