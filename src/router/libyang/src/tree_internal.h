/**
 * @file tree_internal.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang internal functions for manipulating with the data model and
 * data trees.
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_INTERNAL_H_
#define LY_TREE_INTERNAL_H_

#include <stdint.h>

#include "libyang.h"
#include "tree_schema.h"
#include "tree_data.h"
#include "resolve.h"

/* this is used to distinguish lyxml_elem * from a YANG temporary parsing structure, the first byte is compared */
#define LY_YANG_STRUCTURE_FLAG 0x80

/**
 * @brief YANG namespace
 */
#define LY_NSYANG "urn:ietf:params:xml:ns:yang:1"

/**
 * @brief YIN namespace
 */
#define LY_NSYIN "urn:ietf:params:xml:ns:yang:yin:1"

/**
 * @brief NETCONF namespace
 */
#define LY_NSNC "urn:ietf:params:xml:ns:netconf:base:1.0"

/**
 * @brief NACM namespace
 */
#define LY_NSNACM "urn:ietf:params:xml:ns:yang:ietf-netconf-acm"

/**
 * @brief internal parser flag for actions and inline notifications
 */
#define LYD_OPT_ACT_NOTIF 0x100

/**
 * @brief Internal list of built-in types
 */
extern struct lys_tpdf *ly_types[LY_DATA_TYPE_COUNT];

/**
 * @brief Internal structure for data node sorting.
 */
struct lyd_node_pos {
    struct lyd_node *node;
    uint32_t pos;
};

/**
 * @brief Internal structure for LYB parser/printer.
 */
struct lyb_state {
    size_t *written;
    size_t *position;
    uint8_t *inner_chunks;
    int used;
    int size;
    const struct lys_module **models;
    int mod_count;
    struct ly_ctx *ctx;

    /* LYB printer only */
    struct {
        struct lys_node *first_sibling;
        struct hash_table *ht;
    } *sib_ht;
    int sib_ht_count;
};

/* struct lyb_state allocation step */
#define LYB_STATE_STEP 4

/**
 * LYB schema hash constants
 *
 * Hash is divided to collision ID and hash itself.
 *
 * First bits are collision ID until 1 is found. The rest is truncated 32b hash.
 * 1xxx xxxx - collision ID 0 (no collisions)
 * 01xx xxxx - collision ID 1 (collision ID 0 hash collided)
 * 001x xxxx - collision ID 2 ...
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

/* Type large enough for all meta data */
#define LYB_META uint16_t

LYB_HASH lyb_hash(struct lys_node *sibling, uint8_t collision_id);

int lyb_has_schema_model(struct lys_node *sibling, const struct lys_module **models, int mod_count);

/**
 * Macros to work with ::lyd_node#when_status
 * +--- bit 1 - some when-stmt connected with the node (resolve_applies_when() is true)
 * |+-- bit 2 - when-stmt's condition is resolved and it is true
 * ||+- bit 3 - when-stmt's condition is resolved and it is false
 * XXX
 *
 * bit 1 is set when the node is created
 * if none of bits 2 and 3 is set, the when condition is not yet resolved
 */
#define LYD_WHEN       0x04
#define LYD_WHEN_TRUE  0x02
#define LYD_WHEN_FALSE 0x01
#define LYD_WHEN_DONE(status) (!((status) & LYD_WHEN) || ((status) & (LYD_WHEN_TRUE | LYD_WHEN_FALSE)))

/**
 * @brief Type flag for an unresolved type in a grouping.
 */
#define LY_VALUE_UNRESGRP 0x80

#ifdef LY_ENABLED_CACHE

/**
 * @brief Minimum number of children for the parent to create a hash table for them.
 */
#   define LY_CACHE_HT_MIN_CHILDREN 4

    int lyd_hash(struct lyd_node *node);

    void lyd_insert_hash(struct lyd_node *node);

    void lyd_unlink_hash(struct lyd_node *node, struct lyd_node *orig_parent);
#endif

/**
 * @brief Create submodule structure by reading data from memory.
 *
 * @param[in] module Schema tree where to connect the submodule, belongs-to value must match.
 * @param[in] data String containing the submodule specification in the given \p format.
 * @param[in] format Format of the data to read.
 * @param[in] unres list of unresolved items
 * @return Created submodule structure or NULL in case of error.
 */
struct lys_submodule *lys_sub_parse_mem(struct lys_module *module, const char *data, LYS_INFORMAT format,
                                        struct unres_schema *unres);

/**
 * @brief Create submodule structure by reading data from file descriptor.
 *
 * \note Current implementation supports only reading data from standard (disk) file, not from sockets, pipes, etc.
 *
 * @param[in] module Schema tree where to connect the submodule, belongs-to value must match.
 * @param[in] fd File descriptor of a regular file (e.g. sockets are not supported) containing the submodule
 *            specification in the given \p format.
 * @param[in] format Format of the data to read.
 * @param[in] unres list of unresolved items
 * @return Created submodule structure or NULL in case of error.
 */
struct lys_submodule *lys_sub_parse_fd(struct lys_module *module, int fd, LYS_INFORMAT format, struct unres_schema *unres);

/**
 * @brief Free the submodule structure
 *
 * @param[in] submodule The structure to free. Do not use the pointer after calling this function.
 * @param[in] private_destructor Optional destructor function for private objects assigned
 * to the nodes via lys_set_private(). If NULL, the private objects are not freed by libyang.
 */
void lys_submodule_free(struct lys_submodule *submodule, void (*private_destructor)(const struct lys_node *node, void *priv));

/**
 * @brief Add child schema tree node at the end of the parent's child list.
 *
 * If the child is connected somewhere (has a parent), it is completely
 * unlinked and none of the following conditions applies.
 * If the child has prev sibling(s), they are ignored (child is added at the
 * end of the child list).
 * If the child has next sibling(s), all of them are connected with the parent.
 *
 * @param[in] parent Parent node where the \p child will be added.
 * @param[in] module Module where the \p child will be added if the \p parent
 * parameter is NULL (case of top-level elements). The parameter does not change
 * the module of the \p child element. If the \p parent parameter is present,
 * the \p module parameter is ignored.
 * @param[in] child The schema tree node to be added.
 * @param[in] options Parsing options. Only relevant when creating a shorthand case.
 * @return 0 on success, nonzero else
 */
int lys_node_addchild(struct lys_node *parent, struct lys_module *module, struct lys_node *child, int options);

/**
 * @brief Find a valid grouping definition relative to a node.
 *
 * Valid definition means a sibling of \p start or a sibling of any of \p start 's parents.
 *
 * @param[in] name Name of the searched grouping.
 * @param[in] start Definition must be valid (visible) for this node.
 * @return Matching valid grouping or NULL.
 */
struct lys_node_grp *lys_find_grouping_up(const char *name, struct lys_node *start);

/**
 * @brief Check that the \p node being connected into the \p parent has a unique name (identifier).
 *
 * Function is performed also as part of lys_node_addchild().
 *
 * @param[in] node The schema tree node to be checked.
 * @param[in] parent Parent node where the \p child is supposed to be added.
 * @param[in] module Module where the \p child is supposed to be added if the \p parent
 * parameter is NULL (case of top-level elements). The parameter does not change
 * the module of the \p child element. If the \p parent parameter is present,
 * the \p module parameter is ignored.
 * @return 0 on success, nonzero else
 */
int lys_check_id(struct lys_node *node, struct lys_node *parent, struct lys_module *module);

/**
 * @brief Get know if the node contains must or when with XPath expression
 *
 * @param[in] node Node to examine.
 * @return 1 if contains, 0 otherwise
 */
int lys_has_xpath(const struct lys_node *node);

/**
 * @brief Learn if \p type is defined in the local module or from an import.
 *
 * @param[in] type Type to examine.
 * @return non-zero if local, 0 if from an import.
 */
int lys_type_is_local(const struct lys_type *type);

/**
 * @brief Create a copy of the specified schema tree \p node
 *
 * @param[in] module Target module for the duplicated node.
 * @param[in] parent Schema tree node where the node is being connected, NULL in case of top level \p node.
 * @param[in] node Schema tree node to be duplicated.
 * @param[in] unres list of unresolved items
 * @param[in] shallow Whether to copy children and connect to parent/module too.
 * @return Created copy of the provided schema \p node.
 */
struct lys_node *lys_node_dup(struct lys_module *module, struct lys_node *parent, const struct lys_node *node,
                              struct unres_schema *unres, int shallow);

/**
 * @brief duplicate the list of extension instances.
 *
 * @param[in] ctx Context to store errors in.
 * @param[in] mod Module where we are
 * @param[in] orig list of the extension instances to duplicate, the size of the array must correspond with \p size
 * @param[in] size number of items in \p old array to duplicate
 * @param[in] parent Parent structure of the new extension instances list
 * @param[in] parent_type Type of the provide \p parent *
 * @param[in,out] new Address where to store the created list of duplicated extension instances
 * @param[in] shallow Whether to copy children and connect to parent/module too.
 * @param[in] unres list of unresolved items
 *
 */
int lys_ext_dup(struct ly_ctx *ctx, struct lys_module *mod, struct lys_ext_instance **orig, uint8_t size, void *parent,
                LYEXT_PAR parent_type, struct lys_ext_instance ***new, int shallow, struct unres_schema *unres);

/**
 * @brief Iterate over the specified type of the extension instances
 *
 * @param[in] ext Array of extensions to explore
 * @param[in] ext_size Size of the provided \p ext array
 * @param[in] start Index in the \p ext array where to start searching (first call with 0, the consequent calls with
 *            the returned index increased by 1, unless the returned index is -1)
 * @param[in] substmt Type of the extension (its belongins to the specific substatement) to iterate, use
 *            #LYEXT_SUBSTMT_ALL to go through all the extensions in the array
 * @result index in the ext, -1 if not present
 */
int lys_ext_iter(struct lys_ext_instance **ext, uint8_t ext_size, uint8_t start, LYEXT_SUBSTMT substmt);

/**
 * @brief free the array of the extension instances
 */
void lys_extension_instances_free(struct ly_ctx *ctx, struct lys_ext_instance **e, unsigned int size,
                                  void (*private_destructor)(const struct lys_node *node, void *priv));

/**
 * @brief Check that leafref and its target have allowed config combination and there are no cyclic references.
 *
 * @param[in] leafref_target Leaf that is \p leafref's target.
 * @param[in] leafref Leaf or leaflist of type #LY_TYPE_LEAFREF referring \p leafref_target.
 * @return 0 on success, -1 on error.
 */
int lys_leaf_check_leafref(struct lys_node_leaf *leafref_target, struct lys_node *leafref);

/**
 * @brief Free a schema when condition
 *
 * @param[in] libyang context where the schema of the ondition is used.
 * @param[in] w When structure to free.
 * @param[in] private_destructor Destructor for priv member in extension instances
 */
void lys_when_free(struct ly_ctx *ctx, struct lys_when *w,
                   void (*private_destructor)(const struct lys_node *node, void *priv));

/**
 * @brief Free the schema tree restriction (must, ...) structure content
 *
 * @param[in] ctx libyang context where the schema of the restriction is used.
 * @param[in] restr The restriction structure to free. The function actually frees only
 * the content of the structure, so after using this function, caller is supposed to
 * use free(restr). It is done to free the content of structures being allocated as
 * part of array, in that case the free() is used on the whole array.
 * @param[in] private_destructor Destructor for priv member in extension instances
 */
void lys_restr_free(struct ly_ctx *ctx, struct lys_restr *restr,
                    void (*private_destructor)(const struct lys_node *node, void *priv));

/**
 * @brief Free the schema tree type structure content
 *
 * @param[in] ctx libyang context where the schema of the type is used.
 * @param[in] restr The type structure to free. The function actually frees only
 * the content of the structure, so after using this function, caller is supposed to
 * use free(type). It is done to free the content of structures being allocated as
 * part of array, in that case the free() is used on the whole array.
 * @param[in] private_destructor Destructor for priv member in extension instances
 */
void lys_type_free(struct ly_ctx *ctx, struct lys_type *type,
                   void (*private_destructor)(const struct lys_node *node, void *priv));

/**
 * @brief Unlink the schema node from the tree.
 *
 * @param[in] node Schema tree node to unlink.
 */
void lys_node_unlink(struct lys_node *node);

/**
 * @brief Free the schema node structure, includes unlinking it from the tree
 *
 * @param[in] node Schema tree node to free. Do not use the pointer after calling this function.
 * @param[in] private_destructor Optional destructor function for private objects assigned
 * to the nodes via lys_set_private(). If NULL, the private objects are not freed by libyang.
 * @param[in] shallow Whether to do a shallow free only (on a shallow copy of a node).
 */
void lys_node_free(struct lys_node *node, void (*private_destructor)(const struct lys_node *node, void *priv), int shallow);

/**
 * @brief Free (and unlink it from the context) the specified schema.
 *
 * It is dangerous to call this function on schemas already placed into the context's
 * list of modules - there can be many references from other modules and data instances.
 *
 * @param[in] module Data model to free.
 * @param[in] private_destructor Optional destructor function for private objects assigned
 * to the nodes via lys_set_private(). If NULL, the private objects are not freed by libyang.
 * @param[in] free_subs Whether to free included submodules.
 * @param[in] remove_from_ctx Whether to remove this model from context. Always use 1 except
 * when removing all the models (in ly_ctx_destroy()).
 */
void lys_free(struct lys_module *module, void (*private_destructor)(const struct lys_node *node, void *priv),
              int free_subs, int remove_from_ctx);

/**
 * @brief Create a data container knowing it's schema node.
 *
 * @param[in] parent Data parent of the new node.
 * @param[in] schema Schema node of the new node.
 * @param[in] dflt Set dflt flag in the created data nodes
 * @return New node, NULL on error.
 */
struct lyd_node *_lyd_new(struct lyd_node *parent, const struct lys_node *schema, int dflt);

/**
 * @brief Find the parent node of an attribute.
 *
 * @param[in] root Root element of the data tree with the attribute.
 * @param[in] attr Attribute to find.
 *
 * @return Parent of \p attr, NULL if not found.
 */
const struct lyd_node *lyd_attr_parent(const struct lyd_node *root, struct lyd_attr *attr);

/**
 * @brief Internal version of lyd_unlink().
 *
 * @param[in] node Node to unlink.
 * @param[in] permanent 0 - the node will be linked back,
 *                      1 - the node is premanently unlinked,
 *                      2 - the node is being freed.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
int lyd_unlink_internal(struct lyd_node *node, int permanent);

/**
 * @brief Get the canonical value.
 *
 * @param[in] schema Leaf or leaf-list schema node of the value.
 * @param[in] val_str String value to transform.
 * @param[in] val_str_len String value length.
 * @return Canonical value (must be freed), NULL on error.
 */
char *lyd_make_canonical(const struct lys_node *schema, const char *val_str, int val_str_len);

/**
 * @brief Internal version of lyd_insert() and lyd_insert_sibling().
 *
 * @param[in] invalidate Whether to invalidate any nodes. Set 0 only if linking back some temporarily internally unlinked nodes.
 */
int lyd_insert_common(struct lyd_node *parent, struct lyd_node **sibling, struct lyd_node *node, int invalidate);

/**
 * @brief Internal version of lyd_insert_before() and lyd_insert_after().
 *
 * @param[in] invalidate Whether to invalidate any nodes. Set 0 only if linking back some temporarily internally unlinked nodes.
 */
int lyd_insert_nextto(struct lyd_node *sibling, struct lyd_node *node, int before, int invalidate);

/**
 * @brief Find a specific sibling. Does not log.
 *
 * Since \p mod_name is mandatory, augments are handled.
 *
 * @param[in] siblings Siblings to consider. They are first adjusted to
 *                     point to the first sibling.
 * @param[in] mod_name Module name, mandatory.
 * @param[in] mod_name_len Module name length.
 * @param[in] name Node name, mandatory.
 * @param[in] nam_len Node name length.
 * @param[in] type ORed desired type of the node. 0 means any type.
 *                 Does not return groupings, uses, and augments (but can return augment nodes).
 * @param[out] ret Pointer to the node of the desired type. Can be NULL.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference.
 */
int lys_get_sibling(const struct lys_node *siblings, const char *mod_name, int mod_name_len, const char *name,
                    int nam_len, LYS_NODE type, const struct lys_node **ret);

/**
 * @brief Find a specific node that can only appear in the data. Does not log.
 *
 * @param[in] mod Main module with the node. Must be set if \p parent == NULL (top-level node).
 * @param[in] parent Parent of the node. Must be set if \p mod == NULL (nested node).
 * @param[in] name Node name.
 * @param[in] nam_len Node \p name length.
 * @param[in] type ORed desired type of the node. 0 means any (data node) type.
 * @param[in] getnext_opts lys_getnext() options to use.
 * @param[out] ret Pointer to the node of the desired type. Can be NULL.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on fail.
 */
int lys_getnext_data(const struct lys_module *mod, const struct lys_node *parent, const char *name, int nam_len,
                     LYS_NODE type, int getnext_opts, const struct lys_node **ret);

int lyd_get_unique_default(const char* unique_expr, struct lyd_node *list, const char **dflt);

int lyd_build_relative_data_path(const struct lys_module *module, const struct lyd_node *node, const char *schema_id,
                                 char *buf);

void lyd_free_value(lyd_val value, LY_DATA_TYPE value_type, uint8_t value_flags, struct lys_type *type,
                    const char *value_str, lyd_val *old_val, LY_DATA_TYPE *old_val_type, uint8_t *old_val_flags);

int lyd_list_equal(struct lyd_node *node1, struct lyd_node *node2, int with_defaults);

int lys_make_implemented_r(struct lys_module *module, struct unres_schema *unres);

/**
 * @brief Check for (validate) mandatory nodes of a data tree. Checks recursively whole data tree. Requires all when
 * statement to be solved.
 *
 * @param[in] root Data tree to validate.
 * @param[in] ctx libyang context (for the case when the data tree is empty - i.e. root == NULL).
 * @param[in] modules Only check mandatory nodes from these modules. If not set, check for all modules in the context.
 * @param[in] mod_count Number of modules in \p modules.
 * @param[in] options Standard @ref parseroptions.
 * @return EXIT_SUCCESS or EXIT_FAILURE.
 */
int lyd_check_mandatory_tree(struct lyd_node *root, struct ly_ctx *ctx, const struct lys_module **modules, int mod_count,
                             int options);

/**
 * @brief Check if the provided node is inside a grouping.
 *
 * @param[in] node Schema node to check.
 * @return 0 as false, 1 as true
 */
int lys_ingrouping(const struct lys_node *node);

int unres_data_diff_new(struct unres_data *unres, struct lyd_node *subtree, struct lyd_node *parent, int created);

void unres_data_diff_rem(struct unres_data *unres, unsigned int idx);

/**
 * @brief Process (add/clean) default nodes in the data tree and resolve the unresolved items
 *
 * @param[in,out] root  Pointer to the root node of the complete data tree, the root node can be NULL if the data tree
 *                      is empty
 * @param[in] options   Parser options to know the data tree type, see @ref parseroptions.
 * @param[in] ctx       Context for the case the \p root is empty (in that case \p ctx must not be NULL)
 * @param[in] modules   Only modules that will be traversed when adding default values.
 * @param[in] mod_count Number of module names in \p modules.
 * @param[in] data_tree Additional data tree for validating RPC/action/notification. The tree is used to satisfy
 *                      possible references to the datastore content.
 * @param[in] act_notif In case of nested action/notification, the subtree of the action/notification. Note
 *                      that in this case the \p root points to the top level data tree node which provides the context
 *                      for the nested action/notification
 * @param[in] unres     Unresolved data list, the newly added default nodes may need to add some unresolved items
 * @param[in] wd        Whether to add default values.
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int lyd_defaults_add_unres(struct lyd_node **root, int options, struct ly_ctx *ctx, const struct lys_module **modules,
                           int mod_count, const struct lyd_node *data_tree, struct lyd_node *act_notif,
                           struct unres_data *unres, int wd);

void lys_enable_deviations(struct lys_module *module);

void lys_disable_deviations(struct lys_module *module);

void lys_sub_module_remove_devs_augs(struct lys_module *module);

void lys_sub_module_apply_devs_augs(struct lys_module *module);

int apply_aug(struct lys_node_augment *augment, struct unres_schema *unres);

void lys_submodule_module_data_free(struct lys_submodule *submodule);

int lys_copy_union_leafrefs(struct lys_module *mod, struct lys_node *parent, struct lys_type *type,
                            struct lys_type *prev_new, struct unres_schema *unres);

const struct lys_module *lys_parse_fd_(struct ly_ctx *ctx, int fd, LYS_INFORMAT format, const char *revision, int implement);

const struct lys_module *lys_parse_mem_(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format, const char *revision,
                                        int internal, int implement);

/**
 * @brief Get next augment from \p mod augmenting \p aug_target
 */
struct lys_node_augment *lys_getnext_target_aug(struct lys_node_augment *last, const struct lys_module *mod,
                                                const struct lys_node *aug_target);

LY_STMT lys_snode2stmt(LYS_NODE nodetype);
struct lys_node ** lys_child(const struct lys_node *node, LYS_NODE nodetype);

#endif /* LY_TREE_INTERNAL_H_ */
