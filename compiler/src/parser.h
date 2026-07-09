#ifndef OBOE_PARSER_H
#define OBOE_PARSER_H

#include "ast.h"
#include "lexer.h"

Program *parse_program(Token *tokens, int count, const char *filename);

#endif
