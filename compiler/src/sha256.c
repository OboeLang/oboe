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
 * canonical copy: reedbed common/sha256.c -- edit there, then re-vendor
 */
#include "sha256.h"

#include <string.h>

static const uint32_t k_round[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
	0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
	0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
	0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
	0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
	0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static uint32_t rotr(uint32_t x, int n)
{
	return (x >> n) | (x << (32 - n));
}

static void compress(uint32_t h[8], const unsigned char block[64])
{
	uint32_t w[64];

	for (int i = 0; i < 16; i++)
		w[i] = (uint32_t)block[i * 4] << 24 |
		       (uint32_t)block[i * 4 + 1] << 16 |
		       (uint32_t)block[i * 4 + 2] << 8 |
		       (uint32_t)block[i * 4 + 3];
	for (int i = 16; i < 64; i++) {
		uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^
			      (w[i - 15] >> 3);
		uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^
			      (w[i - 2] >> 10);
		w[i] = w[i - 16] + s0 + w[i - 7] + s1;
	}

	uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
	uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];

	for (int i = 0; i < 64; i++) {
		uint32_t s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
		uint32_t ch = (e & f) ^ (~e & g);
		uint32_t t1 = hh + s1 + ch + k_round[i] + w[i];
		uint32_t s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
		uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
		uint32_t t2 = s0 + maj;

		hh = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	h[0] += a;
	h[1] += b;
	h[2] += c;
	h[3] += d;
	h[4] += e;
	h[5] += f;
	h[6] += g;
	h[7] += hh;
}

void sha256_init(struct sha256_ctx *c)
{
	c->h[0] = 0x6a09e667;
	c->h[1] = 0xbb67ae85;
	c->h[2] = 0x3c6ef372;
	c->h[3] = 0xa54ff53a;
	c->h[4] = 0x510e527f;
	c->h[5] = 0x9b05688c;
	c->h[6] = 0x1f83d9ab;
	c->h[7] = 0x5be0cd19;
	c->nbits = 0;
	c->buffered = 0;
}

void sha256_update(struct sha256_ctx *c, const void *data, size_t n)
{
	const unsigned char *p = data;

	c->nbits += (uint64_t)n * 8;

	/* top up a partial block first, so the fast path below stays aligned */
	if (c->buffered) {
		size_t want = 64 - c->buffered;
		size_t take = n < want ? n : want;

		memcpy(c->block + c->buffered, p, take);
		c->buffered += take;
		p += take;
		n -= take;
		if (c->buffered < 64)
			return;
		compress(c->h, c->block);
		c->buffered = 0;
	}

	while (n >= 64) {
		compress(c->h, p);
		p += 64;
		n -= 64;
	}

	if (n) {
		memcpy(c->block, p, n);
		c->buffered = n;
	}
}

void sha256_final(struct sha256_ctx *c, unsigned char out[SHA256_DIGEST_LEN])
{
	uint64_t nbits = c->nbits;

	/* pad with 0x80 then zeros; the length needs 8 bytes, so when fewer than
	   9 remain in this block the padding spills into one more block */
	c->block[c->buffered++] = 0x80;
	if (c->buffered > 56) {
		memset(c->block + c->buffered, 0, 64 - c->buffered);
		compress(c->h, c->block);
		c->buffered = 0;
	}
	memset(c->block + c->buffered, 0, 56 - c->buffered);
	for (int i = 0; i < 8; i++)
		c->block[56 + i] = (unsigned char)(nbits >> (56 - i * 8));
	compress(c->h, c->block);

	for (int i = 0; i < 8; i++) {
		out[i * 4] = (unsigned char)(c->h[i] >> 24);
		out[i * 4 + 1] = (unsigned char)(c->h[i] >> 16);
		out[i * 4 + 2] = (unsigned char)(c->h[i] >> 8);
		out[i * 4 + 3] = (unsigned char)c->h[i];
	}
}

void sha256(const void *data, size_t n, unsigned char out[SHA256_DIGEST_LEN])
{
	struct sha256_ctx c;

	sha256_init(&c);
	sha256_update(&c, data, n);
	sha256_final(&c, out);
}

void sema_format(const unsigned char digest[SHA256_DIGEST_LEN], char *out)
{
	static const char hex[] = "0123456789abcdef";

	memcpy(out, "sha256:", 7);
	for (int i = 0; i < SHA256_DIGEST_LEN; i++) {
		out[7 + i * 2] = hex[digest[i] >> 4];
		out[7 + i * 2 + 1] = hex[digest[i] & 0x0f];
	}
	out[SEMA_STR_LEN - 1] = '\0';
}

bool sema_valid(const char *s)
{
	if (!s || strncmp(s, "sha256:", 7) != 0)
		return false;
	const char *p = s + 7;
	for (int i = 0; i < 64; i++, p++) {
		if (!((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f')))
			return false;
	}
	return *p == '\0';
}

bool sema_equal(const char *a, const char *b)
{
	if (!sema_valid(a) || !sema_valid(b))
		return false;
	/* not a secret, but comparing without an early exit costs nothing and
	   keeps the habit */
	unsigned diff = 0;
	for (int i = 0; i < SEMA_STR_LEN - 1; i++)
		diff |= (unsigned char)a[i] ^ (unsigned char)b[i];
	return diff == 0;
}
