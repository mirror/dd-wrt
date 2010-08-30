#include "parser.h"

/*
 * Grammar: 
 * input => directive // TOKEN_ROOTKW
 * input => rule      // TOKEN_VARNAME
 * input => TOKEN_NEWLINE
 * input => TOKEN_EOF
 *
 * directive => TOKEN_ROOTKW words-zero-or-more TOKEN_NEWLINE
 *
 * words-zero-or-more => <epsilon> // TOKEN_NEWLINE
 * words-zero-or-more => word words-zero-or-more
 *
 * rule => conditions TOKEN_BROP expressions TOKEN_BRCL
 *
 * conditions => condition-lvalue other-conditions
 *
 * other-conditions => <epsilon>  // TOKEN_BROP
 * other-conditions => TOKEN_COMMA condition-lvalue other-conditions
 *
 * condition-lvalue => TOKEN_VARNAME condition-rvalue
 *
 * condition-rvalue => TOKEN_CMP_EQ word
 * condition-rvalue => TOKEN_CMP_NE word
 * condition-rvalue => TOKEN_CMP_REEQ word
 * condition-rvalue => TOKEN_CMP_RENE word
 * condition-rvalue => TOKEN_CMP_IS condition-isset
 *
 * condition-isset => TOKEN_KW_SET
 * condition-isset => TOKEN_KW_UNSET
 *
 * expressions => expression other-expressions
 * other-expressions => <epsilon> // TOKEN_BRCL
 * other-expressions => expression other-expressions // word
 *
 * expression => word words-zero-or-more TOKEN_NEWLINE
 *
 * word => TOKEN_WORD
 * word => TOKEN_VARNAME
 * word => TOKEN_CMP_IS
 * word => TOKEN_KW_SET
 * word => TOKEN_KW_UNSET
 *
 * Newlines are mostly ignored -- this has been skipped for brevity.
 */

static int parser_util_add_condition(struct parser_ctx_t *ctx) {
	int cmp;
	struct condition_t *condition;
	struct token_t *cmptype;
	struct token_t *lvalue;
	struct token_t *rvalue;

	lvalue = token_queue_dequeue(&ctx->tokenqueue);
	cmptype = token_queue_dequeue(&ctx->tokenqueue);
	rvalue = token_queue_dequeue(&ctx->tokenqueue);

	if (lvalue == NULL || cmptype == NULL || rvalue == NULL)
		return -1;

	switch (cmptype->type) {
		case TOKEN_CMP_EQ:
			cmp = CONDITION_CMP_EQ;
			break;
			
		case TOKEN_CMP_NE:
			cmp = CONDITION_CMP_NE;
			break;
			
		case TOKEN_CMP_REEQ:
			cmp = CONDITION_CMP_REEQ;
			break;
			
		case TOKEN_CMP_RENE:
			cmp = CONDITION_CMP_RENE;
			break;
			
		case TOKEN_CMP_IS:
			if (rvalue->type == TOKEN_KW_SET) {
				cmp = CONDITION_CMP_SET;
				break;
			} else if (rvalue->type == TOKEN_KW_UNSET) {
				cmp = CONDITION_CMP_UNSET;
				break;
			} else {
				return -1;
			}

		default:
			return -1;
	}

	condition = condition_create(lvalue->token, rvalue->token, cmp);
	if (condition == NULL)
		return -1;
	
	rule_add_condition(ctx->last_rule, condition);

	token_free(lvalue);
	token_free(cmptype);
	token_free(rvalue);
	return 0;
}

static int parser_util_add_expression(struct parser_ctx_t *ctx) {
	int argc;
	int i;
	char **argv;
	struct expression_t *expression;
	struct token_t *command;

	command = token_queue_dequeue(&ctx->tokenqueue);
	if (command == NULL)
		return -1;

	argv = token_queue_strings(&ctx->tokenqueue);
	argc = token_queue_size(&ctx->tokenqueue);
	token_queue_empty(&ctx->tokenqueue);
	
	expression = expression_create(command->token, argc, argv);
	if (expression == NULL) {
		for (i = 0; i < argc; i++) {
			free(argv[i]);
		}
		free(argv);

		/*
		 * This is a trick to give the user some idea what went wrong
		 */
		token_free(ctx->token);
		ctx->token = command;
		return -1;
	}

	rule_add_expression(ctx->last_rule, expression);
	token_free(command);

	return 0;
}

static int parser_util_add_rule(struct parser_ctx_t *ctx) {
	ruleset_add_rule(ctx->ruleset, ctx->last_rule);
	ctx->last_rule = rule_create();
	return 0;
}

static inline void parser_util_skipnl(struct parser_ctx_t *ctx) {
	PRINTFUNC();
	while (ctx->token->type == TOKEN_NEWLINE) {
		token_free(ctx->token);
		ctx->token = lexer_read_token(&ctx->lexer, &ctx->buffer);
	}
}

static inline int parser_util_isword(struct parser_ctx_t *ctx) {
	switch (ctx->token->type) {
		case TOKEN_WORD:
		case TOKEN_VARNAME:
		case TOKEN_CMP_IS:
		case TOKEN_KW_SET:
		case TOKEN_KW_UNSET:
			return 1;
	}

	return 0;
}

/*
 * word => TOKEN_WORD
 * word => TOKEN_VARNAME
 * word => TOKEN_CMP_IS
 * word => TOKEN_KW_SET
 * word => TOKEN_KW_UNSET
 *
 * FIRST: all of the above
 */
static int parser_word(struct parser_ctx_t *ctx) {
	PRINTFUNC();

	if (parser_util_isword(ctx)) { 
		ctx->token = lexer_read_token(&ctx->lexer, &ctx->buffer);
		return 0;
	}
	return -1;
}

/*
 * words-zero-or-more => <epsilon>
 * words-zero-or-more => word words-zero-or-more
 *
 * FIRST: word
 * FOLLOW: TOKEN_NEWLINE
 */
static int parser_words_zero_or_more(struct parser_ctx_t *ctx) {
	PRINTFUNC();

	if (parser_util_isword(ctx)) {
		token_queue_push(&ctx->tokenqueue, ctx->token);
		if (parser_word(ctx))
			return 0;
		return parser_words_zero_or_more(ctx);
	}

	if (ctx->token->type == TOKEN_NEWLINE) {
		return 0;
	}
	
	return -1;
}

/*
 * condition-isset => TOKEN_KW_SET
 * condition-isset => TOKEN_KW_UNSET
 *
 * FIRST: all of the above
 */
static int parser_condition_isset(struct parser_ctx_t *ctx) {
	PRINTFUNC();

	switch (ctx->token->type) {
		case TOKEN_KW_SET:
		case TOKEN_KW_UNSET:
			ctx->token = lexer_read_token(&ctx->lexer, &ctx->buffer);
			return 0;
	}

	return -1;
}

/*
 * condition-rvalue => TOKEN_CMP_EQ word
 * condition-rvalue => TOKEN_CMP_NE word
 * condition-rvalue => TOKEN_CMP_REEQ word
 * condition-rvalue => TOKEN_CMP_RENE word
 * condition-rvalue => TOKEN_CMP_IS condition-isset
 *
 * FIRST: TOKEN_CMP_EQ, TOKEN_CMP_NE, TOKEN_CMP_REEQ,
 *        TOKEN_CMP_RENE, TOKEN_CMP_IS
 */
static int parser_condition_rvalue(struct parser_ctx_t *ctx) {
	PRINTFUNC();

	switch (ctx->token->type) {
		case TOKEN_CMP_EQ:
		case TOKEN_CMP_NE:
		case TOKEN_CMP_REEQ:
		case TOKEN_CMP_RENE:
			token_queue_push(&ctx->tokenqueue, ctx->token);
			ctx->token = lexer_read_token(&ctx->lexer, &ctx->buffer);

			token_queue_push(&ctx->tokenqueue, ctx->token);
			if (parser_word(ctx))
				return -1;

			return parser_util_add_condition(ctx);

		case TOKEN_CMP_IS:
			token_queue_push(&ctx->tokenqueue, ctx->token);
			ctx->token = lexer_read_token(&ctx->lexer, &ctx->buffer);

			token_queue_push(&ctx->tokenqueue, ctx->token);
			if (parser_condition_isset(ctx))
				return -1;

			return parser_util_add_condition(ctx);
	}
	return -1;
}

/*
 * condition-lvalue => TOKEN_VARNAME condition-rvalue
 *
 * FIRST: TOKEN_VARNAME
 */
static int parser_condition_lvalue(struct parser_ctx_t *ctx) {
	PRINTFUNC();
	if (ctx->token->type != TOKEN_VARNAME)
		return -1;

	token_queue_push(&ctx->tokenqueue, ctx->token);
	ctx->token = lexer_read_token(&ctx->lexer, &ctx->buffer);

	return parser_condition_rvalue(ctx);
}

/*
 * expression => word words-zero-or-more TOKEN_NEWLINE
 *
 * FIRST: Any of word's FIRST (see parser_word)
 */
static int parser_expression(struct parser_ctx_t *ctx) {
	PRINTFUNC();

	if (!parser_util_isword(ctx))
		return -1;

	token_queue_push(&ctx->tokenqueue, ctx->token);
	ctx->token = lexer_read_token(&ctx->lexer, &ctx->buffer);

	if (parser_words_zero_or_more(ctx))
		return -1;

	if (ctx->token->type != TOKEN_NEWLINE)
		return -1;
	token_free(ctx->token);
	ctx->token = lexer_read_token(&ctx->lexer, &ctx->buffer);

	return parser_util_add_expression(ctx);
}

/*
 * expressions => expression other-expressions
 * other-expressions => <epsilon>
 * other-expressions => expression other-expressions
 *
 * FIRST: Any of word's FIRST (see parser_word)
 * FOLLOW: TOKEN_BRCL
 */
static int parser_other_expressions(struct parser_ctx_t *ctx) {
	PRINTFUNC();

	parser_util_skipnl(ctx);

	if (parser_util_isword(ctx)) {
		if (parser_expression(ctx))
			return -1;
		return parser_other_expressions(ctx);
	}

	if (ctx->token->type == TOKEN_BRCL)
		return 0;

	return 0;
}

/*
 * expressions => expression other-expressions
 *
 * FIRST: Any of word's FIRST (see parser_word)
 */
static int parser_expressions(struct parser_ctx_t *ctx) {
	PRINTFUNC();

	if (parser_expression(ctx))
		return -1;
	
	if (parser_other_expressions(ctx))
		return -1;

	return 0;
}

/*
 * other-conditions => <epsilon>  // TOKEN_BROP
 * other-conditions => TOKEN_COMMA condition-lvalue other-conditions
 *
 * FIRST: TOKEN_COMMA
 * FOLLOW: TOKEN_BROP
 */
static int parser_other_conditions(struct parser_ctx_t *ctx) {
	PRINTFUNC();

	parser_util_skipnl(ctx);

	switch (ctx->token->type) {
		case TOKEN_BROP:
			return 0;

		case TOKEN_COMMA:
			token_free(ctx->token);
			ctx->token = lexer_read_token(&ctx->lexer, &ctx->buffer);
			parser_util_skipnl(ctx);
			if (parser_condition_lvalue(ctx))
				return -1;

			return parser_other_conditions(ctx);
	}
	return -1;
}

/*
 * conditions => condition-lvalue other-conditions
 *
 * FIRST: TOKEN_VARNAME
 */
static int parser_conditions(struct parser_ctx_t *ctx) {
	PRINTFUNC();

	if (parser_condition_lvalue(ctx))
		return -1;

	if (parser_other_conditions(ctx))
		return -1;

	return 0;
}

/*
 * directive => TOKEN_ROOTKW words-zero-or-more TOKEN_NEWLINE
 *
 * FIRST: TOKEN_ROOTKW
 */
static int parser_directive(struct parser_ctx_t *ctx) {
	struct token_t *token;

	PRINTFUNC();

	if (ctx->token->type != TOKEN_ROOTKW)
		return -1;
	token_queue_push(&ctx->tokenqueue, ctx->token);

	ctx->token = lexer_read_token(&ctx->lexer, &ctx->buffer);
	if (parser_words_zero_or_more(ctx))
		return -1;
	
	if (ctx->token->type != TOKEN_NEWLINE)
		return -1;
	token_free(ctx->token);
	ctx->token = NULL;

	token = token_queue_dequeue(&ctx->tokenqueue);
	if (!strcmp("$include", token->token)) {
		token_free(token);

		token = token_queue_dequeue(&ctx->tokenqueue);
		while (token != NULL) {
			if (parser_file(token->token, ctx->ruleset)) {
				fprintf(stderr, "Unable to include '%s'\n", token->token);
				token_free(token);
				return -1;
			}
			token_free(token);
			token = token_queue_dequeue(&ctx->tokenqueue);
		}
	} else {
		fprintf(stderr, "Unknown directive '%s'\n", token->token);
		return -1;
	}

	return 0;
}

/*
 * rule => conditions TOKEN_BROP expressions TOKEN_BRCL
 *
 * FIRST: TOKEN_VARNAME
 */
static int parser_rule(struct parser_ctx_t *ctx) {
	PRINTFUNC();

	if (parser_conditions(ctx))
		return -1;

	if (ctx->token->type != TOKEN_BROP)
		return -1;
	token_free(ctx->token);
	ctx->token = lexer_read_token(&ctx->lexer, &ctx->buffer);

	parser_util_skipnl(ctx);

	if (parser_expressions(ctx))
		return -1;

	if (ctx->token->type != TOKEN_BRCL)
		return -1;
	token_free(ctx->token);

	return parser_util_add_rule(ctx);
}
/*
 * input => directive
 * input => rule
 * input => TOKEN_NEWLINE
 * input => TOKEN_EOF
 *
 * FIRST: TOKEN_ROOTKW, TOKEN_VARNAME, TOKEN_NEWLINE, TOKEN_EOF
 */
static int parser_input(struct parser_ctx_t *ctx) {
	PRINTFUNC();

	ctx->token = lexer_read_token(&ctx->lexer, &ctx->buffer);

	parser_util_skipnl(ctx);

	switch (ctx->token->type) {
		case TOKEN_EOF:
			/* This is the only place where EOF is valid. */
			return 0;

		case TOKEN_ROOTKW:
			return parser_directive(ctx);
		
		case TOKEN_VARNAME:
			return parser_rule(ctx);
	}

	return -1;
}

static int parser_parse(struct parser_ctx_t *ctx) {
	int rv;

	rv = 0;
	while (!feof(ctx->lexer.fp)) {
		rv = parser_input(ctx);
		if (rv != 0)
			break;
	}

	if (rv != 0) {
		fprintf(stderr, "Syntax error (%s, line %d): ", ctx->lexer.filename, ctx->lexer.lineno);
		if (ctx->token != NULL) {
			switch (ctx->token->type) {
				case TOKEN_ERROR:
					fprintf(stderr, "Erroreous character after '%s'.\n", ctx->token->token);
					break;

				case TOKEN_EOF:
					fprintf(stderr, "Unexpected end of input.\n");
					break;

				default:
					fprintf(stderr, "Error near '%s'.\n", ctx->token->token);
					break;
			}
		} else {
			fprintf(stderr, "Syntax error: Internal parser error.\n");
		}
	}

	return rv;
}

static void parser_init(struct parser_ctx_t *ctx) {
	buffer_init(&ctx->buffer);
	lexer_init(&ctx->lexer);
	token_queue_init(&ctx->tokenqueue);
	ctx->token = NULL;
}

static void parser_clear(struct parser_ctx_t *ctx) {
	if (ctx->token != NULL)
		token_free(ctx->token);
	
	buffer_clear(&ctx->buffer);
	lexer_clear(&ctx->lexer);
	token_queue_free(&ctx->tokenqueue);
}

int parser_stream(FILE *fp, struct ruleset_t *ruleset) {
	struct parser_ctx_t ctx;
	int rv;

	parser_init(&ctx);

	ctx.lexer.fp = fp;
	ctx.lexer.filename = malloc(20);
	snprintf(ctx.lexer.filename, 20, "(stream %d)", fileno(fp));

	ctx.ruleset = ruleset;
	ctx.last_rule = rule_create();

	rv = parser_parse(&ctx);

	rule_free(ctx.last_rule);
	parser_clear(&ctx);

	return rv;
}

int parser_file(const char *filename, struct ruleset_t *ruleset) {
	struct parser_ctx_t ctx;
	int rv;

	parser_init(&ctx);

	ctx.lexer.fp = fopen(filename, "r");
	if (ctx.lexer.fp == NULL) {
		parser_clear(&ctx);
		return -1;
	}
	ctx.lexer.filename = strdup(filename);

	ctx.ruleset = ruleset;
	ctx.last_rule = rule_create();

	rv = parser_parse(&ctx);

	fclose(ctx.lexer.fp);
	rule_free(ctx.last_rule);
	parser_clear(&ctx);

	return rv;
}
