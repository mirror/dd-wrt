/**
 * @file lyb.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Header for LYB format printer & parser
 *
 * Copyright (c) 2020 - 2022 CESNET, z.s.p.o.
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

struct ly_ctx;
struct lysc_node;

/*
 * LYB format
 *
 * Unlike XML or JSON, it is binary format so most data are represented in similar way but in binary.
 * Some notable differences:
 *
 * - schema nodes are identified based on their hash instead of their string name. In case of collisions
 * an array of hashes is created with each next hash one bit shorter until a unique sequence of all these
 * hashes is found and then all of them are stored.
 *
 * - tree structure is represented as individual strictly bounded "siblings". Each "siblings" begins
 * with its metadata, which consist of 1) the whole "sibling" length in bytes and 2) number
 * of included metadata chunks of nested "siblings".
 *
 * - since length of a "sibling" is not known before it is printed, holes are first written and
 * after the "sibling" is printed, they are filled with actual valid metadata. As a consequence,
 * LYB data cannot be directly printed into streams!
 *
 * - data are preceded with information about all the used modules. It is needed because of
 * possible augments and deviations which must be known beforehand, otherwise schema hashes
 * could be matched to the wrong nodes.
 *
 * This is a short summary of the format:
 * @verbatim

 sb          = siblings_start
 se          = siblings_end
 siblings    = zero-LYB_SIZE_BYTES | (sb instance+ se)
 instance    = node_type model hash node
 model       = 16bit_zero | (model_name_length model_name revision)
 node        = opaq | leaflist | list | any | inner | leaf
 opaq        = opaq_data siblings
 leaflist    = sb leaf+ se
 list        = sb (node_header siblings)+ se
 any         = node_header anydata_data
 inner       = node_header siblings
 leaf        = node_header term_value
 node_header = metadata node_flags

 @endverbatim
 */

/**
 * @brief LYB data node type
 */
enum lylyb_node_type {
    LYB_NODE_TOP,   /**< top-level node */
    LYB_NODE_CHILD, /**< child node with a parent */
    LYB_NODE_OPAQ,  /**< opaque node */
    LYB_NODE_EXT    /**< nested extension data node */
};

/**
 * @brief LYB format parser context
 */
struct lylyb_ctx {
    const struct ly_ctx *ctx;
    uint64_t line;             /* current line */
    struct ly_in *in;          /* input structure */

    const struct lys_module **models;

    struct lyd_lyb_sibling {
        size_t written;
        size_t position;
        uint16_t inner_chunks;
    } *siblings;
    LY_ARRAY_COUNT_TYPE sibling_size;

    /* LYB printer only */
    struct lyd_lyb_sib_ht {
        struct lysc_node *first_sibling;
        struct ly_ht *ht;
    } *sib_hts;
};

/**
 * @brief Destructor for the lylyb_ctx structure
 */
void lyd_lyb_ctx_free(struct lyd_ctx *lydctx);

/* just a shortcut */
#define LYB_LAST_SIBLING(lybctx) lybctx->siblings[LY_ARRAY_COUNT(lybctx->siblings) - 1]

/* struct lyd_lyb_sibling allocation step */
#define LYB_SIBLING_STEP 4

/* current LYB format version */
#define LYB_VERSION_NUM 0x05

/* LYB format version mask of the header byte */
#define LYB_VERSION_MASK 0x0F

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
#define LYB_SIZE_BYTES 2

/* Maximum size that will be written into LYB_SIZE_BYTES (must be large enough) */
#define LYB_SIZE_MAX UINT16_MAX

/* How many bytes are reserved for one data chunk inner chunk count */
#define LYB_INCHUNK_BYTES 2

/* Maximum size that will be written into LYB_INCHUNK_BYTES (must be large enough) */
#define LYB_INCHUNK_MAX UINT16_MAX

/* Just a helper macro */
#define LYB_META_BYTES (LYB_INCHUNK_BYTES + LYB_SIZE_BYTES)

/* model revision as XXXX XXXX XXXX XXXX (2B) (year is offset from 2000)
 *                   YYYY YYYM MMMD DDDD */
#define LYB_REV_YEAR_OFFSET 2000
#define LYB_REV_YEAR_MASK   0xfe00U
#define LYB_REV_YEAR_SHIFT  9
#define LYB_REV_MONTH_MASK  0x01E0U
#define LYB_REV_MONTH_SHIFT 5
#define LYB_REV_DAY_MASK    0x001fU

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
