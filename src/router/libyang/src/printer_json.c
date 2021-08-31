/**
 * @file printer_json.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief JSON printer for libyang data structure
 *
 * Copyright (c) 2015 - 2020 CESNET, z.s.p.o.
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

#include "common.h"
#include "context.h"
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

/**
 * @brief JSON printer context.
 */
struct jsonpr_ctx {
    struct ly_out *out;         /**< output specification */
    uint16_t level;             /**< current indentation level: 0 - no formatting, >= 1 indentation levels */
    uint32_t options;           /**< [Data printer flags](@ref dataprinterflags) */
    const struct ly_ctx *ctx;   /**< libyang context */

    uint16_t level_printed;     /* level where some data were already printed */
    struct ly_set open; /* currently open array(s) */
    const struct lyd_node *print_sibling_metadata;
};

/**
 * @brief Mark that something was already written in the current level,
 * used to decide if a comma is expected between the items
 */
#define LEVEL_PRINTED ctx->level_printed = ctx->level

#define PRINT_COMMA \
    if (ctx->level_printed >= ctx->level) {\
        ly_print_(ctx->out, ",%s", (DO_FORMAT ? "\n" : ""));\
    }

static LY_ERR json_print_node(struct jsonpr_ctx *ctx, const struct lyd_node *node);

/**
 * Compare 2 nodes, despite it is regular data node or an opaq node, and
 * decide if they corresponds to the same schema node.
 *
 * TODO: rewrite lyd_compare_single and use it instead of this
 *
 * @return 1 - matching nodes, 0 - non-matching nodes
 */
static int
matching_node(const struct lyd_node *node1, const struct lyd_node *node2)
{
    assert(node1 || node2);

    if (!node1 || !node2) {
        return 0;
    } else if (node1->schema != node2->schema) {
        return 0;
    }
    if (!node1->schema) {
        /* compare node names */
        struct lyd_node_opaq *onode1 = (struct lyd_node_opaq *)node1;
        struct lyd_node_opaq *onode2 = (struct lyd_node_opaq *)node2;
        if ((onode1->name.name != onode2->name.name) || (onode1->name.prefix != onode2->name.prefix)) {
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Open the JSON array ('[') for the specified @p node
 *
 * @param[in] ctx JSON printer context.
 * @param[in] node First node of the array.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_array_open(struct jsonpr_ctx *ctx, const struct lyd_node *node)
{
    ly_print_(ctx->out, "[%s", DO_FORMAT ? "\n" : "");
    LY_CHECK_RET(ly_set_add(&ctx->open, (void *)node, 0, NULL));
    LEVEL_INC;

    return LY_SUCCESS;
}

/**
 * @brief Get know if the array for the provided @p node is currently open.
 *
 * @param[in] ctx JSON printer context.
 * @param[in] node Data node to check.
 * @return 1 in case the printer is currently in the array belonging to the provided @p node.
 * @return 0 in case the provided @p node is not connected with the currently open array (or there is no open array).
 */
static int
is_open_array(struct jsonpr_ctx *ctx, const struct lyd_node *node)
{
    if (ctx->open.count && matching_node(node, (const struct lyd_node *)ctx->open.objs[ctx->open.count - 1])) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Close the most inner JSON array.
 *
 * @param[in] ctx JSON printer context.
 */
static void
json_print_array_close(struct jsonpr_ctx *ctx)
{
    LEVEL_DEC;
    ly_set_rm_index(&ctx->open, ctx->open.count - 1, NULL);
    ly_print_(ctx->out, "%s%*s]", DO_FORMAT ? "\n" : "", INDENT);
}

/**
 * @brief Get the node's module name to use as the @p node prefix in JSON.
 *
 * @param[in] node Node to process.
 * @return The name of the module where the @p node belongs, it can be NULL in case the module name
 * cannot be determined (source format is XML and the refered namespace is unknown/not implemented in the current context).
 */
static const char *
node_prefix(const struct lyd_node *node)
{
    if (node->schema) {
        return node->schema->module->name;
    } else {
        struct lyd_node_opaq *onode = (struct lyd_node_opaq *)node;
        const struct lys_module *mod;

        switch (onode->format) {
        case LY_VALUE_JSON:
            return onode->name.module_name;
        case LY_VALUE_XML:
            mod = ly_ctx_get_module_implemented_ns(onode->ctx, onode->name.module_ns);
            if (!mod) {
                return NULL;
            }
            return mod->name;
        default:
            /* cannot be created */
            LOGINT(LYD_CTX(node));
        }
    }

    return NULL;
}

/**
 * @brief Compare 2 nodes if the belongs to the same module (if they come from the same namespace)
 *
 * Accepts both regulard a well as opaq nodes.
 *
 * @param[in] node1 The first node to compare.
 * @param[in] node2 The second node to compare.
 * @return 0 in case the nodes' modules are the same
 * @return 1 in case the nodes belongs to different modules
 */
int
json_nscmp(const struct lyd_node *node1, const struct lyd_node *node2)
{
    assert(node1 || node2);

    if (!node1 || !node2) {
        return 1;
    } else if (node1->schema && node2->schema) {
        if (node1->schema->module == node2->schema->module) {
            /* belongs to the same module */
            return 0;
        } else {
            /* different modules */
            return 1;
        }
    } else {
        const char *pref1 = node_prefix(node1);
        const char *pref2 = node_prefix(node2);
        if ((pref1 && pref2) && (pref1 == pref2)) {
            return 0;
        } else {
            return 1;
        }
    }
}

/**
 * @brief Print the @p text as JSON string - encode special characters and add quotation around the string.
 *
 * @param[in] out The output handler.
 * @param[in] text The string to print.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_string(struct ly_out *out, const char *text)
{
    uint64_t i, n;

    if (!text) {
        return LY_SUCCESS;
    }

    ly_write_(out, "\"", 1);
    for (i = n = 0; text[i]; i++) {
        const unsigned char ascii = text[i];
        if (ascii < 0x20) {
            /* control character */
            ly_print_(out, "\\u%.4X", ascii);
        } else {
            switch (ascii) {
            case '"':
                ly_print_(out, "\\\"");
                break;
            case '\\':
                ly_print_(out, "\\\\");
                break;
            default:
                ly_write_(out, &text[i], 1);
                n++;
            }
        }
    }
    ly_write_(out, "\"", 1);

    return LY_SUCCESS;
}

/**
 * @brief Print JSON object's member name, ending by ':'. It resolves if the prefix is supposed to be printed.
 *
 * @param[in] ctx JSON printer context.
 * @param[in] node The data node being printed.
 * @param[in] is_attr Flag if the metadata sign (@) is supposed to be added before the identifier.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_member(struct jsonpr_ctx *ctx, const struct lyd_node *node, ly_bool is_attr)
{
    PRINT_COMMA;
    if ((LEVEL == 1) || json_nscmp(node, (const struct lyd_node *)node->parent)) {
        /* print "namespace" */
        ly_print_(ctx->out, "%*s\"%s%s:%s\":%s", INDENT, is_attr ? "@" : "",
                node_prefix(node), node->schema->name, DO_FORMAT ? " " : "");
    } else {
        ly_print_(ctx->out, "%*s\"%s%s\":%s", INDENT, is_attr ? "@" : "",
                node->schema->name, DO_FORMAT ? " " : "");
    }

    return LY_SUCCESS;
}

/**
 * @brief More generic alternative to json_print_member() to print some special cases of the member names.
 *
 * @param[in] ctx JSON printer context.
 * @param[in] parent Parent node to compare modules deciding if the prefix is printed.
 * @param[in] format Format to decide how to process the @p prefix.
 * @param[in] name Name structure to provide name and prefix to print. If NULL, only "" name is printed.
 * @param[in] is_attr Flag if the metadata sign (@) is supposed to be added before the identifier.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_member2(struct jsonpr_ctx *ctx, const struct lyd_node *parent, LY_VALUE_FORMAT format,
        const struct ly_opaq_name *name, ly_bool is_attr)
{
    const char *module_name = NULL, *name_str;

    PRINT_COMMA;

    /* determine prefix string */
    if (name) {
        const struct lys_module *mod;

        switch (format) {
        case LY_VALUE_JSON:
            module_name = name->module_name;
            break;
        case LY_VALUE_XML:
            mod = ly_ctx_get_module_implemented_ns(ctx->ctx, name->module_ns);
            if (mod) {
                module_name = mod->name;
            }
            break;
        default:
            /* cannot be created */
            LOGINT_RET(ctx->ctx);
        }

        name_str = name->name;
    } else {
        name_str = "";
    }

    /* print the member */
    if (module_name && (!parent || (node_prefix(parent) != module_name))) {
        ly_print_(ctx->out, "%*s\"%s%s:%s\":%s", INDENT, is_attr ? "@" : "", module_name, name_str, DO_FORMAT ? " " : "");
    } else {
        ly_print_(ctx->out, "%*s\"%s%s\":%s", INDENT, is_attr ? "@" : "", name_str, DO_FORMAT ? " " : "");
    }

    return LY_SUCCESS;
}

/**
 * @brief Print data value.
 *
 * @param[in] ctx JSON printer context.
 * @param[in] val Data value to be printed.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_value(struct jsonpr_ctx *ctx, const struct lyd_value *val)
{
    ly_bool dynamic = 0;
    const char *value = val->realtype->plugin->print(ctx->ctx, val, LY_VALUE_JSON, NULL, &dynamic, NULL);

    /* leafref is not supported */
    switch (val->realtype->basetype) {
    case LY_TYPE_BINARY:
    case LY_TYPE_STRING:
    case LY_TYPE_BITS:
    case LY_TYPE_ENUM:
    case LY_TYPE_INST:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
    case LY_TYPE_DEC64:
    case LY_TYPE_IDENT:
    case LY_TYPE_UNION:
        json_print_string(ctx->out, value);
        break;

    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_BOOL:
        ly_print_(ctx->out, "%s", value[0] ? value : "null");
        break;

    case LY_TYPE_EMPTY:
        ly_print_(ctx->out, "[null]");
        break;

    default:
        /* error */
        LOGINT_RET(ctx->ctx);
    }

    if (dynamic) {
        free((char *)value);
    }

    return LY_SUCCESS;
}

/**
 * @brief Print all the attributes of the opaq node.
 *
 * @param[in] ctx JSON printer context.
 * @param[in] node Opaq node where the attributes are placed.
 * @param[in] wdmod With-defaults module to mark that default attribute is supposed to be printed.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_attribute(struct jsonpr_ctx *ctx, const struct lyd_node_opaq *node, const struct lys_module *wdmod)
{
    struct lyd_attr *attr;

    if (wdmod) {
        ly_print_(ctx->out, "%*s\"%s:default\":\"true\"", INDENT, wdmod->name);
        LEVEL_PRINTED;
    }

    for (attr = node->attr; attr; attr = attr->next) {
        PRINT_COMMA;
        json_print_member2(ctx, &node->node, attr->format, &attr->name, 0);

        if (attr->hints & (LYD_VALHINT_BOOLEAN | LYD_VALHINT_DECNUM)) {
            ly_print_(ctx->out, "%s", attr->value[0] ? attr->value : "null");
        } else if (attr->hints & LYD_VALHINT_EMPTY) {
            ly_print_(ctx->out, "[null]");
        } else {
            json_print_string(ctx->out, attr->value);
        }
        LEVEL_PRINTED;
    }

    return LY_SUCCESS;
}

/**
 * @brief Print all the metadata of the node.
 *
 * @param[in] ctx JSON printer context.
 * @param[in] node Node where the metadata are placed.
 * @param[in] wdmod With-defaults module to mark that default attribute is supposed to be printed.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_metadata(struct jsonpr_ctx *ctx, const struct lyd_node *node, const struct lys_module *wdmod)
{
    struct lyd_meta *meta;

    if (wdmod) {
        ly_print_(ctx->out, "%*s\"%s:default\":\"true\"", INDENT, wdmod->name);
        LEVEL_PRINTED;
    }

    for (meta = node->meta; meta; meta = meta->next) {
        PRINT_COMMA;
        ly_print_(ctx->out, "%*s\"%s:%s\":%s", INDENT, meta->annotation->module->name, meta->name, DO_FORMAT ? " " : "");
        LY_CHECK_RET(json_print_value(ctx, &meta->value));
        LEVEL_PRINTED;
    }

    return LY_SUCCESS;
}

/**
 * @brief Print attributes/metadata of the given @p node. Accepts both regular as well as opaq nodes.
 *
 * @param[in] ctx JSON printer context.
 * @param[in] node Data node where the attributes/metadata are placed.
 * @param[in] inner Flag if the @p node is an inner node in the tree.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_attributes(struct jsonpr_ctx *ctx, const struct lyd_node *node, ly_bool inner)
{
    const struct lys_module *wdmod = NULL;

    if ((node->flags & LYD_DEFAULT) && (ctx->options & (LYD_PRINT_WD_ALL_TAG | LYD_PRINT_WD_IMPL_TAG))) {
        /* we have implicit OR explicit default node */
        /* get with-defaults module */
        wdmod = ly_ctx_get_module_implemented(LYD_CTX(node), "ietf-netconf-with-defaults");
    }

    if (node->schema && node->meta) {
        if (inner) {
            LY_CHECK_RET(json_print_member2(ctx, NULL, LY_VALUE_JSON, NULL, 1));
        } else {
            LY_CHECK_RET(json_print_member(ctx, node, 1));
        }
        ly_print_(ctx->out, "{%s", (DO_FORMAT ? "\n" : ""));
        LEVEL_INC;
        LY_CHECK_RET(json_print_metadata(ctx, node, wdmod));
        LEVEL_DEC;
        ly_print_(ctx->out, "%s%*s}", DO_FORMAT ? "\n" : "", INDENT);
        LEVEL_PRINTED;
    } else if (!node->schema && ((struct lyd_node_opaq *)node)->attr) {
        if (inner) {
            LY_CHECK_RET(json_print_member2(ctx, NULL, LY_VALUE_JSON, NULL, 1));
        } else {
            LY_CHECK_RET(json_print_member2(ctx, node, ((struct lyd_node_opaq *)node)->format,
                    &((struct lyd_node_opaq *)node)->name, 1));
        }
        ly_print_(ctx->out, "{%s", (DO_FORMAT ? "\n" : ""));
        LEVEL_INC;
        LY_CHECK_RET(json_print_attribute(ctx, (struct lyd_node_opaq *)node, wdmod));
        LEVEL_DEC;
        ly_print_(ctx->out, "%s%*s}", DO_FORMAT ? "\n" : "", INDENT);
        LEVEL_PRINTED;
    }

    return LY_SUCCESS;
}

/**
 * @brief Print leaf data node including its metadata.
 *
 * @param[in] ctx JSON printer context.
 * @param[in] node Data node to print.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_leaf(struct jsonpr_ctx *ctx, const struct lyd_node *node)
{
    LY_CHECK_RET(json_print_member(ctx, node, 0));
    LY_CHECK_RET(json_print_value(ctx, &((const struct lyd_node_term *)node)->value));
    LEVEL_PRINTED;

    /* print attributes as sibling */
    json_print_attributes(ctx, node, 0);

    return LY_SUCCESS;
}

/**
 * @brief Print anydata data node including its metadata.
 *
 * @param[in] ctx JSON printer context.
 * @param[in] any Anydata node to print.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_anydata(struct jsonpr_ctx *ctx, struct lyd_node_any *any)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *iter;
    uint32_t prev_opts, prev_lo;

    if (!any->value.tree) {
        /* no content */
        return LY_SUCCESS;
    }

    if (any->value_type == LYD_ANYDATA_LYB) {
        uint32_t parser_options = LYD_PARSE_ONLY | LYD_PARSE_OPAQ | LYD_PARSE_STRICT;

        /* turn logging off */
        prev_lo = ly_log_options(0);

        /* try to parse it into a data tree */
        if (lyd_parse_data_mem(ctx->ctx, any->value.mem, LYD_LYB, parser_options, 0, &iter) == LY_SUCCESS) {
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

        LY_LIST_FOR(any->value.tree, iter) {
            ret = json_print_node(ctx, iter);
            LY_CHECK_ERR_RET(ret, LEVEL_DEC, ret);
        }

        ctx->options = prev_opts;
        break;
    case LYD_ANYDATA_JSON:
        /* print without escaping special characters */
        if (!any->value.str[0]) {
            return LY_SUCCESS;
        }
        ly_print_(ctx->out, "%*s%s", INDENT, any->value.str);
        break;
    case LYD_ANYDATA_STRING:
    case LYD_ANYDATA_XML:
    case LYD_ANYDATA_LYB:
        /* JSON and LYB format is not supported */
        LOGWRN(ctx->ctx, "Unable to print anydata content (type %d) as XML.", any->value_type);
        return LY_SUCCESS;
    }

    return LY_SUCCESS;
}

/**
 * @brief Print content of a single container/list data node including its metadata.
 * The envelope specific to container and list are expected to be printed by the caller.
 *
 * @param[in] ctx JSON printer context.
 * @param[in] node Data node to print.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_inner(struct jsonpr_ctx *ctx, const struct lyd_node *node)
{
    struct lyd_node *child;
    ly_bool has_content = 0;

    LY_LIST_FOR(lyd_child(node), child) {
        if (ly_should_print(child, ctx->options)) {
            break;
        }
    }
    if (node->meta || child) {
        has_content = 1;
    } else if (node->schema && (node->schema->nodetype & LYD_NODE_ANY) && ((struct lyd_node_any *)node)->value.tree) {
        has_content = 1;
    }

    if (!node->schema || (node->schema->nodetype != LYS_LIST)) {
        ly_print_(ctx->out, "%s{%s", (is_open_array(ctx, node) && ctx->level_printed >= ctx->level) ? "," : "",
                (DO_FORMAT && has_content) ? "\n" : "");
    } else {
        ly_print_(ctx->out, "%s%*s{%s", (is_open_array(ctx, node) && ctx->level_printed >= ctx->level) ? (DO_FORMAT ? ",\n" : ",") : "",
                INDENT, (DO_FORMAT && has_content) ? "\n" : "");
    }
    LEVEL_INC;

    json_print_attributes(ctx, node, 1);

    if (!node->schema || !(node->schema->nodetype & LYS_ANYDATA)) {
        /* print children */
        LY_LIST_FOR(lyd_child(node), child) {
            LY_CHECK_RET(json_print_node(ctx, child));
        }
    } else {
        /* anydata */
        json_print_anydata(ctx, (struct lyd_node_any *)node);
    }

    LEVEL_DEC;
    if (DO_FORMAT && has_content) {
        ly_print_(ctx->out, "\n%*s}", INDENT);
    } else {
        ly_print_(ctx->out, "}");
    }
    LEVEL_PRINTED;

    return LY_SUCCESS;
}

/**
 * @brief Print container data node including its metadata.
 *
 * @param[in] ctx JSON printer context.
 * @param[in] node Data node to print.
 * @return LY_ERR value.
 */
static int
json_print_container(struct jsonpr_ctx *ctx, const struct lyd_node *node)
{
    LY_CHECK_RET(json_print_member(ctx, node, 0));
    LY_CHECK_RET(json_print_inner(ctx, node));

    return LY_SUCCESS;
}

/**
 * @brief Print single leaf-list or list instance.
 *
 * In case of list, metadata are printed inside the list object. For the leaf-list,
 * metadata are marked in the context for later printing after closing the array next to it using
 * json_print_metadata_leaflist().
 *
 * @param[in] ctx JSON printer context.
 * @param[in] node Data node to print.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_leaf_list(struct jsonpr_ctx *ctx, const struct lyd_node *node)
{
    if (!is_open_array(ctx, node)) {
        LY_CHECK_RET(json_print_member(ctx, node, 0));
        LY_CHECK_RET(json_print_array_open(ctx, node));
        if (node->schema->nodetype == LYS_LEAFLIST) {
            ly_print_(ctx->out, "%*s", INDENT);
        }
    } else if (node->schema->nodetype == LYS_LEAFLIST) {
        ly_print_(ctx->out, ",%s%*s", DO_FORMAT ? "\n" : "", INDENT);
    }

    if (node->schema->nodetype == LYS_LIST) {
        if (!lyd_child(node)) {
            /* empty, e.g. in case of filter */
            ly_print_(ctx->out, "%s%snull", (ctx->level_printed >= ctx->level) ? "," : "", DO_FORMAT ? " " : "");
            LEVEL_PRINTED;
        } else {
            /* print list's content */
            LY_CHECK_RET(json_print_inner(ctx, node));
        }
    } else {
        assert(node->schema->nodetype == LYS_LEAFLIST);
        LY_CHECK_RET(json_print_value(ctx, &((const struct lyd_node_term *)node)->value));

        if (node->meta && !ctx->print_sibling_metadata) {
            ctx->print_sibling_metadata = node;
        }
    }

    if (is_open_array(ctx, node) && (!node->next || (node->next->schema != node->schema))) {
        json_print_array_close(ctx);
    }

    return LY_SUCCESS;
}

/**
 * @brief Print leaf-list's metadata in case they were marked in the last call to json_print_leaf_list().
 * This function is supposed to be called when the leaf-list array is closed.
 *
 * @param[in] ctx JSON printer context.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_metadata_leaflist(struct jsonpr_ctx *ctx)
{
    const struct lyd_node *prev, *node, *iter;

    if (!ctx->print_sibling_metadata) {
        return LY_SUCCESS;
    }

    for (node = ctx->print_sibling_metadata, prev = ctx->print_sibling_metadata->prev;
            prev->next && matching_node(node, prev);
            node = prev, prev = node->prev) {}

    /* node is the first instance of the leaf-list */

    LY_CHECK_RET(json_print_member(ctx, node, 1));
    ly_print_(ctx->out, "[%s", (DO_FORMAT ? "\n" : ""));
    LEVEL_INC;
    LY_LIST_FOR(node, iter) {
        PRINT_COMMA;
        if (iter->meta) {
            ly_print_(ctx->out, "%*s%s", INDENT, DO_FORMAT ? "{\n" : "{");
            LEVEL_INC;
            LY_CHECK_RET(json_print_metadata(ctx, iter, NULL));
            LEVEL_DEC;
            ly_print_(ctx->out, "%s%*s}", DO_FORMAT ? "\n" : "", INDENT);
        } else {
            ly_print_(ctx->out, "null");
        }
        LEVEL_PRINTED;
        if (!matching_node(iter, iter->next)) {
            break;
        }
    }
    LEVEL_DEC;
    ly_print_(ctx->out, "%s%*s]", DO_FORMAT ? "\n" : "", INDENT);
    LEVEL_PRINTED;

    return LY_SUCCESS;
}

/**
 * @brief Print opaq data node including its attributes.
 *
 * @param[in] ctx JSON printer context.
 * @param[in] node Opaq node to print.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_opaq(struct jsonpr_ctx *ctx, const struct lyd_node_opaq *node)
{
    ly_bool first = 1, last = 1;

    if (node->hints & (LYD_NODEHINT_LIST | LYD_NODEHINT_LEAFLIST)) {
        if (node->prev->next && matching_node(node->prev, &node->node)) {
            first = 0;
        }
        if (node->next && matching_node(&node->node, node->next)) {
            last = 0;
        }
    }

    if (first) {
        LY_CHECK_RET(json_print_member2(ctx, lyd_parent(&node->node), node->format, &node->name, 0));

        if (node->hints & (LYD_NODEHINT_LIST | LYD_NODEHINT_LEAFLIST)) {
            LY_CHECK_RET(json_print_array_open(ctx, &node->node));
            LEVEL_INC;
        }
    }
    if (node->child || (node->hints & (LYD_NODEHINT_LIST | LYD_NODEHINT_LEAFLIST))) {
        LY_CHECK_RET(json_print_inner(ctx, &node->node));
        LEVEL_PRINTED;
    } else {
        if (node->hints & LYD_VALHINT_EMPTY) {
            ly_print_(ctx->out, "[null]");
        } else if (node->hints & (LYD_VALHINT_BOOLEAN | LYD_VALHINT_DECNUM)) {
            ly_print_(ctx->out, "%s", node->value);
        } else {
            /* string */
            ly_print_(ctx->out, "\"%s\"", node->value);
        }
        LEVEL_PRINTED;

        /* attributes */
        json_print_attributes(ctx, (const struct lyd_node *)node, 0);

    }
    if (last && (node->hints & (LYD_NODEHINT_LIST | LYD_NODEHINT_LEAFLIST))) {
        json_print_array_close(ctx);
        LEVEL_DEC;
        LEVEL_PRINTED;
    }

    return LY_SUCCESS;
}

/**
 * @brief Print all the types of data node including its metadata.
 *
 * @param[in] ctx JSON printer context.
 * @param[in] node Data node to print.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_node(struct jsonpr_ctx *ctx, const struct lyd_node *node)
{
    if (!ly_should_print(node, ctx->options)) {
        if (is_open_array(ctx, node) && (!node->next || (node->next->schema != node->schema))) {
            json_print_array_close(ctx);
        }
        return LY_SUCCESS;
    }

    if (!node->schema) {
        LY_CHECK_RET(json_print_opaq(ctx, (const struct lyd_node_opaq *)node));
    } else {
        switch (node->schema->nodetype) {
        case LYS_RPC:
        case LYS_ACTION:
        case LYS_NOTIF:
        case LYS_CONTAINER:
            LY_CHECK_RET(json_print_container(ctx, node));
            break;
        case LYS_LEAF:
            LY_CHECK_RET(json_print_leaf(ctx, node));
            break;
        case LYS_LEAFLIST:
        case LYS_LIST:
            LY_CHECK_RET(json_print_leaf_list(ctx, node));
            break;
        case LYS_ANYXML:
        case LYS_ANYDATA:
            LY_CHECK_RET(json_print_container(ctx, node));
            break;
        default:
            LOGINT(node->schema->module->ctx);
            return EXIT_FAILURE;
        }
    }

    ctx->level_printed = ctx->level;

    if (ctx->print_sibling_metadata && !matching_node(node->next, ctx->print_sibling_metadata)) {
        json_print_metadata_leaflist(ctx);
        ctx->print_sibling_metadata = NULL;
    }

    return LY_SUCCESS;
}

LY_ERR
json_print_data(struct ly_out *out, const struct lyd_node *root, uint32_t options)
{
    const struct lyd_node *node;
    struct jsonpr_ctx ctx = {0};
    const char *delimiter = (options & LYD_PRINT_SHRINK) ? "" : "\n";

    if (!root) {
        ly_print_(out, "{}%s", delimiter);
        ly_print_flush(out);
        return LY_SUCCESS;
    }

    ctx.out = out;
    ctx.level = 1;
    ctx.level_printed = 0;
    ctx.options = options;
    ctx.ctx = LYD_CTX(root);

    /* start */
    ly_print_(ctx.out, "{%s", delimiter);

    /* content */
    LY_LIST_FOR(root, node) {
        LY_CHECK_RET(json_print_node(&ctx, node));
        if (!(options & LYD_PRINT_WITHSIBLINGS)) {
            break;
        }
    }

    /* end */
    ly_print_(out, "%s}%s", delimiter, delimiter);

    assert(!ctx.open.count);
    ly_set_erase(&ctx.open, NULL);

    ly_print_flush(out);
    return LY_SUCCESS;
}
