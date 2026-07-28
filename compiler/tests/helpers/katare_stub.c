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

/* A scripted katare server, for testing the client in this repo.
 *
 * The real server lives in the reedbed repository, which oboe's CI does not
 * check out -- so the client's tests need something here that can be made to
 * say anything, including things a correct server never would: an unknown
 * status, a wrong digest, an archive containing a traversal path.
 *
 * Binds port 0, prints `listening <port>`, then serves connections from a
 * script until it runs out. Directives, one per line:
 *
 *   expect <text>     read a line; fail unless it equals <text>
 *   expectpre <text>  read a line; fail unless it starts with <text>
 *   read              read a line and ignore it
 *   send <text>       send <text> CRLF
 *   sendbody <file>   send a file's octets with no framing
 *   accept            wait for the next connection
 *   close             close the current connection
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

static int g_conn = -1;
static unsigned char g_buf[8192];
static size_t g_off, g_len;

static bool send_all(const void *p, size_t n)
{
	const char *b = p;
	size_t off = 0;

	while (off < n) {
		ssize_t w = write(g_conn, b + off, n - off);

		if (w < 0) {
			if (errno == EINTR)
				continue;
			return false;
		}
		off += (size_t)w;
	}
	return true;
}

/* Buffered, because a body follows its line with no separator. */
static bool read_line(char *out, size_t cap)
{
	size_t scanned = 0;

	for (;;) {
		for (size_t i = g_off + scanned; i < g_len; i++) {
			if (g_buf[i] != '\n')
				continue;

			size_t linelen = i - g_off;

			if (linelen > 0 && g_buf[i - 1] == '\r')
				linelen--;
			if (linelen + 1 > cap)
				return false;
			memcpy(out, g_buf + g_off, linelen);
			out[linelen] = '\0';
			g_off = i + 1;
			return true;
		}
		scanned = g_len - g_off;

		if (g_off > 0) {
			memmove(g_buf, g_buf + g_off, g_len - g_off);
			g_len -= g_off;
			g_off = 0;
			scanned = g_len;
		}
		if (g_len == sizeof g_buf)
			return false;

		ssize_t n = read(g_conn, g_buf + g_len, sizeof g_buf - g_len);

		if (n <= 0) {
			if (n < 0 && errno == EINTR)
				continue;
			return false;
		}
		g_len += (size_t)n;
	}
}

static bool send_file(const char *path)
{
	FILE *f = fopen(path, "rb");

	if (!f) {
		fprintf(stderr, "katare_stub: cannot open %s\n", path);
		return false;
	}

	char buf[65536];
	size_t n;
	bool ok = true;

	while (ok && (n = fread(buf, 1, sizeof buf, f)) > 0)
		ok = send_all(buf, n);
	fclose(f);
	return ok;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: katare_stub <script>\n");
		return 2;
	}

	FILE *script = fopen(argv[1], "r");

	if (!script) {
		fprintf(stderr, "katare_stub: cannot open %s\n", argv[1]);
		return 2;
	}

	signal(SIGPIPE, SIG_IGN);

	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	int one = 1;

	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);

	struct sockaddr_in a;

	memset(&a, 0, sizeof a);
	a.sin_family = AF_INET;
	a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	a.sin_port = 0;

	if (bind(lfd, (struct sockaddr *)&a, sizeof a) != 0 ||
	    listen(lfd, 8) != 0) {
		perror("katare_stub");
		return 1;
	}

	socklen_t alen = sizeof a;

	getsockname(lfd, (struct sockaddr *)&a, &alen);
	/* flushed before the first accept, so the harness can block on this
	   line and know the socket is already listening */
	printf("listening %d\n", ntohs(a.sin_port));
	fflush(stdout);

	char line[8192];
	int rc = 0;
	bool accepted = false;

	while (fgets(line, sizeof line, script)) {
		size_t n = strlen(line);

		while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r'))
			line[--n] = '\0';
		if (n == 0 || line[0] == '#')
			continue;

		if (strcmp(line, "accept") == 0 || !accepted) {
			if (accepted && strcmp(line, "accept") == 0) {
				close(g_conn);
				g_conn = -1;
			}
			if (g_conn < 0) {
				struct timeval tv = { .tv_sec = 10 };

				g_conn = accept(lfd, NULL, NULL);
				if (g_conn < 0) {
					perror("katare_stub: accept");
					rc = 1;
					break;
				}
				setsockopt(g_conn, SOL_SOCKET, SO_RCVTIMEO, &tv,
					   sizeof tv);
				setsockopt(g_conn, SOL_SOCKET, SO_SNDTIMEO, &tv,
					   sizeof tv);
				g_off = g_len = 0;
				accepted = true;
			}
			if (strcmp(line, "accept") == 0)
				continue;
		}

		if (strncmp(line, "send ", 5) == 0) {
			char out[8192];
			int m = snprintf(out, sizeof out - 2, "%s", line + 5);

			out[m] = '\r';
			out[m + 1] = '\n';
			if (!send_all(out, (size_t)m + 2))
				break;
		} else if (strncmp(line, "sendbody ", 9) == 0) {
			if (!send_file(line + 9))
				break;
		} else if (strncmp(line, "expect ", 7) == 0) {
			char got[8192];

			if (!read_line(got, sizeof got)) {
				fprintf(stderr,
					"katare_stub: expected '%s', got EOF\n",
					line + 7);
				rc = 1;
				break;
			}
			if (strcmp(got, line + 7) != 0) {
				fprintf(stderr,
					"katare_stub: expected '%s', got '%s'\n",
					line + 7, got);
				rc = 1;
				break;
			}
		} else if (strncmp(line, "expectpre ", 10) == 0) {
			char got[8192];

			if (!read_line(got, sizeof got) ||
			    strncmp(got, line + 10, strlen(line + 10)) != 0) {
				fprintf(stderr,
					"katare_stub: expected prefix '%s'\n",
					line + 10);
				rc = 1;
				break;
			}
		} else if (strcmp(line, "read") == 0) {
			char got[8192];

			read_line(got, sizeof got);
		} else if (strcmp(line, "close") == 0) {
			close(g_conn);
			g_conn = -1;
			accepted = false;
		} else {
			fprintf(stderr, "katare_stub: bad directive: %s\n",
				line);
			rc = 2;
			break;
		}
	}

	if (g_conn >= 0)
		close(g_conn);
	close(lfd);
	fclose(script);
	return rc;
}
