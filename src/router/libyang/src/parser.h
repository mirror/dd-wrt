/**
 * @file parser.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Parsers for libyang
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PARSER_H_
#define LY_PARSER_H_

#include <pcre.h>
#include <sys/mman.h>

#include "libyang.h"
#include "tree_schema.h"
#include "tree_internal.h"

#ifdef __APPLE__
# ifndef MAP_ANONYMOUS
#  define MAP_ANONYMOUS MAP_ANON
# endif
#endif

/**
 * @defgroup yin YIN format support
 * @{
 */
struct lys_module *yin_read_module(struct ly_ctx *ctx, const char *data, const char *revision, int implement);
struct lys_submodule *yin_read_submodule(struct lys_module *module, const char *data,struct unres_schema *unres);

/**@} yin */

/**
 * @defgroup xmldata XML data format support
 * @{
 */
struct lyd_node *xml_read_data(struct ly_ctx *ctx, const char *data, int options);

/**@} xmldata */

/**
 * @defgroup jsondata JSON data format support
 * @{
 */
struct lyd_node *lyd_parse_json(struct ly_ctx *ctx, const char *data, int options, const struct lyd_node *rpc_act,
                                const struct lyd_node *data_tree, const char *yang_data_name);

/**@} jsondata */

/**
 * @defgroup lybdata LYB data format support
 * @{
 */
struct lyd_node *lyd_parse_lyb(struct ly_ctx *ctx, const char *data, int options, const struct lyd_node *data_tree,
                               const char *yang_data_name, int *parsed);

/**@} lybdata */

/**
 * internal options values for schema parsers
 */
#define LYS_PARSE_OPT_CFG_NOINHERIT 0x01 /**< do not inherit config flag */
#define LYS_PARSE_OPT_CFG_IGNORE    0x02 /**< ignore config flag (in rpc, actions, notifications) */
#define LYS_PARSE_OPT_CFG_MASK      0x03
#define LYS_PARSE_OPT_INGRP         0x04 /**< flag to know that parser is inside a grouping */

/* list of YANG statement strings */
extern const char *ly_stmt_str[];

enum LY_IDENT {
    LY_IDENT_SIMPLE,   /* only syntax rules */
    LY_IDENT_FEATURE,
    LY_IDENT_IDENTITY,
    LY_IDENT_TYPE,
    LY_IDENT_NODE,
    LY_IDENT_NAME,     /* uniqueness across the siblings */
    LY_IDENT_PREFIX,
    LY_IDENT_EXTENSION
};
int lyp_yin_fill_ext(void *parent, LYEXT_PAR parent_type, LYEXT_SUBSTMT substmt, uint8_t substmt_index,
                     struct lys_module *module, struct lyxml_elem *yin, struct lys_ext_instance ***ext,
                     uint8_t ext_index, struct unres_schema *unres);

int lyp_yin_parse_complex_ext(struct lys_module *mod, struct lys_ext_instance_complex *ext,
                              struct lyxml_elem *yin, struct unres_schema *unres);
int lyp_yin_parse_subnode_ext(struct lys_module *mod, void *elem, LYEXT_PAR elem_type,
                              struct lyxml_elem *yin, LYEXT_SUBSTMT type, uint8_t i, struct unres_schema *unres);

struct lys_type *lyp_get_next_union_type(struct lys_type *type, struct lys_type *prev_type, int *found);

/* return: 0 - ret set, ok; 1 - ret not set, no log, unknown meta; -1 - ret not set, log, fatal error */
int lyp_fill_attr(struct ly_ctx *ctx, struct lyd_node *parent, const char *module_ns, const char *module_name,
                  const char *attr_name, const char *attr_value, struct lyxml_elem *xml, int options, struct lyd_attr **ret);

int lyp_check_edit_attr(struct ly_ctx *ctx, struct lyd_attr *attr, struct lyd_node *parent, int *editbits);

struct lys_type *lyp_parse_value(struct lys_type *type, const char **value_, struct lyxml_elem *xml,
                                 struct lyd_node_leaf_list *leaf, struct lyd_attr *attr, struct lys_module *local_mod,
                                 int store, int dflt, int trusted);

int lyp_check_length_range(struct ly_ctx *ctx, const char *expr, struct lys_type *type);

int lyp_check_pattern(struct ly_ctx *ctx, const char *pattern, pcre **pcre_precomp);
int lyp_precompile_pattern(struct ly_ctx *ctx, const char *pattern, pcre** pcre_cmp, pcre_extra **pcre_std);

int fill_yin_type(struct lys_module *module, struct lys_node *parent, struct lyxml_elem *yin, struct lys_type *type,
                  int tpdftype, struct unres_schema *unres);

int lyp_check_status(uint16_t flags1, struct lys_module *mod1, const char *name1,
                     uint16_t flags2, struct lys_module *mod2, const char *name2,
                     const struct lys_node *node);

void lyp_del_includedup(struct lys_module *mod, int free_subs);

int dup_typedef_check(const char *type, struct lys_tpdf *tpdf, int size);

int dup_identities_check(const char *id, struct lys_module *module);

/**
 * @brief Get know if the node is part of the RPC/action's input/output
 *
 * @param node Schema node to be examined.
 * @return 1 for true, 0 for false
 */
int lyp_is_rpc_action(struct lys_node *node);

/**
 * @brief Check validity of data parser options.
 *
 * @param options Parser options to be checked.
 * @param func name of the function where called
 * @return 0 for ok, 1 when multiple data types bits are set, or incompatible options are used together.
 */
int lyp_data_check_options(struct ly_ctx *ctx, int options, const char *func);

int lyp_check_identifier(struct ly_ctx *ctx, const char *id, enum LY_IDENT type, struct lys_module *module, struct lys_node *parent);
int lyp_check_date(struct ly_ctx *ctx, const char *date);
int lyp_check_mandatory_augment(struct lys_node_augment *node, const struct lys_node *target);
int lyp_check_mandatory_choice(struct lys_node *node);

int lyp_check_include(struct lys_module *module, const char *value,
                      struct lys_include *inc, struct unres_schema *unres);
int lyp_check_include_missing(struct lys_module *main_module);
int lyp_check_import(struct lys_module *module, const char *value, struct lys_import *imp);
int lyp_check_circmod_add(struct lys_module *module);
void lyp_check_circmod_pop(struct ly_ctx *ctx);

void lyp_sort_revisions(struct lys_module *module);
int lyp_rfn_apply_ext(struct lys_module *module);
int lyp_deviation_apply_ext(struct lys_module *module);
int lyp_mand_check_ext(struct lys_ext_instance_complex *ext, const char *ext_name);

const char *lyp_get_yang_data_template_name(const struct lyd_node *node);
const struct lys_node *lyp_get_yang_data_template(const struct lys_module *module, const char *yang_data_name, int yang_data_name_len);

void lyp_ext_instance_rm(struct ly_ctx *ctx, struct lys_ext_instance ***ext, uint8_t *size, uint8_t index);

/**
 * @brief Propagate imports and includes into the main module
 *
 * @param module Main module
 * @param inc Filled include structure
 * @return 0 for success, 1 for failure
 */
int lyp_propagate_submodule(struct lys_module *module, struct lys_include *inc);

/* return: -1 = error, 0 = succes, 1 = already there (if it was disabled, it is enabled first) */
int lyp_ctx_check_module(struct lys_module *module);

int lyp_ctx_add_module(struct lys_module *module);

/**
 * @brief Add annotations definitions of attributes and URL config used in ietf-netconf RPCs.
 */
int lyp_add_ietf_netconf_annotations_config(struct lys_module *mod);

/**
 * @brief mmap() wrapper for parsers. To unmap, use lyp_munmap().
 *
 * @param[in] prot The desired memory protection as in case of mmap().
 * @param[in] fd File descriptor for getting data.
 * @param[in] addsize Number of additional bytes to be allocated (and zeroed) after the implicitly added
 *                    string-terminating NULL byte.
 * @param[out] length length of the allocated memory.
 * @param[out] addr Pointer to the memory where the file data is mapped.
 * @return 0 on success, non-zero on error.
 */
int lyp_mmap(struct ly_ctx *ctx, int fd, size_t addsize, size_t *length, void **addr);

/**
 * @brief Unmap function for the data mapped by lyp_mmap()
 */
int lyp_munmap(void *addr, size_t length);

/**
 * Store UTF-8 character specified as 4byte integer into the dst buffer.
 * Returns number of written bytes (4 max), expects that dst has enough space.
 *
 * UTF-8 mapping:
 * 00000000 -- 0000007F:    0xxxxxxx
 * 00000080 -- 000007FF:    110xxxxx 10xxxxxx
 * 00000800 -- 0000FFFF:    1110xxxx 10xxxxxx 10xxxxxx
 * 00010000 -- 001FFFFF:    11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 *
 */
unsigned int pututf8(struct ly_ctx *ctx, char *dst, int32_t value);
unsigned int copyutf8(struct ly_ctx *ctx, char *dst, const char *src);

/**
 * @brief Find a module. First, imports from \p module with matching \p prefix, \p name, or both are checked,
 * \p module itself is also compared, and lastly a callback is used if allowed.
 *
 * @param[in] module Module with imports.
 * @param[in] prefix Module prefix to search for.
 * @param[in] pref_len Module \p prefix length. If 0, the whole prefix is used, if not NULL.
 * @param[in] name Module name to search for.
 * @param[in] name_len Module \p name length. If 0, the whole name is used, if not NULL.
 * @param[in] in_data Whether to use data callback if not found after trying all the rest.
 * Import callback is never used because there is no use-case for that.
 *
 * @return Matching module, NULL if not found.
 */
const struct lys_module *lyp_get_module(const struct lys_module *module, const char *prefix, int pref_len,
                                               const char *name, int name_len, int in_data);

/**
 * @brief Find an import from \p module with matching namespace, the \p module itself is also considered.
 *
 * @param[in] module Module with imports.
 * @param[in] ns Namespace to be found.
 */
const struct lys_module *lyp_get_import_module_ns(const struct lys_module *module, const char *ns);

/*
 * Internal functions implementing YANG (extension and user type) plugin support
 * - implemented in plugins.c
 */

/**
 * @brief If available, get the extension plugin for the specified extension
 *
 * @param[in] name Name of the extension
 * @param[in] module Name of the extension's module
 * @param[in] revision Revision of the extension's module
 * @return pointer to the extension plugin structure, NULL if no plugin available
 */
struct lyext_plugin *ext_get_plugin(const char *name, const char *module, const char *revision);

/**
 * @brief Try to store a value as a user type defined by a plugin.
 *
 * @param[in] mod Module of the type.
 * @param[in] type_name Type (typedef) name.
 * @param[in] value_str Value to store as a string.
 * @param[in,out] value Filled value to be overwritten by the user store callback.
 * @return 0 on successful storing, 1 if the type is not a user type, -1 on error.
 */
int lytype_store(const struct lys_module *mod, const char *type_name, const char *value_str, lyd_val *value);

/**
 * @brief Free a user type stored value.
 *
 * @param[in] mod Module of the type.
 * @param[in] type_name Type (typedef) name.
 * @param[in] value Value union to free.
 */
void lytype_free(const struct lys_module *mod, const char *type_name, lyd_val value);

#endif /* LY_PARSER_H_ */
