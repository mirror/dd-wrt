/**
 * @file printer_lyb.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief LYB printer for libyang data structure
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
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
lyb_hash_sequence_check(struct hash_table *ht, struct lysc_node *sibling, LYB_HASH ht_col_id, LYB_HASH compare_col_id)
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
    } while (!lyht_find_next(ht, col_node, lyb_get_hash(*col_node, ht_col_id), (void **)&col_node));

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
lyb_hash_siblings(struct lysc_node *sibling, struct hash_table **ht_p)
{
    struct hash_table *ht;
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
                    lyht_free(ht);
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
            lyht_free(ht);
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
lyb_hash_find(struct hash_table *ht, struct lysc_node *node, LYB_HASH *hash_p)
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
    struct lyd_lyb_subtree *full, *iter;
    size_t to_write;
    uint8_t meta_buf[LYB_META_BYTES];

    while (1) {
        /* check for full data chunks */
        to_write = count;
        full = NULL;
        LY_ARRAY_FOR(lybctx->subtrees, u) {
            /* we want the innermost chunks resolved first, so replace previous full chunks */
            if (lybctx->subtrees[u].written + to_write >= LYB_SIZE_MAX) {
                /* full chunk, do not write more than allowed */
                to_write = LYB_SIZE_MAX - lybctx->subtrees[u].written;
                full = &lybctx->subtrees[u];
            }
        }

        if (!full && !count) {
            break;
        }

        /* we are actually writing some data, not just finishing another chunk */
        if (to_write) {
            LY_CHECK_RET(ly_write_(out, (char *)buf, to_write));

            LY_ARRAY_FOR(lybctx->subtrees, u) {
                /* increase all written counters */
                lybctx->subtrees[u].written += to_write;
                assert(lybctx->subtrees[u].written <= LYB_SIZE_MAX);
            }
            /* decrease count/buf */
            count -= to_write;
            buf += to_write;
        }

        if (full) {
            /* write the meta information (inner chunk count and chunk size) */
            meta_buf[0] = full->written & LYB_BYTE_MASK;
            meta_buf[1] = full->inner_chunks & LYB_BYTE_MASK;
            LY_CHECK_RET(ly_write_skipped(out, full->position, (char *)meta_buf, LYB_META_BYTES));

            /* zero written and inner chunks */
            full->written = 0;
            full->inner_chunks = 0;

            /* skip space for another chunk size */
            LY_CHECK_RET(ly_write_skip(out, LYB_META_BYTES, &full->position));

            /* increase inner chunk count */
            for (iter = &lybctx->subtrees[0]; iter != full; ++iter) {
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
 * @brief Stop the current subtree - write its final metadata.
 *
 * @param[in] out Out structure.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_write_stop_subtree(struct ly_out *out, struct lylyb_ctx *lybctx)
{
    uint8_t meta_buf[LYB_META_BYTES];

    /* write the meta chunk information */
    meta_buf[0] = LYB_LAST_SUBTREE(lybctx).written & LYB_BYTE_MASK;
    meta_buf[1] = LYB_LAST_SUBTREE(lybctx).inner_chunks & LYB_BYTE_MASK;
    LY_CHECK_RET(ly_write_skipped(out, LYB_LAST_SUBTREE(lybctx).position, (char *)&meta_buf, LYB_META_BYTES));

    LY_ARRAY_DECREMENT(lybctx->subtrees);
    return LY_SUCCESS;
}

/**
 * @brief Start a new subtree - skip bytes for its metadata.
 *
 * @param[in] out Out structure.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_write_start_subtree(struct ly_out *out, struct lylyb_ctx *lybctx)
{
    LY_ARRAY_COUNT_TYPE u;

    u = LY_ARRAY_COUNT(lybctx->subtrees);
    if (u == lybctx->subtree_size) {
        LY_ARRAY_CREATE_RET(lybctx->ctx, lybctx->subtrees, u + LYB_SUBTREE_STEP, LY_EMEM);
        lybctx->subtree_size = u + LYB_SUBTREE_STEP;
    }

    LY_ARRAY_INCREMENT(lybctx->subtrees);
    LYB_LAST_SUBTREE(lybctx).written = 0;
    LYB_LAST_SUBTREE(lybctx).inner_chunks = 0;

    /* another inner chunk */
    for (u = 0; u < LY_ARRAY_COUNT(lybctx->subtrees) - 1; ++u) {
        if (lybctx->subtrees[u].inner_chunks == LYB_INCHUNK_MAX) {
            LOGINT(lybctx->ctx);
            return LY_EINT;
        }
        ++lybctx->subtrees[u].inner_chunks;
    }

    LY_CHECK_RET(ly_write_skip(out, LYB_META_BYTES, &LYB_LAST_SUBTREE(lybctx).position));

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
 * @param[in] with_length Whether to precede the string with its length.
 * @param[in] out Out structure.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_write_string(const char *str, size_t str_len, ly_bool with_length, struct ly_out *out, struct lylyb_ctx *lybctx)
{
    if (!str) {
        str = "";
        LY_CHECK_ERR_RET(str_len, LOGINT(lybctx->ctx), LY_EINT);
    }
    if (!str_len) {
        str_len = strlen(str);
    }

    if (with_length) {
        /* print length on 2 bytes */
        if (str_len > UINT16_MAX) {
            LOGINT(lybctx->ctx);
            return LY_EINT;
        }
        LY_CHECK_RET(lyb_write_number(str_len, 2, out, lybctx));
    }

    LY_CHECK_RET(lyb_write(out, (const uint8_t *)str, str_len, lybctx));

    return LY_SUCCESS;
}

/**
 * @brief Print YANG module info.
 *
 * @param[in] out Out structure.
 * @param[in] mod Module to print.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_model(struct ly_out *out, const struct lys_module *mod, struct lylyb_ctx *lybctx)
{
    uint16_t revision;

    /* model name length and model name */
    if (mod) {
        LY_CHECK_RET(lyb_write_string(mod->name, 0, 1, out, lybctx));
    } else {
        LY_CHECK_RET(lyb_write_string("", 0, 1, out, lybctx));
    }

    /* model revision as XXXX XXXX XXXX XXXX (2B) (year is offset from 2000)
     *                   YYYY YYYM MMMD DDDD */
    revision = 0;
    if (mod && mod->revision) {
        int r = atoi(mod->revision);
        r -= LYB_REV_YEAR_OFFSET;
        r <<= LYB_REV_YEAR_SHIFT;

        revision |= r;

        r = atoi(mod->revision + ly_strlen_const("YYYY-"));
        r <<= LYB_REV_MONTH_SHIFT;

        revision |= r;

        r = atoi(mod->revision + ly_strlen_const("YYYY-MM-"));

        revision |= r;
    }
    LY_CHECK_RET(lyb_write_number(revision, sizeof revision, out, lybctx));

    if (mod) {
        /* fill cached hashes, if not already */
        lyb_cache_module_hash(mod);
    }

    return LY_SUCCESS;
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
    const struct lyd_node *node;
    uint32_t i;

    LY_CHECK_RET(ly_set_new(&set));

    /* collect all data node modules */
    LY_LIST_FOR(root, node) {
        if (!node->schema) {
            continue;
        }

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
    }

    /* now write module count on 2 bytes */
    LY_CHECK_GOTO(ret = lyb_write_number(set->count, 2, out, lybctx), cleanup);

    /* and all the used models */
    for (i = 0; i < set->count; ++i) {
        LY_CHECK_GOTO(ret = lyb_print_model(out, set->objs[i], lybctx), cleanup);
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
        LY_CHECK_RET(lyb_write(out, (uint8_t *)&set->count, 1, lybctx));

        /* write all the prefixes */
        for (i = 0; i < set->count; ++i) {
            ns = set->objs[i];

            /* prefix */
            LY_CHECK_RET(lyb_write_string(ns->prefix, 0, 1, out, lybctx));

            /* namespace */
            LY_CHECK_RET(lyb_write_string(ns->uri, 0, 1, out, lybctx));
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
 * @brief Print opaque node.
 *
 * @param[in] opaq Node to print.
 * @param[in] out Out structure.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_opaq(struct lyd_node_opaq *opaq, struct ly_out *out, struct lylyb_ctx *lybctx)
{
    /* prefix */
    LY_CHECK_RET(lyb_write_string(opaq->name.prefix, 0, 1, out, lybctx));

    /* module reference */
    LY_CHECK_RET(lyb_write_string(opaq->name.module_name, 0, 1, out, lybctx));

    /* name */
    LY_CHECK_RET(lyb_write_string(opaq->name.name, 0, 1, out, lybctx));

    /* format */
    LY_CHECK_RET(lyb_write_number(opaq->format, 1, out, lybctx));

    /* value prefixes */
    LY_CHECK_RET(lyb_print_prefix_data(out, opaq->format, opaq->val_prefix_data, lybctx));

    /* value */
    LY_CHECK_RET(lyb_write_string(opaq->value, 0, 0, out, lybctx));

    return LY_SUCCESS;
}

/**
 * @brief Print anydata node.
 *
 * @param[in] anydata Node to print.
 * @param[in] out Out structure.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_anydata(struct lyd_node_any *anydata, struct ly_out *out, struct lylyb_ctx *lybctx)
{
    LY_ERR ret = LY_SUCCESS;
    LYD_ANYDATA_VALUETYPE value_type;
    int len;
    char *buf = NULL;
    const char *str;
    struct ly_out *out2 = NULL;

    if (anydata->value_type == LYD_ANYDATA_DATATREE) {
        /* will be printed as a nested LYB data tree */
        value_type = LYD_ANYDATA_LYB;
    } else {
        value_type = anydata->value_type;
    }

    /* first byte is type */
    LY_CHECK_GOTO(ret = lyb_write(out, (uint8_t *)&value_type, sizeof value_type, lybctx), cleanup);

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
    LY_CHECK_GOTO(ret = lyb_write_string(str, (size_t)len, 0, out, lybctx), cleanup);

cleanup:
    ly_out_free(out2, NULL, 1);
    return ret;
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
lyb_print_term(struct lyd_node_term *term, struct ly_out *out, struct lylyb_ctx *lybctx)
{
    /* print the value */
    return lyb_write_string(lyd_get_value(&term->node), 0, 0, out, lybctx);
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
        LY_CHECK_RET(lyb_write_start_subtree(out, lybctx->lybctx));
        LY_CHECK_RET(lyb_print_model(out, wd_mod, lybctx->lybctx));
        LY_CHECK_RET(lyb_write_string("default", 0, 1, out, lybctx->lybctx));
        LY_CHECK_RET(lyb_write_string("true", 0, 0, out, lybctx->lybctx));
        LY_CHECK_RET(lyb_write_stop_subtree(out, lybctx->lybctx));
    }

    /* write all the node metadata */
    LY_LIST_FOR(node->meta, iter) {
        /* each metadata is a subtree */
        LY_CHECK_RET(lyb_write_start_subtree(out, lybctx->lybctx));

        /* model */
        LY_CHECK_RET(lyb_print_model(out, iter->annotation->module, lybctx->lybctx));

        /* annotation name with length */
        LY_CHECK_RET(lyb_write_string(iter->name, 0, 1, out, lybctx->lybctx));

        /* metadata value */
        LY_CHECK_RET(lyb_write_string(lyd_get_meta_value(iter), 0, 0, out, lybctx->lybctx));

        /* finish metadata subtree */
        LY_CHECK_RET(lyb_write_stop_subtree(out, lybctx->lybctx));
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
        /* each attribute is a subtree */
        LY_CHECK_RET(lyb_write_start_subtree(out, lybctx));

        /* prefix */
        LY_CHECK_RET(lyb_write_string(iter->name.prefix, 0, 1, out, lybctx));

        /* namespace */
        LY_CHECK_RET(lyb_write_string(iter->name.module_name, 0, 1, out, lybctx));

        /* name */
        LY_CHECK_RET(lyb_write_string(iter->name.name, 0, 1, out, lybctx));

        /* format */
        LY_CHECK_RET(lyb_write_number(iter->format, 1, out, lybctx));

        /* value prefixes */
        LY_CHECK_RET(lyb_print_prefix_data(out, iter->format, iter->val_prefix_data, lybctx));

        /* value */
        LY_CHECK_RET(lyb_write_string(iter->value, 0, 0, out, lybctx));

        /* finish attribute subtree */
        LY_CHECK_RET(lyb_write_stop_subtree(out, lybctx));
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
lyb_print_schema_hash(struct ly_out *out, struct lysc_node *schema, struct hash_table **sibling_ht, struct lylyb_ctx *lybctx)
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
 * @brief Print data subtree.
 *
 * @param[in] out Out structure.
 * @param[in] node Root node of the subtree to print.
 * @param[in,out] sibling_ht Cached hash table for these data siblings, created if NULL.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_print_subtree(struct ly_out *out, const struct lyd_node *node, struct hash_table **sibling_ht, struct lyd_lyb_ctx *lybctx)
{
    struct hash_table *child_ht = NULL;

    /* register a new subtree */
    LY_CHECK_RET(lyb_write_start_subtree(out, lybctx->lybctx));

    /* write model info first */
    if (!node->schema && !lyd_parent(node)) {
        LY_CHECK_RET(lyb_print_model(out, NULL, lybctx->lybctx));
    } else if (node->schema && !lysc_data_parent(node->schema)) {
        LY_CHECK_RET(lyb_print_model(out, node->schema->module, lybctx->lybctx));
    }

    /* write schema hash */
    LY_CHECK_RET(lyb_print_schema_hash(out, (struct lysc_node *)node->schema, sibling_ht, lybctx->lybctx));

    /* write any metadata/attributes */
    if (node->schema) {
        LY_CHECK_RET(lyb_print_metadata(out, node, lybctx));
    } else {
        LY_CHECK_RET(lyb_print_attributes(out, (struct lyd_node_opaq *)node, lybctx->lybctx));
    }

    /* write node flags */
    LY_CHECK_RET(lyb_write_number(node->flags, sizeof node->flags, out, lybctx->lybctx));

    /* write node content */
    if (!node->schema) {
        LY_CHECK_RET(lyb_print_opaq((struct lyd_node_opaq *)node, out, lybctx->lybctx));
    } else if (node->schema->nodetype & LYD_NODE_INNER) {
        /* nothing to write */
    } else if (node->schema->nodetype & LYD_NODE_TERM) {
        LY_CHECK_RET(lyb_print_term((struct lyd_node_term *)node, out, lybctx->lybctx));
    } else if (node->schema->nodetype & LYD_NODE_ANY) {
        LY_CHECK_RET(lyb_print_anydata((struct lyd_node_any *)node, out, lybctx->lybctx));
    } else {
        LOGINT_RET(lybctx->lybctx->ctx);
    }

    /* recursively write all the descendants */
    LY_LIST_FOR(lyd_child(node), node) {
        LY_CHECK_RET(lyb_print_subtree(out, node, &child_ht, lybctx));
    }

    /* finish this subtree */
    LY_CHECK_RET(lyb_write_stop_subtree(out, lybctx->lybctx));

    return LY_SUCCESS;
}

LY_ERR
lyb_print_data(struct ly_out *out, const struct lyd_node *root, uint32_t options)
{
    LY_ERR ret = LY_SUCCESS;
    uint8_t zero = 0;
    struct hash_table *top_sibling_ht = NULL;
    const struct lys_module *prev_mod = NULL;
    struct lyd_lyb_ctx *lybctx;
    const struct ly_ctx *ctx = root ? LYD_CTX(root) : NULL;

    lybctx = calloc(1, sizeof *lybctx);
    LY_CHECK_ERR_RET(!lybctx, LOGMEM(ctx), LY_EMEM);
    lybctx->lybctx = calloc(1, sizeof *lybctx->lybctx);
    LY_CHECK_ERR_RET(!lybctx, LOGMEM(ctx), LY_EMEM);

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

    LY_LIST_FOR(root, root) {
        /* do not reuse sibling hash tables from different modules */
        if (!root->schema || (root->schema->module != prev_mod)) {
            top_sibling_ht = NULL;
            prev_mod = root->schema ? root->schema->module : NULL;
        }

        LY_CHECK_GOTO(ret = lyb_print_subtree(out, root, &top_sibling_ht, lybctx), cleanup);

        if (!(options & LYD_PRINT_WITHSIBLINGS)) {
            break;
        }
    }

    /* ending zero byte */
    LY_CHECK_GOTO(ret = lyb_write(out, &zero, sizeof zero, lybctx->lybctx), cleanup);

cleanup:
    lyd_lyb_ctx_free((struct lyd_ctx *)lybctx);
    return ret;
}
