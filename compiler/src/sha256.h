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
 * canonical copy: reedbed common/sha256.h -- edit there, then re-vendor
 */
#ifndef KATARE_SHA256_H
#define KATARE_SHA256_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* FIPS 180-4 SHA-256. Hand-rolled because both the compiler and the server
   refuse third-party dependencies, and because `sema` is the only integrity
   check a package has -- it should not depend on which of sha256sum, shasum or
   openssl happens to be installed. */

#define SHA256_DIGEST_LEN 32
/* "sha256:" + 64 hex + NUL */
#define SEMA_STR_LEN 72

struct sha256_ctx {
	uint32_t h[8];
	uint64_t nbits;
	size_t buffered;
	unsigned char block[64];
};

void sha256_init(struct sha256_ctx *c);
void sha256_update(struct sha256_ctx *c, const void *data, size_t n);
void sha256_final(struct sha256_ctx *c, unsigned char out[SHA256_DIGEST_LEN]);

/* one-shot over a buffer */
void sha256(const void *data, size_t n, unsigned char out[SHA256_DIGEST_LEN]);

/* Formats a digest as the wire form `sha256:<64 lowercase hex>`. `out` must hold
   SEMA_STR_LEN bytes. */
void sema_format(const unsigned char digest[SHA256_DIGEST_LEN], char *out);

/* True when `s` is a well-formed sema: exactly "sha256:" followed by 64
   lowercase hex digits and nothing else. Uppercase hex is rejected, since two
   spellings of one digest would let a record and a ghazema line disagree while
   naming the same bytes. */
bool sema_valid(const char *s);

/* Constant-time-ish equality for two sema strings. Both must already be valid;
   returns false if either is not. */
bool sema_equal(const char *a, const char *b);

#endif
