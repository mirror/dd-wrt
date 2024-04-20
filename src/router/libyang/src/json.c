/**
 * @file json.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Generic JSON format parser for libyang
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "common.h"
#include "in_internal.h"
#include "json.h"

#define JSON_PUSH_STATUS_RET(CTX, STATUS) \
    LY_CHECK_RET(ly_set_add(&CTX->status, (void*)STATUS, 1, NULL))

#define JSON_POP_STATUS_RET(CTX) \
    assert(CTX->status.count); CTX->status.count--;

const char *
lyjson_token2str(enum LYJSON_PARSER_STATUS status)
{
    switch (status) {
    case LYJSON_ERROR:
        return "error";
    case LYJSON_ROOT:
        return "document root";
    case LYJSON_FALSE:
        return "false";
    case LYJSON_TRUE:
        return "true";
    case LYJSON_NULL:
        return "null";
    case LYJSON_OBJECT:
        return "object";
    case LYJSON_OBJECT_CLOSED:
        return "object closed";
    case LYJSON_OBJECT_EMPTY:
        return "empty object";
    case LYJSON_ARRAY:
        return "array";
    case LYJSON_ARRAY_CLOSED:
        return "array closed";
    case LYJSON_ARRAY_EMPTY:
        return "empty array";
    case LYJSON_NUMBER:
        return "number";
    case LYJSON_STRING:
        return "string";
    case LYJSON_END:
        return "end of input";
    }

    return "";
}

static LY_ERR
skip_ws(struct lyjson_ctx *jsonctx)
{
    /* skip leading whitespaces */
    while (*jsonctx->in->current != '\0' && is_jsonws(*jsonctx->in->current)) {
        if (*jsonctx->in->current == '\n') {
            LY_IN_NEW_LINE(jsonctx->in);
        }
        ly_in_skip(jsonctx->in, 1);
    }
    if (*jsonctx->in->current == '\0') {
        JSON_PUSH_STATUS_RET(jsonctx, LYJSON_END);
    }

    return LY_SUCCESS;
}

/*
 * @brief Set value corresponding to the current context's status
 */
static void
lyjson_ctx_set_value(struct lyjson_ctx *jsonctx, const char *value, size_t value_len, ly_bool dynamic)
{
    assert(jsonctx);

    if (jsonctx->dynamic) {
        free((char *)jsonctx->value);
    }
    jsonctx->value = value;
    jsonctx->value_len = value_len;
    jsonctx->dynamic = dynamic;
}

static LY_ERR
lyjson_check_next(struct lyjson_ctx *jsonctx)
{
    if (jsonctx->status.count == 1) {
        /* top level value (JSON-text), ws expected */
        if ((*jsonctx->in->current == '\0') || is_jsonws(*jsonctx->in->current)) {
            return LY_SUCCESS;
        }
    } else if (lyjson_ctx_status(jsonctx, 1) == LYJSON_OBJECT) {
        LY_CHECK_RET(skip_ws(jsonctx));
        if ((*jsonctx->in->current == ',') || (*jsonctx->in->current == '}')) {
            return LY_SUCCESS;
        }
    } else if (lyjson_ctx_status(jsonctx, 1) == LYJSON_ARRAY) {
        LY_CHECK_RET(skip_ws(jsonctx));
        if ((*jsonctx->in->current == ',') || (*jsonctx->in->current == ']')) {
            return LY_SUCCESS;
        }
    }

    LOGVAL(jsonctx->ctx, LYVE_SYNTAX, "Unexpected character \"%c\" after JSON %s.",
            *jsonctx->in->current, lyjson_token2str(lyjson_ctx_status(jsonctx, 0)));
    return LY_EVALID;
}

/**
 * Input is expected to start after the opening quotation-mark.
 * When succeeds, input is moved after the closing quotation-mark.
 */
static LY_ERR
lyjson_string_(struct lyjson_ctx *jsonctx)
{
#define BUFSIZE 24
#define BUFSIZE_STEP 128

    const char *in = jsonctx->in->current, *start;
    char *buf = NULL;
    size_t offset;   /* read offset in input buffer */
    size_t len;      /* length of the output string (write offset in output buffer) */
    size_t size = 0; /* size of the output buffer */
    size_t u;
    uint64_t start_line;

    assert(jsonctx);

    /* init */
    start = in;
    start_line = jsonctx->in->line;
    offset = len = 0;

    /* parse */
    while (in[offset]) {
        if (in[offset] == '\\') {
            /* escape sequence */
            const char *slash = &in[offset];
            uint32_t value;
            uint8_t i = 1;

            if (!buf) {
                /* prepare output buffer */
                buf = malloc(BUFSIZE);
                LY_CHECK_ERR_RET(!buf, LOGMEM(jsonctx->ctx), LY_EMEM);
                size = BUFSIZE;
            }

            /* allocate enough for the offset and next character,
             * we will need 4 bytes at most since we support only the predefined
             * (one-char) entities and character references */
            if (len + offset + 4 >= size) {
                size_t increment;
                for (increment = BUFSIZE_STEP; len + offset + 4 >= size + increment; increment += BUFSIZE_STEP) {}
                buf = ly_realloc(buf, size + increment);
                LY_CHECK_ERR_RET(!buf, LOGMEM(jsonctx->ctx), LY_EMEM);
                size += BUFSIZE_STEP;
            }

            if (offset) {
                /* store what we have so far */
                memcpy(&buf[len], in, offset);
                len += offset;
                in += offset;
                offset = 0;
            }

            switch (in[++offset]) {
            case '"':
                /* quotation mark */
                value = 0x22;
                break;
            case '\\':
                /* reverse solidus */
                value = 0x5c;
                break;
            case '/':
                /* solidus */
                value = 0x2f;
                break;
            case 'b':
                /* backspace */
                value = 0x08;
                break;
            case 'f':
                /* form feed */
                value = 0x0c;
                break;
            case 'n':
                /* line feed */
                value = 0x0a;
                break;
            case 'r':
                /* carriage return */
                value = 0x0d;
                break;
            case 't':
                /* tab */
                value = 0x09;
                break;
            case 'u':
                /* Basic Multilingual Plane character \uXXXX */
                offset++;
                for (value = i = 0; i < 4; i++) {
                    if (!in[offset + i]) {
                        LOGVAL(jsonctx->ctx, LYVE_SYNTAX, "Invalid basic multilingual plane character \"%s\".", slash);
                        goto error;
                    } else if (isdigit(in[offset + i])) {
                        u = (in[offset + i] - '0');
                    } else if (in[offset + i] > 'F') {
                        u = LY_BASE_DEC + (in[offset + i] - 'a');
                    } else {
                        u = LY_BASE_DEC + (in[offset + i] - 'A');
                    }
                    value = (LY_BASE_HEX * value) + u;
                }
                break;
            default:
                /* invalid escape sequence */
                LOGVAL(jsonctx->ctx, LYVE_SYNTAX, "Invalid character escape sequence \\%c.", in[offset]);
                goto error;

            }

            offset += i;   /* add read escaped characters */
            LY_CHECK_ERR_GOTO(ly_pututf8(&buf[len], value, &u),
                    LOGVAL(jsonctx->ctx, LYVE_SYNTAX, "Invalid character reference \"%.*s\" (0x%08x).",
                    (int)(&in[offset] - slash), slash, value),
                    error);
            len += u;      /* update number of bytes in buffer */
            in += offset;  /* move the input by the processed bytes stored in the buffer ... */
            offset = 0;    /* ... and reset the offset index for future moving data into buffer */

        } else if (in[offset] == '"') {
            /* end of string */
            if (buf) {
                /* realloc exact size string */
                buf = ly_realloc(buf, len + offset + 1);
                LY_CHECK_ERR_RET(!buf, LOGMEM(jsonctx->ctx), LY_EMEM);
                size = len + offset + 1;
                memcpy(&buf[len], in, offset);

                /* set terminating NULL byte */
                buf[len + offset] = '\0';
            }
            len += offset;
            ++offset;
            in += offset;
            goto success;
        } else {
            /* get it as UTF-8 character for check */
            const char *c = &in[offset];
            uint32_t code = 0;
            size_t code_len = 0;

            LY_CHECK_ERR_GOTO(ly_getutf8(&c, &code, &code_len),
                    LOGVAL(jsonctx->ctx, LY_VCODE_INCHAR, in[offset]), error);

            LY_CHECK_ERR_GOTO(!is_jsonstrchar(code),
                    LOGVAL(jsonctx->ctx, LYVE_SYNTAX, "Invalid character in JSON string \"%.*s\" (0x%08x).",
                    (int)(&in[offset] - start + code_len), start, code),
                    error);

            /* character is ok, continue */
            offset += code_len;
        }
    }

    /* EOF reached before endchar */
    LOGVAL(jsonctx->ctx, LY_VCODE_EOF);
    LOGVAL_LINE(jsonctx->ctx, start_line, LYVE_SYNTAX, "Missing quotation-mark at the end of a JSON string.");

error:
    free(buf);
    return LY_EVALID;

success:
    jsonctx->in->current = in;
    if (buf) {
        lyjson_ctx_set_value(jsonctx, buf, len, 1);
    } else {
        lyjson_ctx_set_value(jsonctx, start, len, 0);
    }

    return LY_SUCCESS;

#undef BUFSIZE
#undef BUFSIZE_STEP
}

/*
 *
 * Wrapper around lyjson_string_() adding LYJSON_STRING status into context to allow using lyjson_string_() for parsing object's name.
 */
static LY_ERR
lyjson_string(struct lyjson_ctx *jsonctx)
{
    LY_CHECK_RET(lyjson_string_(jsonctx));

    JSON_PUSH_STATUS_RET(jsonctx, LYJSON_STRING);
    LY_CHECK_RET(lyjson_check_next(jsonctx));

    return LY_SUCCESS;
}

/**
 * @brief Allocate buffer for number in string format.
 *
 * @param[in] jsonctx JSON context.
 * @param[in] num_len Required space in bytes for a number.
 * Terminating null byte is added by default.
 * @param[out] buffer Output allocated buffer.
 * @return LY_ERR value.
 */
static LY_ERR
lyjson_get_buffer_for_number(struct lyjson_ctx *jsonctx, size_t num_len, char **buffer)
{
    *buffer = NULL;

    LY_CHECK_ERR_RET((num_len + 1) > LY_NUMBER_MAXLEN, LOGVAL(jsonctx->ctx, LYVE_SEMANTICS,
            "Number encoded as a string exceeded the LY_NUMBER_MAXLEN limit."), LY_EVALID);

    /* allocate buffer for the result (add terminating NULL-byte) */
    *buffer = malloc(num_len + 1);
    LY_CHECK_ERR_RET(!(*buffer), LOGMEM(jsonctx->ctx), LY_EMEM);
    return LY_SUCCESS;
}

static LY_ERR
lyjson_number(struct lyjson_ctx *jsonctx)
{
    size_t offset = 0, exponent = 0;
    const char *in = jsonctx->in->current;
    uint8_t minus = 0;

    if (in[offset] == '-') {
        ++offset;
        minus = 1;
    }

    if (in[offset] == '0') {
        ++offset;
    } else if (isdigit(in[offset])) {
        ++offset;
        while (isdigit(in[offset])) {
            ++offset;
        }
    } else {
invalid_character:
        if (in[offset]) {
            LOGVAL(jsonctx->ctx, LYVE_SYNTAX, "Invalid character in JSON Number value (\"%c\").", in[offset]);
        } else {
            LOGVAL(jsonctx->ctx, LY_VCODE_EOF);
        }
        return LY_EVALID;
    }

    if (in[offset] == '.') {
        ++offset;
        if (!isdigit(in[offset])) {
            goto invalid_character;
        }
        while (isdigit(in[offset])) {
            ++offset;
        }
    }

    if ((in[offset] == 'e') || (in[offset] == 'E')) {
        exponent = offset++;
        if ((in[offset] == '+') || (in[offset] == '-')) {
            ++offset;
        }
        if (!isdigit(in[offset])) {
            goto invalid_character;
        }
        while (isdigit(in[offset])) {
            ++offset;
        }
    }

    if (exponent) {
        /* convert JSON number with exponent into the representation used by YANG */
        long int  e_val;
        char *ptr, *dec_point, *num;
        const char *e_ptr = &in[exponent + 1];
        size_t num_len, i;
        int64_t dp_position; /* final position of the deciaml point */

        errno = 0;
        e_val = strtol(e_ptr, &ptr, LY_BASE_DEC);
        if (errno) {
            LOGVAL(jsonctx->ctx, LYVE_SEMANTICS, "Exponent out-of-bounds in a JSON Number value (%.*s).",
                    (int)(offset - minus - (e_ptr - in)), e_ptr);
            return LY_EVALID;
        }

        if (!e_val) {
            /* exponent is zero, so just cut the part with the exponent */
            num_len = exponent;
            LY_CHECK_RET(lyjson_get_buffer_for_number(jsonctx, num_len, &num));
            memcpy(num, in, num_len);
            num[num_len] = '\0';
            goto store_exp_number;
        }

        dec_point = ly_strnchr(in, '.', exponent);
        if (!dec_point) {
            /* value is integer, we are just ... */
            if (e_val >= 0) {
                /* adding zeros at the end */
                num_len = exponent + e_val;
                dp_position = num_len; /* decimal point is behind the actual value */
            } else if ((size_t)labs(e_val) < (exponent - minus)) {
                /* adding decimal point between the integer's digits */
                num_len = exponent + 1;
                dp_position = exponent + e_val;
            } else {
                /* adding decimal point before the integer with adding leading zero(s) */
                num_len = labs(e_val) + 2 + minus;
                dp_position = exponent + e_val;
            }
            dp_position -= minus;
        } else {
            /* value is decimal, we are moving the decimal point */
            dp_position = dec_point - in + e_val - minus;
            if (dp_position > (ssize_t)exponent) {
                /* moving decimal point after the decimal value make the integer result */
                num_len = dp_position;
            } else if (dp_position < 0) {
                /* moving decimal point before the decimal value requires additional zero(s)
                 * (decimal point is already count in exponent value) */
                num_len = exponent + labs(dp_position) + 1;
            } else if (dp_position == 0) {
                /* moving the decimal point exactly to the beginning will cause a zero character to be added. */
                num_len = exponent + 1;
            } else {
                /* moving decimal point just inside the decimal value does not make any change in length */
                num_len = exponent;
            }
        }

        LY_CHECK_RET(lyjson_get_buffer_for_number(jsonctx, num_len, &num));

        /* compose the resulting vlaue */
        i = 0;
        if (minus) {
            num[i++] = '-';
        }
        /* add leading zeros */
        if (dp_position <= 0) {
            num[i++] = '0';
            num[i++] = '.';
            for ( ; dp_position; dp_position++) {
                num[i++] = '0';
            }
        }
        /* copy the value */
        ly_bool dp_placed;
        size_t j;
        for (dp_placed = dp_position ? 0 : 1, j = minus; j < exponent; j++) {
            if (in[j] == '.') {
                continue;
            }
            if (!dp_placed) {
                if (!dp_position) {
                    num[i++] = '.';
                    dp_placed = 1;
                } else {
                    dp_position--;
                    if (in[j] == '0') {
                        num_len--;
                        continue;
                    }
                }
            }

            num[i++] = in[j];
        }
        /* trailing zeros */
        while (dp_position--) {
            num[i++] = '0';
        }
        /* terminating NULL byte */
        num[i] = '\0';

store_exp_number:
        /* store the modified number */
        lyjson_ctx_set_value(jsonctx, num, num_len, 1);
    } else {
        /* store the number */
        lyjson_ctx_set_value(jsonctx, jsonctx->in->current, offset, 0);
    }
    ly_in_skip(jsonctx->in, offset);

    JSON_PUSH_STATUS_RET(jsonctx, LYJSON_NUMBER);
    LY_CHECK_RET(lyjson_check_next(jsonctx));

    return LY_SUCCESS;
}

static LY_ERR
lyjson_object_name(struct lyjson_ctx *jsonctx)
{
    if (*jsonctx->in->current != '"') {
        LOGVAL(jsonctx->ctx, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(jsonctx->in->current),
                jsonctx->in->current, "a JSON object's member");
        return LY_EVALID;
    }
    ly_in_skip(jsonctx->in, 1);

    LY_CHECK_RET(lyjson_string_(jsonctx));
    LY_CHECK_RET(skip_ws(jsonctx));
    if (*jsonctx->in->current != ':') {
        LOGVAL(jsonctx->ctx, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(jsonctx->in->current), jsonctx->in->current,
                "a JSON object's name-separator ':'");
        return LY_EVALID;
    }
    ly_in_skip(jsonctx->in, 1);
    LY_CHECK_RET(skip_ws(jsonctx));

    return LY_SUCCESS;
}

static LY_ERR
lyjson_object(struct lyjson_ctx *jsonctx)
{
    LY_CHECK_RET(skip_ws(jsonctx));

    if (*jsonctx->in->current == '}') {
        assert(jsonctx->depth);
        jsonctx->depth--;
        /* empty object */
        ly_in_skip(jsonctx->in, 1);
        lyjson_ctx_set_value(jsonctx, NULL, 0, 0);
        JSON_PUSH_STATUS_RET(jsonctx, LYJSON_OBJECT_EMPTY);
        return LY_SUCCESS;
    }

    LY_CHECK_RET(lyjson_object_name(jsonctx));

    /* output data are set by lyjson_string_() */
    JSON_PUSH_STATUS_RET(jsonctx, LYJSON_OBJECT);

    return LY_SUCCESS;
}

/*
 * @brief Process JSON array envelope
 *
 *
 *
 * @param[in] jsonctx JSON parser context
 * @return LY_SUCCESS or LY_EMEM
 */
static LY_ERR
lyjson_array(struct lyjson_ctx *jsonctx)
{
    LY_CHECK_RET(skip_ws(jsonctx));

    if (*jsonctx->in->current == ']') {
        /* empty array */
        ly_in_skip(jsonctx->in, 1);
        JSON_PUSH_STATUS_RET(jsonctx, LYJSON_ARRAY_EMPTY);
    } else {
        JSON_PUSH_STATUS_RET(jsonctx, LYJSON_ARRAY);
    }

    /* erase previous values, array has no value on its own */
    lyjson_ctx_set_value(jsonctx, NULL, 0, 0);

    return LY_SUCCESS;
}

static LY_ERR
lyjson_value(struct lyjson_ctx *jsonctx)
{
    if (jsonctx->status.count && (lyjson_ctx_status(jsonctx, 0) == LYJSON_END)) {
        return LY_SUCCESS;
    }

    if ((*jsonctx->in->current == 'f') && !strncmp(jsonctx->in->current, "false", ly_strlen_const("false"))) {
        /* false */
        lyjson_ctx_set_value(jsonctx, jsonctx->in->current, ly_strlen_const("false"), 0);
        ly_in_skip(jsonctx->in, ly_strlen_const("false"));
        JSON_PUSH_STATUS_RET(jsonctx, LYJSON_FALSE);
        LY_CHECK_RET(lyjson_check_next(jsonctx));

    } else if ((*jsonctx->in->current == 't') && !strncmp(jsonctx->in->current, "true", ly_strlen_const("true"))) {
        /* true */
        lyjson_ctx_set_value(jsonctx, jsonctx->in->current, ly_strlen_const("true"), 0);
        ly_in_skip(jsonctx->in, ly_strlen_const("true"));
        JSON_PUSH_STATUS_RET(jsonctx, LYJSON_TRUE);
        LY_CHECK_RET(lyjson_check_next(jsonctx));

    } else if ((*jsonctx->in->current == 'n') && !strncmp(jsonctx->in->current, "null", ly_strlen_const("null"))) {
        /* none */
        lyjson_ctx_set_value(jsonctx, "", 0, 0);
        ly_in_skip(jsonctx->in, ly_strlen_const("null"));
        JSON_PUSH_STATUS_RET(jsonctx, LYJSON_NULL);
        LY_CHECK_RET(lyjson_check_next(jsonctx));

    } else if (*jsonctx->in->current == '"') {
        /* string */
        ly_in_skip(jsonctx->in, 1);
        LY_CHECK_RET(lyjson_string(jsonctx));

    } else if (*jsonctx->in->current == '[') {
        /* array */
        ly_in_skip(jsonctx->in, 1);
        LY_CHECK_RET(lyjson_array(jsonctx));

    } else if (*jsonctx->in->current == '{') {
        jsonctx->depth++;
        if (jsonctx->depth > LY_MAX_BLOCK_DEPTH) {
            LOGERR(jsonctx->ctx, LY_EINVAL,
                    "The maximum number of block nestings has been exceeded.");
            return LY_EINVAL;
        }
        /* object */
        ly_in_skip(jsonctx->in, 1);
        LY_CHECK_RET(lyjson_object(jsonctx));

    } else if ((*jsonctx->in->current == '-') || ((*jsonctx->in->current >= '0') && (*jsonctx->in->current <= '9'))) {
        /* number */
        LY_CHECK_RET(lyjson_number(jsonctx));

    } else {
        /* unexpected value */
        LOGVAL(jsonctx->ctx, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(jsonctx->in->current),
                jsonctx->in->current, "a JSON value");
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

LY_ERR
lyjson_ctx_new(const struct ly_ctx *ctx, struct ly_in *in, struct lyjson_ctx **jsonctx_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyjson_ctx *jsonctx;

    assert(ctx);
    assert(in);
    assert(jsonctx_p);

    /* new context */
    jsonctx = calloc(1, sizeof *jsonctx);
    LY_CHECK_ERR_RET(!jsonctx, LOGMEM(ctx), LY_EMEM);
    jsonctx->ctx = ctx;
    jsonctx->in = in;

    LOG_LOCINIT(NULL, NULL, NULL, in);

    /* parse JSON value, if any */
    LY_CHECK_GOTO(ret = skip_ws(jsonctx), cleanup);
    if (lyjson_ctx_status(jsonctx, 0) == LYJSON_END) {
        /* empty data input */
        goto cleanup;
    }

    ret = lyjson_value(jsonctx);

    if ((jsonctx->status.count > 1) && (lyjson_ctx_status(jsonctx, 0) == LYJSON_END)) {
        LOGVAL(jsonctx->ctx, LY_VCODE_EOF);
        ret = LY_EVALID;
    }

cleanup:
    if (ret) {
        lyjson_ctx_free(jsonctx);
    } else {
        *jsonctx_p = jsonctx;
    }
    return ret;
}

void
lyjson_ctx_backup(struct lyjson_ctx *jsonctx)
{
    if (jsonctx->backup.dynamic) {
        free((char *)jsonctx->backup.value);
    }
    jsonctx->backup.status = lyjson_ctx_status(jsonctx, 0);
    jsonctx->backup.status_count = jsonctx->status.count;
    jsonctx->backup.value = jsonctx->value;
    jsonctx->backup.value_len = jsonctx->value_len;
    jsonctx->backup.input = jsonctx->in->current;
    jsonctx->backup.dynamic = jsonctx->dynamic;
    jsonctx->backup.depth = jsonctx->depth;
    jsonctx->dynamic = 0;
}

void
lyjson_ctx_restore(struct lyjson_ctx *jsonctx)
{
    if (jsonctx->dynamic) {
        free((char *)jsonctx->value);
    }
    jsonctx->status.count = jsonctx->backup.status_count;
    jsonctx->status.objs[jsonctx->backup.status_count - 1] = (void *)jsonctx->backup.status;
    jsonctx->value = jsonctx->backup.value;
    jsonctx->value_len = jsonctx->backup.value_len;
    jsonctx->in->current = jsonctx->backup.input;
    jsonctx->dynamic = jsonctx->backup.dynamic;
    jsonctx->depth = jsonctx->backup.depth;
    jsonctx->backup.dynamic = 0;
}

LY_ERR
lyjson_ctx_next(struct lyjson_ctx *jsonctx, enum LYJSON_PARSER_STATUS *status)
{
    LY_ERR ret = LY_SUCCESS;
    ly_bool toplevel = 0;
    enum LYJSON_PARSER_STATUS prev;

    assert(jsonctx);

    prev = lyjson_ctx_status(jsonctx, 0);

    if ((prev == LYJSON_OBJECT) || (prev == LYJSON_ARRAY)) {
        /* get value for the object's member OR the first value in the array */
        ret = lyjson_value(jsonctx);
        goto result;
    } else {
        /* the previous token is closed and should be completely processed */
        JSON_POP_STATUS_RET(jsonctx);
        prev = lyjson_ctx_status(jsonctx, 0);
    }

    if (!jsonctx->status.count) {
        /* we are done with the top level value */
        toplevel = 1;
    }
    LY_CHECK_RET(skip_ws(jsonctx));
    if (toplevel && !jsonctx->status.count) {
        /* EOF expected, but there are some data after the top level token */
        LOGVAL(jsonctx->ctx, LYVE_SYNTAX, "Expecting end-of-input, but some data follows the top level JSON value.");
        return LY_EVALID;
    }

    if (toplevel) {
        /* we are done */
        return LY_SUCCESS;
    }

    /* continue with the next token */
    assert(prev == LYJSON_OBJECT || prev == LYJSON_ARRAY);

    if (*jsonctx->in->current == ',') {
        /* sibling item in the ... */
        ly_in_skip(jsonctx->in, 1);
        LY_CHECK_RET(skip_ws(jsonctx));

        if (prev == LYJSON_OBJECT) {
            /* ... object - get another object's member */
            ret = lyjson_object_name(jsonctx);
        } else { /* LYJSON_ARRAY */
            /* ... array - get another complete value */
            ret = lyjson_value(jsonctx);
        }
    } else if (((prev == LYJSON_OBJECT) && (*jsonctx->in->current == '}')) || ((prev == LYJSON_ARRAY) && (*jsonctx->in->current == ']'))) {
        if (*jsonctx->in->current == '}') {
            assert(jsonctx->depth);
            jsonctx->depth--;
        }
        ly_in_skip(jsonctx->in, 1);
        JSON_POP_STATUS_RET(jsonctx);
        JSON_PUSH_STATUS_RET(jsonctx, prev + 1);
    } else {
        /* unexpected value */
        LOGVAL(jsonctx->ctx, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(jsonctx->in->current), jsonctx->in->current,
                prev == LYJSON_ARRAY ? "another JSON value in array" : "another JSON object's member");
        return LY_EVALID;
    }

result:
    if ((ret == LY_SUCCESS) && (jsonctx->status.count > 1) && (lyjson_ctx_status(jsonctx, 0) == LYJSON_END)) {
        LOGVAL(jsonctx->ctx, LY_VCODE_EOF);
        ret = LY_EVALID;
    }

    if ((ret == LY_SUCCESS) && status) {
        *status = lyjson_ctx_status(jsonctx, 0);
    }

    return ret;
}

enum LYJSON_PARSER_STATUS
lyjson_ctx_status(struct lyjson_ctx *jsonctx, uint32_t index)
{
    assert(jsonctx);

    if (jsonctx->status.count < index) {
        return LYJSON_ERROR;
    } else if (jsonctx->status.count == index) {
        return LYJSON_ROOT;
    } else {
        return (enum LYJSON_PARSER_STATUS)(uintptr_t)jsonctx->status.objs[jsonctx->status.count - (index + 1)];
    }
}

void
lyjson_ctx_free(struct lyjson_ctx *jsonctx)
{
    if (!jsonctx) {
        return;
    }

    LOG_LOCBACK(0, 0, 0, 1);

    if (jsonctx->dynamic) {
        free((char *)jsonctx->value);
    }
    if (jsonctx->backup.dynamic) {
        free((char *)jsonctx->backup.value);
    }

    ly_set_erase(&jsonctx->status, NULL);

    free(jsonctx);
}
