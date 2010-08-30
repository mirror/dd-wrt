#ifndef RULES_TOKEN_H
#define RULES_TOKEN_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

#define TOKEN_EOF     (-2)
#define TOKEN_ERROR   (-1)
#define TOKEN_NEWLINE   0	/* \n+ */
#define TOKEN_ROOTKW    1	/* \$[a-zA-Z]+ */
#define TOKEN_VARNAME   2	/* [A-Z]+ */
#define TOKEN_WORD      3	/* [a-zA-Z0-9]+ or quoted-word (with esc sequence) */
#define TOKEN_CMP_IS    4	/* is */
#define TOKEN_CMP_EQ    5	/* == */
#define TOKEN_CMP_NE    6	/* != */
#define TOKEN_CMP_REEQ  7	/* ~~ */
#define TOKEN_CMP_RENE  8	/* !~ */
#define TOKEN_KW_SET    9	/* set */
#define TOKEN_KW_UNSET  10	/* unset */
#define TOKEN_COMMA     11	/* , */
#define TOKEN_BROP      12	/* { */
#define TOKEN_BRCL      13	/* } */

struct token_t {
	char	*token;
	int		type;
};

void token_init(struct token_t *);
void token_set(struct token_t *, struct buffer_t *, int);
void token_free(struct token_t *);

#endif /* RULES_TOKEN_H */
