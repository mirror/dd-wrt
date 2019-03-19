/**
 * @file printer/json.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief JSON printer for libyang data structure
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#include "common.h"
#include "printer.h"
#include "tree_data.h"
#include "resolve.h"
#include "tree_internal.h"

#define INDENT ""
#define LEVEL (level*2)

static int json_print_nodes(struct lyout *out, int level, const struct lyd_node *root, int withsiblings, int toplevel,
                            int options);

int
json_print_string(struct lyout *out, const char *text)
{
    unsigned int i, n;

    if (!text) {
        return 0;
    }

    ly_write(out, "\"", 1);
    for (i = n = 0; text[i]; i++) {
        const unsigned char ascii = text[i];
        if (ascii < 0x20) {
            /* control character */
            n += ly_print(out, "\\u%.4X", ascii);
        } else {
            switch (ascii) {
            case '"':
                n += ly_print(out, "\\\"");
                break;
            case '\\':
                n += ly_print(out, "\\\\");
                break;
            default:
                ly_write(out, &text[i], 1);
                n++;
            }
        }
    }
    ly_write(out, "\"", 1);

    return n + 2;
}

static int
json_print_attrs(struct lyout *out, int level, const struct lyd_node *node, const struct lys_module *wdmod)
{
    struct lyd_attr *attr;
    size_t len;
    char *p;

    if (wdmod) {
        ly_print(out, "%*s\"%s:default\":\"true\"", LEVEL, INDENT, wdmod->name);
        ly_print(out, "%s%s", node->attr ? "," : "", (level ? "\n" : ""));
    }
    for (attr = node->attr; attr; attr = attr->next) {
        if (!attr->annotation) {
            /* skip exception for the NETCONF's attribute since JSON is not defined for NETCONF */
            continue;
        }
        if (lys_main_module(attr->annotation->module) != lys_main_module(node->schema->module)) {
            ly_print(out, "%*s\"%s:%s\":", LEVEL, INDENT, attr->annotation->module->name, attr->name);
        } else {
            ly_print(out, "%*s\"%s\":", LEVEL, INDENT, attr->name);
        }
        /* leafref is not supported */
        switch (attr->value_type) {
        case LY_TYPE_BINARY:
        case LY_TYPE_STRING:
        case LY_TYPE_BITS:
        case LY_TYPE_ENUM:
        case LY_TYPE_INST:
        case LY_TYPE_INT64:
        case LY_TYPE_UINT64:
        case LY_TYPE_DEC64:
            json_print_string(out, attr->value_str);
            break;

        case LY_TYPE_INT8:
        case LY_TYPE_INT16:
        case LY_TYPE_INT32:
        case LY_TYPE_UINT8:
        case LY_TYPE_UINT16:
        case LY_TYPE_UINT32:
        case LY_TYPE_BOOL:
            ly_print(out, "%s", attr->value_str[0] ? attr->value_str : "null");
            break;

        case LY_TYPE_IDENT:
            p = strchr(attr->value_str, ':');
            assert(p);
            len = p - attr->value_str;
            if (!strncmp(attr->value_str, attr->annotation->module->name, len)
                    && !attr->annotation->module->name[len]) {
                /* do not print the prefix, it is the default prefix for this node */
                json_print_string(out, ++p);
            } else {
                json_print_string(out, attr->value_str);
            }
            break;

        case LY_TYPE_EMPTY:
            ly_print(out, "[null]");
            break;

        default:
            /* error */
            ly_print(out, "\"(!error!)\"");
            return EXIT_FAILURE;
        }

        ly_print(out, "%s%s", attr->next ? "," : "", (level ? "\n" : ""));
    }

    return EXIT_SUCCESS;
}

static int
json_print_leaf(struct lyout *out, int level, const struct lyd_node *node, int onlyvalue, int toplevel, int options)
{
    struct lyd_node_leaf_list *leaf = (struct lyd_node_leaf_list *)node, *iter;
    const struct lys_type *type;
    const char *schema = NULL, *p, *mod_name;
    const struct lys_module *wdmod = NULL;
    LY_DATA_TYPE datatype;
    size_t len;

    if ((node->dflt && (options & (LYP_WD_ALL_TAG | LYP_WD_IMPL_TAG))) ||
            (!node->dflt && (options & LYP_WD_ALL_TAG) && lyd_wd_default(leaf))) {
        /* we have implicit OR explicit default node */
        /* get with-defaults module */
        wdmod = ly_ctx_get_module(node->schema->module->ctx, "ietf-netconf-with-defaults", NULL, 1);
    }

    if (!onlyvalue) {
        if (toplevel || !node->parent || nscmp(node, node->parent)) {
            /* print "namespace" */
            schema = lys_node_module(node->schema)->name;
            ly_print(out, "%*s\"%s:%s\":%s", LEVEL, INDENT, schema, node->schema->name, (level ? " " : ""));
        } else {
            ly_print(out, "%*s\"%s\":%s", LEVEL, INDENT, node->schema->name, (level ? " " : ""));
        }
    }

    datatype = leaf->value_type;
contentprint:
    switch (datatype) {
    case LY_TYPE_BINARY:
    case LY_TYPE_STRING:
    case LY_TYPE_BITS:
    case LY_TYPE_ENUM:
    case LY_TYPE_INST:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
    case LY_TYPE_UNION:
    case LY_TYPE_DEC64:
        json_print_string(out, leaf->value_str);
        break;

    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_BOOL:
        ly_print(out, "%s", leaf->value_str[0] ? leaf->value_str : "null");
        break;

    case LY_TYPE_IDENT:
        p = strchr(leaf->value_str, ':');
        assert(p);
        len = p - leaf->value_str;
        mod_name = leaf->schema->module->name;
        if (!strncmp(leaf->value_str, mod_name, len) && !mod_name[len]) {
            /* do not print the prefix, it is the default prefix for this node */
            json_print_string(out, ++p);
        } else {
            json_print_string(out, leaf->value_str);
        }
        break;

    case LY_TYPE_LEAFREF:
        iter = (struct lyd_node_leaf_list *)leaf->value.leafref;
        while (iter && (iter->value_type == LY_TYPE_LEAFREF)) {
            iter = (struct lyd_node_leaf_list *)iter->value.leafref;
        }
        if (!iter) {
            /* unresolved and invalid, but we can learn the correct type anyway */
            type = lyd_leaf_type((struct lyd_node_leaf_list *)leaf);
            if (!type) {
                /* error */
                ly_print(out, "\"(!error!)\"");
                return EXIT_FAILURE;
            }
            datatype = type->base;
        } else {
            datatype = iter->value_type;
        }
        goto contentprint;

    case LY_TYPE_EMPTY:
        ly_print(out, "[null]");
        break;

    default:
        /* error */
        ly_print(out, "\"(!error!)\"");
        return EXIT_FAILURE;
    }

    /* print attributes as sibling leafs */
    if (!onlyvalue && (node->attr || wdmod)) {
        if (schema) {
            ly_print(out, ",%s%*s\"@%s:%s\":%s{%s", (level ? "\n" : ""), LEVEL, INDENT, schema, node->schema->name,
                     (level ? " " : ""), (level ? "\n" : ""));
        } else {
            ly_print(out, ",%s%*s\"@%s\":%s{%s", (level ? "\n" : ""), LEVEL, INDENT, node->schema->name,
                     (level ? " " : ""), (level ? "\n" : ""));
        }
        if (json_print_attrs(out, (level ? level + 1 : level), node, wdmod)) {
            return EXIT_FAILURE;
        }
        ly_print(out, "%*s}", LEVEL, INDENT);
    }

    return EXIT_SUCCESS;
}

static int
json_print_container(struct lyout *out, int level, const struct lyd_node *node, int toplevel, int options)
{
    const char *schema;

    if (toplevel || !node->parent || nscmp(node, node->parent)) {
        /* print "namespace" */
        schema = lys_node_module(node->schema)->name;
        ly_print(out, "%*s\"%s:%s\":%s{%s", LEVEL, INDENT, schema, node->schema->name, (level ? " " : ""), (level ? "\n" : ""));
    } else {
        ly_print(out, "%*s\"%s\":%s{%s", LEVEL, INDENT, node->schema->name, (level ? " " : ""), (level ? "\n" : ""));
    }
    if (level) {
        level++;
    }
    if (node->attr) {
        ly_print(out, "%*s\"@\":%s{%s", LEVEL, INDENT, (level ? " " : ""), (level ? "\n" : ""));
        if (json_print_attrs(out, (level ? level + 1 : level), node, NULL)) {
            return EXIT_FAILURE;
        }
        ly_print(out, "%*s}", LEVEL, INDENT);
        if (node->child) {
            ly_print(out, ",%s", (level ? "\n" : ""));
        }
    }
    if (json_print_nodes(out, level, node->child, 1, 0, options)) {
        return EXIT_FAILURE;
    }
    if (level) {
        level--;
    }
    ly_print(out, "%*s}", LEVEL, INDENT);

    return EXIT_SUCCESS;
}

static int
json_print_leaf_list(struct lyout *out, int level, const struct lyd_node *node, int is_list, int toplevel, int options)
{
    const char *schema = NULL;
    const struct lyd_node *list = node;
    int flag_empty = 0, flag_attrs = 0;

    if (is_list && !list->child) {
        /* empty, e.g. in case of filter */
        flag_empty = 1;
    }

    if (toplevel || !node->parent || nscmp(node, node->parent)) {
        /* print "namespace" */
        schema = lys_node_module(node->schema)->name;
        ly_print(out, "%*s\"%s:%s\":", LEVEL, INDENT, schema, node->schema->name);
    } else {
        ly_print(out, "%*s\"%s\":", LEVEL, INDENT, node->schema->name);
    }

    if (flag_empty) {
        ly_print(out, "%snull", (level ? " " : ""));
        return EXIT_SUCCESS;
    }
    ly_print(out, "%s[%s", (level ? " " : ""), (level ? "\n" : ""));

    if (!is_list && level) {
        ++level;
    }

    while (list) {
        if (is_list) {
            /* list print */
            if (level) {
                ++level;
            }
            ly_print(out, "%*s{%s", LEVEL, INDENT, (level ? "\n" : ""));
            if (level) {
                ++level;
            }
            if (list->attr) {
                ly_print(out, "%*s\"@\":%s{%s", LEVEL, INDENT, (level ? " " : ""), (level ? "\n" : ""));
                if (json_print_attrs(out, (level ? level + 1 : level), list, NULL)) {
                    return EXIT_FAILURE;
                }
                if (list->child) {
                    ly_print(out, "%*s},%s", LEVEL, INDENT, (level ? "\n" : ""));
                } else {
                    ly_print(out, "%*s}", LEVEL, INDENT);
                }
            }
            if (json_print_nodes(out, level, list->child, 1, 0, options)) {
                return EXIT_FAILURE;
            }
            if (level) {
                --level;
            }
            ly_print(out, "%*s}", LEVEL, INDENT);
            if (level) {
                --level;
            }
        } else {
            /* leaf-list print */
            ly_print(out, "%*s", LEVEL, INDENT);
            if (json_print_leaf(out, level, list, 1, toplevel, options)) {
                return EXIT_FAILURE;
            }
            if (list->attr) {
                flag_attrs = 1;
            }
        }
        if (toplevel && !(options & LYP_WITHSIBLINGS)) {
            /* if initially called without LYP_WITHSIBLINGS do not print other list entries */
            break;
        }
        for (list = list->next; list && list->schema != node->schema; list = list->next);
        if (list) {
            ly_print(out, ",%s", (level ? "\n" : ""));
        }
    }

    if (!is_list && level) {
        --level;
    }

    ly_print(out, "%s%*s]", (level ? "\n" : ""), LEVEL, INDENT);

    /* attributes */
    if (!is_list && flag_attrs) {
        if (schema) {
            ly_print(out, ",%s%*s\"@%s:%s\":%s[%s", (level ? "\n" : ""), LEVEL, INDENT, schema, node->schema->name,
                     (level ? " " : ""), (level ? "\n" : ""));
        } else {
            ly_print(out, ",%s%*s\"@%s\":%s[%s", (level ? "\n" : ""), LEVEL, INDENT, node->schema->name,
                     (level ? " " : ""), (level ? "\n" : ""));
        }
        if (level) {
            level++;
        }
        for (list = node; list; ) {
            if (list->attr) {
                ly_print(out, "%*s{%s", LEVEL, INDENT, (level ? " " : ""));
                if (json_print_attrs(out, 0, list, NULL)) {
                    return EXIT_FAILURE;
                }
                ly_print(out, "%*s}", LEVEL, INDENT);
            } else {
                ly_print(out, "%*snull", LEVEL, INDENT);
            }


            for (list = list->next; list && list->schema != node->schema; list = list->next);
            if (list) {
                ly_print(out, ",%s", (level ? "\n" : ""));
            }
        }
        if (level) {
            level--;
        }
        ly_print(out, "%s%*s]", (level ? "\n" : ""), LEVEL, INDENT);
    }

    return EXIT_SUCCESS;
}

static int
json_print_anydataxml(struct lyout *out, int level, const struct lyd_node *node, int toplevel, int options)
{
    struct lyd_node_anydata *any = (struct lyd_node_anydata *)node;
    int is_object = 0;
    char *buf;
    const char *schema = NULL;

    if (toplevel || !node->parent || nscmp(node, node->parent)) {
        /* print "namespace" */
        schema = lys_node_module(node->schema)->name;
        ly_print(out, "%*s\"%s:%s\":", LEVEL, INDENT, schema, node->schema->name);
    } else {
        ly_print(out, "%*s\"%s\":", LEVEL, INDENT, node->schema->name);
    }
    if (level) {
        level++;
    }

    switch (any->value_type) {
    case LYD_ANYDATA_DATATREE:
        is_object = 1;
        ly_print(out, "%s{%s", (level ? " " : ""), (level ? "\n" : ""));
        /* do not print any default values nor empty containers */
        if (json_print_nodes(out, level, any->value.tree, 1, 0,  LYP_WITHSIBLINGS | (options & ~LYP_NETCONF))) {
            return EXIT_FAILURE;
        }
        break;
    case LYD_ANYDATA_JSON:
        if (level) {
            ly_print(out, "\n");
        }
        if (any->value.str) {
            ly_print(out, "%s", any->value.str);
        }
        if (level && (!any->value.str || (any->value.str[strlen(any->value.str) - 1] != '\n'))) {
            /* do not print 2 newlines */
            ly_print(out, "\n");
        }
        break;
    case LYD_ANYDATA_XML:
        lyxml_print_mem(&buf, any->value.xml, (level ? LYXML_PRINT_FORMAT | LYXML_PRINT_NO_LAST_NEWLINE : 0)
                                               | LYXML_PRINT_SIBLINGS);
        if (level) {
            ly_print(out, " ");
        }
        json_print_string(out, buf);
        free(buf);
        break;
    case LYD_ANYDATA_CONSTSTRING:
    case LYD_ANYDATA_SXML:
        if (level) {
            ly_print(out, " ");
        }
        if (any->value.str) {
            json_print_string(out, any->value.str);
        } else {
            ly_print(out, "\"\"");
        }
        break;
    case LYD_ANYDATA_STRING:
    case LYD_ANYDATA_SXMLD:
    case LYD_ANYDATA_JSOND:
    case LYD_ANYDATA_LYBD:
    case LYD_ANYDATA_LYB:
        /* other formats are not supported */
        json_print_string(out, "(error)");
        break;
    }

    /* print attributes as sibling leaf */
    if (node->attr) {
        if (schema) {
            ly_print(out, ",%s%*s\"@%s:%s\":%s{%s", (level ? "\n" : ""), LEVEL, INDENT, schema, node->schema->name,
                     (level ? " " : ""), (level ? "\n" : ""));
        } else {
            ly_print(out, ",%s%*s\"@%s\":%s{%s", (level ? "\n" : ""), LEVEL, INDENT, node->schema->name,
                     (level ? " " : ""), (level ? "\n" : ""));
        }
        if (json_print_attrs(out, (level ? level + 1 : level), node, NULL)) {
            return EXIT_FAILURE;
        }
        ly_print(out, "%*s}", LEVEL, INDENT);
    }

    if (level) {
        level--;
    }
    if (is_object) {
        ly_print(out, "%*s}", LEVEL, INDENT);
    }

    return EXIT_SUCCESS;
}

static int
json_print_nodes(struct lyout *out, int level, const struct lyd_node *root, int withsiblings, int toplevel, int options)
{
    int ret = EXIT_SUCCESS,comma_flag = 0;
    const struct lyd_node *node, *iter;

    LY_TREE_FOR(root, node) {
        if (!lyd_wd_toprint(node, options)) {
            /* wd says do not print */
            continue;
        }

        switch (node->schema->nodetype) {
        case LYS_RPC:
        case LYS_ACTION:
        case LYS_NOTIF:
        case LYS_CONTAINER:
            if (comma_flag) {
                /* print the previous comma */
                ly_print(out, ",%s", (level ? "\n" : ""));
            }
            ret = json_print_container(out, level, node, toplevel, options);
            break;
        case LYS_LEAF:
            if (comma_flag) {
                /* print the previous comma */
                ly_print(out, ",%s", (level ? "\n" : ""));
            }
            ret = json_print_leaf(out, level, node, 0, toplevel, options);
            break;
        case LYS_LEAFLIST:
        case LYS_LIST:
            /* is it already printed? (root node is not) */
            for (iter = node->prev; iter->next && node != root; iter = iter->prev) {
                if (iter == node) {
                    continue;
                }
                if (iter->schema == node->schema) {
                    /* the list has alread some previous instance and therefore it is already printed */
                    break;
                }
            }
            if (!iter->next || node == root) {
                if (comma_flag) {
                    /* print the previous comma */
                    ly_print(out, ",%s", (level ? "\n" : ""));
                }

                /* print the list/leaflist */
                ret = json_print_leaf_list(out, level, node, node->schema->nodetype == LYS_LIST ? 1 : 0, toplevel, options);
            }
            break;
        case LYS_ANYXML:
        case LYS_ANYDATA:
            if (comma_flag) {
                /* print the previous comma */
                ly_print(out, ",%s", (level ? "\n" : ""));
            }
            ret = json_print_anydataxml(out, level, node, toplevel, options);
            break;
        default:
            LOGINT(node->schema->module->ctx);
            ret = EXIT_FAILURE;
            break;
        }

        if (!withsiblings) {
            break;
        }
        comma_flag = 1;
    }
    if (root && level) {
        ly_print(out, "\n");
    }

    return ret;
}

int
json_print_data(struct lyout *out, const struct lyd_node *root, int options)
{
    const struct lyd_node *node, *next;
    int level = 0, action_input = 0;

    if (options & LYP_FORMAT) {
        ++level;
    }

    if (options & LYP_NETCONF) {
        if (root->schema->nodetype != LYS_RPC) {
            /* learn whether we are printing an action */
            LY_TREE_DFS_BEGIN(root, next, node) {
                if (node->schema->nodetype == LYS_ACTION) {
                    break;
                }
                LY_TREE_DFS_END(root, next, node);
            }
        } else {
            node = root;
        }

        if (node && (node->schema->nodetype & (LYS_RPC | LYS_ACTION))) {
            if (node->child && (node->child->schema->parent->nodetype == LYS_OUTPUT)) {
                /* skip the container */
                root = node->child;
            } else if (node->schema->nodetype == LYS_ACTION) {
                action_input = 1;
            }
        }
    }

    /* start */
    ly_print(out, "{%s", (level ? "\n" : ""));

    if (action_input) {
        ly_print(out, "%*s\"yang:action\":%s{%s", LEVEL, INDENT, (level ? " " : ""), (level ? "\n" : ""));
        if (level) {
            ++level;
        }
    }

    /* content */
    if (json_print_nodes(out, level, root, options & LYP_WITHSIBLINGS, 1, options)) {
        return EXIT_FAILURE;
    }

    if (action_input) {
        if (level) {
            --level;
        }
        ly_print(out, "%*s}%s", LEVEL, INDENT, (level ? "\n" : ""));
    }

    /* end */
    ly_print(out, "}%s", (level ? "\n" : ""));

    ly_print_flush(out);
    return EXIT_SUCCESS;
}
