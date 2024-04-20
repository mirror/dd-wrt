/**
 * @file parser_yang.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief YANG parser
 *
 * Copyright (c) 2018 - 2024 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#include "parser_internal.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "context.h"
#include "dict.h"
#include "in_internal.h"
#include "log.h"
#include "ly_common.h"
#include "parser_schema.h"
#include "path.h"
#include "set.h"
#include "tree.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_free.h"
#include "tree_schema_internal.h"

struct lys_glob_unres;

/**
 * @brief Insert WORD into the libyang context's dictionary and store as TARGET.
 *
 * @param[in] CTX yang parser context to access libyang context.
 * @param[in] BUF buffer in case the word is not a constant and can be inserted directly (zero-copy)
 * @param[out] TARGET variable where to store the pointer to the inserted value.
 * @param[in] WORD string to store.
 * @param[in] LEN length of the string in WORD to store.
 */
#define INSERT_WORD_GOTO(CTX, BUF, TARGET, WORD, LEN, RET, LABEL) \
    if (BUF) {LY_CHECK_GOTO(RET = lydict_insert_zc(PARSER_CTX(CTX), WORD, &(TARGET)), LABEL);}\
    else {LY_CHECK_GOTO(RET = lydict_insert(PARSER_CTX(CTX), LEN ? WORD : "", LEN, &(TARGET)), LABEL);}

/**
 * @brief Read from the IN structure COUNT items. Also updates the indent value in yang parser context
 *
 * @param[in] CTX yang parser context to update its indent value.
 * @param[in] COUNT number of items for which the DATA pointer is supposed to move on.
 */
#define MOVE_INPUT(CTX, COUNT) ly_in_skip((CTX)->in, COUNT);(CTX)->indent+=COUNT

/**
 * @brief Loop through all substatements. Starts a for loop and ::YANG_READ_SUBSTMT_NEXT_ITER must be used at its end.
 *
 * @param[in] CTX yang parser context.
 * @param[out] KW YANG keyword read.
 * @param[out] WORD Pointer to the keyword itself.
 * @param[out] WORD_LEN Length of the keyword.
 * @param[out] RET Variable for error storing.
 * @param[in] ERR_LABEL Label to go to on error.
 */
#define YANG_READ_SUBSTMT_FOR_GOTO(CTX, KW, WORD, WORD_LEN, RET, ERR_LABEL) \
    ly_bool __loop_end = 0; \
    if ((RET = get_keyword(CTX, &KW, &WORD, &WORD_LEN))) { \
        goto ERR_LABEL; \
    } \
    if (KW == LY_STMT_SYNTAX_SEMICOLON) { \
        __loop_end = 1; \
    } else if (KW != LY_STMT_SYNTAX_LEFT_BRACE) { \
        LOGVAL_PARSER(CTX, LYVE_SYNTAX_YANG, "Invalid keyword \"%s\", expected \";\" or \"{\".", lyplg_ext_stmt2str(KW)); \
        RET = LY_EVALID; \
        goto ERR_LABEL; \
    } else { \
        YANG_READ_SUBSTMT_NEXT_ITER(CTX, KW, WORD, WORD_LEN, NULL, RET, ERR_LABEL); \
    } \
    while (!__loop_end)

/**
 * @brief Next iteration of ::YANG_READ_SUBSTMT_FOR_GOTO loop.
 *
 * @param[in] CTX yang parser context.
 * @param[out] KW YANG keyword read.
 * @param[out] WORD Pointer to the keyword itself.
 * @param[out] WORD_LEN Length of the keyword.
 * @param[in] EXTS Final extension instance array to store.
 * @param[out] RET Variable for error storing.
 * @param[in] ERR_LABEL Label to go to on error.
 */
#define YANG_READ_SUBSTMT_NEXT_ITER(CTX, KW, WORD, WORD_LEN, EXTS, RET, ERR_LABEL) \
    if ((RET = get_keyword(CTX, &KW, &WORD, &WORD_LEN))) { \
        goto ERR_LABEL; \
    } \
    if (KW == LY_STMT_SYNTAX_RIGHT_BRACE) { \
        if (EXTS && (RET = ly_set_add(&(CTX)->main_ctx->ext_inst, (EXTS), 1, NULL))) { \
            goto ERR_LABEL; \
        } \
        __loop_end = 1; \
    }

LY_ERR parse_container(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_uses(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_choice(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_case(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_list(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_grouping(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node_grp **groupings);

/**
 * @brief Add another character to dynamic buffer, a low-level function.
 *
 * Enlarge if needed. Updates \p input as well as \p buf_used.
 *
 * @param[in] ctx libyang context for logging.
 * @param[in,out] in Input structure.
 * @param[in] len Number of bytes to get from the input string and copy into the buffer.
 * @param[in,out] buf Buffer to use, can be moved by realloc().
 * @param[in,out] buf_len Current size of the buffer.
 * @param[in,out] buf_used Currently used characters of the buffer.
 * @return LY_ERR values.
 */
LY_ERR
buf_add_char(struct ly_ctx *ctx, struct ly_in *in, size_t len, char **buf, size_t *buf_len, size_t *buf_used)
{
#define BUF_STEP 16;
    if (*buf_len <= (*buf_used) + len) {
        *buf_len += BUF_STEP;
        *buf = ly_realloc(*buf, *buf_len);
        LY_CHECK_ERR_RET(!*buf, LOGMEM(ctx), LY_EMEM);
    }
    if (*buf_used) {
        ly_in_read(in, &(*buf)[*buf_used], len);
    } else {
        ly_in_read(in, *buf, len);
    }

    (*buf_used) += len;
    return LY_SUCCESS;
#undef BUF_STEP
}

/**
 * @brief Store a single UTF8 character. It depends whether in a dynamically-allocated buffer or just as a pointer to the data.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] arg Type of the input string to select method of checking character validity.
 * @param[in,out] word_p Word pointer. If buffer (\p word_b) was not yet needed, it is just a pointer to the first
 * stored character. If buffer was needed (\p word_b is non-NULL or \p need_buf is set), it is pointing to the buffer.
 * @param[in,out] word_len Current length of the word pointed to by \p word_p.
 * @param[in,out] word_b Word buffer. Is kept NULL as long as it is not requested (word is a substring of the data).
 * @param[in,out] buf_len Current length of \p word_b.
 * @param[in] need_buf Flag if the dynamically allocated buffer is required.
 * @param[in,out] prefix Storage for internally used flag in case of possible prefixed identifiers:
 * 0 - colon not yet found (no prefix)
 * 1 - \p c is the colon character
 * 2 - prefix already processed, now processing the identifier
 * @return LY_ERR values.
 */
LY_ERR
buf_store_char(struct lysp_yang_ctx *ctx, enum yang_arg arg, char **word_p, size_t *word_len,
        char **word_b, size_t *buf_len, ly_bool need_buf, uint8_t *prefix)
{
    uint32_t c;
    size_t len;

    /* check  valid combination of input paremeters - if need_buf specified, word_b must be provided */
    assert(!need_buf || (need_buf && word_b));

    /* get UTF8 code point (and number of bytes coding the character) */
    LY_CHECK_ERR_RET(ly_getutf8(&ctx->in->current, &c, &len),
            LOGVAL_PARSER(ctx, LY_VCODE_INCHAR, ctx->in->current[-len]), LY_EVALID);
    ctx->in->current -= len;
    if (c == '\n') {
        ctx->indent = 0;
        LY_IN_NEW_LINE(ctx->in);
    } else {
        /* note - even the multibyte character is count as 1 */
        ++ctx->indent;
    }

    /* check character validity */
    switch (arg) {
    case Y_IDENTIF_ARG:
        LY_CHECK_RET(lysp_check_identifierchar((struct lysp_ctx *)ctx, c, !(*word_len), NULL));
        break;
    case Y_PREF_IDENTIF_ARG:
        LY_CHECK_RET(lysp_check_identifierchar((struct lysp_ctx *)ctx, c, !(*word_len), prefix));
        break;
    case Y_STR_ARG:
    case Y_MAYBE_STR_ARG:
        LY_CHECK_RET(lysp_check_stringchar((struct lysp_ctx *)ctx, c));
        break;
    }

    if (word_b && *word_b) {
        /* add another character into buffer */
        if (buf_add_char(PARSER_CTX(ctx), ctx->in, len, word_b, buf_len, word_len)) {
            return LY_EMEM;
        }

        /* in case of realloc */
        *word_p = *word_b;
    } else if (word_b && need_buf) {
        /* first time we need a buffer, copy everything read up to now */
        if (*word_len) {
            *word_b = malloc(*word_len);
            LY_CHECK_ERR_RET(!*word_b, LOGMEM(PARSER_CTX(ctx)), LY_EMEM);
            *buf_len = *word_len;
            memcpy(*word_b, *word_p, *word_len);
        }

        /* add this new character into buffer */
        if (buf_add_char(PARSER_CTX(ctx), ctx->in, len, word_b, buf_len, word_len)) {
            return LY_EMEM;
        }

        /* in case of realloc */
        *word_p = *word_b;
    } else {
        /* just remember the first character pointer */
        if (!*word_p) {
            *word_p = (char *)ctx->in->current;
        }
        /* ... and update the word's length */
        (*word_len) += len;
        ly_in_skip(ctx->in, len);
    }

    return LY_SUCCESS;
}

/**
 * @brief Skip YANG comment in data.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] comment Type of the comment to process:
 *                    1 for a one-line comment,
 *                    2 for a block comment.
 * @return LY_ERR values.
 */
LY_ERR
skip_comment(struct lysp_yang_ctx *ctx, uint8_t comment)
{
    /* internal statuses: */
#define COMMENT_NO        0 /* comment ended */
#define COMMENT_LINE      1 /* in line comment */
#define COMMENT_BLOCK     2 /* in block comment */
#define COMMENT_BLOCK_END 3 /* in block comment with last read character '*' */

    while (ctx->in->current[0] && comment) {
        switch (comment) {
        case COMMENT_LINE:
            if (ctx->in->current[0] == '\n') {
                comment = COMMENT_NO;
                LY_IN_NEW_LINE(ctx->in);
            }
            break;
        case COMMENT_BLOCK:
            if (ctx->in->current[0] == '*') {
                comment = COMMENT_BLOCK_END;
            } else if (ctx->in->current[0] == '\n') {
                LY_IN_NEW_LINE(ctx->in);
            }
            break;
        case COMMENT_BLOCK_END:
            if (ctx->in->current[0] == '/') {
                comment = COMMENT_NO;
            } else if (ctx->in->current[0] != '*') {
                if (ctx->in->current[0] == '\n') {
                    LY_IN_NEW_LINE(ctx->in);
                }
                comment = COMMENT_BLOCK;
            }
            break;
        default:
            LOGINT_RET(PARSER_CTX(ctx));
        }

        if (ctx->in->current[0] == '\n') {
            ctx->indent = 0;
        } else {
            ++ctx->indent;
        }
        ++ctx->in->current;
    }

    if (!ctx->in->current[0] && (comment >= COMMENT_BLOCK)) {
        LOGVAL_PARSER(ctx, LYVE_SYNTAX, "Unexpected end-of-input, non-terminated comment.");
        return LY_EVALID;
    }

    return LY_SUCCESS;

#undef COMMENT_NO
#undef COMMENT_LINE
#undef COMMENT_BLOCK
#undef COMMENT_BLOCK_END
}

/**
 * @brief Read a quoted string from data.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] arg Type of YANG keyword argument expected.
 * @param[out] word_p Pointer to the read quoted string.
 * @param[out] word_b Pointer to a dynamically-allocated buffer holding the read quoted string. If not needed,
 * set to NULL. Otherwise equal to \p word_p.
 * @param[out] word_len Length of the read quoted string.
 * @param[out] buf_len Length of the dynamically-allocated buffer \p word_b.
 * @param[in] indent Current indent (number of YANG spaces). Needed for correct multi-line string
 * indenation in the final quoted string.
 * @return LY_ERR values.
 */
static LY_ERR
read_qstring(struct lysp_yang_ctx *ctx, enum yang_arg arg, char **word_p, char **word_b,
        size_t *word_len, size_t *buf_len)
{
    /* string parsing status: */
#define STRING_ENDED 0 /* string ended */
#define STRING_SINGLE_QUOTED 1 /* string with ' */
#define STRING_DOUBLE_QUOTED 2 /* string with " */
#define STRING_DOUBLE_QUOTED_ESCAPED 3 /* string with " with last character \ */
#define STRING_PAUSED_NEXTSTRING     4 /* string finished, now skipping whitespaces looking for + */
#define STRING_PAUSED_CONTINUE       5 /* string continues after +, skipping whitespaces */

    uint8_t string;
    uint64_t block_indent = 0, current_indent = 0;
    ly_bool need_buf = 0;
    uint8_t prefix = 0;
    const char *c;
    uint64_t trailing_ws = 0; /* current number of stored trailing whitespace characters */

    if (ctx->in->current[0] == '\"') {
        string = STRING_DOUBLE_QUOTED;
        current_indent = block_indent = ctx->indent + 1;
    } else {
        assert(ctx->in->current[0] == '\'');
        string = STRING_SINGLE_QUOTED;
    }
    MOVE_INPUT(ctx, 1);

    while (ctx->in->current[0] && string) {
        switch (string) {
        case STRING_SINGLE_QUOTED:
            if (ctx->in->current[0] == '\'') {
                /* string may be finished, but check for + */
                string = STRING_PAUSED_NEXTSTRING;
                MOVE_INPUT(ctx, 1);
            } else {
                /* check and store character */
                LY_CHECK_RET(buf_store_char(ctx, arg, word_p, word_len, word_b, buf_len, need_buf, &prefix));
            }
            break;
        case STRING_DOUBLE_QUOTED:
            switch (ctx->in->current[0]) {
            case '\"':
                /* string may be finished, but check for + */
                string = STRING_PAUSED_NEXTSTRING;
                MOVE_INPUT(ctx, 1);
                trailing_ws = 0;
                break;
            case '\\':
                /* special character following */
                string = STRING_DOUBLE_QUOTED_ESCAPED;

                /* the backslash sequence is substituted, so we will need a buffer to store the result */
                need_buf = 1;

                /* move forward to the escaped character */
                ++ctx->in->current;

                /* note that the trailing whitespaces are supposed to be trimmed before substitution of
                 * backslash-escaped characters (RFC 7950, 6.1.3), so we have to zero the trailing whitespaces counter */
                trailing_ws = 0;

                /* since the backslash-escaped character is handled as first non-whitespace character, stop eating indentation */
                current_indent = block_indent;
                break;
            case ' ':
                if (current_indent < block_indent) {
                    ++current_indent;
                    MOVE_INPUT(ctx, 1);
                } else {
                    /* check and store whitespace character */
                    LY_CHECK_RET(buf_store_char(ctx, arg, word_p, word_len, word_b, buf_len, need_buf, &prefix));
                    trailing_ws++;
                }
                break;
            case '\t':
                if (current_indent < block_indent) {
                    assert(need_buf);
                    current_indent += Y_TAB_SPACES;
                    ctx->indent += Y_TAB_SPACES;
                    for ( ; current_indent > block_indent; --current_indent, --ctx->indent) {
                        /* store leftover spaces from the tab */
                        c = ctx->in->current;
                        ctx->in->current = " ";
                        LY_CHECK_RET(buf_store_char(ctx, arg, word_p, word_len, word_b, buf_len, need_buf, &prefix));
                        ctx->in->current = c;
                        trailing_ws++;
                    }
                    ++ctx->in->current;
                } else {
                    /* check and store whitespace character */
                    LY_CHECK_RET(buf_store_char(ctx, arg, word_p, word_len, word_b, buf_len, need_buf, &prefix));
                    trailing_ws++;
                    /* additional characters for indentation - only 1 was count in buf_store_char */
                    ctx->indent += Y_TAB_SPACES - 1;
                }
                break;
            case '\r':
                /* newline may be escaped */
                if ((ctx->in->current[1] != '\n') && strncmp(&ctx->in->current[1], "\\n", 2)) {
                    LOGVAL_PARSER(ctx, LY_VCODE_INCHAR, ctx->in->current[0]);
                    return LY_EVALID;
                }

                /* skip this character, do not store it */
                ++ctx->in->current;
            /* fallthrough */
            case '\n':
                if (block_indent) {
                    /* we will be removing the indents so we need our own buffer */
                    need_buf = 1;

                    /* remove trailing tabs and spaces */
                    (*word_len) = *word_len - trailing_ws;

                    /* restart indentation */
                    current_indent = 0;
                }

                /* check and store character */
                LY_CHECK_RET(buf_store_char(ctx, arg, word_p, word_len, word_b, buf_len, need_buf, &prefix));

                /* reset context indentation counter for possible string after this one */
                ctx->indent = 0;
                trailing_ws = 0;
                break;
            default:
                /* first non-whitespace character, stop eating indentation */
                current_indent = block_indent;

                /* check and store character */
                LY_CHECK_RET(buf_store_char(ctx, arg, word_p, word_len, word_b, buf_len, need_buf, &prefix));
                trailing_ws = 0;
                break;
            }
            break;
        case STRING_DOUBLE_QUOTED_ESCAPED:
            /* string encoded characters */
            c = ctx->in->current;
            switch (ctx->in->current[0]) {
            case 'n':
                ctx->in->current = "\n";
                /* fix false newline count in buf_store_char() */
                ctx->in->line--;
                break;
            case 't':
                ctx->in->current = "\t";
                break;
            case '\"':
            case '\\':
                /* ok as is */
                break;
            default:
                LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Double-quoted string unknown special character '\\%c'.",
                        ctx->in->current[0]);
                return LY_EVALID;
            }

            /* check and store character */
            LY_CHECK_RET(buf_store_char(ctx, arg, word_p, word_len, word_b, buf_len, need_buf, &prefix));

            string = STRING_DOUBLE_QUOTED;
            ctx->in->current = c + 1;
            break;
        case STRING_PAUSED_NEXTSTRING:
            switch (ctx->in->current[0]) {
            case '+':
                /* string continues */
                string = STRING_PAUSED_CONTINUE;
                need_buf = 1;
                break;
            case '\r':
                if (ctx->in->current[1] != '\n') {
                    LOGVAL_PARSER(ctx, LY_VCODE_INCHAR, ctx->in->current[0]);
                    return LY_EVALID;
                }
                MOVE_INPUT(ctx, 1);
            /* fallthrough */
            case '\n':
                LY_IN_NEW_LINE(ctx->in);
            /* fall through */
            case ' ':
            case '\t':
                /* just skip */
                break;
            default:
                /* string is finished */
                goto string_end;
            }
            MOVE_INPUT(ctx, 1);
            break;
        case STRING_PAUSED_CONTINUE:
            switch (ctx->in->current[0]) {
            case '\r':
                if (ctx->in->current[1] != '\n') {
                    LOGVAL_PARSER(ctx, LY_VCODE_INCHAR, ctx->in->current[0]);
                    return LY_EVALID;
                }
                MOVE_INPUT(ctx, 1);
            /* fallthrough */
            case '\n':
                LY_IN_NEW_LINE(ctx->in);
            /* fall through */
            case ' ':
            case '\t':
                /* skip */
                break;
            case '\'':
                string = STRING_SINGLE_QUOTED;
                break;
            case '\"':
                string = STRING_DOUBLE_QUOTED;
                break;
            default:
                /* it must be quoted again */
                LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Both string parts divided by '+' must be quoted.");
                return LY_EVALID;
            }
            MOVE_INPUT(ctx, 1);
            break;
        default:
            return LY_EINT;
        }
    }

string_end:
    if ((arg <= Y_PREF_IDENTIF_ARG) && !(*word_len)) {
        /* empty identifier */
        LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Statement argument is required.");
        return LY_EVALID;
    }
    return LY_SUCCESS;

#undef STRING_ENDED
#undef STRING_SINGLE_QUOTED
#undef STRING_DOUBLE_QUOTED
#undef STRING_DOUBLE_QUOTED_ESCAPED
#undef STRING_PAUSED_NEXTSTRING
#undef STRING_PAUSED_CONTINUE
}

/**
 * @brief Get another YANG string from the raw data.
 *
 * @param[in] ctx Yang parser context for logging.
 * @param[in] arg Type of YANG keyword argument expected.
 * @param[out] flags Optional output argument to get flag of the argument's quoting (LYS_*QUOTED - see
 * [schema node flags](@ref snodeflags))
 * @param[out] word_p Pointer to the read string. Can return NULL if @p arg is #Y_MAYBE_STR_ARG.
 * @param[out] word_b Pointer to a dynamically-allocated buffer holding the read string. If not needed,
 * set to NULL. Otherwise equal to @p word_p.
 * @param[out] word_len Length of the read string.
 * @return LY_ERR values.
 */
LY_ERR
get_argument(struct lysp_yang_ctx *ctx, enum yang_arg arg, uint16_t *flags, char **word_p,
        char **word_b, size_t *word_len)
{
    LY_ERR ret;
    size_t buf_len = 0;
    uint8_t prefix = 0;
    ly_bool str_end = 0;

    /* word buffer - dynamically allocated */
    *word_b = NULL;

    /* word pointer - just a pointer to data */
    *word_p = NULL;

    *word_len = 0;
    while (!str_end) {
        switch (ctx->in->current[0]) {
        case '\'':
        case '\"':
            if (*word_len) {
                /* invalid - quotes cannot be in unquoted string and only optsep, ; or { can follow it */
                LOGVAL_PARSER(ctx, LY_VCODE_INSTREXP, 1, ctx->in->current,
                        "unquoted string character, optsep, semicolon or opening brace");
                ret = LY_EVALID;
                goto error;
            }
            if (flags) {
                (*flags) |= (ctx->in->current[0] == '\'') ? LYS_SINGLEQUOTED : LYS_DOUBLEQUOTED;
            }

            LY_CHECK_GOTO(ret = read_qstring(ctx, arg, word_p, word_b, word_len, &buf_len), error);
            if (!*word_p) {
                /* do not return NULL word */
                *word_p = "";
            }
            str_end = 1;
            break;
        case '/':
            if (ctx->in->current[1] == '/') {
                /* one-line comment */
                MOVE_INPUT(ctx, 2);
                LY_CHECK_GOTO(ret = skip_comment(ctx, 1), error);
            } else if (ctx->in->current[1] == '*') {
                /* block comment */
                MOVE_INPUT(ctx, 2);
                LY_CHECK_GOTO(ret = skip_comment(ctx, 2), error);
            } else {
                /* not a comment after all */
                LY_CHECK_GOTO(ret = buf_store_char(ctx, arg, word_p, word_len, word_b, &buf_len, 0, &prefix), error);
            }
            break;
        case ' ':
            if (*word_len) {
                /* word is finished */
                str_end = 1;
                break;
            }
            MOVE_INPUT(ctx, 1);
            break;
        case '\t':
            if (*word_len) {
                /* word is finished */
                str_end = 1;
                break;
            }
            /* tabs count for 8 spaces */
            ctx->indent += Y_TAB_SPACES;

            ++ctx->in->current;
            break;
        case '\r':
            if (ctx->in->current[1] != '\n') {
                LOGVAL_PARSER(ctx, LY_VCODE_INCHAR, ctx->in->current[0]);
                ret = LY_EVALID;
                goto error;
            }
            MOVE_INPUT(ctx, 1);
        /* fallthrough */
        case '\n':
            if (*word_len) {
                /* word is finished */
                str_end = 1;
                break;
            }
            LY_IN_NEW_LINE(ctx->in);
            MOVE_INPUT(ctx, 1);

            /* reset indent */
            ctx->indent = 0;
            break;
        case ';':
        case '{':
            if (*word_len || (arg == Y_MAYBE_STR_ARG)) {
                /* word is finished */
                str_end = 1;
                break;
            }

            LOGVAL_PARSER(ctx, LY_VCODE_INSTREXP, 1, ctx->in->current, "an argument");
            ret = LY_EVALID;
            goto error;
        case '}':
            /* invalid - braces cannot be in unquoted string (opening braces terminates the string and can follow it) */
            LOGVAL_PARSER(ctx, LY_VCODE_INSTREXP, 1, ctx->in->current,
                    "unquoted string character, optsep, semicolon or opening brace");
            ret = LY_EVALID;
            goto error;
        case '\0':
            /* unexpected EOF */
            LOGVAL_PARSER(ctx, LY_VCODE_EOF);
            ret = LY_EVALID;
            goto error;
        default:
            LY_CHECK_GOTO(ret = buf_store_char(ctx, arg, word_p, word_len, word_b, &buf_len, 0, &prefix), error);
            break;
        }
    }

    /* terminating NULL byte for buf */
    if (*word_b) {
        (*word_b) = ly_realloc(*word_b, (*word_len) + 1);
        LY_CHECK_ERR_RET(!(*word_b), LOGMEM(PARSER_CTX(ctx)), LY_EMEM);
        (*word_b)[*word_len] = '\0';
        *word_p = *word_b;
    }

    return LY_SUCCESS;

error:
    free(*word_b);
    *word_b = NULL;
    return ret;
}

/**
 * @brief Get another YANG keyword from the raw data.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[out] kw YANG keyword read.
 * @param[out] word_p Pointer to the keyword in the data. Useful for extension instances.
 * @param[out] word_len Length of the keyword in the data. Useful for extension instances.
 * @return LY_ERR values.
 */
LY_ERR
get_keyword(struct lysp_yang_ctx *ctx, enum ly_stmt *kw, char **word_p, size_t *word_len)
{
    uint8_t prefix;
    const char *word_start;
    size_t len;

    if (word_p) {
        *word_p = NULL;
        *word_len = 0;
    }

    /* first skip "optsep", comments */
    while (ctx->in->current[0]) {
        switch (ctx->in->current[0]) {
        case '/':
            if (ctx->in->current[1] == '/') {
                /* one-line comment */
                MOVE_INPUT(ctx, 2);
                LY_CHECK_RET(skip_comment(ctx, 1));
            } else if (ctx->in->current[1] == '*') {
                /* block comment */
                MOVE_INPUT(ctx, 2);
                LY_CHECK_RET(skip_comment(ctx, 2));
            } else {
                /* error - not a comment after all, keyword cannot start with slash */
                LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Invalid identifier first character '/'.");
                return LY_EVALID;
            }
            continue;
        case '\n':
            /* skip whitespaces (optsep) */
            LY_IN_NEW_LINE(ctx->in);
            ctx->indent = 0;
            break;
        case ' ':
            /* skip whitespaces (optsep) */
            ++ctx->indent;
            break;
        case '\t':
            /* skip whitespaces (optsep) */
            ctx->indent += Y_TAB_SPACES;
            break;
        case '\r':
            /* possible CRLF endline */
            if (ctx->in->current[1] == '\n') {
                break;
            }
        /* fallthrough */
        default:
            /* either a keyword start or an invalid character */
            goto keyword_start;
        }

        ly_in_skip(ctx->in, 1);
    }

keyword_start:
    word_start = ctx->in->current;
    *kw = lysp_match_kw(ctx->in, &ctx->indent);

    if (*kw == LY_STMT_SYNTAX_SEMICOLON) {
        goto success;
    } else if (*kw == LY_STMT_SYNTAX_LEFT_BRACE) {
        ctx->depth++;
        if (ctx->depth > LY_MAX_BLOCK_DEPTH) {
            LOGERR(PARSER_CTX(ctx), LY_EINVAL, "The maximum number of block nestings has been exceeded.");
            return LY_EINVAL;
        }
        goto success;
    } else if (*kw == LY_STMT_SYNTAX_RIGHT_BRACE) {
        ctx->depth--;
        goto success;
    }

    if (*kw != LY_STMT_NONE) {
        /* make sure we have the whole keyword */
        switch (ctx->in->current[0]) {
        case '\r':
            if (ctx->in->current[1] != '\n') {
                LOGVAL_PARSER(ctx, LY_VCODE_INCHAR, ctx->in->current[0]);
                return LY_EVALID;
            }
            MOVE_INPUT(ctx, 1);
        /* fallthrough */
        case '\n':
        case '\t':
        case ' ':
            /* mandatory "sep" is just checked, not eaten so nothing in the context is updated */
            break;
        case ':':
            /* keyword is not actually a keyword, but prefix of an extension.
             * To avoid repeated check of the prefix syntax, move to the point where the colon was read
             * and we will be checking the keyword (extension instance) itself */
            prefix = 1;
            MOVE_INPUT(ctx, 1);
            goto extension;
        case '{':
        case ';':
            /* allowed only for input and output statements which are without arguments */
            if ((*kw == LY_STMT_INPUT) || (*kw == LY_STMT_OUTPUT)) {
                break;
            }
        /* fall through */
        default:
            MOVE_INPUT(ctx, 1);
            LOGVAL_PARSER(ctx, LY_VCODE_INSTREXP, (int)(ctx->in->current - word_start), word_start,
                    "a keyword followed by a separator");
            return LY_EVALID;
        }
    } else {
        /* still can be an extension */
        prefix = 0;

extension:
        while (ctx->in->current[0] && (ctx->in->current[0] != ' ') && (ctx->in->current[0] != '\t') &&
                (ctx->in->current[0] != '\n') && (ctx->in->current[0] != '\r') && (ctx->in->current[0] != '{') &&
                (ctx->in->current[0] != ';')) {
            uint32_t c = 0;

            LY_CHECK_ERR_RET(ly_getutf8(&ctx->in->current, &c, &len),
                    LOGVAL_PARSER(ctx, LY_VCODE_INCHAR, ctx->in->current[-len]), LY_EVALID);
            ++ctx->indent;
            /* check character validity */
            LY_CHECK_RET(lysp_check_identifierchar((struct lysp_ctx *)ctx, c,
                    ctx->in->current - len == word_start ? 1 : 0, &prefix));
        }
        if (!ctx->in->current[0]) {
            LOGVAL_PARSER(ctx, LY_VCODE_EOF);
            return LY_EVALID;
        }

        /* prefix is mandatory for extension instances */
        if (prefix != 2) {
            LOGVAL_PARSER(ctx, LY_VCODE_INSTREXP, (int)(ctx->in->current - word_start), word_start, "a keyword");
            return LY_EVALID;
        }

        *kw = LY_STMT_EXTENSION_INSTANCE;
    }

success:
    if (word_p) {
        *word_p = (char *)word_start;
        *word_len = ctx->in->current - word_start;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse extension instance substatements.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] kw Statement keyword value matching @p word value.
 * @param[in] word Extension instance substatement name (keyword).
 * @param[in] word_len Extension instance substatement name length.
 * @param[in,out] child Children of this extension instance to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_ext_substmt(struct lysp_yang_ctx *ctx, enum ly_stmt kw, char *word, size_t word_len,
        struct lysp_stmt **child)
{
    char *buf;
    LY_ERR ret = LY_SUCCESS;
    enum ly_stmt child_kw;
    struct lysp_stmt *stmt, *par_child;

    stmt = calloc(1, sizeof *stmt);
    LY_CHECK_ERR_RET(!stmt, LOGMEM(NULL), LY_EMEM);

    /* insert into parent statements */
    if (!*child) {
        *child = stmt;
    } else {
        for (par_child = *child; par_child->next; par_child = par_child->next) {}
        par_child->next = stmt;
    }

    /* statement */
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), word, word_len, &stmt->stmt));

    /* get optional argument */
    LY_CHECK_RET(get_argument(ctx, Y_MAYBE_STR_ARG, &stmt->flags, &word, &buf, &word_len));
    if (word) {
        INSERT_WORD_GOTO(ctx, buf, stmt->arg, word, word_len, ret, cleanup);
    }

    stmt->format = LY_VALUE_SCHEMA;
    stmt->prefix_data = PARSER_CUR_PMOD(ctx);
    stmt->kw = kw;

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, child_kw, word, word_len, ret, cleanup) {
        LY_CHECK_GOTO(ret = parse_ext_substmt(ctx, child_kw, word, word_len, &stmt->child), cleanup)
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, child_kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse extension instance.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] ext_name Extension instance substatement name (keyword).
 * @param[in] ext_name_len Extension instance substatement name length.
 * @param[in] parent Current statement parent.
 * @param[in] parent_stmt Type of @p parent statement.
 * @param[in] parent_stmt_index In case of several @p parent_stmt, index of the relevant @p parent statement.
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_ext(struct lysp_yang_ctx *ctx, const char *ext_name, size_t ext_name_len, const void *parent,
        enum ly_stmt parent_stmt, LY_ARRAY_COUNT_TYPE parent_stmt_index, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    struct lysp_ext_instance *e;
    enum ly_stmt kw;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *exts, e, LY_EMEM);

    if (!ly_strnchr(ext_name, ':', ext_name_len)) {
        LOGVAL_PARSER(ctx, LYVE_SYNTAX, "Extension instance \"%.*s\" without the mandatory prefix.",
                (int)ext_name_len, ext_name);
        return LY_EVALID;
    }

    /* store name */
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), ext_name, ext_name_len, &e->name));

    /* get optional argument */
    LY_CHECK_RET(get_argument(ctx, Y_MAYBE_STR_ARG, NULL, &word, &buf, &word_len));
    if (word) {
        INSERT_WORD_GOTO(ctx, buf, e->argument, word, word_len, ret, cleanup);
    }

    /* store the rest of information */
    e->format = LY_VALUE_SCHEMA;
    e->parsed = NULL;
    e->prefix_data = PARSER_CUR_PMOD(ctx);
    e->parent = (void *)parent;
    e->parent_stmt = parent_stmt;
    e->parent_stmt_index = parent_stmt_index;

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        LY_CHECK_GOTO(ret = parse_ext_substmt(ctx, kw, word, word_len, &e->child), cleanup)
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse a generic text field without specific constraints. Those are contact, organization,
 * description, etc...
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] parent Current statement parent.
 * @param[in] parent_stmt Type of statement in @p value.
 * @param[in] parent_stmt_index In case of several @p parent_stmt, index of the relevant @p parent statement.
 * @param[in,out] value Place to store the parsed value.
 * @param[in] arg Type of the YANG keyword argument (of the value).
 * @param[out] flags Optional output argument to get flag of the argument's quoting (LYS_*QUOTED - see
 * [schema node flags](@ref snodeflags))
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_text_field(struct lysp_yang_ctx *ctx, const void *parent, enum ly_stmt parent_stmt, uint32_t parent_stmt_index,
        const char **value, enum yang_arg arg, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (*value) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, lyplg_ext_stmt2str(parent_stmt));
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, arg, flags, &word, &buf, &word_len));

    /* store value and spend buf if allocated */
    INSERT_WORD_GOTO(ctx, buf, *value, word, word_len, ret, cleanup);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, parent, parent_stmt, parent_stmt_index, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), lyplg_ext_stmt2str(parent_stmt));
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the yang-version statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] mod Module to fill.
 * @return LY_ERR values.
 */
static LY_ERR
parse_yangversion(struct lysp_yang_ctx *ctx, struct lysp_module *mod)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (mod->version) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "yang-version");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if ((word_len == 1) && !strncmp(word, "1", word_len)) {
        mod->version = LYS_VERSION_1_0;
    } else if ((word_len == ly_strlen_const("1.1")) && !strncmp(word, "1.1", word_len)) {
        mod->version = LYS_VERSION_1_1;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, "yang-version");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, mod, LY_STMT_YANG_VERSION, 0, &mod->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "yang-version");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the belongs-to statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] submod Submodule to fill.
 * @return LY_ERR values.
 */
static LY_ERR
parse_belongsto(struct lysp_yang_ctx *ctx, struct lysp_submodule *submod)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (submod->prefix) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "belongs-to");
        return LY_EVALID;
    }

    /* get value, it must match the main module */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    if (ly_strncmp(PARSER_CUR_PMOD(ctx)->mod->name, word, word_len)) {
        LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG, "Submodule \"belongs-to\" value \"%.*s\" does not match its module name \"%s\".",
                (int)word_len, word, PARSER_CUR_PMOD(ctx)->mod->name);
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_PREFIX:
            LY_CHECK_RET(parse_text_field(ctx, submod->prefix, LY_STMT_PREFIX, 0, &submod->prefix, Y_IDENTIF_ARG, NULL,
                    &submod->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, submod, LY_STMT_BELONGS_TO, 0, &submod->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "belongs-to");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

    /* mandatory substatements */
    if (!submod->prefix) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "prefix", "belongs-to");
        return LY_EVALID;
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the revision-date statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] rev Buffer to store the parsed value in.
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_revisiondate(struct lysp_yang_ctx *ctx, char *rev, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (rev[0]) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "revision-date");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len));

    /* check value */
    if (lysp_check_date((struct lysp_ctx *)ctx, word, word_len, "revision-date")) {
        free(buf);
        return LY_EVALID;
    }

    /* store value and spend buf if allocated */
    strncpy(rev, word, word_len);
    free(buf);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, rev, LY_STMT_REVISION_DATE, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "revision-date");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the include statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] module_name Name of the module to check name collisions.
 * @param[in,out] includes Parsed includes to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_include(struct lysp_yang_ctx *ctx, const char *module_name, struct lysp_include **includes)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_include *inc;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *includes, inc, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));

    INSERT_WORD_GOTO(ctx, buf, inc->name, word, word_len, ret, cleanup);

    /* submodules share the namespace with the module names, so there must not be
     * a module of the same name in the context, no need for revision matching */
    if (!strcmp(module_name, inc->name) || ly_ctx_get_module_latest(PARSER_CTX(ctx), inc->name)) {
        LOGVAL_PARSER(ctx, LY_VCODE_NAME2_COL, "module", "submodule", inc->name);
        return LY_EVALID;
    }

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            PARSER_CHECK_STMTVER2_RET(ctx, "description", "include");
            LY_CHECK_RET(parse_text_field(ctx, inc->dsc, LY_STMT_DESCRIPTION, 0, &inc->dsc, Y_STR_ARG, NULL, &inc->exts));
            break;
        case LY_STMT_REFERENCE:
            PARSER_CHECK_STMTVER2_RET(ctx, "reference", "include");
            LY_CHECK_RET(parse_text_field(ctx, inc->ref, LY_STMT_REFERENCE, 0, &inc->ref, Y_STR_ARG, NULL, &inc->exts));
            break;
        case LY_STMT_REVISION_DATE:
            LY_CHECK_RET(parse_revisiondate(ctx, inc->rev, &inc->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, inc, LY_STMT_INCLUDE, 0, &inc->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "include");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, inc->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the import statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] module_prefix Prefix of the module to check prefix collisions.
 * @param[in,out] imports Parsed imports to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_import(struct lysp_yang_ctx *ctx, const char *module_prefix, struct lysp_import **imports)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_import *imp;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *imports, imp, LY_EVALID);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, imp->name, word, word_len, ret, cleanup);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_PREFIX:
            LY_CHECK_RET(parse_text_field(ctx, imp->prefix, LY_STMT_PREFIX, 0, &imp->prefix, Y_IDENTIF_ARG, NULL, &imp->exts));
            LY_CHECK_RET(lysp_check_prefix((struct lysp_ctx *)ctx, *imports, module_prefix, &imp->prefix), LY_EVALID);
            break;
        case LY_STMT_DESCRIPTION:
            PARSER_CHECK_STMTVER2_RET(ctx, "description", "import");
            LY_CHECK_RET(parse_text_field(ctx, imp->dsc, LY_STMT_DESCRIPTION, 0, &imp->dsc, Y_STR_ARG, NULL, &imp->exts));
            break;
        case LY_STMT_REFERENCE:
            PARSER_CHECK_STMTVER2_RET(ctx, "reference", "import");
            LY_CHECK_RET(parse_text_field(ctx, imp->ref, LY_STMT_REFERENCE, 0, &imp->ref, Y_STR_ARG, NULL, &imp->exts));
            break;
        case LY_STMT_REVISION_DATE:
            LY_CHECK_RET(parse_revisiondate(ctx, imp->rev, &imp->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, imp, LY_STMT_IMPORT, 0, &imp->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "import");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, imp->exts, ret, cleanup);
    }

    /* mandatory substatements */
    LY_CHECK_ERR_RET(!imp->prefix, LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "prefix", "import"), LY_EVALID);

cleanup:
    return ret;
}

/**
 * @brief Parse the revision statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] revs Parsed revisions to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_revision(struct lysp_yang_ctx *ctx, struct lysp_revision **revs)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_revision *rev;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *revs, rev, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len));

    /* check value */
    if (lysp_check_date((struct lysp_ctx *)ctx, word, word_len, "revision")) {
        free(buf);
        return LY_EVALID;
    }

    strncpy(rev->date, word, word_len);
    free(buf);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, rev->dsc, LY_STMT_DESCRIPTION, 0, &rev->dsc, Y_STR_ARG, NULL, &rev->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, rev->ref, LY_STMT_REFERENCE, 0, &rev->ref, Y_STR_ARG, NULL, &rev->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, rev, LY_STMT_REVISION, 0, &rev->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "revision");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, rev->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse a generic text field that can have more instances such as base.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] parent Current statement parent.
 * @param[in] parent_stmt Type of @p parent statement.
 * @param[in,out] texts Parsed values to add to.
 * @param[in] arg Type of the expected argument.
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_text_fields(struct lysp_yang_ctx *ctx, enum ly_stmt parent_stmt, const char ***texts, enum yang_arg arg,
        struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    const char **item;
    size_t word_len;
    enum ly_stmt kw;

    /* allocate new pointer */
    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *texts, item, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, arg, NULL, &word, &buf, &word_len));

    INSERT_WORD_GOTO(ctx, buf, *item, word, word_len, ret, cleanup);
    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, *texts, parent_stmt, LY_ARRAY_COUNT(*texts) - 1, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), lyplg_ext_stmt2str(parent_stmt));
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse a generic text field that can have more instances such as base.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] parent_stmt Type of statement stored in @p qnames.
 * @param[in,out] qnames Parsed qnames to add to.
 * @param[in] arg Type of the expected argument.
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_qnames(struct lysp_yang_ctx *ctx, enum ly_stmt parent_stmt, struct lysp_qname **qnames, enum yang_arg arg,
        struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    struct lysp_qname *item;
    size_t word_len;
    enum ly_stmt kw;

    /* allocate new pointer */
    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *qnames, item, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, arg, &item->flags, &word, &buf, &word_len));

    INSERT_WORD_GOTO(ctx, buf, item->str, word, word_len, ret, cleanup);
    item->mod = PARSER_CUR_PMOD(ctx);
    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, *qnames, parent_stmt, LY_ARRAY_COUNT(*qnames) - 1, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), lyplg_ext_stmt2str(parent_stmt));
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the config statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] flags Flags to add to.
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_config(struct lysp_yang_ctx *ctx, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (*flags & LYS_CONFIG_MASK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "config");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if ((word_len == ly_strlen_const("true")) && !strncmp(word, "true", word_len)) {
        *flags |= LYS_CONFIG_W;
    } else if ((word_len == ly_strlen_const("false")) && !strncmp(word, "false", word_len)) {
        *flags |= LYS_CONFIG_R;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, "config");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, flags, LY_STMT_CONFIG, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "config");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the mandatory statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] flags Flags to add to.
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_mandatory(struct lysp_yang_ctx *ctx, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (*flags & LYS_MAND_MASK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "mandatory");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if ((word_len == ly_strlen_const("true")) && !strncmp(word, "true", word_len)) {
        *flags |= LYS_MAND_TRUE;
    } else if ((word_len == ly_strlen_const("false")) && !strncmp(word, "false", word_len)) {
        *flags |= LYS_MAND_FALSE;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, "mandatory");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, flags, LY_STMT_MANDATORY, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "mandatory");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse a restriction such as range or length.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] restr_kw Type of this particular restriction.
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_restr(struct lysp_yang_ctx *ctx, enum ly_stmt restr_kw, struct lysp_restr *restr)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_STR_ARG, &restr->arg.flags, &word, &buf, &word_len));

    CHECK_NONEMPTY(ctx, word_len, lyplg_ext_stmt2str(restr_kw));
    INSERT_WORD_GOTO(ctx, buf, restr->arg.str, word, word_len, ret, cleanup);
    restr->arg.mod = PARSER_CUR_PMOD(ctx);
    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, restr->dsc, LY_STMT_DESCRIPTION, 0, &restr->dsc, Y_STR_ARG, NULL, &restr->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, restr->ref, LY_STMT_REFERENCE, 0, &restr->ref, Y_STR_ARG, NULL, &restr->exts));
            break;
        case LY_STMT_ERROR_APP_TAG:
            LY_CHECK_RET(parse_text_field(ctx, restr, LY_STMT_ERROR_APP_TAG, 0, &restr->eapptag, Y_STR_ARG, NULL,
                    &restr->exts));
            break;
        case LY_STMT_ERROR_MESSAGE:
            LY_CHECK_RET(parse_text_field(ctx, restr, LY_STMT_ERROR_MESSAGE, 0, &restr->emsg, Y_STR_ARG, NULL, &restr->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, restr, restr_kw, 0, &restr->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), lyplg_ext_stmt2str(restr_kw));
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, restr->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse a restriction that can have more instances such as must.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] restr_kw Type of this particular restriction.
 * @param[in,out] restrs Restrictions to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_restrs(struct lysp_yang_ctx *ctx, enum ly_stmt restr_kw, struct lysp_restr **restrs)
{
    struct lysp_restr *restr;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *restrs, restr, LY_EMEM);
    return parse_restr(ctx, restr_kw, restr);
}

/**
 * @brief Parse the status statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] flags Flags to add to.
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_status(struct lysp_yang_ctx *ctx, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (*flags & LYS_STATUS_MASK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "status");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if ((word_len == ly_strlen_const("current")) && !strncmp(word, "current", word_len)) {
        *flags |= LYS_STATUS_CURR;
    } else if ((word_len == ly_strlen_const("deprecated")) && !strncmp(word, "deprecated", word_len)) {
        *flags |= LYS_STATUS_DEPRC;
    } else if ((word_len == ly_strlen_const("obsolete")) && !strncmp(word, "obsolete", word_len)) {
        *flags |= LYS_STATUS_OBSLT;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, "status");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, flags, LY_STMT_STATUS, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "status");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the when statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] when_p When pointer to parse to.
 * @return LY_ERR values.
 */
LY_ERR
parse_when(struct lysp_yang_ctx *ctx, struct lysp_when **when_p)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_when *when;
    struct lysf_ctx fctx = {.ctx = PARSER_CTX(ctx)};

    if (*when_p) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "when");
        return LY_EVALID;
    }

    when = calloc(1, sizeof *when);
    LY_CHECK_ERR_GOTO(!when, LOGMEM(PARSER_CTX(ctx)); ret = LY_EMEM, cleanup);

    /* get value */
    LY_CHECK_GOTO(ret = get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len), cleanup);
    CHECK_NONEMPTY(ctx, word_len, "when");
    INSERT_WORD_GOTO(ctx, buf, when->cond, word, word_len, ret, cleanup);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            ret = parse_text_field(ctx, when->dsc, LY_STMT_DESCRIPTION, 0, &when->dsc, Y_STR_ARG, NULL, &when->exts);
            LY_CHECK_GOTO(ret, cleanup);
            break;
        case LY_STMT_REFERENCE:
            ret = parse_text_field(ctx, when->ref, LY_STMT_REFERENCE, 0, &when->ref, Y_STR_ARG, NULL, &when->exts);
            LY_CHECK_GOTO(ret, cleanup);
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_GOTO(ret = parse_ext(ctx, word, word_len, *when_p, LY_STMT_WHEN, 0, &when->exts), cleanup);
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "when");
            ret = LY_EVALID;
            goto cleanup;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, when->exts, ret, cleanup);
    }

cleanup:
    if (ret) {
        lysp_when_free(&fctx, when);
        free(when);
    } else {
        *when_p = when;
    }
    return ret;
}

/**
 * @brief Parse the anydata or anyxml statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] any_kw Type of this particular keyword.
 * @param[in] parent Node parent.
 * @param[in,out] siblings Siblings to add to.
 * @return LY_ERR values.
 */
LY_ERR
parse_any(struct lysp_yang_ctx *ctx, enum ly_stmt any_kw, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    struct lysp_node_anydata *any;
    enum ly_stmt kw;

    /* create new structure and insert into siblings */
    LY_LIST_NEW_RET(PARSER_CTX(ctx), siblings, any, next, LY_EMEM);

    any->nodetype = any_kw == LY_STMT_ANYDATA ? LYS_ANYDATA : LYS_ANYXML;
    any->parent = parent;

    /* get name */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, any->name, word, word_len, ret, cleanup);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(parse_config(ctx, &any->flags, &any->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, any->dsc, LY_STMT_DESCRIPTION, 0, &any->dsc, Y_STR_ARG, NULL, &any->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_IF_FEATURE, &any->iffeatures, Y_STR_ARG, &any->exts));
            break;
        case LY_STMT_MANDATORY:
            LY_CHECK_RET(parse_mandatory(ctx, &any->flags, &any->exts));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(parse_restrs(ctx, kw, &any->musts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, any->ref, LY_STMT_REFERENCE, 0, &any->ref, Y_STR_ARG, NULL, &any->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, &any->flags, &any->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, &any->when));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, any, any_kw, 0, &any->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), lyplg_ext_stmt2str(any_kw));
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, any->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the value or position statement. Substatement of type enum statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] val_kw Type of this particular keyword.
 * @param[in,out] enm Structure to fill.
 * @return LY_ERR values.
 */
LY_ERR
parse_type_enum_value_pos(struct lysp_yang_ctx *ctx, enum ly_stmt val_kw, struct lysp_type_enum *enm)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf = NULL, *word, *ptr;
    size_t word_len;
    long long num = 0;
    unsigned long long unum = 0;
    enum ly_stmt kw;

    if (enm->flags & LYS_SET_VALUE) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, lyplg_ext_stmt2str(val_kw));
        ret = LY_EVALID;
        goto cleanup;
    }
    enm->flags |= LYS_SET_VALUE;

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if (!word_len || (word[0] == '+') || ((word[0] == '0') && (word_len > 1)) || ((val_kw == LY_STMT_POSITION) && !strncmp(word, "-0", 2))) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, lyplg_ext_stmt2str(val_kw));
        ret = LY_EVALID;
        goto cleanup;
    }

    errno = 0;
    if (val_kw == LY_STMT_VALUE) {
        num = strtoll(word, &ptr, LY_BASE_DEC);
        if ((num < INT64_C(-2147483648)) || (num > INT64_C(2147483647))) {
            LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, lyplg_ext_stmt2str(val_kw));
            ret = LY_EVALID;
            goto cleanup;
        }
    } else {
        unum = strtoull(word, &ptr, LY_BASE_DEC);
        if (unum > UINT64_C(4294967295)) {
            LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, lyplg_ext_stmt2str(val_kw));
            ret = LY_EVALID;
            goto cleanup;
        }
    }
    /* we have not parsed the whole argument */
    if ((size_t)(ptr - word) != word_len) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, lyplg_ext_stmt2str(val_kw));
        ret = LY_EVALID;
        goto cleanup;
    }
    if (errno == ERANGE) {
        LOGVAL_PARSER(ctx, LY_VCODE_OOB, (int)word_len, word, lyplg_ext_stmt2str(val_kw));
        ret = LY_EVALID;
        goto cleanup;
    }
    if (val_kw == LY_STMT_VALUE) {
        enm->value = num;
    } else {
        enm->value = unum;
    }

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            ret = parse_ext(ctx, word, word_len, enm, val_kw, 0, &enm->exts);
            LY_CHECK_GOTO(ret, cleanup);
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), lyplg_ext_stmt2str(val_kw));
            ret = LY_EVALID;
            goto cleanup;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    free(buf);
    return ret;
}

/**
 * @brief Parse the enum or bit statement. Substatement of type statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] enum_kw Type of this particular keyword.
 * @param[in,out] enums Enums or bits to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_type_enum(struct lysp_yang_ctx *ctx, enum ly_stmt enum_kw, struct lysp_type_enum **enums)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_type_enum *enm;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *enums, enm, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, enum_kw == LY_STMT_ENUM ? Y_STR_ARG : Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    if (enum_kw == LY_STMT_ENUM) {
        ret = lysp_check_enum_name((struct lysp_ctx *)ctx, (const char *)word, word_len);
        LY_CHECK_ERR_RET(ret, free(buf), ret);
    } /* else nothing specific for YANG_BIT */

    INSERT_WORD_GOTO(ctx, buf, enm->name, word, word_len, ret, cleanup);
    CHECK_UNIQUENESS(ctx, *enums, name, lyplg_ext_stmt2str(enum_kw), enm->name);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, enm->dsc, LY_STMT_DESCRIPTION, 0, &enm->dsc, Y_STR_ARG, NULL, &enm->exts));
            break;
        case LY_STMT_IF_FEATURE:
            PARSER_CHECK_STMTVER2_RET(ctx, "if-feature", lyplg_ext_stmt2str(enum_kw));
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_IF_FEATURE, &enm->iffeatures, Y_STR_ARG, &enm->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, enm->ref, LY_STMT_REFERENCE, 0, &enm->ref, Y_STR_ARG, NULL, &enm->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, &enm->flags, &enm->exts));
            break;
        case LY_STMT_VALUE:
            LY_CHECK_ERR_RET(enum_kw == LY_STMT_BIT, LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw),
                    lyplg_ext_stmt2str(enum_kw)), LY_EVALID);
            LY_CHECK_RET(parse_type_enum_value_pos(ctx, kw, enm));
            break;
        case LY_STMT_POSITION:
            LY_CHECK_ERR_RET(enum_kw == LY_STMT_ENUM, LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw),
                    lyplg_ext_stmt2str(enum_kw)), LY_EVALID);
            LY_CHECK_RET(parse_type_enum_value_pos(ctx, kw, enm));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, enm, enum_kw, 0, &enm->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), lyplg_ext_stmt2str(enum_kw));
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, enm->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the fraction-digits statement. Substatement of type statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] type Type to fill.
 * @return LY_ERR values.
 */
static LY_ERR
parse_type_fracdigits(struct lysp_yang_ctx *ctx, struct lysp_type *type)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf = NULL, *word, *ptr;
    size_t word_len;
    unsigned long long num;
    enum ly_stmt kw;

    if (type->fraction_digits) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "fraction-digits");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_GOTO(ret = get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len), cleanup);

    if (!word_len || (word[0] == '0') || !isdigit(word[0])) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, "fraction-digits");
        ret = LY_EVALID;
        goto cleanup;
    }

    errno = 0;
    num = strtoull(word, &ptr, LY_BASE_DEC);
    /* we have not parsed the whole argument */
    if ((size_t)(ptr - word) != word_len) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, "fraction-digits");
        ret = LY_EVALID;
        goto cleanup;
    }
    if ((errno == ERANGE) || (num > LY_TYPE_DEC64_FD_MAX)) {
        LOGVAL_PARSER(ctx, LY_VCODE_OOB, (int)word_len, word, "fraction-digits");
        ret = LY_EVALID;
        goto cleanup;
    }
    type->fraction_digits = num;

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_GOTO(ret = parse_ext(ctx, word, word_len, type, LY_STMT_FRACTION_DIGITS, 0, &type->exts), cleanup);
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "fraction-digits");
            ret = LY_EVALID;
            goto cleanup;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    free(buf);
    return ret;
}

/**
 * @brief Parse the require-instance statement. Substatement of type statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] type Type to fill.
 * @return LY_ERR values.
 */
static LY_ERR
parse_type_reqinstance(struct lysp_yang_ctx *ctx, struct lysp_type *type)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf = NULL, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (type->flags & LYS_SET_REQINST) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "require-instance");
        return LY_EVALID;
    }
    type->flags |= LYS_SET_REQINST;

    /* get value */
    LY_CHECK_GOTO(ret = get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len), cleanup);

    if ((word_len == ly_strlen_const("true")) && !strncmp(word, "true", word_len)) {
        type->require_instance = 1;
    } else if ((word_len != ly_strlen_const("false")) || strncmp(word, "false", word_len)) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, "require-instance");
        ret = LY_EVALID;
        goto cleanup;
    }

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_GOTO(ret = parse_ext(ctx, word, word_len, type, LY_STMT_REQUIRE_INSTANCE, 0, &type->exts), cleanup);
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "require-instance");
            ret = LY_EVALID;
            goto cleanup;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    free(buf);
    return ret;
}

/**
 * @brief Parse the modifier statement. Substatement of type pattern statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] restr Restriction to fill.
 * @return LY_ERR values.
 */
static LY_ERR
parse_type_pattern_modifier(struct lysp_yang_ctx *ctx, struct lysp_restr *restr)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf = NULL, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (restr->arg.str[0] == LYSP_RESTR_PATTERN_NACK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "modifier");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_GOTO(ret = get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len), cleanup);

    if ((word_len != ly_strlen_const("invert-match")) || strncmp(word, "invert-match", word_len)) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, "modifier");
        ret = LY_EVALID;
        goto cleanup;
    }

    /* replace the value in the dictionary */
    buf = malloc(strlen(restr->arg.str) + 1);
    LY_CHECK_ERR_GOTO(!buf, LOGMEM(PARSER_CTX(ctx)); ret = LY_EMEM, cleanup);
    strcpy(buf, restr->arg.str);
    lydict_remove(PARSER_CTX(ctx), restr->arg.str);

    assert(buf[0] == LYSP_RESTR_PATTERN_ACK);
    buf[0] = LYSP_RESTR_PATTERN_NACK;
    ret = lydict_insert_zc(PARSER_CTX(ctx), buf, &restr->arg.str);
    buf = NULL;
    LY_CHECK_GOTO(ret, cleanup);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_GOTO(ret = parse_ext(ctx, word, word_len, restr, LY_STMT_MODIFIER, 0, &restr->exts), cleanup);
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "modifier");
            ret = LY_EVALID;
            goto cleanup;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    free(buf);
    return ret;
}

/**
 * @brief Parse the pattern statement. Substatement of type statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] patterns Restrictions to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_type_pattern(struct lysp_yang_ctx *ctx, struct lysp_restr **patterns)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_restr *restr;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *patterns, restr, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_STR_ARG, &restr->arg.flags, &word, &buf, &word_len));

    /* add special meaning first byte */
    if (buf) {
        buf = ly_realloc(buf, word_len + 2);
        word = buf;
    } else {
        buf = malloc(word_len + 2);
    }
    LY_CHECK_ERR_RET(!buf, LOGMEM(PARSER_CTX(ctx)), LY_EMEM);
    if (word_len) {
        memmove(buf + 1, word, word_len);
    }
    buf[0] = LYSP_RESTR_PATTERN_ACK; /* pattern's default regular-match flag */
    buf[word_len + 1] = '\0'; /* terminating NULL byte */
    LY_CHECK_RET(lydict_insert_zc(PARSER_CTX(ctx), buf, &restr->arg.str));
    restr->arg.mod = PARSER_CUR_PMOD(ctx);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, restr->dsc, LY_STMT_DESCRIPTION, 0, &restr->dsc, Y_STR_ARG, NULL, &restr->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, restr->ref, LY_STMT_REFERENCE, 0, &restr->ref, Y_STR_ARG, NULL, &restr->exts));
            break;
        case LY_STMT_ERROR_APP_TAG:
            LY_CHECK_RET(parse_text_field(ctx, restr, LY_STMT_ERROR_APP_TAG, 0, &restr->eapptag, Y_STR_ARG, NULL, &restr->exts));
            break;
        case LY_STMT_ERROR_MESSAGE:
            LY_CHECK_RET(parse_text_field(ctx, restr, LY_STMT_ERROR_MESSAGE, 0, &restr->emsg, Y_STR_ARG, NULL, &restr->exts));
            break;
        case LY_STMT_MODIFIER:
            PARSER_CHECK_STMTVER2_RET(ctx, "modifier", "pattern");
            LY_CHECK_RET(parse_type_pattern_modifier(ctx, restr));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, restr, LY_STMT_PATTERN, 0, &restr->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "pattern");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, restr->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the type statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] type Type to wrote to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_type(struct lysp_yang_ctx *ctx, struct lysp_type *type)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    const char *str_path = NULL;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_type *nest_type;

    if (type->name) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "type");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_PREF_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, type->name, word, word_len, ret, cleanup);

    /* set module */
    type->pmod = PARSER_CUR_PMOD(ctx);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_BASE:
            LY_CHECK_RET(parse_text_fields(ctx, LY_STMT_BASE, &type->bases, Y_PREF_IDENTIF_ARG, &type->exts));
            type->flags |= LYS_SET_BASE;
            break;
        case LY_STMT_BIT:
            LY_CHECK_RET(parse_type_enum(ctx, kw, &type->bits));
            type->flags |= LYS_SET_BIT;
            break;
        case LY_STMT_ENUM:
            LY_CHECK_RET(parse_type_enum(ctx, kw, &type->enums));
            type->flags |= LYS_SET_ENUM;
            break;
        case LY_STMT_FRACTION_DIGITS:
            LY_CHECK_RET(parse_type_fracdigits(ctx, type));
            type->flags |= LYS_SET_FRDIGITS;
            break;
        case LY_STMT_LENGTH:
            if (type->length) {
                LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, lyplg_ext_stmt2str(kw));
                return LY_EVALID;
            }
            type->length = calloc(1, sizeof *type->length);
            LY_CHECK_ERR_RET(!type->length, LOGMEM(PARSER_CTX(ctx)), LY_EMEM);

            LY_CHECK_RET(parse_restr(ctx, kw, type->length));
            type->flags |= LYS_SET_LENGTH;
            break;
        case LY_STMT_PATH:
            if (type->path) {
                LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, lyplg_ext_stmt2str(LY_STMT_PATH));
                return LY_EVALID;
            }

            /* Usually, in the parser_yang.c, the result of the parsing is stored directly in the
             * corresponding structure, so in case of failure, the lysp_module_free function will take
             * care of removing the parsed value from the dictionary. But in this case, it is not possible
             * to rely on lysp_module_free because the result of the parsing is stored in a local variable.
             */
            LY_CHECK_ERR_RET(ret = parse_text_field(ctx, type, LY_STMT_PATH, 0, &str_path, Y_STR_ARG, NULL, &type->exts),
                    lydict_remove(PARSER_CTX(ctx), str_path), ret);
            ret = ly_path_parse(PARSER_CTX(ctx), NULL, str_path, 0, 1, LY_PATH_BEGIN_EITHER,
                    LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_LEAFREF, &type->path);
            /* Moreover, even if successful, the string is removed from the dictionary. */
            lydict_remove(PARSER_CTX(ctx), str_path);
            LY_CHECK_RET(ret);
            type->flags |= LYS_SET_PATH;
            break;
        case LY_STMT_PATTERN:
            LY_CHECK_RET(parse_type_pattern(ctx, &type->patterns));
            type->flags |= LYS_SET_PATTERN;
            break;
        case LY_STMT_RANGE:
            if (type->range) {
                LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, lyplg_ext_stmt2str(kw));
                return LY_EVALID;
            }
            type->range = calloc(1, sizeof *type->range);
            LY_CHECK_ERR_RET(!type->range, LOGMEM(PARSER_CTX(ctx)), LY_EMEM);

            LY_CHECK_RET(parse_restr(ctx, kw, type->range));
            type->flags |= LYS_SET_RANGE;
            break;
        case LY_STMT_REQUIRE_INSTANCE:
            LY_CHECK_RET(parse_type_reqinstance(ctx, type));
            /* LYS_SET_REQINST checked and set inside parse_type_reqinstance() */
            break;
        case LY_STMT_TYPE:
            LY_ARRAY_NEW_RET(PARSER_CTX(ctx), type->types, nest_type, LY_EMEM);
            LY_CHECK_RET(parse_type(ctx, nest_type));
            type->flags |= LYS_SET_TYPE;
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, type, LY_STMT_TYPE, 0, &type->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "type");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, type->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the leaf statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] siblings Siblings to add to.
 * @return LY_ERR values.
 */
LY_ERR
parse_leaf(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_leaf *leaf;

    /* create new leaf structure */
    LY_LIST_NEW_RET(PARSER_CTX(ctx), siblings, leaf, next, LY_EMEM);
    leaf->nodetype = LYS_LEAF;
    leaf->parent = parent;

    /* get name */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, leaf->name, word, word_len, ret, cleanup);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(parse_config(ctx, &leaf->flags, &leaf->exts));
            break;
        case LY_STMT_DEFAULT:
            LY_CHECK_RET(parse_text_field(ctx, &leaf->dflt, LY_STMT_DEFAULT, 0, &leaf->dflt.str, Y_STR_ARG,
                    &leaf->dflt.flags, &leaf->exts));
            leaf->dflt.mod = PARSER_CUR_PMOD(ctx);
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, leaf->dsc, LY_STMT_DESCRIPTION, 0, &leaf->dsc, Y_STR_ARG, NULL, &leaf->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_IF_FEATURE, &leaf->iffeatures, Y_STR_ARG, &leaf->exts));
            break;
        case LY_STMT_MANDATORY:
            LY_CHECK_RET(parse_mandatory(ctx, &leaf->flags, &leaf->exts));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(parse_restrs(ctx, kw, &leaf->musts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, leaf->ref, LY_STMT_REFERENCE, 0, &leaf->ref, Y_STR_ARG, NULL, &leaf->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, &leaf->flags, &leaf->exts));
            break;
        case LY_STMT_TYPE:
            LY_CHECK_RET(parse_type(ctx, &leaf->type));
            break;
        case LY_STMT_UNITS:
            LY_CHECK_RET(parse_text_field(ctx, leaf->units, LY_STMT_UNITS, 0, &leaf->units, Y_STR_ARG, NULL, &leaf->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, &leaf->when));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, leaf, LY_STMT_LEAF, 0, &leaf->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "leaf");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, leaf->exts, ret, cleanup);
    }

    /* mandatory substatements */
    if (!leaf->type.name) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "type", "leaf");
        return LY_EVALID;
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the max-elements statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] max Value to write to.
 * @param[in,out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
LY_ERR
parse_maxelements(struct lysp_yang_ctx *ctx, uint32_t *max, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf = NULL, *word, *ptr;
    size_t word_len;
    unsigned long long num;
    enum ly_stmt kw;

    if (*flags & LYS_SET_MAX) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "max-elements");
        return LY_EVALID;
    }
    *flags |= LYS_SET_MAX;

    /* get value */
    LY_CHECK_GOTO(ret = get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len), cleanup);

    if (!word_len || (word[0] == '0') || ((word[0] != 'u') && !isdigit(word[0]))) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, "max-elements");
        ret = LY_EVALID;
        goto cleanup;
    }

    if (ly_strncmp("unbounded", word, word_len)) {
        errno = 0;
        num = strtoull(word, &ptr, LY_BASE_DEC);
        /* we have not parsed the whole argument */
        if ((size_t)(ptr - word) != word_len) {
            LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, "max-elements");
            ret = LY_EVALID;
            goto cleanup;
        }
        if ((errno == ERANGE) || (num > UINT32_MAX)) {
            LOGVAL_PARSER(ctx, LY_VCODE_OOB, (int)word_len, word, "max-elements");
            ret = LY_EVALID;
            goto cleanup;
        }

        *max = num;
    } else {
        /* unbounded */
        *max = 0;
    }

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_GOTO(ret = parse_ext(ctx, word, word_len, max, LY_STMT_MAX_ELEMENTS, 0, exts), cleanup);
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "max-elements");
            ret = LY_EVALID;
            goto cleanup;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    free(buf);
    return ret;
}

/**
 * @brief Parse the min-elements statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] min Value to write to.
 * @param[in,out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 * @return LY_ERR values.
 */
LY_ERR
parse_minelements(struct lysp_yang_ctx *ctx, uint32_t *min, uint16_t *flags, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf = NULL, *word, *ptr;
    size_t word_len;
    unsigned long long num;
    enum ly_stmt kw;

    if (*flags & LYS_SET_MIN) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "min-elements");
        return LY_EVALID;
    }
    *flags |= LYS_SET_MIN;

    /* get value */
    LY_CHECK_GOTO(ret = get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len), cleanup);

    if (!word_len || !isdigit(word[0]) || ((word[0] == '0') && (word_len > 1))) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, "min-elements");
        ret = LY_EVALID;
        goto cleanup;
    }

    errno = 0;
    num = strtoull(word, &ptr, LY_BASE_DEC);
    /* we have not parsed the whole argument */
    if ((size_t)(ptr - word) != word_len) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, "min-elements");
        ret = LY_EVALID;
        goto cleanup;
    }
    if ((errno == ERANGE) || (num > UINT32_MAX)) {
        LOGVAL_PARSER(ctx, LY_VCODE_OOB, (int)word_len, word, "min-elements");
        ret = LY_EVALID;
        goto cleanup;
    }
    *min = num;

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_GOTO(ret = parse_ext(ctx, word, word_len, min, LY_STMT_MIN_ELEMENTS, 0, exts), cleanup);
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "min-elements");
            ret = LY_EVALID;
            goto cleanup;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    free(buf);
    return ret;
}

/**
 * @brief Parse the ordered-by statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] llist List or leaf-list to fill.
 * @return LY_ERR values.
 */
static LY_ERR
parse_orderedby(struct lysp_yang_ctx *ctx, struct lysp_node *llist)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf = NULL, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (llist->flags & LYS_ORDBY_MASK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "ordered-by");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_GOTO(ret = get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len), cleanup);

    if ((word_len == ly_strlen_const("system")) && !strncmp(word, "system", word_len)) {
        llist->flags |= LYS_ORDBY_SYSTEM;
    } else if ((word_len == ly_strlen_const("user")) && !strncmp(word, "user", word_len)) {
        llist->flags |= LYS_ORDBY_USER;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, "ordered-by");
        ret = LY_EVALID;
        goto cleanup;
    }

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_GOTO(ret = parse_ext(ctx, word, word_len, llist, LY_STMT_ORDERED_BY, 0, &llist->exts), cleanup);
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "ordered-by");
            ret = LY_EVALID;
            goto cleanup;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    free(buf);
    return ret;
}

/**
 * @brief Parse the leaf-list statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] siblings Siblings to add to.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_leaflist(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_leaflist *llist;

    /* create new leaf-list structure */
    LY_LIST_NEW_RET(PARSER_CTX(ctx), siblings, llist, next, LY_EMEM);
    llist->nodetype = LYS_LEAFLIST;
    llist->parent = parent;

    /* get name */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, llist->name, word, word_len, ret, cleanup);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(parse_config(ctx, &llist->flags, &llist->exts));
            break;
        case LY_STMT_DEFAULT:
            PARSER_CHECK_STMTVER2_RET(ctx, "default", "leaf-list");
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_DEFAULT, &llist->dflts, Y_STR_ARG, &llist->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, llist->dsc, LY_STMT_DESCRIPTION, 0, &llist->dsc, Y_STR_ARG, NULL, &llist->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_IF_FEATURE, &llist->iffeatures, Y_STR_ARG, &llist->exts));
            break;
        case LY_STMT_MAX_ELEMENTS:
            LY_CHECK_RET(parse_maxelements(ctx, &llist->max, &llist->flags, &llist->exts));
            break;
        case LY_STMT_MIN_ELEMENTS:
            LY_CHECK_RET(parse_minelements(ctx, &llist->min, &llist->flags, &llist->exts));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(parse_restrs(ctx, kw, &llist->musts));
            break;
        case LY_STMT_ORDERED_BY:
            LY_CHECK_RET(parse_orderedby(ctx, &llist->node));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, llist->ref, LY_STMT_REFERENCE, 0, &llist->ref, Y_STR_ARG, NULL, &llist->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, &llist->flags, &llist->exts));
            break;
        case LY_STMT_TYPE:
            LY_CHECK_RET(parse_type(ctx, &llist->type));
            break;
        case LY_STMT_UNITS:
            LY_CHECK_RET(parse_text_field(ctx, llist->units, LY_STMT_UNITS, 0, &llist->units, Y_STR_ARG, NULL, &llist->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, &llist->when));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, llist, LY_STMT_LEAF_LIST, 0, &llist->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "llist");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, llist->exts, ret, cleanup);
    }

    /* mandatory substatements */
    if (!llist->type.name) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "type", "leaf-list");
        return LY_EVALID;
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the refine statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] refines Refines to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_refine(struct lysp_yang_ctx *ctx, struct lysp_refine **refines)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_refine *rf;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *refines, rf, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len));
    CHECK_NONEMPTY(ctx, word_len, "refine");
    INSERT_WORD_GOTO(ctx, buf, rf->nodeid, word, word_len, ret, cleanup);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(parse_config(ctx, &rf->flags, &rf->exts));
            break;
        case LY_STMT_DEFAULT:
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_DEFAULT, &rf->dflts, Y_STR_ARG, &rf->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, rf->dsc, LY_STMT_DESCRIPTION, 0, &rf->dsc, Y_STR_ARG, NULL, &rf->exts));
            break;
        case LY_STMT_IF_FEATURE:
            PARSER_CHECK_STMTVER2_RET(ctx, "if-feature", "refine");
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_IF_FEATURE, &rf->iffeatures, Y_STR_ARG, &rf->exts));
            break;
        case LY_STMT_MAX_ELEMENTS:
            LY_CHECK_RET(parse_maxelements(ctx, &rf->max, &rf->flags, &rf->exts));
            break;
        case LY_STMT_MIN_ELEMENTS:
            LY_CHECK_RET(parse_minelements(ctx, &rf->min, &rf->flags, &rf->exts));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(parse_restrs(ctx, kw, &rf->musts));
            break;
        case LY_STMT_MANDATORY:
            LY_CHECK_RET(parse_mandatory(ctx, &rf->flags, &rf->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, rf->ref, LY_STMT_REFERENCE, 0, &rf->ref, Y_STR_ARG, NULL, &rf->exts));
            break;
        case LY_STMT_PRESENCE:
            LY_CHECK_RET(parse_text_field(ctx, rf->presence, LY_STMT_PRESENCE, 0, &rf->presence, Y_STR_ARG, NULL, &rf->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, rf, LY_STMT_REFINE, 0, &rf->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "refine");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, rf->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the typedef statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] typedefs Typedefs to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_typedef(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_tpdf **typedefs)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_tpdf *tpdf;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *typedefs, tpdf, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, tpdf->name, word, word_len, ret, cleanup);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_DEFAULT:
            LY_CHECK_RET(parse_text_field(ctx, &tpdf->dflt, LY_STMT_DEFAULT, 0, &tpdf->dflt.str, Y_STR_ARG,
                    &tpdf->dflt.flags, &tpdf->exts));
            tpdf->dflt.mod = PARSER_CUR_PMOD(ctx);
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, tpdf->dsc, LY_STMT_DESCRIPTION, 0, &tpdf->dsc, Y_STR_ARG, NULL, &tpdf->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, tpdf->ref, LY_STMT_REFERENCE, 0, &tpdf->ref, Y_STR_ARG, NULL, &tpdf->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, &tpdf->flags, &tpdf->exts));
            break;
        case LY_STMT_TYPE:
            LY_CHECK_RET(parse_type(ctx, &tpdf->type));
            break;
        case LY_STMT_UNITS:
            LY_CHECK_RET(parse_text_field(ctx, tpdf->units, LY_STMT_UNITS, 0, &tpdf->units, Y_STR_ARG, NULL, &tpdf->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, tpdf, LY_STMT_TYPEDEF, 0, &tpdf->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "typedef");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, tpdf->exts, ret, cleanup);
    }

    /* mandatory substatements */
    if (!tpdf->type.name) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "type", "typedef");
        return LY_EVALID;
    }

    /* store data for collision check */
    if (parent) {
        assert(ctx->main_ctx);
        LY_CHECK_RET(ly_set_add(&ctx->main_ctx->tpdfs_nodes, parent, 0, NULL));
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the input or output statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] kw Type of this particular keyword
 * @param[in,out] inout_p Input/output pointer to write to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_inout(struct lysp_yang_ctx *ctx, enum ly_stmt inout_kw, struct lysp_node *parent,
        struct lysp_node_action_inout *inout_p)
{
    LY_ERR ret = LY_SUCCESS;
    char *word;
    size_t word_len;
    enum ly_stmt kw;
    ly_bool input = &((struct lysp_node_action *)parent)->input == inout_p ? 1 : 0;

    if (inout_p->nodetype) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, lyplg_ext_stmt2str(inout_kw));
        return LY_EVALID;
    }

    /* initiate structure */
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), input ? "input" : "output", 0, &inout_p->name));
    inout_p->nodetype = input ? LYS_INPUT : LYS_OUTPUT;
    inout_p->parent = parent;

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", lyplg_ext_stmt2str(inout_kw));
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, kw, (struct lysp_node *)inout_p, &inout_p->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, (struct lysp_node *)inout_p, &inout_p->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, (struct lysp_node *)inout_p, &inout_p->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, (struct lysp_node *)inout_p, &inout_p->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, (struct lysp_node *)inout_p, &inout_p->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, (struct lysp_node *)inout_p, &inout_p->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, (struct lysp_node *)inout_p, &inout_p->child));
            break;
        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, (struct lysp_node *)inout_p, &inout_p->typedefs));
            break;
        case LY_STMT_MUST:
            PARSER_CHECK_STMTVER2_RET(ctx, "must", lyplg_ext_stmt2str(inout_kw));
            LY_CHECK_RET(parse_restrs(ctx, kw, &inout_p->musts));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, (struct lysp_node *)inout_p, &inout_p->groupings));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, inout_p, inout_kw, 0, &inout_p->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), lyplg_ext_stmt2str(inout_kw));
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, inout_p->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the action statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] actions Actions to add to.
 * @return LY_ERR values.
 */
LY_ERR
parse_action(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node_action **actions)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_action *act;

    LY_LIST_NEW_RET(PARSER_CTX(ctx), actions, act, next, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, act->name, word, word_len, ret, cleanup);
    act->nodetype = parent ? LYS_ACTION : LYS_RPC;
    act->parent = parent;

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, act->dsc, LY_STMT_DESCRIPTION, 0, &act->dsc, Y_STR_ARG, NULL, &act->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_IF_FEATURE, &act->iffeatures, Y_STR_ARG, &act->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, act->ref, LY_STMT_REFERENCE, 0, &act->ref, Y_STR_ARG, NULL, &act->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, &act->flags, &act->exts));
            break;

        case LY_STMT_INPUT:
            LY_CHECK_RET(parse_inout(ctx, kw, (struct lysp_node *)act, &act->input));
            break;
        case LY_STMT_OUTPUT:
            LY_CHECK_RET(parse_inout(ctx, kw, (struct lysp_node *)act, &act->output));
            break;

        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, (struct lysp_node *)act, &act->typedefs));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, (struct lysp_node *)act, &act->groupings));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, act, parent ? LY_STMT_ACTION : LY_STMT_RPC, 0, &act->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), parent ? "action" : "rpc");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, act->exts, ret, cleanup);
    }

    /* always initialize inout, they are technically present (needed for later deviations/refines) */
    if (!act->input.nodetype) {
        act->input.nodetype = LYS_INPUT;
        act->input.parent = (struct lysp_node *)act;
        LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), "input", 0, &act->input.name));
    }
    if (!act->output.nodetype) {
        act->output.nodetype = LYS_OUTPUT;
        act->output.parent = (struct lysp_node *)act;
        LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), "output", 0, &act->output.name));
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the notification statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] notifs Notifications to add to.
 * @return LY_ERR values.
 */
LY_ERR
parse_notif(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node_notif **notifs)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_notif *notif;

    LY_LIST_NEW_RET(PARSER_CTX(ctx), notifs, notif, next, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, notif->name, word, word_len, ret, cleanup);
    notif->nodetype = LYS_NOTIF;
    notif->parent = parent;

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, notif->dsc, LY_STMT_DESCRIPTION, 0, &notif->dsc, Y_STR_ARG, NULL,
                    &notif->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_IF_FEATURE, &notif->iffeatures, Y_STR_ARG, &notif->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, notif->ref, LY_STMT_REFERENCE, 0, &notif->ref, Y_STR_ARG, NULL,
                    &notif->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, &notif->flags, &notif->exts));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "notification");
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, kw, (struct lysp_node *)notif, &notif->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, (struct lysp_node *)notif, &notif->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, (struct lysp_node *)notif, &notif->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, (struct lysp_node *)notif, &notif->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, (struct lysp_node *)notif, &notif->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, (struct lysp_node *)notif, &notif->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, (struct lysp_node *)notif, &notif->child));
            break;

        case LY_STMT_MUST:
            PARSER_CHECK_STMTVER2_RET(ctx, "must", "notification");
            LY_CHECK_RET(parse_restrs(ctx, kw, &notif->musts));
            break;
        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, (struct lysp_node *)notif, &notif->typedefs));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, (struct lysp_node *)notif, &notif->groupings));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, notif, LY_STMT_NOTIFICATION, 0, &notif->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "notification");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, notif->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the grouping statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] groupings Groupings to add to.
 * @return LY_ERR values.
 */
LY_ERR
parse_grouping(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node_grp **groupings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_grp *grp;

    LY_LIST_NEW_RET(PARSER_CTX(ctx), groupings, grp, next, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, grp->name, word, word_len, ret, cleanup);
    grp->nodetype = LYS_GROUPING;
    grp->parent = parent;

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, grp->dsc, LY_STMT_DESCRIPTION, 0, &grp->dsc, Y_STR_ARG, NULL, &grp->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, grp->ref, LY_STMT_REFERENCE, 0, &grp->ref, Y_STR_ARG, NULL, &grp->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, &grp->flags, &grp->exts));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "grouping");
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, kw, &grp->node, &grp->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, &grp->node, &grp->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, &grp->node, &grp->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, &grp->node, &grp->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, &grp->node, &grp->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, &grp->node, &grp->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, &grp->node, &grp->child));
            break;

        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, &grp->node, &grp->typedefs));
            break;
        case LY_STMT_ACTION:
            PARSER_CHECK_STMTVER2_RET(ctx, "action", "grouping");
            LY_CHECK_RET(parse_action(ctx, &grp->node, &grp->actions));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, &grp->node, &grp->groupings));
            break;
        case LY_STMT_NOTIFICATION:
            PARSER_CHECK_STMTVER2_RET(ctx, "notification", "grouping");
            LY_CHECK_RET(parse_notif(ctx, &grp->node, &grp->notifs));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, grp, LY_STMT_GROUPING, 0, &grp->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "grouping");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, grp->exts, ret, cleanup);
    }

    /* store data for collision check */
    if (parent) {
        assert(ctx->main_ctx);
        LY_CHECK_RET(ly_set_add(&ctx->main_ctx->grps_nodes, parent, 0, NULL));
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the augment statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] augments Augments to add to.
 * @return LY_ERR values.
 */
LY_ERR
parse_augment(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node_augment **augments)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_augment *aug;

    LY_LIST_NEW_RET(PARSER_CTX(ctx), augments, aug, next, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len));
    CHECK_NONEMPTY(ctx, word_len, "augment");
    INSERT_WORD_GOTO(ctx, buf, aug->nodeid, word, word_len, ret, cleanup);
    aug->nodetype = LYS_AUGMENT;
    aug->parent = parent;

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, aug->dsc, LY_STMT_DESCRIPTION, 0, &aug->dsc, Y_STR_ARG, NULL, &aug->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_IF_FEATURE, &aug->iffeatures, Y_STR_ARG, &aug->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, aug->ref, LY_STMT_REFERENCE, 0, &aug->ref, Y_STR_ARG, NULL, &aug->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, &aug->flags, &aug->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, &aug->when));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "augment");
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, kw, (struct lysp_node *)aug, &aug->child));
            break;
        case LY_STMT_CASE:
            LY_CHECK_RET(parse_case(ctx, (struct lysp_node *)aug, &aug->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, (struct lysp_node *)aug, &aug->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, (struct lysp_node *)aug, &aug->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, (struct lysp_node *)aug, &aug->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, (struct lysp_node *)aug, &aug->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, (struct lysp_node *)aug, &aug->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, (struct lysp_node *)aug, &aug->child));
            break;

        case LY_STMT_ACTION:
            PARSER_CHECK_STMTVER2_RET(ctx, "action", "augment");
            LY_CHECK_RET(parse_action(ctx, (struct lysp_node *)aug, &aug->actions));
            break;
        case LY_STMT_NOTIFICATION:
            PARSER_CHECK_STMTVER2_RET(ctx, "notification", "augment");
            LY_CHECK_RET(parse_notif(ctx, (struct lysp_node *)aug, &aug->notifs));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, aug, LY_STMT_AUGMENT, 0, &aug->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "augment");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, aug->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the uses statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] siblings Siblings to add to.
 * @return LY_ERR values.
 */
LY_ERR
parse_uses(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_uses *uses;

    /* create uses structure */
    LY_LIST_NEW_RET(PARSER_CTX(ctx), siblings, uses, next, LY_EMEM);
    uses->nodetype = LYS_USES;
    uses->parent = parent;

    /* get name */
    LY_CHECK_RET(get_argument(ctx, Y_PREF_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, uses->name, word, word_len, ret, cleanup);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, uses->dsc, LY_STMT_DESCRIPTION, 0, &uses->dsc, Y_STR_ARG, NULL, &uses->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_IF_FEATURE, &uses->iffeatures, Y_STR_ARG, &uses->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, uses->ref, LY_STMT_REFERENCE, 0, &uses->ref, Y_STR_ARG, NULL, &uses->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, &uses->flags, &uses->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, &uses->when));
            break;

        case LY_STMT_REFINE:
            LY_CHECK_RET(parse_refine(ctx, &uses->refines));
            break;
        case LY_STMT_AUGMENT:
            LY_CHECK_RET(parse_augment(ctx, (struct lysp_node *)uses, &uses->augments));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, uses, LY_STMT_USES, 0, &uses->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "uses");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, uses->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the case statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] siblings Siblings to add to.
 * @return LY_ERR values.
 */
LY_ERR
parse_case(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_case *cas;

    /* create new case structure */
    LY_LIST_NEW_RET(PARSER_CTX(ctx), siblings, cas, next, LY_EMEM);
    cas->nodetype = LYS_CASE;
    cas->parent = parent;

    /* get name */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, cas->name, word, word_len, ret, cleanup);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, cas->dsc, LY_STMT_DESCRIPTION, 0, &cas->dsc, Y_STR_ARG, NULL, &cas->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_IF_FEATURE, &cas->iffeatures, Y_STR_ARG, &cas->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, cas->ref, LY_STMT_REFERENCE, 0, &cas->ref, Y_STR_ARG, NULL, &cas->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, &cas->flags, &cas->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, &cas->when));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "case");
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, kw, (struct lysp_node *)cas, &cas->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, (struct lysp_node *)cas, &cas->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, (struct lysp_node *)cas, &cas->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, (struct lysp_node *)cas, &cas->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, (struct lysp_node *)cas, &cas->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, (struct lysp_node *)cas, &cas->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, (struct lysp_node *)cas, &cas->child));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, cas, LY_STMT_CASE, 0, &cas->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "case");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, cas->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the choice statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] siblings Siblings to add to.
 * @return LY_ERR values.
 */
LY_ERR
parse_choice(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_choice *choice;

    /* create new choice structure */
    LY_LIST_NEW_RET(PARSER_CTX(ctx), siblings, choice, next, LY_EMEM);
    choice->nodetype = LYS_CHOICE;
    choice->parent = parent;

    /* get name */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, choice->name, word, word_len, ret, cleanup);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(parse_config(ctx, &choice->flags, &choice->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, choice->dsc, LY_STMT_DESCRIPTION, 0, &choice->dsc, Y_STR_ARG, NULL,
                    &choice->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_IF_FEATURE, &choice->iffeatures, Y_STR_ARG, &choice->exts));
            break;
        case LY_STMT_MANDATORY:
            LY_CHECK_RET(parse_mandatory(ctx, &choice->flags, &choice->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, choice->ref, LY_STMT_REFERENCE, 0, &choice->ref, Y_STR_ARG, NULL,
                    &choice->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, &choice->flags, &choice->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, &choice->when));
            break;
        case LY_STMT_DEFAULT:
            LY_CHECK_RET(parse_text_field(ctx, &choice->dflt, LY_STMT_DEFAULT, 0, &choice->dflt.str, Y_PREF_IDENTIF_ARG,
                    &choice->dflt.flags, &choice->exts));
            choice->dflt.mod = PARSER_CUR_PMOD(ctx);
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "choice");
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, kw, (struct lysp_node *)choice, &choice->child));
            break;
        case LY_STMT_CASE:
            LY_CHECK_RET(parse_case(ctx, (struct lysp_node *)choice, &choice->child));
            break;
        case LY_STMT_CHOICE:
            PARSER_CHECK_STMTVER2_RET(ctx, "choice", "choice");
            LY_CHECK_RET(parse_choice(ctx, (struct lysp_node *)choice, &choice->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, (struct lysp_node *)choice, &choice->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, (struct lysp_node *)choice, &choice->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, (struct lysp_node *)choice, &choice->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, (struct lysp_node *)choice, &choice->child));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, choice, LY_STMT_CHOICE, 0, &choice->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "choice");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, choice->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the container statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] siblings Siblings to add to.
 * @return LY_ERR values.
 */
LY_ERR
parse_container(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = 0;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_container *cont;

    /* create new container structure */
    LY_LIST_NEW_RET(PARSER_CTX(ctx), siblings, cont, next, LY_EMEM);
    cont->nodetype = LYS_CONTAINER;
    cont->parent = parent;

    /* get name */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, cont->name, word, word_len, ret, cleanup);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(parse_config(ctx, &cont->flags, &cont->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, cont->dsc, LY_STMT_DESCRIPTION, 0, &cont->dsc, Y_STR_ARG, NULL, &cont->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_IF_FEATURE, &cont->iffeatures, Y_STR_ARG, &cont->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, cont->ref, LY_STMT_REFERENCE, 0, &cont->ref, Y_STR_ARG, NULL, &cont->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, &cont->flags, &cont->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, &cont->when));
            break;
        case LY_STMT_PRESENCE:
            LY_CHECK_RET(parse_text_field(ctx, cont->presence, LY_STMT_PRESENCE, 0, &cont->presence, Y_STR_ARG, NULL,
                    &cont->exts));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "container");
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, kw, (struct lysp_node *)cont, &cont->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, (struct lysp_node *)cont, &cont->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, (struct lysp_node *)cont, &cont->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, (struct lysp_node *)cont, &cont->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, (struct lysp_node *)cont, &cont->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, (struct lysp_node *)cont, &cont->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, (struct lysp_node *)cont, &cont->child));
            break;

        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, (struct lysp_node *)cont, &cont->typedefs));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(parse_restrs(ctx, kw, &cont->musts));
            break;
        case LY_STMT_ACTION:
            PARSER_CHECK_STMTVER2_RET(ctx, "action", "container");
            LY_CHECK_RET(parse_action(ctx, (struct lysp_node *)cont, &cont->actions));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, (struct lysp_node *)cont, &cont->groupings));
            break;
        case LY_STMT_NOTIFICATION:
            PARSER_CHECK_STMTVER2_RET(ctx, "notification", "container");
            LY_CHECK_RET(parse_notif(ctx, (struct lysp_node *)cont, &cont->notifs));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, cont, LY_STMT_CONTAINER, 0, &cont->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "container");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, cont->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the list statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] siblings Siblings to add to.
 * @return LY_ERR values.
 */
LY_ERR
parse_list(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_node_list *list;

    /* create new list structure */
    LY_LIST_NEW_RET(PARSER_CTX(ctx), siblings, list, next, LY_EMEM);
    list->nodetype = LYS_LIST;
    list->parent = parent;

    /* get name */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, list->name, word, word_len, ret, cleanup);

    /* parse substatements */
    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_CONFIG:
            LY_CHECK_RET(parse_config(ctx, &list->flags, &list->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, list->dsc, LY_STMT_DESCRIPTION, 0, &list->dsc, Y_STR_ARG, NULL, &list->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_IF_FEATURE, &list->iffeatures, Y_STR_ARG, &list->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, list->ref, LY_STMT_REFERENCE, 0, &list->ref, Y_STR_ARG, NULL, &list->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, &list->flags, &list->exts));
            break;
        case LY_STMT_WHEN:
            LY_CHECK_RET(parse_when(ctx, &list->when));
            break;
        case LY_STMT_KEY:
            LY_CHECK_RET(parse_text_field(ctx, list, LY_STMT_KEY, 0, &list->key, Y_STR_ARG, NULL, &list->exts));
            break;
        case LY_STMT_MAX_ELEMENTS:
            LY_CHECK_RET(parse_maxelements(ctx, &list->max, &list->flags, &list->exts));
            break;
        case LY_STMT_MIN_ELEMENTS:
            LY_CHECK_RET(parse_minelements(ctx, &list->min, &list->flags, &list->exts));
            break;
        case LY_STMT_ORDERED_BY:
            LY_CHECK_RET(parse_orderedby(ctx, &list->node));
            break;
        case LY_STMT_UNIQUE:
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_UNIQUE, &list->uniques, Y_STR_ARG, &list->exts));
            break;

        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "list");
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, kw, (struct lysp_node *)list, &list->child));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, (struct lysp_node *)list, &list->child));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, (struct lysp_node *)list, &list->child));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, (struct lysp_node *)list, &list->child));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, (struct lysp_node *)list, &list->child));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, (struct lysp_node *)list, &list->child));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, (struct lysp_node *)list, &list->child));
            break;

        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, (struct lysp_node *)list, &list->typedefs));
            break;
        case LY_STMT_MUST:
            LY_CHECK_RET(parse_restrs(ctx, kw, &list->musts));
            break;
        case LY_STMT_ACTION:
            PARSER_CHECK_STMTVER2_RET(ctx, "action", "list");
            LY_CHECK_RET(parse_action(ctx, (struct lysp_node *)list, &list->actions));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, (struct lysp_node *)list, &list->groupings));
            break;
        case LY_STMT_NOTIFICATION:
            PARSER_CHECK_STMTVER2_RET(ctx, "notification", "list");
            LY_CHECK_RET(parse_notif(ctx, (struct lysp_node *)list, &list->notifs));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, list, LY_STMT_LIST, 0, &list->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "list");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, list->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the yin-element statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] ext Extension to fill.
 * @return LY_ERR values.
 */
static LY_ERR
parse_yinelement(struct lysp_yang_ctx *ctx, struct lysp_ext *ext)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (ext->flags & LYS_YINELEM_MASK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "yin-element");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len));

    if ((word_len == ly_strlen_const("true")) && !strncmp(word, "true", word_len)) {
        ext->flags |= LYS_YINELEM_TRUE;
    } else if ((word_len == ly_strlen_const("false")) && !strncmp(word, "false", word_len)) {
        ext->flags |= LYS_YINELEM_FALSE;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, "yin-element");
        free(buf);
        return LY_EVALID;
    }
    free(buf);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, ext, LY_STMT_YIN_ELEMENT, 0, &ext->exts));
            LY_CHECK_RET(ret);
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "yin-element");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the argument statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] ext Extension to fill.
 * @return LY_ERR values.
 */
static LY_ERR
parse_argument(struct lysp_yang_ctx *ctx, struct lysp_ext *ext)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;

    if (ext->argname) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "argument");
        return LY_EVALID;
    }

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, ext->argname, word, word_len, ret, cleanup);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_YIN_ELEMENT:
            LY_CHECK_RET(parse_yinelement(ctx, ext));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, ext, LY_STMT_ARGUMENT, 0, &ext->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "argument");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, NULL, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the extension statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] extensions Extensions to add to.
 * @return LY_ERR values.
 */
static LY_ERR
parse_extension(struct lysp_yang_ctx *ctx, struct lysp_ext **extensions)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_ext *ex;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *extensions, ex, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, ex->name, word, word_len, ret, cleanup);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, ex->dsc, LY_STMT_DESCRIPTION, 0, &ex->dsc, Y_STR_ARG, NULL, &ex->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, ex->ref, LY_STMT_REFERENCE, 0, &ex->ref, Y_STR_ARG, NULL, &ex->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, &ex->flags, &ex->exts));
            break;
        case LY_STMT_ARGUMENT:
            LY_CHECK_RET(parse_argument(ctx, ex));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, ex, LY_STMT_EXTENSION, 0, &ex->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "extension");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, ex->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the deviate statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] deviates Deviates to add to.
 * @return LY_ERR values.
 */
LY_ERR
parse_deviate(struct lysp_yang_ctx *ctx, struct lysp_deviate **deviates)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf = NULL, *word;
    size_t word_len, dev_mod;
    enum ly_stmt kw;
    struct lysf_ctx fctx = {.ctx = PARSER_CTX(ctx)};
    struct lysp_deviate *d = NULL;
    struct lysp_deviate_add *d_add = NULL;
    struct lysp_deviate_rpl *d_rpl = NULL;
    struct lysp_deviate_del *d_del = NULL;
    const char **d_units = NULL;
    struct lysp_qname **d_uniques = NULL, **d_dflts = NULL;
    struct lysp_restr **d_musts = NULL;
    uint16_t *d_flags = 0;
    uint32_t *d_min = 0, *d_max = 0;

    /* get value */
    LY_CHECK_GOTO(ret = get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len), cleanup);

    if ((word_len == ly_strlen_const("not-supported")) && !strncmp(word, "not-supported", word_len)) {
        dev_mod = LYS_DEV_NOT_SUPPORTED;
    } else if ((word_len == ly_strlen_const("add")) && !strncmp(word, "add", word_len)) {
        dev_mod = LYS_DEV_ADD;
    } else if ((word_len == ly_strlen_const("replace")) && !strncmp(word, "replace", word_len)) {
        dev_mod = LYS_DEV_REPLACE;
    } else if ((word_len == ly_strlen_const("delete")) && !strncmp(word, "delete", word_len)) {
        dev_mod = LYS_DEV_DELETE;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, (int)word_len, word, "deviate");
        ret = LY_EVALID;
        goto cleanup;
    }

    /* create structure */
    switch (dev_mod) {
    case LYS_DEV_NOT_SUPPORTED:
        d = calloc(1, sizeof *d);
        LY_CHECK_ERR_GOTO(!d, LOGMEM(PARSER_CTX(ctx)); ret = LY_EMEM, cleanup);
        break;
    case LYS_DEV_ADD:
        d_add = calloc(1, sizeof *d_add);
        LY_CHECK_ERR_GOTO(!d_add, LOGMEM(PARSER_CTX(ctx)); ret = LY_EMEM, cleanup);
        d = (struct lysp_deviate *)d_add;
        d_units = &d_add->units;
        d_uniques = &d_add->uniques;
        d_dflts = &d_add->dflts;
        d_musts = &d_add->musts;
        d_flags = &d_add->flags;
        d_min = &d_add->min;
        d_max = &d_add->max;
        break;
    case LYS_DEV_REPLACE:
        d_rpl = calloc(1, sizeof *d_rpl);
        LY_CHECK_ERR_GOTO(!d_rpl, LOGMEM(PARSER_CTX(ctx)); ret = LY_EMEM, cleanup);
        d = (struct lysp_deviate *)d_rpl;
        d_units = &d_rpl->units;
        d_flags = &d_rpl->flags;
        d_min = &d_rpl->min;
        d_max = &d_rpl->max;
        break;
    case LYS_DEV_DELETE:
        d_del = calloc(1, sizeof *d_del);
        LY_CHECK_ERR_GOTO(!d_del, LOGMEM(PARSER_CTX(ctx)); ret = LY_EMEM, cleanup);
        d = (struct lysp_deviate *)d_del;
        d_units = &d_del->units;
        d_uniques = &d_del->uniques;
        d_dflts = &d_del->dflts;
        d_musts = &d_del->musts;
        break;
    default:
        assert(0);
        LOGINT(PARSER_CTX(ctx));
        ret = LY_EINT;
        goto cleanup;
    }
    d->mod = dev_mod;

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_CONFIG:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_DELETE:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), lyplg_ext_stmt2str(kw));
                ret = LY_EVALID;
                goto cleanup;
            default:
                LY_CHECK_GOTO(ret = parse_config(ctx, d_flags, &d->exts), cleanup);
                break;
            }
            break;
        case LY_STMT_DEFAULT:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), lyplg_ext_stmt2str(kw));
                ret = LY_EVALID;
                goto cleanup;
            case LYS_DEV_REPLACE:
                ret = parse_text_field(ctx, &d_rpl->dflt, LY_STMT_DEFAULT, 0, &d_rpl->dflt.str, Y_STR_ARG,
                        &d_rpl->dflt.flags, &d->exts);
                LY_CHECK_GOTO(ret, cleanup);
                d_rpl->dflt.mod = PARSER_CUR_PMOD(ctx);
                break;
            default:
                LY_CHECK_GOTO(ret = parse_qnames(ctx, LY_STMT_DEFAULT, d_dflts, Y_STR_ARG, &d->exts), cleanup);
                break;
            }
            break;
        case LY_STMT_MANDATORY:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_DELETE:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), lyplg_ext_stmt2str(kw));
                ret = LY_EVALID;
                goto cleanup;
            default:
                LY_CHECK_GOTO(ret = parse_mandatory(ctx, d_flags, &d->exts), cleanup);
                break;
            }
            break;
        case LY_STMT_MAX_ELEMENTS:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_DELETE:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), lyplg_ext_stmt2str(kw));
                ret = LY_EVALID;
                goto cleanup;
            default:
                LY_CHECK_GOTO(ret = parse_maxelements(ctx, d_max, d_flags, &d->exts), cleanup);
                break;
            }
            break;
        case LY_STMT_MIN_ELEMENTS:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_DELETE:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), lyplg_ext_stmt2str(kw));
                ret = LY_EVALID;
                goto cleanup;
            default:
                LY_CHECK_GOTO(ret = parse_minelements(ctx, d_min, d_flags, &d->exts), cleanup);
                break;
            }
            break;
        case LY_STMT_MUST:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_REPLACE:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), lyplg_ext_stmt2str(kw));
                ret = LY_EVALID;
                goto cleanup;
            default:
                LY_CHECK_GOTO(ret = parse_restrs(ctx, kw, d_musts), cleanup);
                break;
            }
            break;
        case LY_STMT_TYPE:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_ADD:
            case LYS_DEV_DELETE:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), lyplg_ext_stmt2str(kw));
                ret = LY_EVALID;
                goto cleanup;
            default:
                if (d_rpl->type) {
                    LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, lyplg_ext_stmt2str(kw));
                    ret = LY_EVALID;
                    goto cleanup;
                }
                d_rpl->type = calloc(1, sizeof *d_rpl->type);
                LY_CHECK_ERR_GOTO(!d_rpl->type, LOGMEM(PARSER_CTX(ctx)); ret = LY_EMEM, cleanup);
                LY_CHECK_GOTO(ret = parse_type(ctx, d_rpl->type), cleanup);
                break;
            }
            break;
        case LY_STMT_UNIQUE:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
            case LYS_DEV_REPLACE:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), lyplg_ext_stmt2str(kw));
                ret = LY_EVALID;
                goto cleanup;
            default:
                LY_CHECK_GOTO(ret = parse_qnames(ctx, LY_STMT_UNIQUE, d_uniques, Y_STR_ARG, &d->exts), cleanup);
                break;
            }
            break;
        case LY_STMT_UNITS:
            switch (dev_mod) {
            case LYS_DEV_NOT_SUPPORTED:
                LOGVAL_PARSER(ctx, LY_VCODE_INDEV, ly_devmod2str(dev_mod), lyplg_ext_stmt2str(kw));
                ret = LY_EVALID;
                goto cleanup;
            default:
                ret = parse_text_field(ctx, *d_units, LY_STMT_UNITS, 0, d_units, Y_STR_ARG, NULL, &d->exts);
                LY_CHECK_GOTO(ret, cleanup);
                break;
            }
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_GOTO(ret = parse_ext(ctx, word, word_len, d, LY_STMT_DEVIATE, 0, &d->exts), cleanup);
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "deviate");
            ret = LY_EVALID;
            goto cleanup;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, d->exts, ret, cleanup);
    }

cleanup:
    free(buf);
    if (ret) {
        lysp_deviate_free(&fctx, d);
        free(d);
    } else {
        /* insert into siblings */
        LY_LIST_INSERT(deviates, d, next);
    }
    return ret;
}

/**
 * @brief Parse the deviation statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] deviations Deviations to add to.
 * @return LY_ERR values.
 */
LY_ERR
parse_deviation(struct lysp_yang_ctx *ctx, struct lysp_deviation **deviations)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_deviation *dev;
    struct lysf_ctx fctx = {.ctx = PARSER_CTX(ctx)};

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *deviations, dev, LY_EMEM);

    /* get value */
    LY_CHECK_GOTO(ret = get_argument(ctx, Y_STR_ARG, NULL, &word, &buf, &word_len), cleanup);
    CHECK_NONEMPTY(ctx, word_len, "deviation");
    INSERT_WORD_GOTO(ctx, buf, dev->nodeid, word, word_len, ret, cleanup);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            ret = parse_text_field(ctx, dev->dsc, LY_STMT_DESCRIPTION, 0, &dev->dsc, Y_STR_ARG, NULL, &dev->exts);
            LY_CHECK_GOTO(ret, cleanup);
            break;
        case LY_STMT_DEVIATE:
            LY_CHECK_GOTO(ret = parse_deviate(ctx, &dev->deviates), cleanup);
            break;
        case LY_STMT_REFERENCE:
            ret = parse_text_field(ctx, dev->ref, LY_STMT_REFERENCE, 0, &dev->ref, Y_STR_ARG, NULL, &dev->exts);
            LY_CHECK_GOTO(ret, cleanup);
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_GOTO(ret = parse_ext(ctx, word, word_len, dev, LY_STMT_DEVIATION, 0, &dev->exts), cleanup);
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "deviation");
            ret = LY_EVALID;
            goto cleanup;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, dev->exts, ret, cleanup);
    }

    /* mandatory substatements */
    if (!dev->deviates) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "deviate", "deviation");
        ret = LY_EVALID;
        goto cleanup;
    }

cleanup:
    if (ret) {
        lysp_deviation_free(&fctx, dev);
        LY_ARRAY_DECREMENT_FREE(*deviations);
    }
    return ret;
}

/**
 * @brief Parse the feature statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] features Features to add to.
 * @return LY_ERR values.
 */
LY_ERR
parse_feature(struct lysp_yang_ctx *ctx, struct lysp_feature **features)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_feature *feat;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *features, feat, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, feat->name, word, word_len, ret, cleanup);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, feat->dsc, LY_STMT_DESCRIPTION, 0, &feat->dsc, Y_STR_ARG, NULL, &feat->exts));
            break;
        case LY_STMT_IF_FEATURE:
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_IF_FEATURE, &feat->iffeatures, Y_STR_ARG, &feat->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, feat->ref, LY_STMT_REFERENCE, 0, &feat->ref, Y_STR_ARG, NULL, &feat->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, &feat->flags, &feat->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, feat, LY_STMT_FEATURE, 0, &feat->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "feature");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, feat->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse the identity statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] identities Identities to add to.
 * @return LY_ERR values.
 */
LY_ERR
parse_identity(struct lysp_yang_ctx *ctx, struct lysp_ident **identities)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_ident *ident;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *identities, ident, LY_EMEM);

    /* get value */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, ident->name, word, word_len, ret, cleanup);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {
        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, ident->dsc, LY_STMT_DESCRIPTION, 0, &ident->dsc, Y_STR_ARG, NULL, &ident->exts));
            break;
        case LY_STMT_IF_FEATURE:
            PARSER_CHECK_STMTVER2_RET(ctx, "if-feature", "identity");
            LY_CHECK_RET(parse_qnames(ctx, LY_STMT_IF_FEATURE, &ident->iffeatures, Y_STR_ARG, &ident->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, ident->ref, LY_STMT_REFERENCE, 0, &ident->ref, Y_STR_ARG, NULL, &ident->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(parse_status(ctx, &ident->flags, &ident->exts));
            break;
        case LY_STMT_BASE:
            if (ident->bases && (PARSER_CUR_PMOD(ctx)->version < LYS_VERSION_1_1)) {
                LOGVAL_PARSER(ctx, LYVE_SYNTAX_YANG,
                        "Identity can be derived from multiple base identities only in YANG 1.1 modules");
                return LY_EVALID;
            }
            LY_CHECK_RET(parse_text_fields(ctx, LY_STMT_BASE, &ident->bases, Y_PREF_IDENTIF_ARG, &ident->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, ident, LY_STMT_IDENTITY, 0, &ident->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "identity");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, ident->exts, ret, cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Parse module substatements.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] mod Module to write to.
 * @return LY_ERR values.
 */
LY_ERR
parse_module(struct lysp_yang_ctx *ctx, struct lysp_module *mod)
{
    LY_ERR ret = 0;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw, prev_kw = 0;
    enum yang_module_stmt mod_stmt = Y_MOD_MODULE_HEADER;
    const struct lysp_submodule *dup;

    mod->is_submod = 0;

    /* module name */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, mod->mod->name, word, word_len, ret, cleanup);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {

#define CHECK_ORDER(SECTION) \
        if (mod_stmt > SECTION) {\
            LOGVAL_PARSER(ctx, LY_VCODE_INORD, lyplg_ext_stmt2str(kw), lyplg_ext_stmt2str(prev_kw)); return LY_EVALID;\
        } mod_stmt = SECTION

        switch (kw) {
        /* module header */
        case LY_STMT_NAMESPACE:
        case LY_STMT_PREFIX:
            CHECK_ORDER(Y_MOD_MODULE_HEADER);
            break;
        case LY_STMT_YANG_VERSION:
            CHECK_ORDER(Y_MOD_MODULE_HEADER);
            break;
        /* linkage */
        case LY_STMT_INCLUDE:
        case LY_STMT_IMPORT:
            CHECK_ORDER(Y_MOD_LINKAGE);
            break;
        /* meta */
        case LY_STMT_ORGANIZATION:
        case LY_STMT_CONTACT:
        case LY_STMT_DESCRIPTION:
        case LY_STMT_REFERENCE:
            CHECK_ORDER(Y_MOD_META);
            break;

        /* revision */
        case LY_STMT_REVISION:
            CHECK_ORDER(Y_MOD_REVISION);
            break;
        /* body */
        case LY_STMT_ANYDATA:
        case LY_STMT_ANYXML:
        case LY_STMT_AUGMENT:
        case LY_STMT_CHOICE:
        case LY_STMT_CONTAINER:
        case LY_STMT_DEVIATION:
        case LY_STMT_EXTENSION:
        case LY_STMT_FEATURE:
        case LY_STMT_GROUPING:
        case LY_STMT_IDENTITY:
        case LY_STMT_LEAF:
        case LY_STMT_LEAF_LIST:
        case LY_STMT_LIST:
        case LY_STMT_NOTIFICATION:
        case LY_STMT_RPC:
        case LY_STMT_TYPEDEF:
        case LY_STMT_USES:
            mod_stmt = Y_MOD_BODY;
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            /* no place in the statement order defined */
            break;
        default:
            /* error handled in the next switch */
            break;
        }
#undef CHECK_ORDER

        prev_kw = kw;
        switch (kw) {
        /* module header */
        case LY_STMT_YANG_VERSION:
            LY_CHECK_RET(parse_yangversion(ctx, mod));
            break;
        case LY_STMT_NAMESPACE:
            LY_CHECK_RET(parse_text_field(ctx, mod, LY_STMT_NAMESPACE, 0, &mod->mod->ns, Y_STR_ARG, NULL, &mod->exts));
            break;
        case LY_STMT_PREFIX:
            LY_CHECK_RET(parse_text_field(ctx, mod->mod->prefix, LY_STMT_PREFIX, 0, &mod->mod->prefix, Y_IDENTIF_ARG,
                    NULL, &mod->exts));
            break;

        /* linkage */
        case LY_STMT_INCLUDE:
            LY_CHECK_RET(parse_include(ctx, mod->mod->name, &mod->includes));
            break;
        case LY_STMT_IMPORT:
            LY_CHECK_RET(parse_import(ctx, mod->mod->prefix, &mod->imports));
            break;

        /* meta */
        case LY_STMT_ORGANIZATION:
            LY_CHECK_RET(parse_text_field(ctx, mod, LY_STMT_ORGANIZATION, 0, &mod->mod->org, Y_STR_ARG, NULL, &mod->exts));
            break;
        case LY_STMT_CONTACT:
            LY_CHECK_RET(parse_text_field(ctx, mod, LY_STMT_CONTACT, 0, &mod->mod->contact, Y_STR_ARG, NULL, &mod->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, mod->mod->dsc, LY_STMT_DESCRIPTION, 0, &mod->mod->dsc, Y_STR_ARG, NULL,
                    &mod->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, mod->mod->ref, LY_STMT_REFERENCE, 0, &mod->mod->ref, Y_STR_ARG, NULL,
                    &mod->exts));
            break;

        /* revision */
        case LY_STMT_REVISION:
            LY_CHECK_RET(parse_revision(ctx, &mod->revs));
            break;

        /* body */
        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "module");
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, kw, NULL, &mod->data));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, NULL, &mod->data));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, NULL, &mod->data));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, NULL, &mod->data));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, NULL, &mod->data));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, NULL, &mod->data));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, NULL, &mod->data));
            break;

        case LY_STMT_AUGMENT:
            LY_CHECK_RET(parse_augment(ctx, NULL, &mod->augments));
            break;
        case LY_STMT_DEVIATION:
            LY_CHECK_RET(parse_deviation(ctx, &mod->deviations));
            break;
        case LY_STMT_EXTENSION:
            LY_CHECK_RET(parse_extension(ctx, &mod->extensions));
            break;
        case LY_STMT_FEATURE:
            LY_CHECK_RET(parse_feature(ctx, &mod->features));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, NULL, &mod->groupings));
            break;
        case LY_STMT_IDENTITY:
            LY_CHECK_RET(parse_identity(ctx, &mod->identities));
            break;
        case LY_STMT_NOTIFICATION:
            LY_CHECK_RET(parse_notif(ctx, NULL, &mod->notifs));
            break;
        case LY_STMT_RPC:
            LY_CHECK_RET(parse_action(ctx, NULL, &mod->rpcs));
            break;
        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, NULL, &mod->typedefs));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, mod, LY_STMT_MODULE, 0, &mod->exts));
            break;

        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "module");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, mod->exts, ret, cleanup);
    }

    /* mandatory substatements */
    if (!mod->mod->ns) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "namespace", "module");
        return LY_EVALID;
    } else if (!mod->mod->prefix) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "prefix", "module");
        return LY_EVALID;
    }

    /* submodules share the namespace with the module names, so there must not be
     * a submodule of the same name in the context, no need for revision matching */
    dup = ly_ctx_get_submodule_latest(PARSER_CTX(ctx), mod->mod->name);
    if (dup) {
        LOGVAL_PARSER(ctx, LY_VCODE_NAME2_COL, "module", "submodule", mod->mod->name);
        return LY_EVALID;
    }

cleanup:
    return ret;
}

/**
 * @brief Parse submodule substatements.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[out] submod Parsed submodule structure.
 *
 * @return LY_ERR values.
 */
LY_ERR
parse_submodule(struct lysp_yang_ctx *ctx, struct lysp_submodule *submod)
{
    LY_ERR ret = 0;
    char *buf, *word;
    size_t word_len;
    enum ly_stmt kw, prev_kw = 0;
    enum yang_module_stmt mod_stmt = Y_MOD_MODULE_HEADER;
    const struct lysp_submodule *dup;

    submod->is_submod = 1;

    /* submodule name */
    LY_CHECK_RET(get_argument(ctx, Y_IDENTIF_ARG, NULL, &word, &buf, &word_len));
    INSERT_WORD_GOTO(ctx, buf, submod->name, word, word_len, ret, cleanup);

    YANG_READ_SUBSTMT_FOR_GOTO(ctx, kw, word, word_len, ret, cleanup) {

#define CHECK_ORDER(SECTION) \
        if (mod_stmt > SECTION) {LOGVAL_PARSER(ctx, LY_VCODE_INORD, lyplg_ext_stmt2str(kw), lyplg_ext_stmt2str(prev_kw)); return LY_EVALID;}mod_stmt = SECTION

        switch (kw) {
        /* module header */
        case LY_STMT_BELONGS_TO:
            CHECK_ORDER(Y_MOD_MODULE_HEADER);
            break;
        case LY_STMT_YANG_VERSION:
            CHECK_ORDER(Y_MOD_MODULE_HEADER);
            break;
        /* linkage */
        case LY_STMT_INCLUDE:
        case LY_STMT_IMPORT:
            CHECK_ORDER(Y_MOD_LINKAGE);
            break;
        /* meta */
        case LY_STMT_ORGANIZATION:
        case LY_STMT_CONTACT:
        case LY_STMT_DESCRIPTION:
        case LY_STMT_REFERENCE:
            CHECK_ORDER(Y_MOD_META);
            break;

        /* revision */
        case LY_STMT_REVISION:
            CHECK_ORDER(Y_MOD_REVISION);
            break;
        /* body */
        case LY_STMT_ANYDATA:
        case LY_STMT_ANYXML:
        case LY_STMT_AUGMENT:
        case LY_STMT_CHOICE:
        case LY_STMT_CONTAINER:
        case LY_STMT_DEVIATION:
        case LY_STMT_EXTENSION:
        case LY_STMT_FEATURE:
        case LY_STMT_GROUPING:
        case LY_STMT_IDENTITY:
        case LY_STMT_LEAF:
        case LY_STMT_LEAF_LIST:
        case LY_STMT_LIST:
        case LY_STMT_NOTIFICATION:
        case LY_STMT_RPC:
        case LY_STMT_TYPEDEF:
        case LY_STMT_USES:
            mod_stmt = Y_MOD_BODY;
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            /* no place in the statement order defined */
            break;
        default:
            /* error handled in the next switch */
            break;
        }
#undef CHECK_ORDER

        prev_kw = kw;
        switch (kw) {
        /* module header */
        case LY_STMT_YANG_VERSION:
            LY_CHECK_RET(parse_yangversion(ctx, (struct lysp_module *)submod));
            break;
        case LY_STMT_BELONGS_TO:
            LY_CHECK_RET(parse_belongsto(ctx, submod));
            break;

        /* linkage */
        case LY_STMT_INCLUDE:
            if (submod->version == LYS_VERSION_1_1) {
                LOGWRN(PARSER_CTX(ctx), "YANG version 1.1 expects all includes in main module, includes in submodules (%s) are not necessary.",
                        submod->name);
            }
            LY_CHECK_RET(parse_include(ctx, submod->name, &submod->includes));
            break;
        case LY_STMT_IMPORT:
            LY_CHECK_RET(parse_import(ctx, submod->prefix, &submod->imports));
            break;

        /* meta */
        case LY_STMT_ORGANIZATION:
            LY_CHECK_RET(parse_text_field(ctx, submod, LY_STMT_ORGANIZATION, 0, &submod->org, Y_STR_ARG, NULL, &submod->exts));
            break;
        case LY_STMT_CONTACT:
            LY_CHECK_RET(parse_text_field(ctx, submod, LY_STMT_CONTACT, 0, &submod->contact, Y_STR_ARG, NULL, &submod->exts));
            break;
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(parse_text_field(ctx, submod->dsc, LY_STMT_DESCRIPTION, 0, &submod->dsc, Y_STR_ARG, NULL, &submod->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(parse_text_field(ctx, submod->ref, LY_STMT_REFERENCE, 0, &submod->ref, Y_STR_ARG, NULL, &submod->exts));
            break;

        /* revision */
        case LY_STMT_REVISION:
            LY_CHECK_RET(parse_revision(ctx, &submod->revs));
            break;

        /* body */
        case LY_STMT_ANYDATA:
            PARSER_CHECK_STMTVER2_RET(ctx, "anydata", "submodule");
        /* fall through */
        case LY_STMT_ANYXML:
            LY_CHECK_RET(parse_any(ctx, kw, NULL, &submod->data));
            break;
        case LY_STMT_CHOICE:
            LY_CHECK_RET(parse_choice(ctx, NULL, &submod->data));
            break;
        case LY_STMT_CONTAINER:
            LY_CHECK_RET(parse_container(ctx, NULL, &submod->data));
            break;
        case LY_STMT_LEAF:
            LY_CHECK_RET(parse_leaf(ctx, NULL, &submod->data));
            break;
        case LY_STMT_LEAF_LIST:
            LY_CHECK_RET(parse_leaflist(ctx, NULL, &submod->data));
            break;
        case LY_STMT_LIST:
            LY_CHECK_RET(parse_list(ctx, NULL, &submod->data));
            break;
        case LY_STMT_USES:
            LY_CHECK_RET(parse_uses(ctx, NULL, &submod->data));
            break;

        case LY_STMT_AUGMENT:
            LY_CHECK_RET(parse_augment(ctx, NULL, &submod->augments));
            break;
        case LY_STMT_DEVIATION:
            LY_CHECK_RET(parse_deviation(ctx, &submod->deviations));
            break;
        case LY_STMT_EXTENSION:
            LY_CHECK_RET(parse_extension(ctx, &submod->extensions));
            break;
        case LY_STMT_FEATURE:
            LY_CHECK_RET(parse_feature(ctx, &submod->features));
            break;
        case LY_STMT_GROUPING:
            LY_CHECK_RET(parse_grouping(ctx, NULL, &submod->groupings));
            break;
        case LY_STMT_IDENTITY:
            LY_CHECK_RET(parse_identity(ctx, &submod->identities));
            break;
        case LY_STMT_NOTIFICATION:
            LY_CHECK_RET(parse_notif(ctx, NULL, &submod->notifs));
            break;
        case LY_STMT_RPC:
            LY_CHECK_RET(parse_action(ctx, NULL, &submod->rpcs));
            break;
        case LY_STMT_TYPEDEF:
            LY_CHECK_RET(parse_typedef(ctx, NULL, &submod->typedefs));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(parse_ext(ctx, word, word_len, submod, LY_STMT_SUBMODULE, 0, &submod->exts));
            break;

        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, lyplg_ext_stmt2str(kw), "submodule");
            return LY_EVALID;
        }
        YANG_READ_SUBSTMT_NEXT_ITER(ctx, kw, word, word_len, submod->exts, ret, cleanup);
    }

    /* mandatory substatements */
    if (!submod->prefix) {
        LOGVAL_PARSER(ctx, LY_VCODE_MISSTMT, "belongs-to", "submodule");
        return LY_EVALID;
    }

    /* submodules share the namespace with the module names, so there must not be
     * a submodule of the same name in the context, no need for revision matching */
    dup = ly_ctx_get_submodule_latest(PARSER_CTX(ctx), submod->name);
    /* main modules may have different revisions */
    if (dup && strcmp(dup->mod->name, submod->mod->name)) {
        LOGVAL_PARSER(ctx, LY_VCODE_NAME_COL, "submodules", dup->name);
        return LY_EVALID;
    }

cleanup:
    return ret;
}

/**
 * @brief Skip any redundant characters, namely whitespaces and comments.
 *
 * @param[in] ctx Yang parser context.
 * @return LY_SUCCESS on success.
 * @return LY_EVALID on invalid comment.
 */
static LY_ERR
skip_redundant_chars(struct lysp_yang_ctx *ctx)
{
    /* read some trailing spaces, new lines, or comments */
    while (ctx->in->current[0]) {
        if (!strncmp(ctx->in->current, "//", 2)) {
            /* one-line comment */
            ly_in_skip(ctx->in, 2);
            LY_CHECK_RET(skip_comment(ctx, 1));
        } else if (!strncmp(ctx->in->current, "/*", 2)) {
            /* block comment */
            ly_in_skip(ctx->in, 2);
            LY_CHECK_RET(skip_comment(ctx, 2));
        } else if (isspace(ctx->in->current[0])) {
            /* whitespace */
            if (ctx->in->current[0] == '\n') {
                LY_IN_NEW_LINE(ctx->in);
            }
            ly_in_skip(ctx->in, 1);
        } else {
            break;
        }
    }

    return LY_SUCCESS;
}

LY_ERR
yang_parse_submodule(struct lysp_yang_ctx **context, struct ly_ctx *ly_ctx, struct lysp_ctx *main_ctx,
        struct ly_in *in, struct lysp_submodule **submod)
{
    LY_ERR ret = LY_SUCCESS;
    char *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_submodule *mod_p = NULL;
    struct lysf_ctx fctx = {.ctx = ly_ctx};

    assert(context && ly_ctx && main_ctx && in && submod);

    /* create context */
    *context = calloc(1, sizeof **context);
    LY_CHECK_ERR_RET(!(*context), LOGMEM(ly_ctx), LY_EMEM);
    (*context)->format = LYS_IN_YANG;
    (*context)->in = in;
    (*context)->main_ctx = main_ctx;

    mod_p = calloc(1, sizeof *mod_p);
    LY_CHECK_ERR_GOTO(!mod_p, LOGMEM(ly_ctx); ret = LY_EMEM, cleanup);
    mod_p->mod = PARSER_CUR_PMOD(main_ctx)->mod;
    mod_p->parsing = 1;

    /* use main context parsed mods adding the current one */
    (*context)->parsed_mods = main_ctx->parsed_mods;
    ly_set_add((*context)->parsed_mods, mod_p, 1, NULL);

    ly_log_location(NULL, NULL, NULL, in);

    /* skip redundant but valid characters at the beginning */
    ret = skip_redundant_chars(*context);
    LY_CHECK_GOTO(ret, cleanup);

    /* "module"/"submodule" */
    ret = get_keyword(*context, &kw, &word, &word_len);
    LY_CHECK_GOTO(ret, cleanup);

    if (kw == LY_STMT_MODULE) {
        LOGERR(ly_ctx, LY_EDENIED, "Input data contains module in situation when a submodule is expected.");
        ret = LY_EINVAL;
        goto cleanup;
    } else if (kw != LY_STMT_SUBMODULE) {
        LOGVAL_PARSER(*context, LY_VCODE_MOD_SUBOMD, lyplg_ext_stmt2str(kw));
        ret = LY_EVALID;
        goto cleanup;
    }

    /* substatements */
    ret = parse_submodule(*context, mod_p);
    LY_CHECK_GOTO(ret, cleanup);

    /* skip redundant but valid characters at the end */
    ret = skip_redundant_chars(*context);
    LY_CHECK_GOTO(ret, cleanup);
    if (in->current[0]) {
        LOGVAL_PARSER(*context, LY_VCODE_TRAILING_SUBMOD, 15, in->current, strlen(in->current) > 15 ? "..." : "");
        ret = LY_EVALID;
        goto cleanup;
    }

    mod_p->parsing = 0;
    *submod = mod_p;

cleanup:
    ly_log_location_revert(0, 0, 0, 1);
    if (ret) {
        lysp_module_free(&fctx, (struct lysp_module *)mod_p);
        lysp_yang_ctx_free(*context);
        *context = NULL;
    }

    return ret;
}

LY_ERR
yang_parse_module(struct lysp_yang_ctx **context, struct ly_in *in, struct lys_module *mod)
{
    LY_ERR ret = LY_SUCCESS;
    char *word;
    size_t word_len;
    enum ly_stmt kw;
    struct lysp_module *mod_p = NULL;
    struct lysf_ctx fctx = {.ctx = mod->ctx};

    /* create context */
    *context = calloc(1, sizeof **context);
    LY_CHECK_ERR_RET(!(*context), LOGMEM(mod->ctx), LY_EMEM);
    (*context)->format = LYS_IN_YANG;
    LY_CHECK_ERR_RET(ly_set_new(&(*context)->parsed_mods), free(*context); LOGMEM(mod->ctx), LY_EMEM);
    (*context)->in = in;
    (*context)->main_ctx = (struct lysp_ctx *)(*context);

    mod_p = calloc(1, sizeof *mod_p);
    LY_CHECK_ERR_GOTO(!mod_p, LOGMEM(mod->ctx), cleanup);
    mod_p->mod = mod;
    ly_set_add((*context)->parsed_mods, mod_p, 1, NULL);

    ly_log_location(NULL, NULL, NULL, in);

    /* skip redundant but valid characters at the beginning */
    ret = skip_redundant_chars(*context);
    LY_CHECK_GOTO(ret, cleanup);

    /* "module"/"submodule" */
    ret = get_keyword(*context, &kw, &word, &word_len);
    LY_CHECK_GOTO(ret, cleanup);

    if (kw == LY_STMT_SUBMODULE) {
        LOGERR(mod->ctx, LY_EDENIED, "Input data contains submodule which cannot be parsed directly without its main module.");
        ret = LY_EINVAL;
        goto cleanup;
    } else if (kw != LY_STMT_MODULE) {
        LOGVAL_PARSER((*context), LY_VCODE_MOD_SUBOMD, lyplg_ext_stmt2str(kw));
        ret = LY_EVALID;
        goto cleanup;
    }

    /* substatements */
    ret = parse_module(*context, mod_p);
    LY_CHECK_GOTO(ret, cleanup);

    /* skip redundant but valid characters at the end */
    ret = skip_redundant_chars(*context);
    LY_CHECK_GOTO(ret, cleanup);
    if (in->current[0]) {
        LOGVAL_PARSER(*context, LY_VCODE_TRAILING_MOD, 15, in->current, strlen(in->current) > 15 ? "..." : "");
        ret = LY_EVALID;
        goto cleanup;
    }

    mod->parsed = mod_p;

cleanup:
    ly_log_location_revert(0, 0, 0, 1);
    if (ret) {
        lysp_module_free(&fctx, mod_p);
        lysp_yang_ctx_free(*context);
        *context = NULL;
    }

    return ret;
}
