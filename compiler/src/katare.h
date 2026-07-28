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
#ifndef OBOE_KATARE_H
#define OBOE_KATARE_H

#include "sha256.h"

#include <stdbool.h>
#include <stddef.h>

/* A katare client -- see reedbed's KATARE.md.
 *
 * Deliberately not shared with the server's framing code. The server needs
 * timeouts, caps and hostile-input handling on every path; a client needs about
 * two hundred lines of blocking request/response, and sharing them would drag
 * the server's configuration into the compiler. */

#define KAT_LINE_MAX 1024
#define KAT_WORDS_MAX 64
#define KAT_DEFAULT_PORT 440

enum kat_status {
	KAT_SI = 0,
	KAT_KERESEBYR,
	KAT_SENTYRE,
	KAT_EZHAZEBYR,
	KAT_VAZOJ,
	KAT_RAMUZHU,
	KAT_WUWOJI,
	KAT_BYR,
	KAT_KOJA,
	/* Not a status the protocol defines. A client that meets one cannot
	   know whether a body follows, so it must close rather than guess. */
	KAT_UNKNOWN,
	KAT_IOERR
};

struct kat_conn;

/* Splits katare://host[:port][/package]. `pkg` may be NULL if not wanted.
   Accepts a bracketed IPv6 literal. */
bool kat_parse_uri(const char *uri, char *host, size_t hostcap, int *port,
		   char *pkg, size_t pkgcap, char **err);

/* Dials, reads the greeting, sends ours, reads the capability line. */
struct kat_conn *kat_connect(const char *host, int port, char **err);

/* Sends koja and closes. */
void kat_close(struct kat_conn *c);

bool kat_has_cap(const struct kat_conn *c, const char *cap);
unsigned long long kat_body_cap(const struct kat_conn *c);

/* Sends one request line and reads the status line. The words of that line are
   left in the connection for kat_word(); a body, if any, is NOT read here so
   the caller can choose where it goes. */
enum kat_status kat_request(struct kat_conn *c, const char *fmt, ...);

/* Word `i` of the last status line (0 is the status itself), or NULL. */
const char *kat_word(const struct kat_conn *c, int i);
int kat_words(const struct kat_conn *c);

/* Octet count from the last status line's `kyx <n>`, or 0 when it had none. */
unsigned long long kat_body_len(const struct kat_conn *c);
bool kat_has_body(const struct kat_conn *c);

/* Reads the announced body into a fresh NUL-terminated buffer. */
char *kat_read_body(struct kat_conn *c, size_t *len, char **err);

/* Streams the announced body to `fd`, verifying it against `want_sema` as it
   goes. Fails without writing a complete file if the digest disagrees. */
bool kat_read_body_verified(struct kat_conn *c, int fd, const char *want_sema,
			    char **err);

/* Sends a request line whose body follows immediately. */
bool kat_send_body(struct kat_conn *c, const void *p, size_t n, char **err);

const char *kat_status_word(enum kat_status s);

/* Human-readable tail of a wuwoji/vazoj/ramuzhu line, or "". */
const char *kat_reason(const struct kat_conn *c);

#endif
