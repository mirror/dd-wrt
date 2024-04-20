/**
 * @file printer_json.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief JSON printer for libyang data structure
 *
 * Copyright (c) 2015 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>

#include "context.h"
#include "log.h"
#include "ly_common.h"
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

/**
 * @brief JSON printer context.
 */
struct jsonpr_ctx {
    struct ly_out *out;         /**< output specification */
    const struct lyd_node *root;    /**< root node of the subtree being printed */
    const struct lyd_node *parent;  /**< parent of the node being printed */
    uint16_t level;             /**< current indentation level: 0 - no formatting, >= 1 indentation levels */
    uint32_t options;           /**< [Data printer flags](@ref dataprinterflags) */
    const struct ly_ctx *ctx;   /**< libyang context */

    uint16_t level_printed;     /* level where some data were already printed */
    struct ly_set open;         /* currently open array(s) */
    const struct lyd_node *first_leaflist;  /**< first printed leaf-list instance, used when printing its metadata/attributes */
};

/**
 * @brief Mark that something was already written in the current level,
 * used to decide if a comma is expected between the items
 */
#define LEVEL_PRINTED pctx->level_printed = pctx->level

#define PRINT_COMMA \
    if (pctx->level_printed >= pctx->level) { \
        ly_print_(pctx->out, ",%s", (DO_FORMAT ? "\n" : "")); \
    }

static LY_ERR json_print_node(struct jsonpr_ctx *pctx, const struct lyd_node *node);

/**
 * @brief Compare 2 nodes, despite it is regular data node or an opaq node, and
 * decide if they corresponds to the same schema node.
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
json_print_array_open(struct jsonpr_ctx *pctx, const struct lyd_node *node)
{
    ly_print_(pctx->out, "[%s", DO_FORMAT ? "\n" : "");
    LY_CHECK_RET(ly_set_add(&pctx->open, (void *)node, 0, NULL));
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
is_open_array(struct jsonpr_ctx *pctx, const struct lyd_node *node)
{
    if (pctx->open.count && matching_node(node, pctx->open.dnodes[pctx->open.count - 1])) {
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
json_print_array_close(struct jsonpr_ctx *pctx)
{
    LEVEL_DEC;
    ly_set_rm_index(&pctx->open, pctx->open.count - 1, NULL);
    ly_print_(pctx->out, "%s%*s]", DO_FORMAT ? "\n" : "", INDENT);
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
    uint64_t i;

    if (!text) {
        return LY_SUCCESS;
    }

    ly_write_(out, "\"", 1);
    for (i = 0; text[i]; i++) {
        const unsigned char byte = text[i];

        switch (byte) {
        case '"':
            ly_print_(out, "\\\"");
            break;
        case '\\':
            ly_print_(out, "\\\\");
            break;
        case '\r':
            ly_print_(out, "\\r");
            break;
        case '\t':
            ly_print_(out, "\\t");
            break;
        default:
            if (iscntrl(byte)) {
                /* control character */
                ly_print_(out, "\\u%.4X", byte);
            } else {
                /* printable character (even non-ASCII UTF8) */
                ly_write_(out, &text[i], 1);
            }
            break;
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
json_print_member(struct jsonpr_ctx *pctx, const struct lyd_node *node, ly_bool is_attr)
{
    PRINT_COMMA;
    if ((LEVEL == 1) || json_nscmp(node, pctx->parent)) {
        /* print "namespace" */
        ly_print_(pctx->out, "%*s\"%s%s:%s\":%s", INDENT, is_attr ? "@" : "",
                node_prefix(node), node->schema->name, DO_FORMAT ? " " : "");
    } else {
        ly_print_(pctx->out, "%*s\"%s%s\":%s", INDENT, is_attr ? "@" : "", node->schema->name, DO_FORMAT ? " " : "");
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
json_print_member2(struct jsonpr_ctx *pctx, const struct lyd_node *parent, LY_VALUE_FORMAT format,
        const struct ly_opaq_name *name, ly_bool is_attr)
{
    const char *module_name = NULL, *name_str;

    PRINT_COMMA;

    /* determine prefix string */
    if (name) {
        switch (format) {
        case LY_VALUE_JSON:
            module_name = name->module_name;
            break;
        case LY_VALUE_XML: {
            const struct lys_module *mod = NULL;

            if (name->module_ns) {
                mod = ly_ctx_get_module_implemented_ns(pctx->ctx, name->module_ns);
            }
            if (mod) {
                module_name = mod->name;
            }
            break;
        }
        default:
            /* cannot be created */
            LOGINT_RET(pctx->ctx);
        }

        name_str = name->name;
    } else {
        name_str = "";
    }

    /* print the member */
    if (module_name && (!parent || (node_prefix(parent) != module_name))) {
        ly_print_(pctx->out, "%*s\"%s%s:%s\":%s", INDENT, is_attr ? "@" : "", module_name, name_str, DO_FORMAT ? " " : "");
    } else {
        ly_print_(pctx->out, "%*s\"%s%s\":%s", INDENT, is_attr ? "@" : "", name_str, DO_FORMAT ? " " : "");
    }

    return LY_SUCCESS;
}

/**
 * @brief Print data value.
 *
 * @param[in] pctx JSON printer context.
 * @param[in] ctx Context used to print the value.
 * @param[in] val Data value to be printed.
 * @param[in] local_mod Module of the current node.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_value(struct jsonpr_ctx *pctx, const struct ly_ctx *ctx, const struct lyd_value *val,
        const struct lys_module *local_mod)
{
    ly_bool dynamic;
    LY_DATA_TYPE basetype;
    const char *value;

    value = val->realtype->plugin->print(ctx, val, LY_VALUE_JSON, (void *)local_mod, &dynamic, NULL);
    LY_CHECK_RET(!value, LY_EINVAL);
    basetype = val->realtype->basetype;

print_val:
    /* leafref is not supported */
    switch (basetype) {
    case LY_TYPE_UNION:
        /* use the resolved type */
        val = &val->subvalue->value;
        basetype = val->realtype->basetype;
        goto print_val;

    case LY_TYPE_BINARY:
    case LY_TYPE_STRING:
    case LY_TYPE_BITS:
    case LY_TYPE_ENUM:
    case LY_TYPE_INST:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
    case LY_TYPE_DEC64:
    case LY_TYPE_IDENT:
        json_print_string(pctx->out, value);
        break;

    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_BOOL:
        ly_print_(pctx->out, "%s", value[0] ? value : "null");
        break;

    case LY_TYPE_EMPTY:
        ly_print_(pctx->out, "[null]");
        break;

    default:
        /* error */
        LOGINT_RET(pctx->ctx);
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
 * @return LY_ERR value.
 */
static LY_ERR
json_print_attribute(struct jsonpr_ctx *pctx, const struct lyd_node_opaq *node)
{
    struct lyd_attr *attr;

    for (attr = node->attr; attr; attr = attr->next) {
        json_print_member2(pctx, &node->node, attr->format, &attr->name, 0);

        if (attr->hints & (LYD_VALHINT_STRING | LYD_VALHINT_OCTNUM | LYD_VALHINT_HEXNUM | LYD_VALHINT_NUM64)) {
            json_print_string(pctx->out, attr->value);
        } else if (attr->hints & (LYD_VALHINT_BOOLEAN | LYD_VALHINT_DECNUM)) {
            ly_print_(pctx->out, "%s", attr->value[0] ? attr->value : "null");
        } else if (attr->hints & LYD_VALHINT_EMPTY) {
            ly_print_(pctx->out, "[null]");
        } else {
            /* unknown value format with no hints, use universal string */
            json_print_string(pctx->out, attr->value);
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
json_print_metadata(struct jsonpr_ctx *pctx, const struct lyd_node *node, const struct lys_module *wdmod)
{
    struct lyd_meta *meta;

    if (wdmod) {
        ly_print_(pctx->out, "%*s\"%s:default\":%strue", INDENT, wdmod->name, DO_FORMAT ? " " : "");
        LEVEL_PRINTED;
    }

    for (meta = node->meta; meta; meta = meta->next) {
        if (!lyd_metadata_should_print(meta)) {
            continue;
        }
        PRINT_COMMA;
        ly_print_(pctx->out, "%*s\"%s:%s\":%s", INDENT, meta->annotation->module->name, meta->name, DO_FORMAT ? " " : "");
        LY_CHECK_RET(json_print_value(pctx, LYD_CTX(node), &meta->value, NULL));
        LEVEL_PRINTED;
    }

    return LY_SUCCESS;
}

/**
 * @brief Check if a value can be printed for at least one metadata.
 *
 * @param[in] node Node to check.
 * @return 1 if node has printable meta otherwise 0.
 */
static ly_bool
node_has_printable_meta(const struct lyd_node *node)
{
    struct lyd_meta *iter;

    if (!node->meta) {
        return 0;
    }

    LY_LIST_FOR(node->meta, iter) {
        if (lyd_metadata_should_print(iter)) {
            return 1;
        }
    }

    return 0;
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
json_print_attributes(struct jsonpr_ctx *pctx, const struct lyd_node *node, ly_bool inner)
{
    const struct lys_module *wdmod = NULL;

    if (node->schema && (node->schema->nodetype != LYS_CONTAINER) && (((node->flags & LYD_DEFAULT) &&
            (pctx->options & (LYD_PRINT_WD_ALL_TAG | LYD_PRINT_WD_IMPL_TAG))) ||
            ((pctx->options & LYD_PRINT_WD_ALL_TAG) && lyd_is_default(node)))) {
        /* we have implicit OR explicit default node */
        /* get with-defaults module */
        wdmod = ly_ctx_get_module_implemented(LYD_CTX(node), "ietf-netconf-with-defaults");
    }

    if (node->schema && (wdmod || node_has_printable_meta(node))) {
        if (inner) {
            LY_CHECK_RET(json_print_member2(pctx, lyd_parent(node), LY_VALUE_JSON, NULL, 1));
        } else {
            LY_CHECK_RET(json_print_member(pctx, node, 1));
        }
        ly_print_(pctx->out, "{%s", (DO_FORMAT ? "\n" : ""));
        LEVEL_INC;
        LY_CHECK_RET(json_print_metadata(pctx, node, wdmod));
        LEVEL_DEC;
        ly_print_(pctx->out, "%s%*s}", DO_FORMAT ? "\n" : "", INDENT);
        LEVEL_PRINTED;
    } else if (!node->schema && ((struct lyd_node_opaq *)node)->attr) {
        if (inner) {
            LY_CHECK_RET(json_print_member2(pctx, lyd_parent(node), LY_VALUE_JSON, NULL, 1));
        } else {
            LY_CHECK_RET(json_print_member2(pctx, lyd_parent(node), ((struct lyd_node_opaq *)node)->format,
                    &((struct lyd_node_opaq *)node)->name, 1));
        }
        ly_print_(pctx->out, "{%s", (DO_FORMAT ? "\n" : ""));
        LEVEL_INC;
        LY_CHECK_RET(json_print_attribute(pctx, (struct lyd_node_opaq *)node));
        LEVEL_DEC;
        ly_print_(pctx->out, "%s%*s}", DO_FORMAT ? "\n" : "", INDENT);
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
json_print_leaf(struct jsonpr_ctx *pctx, const struct lyd_node *node)
{
    LY_CHECK_RET(json_print_member(pctx, node, 0));
    LY_CHECK_RET(json_print_value(pctx, LYD_CTX(node), &((const struct lyd_node_term *)node)->value, node->schema->module));
    LEVEL_PRINTED;

    /* print attributes as sibling */
    json_print_attributes(pctx, node, 0);

    return LY_SUCCESS;
}

/**
 * @brief Print anydata/anyxml content.
 *
 * @param[in] ctx JSON printer context.
 * @param[in] any Anydata node to print.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_any_content(struct jsonpr_ctx *pctx, struct lyd_node_any *any)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *iter;
    const struct lyd_node *prev_parent;
    uint32_t prev_opts, *prev_lo, temp_lo = 0;

    assert(any->schema->nodetype & LYD_NODE_ANY);

    if ((any->schema->nodetype == LYS_ANYDATA) && (any->value_type != LYD_ANYDATA_DATATREE)) {
        LOGINT_RET(pctx->ctx);
    }

    if (any->value_type == LYD_ANYDATA_LYB) {
        uint32_t parser_options = LYD_PARSE_ONLY | LYD_PARSE_OPAQ | LYD_PARSE_STRICT;

        /* turn logging off */
        prev_lo = ly_temp_log_options(&temp_lo);

        /* try to parse it into a data tree */
        if (lyd_parse_data_mem(pctx->ctx, any->value.mem, LYD_LYB, parser_options, 0, &iter) == LY_SUCCESS) {
            /* successfully parsed */
            free(any->value.mem);
            any->value.tree = iter;
            any->value_type = LYD_ANYDATA_DATATREE;
        }

        /* turn logging on again */
        ly_temp_log_options(prev_lo);
    }

    switch (any->value_type) {
    case LYD_ANYDATA_DATATREE:
        /* print as an object */
        ly_print_(pctx->out, "{%s", DO_FORMAT ? "\n" : "");
        LEVEL_INC;

        /* close opening tag and print data */
        prev_parent = pctx->parent;
        prev_opts = pctx->options;
        pctx->parent = &any->node;
        pctx->options &= ~LYD_PRINT_WITHSIBLINGS;
        LY_LIST_FOR(any->value.tree, iter) {
            ret = json_print_node(pctx, iter);
            LY_CHECK_ERR_RET(ret, LEVEL_DEC, ret);
        }
        pctx->parent = prev_parent;
        pctx->options = prev_opts;

        /* terminate the object */
        LEVEL_DEC;
        if (DO_FORMAT) {
            ly_print_(pctx->out, "\n%*s}", INDENT);
        } else {
            ly_print_(pctx->out, "}");
        }
        break;
    case LYD_ANYDATA_JSON:
        if (!any->value.json) {
            /* no content */
            if (any->schema->nodetype == LYS_ANYXML) {
                ly_print_(pctx->out, "null");
            } else {
                ly_print_(pctx->out, "{}");
            }
        } else {
            /* print without escaping special characters */
            ly_print_(pctx->out, "%s", any->value.json);
        }
        break;
    case LYD_ANYDATA_STRING:
    case LYD_ANYDATA_XML:
        if (!any->value.str) {
            /* no content */
            if (any->schema->nodetype == LYS_ANYXML) {
                ly_print_(pctx->out, "null");
            } else {
                ly_print_(pctx->out, "{}");
            }
        } else {
            /* print as a string */
            json_print_string(pctx->out, any->value.str);
        }
        break;
    case LYD_ANYDATA_LYB:
        /* LYB format is not supported */
        LOGWRN(pctx->ctx, "Unable to print anydata content (type %d) as JSON.", any->value_type);
        break;
    }

    return LY_SUCCESS;
}

/**
 * @brief Print content of a single container/list data node including its metadata.
 * The envelope specific to nodes are expected to be printed by the caller.
 *
 * @param[in] ctx JSON printer context.
 * @param[in] node Data node to print.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_inner(struct jsonpr_ctx *pctx, const struct lyd_node *node)
{
    struct lyd_node *child;
    const struct lyd_node *prev_parent;
    struct lyd_node_opaq *opaq = NULL;
    ly_bool has_content = 0;

    LY_LIST_FOR(lyd_child(node), child) {
        if (lyd_node_should_print(child, pctx->options)) {
            break;
        }
    }
    if (node->meta || child) {
        has_content = 1;
    }
    if (!node->schema) {
        opaq = (struct lyd_node_opaq *)node;
    }

    if ((node->schema && (node->schema->nodetype == LYS_LIST)) ||
            (opaq && (opaq->hints != LYD_HINT_DATA) && (opaq->hints & LYD_NODEHINT_LIST))) {
        ly_print_(pctx->out, "%s%*s{%s", (is_open_array(pctx, node) && (pctx->level_printed >= pctx->level)) ?
                (DO_FORMAT ? ",\n" : ",") : "", INDENT, (DO_FORMAT && has_content) ? "\n" : "");
    } else {
        ly_print_(pctx->out, "%s{%s", (is_open_array(pctx, node) && (pctx->level_printed >= pctx->level)) ? "," : "",
                (DO_FORMAT && has_content) ? "\n" : "");
    }
    LEVEL_INC;

    json_print_attributes(pctx, node, 1);

    /* print children */
    prev_parent = pctx->parent;
    pctx->parent = node;
    LY_LIST_FOR(lyd_child(node), child) {
        LY_CHECK_RET(json_print_node(pctx, child));
    }
    pctx->parent = prev_parent;

    LEVEL_DEC;
    if (DO_FORMAT && has_content) {
        ly_print_(pctx->out, "\n%*s}", INDENT);
    } else {
        ly_print_(pctx->out, "}");
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
json_print_container(struct jsonpr_ctx *pctx, const struct lyd_node *node)
{
    LY_CHECK_RET(json_print_member(pctx, node, 0));
    LY_CHECK_RET(json_print_inner(pctx, node));

    return LY_SUCCESS;
}

/**
 * @brief Print anydata/anyxml data node including its metadata.
 *
 * @param[in] ctx JSON printer context.
 * @param[in] node Data node to print.
 * @return LY_ERR value.
 */
static int
json_print_any(struct jsonpr_ctx *pctx, const struct lyd_node *node)
{
    LY_CHECK_RET(json_print_member(pctx, node, 0));
    LY_CHECK_RET(json_print_any_content(pctx, (struct lyd_node_any *)node));
    LEVEL_PRINTED;

    /* print attributes as sibling */
    json_print_attributes(pctx, node, 0);

    return LY_SUCCESS;
}

/**
 * @brief Check whether a node is the last printed instance of a (leaf-)list.
 *
 * @param[in] ctx JSON printer context.
 * @param[in] node Last printed node.
 * @return Whether it is the last printed instance or not.
 */
static ly_bool
json_print_array_is_last_inst(struct jsonpr_ctx *pctx, const struct lyd_node *node)
{
    if (!is_open_array(pctx, node)) {
        /* no array open */
        return 0;
    }

    if ((pctx->root == node) && !(pctx->options & LYD_PRINT_WITHSIBLINGS)) {
        /* the only printed instance */
        return 1;
    }

    if (!node->next || (node->next->schema != node->schema)) {
        /* last instance */
        return 1;
    }

    return 0;
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
json_print_leaf_list(struct jsonpr_ctx *pctx, const struct lyd_node *node)
{
    const struct lys_module *wdmod = NULL;

    if (!is_open_array(pctx, node)) {
        LY_CHECK_RET(json_print_member(pctx, node, 0));
        LY_CHECK_RET(json_print_array_open(pctx, node));
        if (node->schema->nodetype == LYS_LEAFLIST) {
            ly_print_(pctx->out, "%*s", INDENT);
        }
    } else if (node->schema->nodetype == LYS_LEAFLIST) {
        ly_print_(pctx->out, ",%s%*s", DO_FORMAT ? "\n" : "", INDENT);
    }

    if (node->schema->nodetype == LYS_LIST) {
        /* print list's content */
        LY_CHECK_RET(json_print_inner(pctx, node));
    } else {
        assert(node->schema->nodetype == LYS_LEAFLIST);

        LY_CHECK_RET(json_print_value(pctx, LYD_CTX(node), &((const struct lyd_node_term *)node)->value, node->schema->module));

        if (!pctx->first_leaflist) {
            if (((node->flags & LYD_DEFAULT) && (pctx->options & (LYD_PRINT_WD_ALL_TAG | LYD_PRINT_WD_IMPL_TAG))) ||
                    ((pctx->options & LYD_PRINT_WD_ALL_TAG) && lyd_is_default(node))) {
                /* we have implicit OR explicit default node, get with-defaults module */
                wdmod = ly_ctx_get_module_implemented(LYD_CTX(node), "ietf-netconf-with-defaults");
            }
            if (wdmod || node_has_printable_meta(node)) {
                /* we will be printing metadata for these siblings */
                pctx->first_leaflist = node;
            }
        }
    }

    if (json_print_array_is_last_inst(pctx, node)) {
        json_print_array_close(pctx);
    }

    return LY_SUCCESS;
}

/**
 * @brief Print leaf-list's metadata or opaque nodes attributes.
 * This function is supposed to be called when the leaf-list array is closed.
 *
 * @param[in] ctx JSON printer context.
 * @return LY_ERR value.
 */
static LY_ERR
json_print_meta_attr_leaflist(struct jsonpr_ctx *pctx)
{
    const struct lyd_node *prev, *node, *iter;
    const struct lys_module *wdmod = NULL, *iter_wdmod;
    const struct lyd_node_opaq *opaq = NULL;

    assert(pctx->first_leaflist);

    if (pctx->options & (LYD_PRINT_WD_ALL_TAG | LYD_PRINT_WD_IMPL_TAG)) {
        /* get with-defaults module */
        wdmod = ly_ctx_get_module_implemented(pctx->ctx, "ietf-netconf-with-defaults");
    }

    /* node is the first instance of the leaf-list */
    for (node = pctx->first_leaflist, prev = pctx->first_leaflist->prev;
            prev->next && matching_node(node, prev);
            node = prev, prev = node->prev) {}

    if (node->schema) {
        LY_CHECK_RET(json_print_member(pctx, node, 1));
    } else {
        opaq = (struct lyd_node_opaq *)node;
        LY_CHECK_RET(json_print_member2(pctx, lyd_parent(node), opaq->format, &opaq->name, 1));
    }

    ly_print_(pctx->out, "[%s", (DO_FORMAT ? "\n" : ""));
    LEVEL_INC;
    LY_LIST_FOR(node, iter) {
        PRINT_COMMA;
        if (iter->schema && ((iter->flags & LYD_DEFAULT) || ((pctx->options & LYD_PRINT_WD_ALL_TAG) && lyd_is_default(iter)))) {
            iter_wdmod = wdmod;
        } else {
            iter_wdmod = NULL;
        }
        if ((iter->schema && (node_has_printable_meta(iter) || iter_wdmod)) || (opaq && opaq->attr)) {
            ly_print_(pctx->out, "%*s%s", INDENT, DO_FORMAT ? "{\n" : "{");
            LEVEL_INC;

            if (iter->schema) {
                LY_CHECK_RET(json_print_metadata(pctx, iter, iter_wdmod));
            } else {
                LY_CHECK_RET(json_print_attribute(pctx, (struct lyd_node_opaq *)iter));
            }

            LEVEL_DEC;
            ly_print_(pctx->out, "%s%*s}", DO_FORMAT ? "\n" : "", INDENT);
        } else {
            ly_print_(pctx->out, "%*snull", INDENT);
        }
        LEVEL_PRINTED;
        if (!matching_node(iter, iter->next)) {
            break;
        }
    }
    LEVEL_DEC;
    ly_print_(pctx->out, "%s%*s]", DO_FORMAT ? "\n" : "", INDENT);
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
json_print_opaq(struct jsonpr_ctx *pctx, const struct lyd_node_opaq *node)
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
        LY_CHECK_RET(json_print_member2(pctx, pctx->parent, node->format, &node->name, 0));

        if (node->hints & (LYD_NODEHINT_LIST | LYD_NODEHINT_LEAFLIST)) {
            LY_CHECK_RET(json_print_array_open(pctx, &node->node));
        }
        if (node->hints & LYD_NODEHINT_LEAFLIST) {
            ly_print_(pctx->out, "%*s", INDENT);
        }
    } else if (node->hints & LYD_NODEHINT_LEAFLIST) {
        ly_print_(pctx->out, ",%s%*s", DO_FORMAT ? "\n" : "", INDENT);
    }
    if (node->child || (node->hints & LYD_NODEHINT_LIST)) {
        LY_CHECK_RET(json_print_inner(pctx, &node->node));
        LEVEL_PRINTED;
    } else {
        if (node->hints & LYD_VALHINT_EMPTY) {
            ly_print_(pctx->out, "[null]");
        } else if ((node->hints & (LYD_VALHINT_BOOLEAN | LYD_VALHINT_DECNUM)) && !(node->hints & LYD_VALHINT_NUM64)) {
            ly_print_(pctx->out, "%s", node->value);
        } else {
            /* string or a large number */
            json_print_string(pctx->out, node->value);
        }
        LEVEL_PRINTED;

        if (!(node->hints & LYD_NODEHINT_LEAFLIST)) {
            /* attributes */
            json_print_attributes(pctx, (const struct lyd_node *)node, 0);
        } else if (!pctx->first_leaflist && node->attr) {
            /* attributes printed later */
            pctx->first_leaflist = &node->node;
        }

    }
    if (last && (node->hints & (LYD_NODEHINT_LIST | LYD_NODEHINT_LEAFLIST))) {
        json_print_array_close(pctx);
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
json_print_node(struct jsonpr_ctx *pctx, const struct lyd_node *node)
{
    if (!lyd_node_should_print(node, pctx->options)) {
        if (json_print_array_is_last_inst(pctx, node)) {
            json_print_array_close(pctx);
        }
        return LY_SUCCESS;
    }

    if (!node->schema) {
        LY_CHECK_RET(json_print_opaq(pctx, (const struct lyd_node_opaq *)node));
    } else {
        switch (node->schema->nodetype) {
        case LYS_RPC:
        case LYS_ACTION:
        case LYS_NOTIF:
        case LYS_CONTAINER:
            LY_CHECK_RET(json_print_container(pctx, node));
            break;
        case LYS_LEAF:
            LY_CHECK_RET(json_print_leaf(pctx, node));
            break;
        case LYS_LEAFLIST:
        case LYS_LIST:
            LY_CHECK_RET(json_print_leaf_list(pctx, node));
            break;
        case LYS_ANYDATA:
        case LYS_ANYXML:
            LY_CHECK_RET(json_print_any(pctx, node));
            break;
        default:
            LOGINT(pctx->ctx);
            return EXIT_FAILURE;
        }
    }

    pctx->level_printed = pctx->level;

    if (pctx->first_leaflist && !matching_node(node->next, pctx->first_leaflist)) {
        json_print_meta_attr_leaflist(pctx);
        pctx->first_leaflist = NULL;
    }

    return LY_SUCCESS;
}

LY_ERR
json_print_data(struct ly_out *out, const struct lyd_node *root, uint32_t options)
{
    const struct lyd_node *node;
    struct jsonpr_ctx pctx = {0};
    const char *delimiter = (options & LYD_PRINT_SHRINK) ? "" : "\n";

    if (!root) {
        ly_print_(out, "{}%s", delimiter);
        ly_print_flush(out);
        return LY_SUCCESS;
    }

    pctx.out = out;
    pctx.parent = NULL;
    pctx.level = 1;
    pctx.level_printed = 0;
    pctx.options = options;
    pctx.ctx = LYD_CTX(root);

    /* start */
    ly_print_(pctx.out, "{%s", delimiter);

    /* content */
    LY_LIST_FOR(root, node) {
        pctx.root = node;
        LY_CHECK_RET(json_print_node(&pctx, node));
        if (!(options & LYD_PRINT_WITHSIBLINGS)) {
            break;
        }
    }

    /* end */
    ly_print_(out, "%s}%s", delimiter, delimiter);

    assert(!pctx.open.count);
    ly_set_erase(&pctx.open, NULL);

    ly_print_flush(out);
    return LY_SUCCESS;
}
