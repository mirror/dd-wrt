/**
 * @file printer_lyb.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief LYB printer for libyang data structure
 *
 * Copyright (c) 2020 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "lyb.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "hash_table.h"
#include "log.h"
#include "out.h"
#include "out_internal.h"
#include "plugins_exts/metadata.h"
#include "printer_data.h"
#include "printer_internal.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xml.h"

static LY_ERR lyb_print_siblings(struct ly_out *out, const struct lyd_node *node, struct lyd_lyb_ctx *lybctx);

/**
 * @brief Hash table equal callback for checking hash equality only.
 *
 * Implementation of ::lyht_value_equal_cb.
 */
static ly_bool
lyb_hash_equal_cb(void *UNUSED(val1_p), void *UNUSED(val2_p), ly_bool UNUSED(mod), void *UNUSED(cb_data))
{
    /* for this purpose, if hash matches, the value does also, we do not want 2 values to have the same hash */
    return 1;
}

/**
 * @brief Hash table equal callback for checking value pointer equality only.
 *
 * Implementation of ::lyht_value_equal_cb.
 */
static ly_bool
lyb_ptr_equal_cb(void *val1_p, void *val2_p, ly_bool UNUSED(mod), void *UNUSED(cb_data))
{
    struct lysc_node *val1 = *(struct lysc_node **)val1_p;
    struct lysc_node *val2 = *(struct lysc_node **)val2_p;

    if (val1 == val2) {
        return 1;
    }
    return 0;
}

/**
 * @brief Check that sibling collision hash is safe to insert into hash table.
 *
 * @param[in] ht Hash table.
 * @param[in] sibling Hashed sibling.
 * @param[in] ht_col_id Sibling hash collision ID.
 * @param[in] compare_col_id Last collision ID to compare with.
 * @return LY_SUCCESS when the whole hash sequence does not collide,
 * @return LY_EEXIST when the whole hash sequence sollides.
 */
static LY_ERR
lyb_hash_sequence_check(struct ly_ht *ht, struct lysc_node *sibling, LYB_HASH ht_col_id, LYB_HASH compare_col_id)
{
    struct lysc_node **col_node;

    /* get the first node inserted with last hash col ID ht_col_id */
    if (lyht_find(ht, &sibling, lyb_get_hash(sibling, ht_col_id), (void **)&col_node)) {
        /* there is none. valid situation */
        return LY_SUCCESS;
    }

    lyht_set_cb(ht, lyb_ptr_equal_cb);
    do {
        int64_t j;

        for (j = (int64_t)compare_col_id; j > -1; --j) {
            if (lyb_get_hash(sibling, j) != lyb_get_hash(*col_node, j)) {
                /* one non-colliding hash */
                break;
            }
        }
        if (j == -1) {
            /* all whole hash sequences of nodes inserted with last hash col ID compare_col_id collide */
            lyht_set_cb(ht, lyb_hash_equal_cb);
            return LY_EEXIST;
        }

        /* get next node inserted with last hash col ID ht_col_id */
    } while (!lyht_find_next_with_collision_cb(ht, col_node, lyb_get_hash(*col_node, ht_col_id), lyb_hash_equal_cb,
            (void **)&col_node));

    lyht_set_cb(ht, lyb_hash_equal_cb);
    return LY_SUCCESS;
}

/**
 * @brief Hash all the siblings and add them also into a separate hash table.
 *
 * @param[in] sibling Any sibling in all the siblings on one level.
 * @param[out] ht_p Created hash table.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_hash_siblings(struct lysc_node *sibling, struct ly_ht **ht_p)
{
    struct ly_ht *ht;
    const struct lysc_node *parent;
    const struct lys_module *mod;
    LYB_HASH i;
    uint32_t getnext_opts;

    ht = lyht_new(1, sizeof(struct lysc_node *), lyb_hash_equal_cb, NULL, 1);
    LY_CHECK_ERR_RET(!ht, LOGMEM(sibling->module->ctx), LY_EMEM);

    getnext_opts = 0;
    if (sibling->flags & LYS_IS_OUTPUT) {
        getnext_opts = LYS_GETNEXT_OUTPUT;
    }

    parent = lysc_data_parent(sibling);
    mod = sibling->module;

    sibling = NULL;
    while ((sibling = (struct lysc_node *)lys_getnext(sibling, parent, mod->compiled, getnext_opts))) {
        /* find the first non-colliding hash (or specifically non-colliding hash sequence) */
        for (i = 0; i < LYB_HASH_BITS; ++i) {
            /* check that we are not colliding with nodes inserted with a lower collision ID than ours */
            int64_t j;

            for (j = (int64_t)i - 1; j > -1; --j) {
                if (lyb_hash_sequence_check(ht, sibling, (LYB_HASH)j, i)) {
                    break;
                }
            }
            if (j > -1) {
                /* some check failed, we must use a higher collision ID */
                continue;
            }

            /* try to insert node with the current collision ID */
            if (!lyht_insert_with_resize_cb(ht, &sibling, lyb_get_hash(sibling, i), lyb_ptr_equal_cb, NULL)) {
                /* success, no collision */
                break;
            }

            /* make sure we really cannot insert it with this hash col ID (meaning the whole hash sequence is colliding) */
            if (i && !lyb_hash_sequence_check(ht, sibling, i, i)) {
                /* it can be inserted after all, even though there is already a node with the same last collision ID */
                lyht_set_cb(ht, lyb_ptr_equal_cb);
                if (lyht_insert(ht, &sibling, lyb_get_hash(sibling, i), NULL)) {
                    LOGINT(sibling->module->ctx);
                    lyht_set_cb(ht, lyb_hash_equal_cb);
                    lyht_free(ht, NULL);
                    return LY_EINT;
                }
                lyht_set_cb(ht, lyb_hash_equal_cb);
                break;
            }
            /* there is still another colliding schema node with the same hash sequence, try higher collision ID */
        }

        if (i == LYB_HASH_BITS) {
            /* wow */
            LOGINT(sibling->module->ctx);
            lyht_free(ht, NULL);
            return LY_EINT;
        }
    }

    /* change val equal callback so that the HT is usable for finding value hashes */
    lyht_set_cb(ht, lyb_ptr_equal_cb);

    *ht_p = ht;
    return LY_SUCCESS;
}

/**
 * @brief Find node hash in a hash table.
 *
 * @param[in] ht Hash table to search in.
 * @param[in] node Node to find.
 * @param[out] hash_p First non-colliding hash found.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_hash_find(struct ly_ht *ht, struct lysc_node *node, LYB_HASH *hash_p)
{
    LYB_HASH hash;
    uint32_t i;

    for (i = 0; i < LYB_HASH_BITS; ++i) {
        hash = lyb_get_hash(node, i);
        if (!hash) {
            LOGINT_RET(node->module->ctx);
        }

        if (!lyht_find(ht, &node, hash, NULL)) {
            /* success, no collision */
            break;
        }
    }
    /* cannot happen, we already calculated the hash */
    if (i == LYB_HASH_BITS) {
        LOGINT_RET(node->module->ctx);
    }

    *hash_p = hash;
    return LY_SUCCESS;
}

/**
 * @brief Write metadata about siblings.
 *
 * @param[in] out Out structure.
 * @param[in] sib Contains metadata that is written.
 */
static LY_ERR
lyb_write_sibling_meta(struct ly_out *out, struct lyd_lyb_sibling *sib)
{
    uint8_t meta_buf[LYB_META_BYTES];
    uint64_t num = 0;

    /* write the meta chunk information */
    num = htole64((uint64_t)sib->written & LYB_SIZE_MAX);
    memcpy(meta_buf, &num, LYB_SIZE_BYTES);
    num = htole64((uint64_t)sib->inner_chunks & LYB_INCHUNK_MAX);
    memcpy(meta_buf + LYB_SIZE_BYTES, &num, LYB_INCHUNK_BYTES);

    LY_CHECK_RET(ly_write_skipped(out, sib->position, (char *)&meta_buf, LYB_META_BYTES));

    return LY_SUCCESS;
}

/**
 * @brief Write LYB data fully handling the metadata.
 *
 * @param[in] out Out structure.
 * @param[in] buf Source buffer.
 * @param[in] count Number of bytes to write.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_write(struct ly_out *out, const uint8_t *buf, size_t count, struct lylyb_ctx *lybctx)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lyd_lyb_sibling *full, *iter;
    size_t to_write;

    while (1) {
        /* check for full data chunks */
        to_write = count;
        full = NULL;
        LY_ARRAY_FOR(lybctx->siblings, u) {
            /* we want the innermost chunks resolved first, so replace previous full chunks */
            if (lybctx->siblings[u].written + to_write >= LYB_SIZE_MAX) {
                /* full chunk, do not write more than allowed */
                to_write = LYB_SIZE_MAX - lybctx->siblings[u].written;
                full = &lybctx->siblings[u];
            }
        }

        if (!full && !count) {
            break;
        }

        /* we are actually writing some data, not just finishing another chunk */
        if (to_write) {
            LY_CHECK_RET(ly_write_(out, (char *)buf, to_write));

            LY_ARRAY_FOR(lybctx->siblings, u) {
                /* increase all written counters */
                lybctx->siblings[u].written += to_write;
                assert(lybctx->siblings[u].written <= LYB_SIZE_MAX);
            }
            /* decrease count/buf */
            count -= to_write;
            buf += to_write;
        }

        if (full) {
            /* write the meta information (inner chunk count and chunk size) */
            LY_CHECK_RET(lyb_write_sibling_meta(out, full));

            /* zero written and inner chunks */
            full->written = 0;
            full->inner_chunks = 0;

            /* skip space for another chunk size */
            LY_CHECK_RET(ly_write_skip(out, LYB_META_BYTES, &full->position));

            /* increase inner chunk count */
            for (iter = &lybctx->siblings[0]; iter != full; ++iter) {
                if (iter->inner_chunks == LYB_INCHUNK_MAX) {
                    LOGINT(lybctx->ctx);
                    return LY_EINT;
                }
                ++iter->inner_chunks;
            }
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Stop the current "siblings" - write its final metadata.
 *
 * @param[in] out Out structure.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_write_stop_siblings(struct ly_out *out, struct lylyb_ctx *lybctx)
{
    /* write the meta chunk information */
    lyb_write_sibling_meta(out, &LYB_LAST_SIBLING(lybctx));

    LY_ARRAY_DECREMENT(lybctx->siblings);
    return LY_SUCCESS;
}

/**
 * @brief Start a new "siblings" - skip bytes for its metadata.
 *
 * @param[in] out Out structure.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_write_start_siblings(struct ly_out *out, struct lylyb_ctx *lybctx)
{
    LY_ARRAY_COUNT_TYPE u;

    u = LY_ARRAY_COUNT(lybctx->siblings);
    if (u == lybctx->sibling_size) {
        LY_ARRAY_CREATE_RET(lybctx->ctx, lybctx->siblings, u + LYB_SIBLING_STEP, LY_EMEM);
        lybctx->sibling_size = u + LYB_SIBLING_STEP;
    }

    LY_ARRAY_INCREMENT(lybctx->siblings);
    LYB_LAST_SIBLING(lybctx).written = 0;
    LYB_LAST_SIBLING(lybctx).inner_chunks = 0;

    /* another inner chunk */
    for (u = 0; u < LY_ARRAY_COUNT(lybctx->siblings) - 1; ++u) {
        if (lybctx->siblings[u].inner_chunks == LYB_INCHUNK_MAX) {
            LOGINT(lybctx->ctx);
            return LY_EINT;
        }
        ++lybctx->siblings[u].inner_chunks;
    }

    LY_CHECK_RET(ly_write_skip(out, LYB_META_BYTES, &LYB_LAST_SIBLING(lybctx).position));

    return LY_SUCCESS;
}

/**
 * @brief Write a number.
 *
 * @param[in] num Number to write.
 * @param[in] bytes Actual accessible bytes of @p num.
 * @param[in] out Out structure.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_write_number(uint64_t num, size_t bytes, struct ly_out *out, struct lylyb_ctx *lybctx)
{
    /* correct byte order */
    num = htole64(num);

    return lyb_write(out, (uint8_t *)&num, bytes, lybctx);
}

/**
 * @brief Write a string.
 *
 * @param[in] str String to write.
 * @param[in] str_len Length of @p str.
 * @param[in] len_size Size of @p str_len in bytes.
 * @param[in] out Out structure.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_write_string(const char *str, size_t str_len, uint8_t len_size, struct ly_out *out, struct lylyb_ctx *lybctx)
{
    ly_bool error;

    if (!str) {
        str = "";
        LY_CHECK_ERR_RET(str_len, LOGINT(lybctx->ctx), LY_EINT);
    }

    if (!str_len) {
        str_len = strlen(str);
    }

    switch (len_size) {
    case sizeof(uint8_t):
        error = str_len > UINT8_MAX;
        break;
    case sizeof(uint16_t):
        error = str_len > UINT16_MAX;
        break;
    case sizeof(uint32_t):
        error = str_len > UINT32_MAX;
        break;
    case sizeof(uint64_t):
        error = str_len > UINT64_MAX;
        break;
    default:
        error = 1;
    }
    if (error) {
        LOGINT(lybctx->ctx);
        return LY_EINT;
    }

    LY_CHECK_RET(lyb_write_number(str_len, len_size, out, lybctx));

    LY_CHECK_RET(lyb_write(out, (const uint8_t *)str, str_len, lybctx));

    return LY_SUCCESS;
}

/**
 * @brief Print YANG module info.
 *
 * @param[in] out Out structure.
 * @param[in] mod Module to print.
 * @param[in] with_features Whether to also print enabled features or not.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_model(struct ly_out *out, const struct lys_module *mod, ly_bool with_features, struct lylyb_ctx *lybctx)
{
    LY_ERR rc = LY_SUCCESS;
    uint16_t revision;
    struct ly_set feat_set = {0};
    struct lysp_feature *f = NULL;
    uint32_t i = 0;
    int r;

    /* model name length and model name */
    LY_CHECK_GOTO(rc = lyb_write_string(mod->name, 0, sizeof(uint16_t), out, lybctx), cleanup);

    /* model revision as XXXX XXXX XXXX XXXX (2B) (year is offset from 2000)
     *                   YYYY YYYM MMMD DDDD */
    revision = 0;
    if (mod->revision) {
        r = atoi(mod->revision);
        r -= LYB_REV_YEAR_OFFSET;
        r <<= LYB_REV_YEAR_SHIFT;

        revision |= r;

        r = atoi(mod->revision + ly_strlen_const("YYYY-"));
        r <<= LYB_REV_MONTH_SHIFT;

        revision |= r;

        r = atoi(mod->revision + ly_strlen_const("YYYY-MM-"));

        revision |= r;
    }
    LY_CHECK_GOTO(rc = lyb_write_number(revision, sizeof revision, out, lybctx), cleanup);

    if (with_features) {
        /* collect enabled module features */
        while ((f = lysp_feature_next(f, mod->parsed, &i))) {
            if (f->flags & LYS_FENABLED) {
                LY_CHECK_GOTO(rc = ly_set_add(&feat_set, f, 1, NULL), cleanup);
            }
        }

        /* print enabled feature count and their names */
        LY_CHECK_GOTO(rc = lyb_write_number(feat_set.count, sizeof(uint16_t), out, lybctx), cleanup);
        for (i = 0; i < feat_set.count; ++i) {
            f = feat_set.objs[i];
            LY_CHECK_GOTO(rc = lyb_write_string(f->name, 0, sizeof(uint16_t), out, lybctx), cleanup);
        }
    }

    /* fill cached hashes, if not already */
    lyb_cache_module_hash(mod);

cleanup:
    ly_set_erase(&feat_set, NULL);
    return rc;
}

/**
 * @brief Print all used YANG modules.
 *
 * @param[in] out Out structure.
 * @param[in] root Data root.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_data_models(struct ly_out *out, const struct lyd_node *root, struct lylyb_ctx *lybctx)
{
    struct ly_set *set;
    LY_ARRAY_COUNT_TYPE u;
    LY_ERR ret = LY_SUCCESS;
    struct lys_module *mod;
    const struct lyd_node *elem, *node;
    uint32_t i;

    LY_CHECK_RET(ly_set_new(&set));

    /* collect all data node modules */
    LY_LIST_FOR(root, elem) {
        LYD_TREE_DFS_BEGIN(elem, node) {
            if (node->schema) {
                mod = node->schema->module;
                ret = ly_set_add(set, mod, 0, NULL);
                LY_CHECK_GOTO(ret, cleanup);

                /* add also their modules deviating or augmenting them */
                LY_ARRAY_FOR(mod->deviated_by, u) {
                    ret = ly_set_add(set, mod->deviated_by[u], 0, NULL);
                    LY_CHECK_GOTO(ret, cleanup);
                }
                LY_ARRAY_FOR(mod->augmented_by, u) {
                    ret = ly_set_add(set, mod->augmented_by[u], 0, NULL);
                    LY_CHECK_GOTO(ret, cleanup);
                }

                /* only top-level nodes are processed */
                LYD_TREE_DFS_continue = 1;
            }

            LYD_TREE_DFS_END(elem, node);
        }
    }

    /* now write module count on 2 bytes */
    LY_CHECK_GOTO(ret = lyb_write_number(set->count, 2, out, lybctx), cleanup);

    /* and all the used models */
    for (i = 0; i < set->count; ++i) {
        LY_CHECK_GOTO(ret = lyb_print_model(out, set->objs[i], 1, lybctx), cleanup);
    }

cleanup:
    ly_set_free(set, NULL);
    return ret;
}

/**
 * @brief Print LYB magic number.
 *
 * @param[in] out Out structure.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_magic_number(struct ly_out *out)
{
    /* 'l', 'y', 'b' - 0x6c7962 */
    char magic_number[] = {'l', 'y', 'b'};

    LY_CHECK_RET(ly_write_(out, magic_number, 3));

    return LY_SUCCESS;
}

/**
 * @brief Print LYB header.
 *
 * @param[in] out Out structure.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_header(struct ly_out *out)
{
    uint8_t byte = 0;

    /* version, future flags */
    byte |= LYB_VERSION_NUM;

    LY_CHECK_RET(ly_write_(out, (char *)&byte, 1));

    return LY_SUCCESS;
}

/**
 * @brief Print prefix data.
 *
 * @param[in] out Out structure.
 * @param[in] format Value prefix format.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_prefix_data(struct ly_out *out, LY_VALUE_FORMAT format, const void *prefix_data, struct lylyb_ctx *lybctx)
{
    const struct ly_set *set;
    const struct lyxml_ns *ns;
    uint32_t i;

    switch (format) {
    case LY_VALUE_XML:
        set = prefix_data;
        if (!set) {
            /* no prefix data */
            i = 0;
            LY_CHECK_RET(lyb_write(out, (uint8_t *)&i, 1, lybctx));
            break;
        }
        if (set->count > UINT8_MAX) {
            LOGERR(lybctx->ctx, LY_EINT, "Maximum supported number of prefixes is %u.", UINT8_MAX);
            return LY_EINT;
        }

        /* write number of prefixes on 1 byte */
        LY_CHECK_RET(lyb_write_number(set->count, 1, out, lybctx));

        /* write all the prefixes */
        for (i = 0; i < set->count; ++i) {
            ns = set->objs[i];

            /* prefix */
            LY_CHECK_RET(lyb_write_string(ns->prefix, 0, sizeof(uint16_t), out, lybctx));

            /* namespace */
            LY_CHECK_RET(lyb_write_string(ns->uri, 0, sizeof(uint16_t), out, lybctx));
        }
        break;
    case LY_VALUE_JSON:
    case LY_VALUE_LYB:
        /* nothing to print */
        break;
    default:
        LOGINT_RET(lybctx->ctx);
    }

    return LY_SUCCESS;
}

/**
 * @brief Print term node.
 *
 * @param[in] term Node to print.
 * @param[in] out Out structure.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_term_value(struct lyd_node_term *term, struct ly_out *out, struct lylyb_ctx *lybctx)
{
    LY_ERR ret = LY_SUCCESS;
    ly_bool dynamic = 0;
    void *value;
    size_t value_len = 0;
    int32_t lyb_data_len;
    lyplg_type_print_clb print;

    assert(term->value.realtype && term->value.realtype->plugin && term->value.realtype->plugin->print &&
            term->schema);

    /* Get length of LYB data to print. */
    lyb_data_len = term->value.realtype->plugin->lyb_data_len;

    /* Get value and also print its length only if size is not fixed. */
    print = term->value.realtype->plugin->print;
    if (lyb_data_len < 0) {
        /* Variable-length data. */

        /* Get value and its length from plugin. */
        value = (void *)print(term->schema->module->ctx, &term->value,
                LY_VALUE_LYB, NULL, &dynamic, &value_len);
        LY_CHECK_GOTO(ret, cleanup);

        if (value_len > UINT32_MAX) {
            LOGERR(lybctx->ctx, LY_EINT, "The maximum length of the LYB data "
                    "from a term node must not exceed %" PRIu32 ".", UINT32_MAX);
            ret = LY_EINT;
            goto cleanup;
        }

        /* Print the length of the data as 64-bit unsigned integer. */
        ret = lyb_write_number(value_len, sizeof(uint64_t), out, lybctx);
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        /* Fixed-length data. */

        /* Get value from plugin. */
        value = (void *)print(term->schema->module->ctx, &term->value,
                LY_VALUE_LYB, NULL, &dynamic, NULL);
        LY_CHECK_GOTO(ret, cleanup);

        /* Copy the length from the compiled node. */
        value_len = lyb_data_len;
    }

    /* Print value. */
    if (value_len > 0) {
        /* Print the value simply as it is. */
        ret = lyb_write(out, value, value_len, lybctx);
        LY_CHECK_GOTO(ret, cleanup);
    }

cleanup:
    if (dynamic) {
        free(value);
    }

    return ret;
}

/**
 * @brief Print YANG node metadata.
 *
 * @param[in] out Out structure.
 * @param[in] node Data node whose metadata to print.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_metadata(struct ly_out *out, const struct lyd_node *node, struct lyd_lyb_ctx *lybctx)
{
    uint8_t count = 0;
    const struct lys_module *wd_mod = NULL;
    struct lyd_meta *iter;

    /* with-defaults */
    if (node->schema->nodetype & LYD_NODE_TERM) {
        if (((node->flags & LYD_DEFAULT) && (lybctx->print_options & (LYD_PRINT_WD_ALL_TAG | LYD_PRINT_WD_IMPL_TAG))) ||
                ((lybctx->print_options & LYD_PRINT_WD_ALL_TAG) && lyd_is_default(node))) {
            /* we have implicit OR explicit default node, print attribute only if context include with-defaults schema */
            wd_mod = ly_ctx_get_module_latest(node->schema->module->ctx, "ietf-netconf-with-defaults");
        }
    }

    /* count metadata */
    if (wd_mod) {
        ++count;
    }
    for (iter = node->meta; iter; iter = iter->next) {
        if (count == UINT8_MAX) {
            LOGERR(lybctx->lybctx->ctx, LY_EINT, "Maximum supported number of data node metadata is %u.", UINT8_MAX);
            return LY_EINT;
        }
        ++count;
    }

    /* write number of metadata on 1 byte */
    LY_CHECK_RET(lyb_write(out, &count, 1, lybctx->lybctx));

    if (wd_mod) {
        /* write the "default" metadata */
        LY_CHECK_RET(lyb_print_model(out, wd_mod, 0, lybctx->lybctx));
        LY_CHECK_RET(lyb_write_string("default", 0, sizeof(uint16_t), out, lybctx->lybctx));
        LY_CHECK_RET(lyb_write_string("true", 0, sizeof(uint16_t), out, lybctx->lybctx));
    }

    /* write all the node metadata */
    LY_LIST_FOR(node->meta, iter) {
        /* model */
        LY_CHECK_RET(lyb_print_model(out, iter->annotation->module, 0, lybctx->lybctx));

        /* annotation name with length */
        LY_CHECK_RET(lyb_write_string(iter->name, 0, sizeof(uint16_t), out, lybctx->lybctx));

        /* metadata value */
        LY_CHECK_RET(lyb_write_string(lyd_get_meta_value(iter), 0, sizeof(uint64_t), out, lybctx->lybctx));
    }

    return LY_SUCCESS;
}

/**
 * @brief Print opaque node attributes.
 *
 * @param[in] out Out structure.
 * @param[in] node Opaque node whose attributes to print.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_attributes(struct ly_out *out, const struct lyd_node_opaq *node, struct lylyb_ctx *lybctx)
{
    uint8_t count = 0;
    struct lyd_attr *iter;

    for (iter = node->attr; iter; iter = iter->next) {
        if (count == UINT8_MAX) {
            LOGERR(lybctx->ctx, LY_EINT, "Maximum supported number of data node attributes is %u.", UINT8_MAX);
            return LY_EINT;
        }
        ++count;
    }

    /* write number of attributes on 1 byte */
    LY_CHECK_RET(lyb_write(out, &count, 1, lybctx));

    /* write all the attributes */
    LY_LIST_FOR(node->attr, iter) {
        /* prefix */
        LY_CHECK_RET(lyb_write_string(iter->name.prefix, 0, sizeof(uint16_t), out, lybctx));

        /* namespace */
        LY_CHECK_RET(lyb_write_string(iter->name.module_name, 0, sizeof(uint16_t), out, lybctx));

        /* name */
        LY_CHECK_RET(lyb_write_string(iter->name.name, 0, sizeof(uint16_t), out, lybctx));

        /* format */
        LY_CHECK_RET(lyb_write_number(iter->format, 1, out, lybctx));

        /* value prefixes */
        LY_CHECK_RET(lyb_print_prefix_data(out, iter->format, iter->val_prefix_data, lybctx));

        /* value */
        LY_CHECK_RET(lyb_write_string(iter->value, 0, sizeof(uint64_t), out, lybctx));
    }

    return LY_SUCCESS;
}

/**
 * @brief Print schema node hash.
 *
 * @param[in] out Out structure.
 * @param[in] schema Schema node whose hash to print.
 * @param[in,out] sibling_ht Cached hash table for these siblings, created if NULL.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_schema_hash(struct ly_out *out, struct lysc_node *schema, struct ly_ht **sibling_ht, struct lylyb_ctx *lybctx)
{
    LY_ARRAY_COUNT_TYPE u;
    uint32_t i;
    LYB_HASH hash;
    struct lyd_lyb_sib_ht *sib_ht;
    struct lysc_node *first_sibling;

    if (!schema) {
        /* opaque node, write empty hash */
        hash = 0;
        LY_CHECK_RET(lyb_write(out, &hash, sizeof hash, lybctx));
        return LY_SUCCESS;
    }

    /* create whole sibling HT if not already created and saved */
    if (!*sibling_ht) {
        /* get first schema data sibling */
        first_sibling = (struct lysc_node *)lys_getnext(NULL, lysc_data_parent(schema), schema->module->compiled,
                (schema->flags & LYS_IS_OUTPUT) ? LYS_GETNEXT_OUTPUT : 0);
        LY_ARRAY_FOR(lybctx->sib_hts, u) {
            if (lybctx->sib_hts[u].first_sibling == first_sibling) {
                /* we have already created a hash table for these siblings */
                *sibling_ht = lybctx->sib_hts[u].ht;
                break;
            }
        }

        if (!*sibling_ht) {
            /* we must create sibling hash table */
            LY_CHECK_RET(lyb_hash_siblings(first_sibling, sibling_ht));

            /* and save it */
            LY_ARRAY_NEW_RET(lybctx->ctx, lybctx->sib_hts, sib_ht, LY_EMEM);

            sib_ht->first_sibling = first_sibling;
            sib_ht->ht = *sibling_ht;
        }
    }

    /* get our hash */
    LY_CHECK_RET(lyb_hash_find(*sibling_ht, schema, &hash));

    /* write the hash */
    LY_CHECK_RET(lyb_write(out, &hash, sizeof hash, lybctx));

    if (hash & LYB_HASH_COLLISION_ID) {
        /* no collision for this hash, we are done */
        return LY_SUCCESS;
    }

    /* written hash was a collision, write also all the preceding hashes */
    for (i = 0; !(hash & (LYB_HASH_COLLISION_ID >> i)); ++i) {}

    for ( ; i; --i) {
        hash = lyb_get_hash(schema, i - 1);
        if (!hash) {
            return LY_EINT;
        }
        assert(hash & (LYB_HASH_COLLISION_ID >> (i - 1)));

        LY_CHECK_RET(lyb_write(out, &hash, sizeof hash, lybctx));
    }

    return LY_SUCCESS;
}

/**
 * @brief Print header for non-opaq node.
 *
 * @param[in] out Out structure.
 * @param[in] node Current data node to print.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_node_header(struct ly_out *out, const struct lyd_node *node, struct lyd_lyb_ctx *lybctx)
{
    /* write any metadata */
    LY_CHECK_RET(lyb_print_metadata(out, node, lybctx));

    /* write node flags */
    LY_CHECK_RET(lyb_write_number(node->flags, sizeof node->flags, out, lybctx->lybctx));

    return LY_SUCCESS;
}

/**
 * @brief Print LYB node type.
 *
 * @param[in] out Out structure.
 * @param[in] node Current data node to print.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_lyb_type(struct ly_out *out, const struct lyd_node *node, struct lyd_lyb_ctx *lybctx)
{
    enum lylyb_node_type lyb_type;

    if (node->flags & LYD_EXT) {
        assert(node->schema);
        lyb_type = LYB_NODE_EXT;
    } else if (!node->schema) {
        lyb_type = LYB_NODE_OPAQ;
    } else if (!lysc_data_parent(node->schema)) {
        lyb_type = LYB_NODE_TOP;
    } else {
        lyb_type = LYB_NODE_CHILD;
    }

    LY_CHECK_RET(lyb_write_number(lyb_type, 1, out, lybctx->lybctx));

    return LY_SUCCESS;
}

/**
 * @brief Print inner node.
 *
 * @param[in] out Out structure.
 * @param[in] node Current data node to print.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_node_inner(struct ly_out *out, const struct lyd_node *node, struct lyd_lyb_ctx *lybctx)
{
    /* write necessary basic data */
    LY_CHECK_RET(lyb_print_node_header(out, node, lybctx));

    /* recursively write all the descendants */
    LY_CHECK_RET(lyb_print_siblings(out, lyd_child(node), lybctx));

    return LY_SUCCESS;
}

/**
 * @brief Print opaque node and its descendants.
 *
 * @param[in] out Out structure.
 * @param[in] opaq Node to print.
 * @param[in] lyd_lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_node_opaq(struct ly_out *out, const struct lyd_node_opaq *opaq, struct lyd_lyb_ctx *lyd_lybctx)
{
    struct lylyb_ctx *lybctx = lyd_lybctx->lybctx;

    /* write attributes */
    LY_CHECK_RET(lyb_print_attributes(out, opaq, lybctx));

    /* write node flags */
    LY_CHECK_RET(lyb_write_number(opaq->flags, sizeof opaq->flags, out, lybctx));

    /* prefix */
    LY_CHECK_RET(lyb_write_string(opaq->name.prefix, 0, sizeof(uint16_t), out, lybctx));

    /* module reference */
    LY_CHECK_RET(lyb_write_string(opaq->name.module_name, 0, sizeof(uint16_t), out, lybctx));

    /* name */
    LY_CHECK_RET(lyb_write_string(opaq->name.name, 0, sizeof(uint16_t), out, lybctx));

    /* value */
    LY_CHECK_RET(lyb_write_string(opaq->value, 0, sizeof(uint64_t), out, lybctx));

    /* format */
    LY_CHECK_RET(lyb_write_number(opaq->format, 1, out, lybctx));

    /* value prefixes */
    LY_CHECK_RET(lyb_print_prefix_data(out, opaq->format, opaq->val_prefix_data, lybctx));

    /* recursively write all the descendants */
    LY_CHECK_RET(lyb_print_siblings(out, opaq->child, lyd_lybctx));

    return LY_SUCCESS;
}

/**
 * @brief Print anydata or anyxml node.
 *
 * @param[in] anydata Node to print.
 * @param[in] out Out structure.
 * @param[in] lyd_lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_node_any(struct ly_out *out, struct lyd_node_any *anydata, struct lyd_lyb_ctx *lyd_lybctx)
{
    LY_ERR ret = LY_SUCCESS;
    LYD_ANYDATA_VALUETYPE value_type;
    int len;
    char *buf = NULL;
    const char *str;
    struct ly_out *out2 = NULL;
    struct lylyb_ctx *lybctx = lyd_lybctx->lybctx;

    if ((anydata->schema->nodetype == LYS_ANYDATA) && (anydata->value_type != LYD_ANYDATA_DATATREE)) {
        LOGINT_RET(lybctx->ctx);
    }

    if (anydata->value_type == LYD_ANYDATA_DATATREE) {
        /* will be printed as a nested LYB data tree because the used modules need to be written */
        value_type = LYD_ANYDATA_LYB;
    } else {
        value_type = anydata->value_type;
    }

    /* write necessary basic data */
    LY_CHECK_RET(lyb_print_node_header(out, (struct lyd_node *)anydata, lyd_lybctx));

    /* first byte is type */
    LY_CHECK_GOTO(ret = lyb_write_number(value_type, sizeof value_type, out, lybctx), cleanup);

    if (anydata->value_type == LYD_ANYDATA_DATATREE) {
        /* print LYB data tree to memory */
        LY_CHECK_GOTO(ret = ly_out_new_memory(&buf, 0, &out2), cleanup);
        LY_CHECK_GOTO(ret = lyb_print_data(out2, anydata->value.tree, LYD_PRINT_WITHSIBLINGS), cleanup);

        len = lyd_lyb_data_length(buf);
        assert(len != -1);
        str = buf;
    } else if (anydata->value_type == LYD_ANYDATA_LYB) {
        len = lyd_lyb_data_length(anydata->value.mem);
        assert(len != -1);
        str = anydata->value.mem;
    } else {
        len = strlen(anydata->value.str);
        str = anydata->value.str;
    }

    /* followed by the content */
    LY_CHECK_GOTO(ret = lyb_write_string(str, (size_t)len, sizeof(uint64_t), out, lybctx), cleanup);

cleanup:
    ly_out_free(out2, NULL, 1);
    return ret;
}

/**
 * @brief Print leaf node.
 *
 * @param[in] out Out structure.
 * @param[in] node Current data node to print.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_node_leaf(struct ly_out *out, const struct lyd_node *node, struct lyd_lyb_ctx *lybctx)
{
    /* write necessary basic data */
    LY_CHECK_RET(lyb_print_node_header(out, node, lybctx));

    /* write term value */
    LY_CHECK_RET(lyb_print_term_value((struct lyd_node_term *)node, out, lybctx->lybctx));

    return LY_SUCCESS;
}

/**
 * @brief Print all leaflist nodes which belong to same schema.
 *
 * @param[in] out Out structure.
 * @param[in] node Current data node to print.
 * @param[in] lybctx LYB context.
 * @param[out] printed_node Last node that was printed by this function.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_node_leaflist(struct ly_out *out, const struct lyd_node *node, struct lyd_lyb_ctx *lybctx,
        const struct lyd_node **printed_node)
{
    const struct lysc_node *schema;

    /* register a new sibling */
    LY_CHECK_RET(lyb_write_start_siblings(out, lybctx->lybctx));

    schema = node->schema;

    /* write all the siblings */
    LY_LIST_FOR(node, node) {
        if (schema != node->schema) {
            /* all leaflist nodes was printed */
            break;
        }

        /* write leaf data */
        LY_CHECK_RET(lyb_print_node_leaf(out, node, lybctx));
        *printed_node = node;
    }

    /* finish this sibling */
    LY_CHECK_RET(lyb_write_stop_siblings(out, lybctx->lybctx));

    return LY_SUCCESS;
}

/**
 * @brief Print all list nodes which belong to same schema.
 *
 * @param[in] out Out structure.
 * @param[in] node Current data node to print.
 * @param[in] lybctx LYB context.
 * @param[out] printed_node Last node that was printed by this function.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_node_list(struct ly_out *out, const struct lyd_node *node, struct lyd_lyb_ctx *lybctx,
        const struct lyd_node **printed_node)
{
    const struct lysc_node *schema;

    /* register a new sibling */
    LY_CHECK_RET(lyb_write_start_siblings(out, lybctx->lybctx));

    schema = node->schema;

    LY_LIST_FOR(node, node) {
        if (schema != node->schema) {
            /* all list nodes was printed */
            break;
        }

        /* write necessary basic data */
        LY_CHECK_RET(lyb_print_node_header(out, node, lybctx));

        /* recursively write all the descendants */
        LY_CHECK_RET(lyb_print_siblings(out, lyd_child(node), lybctx));

        *printed_node = node;
    }

    /* finish this sibling */
    LY_CHECK_RET(lyb_write_stop_siblings(out, lybctx->lybctx));

    return LY_SUCCESS;
}

/**
 * @brief Print node.
 *
 * @param[in] out Out structure.
 * @param[in,out] printed_node Current data node to print. Sets to the last printed node.
 * @param[in,out] sibling_ht Cached hash table for these siblings, created if NULL.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_node(struct ly_out *out, const struct lyd_node **printed_node, struct ly_ht **sibling_ht,
        struct lyd_lyb_ctx *lybctx)
{
    const struct lyd_node *node = *printed_node;

    /* write node type */
    LY_CHECK_RET(lyb_print_lyb_type(out, node, lybctx));

    /* write model info first */
    if (node->schema && ((node->flags & LYD_EXT) || !lysc_data_parent(node->schema))) {
        LY_CHECK_RET(lyb_print_model(out, node->schema->module, 0, lybctx->lybctx));
    }

    if (node->flags & LYD_EXT) {
        /* write schema node name */
        LY_CHECK_RET(lyb_write_string(node->schema->name, 0, sizeof(uint16_t), out, lybctx->lybctx));
    } else {
        /* write schema hash */
        LY_CHECK_RET(lyb_print_schema_hash(out, (struct lysc_node *)node->schema, sibling_ht, lybctx->lybctx));
    }

    if (!node->schema) {
        LY_CHECK_RET(lyb_print_node_opaq(out, (struct lyd_node_opaq *)node, lybctx));
    } else if (node->schema->nodetype & LYS_LEAFLIST) {
        LY_CHECK_RET(lyb_print_node_leaflist(out, node, lybctx, &node));
    } else if (node->schema->nodetype == LYS_LIST) {
        LY_CHECK_RET(lyb_print_node_list(out, node, lybctx, &node));
    } else if (node->schema->nodetype & LYD_NODE_ANY) {
        LY_CHECK_RET(lyb_print_node_any(out, (struct lyd_node_any *)node, lybctx));
    } else if (node->schema->nodetype & LYD_NODE_INNER) {
        LY_CHECK_RET(lyb_print_node_inner(out, node, lybctx));
    } else {
        LY_CHECK_RET(lyb_print_node_leaf(out, node, lybctx));
    }

    *printed_node = node;

    return LY_SUCCESS;
}

/**
 * @brief Print siblings.
 *
 * @param[in] out Out structure.
 * @param[in] node Current data node to print.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_siblings(struct ly_out *out, const struct lyd_node *node, struct lyd_lyb_ctx *lybctx)
{
    struct ly_ht *sibling_ht = NULL;
    const struct lys_module *prev_mod = NULL;

    LY_CHECK_RET(lyb_write_start_siblings(out, lybctx->lybctx));

    /* write all the siblings */
    LY_LIST_FOR(node, node) {
        /* do not reuse top-level sibling hash tables from different modules */
        if (!node->schema || (!lysc_data_parent(node->schema) && (node->schema->module != prev_mod))) {
            sibling_ht = NULL;
            prev_mod = node->schema ? node->schema->module : NULL;
        }

        LY_CHECK_RET(lyb_print_node(out, &node, &sibling_ht, lybctx));

        if (!lyd_parent(node) && !(lybctx->print_options & LYD_PRINT_WITHSIBLINGS)) {
            break;
        }
    }

    LY_CHECK_RET(lyb_write_stop_siblings(out, lybctx->lybctx));

    return LY_SUCCESS;
}

LY_ERR
lyb_print_data(struct ly_out *out, const struct lyd_node *root, uint32_t options)
{
    LY_ERR ret = LY_SUCCESS;
    uint8_t zero = 0;
    struct lyd_lyb_ctx *lybctx;
    const struct ly_ctx *ctx = root ? LYD_CTX(root) : NULL;

    lybctx = calloc(1, sizeof *lybctx);
    LY_CHECK_ERR_GOTO(!lybctx, LOGMEM(ctx); ret = LY_EMEM, cleanup);
    lybctx->lybctx = calloc(1, sizeof *lybctx->lybctx);
    LY_CHECK_ERR_GOTO(!lybctx->lybctx, LOGMEM(ctx); ret = LY_EMEM, cleanup);

    lybctx->print_options = options;
    if (root) {
        lybctx->lybctx->ctx = ctx;

        if (root->schema && lysc_data_parent(root->schema)) {
            LOGERR(lybctx->lybctx->ctx, LY_EINVAL, "LYB printer supports only printing top-level nodes.");
            ret = LY_EINVAL;
            goto cleanup;
        }
    }

    /* LYB magic number */
    LY_CHECK_GOTO(ret = lyb_print_magic_number(out), cleanup);

    /* LYB header */
    LY_CHECK_GOTO(ret = lyb_print_header(out), cleanup);

    /* all used models */
    LY_CHECK_GOTO(ret = lyb_print_data_models(out, root, lybctx->lybctx), cleanup);

    ret = lyb_print_siblings(out, root, lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* ending zero byte */
    LY_CHECK_GOTO(ret = lyb_write(out, &zero, sizeof zero, lybctx->lybctx), cleanup);

cleanup:
    lyd_lyb_ctx_free((struct lyd_ctx *)lybctx);
    return ret;
}
