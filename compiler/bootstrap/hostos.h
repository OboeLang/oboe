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
 * Prepended to bootstrap/oboec.c by `make regen-bootstrap`, which also rewrites
 * the generated HOST_OS constant to use it. selfhost/hostos.oboe is a per-OS
 * module, so without this the OS chosen on the generating machine would be
 * frozen into a file every other machine then builds -- a macOS oboec that
 * silently targets Linux, which is what this is fixing.
 *
 * This is only the default for a bare `oboec`; bin/oboe always passes
 * --target-os. The arms match legacy/codegen.c's, BSDs included: they fall
 * through to linux there too.
 *
 * Overridable with -D so the suite can build a deliberately foreign-hosted
 * oboec and check that the default really does follow the compiler, which is
 * not otherwise observable from the host it is tested on.
 */
#ifndef OBOEC_HOST_OS
#if defined(_WIN32)
#define OBOEC_HOST_OS "windows"
#elif defined(__APPLE__)
#define OBOEC_HOST_OS "macos"
#else
#define OBOEC_HOST_OS "linux"
#endif
#endif
