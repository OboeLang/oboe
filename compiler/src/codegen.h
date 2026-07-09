/* © 2026 Sushii64
 * © 2026 robinpie */
#ifndef OBOE_CODEGEN_H
#define OBOE_CODEGEN_H

#include "ast.h"
#include <stdio.h>

void codegen_set_source_dir(const char *dir);
void codegen_program(Program *prog, FILE *out);

#endif
