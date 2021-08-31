/**
 * @file parser_json.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief JSON data parser for libyang
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "in_internal.h"
#include "json.h"
#include "log.h"
#include "parser_data.h"
#include "parser_internal.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "validation.h"

/**
 * @brief Internal context for JSON YANG data parser.
 *
 * Note that the structure maps to the lyd_ctx which is common for all the data parsers
 */
struct lyd_json_ctx {
    const struct lysc_ext_instance *ext; /**< extension instance possibly changing document root context of the data being parsed */
    uint32_t parse_opts;           /**< various @ref dataparseroptions. */
    uint32_t val_opts;             /**< various @ref datavalidationoptions. */
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

    struct lyjson_ctx *jsonctx;    /**< JSON context */
};

/**
 * @brief Free the JSON data parser context.
 *
 * JSON implementation of lyd_ctx_free_clb().
 */
static void
lyd_json_ctx_free(struct lyd_ctx *lydctx)
{
    struct lyd_json_ctx *ctx = (struct lyd_json_ctx *)lydctx;

    if (lydctx) {
        lyd_ctx_free(lydctx);
        lyjson_ctx_free(ctx->jsonctx);
        free(ctx);
    }
}

/**
 * @brief Parse JSON member-name as [\@][prefix:][name]
 *
 * \@ - metadata flag, maps to 1 in @p is_meta_p
 * prefix - name of the module of the data node
 * name - name of the data node
 *
 * All the output parameter are mandatory. Function only parse the member-name, all the appropriate checks are up to the caller.
 *
 * @param[in] value String to parse
 * @param[in] value_len Length of the @p str.
 * @param[out] name_p Pointer to the beginning of the parsed name.
 * @param[out] name_len_p Pointer to the length of the parsed name.
 * @param[out] prefix_p Pointer to the beginning of the parsed prefix. If the member-name does not contain prefix, result is NULL.
 * @param[out] prefix_len_p Pointer to the length of the parsed prefix. If the member-name does not contain prefix, result is 0.
 * @param[out] is_meta_p Pointer to the metadata flag, set to 1 if the member-name contains \@, 0 otherwise.
 */
static void
lydjson_parse_name(const char *value, size_t value_len, const char **name_p, size_t *name_len_p, const char **prefix_p,
        size_t *prefix_len_p, ly_bool *is_meta_p)
{
    const char *name, *prefix = NULL;
    size_t name_len, prefix_len = 0;
    ly_bool is_meta = 0;

    name = memchr(value, ':', value_len);
    if (name != NULL) {
        prefix = value;
        if (*prefix == '@') {
            is_meta = 1;
            prefix++;
        }
        prefix_len = name - prefix;
        name++;
        name_len = value_len - (prefix_len + 1) - is_meta;
    } else {
        name = value;
        if (name[0] == '@') {
            is_meta = 1;
            name++;
        }
        name_len = value_len - is_meta;
    }

    *name_p = name;
    *name_len_p = name_len;
    *prefix_p = prefix;
    *prefix_len_p = prefix_len;
    *is_meta_p = is_meta;
}

/**
 * @brief Get correct prefix (module_name) inside the @p node.
 *
 * @param[in] node Data node to get inherited prefix.
 * @param[in] local_prefix Local prefix to replace the inherited prefix.
 * @param[in] local_prefix_len Length of the @p local_prefix string. In case of 0, the inherited prefix is taken.
 * @param[out] prefix_p Pointer to the resulting prefix string, Note that the result can be NULL in case of no local prefix
 * and no context @p node to get inherited prefix.
 * @param[out] prefix_len_p Pointer to the length of the resulting @p prefix_p string. Note that the result can be 0 in case
 * of no local prefix and no context @p node to get inherited prefix.
 * @return LY_ERR value.
 */
static LY_ERR
lydjson_get_node_prefix(struct lyd_node *node, const char *local_prefix, size_t local_prefix_len, const char **prefix_p,
        size_t *prefix_len_p)
{
    struct lyd_node_opaq *onode;
    const char *module_name = NULL;

    assert(prefix_p && prefix_len_p);

    if (local_prefix_len) {
        *prefix_p = local_prefix;
        *prefix_len_p = local_prefix_len;
        return LY_SUCCESS;
    }

    *prefix_p = NULL;
    while (node) {
        if (node->schema) {
            *prefix_p = node->schema->module->name;
            break;
        }
        onode = (struct lyd_node_opaq *)node;
        if (onode->name.module_name) {
            *prefix_p = onode->name.module_name;
            break;
        } else if (onode->name.prefix) {
            *prefix_p = onode->name.prefix;
            break;
        }
        node = lyd_parent(node);
    }
    *prefix_len_p = ly_strlen(module_name);

    return LY_SUCCESS;
}

/**
 * @brief Get schema node corresponding to the input parameters.
 *
 * @param[in] lydctx JSON data parser context.
 * @param[in] is_attr Flag if the reference to the node is an attribute, for logging only.
 * @param[in] prefix Requested node's prefix (module name).
 * @param[in] prefix_len Length of the @p prefix.
 * @param[in] name Requested node's name.
 * @param[in] name_len Length of the @p name.
 * @param[in] parent Parent of the node being processed, can be NULL in case of top-level.
 * @param[out] snode_p Pointer to the found schema node corresponding to the input parameters.
 * @return LY_SUCCES on success, note that even in this case the returned value of @p snode_p can be NULL, so the data are expected to be parsed as opaq.
 * @return LY_EVALID on failure, error message is logged
 * @return LY_ENOT in case the input data are expected to be skipped
 */
static LY_ERR
lydjson_get_snode(const struct lyd_json_ctx *lydctx, ly_bool is_attr, const char *prefix, size_t prefix_len, const char *name,
        size_t name_len, const struct lyd_node_inner *parent, const struct lysc_node **snode_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lys_module *mod = NULL;
    uint32_t getnext_opts = lydctx->int_opts & LYD_INTOPT_REPLY ? LYS_GETNEXT_OUTPUT : 0;

    /* init return value */
    *snode_p = NULL;

    LOG_LOCSET(NULL, &parent->node, NULL, NULL);

    /* get the element module */
    if (prefix_len) {
        mod = ly_ctx_get_module_implemented2(lydctx->jsonctx->ctx, prefix, prefix_len);
    } else if (parent) {
        if (parent->schema) {
            mod = parent->schema->module;
        }
    } else {
        LOGVAL(lydctx->jsonctx->ctx, LYVE_SYNTAX_JSON, "Top-level JSON object member \"%.*s\" must be namespace-qualified.",
                (int)(is_attr ? name_len + 1 : name_len), is_attr ? name - 1 : name);
        ret = LY_EVALID;
        goto cleanup;
    }
    if (!mod) {
        if (lydctx->parse_opts & LYD_PARSE_STRICT) {
            LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE, "No module named \"%.*s\" in the context.", (int)prefix_len, prefix);
            ret = LY_EVALID;
            goto cleanup;
        }
        if (!(lydctx->parse_opts & LYD_PARSE_OPAQ)) {
            ret = LY_ENOT;
            goto cleanup;
        }
    }

    /* get the schema node */
    if (mod && (!parent || parent->schema)) {
        if (!parent && lydctx->ext) {
            *snode_p = lysc_ext_find_node(lydctx->ext, mod, name, name_len, 0, getnext_opts);
        } else {
            *snode_p = lys_find_child(parent ? parent->schema : NULL, mod, name, name_len, 0, getnext_opts);
        }
        if (!*snode_p) {
            if (lydctx->parse_opts & LYD_PARSE_STRICT) {
                if (lydctx->ext) {
                    if (lydctx->ext->argument) {
                        LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE, "Node \"%.*s\" not found in the \"%s\" %s extension instance.",
                                (int)name_len, name, lydctx->ext->argument, lydctx->ext->def->name);
                    } else {
                        LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE, "Node \"%.*s\" not found in the %s extension instance.",
                                (int)name_len, name, lydctx->ext->def->name);
                    }
                } else {
                    LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE, "Node \"%.*s\" not found in the \"%s\" module.",
                            (int)name_len, name, mod->name);
                }
                ret = LY_EVALID;
                goto cleanup;
            } else if (!(lydctx->parse_opts & LYD_PARSE_OPAQ)) {
                /* skip element with children */
                ret = LY_ENOT;
                goto cleanup;
            }
        } else {
            /* check that schema node is valid and can be used */
            ret = lyd_parser_check_schema((struct lyd_ctx *)lydctx, *snode_p);
        }
    }

cleanup:
    LOG_LOCBACK(0, parent ? 1 : 0, 0, 0);
    return ret;
}

/**
 * @brief Skip the currently open JSON object/array
 * @param[in] jsonctx JSON context with the input data to skip.
 * @return LY_ERR value.
 */
static LY_ERR
lydjson_data_skip(struct lyjson_ctx *jsonctx)
{
    enum LYJSON_PARSER_STATUS status, current;
    size_t sublevels = 1;

    status = lyjson_ctx_status(jsonctx, 0);

    /* skip after the content */
    do {
        LY_CHECK_RET(lyjson_ctx_next(jsonctx, &current));
        if (current == status) {
            sublevels++;
        } else if (current == status + 1) {
            sublevels--;
        }
    } while (current != status + 1 || sublevels);
    /* open the next sibling */
    LY_CHECK_RET(lyjson_ctx_next(jsonctx, NULL));

    return LY_SUCCESS;
}

/**
 * @brief Check that the input data are parseable as the @p list.
 *
 * Checks for all the list's keys. Function does not revert the context state.
 *
 * @param[in] jsonctx JSON parser context.
 * @param[in] list List schema node corresponding to the input data object.
 * @return LY_SUCCESS in case the data are ok for the @p list
 * @return LY_ENOT in case the input data are not sufficient to fully parse the list instance.
 */
static LY_ERR
lydjson_check_list(struct lyjson_ctx *jsonctx, const struct lysc_node *list)
{
    LY_ERR ret = LY_SUCCESS;
    enum LYJSON_PARSER_STATUS status = lyjson_ctx_status(jsonctx, 0);
    struct ly_set key_set = {0};
    const struct lysc_node *snode;
    uint32_t i, status_count;

    assert(list && (list->nodetype == LYS_LIST));
    assert(status == LYJSON_OBJECT);

    /* get all keys into a set (keys do not have if-features or anything) */
    snode = NULL;
    while ((snode = lys_getnext(snode, list, NULL, 0)) && (snode->flags & LYS_KEY)) {
        ret = ly_set_add(&key_set, (void *)snode, 1, NULL);
        LY_CHECK_GOTO(ret, cleanup);
    }

    if (status != LYJSON_OBJECT_EMPTY) {
        status_count = jsonctx->status.count;

        while (key_set.count && status != LYJSON_OBJECT_CLOSED) {
            const char *name, *prefix;
            size_t name_len, prefix_len;
            ly_bool is_attr;

            /* match the key */
            snode = NULL;
            lydjson_parse_name(jsonctx->value, jsonctx->value_len, &name, &name_len, &prefix, &prefix_len, &is_attr);

            if (!is_attr && !prefix) {
                for (i = 0; i < key_set.count; ++i) {
                    snode = (const struct lysc_node *)key_set.objs[i];
                    if (!ly_strncmp(snode->name, name, name_len)) {
                        break;
                    }
                }
                /* go into the item to a) process it as a key or b) start skipping it as another list child */
                ret = lyjson_ctx_next(jsonctx, &status);
                LY_CHECK_GOTO(ret, cleanup);

                if (snode) {
                    /* we have the key, validate the value */
                    if (status < LYJSON_NUMBER) {
                        /* not a terminal */
                        ret = LY_ENOT;
                        goto cleanup;
                    }

                    ret = lys_value_validate(NULL, snode, jsonctx->value, jsonctx->value_len, LY_VALUE_JSON, NULL);
                    LY_CHECK_GOTO(ret, cleanup);

                    /* key with a valid value, remove from the set */
                    ly_set_rm_index(&key_set, i, NULL);
                }
            } else {
                /* start skipping the member we are not interested in */
                ret = lyjson_ctx_next(jsonctx, &status);
                LY_CHECK_GOTO(ret, cleanup);
            }
            /* move to the next child */
            while (status_count < jsonctx->status.count) {
                ret = lyjson_ctx_next(jsonctx, &status);
                LY_CHECK_GOTO(ret, cleanup);
            }
        }
    }

    if (key_set.count) {
        /* some keys are missing/did not validate */
        ret = LY_ENOT;
    }

cleanup:
    ly_set_erase(&key_set, NULL);
    return ret;
}

/**
 * @brief Get the hint for the data type parsers according to the current JSON parser context.
 *
 * @param[in] lydctx JSON data parser context. The context is supposed to be on a value.
 * @param[in,out] status Pointer to the current context status,
 * in some circumstances the function manipulates with the context so the status is updated.
 * @param[out] type_hint_p Pointer to the variable to store the result.
 * @return LY_SUCCESS in case of success.
 * @return LY_EINVAL in case of invalid context status not referring to a value.
 */
static LY_ERR
lydjson_value_type_hint(struct lyd_json_ctx *lydctx, enum LYJSON_PARSER_STATUS *status_p, uint32_t *type_hint_p)
{
    *type_hint_p = 0;

    if (*status_p == LYJSON_ARRAY) {
        /* only [null] */
        LY_CHECK_RET(lyjson_ctx_next(lydctx->jsonctx, status_p));
        LY_CHECK_RET(*status_p != LYJSON_NULL, LY_EINVAL);

        LY_CHECK_RET(lyjson_ctx_next(lydctx->jsonctx, NULL));
        LY_CHECK_RET(lyjson_ctx_status(lydctx->jsonctx, 0) != LYJSON_ARRAY_CLOSED, LY_EINVAL);

        *type_hint_p = LYD_VALHINT_EMPTY;
    } else if (*status_p == LYJSON_STRING) {
        *type_hint_p = LYD_VALHINT_STRING | LYD_VALHINT_NUM64;
    } else if (*status_p == LYJSON_NUMBER) {
        *type_hint_p = LYD_VALHINT_DECNUM;
    } else if ((*status_p == LYJSON_FALSE) || (*status_p == LYJSON_TRUE)) {
        *type_hint_p = LYD_VALHINT_BOOLEAN;
    } else if (*status_p == LYJSON_NULL) {
        *type_hint_p = 0;
    } else {
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

/**
 * @brief Check in advance if the input data are parsable according to the provided @p snode.
 *
 * Note that the checks are done only in case the LYD_PARSE_OPAQ is allowed. Otherwise the same checking
 * is naturally done when the data are really parsed.
 *
 * @param[in] lydctx JSON data parser context. When the function returns, the context is in the same state
 * as before calling, despite it is necessary to process input data for checking.
 * @param[in] snode Schema node corresponding to the member currently being processed in the context.
 * @param[out] type_hint_p Pointer to a variable to store detected value type hint in case of leaf or leaf-list.
 * @return LY_SUCCESS in case the data are ok for the @p snode or the LYD_PARSE_OPAQ is not enabled.
 * @return LY_ENOT in case the input data are not sufficient to fully parse the list instance
 * @return LY_EINVAL in case of invalid leaf JSON encoding
 * and they are expected to be parsed as opaq nodes.
 */
static LY_ERR
lydjson_data_check_opaq(struct lyd_json_ctx *lydctx, const struct lysc_node *snode, uint32_t *type_hint_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyjson_ctx *jsonctx = lydctx->jsonctx;
    enum LYJSON_PARSER_STATUS status;

    assert(snode);

    if (!(snode->nodetype & (LYD_NODE_TERM | LYS_LIST))) {
        /* can always be parsed as a data node if we have the schema node */
        return LY_SUCCESS;
    }

    if (lydctx->parse_opts & LYD_PARSE_OPAQ) {
        /* backup parser */
        lyjson_ctx_backup(jsonctx);
        status = lyjson_ctx_status(jsonctx, 0);

        /* check if the node is parseable. if not, NULL the snode to announce that it is supposed to be parsed
         * as an opaq node */
        switch (snode->nodetype) {
        case LYS_LEAFLIST:
        case LYS_LEAF:
            /* value may not be valid in which case we parse it as an opaque node */
            ret = lydjson_value_type_hint(lydctx, &status, type_hint_p);
            if (ret) {
                break;
            }

            if (lys_value_validate(NULL, snode, jsonctx->value, jsonctx->value_len, LY_VALUE_JSON, NULL)) {
                ret = LY_ENOT;
            }
            break;
        case LYS_LIST:
            /* lists may not have all its keys */
            if (lydjson_check_list(jsonctx, snode)) {
                /* invalid list, parse as opaque if it missing/has invalid some keys */
                ret = LY_ENOT;
            }
            break;
        }

        /* restore parser */
        lyjson_ctx_restore(jsonctx);
    } else if (snode->nodetype & LYD_NODE_TERM) {
        status = lyjson_ctx_status(jsonctx, 0);
        ret = lydjson_value_type_hint(lydctx, &status, type_hint_p);
    }

    return ret;
}

/**
 * @brief Join the forward-referencing metadata with their target data nodes.
 *
 * Note that JSON encoding for YANG data allows forward-referencing metadata only for leafs/leaf-lists.
 *
 * @param[in] lydctx JSON data parser context.
 * @param[in,out] first_p Pointer to the first sibling node variable (top-level or in a particular parent node)
 * as a starting point to search for the metadata's target data node
 * @return LY_SUCCESS on success
 * @return LY_EVALID in case there are some metadata with unresolved target data node instance
 */
static LY_ERR
lydjson_metadata_finish(struct lyd_json_ctx *lydctx, struct lyd_node **first_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *node, *attr, *next, *meta_iter;
    uint64_t instance = 0;
    const char *prev = NULL;
    uint32_t log_location_items = 0;

    /* finish linking metadata */
    LY_LIST_FOR_SAFE(*first_p, next, attr) {
        struct lyd_node_opaq *meta_container = (struct lyd_node_opaq *)attr;
        uint64_t match = 0;
        ly_bool is_attr;
        const char *name, *prefix;
        size_t name_len, prefix_len;
        const struct lysc_node *snode;

        if (attr->schema || (meta_container->name.name[0] != '@')) {
            /* not an opaq metadata node */
            continue;
        }

        LOG_LOCSET(NULL, attr, NULL, NULL);
        log_location_items++;

        if (prev != meta_container->name.name) {
            /* metas' names are stored in dictionary, so checking pointers must works */
            lydict_remove(lydctx->jsonctx->ctx, prev);
            LY_CHECK_GOTO(ret = lydict_insert(lydctx->jsonctx->ctx, meta_container->name.name, 0, &prev), cleanup);
            instance = 1;
        } else {
            instance++;
        }

        /* find the corresponding data node */
        LY_LIST_FOR(*first_p, node) {
            if (!node->schema) {
                /* opaq node - we are going to put into it just a generic attribute. */
                if (strcmp(&meta_container->name.name[1], ((struct lyd_node_opaq *)node)->name.name)) {
                    continue;
                }

                if (((struct lyd_node_opaq *)node)->hints & LYD_NODEHINT_LIST) {
                    LOGVAL(lydctx->jsonctx->ctx, LYVE_SYNTAX, "Metadata container references a sibling list node %s.",
                            ((struct lyd_node_opaq *)node)->name.name);
                    ret = LY_EVALID;
                    goto cleanup;
                }

                /* match */
                match++;
                if (match != instance) {
                    continue;
                }

                LY_LIST_FOR(meta_container->child, meta_iter) {
                    /* convert opaq node to a attribute of the opaq node */
                    struct lyd_node_opaq *meta = (struct lyd_node_opaq *)meta_iter;

                    ret = lyd_create_attr(node, NULL, lydctx->jsonctx->ctx, meta->name.name, strlen(meta->name.name),
                            meta->name.prefix, ly_strlen(meta->name.prefix), meta->name.module_name,
                            ly_strlen(meta->name.module_name), meta->value, ly_strlen(meta->value), NULL, LY_VALUE_JSON,
                            NULL, meta->hints);
                    LY_CHECK_GOTO(ret, cleanup);
                }

                /* done */
                break;
            } else {
                /* this is the second time we are resolving the schema node, so it must succeed,
                 * but remember that snode can be still NULL */
                lydjson_parse_name(meta_container->name.name, strlen(meta_container->name.name), &name, &name_len,
                        &prefix, &prefix_len, &is_attr);
                assert(is_attr);
                ret = lydjson_get_snode(lydctx, is_attr, prefix, prefix_len, name, name_len, (*first_p)->parent, &snode);
                assert(ret == LY_SUCCESS);

                if (snode != node->schema) {
                    continue;
                }

                /* match */
                match++;
                if (match != instance) {
                    continue;
                }

                LY_LIST_FOR(meta_container->child, meta_iter) {
                    /* convert opaq node to a metadata of the node */
                    struct lyd_node_opaq *meta = (struct lyd_node_opaq *)meta_iter;
                    struct lys_module *mod = NULL;
                    ly_bool dynamic = 0;

                    mod = ly_ctx_get_module_implemented(lydctx->jsonctx->ctx, meta->name.prefix);
                    if (mod) {
                        ret = lyd_parser_create_meta((struct lyd_ctx *)lydctx, node, NULL, mod,
                                meta->name.name, strlen(meta->name.name), meta->value, ly_strlen(meta->value),
                                &dynamic, LY_VALUE_JSON, NULL, meta->hints);
                        LY_CHECK_GOTO(ret, cleanup);
                    } else if (lydctx->parse_opts & LYD_PARSE_STRICT) {
                        LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE,
                                "Unknown (or not implemented) YANG module \"%s\" for metadata \"%s%s%s\".",
                                meta->name.prefix, meta->name.prefix, ly_strlen(meta->name.prefix) ? ":" : "", meta->name.name);
                        ret = LY_EVALID;
                        goto cleanup;
                    }
                }
                /* add/correct flags */
                lyd_parse_set_data_flags(node, &lydctx->node_when, &lydctx->node_exts, &node->meta, lydctx->parse_opts);

                /* done */
                break;
            }
        }

        if (match != instance) {
            /* there is no corresponding data node for the metadata */
            if (instance > 1) {
                LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE, "Missing %d%s JSON data instance to be coupled with %s metadata.",
                        instance, instance == 2 ? "nd" : (instance == 3 ? "rd" : "th"), meta_container->name.name);
            } else {
                LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE, "Missing JSON data instance to be coupled with %s metadata.",
                        meta_container->name.name);
            }
            ret = LY_EVALID;
        } else {
            /* remove the opaq attr */
            if (attr == (*first_p)) {
                *first_p = attr->next;
            }
            lyd_free_tree(attr);
        }

        LOG_LOCBACK(0, log_location_items, 0, 0);
        log_location_items = 0;
    }

cleanup:
    lydict_remove(lydctx->jsonctx->ctx, prev);

    LOG_LOCBACK(0, log_location_items, 0, 0);
    return ret;
}

/**
 * @brief Parse a metadata member.
 *
 * @param[in] lydctx JSON data parser context.
 * @param[in] snode Schema node of the metadata parent.
 * @param[in] node Parent node in case the metadata is not forward-referencing (only LYD_NODE_TERM)
 * so the data node does not exists. In such a case the metadata is stored in the context for the later
 * processing by lydjson_metadata_finish().
 * @return LY_SUCCESS on success
 * @return Various LY_ERR values in case of failure.
 */
static LY_ERR
lydjson_metadata(struct lyd_json_ctx *lydctx, const struct lysc_node *snode, struct lyd_node *node)
{
    LY_ERR ret = LY_SUCCESS;
    enum LYJSON_PARSER_STATUS status;
    const char *expected;
    ly_bool in_parent = 0;
    const char *name, *prefix = NULL;
    size_t name_len, prefix_len = 0;
    struct lys_module *mod;
    struct lyd_meta *meta = NULL;
    const struct ly_ctx *ctx = lydctx->jsonctx->ctx;
    ly_bool is_attr = 0;
    struct lyd_node *prev = node;
    uint32_t instance = 0;
    uint16_t nodetype;

    assert(snode || node);

    nodetype = snode ? snode->nodetype : LYS_CONTAINER;

    /* move to the second item in the name/X pair */
    ret = lyjson_ctx_next(lydctx->jsonctx, &status);
    LY_CHECK_GOTO(ret, cleanup);

    /* check attribute encoding */
    switch (nodetype) {
    case LYS_LEAFLIST:
        expected = "@name/array of objects/nulls";

        LY_CHECK_GOTO(status != LYJSON_ARRAY, representation_error);

next_entry:
        instance++;

        /* move into array / next entry */
        ret = lyjson_ctx_next(lydctx->jsonctx, &status);
        LY_CHECK_GOTO(ret, cleanup);

        if (status == LYJSON_ARRAY_CLOSED) {
            /* we are done, move after the array */
            ret = lyjson_ctx_next(lydctx->jsonctx, NULL);
            goto cleanup;
        }
        LY_CHECK_GOTO(status != LYJSON_OBJECT && status != LYJSON_NULL, representation_error);

        if (!node || (node->schema != prev->schema)) {
            LOGVAL(lydctx->jsonctx->ctx, LYVE_REFERENCE, "Missing JSON data instance no. %u of %s:%s to be coupled with metadata.",
                    instance, prev->schema->module->name, prev->schema->name);
            ret = LY_EVALID;
            goto cleanup;
        }

        if (status == LYJSON_NULL) {
            /* continue with the next entry in the leaf-list array */
            prev = node;
            node = node->next;
            goto next_entry;
        }
        break;
    case LYS_LEAF:
    case LYS_ANYXML:
        expected = "@name/object";

        LY_CHECK_GOTO(status != LYJSON_OBJECT, representation_error);
        break;
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_ANYDATA:
    case LYS_NOTIF:
    case LYS_ACTION:
    case LYS_RPC:
        in_parent = 1;
        expected = "@/object";
        LY_CHECK_GOTO(status != LYJSON_OBJECT, representation_error);
        break;
    default:
        LOGINT_RET(ctx);
    }

    /* process all the members inside a single metadata object */
    assert(status == LYJSON_OBJECT);

    LOG_LOCSET(snode, NULL, NULL, NULL);

    while (status != LYJSON_OBJECT_CLOSED) {
        lydjson_parse_name(lydctx->jsonctx->value, lydctx->jsonctx->value_len, &name, &name_len, &prefix, &prefix_len, &is_attr);
        if (!prefix) {
            LOGVAL(ctx, LYVE_SYNTAX_JSON, "Metadata in JSON must be namespace-qualified, missing prefix for \"%.*s\".",
                    (int)lydctx->jsonctx->value_len, lydctx->jsonctx->value);
            ret = LY_EVALID;
            goto cleanup;
        } else if (is_attr) {
            LOGVAL(ctx, LYVE_SYNTAX_JSON, "Invalid format of the Metadata identifier in JSON, unexpected '@' in \"%.*s\"",
                    (int)lydctx->jsonctx->value_len, lydctx->jsonctx->value);
            ret = LY_EVALID;
            goto cleanup;
        }

        /* get the element module */
        mod = ly_ctx_get_module_implemented2(ctx, prefix, prefix_len);
        if (!mod) {
            if (lydctx->parse_opts & LYD_PARSE_STRICT) {
                LOGVAL(ctx, LYVE_REFERENCE, "Prefix \"%.*s\" of the metadata \"%.*s\" does not match any module in the context.",
                        (int)prefix_len, prefix, (int)name_len, name);
                ret = LY_EVALID;
                goto cleanup;
            }
            if (!(lydctx->parse_opts & LYD_PARSE_OPAQ)) {
                /* skip element with children */
                ret = lydjson_data_skip(lydctx->jsonctx);
                LY_CHECK_GOTO(ret, cleanup);
                status = lyjson_ctx_status(lydctx->jsonctx, 0);
                /* end of the item */
                continue;
            }
        }

        /* get the value */
        ret = lyjson_ctx_next(lydctx->jsonctx, NULL);
        LY_CHECK_GOTO(ret, cleanup);

        if (node->schema) {
            /* create metadata */
            meta = NULL;
            ret = lyd_parser_create_meta((struct lyd_ctx *)lydctx, node, &meta, mod, name, name_len, lydctx->jsonctx->value,
                    lydctx->jsonctx->value_len, &lydctx->jsonctx->dynamic, LY_VALUE_JSON, NULL,
                    LYD_HINT_DATA);
            LY_CHECK_GOTO(ret, cleanup);

            /* add/correct flags */
            lyd_parse_set_data_flags(node, &lydctx->node_when, &lydctx->node_exts, &meta, lydctx->parse_opts);
        } else {
            /* create attribute */
            const char *module_name;
            size_t module_name_len;

            lydjson_get_node_prefix(node, prefix, prefix_len, &module_name, &module_name_len);

            /* attr2 is always changed to the created attribute */
            ret = lyd_create_attr(node, NULL, lydctx->jsonctx->ctx, name, name_len, prefix, prefix_len, module_name,
                    module_name_len, lydctx->jsonctx->value, lydctx->jsonctx->value_len, &lydctx->jsonctx->dynamic,
                    LY_VALUE_JSON, NULL, 0);
            LY_CHECK_GOTO(ret, cleanup);
        }
        /* next member */
        ret = lyjson_ctx_next(lydctx->jsonctx, &status);
        LY_CHECK_GOTO(ret, cleanup);
        LY_CHECK_GOTO((status != LYJSON_OBJECT) && (status != LYJSON_OBJECT_CLOSED), representation_error);
    }

    if (nodetype == LYS_LEAFLIST) {
        /* continue by processing another metadata object for the following
         * leaf-list instance since they are allways instantiated in JSON array */
        prev = node;
        node = node->next;
        goto next_entry;
    }

    /* move after the metadata */
    ret = lyjson_ctx_next(lydctx->jsonctx, NULL);
    LY_CHECK_GOTO(ret, cleanup);

    /* success */
    goto cleanup;

representation_error:
    LOGVAL(ctx, LYVE_SYNTAX_JSON,
            "The attribute(s) of %s \"%s\" is expected to be represented as JSON %s, but input data contains @%s/%s.",
            lys_nodetype2str(nodetype), node->schema ? node->schema->name : ((struct lyd_node_opaq *)node)->name.name,
            expected, lyjson_token2str(status), in_parent ? "" : "name");

    ret = LY_EVALID;

cleanup:
    LOG_LOCBACK(1, 0, 0, 0);
    return ret;
}

/**
 * @brief Eat the node pointed by @p node_p by inserting it into @p parent and maintain the @p first_p pointing to the first child node.
 *
 * @param[in] parent Parent node to insert to, can be NULL in case of top-level (or provided first_p).
 * @param[in, out] first_p Pointer to the first sibling node in case of top-level.
 * @param[in, out] node_p pointer to the new node to insert, after the insert is done, pointer is set to NULL.
 */
static void
lydjson_maintain_children(struct lyd_node_inner *parent, struct lyd_node **first_p, struct lyd_node **node_p)
{
    if (*node_p) {
        /* insert, keep first pointer correct */
        lyd_insert_node(&parent->node, first_p, *node_p);
        if (first_p) {
            if (parent) {
                *first_p = parent->child;
            } else {
                while ((*first_p)->prev->next) {
                    *first_p = (*first_p)->prev;
                }
            }
        }
        *node_p = NULL;
    }
}

/**
 * @brief Wrapper for ::lyd_create_opaq().
 *
 * @param[in] lydctx JSON data parser context.
 * @param[in] name Name of the opaq node to create.
 * @param[in] name_len Length of the @p name string.
 * @param[in] prefix Prefix of the opaq node to create.
 * @param[in] prefix_len Length of the @p prefx string.
 * @param[in] parent Data parent of the opaq node to create, can be NULL in case of top level,
 * but must be set if @p first is not.
 * @param[in,out] status_inner_p In case of processing JSON array, this parameter points to a standalone
 * context status of the array content. Otherwise, it is supposed to be the same as @p status_p.
 * @param[out] node_p Pointer to the created opaq node.
 * @return LY_ERR value.
 */
static LY_ERR
lydjson_create_opaq(struct lyd_json_ctx *lydctx, const char *name, size_t name_len, const char *prefix, size_t prefix_len,
        struct lyd_node_inner *parent, enum LYJSON_PARSER_STATUS *status_inner_p, struct lyd_node **node_p)
{
    LY_ERR ret = LY_SUCCESS;
    const char *value = NULL, *module_name;
    size_t value_len = 0, module_name_len = 0;
    ly_bool dynamic = 0;
    uint32_t type_hint = 0;

    if ((*status_inner_p != LYJSON_OBJECT) && (*status_inner_p != LYJSON_OBJECT_EMPTY)) {
        /* prepare for creating opaq node with a value */
        value = lydctx->jsonctx->value;
        value_len = lydctx->jsonctx->value_len;
        dynamic = lydctx->jsonctx->dynamic;
        lydctx->jsonctx->dynamic = 0;

        LY_CHECK_RET(lydjson_value_type_hint(lydctx, status_inner_p, &type_hint));
    }

    /* create node */
    lydjson_get_node_prefix(&parent->node, prefix, prefix_len, &module_name, &module_name_len);
    ret = lyd_create_opaq(lydctx->jsonctx->ctx, name, name_len, prefix, prefix_len, module_name, module_name_len, value,
            value_len, &dynamic, LY_VALUE_JSON, NULL, type_hint, node_p);
    if (dynamic) {
        free((char *)value);
    }

    return ret;
}

static LY_ERR lydjson_subtree_r(struct lyd_json_ctx *lydctx, struct lyd_node *parent, struct lyd_node **first_p,
        struct ly_set *parsed);

/**
 * @brief Parse opaq node from the input.
 *
 * In case of processing array, the whole array is being processed and the resulting @p node_p is the last item of the array.
 *
 * @param[in] lydctx JSON data parser context.
 * @param[in] name Name of the opaq node to create.
 * @param[in] name_len Length of the @p name string.
 * @param[in] prefix Prefix of the opaq node to create.
 * @param[in] prefix_len Length of the @p prefx string.
 * @param[in] parent Data parent of the opaq node to create, can be NULL in case of top level,
 * but must be set if @p first is not.
 * @param[in,out] status_p Pointer to the current status of the parser context,
 * since the function manipulates with the context and process the input, the status can be updated.
 * @param[in,out] status_inner_p In case of processing JSON array, this parameter points to a standalone
 * context status of the array content. Otherwise, it is supposed to be the same as @p status_p.
 * @param[in,out] first_p First top-level/parent sibling, must be set if @p parent is not.
 * @param[out] node_p Pointer to the created opaq node.
 * @return LY_ERR value.
 */
static LY_ERR
lydjson_parse_opaq(struct lyd_json_ctx *lydctx, const char *name, size_t name_len, const char *prefix, size_t prefix_len,
        struct lyd_node_inner *parent, enum LYJSON_PARSER_STATUS *status_p,
        enum LYJSON_PARSER_STATUS *status_inner_p, struct lyd_node **first_p, struct lyd_node **node_p)
{
    LY_ERR ret = LY_SUCCESS;

    ret = lydjson_create_opaq(lydctx, name, name_len, prefix, prefix_len, parent, status_inner_p, node_p);
    LY_CHECK_RET(ret);

    while ((*status_p == LYJSON_ARRAY) || (*status_p == LYJSON_ARRAY_EMPTY)) {
        /* process another instance of the same node */
        /* but first mark the node to be expected a list or a leaf-list */
        ((struct lyd_node_opaq *)*node_p)->hints |= LYD_NODEHINT_LIST | LYD_NODEHINT_LEAFLIST;

        if ((*status_inner_p == LYJSON_OBJECT) || (*status_inner_p == LYJSON_OBJECT_EMPTY)) {
            /* but first process children of the object in the array */
            while (*status_inner_p != LYJSON_OBJECT_CLOSED && *status_inner_p != LYJSON_OBJECT_EMPTY) {
                LY_CHECK_RET(lydjson_subtree_r(lydctx, *node_p, lyd_node_child_p(*node_p), NULL));
                *status_inner_p = lyjson_ctx_status(lydctx->jsonctx, 0);
            }
        }

        LY_CHECK_RET(lyjson_ctx_next(lydctx->jsonctx, status_inner_p));
        if (*status_inner_p == LYJSON_ARRAY_CLOSED) {
            goto finish;
        }

        /* continue with the next instance */
        assert(node_p);
        lydjson_maintain_children(parent, first_p, node_p);
        ret = lydjson_create_opaq(lydctx, name, name_len, prefix, prefix_len, parent, status_inner_p, node_p);
        LY_CHECK_RET(ret);
    }

    if ((*status_p == LYJSON_OBJECT) || (*status_p == LYJSON_OBJECT_EMPTY)) {
        /* process children */
        while (*status_p != LYJSON_OBJECT_CLOSED && *status_p != LYJSON_OBJECT_EMPTY) {
            LY_CHECK_RET(lydjson_subtree_r(lydctx, *node_p, lyd_node_child_p(*node_p), NULL));
            *status_p = lyjson_ctx_status(lydctx->jsonctx, 0);
        }
    }

finish:
    /* finish linking metadata */
    LY_CHECK_RET(lydjson_metadata_finish(lydctx, lyd_node_child_p(*node_p)));

    /* move after the item */
    return lyjson_ctx_next(lydctx->jsonctx, status_p);
}

/**
 * @brief Move to the second item in the name/X pair and parse opaq node from the input.
 *
 * This function is basically the wrapper of the ::lydjson_parse_opaq().
 * In addition, it calls the ::json_ctx_next() and prepares the status_inner_p parameter
 * for ::lydjson_parse_opaq().
 *
 * @param[in] lydctx JSON data parser context.
 * @param[in] name Name of the opaq node to create.
 * @param[in] name_len Length of the @p name string.
 * @param[in] prefix Prefix of the opaq node to create.
 * @param[in] prefix_len Length of the @p prefx string.
 * @param[in] parent Data parent of the opaq node to create, can be NULL in case of top level,
 * but must be set if @p first is not.
 * @param[in,out] status_p Pointer to the current status of the parser context,
 * since the function manipulates with the context and process the input, the status can be updated.
 * @param[in,out] first_p First top-level/parent sibling, must be set if @p parent is not.
 * @param[out] node_p Pointer to the created opaq node.
 * @return LY_ERR value.
 */
static LY_ERR
lydjson_ctx_next_parse_opaq(struct lyd_json_ctx *lydctx, const char *name, size_t name_len,
        const char *prefix, size_t prefix_len, struct lyd_node_inner *parent, enum LYJSON_PARSER_STATUS *status_p,
        struct lyd_node **first_p, struct lyd_node **node_p)
{
    enum LYJSON_PARSER_STATUS status_inner = 0;

    /* move to the second item in the name/X pair */
    LY_CHECK_RET(lyjson_ctx_next(lydctx->jsonctx, status_p));

    if (*status_p == LYJSON_ARRAY) {
        /* move into the array */
        LY_CHECK_RET(lyjson_ctx_next(lydctx->jsonctx, &status_inner));
    } else {
        /* just a flag to pass correct parameters into lydjson_parse_opaq() */
        status_inner = LYJSON_ERROR;
    }

    if (status_inner == LYJSON_ERROR) {
        status_inner = *status_p;
    }

    /* parse opaq node from the input */
    LY_CHECK_RET(lydjson_parse_opaq(lydctx, name, name_len, prefix, prefix_len, parent, status_p, &status_inner, first_p, node_p));

    return LY_SUCCESS;
}

/**
 * @brief Process the attribute container (starting by @)
 *
 * @param[in] lydctx JSON data parser context.
 * @param[in] attr_node The data node referenced by the attribute container, if already known.
 * @param[in] snode The schema node of the data node referenced by the attribute container, if known.
 * @param[in] name Name of the opaq node to create.
 * @param[in] name_len Length of the @p name string.
 * @param[in] prefix Prefix of the opaq node to create.
 * @param[in] prefix_len Length of the @p prefx string.
 * @param[in] parent Data parent of the opaq node to create, can be NULL in case of top level,
 * but must be set if @p first is not.
 * @param[in,out] status_p Pointer to the current status of the parser context,
 * since the function manipulates with the context and process the input, the status can be updated.
 * @param[in,out] first_p First top-level/parent sibling, must be set if @p parent is not.
 * @param[out] node_p Pointer to the created opaq node.
 * @return LY_ERR value.
 */
static LY_ERR
lydjson_parse_attribute(struct lyd_json_ctx *lydctx, struct lyd_node *attr_node, const struct lysc_node *snode,
        const char *name, size_t name_len, const char *prefix, size_t prefix_len, struct lyd_node *parent,
        enum LYJSON_PARSER_STATUS *status_p, struct lyd_node **first_p, struct lyd_node **node_p)
{
    LY_ERR ret = LY_SUCCESS;

    /* parse as an attribute to a node */
    if (!attr_node && snode) {
        /* try to find the instance */
        for (struct lyd_node *iter = *first_p; iter; iter = iter->next) {
            if (iter->schema == snode) {
                attr_node = iter;
                break;
            }
        }
    }
    if (!attr_node) {
        /* parse just as an opaq node with the name beginning with @,
         * later we have to check that it belongs to a standard node
         * and it is supposed to be converted to a metadata */
        uint32_t prev_opts;

        /* backup parser options to parse unknown metadata as opaq nodes and try to resolve them later */
        prev_opts = lydctx->parse_opts;
        lydctx->parse_opts &= ~LYD_PARSE_STRICT;
        lydctx->parse_opts |= LYD_PARSE_OPAQ;

        ret = lydjson_ctx_next_parse_opaq(lydctx, prefix ? prefix - 1 : name - 1, prefix ? prefix_len + name_len + 2 : name_len + 1,
                NULL, 0, (struct lyd_node_inner *)parent, status_p, first_p, node_p);

        /* restore the parser options */
        lydctx->parse_opts = prev_opts;
    } else {
        ret = lydjson_metadata(lydctx, snode, attr_node);
    }

    return ret;
}

/**
 * @brief Parse a single instance of a node.
 *
 * @param[in] lydctx JSON data parser context. When the function returns, the context is in the same state
 * as before calling, despite it is necessary to process input data for checking.
 * @param[in] parent Data parent of the subtree, must be set if @p first is not.
 * @param[in,out] first_p Pointer to the variable holding the first top-level sibling, must be set if @p parent is not.
 * @param[in] snode Schema node corresponding to the member currently being processed in the context.
 * @param[in] name Parsed JSON node name.
 * @param[in] name_len Lenght of @p name.
 * @param[in] prefix Parsed JSON node prefix.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in,out] status JSON parser status, is updated.
 * @param[out] node Parsed data (or opaque) node.
 * @return LY_SUCCESS if a node was successfully parsed,
 * @return LY_EINVAL in case of invalid JSON encoding,
 * @return LY_ERR on other errors.
 */
static LY_ERR
lydjson_parse_instance(struct lyd_json_ctx *lydctx, struct lyd_node_inner *parent, struct lyd_node **first_p,
        const struct lysc_node *snode, const char *name, size_t name_len, const char *prefix, size_t prefix_len,
        enum LYJSON_PARSER_STATUS *status, struct lyd_node **node)
{
    LY_ERR ret;
    uint32_t type_hints = 0;
    uint32_t prev_opts;
    struct lyd_node *tree = NULL;

    ret = lydjson_data_check_opaq(lydctx, snode, &type_hints);
    if (ret == LY_SUCCESS) {
        assert(snode->nodetype & (LYD_NODE_TERM | LYD_NODE_INNER | LYD_NODE_ANY));
        if (snode->nodetype & LYD_NODE_TERM) {
            /* create terminal node */
            ret = lyd_parser_create_term((struct lyd_ctx *)lydctx, snode, lydctx->jsonctx->value,
                    lydctx->jsonctx->value_len, &lydctx->jsonctx->dynamic, LY_VALUE_JSON, NULL,
                    type_hints, node);
            LY_CHECK_RET(ret);

            /* move JSON parser */
            ret = lyjson_ctx_next(lydctx->jsonctx, status);
            LY_CHECK_RET(ret);
        } else if (snode->nodetype & LYD_NODE_INNER) {
            /* create inner node */
            LY_CHECK_RET(*status != LYJSON_OBJECT && *status != LYJSON_OBJECT_EMPTY, LY_EINVAL);

            ret = lyd_create_inner(snode, node);
            LY_CHECK_RET(ret);

            LOG_LOCSET(snode, *node, NULL, NULL);

            /* process children */
            while (*status != LYJSON_OBJECT_CLOSED && *status != LYJSON_OBJECT_EMPTY) {
                ret = lydjson_subtree_r(lydctx, *node, lyd_node_child_p(*node), NULL);
                LY_CHECK_ERR_RET(ret, LOG_LOCBACK(1, 1, 0, 0), ret);
                *status = lyjson_ctx_status(lydctx->jsonctx, 0);
            }

            /* finish linking metadata */
            ret = lydjson_metadata_finish(lydctx, lyd_node_child_p(*node));
            LY_CHECK_ERR_RET(ret, LOG_LOCBACK(1, 1, 0, 0), ret);

            if (snode->nodetype == LYS_LIST) {
                /* check all keys exist */
                ret = lyd_parse_check_keys(*node);
                LY_CHECK_ERR_RET(ret, LOG_LOCBACK(1, 1, 0, 0), ret);
            }

            if (!(lydctx->parse_opts & LYD_PARSE_ONLY)) {
                /* new node validation, autodelete CANNOT occur, all nodes are new */
                ret = lyd_validate_new(lyd_node_child_p(*node), snode, NULL, NULL);
                LY_CHECK_ERR_RET(ret, LOG_LOCBACK(1, 1, 0, 0), ret);

                /* add any missing default children */
                ret = lyd_new_implicit_r(*node, lyd_node_child_p(*node), NULL, NULL, &lydctx->node_when, &lydctx->node_exts,
                        &lydctx->node_types, (lydctx->val_opts & LYD_VALIDATE_NO_STATE) ? LYD_IMPLICIT_NO_STATE : 0, NULL);
                LY_CHECK_ERR_RET(ret, LOG_LOCBACK(1, 1, 0, 0), ret);
            }

            LOG_LOCBACK(1, 1, 0, 0);

            /* move JSON parser */
            ret = lyjson_ctx_next(lydctx->jsonctx, status);
            LY_CHECK_RET(ret);
        } else if (snode->nodetype & LYD_NODE_ANY) {
            /* create any node */
            LY_CHECK_RET(*status != LYJSON_OBJECT && *status != LYJSON_OBJECT_EMPTY, LY_EINVAL);

            /* parse any data tree with correct options */
            /* first backup the current options and then make the parser to process data as opaq nodes */
            prev_opts = lydctx->parse_opts;
            lydctx->parse_opts &= ~LYD_PARSE_STRICT;
            lydctx->parse_opts |= LYD_PARSE_OPAQ;

            /* process the anydata content */
            while (*status != LYJSON_OBJECT_CLOSED && *status != LYJSON_OBJECT_EMPTY) {
                ret = lydjson_subtree_r(lydctx, NULL, &tree, NULL);
                LY_CHECK_RET(ret);
                *status = lyjson_ctx_status(lydctx->jsonctx, 0);
            }

            /* restore parser options */
            lydctx->parse_opts = prev_opts;

            /* finish linking metadata */
            ret = lydjson_metadata_finish(lydctx, &tree);
            LY_CHECK_RET(ret);

            ret = lyd_create_any(snode, tree, LYD_ANYDATA_DATATREE, 1, node);
            LY_CHECK_RET(ret);
        }
    } else if (ret == LY_ENOT) {
        /* parse it again as an opaq node */
        ret = lydjson_parse_opaq(lydctx, name, name_len, prefix, prefix_len, parent,
                status, status, first_p, node);
        LY_CHECK_RET(ret);

        if (snode->nodetype == LYS_LIST) {
            ((struct lyd_node_opaq *)*node)->hints |= LYD_NODEHINT_LIST;
        } else if (snode->nodetype == LYS_LEAFLIST) {
            ((struct lyd_node_opaq *)*node)->hints |= LYD_NODEHINT_LEAFLIST;
        }
    }

    return ret;
}

/**
 * @brief Parse JSON subtree. All leaf-list and list instances of a node are considered one subtree.
 *
 * @param[in] lydctx JSON data parser context.
 * @param[in] parent Data parent of the subtree, must be set if @p first is not.
 * @param[in,out] first_p Pointer to the variable holding the first top-level sibling, must be set if @p parent is not.
 * @param[in,out] parsed Optional set to add all the parsed siblings into.
 * @return LY_ERR value.
 */
static LY_ERR
lydjson_subtree_r(struct lyd_json_ctx *lydctx, struct lyd_node *parent, struct lyd_node **first_p, struct ly_set *parsed)
{
    LY_ERR ret = LY_SUCCESS;
    enum LYJSON_PARSER_STATUS status = lyjson_ctx_status(lydctx->jsonctx, 0);
    const char *name, *prefix = NULL, *expected = NULL;
    size_t name_len, prefix_len = 0;
    ly_bool is_meta = 0;
    const struct lysc_node *snode = NULL;
    struct lyd_node *node = NULL, *attr_node = NULL;
    const struct ly_ctx *ctx = lydctx->jsonctx->ctx;
    char *value = NULL;

    assert(parent || first_p);
    assert(status == LYJSON_OBJECT);

    /* Duplicate ::lyjson_ctx.value because it can be dynamically allocated and later
     * ::lyjson_ctx_next() will release it although this string is needed for the ::lydjson_parse_opaq().
     */
    value = strndup(lydctx->jsonctx->value, lydctx->jsonctx->value_len);
    if (!value) {
        LOGMEM(lydctx->jsonctx->ctx);
        ret = LY_EMEM;
        goto cleanup;
    }

    /* process the node name */
    lydjson_parse_name(value, lydctx->jsonctx->value_len, &name, &name_len, &prefix, &prefix_len, &is_meta);

    if (!is_meta || name_len || prefix_len) {
        /* get the schema node */
        ret = lydjson_get_snode(lydctx, is_meta, prefix, prefix_len, name, name_len, (struct lyd_node_inner *)parent, &snode);
        if (ret == LY_ENOT) {
            /* skip element with children */
            ret = lydjson_data_skip(lydctx->jsonctx);
            LY_CHECK_GOTO(ret, cleanup);
            status = lyjson_ctx_status(lydctx->jsonctx, 0);
            /* nothing for now, continue with another call of lydjson_subtree_r() */
            goto cleanup;
        }
        LY_CHECK_GOTO(ret, cleanup);

        if (!snode) {
            /* we will not be parsing it as metadata */
            is_meta = 0;
        }
    }

    if (is_meta) {
        /* parse as metadata */
        if (!name_len && !prefix_len) {
            /* parent's metadata without a name - use the schema from the parent */
            if (!parent) {
                LOGVAL(ctx, LYVE_SYNTAX_JSON,
                        "Invalid metadata format - \"@\" can be used only inside anydata, container or list entries.");
                ret = LY_EVALID;
                goto cleanup;
            }
            attr_node = parent;
            snode = attr_node->schema;
        }
        ret = lydjson_parse_attribute(lydctx, attr_node, snode, name, name_len, prefix, prefix_len, parent, &status,
                first_p, &node);
        LY_CHECK_GOTO(ret, cleanup);
    } else if (!snode) {
        /* parse as an opaq node */
        assert((lydctx->parse_opts & LYD_PARSE_OPAQ) || (lydctx->int_opts));

        /* opaq node cannot have an empty string as the name. */
        if (name_len == 0) {
            LOGVAL(lydctx->jsonctx->ctx, LYVE_SYNTAX_JSON, "A JSON object member name cannot be a zero-length string.");
            ret = LY_EVALID;
            goto cleanup;
        }

        /* move to the second item in the name/X pair and parse opaq */
        ret = lydjson_ctx_next_parse_opaq(lydctx, name, name_len, prefix, prefix_len,
                (struct lyd_node_inner *)parent, &status, first_p, &node);
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        /* parse as a standard lyd_node but it can still turn out to be an opaque node */

        /* move to the second item in the name/X pair */
        ret = lyjson_ctx_next(lydctx->jsonctx, &status);
        LY_CHECK_GOTO(ret, cleanup);

        /* first check the expected representation according to the nodetype and then continue with the content */
        switch (snode->nodetype) {
        case LYS_LEAFLIST:
        case LYS_LIST:
            if (snode->nodetype == LYS_LEAFLIST) {
                expected = "name/array of values";
            } else {
                expected = "name/array of objects";
            }

            LY_CHECK_GOTO(status != LYJSON_ARRAY, representation_error);

            /* move into array */
            ret = lyjson_ctx_next(lydctx->jsonctx, &status);
            LY_CHECK_GOTO(ret, cleanup);

            /* process all the values/objects */
            do {
                lydjson_maintain_children((struct lyd_node_inner *)parent, first_p, &node);

                ret = lydjson_parse_instance(lydctx, (struct lyd_node_inner *)parent, first_p, snode, name, name_len,
                        prefix, prefix_len, &status, &node);
                if (ret == LY_EINVAL) {
                    goto representation_error;
                } else if (ret) {
                    goto cleanup;
                }
            } while (status != LYJSON_ARRAY_CLOSED);

            /* move after the array */
            ret = lyjson_ctx_next(lydctx->jsonctx, &status);
            LY_CHECK_GOTO(ret, cleanup);

            break;
        case LYS_LEAF:
        case LYS_CONTAINER:
        case LYS_NOTIF:
        case LYS_ACTION:
        case LYS_RPC:
        case LYS_ANYDATA:
        case LYS_ANYXML:
            if (snode->nodetype == LYS_LEAF) {
                if (status == LYJSON_ARRAY) {
                    expected = "name/[null]";
                } else {
                    expected = "name/value";
                }
            } else {
                expected = "name/object";
            }

            /* process the value/object */
            ret = lydjson_parse_instance(lydctx, (struct lyd_node_inner *)parent, first_p, snode, name, name_len,
                    prefix, prefix_len, &status, &node);
            if (ret == LY_EINVAL) {
                goto representation_error;
            } else if (ret) {
                goto cleanup;
            }

            if (snode->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) {
                /* rememeber the RPC/action/notification */
                lydctx->op_node = node;
            }

            break;
        }
    }

    /* finally connect the parsed node */
    lydjson_maintain_children((struct lyd_node_inner *)parent, first_p, &node);

    /* rememeber a successfully parsed node */
    if (parsed) {
        ly_set_add(parsed, node, 1, NULL);
    }

    /* success */
    goto cleanup;

representation_error:
    LOG_LOCSET(NULL, parent, NULL, NULL);
    LOGVAL(ctx, LYVE_SYNTAX_JSON, "The %s \"%s\" is expected to be represented as JSON %s, but input data contains name/%s.",
            lys_nodetype2str(snode->nodetype), snode->name, expected, lyjson_token2str(status));
    LOG_LOCBACK(0, parent ? 1 : 0, 0, 0);
    ret = LY_EVALID;

cleanup:
    free(value);
    lyd_free_tree(node);
    return ret;
}

/**
 * @brief Common start of JSON parser processing different types of the input data.
 *
 * @param[in] ctx libyang context
 * @param[in] in Input structure.
 * @param[in] parse_opts Options for parser, see @ref dataparseroptions.
 * @param[in] val_opts Options for the validation phase, see @ref datavalidationoptions.
 * @param[out] lydctx_p Data parser context to finish validation.
 * @param[out] status Storage for the current context's status
 * @return LY_ERR value.
 */
static LY_ERR
lyd_parse_json_init(const struct ly_ctx *ctx, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts,
        struct lyd_json_ctx **lydctx_p, enum LYJSON_PARSER_STATUS *status)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_json_ctx *lydctx;
    size_t i;

    assert(lydctx_p);
    assert(status);

    /* init context */
    lydctx = calloc(1, sizeof *lydctx);
    LY_CHECK_ERR_RET(!lydctx, LOGMEM(ctx), LY_EMEM);
    lydctx->parse_opts = parse_opts;
    lydctx->val_opts = val_opts;
    lydctx->free = lyd_json_ctx_free;

    /* starting top-level */
    for (i = 0; in->current[i] != '\0' && is_jsonws(in->current[i]); i++) {
        if (in->current[i] == '\n') {
            /* new line */
            LY_IN_NEW_LINE(in);
        }
    }

    LY_CHECK_ERR_RET(ret = lyjson_ctx_new(ctx, in, &lydctx->jsonctx), free(lydctx), ret);
    *status = lyjson_ctx_status(lydctx->jsonctx, 0);
    if ((*status == LYJSON_END) || (*status == LYJSON_OBJECT_EMPTY) || (*status == LYJSON_OBJECT)) {
        *lydctx_p = lydctx;
        return LY_SUCCESS;
    } else {
        /* expecting top-level object */
        LOGVAL(ctx, LYVE_SYNTAX_JSON, "Expected top-level JSON object, but %s found.",
                lyjson_token2str(*status));
        *lydctx_p = NULL;
        lyd_json_ctx_free((struct lyd_ctx *)lydctx);
        return LY_EVALID;
    }
}

LY_ERR
lyd_parse_json(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
        struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, enum lyd_type data_type,
        struct ly_set *parsed, struct lyd_ctx **lydctx_p)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_json_ctx *lydctx = NULL;
    enum LYJSON_PARSER_STATUS status;
    uint32_t int_opts;

    rc = lyd_parse_json_init(ctx, in, parse_opts, val_opts, &lydctx, &status);
    LY_CHECK_GOTO(rc || status == LYJSON_END || status == LYJSON_OBJECT_EMPTY, cleanup);

    assert(status == LYJSON_OBJECT);

    switch (data_type) {
    case LYD_TYPE_DATA_YANG:
        int_opts = LYD_INTOPT_WITH_SIBLINGS;
        break;
    case LYD_TYPE_RPC_YANG:
        int_opts = LYD_INTOPT_RPC | LYD_INTOPT_ACTION | LYD_INTOPT_NO_SIBLINGS;
        break;
    case LYD_TYPE_NOTIF_YANG:
        int_opts = LYD_INTOPT_NOTIF | LYD_INTOPT_NO_SIBLINGS;
        break;
    case LYD_TYPE_REPLY_YANG:
        int_opts = LYD_INTOPT_REPLY | LYD_INTOPT_NO_SIBLINGS;
        break;
    default:
        LOGINT(ctx);
        rc = LY_EINT;
        goto cleanup;
    }
    lydctx->int_opts = int_opts;
    lydctx->ext = ext;

    /* find the operation node if it exists already */
    LY_CHECK_GOTO(rc = lyd_parser_find_operation(parent, int_opts, &lydctx->op_node), cleanup);

    /* read subtree(s) */
    while (lydctx->jsonctx->in->current[0] && (status != LYJSON_OBJECT_CLOSED)) {
        rc = lydjson_subtree_r(lydctx, parent, first_p, parsed);
        LY_CHECK_GOTO(rc, cleanup);

        status = lyjson_ctx_status(lydctx->jsonctx, 0);

        if (!(int_opts & LYD_INTOPT_WITH_SIBLINGS)) {
            break;
        }
    }

    if ((int_opts & LYD_INTOPT_NO_SIBLINGS) && lydctx->jsonctx->in->current[0] &&
            (lyjson_ctx_status(lydctx->jsonctx, 0) != LYJSON_OBJECT_CLOSED)) {
        LOGVAL(ctx, LYVE_SYNTAX, "Unexpected sibling node.");
        rc = LY_EVALID;
        goto cleanup;
    }
    if ((int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_ACTION | LYD_INTOPT_NOTIF | LYD_INTOPT_REPLY)) && !lydctx->op_node) {
        LOGVAL(ctx, LYVE_DATA, "Missing the operation node.");
        rc = LY_EVALID;
        goto cleanup;
    }

    /* finish linking metadata */
    rc = lydjson_metadata_finish(lydctx, parent ? lyd_node_child_p(parent) : first_p);
    LY_CHECK_GOTO(rc, cleanup);

cleanup:
    /* there should be no unresolved types stored */
    assert(!(parse_opts & LYD_PARSE_ONLY) || (!lydctx->node_types.count && !lydctx->meta_types.count &&
            !lydctx->node_when.count));

    if (rc) {
        lyd_json_ctx_free((struct lyd_ctx *)lydctx);
    } else {
        *lydctx_p = (struct lyd_ctx *)lydctx;
    }
    return rc;
}
