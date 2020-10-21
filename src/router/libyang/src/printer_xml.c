/**
 * @file printer_xml.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief XML printer for libyang data structure
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#include "common.h"
#include "parser.h"
#include "printer.h"
#include "xml_internal.h"
#include "tree_data.h"
#include "tree_schema.h"
#include "resolve.h"
#include "tree_internal.h"

#define INDENT ""
#define LEVEL (level ? level*2-2 : 0)

struct mlist {
    struct mlist *next;
    struct lys_module *module;
    uint8_t printed;
};

static int
modlist_add(struct mlist **mlist, const struct lys_module *mod)
{
    struct mlist *iter;

    for (iter = *mlist; iter; iter = iter->next) {
        if (mod == iter->module) {
            break;
        }
    }

    if (!iter) {
        iter = malloc(sizeof *iter);
        LY_CHECK_ERR_RETURN(!iter, LOGMEM(mod->ctx), EXIT_FAILURE);
        iter->next = *mlist;
        iter->module = (struct lys_module *)mod;
        iter->printed = 0;
        *mlist = iter;
    }

    return EXIT_SUCCESS;
}

static void
free_mlist(struct mlist **mlist) {
    struct mlist *miter;
    while (*mlist) {
        miter = *mlist;
        *mlist = miter->next;
        free(miter);
    }
}

static void
xml_print_ns(struct lyout *out, const struct lyd_node *node, struct mlist **mlist, int options)
{
    struct lyd_node *next, *cur, *node2;
    struct lyd_attr *attr;
    const struct lys_module *wdmod = NULL;
    struct mlist *miter;
    int r;

    assert(out);
    assert(node);

    /* add node attribute modules */
    for (attr = node->attr; attr; attr = attr->next) {
        if (!strcmp(node->schema->name, "filter") &&
                (!strcmp(node->schema->module->name, "ietf-netconf") ||
                 !strcmp(node->schema->module->name, "notifications"))) {
            /* exception for NETCONF's filter attributes */
            continue;
        } else {
            r = modlist_add(mlist, lys_main_module(attr->annotation->module));
        }
        if (r) {
            goto print;
        }
    }

    /* add node children nodes and attribute modules */
    switch (node->schema->nodetype) {
    case LYS_LEAFLIST:
    case LYS_LEAF:
        if (node->dflt && (options & (LYP_WD_ALL_TAG | LYP_WD_IMPL_TAG))) {
            /* get with-defaults module and print its namespace */
            wdmod = ly_ctx_get_module(node->schema->module->ctx, "ietf-netconf-with-defaults", NULL, 1);
            if (wdmod && modlist_add(mlist, wdmod)) {
                goto print;
            }
        }
        break;
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_RPC:
    case LYS_ACTION:
    case LYS_NOTIF:
        if (options & (LYP_WD_ALL_TAG | LYP_WD_IMPL_TAG)) {
            /* get with-defaults module and print its namespace */
            wdmod = ly_ctx_get_module(node->schema->module->ctx, "ietf-netconf-with-defaults", NULL, 1);
            if (wdmod && modlist_add(mlist, wdmod)) {
                goto print;
            }
        }

        LY_TREE_FOR(node->child, node2) {
            LY_TREE_DFS_BEGIN(node2, next, cur) {
                for (attr = cur->attr; attr; attr = attr->next) {
                    if (!strcmp(cur->schema->name, "filter") &&
                            (!strcmp(cur->schema->module->name, "ietf-netconf") ||
                             !strcmp(cur->schema->module->name, "notifications"))) {
                        /* exception for NETCONF's filter attributes */
                        continue;
                    } else {
                        r = modlist_add(mlist, lys_main_module(attr->annotation->module));
                    }
                    if (r) {
                        goto print;
                    }
                }
            LY_TREE_DFS_END(node2, next, cur)}
        }
        break;
    default:
        break;
    }

print:
    /* print used namespaces */
    miter = *mlist;
    while (miter) {

        if (!miter->printed) {
            ly_print(out, " xmlns:%s=\"%s\"", miter->module->prefix, miter->module->ns);
            miter->printed = 1;
        }
        miter = miter->next;
    }
}

static int
xml_print_attrs(struct lyout *out, const struct lyd_node *node, int options)
{
    struct lyd_attr *attr;
    const char **prefs, **nss;
    const char *xml_expr = NULL, *mod_name;
    char *ident_val;
    uint32_t ns_count, i;
    int rpc_filter = 0;
    size_t len;
    const struct lys_module *wdmod = NULL;

    LY_PRINT_SET;

    /* with-defaults */
    if (node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
        if ((node->dflt && (options & (LYP_WD_ALL_TAG | LYP_WD_IMPL_TAG))) ||
                (!node->dflt && (options & LYP_WD_ALL_TAG) && lyd_wd_default((struct lyd_node_leaf_list *)node))) {
            /* we have implicit OR explicit default node */
            /* get with-defaults module */
            wdmod = ly_ctx_get_module(node->schema->module->ctx, "ietf-netconf-with-defaults", NULL, 1);
            if (wdmod) {
                /* print attribute only if context include with-defaults schema */
                ly_print(out, " %s:default=\"true\"", wdmod->prefix);
            }
        }
    }
    /* technically, check for the extension get-filter-element-attributes from ietf-netconf */
    if (!strcmp(node->schema->name, "filter")
            && (!strcmp(node->schema->module->name, "ietf-netconf") || !strcmp(node->schema->module->name, "notifications"))) {
        rpc_filter = 1;
    }

    for (attr = node->attr; attr; attr = attr->next) {
        ident_val = NULL;
        if (attr->value_str && ((attr->value_type == LY_TYPE_INST) || (attr->value_type == LY_TYPE_IDENT)
                || (!strcmp(attr->annotation->module->name, "yang") && !strcmp(attr->name, "key")))) {
            if (attr->value_type == LY_TYPE_IDENT) {
                ident_val = strchr(attr->value_str, ':');
                assert(ident_val);
                len = ident_val - attr->value_str;
                mod_name = attr->annotation->module->name;
                if (!strncmp(attr->value_str, mod_name, len) && !mod_name[len]) {
                    /* skip local identity prefix */
                    ++ident_val;
                    goto normal_print;
                }

                /* print the prefix correctly below */
                ident_val = NULL;
            }

            xml_expr = transform_json2xml(node->schema->module, attr->value_str, 1, &prefs, &nss, &ns_count);
            if (!xml_expr) {
                /* error */
                return EXIT_FAILURE;
            }

            for (i = 0; i < ns_count; ++i) {
                ly_print(out, " xmlns:%s=\"%s\"", prefs[i], nss[i]);
            }
            free(prefs);
            free(nss);
        }

normal_print:
        if (rpc_filter) {
            /* exception for NETCONF's filter's attributes */
            if (!strcmp(attr->name, "select")) {
                /* xpath content, we have to convert the JSON format into XML first */
                xml_expr = transform_json2xml(node->schema->module, attr->value_str, 0, &prefs, &nss, &ns_count);
                if (!xml_expr) {
                    /* error */
                    return EXIT_FAILURE;
                }

                for (i = 0; i < ns_count; ++i) {
                    ly_print(out, " xmlns:%s=\"%s\"", prefs[i], nss[i]);
                }
                free(prefs);
                free(nss);
            }
            ly_print(out, " %s=\"", attr->name);
        } else {
            ly_print(out, " %s:%s=\"", attr->annotation->module->prefix, attr->name);
        }

        switch (attr->value_type) {
        case LY_TYPE_BINARY:
        case LY_TYPE_STRING:
        case LY_TYPE_BITS:
        case LY_TYPE_ENUM:
        case LY_TYPE_BOOL:
        case LY_TYPE_DEC64:
        case LY_TYPE_INT8:
        case LY_TYPE_INT16:
        case LY_TYPE_INT32:
        case LY_TYPE_INT64:
        case LY_TYPE_UINT8:
        case LY_TYPE_UINT16:
        case LY_TYPE_UINT32:
        case LY_TYPE_UINT64:
        case LY_TYPE_INST:
            if (attr->value_str) {
                /* xml_expr can contain transformed xpath */
                lyxml_dump_text(out, xml_expr ? xml_expr : attr->value_str, LYXML_DATA_ATTR);
            }
            break;
        case LY_TYPE_IDENT:
            if (attr->value_str) {
                if (ident_val) {
                    lyxml_dump_text(out, ident_val, LYXML_DATA_ATTR);
                } else {
                    /* xml_expr can contain transformed xpath */
                    lyxml_dump_text(out, xml_expr ? xml_expr : attr->value_str, LYXML_DATA_ATTR);
                }
            }
            break;
        /* LY_TYPE_LEAFREF not allowed */
        case LY_TYPE_EMPTY:
            break;

        default:
            /* error */
            LOGINT(node->schema->module->ctx);
            return EXIT_FAILURE;
        }

        ly_print(out, "\"");

        if (xml_expr) {
            lydict_remove(node->schema->module->ctx, xml_expr);
            xml_expr = NULL;
        }
    }

    LY_PRINT_RET(node->schema->module->ctx);
}

static int
xml_print_leaf(struct lyout *out, int level, const struct lyd_node *node, int toplevel, int options)
{
    const struct lyd_node_leaf_list *leaf = (struct lyd_node_leaf_list *)node, *iter;
    const struct lys_type *type;
    struct lys_tpdf *tpdf;
    const char *ns, *mod_name;
    const char **prefs, **nss;
    const char *xml_expr;
    uint32_t ns_count, i;
    LY_DATA_TYPE datatype;
    char *p;
    size_t len;
    enum int_log_opts prev_ilo;
    struct mlist *mlist = NULL;

    LY_PRINT_SET;

    if (toplevel || !node->parent || nscmp(node, node->parent)) {
        /* print "namespace" */
        ns = lyd_node_module(node)->ns;
        ly_print(out, "%*s<%s xmlns=\"%s\"", LEVEL, INDENT, node->schema->name, ns);
    } else {
        ly_print(out, "%*s<%s", LEVEL, INDENT, node->schema->name);
    }

    if (toplevel) {
        xml_print_ns(out, node, &mlist, options);
        free_mlist(&mlist);
    }

    if (xml_print_attrs(out, node, options)) {
        return EXIT_FAILURE;
    }
    datatype = leaf->value_type;

printvalue:
    switch (datatype) {
    case LY_TYPE_STRING:
        ly_ilo_change(NULL, ILO_IGNORE, &prev_ilo, NULL);
        type = lyd_leaf_type((struct lyd_node_leaf_list *)leaf);
        ly_ilo_restore(NULL, prev_ilo, NULL, 0);
        if (type) {
            for (tpdf = type->der;
                tpdf->module && (strcmp(tpdf->name, "xpath1.0") || strcmp(tpdf->module->name, "ietf-yang-types"));
                tpdf = tpdf->type.der);
            /* special handling of ietf-yang-types xpath1.0 */
            if (tpdf->module) {
                /* avoid code duplication - use instance-identifier printer which gets necessary namespaces to print */
                datatype = LY_TYPE_INST;
                goto printvalue;
            }
        }
        /* fallthrough */
    case LY_TYPE_BINARY:
    case LY_TYPE_BITS:
    case LY_TYPE_ENUM:
    case LY_TYPE_BOOL:
    case LY_TYPE_UNION:
    case LY_TYPE_DEC64:
    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
        if (!leaf->value_str || !leaf->value_str[0]) {
            ly_print(out, "/>");
        } else {
            ly_print(out, ">");
            lyxml_dump_text(out, leaf->value_str, LYXML_DATA_ELEM);
            ly_print(out, "</%s>", node->schema->name);
        }
        break;

    case LY_TYPE_IDENT:
        if (!leaf->value_str || !leaf->value_str[0]) {
            ly_print(out, "/>");
            break;
        }
        p = strchr(leaf->value_str, ':');
        assert(p);
        len = p - leaf->value_str;
        mod_name = leaf->schema->module->name;
        if (!strncmp(leaf->value_str, mod_name, len) && !mod_name[len]) {
            ly_print(out, ">");
            lyxml_dump_text(out, ++p, LYXML_DATA_ELEM);
            ly_print(out, "</%s>", node->schema->name);
        } else {
            /* avoid code duplication - use instance-identifier printer which gets necessary namespaces to print */
            datatype = LY_TYPE_INST;
            goto printvalue;
        }
        break;
    case LY_TYPE_INST:
        xml_expr = transform_json2xml(node->schema->module, ((struct lyd_node_leaf_list *)node)->value_str, 1,
                                      &prefs, &nss, &ns_count);
        if (!xml_expr) {
            /* error */
            return EXIT_FAILURE;
        }

        for (i = 0; i < ns_count; ++i) {
            ly_print(out, " xmlns:%s=\"%s\"", prefs[i], nss[i]);
        }
        free(prefs);
        free(nss);

        if (xml_expr[0]) {
            ly_print(out, ">");
            lyxml_dump_text(out, xml_expr, LYXML_DATA_ELEM);
            ly_print(out, "</%s>", node->schema->name);
        } else {
            ly_print(out, "/>");
        }
        lydict_remove(node->schema->module->ctx, xml_expr);
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
                return EXIT_FAILURE;
            }
            datatype = type->base;
        } else {
            datatype = iter->value_type;
        }
        goto printvalue;

    case LY_TYPE_EMPTY:
    case LY_TYPE_UNKNOWN:
        /* treat <edit-config> node without value as empty */
        ly_print(out, "/>");
        break;

    default:
        /* error */
        LOGINT(node->schema->module->ctx);
        return EXIT_FAILURE;
    }

    if (level) {
        ly_print(out, "\n");
    }

    LY_PRINT_RET(node->schema->module->ctx);
}

static int
xml_print_container(struct lyout *out, int level, const struct lyd_node *node, int toplevel, int options)
{
    struct lyd_node *child;
    const char *ns;
    struct mlist *mlist = NULL;

    LY_PRINT_SET;

    if (toplevel || !node->parent || nscmp(node, node->parent)) {
        /* print "namespace" */
        ns = lyd_node_module(node)->ns;
        ly_print(out, "%*s<%s xmlns=\"%s\"", LEVEL, INDENT, node->schema->name, ns);
    } else {
        ly_print(out, "%*s<%s", LEVEL, INDENT, node->schema->name);
    }

    if (toplevel) {
        xml_print_ns(out, node, &mlist, options);
        free_mlist(&mlist);
    }

    if (xml_print_attrs(out, node, options)) {
        return EXIT_FAILURE;
    }

    if (!node->child) {
        ly_print(out, "/>%s", level ? "\n" : "");
        goto finish;
    }
    ly_print(out, ">%s", level ? "\n" : "");

    LY_TREE_FOR(node->child, child) {
        if (xml_print_node(out, level ? level + 1 : 0, child, 0, options)) {
            return EXIT_FAILURE;
        }
    }

    ly_print(out, "%*s</%s>%s", LEVEL, INDENT, node->schema->name, level ? "\n" : "");

finish:
    LY_PRINT_RET(node->schema->module->ctx);
}

static int
xml_print_list(struct lyout *out, int level, const struct lyd_node *node, int is_list, int toplevel, int options)
{
    struct lyd_node *child;
    const char *ns;
    struct mlist *mlist = NULL;

    LY_PRINT_SET;

    if (is_list) {
        /* list print */
        if (toplevel || !node->parent || nscmp(node, node->parent)) {
            /* print "namespace" */
            ns = lyd_node_module(node)->ns;
            ly_print(out, "%*s<%s xmlns=\"%s\"", LEVEL, INDENT, node->schema->name, ns);
        } else {
            ly_print(out, "%*s<%s", LEVEL, INDENT, node->schema->name);
        }

        if (toplevel) {
            xml_print_ns(out, node, &mlist, options);
            free_mlist(&mlist);
        }
        if (xml_print_attrs(out, node, options)) {
            return EXIT_FAILURE;
        }

        if (!node->child) {
            ly_print(out, "/>%s", level ? "\n" : "");
            goto finish;
        }
        ly_print(out, ">%s", level ? "\n" : "");

        LY_TREE_FOR(node->child, child) {
            if (xml_print_node(out, level ? level + 1 : 0, child, 0, options)) {
                return EXIT_FAILURE;
            }
        }

        ly_print(out, "%*s</%s>%s", LEVEL, INDENT, node->schema->name, level ? "\n" : "");
    } else {
        /* leaf-list print */
        xml_print_leaf(out, level, node, toplevel, options);
    }

finish:
    LY_PRINT_RET(node->schema->module->ctx);
}

static int
xml_print_anydata(struct lyout *out, int level, const struct lyd_node *node, int toplevel, int options)
{
    char *buf;
    struct lyd_node_anydata *any = (struct lyd_node_anydata *)node;
    struct lyd_node *iter;
    const char *ns;
    struct mlist *mlist = NULL;

    LY_PRINT_SET;

    if (toplevel || !node->parent || nscmp(node, node->parent)) {
        /* print "namespace" */
        ns = lyd_node_module(node)->ns;
        ly_print(out, "%*s<%s xmlns=\"%s\"", LEVEL, INDENT, node->schema->name, ns);
    } else {
        ly_print(out, "%*s<%s", LEVEL, INDENT, node->schema->name);
    }

    if (toplevel) {
        xml_print_ns(out, node, &mlist, options);
    }
    if (xml_print_attrs(out, node, options)) {
        return EXIT_FAILURE;
    }
    if (!(void*)any->value.tree || (any->value_type == LYD_ANYDATA_CONSTSTRING && !any->value.str[0])) {
        /* no content */
        ly_print(out, "/>%s", level ? "\n" : "");
    } else {
        if (any->value_type == LYD_ANYDATA_LYB) {
            /* parse into a data tree */
            ly_errno = 0;
            iter = lyd_parse_mem(node->schema->module->ctx, any->value.mem, LYD_LYB, LYD_OPT_DATA | LYD_OPT_STRICT
                                 | LYD_OPT_TRUSTED);
            if (!ly_errno) {
                /* successfully parsed */
                free(any->value.mem);
                any->value_type = LYD_ANYDATA_DATATREE;
                any->value.tree = iter;
            }
        }
        if (any->value_type == LYD_ANYDATA_DATATREE) {
            /* print namespaces in the anydata data tree */
            LY_TREE_FOR(any->value.tree, iter) {
                xml_print_ns(out, iter, &mlist, options);
            }
        }
        /* close opening tag ... */
        ly_print(out, ">");
        free_mlist(&mlist);
        /* ... and print anydata content */
        switch (any->value_type) {
        case LYD_ANYDATA_CONSTSTRING:
            lyxml_dump_text(out, any->value.str, LYXML_DATA_ELEM);
            break;
        case LYD_ANYDATA_DATATREE:
            if (any->value.tree) {
                if (level) {
                    ly_print(out, "\n");
                }
                LY_TREE_FOR(any->value.tree, iter) {
                    if (xml_print_node(out, level ? level + 1 : 0, iter, 0, (options & ~(LYP_WITHSIBLINGS | LYP_NETCONF)))) {
                        return EXIT_FAILURE;
                    }
                }
            }
            break;
        case LYD_ANYDATA_XML:
            lyxml_print_mem(&buf, any->value.xml, (level ? LYXML_PRINT_FORMAT | LYXML_PRINT_NO_LAST_NEWLINE : 0)
                                                   | LYXML_PRINT_SIBLINGS);
            ly_print(out, "%s%s", level ? "\n" : "", buf);
            free(buf);
            break;
        case LYD_ANYDATA_SXML:
            /* print without escaping special characters */
            ly_print(out, "%s", any->value.str);
            break;
        case LYD_ANYDATA_JSON:
        case LYD_ANYDATA_LYB:
            /* JSON format is not supported (LYB failed to be converted) */
            LOGWRN(node->schema->module->ctx, "Unable to print anydata content (type %d) as XML.", any->value_type);
            break;
        case LYD_ANYDATA_STRING:
        case LYD_ANYDATA_SXMLD:
        case LYD_ANYDATA_JSOND:
        case LYD_ANYDATA_LYBD:
            /* dynamic strings are used only as input parameters */
            assert(0);
            break;
        }

        /* closing tag */
        ly_print(out, "</%s>%s", node->schema->name, level ? "\n" : "");
    }

    LY_PRINT_RET(node->schema->module->ctx);
}

int
xml_print_node(struct lyout *out, int level, const struct lyd_node *node, int toplevel, int options)
{
    int ret = EXIT_SUCCESS;

    if (!lyd_node_should_print(node, options)) {
        /* wd says do not print */
        return EXIT_SUCCESS;
    }

    switch (node->schema->nodetype) {
    case LYS_NOTIF:
    case LYS_RPC:
    case LYS_ACTION:
    case LYS_CONTAINER:
        ret = xml_print_container(out, level, node, toplevel, options);
        break;
    case LYS_LEAF:
        ret = xml_print_leaf(out, level, node, toplevel, options);
        break;
    case LYS_LEAFLIST:
        ret = xml_print_list(out, level, node, 0, toplevel, options);
        break;
    case LYS_LIST:
        ret = xml_print_list(out, level, node, 1, toplevel, options);
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        ret = xml_print_anydata(out, level, node, toplevel, options);
        break;
    default:
        LOGINT(node->schema->module->ctx);
        ret = EXIT_FAILURE;
        break;
    }

    return ret;
}

int
xml_print_data(struct lyout *out, const struct lyd_node *root, int options)
{
    const struct lyd_node *node, *next;
    struct lys_node *parent = NULL;
    int level, action_input = 0;

    LY_PRINT_SET;

    if (!root) {
        if (out->type == LYOUT_MEMORY || out->type == LYOUT_CALLBACK) {
            ly_print(out, "");
        }
        goto finish;
    }

    level = (options & LYP_FORMAT ? 1 : 0);

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

        if (node) {
            if ((node->schema->nodetype & (LYS_LIST | LYS_CONTAINER | LYS_RPC | LYS_NOTIF | LYS_ACTION)) && node->child) {
                for (parent = lys_parent(node->child->schema); parent && (parent->nodetype == LYS_USES); parent = lys_parent(parent));
            }
            if (parent && (parent->nodetype == LYS_OUTPUT)) {
                /* rpc/action output - skip the container */
                root = node->child;
            } else if (node->schema->nodetype == LYS_ACTION) {
                /* action input - print top-level action element */
                action_input = 1;
            }
        }
    }

    if (action_input) {
        ly_print(out, "%*s<action xmlns=\"%s\">%s", LEVEL, INDENT, LY_NSYANG, level ? "\n" : "");
        if (level) {
            ++level;
        }
    }

    /* content */
    LY_TREE_FOR(root, node) {
        if (xml_print_node(out, level, node, 1, options)) {
            return EXIT_FAILURE;
        }
        if (!(options & LYP_WITHSIBLINGS)) {
            break;
        }
    }

    if (action_input) {
        if (level) {
            --level;
        }
        ly_print(out, "%*s</action>%s", LEVEL, INDENT, level ? "\n" : "");
    }

finish:
    ly_print_flush(out);

    LY_PRINT_RET(NULL);
}

