/**
 * @file plugins_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief internal functions to support extension and type plugins.
 *
 * Copyright (c) 2019-2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PLUGINS_INTERNAL_H_
#define LY_PLUGINS_INTERNAL_H_

#include <stdint.h>

#include "plugins.h"
#include "plugins_exts.h"
#include "tree_schema.h"

#define LY_TYPE_UNKNOWN_STR "unknown"               /**< text representation of ::LY_TYPE_UNKNOWN */
#define LY_TYPE_BINARY_STR "binary"                 /**< text representation of ::LY_TYPE_BINARY */
#define LY_TYPE_UINT8_STR "8bit unsigned integer"   /**< text representation of ::LY_TYPE_UINT8 */
#define LY_TYPE_UINT16_STR "16bit unsigned integer" /**< text representation of ::LY_TYPE_UINT16 */
#define LY_TYPE_UINT32_STR "32bit unsigned integer" /**< text representation of ::LY_TYPE_UINT32 */
#define LY_TYPE_UINT64_STR "64bit unsigned integer" /**< text representation of ::LY_TYPE_UINT64 */
#define LY_TYPE_STRING_STR "string"                 /**< text representation of ::LY_TYPE_STRING */
#define LY_TYPE_BITS_STR "bits"                     /**< text representation of ::LY_TYPE_BITS */
#define LY_TYPE_BOOL_STR "boolean"                  /**< text representation of ::LY_TYPE_BOOL */
#define LY_TYPE_DEC64_STR "decimal64"               /**< text representation of ::LY_TYPE_DEC64 */
#define LY_TYPE_EMPTY_STR "empty"                   /**< text representation of ::LY_TYPE_EMPTY */
#define LY_TYPE_ENUM_STR "enumeration"              /**< text representation of ::LY_TYPE_ENUM */
#define LY_TYPE_IDENT_STR "identityref"             /**< text representation of ::LY_TYPE_IDENT */
#define LY_TYPE_INST_STR "instance-identifier"      /**< text representation of ::LY_TYPE_INST */
#define LY_TYPE_LEAFREF_STR "leafref"               /**< text representation of ::LY_TYPE_LEAFREF */
#define LY_TYPE_UNION_STR "union"                   /**< text representation of ::LY_TYPE_UNION */
#define LY_TYPE_INT8_STR "8bit integer"             /**< text representation of ::LY_TYPE_INT8 */
#define LY_TYPE_INT16_STR "16bit integer"           /**< text representation of ::LY_TYPE_INT16 */
#define LY_TYPE_INT32_STR "32bit integer"           /**< text representation of ::LY_TYPE_INT32 */
#define LY_TYPE_INT64_STR "64bit integer"           /**< text representation of ::LY_TYPE_INT64 */

/**
 * @brief Initiate libyang plugins.
 *
 * Covers both the types and extensions plugins.
 *
 * @return LY_SUCCESS in case of success
 * @return LY_EINT in case of internal error
 * @return LY_EMEM in case of memory allocation failure.
 */
LY_ERR lyplg_init(void);

/**
 * @brief Remove (unload) all the plugins currently available.
 */
void lyplg_clean(void);

/**
 * @brief Find the plugin matching the provided attributes.
 *
 * @param[in] type Type of the plugin to find (type or extension)
 * @param[in] module Name of the module where the type/extension is defined. Must not be NULL, in case of plugins for
 * built-in types, the module is "".
 * @param[in] revision The revision of the module for which the plugin is implemented. NULL is not a wildcard, it matches
 * only the plugins with NULL revision specified.
 * @param[in] name Name of the type/extension which the plugin implements.
 * @return NULL if the plugin matching the restrictions is not present.
 * @return Pointer to the matching ::ly_type_plugin or ::lyext_plugin according to the plugin's @p type.
 */
void *lyplg_find(enum LYPLG type, const char *module, const char *revision, const char *name);

#endif /* LY_PLUGINS_INTERNAL_H_ */
