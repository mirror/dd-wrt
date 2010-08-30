#include "token.h"

void token_init(struct token_t *token) {
	token->token = NULL;
	token->type = TOKEN_ERROR;
}

void token_set(struct token_t *token, struct buffer_t *buffer, int token_type) {
	token->token = malloc(buffer->bufpos + 1);
	memcpy(token->token, buffer->buf, buffer->bufpos);
	token->token[buffer->bufpos] = '\0';
	token->type = token_type;
}

void token_free(struct token_t *token) {
	free(token->token);
	free(token);
}
