/**
 * @file lyb.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Header for LYB format printer & parser
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_LYB_H_
#define LY_LYB_H_

#include <stddef.h>
#include <stdint.h>

#include "parser_internal.h"
#include "set.h"
#include "tree.h"

struct hash_table;
struct ly_ctx;
struct lyd_node;
struct lysc_node;

struct lylyb_ctx {
    const struct ly_ctx *ctx;
    uint64_t line;             /* current line */
    struct ly_in *in;          /* input structure */

    const struct lys_module **models;

    struct lyd_lyb_subtree {
        size_t written;
        size_t position;
        uint8_t inner_chunks;
    } *subtrees;
    LY_ARRAY_COUNT_TYPE subtree_size;

    /* LYB printer only */
    struct lyd_lyb_sib_ht {
        struct lysc_node *first_sibling;
        struct hash_table *ht;
    } *sib_hts;
};

/**
 * @brief Internal structure for LYB parser/printer.
 *
 * Note that the structure maps to the lyd_ctx which is common for all the data parsers
 */
struct lyd_lyb_ctx {
    const struct lysc_ext_instance *ext; /**< extension instance possibly changing document root context of the data being parsed */
    union {
        struct {
            uint32_t parse_opts;   /**< various @ref dataparseroptions. */
            uint32_t val_opts;     /**< various @ref datavalidationoptions. */
        };
        uint32_t print_options;
    };
    uint32_t int_opts;             /**< internal data parser options */
    uint32_t path_len;             /**< used bytes in the path buffer */
    char path[LYD_PARSER_BUFSIZE]; /**< buffer for the generated path */
    struct ly_set node_when;       /**< set of nodes with "when" conditions */
    struct ly_set node_exts;       /**< set of nodes and extensions connected with a plugin providing own validation callback */
    struct ly_set node_types;      /**< set of nodes validated with LY_EINCOMPLETE result */
    struct ly_set meta_types;      /**< set of metadata validated with LY_EINCOMPLETE result */
    struct lyd_node *op_node;      /**< if an RPC/action/notification is being parsed, store the pointer to it */

    /* callbacks */
    lyd_ctx_free_clb free;           /* destructor */

    struct lylyb_ctx *lybctx;      /* lyb format context */
};

/**
 * @brief Destructor for the lylyb_ctx structure
 */
void lyd_lyb_ctx_free(struct lyd_ctx *lydctx);

/**
 * LYB format
 *
 * Unlike XML or JSON, it is binary format so most data are represented in similar way but in binary.
 * Some notable differences:
 *
 * - schema nodes are identified based on their hash instead of their string name. In case of collisions
 * an array of hashes is created with each next hash one bit shorter until a unique sequence of all these
 * hashes is found and then all of them are stored.
 *
 * - tree structure is represented as individual strictly bounded subtrees. Each subtree begins
 * with its metadata, which consist of 1) the whole subtree length in bytes and 2) number
 * of included metadata chunks of nested subtrees.
 *
 * - since length of a subtree is not known before it is printed, holes are first written and
 * after the subtree is printed, they are filled with actual valid metadata. As a consequence,
 * LYB data cannot be directly printed into streams!
 *
 * - data are preceded with information about all the used modules. It is needed because of
 * possible augments and deviations which must be known beforehand, otherwise schema hashes
 * could be matched to the wrong nodes.
 */

/* just a shortcut */
#define LYB_LAST_SUBTREE(lybctx) lybctx->subtrees[LY_ARRAY_COUNT(lybctx->subtrees) - 1]

/* struct lyd_lyb_subtree allocation step */
#define LYB_SUBTREE_STEP 4

/* current LYB format version */
#define LYB_VERSION_NUM 0x10

/* LYB format version mask of the header byte */
#define LYB_VERSION_MASK 0x10

/**
 * LYB schema hash constants
 *
 * Hash is divided to collision ID and hash itself.
 *
 * @anchor collisionid
 *
 * First bits are collision ID until 1 is found. The rest is truncated 32b hash.
 * 1xxx xxxx - collision ID 0 (no collisions)
 * 01xx xxxx - collision ID 1 (collision ID 0 hash collided)
 * 001x xxxx - collision ID 2 ...
 *
 * When finding a match for a unique schema (siblings) hash (sequence of hashes with increasing collision ID), the highest
 * collision ID can be read from the last hash (LYB parser).
 *
 * To learn what is the highest collision ID of a hash that must be included in a unique schema (siblings) hash,
 * collisions with all the preceding sibling schema hashes must be checked (LYB printer).
 */

/* Number of bits the whole hash will take (including hash collision ID) */
#define LYB_HASH_BITS 8

/* Masking 32b hash (collision ID 0) */
#define LYB_HASH_MASK 0x7f

/* Type for storing the whole hash (used only internally, publicly defined directly) */
#define LYB_HASH uint8_t

/* Need to move this first >> collision number (from 0) to get collision ID hash part */
#define LYB_HASH_COLLISION_ID 0x80

/* How many bytes are reserved for one data chunk SIZE (8B is maximum) */
#define LYB_SIZE_BYTES 1

/* Maximum size that will be written into LYB_SIZE_BYTES (must be large enough) */
#define LYB_SIZE_MAX UINT8_MAX

/* How many bytes are reserved for one data chunk inner chunk count */
#define LYB_INCHUNK_BYTES 1

/* Maximum size that will be written into LYB_INCHUNK_BYTES (must be large enough) */
#define LYB_INCHUNK_MAX UINT8_MAX

/* Just a helper macro */
#define LYB_META_BYTES (LYB_INCHUNK_BYTES + LYB_SIZE_BYTES)
#define LYB_BYTE_MASK 0xff

/* model revision as XXXX XXXX XXXX XXXX (2B) (year is offset from 2000)
 *                   YYYY YYYM MMMD DDDD */
#define LYB_REV_YEAR_OFFSET 2000
#define LYB_REV_YEAR_MASK   0xfe00U
#define LYB_REV_YEAR_SHIFT  9
#define LYB_REV_MONTH_MASK  0x01E0U
#define LYB_REV_MONTH_SHIFT 5
#define LYB_REV_DAY_MASK    0x001fU

/* Type large enough for all meta data */
#define LYB_META uint16_t

/**
 * @brief Get single hash for a schema node to be used for LYB data. Read from cache, if possible.
 *
 * @param[in] node Node to hash.
 * @param[in] collision_id Collision ID of the hash to generate, see @ref collisionid.
 * @return Generated hash.
 */
LYB_HASH lyb_get_hash(const struct lysc_node *node, uint8_t collision_id);

/**
 * @brief Fill the hash cache of all the schema nodes of a module.
 *
 * @param[in] mod Module to process.
 */
void lyb_cache_module_hash(const struct lys_module *mod);

/**
 * @brief Check whether a node's module is in a module array.
 *
 * @param[in] node Node to check.
 * @param[in] models Modules in a sized array.
 * @return Boolean value whether @p node's module was found in the given @p models array.
 */
ly_bool lyb_has_schema_model(const struct lysc_node *node, const struct lys_module **models);

#endif /* LY_LYB_H_ */
