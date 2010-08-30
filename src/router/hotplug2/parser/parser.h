#ifndef RULES_PARSER_H
#define RULES_PARSER_H 1

#include "../rules/condition.h"
#include "../rules/expression.h"
#include "../rules/rule.h"
#include "../rules/ruleset.h"

#include "lexer.h"
#include "token_queue.h"
#include "buffer.h"
#include "token.h"

struct parser_ctx_t {
	struct buffer_t buffer;
	struct lexer_ctx_t lexer;
	struct token_queue_t tokenqueue;
	struct token_t *token;

	struct ruleset_t *ruleset;
	struct rule_t *last_rule;
};

#define PRINTFUNC()
int parser_stream(FILE *, struct ruleset_t *);
int parser_file(const char *, struct ruleset_t *);

#endif /* RULES_PARSER_H */
