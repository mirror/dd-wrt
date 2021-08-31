/**
 * @file bits.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in bits type plugin.
 *
 * Copyright (c) 2019-2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* asprintf, strdup */
#include <sys/cdefs.h>

#include "plugins_types.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesBits bits (built-in)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | returned by ::lyplg_type_bits_bitmap_size() | yes | pointer to integer type of the specific size, if size more than 8 use `char *` | bitmap of the set bits |
 */

/**
 * @brief Get the position of the last bit.
 */
#define BITS_LAST_BIT_POSITION(type_bits) (type_bits->bits[LY_ARRAY_COUNT(type_bits->bits) - 1].position)

/**
 * @brief Get a specific byte in a bitmap.
 */
#ifdef IS_BIG_ENDIAN
# define BITS_BITMAP_BYTE(bitmap, size, idx) (bitmap + (size - 1) - idx)
#else
# define BITS_BITMAP_BYTE(bitmap, size, idx) (bitmap + idx)
#endif

API size_t
lyplg_type_bits_bitmap_size(const struct lysc_type_bits *type)
{
    size_t needed_bytes, size;

    LY_CHECK_ARG_RET(NULL, type, type->basetype == LY_TYPE_BITS, 0);

    /* minimum needed bytes to hold all the bit positions */
    needed_bytes = (BITS_LAST_BIT_POSITION(type) / 8) + (BITS_LAST_BIT_POSITION(type) % 8 ? 1 : 0);
    LY_CHECK_ERR_RET(!needed_bytes, LOGINT(NULL), 0);

    if ((needed_bytes == 1) || (needed_bytes == 2)) {
        /* uint8_t or uint16_t */
        size = needed_bytes;
    } else if (needed_bytes < 5) {
        /* uint32_t */
        size = 4;
    } else if (needed_bytes < 9) {
        /* uint64_t */
        size = 8;
    } else {
        /* no basic type, do not round */
        size = needed_bytes;
    }

    return size;
}

API ly_bool
lyplg_type_bits_is_bit_set(const char *bitmap, size_t size, uint32_t bit_position)
{
    char bitmask;

    /* find the byte with our bit */
    (void)size;
    bitmap = BITS_BITMAP_BYTE(bitmap, size, bit_position / 8);
    bit_position %= 8;

    /* generate bitmask */
    bitmask = 1;
    bitmask <<= bit_position;

    /* check if bit set */
    if (*bitmap & bitmask) {
        return 1;
    }
    return 0;
}

/**
 * @brief Set bit at a specific position.
 *
 * @param[in,out] bitmap Bitmap to modify.
 * @param[in] size Size of @p bitmap.
 * @param[in] bit_position Bit position to set.
 */
static void
bits_bit_set(char *bitmap, size_t size, uint32_t bit_position)
{
    char bitmask;

    /* find the byte with our bit */
    (void)size;
    bitmap = BITS_BITMAP_BYTE(bitmap, size, bit_position / 8);
    bit_position %= 8;

    /* generate bitmask */
    bitmask = 1;
    bitmask <<= bit_position;

    /* set the bit */
    *bitmap |= bitmask;
}

/**
 * @brief Convert a list of bit names separated by whitespaces to a bitmap.
 *
 * @param[in] value Value to convert.
 * @param[in] value_len Length of @p value.
 * @param[in] type Type of the value.
 * @param[in,out] bitmap Zeroed bitmap, is filled (set).
 * @param[out] err Error information.
 * @return LY_ERR value.
 */
static LY_ERR
bits_str2bitmap(const char *value, size_t value_len, struct lysc_type_bits *type, char *bitmap, struct ly_err_item **err)
{
    size_t idx_start, idx_end;
    LY_ARRAY_COUNT_TYPE u;
    ly_bool found;

    idx_start = idx_end = 0;
    while (idx_end < value_len) {
        /* skip whitespaces */
        while ((idx_end < value_len) && isspace(value[idx_end])) {
            ++idx_end;
        }
        if (idx_end == value_len) {
            break;
        }

        /* parse bit name */
        idx_start = idx_end;
        while ((idx_end < value_len) && !isspace(value[idx_end])) {
            ++idx_end;
        }

        /* find the bit */
        found = 0;
        LY_ARRAY_FOR(type->bits, u) {
            if (!ly_strncmp(type->bits[u].name, value + idx_start, idx_end - idx_start)) {
                found = 1;
                break;
            }
        }

        /* check if name exists */
        if (!found) {
            return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid bit \"%.*s\".", (int)(idx_end - idx_start),
                    value + idx_start);
        }

        /* check for duplication */
        if (lyplg_type_bits_is_bit_set(bitmap, lyplg_type_bits_bitmap_size(type), type->bits[u].position)) {
            return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Duplicate bit \"%s\".", type->bits[u].name);
        }

        /* set the bit */
        bits_bit_set(bitmap, lyplg_type_bits_bitmap_size(type), type->bits[u].position);
    }

    return LY_SUCCESS;
}

/**
 * @brief Add a bit item into an array.
 *
 * @param[in] position Bit position to add.
 * @param[in] type Bitis type to read the bit positions and names from.
 * @param[in,out] items Array of bit item pointers to add to.
 */
static void
bits_add_item(uint32_t position, struct lysc_type_bits *type, struct lysc_type_bitenum_item **items)
{
    LY_ARRAY_COUNT_TYPE u;

    /* find the bit item */
    LY_ARRAY_FOR(type->bits, u) {
        if (type->bits[u].position == position) {
            break;
        }
    }

    /* add it at the end */
    items[LY_ARRAY_COUNT(items)] = &type->bits[u];
    LY_ARRAY_INCREMENT(items);
}

/**
 * @brief Convert a bitmap to a sized array of pointers to their bit definitions.
 *
 * @param[in] bitmap Bitmap to read from.
 * @param[in] type Bits type.
 * @param[in,out] items Allocated sized array to fill with the set bits.
 */
static void
bits_bitmap2items(const char *bitmap, struct lysc_type_bits *type, struct lysc_type_bitenum_item **items)
{
    size_t i, bitmap_size = lyplg_type_bits_bitmap_size(type);
    uint32_t bit_pos;
    char bitmask;
    const char *byte;

    bit_pos = 0;
    for (i = 0; i < bitmap_size; ++i) {
        /* check this byte (but not necessarily all bits in the last byte) */
        byte = BITS_BITMAP_BYTE(bitmap, bitmap_size, i);
        for (bitmask = 1; bitmask; bitmask <<= 1) {
            if (*byte & bitmask) {
                /* add this bit */
                bits_add_item(bit_pos, type, items);
            }

            if (bit_pos == BITS_LAST_BIT_POSITION(type)) {
                /* we have checked the last valid bit */
                break;
            }

            ++bit_pos;
        }
    }
}

/**
 * @brief Generate canonical value from ordered array of set bit items.
 *
 * @param[in] items Sized array of set bit items.
 * @param[out] canonical Canonical string value.
 * @return LY_ERR value.
 */
static LY_ERR
bits_items2canon(struct lysc_type_bitenum_item **items, char **canonical)
{
    char *ret;
    size_t ret_len;
    LY_ARRAY_COUNT_TYPE u;

    *canonical = NULL;

    /* init value */
    ret = strdup("");
    LY_CHECK_RET(!ret, LY_EMEM);
    ret_len = 0;

    LY_ARRAY_FOR(items, u) {
        if (!ret_len) {
            ret = ly_realloc(ret, strlen(items[u]->name) + 1);
            LY_CHECK_RET(!ret, LY_EMEM);
            strcpy(ret, items[u]->name);

            ret_len = strlen(ret);
        } else {
            ret = ly_realloc(ret, ret_len + 1 + strlen(items[u]->name) + 1);
            LY_CHECK_RET(!ret, LY_EMEM);
            sprintf(ret + ret_len, " %s", items[u]->name);

            ret_len += 1 + strlen(items[u]->name);
        }
    }

    *canonical = ret;
    return LY_SUCCESS;
}

API LY_ERR
lyplg_type_store_bits(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_bits *type_bits = (struct lysc_type_bits *)type;
    struct lyd_value_bits *val;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    LYPLG_TYPE_VAL_INLINE_PREPARE(storage, val);
    LY_CHECK_ERR_GOTO(!val, ret = LY_EMEM, cleanup);
    storage->realtype = type;

    if (format == LY_VALUE_LYB) {
        /* validation */
        if (value_len != lyplg_type_bits_bitmap_size(type_bits)) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB bits value size %zu (expected %zu).",
                    value_len, lyplg_type_bits_bitmap_size(type_bits));
            goto cleanup;
        }

        /* store value (bitmap) */
        if (options & LYPLG_TYPE_STORE_DYNAMIC) {
            val->bitmap = (char *)value;
            options &= ~LYPLG_TYPE_STORE_DYNAMIC;
        } else {
            val->bitmap = malloc(value_len);
            LY_CHECK_ERR_GOTO(!val->bitmap, ret = LY_EMEM, cleanup);
            memcpy(val->bitmap, value, value_len);
        }

        /* allocate and fill the bit item array */
        LY_ARRAY_CREATE_GOTO(ctx, val->items, LY_ARRAY_COUNT(type_bits->bits), ret, cleanup);
        bits_bitmap2items(val->bitmap, type_bits, val->items);

        /* success */
        goto cleanup;
    }

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* allocate the bitmap */
    val->bitmap = malloc(lyplg_type_bits_bitmap_size(type_bits));
    LY_CHECK_ERR_GOTO(!val->bitmap, ret = LY_EMEM, cleanup);
    memset(val->bitmap, 0, lyplg_type_bits_bitmap_size(type_bits));

    /* fill the bitmap */
    ret = bits_str2bitmap(value, value_len, type_bits, val->bitmap, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* allocate and fill the bit item array */
    LY_ARRAY_CREATE_GOTO(ctx, val->items, LY_ARRAY_COUNT(type_bits->bits), ret, cleanup);
    bits_bitmap2items(val->bitmap, type_bits, val->items);

    if (format == LY_VALUE_CANON) {
        /* store canonical value */
        if (options & LYPLG_TYPE_STORE_DYNAMIC) {
            ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
            options &= ~LYPLG_TYPE_STORE_DYNAMIC;
            LY_CHECK_GOTO(ret, cleanup);
        } else {
            ret = lydict_insert(ctx, value_len ? value : "", value_len, &storage->_canonical);
            LY_CHECK_GOTO(ret != LY_SUCCESS, cleanup);
        }
    }

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_bits(ctx, storage);
    }
    return ret;
}

API LY_ERR
lyplg_type_compare_bits(const struct lyd_value *val1, const struct lyd_value *val2)
{
    struct lyd_value_bits *v1, *v2;
    struct lysc_type_bits *type_bits = (struct lysc_type_bits *)val1->realtype;

    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }

    LYD_VALUE_GET(val1, v1);
    LYD_VALUE_GET(val2, v2);

    if (memcmp(v1->bitmap, v2->bitmap, lyplg_type_bits_bitmap_size(type_bits))) {
        return LY_ENOT;
    }
    return LY_SUCCESS;
}

API const void *
lyplg_type_print_bits(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, size_t *value_len)
{
    struct lysc_type_bits *type_bits = (struct lysc_type_bits *)value->realtype;
    struct lyd_value_bits *val;
    char *ret;

    LYD_VALUE_GET(value, val);

    if (format == LY_VALUE_LYB) {
        *dynamic = 0;
        if (value_len) {
            *value_len = lyplg_type_bits_bitmap_size(type_bits);
        }
        return val->bitmap;
    }

    /* generate canonical value if not already */
    if (!value->_canonical) {
        /* get the canonical value */
        if (bits_items2canon(val->items, &ret)) {
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
        *value_len = strlen(value->_canonical);
    }
    return value->_canonical;
}

API LY_ERR
lyplg_type_dup_bits(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ERR ret;
    struct lysc_type_bits *type_bits = (struct lysc_type_bits *)original->realtype;
    LY_ARRAY_COUNT_TYPE u;
    struct lyd_value_bits *orig_val, *dup_val;

    memset(dup, 0, sizeof *dup);

    /* optional canonical value */
    ret = lydict_insert(ctx, original->_canonical, ly_strlen(original->_canonical), &dup->_canonical);
    LY_CHECK_GOTO(ret, error);

    /* allocate value */
    LYPLG_TYPE_VAL_INLINE_PREPARE(dup, dup_val);
    LY_CHECK_ERR_GOTO(!dup_val, ret = LY_EMEM, error);

    LYD_VALUE_GET(original, orig_val);

    /* duplicate bitmap */
    dup_val->bitmap = malloc(lyplg_type_bits_bitmap_size(type_bits));
    LY_CHECK_ERR_GOTO(!dup_val->bitmap, ret = LY_EMEM, error);
    memcpy(dup_val->bitmap, orig_val->bitmap, lyplg_type_bits_bitmap_size(type_bits));

    /* duplicate bit item pointers */
    LY_ARRAY_CREATE_GOTO(ctx, dup_val->items, LY_ARRAY_COUNT(orig_val->items), ret, error);
    LY_ARRAY_FOR(orig_val->items, u) {
        LY_ARRAY_INCREMENT(dup_val->items);
        dup_val->items[u] = orig_val->items[u];
    }

    dup->realtype = original->realtype;
    return LY_SUCCESS;

error:
    lyplg_type_free_bits(ctx, dup);
    return ret;
}

API void
lyplg_type_free_bits(const struct ly_ctx *ctx, struct lyd_value *value)
{
    struct lyd_value_bits *val;

    lydict_remove(ctx, value->_canonical);
    LYD_VALUE_GET(value, val);
    if (val) {
        free(val->bitmap);
        LY_ARRAY_FREE(val->items);
        LYPLG_TYPE_VAL_INLINE_DESTROY(val);
    }
}

/**
 * @brief Plugin information for bits type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_bits[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_BITS_STR,

        .plugin.id = "libyang 2 - bits, version 1",
        .plugin.store = lyplg_type_store_bits,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_bits,
        .plugin.sort = NULL,
        .plugin.print = lyplg_type_print_bits,
        .plugin.duplicate = lyplg_type_dup_bits,
        .plugin.free = lyplg_type_free_bits
    },
    {0}
};
