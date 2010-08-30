#ifndef RULES_TOKEN_QUEUE_H
#define RULES_TOKEN_QUEUE_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"

struct token_queue_t {
	struct token_t **queue;
	int		size;
	int		top;
};

void token_queue_init(struct token_queue_t *);
void token_queue_push(struct token_queue_t *, struct token_t *);
struct token_t *token_queue_pop(struct token_queue_t *);
struct token_t *token_queue_dequeue(struct token_queue_t *);
char **token_queue_strings(struct token_queue_t *);
void token_queue_inspect(struct token_queue_t *);
int token_queue_size(struct token_queue_t *);
void token_queue_empty(struct token_queue_t *);
void token_queue_free(struct token_queue_t *);

#endif /* RULES_TOKEN_QUEUE_H */
