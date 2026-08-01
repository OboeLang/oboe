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
#ifndef OBOE_NUMFMT_H
#define OBOE_NUMFMT_H

#include <stddef.h>

/* The text of a double, in the one spelling the whole toolchain agrees on:
   the shortest %g that still round-trips, with ".0" appended when nothing else
   marks it as a float. This is what the runtime's str() produces for a float
   (ob_to_string), which is what lets the Oboe-written compiler in selfhost/
   reach the same bytes with a plain str() call -- both when it dumps an AST
   and when it emits a float literal into C. `n` should be at least 32. */
void ob_double_text(double d, char *buf, size_t n);

#endif
