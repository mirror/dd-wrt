/**
 * @file tree_data.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Manipulation with libyang data structures
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

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "libyang.h"
#include "common.h"
#include "context.h"
#include "tree_data.h"
#include "parser.h"
#include "resolve.h"
#include "xml_internal.h"
#include "tree_internal.h"
#include "validation.h"
#include "xpath.h"

static struct lys_node *lyd_get_schema_inctx(const struct lyd_node *node, struct ly_ctx *ctx);

static int
lyd_anydata_equal(struct lyd_node *first, struct lyd_node *second)
{
    char *str1 = NULL, *str2 = NULL;
    struct lyd_node_anydata *anydata;

    assert(first->schema->nodetype & LYS_ANYDATA);
    assert(first->schema->nodetype == second->schema->nodetype);

    anydata = (struct lyd_node_anydata *)first;
    if (!anydata->value.str) {
        lyxml_print_mem(&str1, anydata->value.xml, LYXML_PRINT_SIBLINGS);
        anydata->value.str = lydict_insert_zc(anydata->schema->module->ctx, str1);
    }
    str1 = (char *)anydata->value.str;

    anydata = (struct lyd_node_anydata *)second;
    if (!anydata->value.str) {
        lyxml_print_mem(&str2, anydata->value.xml, LYXML_PRINT_SIBLINGS);
        anydata->value.str = lydict_insert_zc(anydata->schema->module->ctx, str2);
    }
    str2 = (char *)anydata->value.str;

    if (first->schema->module->ctx != second->schema->module->ctx) {
        return ly_strequal(str1, str2, 0);
    } else {
        return ly_strequal(str1, str2, 1);
    }
}

/* used in tests */
int
lyd_list_has_keys(struct lyd_node *list)
{
    struct lyd_node *iter;
    struct lys_node_list *slist;
    int i;

    assert(list->schema->nodetype == LYS_LIST);

    /* even though hash is 0, it may be a valid hash, that is what we are going to check */

    slist = (struct lys_node_list *)list->schema;
    if (!slist->keys_size) {
        /* always has keys */
        return 1;
    }

    i = 0;
    iter = list->child;
    while (iter && (i < slist->keys_size)) {
        if (iter->schema != (struct lys_node *)slist->keys[i]) {
            /* missing key */
            return 0;
        }

        ++i;
        iter = iter->next;
    }
    if (i < slist->keys_size) {
        /* missing key */
        return 0;
    }

    /* all keys found */
    return 1;
}

static int
lyd_leaf_val_equal(struct lyd_node *node1, struct lyd_node *node2, int diff_ctx)
{
    assert(node1->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST));
    assert(node1->schema->nodetype == node2->schema->nodetype);

    if (diff_ctx) {
        return ly_strequal(((struct lyd_node_leaf_list *)node1)->value_str, ((struct lyd_node_leaf_list *)node2)->value_str, 0);
    } else {
        return ly_strequal(((struct lyd_node_leaf_list *)node1)->value_str, ((struct lyd_node_leaf_list *)node2)->value_str, 1);
    }
}

/*
 * withdefaults (only for leaf-list):
 * 0 - treat default nodes are normal nodes
 * 1 - only change is that if 2 nodes have the same value, but one is default, the other not, they are considered non-equal
 */
int
lyd_list_equal(struct lyd_node *node1, struct lyd_node *node2, int with_defaults)
{
    int i, diff_ctx;
    struct lyd_node *elem1, *next1, *elem2, *next2;
    struct lys_node *elem1_sch;
    struct ly_ctx *ctx = node2->schema->module->ctx;

    diff_ctx = (node1->schema->module->ctx != node2->schema->module->ctx);

    switch (node2->schema->nodetype) {
    case LYS_LEAFLIST:
        if (lyd_leaf_val_equal(node1, node2, diff_ctx) && (!with_defaults || (node1->dflt == node2->dflt))) {
            return 1;
        }
        break;
    case LYS_LIST:
        if (((struct lys_node_list *)node1->schema)->keys_size) {
            /* lists with keys, their equivalence isb ased on their keys */
            elem1 = node1->child;
            elem2 = node2->child;
            elem1_sch = NULL;
            /* the exact data order is guaranteed */
            for (i = 0; i < ((struct lys_node_list *)node1->schema)->keys_size; ++i) {
                if (diff_ctx && elem1) {
                    /* we have different contexts */
                    if (!elem1_sch) {
                        elem1_sch = lyd_get_schema_inctx(elem1, ctx);
                        if (!elem1_sch) {
                            LOGERR(ctx, LY_EINVAL, "Target context does not contain a required schema node (%s:%s).",
                                   lyd_node_module(elem1)->name, elem1->schema->name);
                            return -1;
                        }
                    } else {
                        /* just move to the next schema node */
                        elem1_sch = elem1_sch->next;
                    }
                }
                if (!elem1 || !elem2 || ((elem1_sch ? elem1_sch : elem1->schema) != elem2->schema)
                        || !lyd_leaf_val_equal(elem1, elem2, diff_ctx)) {
                    break;
                }
                elem1 = elem1->next;
                elem2 = elem2->next;
            }
            if (i == ((struct lys_node_list *)node1->schema)->keys_size) {
                return 1;
            }
        } else {
            /* lists wihtout keys, their equivalence is based on values of all the children (both dierct and indirect) */
            if (!node1->child && !node2->child) {
                /* no children, nothing to compare */
                return 1;
            }

            /* status lists without keys, we need to compare all the children :( */

            /* LY_TREE_DFS_BEGIN for 2 data trees */
            elem1 = next1 = node1->child;
            elem2 = next2 = node2->child;
            while (elem1 && elem2) {
                /* node comparison */
#ifdef LY_ENABLED_CACHE
                if (elem1->hash != elem2->hash) {
                    break;
                }
#endif
                if (diff_ctx) {
                    elem1_sch = lyd_get_schema_inctx(elem1, ctx);
                    if (!elem1_sch) {
                        LOGERR(ctx, LY_EINVAL, "Target context does not contain a required schema node (%s:%s).",
                               lyd_node_module(elem1)->name, elem1->schema->name);
                        return -1;
                    }
                } else {
                    elem1_sch = elem1->schema;
                }
                if (elem1_sch != elem2->schema) {
                    break;
                }
                if (elem2->schema->nodetype == LYS_LIST) {
                    if (!lyd_list_has_keys(elem1) && !lyd_list_has_keys(elem2)) {
                        /* we encountered lists without keys (but have some defined in schema), ignore them for comparison */
                        next1 = NULL;
                        next2 = NULL;
                        goto next_sibling;
                    }
                    /* we will compare all the children of this list instance, not just keys */
                } else if (elem2->schema->nodetype & (LYS_LEAFLIST | LYS_LEAF)) {
                    if (!lyd_leaf_val_equal(elem1, elem2, diff_ctx) && (!with_defaults || (elem1->dflt == elem2->dflt))) {
                        break;
                    }
                } else if (elem2->schema->nodetype & LYS_ANYDATA) {
                    if (!lyd_anydata_equal(elem1, elem2)) {
                        break;
                    }
                }

                /* LY_TREE_DFS_END for 2 data trees */
                if (elem2->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                    next1 = NULL;
                    next2 = NULL;
                } else {
                    next1 = elem1->child;
                    next2 = elem2->child;
                }

next_sibling:
                if (!next1) {
                    next1 = elem1->next;
                }
                if (!next2) {
                    next2 = elem2->next;
                }

                while (!next1) {
                    elem1 = elem1->parent;
                    if (elem1 == node1) {
                        break;
                    }
                    next1 = elem1->next;
                }
                while (!next2) {
                    elem2 = elem2->parent;
                    if (elem2 == node2) {
                        break;
                    }
                    next2 = elem2->next;
                }

                elem1 = next1;
                elem2 = next2;
            }

            if (!elem1 && !elem2) {
                /* all children were successfully compared */
                return 1;
            }
        }
        break;
    default:
        LOGINT(ctx);
        return -1;
    }

    return 0;
}

#ifdef LY_ENABLED_CACHE

static int
lyd_hash_table_val_equal(void *val1_p, void *val2_p, int mod, void *UNUSED(cb_data))
{
    struct lyd_node *val1, *val2;

    val1 = *((struct lyd_node **)val1_p);
    val2 = *((struct lyd_node **)val2_p);

    if (mod) {
        if (val1 == val2) {
            return 1;
        } else {
            return 0;
        }
    }

    if (val1->schema != val2->schema) {
        return 0;
    }

    switch (val1->schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LEAF:
    case LYS_ANYXML:
    case LYS_ANYDATA:
        return 1;
    case LYS_LEAFLIST:
    case LYS_LIST:
        return lyd_list_equal(val1, val2, 0);
    default:
        break;
    }

    LOGINT(val1->schema->module->ctx);
    return 0;
}

static void
lyd_hash_keyless_list_dfs(struct lyd_node *child, uint32_t *hash)
{
    LY_TREE_FOR(child, child) {
        switch (child->schema->nodetype) {
        case LYS_CONTAINER:
            lyd_hash_keyless_list_dfs(child->child, hash);
            break;
        case LYS_LIST:
            /* ignore lists with missing keys */
            if (lyd_list_has_keys(child)) {
                lyd_hash_keyless_list_dfs(child->child, hash);
            }
            break;
        case LYS_LEAFLIST:
        case LYS_ANYXML:
        case LYS_ANYDATA:
        case LYS_LEAF:
            *hash = dict_hash_multi(*hash, (char *)&child->hash, sizeof child->hash);
            break;
        default:
            assert(0);
        }
    }
}

int
lyd_hash(struct lyd_node *node)
{
    struct lyd_node *iter;
    int i;

    assert(!node->hash || ((node->schema->nodetype == LYS_LIST) && !((struct lys_node_list *)node->schema)->keys_size));

    if ((node->schema->nodetype != LYS_LIST) || lyd_list_has_keys(node)) {
        node->hash = dict_hash_multi(0, lyd_node_module(node)->name, strlen(lyd_node_module(node)->name));
        node->hash = dict_hash_multi(node->hash, node->schema->name, strlen(node->schema->name));
        if (node->schema->nodetype == LYS_LEAFLIST) {
            node->hash = dict_hash_multi(node->hash, ((struct lyd_node_leaf_list *)node)->value_str,
                                        strlen(((struct lyd_node_leaf_list *)node)->value_str));
        } else if (node->schema->nodetype == LYS_LIST) {
            if (((struct lys_node_list *)node->schema)->keys_size) {
                for (i = 0, iter = node->child; i < ((struct lys_node_list *)node->schema)->keys_size; ++i, iter = iter->next) {
                    assert(iter);
                    node->hash = dict_hash_multi(node->hash, ((struct lyd_node_leaf_list *)iter)->value_str,
                                                 strlen(((struct lyd_node_leaf_list *)iter)->value_str));
                }
            } else {
                /* no-keys list */
                lyd_hash_keyless_list_dfs(node->child, &node->hash);
            }
        }
        node->hash = dict_hash_multi(node->hash, NULL, 0);
        return 0;
    }

    return 1;
}

static void
lyd_keyless_list_hash_change(struct lyd_node *parent)
{
    int r;

    while (parent && (parent->schema->flags & LYS_CONFIG_R)) {
        if (parent->schema->nodetype == LYS_LIST) {
            if (!((struct lys_node_list *)parent->schema)->keys_size) {
                if (parent->parent && parent->parent->ht) {
                    /* remove the list from the parent */
                    r = lyht_remove(parent->parent->ht, &parent, parent->hash);
                    assert(!r);
                    (void)r;
                }
                /* recalculate the hash */
                lyd_hash(parent);
                if (parent->parent && parent->parent->ht) {
                    /* re-add the list again */
                    r = lyht_insert(parent->parent->ht, &parent, parent->hash, NULL);
                    assert(!r);
                    (void)r;
                }
            } else if (!lyd_list_has_keys(parent)) {
                /* a parent is a list without keys so it cannot be a part of any parent hash */
                break;
            }
        }

        parent = parent->parent;
    }
}

static void
_lyd_insert_hash(struct lyd_node *node, int keyless_list_check)
{
    struct lyd_node *iter;
    int i;

    if (node->parent) {
        if ((node->schema->nodetype != LYS_LIST) || lyd_list_has_keys(node)) {
            if ((node->schema->nodetype == LYS_LEAF) && lys_is_key((struct lys_node_leaf *)node->schema, NULL)) {
                /* we are adding a key which means that it may be the last missing key for our parent's hash */
                if (!lyd_hash(node->parent)) {
                    /* yep, we successfully hashed node->parent so it is technically now added to its parent (hash-wise) */
                    _lyd_insert_hash(node->parent, 0);
                }
            }

            /* create parent hash table if required, otherwise just add the new child */
            if (!node->parent->ht) {
                for (i = 0, iter = node->parent->child; iter; ++i, iter = iter->next) {
                    if ((iter->schema->nodetype == LYS_LIST) && !lyd_list_has_keys(iter)) {
                        /* it will either never have keys and will never be hashed or has not all keys created yet */
                        --i;
                    }
                }
                assert(i <= LY_CACHE_HT_MIN_CHILDREN);
                if (i == LY_CACHE_HT_MIN_CHILDREN) {
                    /* create hash table, insert all the children */
                    node->parent->ht = lyht_new(1, sizeof(struct lyd_node *), lyd_hash_table_val_equal, NULL, 1);
                    LY_TREE_FOR(node->parent->child, iter) {
                        if ((iter->schema->nodetype == LYS_LIST) && !lyd_list_has_keys(iter)) {
                            /* skip lists without keys */
                            continue;
                        }

                        if (lyht_insert(node->parent->ht, &iter, iter->hash, NULL)) {
                            assert(0);
                        }
                    }
                }
            } else {
                if (lyht_insert(node->parent->ht, &node, node->hash, NULL)) {
                    assert(0);
                }
            }

            /* if node was in a state data subtree, wasn't it a part of a key-less list hash? */
            if (keyless_list_check) {
                lyd_keyless_list_hash_change(node->parent);
            }
        }
    }
}

/* we have inserted node into a parent */
void
lyd_insert_hash(struct lyd_node *node)
{
    _lyd_insert_hash(node, 1);
}

static void
_lyd_unlink_hash(struct lyd_node *node, struct lyd_node *orig_parent, int keyless_list_check)
{
#ifndef NDEBUG
    struct lyd_node *iter;

    /* it must already be unlinked otherwise keyless lists would get wrong hash */
    if (keyless_list_check && orig_parent) {
        LY_TREE_FOR(orig_parent->child, iter) {
            assert(iter != node);
        }
    }
#endif

    if (orig_parent) {
        if ((node->schema->nodetype != LYS_LIST) || lyd_list_has_keys(node)) {
            if (orig_parent->ht) {
                if (lyht_remove(orig_parent->ht, &node, node->hash)) {
                    assert(0);
                }

                /* if no longer enough children, free the whole hash table */
                if (orig_parent->ht->used < LY_CACHE_HT_MIN_CHILDREN) {
                    lyht_free(orig_parent->ht);
                    orig_parent->ht = NULL;
                }
            }

            /* if the parent is missing a key now, remove hash, also from parent */
            if (lys_is_key((struct lys_node_leaf *)node->schema, NULL) && orig_parent->hash) {
                assert((orig_parent->schema->nodetype == LYS_LIST) && !lyd_list_has_keys(orig_parent));

                _lyd_unlink_hash(orig_parent, orig_parent->parent, 0);
                orig_parent->hash = 0;
            }

            /* if node was in a state data subtree, shouldn't it be a part of a key-less list hash? */
            if (keyless_list_check) {
                lyd_keyless_list_hash_change(orig_parent);
            }
        }
    }
}

/* we are unlinking a child from a parent */
void
lyd_unlink_hash(struct lyd_node *node, struct lyd_node *orig_parent)
{
    _lyd_unlink_hash(node, orig_parent, 1);
}

#endif

/**
 * @brief get the list of \p data's siblings of the given schema
 */
static int
lyd_get_node_siblings(const struct lyd_node *data, const struct lys_node *schema, struct ly_set *set)
{
    const struct lyd_node *iter;

    assert(set && !set->number);
    assert(schema);
    assert(schema->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_ANYDATA | LYS_NOTIF | LYS_RPC |
                               LYS_ACTION));

    if (!data) {
        return 0;
    }

    LY_TREE_FOR(data, iter) {
        if (iter->schema == schema) {
            ly_set_add(set, (void*)iter, LY_SET_OPT_USEASLIST);
        }
    }

    return set->number;
}

/**
 * Check whether there are any "when" statements on a \p schema node and evaluate them.
 *
 * @return -1 on error, 0 on no when or evaluated to true, 1 on when evaluated to false
 */
static int
lyd_is_when_false(struct lyd_node *root, struct lyd_node *last_parent, struct lys_node *schema, int options)
{
    enum int_log_opts prev_ilo;
    struct lyd_node *current, *dummy;

    if ((!(options & LYD_OPT_TYPEMASK) || (options & (LYD_OPT_CONFIG | LYD_OPT_RPC | LYD_OPT_RPCREPLY | LYD_OPT_NOTIF | LYD_OPT_DATA_TEMPLATE)))
            && resolve_applies_when(schema, 1, last_parent ? last_parent->schema : NULL)) {
        /* evaluate when statements on a dummy data node */
        if (schema->nodetype == LYS_CHOICE) {
            schema = (struct lys_node *)lys_getnext(NULL, schema, NULL, LYS_GETNEXT_NOSTATECHECK);
        }
        dummy = lyd_new_dummy(root, last_parent, schema, NULL, 0);
        if (!dummy) {
            return -1;
        }
        if (!dummy->parent && root) {
            /* connect dummy nodes into the data tree, insert it before the root
             * to optimize later unlinking (lyd_free()) */
            lyd_insert_before(root, dummy);
        }
        for (current = dummy; current; current = current->child) {
            ly_ilo_change(NULL, ILO_IGNORE, &prev_ilo, NULL);
            resolve_when(current, 0, NULL);
            ly_ilo_restore(NULL, prev_ilo, NULL, 0);

            if (current->when_status & LYD_WHEN_FALSE) {
                /* when evaluates to false */
                lyd_free(dummy);
                return 1;
            }

            if (current->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                /* termination node without a child */
                break;
            }
        }
        lyd_free(dummy);
    }

    return 0;
}

/**
 * @param[in] root Root node to be able search the data tree in case of no instance
 * @return
 *  0 - all restrictions met
 *  1 - restrictions not met
 *  2 - schema node not enabled
 */
static int
lyd_check_mandatory_data(struct lyd_node *root, struct lyd_node *last_parent,
                         struct ly_set *instances, struct lys_node *schema, int options)
{
    struct ly_ctx *ctx = schema->module->ctx;
    uint32_t limit;
    uint16_t status;

    if (!instances->number) {
        /* no instance in the data tree - check if the instantiating is enabled
         * (check: if-feature, when, status data in non-status data tree)
         */
        status = (schema->flags & LYS_STATUS_MASK);
        if (lys_is_disabled(schema, 2) || (status && status != LYS_STATUS_CURR)) {
            /* disabled by if-feature */
            return EXIT_SUCCESS;
        } else if ((options & LYD_OPT_TRUSTED) || ((options & LYD_OPT_TYPEMASK) && (schema->flags & LYS_CONFIG_R))) {
            /* status schema node in non-status data tree */
            return EXIT_SUCCESS;
        } else if (lyd_is_when_false(root, last_parent, schema, options)) {
            return EXIT_SUCCESS;
        }
        /* the schema instance is not disabled by anything, continue with checking */
    }

    /* checking various mandatory conditions */
    switch (schema->nodetype) {
    case LYS_LEAF:
    case LYS_ANYXML:
    case LYS_ANYDATA:
        /* mandatory */
        if ((schema->flags & LYS_MAND_TRUE) && !instances->number) {
            LOGVAL(ctx, LYE_MISSELEM, LY_VLOG_LYD, last_parent, schema->name,
                   last_parent ? last_parent->schema->name : lys_node_module(schema)->name);
            return EXIT_FAILURE;
        }
        break;
    case LYS_LIST:
        /* min-elements */
        limit = ((struct lys_node_list *)schema)->min;
        if (limit && limit > instances->number) {
            LOGVAL(ctx, LYE_NOMIN, LY_VLOG_LYD, last_parent, schema->name);
            return EXIT_FAILURE;
        }
        /* max elements */
        limit = ((struct lys_node_list *)schema)->max;
        if (limit && limit < instances->number) {
            LOGVAL(ctx, LYE_NOMAX, LY_VLOG_LYD, instances->set.d[limit], schema->name);
            return EXIT_FAILURE;
        }

        break;

    case LYS_LEAFLIST:
        /* min-elements */
        limit = ((struct lys_node_leaflist *)schema)->min;
        if (limit && limit > instances->number) {
            LOGVAL(ctx, LYE_NOMIN, LY_VLOG_LYD, last_parent, schema->name);
            return EXIT_FAILURE;
        }
        /* max elements */
        limit = ((struct lys_node_leaflist *)schema)->max;
        if (limit && limit < instances->number) {
            LOGVAL(ctx, LYE_NOMAX, LY_VLOG_LYD, instances->set.d[limit], schema->name);
            return EXIT_FAILURE;
        }
        break;
    default:
        /* we cannot get here */
        assert(0);
        break;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Check the specific subtree, specified by \p schema node, for presence of mandatory nodes. Function goes
 * recursively into the subtree.
 *
 * What is being checked:
 * - mandatory statement in leaf, choice, anyxml and anydata
 * - min-elements and max-elements in list and leaf-list
 *
 * @param[in] tree Data tree, needed for case that subtree is NULL (in case of not existing data nodes to explore)
 * @param[in] subtree Depend ons \p toplevel flag:
 *                 toplevel = 1, then subtree is ignored, instead the tree is taken to search in top level data elements (if any)
 *                 toplevel = 0, subtree is the parent data node of the possible instances of the schema node being checked
 * @param[in] last_parent The last present parent data node (so it does not need to be a direct parent) of the possible
 *                 instances of the schema node being checked
 * @param[in] schema The schema node being checked for mandatory nodes
 * @param[in] toplevel, see the \p root parameter description
 * @param[in] options @ref parseroptions to specify the type of the data tree.
 * @return EXIT_SUCCESS or EXIT_FAILURE if there are missing mandatory nodes
 */
static int
lyd_check_mandatory_subtree(struct lyd_node *tree, struct lyd_node *subtree, struct lyd_node *last_parent,
                            struct lys_node *schema, int toplevel, int options)
{
    struct lys_node *siter, *siter_prev;
    struct lyd_node *iter;
    struct ly_set *present = NULL;
    unsigned int u;
    int ret = EXIT_FAILURE;

    assert(schema);

    if (schema->nodetype & (LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_ANYDATA | LYS_CONTAINER)) {
        /* data node */
        present = ly_set_new();
        if (!present) {
            goto error;
        }
        if ((toplevel && tree) || (!toplevel && subtree)) {
            if (toplevel) {
                lyd_get_node_siblings(tree, schema, present);
            } else {
                lyd_get_node_siblings(subtree->child, schema, present);
            }
        }
    }

    switch (schema->nodetype) {
    case LYS_LEAF:
    case LYS_LEAFLIST:
    case LYS_ANYXML:
    case LYS_ANYDATA:
        /* check the schema item */
        if (lyd_check_mandatory_data(tree, last_parent, present, schema, options)) {
            goto error;
        }
        break;
    case LYS_LIST:
        /* check the schema item */
        if (lyd_check_mandatory_data(tree, last_parent, present, schema, options)) {
            goto error;
        }

        /* go recursively */
        for (u = 0; u < present->number; u++) {
            LY_TREE_FOR(schema->child, siter) {
                if (lyd_check_mandatory_subtree(tree, present->set.d[u], present->set.d[u], siter, 0, options)) {
                    goto error;
                }
            }
        }
        break;

    case LYS_CONTAINER:
        if (present->number || !((struct lys_node_container *)schema)->presence) {
            /* if we have existing or non-presence container, go recursively */
            LY_TREE_FOR(schema->child, siter) {
                if (lyd_check_mandatory_subtree(tree, present->number ? present->set.d[0] : NULL,
                                                present->number ? present->set.d[0] : last_parent,
                                                siter, 0, options)) {
                    goto error;
                }
            }
        }
        break;
    case LYS_CHOICE:
        /* get existing node in the data tree from the choice */
        iter = NULL;
        if ((toplevel && tree) || (!toplevel && subtree)) {
            LY_TREE_FOR(toplevel ? tree : subtree->child, iter) {
                for (siter = lys_parent(iter->schema), siter_prev = iter->schema;
                        siter && (siter->nodetype & (LYS_CASE | LYS_USES | LYS_CHOICE));
                        siter_prev = siter, siter = lys_parent(siter)) {
                    if (siter == schema) {
                        /* we have the choice instance */
                        break;
                    }
                }
                if (siter == schema) {
                    /* we have the choice instance;
                     * the condition must be the same as in the loop because of
                     * choice's sibling nodes that break the loop, so siter is not NULL,
                     * but it is not the same as schema */
                    break;
                }
            }
        }
        if (!iter) {
            if (lyd_is_when_false(tree, last_parent, schema, options)) {
                /* nothing to check */
                break;
            }
            if (((struct lys_node_choice *)schema)->dflt) {
                /* there is a default case */
                if (lyd_check_mandatory_subtree(tree, subtree, last_parent, ((struct lys_node_choice *)schema)->dflt,
                                                toplevel, options)) {
                    goto error;
                }
            } else if (schema->flags & LYS_MAND_TRUE) {
                /* choice requires some data to be instantiated */
                LOGVAL(schema->module->ctx, LYE_NOMANDCHOICE, LY_VLOG_LYD, last_parent, schema->name);
                goto error;
            }
        } else {
            /* one of the choice's cases is instantiated, continue into this case */
            /* since iter != NULL, siter must be also != NULL and we also know siter_prev
             * which points to the child of schema leading towards the instantiated data */
            assert(siter && siter_prev);
            if (lyd_check_mandatory_subtree(tree, subtree, last_parent, siter_prev, toplevel, options)) {
                goto error;
            }
        }
        break;
    case LYS_NOTIF:
        /* skip if validating a notification */
        if (!(options & LYD_OPT_NOTIF)) {
            break;
        }
        /* fallthrough */
    case LYS_CASE:
    case LYS_USES:
    case LYS_INPUT:
    case LYS_OUTPUT:
        /* go recursively */
        LY_TREE_FOR(schema->child, siter) {
            if (lyd_check_mandatory_subtree(tree, subtree, last_parent, siter, toplevel, options)) {
                goto error;
            }
        }
        break;
    default:
        /* stop */
        break;
    }

    ret = EXIT_SUCCESS;

error:
    ly_set_free(present);
    return ret;
}

int
lyd_check_mandatory_tree(struct lyd_node *root, struct ly_ctx *ctx, const struct lys_module **modules, int mod_count,
                         int options)
{
    struct lys_node *siter;
    int i;

    assert(root || ctx);
    assert(!(options & LYD_OPT_ACT_NOTIF));

    if (options & (LYD_OPT_EDIT | LYD_OPT_GET | LYD_OPT_GETCONFIG)) {
        /* no check is needed */
        return EXIT_SUCCESS;
    }

    if (!ctx) {
        /* get context */
        ctx = root->schema->module->ctx;
    }

    if (!(options & LYD_OPT_TYPEMASK) || (options & (LYD_OPT_DATA | LYD_OPT_CONFIG))) {
        if (options & LYD_OPT_NOSIBLINGS) {
            if (root && lyd_check_mandatory_subtree(root, NULL, NULL, root->schema, 1, options)) {
                return EXIT_FAILURE;
            }
        } else if (modules && mod_count) {
            for (i = 0; i < mod_count; ++i) {
                LY_TREE_FOR(modules[i]->data, siter) {
                    if (!(siter->nodetype & (LYS_RPC | LYS_NOTIF)) &&
                            lyd_check_mandatory_subtree(root, NULL, NULL, siter, 1, options)) {
                        return EXIT_FAILURE;
                    }
                }
            }
        } else {
            for (i = (options & LYD_OPT_DATA_NO_YANGLIB) ? ctx->internal_module_count : ctx->internal_module_count - 1;
                 i < ctx->models.used;
                 i++) {
                /* skip not implemented and disabled modules */
                if (!ctx->models.list[i]->implemented || ctx->models.list[i]->disabled) {
                    continue;
                }
                LY_TREE_FOR(ctx->models.list[i]->data, siter) {
                    if (!(siter->nodetype & (LYS_RPC | LYS_NOTIF)) &&
                            lyd_check_mandatory_subtree(root, NULL, NULL, siter, 1, options)) {
                        return EXIT_FAILURE;
                    }
                }
            }
        }
    } else if (options & LYD_OPT_NOTIF) {
        if (!root || (root->schema->nodetype != LYS_NOTIF)) {
            LOGERR(ctx, LY_EINVAL, "Subtree is not a single notification.");
            return EXIT_FAILURE;
        }
        if (root->schema->child && lyd_check_mandatory_subtree(root, root, root, root->schema, 0, options)) {
            return EXIT_FAILURE;
        }
    } else if (options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY)) {
        if (!root || !(root->schema->nodetype & (LYS_RPC | LYS_ACTION))) {
            LOGERR(ctx, LY_EINVAL, "Subtree is not a single RPC/action/reply.");
            return EXIT_FAILURE;
        }
        if (options & LYD_OPT_RPC) {
            for (siter = root->schema->child; siter && siter->nodetype != LYS_INPUT; siter = siter->next);
        } else { /* LYD_OPT_RPCREPLY */
            for (siter = root->schema->child; siter && siter->nodetype != LYS_OUTPUT; siter = siter->next);
        }
        if (siter && lyd_check_mandatory_subtree(root, root, root, siter, 0, options)) {
            return EXIT_FAILURE;
        }
    } else if (options & LYD_OPT_DATA_TEMPLATE) {
        if (root && lyd_check_mandatory_subtree(root, NULL, NULL, root->schema, 1, options)) {
            return EXIT_FAILURE;
        }
    } else {
        LOGINT(ctx);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static struct lyd_node *
lyd_parse_(struct ly_ctx *ctx, const struct lyd_node *rpc_act, const char *data, LYD_FORMAT format, int options,
           const struct lyd_node *data_tree, const char *yang_data_name)
{
    struct lyxml_elem *xml;
    struct lyd_node *result = NULL;
    int xmlopt = LYXML_PARSE_MULTIROOT;

    if (!ctx || !data) {
        LOGARG;
        return NULL;
    }

    if (options & LYD_OPT_NOSIBLINGS) {
        xmlopt = 0;
    }

    /* we must free all the errors, otherwise we are unable to properly check returned ly_errno :-/ */
    ly_errno = LY_SUCCESS;
    switch (format) {
    case LYD_XML:
        xml = lyxml_parse_mem(ctx, data, xmlopt);
        if (ly_errno) {
            break;
        }
        if (options & LYD_OPT_RPCREPLY) {
            result = lyd_parse_xml(ctx, &xml, options, rpc_act, data_tree);
        } else if (options & (LYD_OPT_RPC | LYD_OPT_NOTIF)) {
            result = lyd_parse_xml(ctx, &xml, options, data_tree);
        } else if (options & LYD_OPT_DATA_TEMPLATE) {
            result = lyd_parse_xml(ctx, &xml, options, yang_data_name);
        } else {
            result = lyd_parse_xml(ctx, &xml, options);
        }
        lyxml_free_withsiblings(ctx, xml);
        break;
    case LYD_JSON:
        result = lyd_parse_json(ctx, data, options, rpc_act, data_tree, yang_data_name);
        break;
    case LYD_LYB:
        result = lyd_parse_lyb(ctx, data, options, data_tree, yang_data_name, NULL);
        break;
    default:
        /* error */
        break;
    }

    if (ly_errno) {
        lyd_free_withsiblings(result);
        return NULL;
    } else {
        return result;
    }
}

static struct lyd_node *
lyd_parse_data_(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options, va_list ap)
{
    const struct lyd_node *rpc_act = NULL, *data_tree = NULL, *iter;
    const char *yang_data_name = NULL;

    if (lyp_data_check_options(ctx, options, __func__)) {
        return NULL;
    }

    if (options & LYD_OPT_RPCREPLY) {
        rpc_act = va_arg(ap, const struct lyd_node *);
        if (!rpc_act || rpc_act->parent || !(rpc_act->schema->nodetype & (LYS_RPC | LYS_LIST | LYS_CONTAINER))) {
            LOGERR(ctx, LY_EINVAL, "%s: invalid variable parameter (const struct lyd_node *rpc_act).", __func__);
            return NULL;
        }
    }
    if (options & (LYD_OPT_RPC | LYD_OPT_NOTIF | LYD_OPT_RPCREPLY)) {
        data_tree = va_arg(ap, const struct lyd_node *);
        if (data_tree) {
            if (options & LYD_OPT_NOEXTDEPS) {
                LOGERR(ctx, LY_EINVAL, "%s: invalid parameter (variable arg const struct lyd_node *data_tree and LYD_OPT_NOEXTDEPS set).",
                       __func__);
                return NULL;
            }

            LY_TREE_FOR(data_tree, iter) {
                if (iter->parent) {
                    /* a sibling is not top-level */
                    LOGERR(ctx, LY_EINVAL, "%s: invalid variable parameter (const struct lyd_node *data_tree).", __func__);
                    return NULL;
                }
            }

            /* move it to the beginning */
            for (; data_tree->prev->next; data_tree = data_tree->prev);

            /* LYD_OPT_NOSIBLINGS cannot be set in this case */
            if (options & LYD_OPT_NOSIBLINGS) {
                LOGERR(ctx, LY_EINVAL, "%s: invalid parameter (variable arg const struct lyd_node *data_tree with LYD_OPT_NOSIBLINGS).", __func__);
                return NULL;
            }
        }
    }
    if (options & LYD_OPT_DATA_TEMPLATE) {
        yang_data_name = va_arg(ap, const char *);
    }

    return lyd_parse_(ctx, rpc_act, data, format, options, data_tree, yang_data_name);
}

API struct lyd_node *
lyd_parse_mem(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options, ...)
{
    va_list ap;
    struct lyd_node *result;

    va_start(ap, options);
    result = lyd_parse_data_(ctx, data, format, options, ap);
    va_end(ap);

    return result;
}

static struct lyd_node *
lyd_parse_fd_(struct ly_ctx *ctx, int fd, LYD_FORMAT format, int options, va_list ap)
{
    struct lyd_node *ret;
    size_t length;
    char *data;

    if (!ctx || (fd == -1)) {
        LOGARG;
        return NULL;
    }

    if (lyp_mmap(ctx, fd, 0, &length, (void **)&data)) {
        LOGERR(ctx, LY_ESYS, "Mapping file descriptor into memory failed (%s()).", __func__);
        return NULL;
    } else if (!data) {
        return NULL;
    }

    ret = lyd_parse_data_(ctx, data, format, options, ap);

    lyp_munmap(data, length);

    return ret;
}

API struct lyd_node *
lyd_parse_fd(struct ly_ctx *ctx, int fd, LYD_FORMAT format, int options, ...)
{
    struct lyd_node *ret;
    va_list ap;

    va_start(ap, options);
    ret = lyd_parse_fd_(ctx, fd, format, options, ap);
    va_end(ap);

    return ret;
}

API struct lyd_node *
lyd_parse_path(struct ly_ctx *ctx, const char *path, LYD_FORMAT format, int options, ...)
{
    int fd;
    struct lyd_node *ret;
    va_list ap;

    if (!ctx || !path) {
        LOGARG;
        return NULL;
    }

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        LOGERR(ctx, LY_ESYS, "Failed to open data file \"%s\" (%s).", path, strerror(errno));
        return NULL;
    }

    va_start(ap, options);
    ret = lyd_parse_fd_(ctx, fd, format, options, ap);

    va_end(ap);
    close(fd);

    return ret;
}

static struct lys_node *
lyd_new_find_schema(struct lyd_node *parent, const struct lys_module *module, int rpc_output)
{
    struct lys_node *siblings;

    if (!parent) {
        siblings = module->data;
    } else {
        if (!parent->schema) {
            return NULL;
        }
        siblings = parent->schema->child;
        if (siblings && (siblings->nodetype == (rpc_output ? LYS_INPUT : LYS_OUTPUT))) {
            siblings = siblings->next;
        }
        if (siblings && (siblings->nodetype == (rpc_output ? LYS_OUTPUT : LYS_INPUT))) {
            siblings = siblings->child;
        }
    }

    return siblings;
}

struct lyd_node *
_lyd_new(struct lyd_node *parent, const struct lys_node *schema, int dflt)
{
    struct lyd_node *ret;

    ret = calloc(1, sizeof *ret);
    LY_CHECK_ERR_RETURN(!ret, LOGMEM(schema->module->ctx), NULL);

    ret->schema = (struct lys_node *)schema;
    ret->validity = ly_new_node_validity(schema);
    if (resolve_applies_when(schema, 0, NULL)) {
        ret->when_status = LYD_WHEN;
    }
    ret->prev = ret;
    ret->dflt = dflt;

#ifdef LY_ENABLED_CACHE
    lyd_hash(ret);
#endif

    if (parent) {
        if (lyd_insert(parent, ret)) {
            lyd_free(ret);
            return NULL;
        }
    }
    return ret;
}

API struct lyd_node *
lyd_new(struct lyd_node *parent, const struct lys_module *module, const char *name)
{
    const struct lys_node *snode = NULL, *siblings;

    if ((!parent && !module) || !name) {
        LOGARG;
        return NULL;
    }

    siblings = lyd_new_find_schema(parent, module, 0);
    if (!siblings) {
        LOGARG;
        return NULL;
    }

    if (lys_getnext_data(module, lys_parent(siblings), name, strlen(name), LYS_CONTAINER | LYS_LIST | LYS_NOTIF
                         | LYS_RPC | LYS_ACTION, &snode) || !snode) {
        LOGERR(siblings->module->ctx, LY_EINVAL, "Failed to find \"%s\" as a sibling to \"%s:%s\".",
               name, lys_node_module(siblings)->name, siblings->name);
        return NULL;
    }

    return _lyd_new(parent, snode, 0);
}

static struct lyd_node *
lyd_create_leaf(const struct lys_node *schema, const char *val_str, int dflt)
{
    struct lyd_node_leaf_list *ret;

    ret = calloc(1, sizeof *ret);
    LY_CHECK_ERR_RETURN(!ret, LOGMEM(schema->module->ctx), NULL);

    ret->schema = (struct lys_node *)schema;
    ret->validity = ly_new_node_validity(schema);
    if (resolve_applies_when(schema, 0, NULL)) {
        ret->when_status = LYD_WHEN;
    }
    ret->prev = (struct lyd_node *)ret;
    ret->value_type = ((struct lys_node_leaf *)schema)->type.base;
    ret->value_str = lydict_insert(schema->module->ctx, val_str ? val_str : "", 0);
    ret->dflt = dflt;

#ifdef LY_ENABLED_CACHE
    lyd_hash((struct lyd_node *)ret);
#endif

    return (struct lyd_node *)ret;
}

static struct lyd_node *
_lyd_new_leaf(struct lyd_node *parent, const struct lys_node *schema, const char *val_str, int dflt, int edit_leaf)
{
    struct lyd_node *ret;

    ret = lyd_create_leaf(schema, val_str, dflt);
    if (!ret) {
        return NULL;
    }

    /* connect to parent */
    if (parent) {
        if (lyd_insert(parent, ret)) {
            lyd_free(ret);
            return NULL;
        }
    }

    if (edit_leaf && !((struct lyd_node_leaf_list *)ret)->value_str[0]) {
        /* empty edit leaf, it is fine */
        ((struct lyd_node_leaf_list *)ret)->value_type = LY_TYPE_UNKNOWN;
        return ret;
    }

    /* resolve the type correctly (after it was connected to parent cause of log) */
    if (!lyp_parse_value(&((struct lys_node_leaf *)ret->schema)->type, &((struct lyd_node_leaf_list *)ret)->value_str,
                         NULL, (struct lyd_node_leaf_list *)ret, NULL, NULL, 1, dflt, 0)) {
        lyd_free(ret);
        return NULL;
    }

    if ((ret->schema->nodetype == LYS_LEAF) && (ret->schema->flags & LYS_UNIQUE)) {
        for (; parent && (parent->schema->nodetype != LYS_LIST); parent = parent->parent);
        if (parent) {
            parent->validity |= LYD_VAL_UNIQUE;
        } else {
            LOGINT(schema->module->ctx);
        }
    }

    return ret;
}

API struct lyd_node *
lyd_new_leaf(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *val_str)
{
    const struct lys_node *snode = NULL, *siblings;

    if ((!parent && !module) || !name) {
        LOGARG;
        return NULL;
    }

    siblings = lyd_new_find_schema(parent, module, 0);
    if (!siblings) {
        LOGARG;
        return NULL;
    }

    if (lys_getnext_data(module, lys_parent(siblings), name, strlen(name), LYS_LEAFLIST | LYS_LEAF, &snode) || !snode) {
        LOGERR(siblings->module->ctx, LY_EINVAL, "Failed to find \"%s\" as a sibling to \"%s:%s\".",
               name, lys_node_module(siblings)->name, siblings->name);
        return NULL;
    }

    return _lyd_new_leaf(parent, snode, val_str, 0, 0);
}

/**
 * @brief Update (add) default flag of the parents of the added node.
 *
 * @param[in] node Added node
 */
static void
lyd_wd_update_parents(struct lyd_node *node)
{
    struct lyd_node *parent = node->parent, *iter;

    for (parent = node->parent; parent; parent = node->parent) {
        if (parent->dflt || parent->schema->nodetype != LYS_CONTAINER ||
                ((struct lys_node_container *)parent->schema)->presence) {
            /* parent is already default and there is nothing to update or
             * it is not a non-presence container -> stop the loop */
            break;
        }
        /* check that there is still some non default sibling */
        for (iter = node->prev; iter != node; iter = iter->prev) {
            if (!iter->dflt) {
                break;
            }
        }
        if (iter == node && node->prev != node) {
            /* all siblings are implicit default nodes, propagate it to the parent */
            node = node->parent;
            node->dflt = 1;
            continue;
        } else {
            /* stop the loop */
            break;
        }
    }
}

/* op - 0 add, 1 del, 2 mod (add + del) */
static void
check_leaf_list_backlinks(struct lyd_node *node, int op)
{
    struct lyd_node *next, *iter;
    struct lyd_node_leaf_list *leaf_list;
    struct ly_set *set, *data;
    uint32_t i, j;
    int validity_changed = 0;

    assert((op == 0) || (op == 1) || (op == 2));

    /* fix leafrefs */
    LY_TREE_DFS_BEGIN(node, next, iter) {
        /* the node is target of a leafref */
        if ((iter->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) && iter->schema->child) {
            set = (struct ly_set *)iter->schema->child;
            for (i = 0; i < set->number; i++) {
                data = lyd_find_instance(iter, set->set.s[i]);
                if (data) {
                    for (j = 0; j < data->number; j++) {
                        leaf_list = (struct lyd_node_leaf_list *)data->set.d[j];
                        if (((op != 0) && (leaf_list->value_type == LY_TYPE_LEAFREF) && (leaf_list->value.leafref == iter))
                                || ((op != 1) && (leaf_list->value_flags & LY_VALUE_UNRES))) {
                            /* invalidate the leafref, a change concerning it happened */
                            leaf_list->validity |= LYD_VAL_LEAFREF;
                            validity_changed = 1;
                            if (leaf_list->value_type == LY_TYPE_LEAFREF) {
                                /* remove invalid link and put unresolved value back */
                                lyp_parse_value(&((struct lys_node_leaf *)leaf_list->schema)->type, &leaf_list->value_str,
                                                NULL, leaf_list, NULL, NULL, 1, leaf_list->dflt, 0);
                            }
                        }
                    }
                    ly_set_free(data);
                } else {
                    LOGINT(node->schema->module->ctx);
                    return;
                }
            }
        }
        LY_TREE_DFS_END(node, next, iter)
    }

    /* invalidate parent to make sure it will be checked in future validation */
    if (validity_changed && node->parent) {
        node->parent->validity |= LYD_VAL_MAND;
    }
}

API int
lyd_change_leaf(struct lyd_node_leaf_list *leaf, const char *val_str)
{
    const char *backup;
    int val_change, dflt_change;
    struct lyd_node *parent;

    if (!leaf || (leaf->schema->nodetype != LYS_LEAF)) {
        LOGARG;
        return -1;
    }

    backup = leaf->value_str;
    leaf->value_str = lydict_insert(leaf->schema->module->ctx, val_str ? val_str : "", 0);
    /* leaf->value is erased by lyp_parse_value() */

    /* parse the type correctly, makes the value canonical if needed */
    if (!lyp_parse_value(&((struct lys_node_leaf *)leaf->schema)->type, &leaf->value_str, NULL, leaf, NULL, NULL, 1, 0, 0)) {
        lydict_remove(leaf->schema->module->ctx, backup);
        return -1;
    }

    if (!strcmp(backup, leaf->value_str)) {
        /* the value remains the same */
        val_change = 0;
    } else {
        val_change = 1;
    }

    /* value is correct, remove backup */
    lydict_remove(leaf->schema->module->ctx, backup);

    /* clear the default flag, the value is different */
    if (leaf->dflt) {
        for (parent = (struct lyd_node *)leaf; parent; parent = parent->parent) {
            parent->dflt = 0;
        }
        dflt_change = 1;
    } else {
        dflt_change = 0;
    }

    if (val_change) {
        /* make the node non-validated */
        leaf->validity = ly_new_node_validity(leaf->schema);

        /* check possible leafref backlinks */
        check_leaf_list_backlinks((struct lyd_node *)leaf, 2);
    }

    if (val_change && (leaf->schema->flags & LYS_UNIQUE)) {
        for (parent = leaf->parent; parent && (parent->schema->nodetype != LYS_LIST); parent = parent->parent);
        if (parent) {
            parent->validity |= LYD_VAL_UNIQUE;
        } else {
            LOGINT(leaf->schema->module->ctx);
            return -1;
        }
    }

    return (val_change || dflt_change ? 0 : 1);
}

static struct lyd_node *
lyd_create_anydata(struct lyd_node *parent, const struct lys_node *schema, void *value,
                   LYD_ANYDATA_VALUETYPE value_type)
{
    struct lyd_node *iter;
    struct lyd_node_anydata *ret;
    int len;

    ret = calloc(1, sizeof *ret);
    LY_CHECK_ERR_RETURN(!ret, LOGMEM(schema->module->ctx), NULL);

    ret->schema = (struct lys_node *)schema;
    ret->validity = ly_new_node_validity(schema);
    if (resolve_applies_when(schema, 0, NULL)) {
        ret->when_status = LYD_WHEN;
    }
    ret->prev = (struct lyd_node *)ret;

    /* set the value */
    switch (value_type) {
    case LYD_ANYDATA_CONSTSTRING:
    case LYD_ANYDATA_SXML:
    case LYD_ANYDATA_JSON:
        ret->value.str = lydict_insert(schema->module->ctx, (const char *)value, 0);
        break;
    case LYD_ANYDATA_STRING:
    case LYD_ANYDATA_SXMLD:
    case LYD_ANYDATA_JSOND:
        ret->value.str = lydict_insert_zc(schema->module->ctx, (char *)value);
        value_type &= ~LYD_ANYDATA_STRING; /* make const string from string */
        break;
    case LYD_ANYDATA_DATATREE:
        ret->value.tree = (struct lyd_node *)value;
        break;
    case LYD_ANYDATA_XML:
        ret->value.xml = (struct lyxml_elem *)value;
        break;
    case LYD_ANYDATA_LYB:
        len = lyd_lyb_data_length(value);
        if (len == -1) {
            LOGERR(schema->module->ctx, LY_EINVAL, "Invalid LYB data.");
            return NULL;
        }
        ret->value.mem = malloc(len);
        LY_CHECK_ERR_RETURN(!ret->value.mem, LOGMEM(schema->module->ctx); free(ret), NULL);
        memcpy(ret->value.mem, value, len);
        break;
    case LYD_ANYDATA_LYBD:
        ret->value.mem = value;
        value_type &= ~LYD_ANYDATA_STRING; /* make const string from string */
        break;
    }
    ret->value_type = value_type;

#ifdef LY_ENABLED_CACHE
    lyd_hash((struct lyd_node *)ret);
#endif

    /* connect to parent */
    if (parent) {
        if (lyd_insert(parent, (struct lyd_node*)ret)) {
            lyd_free((struct lyd_node*)ret);
            return NULL;
        }

        /* remove the flag from parents */
        for (iter = parent; iter && iter->dflt; iter = iter->parent) {
            iter->dflt = 0;
        }
    }

    return (struct lyd_node*)ret;
}

API struct lyd_node *
lyd_new_anydata(struct lyd_node *parent, const struct lys_module *module, const char *name,
                void *value, LYD_ANYDATA_VALUETYPE value_type)
{
    const struct lys_node *siblings, *snode;

    if ((!parent && !module) || !name) {
        LOGARG;
        return NULL;
    }

    siblings = lyd_new_find_schema(parent, module, 0);
    if (!siblings) {
        LOGARG;
        return NULL;
    }

    if (lys_getnext_data(module, lys_parent(siblings), name, strlen(name), LYS_ANYDATA, &snode) || !snode) {
        LOGERR(siblings->module->ctx, LY_EINVAL, "Failed to find \"%s\" as a sibling to \"%s:%s\".",
               name, lys_node_module(siblings)->name, siblings->name);
        return NULL;
    }

    return lyd_create_anydata(parent, snode, value, value_type);
}

API struct lyd_node *
lyd_new_yangdata(const struct lys_module *module, const char *name_template, const char *name)
{
    const struct lys_node *schema = NULL, *snode;

    if (!module || !name_template || !name) {
        LOGARG;
        return NULL;
    }

    schema = lyp_get_yang_data_template(module, name_template, strlen(name_template));
    if (!schema) {
        LOGERR(module->ctx, LY_EINVAL, "Failed to find yang-data template \"%s\".", name_template);
        return NULL;
    }

    if (lys_getnext_data(module, schema, name, strlen(name), LYS_CONTAINER, &snode) || !snode) {
        LOGERR(module->ctx, LY_EINVAL, "Failed to find \"%s\" as a container child of \"%s:%s\".",
               name, module->name, schema->name);
        return NULL;
    }

    return _lyd_new(NULL, snode, 0);
}

API struct lyd_node *
lyd_new_output(struct lyd_node *parent, const struct lys_module *module, const char *name)
{
    const struct lys_node *snode = NULL, *siblings;

    if ((!parent && !module) || !name) {
        LOGARG;
        return NULL;
    }

    siblings = lyd_new_find_schema(parent, module, 1);
    if (!siblings) {
        LOGARG;
        return NULL;
    }

    if (lys_getnext_data(module, lys_parent(siblings), name, strlen(name), LYS_CONTAINER | LYS_LIST | LYS_NOTIF
                         | LYS_RPC | LYS_ACTION, &snode) || !snode) {
        LOGERR(siblings->module->ctx, LY_EINVAL, "Failed to find \"%s\" as a sibling to \"%s:%s\".",
               name, lys_node_module(siblings)->name, siblings->name);
        return NULL;
    }

    return _lyd_new(parent, snode, 0);
}

API struct lyd_node *
lyd_new_output_leaf(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *val_str)
{
    const struct lys_node *snode = NULL, *siblings;

    if ((!parent && !module) || !name) {
        LOGARG;
        return NULL;
    }

    siblings = lyd_new_find_schema(parent, module, 1);
    if (!siblings) {
        LOGARG;
        return NULL;
    }

    if (lys_getnext_data(module, lys_parent(siblings), name, strlen(name), LYS_LEAFLIST | LYS_LEAF, &snode) || !snode) {
        LOGERR(siblings->module->ctx, LY_EINVAL, "Failed to find \"%s\" as a sibling to \"%s:%s\".",
               name, lys_node_module(siblings)->name, siblings->name);
        return NULL;
    }

    return _lyd_new_leaf(parent, snode, val_str, 0, 0);
}

API struct lyd_node *
lyd_new_output_anydata(struct lyd_node *parent, const struct lys_module *module, const char *name,
                       void *value, LYD_ANYDATA_VALUETYPE value_type)
{
    const struct lys_node *siblings, *snode;

    if ((!parent && !module) || !name) {
        LOGARG;
        return NULL;
    }

    siblings = lyd_new_find_schema(parent, module, 1);
    if (!siblings) {
        LOGARG;
        return NULL;
    }

    if (lys_getnext_data(module, lys_parent(siblings), name, strlen(name), LYS_ANYDATA, &snode) || !snode) {
        LOGERR(siblings->module->ctx, LY_EINVAL, "Failed to find \"%s\" as a sibling to \"%s:%s\".",
               name, lys_node_module(siblings)->name, siblings->name);
        return NULL;
    }

    return lyd_create_anydata(parent, snode, value, value_type);
}

static int
lyd_new_path_list_predicate(struct lyd_node *list, const char *list_name, const char *predicate, int *parsed)
{
    const char *mod_name, *name, *value;
    char *key_val;
    int r, i, mod_name_len, nam_len, val_len, has_predicate;
    struct lys_node_list *slist;
    struct lys_node *key;

    slist = (struct lys_node_list *)list->schema;

    /* is the predicate a number? */
    if (((r = parse_schema_json_predicate(predicate, &mod_name, &mod_name_len, &name, &nam_len, &value, &val_len, &has_predicate)) < 1)
            || !strncmp(name, ".", nam_len)) {
        LOGVAL(slist->module->ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, predicate[-r], &predicate[-r]);
        return -1;
    }

    if (isdigit(name[0])) {
        /* position index - creating without keys */
        *parsed += r;
        return 0;
    }

    /* it's not a number, so there must be some keys */
    if (!slist->keys_size) {
        /* there are none, so pretend we did not parse anything to get invalid char error later */
        return 0;
    }

    /* go through all the keys */
    i = 0;
    goto check_parsed_values;

    for (; i < slist->keys_size; ++i) {
        if (!has_predicate) {
            LOGVAL(slist->module->ctx, LYE_PATH_MISSKEY, LY_VLOG_NONE, NULL, list_name);
            return -1;
        }

        if (((r = parse_schema_json_predicate(predicate, &mod_name, &mod_name_len, &name, &nam_len, &value, &val_len, &has_predicate)) < 1)
                || !strncmp(name, ".", nam_len)) {
            LOGVAL(slist->module->ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, predicate[-r], &predicate[-r]);
            return -1;
        }

check_parsed_values:
        key = (struct lys_node *)slist->keys[i];
        *parsed += r;
        predicate += r;

        if (!value || (!mod_name && (lys_node_module(key) != lys_node_module((struct lys_node *)slist)))
                || (mod_name && (strncmp(lys_node_module(key)->name, mod_name, mod_name_len) || lys_node_module(key)->name[mod_name_len]))
                || strncmp(key->name, name, nam_len) || key->name[nam_len]) {
            LOGVAL(slist->module->ctx, LYE_PATH_INKEY, LY_VLOG_NONE, NULL, name);
            return -1;
        }

        key_val = malloc((val_len + 1) * sizeof(char));
        LY_CHECK_ERR_RETURN(!key_val, LOGMEM(slist->module->ctx), -1);
        strncpy(key_val, value, val_len);
        key_val[val_len] = '\0';

        if (!_lyd_new_leaf(list, key, key_val, 0, 0)) {
            free(key_val);
            return -1;
        }
        free(key_val);
    }

    return 0;
}

static struct lyd_node *
lyd_new_path_update(struct lyd_node *node, void *value, LYD_ANYDATA_VALUETYPE value_type, int dflt)
{
    struct ly_ctx *ctx = node->schema->module->ctx;
    struct lyd_node_anydata *any;
    int len;

    switch (node->schema->nodetype) {
    case LYS_LEAF:
        if (value_type > LYD_ANYDATA_STRING) {
            LOGARG;
            return NULL;
        }

        if (lyd_change_leaf((struct lyd_node_leaf_list *)node, value) == 0) {
            /* there was an actual change */
            if (dflt) {
                node->dflt = 1;
            }
            return node;
        }

        if (dflt) {
            /* maybe the value is the same, but the node is default now */
            node->dflt = 1;
            return node;
        }

        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        /* the nodes are the same if:
         * 1) the value types are strings (LYD_ANYDATA_STRING and LYD_ANYDATA_CONSTSTRING equals)
         *    and the strings equals
         * 2) the value types are the same, but not strings and the pointers (not the content) are the
         *    same
         */
        any = (struct lyd_node_anydata *)node;
        if (any->value_type <= LYD_ANYDATA_STRING && value_type <= LYD_ANYDATA_STRING) {
            if (ly_strequal(any->value.str, (char *)value, 0)) {
                /* values are the same */
                return NULL;
            }
        } else if (any->value_type == value_type) {
            /* compare pointers */
            if ((void *)any->value.tree == value) {
                /* values are the same */
                return NULL;
            }
        }

        /* values are not the same - 1) remove the old one ... */
        switch (any->value_type) {
        case LYD_ANYDATA_CONSTSTRING:
        case LYD_ANYDATA_SXML:
        case LYD_ANYDATA_JSON:
            lydict_remove(ctx, any->value.str);
            break;
        case LYD_ANYDATA_DATATREE:
            lyd_free_withsiblings(any->value.tree);
            break;
        case LYD_ANYDATA_XML:
            lyxml_free_withsiblings(ctx, any->value.xml);
            break;
        case LYD_ANYDATA_LYB:
            free(any->value.mem);
            break;
        case LYD_ANYDATA_STRING:
        case LYD_ANYDATA_SXMLD:
        case LYD_ANYDATA_JSOND:
        case LYD_ANYDATA_LYBD:
            /* dynamic strings are used only as input parameters */
            assert(0);
            break;
        }
        /* ... and 2) store the new one */
        switch (value_type) {
        case LYD_ANYDATA_CONSTSTRING:
        case LYD_ANYDATA_SXML:
        case LYD_ANYDATA_JSON:
            any->value.str = lydict_insert(ctx, (const char *)value, 0);
            break;
        case LYD_ANYDATA_STRING:
        case LYD_ANYDATA_SXMLD:
        case LYD_ANYDATA_JSOND:
            any->value.str = lydict_insert_zc(ctx, (char *)value);
            value_type &= ~LYD_ANYDATA_STRING; /* make const string from string */
            break;
        case LYD_ANYDATA_DATATREE:
            any->value.tree = value;
            break;
        case LYD_ANYDATA_XML:
            any->value.xml = value;
            break;
        case LYD_ANYDATA_LYB:
            len = lyd_lyb_data_length(value);
            if (len == -1) {
                LOGERR(ctx, LY_EINVAL, "Invalid LYB data.");
                return NULL;
            }
            any->value.mem = malloc(len);
            LY_CHECK_ERR_RETURN(!any->value.mem, LOGMEM(ctx), NULL);
            memcpy(any->value.mem, value, len);
            break;
        case LYD_ANYDATA_LYBD:
            any->value.mem = value;
            value_type &= ~LYD_ANYDATA_STRING; /* make const string from string */
            break;
        }
        return node;
    default:
        /* nothing needed - containers, lists and leaf-lists do not have value or it cannot be changed */
        break;
    }

    /* not updated */
    return NULL;
}

API struct lyd_node *
lyd_new_path(struct lyd_node *data_tree, struct ly_ctx *ctx, const char *path, void *value,
             LYD_ANYDATA_VALUETYPE value_type, int options)
{
    char *str;
    const char *mod_name, *name, *val_name, *val, *node_mod_name, *id, *backup_mod_name = NULL, *yang_data_name = NULL;
    struct lyd_node *ret = NULL, *node, *parent = NULL;
    const struct lys_node *schild, *sparent, *tmp;
    const struct lys_node_list *slist;
    const struct lys_module *module, *prev_mod;
    int r, i, parsed = 0, mod_name_len, nam_len, val_name_len, val_len;
    int is_relative = -1, has_predicate, first_iter = 1, edit_leaf;
    int backup_is_relative, backup_mod_name_len, yang_data_name_len;

    if (!path || (!data_tree && !ctx)
            || (!data_tree && (path[0] != '/'))) {
        LOGARG;
        return NULL;
    }

    if (!ctx) {
        ctx = data_tree->schema->module->ctx;
    }

    id = path;

    if (data_tree) {
        /* go through all the siblings and try to find the right parent, if exists,
         * first go through all the next siblings keeping the original order, for positional predicates */
        LY_TREE_FOR(data_tree, node) {
            parent = resolve_partial_json_data_nodeid(id, value_type > LYD_ANYDATA_STRING ? NULL : value, node,
                                                      options, &parsed);
            if (parsed) {
                break;
            }
        }
        for (node = data_tree->prev; !parsed && node->next; node = node->prev) {
            parent = resolve_partial_json_data_nodeid(id, value_type > LYD_ANYDATA_STRING ? NULL : value, node,
                                                      options, &parsed);
        }
        if (parsed == -1) {
            return NULL;
        }
        if (parsed) {
            assert(parent);
            /* if we parsed something we have a relative path now for sure, otherwise we don't know */
            is_relative = 1;

            id += parsed;

            if (!id[0]) {
                /* the node exists, are we supposed to update it or is it default? */
                if (!(options & LYD_PATH_OPT_UPDATE) && (!parent->dflt || (options & LYD_PATH_OPT_DFLT))) {
                    LOGVAL(ctx, LYE_PATH_EXISTS, LY_VLOG_STR, path);
                    return NULL;
                }

                /* no change, the default node already exists */
                if (parent->dflt && (options & LYD_PATH_OPT_DFLT)) {
                    return NULL;
                }

                return lyd_new_path_update(parent, value, value_type, options & LYD_PATH_OPT_DFLT);
            }
        }
    }

    backup_is_relative = is_relative;
    if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, NULL, NULL, 1)) < 1) {
        LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
        return NULL;
    }

    if (name[0] == '#') {
        if (is_relative) {
            LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, '#', name);
            return NULL;
        }
        yang_data_name = name + 1;
        yang_data_name_len = nam_len - 1;
        backup_mod_name = mod_name;
        backup_mod_name_len = mod_name_len;
        /* move to the next node in the path */
        id += r;
    } else {
        is_relative = backup_is_relative;
    }

    if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, &has_predicate, NULL, 0)) < 1) {
        LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
        return NULL;
    }
    /* move to the next node in the path */
    id += r;

    if (backup_mod_name) {
        mod_name = backup_mod_name;
        mod_name_len = backup_mod_name_len;
    }

    /* prepare everything for the schema search loop */
    if (is_relative) {
        /* we are relative to data_tree or parent if some part of the path already exists */
        if (!data_tree) {
            LOGERR(ctx, LY_EINVAL, "%s: provided relative path (%s) without context node.", path);
            return NULL;
        } else if (!parent) {
            parent = data_tree;
        }
        sparent = parent->schema;
        module = prev_mod = lys_node_module(sparent);
    } else {
        /* we are starting from scratch, absolute path */
        assert(!parent);
        if (!mod_name) {
            str = strndup(path, (name + nam_len) - path);
            LOGVAL(ctx, LYE_PATH_MISSMOD, LY_VLOG_STR, str);
            free(str);
            return NULL;
        }

        module = ly_ctx_nget_module(ctx, mod_name, mod_name_len, NULL, 1);

        if (!module) {
            str = strndup(path, (mod_name + mod_name_len) - path);
            LOGVAL(ctx, LYE_PATH_INMOD, LY_VLOG_STR, str);
            free(str);
            return NULL;
        }
        mod_name = NULL;
        mod_name_len = 0;
        prev_mod = module;

        sparent = NULL;
        if (yang_data_name) {
            sparent = lyp_get_yang_data_template(module, yang_data_name, yang_data_name_len);
            if (!sparent) {
                str = strndup(path, (yang_data_name + yang_data_name_len) - path);
                LOGVAL(ctx, LYE_PATH_INNODE, LY_VLOG_STR, str);
                free(str);
                return NULL;
            }
        }
    }

    /* create nodes in a loop */
    while (1) {
        /* find the schema node */
        schild = NULL;
        while ((schild = lys_getnext(schild, sparent, module, 0))) {
            if (schild->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST
                                    | LYS_ANYDATA | LYS_NOTIF | LYS_RPC | LYS_ACTION)) {
                /* module comparison */
                if (mod_name) {
                    node_mod_name = lys_node_module(schild)->name;
                    if (strncmp(node_mod_name, mod_name, mod_name_len) || node_mod_name[mod_name_len]) {
                        continue;
                    }
                } else if (lys_node_module(schild) != prev_mod) {
                    continue;
                }

                /* name check */
                if (strncmp(schild->name, name, nam_len) || schild->name[nam_len]) {
                    continue;
                }

                /* RPC/action in/out check */
                for (tmp = lys_parent(schild); tmp && (tmp->nodetype == LYS_USES); tmp = lys_parent(tmp));
                if (tmp) {
                    if (options & LYD_PATH_OPT_OUTPUT) {
                        if (tmp->nodetype == LYS_INPUT) {
                            continue;
                        }
                    } else {
                        if (tmp->nodetype == LYS_OUTPUT) {
                            continue;
                        }
                    }
                }

                break;
            }
        }

        if (!schild) {
            str = strndup(path, (name + nam_len) - path);
            LOGVAL(ctx, LYE_PATH_INNODE, LY_VLOG_STR, str);
            free(str);
            lyd_free(ret);
            return NULL;
        }

        /* we have the right schema node */
        switch (schild->nodetype) {
        case LYS_CONTAINER:
        case LYS_LIST:
        case LYS_NOTIF:
        case LYS_RPC:
        case LYS_ACTION:
            if (options & LYD_PATH_OPT_NOPARENT) {
                /* these were supposed to exist */
                str = strndup(path, (name + nam_len) - path);
                LOGVAL(ctx, LYE_PATH_MISSPAR, LY_VLOG_STR, str);
                free(str);
                lyd_free(ret);
                return NULL;
            }
            node = _lyd_new(is_relative ? parent : NULL, schild, (options & LYD_PATH_OPT_DFLT) ? 1 : 0);
            break;
        case LYS_LEAF:
        case LYS_LEAFLIST:
            str = NULL;
            if (has_predicate) {
                if ((r = parse_schema_json_predicate(id, NULL, NULL, &val_name, &val_name_len, &val, &val_len, &has_predicate)) < 1) {
                    LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
                    lyd_free(ret);
                    return NULL;
                }
                id += r;

                if ((val_name[0] != '.') || (val_name_len != 1)) {
                    LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, val_name[0], val_name);
                    lyd_free(ret);
                    return NULL;
                }

                str = strndup(val, val_len);
                if (!str) {
                    LOGMEM(ctx);
                    lyd_free(ret);
                    return NULL;
                }
            }
            if (id[0]) {
                LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[0], id);
                free(str);
                lyd_free(ret);
                return NULL;
            }

            if ((options & LYD_PATH_OPT_EDIT) && schild->nodetype == LYS_LEAF) {
                edit_leaf = 1;
            } else {
                edit_leaf = 0;
            }
            node = _lyd_new_leaf(is_relative ? parent : NULL, schild, (str ? str : value),
                                 (options & LYD_PATH_OPT_DFLT) ? 1 : 0, edit_leaf);
            free(str);
            break;
        case LYS_ANYXML:
        case LYS_ANYDATA:
            if (id[0]) {
                LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[0], id);
                lyd_free(ret);
                return NULL;
            }
            if (value_type <= LYD_ANYDATA_STRING && !value) {
                value_type = LYD_ANYDATA_CONSTSTRING;
                value = "";
            }
            node = lyd_create_anydata(is_relative ? parent : NULL, schild, value, value_type);
            break;
        default:
            LOGINT(ctx);
            node = NULL;
            break;
        }

        if (!node) {
            str = strndup(path, id - path);
            if (is_relative) {
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_STR, str, "Failed to create node \"%s\" as a child of \"%s\".",
                       schild->name, parent->schema->name);
            } else {
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_STR, str, "Failed to create node \"%s\".", schild->name);
            }
            free(str);
            lyd_free(ret);
            return NULL;
        }
        /* special case when we are creating a sibling of a top-level data node */
        if (!is_relative) {
            if (data_tree) {
                for (; data_tree->next; data_tree = data_tree->next);
                if (lyd_insert_after(data_tree, node)) {
                    lyd_free(ret);
                    return NULL;
                }
            }
            is_relative = 1;
        }

        if (first_iter) {
            /* sort if needed, but only when inserted somewhere */
            sparent = node->schema;
            do {
                sparent = lys_parent(sparent);
            } while (sparent && (sparent->nodetype != ((options & LYD_PATH_OPT_OUTPUT) ? LYS_OUTPUT : LYS_INPUT)));
            if (sparent && lyd_schema_sort(node, 0)) {
                lyd_free(ret);
                return NULL;
            }

            /* set first created node */
            ret = node;
            first_iter = 0;
        }

        parsed = 0;
        if ((schild->nodetype == LYS_LIST) && has_predicate && lyd_new_path_list_predicate(node, name, id, &parsed)) {
            lyd_free(ret);
            return NULL;
        }
        id += parsed;

        if (!id[0]) {
            /* we are done */
            if (options & LYD_PATH_OPT_NOPARENTRET) {
                /* last created node */
                return node;
            }
            return ret;
        }

        /* prepare for another iteration */
        parent = node;
        sparent = schild;
        prev_mod = lys_node_module(schild);

        /* parse another node */
        if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, &has_predicate, NULL, 0)) < 1) {
            LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
            lyd_free(ret);
            return NULL;
        }
        id += r;

        /* if a key of a list was supposed to be created, it is created as a part of the list instance creation */
        if ((schild->nodetype == LYS_LIST) && !mod_name) {
            slist = (const struct lys_node_list *)schild;
            for (i = 0; i < slist->keys_size; ++i) {
                if (!strncmp(slist->keys[i]->name, name, nam_len) && !slist->keys[i]->name[nam_len]) {
                    /* the path continues? there cannot be anything after a key (leaf) */
                    if (id[0]) {
                        LOGVAL(ctx, LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[0], id);
                        lyd_free(ret);
                        return NULL;
                    }
                    return ret;
                }
            }
        }
    }

    LOGINT(ctx);
    return NULL;
}

API unsigned int
lyd_list_pos(const struct lyd_node *node)
{
    unsigned int pos;
    struct lys_node *schema;

    if (!node || ((node->schema->nodetype != LYS_LIST) && (node->schema->nodetype != LYS_LEAFLIST))) {
        return 0;
    }

    schema = node->schema;
    pos = 0;
    do {
        if (node->schema == schema) {
            ++pos;
        }
        node = node->prev;
    } while (node->next);

    return pos;
}

struct lyd_node *
lyd_new_dummy(struct lyd_node *root, struct lyd_node *parent, const struct lys_node *schema, const char *value, int dflt)
{
    unsigned int index;
    struct ly_set *spath;
    const struct lys_node *siter;
    struct lyd_node *iter, *dummy = NULL;

    assert(schema);
    assert(schema->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_ANYDATA | LYS_NOTIF |
                               LYS_RPC | LYS_ACTION));

    spath = ly_set_new();
    if (!spath) {
        LOGMEM(schema->module->ctx);
        return NULL;
    }

    if (!parent && root) {
        /* find data root */
        for (; root->parent; root = root->parent);   /* vertical move (up) */
        for (; root->prev->next; root = root->prev); /* horizontal move (left) */
    }

    /* build schema path */
    for (siter = schema; siter; siter = lys_parent(siter)) {
        /* stop if we know some of the parents */
        if (parent && parent->schema == siter) {
            break;
        }

        if (siter->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_ANYDATA | LYS_NOTIF |
                               LYS_RPC | LYS_ACTION)) {
            /* we have a node that can appear in data tree */
            ly_set_add(spath, (void*)siter, LY_SET_OPT_USEASLIST);
        } /* else skip the rest node types */
    }

    assert(spath->number > 0);
    index = spath->number;
    if (!parent && !(spath->set.s[index - 1]->nodetype & LYS_LEAFLIST)) {
        /* start by searching for the top-level parent */
        LY_TREE_FOR(root, iter) {
            if (iter->schema == spath->set.s[index - 1]) {
                parent = iter;
                index--;
                break;
            }
        }
    }

    iter = parent;
    while (iter && index && !(spath->set.s[index - 1]->nodetype & LYS_LEAFLIST)) {
        /* search for closer parent on the path */
        LY_TREE_FOR(parent->child, iter) {
            if (iter->schema == spath->set.s[index - 1]) {
                index--;
                parent = iter;
                break;
            }
        }
    }
    while(index) {
        /* create the missing part of the path */
        switch (spath->set.s[index - 1]->nodetype) {
        case LYS_LEAF:
        case LYS_LEAFLIST:
            if (value) {
                iter = _lyd_new_leaf(parent, spath->set.s[index - 1], value, dflt, 0);
            } else {
                iter = lyd_create_leaf(spath->set.s[index - 1], value, dflt);
                if (iter && parent) {
                    if (lyd_insert(parent, iter)) {
                        lyd_free(iter);
                        goto error;
                    }
                }
            }
            break;
        case LYS_CONTAINER:
        case LYS_LIST:
            iter = _lyd_new(parent, spath->set.s[index - 1], dflt);
            break;
        case LYS_ANYXML:
        case LYS_ANYDATA:
            iter = lyd_create_anydata(parent, spath->set.s[index - 1], "", LYD_ANYDATA_CONSTSTRING);
            break;
        default:
            goto error;
        }
        if (!iter) {
            LOGINT(schema->module->ctx);
            goto error;
        }

        /* we say it is valid and it is dummy */
        iter->validity = LYD_VAL_INUSE;

        if (!dummy) {
            dummy = iter;
        }

        /* continue */
        parent = iter;
        index--;
    }

    ly_set_free(spath);

    return dummy;

error:
    ly_set_free(spath);
    lyd_free(dummy);
    return NULL;
}

static struct lys_node *
lys_get_schema_inctx(struct lys_node *schema, struct ly_ctx *ctx)
{
    const struct lys_module *mod, *trg_mod = NULL;
    struct lys_node *parent, *first_sibling = NULL, *iter = NULL;
    struct ly_set *parents;
    unsigned int index;
    uint32_t idx;
    void **ptr;

    if (!ctx || schema->module->ctx == ctx) {
        /* we have the same context */
        return schema;
    }

    /* store the parents chain */
    parents = ly_set_new();
    for (parent = schema; parent; parent = lys_parent(parent)) {
        /* note - augments are skipped so we will work only with the implemented modules
         * (where the augments are applied) */
        if (parent->nodetype != LYS_USES) {
            ly_set_add(parents, parent, LY_SET_OPT_USEASLIST);
        }
    }
    assert(parents->number);
    index = parents->number - 1;

    /* process the parents from the top level */
    /* for the top-level node, we have to locate the module first */
    parent = parents->set.s[index];
    if (parent->nodetype == LYS_EXT) {
        ptr = lys_ext_complex_get_substmt(LY_STMT_NODE, (struct lys_ext_instance_complex *)parent, NULL);
        if (!ptr) {
            ly_set_free(parents);
            return NULL;
        }
        first_sibling = *(struct lys_node **)ptr;
        parent = parents->set.s[--index];
    }
    idx = 0;
    while ((mod = ly_ctx_get_module_iter(ctx, &idx))) {
        trg_mod = lys_node_module(parent);
        /* check module name */
        if (strcmp(mod->name, trg_mod->name)) {
            continue;
        }

        /* check revision */
        if ((!mod->rev_size && !trg_mod->rev_size) ||
                (mod->rev_size && trg_mod->rev_size && !strcmp(mod->rev[0].date, trg_mod->rev[0].date))) {
            /* we have match */
            break;
        }
    }
    /* try data callback */
    if (!mod && trg_mod && ctx->data_clb) {
        LOGDBG(LY_LDGYANG, "Attempting to load '%s' into context using callback ...", trg_mod->name);
        mod = ctx->data_clb(ctx, trg_mod->name, NULL, 0, ctx->data_clb_data);
    }
    if (!mod) {
        ly_set_free(parents);
        return NULL;
    }
    if (!first_sibling) {
        first_sibling = mod->data;
    }

    /* now search in the schema tree for the matching node */
    while (1) {
        lys_get_sibling(first_sibling, trg_mod->name, 0, parent->name, 0, parent->nodetype,
                        (const struct lys_node **)&iter);
        if (!iter) {
            /* not found, iter will be used as NULL result */
            break;
        }

        if (index == 0) {
            /* we are done, iter is the result */
            break;
        } else {
            /* we are going to continue, so update variables for the next loop */
            first_sibling = iter->child;
            parent = parents->set.s[--index];
            iter = NULL;
        }
    }

    ly_set_free(parents);
    return iter;
}

static struct lys_node *
lyd_get_schema_inctx(const struct lyd_node *node, struct ly_ctx *ctx)
{
    assert(node);

    return lys_get_schema_inctx(node->schema, ctx);
}

/* both target and source were validated */
static void
lyd_merge_node_update(struct lyd_node *target, struct lyd_node *source)
{
    struct ly_ctx *ctx;
    struct lyd_node_leaf_list *trg_leaf, *src_leaf;
    struct lyd_node_anydata *trg_any, *src_any;
    int len;

    assert(target->schema->nodetype & (LYS_LEAF | LYS_ANYDATA));
    ctx = target->schema->module->ctx;

    if (ctx == source->schema->module->ctx) {
        /* source and targets are in the same context */
        if (target->schema->nodetype == LYS_LEAF) {
            trg_leaf = (struct lyd_node_leaf_list *)target;
            src_leaf = (struct lyd_node_leaf_list *)source;

            lydict_remove(ctx, trg_leaf->value_str);
            trg_leaf->value_str = src_leaf->value_str;
            src_leaf->value_str = NULL;
            trg_leaf->value_type = src_leaf->value_type;
            src_leaf->value_type = 0;
            if (trg_leaf->value_type == LY_TYPE_LEAFREF) {
                trg_leaf->validity |= LYD_VAL_LEAFREF;
                lyp_parse_value(&((struct lys_node_leaf *)trg_leaf->schema)->type, &trg_leaf->value_str,
                                NULL, trg_leaf, NULL, NULL, 1, src_leaf->dflt, 0);
            } else {
                lyd_free_value(trg_leaf->value, trg_leaf->value_type, trg_leaf->value_flags,
                               &((struct lys_node_leaf *)trg_leaf->schema)->type, NULL, NULL, NULL);
                trg_leaf->value = src_leaf->value;
            }
            src_leaf->value = (lyd_val)0;
            trg_leaf->dflt = src_leaf->dflt;

            check_leaf_list_backlinks(target, 2);
        } else { /* ANYDATA */
            trg_any = (struct lyd_node_anydata *)target;
            src_any = (struct lyd_node_anydata *)source;

            switch(trg_any->value_type) {
            case LYD_ANYDATA_CONSTSTRING:
            case LYD_ANYDATA_SXML:
            case LYD_ANYDATA_JSON:
                lydict_remove(ctx, trg_any->value.str);
                break;
            case LYD_ANYDATA_DATATREE:
                lyd_free_withsiblings(trg_any->value.tree);
                break;
            case LYD_ANYDATA_XML:
                lyxml_free_withsiblings(ctx, trg_any->value.xml);
                break;
            case LYD_ANYDATA_LYB:
                free(trg_any->value.mem);
                break;
            case LYD_ANYDATA_STRING:
            case LYD_ANYDATA_SXMLD:
            case LYD_ANYDATA_JSOND:
            case LYD_ANYDATA_LYBD:
                /* dynamic strings are used only as input parameters */
                assert(0);
                break;
            }

            trg_any->value_type = src_any->value_type;
            trg_any->value = src_any->value;

            src_any->value_type = LYD_ANYDATA_DATATREE;
            src_any->value.tree = NULL;
        }
    } else {
        /* we have different contexts for the target and source */
        if (target->schema->nodetype == LYS_LEAF) {
            trg_leaf = (struct lyd_node_leaf_list *)target;
            src_leaf = (struct lyd_node_leaf_list *)source;

            lydict_remove(ctx, trg_leaf->value_str);
            trg_leaf->value_str = lydict_insert(ctx, src_leaf->value_str, 0);
            lyd_free_value(trg_leaf->value, trg_leaf->value_type, trg_leaf->value_flags,
                           &((struct lys_node_leaf *)trg_leaf->schema)->type, NULL, NULL, NULL);
            trg_leaf->value_type = src_leaf->value_type;
            trg_leaf->dflt = src_leaf->dflt;

            switch (trg_leaf->value_type) {
            case LY_TYPE_BINARY:
            case LY_TYPE_STRING:
                /* value_str pointer is shared in these cases */
                trg_leaf->value.string = trg_leaf->value_str;
                break;
            case LY_TYPE_LEAFREF:
                trg_leaf->validity |= LYD_VAL_LEAFREF;
                lyp_parse_value(&((struct lys_node_leaf *)trg_leaf->schema)->type, &trg_leaf->value_str,
                                NULL, trg_leaf, NULL, NULL, 1, trg_leaf->dflt, 0);
                break;
            case LY_TYPE_INST:
                trg_leaf->value.instance = NULL;
                break;
            case LY_TYPE_UNION:
                /* unresolved union (this must be non-validated tree), duplicate the stored string (duplicated
                 * because of possible change of the value in case of instance-identifier) */
                trg_leaf->value.string = lydict_insert(ctx, src_leaf->value.string, 0);
                break;
            case LY_TYPE_BITS:
            case LY_TYPE_ENUM:
            case LY_TYPE_IDENT:
                /* in case of duplicating bits (no matter if in the same context or not) or enum and identityref into
                 * a different context, searching for the type and duplicating the data is almost as same as resolving
                 * the string value, so due to a simplicity, parse the value for the duplicated leaf */
                lyp_parse_value(&((struct lys_node_leaf *)trg_leaf->schema)->type, &trg_leaf->value_str, NULL,
                                trg_leaf, NULL, NULL, 1, trg_leaf->dflt, 1);
                break;
            default:
                trg_leaf->value = src_leaf->value;
                break;
            }

            check_leaf_list_backlinks(target, 2);
        } else { /* ANYDATA */
            trg_any = (struct lyd_node_anydata *)target;
            src_any = (struct lyd_node_anydata *)source;

            switch(trg_any->value_type) {
            case LYD_ANYDATA_CONSTSTRING:
            case LYD_ANYDATA_SXML:
            case LYD_ANYDATA_JSON:
                lydict_remove(ctx, trg_any->value.str);
                break;
            case LYD_ANYDATA_DATATREE:
                lyd_free_withsiblings(trg_any->value.tree);
                break;
            case LYD_ANYDATA_XML:
                lyxml_free_withsiblings(ctx, trg_any->value.xml);
                break;
            case LYD_ANYDATA_LYB:
                free(trg_any->value.mem);
                break;
            case LYD_ANYDATA_STRING:
            case LYD_ANYDATA_SXMLD:
            case LYD_ANYDATA_JSOND:
            case LYD_ANYDATA_LYBD:
                /* dynamic strings are used only as input parameters */
                assert(0);
                break;
            }

            trg_any->value_type = src_any->value_type;
            if ((void*)src_any->value.tree) {
                /* there is a value to duplicate */
                switch (trg_any->value_type) {
                case LYD_ANYDATA_CONSTSTRING:
                case LYD_ANYDATA_SXML:
                case LYD_ANYDATA_JSON:
                    trg_any->value.str = lydict_insert(ctx, src_any->value.str, 0);
                    break;
                case LYD_ANYDATA_DATATREE:
                    trg_any->value.tree = lyd_dup_to_ctx(src_any->value.tree, 1, ctx);
                    break;
                case LYD_ANYDATA_XML:
                    trg_any->value.xml = lyxml_dup_elem(ctx, src_any->value.xml, NULL, 1);
                    break;
                case LYD_ANYDATA_LYB:
                    len = lyd_lyb_data_length(src_any->value.mem);
                    if (len == -1) {
                        LOGERR(ctx, LY_EINVAL, "Invalid LYB data.");
                        return;
                    }
                    trg_any->value.mem = malloc(len);
                    LY_CHECK_ERR_RETURN(!trg_any->value.mem, LOGMEM(ctx), );
                    memcpy(trg_any->value.mem, src_any->value.mem, len);
                    break;
                case LYD_ANYDATA_STRING:
                case LYD_ANYDATA_SXMLD:
                case LYD_ANYDATA_JSOND:
                case LYD_ANYDATA_LYBD:
                    /* dynamic strings are used only as input parameters */
                    assert(0);
                    break;
                }
            }
        }
    }
}

/* return: 0 (not equal), 1 (equal), -1 (error) */
static int
lyd_merge_node_schema_equal(struct lyd_node *node1, struct lyd_node *node2)
{
    struct lys_node *sch1;

    if (node1->schema->module->ctx == node2->schema->module->ctx) {
        if (node1->schema != node2->schema) {
            return 0;
        }
    } else {
        /* the nodes are in different contexts, get the appropriate schema nodes from the
         * same context */
        sch1 = lyd_get_schema_inctx(node1, node2->schema->module->ctx);
        if (!sch1) {
            LOGERR(node2->schema->module->ctx, LY_EINVAL, "Target context does not contain a required schema node (%s:%s).",
                   lyd_node_module(node1)->name, node1->schema->name);
            return -1;
        } else if (sch1 != node2->schema) {
            /* not matching nodes */
            return 0;
        }
    }

    return 1;
}

/* return: 0 (not equal), 1 (equal), 2 (equal and state leaf-/list marked), -1 (error) */
static int
lyd_merge_node_equal(struct lyd_node *node1, struct lyd_node *node2)
{
    int ret;

    switch (node1->schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LEAF:
    case LYS_ANYXML:
    case LYS_ANYDATA:
    case LYS_RPC:
    case LYS_ACTION:
        return 1;
    case LYS_LEAFLIST:
        if (node1->validity & LYD_VAL_INUSE) {
            /* this instance was already matched, we want to find another so that the number of the istances matches */
            assert(node1->schema->flags & LYS_CONFIG_R);
            return 0;
        }

        ret = lyd_list_equal(node1, node2, 1);
        if ((ret == 1) && (node1->schema->flags & LYS_CONFIG_R)) {
            /* mark it as matched */
            node1->validity |= LYD_VAL_INUSE;
            ret = 2;
        }
        return ret;
    case LYS_LIST:
        if (node1->validity & LYD_VAL_INUSE) {
            /* this instance was already matched, we want to find another so that the number of the istances matches */
            assert(!((struct lys_node_list *)node1->schema)->keys_size);
            return 0;
        }

        ret = lyd_list_equal(node1, node2, 1);
        if ((ret == 1) && !((struct lys_node_list *)node1->schema)->keys_size) {
            /* mark it as matched */
            node1->validity |= LYD_VAL_INUSE;
            ret = 2;
        }
        return ret;
    default:
        break;
    }

    LOGINT(node2->schema->module->ctx);
    return -1;
}

/* spends source */
static int
lyd_merge_parent_children(struct lyd_node *target, struct lyd_node *source, int options)
{
    struct lyd_node *trg_parent, *src, *src_backup, *src_elem, *src_elem_backup, *src_next, *trg_child, *trg_parent_backup;
    int ret, clear_flag = 0;
    struct ly_ctx *ctx = target->schema->module->ctx; /* shortcut */

    LY_TREE_FOR_SAFE(source, src_backup, src) {
        for (src_elem = src_next = src, trg_parent = target;
            src_elem;
            src_elem = src_next) {

            /* it won't get inserted in this case */
            if (src_elem->dflt && (options & LYD_OPT_EXPLICIT)) {
                if (src_elem == src) {
                    /* we are done with this subtree in this case */
                    break;
                }
                trg_child = (struct lyd_node *)1;
                goto src_skip;
            }

            ret = 0;

#ifdef LY_ENABLED_CACHE
            struct lyd_node **trg_child_p;

            /* trees are supposed to be validated so all nodes must have their hash */
            assert(src_elem->hash);

            if (trg_parent->ht) {
                trg_child = NULL;
                if (!lyht_find(trg_parent->ht, &src_elem, src_elem->hash, (void **)&trg_child_p)) {
                    trg_child = *trg_child_p;
                    ret = 1;

                    /* it is a bit more difficult with keyless state lists and leaf-lists */
                    if (((trg_child->schema->nodetype == LYS_LIST) && !((struct lys_node_list *)trg_child->schema)->keys_size)
                            || ((trg_child->schema->nodetype == LYS_LEAFLIST) && (trg_child->schema->flags & LYS_CONFIG_R))) {
                        assert(trg_child->schema->flags & LYS_CONFIG_R);

                        while (trg_child && (trg_child->validity & LYD_VAL_INUSE)) {
                            /* state lists, find one not-already-found */
                            if (lyht_find_next(trg_parent->ht, &trg_child, trg_child->hash, (void **)&trg_child_p)) {
                                trg_child = NULL;
                            } else {
                                trg_child = *trg_child_p;
                            }
                        }
                        if (trg_child) {
                            /* mark it as matched */
                            trg_child->validity |= LYD_VAL_INUSE;
                            ret = 2;
                        } else {
                            /* actually, it was matched already and no other instance found, so now not a match */
                            ret = 0;
                        }
                    }
                }
            } else
#endif
            {
                LY_TREE_FOR(trg_parent->child, trg_child) {
                    /* schema match, data match? */
                    ret = lyd_merge_node_schema_equal(trg_child, src_elem);
                    if (ret == 1) {
                        ret = lyd_merge_node_equal(trg_child, src_elem);
                    }
                    if (ret != 0) {
                        /* even data match */
                        break;
                    }
                }
            }

            if (ret > 0) {
                if (trg_child->schema->nodetype & (LYS_LEAF | LYS_ANYDATA)) {
                    lyd_merge_node_update(trg_child, src_elem);
                } else if (ret == 2) {
                    clear_flag = 1;
                }
            } else if (ret == -1) {
                /* error */
                lyd_free_withsiblings(source);
                return 1;
            }

            /* first prepare for the next iteration */
            src_elem_backup = src_elem;
            trg_parent_backup = trg_parent;
            if (((src_elem->schema->nodetype == LYS_CONTAINER) || ((src_elem->schema->nodetype == LYS_LIST)
                    && ((struct lys_node_list *)src_elem->schema)->keys_size)) && src_elem->child && trg_child) {
                /* go into children */
                src_next = src_elem->child;
                trg_parent = trg_child;
            } else {
src_skip:
                /* no children (or the whole subtree will be inserted), try siblings */
                if (src_elem == src) {
                    /* we are done with this subtree */
                    if (trg_child) {
                        /* it's an empty container, list without keys, or an already-updated leaf/anydata, nothing else to do */
                        break;
                    } else {
                        /* ... but we still need to insert it */
                        src_next = NULL;
                        goto src_insert;
                    }
                } else {
                    src_next = src_elem->next;
                    /* trg_parent does not change */
                }
            }
            while (!src_next) {
                src_elem = src_elem->parent;
                if (src_elem->parent == src->parent) {
                    /* we are done, no next element to process */
                    break;
                }

                /* parent is already processed, go to its sibling */
                src_next = src_elem->next;
                trg_parent = trg_parent->parent;
            }

            if (!trg_child) {
src_insert:
                /* we need to insert the whole subtree */
                if (ctx == src_elem_backup->schema->module->ctx) {
                    /* same context - unlink the subtree and insert it into the target */
                    lyd_unlink(src_elem_backup);
                } else {
                    /* different contexts - before inserting subtree, instead of unlinking, duplicate it into the
                     * target context */
                    src_elem_backup = lyd_dup_to_ctx(src_elem_backup, 1, ctx);
                }

                if (src_elem == source) {
                    /* it will be linked into another data tree and the pointers changed */
                    source = source->next;
                }

                /* insert subtree into the target */
                if (lyd_insert(trg_parent_backup, src_elem_backup)) {
                    LOGINT(ctx);
                    lyd_free_withsiblings(source);
                    return 1;
                }
                if (src_elem == src) {
                    /* we are finished for this src */
                    break;
                }
            }
        }
    }

    lyd_free_withsiblings(source);
    if (clear_flag) {
        return 2;
    }
    return 0;
}

/* spends source */
static int
lyd_merge_siblings(struct lyd_node *target, struct lyd_node *source, int options)
{
    struct lyd_node *trg, *src, *src_backup, *ins;
    int ret, clear_flag = 0;
    struct ly_ctx *ctx = target->schema->module->ctx; /* shortcut */

    while (target->prev->next) {
        target = target->prev;
    }

    LY_TREE_FOR_SAFE(source, src_backup, src) {
        LY_TREE_FOR(target, trg) {
            /* sibling found, merge it */
            ret = lyd_merge_node_schema_equal(trg, src);
            if (ret == 1) {
                ret = lyd_merge_node_equal(trg, src);
            }
            if (ret > 0) {
                if (ret == 2) {
                    clear_flag = 1;
                }

                switch (trg->schema->nodetype) {
                case LYS_LEAF:
                case LYS_ANYXML:
                case LYS_ANYDATA:
                    lyd_merge_node_update(trg, src);
                    break;
                case LYS_LEAFLIST:
                    /* it's already there, nothing to do */
                    break;
                case LYS_LIST:
                case LYS_CONTAINER:
                case LYS_NOTIF:
                case LYS_RPC:
                case LYS_INPUT:
                case LYS_OUTPUT:
                    ret = lyd_merge_parent_children(trg, src->child, options);
                    if (ret == 2) {
                        clear_flag = 1;
                    } else if (ret) {
                        lyd_free_withsiblings(source);
                        return 1;
                    }
                    break;
                default:
                    LOGINT(ctx);
                    lyd_free_withsiblings(source);
                    return 1;
                }
                break;
            } else if (ret == -1) {
                lyd_free_withsiblings(source);
                return 1;
            } /* else not equal, nothing to do */
        }

        /* sibling not found, insert it */
        if (!trg) {
            if (ctx != src->schema->module->ctx) {
                ins = lyd_dup_to_ctx(src, 1, ctx);
            } else {
                lyd_unlink(src);
                if (src == source) {
                    /* just so source is not freed, we inserted it and need it further */
                    source = src_backup;
                }
                ins = src;
            }
            lyd_insert_after(target->prev, ins);
        }
    }

    lyd_free_withsiblings(source);
    if (clear_flag) {
        return 2;
    }
    return 0;
}

API int
lyd_merge_to_ctx(struct lyd_node **trg, const struct lyd_node *src, int options, struct ly_ctx *ctx)
{
    struct lyd_node *node = NULL, *node2, *target, *trg_merge_start, *src_merge_start = NULL;
    const struct lyd_node *iter;
    struct lys_node *src_snode, *sch = NULL;
    int i, src_depth, depth, first_iter, ret, dflt = 1;
    const struct lys_node *parent = NULL;

    if (!trg || !(*trg) || !src) {
        LOGARG;
        return -1;
    }
    target = *trg;

    parent = lys_parent(target->schema);

    /* go up all uses */
    while (parent && (parent->nodetype == LYS_USES)) {
        parent = lys_parent(parent);
    }

    if (parent && !lyp_get_yang_data_template_name(target)) {
        LOGERR(parent->module->ctx, LY_EINVAL, "Target not a top-level data tree.");
        return -1;
    }

    /* get know if we are converting data into a different context */
    if (ctx && target->schema->module->ctx != ctx) {
        /* target's data tree context differs from the target context, move the target
         * data tree into the target context */

        /* get the first target's top-level and store it as the result */
        for (; target->prev->next; target = target->prev);
        *trg = target;

        for (node = NULL, trg_merge_start = target; target; target = target->next) {
            node2 = lyd_dup_to_ctx(target, 1, ctx);
            if (!node2) {
                goto error;
            }
            if (node) {
                if (lyd_insert_after(node->prev, node2)) {
                    goto error;
                }
            } else {
                node = node2;
            }
        }
        target = node;
        node = NULL;
    } else if (src->schema->module->ctx != target->schema->module->ctx) {
        /* the source data will be converted into the target's context during the merge */
        ctx = target->schema->module->ctx;
    } else if (ctx == src->schema->module->ctx) {
        /* no conversion is needed */
        ctx = NULL;
    }

    /* find source top-level schema node */
    for (src_snode = src->schema, src_depth = 0;
         (src_snode = lys_parent(src_snode)) && src_snode->nodetype != LYS_EXT;
         ++src_depth);

    /* find first shared missing schema parent of the subtrees */
    trg_merge_start = target;
    depth = 0;
    first_iter = 1;
    if (src_depth) {
        /* we are going to create missing parents in the following loop,
         * but we will need to know a dflt flag for them. In case the newly
         * created parent is going to have at least one non-default child,
         * it will be also non-default, otherwise it will be the default node */
        if (options & LYD_OPT_NOSIBLINGS) {
            dflt = src->dflt;
        } else {
            LY_TREE_FOR(src, iter) {
                if (!iter->dflt) {
                    /* non default sibling -> parent is going to be
                     * created also as non-default */
                    dflt = 0;
                    break;
                }
            }
        }
    }
    while (1) {
        /* going from down (source root) to up (top-level or the common node with target */
        do {
            for (src_snode = src->schema, i = 0; i < src_depth - depth; src_snode = lys_parent(src_snode), ++i);
            ++depth;
        } while (src_snode != src->schema && (src_snode->nodetype & (LYS_CHOICE | LYS_CASE | LYS_USES)));

        if (src_snode == src->schema) {
            break;
        }

        if (src_snode->nodetype != LYS_CONTAINER) {
            /* we would have to create a list (the only data node with children except container), impossible */
            LOGERR(ctx, LY_EINVAL, "Cannot create %s \"%s\" for the merge.", strnodetype(src_snode->nodetype), src_snode->name);
            goto error;
        }

        /* have we created any missing containers already? if we did,
         * it is totally useless to search for match, there won't ever be */
        if (!src_merge_start) {
            if (first_iter) {
                node = trg_merge_start;
                first_iter = 0;
            } else {
                node = trg_merge_start->child;
            }

            /* find it in target data nodes */
            LY_TREE_FOR(node, node) {
                if (ctx) {
                    /* we have the schema nodes in the different context */
                    sch = lys_get_schema_inctx(src_snode, ctx);
                    if (!sch) {
                        LOGERR(ctx, LY_EINVAL, "Target context does not contain schema node for the data node being "
                               "merged (%s:%s).", lys_node_module(src_snode)->name, src_snode->name);
                        goto error;
                    }
                } else {
                    /* the context is same and comparison of the schema nodes will works fine */
                    sch = src_snode;
                }

                if (node->schema == sch) {
                    trg_merge_start = node;
                    break;
                }
            }

            if (!(options & LYD_OPT_DESTRUCT)) {
                /* the source tree will be duplicated, so to save some work in case
                 * of different target context, create also the parents nodes in the
                 * correct context */
                src_snode = sch;
            }
        } else if (ctx && !(options & LYD_OPT_DESTRUCT)) {
            /* get the schema node in the correct (target) context, same as above,
             * this is done to save some work and have the source in the same context
             * when the provided source tree is below duplicated in the target context
             * and connected into the parents created here */
            src_snode = lys_get_schema_inctx(src_snode, ctx);
            if (!src_snode) {
                LOGERR(ctx, LY_EINVAL, "Target context does not contain schema node for the data node being "
                       "merged (%s:%s).", lys_node_module(src_snode)->name, src_snode->name);
                goto error;
            }
        }

        if (!node) {
            /* it is not there, create it */
            node2 = _lyd_new(NULL, src_snode, dflt);
            if (!src_merge_start) {
                src_merge_start = node2;
            } else {
                if (lyd_insert(node2, src_merge_start)) {
                    goto error;
                }
                src_merge_start = node2;
            }
        }
    }

    /* process source according to options */
    if (options & LYD_OPT_DESTRUCT) {
        LY_TREE_FOR(src, iter) {
            check_leaf_list_backlinks((struct lyd_node *)iter, 2);
            if (options & LYD_OPT_NOSIBLINGS) {
                break;
            }
        }

        node = (struct lyd_node *)src;
        if ((node->prev != node) && (options & LYD_OPT_NOSIBLINGS)) {
            node2 = node->prev;
            lyd_unlink(node);
            lyd_free_withsiblings(node2);
        }
    } else {
        node = NULL;
        for (; src; src = src->next) {
            /* because we already have to duplicate it, do it in the correct context */
            node2 = lyd_dup_to_ctx(src, 1, ctx);
            if (!node2) {
                lyd_free_withsiblings(node);
                goto error;
            }
            if (node) {
                if (lyd_insert_after(node->prev, node2)) {
                    lyd_free_withsiblings(node);
                    goto error;
                }
            } else {
                node = node2;
            }

            if (options & LYD_OPT_NOSIBLINGS) {
                break;
            }
        }
    }

    if (src_merge_start) {
        /* insert data into the created parents */
        /* first, get the lowest created parent, we don't have to check the nodetype since we are
         * creating only a simple chain of containers */
        for (node2 = src_merge_start; node2->child; node2 = node2->child);
        node2->child = node;
        LY_TREE_FOR(node, node) {
            node->parent = node2;
        }
    } else {
        src_merge_start = node;
    }

    if (!first_iter) {
        /* !! src_merge start is a child(ren) of trg_merge_start */
        ret = lyd_merge_parent_children(trg_merge_start, src_merge_start, options);
    } else {
        /* !! src_merge start is a (top-level) sibling(s) of trg_merge_start */
        ret = lyd_merge_siblings(trg_merge_start, src_merge_start, options);
    }
    /* it was freed whatever the return value */
    src_merge_start = NULL;
    if (ret == 2) {
        /* clear remporary LYD_VAL_INUSE validation flags */
        LY_TREE_DFS_BEGIN(target, node2, node) {
            node->validity &= ~LYD_VAL_INUSE;
            LY_TREE_DFS_END(target, node2, node);
        }
        ret = 0;
    } else if (ret) {
        goto error;
    }

    if (target->schema->nodetype == LYS_RPC) {
        lyd_schema_sort(target, 1);
    }

    /* update the pointer to the target tree if needed */
    if (*trg != target) {
        lyd_free_withsiblings(*trg);
        (*trg) = target;
    }
    return ret;

error:
    if (*trg != target) {
        /* target is duplication of the original target in different context,
         * free it due to the error */
        lyd_free_withsiblings(target);
    }
    lyd_free_withsiblings(src_merge_start);
    return -1;
}

API int
lyd_merge(struct lyd_node *target, const struct lyd_node *source, int options)
{
    if (!target || !source) {
        LOGARG;
        return -1;
    }

    return lyd_merge_to_ctx(&target, source, options, target->schema->module->ctx);
}

API void
lyd_free_diff(struct lyd_difflist *diff)
{
    if (diff) {
        free(diff->type);
        free(diff->first);
        free(diff->second);
        free(diff);
    }
}

static int
lyd_difflist_add(struct lyd_difflist *diff, unsigned int *size, unsigned int index,
                 LYD_DIFFTYPE type, struct lyd_node *first, struct lyd_node *second)
{
    void *new;
    struct ly_ctx *ctx = (first ? first->schema->module->ctx : second->schema->module->ctx);

    assert(diff);
    assert(size && *size);

    if (index + 1 == *size) {
        /* it's time to enlarge */
        *size = *size + 16;
        new = realloc(diff->type, *size * sizeof *diff->type);
        LY_CHECK_ERR_RETURN(!new, LOGMEM(ctx), EXIT_FAILURE);
        diff->type = new;

        new = realloc(diff->first, *size * sizeof *diff->first);
        LY_CHECK_ERR_RETURN(!new, LOGMEM(ctx), EXIT_FAILURE);
        diff->first = new;

        new = realloc(diff->second, *size * sizeof *diff->second);
        LY_CHECK_ERR_RETURN(!new, LOGMEM(ctx), EXIT_FAILURE);
        diff->second = new;
    }

    /* insert the item */
    diff->type[index] = type;
    diff->first[index] = first;
    diff->second[index] = second;

    /* terminate the arrays */
    index++;
    diff->type[index] = LYD_DIFF_END;
    diff->first[index] = NULL;
    diff->second[index] = NULL;

    return EXIT_SUCCESS;
}

struct diff_ordered_dist {
    struct diff_ordered_dist *next;
    int dist;
};
struct diff_ordered_item {
    struct lyd_node *first;
    struct lyd_node *second;
    struct diff_ordered_dist *dist;
};
struct diff_ordered {
    struct lys_node *schema;
    struct lyd_node *parent;
    unsigned int count;
    struct diff_ordered_item *items; /* array */
    struct diff_ordered_dist *dist;  /* linked list (1-way, ring) */
    struct diff_ordered_dist *dist_last;  /* aux pointer for faster insertion sort */
};

static int
diff_ordset_insert(struct lyd_node *node, struct ly_set *ordset)
{
    unsigned int i;
    struct diff_ordered *new_ordered, *iter;

    for (i = 0; i < ordset->number; i++) {
        iter = (struct diff_ordered *)ordset->set.g[i];
        if (iter->schema == node->schema && iter->parent == node->parent) {
            break;
        }
    }
    if (i == ordset->number) {
        /* not seen user-ordered list */
        new_ordered = calloc(1, sizeof *new_ordered);
        LY_CHECK_ERR_RETURN(!new_ordered, LOGMEM(node->schema->module->ctx), EXIT_FAILURE);
        new_ordered->schema = node->schema;
        new_ordered->parent = node->parent;

        ly_set_add(ordset, new_ordered, LY_SET_OPT_USEASLIST);
    }
    ((struct diff_ordered *)ordset->set.g[i])->count++;

    return EXIT_SUCCESS;
}

static void
diff_ordset_free(struct ly_set *set)
{
    unsigned int i, j;
    struct diff_ordered *ord;

    if (!set) {
        return;
    }

    for (i = 0; i < set->number; i++) {
        ord = (struct diff_ordered *)set->set.g[i];
        for (j = 0; j < ord->count; j++) {
            free(ord->items[j].dist);
        }
        free(ord->items);
        free(ord);
    }

    ly_set_free(set);
}

/*
 * -1 - error
 *  0 - ok
 *  1 - first and second not the same
 */
static int
lyd_diff_compare(struct lyd_node *first, struct lyd_node *second, int options)
{
    int rc;

    if (first->dflt && !(options & LYD_DIFFOPT_WITHDEFAULTS)) {
        /* the second one cannot be default (see lyd_diff()),
         * so the nodes differs (first one is default node) */
        return 1;
    }

    if (first->schema->nodetype & (LYS_LEAFLIST | LYS_LIST)) {
        if (first->validity & LYD_VAL_INUSE) {
            /* this node was already matched, it cannot be matched twice (except for state leaf-/lists,
             * which we want to keep the count on this way) */
            return 1;
        }

        rc = lyd_list_equal(first, second, (options & LYD_DIFFOPT_WITHDEFAULTS ? 1 : 0));
        if (rc == -1) {
            return -1;
        } else if (!rc) {
            /* list instances differs */
            return 1;
        }
        /* matches */
    }

    return 0;
}

/*
 * -1 - error
 *  0 - ok
 */
static int
lyd_diff_match(struct lyd_node *first, struct lyd_node *second, struct lyd_difflist *diff, unsigned int *size,
               unsigned int *i, struct ly_set *matchset, struct ly_set *ordset, int options)
{
    switch (first->schema->nodetype) {
    case LYS_LEAFLIST:
    case LYS_LIST:
        /* additional work for future move matching in case of user ordered lists */
        if (first->schema->flags & LYS_USERORDERED) {
            diff_ordset_insert(first, ordset);
        }

        /* falls through */
    case LYS_CONTAINER:
        assert(!(second->validity & LYD_VAL_INUSE));
        second->validity |= LYD_VAL_INUSE;
        /* remember the matching node in first for keeping correct pointer in first
         * for comparing when passing through the second tree in lyd_diff().
         * Duplicities are not allowed actually, but they cannot happen since single
         * node can match only one node in the other tree */
        ly_set_add(matchset, first, LY_SET_OPT_USEASLIST);
        break;
    case LYS_LEAF:
        /* check for leaf's modification */
        if (!lyd_leaf_val_equal(first, second, 0) || ((options & LYD_DIFFOPT_WITHDEFAULTS) && (first->dflt != second->dflt))) {
            if (lyd_difflist_add(diff, size, (*i)++, LYD_DIFF_CHANGED, first, second)) {
               return -1;
            }
        }
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        /* check for anydata/anyxml's modification */
        if (!lyd_anydata_equal(first, second) && lyd_difflist_add(diff, size, (*i)++, LYD_DIFF_CHANGED, first, second)) {
            return -1;
        }
        break;
    default:
        LOGINT(first->schema->module->ctx);
        return -1;
    }

    /* mark both that they have matching instance in the other tree */
    assert(!(first->validity & LYD_VAL_INUSE));
    first->validity |= LYD_VAL_INUSE;

    return 0;
}

/* @brief compare if the nodes are equivalent including checking the list's keys
 * Go through the nodes and their parents and in the case of list, compare its keys.
 *
 * @return 0 different, 1 equivalent
 */
static int
lyd_diff_equivnode(struct lyd_node *first, struct lyd_node *second)
{
    struct lyd_node *iter1, *iter2;

    for (iter1 = first, iter2 = second; iter1 && iter2; iter1 = iter1->parent, iter2 = iter2->parent) {
        if (iter1->schema->module->ctx == iter2->schema->module->ctx) {
            if (iter1->schema != iter2->schema) {
                return 0;
            }
        } else {
            if (!ly_strequal(iter1->schema->name, iter2->schema->name, 0)) {
                /* comparing the names is fine, even if they are, in fact, 2 different nodes
                 * with equal names, some of their parents will differ */
                return 0;
            }
        }
        if (iter1->schema->nodetype == LYS_LIST) {
            /* compare keys */
            if (lyd_list_equal(iter1, iter2, 0) != 1) {
                return 0;
            }
        }
    }

    if (iter1 != iter2) {
        /* we are supposed to be in root (NULL) in both trees */
        return 0;
    }

    return 1;
}

static int
lyd_diff_move_preprocess(struct diff_ordered *ordered, struct lyd_node *first, struct lyd_node *second)
{
    struct ly_ctx *ctx = first->schema->module->ctx;
    struct lyd_node *iter;
    unsigned int pos = 0;
    int abs_dist;
    struct diff_ordered_dist *dist_aux;
    struct diff_ordered_dist *dist_iter, *dist_last;
    char *str = NULL;

    /* ordered->count was zeroed and now it is incremented with each added
     * item's information, so it is actually position of the second node
     */

    /* get the position of the first node */
    for (iter = first->prev; iter->next; iter = iter->prev) {
        if (!(iter->validity & LYD_VAL_INUSE)) {
            /* skip deleted nodes */
            continue;
        }
        if (iter->schema == first->schema) {
            pos++;
        }
    }
    if (pos != ordered->count) {
        LOGDBG(LY_LDGDIFF, "detected moved element \"%s\" from %d to %d (distance %d)",
               str = lyd_path(first), pos, ordered->count, ordered->count - pos);
        free(str);
    }

    /* store information, count distance */
    ordered->items[pos].dist = dist_aux = calloc(1, sizeof *dist_aux);
    LY_CHECK_ERR_RETURN(!dist_aux, LOGMEM(ctx), EXIT_FAILURE);
    ordered->items[pos].dist->dist = ordered->count - pos;
    abs_dist = abs(ordered->items[pos].dist->dist);
    ordered->items[pos].first = first;
    ordered->items[pos].second = second;
    ordered->count++;

    /* insert sort of distances, higher first */
    for (dist_iter = ordered->dist, dist_last = NULL;
            dist_iter;
            dist_last = dist_iter, dist_iter = dist_iter->next) {
        if (abs_dist >= abs(dist_iter->dist)) {
            /* found correct place */
            dist_aux->next = dist_iter;
            if (dist_last) {
                dist_last->next = dist_aux;
            }
            break;
        } else if (dist_iter->next == ordered->dist) {
            /* last item */
            dist_aux->next = ordered->dist; /* ring list */
            ordered->dist_last = dist_aux;
            break;
        }
    }
    if (dist_aux->next == ordered->dist) {
        if (ordered->dist_last == dist_aux) {
            /* last item */
            if (!ordered->dist) {
                /* the only item */
                dist_aux->next = dist_aux;
                ordered->dist = ordered->dist_last = dist_aux;
            }
        } else {
            /* first item */
            ordered->dist = dist_aux;
            if (dist_aux->next) {
                /* more than one item, update the last one's next */
                ordered->dist_last->next = dist_aux;
            } else {
                /* the only item */
                ordered->dist_last = dist_aux;
                dist_aux->next = dist_aux; /* ring list */
            }
        }
    }

    return 0;
}

static struct lyd_difflist *
lyd_diff_init_difflist(struct ly_ctx *ctx, unsigned int *size)
{
    struct lyd_difflist *result;

    result = malloc(sizeof *result);
    LY_CHECK_ERR_RETURN(!result, LOGMEM(ctx); *size = 0, NULL);

    *size = 1;
    result->type = calloc(*size, sizeof *result->type);
    result->first = calloc(*size, sizeof *result->first);
    result->second = calloc(*size, sizeof *result->second);
    if (!result->type || !result->first || !result->second) {
        LOGMEM(ctx);
        free(result->second);
        free(result->first);
        free(result->type);
        free(result);
        *size = 0;
        return NULL;
    }

    return result;
}

API struct lyd_difflist *
lyd_diff(struct lyd_node *first, struct lyd_node *second, int options)
{
    struct ly_ctx *ctx;
    int rc;
    struct lyd_node *elem1, *elem2, *iter, *aux, *parent = NULL, *next1, *next2;
    struct lyd_difflist *result, *result2 = NULL;
    void *new;
    unsigned int size, size2, index = 0, index2 = 0, i, j, k;
    struct matchlist_s {
        struct matchlist_s *prev;
        struct ly_set *match;
        unsigned int i;
    } *matchlist = NULL, *mlaux;
    struct ly_set *ordset = NULL;
    struct diff_ordered *ordered;
    struct diff_ordered_dist *dist_aux, *dist_iter;
    struct diff_ordered_item item_aux;

    if (!first) {
        /* all nodes in second were created,
         * but the second must be top level */
        if (second && second->parent) {
            LOGERR(second->schema->module->ctx, LY_EINVAL, "%s: \"first\" parameter is NULL and \"second\" is not top level.", __func__);
            return NULL;
        }
        result = lyd_diff_init_difflist(NULL, &size);
        LY_TREE_FOR(second, iter) {
            if (!iter->dflt || (options & LYD_DIFFOPT_WITHDEFAULTS)) { /* skip the implicit nodes */
                if (lyd_difflist_add(result, &size, index++, LYD_DIFF_CREATED, NULL, iter)) {
                    goto error;
                }
            }
            if (options & LYD_DIFFOPT_NOSIBLINGS) {
                break;
            }
        }
        return result;
    } else if (!second) {
        /* all nodes from first were deleted */
        result = lyd_diff_init_difflist(first->schema->module->ctx, &size);
        LY_TREE_FOR(first, iter) {
            if (!iter->dflt || (options & LYD_DIFFOPT_WITHDEFAULTS)) { /* skip the implicit nodes */
                if (lyd_difflist_add(result, &size, index++, LYD_DIFF_DELETED, iter, NULL)) {
                    goto error;
                }
            }
            if (options & LYD_DIFFOPT_NOSIBLINGS) {
                break;
            }
        }
        return result;
    }

    ctx = first->schema->module->ctx;

    if (options & LYD_DIFFOPT_NOSIBLINGS) {
        /* both trees must start at the same (schema) node */
        if (first->schema != second->schema) {
            LOGERR(ctx, LY_EINVAL, "%s: incompatible trees to compare with LYD_OPT_NOSIBLINGS option.", __func__);
            return NULL;
        }
        /* use first's and second's child to make comparison the same as without LYD_OPT_NOSIBLINGS */
        first = first->child;
        second = second->child;
    } else {
        /* go to the first sibling in both trees */
        if (first->parent) {
            first = first->parent->child;
        } else {
            while (first->prev->next) {
                first = first->prev;
            }
        }

        if (second->parent) {
            second = second->parent->child;
        } else {
            for (; second->prev->next; second = second->prev);
        }

        /* check that both has the same (schema) parent or that they are top-level nodes */
        if ((first->parent && second->parent && first->parent->schema != second->parent->schema) ||
                (!first->parent && first->parent != second->parent)) {
            LOGERR(ctx, LY_EINVAL, "%s: incompatible trees with different parents.", __func__);
            return NULL;
        }
    }
    if (first == second) {
        LOGERR(ctx, LY_EINVAL, "%s: comparing the same tree does not make sense.", __func__);
        return NULL;
    }

    /* initiate resulting structure */
    result = lyd_diff_init_difflist(ctx, &size);
    LY_CHECK_ERR_GOTO(!result, , error);

    /* the records about created and moved items are created in
     * bad order, so the records about created nodes (and their
     * possible moving) is stored separately and added to the
     * main result at the end.
     */
    result2 = lyd_diff_init_difflist(ctx, &size2);
    LY_CHECK_ERR_GOTO(!result2, , error);

    matchlist = malloc(sizeof *matchlist);
    LY_CHECK_ERR_GOTO(!matchlist, LOGMEM(ctx), error);

    matchlist->i = 0;
    matchlist->match = ly_set_new();
    matchlist->prev = NULL;

    ordset = ly_set_new();
    LY_CHECK_ERR_GOTO(!ordset, , error);

    /*
     * compare trees
     */
    /* 1) newly created nodes + changed leafs/anyxmls */
    next1 = first;
    for (elem2 = next2 = second; elem2; elem2 = next2) {
        /* keep right pointer for searching in the first tree */
        elem1 = next1;

        if (elem2->dflt && !(options & LYD_DIFFOPT_WITHDEFAULTS)) {
            /* skip default elements, they could not be created or changed, just deleted */
            goto cmp_continue;
        }

#ifdef LY_ENABLED_CACHE
        struct lyd_node **iter_p;

        if (elem1 && elem1->parent && elem1->parent->ht) {
            iter = NULL;
            if (!lyht_find(elem1->parent->ht, &elem2, elem2->hash, (void **)&iter_p)) {
                iter = *iter_p;
                /* we found a match */
                if (iter->dflt && !(options & LYD_DIFFOPT_WITHDEFAULTS)) {
                    /* the second one cannot be default (see lyd_diff()),
                     * so the nodes differs (first one is default node) */
                    iter = NULL;
                }
                while (iter && (iter->validity & LYD_VAL_INUSE)) {
                    /* state lists, find one not-already-found */
                    assert((iter->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) && (iter->schema->flags & LYS_CONFIG_R));
                    if (lyht_find_next(elem1->parent->ht, &iter, iter->hash, (void **)&iter_p)) {
                        iter = NULL;
                    } else {
                        iter = *iter_p;
                    }
                }
            }
        } else
#endif
        {
            /* search for elem2 instance in the first */
            LY_TREE_FOR(elem1, iter) {
                if (iter->schema != elem2->schema) {
                    continue;
                }

                /* elem2 instance found */
                rc = lyd_diff_compare(iter, elem2, options);
                if (rc == -1) {
                    goto error;
                } else if (rc == 0) {
                    /* match */
                    break;
                } /* else, continue */
            }
        }
        /* we have a match */
        if (iter && lyd_diff_match(iter, elem2, result, &size, &index, matchlist->match, ordset, options)) {
            goto error;
        }

        if (!iter) {
            /* elem2 not found in the first tree */
            if (lyd_difflist_add(result2, &size2, index2++, LYD_DIFF_CREATED, elem1 ? elem1->parent : parent, elem2)) {
                goto error;
            }

            if (elem1 && (elem2->schema->flags & LYS_USERORDERED)) {
                /* store the correct place where the node is supposed to be moved after creation */
                /* if elem1 does not exist, all nodes were created and they will be created in
                 * correct order, so it is not needed to detect moves */
                for (aux = elem2->prev; aux->next; aux = aux->prev) {
                    if (aux->schema == elem2->schema) {
                        /* predecessor found */
                        break;
                    }
                }
                if (!aux->next) {
                    /* predecessor not found */
                    aux = NULL;
                }
                if (lyd_difflist_add(result2, &size2, index2++, LYD_DIFF_MOVEDAFTER2, aux, elem2)) {
                    goto error;
                }
            }
        }

cmp_continue:
        /* select element for the next run                                    1     2
         * - first, process all siblings of a single parent                  / \   / \
         * - then, go to children (deep)                                    3   4 7   8
         * - return to the parent's next sibling children                  / \
         *                                                                5   6
         */
        /* siblings first */
        next1 = elem1;
        next2 = elem2->next;

        if (!next2) {
            /* children */

            /* first pass of the siblings done, some additional work for future
             * detection of move may be needed */
            for (i = ordset->number; i > 0; i--) {
                ordered = (struct diff_ordered *)ordset->set.g[i - 1];
                if (ordered->items) {
                    /* already preprocessed ordered structure */
                    break;
                }
                ordered->items = calloc(ordered->count, sizeof *ordered->items);
                LY_CHECK_ERR_GOTO(!ordered->items, LOGMEM(ctx), error);
                ordered->dist = NULL;
                /* zero the count to be used as a node position in lyd_diff_move_preprocess() */
                ordered->count = 0;
            }

            /* first, get the first sibling */
            if (elem2->parent == second->parent) {
                elem2 = second;
            } else {
                elem2 = elem2->parent->child;
            }

            /* and then find the first child */
            LY_TREE_FOR(elem2, iter) {
                if (!(iter->validity & LYD_VAL_INUSE)) {
                    /* the iter is not present in both trees */
                    continue;
                } else if (matchlist->i == matchlist->match->number) {
                    if (iter == elem2) {
                        /* we already went through all the matching nodes and now we are just supposed to stop
                         * the loop with no iter */
                        iter = NULL;
                        break;
                    } else {
                        /* we have started with some not processed data in matchlist, but now we have
                         * the INUSE iter and no nodes in matchlist to find its equivalent,
                         * so something went wrong somewhere */
                        LOGINT(ctx);
                        goto error;
                    }
                }

                iter->validity &= ~LYD_VAL_INUSE;
                if ((iter->schema->nodetype & (LYS_LEAFLIST | LYS_LIST)) && (iter->schema->flags & LYS_USERORDERED)) {
                    for (j = ordset->number; j > 0; j--) {
                        ordered = (struct diff_ordered *)ordset->set.g[j - 1];
                        if (ordered->schema != iter->schema || !lyd_diff_equivnode(ordered->parent, iter->parent)) {
                            continue;
                        }

                        /* store necessary information for move detection */
                        lyd_diff_move_preprocess(ordered, matchlist->match->set.d[matchlist->i], iter);
                        break;
                    }
                }

                if (((iter->schema->nodetype == LYS_CONTAINER) || ((iter->schema->nodetype == LYS_LIST)
                        && ((struct lys_node_list *)iter->schema)->keys_size)) && iter->child) {
                    while (matchlist->i < matchlist->match->number && matchlist->match->set.d[matchlist->i]->schema != iter->schema) {
                        matchlist->i++;
                    }
                    if (matchlist->i == matchlist->match->number) {
                        /* we have the INUSE iter, so we have to find its equivalent in match list */
                        LOGINT(ctx);
                        goto error;
                    }
                    next1 = matchlist->match->set.d[matchlist->i]->child;
                    if (!next1) {
                        parent = matchlist->match->set.d[matchlist->i];
                    }
                    matchlist->i++;
                    next2 = iter->child;
                    break;
                }
                matchlist->i++;
            }

            if (!iter) {
                /* no child/data on next level */
                if (elem2 == second) {
                    /* done */
                    break;
                }
            } else {
                /* create new matchlist item */
                mlaux = malloc(sizeof *mlaux);
                LY_CHECK_ERR_GOTO(!mlaux, LOGMEM(ctx), error);
                mlaux->i = 0;
                mlaux->match = ly_set_new();
                mlaux->prev = matchlist;
                matchlist = mlaux;
            }
        }

        while (!next2) {
            /* parent */

            /* clean the last match set */
            ly_set_clean(matchlist->match);
            matchlist->i = 0;

            /* try to go to a cousin - child of the next parent's sibling */
            mlaux = matchlist->prev;
            LY_TREE_FOR(elem2->parent->next, iter) {
                if (!(iter->validity & LYD_VAL_INUSE)) {
                    continue;
                } else if (mlaux->i == mlaux->match->number) {
                    if (iter == elem2->parent->next) {
                        /* we already went through all the matching nodes and now we are just supposed to stop
                         * the loop with no iter */
                        iter = NULL;
                        break;
                    } else {
                        /* we have started with some not processed data in matchlist, but now we have
                         * the INUSE iter and no nodes in matchlist to find its equivalent,
                         * so something went wrong somewhere */
                        LOGINT(ctx);
                        goto error;
                    }
                }

                iter->validity &= ~LYD_VAL_INUSE;
                if ((iter->schema->nodetype & (LYS_LEAFLIST | LYS_LIST)) && (iter->schema->flags & LYS_USERORDERED)) {
                    for (j = ordset->number ; j > 0; j--) {
                        ordered = (struct diff_ordered *)ordset->set.g[j - 1];
                        if (ordered->schema != iter->schema || !lyd_diff_equivnode(ordered->parent, iter->parent)) {
                            continue;
                        }

                        /* store necessary information for move detection */
                        lyd_diff_move_preprocess(ordered, mlaux->match->set.d[mlaux->i], iter);
                        break;
                    }
                }

                if (((iter->schema->nodetype == LYS_CONTAINER) || ((iter->schema->nodetype == LYS_LIST)
                        && ((struct lys_node_list *)iter->schema)->keys_size)) && iter->child) {
                    while (mlaux->i < mlaux->match->number && mlaux->match->set.d[mlaux->i]->schema != iter->schema) {
                        mlaux->i++;
                    }
                    if (mlaux->i == mlaux->match->number) {
                        /* we have the INUSE iter, so we have to find its equivalent in match list */
                        LOGINT(ctx);
                        goto error;
                    }
                    next1 = mlaux->match->set.d[mlaux->i]->child;
                    if (!next1) {
                        parent = mlaux->match->set.d[mlaux->i];
                    }
                    mlaux->i++;
                    next2 = iter->child;
                    break;
                }
                mlaux->i++;
            }

            /* if no cousin exists, continue next loop on higher level */
            if (!iter) {
                elem2 = elem2->parent;

                /* remove matchlist item */
                ly_set_free(matchlist->match);
                mlaux = matchlist;
                matchlist = matchlist->prev;
                free(mlaux);

                if (!matchlist->prev) { /* elem2->parent == second->parent */
                    /* done */
                    break;
                }
            }
        }
    }

    ly_set_free(matchlist->match);
    free(matchlist);
    matchlist = NULL;

    /* 2) deleted nodes */
    LY_TREE_DFS_BEGIN(first, next1, elem1) {
        /* search for elem1s deleted in the second */
        if (elem1->validity & LYD_VAL_INUSE) {
            /* erase temporary LYD_VAL_INUSE flag and continue into children */
            elem1->validity &= ~LYD_VAL_INUSE;
        } else if (!elem1->dflt || (options & LYD_DIFFOPT_WITHDEFAULTS)) {
            /* elem1 has no matching node in second, add it into result */
            if (lyd_difflist_add(result, &size, index++, LYD_DIFF_DELETED, elem1, NULL)) {
                goto error;
            }

            /* skip subtree processing of data missing in the second tree */
            goto dfs_nextsibling;
        }

        /* modified LY_TREE_DFS_END() */
        /* select element for the next run - children first */
        if ((elem1->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) || ((elem1->schema->nodetype == LYS_LIST)
                && !((struct lys_node_list *)elem1->schema)->keys_size)) {
            next1 = NULL;
        } else {
            next1 = elem1->child;
        }
        if (!next1) {
dfs_nextsibling:
            /* try siblings */
            next1 = elem1->next;
        }
        while (!next1) {
            /* parent is already processed, go to its sibling */

            elem1 = elem1->parent;
            if (elem1 == first->parent) {
                /* we are done, no next element to process */
                break;
            }

            next1 = elem1->next;
        }
    }

    /* 3) moved nodes (when user-ordered) */
    for (i = 0; i < ordset->number; i++) {
        ordered = (struct diff_ordered *)ordset->set.g[i];
        if (!ordered->dist->dist) {
            /* the dist list is sorted here, but the biggest dist is 0,
             * so nothing changed in order of these items between first
             * and second. We can continue with another user-ordered list.
             */
            continue;
        }

        /* get needed movements
         * - from the biggest distances try to apply node movements
         * on first tree node until they will be ordered as in the
         * second tree - i.e. until there will be no position difference
         */

        for (dist_iter = ordered->dist; ; dist_iter = dist_iter->next) {
            /* dist list is sorted at the beginning, since applying a move causes
             * just a small change in other distances, we assume that the biggest
             * dist is the next one (note that dist list is implemented as ring
             * list). This way we avoid sorting distances after each move. The loop
             * stops when all distances are zero.
             */
            dist_aux = dist_iter;
            while (!dist_iter->dist) {
                /* no dist, so no move. Try another, but when
                 * there is no dist at all, stop the loop
                 */
                dist_iter = dist_iter->next;
                if (dist_iter == dist_aux) {
                    /* all dist we zeroed */
                    goto movedone;
                }
            }
            /* something to move */

            /* get the item to move */
            for (k = 0; k < ordered->count; k++) {
                if (ordered->items[k].dist == dist_iter) {
                    break;
                }
            }

            /* apply the move (distance) */
            memcpy(&item_aux, &ordered->items[k], sizeof item_aux);
            if (dist_iter->dist > 0) {
                /* move to right (other move to left) */
                while (dist_iter->dist) {
                    memcpy(&ordered->items[k], &ordered->items[k + 1], sizeof *ordered->items);
                    ordered->items[k].dist->dist++; /* update moved item distance */
                    dist_iter->dist--;
                    k++;
                }
            } else {
                /* move to left (other move to right) */
                while (dist_iter->dist) {
                    memcpy(&ordered->items[k], &ordered->items[k - 1], sizeof *ordered->items);
                    ordered->items[k].dist->dist--; /* update moved item distance */
                    dist_iter->dist++;
                    k--;
                }
            }
            memcpy(&ordered->items[k], &item_aux, sizeof *ordered->items);

            /* store the transaction into the difflist */
            if (lyd_difflist_add(result, &size, index++, LYD_DIFF_MOVEDAFTER1, item_aux.first,
                                 (k > 0) ? ordered->items[k - 1].first : NULL)) {
                goto error;
            }
            continue;

movedone:
            break;
        }
    }

    diff_ordset_free(ordset);
    ordset = NULL;

    if (index2) {
        /* append result2 with newly created
         * (and possibly moved) nodes */
        if (index + index2 + 1 >= size) {
            /* result must be enlarged */
            size = index + index2 + 1;
            new = realloc(result->type, size * sizeof *result->type);
            LY_CHECK_ERR_GOTO(!new, LOGMEM(ctx), error);
            result->type = new;

            new = realloc(result->first, size * sizeof *result->first);
            LY_CHECK_ERR_GOTO(!new, LOGMEM(ctx), error);
            result->first = new;

            new = realloc(result->second, size * sizeof *result->second);
            LY_CHECK_ERR_GOTO(!new, LOGMEM(ctx), error);
            result->second = new;
        }

        /* append */
        memcpy(&result->type[index], result2->type, (index2 + 1) * sizeof *result->type);
        memcpy(&result->first[index], result2->first, (index2 + 1) * sizeof *result->first);
        memcpy(&result->second[index], result2->second, (index2 + 1) * sizeof *result->second);
    }
    lyd_free_diff(result2);

    return result;

error:
    while (matchlist) {
        mlaux = matchlist;
        matchlist = mlaux->prev;
        ly_set_free(mlaux->match);
        free(mlaux);

    }
    diff_ordset_free(ordset);

    lyd_free_diff(result);
    lyd_free_diff(result2);

    return NULL;
}

static void
lyd_insert_setinvalid(struct lyd_node *node)
{
    struct lyd_node *next, *elem, *parent_list;

    assert(node);

    /* overall validity of the node itself */
    node->validity = ly_new_node_validity(node->schema);

    /* explore changed unique leaves */
    /* first, get know if there is a list in parents chain */
    for (parent_list = node->parent;
         parent_list && parent_list->schema->nodetype != LYS_LIST;
         parent_list = parent_list->parent);
    if (parent_list && !(parent_list->validity & LYD_VAL_UNIQUE)) {
        /* there is a list, so check if we inserted a leaf supposed to be unique */
        for (elem = node; elem; elem = next) {
            if (elem->schema->nodetype == LYS_LIST) {
                /* stop searching to the depth, children would be unique to a list in subtree */
                goto nextsibling;
            }

            if (elem->schema->nodetype == LYS_LEAF && (elem->schema->flags & LYS_UNIQUE)) {
                /* set flag to list for future validation */
                parent_list->validity |= LYD_VAL_UNIQUE;
                break;
            }

            if (elem->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                if (elem == node) {
                    /* stop the loop */
                    break;
                }
                goto nextsibling;
            }

            /* select next elem to process */
            /* go into children */
            next = elem->child;
            /* go through siblings */
            if (!next) {
nextsibling:
                next = elem->next;
                if (!next) {
                    /* no sibling */
                    if (elem == node) {
                        /* we are done, back in start node */
                        break;
                    }
                }
            }
            /* go back to parents */
            while (!next) {
                elem = elem->parent;
                if (elem->parent == node->parent) {
                    /* we are done, back in start node */
                    break;
                }
                /* parent was actually already processed, so go to the parent's sibling */
                next = elem->parent->next;
            }
        }
    }

    if (node->parent) {
        /* if the inserted node is list/leaflist with constraint on max instances,
         * invalidate the parent to make it validate this */
        if (node->schema->nodetype & LYS_LEAFLIST) {
            if (((struct lys_node_leaflist *)node->schema)->max) {
                node->parent->validity |= LYD_VAL_MAND;
            }
        } else if (node->schema->nodetype & LYS_LIST) {
            if (((struct lys_node_list *)node->schema)->max) {
                node->parent->validity |= LYD_VAL_MAND;
            }
        }
    }
}

static void
lyd_replace(struct lyd_node *orig, struct lyd_node *repl, int destroy)
{
    struct lyd_node *iter, *last;

    if (!repl) {
        /* remove the old one */
        goto finish;
    }

    if (repl->parent || repl->prev->next) {
        /* isolate the new node */
        repl->next = NULL;
        repl->prev = repl;
        last = repl;
    } else {
        /* get the last node of a possible list of nodes to be inserted */
        for(last = repl; last->next; last = last->next) {
            /* part of the parent changes */
            last->parent = orig->parent;
        }
    }

    /* parent */
    if (orig->parent) {
        if (orig->parent->child == orig) {
            orig->parent->child = repl;
        }
        orig->parent = NULL;
    }

    /* predecessor */
    if (orig->prev == orig) {
        /* the old was alone */
        goto finish;
    }
    if (orig->prev->next) {
        orig->prev->next = repl;
    }
    repl->prev = orig->prev;
    orig->prev = orig;

    /* successor */
    if (orig->next) {
        orig->next->prev = last;
        last->next = orig->next;
        orig->next = NULL;
    } else {
        /* fix the last pointer */
        if (repl->parent) {
            repl->parent->child->prev = last;
        } else {
            /* get the first sibling */
            for (iter = repl; iter->prev != orig; iter = iter->prev);
            iter->prev = last;
        }
    }

finish:
    /* remove the old one */
    if (destroy) {
        lyd_free(orig);
    }
}

int
lyd_insert_common(struct lyd_node *parent, struct lyd_node **sibling, struct lyd_node *node, int invalidate)
{
    struct lys_node *par1, *par2;
    const struct lys_node *siter;
    struct lyd_node *start, *iter, *ins, *next1, *next2;
    int invalid = 0, isrpc = 0, clrdflt = 0;
    struct ly_set *llists = NULL;
    int i;
    uint8_t pos;
    int stype = LYS_INPUT | LYS_OUTPUT;

    assert(parent || sibling);

    /* get first sibling */
    if (parent) {
        start = parent->child;
    } else {
        for (start = *sibling; start->prev->next; start = start->prev);
    }

    /* check placing the node to the appropriate place according to the schema */
    if (!start) {
        if (!parent) {
            /* empty tree to insert */
            if (node->parent || node->prev->next) {
                /* unlink the node first */
                lyd_unlink_internal(node, 1);
            } /* else insert also node's siblings */
            *sibling = node;
            return EXIT_SUCCESS;
        }
        par1 = parent->schema;
        if (par1->nodetype & (LYS_RPC | LYS_ACTION)) {
            /* it is not clear if the tree being created is going to
             * be rpc (LYS_INPUT) or rpc-reply (LYS_OUTPUT) so we have to
             * compare against LYS_RPC or LYS_ACTION in par2
             */
            stype = LYS_RPC | LYS_ACTION;
        }
    } else if (parent && (parent->schema->nodetype & (LYS_RPC | LYS_ACTION))) {
        par1 = parent->schema;
        stype = LYS_RPC | LYS_ACTION;
    } else {
        for (par1 = lys_parent(start->schema);
             par1 && !(par1->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_INPUT | LYS_OUTPUT | LYS_NOTIF));
             par1 = lys_parent(par1));
    }
    for (par2 = lys_parent(node->schema);
         par2 && !(par2->nodetype & (LYS_CONTAINER | LYS_LIST | stype | LYS_NOTIF));
         par2 = lys_parent(par2));
    if (par1 != par2) {
        LOGERR(parent->schema->module->ctx, LY_EINVAL, "Cannot insert, different parents (\"%s\" and \"%s\").",
               (par1 ? par1->name : "<top-lvl>"), (par2 ? par2->name : "<top-lvl>"));
        return EXIT_FAILURE;
    }

    if (invalidate) {
        invalid = isrpc = lyp_is_rpc_action(node->schema);
        if (!parent || node->parent != parent || isrpc) {
            /* it is not just moving under a parent node or it is in an RPC where
             * nodes order matters, so the validation will be necessary */
            invalid++;
        }
    }

    /* unlink only if it is not a list of siblings without a parent and node is not the first sibling */
    if (node->parent || node->prev->next) {
        /* do it permanent if the parents are not exact same or if it is top-level */
        lyd_unlink_internal(node, invalid);
    }

    llists = ly_set_new();

    /* process the nodes to insert one by one */
    LY_TREE_FOR_SAFE(node, next1, ins) {
        if (invalid == 1) {
            /* auto delete nodes from other cases, if any;
             * this is done only if node->parent != parent */
            if (lyv_multicases(ins, NULL, &start, 1, NULL)) {
                goto error;
            }
        }

        /* isolate the node to be handled separately */
        ins->prev = ins;
        ins->next = NULL;

        iter = NULL;
        if (!ins->dflt) {
            clrdflt = 1;
        }

        /* are we inserting list key? */
        if (!ins->dflt && ins->schema->nodetype == LYS_LEAF && lys_is_key((struct lys_node_leaf *)ins->schema, &pos)) {
            /* yes, we have a key, get know its position */
            for (i = 0, iter = parent->child;
                    iter && i < pos && iter->schema->nodetype == LYS_LEAF;
                    i++, iter = iter->next);
            if (iter) {
                /* insert list's key to the correct position - before the iter */
                if (parent->child == iter) {
                    parent->child = ins;
                }
                if (iter->prev->next) {
                    iter->prev->next = ins;
                }
                ins->prev = iter->prev;
                iter->prev = ins;
                ins->next = iter;

                /* update start element */
                if (parent->child != start) {
                    start = parent->child;
                }
            }

            /* try to find previously present default instance to replace */
        } else if (ins->schema->nodetype == LYS_LEAFLIST) {
            i = (int)llists->number;
            if ((ly_set_add(llists, ins->schema, 0) != i) || ins->dflt) {
                /* each leaf-list must be cleared only once (except when looking for exact same existing dflt nodes) */
                LY_TREE_FOR_SAFE(start, next2, iter) {
                    if (iter->schema == ins->schema) {
                        if ((ins->dflt && (!iter->dflt || ((iter->schema->flags & LYS_CONFIG_W) &&
                                                           !strcmp(((struct lyd_node_leaf_list *)iter)->value_str,
                                                                  ((struct lyd_node_leaf_list *)ins)->value_str))))
                                || (!ins->dflt && iter->dflt)) {
                            if (iter == start) {
                                start = next2;
                            }
                            lyd_free(iter);
                        }
                    }
                }
            }
        } else if (ins->schema->nodetype == LYS_LEAF || (ins->schema->nodetype == LYS_CONTAINER
                        && !((struct lys_node_container *)ins->schema)->presence)) {
            LY_TREE_FOR(start, iter) {
                if (iter->schema == ins->schema) {
                    if (ins->dflt || iter->dflt) {
                        /* replace existing (either explicit or default) node with the new (either explicit or default) node */
                        lyd_replace(iter, ins, 1);
                    } else {
                        /* keep both explicit nodes, let the caller solve it later */
                        iter = NULL;
                    }
                    break;
                }
            }
        }

        if (!iter) {
            if (!start) {
                /* add as the only child of the parent */
                start = ins;
                if (parent) {
                    parent->child = ins;
                }
            } else if (isrpc) {
                /* add to the specific position in rpc/rpc-reply/action */
                for (par1 = ins->schema->parent; !(par1->nodetype & (LYS_INPUT | LYS_OUTPUT)); par1 = lys_parent(par1));
                siter = NULL;
                LY_TREE_FOR(start, iter) {
                    while ((siter = lys_getnext(siter, par1, lys_node_module(par1), 0))) {
                        if (iter->schema == siter || ins->schema == siter) {
                            break;
                        }
                    }
                    if (ins->schema == siter) {
                        if ((siter->nodetype & (LYS_LEAFLIST | LYS_LIST)) && iter->schema == siter) {
                            /* we are inserting leaflist/list instance, but since there are already
                             * some instances of the same leaflist/list, we want to insert the new one
                             * as the last instance, so here we have to move on */
                            while (iter && iter->schema == siter) {
                                iter = iter->next;
                            }
                            if (!iter) {
                                break;
                            }
                        }
                        /* we have the correct place for new node (before the iter) */
                        if (iter == start) {
                            start = ins;
                            if (parent) {
                                parent->child = ins;
                            }
                        } else {
                            iter->prev->next = ins;
                        }
                        ins->prev = iter->prev;
                        iter->prev = ins;
                        ins->next = iter;

                        /* we are done */
                        break;
                    }
                }
                if (!iter) {
                    /* add as the last child of the parent */
                    start->prev->next = ins;
                    ins->prev = start->prev;
                    start->prev = ins;
                }
            } else {
                /* add as the last child of the parent */
                start->prev->next = ins;
                ins->prev = start->prev;
                start->prev = ins;
            }
        }

#ifdef LY_ENABLED_CACHE
        lyd_unlink_hash(ins, ins->parent);
#endif

        ins->parent = parent;

#ifdef LY_ENABLED_CACHE
        lyd_insert_hash(ins);
#endif

        if (invalidate) {
            check_leaf_list_backlinks(ins, 0);
        }

        if (invalid) {
            lyd_insert_setinvalid(ins);
        }
    }
    ly_set_free(llists);

    if (clrdflt) {
        /* remove the dflt flag from parents */
        for (iter = parent; iter && iter->dflt; iter = iter->parent) {
            iter->dflt = 0;
        }
    }

    if (sibling) {
        *sibling = start;
    }
    return EXIT_SUCCESS;

error:
    ly_set_free(llists);
    return EXIT_FAILURE;
}

API int
lyd_insert(struct lyd_node *parent, struct lyd_node *node)
{
    if (!node || !parent || (parent->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA))) {
        LOGARG;
        return EXIT_FAILURE;
    }

    return lyd_insert_common(parent, NULL, node, 1);
}

API int
lyd_insert_sibling(struct lyd_node **sibling, struct lyd_node *node)
{
    if (!sibling || !node) {
        LOGARG;
        return EXIT_FAILURE;
    }

    return lyd_insert_common((*sibling) ? (*sibling)->parent : NULL, sibling, node, 1);

}

int
lyd_insert_nextto(struct lyd_node *sibling, struct lyd_node *node, int before, int invalidate)
{
    struct ly_ctx *ctx;
    struct lys_node *par1, *par2;
    struct lyd_node *iter, *start = NULL, *ins, *next1, *next2, *last;
    struct lyd_node *orig_parent = NULL, *orig_prev = NULL, *orig_next = NULL;
    int invalid = 0;
    char *str;

    assert(sibling);
    assert(node);

    ctx = sibling->schema->module->ctx;

    if (sibling == node) {
        return EXIT_SUCCESS;
    }

    /* check placing the node to the appropriate place according to the schema */
    for (par1 = lys_parent(sibling->schema);
         par1 && !(par1->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_INPUT | LYS_OUTPUT | LYS_ACTION | LYS_NOTIF));
         par1 = lys_parent(par1));
    for (par2 = lys_parent(node->schema);
         par2 && !(par2->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_INPUT | LYS_OUTPUT | LYS_ACTION | LYS_NOTIF));
         par2 = lys_parent(par2));
    if (par1 != par2) {
        LOGERR(ctx, LY_EINVAL, "Cannot insert, different parents (\"%s\" and \"%s\").",
               (par1 ? par1->name : "<top-lvl>"), (par2 ? par2->name : "<top-lvl>"));
        return EXIT_FAILURE;
    }

    if (invalidate && ((node->parent != sibling->parent) || (invalid = lyp_is_rpc_action(node->schema)) || !node->parent)) {
        /* a) it is not just moving under a parent node (invalid = 1) or
         * b) it is in an RPC where nodes order matters (invalid = 2) or
         * c) it is top-level where we don't know if it is the same tree (invalid = 1),
         * so the validation will be necessary */
        if (!node->parent && !invalid) {
            /* c) search in siblings */
            for (iter = node->prev; iter != node; iter = iter->prev) {
                if (iter == sibling) {
                    break;
                }
            }
            if (iter == node) {
                /* node and siblings are not currently in the same data tree */
                invalid++;
            }
        } else { /* a) and b) */
            invalid++;
        }
    }

    /* unlink only if it is not a list of siblings without a parent or node is not the first sibling,
     * always unlink if just moving a node */
    if ((!invalid) || node->parent || node->prev->next) {
        /* remember the original position to be able to revert
         * unlink in case of error */
        orig_parent = node->parent;
        if (node->prev != node) {
            orig_prev = node->prev;
        }
        orig_next = node->next;
        lyd_unlink_internal(node, invalid);
    }

    /* find first sibling node */
    if (sibling->parent) {
        start = sibling->parent->child;
    } else {
        for (start = sibling; start->prev->next; start = start->prev);
    }

    /* process the nodes one by one to clean the current tree */
    if (!invalid) {
        /* just moving one sibling */
        last = node;
        node->parent = sibling->parent;
    } else {
        LY_TREE_FOR_SAFE(node, next1, ins) {
            lyd_insert_setinvalid(ins);

            if (invalid == 1) {
                /* auto delete nodes from other cases */
                if (lyv_multicases(ins, NULL, &start, 1, sibling) == 2) {
                    LOGVAL(ctx, LYE_SPEC, LY_VLOG_LYD, sibling, "Insert request refers node (%s) that is going to be auto-deleted.",
                        ly_errpath(ctx));
                    goto error;
                }
            }

            /* try to find previously present default instance to remove because of
            * inserting the specified node */
            if (ins->schema->nodetype == LYS_LEAFLIST) {
                LY_TREE_FOR_SAFE(start, next2, iter) {
                    if (iter->schema == ins->schema) {
                        if ((ins->dflt && (!iter->dflt || ((iter->schema->flags & LYS_CONFIG_W) &&
                                                        !strcmp(((struct lyd_node_leaf_list *)iter)->value_str,
                                                                ((struct lyd_node_leaf_list *)ins)->value_str))))
                                || (!ins->dflt && iter->dflt)) {
                            /* iter will get deleted */
                            if (iter == sibling) {
                                LOGERR(ctx, LY_EINVAL, "Insert request refers node (%s) that is going to be auto-deleted.",
                                    str = lyd_path(sibling));
                                free(str);
                                goto error;
                            }
                            if (iter == start) {
                                start = next2;
                            }
                            lyd_free(iter);
                        }
                    }
                }
            } else if (ins->schema->nodetype == LYS_LEAF ||
                    (ins->schema->nodetype == LYS_CONTAINER && !((struct lys_node_container *)ins->schema)->presence)) {
                LY_TREE_FOR(start, iter) {
                    if (iter->schema == ins->schema) {
                        if (iter->dflt || ins->dflt) {
                            /* iter gets deleted */
                            if (iter == sibling) {
                                LOGERR(ctx, LY_EINVAL, "Insert request refers node (%s) that is going to be auto-deleted.",
                                    str = lyd_path(sibling));
                                free(str);
                                goto error;
                            }
                            if (iter == start) {
                                start = iter->next;
                            }
                            lyd_free(iter);
                        }
                        break;
                    }
                }
            }

#ifdef LY_ENABLED_CACHE
            lyd_unlink_hash(ins, ins->parent);
#endif

            ins->parent = sibling->parent;

#ifdef LY_ENABLED_CACHE
            lyd_insert_hash(ins);
#endif
            last = ins;
        }
    }

    /* insert the (list of) node(s) to the specified position */
    if (before) {
        if (sibling->prev->next) {
            /* adding into a middle */
            sibling->prev->next = node;
        } else if (sibling->parent) {
            /* at the beginning */
            sibling->parent->child = node;
        }
        node->prev = sibling->prev;
        sibling->prev = last;
        last->next = sibling;
    } else { /* after */
        if (sibling->next) {
            /* adding into a middle - fix the prev pointer of the node after inserted nodes */
            last->next = sibling->next;
            sibling->next->prev = last;
        } else {
            /* at the end - fix the prev pointer of the first node */
            start->prev = last;
        }
        sibling->next = node;
        node->prev = sibling;
    }

    if (invalidate) {
        LY_TREE_FOR(node, next1) {
            check_leaf_list_backlinks(next1, 0);
            if (next1 == last) {
                break;
            }
        }
    }

    return EXIT_SUCCESS;

error:
    /* insert back to the original position */
    if (orig_prev) {
        lyd_insert_after(orig_prev, node);
    } else if (orig_next) {
        lyd_insert_before(orig_next, node);
    } else if (orig_parent) {
        /* there were no siblings */
        orig_parent->child = node;
        node->parent = orig_parent;
    }
    return EXIT_FAILURE;
}

API int
lyd_insert_before(struct lyd_node *sibling, struct lyd_node *node)
{
    if (!node || !sibling) {
        LOGARG;
        return EXIT_FAILURE;
    }

    return lyd_insert_nextto(sibling, node, 1, 1);
}

API int
lyd_insert_after(struct lyd_node *sibling, struct lyd_node *node)
{
    if (!node || !sibling) {
        LOGARG;
        return EXIT_FAILURE;
    }

    return lyd_insert_nextto(sibling, node, 0, 1);
}

static uint32_t
lys_module_pos(struct lys_module *module)
{
    int i;
    uint32_t pos = 1;

    for (i = 0; i < module->ctx->models.used; ++i) {
        if (module->ctx->models.list[i] == module) {
            return pos;
        }
        ++pos;
    }

    LOGINT(module->ctx);
    return 0;
}

static int
lys_module_node_pos_r(struct lys_node *first_sibling, struct lys_node *target, uint32_t *pos)
{
    const struct lys_node *next = NULL;

    /* the schema nodes are actually from data, lys_getnext skips non-data schema nodes for us (we know the parent will not be uses) */
    while ((next = lys_getnext(next, lys_parent(first_sibling), lys_node_module(first_sibling), LYS_GETNEXT_NOSTATECHECK))) {
        ++(*pos);
        if (target == next) {
            return 0;
        }
    }

    LOGINT(first_sibling->module->ctx);
    return 1;
}

static int
lyd_node_pos_cmp(const void *item1, const void *item2)
{
    uint32_t mpos1, mpos2;
    struct lyd_node_pos *np1, *np2;

    np1 = (struct lyd_node_pos *)item1;
    np2 = (struct lyd_node_pos *)item2;

    /* different modules? */
    if (lys_node_module(np1->node->schema) != lys_node_module(np2->node->schema)) {
        mpos1 = lys_module_pos(lys_node_module(np1->node->schema));
        mpos2 = lys_module_pos(lys_node_module(np2->node->schema));
        /* if lys_module_pos failed, there is nothing we can do anyway,
         * at least internal error will be printed */

        if (mpos1 > mpos2) {
            return 1;
        } else {
            return -1;
        }
    }

    if (np1->pos > np2->pos) {
        return 1;
    } else if (np1->pos < np2->pos) {
        return -1;
    }
    return 0;
}

API int
lyd_schema_sort(struct lyd_node *sibling, int recursive)
{
    uint32_t len, i;
    struct lyd_node *node;
    struct lys_node *first_ssibling = NULL;
    struct lyd_node_pos *array;

    if (!sibling) {
        LOGARG;
        return -1;
    }

    /* something actually to sort */
    if (sibling->prev != sibling) {

        /* find the beginning */
        sibling = lyd_first_sibling(sibling);

        /* count siblings */
        len = 0;
        for (node = sibling; node; node = node->next) {
            ++len;
        }

        array = malloc(len * sizeof *array);
        LY_CHECK_ERR_RETURN(!array, LOGMEM(sibling->schema->module->ctx), -1);

        /* fill arrays with positions and corresponding nodes */
        for (i = 0, node = sibling; i < len; ++i, node = node->next) {
            array[i].pos = 0;

            /* we need to repeat this for every module */
            if (!first_ssibling || (lyd_node_module(node) != lys_node_module(first_ssibling))) {
                /* find the data node schema parent */
                first_ssibling = node->schema;
                while (lys_parent(first_ssibling)
                        && (lys_parent(first_ssibling)->nodetype & (LYS_CHOICE | LYS_CASE | LYS_USES))) {
                    first_ssibling = lys_parent(first_ssibling);
                }

                /* find the beginning */
                if (lys_parent(first_ssibling)) {
                    first_ssibling = lys_parent(first_ssibling)->child;
                } else {
                    while (first_ssibling->prev->next) {
                        first_ssibling = first_ssibling->prev;
                    }
                }
            }

            if (lys_module_node_pos_r(first_ssibling, node->schema, &array[i].pos)) {
                free(array);
                return -1;
            }

            array[i].node = node;
        }

        /* sort the arrays */
        qsort(array, len, sizeof *array, lyd_node_pos_cmp);

        /* adjust siblings based on the sorted array */
        for (i = 0; i < len; ++i) {
            /* parent child */
            if (i == 0) {
                /* adjust sibling so that it still points to the beginning */
                sibling = array[i].node;
                if (array[i].node->parent) {
                    array[i].node->parent->child = array[i].node;
                }
            }

            /* prev */
            if (i > 0) {
                array[i].node->prev = array[i - 1].node;
            } else {
                array[i].node->prev = array[len - 1].node;
            }

            /* next */
            if (i < len - 1) {
                array[i].node->next = array[i + 1].node;
            } else {
                array[i].node->next = NULL;
            }
        }
        free(array);
    }

    /* sort all the children recursively */
    if (recursive) {
        LY_TREE_FOR(sibling, node) {
            if ((node->schema->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_RPC | LYS_ACTION | LYS_NOTIF))
                    && node->child && lyd_schema_sort(node->child, recursive)) {
                return -1;
            }
        }
    }

    return EXIT_SUCCESS;
}

static int
_lyd_validate(struct lyd_node **node, struct lyd_node *data_tree, struct ly_ctx *ctx, const struct lys_module **modules,
              int mod_count, struct lyd_difflist **diff, int options)
{
    struct lyd_node *root, *next1, *next2, *iter, *act_notif = NULL;
    int ret = EXIT_FAILURE;
    unsigned int i;
    struct unres_data *unres = NULL;
    const struct lys_module *yanglib_mod;

    unres = calloc(1, sizeof *unres);
    LY_CHECK_ERR_RETURN(!unres, LOGMEM(NULL), EXIT_FAILURE);

    if (diff) {
        unres->store_diff = 1;
        unres->diff = lyd_diff_init_difflist(ctx, &unres->diff_size);
    }

    if ((options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY)) && *node && ((*node)->schema->nodetype != LYS_RPC)) {
        options |= LYD_OPT_ACT_NOTIF;
    }
    if ((options & (LYD_OPT_NOTIF | LYD_OPT_NOTIF_FILTER)) && *node && ((*node)->schema->nodetype != LYS_NOTIF)) {
        options |= LYD_OPT_ACT_NOTIF;
    }

    LY_TREE_FOR_SAFE(*node, next1, root) {
        if (modules) {
            for (i = 0; i < (unsigned)mod_count; ++i) {
                if (lyd_node_module(root) == modules[i]) {
                    break;
                }
            }
            if (i == (unsigned)mod_count) {
                /* skip data that should not be validated */
                continue;
            }
        }

        LY_TREE_DFS_BEGIN(root, next2, iter) {
            if (iter->parent && (iter->schema->nodetype & (LYS_ACTION | LYS_NOTIF))) {
                if (!(options & LYD_OPT_ACT_NOTIF) || act_notif) {
                    LOGVAL(ctx, LYE_INELEM, LY_VLOG_LYD, iter, iter->schema->name);
                    LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Unexpected %s node \"%s\".",
                           (options & LYD_OPT_RPC ? "action" : "notification"), iter->schema->name);
                    goto cleanup;
                }
                act_notif = iter;
            }

            if (lyv_data_context(iter, options, unres) || lyv_data_content(iter, options, unres)) {
                goto cleanup;
            }

            /* basic validation successful */
            iter->validity &= ~LYD_VAL_MAND;

            /* empty non-default, non-presence container without attributes, make it default */
            if (!iter->dflt && (iter->schema->nodetype == LYS_CONTAINER) && !iter->child
                        && !((struct lys_node_container *)iter->schema)->presence && !iter->attr) {
                iter->dflt = 1;
            }

            LY_TREE_DFS_END(root, next2, iter);
        }

        if (options & LYD_OPT_NOSIBLINGS) {
            break;
        }

    }

    if (options & LYD_OPT_ACT_NOTIF) {
        if (!act_notif) {
            LOGVAL(ctx, LYE_MISSELEM, LY_VLOG_LYD, *node, (options & LYD_OPT_RPC ? "action" : "notification"), (*node)->schema->name);
            goto cleanup;
        }
        options &= ~LYD_OPT_ACT_NOTIF;
    }

    if (*node) {
        /* check for uniqueness of top-level lists/leaflists because
         * only the inner instances were tested in lyv_data_content() */
        yanglib_mod = ly_ctx_get_module(ctx ? ctx : (*node)->schema->module->ctx, "ietf-yang-library", NULL, 1);
        LY_TREE_FOR(*node, root) {
            if ((options & LYD_OPT_DATA_ADD_YANGLIB) && yanglib_mod && (root->schema->module == yanglib_mod)) {
                /* ietf-yang-library data present, so ignore the option to add them */
                options &= ~LYD_OPT_DATA_ADD_YANGLIB;
            }

            if (!(root->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) || !(root->validity & LYD_VAL_DUP)) {
                continue;
            }

            if (options & LYD_OPT_TRUSTED) {
                /* just clear the flag */
                root->validity &= ~LYD_VAL_DUP;
                continue;
            }

            if (lyv_data_dup(root, *node)) {
                goto cleanup;
            }
        }
    }

    /* add missing ietf-yang-library if requested */
    if (options & LYD_OPT_DATA_ADD_YANGLIB) {
        if (!(*node)) {
            (*node) = ly_ctx_info(ctx);
        } else if (lyd_merge((*node), ly_ctx_info(ctx), LYD_OPT_DESTRUCT | LYD_OPT_EXPLICIT)) {
            LOGERR(ctx, LY_EINT, "Adding ietf-yang-library data failed.");
            goto cleanup;
        }
    }

    /* add default values, resolve unres and check for mandatory nodes in final tree */
    if (lyd_defaults_add_unres(node, options, ctx, modules, mod_count, data_tree, act_notif, unres, 1)) {
        goto cleanup;
    }
    if (act_notif) {
        if (lyd_check_mandatory_tree(act_notif, ctx, modules, mod_count, options)) {
            goto cleanup;
        }
    } else {
        if (lyd_check_mandatory_tree(*node, ctx, modules, mod_count, options)) {
            goto cleanup;
        }
    }

    /* consolidate diff if created */
    if (diff) {
        assert(unres->store_diff);

        for (i = 0; i < unres->diff_idx; ++i) {
            if (unres->diff->type[i] == LYD_DIFF_CREATED) {
                if (unres->diff->second[i]->parent) {
                    unres->diff->first[i] = (struct lyd_node *)lyd_path(unres->diff->second[i]->parent);
                }
                unres->diff->second[i] = lyd_dup(unres->diff->second[i], LYD_DUP_OPT_RECURSIVE);
            }
        }

        *diff = unres->diff;
        unres->diff = 0;
        unres->diff_idx = 0;
    }

    ret = EXIT_SUCCESS;

cleanup:
    if (unres) {
        free(unres->node);
        free(unres->type);
        for (i = 0; i < unres->diff_idx; ++i) {
            if (unres->diff->type[i] == LYD_DIFF_DELETED) {
                lyd_free_withsiblings(unres->diff->first[i]);
                free(unres->diff->second[i]);
            }
        }
        lyd_free_diff(unres->diff);
        free(unres);
    }

    return ret;
}

API int
lyd_validate(struct lyd_node **node, int options, void *var_arg, ...)
{
    struct lyd_node *iter, *data_tree = NULL;
    struct lyd_difflist **diff = NULL;
    struct ly_ctx *ctx = NULL;
    va_list ap;

    if (!node) {
        LOGARG;
        return EXIT_FAILURE;
    }

    if (lyp_data_check_options(NULL, options, __func__)) {
        return EXIT_FAILURE;
    }

    data_tree = *node;

    if ((!(options & LYD_OPT_TYPEMASK)
            || (options & (LYD_OPT_DATA | LYD_OPT_CONFIG | LYD_OPT_GET | LYD_OPT_GETCONFIG | LYD_OPT_EDIT))) && !(*node)) {
        /* get context with schemas from the var_arg */
        ctx = (struct ly_ctx *)var_arg;
        if (!ctx) {
            LOGERR(NULL, LY_EINVAL, "%s: invalid variable parameter (struct ly_ctx *ctx).", __func__);
            return EXIT_FAILURE;
        }

        /* LYD_OPT_NOSIBLINGS has no meaning here */
        options &= ~LYD_OPT_NOSIBLINGS;
    } else if (options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY | LYD_OPT_NOTIF)) {
        /* LYD_OPT_NOSIBLINGS cannot be set in this case */
        if (options & LYD_OPT_NOSIBLINGS) {
            LOGERR(NULL, LY_EINVAL, "%s: invalid parameter (variable arg const struct lyd_node *data_tree with LYD_OPT_NOSIBLINGS).", __func__);
            return EXIT_FAILURE;
        } else if (!(*node)) {
            LOGARG;
            return EXIT_FAILURE;
        }

        /* get the additional data tree if given */
        data_tree = (struct lyd_node *)var_arg;
        if (data_tree) {
            if (options & LYD_OPT_NOEXTDEPS) {
                LOGERR(NULL, LY_EINVAL, "%s: invalid parameter (variable arg const struct lyd_node *data_tree and LYD_OPT_NOEXTDEPS set).",
                       __func__);
                return EXIT_FAILURE;
            }

            LY_TREE_FOR(data_tree, iter) {
                if (iter->parent) {
                    /* a sibling is not top-level */
                    LOGERR(NULL, LY_EINVAL, "%s: invalid variable parameter (const struct lyd_node *data_tree).", __func__);
                    return EXIT_FAILURE;
                }
            }

            /* move it to the beginning */
            for (; data_tree->prev->next; data_tree = data_tree->prev);
        }
    } else if (options & LYD_OPT_DATA_TEMPLATE) {
        /* get context with schemas from the var_arg */
        if (*node && ((*node)->prev->next || (*node)->next)) {
            /* not allow sibling in top-level */
            LOGERR(NULL, LY_EINVAL, "%s: invalid variable parameter (struct lyd_node *node).", __func__);
            return EXIT_FAILURE;
        }
    }

    if (options & LYD_OPT_VAL_DIFF) {
        va_start(ap, var_arg);
        diff = va_arg(ap, struct lyd_difflist **);
        va_end(ap);
        if (!diff) {
            LOGERR(ctx, LY_EINVAL, "%s: invalid variable parameter (struct lyd_difflist **).", __func__);
            return EXIT_FAILURE;
        }
    }

    if (*node) {
        if (!ctx) {
            ctx = (*node)->schema->module->ctx;
        }
        if (!(options & LYD_OPT_NOSIBLINGS)) {
            /* check that the node is the first sibling */
            while ((*node)->prev->next) {
                *node = (*node)->prev;
            }
        }
    }

    return _lyd_validate(node, data_tree, ctx, NULL, 0, diff, options);
}

API int
lyd_validate_modules(struct lyd_node **node, const struct lys_module **modules, int mod_count, int options, ...)
{
    struct ly_ctx *ctx;
    struct lyd_difflist **diff = NULL;
    va_list ap;

    if (!node || !modules || !mod_count) {
        LOGARG;
        return EXIT_FAILURE;
    }

    ctx = modules[0]->ctx;

    if (*node && !(options & LYD_OPT_NOSIBLINGS)) {
        /* check that the node is the first sibling */
        while ((*node)->prev->next) {
            *node = (*node)->prev;
        }
    }

    if (lyp_data_check_options(ctx, options, __func__)) {
        return EXIT_FAILURE;
    }

    if (!(options & (LYD_OPT_DATA | LYD_OPT_CONFIG | LYD_OPT_GET | LYD_OPT_GETCONFIG | LYD_OPT_EDIT))) {
        LOGERR(NULL, LY_EINVAL, "%s: options include a forbidden data type.", __func__);
        return EXIT_FAILURE;
    }

    if (options & LYD_OPT_VAL_DIFF) {
        va_start(ap, options);
        diff = va_arg(ap, struct lyd_difflist **);
        va_end(ap);
        if (!diff) {
            LOGERR(ctx, LY_EINVAL, "%s: invalid variable parameter (struct lyd_difflist **).", __func__);
            return EXIT_FAILURE;
        }
    }

    return _lyd_validate(node, *node, ctx, modules, mod_count, diff, options);
}

API int
lyd_validate_value(struct lys_node *node, const char *value)
{
    struct lyd_node_leaf_list leaf;
    struct lys_node_leaf *sleaf = (struct lys_node_leaf*)node;
    int ret = EXIT_SUCCESS;

    if (!node || !(node->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        LOGARG;
        return EXIT_FAILURE;
    }

    if (!value) {
        value = "";
    }

    /* dummy leaf */
    memset(&leaf, 0, sizeof leaf);
    leaf.value_str = lydict_insert(node->module->ctx, value, 0);

repeat:
    leaf.value_type = sleaf->type.base;
    leaf.schema = node;

    if (leaf.value_type == LY_TYPE_LEAFREF) {
        if (!sleaf->type.info.lref.target) {
            /* it should either be unresolved leafref (leaf.value_type are ORed flags) or it will be resolved */
            LOGINT(node->module->ctx);
            ret = EXIT_FAILURE;
            goto cleanup;
        }
        sleaf = sleaf->type.info.lref.target;
        goto repeat;
    } else {
        if (!lyp_parse_value(&sleaf->type, &leaf.value_str, NULL, &leaf, NULL, NULL, 0, 0, 0)) {
            ret = EXIT_FAILURE;
            goto cleanup;
        }
    }

cleanup:
    lydict_remove(node->module->ctx, leaf.value_str);
    return ret;
}

/* create an attribute copy */
static struct lyd_attr *
lyd_dup_attr(struct ly_ctx *ctx, struct lyd_node *parent, struct lyd_attr *attr)
{
    struct lyd_attr *ret;

    /* allocate new attr */
    if (!parent->attr) {
        parent->attr = malloc(sizeof *parent->attr);
        ret = parent->attr;
    } else {
        for (ret = parent->attr; ret->next; ret = ret->next);
        ret->next = calloc(1, sizeof *ret);
        ret = ret->next;
    }
    LY_CHECK_ERR_RETURN(!ret, LOGMEM(ctx), NULL);

    /* fill new attr except */
    ret->parent = parent;
    ret->next = NULL;
    ret->annotation = attr->annotation;
    ret->name = lydict_insert(ctx, attr->name, 0);
    ret->value_str = lydict_insert(ctx, attr->value_str, 0);
    ret->value_type = attr->value_type;
    ret->value_flags = attr->value_flags;
    switch (ret->value_type) {
    case LY_TYPE_BINARY:
    case LY_TYPE_STRING:
        /* value_str pointer is shared in these cases */
        ret->value.string = ret->value_str;
        break;
    case LY_TYPE_LEAFREF:
        lyp_parse_value(*((struct lys_type **)lys_ext_complex_get_substmt(LY_STMT_TYPE, ret->annotation, NULL)),
                             &ret->value_str, NULL, NULL, ret, NULL, 1, 0, 0);
        break;
    case LY_TYPE_INST:
        ret->value.instance = NULL;
        break;
    case LY_TYPE_UNION:
        /* unresolved union (this must be non-validated tree), duplicate the stored string (duplicated
         * because of possible change of the value in case of instance-identifier) */
        ret->value.string = lydict_insert(ctx, attr->value.string, 0);
        break;
    case LY_TYPE_ENUM:
    case LY_TYPE_IDENT:
    case LY_TYPE_BITS:
        /* in case of duplicating bits (no matter if in the same context or not) or enum and identityref into
         * a different context, searching for the type and duplicating the data is almost as same as resolving
         * the string value, so due to a simplicity, parse the value for the duplicated leaf */
        lyp_parse_value(*((struct lys_type **)lys_ext_complex_get_substmt(LY_STMT_TYPE, ret->annotation, NULL)),
                             &ret->value_str, NULL, NULL, ret, NULL, 1, 0, 0);
        break;
    default:
        ret->value = attr->value;
        break;
    }
    return ret;
}

int
lyd_unlink_internal(struct lyd_node *node, int permanent)
{
    struct lyd_node *iter;

    if (!node) {
        LOGARG;
        return EXIT_FAILURE;
    }

    if (permanent) {
        check_leaf_list_backlinks(node, 1);
    }

    /* unlink from siblings */
    if (node->prev->next) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        /* unlinking the last node */
        if (node->parent) {
            iter = node->parent->child;
        } else {
            iter = node->prev;
            while (iter->prev != node) {
                iter = iter->prev;
            }
        }
        /* update the "last" pointer from the first node */
        iter->prev = node->prev;
    }

    /* unlink from parent */
    if (node->parent) {
        if (node->parent->child == node) {
            /* the node is the first child */
            node->parent->child = node->next;
        }

#ifdef LY_ENABLED_CACHE
        /* do not remove from parent hash table if freeing the whole subtree */
        if (permanent != 2) {
            lyd_unlink_hash(node, node->parent);
        }
#endif

        node->parent = NULL;
    }

    node->next = NULL;
    node->prev = node;

    return EXIT_SUCCESS;
}

API int
lyd_unlink(struct lyd_node *node)
{
    return lyd_unlink_internal(node, 1);
}

/*
 * - in leaflist it must be added with value_str
 */
static int
_lyd_dup_node_common(struct lyd_node *new_node, const struct lyd_node *orig, struct ly_ctx *ctx, int options)
{
    struct lyd_attr *attr;

    new_node->attr = NULL;
    if (!(options & LYD_DUP_OPT_NO_ATTR)) {
        LY_TREE_FOR(orig->attr, attr) {
            lyd_dup_attr(ctx, new_node, attr);
        }
    }
    new_node->next = NULL;
    new_node->prev = new_node;
    new_node->parent = NULL;
    new_node->validity = ly_new_node_validity(new_node->schema);
    new_node->dflt = orig->dflt;
    new_node->when_status = orig->when_status & LYD_WHEN;
#ifdef LY_ENABLED_CACHE
    /* just copy the hash, it will not change */
    if ((new_node->schema->nodetype != LYS_LIST) || lyd_list_has_keys(new_node)) {
        new_node->hash = orig->hash;
    }
#endif

#ifdef LY_ENABLED_LYD_PRIV
    if (ctx->priv_dup_clb) {
        new_node->priv = ctx->priv_dup_clb(orig->priv);
    }
#endif

    return EXIT_SUCCESS;
}

static struct lyd_node *
_lyd_dup_node(const struct lyd_node *node, const struct lys_node *schema, struct ly_ctx *ctx, int options)
{
    struct lyd_node *new_node = NULL;
    struct lys_node_leaf *sleaf;
    struct lyd_node_leaf_list *new_leaf;
    struct lyd_node_anydata *new_any, *old_any;
    int r;

    /* fill specific part */
    switch (node->schema->nodetype) {
    case LYS_LEAF:
    case LYS_LEAFLIST:
        new_leaf = calloc(1, sizeof *new_leaf);
        new_node = (struct lyd_node *)new_leaf;
        LY_CHECK_ERR_GOTO(!new_node, LOGMEM(ctx), error);
        new_node->schema = (struct lys_node *)schema;

        new_leaf->value_str = lydict_insert(ctx, ((struct lyd_node_leaf_list *)node)->value_str, 0);
        new_leaf->value_type = ((struct lyd_node_leaf_list *)node)->value_type;
        new_leaf->value_flags = ((struct lyd_node_leaf_list *)node)->value_flags;
        if (_lyd_dup_node_common(new_node, node, ctx, options)) {
            goto error;
        }

        /* get schema from the correct context */
        sleaf = (struct lys_node_leaf *)new_leaf->schema;

        switch (new_leaf->value_type) {
        case LY_TYPE_BINARY:
        case LY_TYPE_STRING:
            /* value_str pointer is shared in these cases */
            new_leaf->value.string = new_leaf->value_str;
            break;
        case LY_TYPE_LEAFREF:
            new_leaf->validity |= LYD_VAL_LEAFREF;
            lyp_parse_value(&sleaf->type, &new_leaf->value_str, NULL, new_leaf, NULL, NULL, 1, node->dflt, 0);
            break;
        case LY_TYPE_INST:
            new_leaf->value.instance = NULL;
            break;
        case LY_TYPE_UNION:
            /* unresolved union (this must be non-validated tree), duplicate the stored string (duplicated
             * because of possible change of the value in case of instance-identifier) */
            new_leaf->value.string = lydict_insert(ctx, ((struct lyd_node_leaf_list *)node)->value.string, 0);
            break;
        case LY_TYPE_ENUM:
        case LY_TYPE_IDENT:
        case LY_TYPE_BITS:
            /* in case of duplicating bits (no matter if in the same context or not) or enum and identityref into
             * a different context, searching for the type and duplicating the data is almost as same as resolving
             * the string value, so due to a simplicity, parse the value for the duplicated leaf */
            if (!lyp_parse_value(&sleaf->type, &new_leaf->value_str, NULL, new_leaf, NULL, NULL, 1, node->dflt, 0)) {
                goto error;
            }
            break;
        default:
            new_leaf->value = ((struct lyd_node_leaf_list *)node)->value;
            break;
        }

        if (sleaf->type.der && sleaf->type.der->module) {
            r = lytype_store(sleaf->type.der->module, sleaf->type.der->name, new_leaf->value_str, &new_leaf->value);
            if (r == -1) {
                goto error;
            } else if (!r) {
                new_leaf->value_flags |= LY_VALUE_USER;
            }
        }
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        old_any = (struct lyd_node_anydata *)node;
        new_any = calloc(1, sizeof *new_any);
        new_node = (struct lyd_node *)new_any;
        LY_CHECK_ERR_GOTO(!new_node, LOGMEM(ctx), error);
        new_node->schema = (struct lys_node *)schema;

        if (_lyd_dup_node_common(new_node, node, ctx, options)) {
            goto error;
        }

        new_any->value_type = old_any->value_type;
        if (!(void*)old_any->value.tree) {
            /* no value to duplicate */
            break;
        }
        /* duplicate the value */
        switch (old_any->value_type) {
        case LYD_ANYDATA_CONSTSTRING:
        case LYD_ANYDATA_SXML:
        case LYD_ANYDATA_JSON:
            new_any->value.str = lydict_insert(ctx, old_any->value.str, 0);
            break;
        case LYD_ANYDATA_DATATREE:
            new_any->value.tree = lyd_dup_to_ctx(old_any->value.tree, 1, ctx);
            break;
        case LYD_ANYDATA_XML:
            new_any->value.xml = lyxml_dup_elem(ctx, old_any->value.xml, NULL, 1);
            break;
        case LYD_ANYDATA_LYB:
            r = lyd_lyb_data_length(old_any->value.mem);
            if (r == -1) {
                LOGERR(ctx, LY_EINVAL, "Invalid LYB data.");
                goto error;
            }
            new_any->value.mem = malloc(r);
            LY_CHECK_ERR_GOTO(!new_any->value.mem, LOGMEM(ctx), error);
            memcpy(new_any->value.mem, old_any->value.mem, r);
            break;
        case LYD_ANYDATA_STRING:
        case LYD_ANYDATA_SXMLD:
        case LYD_ANYDATA_JSOND:
        case LYD_ANYDATA_LYBD:
            /* dynamic strings are used only as input parameters */
            assert(0);
            break;
        }
        break;
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_NOTIF:
    case LYS_RPC:
    case LYS_ACTION:
        new_node = calloc(1, sizeof *new_node);
        LY_CHECK_ERR_GOTO(!new_node, LOGMEM(ctx), error);
        new_node->schema = (struct lys_node *)schema;

        if (_lyd_dup_node_common(new_node, node, ctx, options)) {
            goto error;
        }
        break;
    default:
        LOGINT(ctx);
        goto error;
    }

    return new_node;

error:
    lyd_free(new_node);
    return NULL;
}

API struct lyd_node *
lyd_dup_to_ctx(const struct lyd_node *node, int options, struct ly_ctx *ctx)
{
    struct ly_ctx *log_ctx;
    struct lys_node_list *slist;
    struct lys_node *schema;
    const char *yang_data_name;
    const struct lys_module *trg_mod;
    const struct lyd_node *next, *elem;
    struct lyd_node *ret, *parent, *key, *key_dup, *new_node = NULL;
    uint16_t i;

    if (!node) {
        LOGARG;
        return NULL;
    }

    log_ctx = (ctx ? ctx : node->schema->module->ctx);
    if (ctx == node->schema->module->ctx) {
        /* target context is actually the same as the source context,
         * ignore the target context */
        ctx = NULL;
    }

    ret = NULL;
    parent = NULL;

    /* LY_TREE_DFS */
    for (elem = next = node; elem; elem = next) {

        /* find the correct schema */
        if (ctx) {
            schema = NULL;
            if (parent) {
                trg_mod = lyp_get_module(parent->schema->module, NULL, 0, lyd_node_module(elem)->name,
                                         strlen(lyd_node_module(elem)->name), 1);
                if (!trg_mod) {
                    LOGERR(log_ctx, LY_EINVAL, "Target context does not contain model for the data node being duplicated (%s).",
                                lyd_node_module(elem)->name);
                    goto error;
                }
                /* we know its parent, so we can start with it */
                lys_getnext_data(trg_mod, parent->schema, elem->schema->name, strlen(elem->schema->name),
                                 elem->schema->nodetype, (const struct lys_node **)&schema);
            } else {
                /* we have to search in complete context */
                schema = lyd_get_schema_inctx(elem, ctx);
            }

            if (!schema) {
                yang_data_name = lyp_get_yang_data_template_name(elem);
                if (yang_data_name) {
                    LOGERR(log_ctx, LY_EINVAL, "Target context does not contain schema node for the data node being duplicated "
                                        "(%s:#%s/%s).", lyd_node_module(elem)->name, yang_data_name, elem->schema->name);
                } else {
                    LOGERR(log_ctx, LY_EINVAL, "Target context does not contain schema node for the data node being duplicated "
                                        "(%s:%s).", lyd_node_module(elem)->name, elem->schema->name);
                }
                goto error;
            }
        } else {
            schema = elem->schema;
        }

        /* make node copy */
        new_node = _lyd_dup_node(elem, schema, log_ctx, options);
        if (!new_node) {
            goto error;
        }

        if (parent && lyd_insert(parent, new_node)) {
            goto error;
        }

        if (!ret) {
            ret = new_node;
        }

        if (!(options & LYD_DUP_OPT_RECURSIVE)) {
            break;
        }

        /* LY_TREE_DFS_END */
        /* select element for the next run - children first,
         * child exception for lyd_node_leaf and lyd_node_leaflist */
        if (elem->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
            next = NULL;
        } else {
            next = elem->child;
        }
        if (!next) {
            if (elem->parent == node->parent) {
                break;
            }
            /* no children, so try siblings */
            next = elem->next;
        } else {
            parent = new_node;
        }
        new_node = NULL;

        while (!next) {
            /* no siblings, go back through parents */
            elem = elem->parent;
            if (elem->parent == node->parent) {
                break;
            }
            if (!parent) {
                LOGINT(log_ctx);
                goto error;
            }
            parent = parent->parent;
            /* parent is already processed, go to its sibling */
            next = elem->next;
        }
    }

    /* dup all the parents */
    if (options & LYD_DUP_OPT_WITH_PARENTS) {
        parent = ret;
        for (elem = node->parent; elem; elem = elem->parent) {
            new_node = lyd_dup(elem, options & LYD_DUP_OPT_NO_ATTR);
            LY_CHECK_ERR_GOTO(!new_node, LOGMEM(log_ctx), error);

            /* dup all list keys */
            if (new_node->schema->nodetype == LYS_LIST) {
                slist = (struct lys_node_list *)new_node->schema;
                for (key = elem->child, i = 0; key && (i < slist->keys_size); ++i, key = key->next) {
                    if (key->schema != (struct lys_node *)slist->keys[i]) {
                        LOGVAL(log_ctx, LYE_PATH_INKEY, LY_VLOG_LYD, new_node, slist->keys[i]->name);
                        goto error;
                    }

                    key_dup = lyd_dup(key, options & LYD_DUP_OPT_NO_ATTR);
                    LY_CHECK_ERR_GOTO(!key_dup, LOGMEM(log_ctx), error);

                    if (lyd_insert(new_node, key_dup)) {
                        lyd_free(key_dup);
                        goto error;
                    }
                }
                if (!key && (i < slist->keys_size)) {
                    LOGVAL(log_ctx, LYE_PATH_INKEY, LY_VLOG_LYD, new_node, slist->keys[i]->name);
                    goto error;
                }
            }

            /* link together */
            if (lyd_insert(new_node, parent)) {
                ret = parent;
                goto error;
            }
            parent = new_node;
        }
    }

    return ret;

error:
    lyd_free(ret);
    return NULL;
}

API struct lyd_node *
lyd_dup(const struct lyd_node *node, int options)
{
    return lyd_dup_to_ctx(node, options, NULL);
}

static struct lyd_node *
lyd_dup_withsiblings_r(const struct lyd_node *first, struct lyd_node *parent_dup, int options)
{
    struct lyd_node *first_dup = NULL, *prev_dup = NULL, *last_dup;
    const struct lyd_node *next;

    assert(first);

    /* duplicate and connect all siblings */
    LY_TREE_FOR(first, next) {
        last_dup = _lyd_dup_node(next, next->schema, next->schema->module->ctx, options);
        if (!last_dup) {
            goto error;
        }

        /* the whole data tree is exactly the same so we can safely copy the validation flags */
        last_dup->validity = next->validity;
        last_dup->when_status = next->when_status;

        last_dup->parent = parent_dup;
        if (!first_dup) {
            first_dup = last_dup;
        } else {
            assert(prev_dup);
            prev_dup->next = last_dup;
            last_dup->prev = prev_dup;
        }

        if ((next->schema->nodetype & (LYS_LIST | LYS_CONTAINER | LYS_RPC | LYS_ACTION | LYS_NOTIF)) && next->child) {
            /* recursively duplicate all children */
            if (!lyd_dup_withsiblings_r(next->child, last_dup, options)) {
                goto error;
            }
        }

        prev_dup = last_dup;
    }

    /* correctly set last sibling and parent child pointer */
    assert(!prev_dup->next);
    first_dup->prev = prev_dup;
    if (parent_dup) {
        parent_dup->child = first_dup;
    }

    return first_dup;

error:
    /* disconnect and free */
    first_dup->parent = NULL;
    lyd_free_withsiblings(first_dup);
    return NULL;
}

API struct lyd_node *
lyd_dup_withsiblings(const struct lyd_node *node, int options)
{
    const struct lyd_node *iter;
    struct lyd_node *ret, *ret_iter, *tmp;

    if (!node) {
        return NULL;
    }

    /* find first sibling */
    while (node->prev->next) {
        node = node->prev;
    }

    if (node->parent) {
        ret = lyd_dup(node, options);
        if (!ret) {
            return NULL;
        }

        /* copy following siblings */
        ret_iter = ret;
        LY_TREE_FOR(node->next, iter) {
            tmp = lyd_dup(iter, options);
            if (!tmp) {
                lyd_free_withsiblings(ret);
                return NULL;
            }

            if (lyd_insert_after(ret_iter, tmp)) {
                lyd_free_withsiblings(ret);
                return NULL;
            }
            ret_iter = ret_iter->next;
            assert(ret_iter == tmp);
        }
    } else {
        /* duplicating top-level siblings, we can duplicate much more efficiently */
        ret = lyd_dup_withsiblings_r(node, NULL, options);
    }

    return ret;
}

API void
lyd_free_attr(struct ly_ctx *ctx, struct lyd_node *parent, struct lyd_attr *attr, int recursive)
{
    struct lyd_attr *iter;
    struct lys_type **type;

    if (!ctx || !attr) {
        return;
    }

    if (parent) {
        if (parent->attr == attr) {
            if (recursive) {
                parent->attr = NULL;
            } else {
                parent->attr = attr->next;
            }
        } else {
            for (iter = parent->attr; iter->next != attr; iter = iter->next);
            if (iter->next) {
                if (recursive) {
                    iter->next = NULL;
                } else {
                    iter->next = attr->next;
                }
            }
        }
    }

    if (!recursive) {
        attr->next = NULL;
    }

    for(iter = attr; iter; ) {
        attr = iter;
        iter = iter->next;

        lydict_remove(ctx, attr->name);
        type = lys_ext_complex_get_substmt(LY_STMT_TYPE, attr->annotation, NULL);
        assert(type);
        lyd_free_value(attr->value, attr->value_type, attr->value_flags, *type, NULL, NULL, NULL);
        lydict_remove(ctx, attr->value_str);
        free(attr);
    }
}

const struct lyd_node *
lyd_attr_parent(const struct lyd_node *root, struct lyd_attr *attr)
{
    const struct lyd_node *next, *elem;
    struct lyd_attr *node_attr;

    LY_TREE_DFS_BEGIN(root, next, elem) {
        for (node_attr = elem->attr; node_attr; node_attr = node_attr->next) {
            if (node_attr == attr) {
                return elem;
            }
        }
        LY_TREE_DFS_END(root, next, elem)
    }

    return NULL;
}

API struct lyd_attr *
lyd_insert_attr(struct lyd_node *parent, const struct lys_module *mod, const char *name, const char *value)
{
    struct lyd_attr *a, *iter;
    struct ly_ctx *ctx;
    const struct lys_module *module;
    const char *p;
    char *aux;
    int pos, i;

    if (!parent || !name || !value) {
        LOGARG;
        return NULL;
    }
    ctx = parent->schema->module->ctx;

    if ((p = strchr(name, ':'))) {
        /* search for the namespace */
        aux = strndup(name, p - name);
        if (!aux) {
            LOGMEM(ctx);
            return NULL;
        }
        module = ly_ctx_get_module(ctx, aux, NULL, 1);
        free(aux);
        name = p + 1;

        if (!module) {
            /* module not found */
            LOGERR(ctx, LY_EINVAL, "Attribute prefix does not match any implemented schema in the context.");
            return NULL;
        }
    } else if (mod) {
        module = mod;
    } else if (!mod && (!strcmp(name, "type") || !strcmp(name, "select")) && !strcmp(parent->schema->name, "filter")) {
        /* special case of inserting unqualified filter attributes "type" and "select" */
        module = ly_ctx_get_module(ctx, "ietf-netconf", NULL, 1);
        if (!module) {
            LOGERR(ctx, LY_EINVAL, "Attribute prefix does not match any implemented schema in the context.");
            return NULL;
        }
    } else {
        /* no prefix -> module is the same as for the parent */
        module = lyd_node_module(parent);
    }

    pos = -1;
    do {
        if ((unsigned int)(pos + 1) < module->ext_size) {
            i = lys_ext_instance_presence(&ctx->models.list[0]->extensions[0],
                                          &module->ext[pos + 1], module->ext_size - (pos + 1));
            pos = (i == -1) ? -1 : pos + 1 + i;
        } else {
            pos = -1;
        }
        if (pos == -1) {
            LOGERR(ctx, LY_EINVAL, "Attribute does not match any annotation instance definition.");
            return NULL;
        }
    } while (!ly_strequal(module->ext[pos]->arg_value, name, 0));

    a = calloc(1, sizeof *a);
    LY_CHECK_ERR_RETURN(!a, LOGMEM(ctx), NULL);
    a->parent = parent;
    a->next = NULL;
    a->annotation = (struct lys_ext_instance_complex *)module->ext[pos];
    a->name = lydict_insert(ctx, name, 0);
    a->value_str = lydict_insert(ctx, value, 0);
    if (!lyp_parse_value(*((struct lys_type **)lys_ext_complex_get_substmt(LY_STMT_TYPE, a->annotation, NULL)),
                         &a->value_str, NULL, NULL, a, NULL, 1, 0, 0)) {
        lyd_free_attr(ctx, NULL, a, 0);
        return NULL;
    }

    if (!parent->attr) {
        parent->attr = a;
    } else {
        for (iter = parent->attr; iter->next; iter = iter->next);
        iter->next = a;
    }

    return a;
}

void
lyd_free_value(lyd_val value, LY_DATA_TYPE value_type, uint8_t value_flags, struct lys_type *type, lyd_val *old_val,
               LY_DATA_TYPE *old_val_type, uint8_t *old_val_flags)
{
    if (old_val) {
        *old_val = value;
        *old_val_type = value_type;
        *old_val_flags = value_flags;
        /* we only backup the values for now */
        return;
    }

    /* otherwise the value is correctly freed */
    if (value_flags & LY_VALUE_USER) {
        assert(type->der && type->der->module);
        lytype_free(type->der->module, type->der->name, value);
    } else {
        switch (value_type) {
        case LY_TYPE_BITS:
            if (value.bit) {
                free(value.bit);
            }
            break;
        case LY_TYPE_INST:
            if (!(value_flags & LY_VALUE_UNRES)) {
                break;
            }
            /* fallthrough */
        case LY_TYPE_UNION:
            /* unresolved union leaf */
            lydict_remove(type->parent->module->ctx, value.string);
            break;
        default:
            break;
        }
    }
}

static void
_lyd_free_node(struct lyd_node *node)
{
    struct lyd_node_leaf_list *leaf;

    if (!node) {
        return;
    }

    switch (node->schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_RPC:
    case LYS_ACTION:
    case LYS_NOTIF:
#ifdef LY_ENABLED_CACHE
        /* it should be empty because all the children are freed already (only if in debug mode) */
        lyht_free(node->ht);
#endif
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        switch (((struct lyd_node_anydata *)node)->value_type) {
        case LYD_ANYDATA_CONSTSTRING:
        case LYD_ANYDATA_SXML:
        case LYD_ANYDATA_JSON:
            lydict_remove(node->schema->module->ctx, ((struct lyd_node_anydata *)node)->value.str);
            break;
        case LYD_ANYDATA_DATATREE:
            lyd_free_withsiblings(((struct lyd_node_anydata *)node)->value.tree);
            break;
        case LYD_ANYDATA_XML:
            lyxml_free_withsiblings(node->schema->module->ctx, ((struct lyd_node_anydata *)node)->value.xml);
            break;
        case LYD_ANYDATA_LYB:
            free(((struct lyd_node_anydata *)node)->value.mem);
            break;
        case LYD_ANYDATA_STRING:
        case LYD_ANYDATA_SXMLD:
        case LYD_ANYDATA_JSOND:
        case LYD_ANYDATA_LYBD:
            /* dynamic strings are used only as input parameters */
            assert(0);
            break;
        }
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
        leaf = (struct lyd_node_leaf_list *)node;
        lyd_free_value(leaf->value, leaf->value_type, leaf->value_flags, &((struct lys_node_leaf *)leaf->schema)->type,
                       NULL, NULL, NULL);
        lydict_remove(leaf->schema->module->ctx, leaf->value_str);
        break;
    default:
        assert(0);
    }

    lyd_free_attr(node->schema->module->ctx, node, node->attr, 1);
    free(node);
}

static void
lyd_free_internal_r(struct lyd_node *node, int top)
{
    struct lyd_node *next, *iter;

    if (!node) {
        return;
    }

    /* if freeing top-level, always remove it from the parent hash table */
    lyd_unlink_internal(node, (top ? 1 : 2));

    if (!(node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA))) {
        /* free children */
        LY_TREE_FOR_SAFE(node->child, next, iter) {
            lyd_free_internal_r(iter, 0);
        }
    }

    _lyd_free_node(node);
}

API void
lyd_free(struct lyd_node *node)
{
    lyd_free_internal_r(node, 1);
}

static void
lyd_free_withsiblings_r(struct lyd_node *first)
{
    struct lyd_node *next, *node;

    LY_TREE_FOR_SAFE(first, next, node) {
        if (node->schema->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_RPC | LYS_ACTION | LYS_NOTIF)) {
            lyd_free_withsiblings_r(node->child);
        }
        _lyd_free_node(node);
    }
}

API void
lyd_free_withsiblings(struct lyd_node *node)
{
    struct lyd_node *iter, *aux;

    if (!node) {
        return;
    }

    if (node->parent) {
        /* optimization - avoid freeing (unlinking) the last node of the siblings list */
        /* so, first, free the node's predecessors to the beginning of the list ... */
        for(iter = node->prev; iter->next; iter = aux) {
            aux = iter->prev;
            lyd_free(iter);
        }
        /* ... then, the node is the first in the siblings list, so free them all */
        LY_TREE_FOR_SAFE(node, aux, iter) {
            lyd_free(iter);
        }
    } else {
        /* node is top-level so we are freeing the whole data tree, we can just free nodes without any unlinking */
        while (node->prev->next) {
            /* find the first sibling */
            node = node->prev;
        }

        /* free it all */
        lyd_free_withsiblings_r(node);
    }
}

/**
 * Expectations:
 * - list exists in data tree
 * - the leaf (defined by the unique_expr) is not instantiated under the list
 */
int
lyd_get_unique_default(const char* unique_expr, struct lyd_node *list, const char **dflt)
{
    struct ly_ctx *ctx = list->schema->module->ctx;
    const struct lys_node *parent;
    const struct lys_node_leaf *sleaf = NULL;
    struct lys_tpdf *tpdf;
    struct lyd_node *last, *node;
    struct ly_set *s, *r;
    unsigned int i;
    enum int_log_opts prev_ilo;

    assert(unique_expr && list && dflt);
    *dflt = NULL;

    if (resolve_descendant_schema_nodeid(unique_expr, list->schema->child, LYS_LEAF, 1, &parent) || !parent) {
        /* error, but unique expression was checked when the schema was parsed so this should not happened */
        LOGINT(ctx);
        return -1;
    }

    sleaf = (struct lys_node_leaf *)parent;
    if (sleaf->dflt) {
        /* leaf has a default value */
        *dflt = sleaf->dflt;
    } else if (!(sleaf->flags & LYS_MAND_TRUE)) {
        /* get the default value from the type */
        for (tpdf = sleaf->type.der; tpdf && !(*dflt); tpdf = tpdf->type.der) {
            *dflt = tpdf->dflt;
        }
    }

    if (!(*dflt)) {
        return 0;
    }

    /* it has default value, but check if it can appear in the data tree under the list */
    s = ly_set_new();
    for (parent = lys_parent((struct lys_node *)sleaf); parent != list->schema; parent = lys_parent(parent)) {
        if (!(parent->nodetype & (LYS_CONTAINER | LYS_CASE | LYS_CHOICE | LYS_USES))) {
            /* This should be already detected when parsing schema */
            LOGINT(ctx);
            ly_set_free(s);
            return -1;
        }
        ly_set_add(s, (void *)parent, LY_SET_OPT_USEASLIST);
    }

    ly_ilo_change(NULL, ILO_IGNORE, &prev_ilo, NULL);
    for (i = 0, last = list; i < s->number; i++) {
        parent = s->set.s[i]; /* shortcut */

        switch (parent->nodetype) {
        case LYS_CONTAINER:
            if (last) {
                /* find instance in the data */
                r = lyd_find_path(last, parent->name);
                if (!r || r->number > 1) {
                    ly_set_free(r);
                    *dflt = NULL;
                    goto end;
                }
                if (r->number) {
                    last = r->set.d[0];
                } else {
                    last = NULL;
                }
                ly_set_free(r);
            }
            if (((struct lys_node_container *)parent)->presence) {
                /* not-instantiated presence container on path */
                *dflt = NULL;
                goto end;
            }
            break;
        case LYS_CHOICE :
            /* check presence of another case */
            if (!last) {
                continue;
            }

            /* remember the case to be searched in choice by lyv_multicases() */
            if (i + 1 == s->number) {
                parent = (struct lys_node *)sleaf;
            } else if (s->set.s[i + 1]->nodetype == LYS_CASE && (i + 2 < s->number) &&
                    s->set.s[i + 2]->nodetype == LYS_CHOICE) {
                /* nested choices are covered by lyv_multicases, we just have to pass
                 * the lowest choice */
                i++;
                continue;
            } else {
                parent = s->set.s[i + 1];
            }
            node = last->child;
            if (lyv_multicases(NULL, (struct lys_node *)parent, &node, 0, NULL)) {
                /* another case is present */
                *dflt = NULL;
                goto end;
            }
            break;
        default:
            /* LYS_CASE, LYS_USES */
            continue;
        }
    }

end:
    ly_ilo_restore(NULL, prev_ilo, NULL, 0);
    ly_set_free(s);
    return 0;
}

API char *
lyd_path(const struct lyd_node *node)
{
    char *buf = NULL;

    if (!node) {
        LOGARG;
        return NULL;
    }

    if (ly_vlog_build_path(LY_VLOG_LYD, node, &buf, 0, 0)) {
        return NULL;
    }

    return buf;
}

int
lyd_build_relative_data_path(const struct lys_module *module, const struct lyd_node *node, const char *schema_id,
                             char *buf)
{
    const struct lys_node *snode, *schema;
    const char *mod_name, *name;
    int mod_name_len, name_len, len = 0;
    int r, is_relative = -1;

    assert(schema_id && buf);
    schema = node->schema;

    while (*schema_id) {
        if ((r = parse_schema_nodeid(schema_id, &mod_name, &mod_name_len, &name, &name_len, &is_relative, NULL, NULL, 0)) < 1) {
            LOGINT(module->ctx);
            return -1;
        }
        schema_id += r;

        snode = NULL;
        while ((snode = lys_getnext(snode, schema, NULL, LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHCASE | LYS_GETNEXT_NOSTATECHECK))) {
            r = schema_nodeid_siblingcheck(snode, module, mod_name, mod_name_len, name, name_len);
            if (r == 0) {
                schema = snode;
                break;
            } else if (r == 1) {
                continue;
            } else {
                return -1;
            }
        }
        /* no match */
        if (!snode || (!schema_id[0] && snode->nodetype != LYS_LEAF)) {
            LOGINT(module->ctx);
            return -1;
        }

        if (!(snode->nodetype & (LYS_CHOICE | LYS_CASE))) {
            len += sprintf(&buf[len], "%s%s", (len ? "/" : ""), snode->name);
        }
    }

    return len;
}

API struct ly_set *
lyd_find_path(const struct lyd_node *ctx_node, const char *path)
{
    struct lyxp_set xp_set;
    struct ly_set *set;
    char *yang_xpath;
    const char * node_mod_name, *mod_name, *name;
    int mod_name_len, name_len, is_relative = -1;
    uint32_t i;

    if (!ctx_node || !path) {
        LOGARG;
        return NULL;
    }

    if (parse_schema_nodeid(path, &mod_name, &mod_name_len, &name, &name_len, &is_relative, NULL, NULL, 1) > 0) {
        if (name[0] == '#' && !is_relative) {
            node_mod_name = lyd_node_module(ctx_node)->name;
            if (strncmp(mod_name, node_mod_name, mod_name_len) || node_mod_name[mod_name_len]) {
                return NULL;
            }
            path = name + name_len;
        }
    }

    /* transform JSON into YANG XPATH */
    yang_xpath = transform_json2xpath(lyd_node_module(ctx_node), path);
    if (!yang_xpath) {
        return NULL;
    }

    memset(&xp_set, 0, sizeof xp_set);

    if (lyxp_eval(yang_xpath, ctx_node, LYXP_NODE_ELEM, lyd_node_module(ctx_node), &xp_set, 0) != EXIT_SUCCESS) {
        free(yang_xpath);
        return NULL;
    }
    free(yang_xpath);

    set = ly_set_new();
    LY_CHECK_ERR_RETURN(!set, LOGMEM(ctx_node->schema->module->ctx), NULL);

    if (xp_set.type == LYXP_SET_NODE_SET) {
        for (i = 0; i < xp_set.used; ++i) {
            if (xp_set.val.nodes[i].type == LYXP_NODE_ELEM) {
                if (ly_set_add(set, xp_set.val.nodes[i].node, LY_SET_OPT_USEASLIST) < 0) {
                    ly_set_free(set);
                    set = NULL;
                    break;
                }
            }
        }
    }
    /* free xp_set content */
    lyxp_set_cast(&xp_set, LYXP_SET_EMPTY, ctx_node, NULL, 0);

    return set;
}

API struct ly_set *
lyd_find_instance(const struct lyd_node *data, const struct lys_node *schema)
{
    struct ly_set *ret, *ret_aux, *spath;
    const struct lys_node *siter;
    struct lyd_node *iter;
    unsigned int i, j;

    if (!data || !schema ||
            !(schema->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_ANYDATA | LYS_NOTIF | LYS_RPC | LYS_ACTION))) {
        LOGARG;
        return NULL;
    }

    ret = ly_set_new();
    spath = ly_set_new();
    if (!ret || !spath) {
        LOGMEM(schema->module->ctx);
        goto error;
    }

    /* find data root */
    while (data->parent) {
        /* vertical move (up) */
        data = data->parent;
    }
    while (data->prev->next) {
        /* horizontal move (left) */
        data = data->prev;
    }

    /* build schema path */
    for (siter = schema; siter; ) {
        if (siter->nodetype == LYS_AUGMENT) {
            siter = ((struct lys_node_augment *)siter)->target;
            continue;
        } else if (siter->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_ANYDATA | LYS_NOTIF | LYS_RPC | LYS_ACTION)) {
            /* standard data node */
            ly_set_add(spath, (void*)siter, LY_SET_OPT_USEASLIST);

        } /* else skip the rest node types */
        siter = siter->parent;
    }
    if (!spath->number) {
        /* no valid path */
        goto error;
    }

    /* start searching */
    LY_TREE_FOR((struct lyd_node *)data, iter) {
        if (iter->schema == spath->set.s[spath->number - 1]) {
            ly_set_add(ret, iter, LY_SET_OPT_USEASLIST);
        }
    }
    for (i = spath->number - 1; i; i--) {
        if (!ret->number) {
            /* nothing found */
            break;
        }

        ret_aux = ly_set_new();
        if (!ret_aux) {
            LOGMEM(schema->module->ctx);
            goto error;
        }
        for (j = 0; j < ret->number; j++) {
            LY_TREE_FOR(ret->set.d[j]->child, iter) {
                if (iter->schema == spath->set.s[i - 1]) {
                    ly_set_add(ret_aux, iter, LY_SET_OPT_USEASLIST);
                }
            }
        }
        ly_set_free(ret);
        ret = ret_aux;
    }

    ly_set_free(spath);
    return ret;

error:
    ly_set_free(ret);
    ly_set_free(spath);

    return NULL;
}

API struct lyd_node *
lyd_first_sibling(struct lyd_node *node)
{
    struct lyd_node *start;

    if (!node) {
        return NULL;
    }

    /* get the first sibling */
    if (node->parent) {
        start = node->parent->child;
    } else {
        for (start = node; start->prev->next; start = start->prev);
    }

    return start;
}

API struct ly_set *
ly_set_new(void)
{
    struct ly_set *new;

    new = calloc(1, sizeof(struct ly_set));
    LY_CHECK_ERR_RETURN(!new, LOGMEM(NULL), NULL);
    return new;
}

API void
ly_set_free(struct ly_set *set)
{
    if (!set) {
        return;
    }

    free(set->set.g);
    free(set);
}

API int
ly_set_contains(const struct ly_set *set, void *node)
{
    unsigned int i;

    if (!set) {
        return -1;
    }

    for (i = 0; i < set->number; i++) {
        if (set->set.g[i] == node) {
            /* object found */
            return i;
        }
    }

    /* object not found */
    return -1;
}

API struct ly_set *
ly_set_dup(const struct ly_set *set)
{
    struct ly_set *new;

    if (!set) {
        return NULL;
    }

    new = malloc(sizeof *new);
    LY_CHECK_ERR_RETURN(!new, LOGMEM(NULL), NULL);
    new->number = set->number;
    new->size = set->size;
    new->set.g = malloc(new->size * sizeof *(new->set.g));
    LY_CHECK_ERR_RETURN(!new->set.g, LOGMEM(NULL); free(new), NULL);
    memcpy(new->set.g, set->set.g, new->size * sizeof *(new->set.g));

    return new;
}

API int
ly_set_add(struct ly_set *set, void *node, int options)
{
    unsigned int i;
    void **new;

    if (!set || !node) {
        LOGARG;
        return -1;
    }

    if (!(options & LY_SET_OPT_USEASLIST)) {
        /* search for duplication */
        for (i = 0; i < set->number; i++) {
            if (set->set.g[i] == node) {
                /* already in set */
                return i;
            }
        }
    }

    if (set->size == set->number) {
        new = realloc(set->set.g, (set->size + 8) * sizeof *(set->set.g));
        LY_CHECK_ERR_RETURN(!new, LOGMEM(NULL), -1);
        set->size += 8;
        set->set.g = new;
    }

    set->set.g[set->number++] = node;

    return set->number - 1;
}

API int
ly_set_merge(struct ly_set *trg, struct ly_set *src, int options)
{
    unsigned int i, ret;
    void **new;

    if (!trg) {
        LOGARG;
        return -1;
    }

    if (!src) {
        return 0;
    }

    if (!(options & LY_SET_OPT_USEASLIST)) {
        /* remove duplicates */
        i = 0;
        while (i < src->number) {
            if (ly_set_contains(trg, src->set.g[i]) > -1) {
                ly_set_rm_index(src, i);
            } else {
                ++i;
            }
        }
    }

    /* allocate more memory if needed */
    if (trg->size < trg->number + src->number) {
        new = realloc(trg->set.g, (trg->number + src->number) * sizeof *(trg->set.g));
        LY_CHECK_ERR_RETURN(!new, LOGMEM(NULL), -1);
        trg->size = trg->number + src->number;
        trg->set.g = new;
    }

    /* copy contents from src into trg */
    memcpy(trg->set.g + trg->number, src->set.g, src->number * sizeof *(src->set.g));
    ret = src->number;
    trg->number += ret;

    /* cleanup */
    ly_set_free(src);
    return ret;
}

API int
ly_set_rm_index(struct ly_set *set, unsigned int index)
{
    if (!set || (index + 1) > set->number) {
        LOGARG;
        return EXIT_FAILURE;
    }

    if (index == set->number - 1) {
        /* removing last item in set */
        set->set.g[index] = NULL;
    } else {
        /* removing item somewhere in a middle, so put there the last item */
        set->set.g[index] = set->set.g[set->number - 1];
        set->set.g[set->number - 1] = NULL;
    }
    set->number--;

    return EXIT_SUCCESS;
}

API int
ly_set_rm(struct ly_set *set, void *node)
{
    unsigned int i;

    if (!set || !node) {
        LOGARG;
        return EXIT_FAILURE;
    }

    /* get index */
    for (i = 0; i < set->number; i++) {
        if (set->set.g[i] == node) {
            break;
        }
    }
    if (i == set->number) {
        /* node is not in set */
        LOGARG;
        return EXIT_FAILURE;
    }

    return ly_set_rm_index(set, i);
}

API int
ly_set_clean(struct ly_set *set)
{
    if (!set) {
        return EXIT_FAILURE;
    }

    set->number = 0;
    return EXIT_SUCCESS;
}

API int
lyd_wd_default(struct lyd_node_leaf_list *node)
{
    struct lys_node_leaf *leaf;
    struct lys_node_leaflist *llist;
    struct lyd_node *iter;
    struct lys_tpdf *tpdf;
    const char *dflt = NULL, **dflts = NULL;
    uint8_t dflts_size = 0, c, i;

    if (!node || !(node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        return 0;
    }

    if (node->dflt) {
        return 1;
    }

    if (node->schema->nodetype == LYS_LEAF) {
        leaf = (struct lys_node_leaf *)node->schema;

        /* get know if there is a default value */
        if (leaf->dflt) {
            /* leaf has a default value */
            dflt = leaf->dflt;
        } else if (!(leaf->flags & LYS_MAND_TRUE)) {
            /* get the default value from the type */
            for (tpdf = leaf->type.der; tpdf && !dflt; tpdf = tpdf->type.der) {
                dflt = tpdf->dflt;
            }
        }
        if (!dflt) {
            /* no default value */
            return 0;
        }

        /* compare the default value with the value of the leaf */
        if (!ly_strequal(dflt, node->value_str, 1)) {
            return 0;
        }
    } else if (node->schema->module->version >= LYS_VERSION_1_1) { /* LYS_LEAFLIST */
        llist = (struct lys_node_leaflist *)node->schema;

        /* get know if there is a default value */
        if (llist->dflt_size) {
            /* there are default values */
            dflts_size = llist->dflt_size;
            dflts = llist->dflt;
        } else if (!llist->min) {
            /* get the default value from the type */
            for (tpdf = llist->type.der; tpdf && !dflts; tpdf = tpdf->type.der) {
                if (tpdf->dflt) {
                    dflts = &tpdf->dflt;
                    dflts_size = 1;
                    break;
                }
            }
        }

        if (!dflts_size) {
            /* no default values to use */
            return 0;
        }

        /* compare the default value with the value of the leaf */
        /* first, find the first leaf-list's sibling */
        iter = (struct lyd_node *)node;
        if (iter->parent) {
            iter = iter->parent->child;
        } else {
            for (; iter->prev->next; iter = iter->prev);
        }
        for (c = 0; iter; iter = iter->next) {
            if (iter->schema != node->schema) {
                continue;
            }
            if (c == dflts_size) {
                /* to many leaf-list instances */
                return 0;
            }

            if (llist->flags & LYS_USERORDERED) {
                /* we have strict order */
                if (!ly_strequal(dflts[c], ((struct lyd_node_leaf_list *)iter)->value_str, 1)) {
                    return 0;
                }
            } else {
                /* node's value is supposed to match with one of the default values */
                for (i = 0; i < dflts_size; i++) {
                    if (ly_strequal(dflts[i], ((struct lyd_node_leaf_list *)iter)->value_str, 1)) {
                        break;
                    }
                }
                if (i == dflts_size) {
                    /* values do not match */
                    return 0;
                }
            }
            c++;
        }
        if (c != dflts_size) {
            /* different sets of leaf-list instances */
            return 0;
        }
    } else {
        return 0;
    }

    /* all checks ok */
    return 1;
}

int
unres_data_diff_new(struct unres_data *unres, struct lyd_node *subtree, struct lyd_node *parent, int created)
{
    char *parent_xpath = NULL;

    if (created) {
        return lyd_difflist_add(unres->diff, &unres->diff_size, unres->diff_idx++, LYD_DIFF_CREATED, NULL, subtree);
    } else {
        if (parent) {
            parent_xpath = lyd_path(parent);
            LY_CHECK_ERR_RETURN(!parent_xpath, LOGMEM(lyd_node_module(subtree)->ctx), -1);
        }
        return lyd_difflist_add(unres->diff, &unres->diff_size, unres->diff_idx++, LYD_DIFF_DELETED,
                                subtree, (struct lyd_node *)parent_xpath);
    }
}

void
unres_data_diff_rem(struct unres_data *unres, unsigned int idx)
{
    if (unres->diff->type[idx] == LYD_DIFF_DELETED) {
        lyd_free_withsiblings(unres->diff->first[idx]);
        free(unres->diff->second[idx]);
    }

    /* replace by last real value */
    if (idx < unres->diff_idx - 1) {
        unres->diff->type[idx] = unres->diff->type[unres->diff_idx - 1];
        unres->diff->first[idx] = unres->diff->first[unres->diff_idx - 1];
        unres->diff->second[idx] = unres->diff->second[unres->diff_idx - 1];
    }

    /* move the end */
    assert(unres->diff->type[unres->diff_idx] == LYD_DIFF_END);
    unres->diff->type[unres->diff_idx - 1] = unres->diff->type[unres->diff_idx];
    --unres->diff_idx;
}

API void
lyd_free_val_diff(struct lyd_difflist *diff)
{
    uint32_t i;

    if (!diff) {
        return;
    }

    for (i = 0; diff->type[i] != LYD_DIFF_END; ++i) {
        switch (diff->type[i]) {
        case LYD_DIFF_CREATED:
            free(diff->first[i]);
            lyd_free_withsiblings(diff->second[i]);
            break;
        case LYD_DIFF_DELETED:
            lyd_free_withsiblings(diff->first[i]);
            free(diff->second[i]);
            break;
        default:
            /* what to do? */
            break;
        }
    }

    lyd_free_diff(diff);
}

static int
lyd_wd_add_leaf(struct lyd_node **tree, struct lyd_node *last_parent, struct lys_node_leaf *leaf, struct unres_data *unres,
                int check_when_must)
{
    struct lyd_node *dummy = NULL, *current;
    struct lys_tpdf *tpdf;
    const char *dflt = NULL;
    int ret;

    /* get know if there is a default value */
    if (leaf->dflt) {
        /* leaf has a default value */
        dflt = leaf->dflt;
    } else if (!(leaf->flags & LYS_MAND_TRUE)) {
        /* get the default value from the type */
        for (tpdf = leaf->type.der; tpdf && !dflt; tpdf = tpdf->type.der) {
            dflt = tpdf->dflt;
        }
    }
    if (!dflt) {
        /* no default value */
        return EXIT_SUCCESS;
    }

    /* create the node */
    if (!(dummy = lyd_new_dummy(*tree, last_parent, (struct lys_node*)leaf, dflt, 1))) {
        goto error;
    }

    if (unres->store_diff) {
        /* remember this subtree in the diff */
        if (unres_data_diff_new(unres, dummy, NULL, 1)) {
            goto error;
        }
    }

    if (!dummy->parent && (*tree)) {
        /* connect dummy nodes into the data tree (at the end of top level nodes) */
        if (lyd_insert_sibling(tree, dummy)) {
            goto error;
        }
    }
    for (current = dummy; ; current = current->child) {
        /* remember the created data in unres */
        if (check_when_must) {
            if ((current->when_status & LYD_WHEN) && unres_data_add(unres, current, UNRES_WHEN) == -1) {
                goto error;
            }
            if (check_when_must == 2) {
                ret = resolve_applies_must(current);
                if ((ret & 0x1) && (unres_data_add(unres, current, UNRES_MUST) == -1)) {
                    goto error;
                }
                if ((ret & 0x2) && (unres_data_add(unres, current, UNRES_MUST_INOUT) == -1)) {
                    goto error;
                }
            }
        }

        /* clear dummy-node flag */
        current->validity &= ~LYD_VAL_INUSE;

        if (current->schema == (struct lys_node *)leaf) {
            break;
        }
    }
    /* update parent's default flag if needed */
    lyd_wd_update_parents(dummy);

    /* if necessary, remember the created data value in unres */
    if (((struct lyd_node_leaf_list *)current)->value_type == LY_TYPE_LEAFREF) {
        if (unres_data_add(unres, current, UNRES_LEAFREF)) {
            goto error;
        }
    } else if (((struct lyd_node_leaf_list *)current)->value_type == LY_TYPE_INST) {
        if (unres_data_add(unres, current, UNRES_INSTID)) {
            goto error;
        }
    }

    if (!(*tree)) {
        *tree = dummy;
    }
    return EXIT_SUCCESS;

error:
    lyd_free(dummy);
    return EXIT_FAILURE;
}

static int
lyd_wd_add_leaflist(struct lyd_node **tree, struct lyd_node *last_parent, struct lys_node_leaflist *llist,
                    struct unres_data *unres, int check_when_must)
{
    struct lyd_node *dummy, *current, *first = NULL;
    struct lys_tpdf *tpdf;
    const char **dflt = NULL;
    uint8_t dflt_size = 0;
    int i, ret;

    if (llist->module->version < LYS_VERSION_1_1) {
        /* default values on leaf-lists are allowed from YANG 1.1 */
        return EXIT_SUCCESS;
    }

    /* get know if there is a default value */
    if (llist->dflt_size) {
        /* there are default values */
        dflt_size = llist->dflt_size;
        dflt = llist->dflt;
    } else if (!llist->min) {
        /* get the default value from the type */
        for (tpdf = llist->type.der; tpdf && !dflt; tpdf = tpdf->type.der) {
            if (tpdf->dflt) {
                dflt = &tpdf->dflt;
                dflt_size = 1;
                break;
            }
        }
    }

    if (!dflt_size) {
        /* no default values to use */
        return EXIT_SUCCESS;
    }

    for (i = 0; i < dflt_size; i++) {
        /* create the node */
        if (!(dummy = lyd_new_dummy(*tree, last_parent, (struct lys_node*)llist, dflt[i], 1))) {
            goto error;
        }

        if (unres->store_diff) {
            /* remember this subtree in the diff */
            if (unres_data_diff_new(unres, dummy, NULL, 1)) {
                goto error;
            }
        }

        if (!first) {
            first = dummy;
        } else if (!dummy->parent) {
            /* interconnect with the rest of leaf-lists */
            first->prev->next = dummy;
            dummy->prev = first->prev;
            first->prev = dummy;
        }

        for (current = dummy; ; current = current->child) {
            /* remember the created data in unres */
            if (check_when_must) {
                if ((current->when_status & LYD_WHEN) && unres_data_add(unres, current, UNRES_WHEN) == -1) {
                    goto error;
                }
                if (check_when_must == 2) {
                    ret = resolve_applies_must(current);
                    if ((ret & 0x1) && (unres_data_add(unres, current, UNRES_MUST) == -1)) {
                        goto error;
                    }
                    if ((ret & 0x2) && (unres_data_add(unres, current, UNRES_MUST_INOUT) == -1)) {
                        goto error;
                    }
                }
            }

            /* clear dummy-node flag */
            current->validity &= ~LYD_VAL_INUSE;

            if (current->schema == (struct lys_node *)llist) {
                break;
            }
        }

        /* if necessary, remember the created data value in unres */
        if (((struct lyd_node_leaf_list *)current)->value_type == LY_TYPE_LEAFREF) {
            if (unres_data_add(unres, current, UNRES_LEAFREF)) {
                goto error;
            }
        } else if (((struct lyd_node_leaf_list *)current)->value_type == LY_TYPE_INST) {
            if (unres_data_add(unres, current, UNRES_INSTID)) {
                goto error;
            }
        }
    }

    /* insert into the tree */
    if (first && !first->parent && (*tree)) {
        /* connect dummy nodes into the data tree (at the end of top level nodes) */
        if (lyd_insert_sibling(tree, first)) {
            goto error;
        }
    } else if (!(*tree)) {
        *tree = first;
    }

    /* update parent's default flag if needed */
    lyd_wd_update_parents(first);

    return EXIT_SUCCESS;

error:
    lyd_free_withsiblings(first);
    return EXIT_FAILURE;
}

static void
lyd_wd_leaflist_cleanup(struct ly_set *set, struct unres_data *unres)
{
    unsigned int i;

    assert(set);

    /* if there is an instance without the dflt flag, we have to
     * remove all instances with the flag - an instance could be
     * explicitely added, so the default leaflists were invalidated */
    for (i = 0; i < set->number; i++) {
        if (!set->set.d[i]->dflt) {
            break;
        }
    }
    if (i < set->number) {
        for (i = 0; i < set->number; i++) {
            if (set->set.d[i]->dflt) {
                /* remove this default instance */
                if (unres->store_diff) {
                    /* just move it to diff if is being generated */
                    unres_data_diff_new(unres, set->set.d[i], set->set.d[i]->parent, 0);
                    lyd_unlink(set->set.d[i]);
                } else {
                    lyd_free(set->set.d[i]);
                }
            }
        }
    }
}

/**
 * @brief Process (add/clean flags) default nodes in the schema subtree
 *
 * @param[in,out] root Pointer to the root node of the complete data tree, the root node can be NULL if the data tree
 *                     is empty
 * @param[in] last_parent The closest parent in the data tree to the currently processed \p schema node
 * @param[in] subroot  The root node of a data subtree, the node is instance of the \p schema node, NULL in case the
 *                     schema node is not instantiated in the data tree
 * @param[in] schema The schema node to be processed
 * @param[in] toplevel Flag for processing top level schema nodes when \p last_parent and \p subroot are consider as
 *                     unknown
 * @param[in] options  Parser options to know the data tree type, see @ref parseroptions.
 * @param[in] unres    Unresolved data list, the newly added default nodes may need to add some unresolved items
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
static int
lyd_wd_add_subtree(struct lyd_node **root, struct lyd_node *last_parent, struct lyd_node *subroot,
                   struct lys_node *schema, int toplevel, int options, struct unres_data *unres)
{
    struct ly_set *present = NULL;
    struct lys_node *siter, *siter_prev;
    struct lyd_node *iter;
    int i, check_when_must, storing_diff = 0;

    assert(root);

    if ((options & LYD_OPT_TYPEMASK) && (schema->flags & LYS_CONFIG_R)) {
        /* non LYD_OPT_DATA tree, status data are not expected here */
        return EXIT_SUCCESS;
    }

    if (options & (LYD_OPT_NOTIF_FILTER | LYD_OPT_EDIT | LYD_OPT_GET | LYD_OPT_GETCONFIG)) {
        check_when_must = 0; /* check neither */
    } else if (options & LYD_OPT_TRUSTED) {
        check_when_must = 1; /* check only when */
    } else {
        check_when_must = 2; /* check both when and must */
    }

    if (toplevel && (schema->nodetype & (LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_CONTAINER))) {
        /* search for the schema node instance */
        present = ly_set_new();
        if (!present) {
            goto error;
        }
        if ((*root) && lyd_get_node_siblings(*root, schema, present)) {
            /* there are some instances */
            for (i = 0; i < (signed)present->number; i++) {
                if (schema->nodetype & LYS_LEAFLIST) {
                    lyd_wd_leaflist_cleanup(present, unres);
                } else if (schema->nodetype != LYS_LEAF) {
                    if (lyd_wd_add_subtree(root, present->set.d[i], present->set.d[i], schema, 0, options, unres)) {
                        goto error;
                    }
                } /* else LYS_LEAF - nothing to do */
            }
        } else {
            /* no instance */
            if (lyd_wd_add_subtree(root, last_parent, NULL, schema, 0, options, unres)) {
                goto error;
            }
        }

        ly_set_free(present);
        return EXIT_SUCCESS;
    }

    /* skip disabled parts of schema */
    if (!subroot) {
        /* go through all the uses and check whether they are enabled */
        for (siter = schema->parent; siter && (siter->nodetype & (LYS_USES | LYS_CHOICE)); siter = siter->parent) {
            if (lys_is_disabled(siter, 0)) {
                /* ignore disabled uses nodes */
                return EXIT_SUCCESS;
            }
        }

        /* check augment state */
        if (siter && siter->nodetype == LYS_AUGMENT) {
            if (lys_is_disabled(siter, 0)) {
                /* ignore disabled augment */
                return EXIT_SUCCESS;
            }
        }

        /* check the node itself */
        if (lys_is_disabled(schema, 0)) {
            /* ignore disabled data */
            return EXIT_SUCCESS;
        }
    }

    /* go recursively */
    switch (schema->nodetype) {
    case LYS_LIST:
        if (!subroot) {
            /* stop recursion */
            break;
        }
        /* falls through */
    case LYS_CONTAINER:
        if (!subroot) {
            /* container does not exists, continue only in case of non presence container */
            if (((struct lys_node_container *)schema)->presence) {
                /* stop recursion */
                break;
            }
            /* always create empty NP container even if there is no default node,
             * because accroding to RFC, the empty NP container is always part of
             * accessible tree (e.g. for evaluating when and must conditions) */
            subroot = _lyd_new(last_parent, schema, 1);
            /* useless to set mand flag */
            subroot->validity &= ~LYD_VAL_MAND;

            if (unres->store_diff) {
                /* remember this container in the diff */
                if (unres_data_diff_new(unres, subroot, NULL, 1)) {
                    goto error;
                }

                /* do not store diff for recursive calls, created values will be connected to this one */
                storing_diff = 1;
                unres->store_diff = 0;
            }

            if (!last_parent) {
                if (*root) {
                    lyd_insert_common((*root)->parent, root, subroot, 0);
                } else {
                    *root = subroot;
                }
            }
            last_parent = subroot;

            /* remember the created container in unres */
            if (check_when_must) {
                if ((subroot->when_status & LYD_WHEN) && unres_data_add(unres, subroot, UNRES_WHEN) == -1) {
                    goto error;
                }
                if (check_when_must == 2) {
                    i = resolve_applies_must(subroot);
                    if ((i & 0x1) && (unres_data_add(unres, subroot, UNRES_MUST) == -1)) {
                        goto error;
                    }
                    if ((i & 0x2) && (unres_data_add(unres, subroot, UNRES_MUST_INOUT) == -1)) {
                        goto error;
                    }
                }
            }
        } else if (!((struct lys_node_container *)schema)->presence) {
            /* fix default flag on existing containers - set it on all non-presence containers and in case we will
             * have in recursion function some non-default node, it will unset it */
            subroot->dflt = 1;
        }
        /* falls through */
    case LYS_CASE:
    case LYS_USES:
    case LYS_INPUT:
    case LYS_OUTPUT:
    case LYS_NOTIF:

        /* recursion */
        present = ly_set_new();
        if (!present) {
            goto error;
        }
        LY_TREE_FOR(schema->child, siter) {
            if (siter->nodetype & (LYS_CHOICE | LYS_USES)) {
                /* go into without searching for data instance */
                if (lyd_wd_add_subtree(root, last_parent, subroot, siter, toplevel, options, unres)) {
                    goto error;
                }
            } else if (siter->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA)) {
                /* search for the schema node instance */
                if (subroot && lyd_get_node_siblings(subroot->child, siter, present)) {
                    /* there are some instances in the data root */
                    if (siter->nodetype & LYS_LEAFLIST) {
                        /* already have some leaflists, check that they are all
                         * default, if not, remove the default leaflists */
                        lyd_wd_leaflist_cleanup(present, unres);
                    } else if (siter->nodetype != LYS_LEAF) {
                        /* recursion */
                        for (i = 0; i < (signed)present->number; i++) {
                            if (lyd_wd_add_subtree(root, present->set.d[i], present->set.d[i], siter, toplevel, options,
                                                   unres)) {
                                goto error;
                            }
                        }
                    } /* else LYS_LEAF - nothing to do */

                    /* fix default flag (2nd part) - for non-default node with default parent, unset the default flag
                     * from the parents (starting from subroot node) */
                    if (subroot->dflt) {
                        for (i = 0; i < (signed)present->number; i++) {
                            if (!present->set.d[i]->dflt) {
                                for (iter = subroot; iter && iter->dflt; iter = iter->parent) {
                                    iter->dflt = 0;
                                }
                                break;
                            }
                        }
                    }
                    ly_set_clean(present);
                } else {
                    /* no instance */
                    if (lyd_wd_add_subtree(root, last_parent, NULL, siter, toplevel, options, unres)) {
                        goto error;
                    }
                }
            }
        }

        if (storing_diff) {
            /* continue generating the diff in functions above this one */
            unres->store_diff = 1;
        }
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
        if (subroot) {
            /* default shortcase of a choice */
            present = ly_set_new();
            if (!present) {
                goto error;
            }
            lyd_get_node_siblings(subroot->child, schema, present);
            if (present->number) {
                /* the shortcase leaf(-list) exists, stop the processing */
                break;
            }
        }
        if (schema->nodetype == LYS_LEAF) {
            if (lyd_wd_add_leaf(root, last_parent, (struct lys_node_leaf*)schema, unres, check_when_must)) {
                return EXIT_FAILURE;
            }
        } else { /* LYS_LEAFLIST */
            if (lyd_wd_add_leaflist(root, last_parent, (struct lys_node_leaflist*)schema, unres, check_when_must)) {
                goto error;
            }
        }
        break;
    case LYS_CHOICE:
        /* get existing node in the data root from the choice */
        iter = NULL;
        if ((toplevel && (*root)) || (!toplevel && subroot)) {
            LY_TREE_FOR(toplevel ? (*root) : subroot->child, iter) {
                for (siter = lys_parent(iter->schema), siter_prev = iter->schema;
                        siter && (siter->nodetype & (LYS_CASE | LYS_USES | LYS_CHOICE));
                        siter_prev = siter, siter = lys_parent(siter)) {
                    if (siter == schema) {
                        /* we have the choice instance */
                        break;
                    }
                }
                if (siter == schema) {
                    /* we have the choice instance;
                     * the condition must be the same as in the loop because of
                     * choice's sibling nodes that break the loop, so siter is not NULL,
                     * but it is not the same as schema */
                    break;
                }
            }
        }
        if (!iter) {
            if (((struct lys_node_choice *)schema)->dflt) {
                /* there is a default case */
                if (lyd_wd_add_subtree(root, last_parent, subroot, ((struct lys_node_choice *)schema)->dflt,
                                       toplevel, options, unres)) {
                    goto error;
                }
            }
        } else {
            /* one of the choice's cases is instantiated, continue into this case */
            /* since iter != NULL, siter must be also != NULL and we also know siter_prev
             * which points to the child of schema leading towards the instantiated data */
            assert(siter && siter_prev);
            if (lyd_wd_add_subtree(root, last_parent, subroot, siter_prev, toplevel, options, unres)) {
                goto error;
            }
        }
        break;
    default:
        /* LYS_ANYXML, LYS_ANYDATA, LYS_USES, LYS_GROUPING - do nothing */
        break;
    }

    ly_set_free(present);
    return EXIT_SUCCESS;

error:
    ly_set_free(present);
    return EXIT_FAILURE;
}

/**
 * @brief Covering function to process (add/clean) default nodes in the data tree
 * @param[in,out] root Pointer to the root node of the complete data tree, the root node can be NULL if the data tree
 *                     is empty
 * @param[in] ctx      Context for the case the data tree is empty (in that case \p ctx must not be NULL)
 * @param[in] options  Parser options to know the data tree type, see @ref parseroptions.
 * @param[in] unres    Unresolved data list, the newly added default nodes may need to add some unresolved items
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
static int
lyd_wd_add(struct lyd_node **root, struct ly_ctx *ctx, const struct lys_module **modules, int mod_count,
           struct unres_data *unres, int options)
{
    struct lys_node *siter;
    int i;

    assert(root && !(options & LYD_OPT_ACT_NOTIF));
    assert(*root || ctx);
    assert(!(options & LYD_OPT_NOSIBLINGS) || *root);

    if (options & (LYD_OPT_EDIT | LYD_OPT_GET | LYD_OPT_GETCONFIG)) {
        /* no change supposed */
        return EXIT_SUCCESS;
    }

    if (!ctx) {
        ctx = (*root)->schema->module->ctx;
    }

    if (!(options & LYD_OPT_TYPEMASK) || (options & (LYD_OPT_DATA | LYD_OPT_CONFIG))) {
        if (options & LYD_OPT_NOSIBLINGS) {
            if (lyd_wd_add_subtree(root, NULL, NULL, (*root)->schema, 1, options, unres)) {
                return EXIT_FAILURE;
            }
        } else if (modules && mod_count) {
            for (i = 0; i < mod_count; ++i) {
                LY_TREE_FOR(modules[i]->data, siter) {
                    if (!(siter->nodetype & (LYS_CONTAINER | LYS_CHOICE | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA |
                                             LYS_USES))) {
                        continue;
                    }
                    if (lyd_wd_add_subtree(root, NULL, NULL, siter, 1, options, unres)) {
                        return EXIT_FAILURE;
                    }
                }
            }
        } else {
            for (i = 0; i < ctx->models.used; i++) {
                /* skip not implemented and disabled modules */
                if (!ctx->models.list[i]->implemented || ctx->models.list[i]->disabled) {
                    continue;
                }
                LY_TREE_FOR(ctx->models.list[i]->data, siter) {
                    if (!(siter->nodetype & (LYS_CONTAINER | LYS_CHOICE | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA |
                                             LYS_USES))) {
                        continue;
                    }
                    if (lyd_wd_add_subtree(root, NULL, NULL, siter, 1, options, unres)) {
                        return EXIT_FAILURE;
                    }
                }
            }
        }
    } else if (options & LYD_OPT_NOTIF) {
        if (!(*root) || ((*root)->schema->nodetype != LYS_NOTIF)) {
            LOGERR(ctx, LY_EINVAL, "Subtree is not a single notification.");
            return EXIT_FAILURE;
        }
        if (lyd_wd_add_subtree(root, *root, *root, (*root)->schema, 0, options, unres)) {
            return EXIT_FAILURE;
        }
    } else if (options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY)) {
        if (!(*root) || !((*root)->schema->nodetype & (LYS_RPC | LYS_ACTION))) {
            LOGERR(ctx, LY_EINVAL, "Subtree is not a single RPC/action/reply.");
            return EXIT_FAILURE;
        }
        if (options & LYD_OPT_RPC) {
            for (siter = (*root)->schema->child; siter && siter->nodetype != LYS_INPUT; siter = siter->next);
        } else { /* LYD_OPT_RPCREPLY */
            for (siter = (*root)->schema->child; siter && siter->nodetype != LYS_OUTPUT; siter = siter->next);
        }
        if (siter) {
            if (lyd_wd_add_subtree(root, *root, *root, siter, 0, options, unres)) {
                return EXIT_FAILURE;
            }
        }
    } else if (options & LYD_OPT_DATA_TEMPLATE) {
        if (lyd_wd_add_subtree(root, NULL, NULL, (*root)->schema, 1, options, unres)) {
            return EXIT_FAILURE;
        }
    } else {
        LOGINT(ctx);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int
lyd_defaults_add_unres(struct lyd_node **root, int options, struct ly_ctx *ctx, const struct lys_module **modules,
                       int mod_count, const struct lyd_node *data_tree, struct lyd_node *act_notif,
                       struct unres_data *unres, int wd)
{
    struct lyd_node *msg_sibling = NULL, *msg_parent = NULL, *data_tree_sibling, *data_tree_parent;
    struct lys_node *msg_op = NULL;
    struct ly_set *set;
    int ret = EXIT_FAILURE;

    assert(root && (*root || ctx) && unres && !(options & LYD_OPT_ACT_NOTIF));

    if (!ctx) {
        ctx = (*root)->schema->module->ctx;
    }

    if ((options & LYD_OPT_NOSIBLINGS) && !(*root)) {
        LOGERR(ctx, LY_EINVAL, "Cannot add default values for one module (LYD_OPT_NOSIBLINGS) without any data.");
        return EXIT_FAILURE;
    }

    if (options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY | LYD_OPT_NOTIF)) {
        if (!(*root)) {
            LOGERR(ctx, LY_EINVAL, "Cannot add default values to RPC, RPC reply, and notification without at least the empty container.");
            return EXIT_FAILURE;
        }
        if ((options & LYD_OPT_RPC) && !act_notif && ((*root)->schema->nodetype != LYS_RPC)) {
            LOGERR(ctx, LY_EINVAL, "Not valid RPC/action data.");
            return EXIT_FAILURE;
        }
        if ((options & LYD_OPT_RPCREPLY) && !act_notif && ((*root)->schema->nodetype != LYS_RPC)) {
            LOGERR(ctx, LY_EINVAL, "Not valid reply data.");
            return EXIT_FAILURE;
        }
        if ((options & LYD_OPT_NOTIF) && !act_notif && ((*root)->schema->nodetype != LYS_NOTIF)) {
            LOGERR(ctx, LY_EINVAL, "Not valid notification data.");
            return EXIT_FAILURE;
        }

        /* remember the operation/notification schema */
        msg_op = act_notif ? act_notif->schema : (*root)->schema;
    } else if (*root && (*root)->parent) {
        /* we have inner node, so it will be considered as
         * a root of subtree where to add default nodes and
         * no of its siblings will be affected */
        options |= LYD_OPT_NOSIBLINGS;
    }

    /* add missing default nodes */
    if (wd && lyd_wd_add((act_notif ? &act_notif : root), ctx, modules, mod_count, unres, options)) {
        return EXIT_FAILURE;
    }

    /* check leafrefs and/or instids if any */
    if (unres && unres->count) {
        if (!(*root)) {
            LOGINT(ctx);
            return EXIT_FAILURE;
        }

        /* temporarily link the additional data tree to the RPC/action/notification */
        if (data_tree && (options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY | LYD_OPT_NOTIF))) {
            /* duplicate the message tree - if it gets deleted we would not be able to positively identify it */
            msg_parent = NULL;
            msg_sibling = *root;

            if (act_notif) {
                /* fun case */
                data_tree_parent = NULL;
                data_tree_sibling = (struct lyd_node *)data_tree;
                while (data_tree_sibling) {
                    while (data_tree_sibling) {
                        if ((data_tree_sibling->schema == msg_sibling->schema)
                                && ((msg_sibling->schema->nodetype != LYS_LIST)
                                    || lyd_list_equal(data_tree_sibling, msg_sibling, 0))) {
                            /* match */
                            break;
                        }

                        data_tree_sibling = data_tree_sibling->next;
                    }

                    if (data_tree_sibling) {
                        /* prepare for the new data_tree iteration */
                        data_tree_parent = data_tree_sibling;
                        data_tree_sibling = data_tree_sibling->child;

                        /* find new action sibling to search for later (skip list keys) */
                        msg_parent = msg_sibling;
                        assert(msg_sibling->child);
                        for (msg_sibling = msg_sibling->child;
                                msg_sibling->schema->nodetype == LYS_LEAF;
                                msg_sibling = msg_sibling->next) {
                            assert(msg_sibling->next);
                        }
                        if (msg_sibling->schema->nodetype & (LYS_ACTION | LYS_NOTIF)) {
                            /* we are done */
                            assert(act_notif->parent);
                            assert(act_notif->parent->schema == data_tree_parent->schema);
                            assert(msg_sibling == act_notif);
                            break;
                        }
                    }
                }

                /* loop ended after the first iteration, set the values correctly */
                if (!data_tree_parent) {
                    data_tree_sibling = (struct lyd_node *)data_tree;
                }

            } else {
                /* easy case */
                data_tree_parent = NULL;
                data_tree_sibling = (struct lyd_node *)data_tree;
            }

            /* unlink msg_sibling if needed (won't do anything otherwise) */
            lyd_unlink_internal(msg_sibling, 0);

            /* now we can insert msg_sibling into data_tree_parent or next to data_tree_sibling */
            assert(data_tree_parent || data_tree_sibling);
            if (data_tree_parent) {
                if (lyd_insert_common(data_tree_parent, NULL, msg_sibling, 0)) {
                    goto unlink_datatree;
                }
            } else {
                assert(!data_tree_sibling->parent);
                if (lyd_insert_nextto(data_tree_sibling->prev, msg_sibling, 0, 0)) {
                    goto unlink_datatree;
                }
            }
        }

        if (resolve_unres_data(ctx, unres, root, options)) {
            goto unlink_datatree;
        }

        /* we are done */
        ret = EXIT_SUCCESS;

        /* check that the operation/notification tree was not removed */
        if (options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY | LYD_OPT_NOTIF)) {
            set = NULL;
            if (data_tree) {
                set = lyd_find_instance(data_tree_parent ? data_tree_parent : data_tree_sibling, msg_op);
                assert(set && ((set->number == 0) || (set->number == 1)));
            } else if (*root) {
                set = lyd_find_instance(*root, msg_op);
                assert(set && ((set->number == 0) || (set->number == 1)));
            }
            if (!set || !set->number) {
                /* it was removed, handle specially */
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_LYS, msg_op, "Operation/notification not supported because of the current configuration.");
                ret = EXIT_FAILURE;
            }
            ly_set_free(set);
        }

unlink_datatree:
        /* put the trees back in order */
        if (data_tree && (options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY | LYD_OPT_NOTIF))) {
            /* unlink and insert it back, if there is a parent  */
            lyd_unlink_internal(msg_sibling, 0);
            if (msg_parent) {
                lyd_insert_common(msg_parent, NULL, msg_sibling, 0);
            }
        }
    } else {
        /* we are done */
        ret = EXIT_SUCCESS;
    }

    return ret;
}

API struct lys_module *
lyd_node_module(const struct lyd_node *node)
{
    if (!node) {
        return NULL;
    }

    return node->schema->module->type ? ((struct lys_submodule *)node->schema->module)->belongsto : node->schema->module;
}

API double
lyd_dec64_to_double(const struct lyd_node *node)
{
    if (!node || !(node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))
            || (((struct lys_node_leaf *)node->schema)->type.base != LY_TYPE_DEC64)) {
        LOGARG;
        return 0;
    }

    return atof(((struct lyd_node_leaf_list *)node)->value_str);
}

API const struct lys_type *
lyd_leaf_type(const struct lyd_node_leaf_list *leaf)
{
    struct lys_type *type;

    if (!leaf || !(leaf->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        return NULL;
    }

    type = &((struct lys_node_leaf *)leaf->schema)->type;

    do {
        if (type->base == LY_TYPE_LEAFREF) {
            type = &type->info.lref.target->type;
        } else if (type->base == LY_TYPE_UNION) {
            if (type->info.uni.has_ptr_type && leaf->validity) {
                /* we don't know what it will be after resolution (validation) */
                LOGVAL(leaf->schema->module->ctx, LYE_SPEC, LY_VLOG_LYD, leaf,
                       "Unable to determine the type of value \"%s\" from union type \"%s\" prior to validation.",
                       leaf->value_str, type->der->name);
                return NULL;
            }

            if (resolve_union((struct lyd_node_leaf_list *)leaf, type, 0, 0, &type)) {
                /* resolve union failed */
                return NULL;
            }
        }
    } while (type->base == LY_TYPE_LEAFREF);

    return type;
}

API int
lyd_lyb_data_length(const char *data)
{
    const char *ptr;
    uint16_t i, mod_count, str_len;
    uint8_t tmp_buf[2];
    LYB_META meta;

    if (!data) {
        return -1;
    }

    ptr = data;

    /* magic number */
    if ((ptr[0] != 'l') || (ptr[1] != 'y') || (ptr[2] != 'b')) {
        return -1;
    }
    ptr += 3;

    /* header */
    ++ptr;

    /* models */
    memcpy(tmp_buf, ptr, 2);
    ptr += 2;
    mod_count = tmp_buf[0] | (tmp_buf[1] << 8);

    for (i = 0; i < mod_count; ++i) {
        /* model name */
        memcpy(tmp_buf, ptr, 2);
        ptr += 2;
        str_len = tmp_buf[0] | (tmp_buf[1] << 8);

        ptr += str_len;

        /* revision */
        ptr += 2;
    }

    if (ptr[0]) {
        /* subtrees */
        do {
            memcpy(&meta, ptr, LYB_META_BYTES);
            ptr += LYB_META_BYTES;

            /* read whole subtree (chunk size) */
            ptr += *((uint8_t *)&meta);
            /* skip inner chunks (inner chunk count) */
            ptr += *(((uint8_t *)&meta) + LYB_SIZE_BYTES) * LYB_META_BYTES;
        } while ((*((uint8_t *)&meta) == LYB_SIZE_MAX) || ptr[0]);
    }

    /* ending zero */
    ++ptr;

    return ptr - data;
}

#ifdef LY_ENABLED_LYD_PRIV

API void *
lyd_set_private(const struct lyd_node *node, void *priv)
{
    void *prev;

    if (!node) {
        LOGARG;
        return NULL;
    }

    prev = node->priv;
    ((struct lyd_node *)node)->priv = priv;

    return prev;
}

#endif
