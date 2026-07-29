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
#ifndef OBOE_PKG_H
#define OBOE_PKG_H

#include <stdbool.h>

/* The package-manager commands: oboe get, install, publish and the fetching
   half of oboe tidy. Each returns a process exit status. */

/* Directory holding the running oboe binary. Implemented in main.c, which
   already needs it to find the bundled runtime; declared here so the installer
   can re-invoke the compiler rather than duplicating the platform dance. */
char *oboe_home(void);

int cmd_get(int argc, char **argv);
int cmd_install(int argc, char **argv);
int cmd_publish(int argc, char **argv);

/* Installs whatever project.jsonc declares and .oboe/lock does not already
   satisfy. Returns 0 when everything was already in place, having opened no
   connection -- that is what makes tidy usable offline. */
int pkg_tidy(bool verbose);

/* sha256 of each argument, in the wire form. */
int cmd_sema(int argc, char **argv);

#endif
