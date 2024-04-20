/**
 * @file lyb.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief LYB format common functionality.
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "lyb.h"

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "tree_schema.h"

/**
 * @brief Generate single hash for a schema node to be used for LYB data.
 *
 * @param[in] node Node to hash.
 * @param[in] collision_id Collision ID of the hash to generate.
 * @return Generated hash.
 */
static LYB_HASH
lyb_generate_hash(const struct lysc_node *node, uint8_t collision_id)
{
    const struct lys_module *mod = node->module;
    uint32_t full_hash;
    LYB_HASH hash;

    /* generate full hash */
    full_hash = dict_hash_multi(0, mod->name, strlen(mod->name));
    full_hash = dict_hash_multi(full_hash, node->name, strlen(node->name));
    if (collision_id) {
        size_t ext_len;

        if (collision_id > strlen(mod->name)) {
            /* fine, we will not hash more bytes, just use more bits from the hash than previously */
            ext_len = strlen(mod->name);
        } else {
            /* use one more byte from the module name than before */
            ext_len = collision_id;
        }
        full_hash = dict_hash_multi(full_hash, mod->name, ext_len);
    }
    full_hash = dict_hash_multi(full_hash, NULL, 0);

    /* use the shortened hash */
    hash = full_hash & (LYB_HASH_MASK >> collision_id);
    /* add collision identificator */
    hash |= LYB_HASH_COLLISION_ID >> collision_id;

    return hash;
}

LYB_HASH
lyb_get_hash(const struct lysc_node *node, uint8_t collision_id)
{
    /* hashes must be cached */
    assert(node->hash[0]);

    if (collision_id < LYS_NODE_HASH_COUNT) {
        /* read from cache */
        return node->hash[collision_id];
    }

    /* generate */
    return lyb_generate_hash(node, collision_id);
}

/**
 * @brief Module DFS callback filling all cached hashes of a schema node.
 */
static LY_ERR
lyb_cache_node_hash_cb(struct lysc_node *node, void *UNUSED(data), ly_bool *UNUSED(dfs_continue))
{
    if (node->hash[0]) {
        /* already cached, stop the DFS */
        return LY_EEXIST;
    }

    for (uint8_t i = 0; i < LYS_NODE_HASH_COUNT; ++i) {
        /* store the hash in the cache */
        node->hash[i] = lyb_generate_hash(node, i);
    }

    return LY_SUCCESS;
}

void
lyb_cache_module_hash(const struct lys_module *mod)
{
    /* LOCK */
    pthread_mutex_lock(&mod->ctx->lyb_hash_lock);

    /* store all cached hashes for all the nodes */
    lysc_module_dfs_full(mod, lyb_cache_node_hash_cb, NULL);

    /* UNLOCK */
    pthread_mutex_unlock(&mod->ctx->lyb_hash_lock);
}

ly_bool
lyb_has_schema_model(const struct lysc_node *node, const struct lys_module **models)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(models, u) {
        if (node->module == models[u]) {
            return 1;
        }
    }

    return 0;
}
