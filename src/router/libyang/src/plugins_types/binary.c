/**
 * @file binary.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in binary type plugin.
 *
 * Copyright (c) 2019-2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "plugins_types.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesBinary binary (built-in)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | binary value size | yes | `void *` | value in binary |
 */

/**
 * @brief base64 encode table
 */
static const char b64_etable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * @brief Encode binary value into a base64 string value.
 *
 * Reference https://tools.ietf.org/html/rfc4648#section-4
 *
 * @param[in] ctx libyang context.
 * @param[in] data Binary data value.
 * @param[in] size Size of @p data.
 * @param[out] str Encoded base64 string.
 * @param[out] str_len Length of returned @p str.
 * @return LY_ERR value.
 */
static LY_ERR
binary_base64_encode(const struct ly_ctx *ctx, const char *data, size_t size, char **str, size_t *str_len)
{
    uint32_t i;
    char *ptr;

    *str_len = (size + 2) / 3 * 4;
    *str = malloc(*str_len + 1);
    LY_CHECK_ERR_RET(!*str, LOGMEM(ctx), LY_EMEM);

    ptr = *str;
    for (i = 0; i < size - 2; i += 3) {
        *ptr++ = b64_etable[(data[i] >> 2) & 0x3F];
        *ptr++ = b64_etable[((data[i] & 0x3) << 4) | ((int)(data[i + 1] & 0xF0) >> 4)];
        *ptr++ = b64_etable[((data[i + 1] & 0xF) << 2) | ((int)(data[i + 2] & 0xC0) >> 6)];
        *ptr++ = b64_etable[data[i + 2] & 0x3F];
    }
    if (i < size) {
        *ptr++ = b64_etable[(data[i] >> 2) & 0x3F];
        if (i == (size - 1)) {
            *ptr++ = b64_etable[((data[i] & 0x3) << 4)];
            *ptr++ = '=';
        } else {
            *ptr++ = b64_etable[((data[i] & 0x3) << 4) | ((int)(data[i + 1] & 0xF0) >> 4)];
            *ptr++ = b64_etable[((data[i + 1] & 0xF) << 2)];
        }
        *ptr++ = '=';
    }
    *ptr = '\0';

    return LY_SUCCESS;
}

/**
 * @brief base64 decode table
 */
static const int b64_dtable[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 63, 62, 62, 63, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
    7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,
    0,  0,  0, 63,  0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};

/**
 * @brief Decode the binary value from a base64 string value.
 *
 * Reference https://tools.ietf.org/html/rfc4648#section-4
 *
 * @param[in] value Base64-encoded string value.
 * @param[in] value_len Length of @p value.
 * @param[out] data Decoded binary value.
 * @param[out] size Size of @p data.
 * @return LY_ERR value.
 */
static LY_ERR
binary_base64_decode(const char *value, size_t value_len, void **data, size_t *size)
{
    unsigned char *ptr = (unsigned char *)value;
    uint32_t pad_chars, octet_count;
    char *str;

    if (!value_len || (ptr[value_len - 1] != '=')) {
        pad_chars = 0;
    } else if (ptr[value_len - 2] == '=') {
        pad_chars = 1;
    } else {
        pad_chars = 2;
    }

    octet_count = ((value_len + 3) / 4 - (pad_chars ? 1 : 0)) * 4;
    *size = octet_count / 4 * 3 + pad_chars;

    str = malloc(*size + 1);
    LY_CHECK_RET(!str, LY_EMEM);
    str[*size] = '\0';

    for (uint32_t i = 0, j = 0; i < octet_count; i += 4) {
        int n = b64_dtable[ptr[i]] << 18 | b64_dtable[ptr[i + 1]] << 12 | b64_dtable[ptr[i + 2]] << 6 | b64_dtable[ptr[i + 3]];
        str[j++] = n >> 16;
        str[j++] = n >> 8 & 0xFF;
        str[j++] = n & 0xFF;
    }
    if (pad_chars) {
        int n = b64_dtable[ptr[octet_count]] << 18 | b64_dtable[ptr[octet_count + 1]] << 12;
        str[*size - pad_chars] = n >> 16;

        if (pad_chars == 2) {
            n |= b64_dtable[ptr[octet_count + 2]] << 6;
            n >>= 8 & 0xFF;
            str[*size - pad_chars + 1] = n;
        }
    }

    *data = str;
    return LY_SUCCESS;
}

/**
 * @brief Validate a base64 string.
 *
 * @param[in] value Value to validate.
 * @param[in] value_len Length of @p value.
 * @param[in] type type of the value.
 * @param[out] err Error information.
 * @return LY_ERR value.
 */
static LY_ERR
binary_base64_validate(const char *value, size_t value_len, const struct lysc_type_bin *type, struct ly_err_item **err)
{
    uint32_t idx, pad;

    /* check correct characters in base64 */
    idx = 0;
    while ((idx < value_len) &&
            ((('A' <= value[idx]) && (value[idx] <= 'Z')) ||
            (('a' <= value[idx]) && (value[idx] <= 'z')) ||
            (('0' <= value[idx]) && (value[idx] <= '9')) ||
            ('+' == value[idx]) || ('/' == value[idx]))) {
        idx++;
    }

    /* find end of padding */
    pad = 0;
    while ((idx + pad < value_len) && (pad < 2) && (value[idx + pad] == '=')) {
        pad++;
    }

    /* check if value is valid base64 value */
    if (value_len != idx + pad) {
        if (isprint(value[idx + pad])) {
            return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid Base64 character '%c'.", value[idx + pad]);
        } else {
            return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid Base64 character 0x%x.", value[idx + pad]);
        }
    }

    if (value_len & 3) {
        /* base64 length must be multiple of 4 chars */
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Base64 encoded value length must be divisible by 4.");
    }

    /* length restriction of the binary value */
    if (type->length) {
        const uint32_t octet_count = ((idx + pad) / 4) * 3 - pad;
        LY_CHECK_RET(lyplg_type_validate_range(LY_TYPE_BINARY, type->length, octet_count, value, value_len, err));
    }

    return LY_SUCCESS;
}

API LY_ERR
lyplg_type_store_binary(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_bin *type_bin = (struct lysc_type_bin *)type;
    struct lyd_value_binary *val;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    LYPLG_TYPE_VAL_INLINE_PREPARE(storage, val);
    LY_CHECK_ERR_GOTO(!val, ret = LY_EMEM, cleanup);
    storage->realtype = type;

    if (format == LY_VALUE_LYB) {
        /* store value */
        if (options & LYPLG_TYPE_STORE_DYNAMIC) {
            val->data = (void *)value;
            options &= ~LYPLG_TYPE_STORE_DYNAMIC;
        } else {
            val->data = malloc(value_len);
            LY_CHECK_ERR_GOTO(!val->data, ret = LY_EMEM, cleanup);
            memcpy(val->data, value, value_len);
        }

        /* store size */
        val->size = value_len;

        /* success */
        goto cleanup;
    }

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* validate */
    if (format != LY_VALUE_CANON) {
        ret = binary_base64_validate(value, value_len, type_bin, err);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* get the binary value */
    ret = binary_base64_decode(value, value_len, &val->data, &val->size);
    LY_CHECK_GOTO(ret, cleanup);

    /* store canonical value, it always is */
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
        options &= ~LYPLG_TYPE_STORE_DYNAMIC;
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        ret = lydict_insert(ctx, value_len ? value : "", value_len, &storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);
    }

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_binary(ctx, storage);
    }
    return ret;
}

API LY_ERR
lyplg_type_compare_binary(const struct lyd_value *val1, const struct lyd_value *val2)
{
    struct lyd_value_binary *v1, *v2;

    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }

    LYD_VALUE_GET(val1, v1);
    LYD_VALUE_GET(val2, v2);

    if ((v1->size != v2->size) || memcmp(v1->data, v2->data, v1->size)) {
        return LY_ENOT;
    }
    return LY_SUCCESS;
}

API const void *
lyplg_type_print_binary(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, size_t *value_len)
{
    struct lyd_value_binary *val;
    char *ret;
    size_t ret_len = 0;

    LYD_VALUE_GET(value, val);

    if (format == LY_VALUE_LYB) {
        *dynamic = 0;
        if (value_len) {
            *value_len = val->size;
        }
        return val->data;
    }

    /* generate canonical value if not already */
    if (!value->_canonical) {
        /* get the base64 string value */
        if (binary_base64_encode(ctx, val->data, val->size, &ret, &ret_len)) {
            return NULL;
        }

        /* store it */
        if (lydict_insert_zc(ctx, ret, (const char **)&value->_canonical)) {
            LOGMEM(ctx);
            return NULL;
        }
    }

    /* use the cached canonical value */
    if (dynamic) {
        *dynamic = 0;
    }
    if (value_len) {
        *value_len = ret_len ? ret_len : strlen(value->_canonical);
    }
    return value->_canonical;
}

API LY_ERR
lyplg_type_dup_binary(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ERR ret;
    struct lyd_value_binary *orig_val, *dup_val;

    ret = lydict_insert(ctx, original->_canonical, ly_strlen(original->_canonical), &dup->_canonical);
    LY_CHECK_RET(ret);

    LYPLG_TYPE_VAL_INLINE_PREPARE(dup, dup_val);
    if (!dup_val) {
        lydict_remove(ctx, dup->_canonical);
        return LY_EMEM;
    }

    LYD_VALUE_GET(original, orig_val);
    dup_val->data = malloc(orig_val->size);
    if (!dup_val->data) {
        lydict_remove(ctx, dup->_canonical);
        LYPLG_TYPE_VAL_INLINE_DESTROY(dup_val);
        return LY_EMEM;
    }
    memcpy(dup_val->data, orig_val->data, orig_val->size);
    dup_val->size = orig_val->size;

    dup->realtype = original->realtype;
    return LY_SUCCESS;
}

API void
lyplg_type_free_binary(const struct ly_ctx *ctx, struct lyd_value *value)
{
    struct lyd_value_binary *val;

    lydict_remove(ctx, value->_canonical);
    LYD_VALUE_GET(value, val);
    if (val) {
        free(val->data);
        LYPLG_TYPE_VAL_INLINE_DESTROY(val);
    }
}

/**
 * @brief Plugin information for binray type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_binary[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_BINARY_STR,

        .plugin.id = "libyang 2 - binary, version 1",
        .plugin.store = lyplg_type_store_binary,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_binary,
        .plugin.sort = NULL,
        .plugin.print = lyplg_type_print_binary,
        .plugin.duplicate = lyplg_type_dup_binary,
        .plugin.free = lyplg_type_free_binary,
    },
    {0}
};
