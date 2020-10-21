/**
 * @file printer/tree.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief TREE printer for libyang data model structure
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "printer.h"
#include "tree_schema.h"

/* module: <name>
 * <X>+--rw <node-name> */
#define LY_TREE_MOD_DATA_INDENT 2

/* <^>rpcs:
 * <X>+---x <rpc-name> */
#define LY_TREE_OP_DATA_INDENT 4

/* +--rw leaf<X>string */
#define LY_TREE_TYPE_INDENT 3

/* +--rw leaf
 * |     <X>string */
#define LY_TREE_WRAP_INDENT 2

/* these options are mostly inherited in recursive print, non-recursive options are parameters */
typedef struct {
    const struct lys_module *module; /**< (sub)module we are printing from */
    uint8_t base_indent;             /**< base indent size of all the printed text */
    uint64_t indent;                 /**< bit-field of sibling (1)/ no sibling(0) on corresponding depths */
    uint16_t line_length;            /**< maximum desired line length */
    int spec_config;                 /**< special config flags - 0 (no special config status),
                                          1 (read-only - rpc output, notification), 2 (write-only - rpc input) */
    int options;                     /**< user-specified tree printer options */
} tp_opts;

static void tree_print_snode(struct lyout *out, int level, uint16_t max_name_len, const struct lys_node *node, int mask,
                             const struct lys_node *aug_parent, int subtree, tp_opts *opts);

static int
tree_print_indent(struct lyout *out, int level, tp_opts *opts)
{
    int i, ret = 0;

    if (opts->base_indent) {
        ret += ly_print(out, "%*s", opts->base_indent, " ");
    }
    for (i = 0; i < level; ++i) {
        if (opts->indent & (1 << i)) {
            ret += ly_print(out, "|  ");
        } else {
            ret += ly_print(out, "   ");
        }
    }

    return ret;
}

static int
tree_sibling_is_valid_child(const struct lys_node *node, int including, const struct lys_module *module,
                            const struct lys_node *aug_parent, LYS_NODE nodetype)
{
    struct lys_node *cur, *cur2;

    assert(!aug_parent || (aug_parent->nodetype == LYS_AUGMENT));

    if (!node) {
        return 0;
    } else if (!lys_parent(node) && !strcmp(node->name, "config") && !strcmp(node->module->name, "ietf-netconf")) {
        /* node added by libyang, not actually in the model */
        return 0;
    }

    /* has a following printed child */
    LY_TREE_FOR((struct lys_node *)(including ? node : node->next), cur) {
        if (aug_parent && (cur->parent != aug_parent)) {
            /* we are done traversing this augment, the nodes are all direct siblings */
            return 0;
        }

        if (module->type && (lys_main_module(module) != lys_node_module(cur))) {
            continue;
        }

        if (!lys_is_disabled(cur, 0)) {
            if ((cur->nodetype == LYS_USES) || ((cur->nodetype == LYS_CASE) && (cur->flags & LYS_IMPLICIT))) {
                if (tree_sibling_is_valid_child(cur->child, 1, module, NULL, nodetype)) {
                    return 1;
                }
            } else {
                switch (nodetype) {
                case LYS_GROUPING:
                    /* we are printing groupings, they are printed separately */
                    if (cur->nodetype == LYS_GROUPING) {
                        return 0;
                    }
                    break;
                case LYS_RPC:
                    if (cur->nodetype == LYS_RPC) {
                        return 1;
                    }
                    break;
                case LYS_NOTIF:
                    if (cur->nodetype == LYS_NOTIF) {
                        return 1;
                    }
                    break;
                default:
                    if (cur->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_CHOICE
                            | LYS_CASE | LYS_ACTION)) {
                        return 1;
                    }
                    if ((cur->nodetype & (LYS_INPUT | LYS_OUTPUT)) && cur->child) {
                        return 1;
                    }
                    /* only nested notifications count here (not top-level) */
                    if (cur->nodetype == LYS_NOTIF) {
                        for (cur2 = lys_parent(cur); cur2 && (cur2->nodetype == LYS_USES); cur2 = lys_parent(cur2));
                        if (cur2) {
                            return 1;
                        }
                    }
                    break;
                }
            }
        }
    }

    /* if in uses, the following printed child can actually be in the parent node :-/ */
    if (lys_parent(node) && (lys_parent(node)->nodetype == LYS_USES)) {
        return tree_sibling_is_valid_child(lys_parent(node), 0, module, NULL, nodetype);
    }

    return 0;
}

static void
tree_next_indent(int level, const struct lys_node *node, const struct lys_node *aug_parent, tp_opts *opts)
{
    int next_is_case = 0, has_next = 0;

    if (level > 64) {
        LOGINT(node->module->ctx);
        return;
    }

    /* clear level indent (it may have been set for some line wrapping) */
    opts->indent &= ~(uint64_t)(1ULL << (level - 1));

    /* this is the direct child of a case */
    if ((node->nodetype != LYS_CASE) && lys_parent(node) && (lys_parent(node)->nodetype & (LYS_CASE | LYS_CHOICE))) {
        /* it is not the only child */
        if (node->next && lys_parent(node->next) && (lys_parent(node->next)->nodetype == LYS_CHOICE)) {
            next_is_case = 1;
        }
    }

    /* next is a node that will actually be printed */
    has_next = tree_sibling_is_valid_child(node, 0, opts->module, aug_parent, node->nodetype);

    /* set level indent */
    if (has_next && !next_is_case) {
        opts->indent |= (uint64_t)1ULL << (level - 1);
    }
}

static uint16_t
tree_get_max_name_len(const struct lys_node *sibling, const struct lys_node *aug_parent, int type_mask,
                      tp_opts *opts)
{
    const struct lys_node *sub;
    struct lys_module *nodemod;
    unsigned int max_name_len = 0, name_len;

    LY_TREE_FOR(sibling, sub) {
        if (opts->module->type && (sub->module != opts->module)) {
            /* when printing submodule, we are only concerned with its own data (they are in the module data) */
            continue;
        }
        if (aug_parent && (sub->parent != aug_parent)) {
            /* when printing augment children, skip other target children */
            continue;
        }
        if (!(sub->nodetype & type_mask)) {
            /* this sibling will not be printed */
            continue;
        }

        if ((sub->nodetype == LYS_USES) && !(opts->options & LYS_OUTOPT_TREE_USES)) {
            name_len = tree_get_max_name_len(sub->child, NULL, type_mask, opts);
        } else {
            nodemod = lys_node_module(sub);
            name_len = strlen(sub->name);
            if (lys_main_module(opts->module) != nodemod) {
                /* ":" */
                ++name_len;
                if (opts->options & LYS_OUTOPT_TREE_RFC) {
                    name_len += strlen(nodemod->prefix);
                } else {
                    name_len += strlen(nodemod->name);
                }
            }

            /* add characters for optional opts */
            switch (sub->nodetype) {
            case LYS_LEAF:
            case LYS_LEAFLIST:
            case LYS_LIST:
            case LYS_ANYDATA:
            case LYS_ANYXML:
            case LYS_CONTAINER:
            case LYS_CASE:
                ++name_len;
                break;
            case LYS_CHOICE:
                /* choice is longer :-/ */
                name_len += 2;
                if (!(sub->flags & LYS_MAND_TRUE)) {
                    ++name_len;
                }
                break;
            default:
                break;
            }
        }

        if (name_len > max_name_len) {
            max_name_len = name_len;
        }
    }

    return max_name_len;
}

static int
tree_leaf_is_mandatory(const struct lys_node *node)
{
    const struct lys_node *parent;
    struct lys_node_list *list;
    uint16_t i;

    for (parent = lys_parent(node); parent && parent->nodetype == LYS_USES; parent = lys_parent(parent));
    if (parent && parent->nodetype == LYS_LIST) {
        list = (struct lys_node_list *)parent;
        for (i = 0; i < list->keys_size; i++) {
            if (list->keys[i] == (struct lys_node_leaf *)node) {
                return 1;
            }
        }
    }

    return 0;
}

static int
tree_print_wrap(struct lyout *out, int level, int line_printed, uint8_t indent, uint16_t len, tp_opts *opts)
{
    if (opts->line_length && (line_printed + indent + len > opts->line_length)) {
        ly_print(out, "\n");
        line_printed = tree_print_indent(out, level, opts);
        /* 3 for config + space */
        line_printed += ly_print(out, "%*s", 3 + LY_TREE_WRAP_INDENT, "");
    } else {
        line_printed += ly_print(out, "%*s", indent, "");
    }

    return line_printed;
}

static int
tree_print_prefix(struct lyout *out, const struct lys_node *node, tp_opts *opts)
{
    uint16_t ret = 0;
    const struct lys_module *nodemod;

    nodemod = lys_node_module(node);
    if (lys_main_module(opts->module) != nodemod) {
        if (opts->options & LYS_OUTOPT_TREE_RFC) {
            ret = ly_print(out, "%s:", nodemod->prefix);
        } else {
            ret = ly_print(out, "%s:", nodemod->name);
        }
    }

    return ret;
}

static int
tree_print_type(struct lyout *out, const struct lys_type *type, int options, const char **out_str)
{
    struct lys_module *type_mod = ((struct lys_tpdf *)type->parent)->module;
    const char *str;
    char *tmp;
    int printed;

    if ((type->base == LY_TYPE_LEAFREF) && !type->der->module) {
        if (options & LYS_OUTOPT_TREE_NO_LEAFREF) {
            if (out_str) {
                printed = 7;
                *out_str = lydict_insert(type_mod->ctx, "leafref", printed);
            } else {
                printed = ly_print(out, "leafref");
            }
        } else {
            if (options & LYS_OUTOPT_TREE_RFC) {
                str = transform_json2schema(type_mod, type->info.lref.path);
                if (out_str) {
                    printed = 3 + strlen(str);
                    tmp = malloc(printed + 1);
                    LY_CHECK_ERR_RETURN(!tmp, LOGMEM(type_mod->ctx), 0);
                    sprintf(tmp, "-> %s", str);
                    *out_str = lydict_insert_zc(type_mod->ctx, tmp);
                } else {
                    printed = ly_print(out, "-> %s", str);
                }
                lydict_remove(type_mod->ctx, str);
            } else {
                if (out_str) {
                    printed = 3 + strlen(type->info.lref.path);
                    tmp = malloc(printed + 1);
                    LY_CHECK_ERR_RETURN(!tmp, LOGMEM(type_mod->ctx), 0);
                    sprintf(tmp, "-> %s", type->info.lref.path);
                    *out_str = lydict_insert_zc(type_mod->ctx, tmp);
                } else {
                    printed = ly_print(out, "-> %s", type->info.lref.path);
                }
            }
        }
    } else if (!lys_type_is_local(type)) {
        if (options & LYS_OUTOPT_TREE_RFC) {
            str = transform_module_name2import_prefix(type_mod, type->der->module->name);
            if (out_str) {
                printed = strlen(str) + 1 + strlen(type->der->name);
                tmp = malloc(printed + 1);
                LY_CHECK_ERR_RETURN(!tmp, LOGMEM(type_mod->ctx), 0);
                sprintf(tmp, "%s:%s", str, type->der->name);
                *out_str = lydict_insert_zc(type_mod->ctx, tmp);
            } else {
                printed = ly_print(out, "%s:%s", str, type->der->name);
            }
        } else {
            if (out_str) {
                printed = strlen(type->der->module->name) + 1 + strlen(type->der->name);
                tmp = malloc(printed + 1);
                LY_CHECK_ERR_RETURN(!tmp, LOGMEM(type_mod->ctx), 0);
                sprintf(tmp, "%s:%s", type->der->module->name, type->der->name);
                *out_str = lydict_insert_zc(type_mod->ctx, tmp);
            } else {
                printed = ly_print(out, "%s:%s", type->der->module->name, type->der->name);
            }
        }
    } else {
        if (out_str) {
            printed = strlen(type->der->name);
            *out_str = lydict_insert(type_mod->ctx, type->der->name, printed);
        } else {
            printed = ly_print(out, "%s", type->der->name);
        }
    }

    return printed;
}

static int
tree_print_config(struct lyout *out, const struct lys_node *node, int spec_config)
{
    int ret;

    switch (node->nodetype) {
    case LYS_RPC:
    case LYS_ACTION:
        return ly_print(out, "-x ");
    case LYS_NOTIF:
        return ly_print(out, "-n ");
    case LYS_USES:
        return ly_print(out, "-u ");
    case LYS_CASE:
        return ly_print(out, ":(");
    default:
        break;
    }

    if (spec_config == 1) {
        ret = ly_print(out, "-w ");
    } else if (spec_config == 2) {
        ret = ly_print(out, "ro ");
    } else {
        ret = ly_print(out, "%s ", (node->flags & LYS_CONFIG_W) ? "rw" : (node->flags & LYS_CONFIG_R) ? "ro" : "--");
    }

    if (node->nodetype == LYS_CHOICE) {
        ret += ly_print(out, "(");
    }
    return ret;
}

static int
tree_print_features(struct lyout *out, struct lys_iffeature *iff1, uint8_t iff1_size, struct lys_iffeature *iff2,
                    uint8_t iff2_size, tp_opts *opts, const char **out_str)
{
    int i, printed;
    struct lyout *o;

    if (!iff1_size && !iff2_size) {
        return 0;
    }

    if (out_str) {
        o = malloc(sizeof *o);
        LY_CHECK_ERR_RETURN(!o, LOGMEM(NULL), 0);
        o->type = LYOUT_MEMORY;
        o->method.mem.buf = NULL;
        o->method.mem.len = 0;
        o->method.mem.size = 0;
    } else {
        o = out;
    }

    printed = ly_print(o, "{");
    for (i = 0; i < iff1_size; i++) {
        if (i > 0) {
            printed += ly_print(o, ",");
        }
        printed += ly_print_iffeature(o, opts->module, &iff1[i], opts->options & LYS_OUTOPT_TREE_RFC ? 2 : 1);
    }
    for (i = 0; i < iff2_size; i++) {
        if (i > 0) {
            printed += ly_print(o, ",");
        }
        printed += ly_print_iffeature(o, opts->module, &iff2[i], opts->options & LYS_OUTOPT_TREE_RFC ? 2 : 1);
    }
    printed += ly_print(o, "}?");

    if (out_str) {
        *out_str = lydict_insert_zc(opts->module->ctx, o->method.mem.buf);
        free(o);
    }

    return printed;
}

static int
tree_print_keys(struct lyout *out, struct lys_node_leaf **keys, uint8_t keys_size, tp_opts *opts, const char **out_str)
{
    int i, printed;
    struct lyout *o;

    if (!keys_size) {
        return 0;
    }

    if (out_str) {
        o = malloc(sizeof *o);
        LY_CHECK_ERR_RETURN(!o, LOGMEM(NULL), 0);
        o->type = LYOUT_MEMORY;
        o->method.mem.buf = NULL;
        o->method.mem.len = 0;
        o->method.mem.size = 0;
    } else {
        o = out;
    }

    printed = ly_print(o, "[");
    for (i = 0; i < keys_size; i++) {
        printed += ly_print(o, "%s%s", keys[i]->name, i + 1 < keys_size ? " " : "]");
    }

    if (out_str) {
        *out_str = lydict_insert_zc(opts->module->ctx, o->method.mem.buf);
        free(o);
    }

    return printed;
}

/**
 * @brief Print schema node in YANG tree diagram formatting.
 *
 * @param[in] out libyang output.
 * @param[in] level Current level of depth.
 * @param[in] max_name_len Maximal name length of all the siblings (relevant only for nodes with type).
 * @param[in] node Schema node to print.
 * @param[in] mask Type mask of children nodes to be printed.
 * @param[in] aug_parent Augment node parent in case we are printing its direct children.
 * @param[in] opts Tree printer options structure.
 */
static void
tree_print_snode(struct lyout *out, int level, uint16_t max_name_len, const struct lys_node *node, int mask,
                 const struct lys_node *aug_parent, int subtree, tp_opts *opts)
{
    struct lys_node *sub;
    int line_len, node_len, child_mask;
    uint8_t text_len, text_indent;
    uint16_t max_child_len;
    const char *text_str;

    /* disabled/not printed node */
    if (lys_is_disabled(node, (node->parent && node->parent->nodetype == LYS_AUGMENT) ? 1 : 0) || !(node->nodetype & mask)) {
        return;
    }

    /* implicit input/output/case */
    if (((node->nodetype & mask) & (LYS_INPUT | LYS_OUTPUT | LYS_CASE)) && (node->flags & LYS_IMPLICIT)) {
        if ((node->nodetype != LYS_CASE) || lys_is_disabled(node->child, 0)) {
            return;
        }
    }

    /* special uses and grouping handling */
    switch (node->nodetype & mask) {
    case LYS_USES:
        if (opts->options & LYS_OUTOPT_TREE_USES) {
            break;
        }
        /* fallthrough */
    case LYS_GROUPING:
        goto print_children;
    case LYS_ANYXML:
        if (!lys_parent(node) && !strcmp(node->name, "config") && !strcmp(node->module->name, "ietf-netconf")) {
            /* node added by libyang, not actually in the model */
            return;
        }
        break;
    default:
        break;
    }

    /* print indent */
    line_len = tree_print_indent(out, level, opts);
    /* print status */
    line_len += ly_print(out, "%s--", (node->flags & LYS_STATUS_DEPRC ? "x" : (node->flags & LYS_STATUS_OBSLT ? "o" : "+")));
    /* print config flags (or special opening for case, choice) */
    line_len += tree_print_config(out, node, opts->spec_config);
    /* print optionally prefix */
    node_len = tree_print_prefix(out, node, opts);
    /* print name */
    node_len += ly_print(out, node->name);

    /* print one-character opts */
    switch (node->nodetype & mask) {
    case LYS_LEAF:
        if (!(node->flags & LYS_MAND_TRUE) && !tree_leaf_is_mandatory(node)) {
            node_len += ly_print(out, "?");
        }
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        if (!(node->flags & LYS_MAND_TRUE)) {
            node_len += ly_print(out, "?");
        }
        break;
    case LYS_CONTAINER:
        if (((struct lys_node_container *)node)->presence) {
            node_len += ly_print(out, "!");
        }
        break;
    case LYS_LIST:
    case LYS_LEAFLIST:
        node_len += ly_print(out, "*");
        break;
    case LYS_CASE:
        /* kinda shady, but consistent in a way */
        node_len += ly_print(out, ")");
        break;
    case LYS_CHOICE:
        node_len += ly_print(out, ")");
        if (!(node->flags & LYS_MAND_TRUE)) {
            node_len += ly_print(out, "?");
        }
        break;
    default:
        break;
    }
    line_len += node_len;

    /**
     * wrapped print
     */

    /* learn next level indent (there is never a sibling for subtree) */
    ++level;
    if (!subtree) {
        tree_next_indent(level, node, aug_parent, opts);
    }

    /* print type/keys */
    switch (node->nodetype & mask) {
    case LYS_LEAF:
    case LYS_LEAFLIST:
        assert(max_name_len);
        text_indent = LY_TREE_TYPE_INDENT + (uint8_t)(max_name_len - node_len);
        text_len = tree_print_type(out, &((struct lys_node_leaf *)node)->type, opts->options, &text_str);
        line_len = tree_print_wrap(out, level, line_len, text_indent, text_len, opts);
        line_len += ly_print(out, text_str);
        lydict_remove(opts->module->ctx, text_str);
        break;
    case LYS_ANYDATA:
        assert(max_name_len);
        text_indent = LY_TREE_TYPE_INDENT + (uint8_t)(max_name_len - node_len);
        line_len = tree_print_wrap(out, level, line_len, text_indent, 7, opts);
        line_len += ly_print(out, "anydata");
        break;
    case LYS_ANYXML:
        assert(max_name_len);
        text_indent = LY_TREE_TYPE_INDENT + (uint8_t)(max_name_len - node_len);
        line_len = tree_print_wrap(out, level, line_len, text_indent, 6, opts);
        line_len += ly_print(out, "anyxml");
        break;
    case LYS_LIST:
        text_len = tree_print_keys(out, ((struct lys_node_list *)node)->keys, ((struct lys_node_list *)node)->keys_size,
                                   opts, &text_str);
        if (text_len) {
            line_len = tree_print_wrap(out, level, line_len, 1, text_len, opts);
            line_len += ly_print(out, text_str);
            lydict_remove(opts->module->ctx, text_str);
        }
        break;
    default:
        break;
    }

    /* print default */
    if (!(opts->options & LYS_OUTOPT_TREE_RFC)) {
        switch (node->nodetype & mask) {
        case LYS_LEAF:
            text_str = ((struct lys_node_leaf *)node)->dflt;
            if (text_str) {
                line_len = tree_print_wrap(out, level, line_len, 1, 2 + strlen(text_str), opts);
                line_len += ly_print(out, "<%s>", text_str);
            }
            break;
        case LYS_CHOICE:
            sub = ((struct lys_node_choice *)node)->dflt;
            if (sub) {
                line_len = tree_print_wrap(out, level, line_len, 1, 2 + strlen(sub->name), opts);
                line_len += ly_print(out, "<%s>", sub->name);
            }
            break;
        default:
            break;
        }
    }

    /* print if-features */
    switch (node->nodetype & mask) {
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_CHOICE:
    case LYS_CASE:
    case LYS_ANYDATA:
    case LYS_ANYXML:
    case LYS_LEAF:
    case LYS_LEAFLIST:
    case LYS_RPC:
    case LYS_ACTION:
    case LYS_NOTIF:
    case LYS_USES:
        if (node->parent && (node->parent->nodetype == LYS_AUGMENT)) {
            /* if-features from an augment are de facto inherited */
            text_len = tree_print_features(out, node->iffeature, node->iffeature_size,
                                           node->parent->iffeature, node->parent->iffeature_size, opts, &text_str);
        } else {
            text_len = tree_print_features(out, node->iffeature, node->iffeature_size, NULL, 0, opts, &text_str);
        }
        if (text_len) {
            line_len = tree_print_wrap(out, level, line_len, 1, text_len, opts);
            line_len += ly_print(out, text_str);
            lydict_remove(opts->module->ctx, text_str);
        }
        break;
    default:
        /* only grouping */
        break;
    }

    /* this node is finished printing */
    ly_print(out, "\n");

    if ((subtree == 1) || ((node->nodetype & mask) == LYS_USES)) {
        /* we are printing subtree parents, finish here (or uses option) */
        return;
    }

    /* set special config flag */
    switch (node->nodetype & mask) {
    case LYS_INPUT:
        opts->spec_config = 1;
        break;
    case LYS_OUTPUT:
    case LYS_NOTIF:
        opts->spec_config = 2;
        break;
    default:
        break;
    }

print_children:
    /* set child mask and learn the longest child name (needed only if a child can have type) */
    switch (node->nodetype & mask) {
    case LYS_LEAF:
    case LYS_LEAFLIST:
    case LYS_ANYDATA:
    case LYS_ANYXML:
        child_mask = 0;
        max_child_len = 0;
        break;
    case LYS_RPC:
    case LYS_ACTION:
        child_mask = LYS_INPUT | LYS_OUTPUT;
        max_child_len = 0;
        break;
    case LYS_CHOICE:
        child_mask = LYS_CASE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA;
        max_child_len = tree_get_max_name_len(node->child, NULL, child_mask, opts);
        break;
    case LYS_CASE:
    case LYS_NOTIF:
        child_mask = LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_USES;
        max_child_len = tree_get_max_name_len(node->child, NULL, child_mask, opts);
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        child_mask = LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_USES;
        max_child_len = tree_get_max_name_len(node->child, NULL, child_mask, opts);
        break;
    case LYS_USES:
        child_mask = LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_USES | LYS_ACTION | LYS_NOTIF;
        /* inherit the name length from the parent, it does not change */
        max_child_len = max_name_len;
        break;
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_GROUPING:
        child_mask = LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_USES | LYS_ACTION | LYS_NOTIF;
        max_child_len = tree_get_max_name_len(node->child, NULL, child_mask, opts);
        break;
    default:
        child_mask = 0;
        max_child_len = 0;
        LOGINT(node->module->ctx);
        break;
    }

    /* print descendants (children) */
    if (child_mask) {
        LY_TREE_FOR(node->child, sub) {
            /* submodule, foreign augments */
            if (opts->module->type && (sub->parent != node) && (sub->module != opts->module)) {
                continue;
            }
            tree_print_snode(out, level, max_child_len, sub, child_mask, NULL, 0, opts);
        }
    }

    /* reset special config flag */
    switch (node->nodetype & mask) {
    case LYS_INPUT:
    case LYS_OUTPUT:
    case LYS_NOTIF:
        opts->spec_config = 0;
        break;
    default:
        break;
    }
}

static void
tree_print_subtree(struct lyout *out, const struct lys_node *node, tp_opts *opts)
{
    unsigned int depth, i, j;
    int level = 0;
    uint16_t max_child_len;
    const struct lys_node *parent;

    /* learn the depth of the node */
    depth = 0;
    parent = node;
    while (lys_parent(parent)) {
        if (lys_parent(parent)->nodetype != LYS_USES) {
            ++depth;
        }
        parent = lys_parent(parent);
    }

    if (parent->nodetype == LYS_RPC) {
        ly_print(out, "\n%*srpcs:\n", LY_TREE_MOD_DATA_INDENT, "");
        opts->base_indent = LY_TREE_OP_DATA_INDENT;
    } else if (parent->nodetype == LYS_NOTIF) {
        ly_print(out, "\n%*snotifications:\n", LY_TREE_MOD_DATA_INDENT, "");
        opts->base_indent = LY_TREE_OP_DATA_INDENT;
    }

    /* print all the parents */
    if (depth) {
        i = depth;
        do {
            parent = node;
            for (j = 0; j < i; ++j) {
                do {
                    parent = lys_parent(parent);
                } while (parent->nodetype == LYS_USES);
            }

            tree_print_snode(out, level, 0, parent, LYS_CONTAINER | LYS_LIST | LYS_NOTIF | LYS_RPC | LYS_ACTION
                                                    | LYS_INPUT | LYS_OUTPUT, NULL, 1, opts);

            ++level;
            --i;
        } while (i);
    }

    /* print the node and its descendants */
    max_child_len = tree_get_max_name_len(node, NULL, LYS_LEAF|LYS_LEAFLIST|LYS_ANYDATA, opts);
    tree_print_snode(out, level, max_child_len, node, LYS_ANY, NULL, 2, opts);
}

static int
tree_print_aug_target(struct lyout *out, int line_printed, uint8_t indent, const char *path, tp_opts *opts)
{
    int printed, is_last, len;
    const char *cur, *next;

    printed = line_printed;
    cur = path;
    do {
        next = strchr(cur + 1, '/');
        if (!next) {
            len = strlen(cur) + 1;
            is_last = 1;
        } else {
            len = next - cur;
            is_last = 0;
        }

        if (opts->line_length && cur != path && (printed + len > opts->line_length)) {
            /* line_printed is treated as the base indent */
            printed = ly_print(out, "\n%*s", line_printed + indent, "");
            /* minus the newline */
            --printed;
        }
        printed += ly_print(out, "%.*s%s", len, cur, is_last ? ":" : "");

        cur = next;
    } while (!is_last);

    return printed;
}

int
tree_print_model(struct lyout *out, const struct lys_module *module, const char *target_schema_path,
                 int ll, int options)
{
    struct lys_node *node = NULL, *data, *aug;
    struct ly_set *set;
    uint16_t max_child_len;
    int have_rpcs = 0, have_notifs = 0, have_grps = 0, have_augs = 0, printed;
    const char *str;
    int i, mask;
    tp_opts opts;

    memset(&opts, 0, sizeof opts);
    opts.module = module;
    opts.line_length = ll;
    opts.options = options;

    /* we are printing only a subtree */
    if (target_schema_path) {
        set = lys_find_path(module, NULL, target_schema_path);
        if (!set) {
            return EXIT_FAILURE;
        } else if (set->number != 1) {
            LOGVAL(module->ctx, LYE_PATH_INNODE, LY_VLOG_NONE, NULL);
            if (set->number == 0) {
                LOGVAL(module->ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Schema path \"%s\" did not match any nodes.", target_schema_path);
            } else {
                LOGVAL(module->ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Schema path \"%s\" matched more nodes.", target_schema_path);
            }
            ly_set_free(set);
            return EXIT_FAILURE;
        }

        node = set->set.s[0];
        ly_set_free(set);
    }

    if (module->type) {
        ly_print(out, "submodule: %s", module->name);
        data = ((struct lys_submodule *)module)->belongsto->data;
        if (options & LYS_OUTOPT_TREE_RFC) {
            ly_print(out, "\n");
        } else {
            ly_print(out, " (belongs-to %s)\n", ((struct lys_submodule *)module)->belongsto->name);
        }
    } else {
        ly_print(out, "module: %s\n", module->name);
        data = module->data;
    }

    /* only subtree */
    if (target_schema_path) {
        opts.base_indent = LY_TREE_MOD_DATA_INDENT;
        tree_print_subtree(out, node, &opts);
        return EXIT_SUCCESS;
    }

    /* module */
    opts.base_indent = LY_TREE_MOD_DATA_INDENT;
    mask = LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_USES;
    max_child_len = tree_get_max_name_len(data, NULL, mask, &opts);
    LY_TREE_FOR(data, node) {
        if (opts.module->type && (node->module != opts.module)) {
            /* we're printing the submodule only */
            continue;
        }

        switch (node->nodetype) {
        case LYS_RPC:
            if (!lys_is_disabled(node, 0)) {
                have_rpcs++;
            }
            break;
        case LYS_NOTIF:
            if (!lys_is_disabled(node, 0)) {
                have_notifs++;
            }
            break;
        case LYS_GROUPING:
            if ((options & LYS_OUTOPT_TREE_GROUPING) && !lys_is_disabled(node, 0)) {
                have_grps++;
            }
            break;
        default:
            tree_print_snode(out, 0, max_child_len, node, mask, NULL, 0, &opts);
            break;
        }
    }

    /* all remaining nodes printed with operation indent */
    opts.base_indent = LY_TREE_OP_DATA_INDENT;

    /* augments */
    for (i = 0; i < module->augment_size; i++) {
        if ((module->type && (module->augment[i].target->module == module))
                || (!module->type && (lys_node_module(module->augment[i].target) == module))
                || lys_is_disabled((struct lys_node *)&module->augment[i], 0)) {
            /* submodule, target is our submodule or module, target is in our module or any submodules */
            continue;
        }

        if (!have_augs) {
            ly_print(out, "\n");
            have_augs = 1;
        }

        printed = ly_print(out, "%*saugment ", LY_TREE_MOD_DATA_INDENT, "");
        if (options & LYS_OUTOPT_TREE_RFC) {
            str = transform_json2schema(module, module->augment[i].target_name);
            tree_print_aug_target(out, printed, LY_TREE_WRAP_INDENT, str, &opts);
            lydict_remove(module->ctx, str);
        } else {
            tree_print_aug_target(out, printed, LY_TREE_WRAP_INDENT, module->augment[i].target_name, &opts);
        }
        ly_print(out, "\n");

        aug = (struct lys_node *)&module->augment[i];
        mask = LYS_CHOICE | LYS_CASE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_USES
               | LYS_ACTION | LYS_NOTIF;
        max_child_len = tree_get_max_name_len(aug->child, aug, mask, &opts);
        LY_TREE_FOR(aug->child, node) {
            /* submodule, foreign augments */
            if (node->parent != aug) {
                continue;
            }
            tree_print_snode(out, 0, max_child_len, node, mask, aug, 0, &opts);
        }
    }

    /* rpcs */
    if (have_rpcs) {
        ly_print(out, "\n%*srpcs:\n", LY_TREE_MOD_DATA_INDENT, "");

        LY_TREE_FOR(data, node) {
            tree_print_snode(out, 0, 0, node, LYS_RPC, NULL, 0, &opts);
        }
    }

    /* notifications */
    if (have_notifs) {
        ly_print(out, "\n%*snotifications:\n", LY_TREE_MOD_DATA_INDENT, "");

        LY_TREE_FOR(data, node) {
            tree_print_snode(out, 0, 0, node, LYS_NOTIF, NULL, 0, &opts);
        }
    }

    /* groupings */
    if ((options & LYS_OUTOPT_TREE_GROUPING) && have_grps) {
        ly_print(out, "\n");
        LY_TREE_FOR(data, node) {
            if (node->nodetype == LYS_GROUPING) {
                ly_print(out, "%*sgrouping %s:\n", LY_TREE_MOD_DATA_INDENT, "", node->name);

                tree_print_snode(out, 0, 0, node, LYS_GROUPING, NULL, 0, &opts);
            }
        }
    }

    ly_print_flush(out);

    return EXIT_SUCCESS;
}
