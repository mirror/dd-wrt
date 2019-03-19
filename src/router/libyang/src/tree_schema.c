/**
 * @file tree_schema.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Manipulation with libyang schema data structures
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#ifdef __APPLE__
#   include <sys/param.h>
#endif
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include "common.h"
#include "context.h"
#include "parser.h"
#include "resolve.h"
#include "xml.h"
#include "xpath.h"
#include "xml_internal.h"
#include "tree_internal.h"
#include "validation.h"
#include "parser_yang.h"

static int lys_type_dup(struct lys_module *mod, struct lys_node *parent, struct lys_type *new, struct lys_type *old,
                        int in_grp, int shallow, struct unres_schema *unres);

API const struct lys_node_list *
lys_is_key(const struct lys_node_leaf *node, uint8_t *index)
{
    struct lys_node *parent = (struct lys_node *)node;
    struct lys_node_list *list;
    uint8_t i;

    if (!node || node->nodetype != LYS_LEAF) {
        return NULL;
    }

    do {
        parent = lys_parent(parent);
    } while (parent && parent->nodetype == LYS_USES);

    if (!parent || parent->nodetype != LYS_LIST) {
        return NULL;
    }

    list = (struct lys_node_list*)parent;
    for (i = 0; i < list->keys_size; i++) {
        if (list->keys[i] == node) {
            if (index) {
                (*index) = i;
            }
            return list;
        }
    }
    return NULL;
}

API const struct lys_node *
lys_is_disabled(const struct lys_node *node, int recursive)
{
    int i;

    if (!node) {
        return NULL;
    }

check:
    if (node->nodetype != LYS_INPUT && node->nodetype != LYS_OUTPUT) {
        /* input/output does not have if-feature, so skip them */

        /* check local if-features */
        for (i = 0; i < node->iffeature_size; i++) {
            if (!resolve_iffeature(&node->iffeature[i])) {
                return node;
            }
        }
    }

    if (!recursive) {
        return NULL;
    }

    /* go through parents */
    if (node->nodetype == LYS_AUGMENT) {
        /* go to parent actually means go to the target node */
        node = ((struct lys_node_augment *)node)->target;
        if (!node) {
            /* unresolved augment, let's say it's enabled */
            return NULL;
        }
    } else if (node->nodetype == LYS_EXT) {
        return NULL;
    } else if (node->parent) {
        node = node->parent;
    } else {
        return NULL;
    }

    if (recursive == 2) {
        /* continue only if the node cannot have a data instance */
        if (node->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST)) {
            return NULL;
        }
    }
    goto check;
}

API const struct lys_type *
lys_getnext_union_type(const struct lys_type *last, const struct lys_type *type)
{
    int found = 0;

    if (!type || (type->base != LY_TYPE_UNION)) {
        return NULL;
    }

    return lyp_get_next_union_type((struct lys_type *)type, (struct lys_type *)last, &found);
}

int
lys_get_sibling(const struct lys_node *siblings, const char *mod_name, int mod_name_len, const char *name,
                int nam_len, LYS_NODE type, const struct lys_node **ret)
{
    const struct lys_node *node, *parent = NULL;
    const struct lys_module *mod = NULL;
    const char *node_mod_name;

    assert(siblings && mod_name && name);
    assert(!(type & (LYS_USES | LYS_GROUPING)));

    /* fill the lengths in case the caller is so indifferent */
    if (!mod_name_len) {
        mod_name_len = strlen(mod_name);
    }
    if (!nam_len) {
        nam_len = strlen(name);
    }

    while (siblings && (siblings->nodetype == LYS_USES)) {
        siblings = siblings->child;
    }
    if (!siblings) {
        /* unresolved uses */
        return EXIT_FAILURE;
    }

    if (siblings->nodetype == LYS_GROUPING) {
        for (node = siblings; (node->nodetype == LYS_GROUPING) && (node->prev != siblings); node = node->prev);
        if (node->nodetype == LYS_GROUPING) {
            /* we went through all the siblings, only groupings there - no valid sibling */
            return EXIT_FAILURE;
        }
        /* update siblings to be valid */
        siblings = node;
    }

    /* set parent correctly */
    parent = lys_parent(siblings);

    /* go up all uses */
    while (parent && (parent->nodetype == LYS_USES)) {
        parent = lys_parent(parent);
    }

    if (!parent) {
        /* handle situation when there is a top-level uses referencing a foreign grouping */
        for (node = siblings; lys_parent(node) && (node->nodetype == LYS_USES); node = lys_parent(node));
        mod = lys_node_module(node);
    }

    /* try to find the node */
    node = NULL;
    while ((node = lys_getnext(node, parent, mod, LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHCASE | LYS_GETNEXT_WITHINOUT))) {
        if (!type || (node->nodetype & type)) {
            /* module name comparison */
            node_mod_name = lys_node_module(node)->name;
            if (!ly_strequal(node_mod_name, mod_name, 1) && (strncmp(node_mod_name, mod_name, mod_name_len) || node_mod_name[mod_name_len])) {
                continue;
            }

            /* direct name check */
            if (ly_strequal(node->name, name, 1) || (!strncmp(node->name, name, nam_len) && !node->name[nam_len])) {
                if (ret) {
                    *ret = node;
                }
                return EXIT_SUCCESS;
            }
        }
    }

    return EXIT_FAILURE;
}

int
lys_getnext_data(const struct lys_module *mod, const struct lys_node *parent, const char *name, int nam_len,
                 LYS_NODE type, const struct lys_node **ret)
{
    const struct lys_node *node;

    assert((mod || parent) && name);
    assert(!(type & (LYS_AUGMENT | LYS_USES | LYS_GROUPING | LYS_CHOICE | LYS_CASE | LYS_INPUT | LYS_OUTPUT)));

    if (!mod) {
        mod = lys_node_module(parent);
    }

    /* try to find the node */
    node = NULL;
    while ((node = lys_getnext(node, parent, mod, 0))) {
        if (!type || (node->nodetype & type)) {
            /* module check */
            if (lys_node_module(node) != lys_main_module(mod)) {
                continue;
            }

            /* direct name check */
            if (!strncmp(node->name, name, nam_len) && !node->name[nam_len]) {
                if (ret) {
                    *ret = node;
                }
                return EXIT_SUCCESS;
            }
        }
    }

    return EXIT_FAILURE;
}

API const struct lys_node *
lys_getnext(const struct lys_node *last, const struct lys_node *parent, const struct lys_module *module, int options)
{
    const struct lys_node *next, *aug_parent;
    struct lys_node **snode;

    if ((!parent && !module) || (module && module->type) || (parent && (parent->nodetype == LYS_USES) && !(options & LYS_GETNEXT_PARENTUSES))) {
        LOGARG;
        return NULL;
    }

    if (!last) {
        /* first call */

        /* get know where to start */
        if (parent) {
            /* schema subtree */
            snode = lys_child(parent, LYS_UNKNOWN);
            /* do not return anything if the augment does not have any children */
            if (!snode || !(*snode) || ((parent->nodetype == LYS_AUGMENT) && ((*snode)->parent != parent))) {
                return NULL;
            }
            next = last = *snode;
        } else {
            /* top level data */
            if (!(options & LYS_GETNEXT_NOSTATECHECK) && (module->disabled || !module->implemented)) {
                /* nothing to return from a disabled/imported module */
                return NULL;
            }
            next = last = module->data;
        }
    } else if ((last->nodetype == LYS_USES) && (options & LYS_GETNEXT_INTOUSES) && last->child) {
        /* continue with uses content */
        next = last->child;
    } else {
        /* continue after the last returned value */
        next = last->next;
    }

repeat:
    if (parent && (parent->nodetype == LYS_AUGMENT) && next) {
        /* do not return anything outside the parent augment */
        aug_parent = next->parent;
        do {
            while (aug_parent && (aug_parent->nodetype != LYS_AUGMENT)) {
                aug_parent = aug_parent->parent;
            }
            if (aug_parent) {
                if (aug_parent == parent) {
                    break;
                }
                aug_parent = ((struct lys_node_augment *)aug_parent)->target;
            }

        } while (aug_parent);
        if (!aug_parent) {
            return NULL;
        }
    }
    while (next && (next->nodetype == LYS_GROUPING)) {
        if (options & LYS_GETNEXT_WITHGROUPING) {
            return next;
        }
        next = next->next;
    }

    if (!next) {     /* cover case when parent is augment */
        if (!last || last->parent == parent || lys_parent(last) == parent) {
            /* no next element */
            return NULL;
        }
        last = lys_parent(last);
        next = last->next;
        goto repeat;
    } else {
        last = next;
    }

    if (!(options & LYS_GETNEXT_NOSTATECHECK) && lys_is_disabled(next, 0)) {
        next = next->next;
        goto repeat;
    }

    switch (next->nodetype) {
    case LYS_INPUT:
    case LYS_OUTPUT:
        if (options & LYS_GETNEXT_WITHINOUT) {
            return next;
        } else if (next->child) {
            next = next->child;
        } else {
            next = next->next;
        }
        goto repeat;

    case LYS_CASE:
        if (options & LYS_GETNEXT_WITHCASE) {
            return next;
        } else if (next->child) {
            next = next->child;
        } else {
            next = next->next;
        }
        goto repeat;

    case LYS_USES:
        /* go into */
        if (options & LYS_GETNEXT_WITHUSES) {
            return next;
        } else if (next->child) {
            next = next->child;
        } else {
            next = next->next;
        }
        goto repeat;

    case LYS_RPC:
    case LYS_ACTION:
    case LYS_NOTIF:
    case LYS_LEAF:
    case LYS_ANYXML:
    case LYS_ANYDATA:
    case LYS_LIST:
    case LYS_LEAFLIST:
        return next;

    case LYS_CONTAINER:
        if (!((struct lys_node_container *)next)->presence && (options & LYS_GETNEXT_INTONPCONT)) {
            if (next->child) {
                /* go into */
                next = next->child;
            } else {
                next = next->next;
            }
            goto repeat;
        } else {
            return next;
        }

    case LYS_CHOICE:
        if (options & LYS_GETNEXT_WITHCHOICE) {
            return next;
        } else if (next->child) {
            /* go into */
            next = next->child;
        } else {
            next = next->next;
        }
        goto repeat;

    default:
        /* we should not be here */
        return NULL;
    }
}

void
lys_node_unlink(struct lys_node *node)
{
    struct lys_node *parent, *first, **pp = NULL;
    struct lys_module *main_module;

    if (!node) {
        return;
    }

    /* unlink from data model if necessary */
    if (node->module) {
        /* get main module with data tree */
        main_module = lys_node_module(node);
        if (main_module->data == node) {
            main_module->data = node->next;
        }
    }

    /* store pointers to important nodes */
    parent = node->parent;
    if (parent && (parent->nodetype == LYS_AUGMENT)) {
        /* handle augments - first, unlink it from the augment parent ... */
        if (parent->child == node) {
            parent->child = (node->next && node->next->parent == parent) ? node->next : NULL;
        }

        if (parent->flags & LYS_NOTAPPLIED) {
            /* data are not connected in the target, so we cannot continue with the target as a parent */
            parent = NULL;
        } else {
            /* data are connected in target, so we will continue with the target as a parent */
            parent = ((struct lys_node_augment *)parent)->target;
        }
    }

    /* unlink from parent */
    if (parent) {
        if (parent->nodetype == LYS_EXT) {
            pp = (struct lys_node **)lys_ext_complex_get_substmt(lys_snode2stmt(node->nodetype),
                                                                 (struct lys_ext_instance_complex*)parent, NULL);
            if (*pp == node) {
                *pp = node->next;
            }
        } else if (parent->child == node) {
            parent->child = node->next;
        }
        node->parent = NULL;
    }

    /* unlink from siblings */
    if (node->prev == node) {
        /* there are no more siblings */
        return;
    }
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        /* unlinking the last element */
        if (parent) {
            if (parent->nodetype == LYS_EXT) {
                first = *(struct lys_node **)pp;
            } else {
                first = parent->child;
            }
        } else {
            first = node;
            while (first->prev->next) {
                first = first->prev;
            }
        }
        first->prev = node->prev;
    }
    if (node->prev->next) {
        node->prev->next = node->next;
    }

    /* clean up the unlinked element */
    node->next = NULL;
    node->prev = node;
}

struct lys_node_grp *
lys_find_grouping_up(const char *name, struct lys_node *start)
{
    struct lys_node *par_iter, *iter, *stop;

    for (par_iter = start; par_iter; par_iter = par_iter->parent) {
        /* top-level augment, look into module (uses augment is handled correctly below) */
        if (par_iter->parent && !par_iter->parent->parent && (par_iter->parent->nodetype == LYS_AUGMENT)) {
            par_iter = lys_main_module(par_iter->parent->module)->data;
            if (!par_iter) {
                break;
            }
        }

        if (par_iter->nodetype == LYS_EXT) {
            /* we are in a top-level extension, search grouping in top-level groupings */
            par_iter = lys_main_module(par_iter->module)->data;
            if (!par_iter) {
                /* not connected yet, wait */
                return NULL;
            }
        } else if (par_iter->parent && (par_iter->parent->nodetype & (LYS_CHOICE | LYS_CASE | LYS_AUGMENT | LYS_USES))) {
            continue;
        }

        for (iter = par_iter, stop = NULL; iter; iter = iter->prev) {
            if (!stop) {
                stop = par_iter;
            } else if (iter == stop) {
                break;
            }
            if (iter->nodetype != LYS_GROUPING) {
                continue;
            }

            if (!strcmp(name, iter->name)) {
                return (struct lys_node_grp *)iter;
            }
        }
    }

    return NULL;
}

/*
 * get next grouping in the root's subtree, in the
 * first call, tha last is NULL
 */
static struct lys_node_grp *
lys_get_next_grouping(struct lys_node_grp *lastgrp, struct lys_node *root)
{
    struct lys_node *last = (struct lys_node *)lastgrp;
    struct lys_node *next;

    assert(root);

    if (!last) {
        last = root;
    }

    while (1) {
        if ((last->nodetype & (LYS_CONTAINER | LYS_CHOICE | LYS_LIST | LYS_GROUPING | LYS_INPUT | LYS_OUTPUT))) {
            next = last->child;
        } else {
            next = NULL;
        }
        if (!next) {
            if (last == root) {
                /* we are done */
                return NULL;
            }

            /* no children, go to siblings */
            next = last->next;
        }
        while (!next) {
            /* go back through parents */
            if (lys_parent(last) == root) {
                /* we are done */
                return NULL;
            }
            next = last->next;
            last = lys_parent(last);
        }

        if (next->nodetype == LYS_GROUPING) {
            return (struct lys_node_grp *)next;
        }

        last = next;
    }
}

/* logs directly */
int
lys_check_id(struct lys_node *node, struct lys_node *parent, struct lys_module *module)
{
    struct lys_node *start, *stop, *iter;
    struct lys_node_grp *grp;
    int down, up;

    assert(node);

    if (!parent) {
        assert(module);
    } else {
        module = parent->module;
    }
    module = lys_main_module(module);

    switch (node->nodetype) {
    case LYS_GROUPING:
        /* 6.2.1, rule 6 */
        if (parent) {
            start = *lys_child(parent, LYS_GROUPING);
            if (!start) {
                down = 0;
                start = parent;
            } else {
                down = 1;
            }
            if (parent->nodetype == LYS_EXT) {
                up = 0;
            } else {
                up = 1;
            }
        } else {
            down = up = 1;
            start = module->data;
        }
        /* go up */
        if (up && lys_find_grouping_up(node->name, start)) {
            LOGVAL(module->ctx, LYE_DUPID, LY_VLOG_LYS, node, "grouping", node->name);
            return EXIT_FAILURE;
        }
        /* go down, because grouping can be defined after e.g. container in which is collision */
        if (down) {
            for (iter = start, stop = NULL; iter; iter = iter->prev) {
                if (!stop) {
                    stop = start;
                } else if (iter == stop) {
                    break;
                }
                if (!(iter->nodetype & (LYS_CONTAINER | LYS_CHOICE | LYS_LIST | LYS_GROUPING | LYS_INPUT | LYS_OUTPUT))) {
                    continue;
                }

                grp = NULL;
                while ((grp = lys_get_next_grouping(grp, iter))) {
                    if (ly_strequal(node->name, grp->name, 1)) {
                        LOGVAL(module->ctx, LYE_DUPID,LY_VLOG_LYS, node, "grouping", node->name);
                        return EXIT_FAILURE;
                    }
                }
            }
        }
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
    case LYS_LIST:
    case LYS_CONTAINER:
    case LYS_CHOICE:
    case LYS_ANYDATA:
        /* 6.2.1, rule 7 */
        if (parent) {
            iter = parent;
            while (iter && (iter->nodetype & (LYS_USES | LYS_CASE | LYS_CHOICE | LYS_AUGMENT))) {
                if (iter->nodetype == LYS_AUGMENT) {
                    if (((struct lys_node_augment *)iter)->target) {
                        /* augment is resolved, go up */
                        iter = ((struct lys_node_augment *)iter)->target;
                        continue;
                    }
                    /* augment is not resolved, this is the final parent */
                    break;
                }
                iter = iter->parent;
            }

            if (!iter) {
                stop = NULL;
                iter = module->data;
            } else if (iter->nodetype == LYS_EXT) {
                stop = iter;
                iter = (struct lys_node *)lys_child(iter, node->nodetype);
                if (iter) {
                    iter = *(struct lys_node **)iter;
                }
            } else {
                stop = iter;
                iter = iter->child;
            }
        } else {
            stop = NULL;
            iter = module->data;
        }
        while (iter) {
            if (iter->nodetype & (LYS_USES | LYS_CASE)) {
                iter = iter->child;
                continue;
            }

            if (iter->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_CONTAINER | LYS_CHOICE | LYS_ANYDATA)) {
                if (iter->module == node->module && ly_strequal(iter->name, node->name, 1)) {
                    LOGVAL(module->ctx, LYE_DUPID, LY_VLOG_LYS, node, strnodetype(node->nodetype), node->name);
                    return EXIT_FAILURE;
                }
            }

            /* special case for choice - we must check the choice's name as
             * well as the names of nodes under the choice
             */
            if (iter->nodetype == LYS_CHOICE) {
                iter = iter->child;
                continue;
            }

            /* go to siblings */
            if (!iter->next) {
                /* no sibling, go to parent's sibling */
                do {
                    /* for parent LYS_AUGMENT */
                    if (iter->parent == stop) {
                        iter = stop;
                        break;
                    }
                    iter = lys_parent(iter);
                    if (iter && iter->next) {
                        break;
                    }
                } while (iter != stop);

                if (iter == stop) {
                    break;
                }
            }
            iter = iter->next;
        }
        break;
    case LYS_CASE:
        /* 6.2.1, rule 8 */
        if (parent) {
            start = *lys_child(parent, LYS_CASE);
        } else {
            start = module->data;
        }

        LY_TREE_FOR(start, iter) {
            if (!(iter->nodetype & (LYS_ANYDATA | LYS_CASE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST))) {
                continue;
            }

            if (iter->module == node->module && ly_strequal(iter->name, node->name, 1)) {
                LOGVAL(module->ctx, LYE_DUPID, LY_VLOG_LYS, node, "case", node->name);
                return EXIT_FAILURE;
            }
        }
        break;
    default:
        /* no check needed */
        break;
    }

    return EXIT_SUCCESS;
}

/* logs directly */
int
lys_node_addchild(struct lys_node *parent, struct lys_module *module, struct lys_node *child, int options)
{
    struct ly_ctx *ctx = child->module->ctx;
    struct lys_node *iter, **pchild, *log_parent;
    struct lys_node_inout *in, *out;
    struct lys_node_case *c;
    int type, shortcase = 0;
    void *p;
    struct lyext_substmt *info = NULL;

    assert(child);

    if (parent) {
        type = parent->nodetype;
        module = parent->module;
        log_parent = parent;

        if (type == LYS_USES) {
            /* we are adding children to uses -> we must be copying grouping contents into it, so properly check the parent */
            log_parent = lys_parent(log_parent);
            while (log_parent && (log_parent->nodetype == LYS_USES)) {
                log_parent = lys_parent(log_parent);
            }
            if (log_parent) {
                type = log_parent->nodetype;
            } else {
                type = 0;
            }
        }
    } else {
        assert(module);
        assert(!(child->nodetype & (LYS_INPUT | LYS_OUTPUT)));
        type = 0;
        log_parent = NULL;
    }

    /* checks */
    switch (type) {
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_GROUPING:
    case LYS_USES:
        if (!(child->nodetype &
                (LYS_ANYDATA | LYS_CHOICE | LYS_CONTAINER | LYS_GROUPING | LYS_LEAF |
                 LYS_LEAFLIST | LYS_LIST | LYS_USES | LYS_ACTION | LYS_NOTIF))) {
            LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, log_parent, strnodetype(child->nodetype), strnodetype(log_parent->nodetype));
            return EXIT_FAILURE;
        }
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
    case LYS_NOTIF:
        if (!(child->nodetype &
                (LYS_ANYDATA | LYS_CHOICE | LYS_CONTAINER | LYS_GROUPING | LYS_LEAF |
                 LYS_LEAFLIST | LYS_LIST | LYS_USES))) {
            LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, log_parent, strnodetype(child->nodetype), strnodetype(log_parent->nodetype));
            return EXIT_FAILURE;
        }
        break;
    case LYS_CHOICE:
        if (!(child->nodetype &
                (LYS_ANYDATA | LYS_CASE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_CHOICE))) {
            LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, log_parent, strnodetype(child->nodetype), "choice");
            return EXIT_FAILURE;
        }
        if (child->nodetype != LYS_CASE) {
            shortcase = 1;
        }
        break;
    case LYS_CASE:
        if (!(child->nodetype &
                (LYS_ANYDATA | LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_USES))) {
            LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, log_parent, strnodetype(child->nodetype), "case");
            return EXIT_FAILURE;
        }
        break;
    case LYS_RPC:
    case LYS_ACTION:
        if (!(child->nodetype & (LYS_INPUT | LYS_OUTPUT | LYS_GROUPING))) {
            LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, log_parent, strnodetype(child->nodetype), "rpc");
            return EXIT_FAILURE;
        }
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
    case LYS_ANYXML:
    case LYS_ANYDATA:
        LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, log_parent, strnodetype(child->nodetype), strnodetype(log_parent->nodetype));
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "The \"%s\" statement cannot have any data substatement.",
               strnodetype(log_parent->nodetype));
        return EXIT_FAILURE;
    case LYS_AUGMENT:
        if (!(child->nodetype &
                (LYS_ANYDATA | LYS_CASE | LYS_CHOICE | LYS_CONTAINER | LYS_LEAF
                | LYS_LEAFLIST | LYS_LIST | LYS_USES | LYS_ACTION | LYS_NOTIF))) {
            LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, log_parent, strnodetype(child->nodetype), strnodetype(log_parent->nodetype));
            return EXIT_FAILURE;
        }
        break;
    case LYS_UNKNOWN:
        /* top level */
        if (!(child->nodetype &
                (LYS_ANYDATA | LYS_CHOICE | LYS_CONTAINER | LYS_LEAF | LYS_GROUPING
                | LYS_LEAFLIST | LYS_LIST | LYS_USES | LYS_RPC | LYS_NOTIF | LYS_AUGMENT))) {
            LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, log_parent, strnodetype(child->nodetype), "(sub)module");
            return EXIT_FAILURE;
        }
        break;
    case LYS_EXT:
        /* plugin-defined */
        p = lys_ext_complex_get_substmt(lys_snode2stmt(child->nodetype), (struct lys_ext_instance_complex*)log_parent, &info);
        if (!p) {
            LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_LYS, log_parent, strnodetype(child->nodetype),
                   ((struct lys_ext_instance_complex*)log_parent)->def->name);
            return EXIT_FAILURE;
        }
        /* TODO check cardinality */
        break;
    }

    /* check identifier uniqueness */
    if (!(module->ctx->models.flags & LY_CTX_TRUSTED) && lys_check_id(child, parent, module)) {
        return EXIT_FAILURE;
    }

    if (child->parent) {
        lys_node_unlink(child);
    }

    if ((child->nodetype & (LYS_INPUT | LYS_OUTPUT)) && parent->nodetype != LYS_EXT) {
        /* find the implicit input/output node */
        LY_TREE_FOR(parent->child, iter) {
            if (iter->nodetype == child->nodetype) {
                break;
            }
        }
        assert(iter);

        /* switch the old implicit node (iter) with the new one (child) */
        if (parent->child == iter) {
            /* first child */
            parent->child = child;
        } else {
            iter->prev->next = child;
        }
        child->prev = iter->prev;
        child->next = iter->next;
        if (iter->next) {
            iter->next->prev = child;
        } else {
            /* last child */
            parent->child->prev = child;
        }
        child->parent = parent;

        /* isolate the node and free it */
        iter->next = NULL;
        iter->prev = iter;
        iter->parent = NULL;
        lys_node_free(iter, NULL, 0);
    } else {
        if (shortcase) {
            /* create the implicit case to allow it to serve as a target of the augments,
             * it won't be printed, but it will be present in the tree */
            c = calloc(1, sizeof *c);
            LY_CHECK_ERR_RETURN(!c, LOGMEM(ctx), EXIT_FAILURE);
            c->name = lydict_insert(module->ctx, child->name, 0);
            c->flags = LYS_IMPLICIT;
            if (!(options & (LYS_PARSE_OPT_CFG_IGNORE | LYS_PARSE_OPT_CFG_NOINHERIT))) {
                /* get config flag from parent */
                c->flags |= parent->flags & LYS_CONFIG_MASK;
            }
            c->module = module;
            c->nodetype = LYS_CASE;
            c->prev = (struct lys_node*)c;
            lys_node_addchild(parent, module, (struct lys_node*)c, options);
            parent = (struct lys_node*)c;
        }
        /* connect the child correctly */
        if (!parent) {
            if (module->data) {
                module->data->prev->next = child;
                child->prev = module->data->prev;
                module->data->prev = child;
            } else {
                module->data = child;
            }
        } else {
            pchild = lys_child(parent, child->nodetype);
            assert(pchild);

            child->parent = parent;
            if (!(*pchild)) {
                /* the only/first child of the parent */
                *pchild = child;
                iter = child;
            } else {
                /* add a new child at the end of parent's child list */
                iter = (*pchild)->prev;
                iter->next = child;
                child->prev = iter;
            }
            while (iter->next) {
                iter = iter->next;
                iter->parent = parent;
            }
            (*pchild)->prev = iter;
        }
    }

    /* check config value (but ignore them in groupings and augments) */
    for (iter = parent; iter && !(iter->nodetype & (LYS_GROUPING | LYS_AUGMENT | LYS_EXT)); iter = iter->parent);
    if (parent && !iter) {
        for (iter = child; iter && !(iter->nodetype & (LYS_NOTIF | LYS_INPUT | LYS_OUTPUT | LYS_RPC)); iter = iter->parent);
        if (!iter && (parent->flags & LYS_CONFIG_R) && (child->flags & LYS_CONFIG_W)) {
            LOGVAL(ctx, LYE_INARG, LY_VLOG_LYS, child, "true", "config");
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "State nodes cannot have configuration nodes as children.");
            return EXIT_FAILURE;
        }
    }

    /* propagate information about status data presence */
    if ((child->nodetype & (LYS_CONTAINER | LYS_CHOICE | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA)) &&
            (child->flags & LYS_INCL_STATUS)) {
        for(iter = parent; iter; iter = lys_parent(iter)) {
            /* store it only into container or list - the only data inner nodes */
            if (iter->nodetype & (LYS_CONTAINER | LYS_LIST)) {
                if (iter->flags & LYS_INCL_STATUS) {
                    /* done, someone else set it already from here */
                    break;
                }
                /* set flag about including status data */
                iter->flags |= LYS_INCL_STATUS;
            }
        }
    }

    /* create implicit input/output nodes to have available them as possible target for augment */
    if ((child->nodetype & (LYS_RPC | LYS_ACTION)) && !child->child) {
        in = calloc(1, sizeof *in);
        out = calloc(1, sizeof *out);
        if (!in || !out) {
            LOGMEM(ctx);
            free(in);
            free(out);
            return EXIT_FAILURE;
        }
        in->nodetype = LYS_INPUT;
        in->name = lydict_insert(child->module->ctx, "input", 5);
        out->nodetype = LYS_OUTPUT;
        out->name = lydict_insert(child->module->ctx, "output", 6);
        in->module = out->module = child->module;
        in->parent = out->parent = child;
        in->flags = out->flags = LYS_IMPLICIT;
        in->next = (struct lys_node *)out;
        in->prev = (struct lys_node *)out;
        out->prev = (struct lys_node *)in;
        child->child = (struct lys_node *)in;
    }
    return EXIT_SUCCESS;
}

const struct lys_module *
lys_parse_mem_(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format, const char *revision, int internal, int implement)
{
    char *enlarged_data = NULL;
    struct lys_module *mod = NULL;
    unsigned int len;

    if (!ctx || !data) {
        LOGARG;
        return NULL;
    }

    if (!internal && format == LYS_IN_YANG) {
        /* enlarge data by 2 bytes for flex */
        len = strlen(data);
        enlarged_data = malloc((len + 2) * sizeof *enlarged_data);
        LY_CHECK_ERR_RETURN(!enlarged_data, LOGMEM(ctx), NULL);
        memcpy(enlarged_data, data, len);
        enlarged_data[len] = enlarged_data[len + 1] = '\0';
        data = enlarged_data;
    }

    switch (format) {
    case LYS_IN_YIN:
        mod = yin_read_module(ctx, data, revision, implement);
        break;
    case LYS_IN_YANG:
        mod = yang_read_module(ctx, data, 0, revision, implement);
        break;
    default:
        LOGERR(ctx, LY_EINVAL, "Invalid schema input format.");
        break;
    }

    free(enlarged_data);

    /* hack for NETCONF's edit-config's operation attribute. It is not defined in the schema, but since libyang
     * implements YANG metadata (annotations), we need its definition. Because the ietf-netconf schema is not the
     * internal part of libyang, we cannot add the annotation into the schema source, but we do it here to have
     * the anotation definitions available in the internal schema structure. There is another hack in schema
     * printers to do not print this internally added annotation. */
    if (mod && ly_strequal(mod->name, "ietf-netconf", 0)) {
        if (lyp_add_ietf_netconf_annotations_config(mod)) {
            lys_free(mod, NULL, 1, 1);
            return NULL;
        }
    }

    return mod;
}

API const struct lys_module *
lys_parse_mem(struct ly_ctx *ctx, const char *data, LYS_INFORMAT format)
{
    return lys_parse_mem_(ctx, data, format, NULL, 0, 1);
}

struct lys_submodule *
lys_sub_parse_mem(struct lys_module *module, const char *data, LYS_INFORMAT format, struct unres_schema *unres)
{
    char *enlarged_data = NULL;
    struct lys_submodule *submod = NULL;
    unsigned int len;

    assert(module);
    assert(data);

    if (format == LYS_IN_YANG) {
        /* enlarge data by 2 bytes for flex */
        len = strlen(data);
        enlarged_data = malloc((len + 2) * sizeof *enlarged_data);
        LY_CHECK_ERR_RETURN(!enlarged_data, LOGMEM(module->ctx), NULL);
        memcpy(enlarged_data, data, len);
        enlarged_data[len] = enlarged_data[len + 1] = '\0';
        data = enlarged_data;
    }

    /* get the main module */
    module = lys_main_module(module);

    switch (format) {
    case LYS_IN_YIN:
        submod = yin_read_submodule(module, data, unres);
        break;
    case LYS_IN_YANG:
        submod = yang_read_submodule(module, data, 0, unres);
        break;
    default:
        assert(0);
        break;
    }

    free(enlarged_data);
    return submod;
}

API const struct lys_module *
lys_parse_path(struct ly_ctx *ctx, const char *path, LYS_INFORMAT format)
{
    int fd;
    const struct lys_module *ret;
    const char *rev, *dot, *filename;
    size_t len;

    if (!ctx || !path) {
        LOGARG;
        return NULL;
    }

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        LOGERR(ctx, LY_ESYS, "Opening file \"%s\" failed (%s).", path, strerror(errno));
        return NULL;
    }

    ret = lys_parse_fd(ctx, fd, format);
    close(fd);

    if (!ret) {
        /* error */
        return NULL;
    }

    /* check that name and revision match filename */
    filename = strrchr(path, '/');
    if (!filename) {
        filename = path;
    } else {
        filename++;
    }
    rev = strchr(filename, '@');
    dot = strrchr(filename, '.');

    /* name */
    len = strlen(ret->name);
    if (strncmp(filename, ret->name, len) ||
            ((rev && rev != &filename[len]) || (!rev && dot != &filename[len]))) {
        LOGWRN(ctx, "File name \"%s\" does not match module name \"%s\".", filename, ret->name);
    }
    if (rev) {
        len = dot - ++rev;
        if (!ret->rev_size || len != 10 || strncmp(ret->rev[0].date, rev, len)) {
            LOGWRN(ctx, "File name \"%s\" does not match module revision \"%s\".", filename,
                   ret->rev_size ? ret->rev[0].date : "none");
        }
    }

    if (!ret->filepath) {
        /* store URI */
        char rpath[PATH_MAX];
        if (realpath(path, rpath) != NULL) {
            ((struct lys_module *)ret)->filepath = lydict_insert(ctx, rpath, 0);
        } else {
            ((struct lys_module *)ret)->filepath = lydict_insert(ctx, path, 0);
        }
    }

    return ret;
}

API const struct lys_module *
lys_parse_fd(struct ly_ctx *ctx, int fd, LYS_INFORMAT format)
{
    return lys_parse_fd_(ctx, fd, format, NULL, 1);
}

static void
lys_parse_set_filename(struct ly_ctx *ctx, const char **filename, int fd)
{
#ifdef __APPLE__
    char path[MAXPATHLEN];
#else
    int len;
    char path[PATH_MAX], proc_path[32];
#endif

#ifdef __APPLE__
    if (fcntl(fd, F_GETPATH, path) != -1) {
        *filename = lydict_insert(ctx, path, 0);
    }
#else
    /* get URI if there is /proc */
    sprintf(proc_path, "/proc/self/fd/%d", fd);
    if ((len = readlink(proc_path, path, PATH_MAX - 1)) > 0) {
        *filename = lydict_insert(ctx, path, len);
    }
#endif
}

const struct lys_module *
lys_parse_fd_(struct ly_ctx *ctx, int fd, LYS_INFORMAT format, const char *revision, int implement)
{
    const struct lys_module *module;
    size_t length;
    char *addr;

    if (!ctx || fd < 0) {
        LOGARG;
        return NULL;
    }

    if (lyp_mmap(ctx, fd, format == LYS_IN_YANG ? 1 : 0, &length, (void **)&addr)) {
        LOGERR(ctx, LY_ESYS, "Mapping file descriptor into memory failed (%s()).", __func__);
        return NULL;
    } else if (!addr) {
        LOGERR(ctx, LY_EINVAL, "Empty schema file.");
        return NULL;
    }

    module = lys_parse_mem_(ctx, addr, format, revision, 1, implement);
    lyp_munmap(addr, length);

    if (module && !module->filepath) {
        lys_parse_set_filename(ctx, (const char **)&module->filepath, fd);
    }

    return module;
}

struct lys_submodule *
lys_sub_parse_fd(struct lys_module *module, int fd, LYS_INFORMAT format, struct unres_schema *unres)
{
    struct lys_submodule *submodule;
    size_t length;
    char *addr;

    assert(module);
    assert(fd >= 0);

    if (lyp_mmap(module->ctx, fd, format == LYS_IN_YANG ? 1 : 0, &length, (void **)&addr)) {
        LOGERR(module->ctx, LY_ESYS, "Mapping file descriptor into memory failed (%s()).", __func__);
        return NULL;
    } else if (!addr) {
        LOGERR(module->ctx, LY_EINVAL, "Empty submodule schema file.");
        return NULL;
    }

    /* get the main module */
    module = lys_main_module(module);

    switch (format) {
    case LYS_IN_YIN:
        submodule = yin_read_submodule(module, addr, unres);
        break;
    case LYS_IN_YANG:
        submodule = yang_read_submodule(module, addr, 0, unres);
        break;
    default:
        LOGINT(module->ctx);
        return NULL;
    }

    lyp_munmap(addr, length);

    if (submodule && !submodule->filepath) {
        lys_parse_set_filename(module->ctx, (const char **)&submodule->filepath, fd);
    }

    return submodule;

}

API int
lys_search_localfile(const char * const *searchpaths, int cwd, const char *name, const char *revision, char **localfile, LYS_INFORMAT *format)
{
    size_t len, flen, match_len = 0, dir_len;
    int i, implicit_cwd = 0, ret = EXIT_FAILURE;
    char *wd, *wn = NULL;
    DIR *dir = NULL;
    struct dirent *file;
    char *match_name = NULL;
    LYS_INFORMAT format_aux, match_format = 0;
    unsigned int u;
    struct ly_set *dirs;
    struct stat st;

    if (!localfile) {
        LOGARG;
        return EXIT_FAILURE;
    }

    /* start to fill the dir fifo with the context's search path (if set)
     * and the current working directory */
    dirs = ly_set_new();
    if (!dirs) {
        LOGMEM(NULL);
        return EXIT_FAILURE;
    }

    len = strlen(name);
    if (cwd) {
        wd = get_current_dir_name();
        if (!wd) {
            LOGMEM(NULL);
            goto cleanup;
        } else {
            /* add implicit current working directory (./) to be searched,
             * this directory is not searched recursively */
            if (ly_set_add(dirs, wd, 0) == -1) {
                goto cleanup;
            }
            implicit_cwd = 1;
        }
    }
    if (searchpaths) {
        for (i = 0; searchpaths[i]; i++) {
            /* check for duplicities with the implicit current working directory */
            if (implicit_cwd && !strcmp(dirs->set.g[0], searchpaths[i])) {
                implicit_cwd = 0;
                continue;
            }
            wd = strdup(searchpaths[i]);
            if (!wd) {
                LOGMEM(NULL);
                goto cleanup;
            } else if (ly_set_add(dirs, wd, 0) == -1) {
                goto cleanup;
            }
        }
    }
    wd = NULL;

    /* start searching */
    while (dirs->number) {
        free(wd);
        free(wn); wn = NULL;

        dirs->number--;
        wd = (char *)dirs->set.g[dirs->number];
        dirs->set.g[dirs->number] = NULL;
        LOGVRB("Searching for \"%s\" in %s.", name, wd);

        if (dir) {
            closedir(dir);
        }
        dir = opendir(wd);
        dir_len = strlen(wd);
        if (!dir) {
            LOGWRN(NULL, "Unable to open directory \"%s\" for searching (sub)modules (%s).", wd, strerror(errno));
        } else {
            while ((file = readdir(dir))) {
                if (!strcmp(".", file->d_name) || !strcmp("..", file->d_name)) {
                    /* skip . and .. */
                    continue;
                }
                free(wn);
                if (asprintf(&wn, "%s/%s", wd, file->d_name) == -1) {
                    LOGMEM(NULL);
                    goto cleanup;
                }
                if (stat(wn, &st) == -1) {
                    LOGWRN(NULL, "Unable to get information about \"%s\" file in \"%s\" when searching for (sub)modules (%s)",
                           file->d_name, wd, strerror(errno));
                    continue;
                }
                if (S_ISDIR(st.st_mode) && (dirs->number || !implicit_cwd)) {
                    /* we have another subdirectory in searchpath to explore,
                     * subdirectories are not taken into account in current working dir (dirs->set.g[0]) */
                    if (ly_set_add(dirs, wn, 0) == -1) {
                        goto cleanup;
                    }
                    /* continue with the next item in current directory */
                    wn = NULL;
                    continue;
                } else if (!S_ISREG(st.st_mode)) {
                    /* not a regular file (note that we see the target of symlinks instead of symlinks */
                    continue;
                }

                /* here we know that the item is a file which can contain a module */
                if (strncmp(name, file->d_name, len) ||
                        (file->d_name[len] != '.' && file->d_name[len] != '@')) {
                    /* different filename than the module we search for */
                    continue;
                }

                /* get type according to filename suffix */
                flen = strlen(file->d_name);
                if (!strcmp(&file->d_name[flen - 4], ".yin")) {
                    format_aux = LYS_IN_YIN;
                } else if (!strcmp(&file->d_name[flen - 5], ".yang")) {
                    format_aux = LYS_IN_YANG;
                } else {
                    /* not supportde suffix/file format */
                    continue;
                }

                if (revision) {
                    /* we look for the specific revision, try to get it from the filename */
                    if (file->d_name[len] == '@') {
                        /* check revision from the filename */
                        if (strncmp(revision, &file->d_name[len + 1], strlen(revision))) {
                            /* another revision */
                            continue;
                        } else {
                            /* exact revision */
                            free(match_name);
                            match_name = wn;
                            wn = NULL;
                            match_len = dir_len + 1 + len;
                            match_format = format_aux;
                            goto success;
                        }
                    } else {
                        /* continue trying to find exact revision match, use this only if not found */
                        free(match_name);
                        match_name = wn;
                        wn = NULL;
                        match_len = dir_len + 1 +len;
                        match_format = format_aux;
                        continue;
                    }
                } else {
                    /* remember the revision and try to find the newest one */
                    if (match_name) {
                        if (file->d_name[len] != '@' || lyp_check_date(NULL, &file->d_name[len + 1])) {
                            continue;
                        } else if (match_name[match_len] == '@' &&
                                (strncmp(&match_name[match_len + 1], &file->d_name[len + 1], LY_REV_SIZE - 1) >= 0)) {
                            continue;
                        }
                        free(match_name);
                    }

                    match_name = wn;
                    wn = NULL;
                    match_len = dir_len + 1 + len;
                    match_format = format_aux;
                    continue;
                }
            }
        }
    }

success:
    (*localfile) = match_name;
    match_name = NULL;
    if (format) {
        (*format) = match_format;
    }
    ret = EXIT_SUCCESS;

cleanup:
    free(wn);
    free(wd);
    if (dir) {
        closedir(dir);
    }
    free(match_name);
    for (u = 0; u < dirs->number; u++) {
        free(dirs->set.g[u]);
    }
    ly_set_free(dirs);

    return ret;
}

int
lys_ext_iter(struct lys_ext_instance **ext, uint8_t ext_size, uint8_t start, LYEXT_SUBSTMT substmt)
{
    unsigned int u;

    for (u = start; u < ext_size; u++) {
        if (ext[u]->insubstmt == substmt) {
            return u;
        }
    }

    return -1;
}

/*
 * duplicate extension instance
 */
int
lys_ext_dup(struct ly_ctx *ctx, struct lys_module *mod, struct lys_ext_instance **orig, uint8_t size, void *parent,
            LYEXT_PAR parent_type, struct lys_ext_instance ***new, int shallow, struct unres_schema *unres)
{
    int i;
    uint8_t u = 0;
    struct lys_ext_instance **result;
    struct unres_ext *info, *info_orig;
    size_t len;

    assert(new);

    if (!size) {
        if (orig) {
            LOGINT(ctx);
            return EXIT_FAILURE;
        }
        (*new) = NULL;
        return EXIT_SUCCESS;
    }

    (*new) = result = calloc(size, sizeof *result);
    LY_CHECK_ERR_RETURN(!result, LOGMEM(ctx), EXIT_FAILURE);
    for (u = 0; u < size; u++) {
        if (orig[u]) {
            /* resolved extension instance, just duplicate it */
            switch(orig[u]->ext_type) {
            case LYEXT_FLAG:
                result[u] = malloc(sizeof(struct lys_ext_instance));
                LY_CHECK_ERR_GOTO(!result[u], LOGMEM(ctx), error);
                break;
            case LYEXT_COMPLEX:
                len = ((struct lyext_plugin_complex*)orig[u]->def->plugin)->instance_size;
                result[u] = calloc(1, len);
                LY_CHECK_ERR_GOTO(!result[u], LOGMEM(ctx), error);

                ((struct lys_ext_instance_complex*)result[u])->substmt = ((struct lyext_plugin_complex*)orig[u]->def->plugin)->substmt;
                /* TODO duplicate data in extension instance content */
                memcpy((void*)result[u] + sizeof(**orig), (void*)orig[u] + sizeof(**orig), len - sizeof(**orig));
                break;
            }
            /* generic part */
            result[u]->def = orig[u]->def;
            result[u]->flags = LYEXT_OPT_CONTENT;
            result[u]->arg_value = lydict_insert(ctx, orig[u]->arg_value, 0);
            result[u]->parent = parent;
            result[u]->parent_type = parent_type;
            result[u]->insubstmt = orig[u]->insubstmt;
            result[u]->insubstmt_index = orig[u]->insubstmt_index;
            result[u]->ext_type = orig[u]->ext_type;
            result[u]->priv = NULL;
            result[u]->nodetype = LYS_EXT;
            result[u]->module = mod;

            /* extensions */
            result[u]->ext_size = orig[u]->ext_size;
            if (lys_ext_dup(ctx, mod, orig[u]->ext, orig[u]->ext_size, result[u],
                            LYEXT_PAR_EXTINST, &result[u]->ext, shallow, unres)) {
                goto error;
            }

            /* in case of shallow copy (duplication for deviation), duplicate only the link to private data
             * in a new copy, otherwise (grouping instantiation) do not duplicate the private data */
            if (shallow) {
                result[u]->priv = orig[u]->priv;
            }
        } else {
            /* original extension is not yet resolved, so duplicate it in unres */
            i = unres_schema_find(unres, -1, &orig, UNRES_EXT);
            if (i == -1) {
                /* extension not found in unres */
                LOGINT(ctx);
                goto error;
            }
            info_orig = unres->str_snode[i];
            info = malloc(sizeof *info);
            LY_CHECK_ERR_GOTO(!info, LOGMEM(ctx), error);
            info->datatype = info_orig->datatype;
            if (info->datatype == LYS_IN_YIN) {
                info->data.yin = lyxml_dup_elem(ctx, info_orig->data.yin, NULL, 1);
            } /* else TODO YANG */
            info->parent = parent;
            info->mod = mod;
            info->parent_type = parent_type;
            info->ext_index = u;
            if (unres_schema_add_node(info->mod, unres, new, UNRES_EXT, (struct lys_node *)info) == -1) {
                goto error;
            }
        }
    }

    return EXIT_SUCCESS;

error:
    (*new) = NULL;
    lys_extension_instances_free(ctx, result, u, NULL);
    return EXIT_FAILURE;
}

static struct lys_restr *
lys_restr_dup(struct lys_module *mod, struct lys_restr *old, int size, int shallow, struct unres_schema *unres)
{
    struct lys_restr *result;
    int i;

    if (!size) {
        return NULL;
    }

    result = calloc(size, sizeof *result);
    LY_CHECK_ERR_RETURN(!result, LOGMEM(mod->ctx), NULL);

    for (i = 0; i < size; i++) {
        result[i].ext_size = old[i].ext_size;
        lys_ext_dup(mod->ctx, mod, old[i].ext, old[i].ext_size, &result[i], LYEXT_PAR_RESTR, &result[i].ext, shallow, unres);
        result[i].expr = lydict_insert(mod->ctx, old[i].expr, 0);
        result[i].dsc = lydict_insert(mod->ctx, old[i].dsc, 0);
        result[i].ref = lydict_insert(mod->ctx, old[i].ref, 0);
        result[i].eapptag = lydict_insert(mod->ctx, old[i].eapptag, 0);
        result[i].emsg = lydict_insert(mod->ctx, old[i].emsg, 0);
    }

    return result;
}

void
lys_restr_free(struct ly_ctx *ctx, struct lys_restr *restr,
               void (*private_destructor)(const struct lys_node *node, void *priv))
{
    assert(ctx);
    if (!restr) {
        return;
    }

    lys_extension_instances_free(ctx, restr->ext, restr->ext_size, private_destructor);
    lydict_remove(ctx, restr->expr);
    lydict_remove(ctx, restr->dsc);
    lydict_remove(ctx, restr->ref);
    lydict_remove(ctx, restr->eapptag);
    lydict_remove(ctx, restr->emsg);
}

API void
lys_iffeature_free(struct ly_ctx *ctx, struct lys_iffeature *iffeature, uint8_t iffeature_size,
                   int shallow, void (*private_destructor)(const struct lys_node *node, void *priv))
{
    uint8_t i;

    for (i = 0; i < iffeature_size; ++i) {
        lys_extension_instances_free(ctx, iffeature[i].ext, iffeature[i].ext_size, private_destructor);
        if (!shallow) {
            free(iffeature[i].expr);
            free(iffeature[i].features);
        }
    }
    free(iffeature);
}

static int
type_dup(struct lys_module *mod, struct lys_node *parent, struct lys_type *new, struct lys_type *old,
         LY_DATA_TYPE base, int in_grp, int shallow, struct unres_schema *unres)
{
    int i;
    unsigned int u;

    switch (base) {
    case LY_TYPE_BINARY:
        if (old->info.binary.length) {
            new->info.binary.length = lys_restr_dup(mod, old->info.binary.length, 1, shallow, unres);
        }
        break;

    case LY_TYPE_BITS:
        new->info.bits.count = old->info.bits.count;
        if (new->info.bits.count) {
            new->info.bits.bit = calloc(new->info.bits.count, sizeof *new->info.bits.bit);
            LY_CHECK_ERR_RETURN(!new->info.bits.bit, LOGMEM(mod->ctx), -1);

            for (u = 0; u < new->info.bits.count; u++) {
                new->info.bits.bit[u].name = lydict_insert(mod->ctx, old->info.bits.bit[u].name, 0);
                new->info.bits.bit[u].dsc = lydict_insert(mod->ctx, old->info.bits.bit[u].dsc, 0);
                new->info.bits.bit[u].ref = lydict_insert(mod->ctx, old->info.bits.bit[u].ref, 0);
                new->info.bits.bit[u].flags = old->info.bits.bit[u].flags;
                new->info.bits.bit[u].pos = old->info.bits.bit[u].pos;
                new->info.bits.bit[u].ext_size = old->info.bits.bit[u].ext_size;
                if (lys_ext_dup(mod->ctx, mod, old->info.bits.bit[u].ext, old->info.bits.bit[u].ext_size,
                                &new->info.bits.bit[u], LYEXT_PAR_TYPE_BIT,
                                &new->info.bits.bit[u].ext, shallow, unres)) {
                    return -1;
                }
            }
        }
        break;

    case LY_TYPE_DEC64:
        new->info.dec64.dig = old->info.dec64.dig;
        new->info.dec64.div = old->info.dec64.div;
        if (old->info.dec64.range) {
            new->info.dec64.range = lys_restr_dup(mod, old->info.dec64.range, 1, shallow, unres);
        }
        break;

    case LY_TYPE_ENUM:
        new->info.enums.count = old->info.enums.count;
        if (new->info.enums.count) {
            new->info.enums.enm = calloc(new->info.enums.count, sizeof *new->info.enums.enm);
            LY_CHECK_ERR_RETURN(!new->info.enums.enm, LOGMEM(mod->ctx), -1);

            for (u = 0; u < new->info.enums.count; u++) {
                new->info.enums.enm[u].name = lydict_insert(mod->ctx, old->info.enums.enm[u].name, 0);
                new->info.enums.enm[u].dsc = lydict_insert(mod->ctx, old->info.enums.enm[u].dsc, 0);
                new->info.enums.enm[u].ref = lydict_insert(mod->ctx, old->info.enums.enm[u].ref, 0);
                new->info.enums.enm[u].flags = old->info.enums.enm[u].flags;
                new->info.enums.enm[u].value = old->info.enums.enm[u].value;
                new->info.enums.enm[u].ext_size = old->info.enums.enm[u].ext_size;
                if (lys_ext_dup(mod->ctx, mod, old->info.enums.enm[u].ext, old->info.enums.enm[u].ext_size,
                                &new->info.enums.enm[u], LYEXT_PAR_TYPE_ENUM,
                                &new->info.enums.enm[u].ext, shallow, unres)) {
                    return -1;
                }
            }
        }
        break;

    case LY_TYPE_IDENT:
        new->info.ident.count = old->info.ident.count;
        if (old->info.ident.count) {
            new->info.ident.ref = malloc(old->info.ident.count * sizeof *new->info.ident.ref);
            LY_CHECK_ERR_RETURN(!new->info.ident.ref, LOGMEM(mod->ctx), -1);
            memcpy(new->info.ident.ref, old->info.ident.ref, old->info.ident.count * sizeof *new->info.ident.ref);
        } else {
            /* there can be several unresolved base identities, duplicate them all */
            i = -1;
            do {
                i = unres_schema_find(unres, i, old, UNRES_TYPE_IDENTREF);
                if (i != -1) {
                    if (unres_schema_add_str(mod, unres, new, UNRES_TYPE_IDENTREF, unres->str_snode[i]) == -1) {
                        return -1;
                    }
                }
                --i;
            } while (i > -1);
        }
        break;

    case LY_TYPE_INST:
        new->info.inst.req = old->info.inst.req;
        break;

    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
        if (old->info.num.range) {
            new->info.num.range = lys_restr_dup(mod, old->info.num.range, 1, shallow, unres);
        }
        break;

    case LY_TYPE_LEAFREF:
        if (old->info.lref.path) {
            new->info.lref.path = lydict_insert(mod->ctx, old->info.lref.path, 0);
            new->info.lref.req = old->info.lref.req;
            if (!in_grp && unres_schema_add_node(mod, unres, new, UNRES_TYPE_LEAFREF, parent) == -1) {
                return -1;
            }
        }
        break;

    case LY_TYPE_STRING:
        if (old->info.str.length) {
            new->info.str.length = lys_restr_dup(mod, old->info.str.length, 1, shallow, unres);
        }
        if (old->info.str.pat_count) {
            new->info.str.patterns = lys_restr_dup(mod, old->info.str.patterns, old->info.str.pat_count, shallow, unres);
            new->info.str.pat_count = old->info.str.pat_count;
#ifdef LY_ENABLED_CACHE
            if (!in_grp) {
                new->info.str.patterns_pcre = malloc(new->info.str.pat_count * 2 * sizeof *new->info.str.patterns_pcre);
                LY_CHECK_ERR_RETURN(!new->info.str.patterns_pcre, LOGMEM(mod->ctx), -1);
                for (u = 0; u < new->info.str.pat_count; u++) {
                    if (lyp_precompile_pattern(mod->ctx, &new->info.str.patterns[u].expr[1],
                                              (pcre**)&new->info.str.patterns_pcre[2 * u],
                                              (pcre_extra**)&new->info.str.patterns_pcre[2 * u + 1])) {
                        free(new->info.str.patterns_pcre);
                        new->info.str.patterns_pcre = NULL;
                        return -1;
                    }
                }
            }
#endif
        }
        break;

    case LY_TYPE_UNION:
        new->info.uni.has_ptr_type = old->info.uni.has_ptr_type;
        new->info.uni.count = old->info.uni.count;
        if (new->info.uni.count) {
            new->info.uni.types = calloc(new->info.uni.count, sizeof *new->info.uni.types);
            LY_CHECK_ERR_RETURN(!new->info.uni.types, LOGMEM(mod->ctx), -1);

            for (u = 0; u < new->info.uni.count; u++) {
                if (lys_type_dup(mod, parent, &(new->info.uni.types[u]), &(old->info.uni.types[u]), in_grp,
                        shallow, unres)) {
                    return -1;
                }
            }
        }
        break;

    default:
        /* nothing to do for LY_TYPE_BOOL, LY_TYPE_EMPTY */
        break;
    }

    return EXIT_SUCCESS;
}

struct yang_type *
lys_yang_type_dup(struct lys_module *module, struct lys_node *parent, struct yang_type *old, struct lys_type *type,
                  int in_grp, int shallow, struct unres_schema *unres)
{
    struct yang_type *new;

    new = calloc(1, sizeof *new);
    LY_CHECK_ERR_RETURN(!new, LOGMEM(module->ctx), NULL);
    new->flags = old->flags;
    new->base = old->base;
    new->name = lydict_insert(module->ctx, old->name, 0);
    new->type = type;
    if (!new->name) {
        LOGMEM(module->ctx);
        goto error;
    }
    if (type_dup(module, parent, type, old->type, new->base, in_grp, shallow, unres)) {
        new->type->base = new->base;
        lys_type_free(module->ctx, new->type, NULL);
        memset(&new->type->info, 0, sizeof new->type->info);
        goto error;
    }
    return new;

error:
    free(new);
    return NULL;
}

int
lys_copy_union_leafrefs(struct lys_module *mod, struct lys_node *parent, struct lys_type *type, struct lys_type *prev_new,
                        struct unres_schema *unres)
{
    struct lys_type new;
    unsigned int i, top_type;
    struct lys_ext_instance **ext;
    uint8_t ext_size;
    void *reloc;

    if (!prev_new) {
        /* this is the "top-level" type, meaning it is a real type and no typedef directly above */
        top_type = 1;

        memset(&new, 0, sizeof new);

        new.base = type->base;
        new.parent = (struct lys_tpdf *)parent;

        prev_new = &new;
    } else {
        /* this is not top-level type, just a type of a typedef */
        top_type = 0;
    }

    assert(type->der);
    if (type->der->module) {
        /* typedef, skip it, but keep the extensions */
        ext_size = type->ext_size;
        if (lys_ext_dup(mod->ctx, mod, type->ext, type->ext_size, prev_new, LYEXT_PAR_TYPE, &ext, 0, unres)) {
            return -1;
        }
        if (prev_new->ext) {
            reloc = realloc(prev_new->ext, (prev_new->ext_size + ext_size) * sizeof *prev_new->ext);
            LY_CHECK_ERR_RETURN(!reloc, LOGMEM(mod->ctx), -1);
            prev_new->ext = reloc;

            memcpy(prev_new->ext + prev_new->ext_size, ext, ext_size * sizeof *ext);
            free(ext);

            prev_new->ext_size += ext_size;
        } else {
            prev_new->ext = ext;
            prev_new->ext_size = ext_size;
        }

        if (lys_copy_union_leafrefs(mod, parent, &type->der->type, prev_new, unres)) {
            return -1;
        }
    } else {
        /* type, just make a deep copy */
        switch (type->base) {
        case LY_TYPE_UNION:
            prev_new->info.uni.has_ptr_type = type->info.uni.has_ptr_type;
            prev_new->info.uni.count = type->info.uni.count;
            /* this cannot be a typedef anymore */
            assert(prev_new->info.uni.count);

            prev_new->info.uni.types = calloc(prev_new->info.uni.count, sizeof *prev_new->info.uni.types);
            LY_CHECK_ERR_RETURN(!prev_new->info.uni.types, LOGMEM(mod->ctx), -1);

            for (i = 0; i < prev_new->info.uni.count; i++) {
                if (lys_copy_union_leafrefs(mod, parent, &(type->info.uni.types[i]), &(prev_new->info.uni.types[i]), unres)) {
                    return -1;
                }
            }

            prev_new->der = type->der;
            break;
        default:
            if (lys_type_dup(mod, parent, prev_new, type, 0, 0, unres)) {
                return -1;
            }
            break;
        }
    }

    if (top_type) {
        memcpy(type, prev_new, sizeof *type);
    }
    return EXIT_SUCCESS;
}

API const void *
lys_ext_instance_substmt(const struct lys_ext_instance *ext)
{
    if (!ext) {
        return NULL;
    }

    switch (ext->insubstmt) {
    case LYEXT_SUBSTMT_SELF:
    case LYEXT_SUBSTMT_MODIFIER:
    case LYEXT_SUBSTMT_VERSION:
        return NULL;
    case LYEXT_SUBSTMT_ARGUMENT:
        if (ext->parent_type == LYEXT_PAR_EXT) {
            return ((struct lys_ext_instance*)ext->parent)->arg_value;
        }
        break;
    case LYEXT_SUBSTMT_BASE:
        if (ext->parent_type == LYEXT_PAR_TYPE) {
            return ((struct lys_type*)ext->parent)->info.ident.ref[ext->insubstmt_index];
        } else if (ext->parent_type == LYEXT_PAR_IDENT) {
            return ((struct lys_ident*)ext->parent)->base[ext->insubstmt_index];
        }
        break;
    case LYEXT_SUBSTMT_BELONGSTO:
        if (ext->parent_type == LYEXT_PAR_MODULE && ((struct lys_module*)ext->parent)->type) {
            return ((struct lys_submodule*)ext->parent)->belongsto;
        }
        break;
    case LYEXT_SUBSTMT_CONFIG:
    case LYEXT_SUBSTMT_MANDATORY:
        if (ext->parent_type == LYEXT_PAR_NODE) {
            return &((struct lys_node*)ext->parent)->flags;
        } else if (ext->parent_type == LYEXT_PAR_DEVIATE) {
            return &((struct lys_deviate*)ext->parent)->flags;
        } else if (ext->parent_type == LYEXT_PAR_REFINE) {
            return &((struct lys_refine*)ext->parent)->flags;
        }
        break;
    case LYEXT_SUBSTMT_CONTACT:
        if (ext->parent_type == LYEXT_PAR_MODULE) {
            return ((struct lys_module*)ext->parent)->contact;
        }
        break;
    case LYEXT_SUBSTMT_DEFAULT:
        if (ext->parent_type == LYEXT_PAR_NODE) {
            switch (((struct lys_node*)ext->parent)->nodetype) {
            case LYS_LEAF:
            case LYS_LEAFLIST:
                /* in case of leaf, the index is supposed to be 0, so it will return the
                 * correct pointer despite the leaf structure does not have dflt as array */
                return ((struct lys_node_leaflist*)ext->parent)->dflt[ext->insubstmt_index];
            case LYS_CHOICE:
                return ((struct lys_node_choice*)ext->parent)->dflt;
            default:
                /* internal error */
                break;
            }
        } else if (ext->parent_type == LYEXT_PAR_TPDF) {
            return ((struct lys_tpdf*)ext->parent)->dflt;
        } else if (ext->parent_type == LYEXT_PAR_DEVIATE) {
            return ((struct lys_deviate*)ext->parent)->dflt[ext->insubstmt_index];
        } else if (ext->parent_type == LYEXT_PAR_REFINE) {
            return &((struct lys_refine*)ext->parent)->dflt[ext->insubstmt_index];
        }
        break;
    case LYEXT_SUBSTMT_DESCRIPTION:
        switch (ext->parent_type) {
        case LYEXT_PAR_NODE:
            return ((struct lys_node*)ext->parent)->dsc;
        case LYEXT_PAR_MODULE:
            return ((struct lys_module*)ext->parent)->dsc;
        case LYEXT_PAR_IMPORT:
            return ((struct lys_import*)ext->parent)->dsc;
        case LYEXT_PAR_INCLUDE:
            return ((struct lys_include*)ext->parent)->dsc;
        case LYEXT_PAR_EXT:
            return ((struct lys_ext*)ext->parent)->dsc;
        case LYEXT_PAR_FEATURE:
            return ((struct lys_feature*)ext->parent)->dsc;
        case LYEXT_PAR_TPDF:
            return ((struct lys_tpdf*)ext->parent)->dsc;
        case LYEXT_PAR_TYPE_BIT:
            return ((struct lys_type_bit*)ext->parent)->dsc;
        case LYEXT_PAR_TYPE_ENUM:
            return ((struct lys_type_enum*)ext->parent)->dsc;
        case LYEXT_PAR_RESTR:
            return ((struct lys_restr*)ext->parent)->dsc;
        case LYEXT_PAR_WHEN:
            return ((struct lys_when*)ext->parent)->dsc;
        case LYEXT_PAR_IDENT:
            return ((struct lys_ident*)ext->parent)->dsc;
        case LYEXT_PAR_DEVIATION:
            return ((struct lys_deviation*)ext->parent)->dsc;
        case LYEXT_PAR_REVISION:
            return ((struct lys_revision*)ext->parent)->dsc;
        case LYEXT_PAR_REFINE:
            return ((struct lys_refine*)ext->parent)->dsc;
        default:
            break;
        }
        break;
    case LYEXT_SUBSTMT_ERRTAG:
        if (ext->parent_type == LYEXT_PAR_RESTR) {
            return ((struct lys_restr*)ext->parent)->eapptag;
        }
        break;
    case LYEXT_SUBSTMT_ERRMSG:
        if (ext->parent_type == LYEXT_PAR_RESTR) {
            return ((struct lys_restr*)ext->parent)->emsg;
        }
        break;
    case LYEXT_SUBSTMT_DIGITS:
        if (ext->parent_type == LYEXT_PAR_TYPE && ((struct lys_type*)ext->parent)->base == LY_TYPE_DEC64) {
            return &((struct lys_type*)ext->parent)->info.dec64.dig;
        }
        break;
    case LYEXT_SUBSTMT_KEY:
        if (ext->parent_type == LYEXT_PAR_NODE && ((struct lys_node*)ext->parent)->nodetype == LYS_LIST) {
            return ((struct lys_node_list*)ext->parent)->keys;
        }
        break;
    case LYEXT_SUBSTMT_MAX:
        if (ext->parent_type == LYEXT_PAR_NODE) {
            if (((struct lys_node*)ext->parent)->nodetype == LYS_LIST) {
                return &((struct lys_node_list*)ext->parent)->max;
            } else if (((struct lys_node*)ext->parent)->nodetype == LYS_LEAFLIST) {
                return &((struct lys_node_leaflist*)ext->parent)->max;
            }
        } else if (ext->parent_type == LYEXT_PAR_REFINE) {
            return &((struct lys_refine*)ext->parent)->mod.list.max;
        }
        break;
    case LYEXT_SUBSTMT_MIN:
        if (ext->parent_type == LYEXT_PAR_NODE) {
            if (((struct lys_node*)ext->parent)->nodetype == LYS_LIST) {
                return &((struct lys_node_list*)ext->parent)->min;
            } else if (((struct lys_node*)ext->parent)->nodetype == LYS_LEAFLIST) {
                return &((struct lys_node_leaflist*)ext->parent)->min;
            }
        } else if (ext->parent_type == LYEXT_PAR_REFINE) {
            return &((struct lys_refine*)ext->parent)->mod.list.min;
        }
        break;
    case LYEXT_SUBSTMT_NAMESPACE:
        if (ext->parent_type == LYEXT_PAR_MODULE && !((struct lys_module*)ext->parent)->type) {
            return ((struct lys_module*)ext->parent)->ns;
        }
        break;
    case LYEXT_SUBSTMT_ORDEREDBY:
        if (ext->parent_type == LYEXT_PAR_NODE &&
                (((struct lys_node*)ext->parent)->nodetype & (LYS_LIST | LYS_LEAFLIST))) {
            return &((struct lys_node_list*)ext->parent)->flags;
        }
        break;
    case LYEXT_SUBSTMT_ORGANIZATION:
        if (ext->parent_type == LYEXT_PAR_MODULE) {
            return ((struct lys_module*)ext->parent)->org;
        }
        break;
    case LYEXT_SUBSTMT_PATH:
        if (ext->parent_type == LYEXT_PAR_TYPE && ((struct lys_type*)ext->parent)->base == LY_TYPE_LEAFREF) {
            return ((struct lys_type*)ext->parent)->info.lref.path;
        }
        break;
    case LYEXT_SUBSTMT_POSITION:
        if (ext->parent_type == LYEXT_PAR_TYPE_BIT) {
            return &((struct lys_type_bit*)ext->parent)->pos;
        }
        break;
    case LYEXT_SUBSTMT_PREFIX:
        if (ext->parent_type == LYEXT_PAR_MODULE) {
            /* covers also lys_submodule */
            return ((struct lys_module*)ext->parent)->prefix;
        } else if (ext->parent_type == LYEXT_PAR_IMPORT) {
            return ((struct lys_import*)ext->parent)->prefix;
        }
        break;
    case LYEXT_SUBSTMT_PRESENCE:
        if (ext->parent_type == LYEXT_PAR_NODE && ((struct lys_node*)ext->parent)->nodetype == LYS_CONTAINER) {
            return ((struct lys_node_container*)ext->parent)->presence;
        } else if (ext->parent_type == LYEXT_PAR_REFINE) {
            return ((struct lys_refine*)ext->parent)->mod.presence;
        }
        break;
    case LYEXT_SUBSTMT_REFERENCE:
        switch (ext->parent_type) {
        case LYEXT_PAR_NODE:
            return ((struct lys_node*)ext->parent)->ref;
        case LYEXT_PAR_MODULE:
            return ((struct lys_module*)ext->parent)->ref;
        case LYEXT_PAR_IMPORT:
            return ((struct lys_import*)ext->parent)->ref;
        case LYEXT_PAR_INCLUDE:
            return ((struct lys_include*)ext->parent)->ref;
        case LYEXT_PAR_EXT:
            return ((struct lys_ext*)ext->parent)->ref;
        case LYEXT_PAR_FEATURE:
            return ((struct lys_feature*)ext->parent)->ref;
        case LYEXT_PAR_TPDF:
            return ((struct lys_tpdf*)ext->parent)->ref;
        case LYEXT_PAR_TYPE_BIT:
            return ((struct lys_type_bit*)ext->parent)->ref;
        case LYEXT_PAR_TYPE_ENUM:
            return ((struct lys_type_enum*)ext->parent)->ref;
        case LYEXT_PAR_RESTR:
            return ((struct lys_restr*)ext->parent)->ref;
        case LYEXT_PAR_WHEN:
            return ((struct lys_when*)ext->parent)->ref;
        case LYEXT_PAR_IDENT:
            return ((struct lys_ident*)ext->parent)->ref;
        case LYEXT_PAR_DEVIATION:
            return ((struct lys_deviation*)ext->parent)->ref;
        case LYEXT_PAR_REVISION:
            return ((struct lys_revision*)ext->parent)->ref;
        case LYEXT_PAR_REFINE:
            return ((struct lys_refine*)ext->parent)->ref;
        default:
            break;
        }
        break;
    case LYEXT_SUBSTMT_REQINSTANCE:
        if (ext->parent_type == LYEXT_PAR_TYPE) {
            if (((struct lys_type*)ext->parent)->base == LY_TYPE_LEAFREF) {
                return &((struct lys_type*)ext->parent)->info.lref.req;
            } else if (((struct lys_type*)ext->parent)->base == LY_TYPE_INST) {
                return &((struct lys_type*)ext->parent)->info.inst.req;
            }
        }
        break;
    case LYEXT_SUBSTMT_REVISIONDATE:
        if (ext->parent_type == LYEXT_PAR_IMPORT) {
            return ((struct lys_import*)ext->parent)->rev;
        } else if (ext->parent_type == LYEXT_PAR_INCLUDE) {
            return ((struct lys_include*)ext->parent)->rev;
        }
        break;
    case LYEXT_SUBSTMT_STATUS:
        switch (ext->parent_type) {
        case LYEXT_PAR_NODE:
        case LYEXT_PAR_IDENT:
        case LYEXT_PAR_TPDF:
        case LYEXT_PAR_EXT:
        case LYEXT_PAR_FEATURE:
        case LYEXT_PAR_TYPE_ENUM:
        case LYEXT_PAR_TYPE_BIT:
            /* in all structures the flags member is at the same offset */
            return &((struct lys_node*)ext->parent)->flags;
        default:
            break;
        }
        break;
    case LYEXT_SUBSTMT_UNIQUE:
        if (ext->parent_type == LYEXT_PAR_DEVIATE) {
            return &((struct lys_deviate*)ext->parent)->unique[ext->insubstmt_index];
        } else if (ext->parent_type == LYEXT_PAR_NODE && ((struct lys_node*)ext->parent)->nodetype == LYS_LIST) {
            return &((struct lys_node_list*)ext->parent)->unique[ext->insubstmt_index];
        }
        break;
    case LYEXT_SUBSTMT_UNITS:
        if (ext->parent_type == LYEXT_PAR_NODE &&
                (((struct lys_node*)ext->parent)->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            /* units is at the same offset in both lys_node_leaf and lys_node_leaflist */
            return ((struct lys_node_leaf*)ext->parent)->units;
        } else if (ext->parent_type == LYEXT_PAR_TPDF) {
            return ((struct lys_tpdf*)ext->parent)->units;
        } else if (ext->parent_type == LYEXT_PAR_DEVIATE) {
            return ((struct lys_deviate*)ext->parent)->units;
        }
        break;
    case LYEXT_SUBSTMT_VALUE:
        if (ext->parent_type == LYEXT_PAR_TYPE_ENUM) {
            return &((struct lys_type_enum*)ext->parent)->value;
        }
        break;
    case LYEXT_SUBSTMT_YINELEM:
        if (ext->parent_type == LYEXT_PAR_EXT) {
            return &((struct lys_ext*)ext->parent)->flags;
        }
        break;
    }
    LOGINT(ext->module->ctx);
    return NULL;
}

static int
lys_type_dup(struct lys_module *mod, struct lys_node *parent, struct lys_type *new, struct lys_type *old,
            int in_grp, int shallow, struct unres_schema *unres)
{
    int i;

    new->base = old->base;
    new->der = old->der;
    new->parent = (struct lys_tpdf *)parent;
    new->ext_size = old->ext_size;
    if (lys_ext_dup(mod->ctx, mod, old->ext, old->ext_size, new, LYEXT_PAR_TYPE, &new->ext, shallow, unres)) {
        return -1;
    }

    i = unres_schema_find(unres, -1, old, UNRES_TYPE_DER);
    if (i != -1) {
        /* HACK (serious one) for unres */
        /* nothing else we can do but duplicate it immediately */
        if (((struct lyxml_elem *)old->der)->flags & LY_YANG_STRUCTURE_FLAG) {
            new->der = (struct lys_tpdf *)lys_yang_type_dup(mod, parent, (struct yang_type *)old->der, new, in_grp,
                                                            shallow, unres);
        } else {
            new->der = (struct lys_tpdf *)lyxml_dup_elem(mod->ctx, (struct lyxml_elem *)old->der, NULL, 1);
        }
        /* all these unres additions can fail even though they did not before */
        if (!new->der || (unres_schema_add_node(mod, unres, new, UNRES_TYPE_DER, parent) == -1)) {
            return -1;
        }
        return EXIT_SUCCESS;
    }

    return type_dup(mod, parent, new, old, new->base, in_grp, shallow, unres);
}

void
lys_type_free(struct ly_ctx *ctx, struct lys_type *type,
              void (*private_destructor)(const struct lys_node *node, void *priv))
{
    unsigned int i;

    assert(ctx);
    if (!type) {
        return;
    }

    lys_extension_instances_free(ctx, type->ext, type->ext_size, private_destructor);

    switch (type->base) {
    case LY_TYPE_BINARY:
        lys_restr_free(ctx, type->info.binary.length, private_destructor);
        free(type->info.binary.length);
        break;
    case LY_TYPE_BITS:
        for (i = 0; i < type->info.bits.count; i++) {
            lydict_remove(ctx, type->info.bits.bit[i].name);
            lydict_remove(ctx, type->info.bits.bit[i].dsc);
            lydict_remove(ctx, type->info.bits.bit[i].ref);
            lys_iffeature_free(ctx, type->info.bits.bit[i].iffeature, type->info.bits.bit[i].iffeature_size, 0,
                               private_destructor);
            lys_extension_instances_free(ctx, type->info.bits.bit[i].ext, type->info.bits.bit[i].ext_size,
                                         private_destructor);
        }
        free(type->info.bits.bit);
        break;

    case LY_TYPE_DEC64:
        lys_restr_free(ctx, type->info.dec64.range, private_destructor);
        free(type->info.dec64.range);
        break;

    case LY_TYPE_ENUM:
        for (i = 0; i < type->info.enums.count; i++) {
            lydict_remove(ctx, type->info.enums.enm[i].name);
            lydict_remove(ctx, type->info.enums.enm[i].dsc);
            lydict_remove(ctx, type->info.enums.enm[i].ref);
            lys_iffeature_free(ctx, type->info.enums.enm[i].iffeature, type->info.enums.enm[i].iffeature_size, 0,
                               private_destructor);
            lys_extension_instances_free(ctx, type->info.enums.enm[i].ext, type->info.enums.enm[i].ext_size,
                                         private_destructor);
        }
        free(type->info.enums.enm);
        break;

    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
        lys_restr_free(ctx, type->info.num.range, private_destructor);
        free(type->info.num.range);
        break;

    case LY_TYPE_LEAFREF:
        lydict_remove(ctx, type->info.lref.path);
        break;

    case LY_TYPE_STRING:
        lys_restr_free(ctx, type->info.str.length, private_destructor);
        free(type->info.str.length);
        for (i = 0; i < type->info.str.pat_count; i++) {
            lys_restr_free(ctx, &type->info.str.patterns[i], private_destructor);
#ifdef LY_ENABLED_CACHE
            if (type->info.str.patterns_pcre) {
                pcre_free((pcre*)type->info.str.patterns_pcre[2 * i]);
                pcre_free_study((pcre_extra*)type->info.str.patterns_pcre[2 * i + 1]);
            }
#endif
        }
        free(type->info.str.patterns);
#ifdef LY_ENABLED_CACHE
        free(type->info.str.patterns_pcre);
#endif
        break;

    case LY_TYPE_UNION:
        for (i = 0; i < type->info.uni.count; i++) {
            lys_type_free(ctx, &type->info.uni.types[i], private_destructor);
        }
        free(type->info.uni.types);
        break;

    case LY_TYPE_IDENT:
        free(type->info.ident.ref);
        break;

    default:
        /* nothing to do for LY_TYPE_INST, LY_TYPE_BOOL, LY_TYPE_EMPTY */
        break;
    }
}

static void
lys_tpdf_free(struct ly_ctx *ctx, struct lys_tpdf *tpdf,
              void (*private_destructor)(const struct lys_node *node, void *priv))
{
    assert(ctx);
    if (!tpdf) {
        return;
    }

    lydict_remove(ctx, tpdf->name);
    lydict_remove(ctx, tpdf->dsc);
    lydict_remove(ctx, tpdf->ref);

    lys_type_free(ctx, &tpdf->type, private_destructor);

    lydict_remove(ctx, tpdf->units);
    lydict_remove(ctx, tpdf->dflt);

    lys_extension_instances_free(ctx, tpdf->ext, tpdf->ext_size, private_destructor);
}

static struct lys_when *
lys_when_dup(struct lys_module *mod, struct lys_when *old, int shallow, struct unres_schema *unres)
{
    struct lys_when *new;

    if (!old) {
        return NULL;
    }

    new = calloc(1, sizeof *new);
    LY_CHECK_ERR_RETURN(!new, LOGMEM(mod->ctx), NULL);
    new->cond = lydict_insert(mod->ctx, old->cond, 0);
    new->dsc = lydict_insert(mod->ctx, old->dsc, 0);
    new->ref = lydict_insert(mod->ctx, old->ref, 0);
    new->ext_size = old->ext_size;
    lys_ext_dup(mod->ctx, mod, old->ext, old->ext_size, new, LYEXT_PAR_WHEN, &new->ext, shallow, unres);

    return new;
}

void
lys_when_free(struct ly_ctx *ctx, struct lys_when *w,
              void (*private_destructor)(const struct lys_node *node, void *priv))
{
    if (!w) {
        return;
    }

    lys_extension_instances_free(ctx, w->ext, w->ext_size, private_destructor);
    lydict_remove(ctx, w->cond);
    lydict_remove(ctx, w->dsc);
    lydict_remove(ctx, w->ref);

    free(w);
}

static void
lys_augment_free(struct ly_ctx *ctx, struct lys_node_augment *aug,
                 void (*private_destructor)(const struct lys_node *node, void *priv))
{
    struct lys_node *next, *sub;

    /* children from a resolved augment are freed under the target node */
    if (!aug->target || (aug->flags & LYS_NOTAPPLIED)) {
        LY_TREE_FOR_SAFE(aug->child, next, sub) {
            lys_node_free(sub, private_destructor, 0);
        }
    }

    lydict_remove(ctx, aug->target_name);
    lydict_remove(ctx, aug->dsc);
    lydict_remove(ctx, aug->ref);

    lys_iffeature_free(ctx, aug->iffeature, aug->iffeature_size, 0, private_destructor);
    lys_extension_instances_free(ctx, aug->ext, aug->ext_size, private_destructor);

    lys_when_free(ctx, aug->when, private_destructor);
}

static void
lys_ident_free(struct ly_ctx *ctx, struct lys_ident *ident,
               void (*private_destructor)(const struct lys_node *node, void *priv))
{
    assert(ctx);
    if (!ident) {
        return;
    }

    free(ident->base);
    ly_set_free(ident->der);
    lydict_remove(ctx, ident->name);
    lydict_remove(ctx, ident->dsc);
    lydict_remove(ctx, ident->ref);
    lys_iffeature_free(ctx, ident->iffeature, ident->iffeature_size, 0, private_destructor);
    lys_extension_instances_free(ctx, ident->ext, ident->ext_size, private_destructor);

}

static void
lys_grp_free(struct ly_ctx *ctx, struct lys_node_grp *grp,
             void (*private_destructor)(const struct lys_node *node, void *priv))
{
    int i;

    /* handle only specific parts for LYS_GROUPING */
    for (i = 0; i < grp->tpdf_size; i++) {
        lys_tpdf_free(ctx, &grp->tpdf[i], private_destructor);
    }
    free(grp->tpdf);
}

static void
lys_rpc_action_free(struct ly_ctx *ctx, struct lys_node_rpc_action *rpc_act,
             void (*private_destructor)(const struct lys_node *node, void *priv))
{
    int i;

    /* handle only specific parts for LYS_GROUPING */
    for (i = 0; i < rpc_act->tpdf_size; i++) {
        lys_tpdf_free(ctx, &rpc_act->tpdf[i], private_destructor);
    }
    free(rpc_act->tpdf);
}

static void
lys_inout_free(struct ly_ctx *ctx, struct lys_node_inout *io,
               void (*private_destructor)(const struct lys_node *node, void *priv))
{
    int i;

    /* handle only specific parts for LYS_INPUT and LYS_OUTPUT */
    for (i = 0; i < io->tpdf_size; i++) {
        lys_tpdf_free(ctx, &io->tpdf[i], private_destructor);
    }
    free(io->tpdf);

    for (i = 0; i < io->must_size; i++) {
        lys_restr_free(ctx, &io->must[i], private_destructor);
    }
    free(io->must);
}

static void
lys_notif_free(struct ly_ctx *ctx, struct lys_node_notif *notif,
               void (*private_destructor)(const struct lys_node *node, void *priv))
{
    int i;

    for (i = 0; i < notif->must_size; i++) {
        lys_restr_free(ctx, &notif->must[i], private_destructor);
    }
    free(notif->must);

    for (i = 0; i < notif->tpdf_size; i++) {
        lys_tpdf_free(ctx, &notif->tpdf[i], private_destructor);
    }
    free(notif->tpdf);
}
static void
lys_anydata_free(struct ly_ctx *ctx, struct lys_node_anydata *anyxml,
                 void (*private_destructor)(const struct lys_node *node, void *priv))
{
    int i;

    for (i = 0; i < anyxml->must_size; i++) {
        lys_restr_free(ctx, &anyxml->must[i], private_destructor);
    }
    free(anyxml->must);

    lys_when_free(ctx, anyxml->when, private_destructor);
}

static void
lys_leaf_free(struct ly_ctx *ctx, struct lys_node_leaf *leaf,
              void (*private_destructor)(const struct lys_node *node, void *priv))
{
    int i;

    /* leafref backlinks */
    ly_set_free((struct ly_set *)leaf->backlinks);

    for (i = 0; i < leaf->must_size; i++) {
        lys_restr_free(ctx, &leaf->must[i], private_destructor);
    }
    free(leaf->must);

    lys_when_free(ctx, leaf->when, private_destructor);

    lys_type_free(ctx, &leaf->type, private_destructor);
    lydict_remove(ctx, leaf->units);
    lydict_remove(ctx, leaf->dflt);
}

static void
lys_leaflist_free(struct ly_ctx *ctx, struct lys_node_leaflist *llist,
                  void (*private_destructor)(const struct lys_node *node, void *priv))
{
    int i;

    if (llist->backlinks) {
        /* leafref backlinks */
        ly_set_free(llist->backlinks);
    }

    for (i = 0; i < llist->must_size; i++) {
        lys_restr_free(ctx, &llist->must[i], private_destructor);
    }
    free(llist->must);

    for (i = 0; i < llist->dflt_size; i++) {
        lydict_remove(ctx, llist->dflt[i]);
    }
    free(llist->dflt);

    lys_when_free(ctx, llist->when, private_destructor);

    lys_type_free(ctx, &llist->type, private_destructor);
    lydict_remove(ctx, llist->units);
}

static void
lys_list_free(struct ly_ctx *ctx, struct lys_node_list *list,
              void (*private_destructor)(const struct lys_node *node, void *priv))
{
    int i, j;

    /* handle only specific parts for LY_NODE_LIST */
    lys_when_free(ctx, list->when, private_destructor);

    for (i = 0; i < list->must_size; i++) {
        lys_restr_free(ctx, &list->must[i], private_destructor);
    }
    free(list->must);

    for (i = 0; i < list->tpdf_size; i++) {
        lys_tpdf_free(ctx, &list->tpdf[i], private_destructor);
    }
    free(list->tpdf);

    free(list->keys);

    for (i = 0; i < list->unique_size; i++) {
        for (j = 0; j < list->unique[i].expr_size; j++) {
            lydict_remove(ctx, list->unique[i].expr[j]);
        }
        free(list->unique[i].expr);
    }
    free(list->unique);

    lydict_remove(ctx, list->keys_str);
}

static void
lys_container_free(struct ly_ctx *ctx, struct lys_node_container *cont,
                   void (*private_destructor)(const struct lys_node *node, void *priv))
{
    int i;

    /* handle only specific parts for LY_NODE_CONTAINER */
    lydict_remove(ctx, cont->presence);

    for (i = 0; i < cont->tpdf_size; i++) {
        lys_tpdf_free(ctx, &cont->tpdf[i], private_destructor);
    }
    free(cont->tpdf);

    for (i = 0; i < cont->must_size; i++) {
        lys_restr_free(ctx, &cont->must[i], private_destructor);
    }
    free(cont->must);

    lys_when_free(ctx, cont->when, private_destructor);
}

static void
lys_feature_free(struct ly_ctx *ctx, struct lys_feature *f,
                 void (*private_destructor)(const struct lys_node *node, void *priv))
{
    lydict_remove(ctx, f->name);
    lydict_remove(ctx, f->dsc);
    lydict_remove(ctx, f->ref);
    lys_iffeature_free(ctx, f->iffeature, f->iffeature_size, 0, private_destructor);
    ly_set_free(f->depfeatures);
    lys_extension_instances_free(ctx, f->ext, f->ext_size, private_destructor);
}

static void
lys_extension_free(struct ly_ctx *ctx, struct lys_ext *e,
                   void (*private_destructor)(const struct lys_node *node, void *priv))
{
    lydict_remove(ctx, e->name);
    lydict_remove(ctx, e->dsc);
    lydict_remove(ctx, e->ref);
    lydict_remove(ctx, e->argument);
    lys_extension_instances_free(ctx, e->ext, e->ext_size, private_destructor);
}

static void
lys_deviation_free(struct lys_module *module, struct lys_deviation *dev,
                   void (*private_destructor)(const struct lys_node *node, void *priv))
{
    int i, j, k;
    struct ly_ctx *ctx;
    struct lys_node *next, *elem;

    ctx = module->ctx;

    lydict_remove(ctx, dev->target_name);
    lydict_remove(ctx, dev->dsc);
    lydict_remove(ctx, dev->ref);
    lys_extension_instances_free(ctx, dev->ext, dev->ext_size, private_destructor);

    if (!dev->deviate) {
        return;
    }

    /* it could not be applied because it failed to be applied */
    if (dev->orig_node) {
        /* the module was freed, but we only need the context from orig_node, use ours */
        if (dev->deviate[0].mod == LY_DEVIATE_NO) {
            /* it's actually a node subtree, we need to update modules on all the nodes :-/ */
            LY_TREE_DFS_BEGIN(dev->orig_node, next, elem) {
                elem->module = module;

                LY_TREE_DFS_END(dev->orig_node, next, elem);
            }
            lys_node_free(dev->orig_node, NULL, 0);
        } else {
            /* it's just a shallow copy, freeing one node */
            dev->orig_node->module = module;
            lys_node_free(dev->orig_node, NULL, 1);
        }
    }

    for (i = 0; i < dev->deviate_size; i++) {
        lys_extension_instances_free(ctx, dev->deviate[i].ext, dev->deviate[i].ext_size, private_destructor);

        for (j = 0; j < dev->deviate[i].dflt_size; j++) {
            lydict_remove(ctx, dev->deviate[i].dflt[j]);
        }
        free(dev->deviate[i].dflt);

        lydict_remove(ctx, dev->deviate[i].units);

        if (dev->deviate[i].mod == LY_DEVIATE_DEL) {
            for (j = 0; j < dev->deviate[i].must_size; j++) {
                lys_restr_free(ctx, &dev->deviate[i].must[j], private_destructor);
            }
            free(dev->deviate[i].must);

            for (j = 0; j < dev->deviate[i].unique_size; j++) {
                for (k = 0; k < dev->deviate[i].unique[j].expr_size; k++) {
                    lydict_remove(ctx, dev->deviate[i].unique[j].expr[k]);
                }
                free(dev->deviate[i].unique[j].expr);
            }
            free(dev->deviate[i].unique);
        }
    }
    free(dev->deviate);
}

static void
lys_uses_free(struct ly_ctx *ctx, struct lys_node_uses *uses,
              void (*private_destructor)(const struct lys_node *node, void *priv))
{
    int i, j;

    for (i = 0; i < uses->refine_size; i++) {
        lydict_remove(ctx, uses->refine[i].target_name);
        lydict_remove(ctx, uses->refine[i].dsc);
        lydict_remove(ctx, uses->refine[i].ref);

        lys_iffeature_free(ctx, uses->refine[i].iffeature, uses->refine[i].iffeature_size, 0, private_destructor);

        for (j = 0; j < uses->refine[i].must_size; j++) {
            lys_restr_free(ctx, &uses->refine[i].must[j], private_destructor);
        }
        free(uses->refine[i].must);

        for (j = 0; j < uses->refine[i].dflt_size; j++) {
            lydict_remove(ctx, uses->refine[i].dflt[j]);
        }
        free(uses->refine[i].dflt);

        lys_extension_instances_free(ctx, uses->refine[i].ext, uses->refine[i].ext_size, private_destructor);

        if (uses->refine[i].target_type & LYS_CONTAINER) {
            lydict_remove(ctx, uses->refine[i].mod.presence);
        }
    }
    free(uses->refine);

    for (i = 0; i < uses->augment_size; i++) {
        lys_augment_free(ctx, &uses->augment[i], private_destructor);
    }
    free(uses->augment);

    lys_when_free(ctx, uses->when, private_destructor);
}

void
lys_node_free(struct lys_node *node, void (*private_destructor)(const struct lys_node *node, void *priv), int shallow)
{
    struct ly_ctx *ctx;
    struct lys_node *sub, *next;

    if (!node) {
        return;
    }

    assert(node->module);
    assert(node->module->ctx);

    ctx = node->module->ctx;

    /* remove private object */
    if (node->priv && private_destructor) {
        private_destructor(node, node->priv);
    }

    /* common part */
    lydict_remove(ctx, node->name);
    if (!(node->nodetype & (LYS_INPUT | LYS_OUTPUT))) {
        lys_iffeature_free(ctx, node->iffeature, node->iffeature_size, shallow, private_destructor);
        lydict_remove(ctx, node->dsc);
        lydict_remove(ctx, node->ref);
    }

    if (!shallow && !(node->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        LY_TREE_FOR_SAFE(node->child, next, sub) {
            lys_node_free(sub, private_destructor, 0);
        }
    }

    lys_extension_instances_free(ctx, node->ext, node->ext_size, private_destructor);

    /* specific part */
    switch (node->nodetype) {
    case LYS_CONTAINER:
        lys_container_free(ctx, (struct lys_node_container *)node, private_destructor);
        break;
    case LYS_CHOICE:
        lys_when_free(ctx, ((struct lys_node_choice *)node)->when, private_destructor);
        break;
    case LYS_LEAF:
        lys_leaf_free(ctx, (struct lys_node_leaf *)node, private_destructor);
        break;
    case LYS_LEAFLIST:
        lys_leaflist_free(ctx, (struct lys_node_leaflist *)node, private_destructor);
        break;
    case LYS_LIST:
        lys_list_free(ctx, (struct lys_node_list *)node, private_destructor);
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        lys_anydata_free(ctx, (struct lys_node_anydata *)node, private_destructor);
        break;
    case LYS_USES:
        lys_uses_free(ctx, (struct lys_node_uses *)node, private_destructor);
        break;
    case LYS_CASE:
        lys_when_free(ctx, ((struct lys_node_case *)node)->when, private_destructor);
        break;
    case LYS_AUGMENT:
        /* do nothing */
        break;
    case LYS_GROUPING:
        lys_grp_free(ctx, (struct lys_node_grp *)node, private_destructor);
        break;
    case LYS_RPC:
    case LYS_ACTION:
        lys_rpc_action_free(ctx, (struct lys_node_rpc_action *)node, private_destructor);
        break;
    case LYS_NOTIF:
        lys_notif_free(ctx, (struct lys_node_notif *)node, private_destructor);
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        lys_inout_free(ctx, (struct lys_node_inout *)node, private_destructor);
        break;
    case LYS_EXT:
    case LYS_UNKNOWN:
        LOGINT(ctx);
        break;
    }

    /* again common part */
    lys_node_unlink(node);
    free(node);
}

API struct lys_module *
lys_implemented_module(const struct lys_module *mod)
{
    struct ly_ctx *ctx;
    int i;

    if (!mod || mod->implemented) {
        /* invalid argument or the module itself is implemented */
        return (struct lys_module *)mod;
    }

    ctx = mod->ctx;
    for (i = 0; i < ctx->models.used; i++) {
        if (!ctx->models.list[i]->implemented) {
            continue;
        }

        if (ly_strequal(mod->name, ctx->models.list[i]->name, 1)) {
            /* we have some revision of the module implemented */
            return ctx->models.list[i];
        }
    }

    /* we have no revision of the module implemented, return the module itself,
     * it is up to the caller to set the module implemented when needed */
    return (struct lys_module *)mod;
}

/* free_int_mods - flag whether to free the internal modules as well */
static void
module_free_common(struct lys_module *module, void (*private_destructor)(const struct lys_node *node, void *priv))
{
    struct ly_ctx *ctx;
    struct lys_node *next, *iter;
    unsigned int i;

    assert(module->ctx);
    ctx = module->ctx;

    /* just free the import array, imported modules will stay in the context */
    for (i = 0; i < module->imp_size; i++) {
        lydict_remove(ctx, module->imp[i].prefix);
        lydict_remove(ctx, module->imp[i].dsc);
        lydict_remove(ctx, module->imp[i].ref);
        lys_extension_instances_free(ctx, module->imp[i].ext, module->imp[i].ext_size, private_destructor);
    }
    free(module->imp);

    /* submodules don't have data tree, the data nodes
     * are placed in the main module altogether */
    if (!module->type) {
        LY_TREE_FOR_SAFE(module->data, next, iter) {
            lys_node_free(iter, private_destructor, 0);
        }
    }

    lydict_remove(ctx, module->dsc);
    lydict_remove(ctx, module->ref);
    lydict_remove(ctx, module->org);
    lydict_remove(ctx, module->contact);
    lydict_remove(ctx, module->filepath);

    /* revisions */
    for (i = 0; i < module->rev_size; i++) {
        lys_extension_instances_free(ctx, module->rev[i].ext, module->rev[i].ext_size, private_destructor);
        lydict_remove(ctx, module->rev[i].dsc);
        lydict_remove(ctx, module->rev[i].ref);
    }
    free(module->rev);

    /* identities */
    for (i = 0; i < module->ident_size; i++) {
        lys_ident_free(ctx, &module->ident[i], private_destructor);
    }
    module->ident_size = 0;
    free(module->ident);

    /* typedefs */
    for (i = 0; i < module->tpdf_size; i++) {
        lys_tpdf_free(ctx, &module->tpdf[i], private_destructor);
    }
    free(module->tpdf);

    /* extension instances */
    lys_extension_instances_free(ctx, module->ext, module->ext_size, private_destructor);

    /* augment */
    for (i = 0; i < module->augment_size; i++) {
        lys_augment_free(ctx, &module->augment[i], private_destructor);
    }
    free(module->augment);

    /* features */
    for (i = 0; i < module->features_size; i++) {
        lys_feature_free(ctx, &module->features[i], private_destructor);
    }
    free(module->features);

    /* deviations */
    for (i = 0; i < module->deviation_size; i++) {
        lys_deviation_free(module, &module->deviation[i], private_destructor);
    }
    free(module->deviation);

    /* extensions */
    for (i = 0; i < module->extensions_size; i++) {
        lys_extension_free(ctx, &module->extensions[i], private_destructor);
    }
    free(module->extensions);

    lydict_remove(ctx, module->name);
    lydict_remove(ctx, module->prefix);
}

void
lys_submodule_free(struct lys_submodule *submodule, void (*private_destructor)(const struct lys_node *node, void *priv))
{
    int i;

    if (!submodule) {
        return;
    }

    /* common part with struct ly_module */
    module_free_common((struct lys_module *)submodule, private_destructor);

    /* include */
    for (i = 0; i < submodule->inc_size; i++) {
        lydict_remove(submodule->ctx, submodule->inc[i].dsc);
        lydict_remove(submodule->ctx, submodule->inc[i].ref);
        lys_extension_instances_free(submodule->ctx, submodule->inc[i].ext, submodule->inc[i].ext_size, private_destructor);
        /* complete submodule free is done only from main module since
         * submodules propagate their includes to the main module */
    }
    free(submodule->inc);

    free(submodule);
}

int
lys_ingrouping(const struct lys_node *node)
{
    const struct lys_node *iter = node;
    assert(node);

    for(iter = node; iter && iter->nodetype != LYS_GROUPING; iter = lys_parent(iter));
    if (!iter) {
        return 0;
    } else {
        return 1;
    }
}

/*
 * final: 0 - do not change config flags; 1 - inherit config flags from the parent; 2 - remove config flags
 */
static struct lys_node *
lys_node_dup_recursion(struct lys_module *module, struct lys_node *parent, const struct lys_node *node,
                       struct unres_schema *unres, int shallow, int finalize)
{
    struct lys_node *retval = NULL, *iter, *p;
    struct ly_ctx *ctx = module->ctx;
    int i, j, rc;
    unsigned int size, size1, size2;
    struct unres_list_uniq *unique_info;
    uint16_t flags;

    struct lys_node_container *cont = NULL;
    struct lys_node_container *cont_orig = (struct lys_node_container *)node;
    struct lys_node_choice *choice = NULL;
    struct lys_node_choice *choice_orig = (struct lys_node_choice *)node;
    struct lys_node_leaf *leaf = NULL;
    struct lys_node_leaf *leaf_orig = (struct lys_node_leaf *)node;
    struct lys_node_leaflist *llist = NULL;
    struct lys_node_leaflist *llist_orig = (struct lys_node_leaflist *)node;
    struct lys_node_list *list = NULL;
    struct lys_node_list *list_orig = (struct lys_node_list *)node;
    struct lys_node_anydata *any = NULL;
    struct lys_node_anydata *any_orig = (struct lys_node_anydata *)node;
    struct lys_node_uses *uses = NULL;
    struct lys_node_uses *uses_orig = (struct lys_node_uses *)node;
    struct lys_node_rpc_action *rpc = NULL;
    struct lys_node_inout *io = NULL;
    struct lys_node_notif *ntf = NULL;
    struct lys_node_case *cs = NULL;
    struct lys_node_case *cs_orig = (struct lys_node_case *)node;

    /* we cannot just duplicate memory since the strings are stored in
     * dictionary and we need to update dictionary counters.
     */

    switch (node->nodetype) {
    case LYS_CONTAINER:
        cont = calloc(1, sizeof *cont);
        retval = (struct lys_node *)cont;
        break;

    case LYS_CHOICE:
        choice = calloc(1, sizeof *choice);
        retval = (struct lys_node *)choice;
        break;

    case LYS_LEAF:
        leaf = calloc(1, sizeof *leaf);
        retval = (struct lys_node *)leaf;
        break;

    case LYS_LEAFLIST:
        llist = calloc(1, sizeof *llist);
        retval = (struct lys_node *)llist;
        break;

    case LYS_LIST:
        list = calloc(1, sizeof *list);
        retval = (struct lys_node *)list;
        break;

    case LYS_ANYXML:
    case LYS_ANYDATA:
        any = calloc(1, sizeof *any);
        retval = (struct lys_node *)any;
        break;

    case LYS_USES:
        uses = calloc(1, sizeof *uses);
        retval = (struct lys_node *)uses;
        break;

    case LYS_CASE:
        cs = calloc(1, sizeof *cs);
        retval = (struct lys_node *)cs;
        break;

    case LYS_RPC:
    case LYS_ACTION:
        rpc = calloc(1, sizeof *rpc);
        retval = (struct lys_node *)rpc;
        break;

    case LYS_INPUT:
    case LYS_OUTPUT:
        io = calloc(1, sizeof *io);
        retval = (struct lys_node *)io;
        break;

    case LYS_NOTIF:
        ntf = calloc(1, sizeof *ntf);
        retval = (struct lys_node *)ntf;
        break;

    default:
        LOGINT(ctx);
        goto error;
    }
    LY_CHECK_ERR_RETURN(!retval, LOGMEM(ctx), NULL);

    /*
     * duplicate generic part of the structure
     */
    retval->name = lydict_insert(ctx, node->name, 0);
    retval->dsc = lydict_insert(ctx, node->dsc, 0);
    retval->ref = lydict_insert(ctx, node->ref, 0);
    retval->flags = node->flags;

    retval->module = module;
    retval->nodetype = node->nodetype;

    retval->prev = retval;

    retval->ext_size = node->ext_size;
    if (lys_ext_dup(ctx, module, node->ext, node->ext_size, retval, LYEXT_PAR_NODE, &retval->ext, shallow, unres)) {
        goto error;
    }

    if (node->iffeature_size) {
        retval->iffeature_size = node->iffeature_size;
        retval->iffeature = calloc(retval->iffeature_size, sizeof *retval->iffeature);
        LY_CHECK_ERR_GOTO(!retval->iffeature, LOGMEM(ctx), error);
    }

    if (!shallow) {
        for (i = 0; i < node->iffeature_size; ++i) {
            resolve_iffeature_getsizes(&node->iffeature[i], &size1, &size2);
            if (size1) {
                /* there is something to duplicate */

                /* duplicate compiled expression */
                size = (size1 / 4) + (size1 % 4) ? 1 : 0;
                retval->iffeature[i].expr = malloc(size * sizeof *retval->iffeature[i].expr);
                LY_CHECK_ERR_GOTO(!retval->iffeature[i].expr, LOGMEM(ctx), error);
                memcpy(retval->iffeature[i].expr, node->iffeature[i].expr, size * sizeof *retval->iffeature[i].expr);

                /* list of feature pointer must be updated to point to the resulting tree */
                retval->iffeature[i].features = calloc(size2, sizeof *retval->iffeature[i].features);
                LY_CHECK_ERR_GOTO(!retval->iffeature[i].features, LOGMEM(ctx); free(retval->iffeature[i].expr), error);

                for (j = 0; (unsigned int)j < size2; j++) {
                    rc = unres_schema_dup(module, unres, &node->iffeature[i].features[j], UNRES_IFFEAT,
                                          &retval->iffeature[i].features[j]);
                    if (rc == EXIT_FAILURE) {
                        /* feature is resolved in origin, so copy it
                         * - duplication is used for instantiating groupings
                         * and if-feature inside grouping is supposed to be
                         * resolved inside the original grouping, so we want
                         * to keep pointers to features from the grouping
                         * context */
                        retval->iffeature[i].features[j] = node->iffeature[i].features[j];
                    } else if (rc == -1) {
                        goto error;
                    } /* else unres was duplicated */
                }
            }

            /* duplicate if-feature's extensions */
            retval->iffeature[i].ext_size = node->iffeature[i].ext_size;
            if (lys_ext_dup(ctx, module, node->iffeature[i].ext, node->iffeature[i].ext_size,
                            &retval->iffeature[i], LYEXT_PAR_IFFEATURE, &retval->iffeature[i].ext, shallow, unres)) {
                goto error;
            }
        }

        /* inherit config flags */
        p = parent;
        do {
            for (iter = p; iter && (iter->nodetype == LYS_USES); iter = iter->parent);
        } while (iter && iter->nodetype == LYS_AUGMENT && (p = lys_parent(iter)));
        if (iter) {
            flags = iter->flags & LYS_CONFIG_MASK;
        } else {
            /* default */
            flags = LYS_CONFIG_W;
        }

        switch (finalize) {
        case 1:
            /* inherit config flags */
            if (retval->flags & LYS_CONFIG_SET) {
                /* skip nodes with an explicit config value */
                if ((flags & LYS_CONFIG_R) && (retval->flags & LYS_CONFIG_W)) {
                    LOGVAL(ctx, LYE_INARG, LY_VLOG_LYS, retval, "true", "config");
                    LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "State nodes cannot have configuration nodes as children.");
                    goto error;
                }
                break;
            }

            if (retval->nodetype != LYS_USES) {
                retval->flags = (retval->flags & ~LYS_CONFIG_MASK) | flags;
            }

            /* inherit status */
            if ((parent->flags & LYS_STATUS_MASK) > (retval->flags & LYS_STATUS_MASK)) {
                /* but do it only in case the parent has a stonger status */
                retval->flags &= ~LYS_STATUS_MASK;
                retval->flags |= (parent->flags & LYS_STATUS_MASK);
            }
            break;
        case 2:
            /* erase config flags */
            retval->flags &= ~LYS_CONFIG_MASK;
            retval->flags &= ~LYS_CONFIG_SET;
            break;
        }

        /* connect it to the parent */
        if (lys_node_addchild(parent, retval->module, retval, 0)) {
            goto error;
        }

        /* go recursively */
        if (!(node->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            LY_TREE_FOR(node->child, iter) {
                if (iter->nodetype & LYS_GROUPING) {
                    /* do not instantiate groupings */
                    continue;
                }
                if (!lys_node_dup_recursion(module, retval, iter, unres, 0, finalize)) {
                    goto error;
                }
            }
        }

        if (finalize == 1) {
            /* check that configuration lists have keys
             * - we really want to check keys_size in original node, because the keys are
             * not yet resolved here, it is done below in nodetype specific part */
            if ((retval->nodetype == LYS_LIST) && (retval->flags & LYS_CONFIG_W)
                    && !((struct lys_node_list *)node)->keys_size) {
                LOGVAL(ctx, LYE_MISSCHILDSTMT, LY_VLOG_LYS, retval, "key", "list");
                goto error;
            }
        }
    } else {
        memcpy(retval->iffeature, node->iffeature, retval->iffeature_size * sizeof *retval->iffeature);
    }

    /*
     * duplicate specific part of the structure
     */
    switch (node->nodetype) {
    case LYS_CONTAINER:
        if (cont_orig->when) {
            cont->when = lys_when_dup(module, cont_orig->when, shallow, unres);
            LY_CHECK_GOTO(!cont->when, error);
        }
        cont->presence = lydict_insert(ctx, cont_orig->presence, 0);

        if (cont_orig->must) {
            cont->must = lys_restr_dup(module, cont_orig->must, cont_orig->must_size, shallow, unres);
            LY_CHECK_GOTO(!cont->must, error);
            cont->must_size = cont_orig->must_size;
        }

        /* typedefs are not needed in instantiated grouping, nor the deviation's shallow copy */

        break;
    case LYS_CHOICE:
        if (choice_orig->when) {
            choice->when = lys_when_dup(module, choice_orig->when, shallow, unres);
            LY_CHECK_GOTO(!choice->when, error);
        }

        if (!shallow) {
            if (choice_orig->dflt) {
                rc = lys_get_sibling(choice->child, lys_node_module(retval)->name, 0, choice_orig->dflt->name, 0,
                                            LYS_ANYDATA | LYS_CASE | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST,
                                            (const struct lys_node **)&choice->dflt);
                if (rc) {
                    if (rc == EXIT_FAILURE) {
                        LOGINT(ctx);
                    }
                    goto error;
                }
            } else {
                /* useless to check return value, we don't know whether
                * there really wasn't any default defined or it just hasn't
                * been resolved, we just hope for the best :)
                */
                unres_schema_dup(module, unres, choice_orig, UNRES_CHOICE_DFLT, choice);
            }
        } else {
            choice->dflt = choice_orig->dflt;
        }
        break;

    case LYS_LEAF:
        if (lys_type_dup(module, retval, &(leaf->type), &(leaf_orig->type), lys_ingrouping(retval), shallow, unres)) {
            goto error;
        }
        leaf->units = lydict_insert(module->ctx, leaf_orig->units, 0);

        if (leaf_orig->dflt) {
            leaf->dflt = lydict_insert(ctx, leaf_orig->dflt, 0);
        }

        if (leaf_orig->must) {
            leaf->must = lys_restr_dup(module, leaf_orig->must, leaf_orig->must_size, shallow, unres);
            LY_CHECK_GOTO(!leaf->must, error);
            leaf->must_size = leaf_orig->must_size;
        }

        if (leaf_orig->when) {
            leaf->when = lys_when_dup(module, leaf_orig->when, shallow, unres);
            LY_CHECK_GOTO(!leaf->when, error);
        }
        break;

    case LYS_LEAFLIST:
        if (lys_type_dup(module, retval, &(llist->type), &(llist_orig->type), lys_ingrouping(retval), shallow, unres)) {
            goto error;
        }
        llist->units = lydict_insert(module->ctx, llist_orig->units, 0);

        llist->min = llist_orig->min;
        llist->max = llist_orig->max;

        if (llist_orig->must) {
            llist->must = lys_restr_dup(module, llist_orig->must, llist_orig->must_size, shallow, unres);
            LY_CHECK_GOTO(!llist->must, error);
            llist->must_size = llist_orig->must_size;
        }

        if (llist_orig->dflt) {
            llist->dflt = malloc(llist_orig->dflt_size * sizeof *llist->dflt);
            LY_CHECK_ERR_GOTO(!llist->dflt, LOGMEM(ctx), error);
            llist->dflt_size = llist_orig->dflt_size;

            for (i = 0; i < llist->dflt_size; i++) {
                llist->dflt[i] = lydict_insert(ctx, llist_orig->dflt[i], 0);
            }
        }

        if (llist_orig->when) {
            llist->when = lys_when_dup(module, llist_orig->when, shallow, unres);
        }
        break;

    case LYS_LIST:
        list->min = list_orig->min;
        list->max = list_orig->max;

        if (list_orig->must) {
            list->must = lys_restr_dup(module, list_orig->must, list_orig->must_size, shallow, unres);
            LY_CHECK_GOTO(!list->must, error);
            list->must_size = list_orig->must_size;
        }

        /* typedefs are not needed in instantiated grouping, nor the deviation's shallow copy */

        if (list_orig->keys_size) {
            list->keys = calloc(list_orig->keys_size, sizeof *list->keys);
            LY_CHECK_ERR_GOTO(!list->keys, LOGMEM(ctx), error);
            list->keys_str = lydict_insert(ctx, list_orig->keys_str, 0);
            list->keys_size = list_orig->keys_size;

            if (!shallow) {
                if (unres_schema_add_node(module, unres, list, UNRES_LIST_KEYS, NULL) == -1) {
                    goto error;
                }
            } else {
                memcpy(list->keys, list_orig->keys, list_orig->keys_size * sizeof *list->keys);
            }
        }

        if (list_orig->unique) {
            list->unique = malloc(list_orig->unique_size * sizeof *list->unique);
            LY_CHECK_ERR_GOTO(!list->unique, LOGMEM(ctx), error);
            list->unique_size = list_orig->unique_size;

            for (i = 0; i < list->unique_size; ++i) {
                list->unique[i].expr = malloc(list_orig->unique[i].expr_size * sizeof *list->unique[i].expr);
                LY_CHECK_ERR_GOTO(!list->unique[i].expr, LOGMEM(ctx), error);
                list->unique[i].expr_size = list_orig->unique[i].expr_size;
                for (j = 0; j < list->unique[i].expr_size; j++) {
                    list->unique[i].expr[j] = lydict_insert(ctx, list_orig->unique[i].expr[j], 0);

                    /* if it stays in unres list, duplicate it also there */
                    unique_info = malloc(sizeof *unique_info);
                    LY_CHECK_ERR_GOTO(!unique_info, LOGMEM(ctx), error);
                    unique_info->list = (struct lys_node *)list;
                    unique_info->expr = list->unique[i].expr[j];
                    unique_info->trg_type = &list->unique[i].trg_type;
                    unres_schema_dup(module, unres, &list_orig, UNRES_LIST_UNIQ, unique_info);
                }
            }
        }

        if (list_orig->when) {
            list->when = lys_when_dup(module, list_orig->when, shallow, unres);
            LY_CHECK_GOTO(!list->when, error);
        }
        break;

    case LYS_ANYXML:
    case LYS_ANYDATA:
        if (any_orig->must) {
            any->must = lys_restr_dup(module, any_orig->must, any_orig->must_size, shallow, unres);
            LY_CHECK_GOTO(!any->must, error);
            any->must_size = any_orig->must_size;
        }

        if (any_orig->when) {
            any->when = lys_when_dup(module, any_orig->when, shallow, unres);
            LY_CHECK_GOTO(!any->when, error);
        }
        break;

    case LYS_USES:
        uses->grp = uses_orig->grp;

        if (uses_orig->when) {
            uses->when = lys_when_dup(module, uses_orig->when, shallow, unres);
            LY_CHECK_GOTO(!uses->when, error);
        }
        /* it is not needed to duplicate refine, nor augment. They are already applied to the uses children */
        break;

    case LYS_CASE:
        if (cs_orig->when) {
            cs->when = lys_when_dup(module, cs_orig->when, shallow, unres);
            LY_CHECK_GOTO(!cs->when, error);
        }
        break;

    case LYS_ACTION:
    case LYS_RPC:
    case LYS_INPUT:
    case LYS_OUTPUT:
    case LYS_NOTIF:
        /* typedefs are not needed in instantiated grouping, nor the deviation's shallow copy */
        break;

    default:
        /* LY_NODE_AUGMENT */
        LOGINT(ctx);
        goto error;
    }

    return retval;

error:
    lys_node_free(retval, NULL, 0);
    return NULL;
}

int
lys_has_xpath(const struct lys_node *node)
{
    assert(node);

    switch (node->nodetype) {
    case LYS_AUGMENT:
        if (((struct lys_node_augment *)node)->when) {
            return 1;
        }
        break;
    case LYS_CASE:
        if (((struct lys_node_case *)node)->when) {
            return 1;
        }
        break;
    case LYS_CHOICE:
        if (((struct lys_node_choice *)node)->when) {
            return 1;
        }
        break;
    case LYS_ANYDATA:
        if (((struct lys_node_anydata *)node)->when || ((struct lys_node_anydata *)node)->must_size) {
            return 1;
        }
        break;
    case LYS_LEAF:
        if (((struct lys_node_leaf *)node)->when || ((struct lys_node_leaf *)node)->must_size) {
            return 1;
        }
        break;
    case LYS_LEAFLIST:
        if (((struct lys_node_leaflist *)node)->when || ((struct lys_node_leaflist *)node)->must_size) {
            return 1;
        }
        break;
    case LYS_LIST:
        if (((struct lys_node_list *)node)->when || ((struct lys_node_list *)node)->must_size) {
            return 1;
        }
        break;
    case LYS_CONTAINER:
        if (((struct lys_node_container *)node)->when || ((struct lys_node_container *)node)->must_size) {
            return 1;
        }
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        if (((struct lys_node_inout *)node)->must_size) {
            return 1;
        }
        break;
    case LYS_NOTIF:
        if (((struct lys_node_notif *)node)->must_size) {
            return 1;
        }
        break;
    case LYS_USES:
        if (((struct lys_node_uses *)node)->when) {
            return 1;
        }
        break;
    default:
        /* does not have XPath */
        break;
    }

    return 0;
}

int
lys_type_is_local(const struct lys_type *type)
{
    if (!type->der->module) {
        /* build-in type */
        return 1;
    }
    /* type->parent can be either a typedef or leaf/leaf-list, but module pointers are compatible */
    return (lys_main_module(type->der->module) == lys_main_module(((struct lys_tpdf *)type->parent)->module));
}

/*
 * shallow -
 *         - do not inherit status from the parent
 */
struct lys_node *
lys_node_dup(struct lys_module *module, struct lys_node *parent, const struct lys_node *node,
             struct unres_schema *unres, int shallow)
{
    struct lys_node *p = NULL;
    int finalize = 0;
    struct lys_node *result, *iter, *next;

    if (!shallow) {
        /* get know where in schema tree we are to know what should be done during instantiation of the grouping */
        for (p = parent;
             p && !(p->nodetype & (LYS_NOTIF | LYS_INPUT | LYS_OUTPUT | LYS_RPC | LYS_ACTION | LYS_GROUPING));
             p = lys_parent(p));
        finalize = p ? ((p->nodetype == LYS_GROUPING) ? 0 : 2) : 1;
    }

    result = lys_node_dup_recursion(module, parent, node, unres, shallow, finalize);
    if (finalize) {
        /* check xpath expressions in the instantiated tree */
        for (iter = next = result; iter; iter = next) {
            if (lys_has_xpath(iter) && unres_schema_add_node(module, unres, iter, UNRES_XPATH, NULL) == -1) {
                /* invalid xpath */
                return NULL;
            }

            /* select next item */
            if (iter->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA | LYS_GROUPING)) {
                /* child exception for leafs, leaflists and anyxml without children, ignore groupings */
                next = NULL;
            } else {
                next = iter->child;
            }
            if (!next) {
                /* no children, try siblings */
                if (iter == result) {
                    /* we are done, no next element to process */
                    break;
                }
                next = iter->next;
            }
            while (!next) {
                /* parent is already processed, go to its sibling */
                iter = lys_parent(iter);
                if (lys_parent(iter) == lys_parent(result)) {
                    /* we are done, no next element to process */
                    break;
                }
                next = iter->next;
            }
        }
    }

    return result;
}

/**
 * @brief Switch contents of two same schema nodes. One of the nodes
 * is expected to be ashallow copy of the other.
 *
 * @param[in] node1 Node whose contents will be switched with \p node2.
 * @param[in] node2 Node whose contents will be switched with \p node1.
 */
static void
lys_node_switch(struct lys_node *node1, struct lys_node *node2)
{
    const size_t mem_size = 104;
    uint8_t mem[mem_size];
    size_t offset, size;

    assert((node1->module == node2->module) && ly_strequal(node1->name, node2->name, 1) && (node1->nodetype == node2->nodetype));

    /*
     * Initially, the nodes were really switched in the tree which
     * caused problems for some other nodes with pointers (augments, leafrefs, ...)
     * because their pointers were not being updated. Code kept in case there is
     * a use of it in future (it took some debugging to cover all the cases).

    * sibling next *
    if (node1->prev->next) {
        node1->prev->next = node2;
    }

    * sibling prev *
    if (node1->next) {
        node1->next->prev = node2;
    } else {
        for (child = node1->prev; child->prev->next; child = child->prev);
        child->prev = node2;
    }

    * next *
    node2->next = node1->next;
    node1->next = NULL;

    * prev *
    if (node1->prev != node1) {
        node2->prev = node1->prev;
    }
    node1->prev = node1;

    * parent child *
    if (node1->parent) {
        if (node1->parent->child == node1) {
            node1->parent->child = node2;
        }
    } else if (lys_main_module(node1->module)->data == node1) {
        lys_main_module(node1->module)->data = node2;
    }

    * parent *
    node2->parent = node1->parent;
    node1->parent = NULL;

    * child parent *
    LY_TREE_FOR(node1->child, child) {
        if (child->parent == node1) {
            child->parent = node2;
        }
    }

    * child *
    node2->child = node1->child;
    node1->child = NULL;
    */

    /* switch common node part */
    offset = 3 * sizeof(char *);
    size = sizeof(uint16_t) + 6 * sizeof(uint8_t) + sizeof(struct lys_ext_instance **) + sizeof(struct lys_iffeature *);
    memcpy(mem, ((uint8_t *)node1) + offset, size);
    memcpy(((uint8_t *)node1) + offset, ((uint8_t *)node2) + offset, size);
    memcpy(((uint8_t *)node2) + offset, mem, size);

    /* switch node-specific data */
    offset = sizeof(struct lys_node);
    switch (node1->nodetype) {
    case LYS_CONTAINER:
        size = sizeof(struct lys_node_container) - offset;
        break;
    case LYS_CHOICE:
        size = sizeof(struct lys_node_choice) - offset;
        break;
    case LYS_LEAF:
        size = sizeof(struct lys_node_leaf) - offset;
        break;
    case LYS_LEAFLIST:
        size = sizeof(struct lys_node_leaflist) - offset;
        break;
    case LYS_LIST:
        size = sizeof(struct lys_node_list) - offset;
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        size = sizeof(struct lys_node_anydata) - offset;
        break;
    case LYS_CASE:
        size = sizeof(struct lys_node_case) - offset;
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        size = sizeof(struct lys_node_inout) - offset;
        break;
    case LYS_NOTIF:
        size = sizeof(struct lys_node_notif) - offset;
        break;
    case LYS_RPC:
    case LYS_ACTION:
        size = sizeof(struct lys_node_rpc_action) - offset;
        break;
    default:
        assert(0);
        LOGINT(node1->module->ctx);
        return;
    }
    assert(size <= mem_size);
    memcpy(mem, ((uint8_t *)node1) + offset, size);
    memcpy(((uint8_t *)node1) + offset, ((uint8_t *)node2) + offset, size);
    memcpy(((uint8_t *)node2) + offset, mem, size);

    /* typedefs were not copied to the backup node, so always reuse them,
     * in leaves/leaf-lists we must correct the type parent pointer */
    switch (node1->nodetype) {
    case LYS_CONTAINER:
        ((struct lys_node_container *)node1)->tpdf_size = ((struct lys_node_container *)node2)->tpdf_size;
        ((struct lys_node_container *)node1)->tpdf = ((struct lys_node_container *)node2)->tpdf;
        ((struct lys_node_container *)node2)->tpdf_size = 0;
        ((struct lys_node_container *)node2)->tpdf = NULL;
        break;
    case LYS_LIST:
        ((struct lys_node_list *)node1)->tpdf_size = ((struct lys_node_list *)node2)->tpdf_size;
        ((struct lys_node_list *)node1)->tpdf = ((struct lys_node_list *)node2)->tpdf;
        ((struct lys_node_list *)node2)->tpdf_size = 0;
        ((struct lys_node_list *)node2)->tpdf = NULL;
        break;
    case LYS_RPC:
    case LYS_ACTION:
        ((struct lys_node_rpc_action *)node1)->tpdf_size = ((struct lys_node_rpc_action *)node2)->tpdf_size;
        ((struct lys_node_rpc_action *)node1)->tpdf = ((struct lys_node_rpc_action *)node2)->tpdf;
        ((struct lys_node_rpc_action *)node2)->tpdf_size = 0;
        ((struct lys_node_rpc_action *)node2)->tpdf = NULL;
        break;
    case LYS_NOTIF:
        ((struct lys_node_notif *)node1)->tpdf_size = ((struct lys_node_notif *)node2)->tpdf_size;
        ((struct lys_node_notif *)node1)->tpdf = ((struct lys_node_notif *)node2)->tpdf;
        ((struct lys_node_notif *)node2)->tpdf_size = 0;
        ((struct lys_node_notif *)node2)->tpdf = NULL;
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        ((struct lys_node_inout *)node1)->tpdf_size = ((struct lys_node_inout *)node2)->tpdf_size;
        ((struct lys_node_inout *)node1)->tpdf = ((struct lys_node_inout *)node2)->tpdf;
        ((struct lys_node_inout *)node2)->tpdf_size = 0;
        ((struct lys_node_inout *)node2)->tpdf = NULL;
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
        ((struct lys_node_leaf *)node1)->type.parent = (struct lys_tpdf *)node1;
        ((struct lys_node_leaf *)node2)->type.parent = (struct lys_tpdf *)node2;
    default:
        break;
    }
}

void
lys_free(struct lys_module *module, void (*private_destructor)(const struct lys_node *node, void *priv), int free_subs, int remove_from_ctx)
{
    struct ly_ctx *ctx;
    int i;

    if (!module) {
        return;
    }

    /* remove schema from the context */
    ctx = module->ctx;
    if (remove_from_ctx && ctx->models.used) {
        for (i = 0; i < ctx->models.used; i++) {
            if (ctx->models.list[i] == module) {
                /* move all the models to not change the order in the list */
                ctx->models.used--;
                memmove(&ctx->models.list[i], ctx->models.list[i + 1], (ctx->models.used - i) * sizeof *ctx->models.list);
                ctx->models.list[ctx->models.used] = NULL;
                /* we are done */
                break;
            }
        }
    }

    /* common part with struct ly_submodule */
    module_free_common(module, private_destructor);

    /* include */
    for (i = 0; i < module->inc_size; i++) {
        lydict_remove(ctx, module->inc[i].dsc);
        lydict_remove(ctx, module->inc[i].ref);
        lys_extension_instances_free(ctx, module->inc[i].ext, module->inc[i].ext_size, private_destructor);
        /* complete submodule free is done only from main module since
         * submodules propagate their includes to the main module */
        if (free_subs) {
            lys_submodule_free(module->inc[i].submodule, private_destructor);
        }
    }
    free(module->inc);

    /* specific items to free */
    lydict_remove(ctx, module->ns);

    free(module);
}

static void
lys_features_disable_recursive(struct lys_feature *f)
{
    unsigned int i;
    struct lys_feature *depf;

    /* disable the feature */
    f->flags &= ~LYS_FENABLED;

    /* by disabling feature we have to disable also all features that depends on this feature */
    if (f->depfeatures) {
        for (i = 0; i < f->depfeatures->number; i++) {
            depf = (struct lys_feature *)f->depfeatures->set.g[i];
            if (depf->flags & LYS_FENABLED) {
                lys_features_disable_recursive(depf);
            }
        }
    }
}

/*
 * op: 1 - enable, 0 - disable
 */
static int
lys_features_change(const struct lys_module *module, const char *name, int op)
{
    int all = 0;
    int i, j, k;
    int progress, faili, failj, failk;

    uint8_t fsize;
    struct lys_feature *f;

    if (!module || !name || !strlen(name)) {
        LOGARG;
        return EXIT_FAILURE;
    }

    if (!strcmp(name, "*")) {
        /* enable all */
        all = 1;
    }

    progress = failk = 1;
    while (progress && failk) {
        for (i = -1, failk = progress = 0; i < module->inc_size; i++) {
            if (i == -1) {
                fsize = module->features_size;
                f = module->features;
            } else {
                fsize = module->inc[i].submodule->features_size;
                f = module->inc[i].submodule->features;
            }

            for (j = 0; j < fsize; j++) {
                if (all || !strcmp(f[j].name, name)) {
                    if ((op && (f[j].flags & LYS_FENABLED)) || (!op && !(f[j].flags & LYS_FENABLED))) {
                        if (all) {
                            /* skip already set features */
                            continue;
                        } else {
                            /* feature already set correctly */
                            return EXIT_SUCCESS;
                        }
                    }

                    if (op) {
                        /* check referenced features if they are enabled */
                        for (k = 0; k < f[j].iffeature_size; k++) {
                            if (!resolve_iffeature(&f[j].iffeature[k])) {
                                if (all) {
                                    faili = i;
                                    failj = j;
                                    failk = k + 1;
                                    break;
                                } else {
                                    LOGERR(module->ctx, LY_EINVAL, "Feature \"%s\" is disabled by its %d. if-feature condition.",
                                           f[j].name, k + 1);
                                    return EXIT_FAILURE;
                                }
                            }
                        }

                        if (k == f[j].iffeature_size) {
                            /* the last check passed, do the change */
                            f[j].flags |= LYS_FENABLED;
                            progress++;
                        }
                    } else {
                        lys_features_disable_recursive(&f[j]);
                        progress++;
                    }
                    if (!all) {
                        /* stop in case changing a single feature */
                        return EXIT_SUCCESS;
                    }
                }
            }
        }
    }
    if (failk) {
        /* print info about the last failing feature */
        LOGERR(module->ctx, LY_EINVAL, "Feature \"%s\" is disabled by its %d. if-feature condition.",
               faili == -1 ? module->features[failj].name : module->inc[faili].submodule->features[failj].name, failk);
        return EXIT_FAILURE;
    }

    if (all) {
        return EXIT_SUCCESS;
    } else {
        /* the specified feature not found */
        return EXIT_FAILURE;
    }
}

API int
lys_features_enable(const struct lys_module *module, const char *feature)
{
    return lys_features_change(module, feature, 1);
}

API int
lys_features_disable(const struct lys_module *module, const char *feature)
{
    return lys_features_change(module, feature, 0);
}

API int
lys_features_state(const struct lys_module *module, const char *feature)
{
    int i, j;

    if (!module || !feature) {
        return -1;
    }

    /* search for the specified feature */
    /* module itself */
    for (i = 0; i < module->features_size; i++) {
        if (!strcmp(feature, module->features[i].name)) {
            if (module->features[i].flags & LYS_FENABLED) {
                return 1;
            } else {
                return 0;
            }
        }
    }

    /* submodules */
    for (j = 0; j < module->inc_size; j++) {
        for (i = 0; i < module->inc[j].submodule->features_size; i++) {
            if (!strcmp(feature, module->inc[j].submodule->features[i].name)) {
                if (module->inc[j].submodule->features[i].flags & LYS_FENABLED) {
                    return 1;
                } else {
                    return 0;
                }
            }
        }
    }

    /* feature definition not found */
    return -1;
}

API const char **
lys_features_list(const struct lys_module *module, uint8_t **states)
{
    const char **result = NULL;
    int i, j;
    unsigned int count;

    if (!module) {
        return NULL;
    }

    count = module->features_size;
    for (i = 0; i < module->inc_size; i++) {
        count += module->inc[i].submodule->features_size;
    }
    result = malloc((count + 1) * sizeof *result);
    LY_CHECK_ERR_RETURN(!result, LOGMEM(module->ctx), NULL);

    if (states) {
        *states = malloc((count + 1) * sizeof **states);
        LY_CHECK_ERR_RETURN(!(*states), LOGMEM(module->ctx); free(result), NULL);
    }
    count = 0;

    /* module itself */
    for (i = 0; i < module->features_size; i++) {
        result[count] = module->features[i].name;
        if (states) {
            if (module->features[i].flags & LYS_FENABLED) {
                (*states)[count] = 1;
            } else {
                (*states)[count] = 0;
            }
        }
        count++;
    }

    /* submodules */
    for (j = 0; j < module->inc_size; j++) {
        for (i = 0; i < module->inc[j].submodule->features_size; i++) {
            result[count] = module->inc[j].submodule->features[i].name;
            if (states) {
                if (module->inc[j].submodule->features[i].flags & LYS_FENABLED) {
                    (*states)[count] = 1;
                } else {
                    (*states)[count] = 0;
                }
            }
            count++;
        }
    }

    /* terminating NULL byte */
    result[count] = NULL;

    return result;
}

API struct lys_module *
lys_node_module(const struct lys_node *node)
{
    if (!node) {
        return NULL;
    }

    return node->module->type ? ((struct lys_submodule *)node->module)->belongsto : node->module;
}

API struct lys_module *
lys_main_module(const struct lys_module *module)
{
    if (!module) {
        return NULL;
    }

    return (module->type ? ((struct lys_submodule *)module)->belongsto : (struct lys_module *)module);
}

API struct lys_node *
lys_parent(const struct lys_node *node)
{
    struct lys_node *parent;

    if (!node) {
        return NULL;
    }

    if (node->nodetype == LYS_EXT) {
        if (((struct lys_ext_instance_complex*)node)->parent_type != LYEXT_PAR_NODE) {
            return NULL;
        }
        parent = (struct lys_node*)((struct lys_ext_instance_complex*)node)->parent;
    } else if (!node->parent) {
        return NULL;
    } else {
        parent = node->parent;
    }

    if (parent->nodetype == LYS_AUGMENT) {
        return ((struct lys_node_augment *)parent)->target;
    } else {
        return parent;
    }
}

struct lys_node **
lys_child(const struct lys_node *node, LYS_NODE nodetype)
{
    void *pp;
    assert(node);

    if (node->nodetype == LYS_EXT) {
        pp = lys_ext_complex_get_substmt(lys_snode2stmt(nodetype), (struct lys_ext_instance_complex*)node, NULL);
        if (!pp) {
            return NULL;
        }
        return (struct lys_node **)pp;
    } else if (node->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
        return NULL;
    } else {
        return (struct lys_node **)&node->child;
    }
}

API void *
lys_set_private(const struct lys_node *node, void *priv)
{
    void *prev;

    if (!node) {
        LOGARG;
        return NULL;
    }

    prev = node->priv;
    ((struct lys_node *)node)->priv = priv;

    return prev;
}

int
lys_leaf_add_leafref_target(struct lys_node_leaf *leafref_target, struct lys_node *leafref)
{
    struct lys_node_leaf *iter;
    struct ly_ctx *ctx = leafref_target->module->ctx;

    if (!(leafref_target->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        LOGINT(ctx);
        return -1;
    }

    /* check for config flag */
    if (((struct lys_node_leaf*)leafref)->type.info.lref.req != -1 &&
            (leafref->flags & LYS_CONFIG_W) && (leafref_target->flags & LYS_CONFIG_R)) {
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_LYS, leafref,
               "The leafref %s is config but refers to a non-config %s.",
               strnodetype(leafref->nodetype), strnodetype(leafref_target->nodetype));
        return -1;
    }
    /* check for cycles */
    for (iter = leafref_target; iter && iter->type.base == LY_TYPE_LEAFREF; iter = iter->type.info.lref.target) {
        if ((void *)iter == (void *)leafref) {
            /* cycle detected */
            LOGVAL(ctx, LYE_CIRC_LEAFREFS, LY_VLOG_LYS, leafref);
            return -1;
        }
    }

    /* create fake child - the ly_set structure to hold the list of
     * leafrefs referencing the leaf(-list) */
    if (!leafref_target->backlinks) {
        leafref_target->backlinks = (void *)ly_set_new();
        if (!leafref_target->backlinks) {
            LOGMEM(ctx);
            return -1;
        }
    }
    ly_set_add(leafref_target->backlinks, leafref, 0);

    return 0;
}

/* not needed currently */
#if 0

static const char *
lys_data_path_reverse(const struct lys_node *node, char * const buf, uint32_t buf_len)
{
    struct lys_module *prev_mod;
    uint32_t str_len, mod_len, buf_idx;

    if (!(node->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA))) {
        LOGINT;
        return NULL;
    }

    buf_idx = buf_len - 1;
    buf[buf_idx] = '\0';

    while (node) {
        if (lys_parent(node)) {
            prev_mod = lys_node_module(lys_parent(node));
        } else {
            prev_mod = NULL;
        }

        if (node->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
            str_len = strlen(node->name);

            if (prev_mod != node->module) {
                mod_len = strlen(node->module->name);
            } else {
                mod_len = 0;
            }

            if (buf_idx < 1 + (mod_len ? mod_len + 1 : 0) + str_len) {
                LOGINT;
                return NULL;
            }

            buf_idx -= 1 + (mod_len ? mod_len + 1 : 0) + str_len;

            buf[buf_idx] = '/';
            if (mod_len) {
                memcpy(buf + buf_idx + 1, node->module->name, mod_len);
                buf[buf_idx + 1 + mod_len] = ':';
            }
            memcpy(buf + buf_idx + 1 + (mod_len ? mod_len + 1 : 0), node->name, str_len);
        }

        node = lys_parent(node);
    }

    return buf + buf_idx;
}

#endif

API struct ly_set *
lys_xpath_atomize(const struct lys_node *ctx_node, enum lyxp_node_type ctx_node_type, const char *expr, int options)
{
    struct lyxp_set set;
    struct ly_set *ret_set;
    uint32_t i;

    if (!ctx_node || !expr) {
        LOGARG;
        return NULL;
    }

    /* adjust the root */
    if ((ctx_node_type == LYXP_NODE_ROOT) || (ctx_node_type == LYXP_NODE_ROOT_CONFIG)) {
        do {
            ctx_node = lys_getnext(NULL, NULL, lys_node_module(ctx_node), LYS_GETNEXT_NOSTATECHECK);
        } while ((ctx_node_type == LYXP_NODE_ROOT_CONFIG) && (ctx_node->flags & LYS_CONFIG_R));
    }

    memset(&set, 0, sizeof set);

    if (options & LYXP_MUST) {
        options &= ~LYXP_MUST;
        options |= LYXP_SNODE_MUST;
    } else if (options & LYXP_WHEN) {
        options &= ~LYXP_WHEN;
        options |= LYXP_SNODE_WHEN;
    } else {
        options |= LYXP_SNODE;
    }

    if (lyxp_atomize(expr, ctx_node, ctx_node_type, &set, options, NULL)) {
        free(set.val.snodes);
        LOGVAL(ctx_node->module->ctx, LYE_SPEC, LY_VLOG_LYS, ctx_node, "Resolving XPath expression \"%s\" failed.", expr);
        return NULL;
    }

    ret_set = ly_set_new();

    for (i = 0; i < set.used; ++i) {
        switch (set.val.snodes[i].type) {
        case LYXP_NODE_ELEM:
            if (ly_set_add(ret_set, set.val.snodes[i].snode, LY_SET_OPT_USEASLIST) == -1) {
                ly_set_free(ret_set);
                free(set.val.snodes);
                return NULL;
            }
            break;
        default:
            /* ignore roots, text and attr should not ever appear */
            break;
        }
    }

    free(set.val.snodes);
    return ret_set;
}

API struct ly_set *
lys_node_xpath_atomize(const struct lys_node *node, int options)
{
    const struct lys_node *next, *elem, *parent, *tmp;
    struct lyxp_set set;
    struct ly_set *ret_set;
    uint16_t i;

    if (!node) {
        LOGARG;
        return NULL;
    }

    for (parent = node; parent && !(parent->nodetype & (LYS_NOTIF | LYS_INPUT | LYS_OUTPUT)); parent = lys_parent(parent));
    if (!parent) {
        /* not in input, output, or notification */
        return NULL;
    }

    ret_set = ly_set_new();
    if (!ret_set) {
        return NULL;
    }

    LY_TREE_DFS_BEGIN(node, next, elem) {
        if ((options & LYXP_NO_LOCAL) && !(elem->flags & (LYS_XPCONF_DEP | LYS_XPSTATE_DEP))) {
            /* elem has no dependencies from other subtrees and local nodes get discarded */
            goto next_iter;
        }

        if (lyxp_node_atomize(elem, &set, 0)) {
            ly_set_free(ret_set);
            free(set.val.snodes);
            return NULL;
        }

        for (i = 0; i < set.used; ++i) {
            switch (set.val.snodes[i].type) {
            case LYXP_NODE_ELEM:
                if (options & LYXP_NO_LOCAL) {
                    for (tmp = set.val.snodes[i].snode; tmp && (tmp != parent); tmp = lys_parent(tmp));
                    if (tmp) {
                        /* in local subtree, discard */
                        break;
                    }
                }
                if (ly_set_add(ret_set, set.val.snodes[i].snode, 0) == -1) {
                    ly_set_free(ret_set);
                    free(set.val.snodes);
                    return NULL;
                }
                break;
            default:
                /* ignore roots, text and attr should not ever appear */
                break;
            }
        }

        free(set.val.snodes);
        if (!(options & LYXP_RECURSIVE)) {
            break;
        }
next_iter:
        LY_TREE_DFS_END(node, next, elem);
    }

    return ret_set;
}

/* logs */
int
apply_aug(struct lys_node_augment *augment, struct unres_schema *unres)
{
    struct lys_node *child, *parent;
    int clear_config;
    unsigned int u;
    uint8_t *v;
    struct lys_ext_instance *ext;

    assert(augment->target && (augment->flags & LYS_NOTAPPLIED));

    if (!augment->child) {
        /* nothing to apply */
        goto success;
    }

    /* inherit config information from actual parent */
    for (parent = augment->target; parent && !(parent->nodetype & (LYS_NOTIF | LYS_INPUT | LYS_OUTPUT | LYS_RPC)); parent = lys_parent(parent));
    clear_config = (parent) ? 1 : 0;
    LY_TREE_FOR(augment->child, child) {
        if (inherit_config_flag(child, augment->target->flags & LYS_CONFIG_MASK, clear_config)) {
            return -1;
        }
    }

    /* inherit extensions if any */
    for (u = 0; u < augment->target->ext_size; u++) {
        ext = augment->target->ext[u]; /* shortcut */
        if (ext && ext->def->plugin && (ext->def->plugin->flags & LYEXT_OPT_INHERIT)) {
            v = malloc(sizeof *v);
            LY_CHECK_ERR_RETURN(!v, LOGMEM(augment->module->ctx), -1);
            *v = u;
            if (unres_schema_add_node(lys_main_module(augment->module), unres, &augment->target->ext,
                    UNRES_EXT_FINALIZE, (struct lys_node *)v) == -1) {
                /* something really bad happend since the extension finalization is not actually
                 * being resolved while adding into unres, so something more serious with the unres
                 * list itself must happened */
                return -1;
            }
        }
    }

    /* reconnect augmenting data into the target - add them to the target child list */
    if (augment->target->child) {
        child = augment->target->child->prev;
        child->next = augment->child;
        augment->target->child->prev = augment->child->prev;
        augment->child->prev = child;
    } else {
        augment->target->child = augment->child;
    }

success:
    /* remove the flag about not applicability */
    augment->flags &= ~LYS_NOTAPPLIED;
    return EXIT_SUCCESS;
}

static void
remove_aug(struct lys_node_augment *augment)
{
    struct lys_node *last, *elem;

    if ((augment->flags & LYS_NOTAPPLIED) || !augment->target) {
        /* skip already not applied augment */
        return;
    }

    elem = augment->child;
    if (elem) {
        LY_TREE_FOR(elem, last) {
            if (!last->next || (last->next->parent != (struct lys_node *)augment)) {
                break;
            }
        }
        /* elem is first augment child, last is the last child */

        /* parent child ptr */
        if (augment->target->child == elem) {
            augment->target->child = last->next;
        }

        /* parent child next ptr */
        if (elem->prev->next) {
            elem->prev->next = last->next;
        }

        /* parent child prev ptr */
        if (last->next) {
            last->next->prev = elem->prev;
        } else if (augment->target->child) {
            augment->target->child->prev = elem->prev;
        }

        /* update augment children themselves */
        elem->prev = last;
        last->next = NULL;
    }

    /* augment->target still keeps the resolved target, but for lys_augment_free()
     * we have to keep information that this augment is not applied to free its data */
    augment->flags |= LYS_NOTAPPLIED;
}

/*
 * @param[in] module - the module where the deviation is defined
 */
static void
lys_switch_deviation(struct lys_deviation *dev, const struct lys_module *module, struct unres_schema *unres)
{
    int ret, reapply = 0;
    char *parent_path;
    struct lys_node *target = NULL, *parent;
    struct lys_node_inout *inout;
    struct ly_set *set;

    if (!dev->deviate) {
        return;
    }

    if (dev->deviate[0].mod == LY_DEVIATE_NO) {
        if (dev->orig_node) {
            /* removing not-supported deviation ... */
            if (strrchr(dev->target_name, '/') != dev->target_name) {
                /* ... from a parent */

                /* reconnect to its previous position */
                parent = dev->orig_node->parent;
                if (parent && (parent->nodetype == LYS_AUGMENT)) {
                    dev->orig_node->parent = NULL;
                    /* the original node was actually from augment, we have to get know if the augment is
                     * applied (its module is enabled and implemented). If yes, the node will be connected
                     * to the augment and the linkage with the target will be fixed if needed, otherwise
                     * it will be connected only to the augment */
                    if (!(parent->flags & LYS_NOTAPPLIED)) {
                        /* start with removing augment if applied before adding nodes, we have to make sure
                         * that everything will be connect correctly */
                        remove_aug((struct lys_node_augment *)parent);
                        reapply = 1;
                    }
                    /* connect the deviated node back into the augment */
                    lys_node_addchild(parent, NULL, dev->orig_node, 0);
                    if (reapply) {
                        /* augment is supposed to be applied, so fix pointers in target and the status of the original node */
                        assert(lys_node_module(parent)->implemented);
                        parent->flags |= LYS_NOTAPPLIED; /* allow apply_aug() */
                        apply_aug((struct lys_node_augment *)parent, unres);
                    }
                } else if (parent && (parent->nodetype == LYS_USES)) {
                    /* uses child */
                    lys_node_addchild(parent, NULL, dev->orig_node, 0);
                } else {
                    /* non-augment, non-toplevel */
                    parent_path = strndup(dev->target_name, strrchr(dev->target_name, '/') - dev->target_name);
                    ret = resolve_schema_nodeid(parent_path, NULL, module, &set, 0, 1);
                    free(parent_path);
                    if (ret == -1) {
                        LOGINT(module->ctx);
                        ly_set_free(set);
                        return;
                    }
                    target = set->set.s[0];
                    ly_set_free(set);

                    lys_node_addchild(target, NULL, dev->orig_node, 0);
                }
            } else {
                /* ... from top-level data */
                lys_node_addchild(NULL, (struct lys_module *)dev->orig_node->module, dev->orig_node, 0);
            }

            dev->orig_node = NULL;
        } else {
            /* adding not-supported deviation */
            ret = resolve_schema_nodeid(dev->target_name, NULL, module, &set, 0, 1);
            if (ret == -1) {
                LOGINT(module->ctx);
                ly_set_free(set);
                return;
            }
            target = set->set.s[0];
            ly_set_free(set);

            /* unlink and store the original node */
            parent = target->parent;
            lys_node_unlink(target);
            if (parent) {
                if (parent->nodetype & (LYS_AUGMENT | LYS_USES)) {
                    /* hack for augment, because when the original will be sometime reconnected back, we actually need
                     * to reconnect it to both - the augment and its target (which is deduced from the deviations target
                     * path), so we need to remember the augment as an addition */
                    /* we also need to remember the parent uses so that we connect it back to it when switching deviation state */
                    target->parent = parent;
                } else if (parent->nodetype & (LYS_RPC | LYS_ACTION)) {
                    /* re-create implicit node */
                    inout = calloc(1, sizeof *inout);
                    LY_CHECK_ERR_RETURN(!inout, LOGMEM(module->ctx), );

                    inout->nodetype = target->nodetype;
                    inout->name = lydict_insert(module->ctx, (inout->nodetype == LYS_INPUT) ? "input" : "output", 0);
                    inout->module = target->module;
                    inout->flags = LYS_IMPLICIT;

                    /* insert it manually */
                    assert(parent->child && !parent->child->next
                    && (parent->child->nodetype == (inout->nodetype == LYS_INPUT ? LYS_OUTPUT : LYS_INPUT)));
                    parent->child->next = (struct lys_node *)inout;
                    inout->prev = parent->child;
                    parent->child->prev = (struct lys_node *)inout;
                    inout->parent = parent;
                }
            }
            dev->orig_node = target;
        }
    } else {
        ret = resolve_schema_nodeid(dev->target_name, NULL, module, &set, 0, 1);
        if (ret == -1) {
            LOGINT(module->ctx);
            ly_set_free(set);
            return;
        }
        target = set->set.s[0];
        ly_set_free(set);

        /* contents are switched */
        lys_node_switch(target, dev->orig_node);
    }
}

/* temporarily removes or applies deviations, updates module deviation flag accordingly */
void
lys_enable_deviations(struct lys_module *module)
{
    uint32_t i = 0, j;
    const struct lys_module *mod;
    const char *ptr;
    struct unres_schema *unres;

    if (module->deviated) {
        unres = calloc(1, sizeof *unres);
        LY_CHECK_ERR_RETURN(!unres, LOGMEM(module->ctx), );

        while ((mod = ly_ctx_get_module_iter(module->ctx, &i))) {
            if (mod == module) {
                continue;
            }

            for (j = 0; j < mod->deviation_size; ++j) {
                ptr = strstr(mod->deviation[j].target_name, module->name);
                if (ptr && ptr[strlen(module->name)] == ':') {
                    lys_switch_deviation(&mod->deviation[j], mod, unres);
                }
            }
        }

        assert(module->deviated == 2);
        module->deviated = 1;

        for (j = 0; j < module->inc_size; j++) {
            if (module->inc[j].submodule->deviated) {
                module->inc[j].submodule->deviated = module->deviated;
            }
        }

        if (unres->count) {
            resolve_unres_schema(module, unres);
        }
        unres_schema_free(module, &unres, 1);
    }
}

void
lys_disable_deviations(struct lys_module *module)
{
    uint32_t i, j;
    const struct lys_module *mod;
    const char *ptr;
    struct unres_schema *unres;

    if (module->deviated) {
        unres = calloc(1, sizeof *unres);
        LY_CHECK_ERR_RETURN(!unres, LOGMEM(module->ctx), );

        i = module->ctx->models.used;
        while (i--) {
            mod = module->ctx->models.list[i];

            if (mod == module) {
                continue;
            }

            j = mod->deviation_size;
            while (j--) {
                ptr = strstr(mod->deviation[j].target_name, module->name);
                if (ptr && ptr[strlen(module->name)] == ':') {
                    lys_switch_deviation(&mod->deviation[j], mod, unres);
                }
            }
        }

        assert(module->deviated == 1);
        module->deviated = 2;

        for (j = 0; j < module->inc_size; j++) {
            if (module->inc[j].submodule->deviated) {
                module->inc[j].submodule->deviated = module->deviated;
            }
        }

        if (unres->count) {
            resolve_unres_schema(module, unres);
        }
        unres_schema_free(module, &unres, 1);
    }
}

static void
apply_dev(struct lys_deviation *dev, const struct lys_module *module, struct unres_schema *unres)
{
    lys_switch_deviation(dev, module, unres);

    assert(dev->orig_node);
    lys_node_module(dev->orig_node)->deviated = 1; /* main module */
    dev->orig_node->module->deviated = 1;          /* possible submodule */
}

static void
remove_dev(struct lys_deviation *dev, const struct lys_module *module, struct unres_schema *unres)
{
    uint32_t idx = 0, j;
    const struct lys_module *mod;
    struct lys_module *target_mod, *target_submod;
    const char *ptr;

    if (dev->orig_node) {
        target_mod = lys_node_module(dev->orig_node);
        target_submod = dev->orig_node->module;
    } else {
        LOGINT(module->ctx);
        return;
    }
    lys_switch_deviation(dev, module, unres);

    /* clear the deviation flag if possible */
    while ((mod = ly_ctx_get_module_iter(module->ctx, &idx))) {
        if ((mod == module) || (mod == target_mod)) {
            continue;
        }

        for (j = 0; j < mod->deviation_size; ++j) {
            ptr = strstr(mod->deviation[j].target_name, target_mod->name);
            if (ptr && (ptr[strlen(target_mod->name)] == ':')) {
                /* some other module deviation targets the inspected module, flag remains */
                break;
            }
        }

        if (j < mod->deviation_size) {
            break;
        }
    }

    if (!mod) {
        target_mod->deviated = 0;    /* main module */
        target_submod->deviated = 0; /* possible submodule */
    }
}

void
lys_sub_module_apply_devs_augs(struct lys_module *module)
{
    uint8_t u, v;
    struct unres_schema *unres;

    assert(module->implemented);

    unres = calloc(1, sizeof *unres);
    LY_CHECK_ERR_RETURN(!unres, LOGMEM(module->ctx), );

    /* apply deviations */
    for (u = 0; u < module->deviation_size; ++u) {
        apply_dev(&module->deviation[u], module, unres);
    }
    /* apply augments */
    for (u = 0; u < module->augment_size; ++u) {
        apply_aug(&module->augment[u], unres);
    }

    /* apply deviations and augments defined in submodules */
    for (v = 0; v < module->inc_size; ++v) {
        for (u = 0; u < module->inc[v].submodule->deviation_size; ++u) {
            apply_dev(&module->inc[v].submodule->deviation[u], module, unres);
        }

        for (u = 0; u < module->inc[v].submodule->augment_size; ++u) {
            apply_aug(&module->inc[v].submodule->augment[u], unres);
        }
    }

    if (unres->count) {
        resolve_unres_schema(module, unres);
    }
    /* nothing else left to do even if something is not resolved */
    unres_schema_free(module, &unres, 1);
}

void
lys_sub_module_remove_devs_augs(struct lys_module *module)
{
    uint8_t u, v, w;
    struct unres_schema *unres;

    unres = calloc(1, sizeof *unres);
    LY_CHECK_ERR_RETURN(!unres, LOGMEM(module->ctx), );

    /* remove applied deviations */
    for (u = 0; u < module->deviation_size; ++u) {
        /* the deviation could not be applied because it failed to be applied in the first place*/
        if (module->deviation[u].orig_node) {
            remove_dev(&module->deviation[u], module, unres);
        }

        /* Free the deviation's must array(s). These are shallow copies of the arrays
           on the target node(s), so a deep free is not needed. */
        for (v = 0; v < module->deviation[u].deviate_size; ++v) {
            if (module->deviation[u].deviate[v].mod == LY_DEVIATE_ADD) {
                free(module->deviation[u].deviate[v].must);
            }
        }
    }
    /* remove applied augments */
    for (u = 0; u < module->augment_size; ++u) {
        remove_aug(&module->augment[u]);
    }

    /* remove deviation and augments defined in submodules */
    for (v = 0; v < module->inc_size && module->inc[v].submodule; ++v) {
        for (u = 0; u < module->inc[v].submodule->deviation_size; ++u) {
            if (module->inc[v].submodule->deviation[u].orig_node) {
                remove_dev(&module->inc[v].submodule->deviation[u], module, unres);
            }

            /* Free the deviation's must array(s). These are shallow copies of the arrays
               on the target node(s), so a deep free is not needed. */
            for (w = 0; w < module->inc[v].submodule->deviation[u].deviate_size; ++w) {
                if (module->inc[v].submodule->deviation[u].deviate[w].mod == LY_DEVIATE_ADD) {
                    free(module->inc[v].submodule->deviation[u].deviate[w].must);
                }
            }
        }

        for (u = 0; u < module->inc[v].submodule->augment_size; ++u) {
            remove_aug(&module->inc[v].submodule->augment[u]);
        }
    }

    if (unres->count) {
        resolve_unres_schema(module, unres);
    }
    /* nothing else left to do even if something is not resolved */
    unres_schema_free(module, &unres, 1);
}

int
lys_make_implemented_r(struct lys_module *module, struct unres_schema *unres)
{
    struct ly_ctx *ctx;
    struct lys_node *root, *next, *node;
    struct lys_module *target_module;
    uint16_t i, j, k;

    assert(module->implemented);
    ctx = module->ctx;

    for (i = 0; i < ctx->models.used; ++i) {
        if (module == ctx->models.list[i]) {
            continue;
        }

        if (!strcmp(module->name, ctx->models.list[i]->name) && ctx->models.list[i]->implemented) {
            LOGERR(ctx, LY_EINVAL, "Module \"%s\" in another revision already implemented.", module->name);
            return EXIT_FAILURE;
        }
    }

    for (i = 0; i < module->augment_size; i++) {

        /* make target module implemented if was not */
        assert(module->augment[i].target);
        target_module = lys_node_module(module->augment[i].target);
        if (!target_module->implemented) {
            target_module->implemented = 1;
            if (unres_schema_add_node(target_module, unres, NULL, UNRES_MOD_IMPLEMENT, NULL) == -1) {
                return -1;
            }
        }

        /* apply augment */
        if ((module->augment[i].flags & LYS_NOTAPPLIED) && apply_aug(&module->augment[i], unres)) {
            return -1;
        }
    }

    /* identities */
    for (i = 0; i < module->ident_size; i++) {
        for (j = 0; j < module->ident[i].base_size; j++) {
            resolve_identity_backlink_update(&module->ident[i], module->ident[i].base[j]);
        }
    }

    /* process augments in submodules */
    for (i = 0; i < module->inc_size && module->inc[i].submodule; ++i) {
        module->inc[i].submodule->implemented = 1;

        for (j = 0; j < module->inc[i].submodule->augment_size; j++) {

            /* make target module implemented if it was not */
            assert(module->inc[i].submodule->augment[j].target);
            target_module = lys_node_module(module->inc[i].submodule->augment[j].target);
            if (!target_module->implemented) {
                target_module->implemented = 1;
                if (unres_schema_add_node(target_module, unres, NULL, UNRES_MOD_IMPLEMENT, NULL) == -1) {
                    return -1;
                }
            }

            /* apply augment */
            if ((module->inc[i].submodule->augment[j].flags & LYS_NOTAPPLIED) && apply_aug(&module->inc[i].submodule->augment[j], unres)) {
                return -1;
            }
        }

        /* identities */
        for (j = 0; j < module->inc[i].submodule->ident_size; j++) {
            for (k = 0; k < module->inc[i].submodule->ident[j].base_size; k++) {
                resolve_identity_backlink_update(&module->inc[i].submodule->ident[j],
                                                 module->inc[i].submodule->ident[j].base[k]);
            }
        }
    }

    LY_TREE_FOR(module->data, root) {
        /* handle leafrefs and recursively change the implemented flags in the leafref targets */
        LY_TREE_DFS_BEGIN(root, next, node) {
            if (node->nodetype == LYS_GROUPING) {
                goto nextsibling;
            }
            if (node->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
                if (((struct lys_node_leaf *)node)->type.base == LY_TYPE_LEAFREF) {
                    if (unres_schema_add_node(module, unres, &((struct lys_node_leaf *)node)->type,
                                              UNRES_TYPE_LEAFREF, node) == -1) {
                        return -1;
                    }
                }
            }

            /* modified LY_TREE_DFS_END */
            next = node->child;
            /* child exception for leafs, leaflists and anyxml without children */
            if (node->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                next = NULL;
            }
            if (!next) {
nextsibling:
                /* no children */
                if (node == root) {
                    /* we are done, root has no children */
                    break;
                }
                /* try siblings */
                next = node->next;
            }
            while (!next) {
                /* parent is already processed, go to its sibling */
                node = lys_parent(node);
                /* no siblings, go back through parents */
                if (lys_parent(node) == lys_parent(root)) {
                    /* we are done, no next element to process */
                    break;
                }
                next = node->next;
            }
        }
    }

    return EXIT_SUCCESS;
}

API int
lys_set_implemented(const struct lys_module *module)
{
    struct unres_schema *unres;
    int disabled = 0;

    if (!module) {
        LOGARG;
        return EXIT_FAILURE;
    }

    module = lys_main_module(module);

    if (module->disabled) {
        disabled = 1;
        lys_set_enabled(module);
    }

    if (module->implemented) {
        return EXIT_SUCCESS;
    }

    unres = calloc(1, sizeof *unres);
    if (!unres) {
        LOGMEM(module->ctx);
        if (disabled) {
            /* set it back disabled */
            lys_set_disabled(module);
        }
        return EXIT_FAILURE;
    }
    /* recursively make the module implemented */
    ((struct lys_module *)module)->implemented = 1;
    if (lys_make_implemented_r((struct lys_module *)module, unres)) {
        goto error;
    }

    /* try again resolve augments in other modules possibly augmenting this one,
     * since we have just enabled it
     */
    /* resolve rest of unres items */
    if (unres->count && resolve_unres_schema((struct lys_module *)module, unres)) {
        goto error;
    }
    unres_schema_free(NULL, &unres, 0);

    LOGVRB("Module \"%s%s%s\" now implemented.", module->name, (module->rev_size ? "@" : ""),
           (module->rev_size ? module->rev[0].date : ""));
    return EXIT_SUCCESS;

error:
    if (disabled) {
        /* set it back disabled */
        lys_set_disabled(module);
    }

    ((struct lys_module *)module)->implemented = 0;
    unres_schema_free((struct lys_module *)module, &unres, 1);
    return EXIT_FAILURE;
}

void
lys_submodule_module_data_free(struct lys_submodule *submodule)
{
    struct lys_node *next, *elem;

    /* remove parsed data */
    LY_TREE_FOR_SAFE(submodule->belongsto->data, next, elem) {
        if (elem->module == (struct lys_module *)submodule) {
            lys_node_free(elem, NULL, 0);
        }
    }
}

API char *
lys_path(const struct lys_node *node, int options)
{
    char *buf = NULL;

    if (!node) {
        LOGARG;
        return NULL;
    }

    if (ly_vlog_build_path(LY_VLOG_LYS, node, &buf, (options & LYS_PATH_FIRST_PREFIX) ? 0 : 1, 0)) {
        return NULL;
    }

    return buf;
}

API char *
lys_data_path(const struct lys_node *node)
{
    char *result = NULL, buf[1024];
    const char *separator, *name;
    int i, used;
    struct ly_set *set;
    const struct lys_module *prev_mod;

    if (!node) {
        LOGARG;
        return NULL;
    }

    buf[0] = '\0';
    set = ly_set_new();
    LY_CHECK_ERR_GOTO(!set, LOGMEM(node->module->ctx), cleanup);

    while (node) {
        ly_set_add(set, (void *)node, 0);
        do {
            node = lys_parent(node);
        } while (node && (node->nodetype & (LYS_USES | LYS_CHOICE | LYS_CASE | LYS_INPUT | LYS_OUTPUT)));
    }

    prev_mod = NULL;
    used = 0;
    for (i = set->number - 1; i > -1; --i) {
        node = set->set.s[i];
        if (node->nodetype == LYS_EXT) {
            if (strcmp(((struct lys_ext_instance *)node)->def->name, "yang-data")) {
                continue;
            }
            name = ((struct lys_ext_instance *)node)->arg_value;
            separator = ":#";
        } else {
            name = node->name;
            separator = ":";
        }
        used += sprintf(buf + used, "/%s%s%s", (lys_node_module(node) == prev_mod ? "" : lys_node_module(node)->name),
                        (lys_node_module(node) == prev_mod ? "" : separator), name);
        prev_mod = lys_node_module(node);
    }

    result = strdup(buf);
    LY_CHECK_ERR_GOTO(!result, LOGMEM(node->module->ctx), cleanup);

cleanup:
    ly_set_free(set);
    return result;
}

struct lys_node_augment *
lys_getnext_target_aug(struct lys_node_augment *last, const struct lys_module *mod, const struct lys_node *aug_target)
{
    struct lys_node *child;
    struct lys_node_augment *aug;
    int i, j, last_found;

    assert(mod && aug_target);

    if (!last) {
        last_found = 1;
    } else {
        last_found = 0;
    }

    /* search module augments */
    for (i = 0; i < mod->augment_size; ++i) {
        if (!mod->augment[i].target) {
            /* still unresolved, skip */
            continue;
        }

        if (mod->augment[i].target == aug_target) {
            if (last_found) {
                /* next match after last */
                return &mod->augment[i];
            }

            if (&mod->augment[i] == last) {
                last_found = 1;
            }
        }
    }

    /* search submodule augments */
    for (i = 0; i < mod->inc_size; ++i) {
        for (j = 0; j < mod->inc[i].submodule->augment_size; ++j) {
            if (!mod->inc[i].submodule->augment[j].target) {
                continue;
            }

            if (mod->inc[i].submodule->augment[j].target == aug_target) {
                if (last_found) {
                    /* next match after last */
                    return &mod->inc[i].submodule->augment[j];
                }

                if (&mod->inc[i].submodule->augment[j] == last) {
                    last_found = 1;
                }
            }
        }
    }

    /* we also need to check possible augments to choices */
    LY_TREE_FOR(aug_target->child, child) {
        if (child->nodetype == LYS_CHOICE) {
            aug = lys_getnext_target_aug(last, mod, child);
            if (aug) {
                return aug;
            }
        }
    }

    return NULL;
}

API struct ly_set *
lys_find_path(const struct lys_module *cur_module, const struct lys_node *cur_node, const char *path)
{
    struct ly_set *ret;
    int rc;

    if ((!cur_module && !cur_node) || !path) {
        return NULL;
    }

    rc = resolve_schema_nodeid(path, cur_node, cur_module, &ret, 1, 1);
    if (rc == -1) {
        return NULL;
    }

    return ret;
}

static void
lys_extcomplex_free_str(struct ly_ctx *ctx, struct lys_ext_instance_complex *ext, LY_STMT stmt)
{
    struct lyext_substmt *info;
    const char **str, ***a;
    int c;

    str = lys_ext_complex_get_substmt(stmt, ext, &info);
    if (!str || !(*str)) {
        return;
    }
    if (info->cardinality >= LY_STMT_CARD_SOME) {
        /* we have array */
        a = (const char ***)str;
        for (str = (*(const char ***)str), c = 0; str[c]; c++) {
            lydict_remove(ctx, str[c]);
        }
        free(a[0]);
        if (stmt == LY_STMT_BELONGSTO) {
            for (str = a[1], c = 0; str[c]; c++) {
                lydict_remove(ctx, str[c]);
            }
            free(a[1]);
        } else if (stmt == LY_STMT_ARGUMENT) {
            free(a[1]);
        }
    } else {
        lydict_remove(ctx, str[0]);
        if (stmt == LY_STMT_BELONGSTO) {
            lydict_remove(ctx, str[1]);
        }
    }
}

void
lys_extension_instances_free(struct ly_ctx *ctx, struct lys_ext_instance **e, unsigned int size,
                             void (*private_destructor)(const struct lys_node *node, void *priv))
{
    unsigned int i, j, k;
    struct lyext_substmt *substmt;
    void **pp, **start;
    struct lys_node *siter, *snext;

#define EXTCOMPLEX_FREE_STRUCT(STMT, TYPE, FUNC, FREE, ARGS...)                               \
    pp = lys_ext_complex_get_substmt(STMT, (struct lys_ext_instance_complex *)e[i], NULL);    \
    if (!pp || !(*pp)) { break; }                                                             \
    if (substmt[j].cardinality >= LY_STMT_CARD_SOME) { /* process array */                    \
        for (start = pp = *pp; *pp; pp++) {                                                   \
            FUNC(ctx, (TYPE *)(*pp), ##ARGS, private_destructor);                             \
            if (FREE) { free(*pp); }                                                          \
        }                                                                                     \
        free(start);                                                                          \
    } else { /* single item */                                                                \
        FUNC(ctx, (TYPE *)(*pp), ##ARGS, private_destructor);                                 \
        if (FREE) { free(*pp); }                                                              \
    }

    if (!size || !e) {
        return;
    }

    for (i = 0; i < size; i++) {
        if (!e[i]) {
            continue;
        }

        if (e[i]->flags & (LYEXT_OPT_INHERIT)) {
            /* no free, this is just a shadow copy of the original extension instance */
        } else {
            if (e[i]->flags & (LYEXT_OPT_YANG)) {
                free(e[i]->def);     /* remove name of instance extension */
                e[i]->def = NULL;
                yang_free_ext_data((struct yang_ext_substmt *)e[i]->parent); /* remove backup part of yang file */
            }
            /* remove private object */
            if (e[i]->priv && private_destructor) {
                private_destructor((struct lys_node*)e[i], e[i]->priv);
            }
            lys_extension_instances_free(ctx, e[i]->ext, e[i]->ext_size, private_destructor);
            lydict_remove(ctx, e[i]->arg_value);
        }

        if (e[i]->def && e[i]->def->plugin && e[i]->def->plugin->type == LYEXT_COMPLEX
                && ((e[i]->flags & LYEXT_OPT_CONTENT) == 0)) {
            substmt = ((struct lys_ext_instance_complex *)e[i])->substmt;
            for (j = 0; substmt[j].stmt; j++) {
                switch(substmt[j].stmt) {
                case LY_STMT_DESCRIPTION:
                case LY_STMT_REFERENCE:
                case LY_STMT_UNITS:
                case LY_STMT_ARGUMENT:
                case LY_STMT_DEFAULT:
                case LY_STMT_ERRTAG:
                case LY_STMT_ERRMSG:
                case LY_STMT_PREFIX:
                case LY_STMT_NAMESPACE:
                case LY_STMT_PRESENCE:
                case LY_STMT_REVISIONDATE:
                case LY_STMT_KEY:
                case LY_STMT_BASE:
                case LY_STMT_BELONGSTO:
                case LY_STMT_CONTACT:
                case LY_STMT_ORGANIZATION:
                case LY_STMT_PATH:
                    lys_extcomplex_free_str(ctx, (struct lys_ext_instance_complex *)e[i], substmt[j].stmt);
                    break;
                case LY_STMT_TYPE:
                    EXTCOMPLEX_FREE_STRUCT(LY_STMT_TYPE, struct lys_type, lys_type_free, 1);
                    break;
                case LY_STMT_TYPEDEF:
                    EXTCOMPLEX_FREE_STRUCT(LY_STMT_TYPEDEF, struct lys_tpdf, lys_tpdf_free, 1);
                    break;
                case LY_STMT_IFFEATURE:
                    EXTCOMPLEX_FREE_STRUCT(LY_STMT_IFFEATURE, struct lys_iffeature, lys_iffeature_free, 0, 1, 0);
                    break;
                case LY_STMT_MAX:
                case LY_STMT_MIN:
                case LY_STMT_POSITION:
                case LY_STMT_VALUE:
                    pp = (void**)&((struct lys_ext_instance_complex *)e[i])->content[substmt[j].offset];
                    if (substmt[j].cardinality >= LY_STMT_CARD_SOME && *pp) {
                        for(k = 0; ((uint32_t**)(*pp))[k]; k++) {
                            free(((uint32_t**)(*pp))[k]);
                        }
                    }
                    free(*pp);
                    break;
                case LY_STMT_DIGITS:
                    if (substmt[j].cardinality >= LY_STMT_CARD_SOME) {
                        /* free the array */
                        pp = (void**)&((struct lys_ext_instance_complex *)e[i])->content[substmt[j].offset];
                        free(*pp);
                    }
                    break;
                case LY_STMT_MODULE:
                    /* modules are part of the context, so they will be freed there */
                    if (substmt[j].cardinality >= LY_STMT_CARD_SOME) {
                        /* free the array */
                        pp = (void**)&((struct lys_ext_instance_complex *)e[i])->content[substmt[j].offset];
                        free(*pp);
                    }
                    break;
                case LY_STMT_ACTION:
                case LY_STMT_ANYDATA:
                case LY_STMT_ANYXML:
                case LY_STMT_CASE:
                case LY_STMT_CHOICE:
                case LY_STMT_CONTAINER:
                case LY_STMT_GROUPING:
                case LY_STMT_INPUT:
                case LY_STMT_LEAF:
                case LY_STMT_LEAFLIST:
                case LY_STMT_LIST:
                case LY_STMT_NOTIFICATION:
                case LY_STMT_OUTPUT:
                case LY_STMT_RPC:
                case LY_STMT_USES:
                    pp = (void**)&((struct lys_ext_instance_complex *)e[i])->content[substmt[j].offset];
                    LY_TREE_FOR_SAFE((struct lys_node *)(*pp), snext, siter) {
                        lys_node_free(siter, NULL, 0);
                    }
                    *pp = NULL;
                    break;
                case LY_STMT_UNIQUE:
                    pp = lys_ext_complex_get_substmt(LY_STMT_UNIQUE, (struct lys_ext_instance_complex *)e[i], NULL);
                    if (!pp || !(*pp)) {
                        break;
                    }
                    if (substmt[j].cardinality >= LY_STMT_CARD_SOME) { /* process array */
                        for (start = pp = *pp; *pp; pp++) {
                            for (k = 0; k < (*(struct lys_unique**)pp)->expr_size; k++) {
                                lydict_remove(ctx, (*(struct lys_unique**)pp)->expr[k]);
                            }
                            free((*(struct lys_unique**)pp)->expr);
                            free(*pp);
                        }
                        free(start);
                    } else { /* single item */
                        for (k = 0; k < (*(struct lys_unique**)pp)->expr_size; k++) {
                            lydict_remove(ctx, (*(struct lys_unique**)pp)->expr[k]);
                        }
                        free((*(struct lys_unique**)pp)->expr);
                        free(*pp);
                    }
                    break;
                case LY_STMT_LENGTH:
                case LY_STMT_MUST:
                case LY_STMT_PATTERN:
                case LY_STMT_RANGE:
                    EXTCOMPLEX_FREE_STRUCT(substmt[j].stmt, struct lys_restr, lys_restr_free, 1);
                    break;
                case LY_STMT_WHEN:
                    EXTCOMPLEX_FREE_STRUCT(LY_STMT_WHEN, struct lys_when, lys_when_free, 0);
                    break;
                case LY_STMT_REVISION:
                    pp = lys_ext_complex_get_substmt(LY_STMT_REVISION, (struct lys_ext_instance_complex *)e[i], NULL);
                    if (!pp || !(*pp)) {
                        break;
                    }
                    if (substmt[j].cardinality >= LY_STMT_CARD_SOME) { /* process array */
                        for (start = pp = *pp; *pp; pp++) {
                            lydict_remove(ctx, (*(struct lys_revision**)pp)->dsc);
                            lydict_remove(ctx, (*(struct lys_revision**)pp)->ref);
                            lys_extension_instances_free(ctx, (*(struct lys_revision**)pp)->ext,
                                                         (*(struct lys_revision**)pp)->ext_size, private_destructor);
                            free(*pp);
                        }
                        free(start);
                    } else { /* single item */
                        lydict_remove(ctx, (*(struct lys_revision**)pp)->dsc);
                        lydict_remove(ctx, (*(struct lys_revision**)pp)->ref);
                        lys_extension_instances_free(ctx, (*(struct lys_revision**)pp)->ext,
                                                     (*(struct lys_revision**)pp)->ext_size, private_destructor);
                        free(*pp);
                    }
                    break;
                default:
                    /* nothing to free */
                    break;
                }
            }
        }

        free(e[i]);
    }
    free(e);

#undef EXTCOMPLEX_FREE_STRUCT
}
