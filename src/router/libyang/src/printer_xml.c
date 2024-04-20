/**
 * @file printer_xml.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief XML printer for libyang data structure
 *
 * Copyright (c) 2015 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "context.h"
#include "dict.h"
#include "log.h"
#include "out.h"
#include "out_internal.h"
#include "parser_data.h"
#include "plugins_exts/metadata.h"
#include "plugins_types.h"
#include "printer_data.h"
#include "printer_internal.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_schema.h"
#include "xml.h"

/**
 * @brief XML printer context.
 */
struct xmlpr_ctx {
    struct ly_out *out;       /**< output specification */
    uint16_t level;           /**< current indentation level: 0 - no formatting, >= 1 indentation levels */
    uint32_t options;         /**< [Data printer flags](@ref dataprinterflags) */
    const struct ly_ctx *ctx; /**< libyang context */
    struct ly_set prefix;     /**< printed namespace prefixes */
    struct ly_set ns;         /**< printed namespaces */
};

#define LYXML_PREFIX_REQUIRED 0x01  /**< The prefix is not just a suggestion but a requirement. */
#define LYXML_PREFIX_DEFAULT  0x02  /**< The namespace is required to be a default (without prefix) */

/**
 * @brief Print a namespace if not already printed.
 *
 * @param[in] ctx XML printer context.
 * @param[in] ns Namespace to print, expected to be in dictionary.
 * @param[in] new_prefix Suggested new prefix, NULL for a default namespace without prefix. Stored in the dictionary.
 * @param[in] prefix_opts Prefix options changing the meaning of parameters.
 * @return Printed prefix of the namespace to use.
 */
static const char *
xml_print_ns(struct xmlpr_ctx *pctx, const char *ns, const char *new_prefix, uint32_t prefix_opts)
{
    uint32_t i;

    for (i = pctx->ns.count; i > 0; --i) {
        if (!new_prefix) {
            /* find default namespace */
            if (!pctx->prefix.objs[i - 1]) {
                if (!strcmp(pctx->ns.objs[i - 1], ns)) {
                    /* matching default namespace */
                    return pctx->prefix.objs[i - 1];
                }
                /* not matching default namespace */
                break;
            }
        } else {
            /* find prefixed namespace */
            if (!strcmp(pctx->ns.objs[i - 1], ns)) {
                if (!pctx->prefix.objs[i - 1]) {
                    /* default namespace is not interesting */
                    continue;
                }

                if (!strcmp(pctx->prefix.objs[i - 1], new_prefix) || !(prefix_opts & LYXML_PREFIX_REQUIRED)) {
                    /* the same prefix or can be any */
                    return pctx->prefix.objs[i - 1];
                }
            }
        }
    }

    /* suitable namespace not found, must be printed */
    ly_print_(pctx->out, " xmlns%s%s=\"%s\"", new_prefix ? ":" : "", new_prefix ? new_prefix : "", ns);

    /* and added into namespaces */
    if (new_prefix) {
        LY_CHECK_RET(lydict_insert(pctx->ctx, new_prefix, 0, &new_prefix), NULL);
    }
    LY_CHECK_RET(ly_set_add(&pctx->prefix, (void *)new_prefix, 1, NULL), NULL);
    LY_CHECK_RET(ly_set_add(&pctx->ns, (void *)ns, 1, &i), NULL);

    /* return it */
    return pctx->prefix.objs[i];
}

static const char *
xml_print_ns_opaq(struct xmlpr_ctx *pctx, LY_VALUE_FORMAT format, const struct ly_opaq_name *name, uint32_t prefix_opts)
{
    switch (format) {
    case LY_VALUE_XML:
        if (name->module_ns) {
            return xml_print_ns(pctx, name->module_ns, (prefix_opts & LYXML_PREFIX_DEFAULT) ? NULL : name->prefix, prefix_opts);
        }
        break;
    case LY_VALUE_JSON:
        if (name->module_name) {
            const struct lys_module *mod = ly_ctx_get_module_latest(pctx->ctx, name->module_name);

            if (mod) {
                return xml_print_ns(pctx, mod->ns, (prefix_opts & LYXML_PREFIX_DEFAULT) ? NULL : name->prefix, prefix_opts);
            }
        }
        break;
    default:
        /* cannot be created */
        LOGINT(pctx->ctx);
    }

    return NULL;
}

/**
 * @brief Print prefix data.
 *
 * @param[in] ctx XML printer context.
 * @param[in] format Value prefix format, only ::LY_VALUE_XML supported.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[in] prefix_opts Prefix options changing the meaning of parameters.
 * @return LY_ERR value.
 */
static void
xml_print_ns_prefix_data(struct xmlpr_ctx *pctx, LY_VALUE_FORMAT format, void *prefix_data, uint32_t prefix_opts)
{
    const struct ly_set *set;
    const struct lyxml_ns *ns;
    uint32_t i;

    switch (format) {
    case LY_VALUE_XML:
        set = prefix_data;
        for (i = 0; i < set->count; ++i) {
            ns = set->objs[i];
            if (!ns->prefix) {
                /* default namespace is not for the element */
                continue;
            }

            xml_print_ns(pctx, ns->uri, (prefix_opts & LYXML_PREFIX_DEFAULT) ? NULL : ns->prefix, prefix_opts);
        }
        break;
    default:
        /* cannot be created */
        LOGINT(pctx->ctx);
    }
}

/**
 * @brief Print metadata of a node.
 *
 * @param[in] pctx XML printer context.
 * @param[in] node Data node with metadata.
 */
static void
xml_print_meta(struct xmlpr_ctx *pctx, const struct lyd_node *node)
{
    struct lyd_meta *meta;
    const struct lys_module *mod;
    struct ly_set ns_list = {0};
    LY_ARRAY_COUNT_TYPE u;
    ly_bool dynamic, filter_attrs = 0;
    const char *value;
    uint32_t i;

    /* with-defaults */
    if (node->schema->nodetype & LYD_NODE_TERM) {
        if (((node->flags & LYD_DEFAULT) && (pctx->options & (LYD_PRINT_WD_ALL_TAG | LYD_PRINT_WD_IMPL_TAG))) ||
                ((pctx->options & LYD_PRINT_WD_ALL_TAG) && lyd_is_default(node))) {
            /* we have implicit OR explicit default node, print attribute only if context include with-defaults schema */
            mod = ly_ctx_get_module_latest(LYD_CTX(node), "ietf-netconf-with-defaults");
            if (mod) {
                ly_print_(pctx->out, " %s:default=\"true\"", xml_print_ns(pctx, mod->ns, mod->prefix, 0));
            }
        }
    }

    /* check for NETCONF filter unqualified attributes */
    if (!strcmp(node->schema->module->name, "notifications")) {
        filter_attrs = 1;
    } else {
        LY_ARRAY_FOR(node->schema->exts, u) {
            if (!strcmp(node->schema->exts[u].def->name, "get-filter-element-attributes") &&
                    !strcmp(node->schema->exts[u].def->module->name, "ietf-netconf")) {
                filter_attrs = 1;
                break;
            }
        }
    }

    for (meta = node->meta; meta; meta = meta->next) {
        /* store the module of the default namespace, NULL because there is none */
        ly_set_add(&ns_list, NULL, 0, NULL);

        /* print the value */
        value = meta->value.realtype->plugin->print(LYD_CTX(node), &meta->value, LY_VALUE_XML, &ns_list, &dynamic, NULL);

        /* print namespaces connected with the value's prefixes */
        for (i = 1; i < ns_list.count; ++i) {
            mod = ns_list.objs[i];
            xml_print_ns(pctx, mod->ns, mod->prefix, 1);
        }
        ly_set_erase(&ns_list, NULL);

        mod = meta->annotation->module;
        if (filter_attrs && !strcmp(mod->name, "ietf-netconf") && (!strcmp(meta->name, "type") ||
                !strcmp(meta->name, "select"))) {
            /* print special NETCONF filter unqualified attributes */
            ly_print_(pctx->out, " %s=\"", meta->name);
        } else {
            /* print the metadata with its namespace */
            ly_print_(pctx->out, " %s:%s=\"", xml_print_ns(pctx, mod->ns, mod->prefix, 1), meta->name);
        }

        /* print metadata value */
        if (value && value[0]) {
            lyxml_dump_text(pctx->out, value, 1);
        }
        ly_print_(pctx->out, "\"");
        if (dynamic) {
            free((void *)value);
        }
    }
}

/**
 * @brief Print generic XML element despite of the data node type.
 *
 * Prints the element name, attributes and necessary namespaces.
 *
 * @param[in] ctx XML printer context.
 * @param[in] node Data node to be printed.
 */
static void
xml_print_node_open(struct xmlpr_ctx *pctx, const struct lyd_node *node)
{
    /* print node name */
    ly_print_(pctx->out, "%*s<%s", INDENT, node->schema->name);

    /* print default namespace */
    xml_print_ns(pctx, node->schema->module->ns, NULL, 0);

    /* print metadata */
    xml_print_meta(pctx, node);
}

static LY_ERR
xml_print_attr(struct xmlpr_ctx *pctx, const struct lyd_node_opaq *node)
{
    const struct lyd_attr *attr;
    const char *pref;

    LY_LIST_FOR(node->attr, attr) {
        pref = NULL;
        if (attr->name.prefix) {
            /* print attribute namespace */
            pref = xml_print_ns_opaq(pctx, attr->format, &attr->name, 0);
        }

        /* print namespaces connected with the value's prefixes */
        if (attr->val_prefix_data) {
            xml_print_ns_prefix_data(pctx, attr->format, attr->val_prefix_data, LYXML_PREFIX_REQUIRED);
        }

        /* print the attribute with its prefix and value */
        ly_print_(pctx->out, " %s%s%s=\"", pref ? pref : "", pref ? ":" : "", attr->name.name);
        lyxml_dump_text(pctx->out, attr->value, 1);
        ly_print_(pctx->out, "\""); /* print attribute value terminator */

    }

    return LY_SUCCESS;
}

static LY_ERR
xml_print_opaq_open(struct xmlpr_ctx *pctx, const struct lyd_node_opaq *node)
{
    /* print node name */
    ly_print_(pctx->out, "%*s<%s", INDENT, node->name.name);

    if (node->name.prefix || node->name.module_ns) {
        /* print default namespace */
        xml_print_ns_opaq(pctx, node->format, &node->name, LYXML_PREFIX_DEFAULT);
    }

    /* print attributes */
    LY_CHECK_RET(xml_print_attr(pctx, node));

    return LY_SUCCESS;
}

static LY_ERR xml_print_node(struct xmlpr_ctx *pctx, const struct lyd_node *node);

/**
 * @brief Print XML element representing lyd_node_term.
 *
 * @param[in] ctx XML printer context.
 * @param[in] node Data node to be printed.
 * @return LY_ERR value.
 */
static LY_ERR
xml_print_term(struct xmlpr_ctx *pctx, const struct lyd_node_term *node)
{
    LY_ERR rc = LY_SUCCESS;
    struct ly_set ns_list = {0};
    ly_bool dynamic = 0;
    const char *value = NULL;
    const struct lys_module *mod;
    uint32_t i;

    /* store the module of the default namespace */
    if ((rc = ly_set_add(&ns_list, node->schema->module, 0, NULL))) {
        LOGMEM(pctx->ctx);
        goto cleanup;
    }

    /* print the value */
    value = ((struct lysc_node_leaf *)node->schema)->type->plugin->print(LYD_CTX(node), &node->value, LY_VALUE_XML,
            &ns_list, &dynamic, NULL);
    LY_CHECK_ERR_GOTO(!value, rc = LY_EINVAL, cleanup);

    /* print node opening */
    xml_print_node_open(pctx, &node->node);

    /* print namespaces connected with the values's prefixes */
    for (i = 1; i < ns_list.count; ++i) {
        mod = ns_list.objs[i];
        ly_print_(pctx->out, " xmlns:%s=\"%s\"", mod->prefix, mod->ns);
    }

    if (!value[0]) {
        ly_print_(pctx->out, "/>%s", DO_FORMAT ? "\n" : "");
    } else {
        ly_print_(pctx->out, ">");
        lyxml_dump_text(pctx->out, value, 0);
        ly_print_(pctx->out, "</%s>%s", node->schema->name, DO_FORMAT ? "\n" : "");
    }

cleanup:
    ly_set_erase(&ns_list, NULL);
    if (dynamic) {
        free((void *)value);
    }
    return rc;
}

/**
 * @brief Print XML element representing lyd_node_inner.
 *
 * @param[in] ctx XML printer context.
 * @param[in] node Data node to be printed.
 * @return LY_ERR value.
 */
static LY_ERR
xml_print_inner(struct xmlpr_ctx *pctx, const struct lyd_node_inner *node)
{
    LY_ERR ret;
    struct lyd_node *child;

    xml_print_node_open(pctx, &node->node);

    LY_LIST_FOR(node->child, child) {
        if (lyd_node_should_print(child, pctx->options)) {
            break;
        }
    }
    if (!child) {
        /* there are no children that will be printed */
        ly_print_(pctx->out, "/>%s", DO_FORMAT ? "\n" : "");
        return LY_SUCCESS;
    }

    /* children */
    ly_print_(pctx->out, ">%s", DO_FORMAT ? "\n" : "");

    LEVEL_INC;
    LY_LIST_FOR(node->child, child) {
        ret = xml_print_node(pctx, child);
        LY_CHECK_ERR_RET(ret, LEVEL_DEC, ret);
    }
    LEVEL_DEC;

    ly_print_(pctx->out, "%*s</%s>%s", INDENT, node->schema->name, DO_FORMAT ? "\n" : "");

    return LY_SUCCESS;
}

static LY_ERR
xml_print_anydata(struct xmlpr_ctx *pctx, const struct lyd_node_any *node)
{
    struct lyd_node_any *any = (struct lyd_node_any *)node;
    struct lyd_node *iter;
    uint32_t prev_opts, temp_lo = 0;
    LY_ERR ret;

    if ((node->schema->nodetype == LYS_ANYDATA) && (node->value_type != LYD_ANYDATA_DATATREE)) {
        LOGINT_RET(pctx->ctx);
    }

    xml_print_node_open(pctx, &node->node);

    if (!any->value.tree) {
        /* no content */
no_content:
        ly_print_(pctx->out, "/>%s", DO_FORMAT ? "\n" : "");
        return LY_SUCCESS;
    } else {
        if (any->value_type == LYD_ANYDATA_LYB) {
            /* turn logging off */
            ly_temp_log_options(&temp_lo);

            /* try to parse it into a data tree */
            if (lyd_parse_data_mem((struct ly_ctx *)LYD_CTX(node), any->value.mem, LYD_LYB,
                    LYD_PARSE_ONLY | LYD_PARSE_OPAQ | LYD_PARSE_STRICT, 0, &iter) == LY_SUCCESS) {
                /* successfully parsed */
                free(any->value.mem);
                any->value.tree = iter;
                any->value_type = LYD_ANYDATA_DATATREE;
            }

            /* turn logging on again */
            ly_temp_log_options(NULL);
        }

        switch (any->value_type) {
        case LYD_ANYDATA_DATATREE:
            /* close opening tag and print data */
            prev_opts = pctx->options;
            pctx->options &= ~LYD_PRINT_WITHSIBLINGS;
            LEVEL_INC;

            ly_print_(pctx->out, ">%s", DO_FORMAT ? "\n" : "");
            LY_LIST_FOR(any->value.tree, iter) {
                ret = xml_print_node(pctx, iter);
                LY_CHECK_ERR_RET(ret, LEVEL_DEC, ret);
            }

            LEVEL_DEC;
            pctx->options = prev_opts;
            break;
        case LYD_ANYDATA_STRING:
            /* escape XML-sensitive characters */
            if (!any->value.str[0]) {
                goto no_content;
            }
            /* close opening tag and print data */
            ly_print_(pctx->out, ">");
            lyxml_dump_text(pctx->out, any->value.str, 0);
            break;
        case LYD_ANYDATA_XML:
            /* print without escaping special characters */
            if (!any->value.str[0]) {
                goto no_content;
            }
            ly_print_(pctx->out, ">%s", any->value.str);
            break;
        case LYD_ANYDATA_JSON:
        case LYD_ANYDATA_LYB:
            /* JSON and LYB format is not supported */
            LOGWRN(pctx->ctx, "Unable to print anydata content (type %d) as XML.", any->value_type);
            goto no_content;
        }

        /* closing tag */
        if (any->value_type == LYD_ANYDATA_DATATREE) {
            ly_print_(pctx->out, "%*s</%s>%s", INDENT, node->schema->name, DO_FORMAT ? "\n" : "");
        } else {
            ly_print_(pctx->out, "</%s>%s", node->schema->name, DO_FORMAT ? "\n" : "");
        }
    }

    return LY_SUCCESS;
}

static LY_ERR
xml_print_opaq(struct xmlpr_ctx *pctx, const struct lyd_node_opaq *node)
{
    LY_ERR ret;
    struct lyd_node *child;

    LY_CHECK_RET(xml_print_opaq_open(pctx, node));

    if (node->value[0]) {
        /* print namespaces connected with the value's prefixes */
        if (node->val_prefix_data) {
            xml_print_ns_prefix_data(pctx, node->format, node->val_prefix_data, LYXML_PREFIX_REQUIRED);
        }

        ly_print_(pctx->out, ">");
        lyxml_dump_text(pctx->out, node->value, 0);
    }

    if (node->child) {
        /* children */
        if (!node->value[0]) {
            ly_print_(pctx->out, ">%s", DO_FORMAT ? "\n" : "");
        }

        LEVEL_INC;
        LY_LIST_FOR(node->child, child) {
            ret = xml_print_node(pctx, child);
            LY_CHECK_ERR_RET(ret, LEVEL_DEC, ret);
        }
        LEVEL_DEC;

        ly_print_(pctx->out, "%*s</%s>%s", INDENT, node->name.name, DO_FORMAT ? "\n" : "");
    } else if (node->value[0]) {
        ly_print_(pctx->out, "</%s>%s", node->name.name, DO_FORMAT ? "\n" : "");
    } else {
        /* no value or children */
        ly_print_(pctx->out, "/>%s", DO_FORMAT ? "\n" : "");
    }

    return LY_SUCCESS;
}

/**
 * @brief Print XML element representing lyd_node.
 *
 * @param[in] ctx XML printer context.
 * @param[in] node Data node to be printed.
 * @return LY_ERR value.
 */
static LY_ERR
xml_print_node(struct xmlpr_ctx *pctx, const struct lyd_node *node)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t ns_count;

    if (!lyd_node_should_print(node, pctx->options)) {
        /* do not print at all */
        return LY_SUCCESS;
    }

    /* remember namespace definition count on this level */
    ns_count = pctx->ns.count;

    if (!node->schema) {
        ret = xml_print_opaq(pctx, (const struct lyd_node_opaq *)node);
    } else {
        switch (node->schema->nodetype) {
        case LYS_CONTAINER:
        case LYS_LIST:
        case LYS_NOTIF:
        case LYS_RPC:
        case LYS_ACTION:
            ret = xml_print_inner(pctx, (const struct lyd_node_inner *)node);
            break;
        case LYS_LEAF:
        case LYS_LEAFLIST:
            ret = xml_print_term(pctx, (const struct lyd_node_term *)node);
            break;
        case LYS_ANYXML:
        case LYS_ANYDATA:
            ret = xml_print_anydata(pctx, (const struct lyd_node_any *)node);
            break;
        default:
            LOGINT(pctx->ctx);
            ret = LY_EINT;
            break;
        }
    }

    /* remove all added namespaces */
    while (ns_count < pctx->ns.count) {
        lydict_remove(pctx->ctx, pctx->prefix.objs[pctx->prefix.count - 1]);
        ly_set_rm_index(&pctx->prefix, pctx->prefix.count - 1, NULL);
        ly_set_rm_index(&pctx->ns, pctx->ns.count - 1, NULL);
    }

    return ret;
}

LY_ERR
xml_print_data(struct ly_out *out, const struct lyd_node *root, uint32_t options)
{
    const struct lyd_node *node;
    struct xmlpr_ctx pctx = {0};

    if (!root) {
        if ((out->type == LY_OUT_MEMORY) || (out->type == LY_OUT_CALLBACK)) {
            ly_print_(out, "");
        }
        goto finish;
    }

    pctx.out = out;
    pctx.level = 0;
    pctx.options = options;
    pctx.ctx = LYD_CTX(root);

    /* content */
    LY_LIST_FOR(root, node) {
        LY_CHECK_RET(xml_print_node(&pctx, node));
        if (!(options & LYD_PRINT_WITHSIBLINGS)) {
            break;
        }
    }

finish:
    assert(!pctx.prefix.count && !pctx.ns.count);
    ly_set_erase(&pctx.prefix, NULL);
    ly_set_erase(&pctx.ns, NULL);
    ly_print_flush(out);
    return LY_SUCCESS;
}
