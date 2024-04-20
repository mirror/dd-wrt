/**
 * @file parser_xml.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief XML data parser for libyang
 *
 * Copyright (c) 2019 - 2022 CESNET, z.s.p.o.
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
#include "log.h"
#include "parser_data.h"
#include "parser_internal.h"
#include "plugins_exts.h"
#include "plugins_internal.h"
#include "schema_compile_node.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "validation.h"
#include "xml.h"

static LY_ERR lydxml_subtree_r(struct lyd_xml_ctx *lydctx, struct lyd_node *parent, struct lyd_node **first_p,
        struct ly_set *parsed);

void
lyd_xml_ctx_free(struct lyd_ctx *lydctx)
{
    struct lyd_xml_ctx *ctx = (struct lyd_xml_ctx *)lydctx;

    lyd_ctx_free(lydctx);
    lyxml_ctx_free(ctx->xmlctx);
    free(ctx);
}

/**
 * @brief Parse and create XML metadata.
 *
 * @param[in] lydctx XML data parser context.
 * @param[in] sparent Schema node of the parent.
 * @param[out] meta List of created metadata instances.
 * @return LY_ERR value.
 */
static LY_ERR
lydxml_metadata(struct lyd_xml_ctx *lydctx, const struct lysc_node *sparent, struct lyd_meta **meta)
{
    LY_ERR ret = LY_SUCCESS;
    const struct lyxml_ns *ns;
    struct lys_module *mod;
    const char *name;
    size_t name_len;
    LY_ARRAY_COUNT_TYPE u;
    ly_bool filter_attrs = 0;
    struct lyxml_ctx *xmlctx = lydctx->xmlctx;

    *meta = NULL;

    LOG_LOCSET(sparent, NULL, NULL, NULL);

    /* check for NETCONF filter unqualified attributes */
    if (!strcmp(sparent->module->name, "notifications")) {
        /* ancient module that does not even use the extension */
        filter_attrs = 1;
    } else {
        LY_ARRAY_FOR(sparent->exts, u) {
            if (!strcmp(sparent->exts[u].def->name, "get-filter-element-attributes") &&
                    !strcmp(sparent->exts[u].def->module->name, "ietf-netconf")) {
                filter_attrs = 1;
                break;
            }
        }
    }

    while (xmlctx->status == LYXML_ATTRIBUTE) {
        if (!xmlctx->prefix_len) {
            /* in XML all attributes must be prefixed except NETCONF filter ones marked by an extension */
            if (filter_attrs && (!ly_strncmp("type", xmlctx->name, xmlctx->name_len) ||
                    !ly_strncmp("select", xmlctx->name, xmlctx->name_len))) {
                mod = ly_ctx_get_module_implemented(xmlctx->ctx, "ietf-netconf");
                if (!mod) {
                    LOGVAL(xmlctx->ctx, LYVE_REFERENCE,
                            "Missing (or not implemented) YANG module \"ietf-netconf\" for special filter attributes.");
                    ret = LY_ENOTFOUND;
                    goto cleanup;
                }
                goto create_meta;
            }

            if (lydctx->parse_opts & LYD_PARSE_STRICT) {
                LOGVAL(xmlctx->ctx, LYVE_REFERENCE, "Missing mandatory prefix for XML metadata \"%.*s\".",
                        (int)xmlctx->name_len, xmlctx->name);
                ret = LY_EVALID;
                goto cleanup;
            }

            /* skip attr */
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
            assert(xmlctx->status == LYXML_ATTR_CONTENT);
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
            continue;
        }

        /* get namespace of the attribute to find its annotation definition */
        ns = lyxml_ns_get(&xmlctx->ns, xmlctx->prefix, xmlctx->prefix_len);
        if (!ns) {
            /* unknown namespace, XML error */
            LOGVAL(xmlctx->ctx, LYVE_REFERENCE, "Unknown XML prefix \"%.*s\".", (int)xmlctx->prefix_len, xmlctx->prefix);
            ret = LY_ENOTFOUND;
            goto cleanup;
        }

        /* get the module with metadata definition */
        mod = ly_ctx_get_module_implemented_ns(xmlctx->ctx, ns->uri);
        if (!mod) {
            if (lydctx->parse_opts & LYD_PARSE_STRICT) {
                LOGVAL(xmlctx->ctx, LYVE_REFERENCE,
                        "Unknown (or not implemented) YANG module with namespace \"%s\" for metadata \"%.*s%s%.*s\".",
                        ns->uri, (int)xmlctx->prefix_len, xmlctx->prefix, xmlctx->prefix_len ? ":" : "",
                        (int)xmlctx->name_len, xmlctx->name);
                ret = LY_ENOTFOUND;
                goto cleanup;
            }

            /* skip attr */
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
            assert(xmlctx->status == LYXML_ATTR_CONTENT);
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
            continue;
        }

create_meta:
        /* remember meta name and get its content */
        name = xmlctx->name;
        name_len = xmlctx->name_len;
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
        assert(xmlctx->status == LYXML_ATTR_CONTENT);

        /* create metadata */
        ret = lyd_parser_create_meta((struct lyd_ctx *)lydctx, NULL, meta, mod, name, name_len, xmlctx->value,
                xmlctx->value_len, &xmlctx->dynamic, LY_VALUE_XML, &xmlctx->ns, LYD_HINT_DATA, sparent);
        LY_CHECK_GOTO(ret, cleanup);

        /* next attribute */
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
    }

cleanup:
    LOG_LOCBACK(1, 0, 0, 0);
    if (ret) {
        lyd_free_meta_siblings(*meta);
        *meta = NULL;
    }
    return ret;
}

static LY_ERR
lydxml_attrs(struct lyxml_ctx *xmlctx, struct lyd_attr **attr)
{
    LY_ERR ret = LY_SUCCESS;
    const struct lyxml_ns *ns;
    void *val_prefix_data;
    LY_VALUE_FORMAT format;
    struct lyd_attr *attr2;
    const char *name, *prefix;
    size_t name_len, prefix_len;

    assert(attr);
    *attr = NULL;

    while (xmlctx->status == LYXML_ATTRIBUTE) {
        if (*attr) {
            attr2 = *attr;
        } else {
            attr2 = NULL;
        }

        /* remember attr prefix, name, and get its content */
        prefix = xmlctx->prefix;
        prefix_len = xmlctx->prefix_len;
        name = xmlctx->name;
        name_len = xmlctx->name_len;
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
        assert(xmlctx->status == LYXML_ATTR_CONTENT);

        /* handle special "xml" attribute prefix */
        if ((prefix_len == 3) && !strncmp(prefix, "xml", 3)) {
            name = prefix;
            name_len += 1 + prefix_len;
            prefix = NULL;
            prefix_len = 0;
        }

        /* find namespace of the attribute, if any */
        ns = NULL;
        if (prefix_len) {
            ns = lyxml_ns_get(&xmlctx->ns, prefix, prefix_len);
            if (!ns) {
                LOGVAL(xmlctx->ctx, LYVE_REFERENCE, "Unknown XML prefix \"%.*s\".", (int)prefix_len, prefix);
                ret = LY_EVALID;
                goto cleanup;
            }
        }

        /* get value prefixes */
        val_prefix_data = NULL;
        LY_CHECK_GOTO(ret = ly_store_prefix_data(xmlctx->ctx, xmlctx->value, xmlctx->value_len, LY_VALUE_XML,
                &xmlctx->ns, &format, &val_prefix_data), cleanup);

        /* attr2 is always changed to the created attribute */
        ret = lyd_create_attr(NULL, &attr2, xmlctx->ctx, name, name_len, prefix, prefix_len, ns ? ns->uri : NULL,
                ns ? strlen(ns->uri) : 0, xmlctx->value, xmlctx->value_len, &xmlctx->dynamic, format, val_prefix_data,
                LYD_HINT_DATA);
        LY_CHECK_GOTO(ret, cleanup);

        if (!*attr) {
            *attr = attr2;
        }

        /* next attribute */
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
    }

cleanup:
    if (ret) {
        lyd_free_attr_siblings(xmlctx->ctx, *attr);
        *attr = NULL;
    }
    return ret;
}

static LY_ERR
lydxml_check_list(struct lyxml_ctx *xmlctx, const struct lysc_node *list)
{
    LY_ERR ret = LY_SUCCESS, r;
    enum LYXML_PARSER_STATUS next;
    struct ly_set key_set = {0};
    const struct lysc_node *snode;
    uint32_t i, parents_count;

    assert(list && (list->nodetype == LYS_LIST));

    /* get all keys into a set (keys do not have if-features or anything) */
    snode = NULL;
    while ((snode = lys_getnext(snode, list, NULL, 0)) && (snode->flags & LYS_KEY)) {
        ret = ly_set_add(&key_set, (void *)snode, 1, NULL);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* remember parent count */
    parents_count = xmlctx->elements.count;

    while (xmlctx->status == LYXML_ELEMENT) {
        /* find key definition */
        for (i = 0; i < key_set.count; ++i) {
            snode = (const struct lysc_node *)key_set.objs[i];
            if (!ly_strncmp(snode->name, xmlctx->name, xmlctx->name_len)) {
                break;
            }
        }
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);

        /* skip attributes */
        while (xmlctx->status == LYXML_ATTRIBUTE) {
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
            assert(xmlctx->status == LYXML_ATTR_CONTENT);
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
        }

        assert(xmlctx->status == LYXML_ELEM_CONTENT);
        if (i < key_set.count) {
            /* validate the value */
            r = ly_value_validate(NULL, snode, xmlctx->value, xmlctx->value_len, LY_VALUE_XML, &xmlctx->ns, LYD_HINT_DATA);
            if (!r) {
                /* key with a valid value, remove from the set */
                ly_set_rm_index(&key_set, i, NULL);
            }
        }

        /* parser next */
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);

        /* skip any children, resursively */
        while (xmlctx->status == LYXML_ELEMENT) {
            while (parents_count < xmlctx->elements.count) {
                LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
            }
            assert(xmlctx->status == LYXML_ELEM_CLOSE);
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
        }

        /* parser next, but do not parse closing element of the list because it would remove its namespaces */
        assert(xmlctx->status == LYXML_ELEM_CLOSE);
        LY_CHECK_GOTO(ret = lyxml_ctx_peek(xmlctx, &next), cleanup);
        if (next != LYXML_ELEM_CLOSE) {
            LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), cleanup);
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
 * @brief Skip an element with all its descendants.
 *
 * @param[in] xmlctx XML parser context.
 * @return LY_ERR value.
 */
static LY_ERR
lydxml_data_skip(struct lyxml_ctx *xmlctx)
{
    uint32_t parents_count;

    /* remember current number of parents */
    parents_count = xmlctx->elements.count;
    assert(parents_count);

    /* skip after the content */
    while (xmlctx->status != LYXML_ELEM_CONTENT) {
        LY_CHECK_RET(lyxml_ctx_next(xmlctx));
    }
    LY_CHECK_RET(lyxml_ctx_next(xmlctx));

    /* skip all children elements, recursively, if any */
    while (parents_count <= xmlctx->elements.count) {
        LY_CHECK_RET(lyxml_ctx_next(xmlctx));
    }

    /* close element */
    assert(xmlctx->status == LYXML_ELEM_CLOSE);
    LY_CHECK_RET(lyxml_ctx_next(xmlctx));

    return LY_SUCCESS;
}

/**
 * @brief Check that the current element can be parsed as a data node.
 *
 * @param[in] lydctx XML data parser context.
 * @param[in,out] snode Found schema node, set to NULL if data node cannot be created.
 * @return LY_ERR value.
 */
static LY_ERR
lydxml_data_check_opaq(struct lyd_xml_ctx *lydctx, const struct lysc_node **snode)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxml_ctx *xmlctx = lydctx->xmlctx, pxmlctx;

    if (!(lydctx->parse_opts & LYD_PARSE_OPAQ)) {
        /* only checks specific to opaque nodes */
        return LY_SUCCESS;
    }

    if (!((*snode)->nodetype & (LYD_NODE_TERM | LYD_NODE_INNER))) {
        /* nothing to check */
        return LY_SUCCESS;
    }

    assert(xmlctx->elements.count);

    /* backup parser */
    LY_CHECK_RET(lyxml_ctx_backup(xmlctx, &pxmlctx));

    /* skip attributes */
    while (xmlctx->status == LYXML_ATTRIBUTE) {
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), restore);
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), restore);
    }

    if ((*snode)->nodetype & LYD_NODE_TERM) {
        /* value may not be valid in which case we parse it as an opaque node */
        if (ly_value_validate(NULL, *snode, xmlctx->value, xmlctx->value_len, LY_VALUE_XML, &xmlctx->ns, LYD_HINT_DATA)) {
            LOGVRB("Parsing opaque term node \"%s\" with invalid value \"%.*s\".", (*snode)->name, (int)xmlctx->value_len,
                    xmlctx->value);
            *snode = NULL;
        }
    } else if ((*snode)->nodetype == LYS_LIST) {
        /* skip content */
        LY_CHECK_GOTO(ret = lyxml_ctx_next(xmlctx), restore);

        if (lydxml_check_list(xmlctx, *snode)) {
            /* invalid list, parse as opaque if it missing/has invalid some keys */
            LOGVRB("Parsing opaque list node \"%s\" with missing/invalid keys.", (*snode)->name);
            *snode = NULL;
        }
    } else {
        /* if there is a non-WS value, it cannot be parsed as an inner node */
        assert(xmlctx->status == LYXML_ELEM_CONTENT);
        if (!xmlctx->ws_only) {
            *snode = NULL;
        }
    }

restore:
    /* restore parser */
    lyxml_ctx_restore(xmlctx, &pxmlctx);
    return ret;
}

/**
 * @brief Get sensible data hints for an opaque node.
 *
 * @param[in] name Node name.
 * @param[in] name_len Length of @p name.
 * @param[in] value Node value.
 * @param[in] value_len Length of @p value.
 * @param[in] first Node first sibling.
 * @param[in] ns Node module namespace, NULL for no namespace.
 * @param[out] hints Data hints to use.
 * @param[out] anchor Anchor to insert after in case of a list.
 */
static void
lydxml_get_hints_opaq(const char *name, size_t name_len, const char *value, size_t value_len, const struct lyd_node *first,
        const char *ns, uint32_t *hints, struct lyd_node **anchor)
{
    struct lyd_node_opaq *opaq;
    char *ptr;
    long num;

    *hints = 0;
    *anchor = NULL;

    if (!value_len) {
        /* no value but it may also be zero-length string */
        *hints |= LYD_VALHINT_EMPTY | LYD_VALHINT_STRING;
    } else if (!strncmp(value, "true", value_len) || !strncmp(value, "false", value_len)) {
        /* boolean value */
        *hints |= LYD_VALHINT_BOOLEAN;
    } else {
        num = strtol(value, &ptr, 10);
        if ((unsigned)(ptr - value) == value_len) {
            /* number value */
            *hints |= LYD_VALHINT_DECNUM;
            if ((num < INT32_MIN) || (num > UINT32_MAX)) {
                /* large number */
                *hints |= LYD_VALHINT_NUM64;
            }
        } else {
            /* string value */
            *hints |= LYD_VALHINT_STRING;
        }
    }

    if (!first) {
        return;
    }

    /* search backwards to find the last instance */
    do {
        first = first->prev;
        if (first->schema) {
            continue;
        }

        opaq = (struct lyd_node_opaq *)first;
        assert(opaq->format == LY_VALUE_XML);
        if (!ly_strncmp(opaq->name.name, name, name_len) &&
                ((ns && !strcmp(opaq->name.module_ns, ns)) || (!ns && !opaq->name.module_ns))) {
            if (opaq->value && opaq->value[0]) {
                /* leaf-list nodes */
                opaq->hints |= LYD_NODEHINT_LEAFLIST;
                *hints |= LYD_NODEHINT_LEAFLIST;
            } else {
                /* list nodes */
                opaq->hints |= LYD_NODEHINT_LIST;
                *hints |= LYD_NODEHINT_LIST;
            }
            *anchor = (struct lyd_node *)first;
            break;
        }
    } while (first->prev->next);
}

/**
 * @brief Get schema node for the current element.
 *
 * @param[in] lydctx XML data parser context.
 * @param[in] parent Parsed parent data node, if any.
 * @param[in] prefix Element prefix, if any.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in] name Element name.
 * @param[in] name_len Length of @p name.
 * @param[out] snode Found schema node, NULL if no suitable was found.
 * @param[out] ext Extension instance that provided @p snode, if any.
 * @return LY_SUCCESS on success;
 * @return LY_ERR on error.
 */
static LY_ERR
lydxml_subtree_get_snode(struct lyd_xml_ctx *lydctx, const struct lyd_node *parent, const char *prefix, size_t prefix_len,
        const char *name, size_t name_len, const struct lysc_node **snode, struct lysc_ext_instance **ext)
{
    LY_ERR r;
    struct lyxml_ctx *xmlctx;
    const struct ly_ctx *ctx;
    const struct lyxml_ns *ns;
    struct lys_module *mod;
    uint32_t getnext_opts;

    xmlctx = lydctx->xmlctx;
    ctx = xmlctx->ctx;
    getnext_opts = lydctx->int_opts & LYD_INTOPT_REPLY ? LYS_GETNEXT_OUTPUT : 0;

    *snode = NULL;
    *ext = NULL;

    /* get current namespace */
    ns = lyxml_ns_get(&xmlctx->ns, prefix, prefix_len);
    if (!ns) {
        if (lydctx->int_opts & LYD_INTOPT_ANY) {
            goto unknown_module;
        }

        if (prefix_len) {
            LOGVAL(ctx, LYVE_REFERENCE, "Unknown XML prefix \"%.*s\".", (int)prefix_len, prefix);
        } else {
            LOGVAL(ctx, LYVE_REFERENCE, "Missing XML namespace.");
        }
        return LY_EVALID;
    }

    /* get the element module, use parent context if possible because of extensions */
    mod = ly_ctx_get_module_implemented_ns(parent ? LYD_CTX(parent) : ctx, ns->uri);
    if (!mod) {
        /* check for extension data */
        r = ly_nested_ext_schema(parent, NULL, prefix, prefix_len, LY_VALUE_XML, &lydctx->xmlctx->ns, name, name_len,
                snode, ext);
        if (r != LY_ENOT) {
            /* success or error */
            return r;
        }

unknown_module:
        if (lydctx->parse_opts & LYD_PARSE_STRICT) {
            if (ns) {
                LOGVAL(ctx, LYVE_REFERENCE, "No module with namespace \"%s\" in the context.", ns->uri);
            } else if (prefix_len) {
                LOGVAL(ctx, LYVE_REFERENCE, "No module with namespace \"%.*s\" in the context.", (int)prefix_len, prefix);
            } else {
                LOGVAL(ctx, LYVE_REFERENCE, "No default namespace in the context.");
            }
            return LY_EVALID;
        }
        return LY_SUCCESS;
    }

    /* get the schema node */
    if (!parent && lydctx->ext) {
        *snode = lysc_ext_find_node(lydctx->ext, mod, name, name_len, 0, getnext_opts);
    } else {
        /* try to find parent schema node even if it is an opaque node (not connected to the parent) */
        *snode = lys_find_child(lyd_parser_node_schema(parent), mod, name, name_len, 0, getnext_opts);
    }
    if (!*snode) {
        /* check for extension data */
        r = ly_nested_ext_schema(parent, NULL, prefix, prefix_len, LY_VALUE_XML, &lydctx->xmlctx->ns, name,
                name_len, snode, ext);
        if (r != LY_ENOT) {
            /* success or error */
            return r;
        }

        /* unknown data node */
        if (lydctx->parse_opts & LYD_PARSE_STRICT) {
            if (parent) {
                LOGVAL(ctx, LYVE_REFERENCE, "Node \"%.*s\" not found as a child of \"%s\" node.",
                        (int)name_len, name, LYD_NAME(parent));
            } else if (lydctx->ext) {
                if (lydctx->ext->argument) {
                    LOGVAL(ctx, LYVE_REFERENCE, "Node \"%.*s\" not found in the \"%s\" %s extension instance.",
                            (int)name_len, name, lydctx->ext->argument, lydctx->ext->def->name);
                } else {
                    LOGVAL(ctx, LYVE_REFERENCE, "Node \"%.*s\" not found in the %s extension instance.",
                            (int)name_len, name, lydctx->ext->def->name);
                }
            } else {
                LOGVAL(ctx, LYVE_REFERENCE, "Node \"%.*s\" not found in the \"%s\" module.",
                        (int)name_len, name, mod->name);
            }
            return LY_EVALID;
        }
        return LY_SUCCESS;
    } else {
        /* check that schema node is valid and can be used */
        LY_CHECK_RET(lyd_parser_check_schema((struct lyd_ctx *)lydctx, *snode));
        LY_CHECK_RET(lydxml_data_check_opaq(lydctx, snode));
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse an XML opque node.
 *
 * @param[in] lydctx XML YANG data parser context.
 * @param[in] sibling Existing sibling node, if any.
 * @param[in] prefix Parsed node prefix.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in] name Parsed node name.
 * @param[in] name_len Length of @p name.
 * @param[out] insert_anchor Optional anchor node for inserting this node.
 * @param[out] node Created node.
 * @return LY_ERR value.
 */
static LY_ERR
lydxml_subtree_opaq(struct lyd_xml_ctx *lydctx, const struct lyd_node *sibling, const char *prefix, uint32_t prefix_len,
        const char *name, uint32_t name_len, struct lyd_node **insert_anchor, struct lyd_node **node)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyxml_ctx *xmlctx = lydctx->xmlctx;
    struct lyd_node_opaq *opaq;
    const char *ns_uri, *value = NULL;
    size_t value_len;
    ly_bool ws_only, dynamic = 0;
    const struct lyxml_ns *ns;
    uint32_t hints;
    void *val_prefix_data = NULL;
    LY_VALUE_FORMAT format;

    assert(lydctx->parse_opts & LYD_PARSE_OPAQ);

    *node = NULL;

    /* remember the value */
    value = xmlctx->value;
    value_len = xmlctx->value_len;
    ws_only = xmlctx->ws_only;
    dynamic = xmlctx->dynamic;
    if (dynamic) {
        xmlctx->dynamic = 0;
    }

    /* get value prefixes, if any */
    rc = ly_store_prefix_data(xmlctx->ctx, value, value_len, LY_VALUE_XML, &xmlctx->ns, &format, &val_prefix_data);
    LY_CHECK_GOTO(rc, cleanup);

    /* get NS again, it may have been backed up and restored */
    ns = lyxml_ns_get(&xmlctx->ns, prefix, prefix_len);
    ns_uri = ns ? ns->uri : NULL;

    /* get best-effort node hints */
    lydxml_get_hints_opaq(name, name_len, xmlctx->value, xmlctx->value_len, sibling, ns_uri, &hints, insert_anchor);

    /* create the node without value */
    rc = lyd_create_opaq(xmlctx->ctx, name, name_len, prefix, prefix_len, ns_uri, ns_uri ? strlen(ns_uri) : 0, NULL, 0,
            NULL, format, NULL, hints, node);
    LY_CHECK_GOTO(rc, cleanup);

    assert(*node);
    LOG_LOCSET(NULL, *node, NULL, NULL);

    /* parser next */
    rc = lyxml_ctx_next(xmlctx);
    LY_CHECK_GOTO(rc, cleanup);

    /* process children */
    while (xmlctx->status == LYXML_ELEMENT) {
        rc = lydxml_subtree_r(lydctx, *node, lyd_node_child_p(*node), NULL);
        LY_CHECK_GOTO(rc, cleanup);
    }

    /* update the value */
    opaq = (struct lyd_node_opaq *)*node;
    if (opaq->child) {
        if (!ws_only) {
            LOGVAL(xmlctx->ctx, LYVE_SYNTAX_XML, "Mixed XML content node \"%s\" found, not supported.", LYD_NAME(opaq));
            rc = LY_EVALID;
            goto cleanup;
        }
    } else if (value_len) {
        lydict_remove(xmlctx->ctx, opaq->value);
        if (dynamic) {
            LY_CHECK_GOTO(rc = lydict_insert_zc(xmlctx->ctx, (char *)value, &opaq->value), cleanup);
            dynamic = 0;
        } else {
            LY_CHECK_GOTO(rc = lydict_insert(xmlctx->ctx, value, value_len, &opaq->value), cleanup);
        }
    }

    /* always store val_prefix_data because the format requires them */
    assert(!opaq->val_prefix_data);
    opaq->val_prefix_data = val_prefix_data;
    val_prefix_data = NULL;

cleanup:
    if (*node) {
        LOG_LOCBACK(0, 1, 0, 0);
    }
    ly_free_prefix_data(format, val_prefix_data);
    if (dynamic) {
        free((char *)value);
    }
    if (rc) {
        lyd_free_tree(*node);
        *node = NULL;
    }
    return rc;
}

/**
 * @brief Parse an XML leaf/leaf-list node.
 *
 * @param[in] lydctx XML YANG data parser context.
 * @param[in] parent Parent node, if any.
 * @param[in] snode Schema node of the new node.
 * @param[out] node Created node.
 * @return LY_ERR value.
 */
static LY_ERR
lydxml_subtree_term(struct lyd_xml_ctx *lydctx, struct lyd_node *parent, const struct lysc_node *snode,
        struct lyd_node **node)
{
    LY_ERR r, rc = LY_SUCCESS;
    struct lyxml_ctx *xmlctx = lydctx->xmlctx;
    struct lyd_node *anchor;

    *node = NULL;

    /* create node */
    r = lyd_parser_create_term((struct lyd_ctx *)lydctx, snode, xmlctx->value, xmlctx->value_len, &xmlctx->dynamic,
            LY_VALUE_XML, &xmlctx->ns, LYD_HINT_DATA, node);
    LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);

    if (*node) {
        LOG_LOCSET(NULL, *node, NULL, NULL);
    }

    if (*node && parent && (snode->flags & LYS_KEY)) {
        /* check the key order, the anchor must never be a key */
        anchor = lyd_insert_get_next_anchor(lyd_child(parent), *node);
        if (anchor && anchor->schema && (anchor->schema->flags & LYS_KEY)) {
            if (lydctx->parse_opts & LYD_PARSE_STRICT) {
                LOGVAL(xmlctx->ctx, LYVE_DATA, "Invalid position of the key \"%s\" in a list.", snode->name);
                r = LY_EVALID;
                LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
            } else {
                LOGWRN(xmlctx->ctx, "Invalid position of the key \"%s\" in a list.", snode->name);
            }
        }
    }

    /* parser next */
    r = lyxml_ctx_next(xmlctx);
    LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

    /* no children expected */
    if (xmlctx->status == LYXML_ELEMENT) {
        LOGVAL(xmlctx->ctx, LYVE_SYNTAX, "Child element \"%.*s\" inside a terminal node \"%s\" found.",
                (int)xmlctx->name_len, xmlctx->name, snode->name);
        r = LY_EVALID;
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
    }

cleanup:
    if (*node) {
        LOG_LOCBACK(0, 1, 0, 0);
    }
    if (rc && (!(lydctx->val_opts & LYD_VALIDATE_MULTI_ERROR) || (rc != LY_EVALID))) {
        lyd_free_tree(*node);
        *node = NULL;
    }
    return rc;
}

/**
 * @brief Parse an XML inner node.
 *
 * @param[in] lydctx XML YANG data parser context.
 * @param[in] snode Schema node of the new node.
 * @param[in] ext Extension instance of @p snode, if any.
 * @param[out] node Created node.
 * @return LY_ERR value.
 */
static LY_ERR
lydxml_subtree_inner(struct lyd_xml_ctx *lydctx, const struct lysc_node *snode, const struct lysc_ext_instance *ext,
        struct lyd_node **node)
{
    LY_ERR r, rc = LY_SUCCESS;
    struct lyxml_ctx *xmlctx = lydctx->xmlctx;
    uint32_t prev_parse_opts = lydctx->parse_opts;

    *node = NULL;

    if (!xmlctx->ws_only) {
        /* value in inner node */
        LOGVAL(xmlctx->ctx, LYVE_SYNTAX, "Text value \"%.*s\" inside an inner node \"%s\" found.",
                (int)xmlctx->value_len, xmlctx->value, snode->name);
        r = LY_EVALID;
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
    }

    /* create node */
    rc = lyd_create_inner(snode, node);
    LY_CHECK_GOTO(rc, cleanup);

    assert(*node);
    LOG_LOCSET(NULL, *node, NULL, NULL);

    /* parser next */
    rc = lyxml_ctx_next(xmlctx);
    LY_CHECK_GOTO(rc, cleanup);

    if (ext) {
        /* only parse these extension data and validate afterwards */
        lydctx->parse_opts |= LYD_PARSE_ONLY;
    }

    /* process children */
    while (xmlctx->status == LYXML_ELEMENT) {
        r = lydxml_subtree_r(lydctx, *node, lyd_node_child_p(*node), NULL);
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
    }

    /* restore options */
    lydctx->parse_opts = prev_parse_opts;

    if (snode->nodetype == LYS_LIST) {
        /* check all keys exist */
        r = lyd_parse_check_keys(*node);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
    }

    if (!(lydctx->parse_opts & LYD_PARSE_ONLY) && !rc) {
        /* new node validation, autodelete CANNOT occur (it can if multi-error), all nodes are new */
        r = lyd_validate_new(lyd_node_child_p(*node), snode, NULL, lydctx->val_opts, NULL);
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);

        /* add any missing default children */
        r = lyd_new_implicit_r(*node, lyd_node_child_p(*node), NULL, NULL, &lydctx->node_when, &lydctx->node_types,
                &lydctx->ext_node, (lydctx->val_opts & LYD_VALIDATE_NO_STATE) ? LYD_IMPLICIT_NO_STATE : 0, NULL);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
    }

    if (snode->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) {
        /* rememeber the RPC/action/notification */
        lydctx->op_node = *node;
    }

cleanup:
    if (*node) {
        LOG_LOCBACK(0, 1, 0, 0);
    }
    lydctx->parse_opts = prev_parse_opts;
    if (rc && ((*node && !(*node)->hash) || !(lydctx->val_opts & LYD_VALIDATE_MULTI_ERROR) || (rc != LY_EVALID))) {
        /* list without keys is unusable or an error */
        lyd_free_tree(*node);
        *node = NULL;
    }
    return rc;
}

/**
 * @brief Parse an XML anyxml/anydata node.
 *
 * @param[in] lydctx XML YANG data parser context.
 * @param[in] snode Schema node of the new node.
 * @param[in] ext Extension instance of @p snode, if any.
 * @param[out] node Created node.
 * @return LY_ERR value.
 */
static LY_ERR
lydxml_subtree_any(struct lyd_xml_ctx *lydctx, const struct lysc_node *snode, const struct lysc_ext_instance *ext,
        struct lyd_node **node)
{
    LY_ERR r, rc = LY_SUCCESS;
    struct lyxml_ctx *xmlctx = lydctx->xmlctx;
    uint32_t prev_parse_opts = lydctx->parse_opts, prev_int_opts = lydctx->int_opts;
    struct lyd_node *child = NULL;
    char *val = NULL;
    ly_bool log_node = 0;

    *node = NULL;

    if ((snode->nodetype == LYS_ANYDATA) && !xmlctx->ws_only) {
        /* value in anydata node, we expect a tree */
        LOGVAL(xmlctx->ctx, LYVE_SYNTAX, "Text value \"%.*s\" inside an anydata node \"%s\" found.",
                xmlctx->value_len < 20 ? (int)xmlctx->value_len : 20, xmlctx->value, snode->name);
        r = LY_EVALID;
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
    }

    if (!xmlctx->ws_only) {
        /* use an arbitrary text value for anyxml */
        val = strndup(xmlctx->value, xmlctx->value_len);
        LY_CHECK_ERR_GOTO(!val, LOGMEM(xmlctx->ctx); rc = LY_EMEM, cleanup);

        /* parser next */
        r = lyxml_ctx_next(xmlctx);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

        /* create node */
        r = lyd_create_any(snode, val, LYD_ANYDATA_STRING, 1, node);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
        val = NULL;
    } else {
        /* create node */
        r = lyd_create_any(snode, NULL, LYD_ANYDATA_DATATREE, 1, node);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

        assert(*node);
        LOG_LOCSET(NULL, *node, NULL, NULL);
        log_node = 1;

        /* parser next */
        r = lyxml_ctx_next(xmlctx);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

        /* update options so that generic data can be parsed */
        lydctx->parse_opts &= ~LYD_PARSE_STRICT;
        lydctx->parse_opts |= LYD_PARSE_OPAQ | (ext ? LYD_PARSE_ONLY : 0);
        lydctx->int_opts |= LYD_INTOPT_ANY | LYD_INTOPT_WITH_SIBLINGS;

        /* parse any data tree */
        while (xmlctx->status == LYXML_ELEMENT) {
            r = lydxml_subtree_r(lydctx, NULL, &child, NULL);
            LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
        }

        /* assign the data tree */
        ((struct lyd_node_any *)*node)->value.tree = child;
        child = NULL;
    }

cleanup:
    if (log_node) {
        LOG_LOCBACK(0, 1, 0, 0);
    }
    lydctx->parse_opts = prev_parse_opts;
    lydctx->int_opts = prev_int_opts;
    free(val);
    lyd_free_tree(child);
    if (rc && (!(lydctx->val_opts & LYD_VALIDATE_MULTI_ERROR) || (rc != LY_EVALID))) {
        lyd_free_tree(*node);
        *node = NULL;
    }
    return rc;
}

/**
 * @brief Parse an XML subtree, recursively.
 *
 * @param[in] lydctx XML YANG data parser context.
 * @param[in,out] parent Parent node where the children are inserted. NULL in case of parsing top-level elements.
 * @param[in,out] first_p Pointer to the first (@p parent or top-level) child.
 * @param[in,out] parsed Optional set to add all the parsed siblings into.
 * @return LY_ERR value.
 */
static LY_ERR
lydxml_subtree_r(struct lyd_xml_ctx *lydctx, struct lyd_node *parent, struct lyd_node **first_p, struct ly_set *parsed)
{
    LY_ERR r, rc = LY_SUCCESS;
    const char *prefix, *name;
    size_t prefix_len, name_len;
    struct lyxml_ctx *xmlctx;
    const struct ly_ctx *ctx;
    struct lyd_meta *meta = NULL;
    struct lyd_attr *attr = NULL;
    const struct lysc_node *snode = NULL;
    struct lysc_ext_instance *ext = NULL;
    uint32_t orig_parse_opts;
    struct lyd_node *node = NULL, *insert_anchor = NULL;
    ly_bool parse_subtree;

    assert(parent || first_p);

    xmlctx = lydctx->xmlctx;
    ctx = xmlctx->ctx;

    parse_subtree = lydctx->parse_opts & LYD_PARSE_SUBTREE ? 1 : 0;
    /* all descendants should be parsed */
    lydctx->parse_opts &= ~LYD_PARSE_SUBTREE;
    orig_parse_opts = lydctx->parse_opts;

    assert(xmlctx->status == LYXML_ELEMENT);

    /* remember element prefix and name */
    prefix = xmlctx->prefix;
    prefix_len = xmlctx->prefix_len;
    name = xmlctx->name;
    name_len = xmlctx->name_len;

    /* parser next */
    rc = lyxml_ctx_next(xmlctx);
    LY_CHECK_GOTO(rc, cleanup);

    if ((lydctx->int_opts & LYD_INTOPT_EVENTTIME) && !parent && name_len && !prefix_len &&
            !ly_strncmp("eventTime", name, name_len)) {
        /* parse eventTime, create node */
        assert(xmlctx->status == LYXML_ELEM_CONTENT);
        rc = lyd_create_opaq(xmlctx->ctx, name, name_len, prefix, prefix_len,
                "urn:ietf:params:xml:ns:netconf:notification:1.0", 47, xmlctx->value,
                xmlctx->ws_only ? 0 : xmlctx->value_len, NULL, LY_VALUE_XML, NULL, LYD_HINT_DATA, &node);
        LY_CHECK_GOTO(rc, cleanup);

        /* validate the value */
        r = lyd_parser_notif_eventtime_validate(node);
        LY_CHECK_ERR_GOTO(r, rc = r; lyd_free_tree(node), cleanup);

        /* parser next */
        r = lyxml_ctx_next(xmlctx);
        LY_CHECK_ERR_GOTO(r, rc = r; lyd_free_tree(node), cleanup);
        if (xmlctx->status != LYXML_ELEM_CLOSE) {
            LOGVAL(ctx, LYVE_DATA, "Unexpected notification \"eventTime\" node children.");
            rc = LY_EVALID;
            lyd_free_tree(node);
            goto cleanup;
        }

        goto node_parsed;
    }

    /* get the schema node */
    r = lydxml_subtree_get_snode(lydctx, parent, prefix, prefix_len, name, name_len, &snode, &ext);
    if (r) {
        rc = r;
        if ((r == LY_EVALID) && (lydctx->val_opts & LYD_VALIDATE_MULTI_ERROR)) {
            /* skip the invalid data */
            if ((r = lydxml_data_skip(xmlctx))) {
                rc = r;
            }
        }
        goto cleanup;
    } else if (!snode && !(lydctx->parse_opts & LYD_PARSE_OPAQ)) {
        LOGVRB("Skipping parsing of unknown node \"%.*s\".", (int)name_len, name);

        /* skip element with children */
        rc = lydxml_data_skip(xmlctx);
        goto cleanup;
    }

    /* create metadata/attributes */
    if (xmlctx->status == LYXML_ATTRIBUTE) {
        if (snode) {
            rc = lydxml_metadata(lydctx, snode, &meta);
            LY_CHECK_GOTO(rc, cleanup);
        } else {
            assert(lydctx->parse_opts & LYD_PARSE_OPAQ);
            rc = lydxml_attrs(xmlctx, &attr);
            LY_CHECK_GOTO(rc, cleanup);
        }
    }

    assert(xmlctx->status == LYXML_ELEM_CONTENT);
    if (!snode) {
        /* opaque */
        r = lydxml_subtree_opaq(lydctx, parent ? lyd_child(parent) : *first_p, prefix, prefix_len, name, name_len,
                &insert_anchor, &node);
    } else if (snode->nodetype & LYD_NODE_TERM) {
        /* term */
        r = lydxml_subtree_term(lydctx, parent, snode, &node);
    } else if (snode->nodetype & LYD_NODE_INNER) {
        /* inner */
        r = lydxml_subtree_inner(lydctx, snode, ext, &node);
    } else {
        /* any */
        assert(snode->nodetype & LYD_NODE_ANY);
        r = lydxml_subtree_any(lydctx, snode, ext, &node);
    }
    LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);

node_parsed:
    if (node && snode) {
        /* add/correct flags */
        r = lyd_parse_set_data_flags(node, &meta, (struct lyd_ctx *)lydctx, ext);
        LY_CHECK_ERR_GOTO(r, rc = r; lyd_free_tree(node), cleanup);

        if (!(lydctx->parse_opts & LYD_PARSE_ONLY)) {
            /* store for ext instance node validation, if needed */
            r = lyd_validate_node_ext(node, &lydctx->ext_node);
            LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
        }
    }

    /* parser next */
    assert(xmlctx->status == LYXML_ELEM_CLOSE);
    if (!parse_subtree) {
        r = lyxml_ctx_next(xmlctx);
        LY_CHECK_ERR_GOTO(r, rc = r; lyd_free_tree(node), cleanup);
    }

    LY_CHECK_GOTO(!node, cleanup);

    /* add metadata/attributes */
    if (snode) {
        lyd_insert_meta(node, meta, 0);
        meta = NULL;
    } else {
        lyd_insert_attr(node, attr);
        attr = NULL;
    }

    /* insert, keep first pointer correct */
    if (insert_anchor) {
        lyd_insert_after(insert_anchor, node);
    } else if (ext) {
        r = lyplg_ext_insert(parent, node);
        LY_CHECK_ERR_GOTO(r, rc = r; lyd_free_tree(node), cleanup);
    } else {
        lyd_insert_node(parent, first_p, node, lydctx->parse_opts & LYD_PARSE_ORDERED ? 1 : 0);
    }
    while (!parent && (*first_p)->prev->next) {
        *first_p = (*first_p)->prev;
    }

    /* rememeber a successfully parsed node */
    if (parsed) {
        ly_set_add(parsed, node, 1, NULL);
    }

cleanup:
    lydctx->parse_opts = orig_parse_opts;
    lyd_free_meta_siblings(meta);
    lyd_free_attr_siblings(ctx, attr);
    return rc;
}

/**
 * @brief Parse a specific XML element into an opaque node.
 *
 * @param[in] xmlctx XML parser context.
 * @param[in] name Name of the element.
 * @param[in] uri URI of the element.
 * @param[in] value Whether a value is expected in the element.
 * @param[out] evnp Parsed envelope (opaque node).
 * @return LY_SUCCESS on success.
 * @return LY_ENOT if the specified element did not match.
 * @return LY_ERR value on error.
 */
static LY_ERR
lydxml_envelope(struct lyxml_ctx *xmlctx, const char *name, const char *uri, ly_bool value, struct lyd_node **envp)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lyxml_ns *ns;
    struct lyd_attr *attr = NULL;
    const char *prefix;
    size_t prefix_len;

    if (xmlctx->status != LYXML_ELEMENT) {
        /* nothing to parse */
        return LY_ENOT;
    }

    if (ly_strncmp(name, xmlctx->name, xmlctx->name_len)) {
        /* not the expected element */
        return LY_ENOT;
    }

    prefix = xmlctx->prefix;
    prefix_len = xmlctx->prefix_len;
    ns = lyxml_ns_get(&xmlctx->ns, prefix, prefix_len);
    if (!ns) {
        LOGVAL(xmlctx->ctx, LYVE_REFERENCE, "Unknown XML prefix \"%.*s\".", (int)prefix_len, prefix);
        return LY_EVALID;
    } else if (strcmp(ns->uri, uri)) {
        /* different namespace */
        return LY_ENOT;
    }

    LY_CHECK_RET(lyxml_ctx_next(xmlctx));

    /* create attributes */
    if (xmlctx->status == LYXML_ATTRIBUTE) {
        LY_CHECK_RET(lydxml_attrs(xmlctx, &attr));
    }

    assert(xmlctx->status == LYXML_ELEM_CONTENT);
    if (!value && !xmlctx->ws_only) {
        LOGVAL(xmlctx->ctx, LYVE_SYNTAX, "Unexpected value \"%.*s\" in the \"%s\" element.",
                (int)xmlctx->value_len, xmlctx->value, name);
        rc = LY_EVALID;
        goto cleanup;
    }

    /* create node */
    rc = lyd_create_opaq(xmlctx->ctx, name, strlen(name), prefix, prefix_len, uri, strlen(uri), xmlctx->value,
            xmlctx->ws_only ? 0 : xmlctx->value_len, NULL, LY_VALUE_XML, NULL, 0, envp);
    LY_CHECK_GOTO(rc, cleanup);

    /* assign atributes */
    ((struct lyd_node_opaq *)(*envp))->attr = attr;
    attr = NULL;

    /* parser next element */
    LY_CHECK_GOTO(rc = lyxml_ctx_next(xmlctx), cleanup);

cleanup:
    lyd_free_attr_siblings(xmlctx->ctx, attr);
    if (rc) {
        lyd_free_tree(*envp);
        *envp = NULL;
    }
    return rc;
}

LY_ERR
lyd_parse_xml(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
        struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, uint32_t int_opts,
        struct ly_set *parsed, ly_bool *subtree_sibling, struct lyd_ctx **lydctx_p)
{
    LY_ERR r, rc = LY_SUCCESS;
    struct lyd_xml_ctx *lydctx;
    ly_bool parsed_data_nodes = 0, close_elem = 0;
    struct lyd_node *act = NULL;
    enum LYXML_PARSER_STATUS status;

    assert(ctx && in && lydctx_p);
    assert(!(parse_opts & ~LYD_PARSE_OPTS_MASK));
    assert(!(val_opts & ~LYD_VALIDATE_OPTS_MASK));

    /* init context */
    lydctx = calloc(1, sizeof *lydctx);
    LY_CHECK_ERR_RET(!lydctx, LOGMEM(ctx), LY_EMEM);
    LY_CHECK_GOTO(rc = lyxml_ctx_new(ctx, in, &lydctx->xmlctx), cleanup);
    lydctx->parse_opts = parse_opts;
    lydctx->val_opts = val_opts;
    lydctx->int_opts = int_opts;
    lydctx->free = lyd_xml_ctx_free;
    lydctx->ext = ext;

    /* find the operation node if it exists already */
    LY_CHECK_GOTO(rc = lyd_parser_find_operation(parent, int_opts, &lydctx->op_node), cleanup);

    if ((int_opts & LYD_INTOPT_RPC) && (int_opts & LYD_INTOPT_ACTION)) {
        /* can be either, try to parse "action" */
        if (!lydxml_envelope(lydctx->xmlctx, "action", "urn:ietf:params:xml:ns:yang:1", 0, &act)) {
            close_elem = 1;
            int_opts &= ~LYD_INTOPT_RPC;
        }
    }

    /* parse XML data */
    while (lydctx->xmlctx->status == LYXML_ELEMENT) {
        r = lydxml_subtree_r(lydctx, parent, first_p, parsed);
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);

        parsed_data_nodes = 1;

        if (!(int_opts & LYD_INTOPT_WITH_SIBLINGS)) {
            break;
        }
    }

    /* close an opened element */
    if (close_elem) {
        if (lydctx->xmlctx->status != LYXML_ELEM_CLOSE) {
            assert(lydctx->xmlctx->status == LYXML_ELEMENT);
            LOGVAL(lydctx->xmlctx->ctx, LYVE_SYNTAX, "Unexpected child element \"%.*s\".",
                    (int)lydctx->xmlctx->name_len, lydctx->xmlctx->name);
            rc = LY_EVALID;
            goto cleanup;
        }

        LY_CHECK_GOTO(rc = lyxml_ctx_next(lydctx->xmlctx), cleanup);
    }

    /* check final state */
    if ((int_opts & LYD_INTOPT_NO_SIBLINGS) && (lydctx->xmlctx->status == LYXML_ELEMENT)) {
        LOGVAL(ctx, LYVE_SYNTAX, "Unexpected sibling node.");
        r = LY_EVALID;
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
    }
    if ((int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_ACTION | LYD_INTOPT_NOTIF | LYD_INTOPT_REPLY)) && !lydctx->op_node) {
        LOGVAL(ctx, LYVE_DATA, "Missing the operation node.");
        r = LY_EVALID;
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
    }

    if (!parsed_data_nodes) {
        /* no data nodes were parsed */
        lydctx->op_node = NULL;
    }

    if (parse_opts & LYD_PARSE_SUBTREE) {
        /* check for a sibling element */
        assert(subtree_sibling);
        if (!lyxml_ctx_peek(lydctx->xmlctx, &status) && (status == LYXML_ELEMENT)) {
            *subtree_sibling = 1;
        } else {
            *subtree_sibling = 0;
        }
    }

cleanup:
    /* there should be no unres stored if validation should be skipped */
    assert(!(parse_opts & LYD_PARSE_ONLY) || (!lydctx->node_types.count && !lydctx->meta_types.count &&
            !lydctx->node_when.count));

    lyd_free_tree(act);
    if (rc && (!(lydctx->val_opts & LYD_VALIDATE_MULTI_ERROR) || (rc != LY_EVALID))) {
        lyd_xml_ctx_free((struct lyd_ctx *)lydctx);
    } else {
        *lydctx_p = (struct lyd_ctx *)lydctx;

        /* the XML context is no more needed, freeing it also stops logging line numbers which would be confusing now */
        lyxml_ctx_free(lydctx->xmlctx);
        lydctx->xmlctx = NULL;
    }
    return rc;
}

/**
 * @brief Parse all expected non-data XML elements of a NETCONF rpc message.
 *
 * @param[in] xmlctx XML parser context.
 * @param[out] evnp Parsed envelope(s) (opaque node).
 * @param[out] int_opts Internal options for parsing the rest of YANG data.
 * @param[out] close_elem Number of parsed opened elements that need to be closed.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value on error.
 */
static LY_ERR
lydxml_env_netconf_rpc(struct lyxml_ctx *xmlctx, struct lyd_node **envp, uint32_t *int_opts, uint32_t *close_elem)
{
    LY_ERR rc = LY_SUCCESS, r;
    struct lyd_node *child;

    assert(envp && !*envp);

    /* parse "rpc" */
    r = lydxml_envelope(xmlctx, "rpc", "urn:ietf:params:xml:ns:netconf:base:1.0", 0, envp);
    LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

    /* parse "action", if any */
    r = lydxml_envelope(xmlctx, "action", "urn:ietf:params:xml:ns:yang:1", 0, &child);
    if (r == LY_SUCCESS) {
        /* insert */
        lyd_insert_node(*envp, NULL, child, 0);

        /* NETCONF action */
        *int_opts = LYD_INTOPT_NO_SIBLINGS | LYD_INTOPT_ACTION;
        *close_elem = 2;
    } else if (r == LY_ENOT) {
        /* NETCONF RPC */
        *int_opts = LYD_INTOPT_NO_SIBLINGS | LYD_INTOPT_RPC;
        *close_elem = 1;
    } else {
        rc = r;
        goto cleanup;
    }

cleanup:
    if (rc) {
        lyd_free_tree(*envp);
        *envp = NULL;
    }
    return rc;
}

/**
 * @brief Parse an XML element as an opaque node subtree.
 *
 * @param[in] xmlctx XML parser context.
 * @param[in] parent Parent to append nodes to.
 * @return LY_ERR value.
 */
static LY_ERR
lydxml_opaq_r(struct lyxml_ctx *xmlctx, struct lyd_node *parent)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lyxml_ns *ns;
    struct lyd_attr *attr = NULL;
    struct lyd_node *node = NULL;
    struct lyd_node_opaq *opaq;
    const char *name, *prefix, *value = NULL;
    size_t name_len, prefix_len, value_len;
    ly_bool ws_only, dynamic = 0;

    assert(xmlctx->status == LYXML_ELEMENT);

    name = xmlctx->name;
    name_len = xmlctx->name_len;
    prefix = xmlctx->prefix;
    prefix_len = xmlctx->prefix_len;
    ns = lyxml_ns_get(&xmlctx->ns, prefix, prefix_len);
    if (!ns) {
        LOGVAL(xmlctx->ctx, LYVE_REFERENCE, "Unknown XML prefix \"%.*s\".", (int)prefix_len, prefix);
        return LY_EVALID;
    }

    LY_CHECK_RET(lyxml_ctx_next(xmlctx));

    /* create attributes */
    if (xmlctx->status == LYXML_ATTRIBUTE) {
        LY_CHECK_RET(lydxml_attrs(xmlctx, &attr));
    }

    /* remember the value */
    assert(xmlctx->status == LYXML_ELEM_CONTENT);
    value = xmlctx->value;
    value_len = xmlctx->value_len;
    ws_only = xmlctx->ws_only;
    dynamic = xmlctx->dynamic;
    if (dynamic) {
        xmlctx->dynamic = 0;
    }

    /* create the node without value */
    rc = lyd_create_opaq(xmlctx->ctx, name, name_len, prefix, prefix_len, ns->uri, strlen(ns->uri), NULL, 0, NULL,
            LY_VALUE_XML, NULL, 0, &node);
    LY_CHECK_GOTO(rc, cleanup);

    /* assign atributes */
    ((struct lyd_node_opaq *)node)->attr = attr;
    attr = NULL;

    /* parser next element */
    LY_CHECK_GOTO(rc = lyxml_ctx_next(xmlctx), cleanup);

    /* parse all the descendants */
    while (xmlctx->status == LYXML_ELEMENT) {
        rc = lydxml_opaq_r(xmlctx, node);
        LY_CHECK_GOTO(rc, cleanup);
    }

    /* insert */
    lyd_insert_node(parent, NULL, node, 1);

    /* update the value */
    opaq = (struct lyd_node_opaq *)node;
    if (opaq->child) {
        if (!ws_only) {
            LOGVAL(xmlctx->ctx, LYVE_SYNTAX_XML, "Mixed XML content node \"%s\" found, not supported.", LYD_NAME(node));
            rc = LY_EVALID;
            goto cleanup;
        }
    } else if (value_len) {
        lydict_remove(xmlctx->ctx, opaq->value);
        if (dynamic) {
            LY_CHECK_GOTO(rc = lydict_insert_zc(xmlctx->ctx, (char *)value, &opaq->value), cleanup);
            dynamic = 0;
        } else {
            LY_CHECK_GOTO(rc = lydict_insert(xmlctx->ctx, value, value_len, &opaq->value), cleanup);
        }
    }

cleanup:
    lyd_free_attr_siblings(xmlctx->ctx, attr);
    if (dynamic) {
        free((char *)value);
    }
    if (rc) {
        lyd_free_tree(node);
    }
    return rc;
}

/**
 * @brief Parse all expected non-data XML elements of the error-info element in NETCONF rpc-reply message.
 *
 * @param[in] xmlctx XML parser context.
 * @param[in] parent Parent to append nodes to.
 * @return LY_ERR value.
 */
static LY_ERR
lydxml_env_netconf_rpc_reply_error_info(struct lyxml_ctx *xmlctx, struct lyd_node *parent)
{
    LY_ERR r;
    struct lyd_node *child, *iter;
    ly_bool no_dup;

    /* there must be some child */
    if (xmlctx->status == LYXML_ELEM_CLOSE) {
        LOGVAL(xmlctx->ctx, LYVE_SYNTAX, "Missing child elements of \"error-info\".");
        return LY_EVALID;
    }

    while (xmlctx->status == LYXML_ELEMENT) {
        child = NULL;

        /*
         * session-id
         */
        r = lydxml_envelope(xmlctx, "session-id", "urn:ietf:params:xml:ns:netconf:base:1.0", 1, &child);
        if (r == LY_SUCCESS) {
            no_dup = 1;
            goto check_child;
        } else if (r != LY_ENOT) {
            goto error;
        }

        /*
         * bad-attribute
         */
        r = lydxml_envelope(xmlctx, "bad-attribute", "urn:ietf:params:xml:ns:netconf:base:1.0", 1, &child);
        if (r == LY_SUCCESS) {
            no_dup = 1;
            goto check_child;
        } else if (r != LY_ENOT) {
            goto error;
        }

        /*
         * bad-element
         */
        r = lydxml_envelope(xmlctx, "bad-element", "urn:ietf:params:xml:ns:netconf:base:1.0", 1, &child);
        if (r == LY_SUCCESS) {
            no_dup = 1;
            goto check_child;
        } else if (r != LY_ENOT) {
            goto error;
        }

        /*
         * bad-namespace
         */
        r = lydxml_envelope(xmlctx, "bad-namespace", "urn:ietf:params:xml:ns:netconf:base:1.0", 1, &child);
        if (r == LY_SUCCESS) {
            no_dup = 1;
            goto check_child;
        } else if (r != LY_ENOT) {
            goto error;
        }

        if (r == LY_ENOT) {
            assert(xmlctx->status == LYXML_ELEMENT);

            /* custom elements, parse all the siblings */
            while (xmlctx->status == LYXML_ELEMENT) {
                LY_CHECK_GOTO(r = lydxml_opaq_r(xmlctx, parent), error);
                LY_CHECK_GOTO(r = lyxml_ctx_next(xmlctx), error);
            }
            continue;
        }

check_child:
        /* check for duplicates */
        if (no_dup) {
            LY_LIST_FOR(lyd_child(parent), iter) {
                if ((((struct lyd_node_opaq *)iter)->name.name == ((struct lyd_node_opaq *)child)->name.name) &&
                        (((struct lyd_node_opaq *)iter)->name.module_ns == ((struct lyd_node_opaq *)child)->name.module_ns)) {
                    LOGVAL(xmlctx->ctx, LYVE_REFERENCE, "Duplicate element \"%s\" in \"error-info\".",
                            ((struct lyd_node_opaq *)child)->name.name);
                    r = LY_EVALID;
                    goto error;
                }
            }
        }

        /* finish child parsing */
        if (xmlctx->status != LYXML_ELEM_CLOSE) {
            assert(xmlctx->status == LYXML_ELEMENT);
            LOGVAL(xmlctx->ctx, LYVE_SYNTAX, "Unexpected child element \"%.*s\" of \"error-info\".",
                    (int)xmlctx->name_len, xmlctx->name);
            r = LY_EVALID;
            goto error;
        }
        LY_CHECK_GOTO(r = lyxml_ctx_next(xmlctx), error);

        /* insert */
        lyd_insert_node(parent, NULL, child, 1);
    }

    return LY_SUCCESS;

error:
    lyd_free_tree(child);
    return r;
}

/**
 * @brief Parse all expected non-data XML elements of the rpc-error element in NETCONF rpc-reply message.
 *
 * @param[in] xmlctx XML parser context.
 * @param[in] parent Parent to append nodes to.
 * @return LY_ERR value.
 */
static LY_ERR
lydxml_env_netconf_rpc_reply_error(struct lyxml_ctx *xmlctx, struct lyd_node *parent)
{
    LY_ERR r;
    struct lyd_node *child, *iter;
    const char *val;
    ly_bool no_dup;

    /* there must be some child */
    if (xmlctx->status == LYXML_ELEM_CLOSE) {
        LOGVAL(xmlctx->ctx, LYVE_SYNTAX, "Missing child elements of \"rpc-error\".");
        return LY_EVALID;
    }

    while (xmlctx->status == LYXML_ELEMENT) {
        child = NULL;

        /*
         * error-type
         */
        r = lydxml_envelope(xmlctx, "error-type", "urn:ietf:params:xml:ns:netconf:base:1.0", 1, &child);
        if (r == LY_SUCCESS) {
            val = ((struct lyd_node_opaq *)child)->value;
            if (strcmp(val, "transport") && strcmp(val, "rpc") && strcmp(val, "protocol") && strcmp(val, "application")) {
                LOGVAL(xmlctx->ctx, LYVE_REFERENCE, "Invalid value \"%s\" of element \"%s\".", val,
                        ((struct lyd_node_opaq *)child)->name.name);
                r = LY_EVALID;
                goto error;
            }

            no_dup = 1;
            goto check_child;
        } else if (r != LY_ENOT) {
            goto error;
        }

        /*
         * error-tag
         */
        r = lydxml_envelope(xmlctx, "error-tag", "urn:ietf:params:xml:ns:netconf:base:1.0", 1, &child);
        if (r == LY_SUCCESS) {
            val = ((struct lyd_node_opaq *)child)->value;
            if (strcmp(val, "in-use") && strcmp(val, "invalid-value") && strcmp(val, "too-big") &&
                    strcmp(val, "missing-attribute") && strcmp(val, "bad-attribute") &&
                    strcmp(val, "unknown-attribute") && strcmp(val, "missing-element") && strcmp(val, "bad-element") &&
                    strcmp(val, "unknown-element") && strcmp(val, "unknown-namespace") && strcmp(val, "access-denied") &&
                    strcmp(val, "lock-denied") && strcmp(val, "resource-denied") && strcmp(val, "rollback-failed") &&
                    strcmp(val, "data-exists") && strcmp(val, "data-missing") && strcmp(val, "operation-not-supported") &&
                    strcmp(val, "operation-failed") && strcmp(val, "malformed-message")) {
                LOGVAL(xmlctx->ctx, LYVE_REFERENCE, "Invalid value \"%s\" of element \"%s\".", val,
                        ((struct lyd_node_opaq *)child)->name.name);
                r = LY_EVALID;
                goto error;
            }

            no_dup = 1;
            goto check_child;
        } else if (r != LY_ENOT) {
            goto error;
        }

        /*
         * error-severity
         */
        r = lydxml_envelope(xmlctx, "error-severity", "urn:ietf:params:xml:ns:netconf:base:1.0", 1, &child);
        if (r == LY_SUCCESS) {
            val = ((struct lyd_node_opaq *)child)->value;
            if (strcmp(val, "error") && strcmp(val, "warning")) {
                LOGVAL(xmlctx->ctx, LYVE_REFERENCE, "Invalid value \"%s\" of element \"%s\".", val,
                        ((struct lyd_node_opaq *)child)->name.name);
                r = LY_EVALID;
                goto error;
            }

            no_dup = 1;
            goto check_child;
        } else if (r != LY_ENOT) {
            goto error;
        }

        /*
         * error-app-tag
         */
        r = lydxml_envelope(xmlctx, "error-app-tag", "urn:ietf:params:xml:ns:netconf:base:1.0", 1, &child);
        if (r == LY_SUCCESS) {
            no_dup = 1;
            goto check_child;
        } else if (r != LY_ENOT) {
            goto error;
        }

        /*
         * error-path
         */
        r = lydxml_envelope(xmlctx, "error-path", "urn:ietf:params:xml:ns:netconf:base:1.0", 1, &child);
        if (r == LY_SUCCESS) {
            no_dup = 1;
            goto check_child;
        } else if (r != LY_ENOT) {
            goto error;
        }

        /*
         * error-message
         */
        r = lydxml_envelope(xmlctx, "error-message", "urn:ietf:params:xml:ns:netconf:base:1.0", 1, &child);
        if (r == LY_SUCCESS) {
            no_dup = 1;
            goto check_child;
        } else if (r != LY_ENOT) {
            goto error;
        }

        /* error-info */
        r = lydxml_envelope(xmlctx, "error-info", "urn:ietf:params:xml:ns:netconf:base:1.0", 0, &child);
        if (r == LY_SUCCESS) {
            /* parse all the descendants */
            LY_CHECK_GOTO(r = lydxml_env_netconf_rpc_reply_error_info(xmlctx, child), error);

            no_dup = 0;
            goto check_child;
        } else if (r != LY_ENOT) {
            goto error;
        }

        if (r == LY_ENOT) {
            assert(xmlctx->status == LYXML_ELEMENT);
            LOGVAL(xmlctx->ctx, LYVE_SYNTAX, "Unexpected child element \"%.*s\" of \"rpc-error\".",
                    (int)xmlctx->name_len, xmlctx->name);
            r = LY_EVALID;
            goto error;
        }

check_child:
        /* check for duplicates */
        if (no_dup) {
            LY_LIST_FOR(lyd_child(parent), iter) {
                if ((((struct lyd_node_opaq *)iter)->name.name == ((struct lyd_node_opaq *)child)->name.name) &&
                        (((struct lyd_node_opaq *)iter)->name.module_ns == ((struct lyd_node_opaq *)child)->name.module_ns)) {
                    LOGVAL(xmlctx->ctx, LYVE_REFERENCE, "Duplicate element \"%s\" in \"rpc-error\".",
                            ((struct lyd_node_opaq *)child)->name.name);
                    r = LY_EVALID;
                    goto error;
                }
            }
        }

        /* finish child parsing */
        if (xmlctx->status != LYXML_ELEM_CLOSE) {
            assert(xmlctx->status == LYXML_ELEMENT);
            LOGVAL(xmlctx->ctx, LYVE_SYNTAX, "Unexpected child element \"%.*s\" of \"rpc-error\".",
                    (int)xmlctx->name_len, xmlctx->name);
            r = LY_EVALID;
            goto error;
        }
        LY_CHECK_GOTO(r = lyxml_ctx_next(xmlctx), error);

        /* insert */
        lyd_insert_node(parent, NULL, child, 1);
    }

    return LY_SUCCESS;

error:
    lyd_free_tree(child);
    return r;
}

/**
 * @brief Parse all expected non-data XML elements of a NETCONF rpc-reply message.
 *
 * @param[in] xmlctx XML parser context.
 * @param[out] evnp Parsed envelope(s) (opaque node).
 * @param[out] int_opts Internal options for parsing the rest of YANG data.
 * @param[out] close_elem Number of parsed opened elements that need to be closed.
 * @return LY_SUCCESS on success.
 * @return LY_ERR value on error.
 */
static LY_ERR
lydxml_env_netconf_reply(struct lyxml_ctx *xmlctx, struct lyd_node **envp, uint32_t *int_opts, uint32_t *close_elem)
{
    LY_ERR rc = LY_SUCCESS, r;
    struct lyd_node *child = NULL;
    const char *parsed_elem = NULL;

    assert(envp && !*envp);

    /* parse "rpc-reply" */
    r = lydxml_envelope(xmlctx, "rpc-reply", "urn:ietf:params:xml:ns:netconf:base:1.0", 0, envp);
    LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

    /* there must be some child */
    if (xmlctx->status == LYXML_ELEM_CLOSE) {
        LOGVAL(xmlctx->ctx, LYVE_SYNTAX, "Missing child elements of \"rpc-reply\".");
        rc = LY_EVALID;
        goto cleanup;
    }

    /* try to parse "ok" */
    r = lydxml_envelope(xmlctx, "ok", "urn:ietf:params:xml:ns:netconf:base:1.0", 0, &child);
    if (r == LY_SUCCESS) {
        /* insert */
        lyd_insert_node(*envp, NULL, child, 1);

        /* finish child parsing */
        if (xmlctx->status != LYXML_ELEM_CLOSE) {
            assert(xmlctx->status == LYXML_ELEMENT);
            LOGVAL(xmlctx->ctx, LYVE_SYNTAX, "Unexpected child element \"%.*s\" of \"ok\".",
                    (int)xmlctx->name_len, xmlctx->name);
            rc = LY_EVALID;
            goto cleanup;
        }
        LY_CHECK_GOTO(rc = lyxml_ctx_next(xmlctx), cleanup);

        /* success */
        parsed_elem = "ok";
        goto finish;
    } else if (r != LY_ENOT) {
        rc = r;
        goto cleanup;
    }

    /* try to parse all "rpc-error" elements */
    while (xmlctx->status == LYXML_ELEMENT) {
        r = lydxml_envelope(xmlctx, "rpc-error", "urn:ietf:params:xml:ns:netconf:base:1.0", 0, &child);
        if (r == LY_ENOT) {
            break;
        } else if (r) {
            rc = r;
            goto cleanup;
        }

        /* insert */
        lyd_insert_node(*envp, NULL, child, 1);

        /* parse all children of "rpc-error" */
        LY_CHECK_GOTO(rc = lydxml_env_netconf_rpc_reply_error(xmlctx, child), cleanup);

        /* finish child parsing */
        assert(xmlctx->status == LYXML_ELEM_CLOSE);
        LY_CHECK_GOTO(rc = lyxml_ctx_next(xmlctx), cleanup);

        parsed_elem = "rpc-error";
    }

finish:
    if (parsed_elem) {
        /* NETCONF rpc-reply with no data */
        if (xmlctx->status != LYXML_ELEM_CLOSE) {
            assert(xmlctx->status == LYXML_ELEMENT);
            LOGVAL(xmlctx->ctx, LYVE_SYNTAX, "Unexpected sibling element \"%.*s\" of \"%s\".",
                    (int)xmlctx->name_len, xmlctx->name, parsed_elem);
            rc = LY_EVALID;
            goto cleanup;
        }
    }

    /* NETCONF rpc-reply */
    *int_opts = LYD_INTOPT_WITH_SIBLINGS | LYD_INTOPT_REPLY;
    *close_elem = 1;

cleanup:
    if (rc) {
        lyd_free_tree(*envp);
        *envp = NULL;
    }
    return rc;
}

LY_ERR
lyd_parse_xml_netconf(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
        struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, enum lyd_type data_type,
        struct lyd_node **envp, struct ly_set *parsed, struct lyd_ctx **lydctx_p)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_xml_ctx *lydctx;
    struct lyd_node *node;
    uint32_t i, int_opts = 0, close_elem = 0;
    ly_bool parsed_data_nodes = 0;

    assert(ctx && in && lydctx_p);
    assert(!(parse_opts & ~LYD_PARSE_OPTS_MASK));
    assert(!(val_opts & ~LYD_VALIDATE_OPTS_MASK));
    assert(!(parse_opts & LYD_PARSE_SUBTREE));

    /* init context */
    lydctx = calloc(1, sizeof *lydctx);
    LY_CHECK_ERR_RET(!lydctx, LOGMEM(ctx), LY_EMEM);
    LY_CHECK_GOTO(rc = lyxml_ctx_new(ctx, in, &lydctx->xmlctx), cleanup);
    lydctx->parse_opts = parse_opts;
    lydctx->val_opts = val_opts;
    lydctx->free = lyd_xml_ctx_free;
    lydctx->ext = ext;

    switch (data_type) {
    case LYD_TYPE_RPC_NETCONF:
        assert(!parent);
        rc = lydxml_env_netconf_rpc(lydctx->xmlctx, envp, &int_opts, &close_elem);
        if (rc == LY_ENOT) {
            LOGVAL(ctx, LYVE_DATA, "Missing NETCONF <rpc> envelope or in incorrect namespace.");
        }
        LY_CHECK_GOTO(rc, cleanup);
        break;
    case LYD_TYPE_NOTIF_NETCONF:
        assert(!parent);

        /* parse "notification" */
        rc = lydxml_envelope(lydctx->xmlctx, "notification", "urn:ietf:params:xml:ns:netconf:notification:1.0", 0, envp);
        if (rc == LY_ENOT) {
            LOGVAL(ctx, LYVE_DATA, "Missing NETCONF <notification> envelope or in incorrect namespace.");
        }
        LY_CHECK_GOTO(rc, cleanup);

        /* NETCONF notification */
        int_opts = LYD_INTOPT_WITH_SIBLINGS | LYD_INTOPT_NOTIF | LYD_INTOPT_EVENTTIME;
        close_elem = 1;
        break;
    case LYD_TYPE_REPLY_NETCONF:
        assert(parent);
        rc = lydxml_env_netconf_reply(lydctx->xmlctx, envp, &int_opts, &close_elem);
        if (rc == LY_ENOT) {
            LOGVAL(ctx, LYVE_DATA, "Missing NETCONF <rpc-reply> envelope or in incorrect namespace.");
        }
        LY_CHECK_GOTO(rc, cleanup);
        break;
    case LYD_TYPE_RPC_RESTCONF:
        assert(parent);

        /* parse "input" */
        rc = lydxml_envelope(lydctx->xmlctx, "input", lyd_node_module(parent)->ns, 0, envp);
        if (rc == LY_ENOT) {
            LOGVAL(ctx, LYVE_DATA, "Missing RESTCONF \"input\" object or in incorrect namespace.");
        }
        LY_CHECK_GOTO(rc, cleanup);

        int_opts = LYD_INTOPT_WITH_SIBLINGS | LYD_INTOPT_RPC | LYD_INTOPT_ACTION;
        close_elem = 1;
        break;
    case LYD_TYPE_REPLY_RESTCONF:
        assert(parent);

        /* parse "output" */
        rc = lydxml_envelope(lydctx->xmlctx, "output", lyd_node_module(parent)->ns, 0, envp);
        if (rc == LY_ENOT) {
            LOGVAL(ctx, LYVE_DATA, "Missing RESTCONF \"output\" object or in incorrect namespace.");
        }
        LY_CHECK_GOTO(rc, cleanup);

        int_opts = LYD_INTOPT_WITH_SIBLINGS | LYD_INTOPT_REPLY;
        close_elem = 1;
        break;
    default:
        LOGINT(ctx);
        rc = LY_EINT;
        goto cleanup;
    }

    lydctx->int_opts = int_opts;

    /* find the operation node if it exists already */
    LY_CHECK_GOTO(rc = lyd_parser_find_operation(parent, int_opts, &lydctx->op_node), cleanup);

    /* parse XML data */
    while (lydctx->xmlctx->status == LYXML_ELEMENT) {
        LY_CHECK_GOTO(rc = lydxml_subtree_r(lydctx, parent, first_p, parsed), cleanup);
        parsed_data_nodes = 1;

        if (!(int_opts & LYD_INTOPT_WITH_SIBLINGS)) {
            break;
        }
    }

    /* close all opened elements */
    for (i = 0; i < close_elem; ++i) {
        if (lydctx->xmlctx->status != LYXML_ELEM_CLOSE) {
            assert(lydctx->xmlctx->status == LYXML_ELEMENT);
            LOGVAL(lydctx->xmlctx->ctx, LYVE_SYNTAX, "Unexpected child element \"%.*s\".",
                    (int)lydctx->xmlctx->name_len, lydctx->xmlctx->name);
            rc = LY_EVALID;
            goto cleanup;
        }

        LY_CHECK_GOTO(rc = lyxml_ctx_next(lydctx->xmlctx), cleanup);
    }

    /* check final state */
    if ((int_opts & LYD_INTOPT_NO_SIBLINGS) && (lydctx->xmlctx->status == LYXML_ELEMENT)) {
        LOGVAL(ctx, LYVE_SYNTAX, "Unexpected sibling node.");
        rc = LY_EVALID;
        goto cleanup;
    }
    if ((int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_ACTION | LYD_INTOPT_NOTIF | LYD_INTOPT_REPLY)) && !lydctx->op_node) {
        LOGVAL(ctx, LYVE_DATA, "Missing the operation node.");
        rc = LY_EVALID;
        goto cleanup;
    }
    if (int_opts & LYD_INTOPT_EVENTTIME) {
        /* parse as a child of the envelope */
        node = (*first_p)->prev;
        if (node->schema) {
            LOGVAL(ctx, LYVE_DATA, "Missing notification \"eventTime\" node.");
            rc = LY_EVALID;
            goto cleanup;
        } else {
            /* can be the only opaque node and an operation had to be parsed */
            assert(!strcmp(LYD_NAME(node), "eventTime") && (*first_p)->next);
            lyd_unlink(node);
            assert(*envp);
            lyd_insert_child(*envp, node);
        }
    }

    if (!parsed_data_nodes) {
        /* no data nodes were parsed */
        lydctx->op_node = NULL;
    }

cleanup:
    /* there should be no unres stored if validation should be skipped */
    assert(!(parse_opts & LYD_PARSE_ONLY) || (!lydctx->node_types.count && !lydctx->meta_types.count &&
            !lydctx->node_when.count));

    if (rc) {
        lyd_xml_ctx_free((struct lyd_ctx *)lydctx);
    } else {
        *lydctx_p = (struct lyd_ctx *)lydctx;

        /* the XML context is no more needed, freeing it also stops logging line numbers which would be confusing now */
        lyxml_ctx_free(lydctx->xmlctx);
        lydctx->xmlctx = NULL;
    }
    return rc;
}
