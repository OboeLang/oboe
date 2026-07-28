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
#ifndef OBOE_PROJECTEDIT_H
#define OBOE_PROJECTEDIT_H

#include <stdbool.h>

/* Edits to project.jsonc's dependency list.
 *
 * These are line-oriented edits, never a reserialise. The file is hand-written:
 * it has comments, a chosen indentation and an order its author picked, and a
 * package manager that reformatted it on every install would be intolerable.
 *
 * The add and remove paths have to agree byte for byte about indentation and
 * comma placement, which is why they live together rather than in whichever
 * file happened to need one first. */

/* Adds `pkg`, or replaces its constraint when already present. Creates the
   `dependencies` object if the file has none. */
bool add_dependency_line(const char *path, const char *pkg,
			 const char *constraint);

/* Drops `pkg`'s line, cleaning up a comma left dangling before the closing
   brace. False when there was nothing to remove. */
bool remove_dependency_line(const char *path, const char *pkg);

#endif
