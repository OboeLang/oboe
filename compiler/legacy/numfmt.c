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
#include "numfmt.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ob_double_text(double d, char *buf, size_t n)
{
	if (isnan(d)) {
		snprintf(buf, n, "nan");
		return;
	}
	if (isinf(d)) {
		snprintf(buf, n, "%s", d < 0 ? "-inf" : "inf");
		return;
	}
	/* 17 significant digits always round-trip a double, so the loop is
	   bounded; the first precision that survives strtod is the shortest. */
	for (int prec = 1; prec <= 17; prec++) {
		snprintf(buf, n, "%.*g", prec, d);
		if (strtod(buf, NULL) == d)
			break;
	}
	if (!strpbrk(buf, ".eE"))
		strncat(buf, ".0", n - strlen(buf) - 1);
}
