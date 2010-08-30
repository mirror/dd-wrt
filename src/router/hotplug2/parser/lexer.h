#ifndef RULES_LEXER_H
#define RULES_LEXER_H 1

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"
#include "buffer.h"

struct lexer_ctx_t {
	FILE	*fp;
	char	*filename;
	int		charno, lineno;
};

void lexer_init(struct lexer_ctx_t *);
void lexer_clear(struct lexer_ctx_t *);
struct token_t *lexer_read_token(struct lexer_ctx_t *, struct buffer_t *);

#endif /* RULES_LEXER_H */
