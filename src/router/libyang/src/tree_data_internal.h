/**
 * @file tree_data_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief internal functions for YANG schema trees.
 *
 * Copyright (c) 2015 - 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_DATA_INTERNAL_H_
#define LY_TREE_DATA_INTERNAL_H_

#include "log.h"
#include "plugins_types.h"
#include "set.h"
#include "tree_data.h"

#include <stddef.h>

struct ly_path_predicate;
struct lysc_module;

#define LY_XML_SUFFIX ".xml"
#define LY_XML_SUFFIX_LEN 4
#define LY_JSON_SUFFIX ".json"
#define LY_JSON_SUFFIX_LEN 5
#define LY_LYB_SUFFIX ".lyb"
#define LY_LYB_SUFFIX_LEN 4

/**
 * @brief Internal structure for remembering "used" instances of lists with duplicate instances allowed.
 */
struct lyd_dup_inst {
    struct ly_set *inst_set;
    uint32_t used;
};

/**
 * @brief Update a found inst using a duplicate instance cache. Needs to be called for every "used"
 * (that should not be considered next time) instance.
 *
 * @param[in,out] inst Found instance, is updated so that the same instance is not returned twice.
 * @param[in] siblings Siblings where @p inst was found.
 * @param[in,out] dup_inst_cache Duplicate instance cache.
 * @return LY_ERR value.
 */
LY_ERR lyd_dup_inst_next(struct lyd_node **inst, const struct lyd_node *siblings,
        struct lyd_dup_inst **dup_inst_cache);

/**
 * @brief Free duplicate instance cache.
 *
 * @param[in] dup_inst Duplicate instance cache to free.
 */
void lyd_dup_inst_free(struct lyd_dup_inst *dup_inst);

/**
 * @brief Check whether a node to be deleted is the root node, move it if it is.
 *
 * @param[in] root Root sibling.
 * @param[in] to_del Node to be deleted.
 * @param[in] mod If set, it is expected @p tree should point to the first node of @p mod. Otherwise it will simply be
 * the first top-level sibling.
 */
void lyd_del_move_root(struct lyd_node **root, const struct lyd_node *to_del, const struct lys_module *mod);

/**
 * @brief Get address of a node's child pointer if any.
 *
 * @param[in] node Node to check.
 * @return Address of the node's child member,
 * @return NULL if there is no child pointer.
 */
struct lyd_node **lyd_node_child_p(struct lyd_node *node);

/**
 * @brief Just like ::lys_getnext() but iterates over all data instances of the schema nodes.
 *
 * @param[in] last Last returned data node.
 * @param[in] sibling Data node sibling to search in.
 * @param[in,out] slast Schema last node, set to NULL for first call and do not change afterwards.
 * May not be set if the function is used only for any suitable node existence check (such as the existence
 * of any choice case data).
 * @param[in] parent Schema parent of the iterated children nodes.
 * @param[in] module Schema module of the iterated top-level nodes.
 * @return Next matching data node,
 * @return NULL if last data node was already returned.
 */
struct lyd_node *lys_getnext_data(const struct lyd_node *last, const struct lyd_node *sibling,
        const struct lysc_node **slast, const struct lysc_node *parent,
        const struct lysc_module *module);

/**
 * @brief Create a term (leaf/leaf-list) node from a string value.
 *
 * Hash is calculated and new node flag is set.
 *
 * @param[in] schema Schema node of the new data node.
 * @param[in] value String value to be parsed.
 * @param[in] value_len Length of @p value, must be set correctly.
 * @param[in,out] dynamic Flag if @p value is dynamically allocated, is adjusted when @p value is consumed.
 * @param[in] format Input format of @p value.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[in] hints [Value hints](@ref lydvalhints) from the parser regarding the value type.
 * @param[out] incomplete Whether the value needs to be resolved.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_EINCOMPLETE in case data tree is needed to finish the validation.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_term(const struct lysc_node *schema, const char *value, size_t value_len, ly_bool *dynamic,
        LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, ly_bool *incomplete, struct lyd_node **node);

/**
 * @brief Create a term (leaf/leaf-list) node from a parsed value by duplicating it.
 *
 * Hash is calculated and new node flag is set.
 *
 * @param[in] schema Schema node of the new data node.
 * @param[in] val Parsed value to use.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_term2(const struct lysc_node *schema, const struct lyd_value *val, struct lyd_node **node);

/**
 * @brief Create an inner (container/list/RPC/action/notification) node.
 *
 * Hash is calculated and new node flag is set except
 * for list with keys, when the hash is not calculated!
 * Also, non-presence container has its default flag set.
 *
 * @param[in] schema Schema node of the new data node.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_inner(const struct lysc_node *schema, struct lyd_node **node);

/**
 * @brief Create a list with all its keys (cannot be used for key-less list).
 *
 * Hash is calculated and new node flag is set.
 *
 * @param[in] schema Schema node of the new data node.
 * @param[in] predicates Compiled key list predicates.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_list(const struct lysc_node *schema, const struct ly_path_predicate *predicates, struct lyd_node **node);

/**
 * @brief Create an anyxml/anydata node.
 *
 * Hash is calculated and flags are properly set based on @p is_valid.
 *
 * @param[in] schema Schema node of the new data node.
 * @param[in] value Value of the any node.
 * @param[in] value_type Value type of the value.
 * @param[in] use_value Whether to directly assign (eat) the value or duplicate it.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_any(const struct lysc_node *schema, const void *value, LYD_ANYDATA_VALUETYPE value_type,
        ly_bool use_value, struct lyd_node **node);

/**
 * @brief Create an opaque node.
 *
 * @param[in] ctx libyang context.
 * @param[in] name Element name.
 * @param[in] name_len Length of @p name, must be set correctly.
 * @param[in] prefix Element prefix.
 * @param[in] pref_len Length of @p prefix, must be set correctly.
 * @param[in] module_key Mandatory key to reference module, can be namespace or name.
 * @param[in] module_key_len Length of @p module_key, must be set correctly.
 * @param[in] value String value to be parsed.
 * @param[in] value_len Length of @p value, must be set correctly.
 * @param[in,out] dynamic Flag if @p value is dynamically allocated, is adjusted when @p value is consumed.
 * @param[in] format Input format of @p value and @p ns.
 * @param[in] val_prefix_data Format-specific prefix data, param is spent (even in case the function fails):
 *      LY_PREF_SCHEMA          - const struct lysp_module * (module used for resolving prefixes from imports)
 *      LY_PREF_SCHEMA_RESOLVED - struct lyd_value_prefix * (sized array of pairs: prefix - module)
 *      LY_PREF_XML             - const struct ly_set * (set with defined namespaces stored as ::lyxml_ns)
 *      LY_PREF_JSON            - NULL
 * @param[in] hints [Hints](@ref lydhints) from the parser regarding the node/value type.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_opaq(const struct ly_ctx *ctx, const char *name, size_t name_len, const char *prefix, size_t pref_len,
        const char *module_key, size_t module_key_len, const char *value, size_t value_len, ly_bool *dynamic,
        LY_VALUE_FORMAT format, void *val_prefix_data, uint32_t hints, struct lyd_node **node);

/**
 * @brief Check the existence and create any non-existing implicit siblings, recursively for the created nodes.
 *
 * @param[in] parent Parent of the potential default values, NULL for top-level siblings.
 * @param[in,out] first First sibling.
 * @param[in] sparent Schema parent of the siblings, NULL if schema of @p parent can be used.
 * @param[in] mod Module of the default values, NULL for nested siblings.
 * @param[in] node_when Optional set to add nodes with "when" conditions into.
 * @param[in] node_exts Optional set to add nodes and extension instances having own validation plugin callback into it.
 * @param[in] node_types Optional set to add nodes with unresolved types into.
 * @param[in] impl_opts Implicit options (@ref implicitoptions).
 * @param[in,out] diff Validation diff.
 * @return LY_ERR value.
 */
LY_ERR lyd_new_implicit_r(struct lyd_node *parent, struct lyd_node **first, const struct lysc_node *sparent,
        const struct lys_module *mod, struct ly_set *node_when, struct ly_set *node_exts, struct ly_set *node_types,
        uint32_t impl_opts, struct lyd_node **diff);

/**
 * @brief Find the next node, before which to insert the new node.
 *
 * @param[in] first_sibling First sibling of the nodes to consider.
 * @param[in] new_node Node that will be inserted.
 * @return Node to insert after.
 * @return NULL if the new node should be first.
 */
struct lyd_node *lyd_insert_get_next_anchor(const struct lyd_node *first_sibling, const struct lyd_node *new_node);

/**
 * @brief Insert a node into parent/siblings. Order and hashes are fully handled.
 *
 * @param[in] parent Parent to insert into, NULL for top-level sibling.
 * @param[in,out] first_sibling First sibling, NULL if no top-level sibling exist yet. Can be also NULL if @p parent is set.
 * @param[in] node Individual node (without siblings) to insert.
 */
void lyd_insert_node(struct lyd_node *parent, struct lyd_node **first_sibling, struct lyd_node *node);

/**
 * @brief Insert a metadata (last) into a parent
 *
 * @param[in] parent Parent of the metadata.
 * @param[in] meta Metadata (list) to be added into the @p parent.
 * @param[in] clear_dflt Whether to clear dflt flag starting from @p parent, recursively all NP containers.
 */
void lyd_insert_meta(struct lyd_node *parent, struct lyd_meta *meta, ly_bool clear_dflt);

/**
 * @brief Create and insert a metadata (last) into a parent.
 *
 * @param[in] parent Parent of the metadata, can be NULL.
 * @param[in,out] meta Metadata list to add at its end if @p parent is NULL, returned created attribute.
 * @param[in] mod Metadata module (with the annotation definition).
 * @param[in] name Attribute name.
 * @param[in] name_len Length of @p name, must be set correctly.
 * @param[in] value String value to be parsed.
 * @param[in] value_len Length of @p value, must be set correctly.
 * @param[in,out] dynamic Flag if @p value is dynamically allocated, is adjusted when @p value is consumed.
 * @param[in] format Input format of @p value.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[in] hints [Value hints](@ref lydvalhints) from the parser regarding the value type.
 * @param[in] clear_dflt Whether to clear dflt flag starting from @p parent, recursively all NP containers.
 * @param[out] incomplete Whether the value needs to be resolved.
 * @return LY_SUCCESS on success.
 * @return LY_EINCOMPLETE in case data tree is needed to finish the validation.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_meta(struct lyd_node *parent, struct lyd_meta **meta, const struct lys_module *mod, const char *name,
        size_t name_len, const char *value, size_t value_len, ly_bool *dynamic, LY_VALUE_FORMAT format,
        void *prefix_data, uint32_t hints, ly_bool clear_dlft, ly_bool *incomplete);

/**
 * @brief Insert an attribute (last) into a parent
 *
 * @param[in] parent Parent of the attributes.
 * @param[in] attr Attribute (list) to be added into the @p parent.
 */
void lyd_insert_attr(struct lyd_node *parent, struct lyd_attr *attr);

/**
 * @brief Create and insert a generic attribute (last) into a parent.
 *
 * @param[in] parent Parent of the attribute, can be NULL.
 * @param[in,out] attr Attribute list to add at its end if @p parent is NULL, returned created attribute.
 * @param[in] ctx libyang context.
 * @param[in] name Attribute name.
 * @param[in] name_len Length of @p name, must be set correctly.
 * @param[in] prefix Attribute prefix.
 * @param[in] prefix_len Attribute prefix length.
 * @param[in] module_key Mandatory key to reference module, can be namespace or name.
 * @param[in] module_key_len Length of @p module_key, must be set correctly.
 * @param[in] value String value to be parsed.
 * @param[in] value_len Length of @p value, must be set correctly.
 * @param[in,out] dynamic Flag if @p value is dynamically allocated, is adjusted when @p value is consumed.
 * @param[in] format Input format of @p value and @p ns.
 * @param[in] val_prefix_data Format-specific prefix data, param is spent (even in case the function fails).
 * @param[in] hints [Hints](@ref lydhints) from the parser regarding the node/value type.
 * @return LY_SUCCESS on success,
 * @return LY_ERR value on error.
 */
LY_ERR lyd_create_attr(struct lyd_node *parent, struct lyd_attr **attr, const struct ly_ctx *ctx, const char *name,
        size_t name_len, const char *prefix, size_t prefix_len, const char *module_key, size_t module_key_len,
        const char *value, size_t value_len, ly_bool *dynamic, LY_VALUE_FORMAT format, void *val_prefix_data, uint32_t hints);

/**
 * @brief Store and canonize the given @p value into @p val according to the schema node type rules.
 *
 * @param[in] ctx libyang context.
 * @param[in,out] val Storage for the value.
 * @param[in] type Type of the value.
 * @param[in] value Value to be parsed, must not be NULL.
 * @param[in] value_len Length of the give @p value, must be set correctly.
 * @param[in,out] dynamic Flag if @p value is dynamically allocated, is adjusted when @p value is consumed.
 * @param[in] format Input format of @p value.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[in] hints [Value hints](@ref lydvalhints) from the parser.
 * @param[in] ctx_node Context schema node.
 * @param[out] incomplete Optional, set if the value also needs to be resolved.
 * @return LY_SUCCESS on success,
 * @return LY_ERR value on error.
 */
LY_ERR lyd_value_store(const struct ly_ctx *ctx, struct lyd_value *val, const struct lysc_type *type, const void *value,
        size_t value_len, ly_bool *dynamic, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *ctx_node, ly_bool *incomplete);

/**
 * @brief Validate previously incompletely stored value.
 *
 * @param[in] ctx libyang context.
 * @param[in] type Schema type of the value (not the stored one, but the original one).
 * @param[in,out] val Stored value to resolve.
 * @param[in] ctx_node Context node for the resolution.
 * @param[in] tree Data tree for the resolution.
 * @return LY_SUCCESS on success,
 * @return LY_ERR value on error.
 */
LY_ERR lyd_value_validate_incomplete(const struct ly_ctx *ctx, const struct lysc_type *type, struct lyd_value *val,
        const struct lyd_node *ctx_node, const struct lyd_node *tree);

/**
 * @brief Check type restrictions applicable to the particular leaf/leaf-list with the given string @p value coming
 * from a schema.
 *
 * This function check just the type's restriction, if you want to check also the data tree context (e.g. in case of
 * require-instance restriction), use ::lyd_value_validate().
 *
 * @param[in] ctx libyang context for logging (function does not log errors when @p ctx is NULL)
 * @param[in] node Schema node for the @p value.
 * @param[in] value String value to be checked, expected to be in JSON format.
 * @param[in] value_len Length of the given @p value (mandatory).
 * @param[in] format Value prefix format.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @return LY_SUCCESS on success
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lys_value_validate(const struct ly_ctx *ctx, const struct lysc_node *node, const char *value, size_t value_len,
        LY_VALUE_FORMAT format, void *prefix_data);

/**
 * @defgroup datahash Data nodes hash manipulation
 * @ingroup datatree
 */

/**
 * @brief Generate hash for the node.
 *
 * @param[in] node Data node to (re)generate hash value.
 * @return LY_ERR value.
 */
LY_ERR lyd_hash(struct lyd_node *node);

/**
 * @brief Insert hash of the node into the hash table of its parent.
 *
 * @param[in] node Data node which hash will be inserted into the ::lyd_node_inner.children_ht hash table of its parent.
 * @return LY_ERR value.
 */
LY_ERR lyd_insert_hash(struct lyd_node *node);

/**
 * @brief Maintain node's parent's children hash table when unlinking the node.
 *
 * When completely freeing data tree, it is expected to free the parent's children hash table first, at once.
 *
 * @param[in] node The data node being unlinked from its parent.
 */
void lyd_unlink_hash(struct lyd_node *node);

/** @} datahash */

/**
 * @brief Iterate over implemented modules for functions that accept specific modules or the whole context.
 *
 * @param[in] tree Data tree.
 * @param[in] module Selected module, NULL for all.
 * @param[in] ctx Context, NULL for selected modules.
 * @param[in,out] i Iterator, set to 0 on first call.
 * @param[out] first First sibling of the returned module.
 * @return Next module.
 * @return NULL if all modules were traversed.
 */
const struct lys_module *lyd_mod_next_module(struct lyd_node *tree, const struct lys_module *module,
        const struct ly_ctx *ctx, uint32_t *i, struct lyd_node **first);

/**
 * @brief Iterate over modules for functions that want to traverse all the top-level data.
 *
 * @param[in,out] next Pointer to the next module data, set to first top-level sibling on first call.
 * @param[out] first First sibling of the returned module.
 * @return Next module.
 * @return NULL if all modules were traversed.
 */
const struct lys_module *lyd_data_next_module(struct lyd_node **next, struct lyd_node **first);

/**
 * @brief Check that a list has all its keys.
 *
 * @param[in] node List to check.
 * @return LY_SUCCESS on success.
 * @return LY_ENOT on a missing key.
 */
LY_ERR lyd_parse_check_keys(struct lyd_node *node);

/**
 * @brief Set data flags for a newly parsed node.
 *
 * @param[in] node Node to use.
 * @param[in,out] when_check Set of nodes with unresolved when.
 * @param[in,out] exts_check Set of nodes and their extension instances if they have own validation callback.
 * @param[in,out] meta Node metadata, may be removed from.
 * @param[in] options Parse options.
 */
void lyd_parse_set_data_flags(struct lyd_node *node, struct ly_set *when_check, struct ly_set *exts_check,
        struct lyd_meta **meta, uint32_t options);

/**
 * @brief Append all list key predicates to path.
 *
 * @param[in] node Node with keys to print.
 * @param[in,out] buffer Buffer to print to.
 * @param[in,out] buflen Current buffer length.
 * @param[in,out] bufused Current number of characters used in @p buffer.
 * @param[in] is_static Whether buffer is static or can be reallocated.
 * @return LY_ERR
 */
LY_ERR lyd_path_list_predicate(const struct lyd_node *node, char **buffer, size_t *buflen, size_t *bufused, ly_bool is_static);

/**
 * @brief Free stored prefix data.
 *
 * @param[in] format Format of the prefixes.
 * @param[in] prefix_data Format-specific data to free:
 *      LY_PREF_SCHEMA          - const struct lysp_module * (module used for resolving prefixes from imports)
 *      LY_PREF_SCHEMA_RESOLVED - struct lyd_value_prefix * (sized array of pairs: prefix - module)
 *      LY_PREF_XML             - const struct ly_set * (set with defined namespaces stored as ::lyxml_ns)
 *      LY_PREF_JSON            - NULL
 */
void ly_free_prefix_data(LY_VALUE_FORMAT format, void *prefix_data);

/**
 * @brief Duplicate prefix data.
 *
 * @param[in] ctx libyang context.
 * @param[in] format Format of the prefixes in the value.
 * @param[in] prefix_data Prefix data to duplicate.
 * @param[out] prefix_data_p Duplicated prefix data.
 * @return LY_ERR value.
 */
LY_ERR ly_dup_prefix_data(const struct ly_ctx *ctx, LY_VALUE_FORMAT format, const void *prefix_data, void **prefix_data_p);

/**
 * @brief Store used prefixes in a string.
 *
 * If @p prefix_data_p are non-NULL, they are treated as valid according to the @p format_p and new possible
 * prefixes are simply added. This way it is possible to store prefix data for several strings together.
 *
 * @param[in] ctx libyang context.
 * @param[in] value Value to be parsed.
 * @param[in] value_len Length of the @p value.
 * @param[in] format Format of the prefixes in the value.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[in,out] format_p Resulting format of the prefixes.
 * @param[in,out] prefix_data_p Resulting prefix data for the value in format @p format_p.
 * @return LY_ERR value.
 */
LY_ERR ly_store_prefix_data(const struct ly_ctx *ctx, const void *value, size_t value_len, LY_VALUE_FORMAT format,
        const void *prefix_data, LY_VALUE_FORMAT *format_p, void **prefix_data_p);

/**
 * @brief Get string name of the format.
 *
 * @param[in] format Format whose name to get.
 * @return Format string name.
 */
const char *ly_format2str(LY_VALUE_FORMAT format);

#endif /* LY_TREE_DATA_INTERNAL_H_ */
