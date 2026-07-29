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
#include "katare.h"

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#ifndef OBOE_VERSION
#define OBOE_VERSION "0.1"
#endif

#define KAT_AGENT "oboe/" OBOE_VERSION
#define KAT_PROTOCOL "katare/1"
#define KAT_CONNECT_TIMEOUT 15
#define KAT_IO_TIMEOUT 60
#define KAT_CAPS_MAX 32

struct kat_conn {
	int fd;
	unsigned char buf[8192];
	size_t off, len;

	char line[KAT_LINE_MAX];
	char *word[KAT_WORDS_MAX];
	int nwords;
	unsigned long long body_len;
	bool body;

	char caps[KAT_CAPS_MAX][32];
	int ncaps;
	unsigned long long cap_body;
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

/* ---- URI ---------------------------------------------------------------- */

bool kat_parse_uri(const char *uri, char *host, size_t hostcap, int *port,
		   char *pkg, size_t pkgcap, char **err)
{
	static const char scheme[] = "katare://";

	if (!uri || strncmp(uri, scheme, sizeof scheme - 1) != 0)
		return fail(err, "registry URL must start with katare://");

	const char *p = uri + sizeof scheme - 1;

	*port = KAT_DEFAULT_PORT;
	if (pkg && pkgcap)
		pkg[0] = '\0';

	size_t hn = 0;

	if (*p == '[') {
		/* a bracketed IPv6 literal: the colons inside are part of the
		   address, so the port can only follow the closing bracket */
		p++;
		while (*p && *p != ']') {
			if (hn + 1 >= hostcap)
				return fail(err, "host too long");
			host[hn++] = *p++;
		}
		if (*p != ']')
			return fail(err, "unterminated [ in registry URL");
		p++;
	} else {
		while (*p && *p != ':' && *p != '/') {
			if (hn + 1 >= hostcap)
				return fail(err, "host too long");
			host[hn++] = *p++;
		}
	}
	host[hn] = '\0';
	if (hn == 0)
		return fail(err, "registry URL has no host");

	if (*p == ':') {
		p++;
		int v = 0, digits = 0;

		while (*p >= '0' && *p <= '9') {
			v = v * 10 + (*p++ - '0');
			if (++digits > 5 || v > 65535)
				return fail(err, "bad port in registry URL");
		}
		if (digits == 0)
			return fail(err, "bad port in registry URL");
		*port = v;
	}

	if (*p == '/') {
		p++;
		if (pkg && pkgcap) {
			size_t n = 0;

			while (*p && *p != '/' && *p != '?') {
				if (n + 1 >= pkgcap)
					return fail(err,
						    "package name too long");
				pkg[n++] = *p++;
			}
			pkg[n] = '\0';
		}
	}
	return true;
}

/* ---- dialling ----------------------------------------------------------- */

/* Non-blocking connect plus poll, rather than a socket option: the portable
   spelling of a connect timeout. TCP_USER_TIMEOUT is Linux-only. */
static int dial_one(struct addrinfo *ai, int seconds)
{
	int fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

	if (fd < 0)
		return -1;

	int flags = fcntl(fd, F_GETFL, 0);

	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	if (connect(fd, ai->ai_addr, ai->ai_addrlen) != 0) {
		if (errno != EINPROGRESS) {
			close(fd);
			return -1;
		}

		struct pollfd pf = { .fd = fd, .events = POLLOUT };
		int r = poll(&pf, 1, seconds * 1000);

		if (r <= 0) {
			close(fd);
			return -1;
		}

		int soerr = 0;
		socklen_t slen = sizeof soerr;

		if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &soerr, &slen) != 0 ||
		    soerr != 0) {
			close(fd);
			return -1;
		}
	}

	fcntl(fd, F_SETFL, flags);

	struct timeval tv = { .tv_sec = KAT_IO_TIMEOUT, .tv_usec = 0 };

	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
	setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof tv);
	return fd;
}

/* ---- framing ------------------------------------------------------------ */

static bool refill(struct kat_conn *c)
{
	if (c->off > 0) {
		memmove(c->buf, c->buf + c->off, c->len - c->off);
		c->len -= c->off;
		c->off = 0;
	}
	if (c->len == sizeof c->buf)
		return false;

	for (;;) {
		ssize_t n =
			read(c->fd, c->buf + c->len, sizeof c->buf - c->len);

		if (n > 0) {
			c->len += (size_t)n;
			return true;
		}
		if (n < 0 && errno == EINTR)
			continue;
		return false;
	}
}

/* A body follows its line with no separator, so lines have to come out of a
   buffer that the body read then continues from. */
static bool read_line(struct kat_conn *c, char *out, size_t cap)
{
	size_t scanned = 0;

	for (;;) {
		for (size_t i = c->off + scanned; i < c->len; i++) {
			if (c->buf[i] != '\n')
				continue;

			size_t linelen = i - c->off;

			if (linelen == 0 || c->buf[i - 1] != '\r')
				return false;

			size_t content = linelen - 1;

			if (content + 1 > cap)
				return false;
			memcpy(out, c->buf + c->off, content);
			out[content] = '\0';
			c->off = i + 1;
			return true;
		}
		scanned = c->len - c->off;
		if (scanned > KAT_LINE_MAX)
			return false;
		if (!refill(c))
			return false;
	}
}

static bool write_all(struct kat_conn *c, const void *p, size_t n)
{
	const unsigned char *b = p;
	size_t off = 0;

	while (off < n) {
		ssize_t w = write(c->fd, b + off, n - off);

		if (w < 0) {
			if (errno == EINTR)
				continue;
			return false;
		}
		off += (size_t)w;
	}
	return true;
}

static bool write_line(struct kat_conn *c, const char *fmt, va_list ap)
{
	char buf[KAT_LINE_MAX];
	int n = vsnprintf(buf, KAT_LINE_MAX - 1, fmt, ap);

	if (n < 0 || n > KAT_LINE_MAX - 2)
		return false;
	buf[n] = '\r';
	buf[n + 1] = '\n';
	return write_all(c, buf, (size_t)n + 2);
}

static int tokenize(char *line, char **argv, int max)
{
	int argc = 0;
	char *p = line;

	if (!*p)
		return 0;
	for (;;) {
		if (argc >= max)
			return -1;
		argv[argc++] = p;
		while (*p && *p != ' ')
			p++;
		if (!*p)
			return argc;
		*p++ = '\0';
		if (*p == ' ' || !*p)
			return -1;
	}
}

static bool parse_count(const char *s, unsigned long long *out)
{
	if (!s || !*s)
		return false;
	if (s[0] == '0' && s[1])
		return false;

	unsigned long long v = 0;
	int digits = 0;

	for (const char *p = s; *p; p++) {
		if (*p < '0' || *p > '9')
			return false;
		if (++digits > 10)
			return false;
		v = v * 10 + (unsigned long long)(*p - '0');
	}
	*out = v;
	return true;
}

/* ---- status ------------------------------------------------------------- */

static const struct {
	const char *word;
	enum kat_status st;
} k_statuses[] = { { "si", KAT_SI },	       { "keresebyr", KAT_KERESEBYR },
		   { "sentyre", KAT_SENTYRE }, { "ezhazebyr", KAT_EZHAZEBYR },
		   { "vazoj", KAT_VAZOJ },     { "ramuzhu", KAT_RAMUZHU },
		   { "wuwoji", KAT_WUWOJI },   { "byr", KAT_BYR },
		   { "koja", KAT_KOJA } };

const char *kat_status_word(enum kat_status s)
{
	for (size_t i = 0; i < sizeof k_statuses / sizeof *k_statuses; i++)
		if (k_statuses[i].st == s)
			return k_statuses[i].word;
	return s == KAT_IOERR ? "connection error" : "unknown status";
}

static enum kat_status classify(const char *word)
{
	for (size_t i = 0; i < sizeof k_statuses / sizeof *k_statuses; i++)
		if (strcmp(k_statuses[i].word, word) == 0)
			return k_statuses[i].st;
	return KAT_UNKNOWN;
}

/* ---- connect ------------------------------------------------------------ */

struct kat_conn *kat_connect(const char *host, int port, char **err)
{
	char portstr[16];

	snprintf(portstr, sizeof portstr, "%d", port);

	struct addrinfo hints, *res = NULL;

	memset(&hints, 0, sizeof hints);
	hints.ai_socktype = SOCK_STREAM;

	int rc = getaddrinfo(host, portstr, &hints, &res);

	if (rc != 0) {
		fail(err, "%s: %s", host, gai_strerror(rc));
		return NULL;
	}

	int fd = -1;

	for (struct addrinfo *ai = res; ai && fd < 0; ai = ai->ai_next)
		fd = dial_one(ai, KAT_CONNECT_TIMEOUT);
	freeaddrinfo(res);

	if (fd < 0) {
		fail(err, "cannot reach %s port %d", host, port);
		return NULL;
	}

	struct kat_conn *c = calloc(1, sizeof *c);

	if (!c) {
		close(fd);
		fail(err, "out of memory");
		return NULL;
	}
	c->fd = fd;
	c->cap_body = 64ULL * 1024 * 1024;

	/* the server speaks first */
	char line[KAT_LINE_MAX];

	if (!read_line(c, line, sizeof line)) {
		kat_close(c);
		fail(err, "no greeting from %s", host);
		return NULL;
	}

	char *w[KAT_WORDS_MAX];
	int n = tokenize(line, w, KAT_WORDS_MAX);

	if (n != 3 || strcmp(w[0], "dijabon") != 0) {
		kat_close(c);
		fail(err, "%s does not speak katare", host);
		return NULL;
	}
	if (strcmp(w[1], KAT_PROTOCOL) != 0) {
		kat_close(c);
		fail(err, "%s speaks %s, this oboe speaks %s", host, w[1],
		     KAT_PROTOCOL);
		return NULL;
	}

	{
		char greet[KAT_LINE_MAX];
		int gn = snprintf(greet, sizeof greet - 2, "dijabon %s %s",
				  KAT_PROTOCOL, KAT_AGENT);

		greet[gn] = '\r';
		greet[gn + 1] = '\n';
		if (!write_all(c, greet, (size_t)gn + 2)) {
			kat_close(c);
			fail(err, "%s closed during the handshake", host);
			return NULL;
		}
	}

	if (!read_line(c, line, sizeof line)) {
		kat_close(c);
		fail(err, "%s closed before advertising capabilities", host);
		return NULL;
	}
	n = tokenize(line, w, KAT_WORDS_MAX);
	if (n < 1 || classify(w[0]) != KAT_SI) {
		kat_close(c);
		fail(err, "%s refused the handshake: %s", host,
		     n > 0 ? line : "no reply");
		return NULL;
	}

	for (int i = 1; i < n && c->ncaps < KAT_CAPS_MAX; i++) {
		/* the body cap rides along as a bare word so the capability
		   grammar stays "bare words" */
		if (strncmp(w[i], "kyx", 3) == 0) {
			unsigned long long v;

			if (parse_count(w[i] + 3, &v))
				c->cap_body = v;
			continue;
		}
		snprintf(c->caps[c->ncaps++], sizeof c->caps[0], "%s", w[i]);
	}
	return c;
}

void kat_close(struct kat_conn *c)
{
	if (!c)
		return;
	if (c->fd >= 0) {
		write_all(c, "koja\r\n", 6);
		shutdown(c->fd, SHUT_WR);
		close(c->fd);
	}
	free(c);
}

bool kat_has_cap(const struct kat_conn *c, const char *cap)
{
	for (int i = 0; i < c->ncaps; i++)
		if (strcmp(c->caps[i], cap) == 0)
			return true;
	return false;
}

unsigned long long kat_body_cap(const struct kat_conn *c)
{
	return c->cap_body;
}

/* ---- request/response ---------------------------------------------------- */

bool kat_send_line(struct kat_conn *c, const char *fmt, ...)
{
	va_list ap;

	c->nwords = 0;
	c->body = false;
	c->body_len = 0;

	va_start(ap, fmt);
	bool ok = write_line(c, fmt, ap);
	va_end(ap);
	return ok;
}

enum kat_status kat_read_status(struct kat_conn *c)
{
	c->nwords = 0;
	c->body = false;
	c->body_len = 0;

	if (!read_line(c, c->line, sizeof c->line))
		return KAT_IOERR;

	c->nwords = tokenize(c->line, c->word, KAT_WORDS_MAX);
	if (c->nwords <= 0)
		return KAT_IOERR;

	if (c->nwords >= 2 && strcmp(c->word[c->nwords - 2], "kyx") == 0) {
		if (!parse_count(c->word[c->nwords - 1], &c->body_len))
			return KAT_IOERR;
		c->body = true;
	}
	return classify(c->word[0]);
}

enum kat_status kat_request(struct kat_conn *c, const char *fmt, ...)
{
	va_list ap;

	c->nwords = 0;
	c->body = false;
	c->body_len = 0;

	va_start(ap, fmt);
	bool ok = write_line(c, fmt, ap);
	va_end(ap);

	if (!ok)
		return KAT_IOERR;
	if (!read_line(c, c->line, sizeof c->line))
		return KAT_IOERR;

	c->nwords = tokenize(c->line, c->word, KAT_WORDS_MAX);
	if (c->nwords <= 0)
		return KAT_IOERR;

	/* a trailing `kyx <n>` means a body follows immediately */
	if (c->nwords >= 2 && strcmp(c->word[c->nwords - 2], "kyx") == 0) {
		if (!parse_count(c->word[c->nwords - 1], &c->body_len))
			return KAT_IOERR;
		c->body = true;
	}
	return classify(c->word[0]);
}

const char *kat_word(const struct kat_conn *c, int i)
{
	return i >= 0 && i < c->nwords ? c->word[i] : NULL;
}

int kat_words(const struct kat_conn *c)
{
	return c->nwords;
}

unsigned long long kat_body_len(const struct kat_conn *c)
{
	return c->body_len;
}

bool kat_has_body(const struct kat_conn *c)
{
	return c->body;
}

const char *kat_reason(const struct kat_conn *c)
{
	return c->nwords > 1 ? c->word[1] : "";
}

char *kat_read_body(struct kat_conn *c, size_t *len, char **err)
{
	if (!c->body) {
		fail(err, "no body announced");
		return NULL;
	}
	if (c->body_len > c->cap_body) {
		fail(err, "body larger than the server's own cap");
		return NULL;
	}

	char *buf = malloc((size_t)c->body_len + 1);

	if (!buf) {
		fail(err, "out of memory");
		return NULL;
	}

	unsigned long long got = 0;

	while (got < c->body_len) {
		if (c->off == c->len && !refill(c)) {
			free(buf);
			fail(err, "connection closed mid-body");
			return NULL;
		}
		size_t have = c->len - c->off;
		size_t take = have < c->body_len - got ?
				      have :
				      (size_t)(c->body_len - got);

		memcpy(buf + got, c->buf + c->off, take);
		c->off += take;
		got += take;
	}
	buf[got] = '\0';
	c->body = false;
	if (len)
		*len = (size_t)got;
	return buf;
}

bool kat_read_body_verified(struct kat_conn *c, int fd, const char *want_sema,
			    char **err)
{
	if (!c->body)
		return fail(err, "no body announced");
	if (!sema_valid(want_sema))
		return fail(err, "the server sent a malformed digest");

	struct sha256_ctx ctx;

	sha256_init(&ctx);

	unsigned long long got = 0;

	while (got < c->body_len) {
		if (c->off == c->len && !refill(c))
			return fail(err, "connection closed mid-download");

		size_t have = c->len - c->off;
		size_t take = have < c->body_len - got ?
				      have :
				      (size_t)(c->body_len - got);

		/* hashed as it streams, so nothing has to be buffered whole */
		sha256_update(&ctx, c->buf + c->off, take);

		size_t done = 0;

		while (done < take) {
			ssize_t w =
				write(fd, c->buf + c->off + done, take - done);

			if (w < 0) {
				if (errno == EINTR)
					continue;
				return fail(err, "write failed: %s",
					    strerror(errno));
			}
			done += (size_t)w;
		}
		c->off += take;
		got += take;
	}
	c->body = false;

	unsigned char digest[SHA256_DIGEST_LEN];
	char actual[SEMA_STR_LEN];

	sha256_final(&ctx, digest);
	sema_format(digest, actual);

	if (!sema_equal(actual, want_sema))
		return fail(
			err,
			"digest mismatch: expected %s but the data hashes to %s",
			want_sema, actual);
	return true;
}

bool kat_send_body(struct kat_conn *c, const void *p, size_t n, char **err)
{
	if (!write_all(c, p, n))
		return fail(err, "connection closed while sending");
	return true;
}
