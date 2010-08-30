#include "lexer.h"

static inline int isword(int c) {
	return (c >= '?' && c <= 'z') || \
	       (c >= '#' && c <= '+') || \
	       (c >= '0' && c <= '9') || \
	       (c >= '-' && c <= '/') || \
            c == ':' || c == ';' || c == '|';
}

void lexer_init(struct lexer_ctx_t *ctx) {
	ctx->fp = NULL;
	ctx->filename = NULL;
	ctx->charno = ctx->lineno = 0;
}

void lexer_clear(struct lexer_ctx_t *ctx) {
	free(ctx->filename);
	ctx->filename = NULL;
}

struct token_t *lexer_read_token(struct lexer_ctx_t *ctx, struct buffer_t *buffer) {
	int status;
	int c;
	struct token_t *token;

	token = malloc(sizeof(struct token_t));
	token_init(token);

	buffer_flush(buffer);

	status = 0;
	while (1) {
		c = getc(ctx->fp);

		if (c != '\n') {
			ctx->charno++;
		} else {
			ctx->lineno++;
			ctx->charno = 0;
		}

		switch (status) {
			/* Start of automaton */
			case 0:
				if (c == EOF) {
					token_set(token, buffer, TOKEN_EOF);
					return token;
				}
				if (c == '"') { status = 11; break; }
				if (c == '\n') { status = 13; break; }
				if (c == '#') { status = 15; break; }
				if (isspace(c)) { break; }

				buffer_push(buffer, c);

				if (c == ',') { 
					token_set(token, buffer, TOKEN_COMMA);
					return token;
				}
				if (c == '{') { 
					token_set(token, buffer, TOKEN_BROP);
					return token;
				}
				if (c == '}') { 
					token_set(token, buffer, TOKEN_BRCL);
					return token;
				}
				if (c == '$') { status = 1; break; }
				if (c == 'i') { status = 3; break; }
				if (c == 's') { status = 8; break; }
				if (c == 'u') { status = 16; break; }
				if (c == '=') { status = 6; break; }
				if (c == '~') { status = 7; break; }
				if (c == '!') { status = 14; break; }
				if (isupper(c)) { status = 2; break; }
				if (isword(c)) { status = 5; break; }

				token_set(token, buffer, TOKEN_ERROR);
				return token;

			/* $include... */
			case 1:
				if (isalpha(c)) { 
					buffer_push(buffer, c);
					break;
				}
				ungetc(c, ctx->fp);
				token_set(token, buffer, TOKEN_ROOTKW);
				return token;

			/* ACTION... */
			case 2:
				if (isupper(c)) {
					buffer_push(buffer, c);
					break;
				}
				ungetc(c, ctx->fp);
				token_set(token, buffer, TOKEN_VARNAME);
				return token;

			/* "is" */
			case 3:
				if (c == 's') {
					buffer_push(buffer, c);
					status = 4;
					break;
				}
				ungetc(c, ctx->fp);
				status = 5;
				break;

			/* "is"*/
			case 4:
				if (isword(c)) { 
					buffer_push(buffer, c);
					status = 5;
					break;
				}
				ungetc(c, ctx->fp);
				token_set(token, buffer, TOKEN_CMP_IS);
				return token;

			/* Any word */
			case 5:
				if (isword(c)) {
					buffer_push(buffer, c);
					break;
				}
				ungetc(c, ctx->fp);
				token_set(token, buffer, TOKEN_WORD);
				return token;

			/* "==" */
			case 6:
				if (c == '=') {
					buffer_push(buffer, c);
					token_set(token, buffer, TOKEN_CMP_EQ);
					return token;
				}
				token_set(token, buffer, TOKEN_ERROR);
				return token;

			/* "~~" */
			case 7:
				if (c == '~') {
					buffer_push(buffer, c);
					token_set(token, buffer, TOKEN_CMP_REEQ);
					return token;
				}
				token_set(token, buffer, TOKEN_ERROR);
				return token;

			/* "set" */
			case 8:
				if (c == 'e') {
					buffer_push(buffer, c);
					status = 9;
					break;
				}
				ungetc(c, ctx->fp);
				status = 5;
				break;

			/* "set" */
			case 9:
				if (c == 't') {
					buffer_push(buffer, c);
					status = 10;
					break;
				}
				ungetc(c, ctx->fp);
				status = 5;
				break;

			/* "set" */
			case 10:
				if (isword(c)) {
					buffer_push(buffer, c);
					status = 5;
					break;
				}
				ungetc(c, ctx->fp);
				token_set(token, buffer, TOKEN_KW_SET);
				return token;

			/* quoted word */
			case 11:
				if (c == '\\') {
					status = 12;
					break;
				}
				if (c == '"') {
					token_set(token, buffer, TOKEN_WORD);
					return token;
				}
				buffer_push(buffer, c);
				break;

			/* escape sequence */
			case 12:
				switch (c) {
					case 'n':
						buffer_push(buffer, '\n');
						break;
					case 't':
						buffer_push(buffer, '\t');
						break;
					case 'r':
						buffer_push(buffer, '\r');
						break;
					default:
						buffer_push(buffer, c);
				}
				status = 11;
				break;

			/* newline(s) */
			case 13:
				if (c == '\n') { break; }

				ungetc(c, ctx->fp);
				token_set(token, buffer, TOKEN_NEWLINE);
				return token;

			/* "!=" or "!~" */
			case 14:
				if (c == '=') {
					buffer_push(buffer, c);
					token_set(token, buffer, TOKEN_CMP_NE);
					return token;
				}
				if (c == '~') {
					buffer_push(buffer, c);
					token_set(token, buffer, TOKEN_CMP_RENE);
					return token;
				}
				token_set(token, buffer, TOKEN_ERROR);
				return token;
			
			/* Comment */
			case 15:
				if (c == '\n') { status = 0; }
				break;

			/* "unset" */
			case 16:
				if (c == 'n') {
					buffer_push(buffer, c);
					status = 17;
					break;
				}
				token_set(token, buffer, TOKEN_ERROR);
				return token;

			/* "unset" */
			case 17:
				if (c == 's') {
					buffer_push(buffer, c);
					status = 18;
					break;
				}
				token_set(token, buffer, TOKEN_ERROR);
				return token;

			/* "unset" */
			case 18:
				if (c == 'e') {
					buffer_push(buffer, c);
					status = 19;
					break;
				}
				token_set(token, buffer, TOKEN_ERROR);
				return token;

			/* "unset" */
			case 19:
				if (c == 't') {
					buffer_push(buffer, c);
					status = 20;
					break;
				}
				token_set(token, buffer, TOKEN_ERROR);
				return token;

			/* "unset" */
			case 20:
				if (isword(c)) {
					buffer_push(buffer, c);
					status = 5;
					break;
				}
				ungetc(c, ctx->fp);
				token_set(token, buffer, TOKEN_KW_UNSET);
				return token;
		}
	}

	token_set(token, buffer, TOKEN_ERROR);
	return token;
}
