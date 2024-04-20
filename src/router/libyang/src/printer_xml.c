/**
 * @file printer_xml.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief XML printer for libyang data structure
 *
 * Copyright (c) 2015 - 2019 CESNET, z.s.p.o.
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
xml_print_ns(struct xmlpr_ctx *ctx, const char *ns, const char *new_prefix, uint32_t prefix_opts)
{
    uint32_t i;

    for (i = ctx->ns.count; i > 0; --i) {
        if (!new_prefix) {
            /* find default namespace */
            if (!ctx->prefix.objs[i - 1]) {
                if (!strcmp(ctx->ns.objs[i - 1], ns)) {
                    /* matching default namespace */
                    return ctx->prefix.objs[i - 1];
                }
                /* not matching default namespace */
                break;
            }
        } else {
            /* find prefixed namespace */
            if (!strcmp(ctx->ns.objs[i - 1], ns)) {
                if (!ctx->prefix.objs[i - 1]) {
                    /* default namespace is not interesting */
                    continue;
                }

                if (!strcmp(ctx->prefix.objs[i - 1], new_prefix) || !(prefix_opts & LYXML_PREFIX_REQUIRED)) {
                    /* the same prefix or can be any */
                    return ctx->prefix.objs[i - 1];
                }
            }
        }
    }

    /* suitable namespace not found, must be printed */
    ly_print_(ctx->out, " xmlns%s%s=\"%s\"", new_prefix ? ":" : "", new_prefix ? new_prefix : "", ns);

    /* and added into namespaces */
    if (new_prefix) {
        LY_CHECK_RET(lydict_insert(ctx->ctx, new_prefix, 0, &new_prefix), NULL);
    }
    LY_CHECK_RET(ly_set_add(&ctx->prefix, (void *)new_prefix, 1, NULL), NULL);
    LY_CHECK_RET(ly_set_add(&ctx->ns, (void *)ns, 1, &i), NULL);

    /* return it */
    return ctx->prefix.objs[i];
}

static const char *
xml_print_ns_opaq(struct xmlpr_ctx *ctx, LY_VALUE_FORMAT format, const struct ly_opaq_name *name, uint32_t prefix_opts)
{
    switch (format) {
    case LY_VALUE_XML:
        return xml_print_ns(ctx, name->module_ns, (prefix_opts & LYXML_PREFIX_DEFAULT) ? NULL : name->prefix, prefix_opts);
        break;
    case LY_VALUE_JSON:
        if (name->module_name) {
            const struct lys_module *mod = ly_ctx_get_module_latest(ctx->ctx, name->module_name);
            if (mod) {
                return xml_print_ns(ctx, mod->ns, (prefix_opts & LYXML_PREFIX_DEFAULT) ? NULL : name->prefix, prefix_opts);
            }
        }
        break;
    default:
        /* cannot be created */
        LOGINT(ctx->ctx);
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
xml_print_ns_prefix_data(struct xmlpr_ctx *ctx, LY_VALUE_FORMAT format, void *prefix_data, uint32_t prefix_opts)
{
    const struct ly_set *set;
    const struct lyxml_ns *ns;
    uint32_t i;

    switch (format) {
    case LY_VALUE_XML:
        set = prefix_data;
        for (i = 0; i < set->count; ++i) {
            ns = set->objs[i];
            xml_print_ns(ctx, ns->uri, (prefix_opts & LYXML_PREFIX_DEFAULT) ? NULL : ns->prefix, prefix_opts);
        }
        break;
    default:
        /* cannot be created */
        LOGINT(ctx->ctx);
    }
}

/**
 * TODO
 */
static void
xml_print_meta(struct xmlpr_ctx *ctx, const struct lyd_node *node)
{
    struct lyd_meta *meta;
    const struct lys_module *mod;
    struct ly_set ns_list = {0};

#if 0
    const char **prefs, **nss;
    const char *xml_expr = NULL, *mod_name;
    uint32_t ns_count, i;
    ly_bool rpc_filter = 0;
    char *p;
    size_t len;
#endif
    ly_bool dynamic;

    /* with-defaults */
    if (node->schema->nodetype & LYD_NODE_TERM) {
        if (((node->flags & LYD_DEFAULT) && (ctx->options & (LYD_PRINT_WD_ALL_TAG | LYD_PRINT_WD_IMPL_TAG))) ||
                ((ctx->options & LYD_PRINT_WD_ALL_TAG) && lyd_is_default(node))) {
            /* we have implicit OR explicit default node, print attribute only if context include with-defaults schema */
            mod = ly_ctx_get_module_latest(LYD_CTX(node), "ietf-netconf-with-defaults");
            if (mod) {
                ly_print_(ctx->out, " %s:default=\"true\"", xml_print_ns(ctx, mod->ns, mod->prefix, 0));
            }
        }
    }
#if 0
    /* technically, check for the extension get-filter-element-attributes from ietf-netconf */
    if (!strcmp(node->schema->name, "filter") &&
            (!strcmp(node->schema->module->name, "ietf-netconf") || !strcmp(node->schema->module->name, "notifications"))) {
        rpc_filter = 1;
    }
#endif
    for (meta = node->meta; meta; meta = meta->next) {
        const char *value = meta->value.realtype->plugin->print(LYD_CTX(node), &meta->value, LY_VALUE_XML, &ns_list,
                &dynamic, NULL);

        /* print namespaces connected with the value's prefixes */
        for (uint32_t u = 0; u < ns_list.count; ++u) {
            mod = (const struct lys_module *)ns_list.objs[u];
            xml_print_ns(ctx, mod->ns, mod->prefix, 1);
        }
        ly_set_erase(&ns_list, NULL);

#if 0
        if (rpc_filter) {
            /* exception for NETCONF's filter's attributes */
            if (!strcmp(meta->name, "select")) {
                /* xpath content, we have to convert the JSON format into XML first */
                xml_expr = transform_json2xml(node->schema->module, meta->value_str, 0, &prefs, &nss, &ns_count);
                if (!xml_expr) {
                    /* error */
                    return EXIT_FAILURE;
                }

                for (i = 0; i < ns_count; ++i) {
                    ly_print_(out, " xmlns:%s=\"%s\"", prefs[i], nss[i]);
                }
                free(prefs);
                free(nss);
            }
            ly_print_(out, " %s=\"", meta->name);
        } else {
#endif
        /* print the metadata with its namespace */
        mod = meta->annotation->module;
        ly_print_(ctx->out, " %s:%s=\"", xml_print_ns(ctx, mod->ns, mod->prefix, 1), meta->name);
#if 0
    }
#endif

        /* print metadata value */
        if (value && value[0]) {
            lyxml_dump_text(ctx->out, value, 1);
        }
        ly_print_(ctx->out, "\"");
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
xml_print_node_open(struct xmlpr_ctx *ctx, const struct lyd_node *node)
{
    /* print node name */
    ly_print_(ctx->out, "%*s<%s", INDENT, node->schema->name);

    /* print default namespace */
    xml_print_ns(ctx, node->schema->module->ns, NULL, 0);

    /* print metadata */
    xml_print_meta(ctx, node);
}

static LY_ERR
xml_print_attr(struct xmlpr_ctx *ctx, const struct lyd_node_opaq *node)
{
    const struct lyd_attr *attr;
    const char *pref;

    LY_LIST_FOR(node->attr, attr) {
        pref = NULL;
        if (attr->name.prefix) {
            /* print attribute namespace */
            pref = xml_print_ns_opaq(ctx, attr->format, &attr->name, 0);
        }

        /* print namespaces connected with the value's prefixes */
        if (attr->val_prefix_data) {
            xml_print_ns_prefix_data(ctx, attr->format, attr->val_prefix_data, LYXML_PREFIX_REQUIRED);
        }

        /* print the attribute with its prefix and value */
        ly_print_(ctx->out, " %s%s%s=\"", pref ? pref : "", pref ? ":" : "", attr->name.name);
        lyxml_dump_text(ctx->out, attr->value, 1);
        ly_print_(ctx->out, "\""); /* print attribute value terminator */

    }

    return LY_SUCCESS;
}

static LY_ERR
xml_print_opaq_open(struct xmlpr_ctx *ctx, const struct lyd_node_opaq *node)
{
    /* print node name */
    ly_print_(ctx->out, "%*s<%s", INDENT, node->name.name);

    /* print default namespace */
    xml_print_ns_opaq(ctx, node->format, &node->name, LYXML_PREFIX_DEFAULT);

    /* print attributes */
    LY_CHECK_RET(xml_print_attr(ctx, node));

    return LY_SUCCESS;
}

static LY_ERR xml_print_node(struct xmlpr_ctx *ctx, const struct lyd_node *node);

/**
 * @brief Print XML element representing lyd_node_term.
 *
 * @param[in] ctx XML printer context.
 * @param[in] node Data node to be printed.
 */
static void
xml_print_term(struct xmlpr_ctx *ctx, const struct lyd_node_term *node)
{
    struct ly_set ns_list = {0};
    ly_bool dynamic;
    const char *value;

    xml_print_node_open(ctx, &node->node);
    value = ((struct lysc_node_leaf *)node->schema)->type->plugin->print(LYD_CTX(node), &node->value, LY_VALUE_XML,
            &ns_list, &dynamic, NULL);

    /* print namespaces connected with the values's prefixes */
    for (uint32_t u = 0; u < ns_list.count; ++u) {
        const struct lys_module *mod = (const struct lys_module *)ns_list.objs[u];
        ly_print_(ctx->out, " xmlns:%s=\"%s\"", mod->prefix, mod->ns);
    }
    ly_set_erase(&ns_list, NULL);

    if (!value || !value[0]) {
        ly_print_(ctx->out, "/>%s", DO_FORMAT ? "\n" : "");
    } else {
        ly_print_(ctx->out, ">");
        lyxml_dump_text(ctx->out, value, 0);
        ly_print_(ctx->out, "</%s>%s", node->schema->name, DO_FORMAT ? "\n" : "");
    }
    if (dynamic) {
        free((void *)value);
    }
}

/**
 * @brief Print XML element representing lyd_node_inner.
 *
 * @param[in] ctx XML printer context.
 * @param[in] node Data node to be printed.
 * @return LY_ERR value.
 */
static LY_ERR
xml_print_inner(struct xmlpr_ctx *ctx, const struct lyd_node_inner *node)
{
    LY_ERR ret;
    struct lyd_node *child;

    xml_print_node_open(ctx, &node->node);

    LY_LIST_FOR(node->child, child) {
        if (ly_should_print(child, ctx->options)) {
            break;
        }
    }
    if (!child) {
        /* there are no children that will be printed */
        ly_print_(ctx->out, "/>%s", DO_FORMAT ? "\n" : "");
        return LY_SUCCESS;
    }

    /* children */
    ly_print_(ctx->out, ">%s", DO_FORMAT ? "\n" : "");

    LEVEL_INC;
    LY_LIST_FOR(node->child, child) {
        ret = xml_print_node(ctx, child);
        LY_CHECK_ERR_RET(ret, LEVEL_DEC, ret);
    }
    LEVEL_DEC;

    ly_print_(ctx->out, "%*s</%s>%s", INDENT, node->schema->name, DO_FORMAT ? "\n" : "");

    return LY_SUCCESS;
}

static LY_ERR
xml_print_anydata(struct xmlpr_ctx *ctx, const struct lyd_node_any *node)
{
    struct lyd_node_any *any = (struct lyd_node_any *)node;
    struct lyd_node *iter;
    uint32_t prev_opts, prev_lo;
    LY_ERR ret;

    xml_print_node_open(ctx, &node->node);

    if (!any->value.tree) {
        /* no content */
no_content:
        ly_print_(ctx->out, "/>%s", DO_FORMAT ? "\n" : "");
        return LY_SUCCESS;
    } else {
        if (any->value_type == LYD_ANYDATA_LYB) {
            /* turn logging off */
            prev_lo = ly_log_options(0);

            /* try to parse it into a data tree */
            if (lyd_parse_data_mem((struct ly_ctx *)LYD_CTX(node), any->value.mem, LYD_LYB, LYD_PARSE_ONLY | LYD_PARSE_OPAQ | LYD_PARSE_STRICT, 0, &iter) == LY_SUCCESS) {
                /* successfully parsed */
                free(any->value.mem);
                any->value.tree = iter;
                any->value_type = LYD_ANYDATA_DATATREE;
            }

            /* turn loggin on again */
            ly_log_options(prev_lo);
        }

        switch (any->value_type) {
        case LYD_ANYDATA_DATATREE:
            /* close opening tag and print data */
            prev_opts = ctx->options;
            ctx->options &= ~LYD_PRINT_WITHSIBLINGS;
            LEVEL_INC;

            ly_print_(ctx->out, ">%s", DO_FORMAT ? "\n" : "");
            LY_LIST_FOR(any->value.tree, iter) {
                ret = xml_print_node(ctx, iter);
                LY_CHECK_ERR_RET(ret, LEVEL_DEC, ret);
            }

            LEVEL_DEC;
            ctx->options = prev_opts;
            break;
        case LYD_ANYDATA_STRING:
            /* escape XML-sensitive characters */
            if (!any->value.str[0]) {
                goto no_content;
            }
            /* close opening tag and print data */
            ly_print_(ctx->out, ">");
            lyxml_dump_text(ctx->out, any->value.str, 0);
            break;
        case LYD_ANYDATA_XML:
            /* print without escaping special characters */
            if (!any->value.str[0]) {
                goto no_content;
            }
            ly_print_(ctx->out, ">%s", any->value.str);
            break;
        case LYD_ANYDATA_JSON:
        case LYD_ANYDATA_LYB:
            /* JSON and LYB format is not supported */
            LOGWRN(LYD_CTX(node), "Unable to print anydata content (type %d) as XML.", any->value_type);
            goto no_content;
        }

        /* closing tag */
        if (any->value_type == LYD_ANYDATA_DATATREE) {
            ly_print_(ctx->out, "%*s</%s>%s", INDENT, node->schema->name, DO_FORMAT ? "\n" : "");
        } else {
            ly_print_(ctx->out, "</%s>%s", node->schema->name, DO_FORMAT ? "\n" : "");
        }
    }

    return LY_SUCCESS;
}

static LY_ERR
xml_print_opaq(struct xmlpr_ctx *ctx, const struct lyd_node_opaq *node)
{
    LY_ERR ret;
    struct lyd_node *child;

    LY_CHECK_RET(xml_print_opaq_open(ctx, node));

    if (node->value[0]) {
        /* print namespaces connected with the value's prefixes */
        if (node->val_prefix_data) {
            xml_print_ns_prefix_data(ctx, node->format, node->val_prefix_data, LYXML_PREFIX_REQUIRED);
        }

        ly_print_(ctx->out, ">");
        lyxml_dump_text(ctx->out, node->value, 0);
    }

    if (node->child) {
        /* children */
        if (!node->value[0]) {
            ly_print_(ctx->out, ">%s", DO_FORMAT ? "\n" : "");
        }

        LEVEL_INC;
        LY_LIST_FOR(node->child, child) {
            ret = xml_print_node(ctx, child);
            LY_CHECK_ERR_RET(ret, LEVEL_DEC, ret);
        }
        LEVEL_DEC;

        ly_print_(ctx->out, "%*s</%s>%s", INDENT, node->name.name, DO_FORMAT ? "\n" : "");
    } else if (node->value[0]) {
        ly_print_(ctx->out, "</%s>%s", node->name.name, DO_FORMAT ? "\n" : "");
    } else {
        /* no value or children */
        ly_print_(ctx->out, "/>%s", DO_FORMAT ? "\n" : "");
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
xml_print_node(struct xmlpr_ctx *ctx, const struct lyd_node *node)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t ns_count;

    if (!ly_should_print(node, ctx->options)) {
        /* do not print at all */
        return LY_SUCCESS;
    }

    /* remember namespace definition count on this level */
    ns_count = ctx->ns.count;

    if (!node->schema) {
        ret = xml_print_opaq(ctx, (const struct lyd_node_opaq *)node);
    } else {
        switch (node->schema->nodetype) {
        case LYS_CONTAINER:
        case LYS_LIST:
        case LYS_NOTIF:
        case LYS_RPC:
        case LYS_ACTION:
            ret = xml_print_inner(ctx, (const struct lyd_node_inner *)node);
            break;
        case LYS_LEAF:
        case LYS_LEAFLIST:
            xml_print_term(ctx, (const struct lyd_node_term *)node);
            break;
        case LYS_ANYXML:
        case LYS_ANYDATA:
            ret = xml_print_anydata(ctx, (const struct lyd_node_any *)node);
            break;
        default:
            LOGINT(node->schema->module->ctx);
            ret = LY_EINT;
            break;
        }
    }

    /* remove all added namespaces */
    while (ns_count < ctx->ns.count) {
        lydict_remove(ctx->ctx, ctx->prefix.objs[ctx->prefix.count - 1]);
        ly_set_rm_index(&ctx->prefix, ctx->prefix.count - 1, NULL);
        ly_set_rm_index(&ctx->ns, ctx->ns.count - 1, NULL);
    }

    return ret;
}

LY_ERR
xml_print_data(struct ly_out *out, const struct lyd_node *root, uint32_t options)
{
    const struct lyd_node *node;
    struct xmlpr_ctx ctx = {0};

    if (!root) {
        if ((out->type == LY_OUT_MEMORY) || (out->type == LY_OUT_CALLBACK)) {
            ly_print_(out, "");
        }
        goto finish;
    }

    ctx.out = out;
    ctx.level = 0;
    ctx.options = options;
    ctx.ctx = LYD_CTX(root);

    /* content */
    LY_LIST_FOR(root, node) {
        LY_CHECK_RET(xml_print_node(&ctx, node));
        if (!(options & LYD_PRINT_WITHSIBLINGS)) {
            break;
        }
    }

finish:
    assert(!ctx.prefix.count && !ctx.ns.count);
    ly_set_erase(&ctx.prefix, NULL);
    ly_set_erase(&ctx.ns, NULL);
    ly_print_flush(out);
    return LY_SUCCESS;
}
