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
#ifndef OBOE_CODEGEN_H
#define OBOE_CODEGEN_H

#include "ast.h"
#include <stdio.h>

void codegen_set_source_dir(const char *dir);
/* "linux", "windows" or "macos"; affects `foo.<os>.oboe` module resolution.
   Defaults to the host OS. */
void codegen_set_target_os(const char *os);
/* Loads the main file and every transitively imported module, then generates
   one C translation unit for the whole program. */
void codegen_compile(const char *main_path, FILE *out);

#endif
