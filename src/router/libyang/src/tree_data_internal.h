/**
 * @file tree_data_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief internal functions for YANG schema trees.
 *
 * Copyright (c) 2015 - 2023 CESNET, z.s.p.o.
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
#include "tree_data.h"

#include <stddef.h>

struct ly_path_predicate;
struct lyd_ctx;
struct lysc_module;

#define LY_XML_SUFFIX ".xml"
#define LY_XML_SUFFIX_LEN 4
#define LY_JSON_SUFFIX ".json"
#define LY_JSON_SUFFIX_LEN 5
#define LY_LYB_SUFFIX ".lyb"
#define LY_LYB_SUFFIX_LEN 4

/**
 * @brief Internal item structure for remembering "used" instances of duplicate node instances.
 */
struct lyd_dup_inst {
    struct ly_set *set;
    uint32_t used;
};

/**
 * @brief Update a found inst using a duplicate instance cache hash table. Needs to be called for every "used"
 * (that should not be considered next time) instance.
 *
 * @param[in,out] inst Found instance, is updated so that the same instance is not returned twice.
 * @param[in] siblings Siblings where @p inst was found.
 * @param[in] dup_inst_ht Duplicate instance cache hash table.
 * @return LY_ERR value.
 */
LY_ERR lyd_dup_inst_next(struct lyd_node **inst, const struct lyd_node *siblings, struct ly_ht **dup_inst_ht);

/**
 * @brief Free duplicate instance cache.
 *
 * @param[in] dup_inst Duplicate instance cache hash table to free.
 */
void lyd_dup_inst_free(struct ly_ht *dup_inst_ht);

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
        const struct lysc_node **slast, const struct lysc_node *parent, const struct lysc_module *module);

/**
 * @brief Get address of a node's child pointer if any.
 *
 * @param[in] node Node to check.
 * @return Address of the node's child member,
 * @return NULL if there is no child pointer.
 */
struct lyd_node **lyd_node_child_p(struct lyd_node *node);

/**
 * @brief Update node pointer to point to the first data node of a module, leave unchanged if there is none.
 *
 * @param[in,out] node Node pointer, may be updated.
 * @param[in] mod Module whose data to search for.
 */
void lyd_first_module_sibling(struct lyd_node **node, const struct lys_module *mod);

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
 * @brief Set dflt flag for a NP container if applicable, recursively for parents.
 *
 * @param[in] node Node whose criteria for the dflt flag has changed.
 */
void lyd_cont_set_dflt(struct lyd_node *node);

/**
 * @brief Search in the given siblings (NOT recursively) for the first schema node data instance.
 * Uses hashes - should be used whenever possible for best performance.
 *
 * @param[in] siblings Siblings to search in including preceding and succeeding nodes.
 * @param[in] schema Target data node schema to find.
 * @param[out] match Can be NULL, otherwise the found data node.
 * @return LY_SUCCESS on success, @p match set.
 * @return LY_ENOTFOUND if not found, @p match set to NULL.
 * @return LY_ERR value if another error occurred.
 */
LY_ERR lyd_find_sibling_schema(const struct lyd_node *siblings, const struct lysc_node *schema, struct lyd_node **match);

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
 * @brief Try to get schema node for data with a parent based on an extension instance.
 *
 * @param[in] parent Parsed parent data node. Set if @p sparent is NULL.
 * @param[in] sparent Schema parent node. Set if @p sparent is NULL.
 * @param[in] prefix Element prefix, if any.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in] format Format of @p prefix.
 * @param[in] prefix_data Format-specific data.
 * @param[in] name Element name.
 * @param[in] name_len Length of @p name.
 * @param[out] snode Found schema node, NULL if no suitable was found.
 * @param[out] ext Optional extension instance that provided @p snode.
 * @return LY_SUCCESS on success;
 * @return LY_ENOT if no extension instance parsed the data;
 * @return LY_ERR on error.
 */
LY_ERR ly_nested_ext_schema(const struct lyd_node *parent, const struct lysc_node *sparent, const char *prefix,
        size_t prefix_len, LY_VALUE_FORMAT format, void *prefix_data, const char *name, size_t name_len,
        const struct lysc_node **snode, struct lysc_ext_instance **ext);

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

/**
 * @brief Create a term (leaf/leaf-list) node from a string value.
 *
 * Hash is calculated and new node flag is set.
 *
 * @param[in] schema Schema node of the new data node.
 * @param[in] value String value to be parsed.
 * @param[in] value_len Length of @p value, must be set correctly.
 * @param[in] is_utf8 Whether @p value is a valid UTF-8 string, if applicable.
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
LY_ERR lyd_create_term(const struct lysc_node *schema, const char *value, size_t value_len, ly_bool is_utf8,
        ly_bool *dynamic, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, ly_bool *incomplete,
        struct lyd_node **node);

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
 * @param[in] vars Array of defined variables to use in predicates, may be NULL.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_list(const struct lysc_node *schema, const struct ly_path_predicate *predicates,
        const struct lyxp_var *vars, struct lyd_node **node);

/**
 * @brief Create a list with all its keys (cannot be used for key-less list).
 *
 * Hash is calculated and new node flag is set.
 *
 * @param[in] schema Schema node of the new data node.
 * @param[in] keys Key list predicates.
 * @param[in] keys_len Length of @p keys.
 * @param[out] node Created node.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_list2(const struct lysc_node *schema, const char *keys, size_t keys_len, struct lyd_node **node);

/**
 * @brief Create an anyxml/anydata node.
 *
 * Hash is calculated and flags are properly set based on @p is_valid.
 *
 * @param[in] schema Schema node of the new data node.
 * @param[in] value Value of the any node.
 * @param[in] value_type Value type of the value.
 * @param[in] use_value Whether to use dynamic @p value or duplicate it.
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
 * @param[in] node_types Optional set to add nodes with unresolved types into.
 * @param[in] ext_node Optional set to add nodes with extension instance node callbacks into.
 * @param[in] impl_opts Implicit options (@ref implicitoptions).
 * @param[in,out] diff Validation diff.
 * @return LY_ERR value.
 */
LY_ERR lyd_new_implicit_r(struct lyd_node *parent, struct lyd_node **first, const struct lysc_node *sparent,
        const struct lys_module *mod, struct ly_set *node_when, struct ly_set *node_types, struct ly_set *ext_node,
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
 * @brief Insert node after a sibling.
 *
 * Handles inserting into NP containers and key-less lists.
 *
 * @param[in] sibling Sibling to insert after.
 * @param[in] node Node to insert.
 */
void lyd_insert_after_node(struct lyd_node *sibling, struct lyd_node *node);

/**
 * @brief Insert node before a sibling.
 *
 * Handles inserting into NP containers and key-less lists.
 *
 * @param[in] sibling Sibling to insert before.
 * @param[in] node Node to insert.
 */
void lyd_insert_before_node(struct lyd_node *sibling, struct lyd_node *node);

/**
 * @brief Insert a node into parent/siblings. Order and hashes are fully handled.
 *
 * @param[in] parent Parent to insert into, NULL for top-level sibling.
 * @param[in,out] first_sibling First sibling, NULL if no top-level sibling exist yet. Can be also NULL if @p parent is set.
 * @param[in] node Individual node (without siblings) to insert.
 * @param[in] last If set, do not search for the correct anchor but always insert at the end.
 */
void lyd_insert_node(struct lyd_node *parent, struct lyd_node **first_sibling, struct lyd_node *node, ly_bool last);

/**
 * @brief Unlink the specified data subtree.
 *
 * @param[in] node Data tree node to be unlinked (together with all the children).
 */
void lyd_unlink(struct lyd_node *node);

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
 * @param[in] is_utf8 Whether @p value is a valid UTF-8 string, if applicable.
 * @param[in,out] dynamic Flag if @p value is dynamically allocated, is adjusted when @p value is consumed.
 * @param[in] format Input format of @p value.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[in] hints [Value hints](@ref lydvalhints) from the parser regarding the value type.
 * @param[in] ctx_node Value context node, may be NULL for metadata.
 * @param[in] clear_dflt Whether to clear dflt flag starting from @p parent, recursively all NP containers.
 * @param[out] incomplete Whether the value needs to be resolved.
 * @return LY_SUCCESS on success.
 * @return LY_EINCOMPLETE in case data tree is needed to finish the validation.
 * @return LY_ERR value if an error occurred.
 */
LY_ERR lyd_create_meta(struct lyd_node *parent, struct lyd_meta **meta, const struct lys_module *mod, const char *name,
        size_t name_len, const char *value, size_t value_len, ly_bool is_utf8, ly_bool *dynamic, LY_VALUE_FORMAT format,
        void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node, ly_bool clear_dflt, ly_bool *incomplete);

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
 * @param[in] is_utf8 Whether @p value is a valid UTF-8 string, if applicable.
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
        size_t value_len, ly_bool is_utf8, ly_bool *dynamic, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
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
 * @brief Check type restrictions applicable to the particular leaf/leaf-list with the given string @p value.
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
 * @param[in] hints Value encoding hints.
 * @return LY_SUCCESS on success
 * @return LY_ERR value if an error occurred.
 */
LY_ERR ly_value_validate(const struct ly_ctx *ctx, const struct lysc_node *node, const char *value, size_t value_len,
        LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints);

/**
 * @defgroup datahash Data nodes hash manipulation
 * @ingroup datatree
 * @{
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
 * @brief Append all list key predicates to path.
 *
 * @param[in] node Node with keys to print.
 * @param[in,out] buffer Buffer to print to.
 * @param[in,out] buflen Current buffer length.
 * @param[in,out] bufused Current number of characters used in @p buffer.
 * @param[in] is_static Whether buffer is static or can be reallocated.
 * @return LY_ERR value.
 */
LY_ERR lyd_path_list_predicate(const struct lyd_node *node, char **buffer, size_t *buflen, size_t *bufused, ly_bool is_static);

/**
 * @brief Generate a path similar to ::lyd_path() except read the parents from a set.
 *
 * @param[in] dnodes Set with the data nodes, from parent to the last descendant.
 * @param[in] pathtype Type of data path to generate.
 * @return Generated data path.
 */
char *lyd_path_set(const struct ly_set *dnodes, LYD_PATH_TYPE pathtype);

/**
 * @brief Remove an object on the specific set index keeping the order of the other objects.
 *
 * @param[in] set Set from which a node will be removed.
 * @param[in] index Index of the object to remove in the \p set.
 * @param[in] destructor Optional function to free the objects being removed.
 * @return LY_ERR value.
 */
LY_ERR ly_set_rm_index_ordered(struct ly_set *set, uint32_t index, void (*destructor)(void *obj));

#endif /* LY_TREE_DATA_INTERNAL_H_ */
