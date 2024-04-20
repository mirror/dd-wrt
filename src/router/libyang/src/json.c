/**
 * @file json.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Generic JSON format parser for libyang
 *
 * Copyright (c) 2020 - 2023 CESNET, z.s.p.o.
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
#include "tree_schema_internal.h"

const char *
lyjson_token2str(enum LYJSON_PARSER_STATUS status)
{
    switch (status) {
    case LYJSON_ERROR:
        return "error";
    case LYJSON_OBJECT:
        return "object";
    case LYJSON_OBJECT_NEXT:
        return "object next";
    case LYJSON_OBJECT_CLOSED:
        return "object closed";
    case LYJSON_ARRAY:
        return "array";
    case LYJSON_ARRAY_NEXT:
        return "array next";
    case LYJSON_ARRAY_CLOSED:
        return "array closed";
    case LYJSON_OBJECT_NAME:
        return "object name";
    case LYJSON_NUMBER:
        return "number";
    case LYJSON_STRING:
        return "string";
    case LYJSON_TRUE:
        return "true";
    case LYJSON_FALSE:
        return "false";
    case LYJSON_NULL:
        return "null";
    case LYJSON_END:
        return "end of input";
    }

    return "";
}

enum LYJSON_PARSER_STATUS
lyjson_ctx_status(struct lyjson_ctx *jsonctx)
{
    assert(jsonctx);

    if (!jsonctx->status.count) {
        return LYJSON_END;
    }

    return (enum LYJSON_PARSER_STATUS)(uintptr_t)jsonctx->status.objs[jsonctx->status.count - 1];
}

uint32_t
lyjson_ctx_depth(struct lyjson_ctx *jsonctx)
{
    return jsonctx->status.count;
}

/**
 * @brief Skip WS in the JSON context.
 *
 * @param[in] jsonctx JSON parser context.
 */
static void
lyjson_skip_ws(struct lyjson_ctx *jsonctx)
{
    /* skip whitespaces */
    while (is_jsonws(*jsonctx->in->current)) {
        if (*jsonctx->in->current == '\n') {
            LY_IN_NEW_LINE(jsonctx->in);
        }
        ly_in_skip(jsonctx->in, 1);
    }
}

/**
 * @brief Set value in the JSON context.
 *
 * @param[in] jsonctx JSON parser context.
 * @param[in] value Value to set.
 * @param[in] value_len Length of @p value.
 * @param[in] dynamic Whether @p value is dynamically-allocated.
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

/**
 * @brief Parse a JSON string (starting after double quotes) and store it in the context.
 *
 * @param[in] jsonctx JSON parser context.
 * @return LY_ERR value.
 */
static LY_ERR
lyjson_string(struct lyjson_ctx *jsonctx)
{
    const char *in = jsonctx->in->current, *start, *c;
    char *buf = NULL;
    size_t offset;   /* read offset in input buffer */
    size_t len;      /* length of the output string (write offset in output buffer) */
    size_t size = 0; /* size of the output buffer */
    size_t u;
    uint64_t start_line;
    uint32_t value;
    uint8_t i;

    assert(jsonctx);

    /* init */
    start = in;
    start_line = jsonctx->in->line;
    offset = len = 0;

    /* parse */
    while (in[offset]) {
        switch (in[offset]) {
        case '\\':
            /* escape sequence */
            c = &in[offset];
            if (!buf) {
                /* prepare output buffer */
                buf = malloc(LYJSON_STRING_BUF_START);
                LY_CHECK_ERR_RET(!buf, LOGMEM(jsonctx->ctx), LY_EMEM);
                size = LYJSON_STRING_BUF_START;
            }

            /* allocate enough for the offset and next character,
             * we will need 4 bytes at most since we support only the predefined
             * (one-char) entities and character references */
            if (len + offset + 4 >= size) {
                size_t increment;

                for (increment = LYJSON_STRING_BUF_STEP; len + offset + 4 >= size + increment; increment += LYJSON_STRING_BUF_STEP) {}
                buf = ly_realloc(buf, size + increment);
                LY_CHECK_ERR_RET(!buf, LOGMEM(jsonctx->ctx), LY_EMEM);
                size += LYJSON_STRING_BUF_STEP;
            }

            if (offset) {
                /* store what we have so far */
                memcpy(&buf[len], in, offset);
                len += offset;
                in += offset;
                offset = 0;
            }

            i = 1;
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
                        LOGVAL(jsonctx->ctx, LYVE_SYNTAX, "Invalid basic multilingual plane character \"%s\".", c);
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
                    (int)(&in[offset] - c), c, value),
                    error);
            len += u;      /* update number of bytes in buffer */
            in += offset;  /* move the input by the processed bytes stored in the buffer ... */
            offset = 0;    /* ... and reset the offset index for future moving data into buffer */
            break;

        case '"':
            /* end of string */
            if (buf) {
                /* realloc exact size string */
                buf = ly_realloc(buf, len + offset + 1);
                LY_CHECK_ERR_RET(!buf, LOGMEM(jsonctx->ctx), LY_EMEM);
                size = len + offset + 1;
                if (offset) {
                    memcpy(&buf[len], in, offset);
                }

                /* set terminating NULL byte */
                buf[len + offset] = '\0';
            }
            len += offset;
            ++offset;
            in += offset;
            goto success;

        default:
            /* get it as UTF-8 character for check */
            c = &in[offset];
            LY_CHECK_ERR_GOTO(ly_getutf8(&c, &value, &u),
                    LOGVAL(jsonctx->ctx, LY_VCODE_INCHAR, in[offset]), error);

            LY_CHECK_ERR_GOTO(!is_jsonstrchar(value),
                    LOGVAL(jsonctx->ctx, LYVE_SYNTAX, "Invalid character in JSON string \"%.*s\" (0x%08x).",
                    (int)(&in[offset] - start + u), start, value),
                    error);

            /* character is ok, continue */
            offset += u;
            break;
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
}

/**
 * @brief Calculate how many @p c characters there are in a row.
 *
 * @param[in] str Count from this position.
 * @param[in] end Position after the last checked character.
 * @param[in] c Checked character.
 * @param[in] backwards Set to 1, if to proceed from end-1 to str.
 * @return Number of characters in a row.
 */
static uint32_t
lyjson_count_in_row(const char *str, const char *end, char c, ly_bool backwards)
{
    uint32_t cnt;

    assert(str && end);

    if (str >= end) {
        return 0;
    }

    if (!backwards) {
        for (cnt = 0; (str != end) && (*str == c); ++str, ++cnt) {}
    } else {
        --end;
        --str;
        for (cnt = 0; (str != end) && (*end == c); --end, ++cnt) {}
    }

    return cnt;
}

/**
 * @brief Check if the number can be shortened to zero.
 *
 * @param[in] in Start of input string;
 * @param[in] end End of input string;
 * @return 1 if number is zero, otherwise 0.
 */
static ly_bool
lyjson_number_is_zero(const char *in, const char *end)
{
    assert(in < end);

    if ((in[0] == '-') || (in[0] == '+')) {
        in++;
        assert(in < end);
    }
    if ((in[0] == '0') && (in[1] == '.')) {
        in += 2;
        if (!(in < end)) {
            return 1;
        }
    }

    return lyjson_count_in_row(in, end, '0', 0) == end - in;
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
lyjson_get_buffer_for_number(const struct ly_ctx *ctx, uint64_t num_len, char **buffer)
{
    *buffer = NULL;

    LY_CHECK_ERR_RET((num_len + 1) > LY_NUMBER_MAXLEN, LOGVAL(ctx, LYVE_SEMANTICS,
            "Number encoded as a string exceeded the LY_NUMBER_MAXLEN limit."), LY_EVALID);

    /* allocate buffer for the result (add NULL-byte) */
    *buffer = malloc(num_len + 1);
    LY_CHECK_ERR_RET(!(*buffer), LOGMEM(ctx), LY_EMEM);
    return LY_SUCCESS;
}

/**
 * @brief Copy the 'numeric part' (@p num) except its decimal point
 * (@p dec_point) and insert the new decimal point (@p dp_position)
 * only if it is to be placed in the 'numeric part' range (@p num).
 *
 * @param[in] num Begin of the 'numeric part'.
 * @param[in] num_len Length of the 'numeric part'.
 * @param[in] dec_point Pointer to the old decimal point.
 * If it has a NULL value, it is ignored.
 * @param[in] dp_position Position of the new decimal point.
 * If it has a negative value, it is ignored.
 * @param[out] dst Memory into which the copied result is written.
 * @return Number of characters written to the @p dst.
 */
static uint32_t
lyjson_exp_number_copy_num_part(const char *num, uint32_t num_len, char *dec_point, int32_t dp_position, char *dst)
{
    int32_t dec_point_idx;
    int32_t n, d;

    assert(num && dst);

    dec_point_idx = dec_point ? dec_point - num : INT32_MAX;
    assert((dec_point_idx >= 0) && (dec_point_idx != dp_position));

    for (n = 0, d = 0; (uint32_t)n < num_len; n++) {
        if (n == dec_point_idx) {
            continue;
        } else if (d == dp_position) {
            dst[d++] = '.';
            dst[d++] = num[n];
        } else {
            dst[d++] = num[n];
        }
    }

    return d;
}

/**
 * @brief Convert JSON number with exponent into the representation
 * used by YANG.
 *
 * The input numeric string must be syntactically valid. Also, before
 * calling this function, checks should be performed using the
 * ::lyjson_number_is_zero().
 *
 * @param[in] ctx Context for the error message.
 * @param[in] in Beginning of the string containing the number.
 * @param[in] exponent Pointer to the letter E/e.
 * @param[in] total_len Total size of the input number.
 * @param[out] res Conversion result.
 * @param[out] res_len Length of the result.
 * @return LY_ERR value.
 */
static LY_ERR
lyjson_exp_number(const struct ly_ctx *ctx, const char *in, const char *exponent, uint64_t total_len, char **res,
        size_t *res_len)
{

#define MAYBE_WRITE_MINUS(ARRAY, INDEX, FLAG) \
        if (FLAG) { \
            ARRAY[INDEX++] = '-'; \
        }

/* Length of leading zero followed by the decimal point. */
#define LEADING_ZERO 1

/* Flags for the ::lyjson_count_in_row() */
#define FORWARD 0
#define BACKWARD 1

    /* Buffer where the result is stored. */
    char *buf;
    /* Size without space for terminating NULL-byte. */
    uint64_t buf_len;
    /* Index to buf. */
    uint32_t i = 0;
    /* A 'numeric part' doesn't contain a minus sign or an leading zero.
     * For example, in 0.45, there is the leading zero.
     */
    const char *num;
    /* Length of the 'numeric part' ends before E/e. */
    uint16_t num_len;
    /* Position of decimal point in the num. */
    char *dec_point;
    /* Final position of decimal point in the buf. */
    int32_t dp_position;
    /* Exponent as integer. */
    long long e_val;
    /* Byte for the decimal point. */
    int8_t dot;
    /* Required additional byte for the minus sign. */
    uint8_t minus;
    /* The number of zeros. */
    long zeros;
    /* If the number starts with leading zero followed by the decimal point. */
    ly_bool leading_zero;

    assert(ctx && in && exponent && res && res_len && (total_len > 2));
    assert((in < exponent) && ((*exponent == 'e') || (*exponent == 'E')));

    if ((exponent - in) > UINT16_MAX) {
        LOGVAL(ctx, LYVE_SEMANTICS, "JSON number is too long.");
        return LY_EVALID;
    }

    /* Convert exponent. */
    errno = 0;
    e_val = strtoll(exponent + 1, NULL, LY_BASE_DEC);
    if (errno || (e_val > UINT16_MAX) || (e_val < -UINT16_MAX)) {
        LOGVAL(ctx, LYVE_SEMANTICS,
                "Exponent out-of-bounds in a JSON Number value (%.*s).",
                (int)total_len, in);
        return LY_EVALID;
    }

    minus = in[0] == '-';
    if (in[minus] == '0') {
        assert(in[minus + 1] == '.');
        leading_zero = 1;
        /* The leading zero has been found, it will be skipped. */
        num = &in[minus + 1];
    } else {
        leading_zero = 0;
        /* Set to the first number. */
        num = &in[minus];
    }
    num_len = exponent - num;

    /* Find the location of the decimal points. */
    dec_point = ly_strnchr(num, '.', num_len);
    dp_position = dec_point ?
            dec_point - num + e_val :
            num_len + e_val;

    /* Remove zeros after the decimal point from the end of
     * the 'numeric part' because these are useless.
     * (For example, in 40.001000 these are the last 3).
     */
    num_len -= dp_position > 0 ?
            lyjson_count_in_row(num + dp_position - 1, exponent, '0', BACKWARD) :
            lyjson_count_in_row(num, exponent, '0', BACKWARD);

    /* Decide what to do with the dot from the 'numeric part'. */
    if (dec_point && ((int32_t)(num_len - 1) == dp_position)) {
        /* Decimal point in the last place is useless. */
        dot = -1;
    } else if (dec_point) {
        /* Decimal point is shifted. */
        dot = 0;
    } else {
        /* Additional byte for the decimal point is requred. */
        dot = 1;
    }

    /* Final composition of the result. */
    if (dp_position <= 0) {
        /* Adding decimal point before the integer with adding additional zero(s). */

        zeros = labs(dp_position);
        buf_len = minus + LEADING_ZERO + dot + zeros + num_len;
        LY_CHECK_RET(lyjson_get_buffer_for_number(ctx, buf_len, &buf));
        MAYBE_WRITE_MINUS(buf, i, minus);
        buf[i++] = '0';
        buf[i++] = '.';
        memset(buf + i, '0', zeros);
        i += zeros;
        dp_position = -1;
        lyjson_exp_number_copy_num_part(num, num_len, dec_point, dp_position, buf + i);
    } else if (leading_zero && (dp_position < (ssize_t)num_len)) {
        /* Insert decimal point between the integer's digits. */

        /* Set a new range of 'numeric part'. Old decimal point is skipped. */
        num++;
        num_len--;
        dp_position--;
        /* Get the number of useless zeros between the old
         * and new decimal point. For example, in the number 0.005E1,
         * there is one useless zero.
         */
        zeros = lyjson_count_in_row(num, num + dp_position + 1, '0', FORWARD);
        /* If the new decimal point will be in the place of the first non-zero subnumber. */
        if (zeros == (dp_position + 1)) {
            /* keep one zero as leading zero */
            zeros--;
            /* new decimal point will be behind the leading zero */
            dp_position = 1;
            dot = 1;
        } else {
            dot = 0;
        }
        buf_len = minus + dot + (num_len - zeros);
        LY_CHECK_RET(lyjson_get_buffer_for_number(ctx, buf_len, &buf));
        MAYBE_WRITE_MINUS(buf, i, minus);
        /* Skip useless zeros and copy. */
        lyjson_exp_number_copy_num_part(num + zeros, num_len - zeros, NULL, dp_position, buf + i);
    } else if (dp_position < (ssize_t)num_len) {
        /* Insert decimal point between the integer's digits. */

        buf_len = minus + dot + num_len;
        LY_CHECK_RET(lyjson_get_buffer_for_number(ctx, buf_len, &buf));
        MAYBE_WRITE_MINUS(buf, i, minus);
        lyjson_exp_number_copy_num_part(num, num_len, dec_point, dp_position, buf + i);
    } else if (leading_zero) {
        /* Adding decimal point after the decimal value make the integer result. */

        /* Set a new range of 'numeric part'. Old decimal point is skipped. */
        num++;
        num_len--;
        /* Get the number of useless zeros. */
        zeros = lyjson_count_in_row(num, num + num_len, '0', FORWARD);
        buf_len = minus + dp_position - zeros;
        LY_CHECK_RET(lyjson_get_buffer_for_number(ctx, buf_len, &buf));
        MAYBE_WRITE_MINUS(buf, i, minus);
        /* Skip useless zeros and copy. */
        i += lyjson_exp_number_copy_num_part(num + zeros, num_len - zeros, NULL, dp_position, buf + i);
        /* Add multiples of ten behind the 'numeric part'. */
        memset(buf + i, '0', buf_len - i);
    } else {
        /* Adding decimal point after the decimal value make the integer result. */

        buf_len = minus + dp_position;
        LY_CHECK_RET(lyjson_get_buffer_for_number(ctx, buf_len, &buf));
        MAYBE_WRITE_MINUS(buf, i, minus);
        i += lyjson_exp_number_copy_num_part(num, num_len, dec_point, dp_position, buf + i);
        /* Add multiples of ten behind the 'numeric part'. */
        memset(buf + i, '0', buf_len - i);
    }

    buf[buf_len] = '\0';
    *res = buf;
    *res_len = buf_len;

#undef MAYBE_WRITE_MINUS
#undef LEADING_ZERO
#undef FORWARD
#undef BACKWARD

    return LY_SUCCESS;
}

/**
 * @brief Parse a JSON number and store it in the context.
 *
 * @param[in] jsonctx JSON parser context.
 * @return LY_ERR value.
 */
static LY_ERR
lyjson_number(struct lyjson_ctx *jsonctx)
{
    size_t offset = 0, num_len;
    const char *in = jsonctx->in->current, *exponent = NULL;
    uint8_t minus = 0;
    char *num;

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
        exponent = &in[offset];
        ++offset;
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

    if (lyjson_number_is_zero(in, exponent ? exponent : &in[offset])) {
        lyjson_ctx_set_value(jsonctx, in, minus + 1, 0);
    } else if (exponent && lyjson_number_is_zero(exponent + 1, &in[offset])) {
        lyjson_ctx_set_value(jsonctx, in, exponent - in, 0);
    } else if (exponent) {
        LY_CHECK_RET(lyjson_exp_number(jsonctx->ctx, in, exponent, offset, &num, &num_len));
        lyjson_ctx_set_value(jsonctx, num, num_len, 1);
    } else {
        if (offset > LY_NUMBER_MAXLEN) {
            LOGVAL(jsonctx->ctx, LYVE_SEMANTICS,
                    "Number encoded as a string exceeded the LY_NUMBER_MAXLEN limit.");
            return LY_EVALID;
        }
        lyjson_ctx_set_value(jsonctx, in, offset, 0);
    }
    ly_in_skip(jsonctx->in, offset);

    return LY_SUCCESS;
}

LY_ERR
lyjson_ctx_new(const struct ly_ctx *ctx, struct ly_in *in, struct lyjson_ctx **jsonctx_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyjson_ctx *jsonctx;

    assert(ctx && in && jsonctx_p);

    /* new context */
    jsonctx = calloc(1, sizeof *jsonctx);
    LY_CHECK_ERR_RET(!jsonctx, LOGMEM(ctx), LY_EMEM);
    jsonctx->ctx = ctx;
    jsonctx->in = in;

    LOG_LOCSET(NULL, NULL, NULL, in);

    /* WS are always expected to be skipped */
    lyjson_skip_ws(jsonctx);

    if (jsonctx->in->current[0] == '\0') {
        /* empty file, invalid */
        LOGVAL(jsonctx->ctx, LYVE_SYNTAX, "Empty JSON file.");
        ret = LY_EVALID;
        goto cleanup;
    }

    /* start JSON parsing */
    LY_CHECK_GOTO(ret = lyjson_ctx_next(jsonctx, NULL), cleanup);

cleanup:
    if (ret) {
        lyjson_ctx_free(jsonctx);
    } else {
        *jsonctx_p = jsonctx;
    }
    return ret;
}

/**
 * @brief Parse next JSON token, object-name is expected.
 *
 * @param[in] jsonctx JSON parser context.
 * @return LY_ERR value.
 */
static LY_ERR
lyjson_next_object_name(struct lyjson_ctx *jsonctx)
{
    switch (*jsonctx->in->current) {
    case '\0':
        /* EOF */
        LOGVAL(jsonctx->ctx, LY_VCODE_EOF);
        return LY_EVALID;

    case '"':
        /* object name */
        ly_in_skip(jsonctx->in, 1);
        LY_CHECK_RET(lyjson_string(jsonctx));
        lyjson_skip_ws(jsonctx);

        if (*jsonctx->in->current != ':') {
            LOGVAL(jsonctx->ctx, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(jsonctx->in->current), jsonctx->in->current,
                    "a JSON value name-separator ':'");
            return LY_EVALID;
        }
        ly_in_skip(jsonctx->in, 1);
        LYJSON_STATUS_PUSH_RET(jsonctx, LYJSON_OBJECT_NAME);
        break;

    case '}':
        /* object end */
        ly_in_skip(jsonctx->in, 1);
        LYJSON_STATUS_PUSH_RET(jsonctx, LYJSON_OBJECT_CLOSED);
        break;

    default:
        /* unexpected value */
        LOGVAL(jsonctx->ctx, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(jsonctx->in->current),
                jsonctx->in->current, "a JSON object name");
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse next JSON token, value is expected.
 *
 * @param[in] jsonctx JSON parser context.
 * @param[in] array_end Whether array-end is accepted or not.
 * @return LY_ERR value.
 */
static LY_ERR
lyjson_next_value(struct lyjson_ctx *jsonctx, ly_bool array_end)
{
    switch (*jsonctx->in->current) {
    case '\0':
        /* EOF */
        LOGVAL(jsonctx->ctx, LY_VCODE_EOF);
        return LY_EVALID;

    case '"':
        /* string */
        ly_in_skip(jsonctx->in, 1);
        LY_CHECK_RET(lyjson_string(jsonctx));
        LYJSON_STATUS_PUSH_RET(jsonctx, LYJSON_STRING);
        break;

    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        /* number */
        LY_CHECK_RET(lyjson_number(jsonctx));
        LYJSON_STATUS_PUSH_RET(jsonctx, LYJSON_NUMBER);
        break;

    case '{':
        /* object */
        ly_in_skip(jsonctx->in, 1);
        LYJSON_STATUS_PUSH_RET(jsonctx, LYJSON_OBJECT);
        break;

    case '[':
        /* array */
        ly_in_skip(jsonctx->in, 1);
        LYJSON_STATUS_PUSH_RET(jsonctx, LYJSON_ARRAY);
        break;

    case 't':
        if (strncmp(jsonctx->in->current + 1, "rue", ly_strlen_const("rue"))) {
            goto unexpected_value;
        }

        /* true */
        lyjson_ctx_set_value(jsonctx, jsonctx->in->current, ly_strlen_const("true"), 0);
        ly_in_skip(jsonctx->in, ly_strlen_const("true"));
        LYJSON_STATUS_PUSH_RET(jsonctx, LYJSON_TRUE);
        break;

    case 'f':
        if (strncmp(jsonctx->in->current + 1, "alse", ly_strlen_const("alse"))) {
            goto unexpected_value;
        }

        /* false */
        lyjson_ctx_set_value(jsonctx, jsonctx->in->current, ly_strlen_const("false"), 0);
        ly_in_skip(jsonctx->in, ly_strlen_const("false"));
        LYJSON_STATUS_PUSH_RET(jsonctx, LYJSON_FALSE);
        break;

    case 'n':
        if (strncmp(jsonctx->in->current + 1, "ull", ly_strlen_const("ull"))) {
            goto unexpected_value;
        }

        /* null */
        lyjson_ctx_set_value(jsonctx, "", 0, 0);
        ly_in_skip(jsonctx->in, ly_strlen_const("null"));
        LYJSON_STATUS_PUSH_RET(jsonctx, LYJSON_NULL);
        break;

    case ']':
        if (!array_end) {
            goto unexpected_value;
        }

        /* array end */
        ly_in_skip(jsonctx->in, 1);
        LYJSON_STATUS_PUSH_RET(jsonctx, LYJSON_ARRAY_CLOSED);
        break;

    default:
unexpected_value:
        LOGVAL(jsonctx->ctx, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(jsonctx->in->current),
                jsonctx->in->current, "a JSON value");
        return LY_EVALID;
    }

    if (jsonctx->status.count > LY_MAX_BLOCK_DEPTH * 10) {
        LOGERR(jsonctx->ctx, LY_EINVAL, "Maximum number %d of nestings has been exceeded.", LY_MAX_BLOCK_DEPTH * 10);
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse next JSON token, object-next-item is expected.
 *
 * @param[in] jsonctx JSON parser context.
 * @return LY_ERR value.
 */
static LY_ERR
lyjson_next_object_item(struct lyjson_ctx *jsonctx)
{
    switch (*jsonctx->in->current) {
    case '\0':
        /* EOF */
        LOGVAL(jsonctx->ctx, LY_VCODE_EOF);
        return LY_EVALID;

    case '}':
        /* object end */
        ly_in_skip(jsonctx->in, 1);
        LYJSON_STATUS_PUSH_RET(jsonctx, LYJSON_OBJECT_CLOSED);
        break;

    case ',':
        /* next object item */
        ly_in_skip(jsonctx->in, 1);
        LYJSON_STATUS_PUSH_RET(jsonctx, LYJSON_OBJECT_NEXT);
        break;

    default:
        /* unexpected value */
        LOGVAL(jsonctx->ctx, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(jsonctx->in->current),
                jsonctx->in->current, "a JSON object-end or next item");
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse next JSON token, array-next-item is expected.
 *
 * @param[in] jsonctx JSON parser context.
 * @return LY_ERR value.
 */
static LY_ERR
lyjson_next_array_item(struct lyjson_ctx *jsonctx)
{
    switch (*jsonctx->in->current) {
    case '\0':
        /* EOF */
        LOGVAL(jsonctx->ctx, LY_VCODE_EOF);
        return LY_EVALID;

    case ']':
        /* array end */
        ly_in_skip(jsonctx->in, 1);
        LYJSON_STATUS_PUSH_RET(jsonctx, LYJSON_ARRAY_CLOSED);
        break;

    case ',':
        /* next array item */
        ly_in_skip(jsonctx->in, 1);
        LYJSON_STATUS_PUSH_RET(jsonctx, LYJSON_ARRAY_NEXT);
        break;

    default:
        /* unexpected value */
        LOGVAL(jsonctx->ctx, LY_VCODE_INSTREXP, LY_VCODE_INSTREXP_len(jsonctx->in->current),
                jsonctx->in->current, "a JSON array-end or next item");
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

LY_ERR
lyjson_ctx_next(struct lyjson_ctx *jsonctx, enum LYJSON_PARSER_STATUS *status)
{
    LY_ERR ret = LY_SUCCESS;
    enum LYJSON_PARSER_STATUS cur;

    assert(jsonctx);

    cur = lyjson_ctx_status(jsonctx);
    switch (cur) {
    case LYJSON_OBJECT:
        LY_CHECK_GOTO(ret = lyjson_next_object_name(jsonctx), cleanup);
        break;
    case LYJSON_ARRAY:
        LY_CHECK_GOTO(ret = lyjson_next_value(jsonctx, 1), cleanup);
        break;
    case LYJSON_OBJECT_NEXT:
        LYJSON_STATUS_POP(jsonctx);
        LY_CHECK_GOTO(ret = lyjson_next_object_name(jsonctx), cleanup);
        break;
    case LYJSON_ARRAY_NEXT:
        LYJSON_STATUS_POP(jsonctx);
        LY_CHECK_GOTO(ret = lyjson_next_value(jsonctx, 0), cleanup);
        break;
    case LYJSON_OBJECT_NAME:
        lyjson_ctx_set_value(jsonctx, NULL, 0, 0);
        LYJSON_STATUS_POP(jsonctx);
        LY_CHECK_GOTO(ret = lyjson_next_value(jsonctx, 0), cleanup);
        break;
    case LYJSON_OBJECT_CLOSED:
    case LYJSON_ARRAY_CLOSED:
        LYJSON_STATUS_POP(jsonctx);
    /* fallthrough */
    case LYJSON_NUMBER:
    case LYJSON_STRING:
    case LYJSON_TRUE:
    case LYJSON_FALSE:
    case LYJSON_NULL:
        lyjson_ctx_set_value(jsonctx, NULL, 0, 0);
        LYJSON_STATUS_POP(jsonctx);
        cur = lyjson_ctx_status(jsonctx);

        if (cur == LYJSON_OBJECT) {
            LY_CHECK_GOTO(ret = lyjson_next_object_item(jsonctx), cleanup);
            break;
        } else if (cur == LYJSON_ARRAY) {
            LY_CHECK_GOTO(ret = lyjson_next_array_item(jsonctx), cleanup);
            break;
        }

        assert(cur == LYJSON_END);
        goto cleanup;
    case LYJSON_END:
        LY_CHECK_GOTO(ret = lyjson_next_value(jsonctx, 0), cleanup);
        break;
    case LYJSON_ERROR:
        LOGINT(jsonctx->ctx);
        ret = LY_EINT;
        goto cleanup;
    }

    /* skip WS */
    lyjson_skip_ws(jsonctx);

cleanup:
    if (!ret && status) {
        *status = lyjson_ctx_status(jsonctx);
    }
    return ret;
}

void
lyjson_ctx_backup(struct lyjson_ctx *jsonctx)
{
    if (jsonctx->backup.dynamic) {
        free((char *)jsonctx->backup.value);
    }
    jsonctx->backup.status = lyjson_ctx_status(jsonctx);
    jsonctx->backup.status_count = jsonctx->status.count;
    jsonctx->backup.value = jsonctx->value;
    jsonctx->backup.value_len = jsonctx->value_len;
    jsonctx->backup.input = jsonctx->in->current;
    jsonctx->backup.dynamic = jsonctx->dynamic;
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
    jsonctx->backup.dynamic = 0;
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
