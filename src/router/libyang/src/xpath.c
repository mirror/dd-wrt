/**
 * @file xpath.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief YANG XPath evaluation functions
 *
 * Copyright (c) 2015 - 2017 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

/* needed by libmath functions isfinite(), isinf(), isnan(), signbit(), ... */
#define _ISOC99_SOURCE

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include <math.h>
#include <pcre.h>

#include "xpath.h"
#include "libyang.h"
#include "xml_internal.h"
#include "tree_schema.h"
#include "tree_data.h"
#include "context.h"
#include "tree_internal.h"
#include "common.h"
#include "resolve.h"
#include "printer.h"
#include "parser.h"
#include "hash_table.h"

static const struct lyd_node *moveto_get_root(const struct lyd_node *cur_node, int options,
                                              enum lyxp_node_type *root_type);
static int reparse_or_expr(struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *exp_idx);
static int set_snode_insert_node(struct lyxp_set *set, const struct lys_node *node, enum lyxp_node_type node_type);
static int eval_expr_select(struct lyxp_expr *exp, uint16_t *exp_idx, enum lyxp_expr_type etype, struct lyd_node *cur_node,
                            struct lys_module *local_mod, struct lyxp_set *set, int options);

void
lyxp_expr_free(struct lyxp_expr *expr)
{
    uint16_t i;

    if (!expr) {
        return;
    }

    free(expr->expr);
    free(expr->tokens);
    free(expr->expr_pos);
    free(expr->tok_len);
    if (expr->repeat) {
        for (i = 0; i < expr->used; ++i) {
            free(expr->repeat[i]);
        }
    }
    free(expr->repeat);
    free(expr);
}

/**
 * @brief Print the type of an XPath \p set.
 *
 * @param[in] set Set to use.
 *
 * @return Set type string.
 */
static const char *
print_set_type(struct lyxp_set *set)
{
    switch (set->type) {
    case LYXP_SET_EMPTY:
        return "empty";
    case LYXP_SET_NODE_SET:
        return "node set";
    case LYXP_SET_SNODE_SET:
        return "schema node set";
    case LYXP_SET_BOOLEAN:
        return "boolean";
    case LYXP_SET_NUMBER:
        return "number";
    case LYXP_SET_STRING:
        return "string";
    }

    return NULL;
}

/**
 * @brief Print an XPath token \p tok type.
 *
 * @param[in] tok Token to use.
 *
 * @return Token type string.
 */
static const char *
print_token(enum lyxp_token tok)
{
    switch (tok) {
    case LYXP_TOKEN_PAR1:
        return "(";
    case LYXP_TOKEN_PAR2:
        return ")";
    case LYXP_TOKEN_BRACK1:
        return "[";
    case LYXP_TOKEN_BRACK2:
        return "]";
    case LYXP_TOKEN_DOT:
        return ".";
    case LYXP_TOKEN_DDOT:
        return "..";
    case LYXP_TOKEN_AT:
        return "@";
    case LYXP_TOKEN_COMMA:
        return ",";
    case LYXP_TOKEN_NAMETEST:
        return "NameTest";
    case LYXP_TOKEN_NODETYPE:
        return "NodeType";
    case LYXP_TOKEN_FUNCNAME:
        return "FunctionName";
    case LYXP_TOKEN_OPERATOR_LOG:
        return "Operator(Logic)";
    case LYXP_TOKEN_OPERATOR_COMP:
        return "Operator(Comparison)";
    case LYXP_TOKEN_OPERATOR_MATH:
        return "Operator(Math)";
    case LYXP_TOKEN_OPERATOR_UNI:
        return "Operator(Union)";
    case LYXP_TOKEN_OPERATOR_PATH:
        return "Operator(Path)";
    case LYXP_TOKEN_LITERAL:
        return "Literal";
    case LYXP_TOKEN_NUMBER:
        return "Number";
    default:
        LOGINT(NULL);
        return "";
    }
}

/**
 * @brief Print the whole expression \p exp to debug output.
 *
 * @param[in] exp Expression to use.
 */
static void
print_expr_struct_debug(struct lyxp_expr *exp)
{
    uint16_t i, j;
    char tmp[128];

    if (!exp || (ly_log_level < LY_LLDBG)) {
        return;
    }

    LOGDBG(LY_LDGXPATH, "expression \"%s\":", exp->expr);
    for (i = 0; i < exp->used; ++i) {
        sprintf(tmp, "\ttoken %s, in expression \"%.*s\"", print_token(exp->tokens[i]), exp->tok_len[i],
               &exp->expr[exp->expr_pos[i]]);
        if (exp->repeat[i]) {
            sprintf(tmp + strlen(tmp), " (repeat %d", exp->repeat[i][0]);
            for (j = 1; exp->repeat[i][j]; ++j) {
                sprintf(tmp + strlen(tmp), ", %d", exp->repeat[i][j]);
            }
            strcat(tmp, ")");
        }
        LOGDBG(LY_LDGXPATH, tmp);
    }
}

#ifndef NDEBUG

/**
 * @brief Print XPath set content to debug output.
 *
 * @param[in] set Set to print.
 */
static void
print_set_debug(struct lyxp_set *set)
{
    uint32_t i;
    char *str_num;
    struct lyxp_set_node *item;
    struct lyxp_set_snode *sitem;

    if (ly_log_level < LY_LLDBG) {
        return;
    }

    switch (set->type) {
    case LYXP_SET_NODE_SET:
        LOGDBG(LY_LDGXPATH, "set NODE SET:");
        for (i = 0; i < set->used; ++i) {
            item = &set->val.nodes[i];

            switch (item->type) {
            case LYXP_NODE_ROOT:
                LOGDBG(LY_LDGXPATH, "\t%d (pos %u): ROOT", i + 1, item->pos);
                break;
            case LYXP_NODE_ROOT_CONFIG:
                LOGDBG(LY_LDGXPATH, "\t%d (pos %u): ROOT CONFIG", i + 1, item->pos);
                break;
            case LYXP_NODE_ELEM:
                if ((item->node->schema->nodetype == LYS_LIST)
                        && (item->node->child->schema->nodetype == LYS_LEAF)) {
                    LOGDBG(LY_LDGXPATH, "\t%d (pos %u): ELEM %s (1st child val: %s)", i + 1, item->pos,
                           item->node->schema->name,
                           ((struct lyd_node_leaf_list *)item->node->child)->value_str);
                } else if (item->node->schema->nodetype == LYS_LEAFLIST) {
                    LOGDBG(LY_LDGXPATH, "\t%d (pos %u): ELEM %s (val: %s)", i + 1, item->pos,
                           item->node->schema->name,
                           ((struct lyd_node_leaf_list *)item->node)->value_str);
                } else {
                    LOGDBG(LY_LDGXPATH, "\t%d (pos %u): ELEM %s", i + 1, item->pos, item->node->schema->name);
                }
                break;
            case LYXP_NODE_TEXT:
                if (item->node->schema->nodetype & LYS_ANYDATA) {
                    LOGDBG(LY_LDGXPATH, "\t%d (pos %u): TEXT <%s>", i + 1, item->pos,
                           item->node->schema->nodetype == LYS_ANYXML ? "anyxml" : "anydata");
                } else {
                    LOGDBG(LY_LDGXPATH, "\t%d (pos %u): TEXT %s", i + 1, item->pos,
                           ((struct lyd_node_leaf_list *)item->node)->value_str);
                }
                break;
            case LYXP_NODE_ATTR:
                LOGDBG(LY_LDGXPATH, "\t%d (pos %u): ATTR %s = %s", i + 1, item->pos, set->val.attrs[i].attr->name,
                       set->val.attrs[i].attr->value);
                break;
            }
        }
        break;

    case LYXP_SET_SNODE_SET:
        LOGDBG(LY_LDGXPATH, "set SNODE SET:");
        for (i = 0; i < set->used; ++i) {
            sitem = &set->val.snodes[i];

            switch (sitem->type) {
            case LYXP_NODE_ROOT:
                LOGDBG(LY_LDGXPATH, "\t%d (%u): ROOT", i + 1, sitem->in_ctx);
                break;
            case LYXP_NODE_ROOT_CONFIG:
                LOGDBG(LY_LDGXPATH, "\t%d (%u): ROOT CONFIG", i + 1, sitem->in_ctx);
                break;
            case LYXP_NODE_ELEM:
                LOGDBG(LY_LDGXPATH, "\t%d (%u): ELEM %s", i + 1, sitem->in_ctx, sitem->snode->name);
                break;
            default:
                LOGINT(NULL);
                break;
            }
        }
        break;

    case LYXP_SET_EMPTY:
        LOGDBG(LY_LDGXPATH, "set EMPTY");
        break;

    case LYXP_SET_BOOLEAN:
        LOGDBG(LY_LDGXPATH, "set BOOLEAN");
        LOGDBG(LY_LDGXPATH, "\t%s", (set->val.bool ? "true" : "false"));
        break;

    case LYXP_SET_STRING:
        LOGDBG(LY_LDGXPATH, "set STRING");
        LOGDBG(LY_LDGXPATH, "\t%s", set->val.str);
        break;

    case LYXP_SET_NUMBER:
        LOGDBG(LY_LDGXPATH, "set NUMBER");

        if (isnan(set->val.num)) {
            str_num = strdup("NaN");
        } else if ((set->val.num == 0) || (set->val.num == -0.0f)) {
            str_num = strdup("0");
        } else if (isinf(set->val.num) && !signbit(set->val.num)) {
            str_num = strdup("Infinity");
        } else if (isinf(set->val.num) && signbit(set->val.num)) {
            str_num = strdup("-Infinity");
        } else if ((long long)set->val.num == set->val.num) {
            if (asprintf(&str_num, "%lld", (long long)set->val.num) == -1) {
                str_num = NULL;
            }
        } else {
            if (asprintf(&str_num, "%03.1Lf", set->val.num) == -1) {
                str_num = NULL;
            }
        }
        LY_CHECK_ERR_RETURN(!str_num, LOGMEM(NULL), );

        LOGDBG(LY_LDGXPATH, "\t%s", str_num);
        free(str_num);
    }
}

#endif

/**
 * @brief Realloc the string \p str.
 *
 * @param[in] needed How much free space is required.
 * @param[in,out] str Pointer to the string to use.
 * @param[in,out] used Used bytes in \p str.
 * @param[in,out] size Allocated bytes in \p str.
 */
static void
cast_string_realloc(uint16_t needed, char **str, uint16_t *used, uint16_t *size)
{
    if (*size - *used < needed) {
        do {
            *size += LYXP_STRING_CAST_SIZE_STEP;
        } while (*size - *used < needed);
        *str = ly_realloc(*str, *size * sizeof(char));
        LY_CHECK_ERR_RETURN(!(*str), LOGMEM(NULL), );
    }
}

/**
 * @brief Cast nodes recursively to one string \p str.
 *
 * @param[in] node Node to cast.
 * @param[in] fake_cont Whether to put the data into a "fake" container.
 * @param[in] root_type Type of the XPath root.
 * @param[in] indent Current indent.
 * @param[in,out] str Resulting string.
 * @param[in,out] used Used bytes in \p str.
 * @param[in,out] size Allocated bytes in \p str.
 */
static void
cast_string_recursive(struct lyd_node *node, struct lys_module *local_mod, int fake_cont, enum lyxp_node_type root_type,
                      uint16_t indent, char **str, uint16_t *used, uint16_t *size)
{
    char *buf, *line, *ptr;
    const char *value_str;
    struct lyd_node *child;
    struct lyd_node_anydata *any;

    if ((root_type == LYXP_NODE_ROOT_CONFIG) && (node->schema->flags & LYS_CONFIG_R)) {
        return;
    }

    if (fake_cont) {
        cast_string_realloc(1, str, used, size);
        strcpy(*str + (*used - 1), "\n");
        ++(*used);

        ++indent;
    }

    switch (node->schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_RPC:
    case LYS_NOTIF:
        cast_string_realloc(1, str, used, size);
        strcpy(*str + (*used - 1), "\n");
        ++(*used);

        LY_TREE_FOR(node->child, child) {
            cast_string_recursive(child, local_mod, 0, root_type, indent + 1, str, used, size);
        }

        break;

    case LYS_LEAF:
    case LYS_LEAFLIST:
        value_str = ((struct lyd_node_leaf_list *)node)->value_str;
        if (!value_str) {
            value_str = "";
        }

        /* print indent */
        cast_string_realloc(indent * 2 + strlen(value_str) + 1, str, used, size);
        memset(*str + (*used - 1), ' ', indent * 2);
        *used += indent * 2;

        /* print value */
        if (*used == 1) {
            sprintf(*str + (*used - 1), "%s", value_str);
            *used += strlen(value_str);
        } else {
            sprintf(*str + (*used - 1), "%s\n", value_str);
            *used += strlen(value_str) + 1;
        }

        break;

    case LYS_ANYXML:
    case LYS_ANYDATA:
        any = (struct lyd_node_anydata *)node;
        if (!(void*)any->value.tree) {
            /* no content */
            buf = strdup("");
            LY_CHECK_ERR_RETURN(!buf, LOGMEM(local_mod->ctx), );
        } else {
            switch (any->value_type) {
            case LYD_ANYDATA_CONSTSTRING:
            case LYD_ANYDATA_SXML:
            case LYD_ANYDATA_JSON:
                buf = strdup(any->value.str);
                LY_CHECK_ERR_RETURN(!buf, LOGMEM(local_mod->ctx), );
                break;
            case LYD_ANYDATA_DATATREE:
                if (lyd_print_mem(&buf, any->value.tree, LYD_XML, LYP_WITHSIBLINGS)) {
                    return;
                }
                break;
            case LYD_ANYDATA_XML:
                if (lyxml_print_mem(&buf, any->value.xml, LYXML_PRINT_SIBLINGS)) {
                    return;
                }
                break;
            case LYD_ANYDATA_LYB:
                LOGERR(local_mod->ctx, LY_EINVAL, "Cannot convert LYB anydata into string.");
                return;
            case LYD_ANYDATA_STRING:
            case LYD_ANYDATA_SXMLD:
            case LYD_ANYDATA_JSOND:
            case LYD_ANYDATA_LYBD:
                /* dynamic strings are used only as input parameters */
                assert(0);
                break;
            }
        }

        line = strtok_r(buf, "\n", &ptr);
        do {
            cast_string_realloc(indent * 2 + strlen(line) + 1, str, used, size);
            memset(*str + (*used - 1), ' ', indent * 2);
            *used += indent * 2;

            strcpy(*str + (*used - 1), line);
            *used += strlen(line);

            strcpy(*str + (*used - 1), "\n");
            *used += 1;
        } while ((line = strtok_r(NULL, "\n", &ptr)));

        free(buf);
        break;

    default:
        LOGINT(local_mod->ctx);
        break;
    }

    if (fake_cont) {
        cast_string_realloc(1, str, used, size);
        strcpy(*str + (*used - 1), "\n");
        ++(*used);

        --indent;
    }
}

/**
 * @brief Cast an element into a string.
 *
 * @param[in] node Node to cast.
 * @param[in] fake_cont Whether to put the data into a "fake" container.
 * @param[in] root_type Type of the XPath root.
 *
 * @return Element cast to dynamically-allocated string.
 */
static char *
cast_string_elem(struct lyd_node *node, struct lys_module *local_mod, int fake_cont, enum lyxp_node_type root_type)
{
    char *str;
    uint16_t used, size;

    str = malloc(LYXP_STRING_CAST_SIZE_START * sizeof(char));
    LY_CHECK_ERR_RETURN(!str, LOGMEM(local_mod->ctx), NULL);
    str[0] = '\0';
    used = 1;
    size = LYXP_STRING_CAST_SIZE_START;

    cast_string_recursive(node, local_mod, fake_cont, root_type, 0, &str, &used, &size);

    if (size > used) {
        str = ly_realloc(str, used * sizeof(char));
        LY_CHECK_ERR_RETURN(!str, LOGMEM(local_mod->ctx), NULL);
    }
    return str;
}

/**
 * @brief Cast a LYXP_SET_NODE_SET set into a string.
 *        Context position aware.
 *
 * @param[in] set Set to cast.
 * @param[in] cur_node Original context node.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return Cast string in the dictionary.
 */
static char *
cast_node_set_to_string(struct lyxp_set *set, struct lyd_node *cur_node, struct lys_module *local_mod, int options)
{
    enum lyxp_node_type root_type;
    char *str;

    if ((set->val.nodes[0].type != LYXP_NODE_ATTR) && (set->val.nodes[0].node->validity & LYD_VAL_INUSE)) {
        LOGVAL(local_mod->ctx, LYE_XPATH_DUMMY, LY_VLOG_LYD, set->val.nodes[0].node, set->val.nodes[0].node->schema->name);
        return NULL;
    }

    moveto_get_root(cur_node, options, &root_type);

    switch (set->val.nodes[0].type) {
    case LYXP_NODE_ROOT:
    case LYXP_NODE_ROOT_CONFIG:
        return cast_string_elem(set->val.nodes[0].node, local_mod, 1, root_type);
    case LYXP_NODE_ELEM:
    case LYXP_NODE_TEXT:
        return cast_string_elem(set->val.nodes[0].node, local_mod, 0, root_type);
    case LYXP_NODE_ATTR:
        str = strdup(set->val.attrs[0].attr->value_str);
        if (!str) {
            LOGMEM(local_mod->ctx);
        }
        return str;
    }

    LOGINT(local_mod->ctx);
    return NULL;
}

/**
 * @brief Cast a string into an XPath number.
 *
 * @param[in] str String to use.
 *
 * @return Cast number.
 */
static long double
cast_string_to_number(const char *str)
{
    long double num;
    char *ptr;

    errno = 0;
    num = strtold(str, &ptr);
    if (errno || *ptr) {
        num = NAN;
    }
    return num;
}

/*
 * lyxp_set manipulation functions
 */

#ifdef LY_ENABLED_CACHE

static int
set_values_equal_cb(void *val1_p, void *val2_p, int UNUSED(mod), void *UNUSED(cb_data))
{
    struct lyxp_set_hash_node *val1, *val2;

    val1 = (struct lyxp_set_hash_node *)val1_p;
    val2 = (struct lyxp_set_hash_node *)val2_p;

    if ((val1->node == val2->node) && (val1->type == val2->type)) {
        return 1;
    }

    return 0;
}

static void
set_insert_node_hash(struct lyxp_set *set, struct lyd_node *node, enum lyxp_node_type type)
{
    int r;
    uint32_t i, hash;
    struct lyxp_set_hash_node hnode;

    if (!set->ht && (set->used >= LY_CACHE_HT_MIN_CHILDREN)) {
        /* create hash table and add all the nodes */
        set->ht = lyht_new(1, sizeof(struct lyxp_set_hash_node), set_values_equal_cb, NULL, 1);
        for (i = 0; i < set->used; ++i) {
            hnode.node = set->val.nodes[i].node;
            hnode.type = set->val.nodes[i].type;

            hash = dict_hash_multi(0, (const char *)&hnode.node, sizeof hnode.node);
            hash = dict_hash_multi(hash, (const char *)&hnode.type, sizeof hnode.type);
            hash = dict_hash_multi(hash, NULL, 0);

            r = lyht_insert(set->ht, &hnode, hash, NULL);
            assert(!r);
            (void)r;
        }
    } else if (set->ht) {
        assert(node);

        /* add the new node into hash table */
        hnode.node = node;
        hnode.type = type;

        hash = dict_hash_multi(0, (const char *)&hnode.node, sizeof hnode.node);
        hash = dict_hash_multi(hash, (const char *)&hnode.type, sizeof hnode.type);
        hash = dict_hash_multi(hash, NULL, 0);

        r = lyht_insert(set->ht, &hnode, hash, NULL);
        assert(!r);
        (void)r;
    }
}

static void
set_remove_node_hash(struct lyxp_set *set, struct lyd_node *node, enum lyxp_node_type type)
{
    int r;
    struct lyxp_set_hash_node hnode;
    uint32_t hash;

    if (set->ht) {
        hnode.node = node;
        hnode.type = type;

        hash = dict_hash_multi(0, (const char *)&hnode.node, sizeof hnode.node);
        hash = dict_hash_multi(hash, (const char *)&hnode.type, sizeof hnode.type);
        hash = dict_hash_multi(hash, NULL, 0);

        r = lyht_remove(set->ht, &hnode, hash);
        assert(!r);
        (void)r;

        if (!set->ht->used) {
            lyht_free(set->ht);
            set->ht = NULL;
        }
    }
}

static int
set_dup_node_hash_check(const struct lyxp_set *set, struct lyd_node *node, enum lyxp_node_type type, int skip_idx)
{
    struct lyxp_set_hash_node hnode, *match_p;
    uint32_t hash;

    hnode.node = node;
    hnode.type = type;

    hash = dict_hash_multi(0, (const char *)&hnode.node, sizeof hnode.node);
    hash = dict_hash_multi(hash, (const char *)&hnode.type, sizeof hnode.type);
    hash = dict_hash_multi(hash, NULL, 0);

    if (!lyht_find(set->ht, &hnode, hash, (void **)&match_p)) {
        if ((skip_idx > -1) && (set->val.nodes[skip_idx].node == match_p->node) && (set->val.nodes[skip_idx].type == match_p->type)) {
            /* we found it on the index that should be skipped, find another */
            hnode = *match_p;
            if (lyht_find_next(set->ht, &hnode, hash, (void **)&match_p)) {
                /* none other found */
                return 0;
            }
        }

        return 1;
    }

    /* not found */
    return 0;
}

#endif

static void
set_free_content(struct lyxp_set *set)
{
    if (!set) {
        return;
    }

    if (set->type == LYXP_SET_NODE_SET) {
        free(set->val.nodes);
#ifdef LY_ENABLED_CACHE
        lyht_free(set->ht);
        set->ht = NULL;
#endif
    } else if (set->type == LYXP_SET_SNODE_SET) {
        free(set->val.snodes);
    } else if (set->type == LYXP_SET_STRING) {
        free(set->val.str);
    }
    set->type = LYXP_SET_EMPTY;
}

void
lyxp_set_free(struct lyxp_set *set)
{
    if (!set) {
        return;
    }

    set_free_content(set);
    free(set);
}

/**
 * @brief Create a deep copy of a \p set.
 *
 * @param[in] set Set to copy.
 *
 * @return Copy of \p set.
 */
static struct lyxp_set *
set_copy(struct lyxp_set *set)
{
    struct lyxp_set *ret;
    uint16_t i;

    if (!set) {
        return NULL;
    }

    ret = malloc(sizeof *ret);
    LY_CHECK_ERR_RETURN(!ret, LOGMEM(NULL), NULL);

    if (set->type == LYXP_SET_SNODE_SET) {
        memset(ret, 0, sizeof *ret);
        ret->type = set->type;

        for (i = 0; i < set->used; ++i) {
            if (set->val.snodes[i].in_ctx == 1) {
                if (set_snode_insert_node(ret, set->val.snodes[i].snode, set->val.snodes[i].type)) {
                    lyxp_set_free(ret);
                    return NULL;
                }
            }
        }
    } else if (set->type == LYXP_SET_NODE_SET) {
        ret->type = set->type;
        ret->val.nodes = malloc(set->used * sizeof *ret->val.nodes);
        LY_CHECK_ERR_RETURN(!ret->val.nodes, LOGMEM(NULL); free(ret), NULL);
        memcpy(ret->val.nodes, set->val.nodes, set->used * sizeof *ret->val.nodes);

        ret->used = ret->size = set->used;
        ret->ctx_pos = set->ctx_pos;
        ret->ctx_size = set->ctx_size;

#ifdef LY_ENABLED_CACHE
        ret->ht = lyht_dup(set->ht);
#endif
    } else {
       memcpy(ret, set, sizeof *ret);
       if (set->type == LYXP_SET_STRING) {
           ret->val.str = strdup(set->val.str);
           LY_CHECK_ERR_RETURN(!ret->val.str, LOGMEM(NULL); free(ret), NULL);
       }
    }

    return ret;
}

/**
 * @brief Fill XPath set with a string. Any current data are disposed of.
 *
 * @param[in] set Set to fill.
 * @param[in] string String to fill into \p set.
 * @param[in] str_len Length of \p string. 0 is a valid value!
 * @param[in] ctx libyang context to use.
 */
static void
set_fill_string(struct lyxp_set *set, const char *string, uint16_t str_len)
{
    set_free_content(set);

    set->type = LYXP_SET_STRING;
    if ((str_len == 0) && (string[0] != '\0')) {
        string = "";
    }
    set->val.str = strndup(string, str_len);
}

/**
 * @brief Fill XPath set with a number. Any current data are disposed of.
 *
 * @param[in] set Set to fill.
 * @param[in] number Number to fill into \p set.
 */
static void
set_fill_number(struct lyxp_set *set, long double number)
{
    set_free_content(set);

    set->type = LYXP_SET_NUMBER;
    set->val.num = number;
}

/**
 * @brief Fill XPath set with a boolean. Any current data are disposed of.
 *
 * @param[in] set Set to fill.
 * @param[in] boolean Boolean to fill into \p set.
 */
static void
set_fill_boolean(struct lyxp_set *set, int boolean)
{
    set_free_content(set);

    set->type = LYXP_SET_BOOLEAN;
    set->val.bool = boolean;
}

/**
 * @brief Fill XPath set with the value from another set (deep assign).
 *        Any current data are disposed of.
 *
 * @param[in] trg Set to fill.
 * @param[in] src Source set to copy into \p trg.
 */
static void
set_fill_set(struct lyxp_set *trg, struct lyxp_set *src)
{
    if (!trg || !src) {
        return;
    }

    if (src->type == LYXP_SET_SNODE_SET) {
        trg->type = LYXP_SET_SNODE_SET;
        trg->used = src->used;
        trg->size = src->used;

        trg->val.snodes = ly_realloc(trg->val.snodes, trg->size * sizeof *trg->val.nodes);
        LY_CHECK_ERR_RETURN(!trg->val.nodes, LOGMEM(NULL); memset(trg, 0, sizeof *trg), );
        memcpy(trg->val.nodes, src->val.nodes, src->used * sizeof *src->val.nodes);
    } else if (src->type == LYXP_SET_BOOLEAN) {
        set_fill_boolean(trg, src->val.bool);
    } else if (src->type ==  LYXP_SET_NUMBER) {
        set_fill_number(trg, src->val.num);
    } else if (src->type == LYXP_SET_STRING) {
        set_fill_string(trg, src->val.str, strlen(src->val.str));
    } else {
        if (trg->type == LYXP_SET_NODE_SET) {
            free(trg->val.nodes);
        } else if (trg->type == LYXP_SET_STRING) {
            free(trg->val.str);
        }

        if (src->type == LYXP_SET_EMPTY) {
            trg->type = LYXP_SET_EMPTY;
        } else {
            assert(src->type == LYXP_SET_NODE_SET);

            trg->type = LYXP_SET_NODE_SET;
            trg->used = src->used;
            trg->size = src->used;
            trg->ctx_pos = src->ctx_pos;
            trg->ctx_size = src->ctx_size;

            trg->val.nodes = malloc(trg->used * sizeof *trg->val.nodes);
            LY_CHECK_ERR_RETURN(!trg->val.nodes, LOGMEM(NULL); memset(trg, 0, sizeof *trg), );
            memcpy(trg->val.nodes, src->val.nodes, src->used * sizeof *src->val.nodes);
#ifdef LY_ENABLED_CACHE
            trg->ht = lyht_dup(src->ht);
#endif
        }
    }


}

static void
set_snode_clear_ctx(struct lyxp_set *set)
{
    uint32_t i;

    for (i = 0; i < set->used; ++i) {
        if (set->val.snodes[i].in_ctx == 1) {
            set->val.snodes[i].in_ctx = 0;
        }
    }
}

/**
 * @brief Remove a node from a set. Removing last node changes
 *        \p set into LYXP_SET_EMPTY. Context position aware.
 *
 * @param[in] set Set to use.
 * @param[in] idx Index from \p set of the node to be removed.
 */
static void
set_remove_node(struct lyxp_set *set, uint32_t idx)
{
    assert(set && (set->type == LYXP_SET_NODE_SET));
    assert(idx < set->used);

#ifdef LY_ENABLED_CACHE
    set_remove_node_hash(set, set->val.nodes[idx].node, set->val.nodes[idx].type);
#endif

    --set->used;
    if (set->used) {
        memmove(&set->val.nodes[idx], &set->val.nodes[idx + 1],
                (set->used - idx) * sizeof *set->val.nodes);
    } else {
        set_free_content(set);
        /* this changes it to LYXP_SET_EMPTY */
        memset(set, 0, sizeof *set);
    }
}

/**
 * @brief Check for duplicates in a node set.
 *
 * @param[in] set Set to check.
 * @param[in] node Node to look for in \p set.
 * @param[in] node_type Type of \p node.
 * @param[in] skip_idx Index from \p set to skip.
 *
 * @return 0 on success, 1 on duplicate found.
 */
static int
set_dup_node_check(const struct lyxp_set *set, const struct lyd_node *node, enum lyxp_node_type node_type, int skip_idx)
{
    uint32_t i;

#ifdef LY_ENABLED_CACHE
    if (set->ht) {
        return set_dup_node_hash_check(set, (struct lyd_node *)node, node_type, skip_idx);
    }
#endif

    for (i = 0; i < set->used; ++i) {
        if ((skip_idx > -1) && (i == (unsigned)skip_idx)) {
            continue;
        }

        if ((set->val.nodes[i].node == node) && (set->val.nodes[i].type == node_type)) {
            return 1;
        }
    }

    return 0;
}

static int
set_snode_dup_node_check(struct lyxp_set *set, const struct lys_node *node, enum lyxp_node_type node_type, int skip_idx)
{
    uint32_t i;

    for (i = 0; i < set->used; ++i) {
        if ((skip_idx > -1) && (i == (unsigned)skip_idx)) {
            continue;
        }

        if ((set->val.snodes[i].snode == node) && (set->val.snodes[i].type == node_type)) {
            return i;
        }
    }

    return -1;
}

static void
set_snode_merge(struct lyxp_set *set1, struct lyxp_set *set2)
{
    uint32_t orig_used, i, j;

    assert(((set1->type == LYXP_SET_SNODE_SET) || (set1->type == LYXP_SET_EMPTY))
        && ((set2->type == LYXP_SET_SNODE_SET) || (set2->type == LYXP_SET_EMPTY)));

    if (set2->type == LYXP_SET_EMPTY) {
        return;
    }

    if (set1->type == LYXP_SET_EMPTY) {
        memcpy(set1, set2, sizeof *set1);
        return;
    }

    if (set1->used + set2->used > set1->size) {
        set1->size = set1->used + set2->used;
        set1->val.snodes = ly_realloc(set1->val.snodes, set1->size * sizeof *set1->val.snodes);
        LY_CHECK_ERR_RETURN(!set1->val.snodes, LOGMEM(NULL), );
    }

    orig_used = set1->used;

    for (i = 0; i < set2->used; ++i) {
        for (j = 0; j < orig_used; ++j) {
            /* detect duplicities */
            if (set1->val.snodes[j].snode == set2->val.snodes[i].snode) {
                break;
            }
        }

        if (j == orig_used) {
            memcpy(&set1->val.snodes[set1->used], &set2->val.snodes[i], sizeof *set2->val.snodes);
            ++set1->used;
        }
    }

    free(set2->val.snodes);
    memset(set2, 0, sizeof *set2);
}

/**
 * @brief Insert a node into a set. Context position aware.
 *
 * @param[in] set Set to use.
 * @param[in] node Node to insert to \p set.
 * @param[in] pos Sort position of \p node. If left 0, it is filled just before sorting.
 * @param[in] node_type Node type of \p node.
 * @param[in] idx Index in \p set to insert into.
 */
static void
set_insert_node(struct lyxp_set *set, const struct lyd_node *node, uint32_t pos, enum lyxp_node_type node_type, uint32_t idx)
{
    assert(set && ((set->type == LYXP_SET_NODE_SET) || (set->type == LYXP_SET_EMPTY)));

    if (set->type == LYXP_SET_EMPTY) {
        /* first item */
        if (idx) {
            /* no real harm done, but it is a bug */
            LOGINT(NULL);
            idx = 0;
        }
        set->val.nodes = malloc(LYXP_SET_SIZE_START * sizeof *set->val.nodes);
        LY_CHECK_ERR_RETURN(!set->val.nodes, LOGMEM(NULL), );
        set->type = LYXP_SET_NODE_SET;
        set->used = 0;
        set->size = LYXP_SET_SIZE_START;
        set->ctx_pos = 1;
        set->ctx_size = 1;
#ifdef LY_ENABLED_CACHE
        set->ht = NULL;
#endif
    } else {
        /* not an empty set */
        if (set->used == set->size) {

            /* set is full */
            set->val.nodes = ly_realloc(set->val.nodes, (set->size + LYXP_SET_SIZE_STEP) * sizeof *set->val.nodes);
            LY_CHECK_ERR_RETURN(!set->val.nodes, LOGMEM(NULL), );
            set->size += LYXP_SET_SIZE_STEP;
        }

        if (idx > set->used) {
            LOGINT(NULL);
            idx = set->used;
        }

        /* make space for the new node */
        if (idx < set->used) {
            memmove(&set->val.nodes[idx + 1], &set->val.nodes[idx], (set->used - idx) * sizeof *set->val.nodes);
        }
    }

    /* finally assign the value */
    set->val.nodes[idx].node = (struct lyd_node *)node;
    set->val.nodes[idx].type = node_type;
    set->val.nodes[idx].pos = pos;
    ++set->used;

#ifdef LY_ENABLED_CACHE
    set_insert_node_hash(set, (struct lyd_node *)node, node_type);
#endif
}

static int
set_snode_insert_node(struct lyxp_set *set, const struct lys_node *node, enum lyxp_node_type node_type)
{
    int ret;

    assert(set->type == LYXP_SET_SNODE_SET);

    ret = set_snode_dup_node_check(set, node, node_type, -1);
    if (ret > -1) {
        set->val.snodes[ret].in_ctx = 1;
    } else {
        if (set->used == set->size) {
            set->val.snodes = ly_realloc(set->val.snodes, (set->size + LYXP_SET_SIZE_STEP) * sizeof *set->val.snodes);
            LY_CHECK_ERR_RETURN(!set->val.snodes, LOGMEM(node->module->ctx), -1);
            set->size += LYXP_SET_SIZE_STEP;
        }

        ret = set->used;
        set->val.snodes[ret].snode = (struct lys_node *)node;
        set->val.snodes[ret].type = node_type;
        set->val.snodes[ret].in_ctx = 1;
        ++set->used;
    }

    return ret;
}

/**
 * @brief Replace a node in a set with another. Context position aware.
 *
 * @param[in] set Set to use.
 * @param[in] node Node to insert to \p set.
 * @param[in] pos Sort position of \p node. If left 0, it is filled just before sorting.
 * @param[in] node_type Node type of \p node.
 * @param[in] idx Index in \p set of the node to replace.
 */
static void
set_replace_node(struct lyxp_set *set, const struct lyd_node *node, uint32_t pos, enum lyxp_node_type node_type, uint32_t idx)
{
    assert(set && (idx < set->used));

#ifdef LY_ENABLED_CACHE
    set_remove_node_hash(set, set->val.nodes[idx].node, set->val.nodes[idx].type);
#endif
    set->val.nodes[idx].node = (struct lyd_node *)node;
    set->val.nodes[idx].type = node_type;
    set->val.nodes[idx].pos = pos;
#ifdef LY_ENABLED_CACHE
    set_insert_node_hash(set, set->val.nodes[idx].node, set->val.nodes[idx].type);
#endif
}

static uint32_t
set_snode_new_in_ctx(struct lyxp_set *set)
{
    uint32_t ret_ctx, i;

    assert(set->type == LYXP_SET_SNODE_SET);

    ret_ctx = 3;
retry:
    for (i = 0; i < set->used; ++i) {
        if (set->val.snodes[i].in_ctx >= ret_ctx) {
            ret_ctx = set->val.snodes[i].in_ctx + 1;
            goto retry;
        }
    }
    for (i = 0; i < set->used; ++i) {
        if (set->val.snodes[i].in_ctx == 1) {
            set->val.snodes[i].in_ctx = ret_ctx;
        }
    }

    return ret_ctx;
}

/**
 * @brief Get unique \p node position in the data.
 *
 * @param[in] node Node to find.
 * @param[in] node_type Node type of \p node.
 * @param[in] root Root node.
 * @param[in] root_type Type of the XPath \p root node.
 * @param[in] prev Node that we think is before \p node in DFS from \p root. Can optionally
 * be used to increase efficiency and start the DFS from this node.
 * @param[in] prev_pos Node \p prev position. Optional, but must be set if \p prev is set.
 *
 * @return Node position.
 */
static uint32_t
get_node_pos(const struct lyd_node *node, enum lyxp_node_type node_type, const struct lyd_node *root,
             enum lyxp_node_type root_type, const struct lyd_node **prev, uint32_t *prev_pos)
{
    const struct lyd_node *next, *elem, *top_sibling;
    uint32_t pos = 1;

    assert(prev && prev_pos && !root->prev->next);

    if ((node_type == LYXP_NODE_ROOT) || (node_type == LYXP_NODE_ROOT_CONFIG)) {
        return 0;
    }

    if (*prev) {
        /* start from the previous element instead from the root */
        elem = next = *prev;
        pos = *prev_pos;
        for (top_sibling = elem; top_sibling->parent; top_sibling = top_sibling->parent);
        goto dfs_search;
    }

    LY_TREE_FOR(root, top_sibling) {
        /* TREE DFS */
        LY_TREE_DFS_BEGIN(top_sibling, next, elem) {
dfs_search:
            if ((root_type == LYXP_NODE_ROOT_CONFIG) && (elem->schema->flags & LYS_CONFIG_R)) {
                goto skip_children;
            }

            if (elem == node) {
                break;
            }
            ++pos;

            /* TREE DFS END */
            /* select element for the next run - children first,
             * child exception for lyd_node_leaf and lyd_node_leaflist, but not the root */
            if (elem->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                next = NULL;
            } else {
                next = elem->child;
            }
            if (!next) {
skip_children:
                /* no children */
                if (elem == top_sibling) {
                    /* we are done, root has no children */
                    elem = NULL;
                    break;
                }
                /* try siblings */
                next = elem->next;
            }
            while (!next) {
                /* no siblings, go back through parents */
                if (elem->parent == top_sibling->parent) {
                    /* we are done, no next element to process */
                    elem = NULL;
                    break;
                }
                /* parent is already processed, go to its sibling */
                elem = elem->parent;
                next = elem->next;
            }
        }

        /* node found */
        if (elem) {
            break;
        }
    }

    if (!elem) {
        if (!(*prev)) {
            /* we went from root and failed to find it, cannot be */
            LOGINT(node->schema->module->ctx);
            return 0;
        } else {
            /* node is before prev, we assumed otherwise :( */
            //LOGDBG(LY_LDGXPATH, "get_node_pos optimalization fail.");

            *prev = NULL;
            *prev_pos = 0;

            elem = next = top_sibling = root;
            pos = 1;
            goto dfs_search;
        }
    }

    /*if (*prev) {
        LOGDBG(LY_LDGXPATH, "get_node_pos optimalization success.");
    }*/

    /* remember the last found node for next time */
    *prev = node;
    *prev_pos = pos;

    return pos;
}

/**
 * @brief Assign (fill) missing node positions.
 *
 * @param[in] set Set to fill positions in.
 * @param[in] root Context root node.
 * @param[in] root_type Context root type.
 *
 * @return 0 on success, -1 on error.
 */
static int
set_assign_pos(struct lyxp_set *set, const struct lyd_node *root, enum lyxp_node_type root_type)
{
    const struct lyd_node *prev = NULL, *tmp_node;
    uint32_t i, tmp_pos = 0;

    for (i = 0; i < set->used; ++i) {
        if (!set->val.nodes[i].pos) {
            tmp_node = NULL;
            switch (set->val.nodes[i].type) {
            case LYXP_NODE_ATTR:
                tmp_node = lyd_attr_parent(root, set->val.attrs[i].attr);
                if (!tmp_node) {
                    LOGINT(root->schema->module->ctx);
                    return -1;
                }
                /* fallthrough */
            case LYXP_NODE_ELEM:
            case LYXP_NODE_TEXT:
                if (!tmp_node) {
                    tmp_node = set->val.nodes[i].node;
                }
                set->val.nodes[i].pos = get_node_pos(tmp_node, set->val.nodes[i].type, root, root_type, &prev, &tmp_pos);
                break;
            default:
                /* all roots have position 0 */
                break;
            }
        }
    }

    return 0;
}

/**
 * @brief Get unique \p attr position in the parent attributes.
 *
 * @param[in] attr Attr to use.
 * @param[in] parent Parent of \p attr.
 *
 * @return Attribute position.
 */
static uint16_t
get_attr_pos(struct lyd_attr *attr, const struct lyd_node *parent)
{
    uint16_t pos = 0;
    struct lyd_attr *attr2;

    for (attr2 = parent->attr; attr2 && (attr2 != attr); attr2 = attr2->next) {
        ++pos;
    }

    assert(attr2);
    return pos;
}

/**
 * @brief Compare 2 nodes in respect to XPath document order.
 *
 * @param[in] idx1 Index of the 1st node in \p set1.
 * @param[in] set1 Set with the 1st node on index \p idx1.
 * @param[in] idx2 Index of the 2nd node in \p set2.
 * @param[in] set2 Set with the 2nd node on index \p idx2.
 * @param[in] root Context root node.
 *
 * @return If 1st > 2nd returns 1, 1st == 2nd returns 0, and 1st < 2nd returns -1.
 */
static int
set_sort_compare(struct lyxp_set_node *item1, struct lyxp_set_node *item2,
                 const struct lyd_node *root)
{
    const struct lyd_node *tmp_node;
    uint32_t attr_pos1 = 0, attr_pos2 = 0;

    if (item1->pos < item2->pos) {
        return -1;
    }

    if (item1->pos > item2->pos) {
        return 1;
    }

    /* node positions are equal, the fun case */

    /* 1st ELEM - == - 2nd TEXT, 1st TEXT - == - 2nd ELEM */
    /* special case since text nodes are actually saved as their parents */
    if ((item1->node == item2->node) && (item1->type != item2->type)) {
        if (item1->type == LYXP_NODE_ELEM) {
            assert(item2->type == LYXP_NODE_TEXT);
            return -1;
        } else {
            assert((item1->type == LYXP_NODE_TEXT) && (item2->type == LYXP_NODE_ELEM));
            return 1;
        }
    }

    /* we need attr positions now */
    if (item1->type == LYXP_NODE_ATTR) {
        tmp_node = lyd_attr_parent(root, (struct lyd_attr *)item1->node);
        if (!tmp_node) {
            LOGINT(root->schema->module->ctx);
            return -1;
        }
        attr_pos1 = get_attr_pos((struct lyd_attr *)item1->node, tmp_node);
    }
    if (item2->type == LYXP_NODE_ATTR) {
        tmp_node = lyd_attr_parent(root, (struct lyd_attr *)item2->node);
        if (!tmp_node) {
            LOGINT(root->schema->module->ctx);
            return -1;
        }
        attr_pos2 = get_attr_pos((struct lyd_attr *)item2->node, tmp_node);
    }

    /* 1st ROOT - 2nd ROOT, 1st ELEM - 2nd ELEM, 1st TEXT - 2nd TEXT, 1st ATTR - =pos= - 2nd ATTR */
    /* check for duplicates */
    if (item1->node == item2->node) {
        assert((item1->type == item2->type) && ((item1->type != LYXP_NODE_ATTR) || (attr_pos1 == attr_pos2)));
        return 0;
    }

    /* 1st ELEM - 2nd TEXT, 1st ELEM - any pos - 2nd ATTR */
    /* elem is always first, 2nd node is after it */
    if (item1->type == LYXP_NODE_ELEM) {
        assert(item2->type != LYXP_NODE_ELEM);
        return -1;
    }

    /* 1st TEXT - 2nd ELEM, 1st TEXT - any pos - 2nd ATTR, 1st ATTR - any pos - 2nd ELEM, 1st ATTR - >pos> - 2nd ATTR */
    /* 2nd is before 1st */
    if (((item1->type == LYXP_NODE_TEXT)
            && ((item2->type == LYXP_NODE_ELEM) || (item2->type == LYXP_NODE_ATTR)))
            || ((item1->type == LYXP_NODE_ATTR) && (item2->type == LYXP_NODE_ELEM))
            || (((item1->type == LYXP_NODE_ATTR) && (item2->type == LYXP_NODE_ATTR))
            && (attr_pos1 > attr_pos2))) {
        return 1;
    }

    /* 1st ATTR - any pos - 2nd TEXT, 1st ATTR <pos< - 2nd ATTR */
    /* 2nd is after 1st */
    return -1;
}

static int
set_comp_cast(struct lyxp_set *trg, struct lyxp_set *src, enum lyxp_set_type type, const struct lyd_node *cur_node,
              const struct lys_module *local_mod, uint32_t src_idx, int options)
{
    assert(src->type == LYXP_SET_NODE_SET);

    memset(trg, 0, sizeof *trg);

    /* insert node into target set */
    set_insert_node(trg, src->val.nodes[src_idx].node, src->val.nodes[src_idx].pos, src->val.nodes[src_idx].type, 0);

    /* cast target set appropriately */
    if (lyxp_set_cast(trg, type, cur_node, local_mod, options)) {
        set_free_content(trg);
        return -1;
    }

    return EXIT_SUCCESS;
}

#ifndef NDEBUG

/**
 * @brief Bubble sort \p set into XPath document order.
 *        Context position aware. Unused in the 'Release' build target.
 *
 * @param[in] set Set to sort.
 * @param[in] cur_node Original context node.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return How many times the whole set was traversed - 1 (if set was sorted, returns 0).
 */
static int
set_sort(struct lyxp_set *set, const struct lyd_node *cur_node, int options)
{
    uint32_t i, j;
    int ret = 0, cmp, inverted, change;
    const struct lyd_node *root;
    enum lyxp_node_type root_type;
    struct lyxp_set_node item;

    if ((set->type != LYXP_SET_NODE_SET) || (set->used == 1)) {
        return 0;
    }

    /* get root */
    root = moveto_get_root(cur_node, options, &root_type);

    /* fill positions */
    if (set_assign_pos(set, root, root_type)) {
        return -1;
    }

    LOGDBG(LY_LDGXPATH, "SORT BEGIN");
    print_set_debug(set);

    for (i = 0; i < set->used; ++i) {
        inverted = 0;
        change = 0;

        for (j = 1; j < set->used - i; ++j) {
            /* compare node positions */
            if (inverted) {
                cmp = set_sort_compare(&set->val.nodes[j], &set->val.nodes[j - 1], root);
            } else {
                cmp = set_sort_compare(&set->val.nodes[j - 1], &set->val.nodes[j], root);
            }

            /* swap if needed */
            if ((inverted && (cmp < 0)) || (!inverted && (cmp > 0))) {
                change = 1;

                item = set->val.nodes[j - 1];
                set->val.nodes[j - 1] = set->val.nodes[j];
                set->val.nodes[j] = item;
            } else {
                /* whether node_pos1 should be smaller than node_pos2 or the other way around */
                inverted = !inverted;
            }
        }

        ++ret;

        if (!change) {
            break;
        }
    }

    LOGDBG(LY_LDGXPATH, "SORT END %d", ret);
    print_set_debug(set);

#ifdef LY_ENABLED_CACHE
    struct lyxp_set_hash_node hnode;
    uint64_t hash;

    /* check node hashes */
    if (set->used >= LY_CACHE_HT_MIN_CHILDREN) {
        assert(set->ht);
        for (i = 0; i < set->used; ++i) {
            hnode.node = set->val.nodes[i].node;
            hnode.type = set->val.nodes[i].type;

            hash = dict_hash_multi(0, (const char *)&hnode.node, sizeof hnode.node);
            hash = dict_hash_multi(hash, (const char *)&hnode.type, sizeof hnode.type);
            hash = dict_hash_multi(hash, NULL, 0);

            assert(!lyht_find(set->ht, &hnode, hash, NULL));
        }
    }
#endif

    return ret - 1;
}

/**
 * @brief Remove duplicate entries in a sorted node set.
 *
 * @param[in] set Sorted set to check.
 *
 * @return EXIT_SUCCESS if no duplicates were found,
 *         EXIT_FAILURE otherwise.
 */
static int
set_sorted_dup_node_clean(struct lyxp_set *set)
{
    uint32_t i = 0;
    int ret = EXIT_SUCCESS;

    if (set->used > 1) {
        while (i < set->used - 1) {
            if ((set->val.nodes[i].node == set->val.nodes[i + 1].node)
                    && (set->val.nodes[i].type == set->val.nodes[i + 1].type)) {
                set_remove_node(set, i + 1);
            ret = EXIT_FAILURE;
            } else {
                ++i;
            }
        }
    }

    return ret;
}

#endif

/**
 * @brief Merge 2 sorted sets into one.
 *
 * @param[in,out] trg Set to merge into. Duplicates are removed.
 * @param[in] src Set to be merged into \p trg. It is cast to #LYXP_SET_EMPTY on success.
 * @param[in] cur_node Original context node.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return 0 on success, -1 on error.
 */
static int
set_sorted_merge(struct lyxp_set *trg, struct lyxp_set *src, struct lyd_node *cur_node, int options)
{
    uint32_t i, j, count, dup_count;
    int cmp;
    const struct lyd_node *root;
    enum lyxp_node_type root_type;

    if (((trg->type != LYXP_SET_NODE_SET) && (trg->type != LYXP_SET_EMPTY))
            || ((src->type != LYXP_SET_NODE_SET) && (src->type != LYXP_SET_EMPTY))) {
        return -1;
    }

    if (src->type == LYXP_SET_EMPTY) {
        return 0;
    } else if (trg->type == LYXP_SET_EMPTY) {
        set_fill_set(trg, src);
        lyxp_set_cast(src, LYXP_SET_EMPTY, cur_node, NULL, options);
        return 0;
    }

    /* get root */
    root = moveto_get_root(cur_node, options, &root_type);

    /* fill positions */
    if (set_assign_pos(trg, root, root_type) || set_assign_pos(src, root, root_type)) {
        return -1;
    }

#ifndef NDEBUG
    LOGDBG(LY_LDGXPATH, "MERGE target");
    print_set_debug(trg);
    LOGDBG(LY_LDGXPATH, "MERGE source");
    print_set_debug(src);
#endif

    /* make memory for the merge (duplicates are not detected yet, so space
     * will likely be wasted on them, too bad) */
    if (trg->size - trg->used < src->used) {
        trg->size = trg->used + src->used;

        trg->val.nodes = ly_realloc(trg->val.nodes, trg->size * sizeof *trg->val.nodes);
        LY_CHECK_ERR_RETURN(!trg->val.nodes, LOGMEM(cur_node->schema->module->ctx), -1);
    }

    i = 0;
    j = 0;
    count = 0;
    dup_count = 0;
    do {
        cmp = set_sort_compare(&src->val.nodes[i], &trg->val.nodes[j], root);
        if (!cmp) {
            if (!count) {
                /* duplicate, just skip it */
                ++i;
                ++j;
            } else {
                /* we are copying something already, so let's copy the duplicate too,
                 * we are hoping that afterwards there are some more nodes to
                 * copy and this way we can copy them all together */
                ++count;
                ++dup_count;
                ++i;
                ++j;
            }
        } else if (cmp < 0) {
            /* inserting src node into trg, just remember it for now */
            ++count;
            ++i;

#ifdef LY_ENABLED_CACHE
            /* insert the hash now */
            set_insert_node_hash(trg, src->val.nodes[i - 1].node, src->val.nodes[i - 1].type);
#endif
        } else if (count) {
copy_nodes:
            /* time to actually copy the nodes, we have found the largest block of nodes */
            memmove(&trg->val.nodes[j + (count - dup_count)],
                    &trg->val.nodes[j],
                    (trg->used - j) * sizeof *trg->val.nodes);
            memcpy(&trg->val.nodes[j - dup_count], &src->val.nodes[i - count], count * sizeof *src->val.nodes);

            trg->used += count - dup_count;
            /* do not change i, except the copying above, we are basically doing exactly what is in the else branch below */
            j += count - dup_count;

            count = 0;
            dup_count = 0;
        } else {
            ++j;
        }
    } while ((i < src->used) && (j < trg->used));

    if ((i < src->used) || count) {
#ifdef LY_ENABLED_CACHE
        uint32_t k;

        /* insert all the hashes first */
        for (k = i; k < src->used; ++k) {
            set_insert_node_hash(trg, src->val.nodes[k].node, src->val.nodes[k].type);
        }
#endif
        /* loop ended, but we need to copy something at trg end */
        count += src->used - i;
        i = src->used;
        goto copy_nodes;
    }

#ifdef LY_ENABLED_CACHE
    /* we are inserting hashes before the actual node insert, which causes
     * situations when there were initially not enough items for a hash table,
     * but even after some were inserted, hash table was not created (during
     * insertion the number of items is not updated yet) */
    if (!trg->ht && (trg->used >= LY_CACHE_HT_MIN_CHILDREN)) {
        set_insert_node_hash(trg, NULL, 0);
    }
#endif

#ifndef NDEBUG
    LOGDBG(LY_LDGXPATH, "MERGE result");
    print_set_debug(trg);
#endif

    lyxp_set_cast(src, LYXP_SET_EMPTY, cur_node, NULL, options);
    return 0;
}

/*
 * (re)parse functions
 *
 * Parse functions parse the expression into
 * tokens (syntactic analysis).
 *
 * Reparse functions perform semantic analysis
 * (do not save the result, just a check) of
 * the expression and fill repeat indices.
 */

/**
 * @brief Add \p token into the expression \p exp.
 *
 * @param[in] exp Expression to use.
 * @param[in] token Token to add.
 * @param[in] expr_pos Token position in the XPath expression.
 * @param[in] tok_len Token length in the XPath expression.
 * @return 0 on success, -1 on error.
 */
static int
exp_add_token(struct lyxp_expr *exp, enum lyxp_token token, uint16_t expr_pos, uint16_t tok_len)
{
    uint32_t prev;

    if (exp->used == exp->size) {
        prev = exp->size;
        exp->size += LYXP_EXPR_SIZE_STEP;
        if (prev > exp->size) {
            LOGINT(NULL);
            return -1;
        }

        exp->tokens = ly_realloc(exp->tokens, exp->size * sizeof *exp->tokens);
        LY_CHECK_ERR_RETURN(!exp->tokens, LOGMEM(NULL), -1);
        exp->expr_pos = ly_realloc(exp->expr_pos, exp->size * sizeof *exp->expr_pos);
        LY_CHECK_ERR_RETURN(!exp->expr_pos, LOGMEM(NULL), -1);
        exp->tok_len = ly_realloc(exp->tok_len, exp->size * sizeof *exp->tok_len);
        LY_CHECK_ERR_RETURN(!exp->tok_len, LOGMEM(NULL), -1);
    }

    exp->tokens[exp->used] = token;
    exp->expr_pos[exp->used] = expr_pos;
    exp->tok_len[exp->used] = tok_len;
    ++exp->used;
    return 0;
}

/**
 * @brief Look at the next token and check its kind.
 *
 * @param[in] exp Expression to use.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] want_tok Expected token.
 * @param[in] strict Whether the token is strictly required (print error if
 * not the next one) or we simply want to check whether it is the next or not.
 *
 * @return EXIT_SUCCESS if the current token matches the expected one,
 *         -1 otherwise.
 */
static int
exp_check_token(struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t exp_idx, enum lyxp_token want_tok, int strict)
{
    if (exp->used == exp_idx) {
        if (strict) {
            LOGVAL(ctx, LYE_XPATH_EOF, LY_VLOG_NONE, NULL);
        }
        return -1;
    }

    if (want_tok && (exp->tokens[exp_idx] != want_tok)) {
        if (strict) {
            LOGVAL(ctx, LYE_XPATH_INTOK, LY_VLOG_NONE, NULL,
                   print_token(exp->tokens[exp_idx]), &exp->expr[exp->expr_pos[exp_idx]]);
        }
        return -1;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Stack operation push on the repeat array.
 *
 * @param[in] exp Expression to use.
 * @param[in] exp_idx Position in the expresion \p exp.
 * @param[in] repeat_op_idx Index from \p exp of the operator token. This value is pushed.
 */
static void
exp_repeat_push(struct lyxp_expr *exp, uint16_t exp_idx, uint16_t repeat_op_idx)
{
    uint16_t i;

    if (exp->repeat[exp_idx]) {
        for (i = 0; exp->repeat[exp_idx][i]; ++i);
        exp->repeat[exp_idx] = realloc(exp->repeat[exp_idx], (i + 2) * sizeof *exp->repeat[exp_idx]);
        LY_CHECK_ERR_RETURN(!exp->repeat[exp_idx], LOGMEM(NULL), );
        exp->repeat[exp_idx][i] = repeat_op_idx;
        exp->repeat[exp_idx][i + 1] = 0;
    } else {
        exp->repeat[exp_idx] = calloc(2, sizeof *exp->repeat[exp_idx]);
        LY_CHECK_ERR_RETURN(!exp->repeat[exp_idx], LOGMEM(NULL), );
        exp->repeat[exp_idx][0] = repeat_op_idx;
    }
}

/**
 * @brief Reparse Predicate. Logs directly on error.
 *
 * [7] Predicate ::= '[' Expr ']'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_predicate(struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *exp_idx)
{
    if (exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_BRACK1, 1)) {
        return -1;
    }
    ++(*exp_idx);

    if (reparse_or_expr(ctx, exp, exp_idx)) {
        return -1;
    }

    if (exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_BRACK2, 1)) {
        return -1;
    }
    ++(*exp_idx);

    return EXIT_SUCCESS;
}

/**
 * @brief Reparse RelativeLocationPath. Logs directly on error.
 *
 * [4] RelativeLocationPath ::= Step | RelativeLocationPath '/' Step | RelativeLocationPath '//' Step
 * [5] Step ::= '@'? NodeTest Predicate* | '.' | '..'
 * [6] NodeTest ::= NameTest | NodeType '(' ')'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
reparse_relative_location_path(struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *exp_idx)
{
    if (exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_NONE, 1)) {
        return -1;
    }

    goto step;
    do {
        /* '/' or '//' */
        ++(*exp_idx);

        if (exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_NONE, 1)) {
            return -1;
        }
step:
        /* Step */
        switch (exp->tokens[*exp_idx]) {
        case LYXP_TOKEN_DOT:
            ++(*exp_idx);
            break;

        case LYXP_TOKEN_DDOT:
            ++(*exp_idx);
            break;

        case LYXP_TOKEN_AT:
            ++(*exp_idx);

            if (exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_NONE, 1)) {
                return -1;
            }
            if ((exp->tokens[*exp_idx] != LYXP_TOKEN_NAMETEST) && (exp->tokens[*exp_idx] != LYXP_TOKEN_NODETYPE)) {
                LOGVAL(ctx, LYE_XPATH_INTOK, LY_VLOG_NONE, NULL,
                       print_token(exp->tokens[*exp_idx]), &exp->expr[exp->expr_pos[*exp_idx]]);
                return -1;
            }
            /* fall through */
        case LYXP_TOKEN_NAMETEST:
            ++(*exp_idx);
            goto reparse_predicate;
            break;

        case LYXP_TOKEN_NODETYPE:
            ++(*exp_idx);

            /* '(' */
            if (exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_PAR1, 1)) {
                return -1;
            }
            ++(*exp_idx);

            /* ')' */
            if (exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_PAR2, 1)) {
                return -1;
            }
            ++(*exp_idx);

reparse_predicate:
            /* Predicate* */
            while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_BRACK1)) {
                if (reparse_predicate(ctx, exp, exp_idx)) {
                    return -1;
                }
            }
            break;
        default:
            LOGVAL(ctx, LYE_XPATH_INTOK, LY_VLOG_NONE, NULL,
                   print_token(exp->tokens[*exp_idx]), &exp->expr[exp->expr_pos[*exp_idx]]);
            return -1;
        }
    } while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_OPERATOR_PATH));

    return EXIT_SUCCESS;
}

/**
 * @brief Reparse AbsoluteLocationPath. Logs directly on error.
 *
 * [3] AbsoluteLocationPath ::= '/' RelativeLocationPath? | '//' RelativeLocationPath
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_absolute_location_path(struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *exp_idx)
{
    if (exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_OPERATOR_PATH, 1)) {
        return -1;
    }

    /* '/' RelativeLocationPath? */
    if (exp->tok_len[*exp_idx] == 1) {
        /* '/' */
        ++(*exp_idx);

        if (exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_NONE, 0)) {
            return EXIT_SUCCESS;
        }
        switch (exp->tokens[*exp_idx]) {
        case LYXP_TOKEN_DOT:
        case LYXP_TOKEN_DDOT:
        case LYXP_TOKEN_AT:
        case LYXP_TOKEN_NAMETEST:
        case LYXP_TOKEN_NODETYPE:
            if (reparse_relative_location_path(ctx, exp, exp_idx)) {
                return -1;
            }
            /* fall through */
        default:
            break;
        }

    /* '//' RelativeLocationPath */
    } else {
        /* '//' */
        ++(*exp_idx);

        if (reparse_relative_location_path(ctx, exp, exp_idx)) {
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Reparse FunctionCall. Logs directly on error.
 *
 * [9] FunctionCall ::= FunctionName '(' ( Expr ( ',' Expr )* )? ')'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_function_call(struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *exp_idx)
{
    int min_arg_count = -1, max_arg_count, arg_count;
    uint16_t func_exp_idx;

    if (exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_FUNCNAME, 1)) {
        return -1;
    }
    func_exp_idx = *exp_idx;
    switch (exp->tok_len[*exp_idx]) {
    case 3:
        if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "not", 3)) {
            min_arg_count = 1;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "sum", 3)) {
            min_arg_count = 1;
            max_arg_count = 1;
        }
        break;
    case 4:
        if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "lang", 4)) {
            min_arg_count = 1;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "last", 4)) {
            min_arg_count = 0;
            max_arg_count = 0;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "name", 4)) {
            min_arg_count = 0;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "true", 4)) {
            min_arg_count = 0;
            max_arg_count = 0;
        }
        break;
    case 5:
        if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "count", 5)) {
            min_arg_count = 1;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "false", 5)) {
            min_arg_count = 0;
            max_arg_count = 0;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "floor", 5)) {
            min_arg_count = 1;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "round", 5)) {
            min_arg_count = 1;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "deref", 5)) {
            min_arg_count = 1;
            max_arg_count = 1;
        }
        break;
    case 6:
        if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "concat", 6)) {
            min_arg_count = 2;
            max_arg_count = 3;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "number", 6)) {
            min_arg_count = 0;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "string", 6)) {
            min_arg_count = 0;
            max_arg_count = 1;
        }
        break;
    case 7:
        if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "boolean", 7)) {
            min_arg_count = 1;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "ceiling", 7)) {
            min_arg_count = 1;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "current", 7)) {
            min_arg_count = 0;
            max_arg_count = 0;
        }
        break;
    case 8:
        if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "contains", 8)) {
            min_arg_count = 2;
            max_arg_count = 2;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "position", 8)) {
            min_arg_count = 0;
            max_arg_count = 0;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "re-match", 8)) {
            min_arg_count = 2;
            max_arg_count = 2;
        }
        break;
    case 9:
        if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "substring", 9)) {
            min_arg_count = 2;
            max_arg_count = 3;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "translate", 9)) {
            min_arg_count = 3;
            max_arg_count = 3;
        }
        break;
    case 10:
        if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "local-name", 10)) {
            min_arg_count = 0;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "enum-value", 10)) {
            min_arg_count = 1;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "bit-is-set", 10)) {
            min_arg_count = 2;
            max_arg_count = 2;
        }
        break;
    case 11:
        if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "starts-with", 11)) {
            min_arg_count = 2;
            max_arg_count = 2;
        }
        break;
    case 12:
        if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "derived-from", 12)) {
            min_arg_count = 2;
            max_arg_count = 2;
        }
        break;
    case 13:
        if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "namespace-uri", 13)) {
            min_arg_count = 0;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "string-length", 13)) {
            min_arg_count = 0;
            max_arg_count = 1;
        }
        break;
    case 15:
        if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "normalize-space", 15)) {
            min_arg_count = 0;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "substring-after", 15)) {
            min_arg_count = 2;
            max_arg_count = 2;
        }
        break;
    case 16:
        if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "substring-before", 16)) {
            min_arg_count = 2;
            max_arg_count = 2;
        }
        break;
    case 20:
        if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "derived-from-or-self", 20)) {
            min_arg_count = 2;
            max_arg_count = 2;
        }
        break;
    }
    if (min_arg_count == -1) {
        LOGVAL(ctx, LYE_XPATH_INFUNC, LY_VLOG_NONE, NULL, exp->tok_len[*exp_idx], &exp->expr[exp->expr_pos[*exp_idx]]);
        return -1;
    }
    ++(*exp_idx);

    /* '(' */
    if (exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_PAR1, 1)) {
        return -1;
    }
    ++(*exp_idx);

    /* ( Expr ( ',' Expr )* )? */
    arg_count = 0;
    if (exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_NONE, 1)) {
        return -1;
    }
    if (exp->tokens[*exp_idx] != LYXP_TOKEN_PAR2) {
        ++arg_count;
        if (reparse_or_expr(ctx, exp, exp_idx)) {
            return -1;
        }
    }
    while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_COMMA)) {
        ++(*exp_idx);

        ++arg_count;
        if (reparse_or_expr(ctx, exp, exp_idx)) {
            return -1;
        }
    }

    /* ')' */
    if (exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_PAR2, 1)) {
        return -1;
    }
    ++(*exp_idx);

    if ((arg_count < min_arg_count) || (arg_count > max_arg_count)) {
        LOGVAL(ctx, LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, exp->tok_len[func_exp_idx],
               &exp->expr[exp->expr_pos[func_exp_idx]]);
        return -1;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Reparse PathExpr. Logs directly on error.
 *
 * [10] PathExpr ::= LocationPath | PrimaryExpr Predicate*
 *                 | PrimaryExpr Predicate* '/' RelativeLocationPath
 *                 | PrimaryExpr Predicate* '//' RelativeLocationPath
 * [2] LocationPath ::= RelativeLocationPath | AbsoluteLocationPath
 * [8] PrimaryExpr ::= '(' Expr ')' | Literal | Number | FunctionCall
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_path_expr(struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *exp_idx)
{
    if (exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_NONE, 1)) {
        return -1;
    }

    switch (exp->tokens[*exp_idx]) {
    case LYXP_TOKEN_PAR1:
        /* '(' Expr ')' Predicate* */
        ++(*exp_idx);

        if (reparse_or_expr(ctx, exp, exp_idx)) {
            return -1;
        }

        if (exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_PAR2, 1)) {
            return -1;
        }
        ++(*exp_idx);
        goto predicate;
        break;
    case LYXP_TOKEN_DOT:
    case LYXP_TOKEN_DDOT:
    case LYXP_TOKEN_AT:
    case LYXP_TOKEN_NAMETEST:
    case LYXP_TOKEN_NODETYPE:
        /* RelativeLocationPath */
        if (reparse_relative_location_path(ctx, exp, exp_idx)) {
            return -1;
        }
        break;
    case LYXP_TOKEN_FUNCNAME:
        /* FunctionCall */
        if (reparse_function_call(ctx, exp, exp_idx)) {
            return -1;
        }
        goto predicate;
        break;
    case LYXP_TOKEN_OPERATOR_PATH:
        /* AbsoluteLocationPath */
        if (reparse_absolute_location_path(ctx, exp, exp_idx)) {
            return -1;
        }
        break;
    case LYXP_TOKEN_LITERAL:
        /* Literal */
        ++(*exp_idx);
        goto predicate;
        break;
    case LYXP_TOKEN_NUMBER:
        /* Number */
        ++(*exp_idx);
        goto predicate;
        break;
    default:
        LOGVAL(ctx, LYE_XPATH_INTOK, LY_VLOG_NONE, NULL,
               print_token(exp->tokens[*exp_idx]), &exp->expr[exp->expr_pos[*exp_idx]]);
        return -1;
    }

    return EXIT_SUCCESS;

predicate:
    /* Predicate* */
    while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_BRACK1)) {
        if (reparse_predicate(ctx, exp, exp_idx)) {
            return -1;
        }
    }

    /* ('/' or '//') RelativeLocationPath */
    if ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_OPERATOR_PATH)) {

        /* '/' or '//' */
        ++(*exp_idx);

        if (reparse_relative_location_path(ctx, exp, exp_idx)) {
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Reparse UnaryExpr. Logs directly on error.
 *
 * [17] UnaryExpr ::= UnionExpr | '-' UnaryExpr
 * [18] UnionExpr ::= PathExpr | UnionExpr '|' PathExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_unary_expr(struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *exp_idx)
{
    uint16_t prev_exp;

    /* ('-')* */
    prev_exp = *exp_idx;
    while (!exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_OPERATOR_MATH, 0)
            && (exp->expr[exp->expr_pos[*exp_idx]] == '-')) {
        exp_repeat_push(exp, prev_exp, LYXP_EXPR_UNARY);
        ++(*exp_idx);
    }

    /* PathExpr */
    prev_exp = *exp_idx;
    if (reparse_path_expr(ctx, exp, exp_idx)) {
        return -1;
    }

    /* ('|' PathExpr)* */
    while (!exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_OPERATOR_UNI, 0)) {
        exp_repeat_push(exp, prev_exp, LYXP_EXPR_UNION);
        ++(*exp_idx);

        if (reparse_path_expr(ctx, exp, exp_idx)) {
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Reparse AdditiveExpr. Logs directly on error.
 *
 * [15] AdditiveExpr ::= MultiplicativeExpr
 *                     | AdditiveExpr '+' MultiplicativeExpr
 *                     | AdditiveExpr '-' MultiplicativeExpr
 * [16] MultiplicativeExpr ::= UnaryExpr
 *                     | MultiplicativeExpr '*' UnaryExpr
 *                     | MultiplicativeExpr 'div' UnaryExpr
 *                     | MultiplicativeExpr 'mod' UnaryExpr
 *
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_additive_expr(struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *exp_idx)
{
    uint16_t prev_add_exp, prev_mul_exp;

    prev_add_exp = *exp_idx;
    goto reparse_multiplicative_expr;

    /* ('+' / '-' MultiplicativeExpr)* */
    while (!exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_OPERATOR_MATH, 0)
            && ((exp->expr[exp->expr_pos[*exp_idx]] == '+') || (exp->expr[exp->expr_pos[*exp_idx]] == '-'))) {
        exp_repeat_push(exp, prev_add_exp, LYXP_EXPR_ADDITIVE);
        ++(*exp_idx);

reparse_multiplicative_expr:
        /* UnaryExpr */
        prev_mul_exp = *exp_idx;
        if (reparse_unary_expr(ctx, exp, exp_idx)) {
            return -1;
        }

        /* ('*' / 'div' / 'mod' UnaryExpr)* */
        while (!exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_OPERATOR_MATH, 0)
                && ((exp->expr[exp->expr_pos[*exp_idx]] == '*') || (exp->tok_len[*exp_idx] == 3))) {
            exp_repeat_push(exp, prev_mul_exp, LYXP_EXPR_MULTIPLICATIVE);
            ++(*exp_idx);

            if (reparse_unary_expr(ctx, exp, exp_idx)) {
                return -1;
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Reparse EqualityExpr. Logs directly on error.
 *
 * [13] EqualityExpr ::= RelationalExpr | EqualityExpr '=' RelationalExpr
 *                     | EqualityExpr '!=' RelationalExpr
 * [14] RelationalExpr ::= AdditiveExpr
 *                       | RelationalExpr '<' AdditiveExpr
 *                       | RelationalExpr '>' AdditiveExpr
 *                       | RelationalExpr '<=' AdditiveExpr
 *                       | RelationalExpr '>=' AdditiveExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_equality_expr(struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *exp_idx)
{
    uint16_t prev_eq_exp, prev_rel_exp;

    prev_eq_exp = *exp_idx;
    goto reparse_additive_expr;

    /* ('=' / '!=' RelationalExpr)* */
    while (!exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_OPERATOR_COMP, 0)
            && ((exp->expr[exp->expr_pos[*exp_idx]] == '=') || (exp->expr[exp->expr_pos[*exp_idx]] == '!'))) {
        exp_repeat_push(exp, prev_eq_exp, LYXP_EXPR_EQUALITY);
        ++(*exp_idx);

reparse_additive_expr:
        /* AdditiveExpr */
        prev_rel_exp = *exp_idx;
        if (reparse_additive_expr(ctx, exp, exp_idx)) {
            return -1;
        }

        /* ('<' / '>' / '<=' / '>=' AdditiveExpr)* */
        while (!exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_OPERATOR_COMP, 0)
                && ((exp->expr[exp->expr_pos[*exp_idx]] == '<') || (exp->expr[exp->expr_pos[*exp_idx]] == '>'))) {
            exp_repeat_push(exp, prev_rel_exp, LYXP_EXPR_RELATIONAL);
            ++(*exp_idx);

            if (reparse_additive_expr(ctx, exp, exp_idx)) {
                return -1;
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Reparse OrExpr. Logs directly on error.
 *
 * [11] OrExpr ::= AndExpr | OrExpr 'or' AndExpr
 * [12] AndExpr ::= EqualityExpr | AndExpr 'and' EqualityExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_or_expr(struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *exp_idx)
{
    uint16_t prev_or_exp, prev_and_exp;

    prev_or_exp = *exp_idx;
    goto reparse_equality_expr;

    /* ('or' AndExpr)* */
    while (!exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_OPERATOR_LOG, 0) && (exp->tok_len[*exp_idx] == 2)) {
        exp_repeat_push(exp, prev_or_exp, LYXP_EXPR_OR);
        ++(*exp_idx);

reparse_equality_expr:
        /* EqualityExpr */
        prev_and_exp = *exp_idx;
        if (reparse_equality_expr(ctx, exp, exp_idx)) {
            return -1;
        }

        /* ('and' EqualityExpr)* */
        while (!exp_check_token(ctx, exp, *exp_idx, LYXP_TOKEN_OPERATOR_LOG, 0) && (exp->tok_len[*exp_idx] == 3)) {
            exp_repeat_push(exp, prev_and_exp, LYXP_EXPR_AND);
            ++(*exp_idx);

            if (reparse_equality_expr(ctx, exp, exp_idx)) {
                return -1;
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Parse NCName.
 *
 * @param[in] ncname Name to parse.
 *
 * @return Length of \p ncname valid characters.
 */
static uint16_t
parse_ncname(struct ly_ctx *ctx, const char *ncname)
{
    uint16_t parsed = 0;
    int uc;
    unsigned int size;

    uc = lyxml_getutf8(ctx, &ncname[parsed], &size);
    if (!is_xmlnamestartchar(uc) || (uc == ':')) {
       return parsed;
    }

    do {
        parsed += size;
        if (!ncname[parsed]) {
            break;
        }
        uc = lyxml_getutf8(ctx, &ncname[parsed], &size);
    } while (is_xmlnamechar(uc) && (uc != ':'));

    return parsed;
}

struct lyxp_expr *
lyxp_parse_expr(struct ly_ctx *ctx, const char *expr)
{
    struct lyxp_expr *ret;
    uint16_t parsed = 0, tok_len, ncname_len;
    enum lyxp_token tok_type;
    int prev_function_check = 0;

    if (strlen(expr) > UINT16_MAX) {
        LOGERR(ctx, LY_EINVAL, "XPath expression cannot be longer than %ud characters.", UINT16_MAX);
        return NULL;
    }

    /* init lyxp_expr structure */
    ret = calloc(1, sizeof *ret);
    LY_CHECK_ERR_GOTO(!ret, LOGMEM(ctx), error);
    ret->expr = strdup(expr);
    LY_CHECK_ERR_GOTO(!ret->expr, LOGMEM(ctx), error);
    ret->used = 0;
    ret->size = LYXP_EXPR_SIZE_START;
    ret->tokens = malloc(ret->size * sizeof *ret->tokens);
    LY_CHECK_ERR_GOTO(!ret->tokens, LOGMEM(ctx), error);

    ret->expr_pos = malloc(ret->size * sizeof *ret->expr_pos);
    LY_CHECK_ERR_GOTO(!ret->expr_pos, LOGMEM(ctx), error);

    ret->tok_len = malloc(ret->size * sizeof *ret->tok_len);
    LY_CHECK_ERR_GOTO(!ret->tok_len, LOGMEM(ctx), error);

    while (is_xmlws(expr[parsed])) {
        ++parsed;
    }

    do {
        if (expr[parsed] == '(') {

            /* '(' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_PAR1;

            if (prev_function_check && ret->used && (ret->tokens[ret->used - 1] == LYXP_TOKEN_NAMETEST)) {
                /* it is a NodeType/FunctionName after all */
                if (((ret->tok_len[ret->used - 1] == 4)
                        && (!strncmp(&expr[ret->expr_pos[ret->used - 1]], "node", 4)
                        || !strncmp(&expr[ret->expr_pos[ret->used - 1]], "text", 4))) ||
                        ((ret->tok_len[ret->used - 1] == 7)
                        && !strncmp(&expr[ret->expr_pos[ret->used - 1]], "comment", 7))) {
                    ret->tokens[ret->used - 1] = LYXP_TOKEN_NODETYPE;
                } else {
                    ret->tokens[ret->used - 1] = LYXP_TOKEN_FUNCNAME;
                }
                prev_function_check = 0;
            }

        } else if (expr[parsed] == ')') {

            /* ')' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_PAR2;

        } else if (expr[parsed] == '[') {

            /* '[' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_BRACK1;

        } else if (expr[parsed] == ']') {

            /* ']' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_BRACK2;

        } else if (!strncmp(&expr[parsed], "..", 2)) {

            /* '..' */
            tok_len = 2;
            tok_type = LYXP_TOKEN_DDOT;

        } else if ((expr[parsed] == '.') && (!isdigit(expr[parsed + 1]))) {

            /* '.' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_DOT;

        } else if (expr[parsed] == '@') {

            /* '@' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_AT;

        } else if (expr[parsed] == ',') {

            /* ',' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_COMMA;

        } else if (expr[parsed] == '\'') {

            /* Literal with ' */
            for (tok_len = 1; (expr[parsed + tok_len] != '\0') && (expr[parsed + tok_len] != '\''); ++tok_len);
            if (expr[parsed + tok_len] == '\0') {
                LOGVAL(ctx, LYE_XPATH_NOEND, LY_VLOG_NONE, NULL, expr[parsed], &expr[parsed]);
                goto error;
            }
            ++tok_len;
            tok_type = LYXP_TOKEN_LITERAL;

        } else if (expr[parsed] == '\"') {

            /* Literal with " */
            for (tok_len = 1; (expr[parsed + tok_len] != '\0') && (expr[parsed + tok_len] != '\"'); ++tok_len);
            if (expr[parsed + tok_len] == '\0') {
                LOGVAL(ctx, LYE_XPATH_NOEND, LY_VLOG_NONE, NULL, expr[parsed], &expr[parsed]);
                goto error;
            }
            ++tok_len;
            tok_type = LYXP_TOKEN_LITERAL;

        } else if ((expr[parsed] == '.') || (isdigit(expr[parsed]))) {

            /* Number */
            for (tok_len = 0; isdigit(expr[parsed + tok_len]); ++tok_len);
            if (expr[parsed + tok_len] == '.') {
                ++tok_len;
                for (; isdigit(expr[parsed + tok_len]); ++tok_len);
            }
            tok_type = LYXP_TOKEN_NUMBER;

        } else if (expr[parsed] == '/') {

            /* Operator '/', '//' */
            if (!strncmp(&expr[parsed], "//", 2)) {
                tok_len = 2;
            } else {
                tok_len = 1;
            }
            tok_type = LYXP_TOKEN_OPERATOR_PATH;

        } else if  (!strncmp(&expr[parsed], "!=", 2) || !strncmp(&expr[parsed], "<=", 2)
                || !strncmp(&expr[parsed], ">=", 2)) {

            /* Operator '!=', '<=', '>=' */
            tok_len = 2;
            tok_type = LYXP_TOKEN_OPERATOR_COMP;

        } else if (expr[parsed] == '|') {

            /* Operator '|' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_OPERATOR_UNI;

        } else if ((expr[parsed] == '+') || (expr[parsed] == '-')) {

            /* Operator '+', '-' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_OPERATOR_MATH;

        } else if ((expr[parsed] == '=') || (expr[parsed] == '<') || (expr[parsed] == '>')) {

            /* Operator '=', '<', '>' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_OPERATOR_COMP;

        } else if (ret->used && (ret->tokens[ret->used - 1] != LYXP_TOKEN_AT)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_PAR1)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_BRACK1)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_COMMA)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_OPERATOR_LOG)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_OPERATOR_COMP)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_OPERATOR_MATH)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_OPERATOR_UNI)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_OPERATOR_PATH)) {

            /* Operator '*', 'or', 'and', 'mod', or 'div' */
            if (expr[parsed] == '*') {
                tok_len = 1;
                tok_type = LYXP_TOKEN_OPERATOR_MATH;

            } else if (!strncmp(&expr[parsed], "or", 2)) {
                tok_len = 2;
                tok_type = LYXP_TOKEN_OPERATOR_LOG;

            } else if (!strncmp(&expr[parsed], "and", 3)) {
                tok_len = 3;
                tok_type = LYXP_TOKEN_OPERATOR_LOG;

            } else if (!strncmp(&expr[parsed], "mod", 3) || !strncmp(&expr[parsed], "div", 3)) {
                tok_len = 3;
                tok_type = LYXP_TOKEN_OPERATOR_MATH;

            } else {
                LOGVAL(ctx, LYE_INCHAR, LY_VLOG_NONE, NULL, expr[parsed], &expr[parsed]);
                if (prev_function_check) {
                    LOGVAL(ctx, LYE_SPEC, LY_VLOG_PREV, NULL, "Perhaps \"%.*s\" is supposed to be a function call.",
                           ret->tok_len[ret->used - 1], &ret->expr[ret->expr_pos[ret->used - 1]]);
                }
                goto error;
            }
        } else if (expr[parsed] == '*') {

            /* NameTest '*' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_NAMETEST;

        } else {

            /* NameTest (NCName ':' '*' | QName) or NodeType/FunctionName */
            ncname_len = parse_ncname(ctx, &expr[parsed]);
            if (!ncname_len) {
                LOGVAL(ctx, LYE_INCHAR, LY_VLOG_NONE, NULL, expr[parsed], &expr[parsed]);
                goto error;
            }
            tok_len = ncname_len;

            if (expr[parsed + tok_len] == ':') {
                ++tok_len;
                if (expr[parsed + tok_len] == '*') {
                    ++tok_len;
                } else {
                    ncname_len = parse_ncname(ctx, &expr[parsed + tok_len]);
                    if (!ncname_len) {
                        LOGVAL(ctx, LYE_INCHAR, LY_VLOG_NONE, NULL, expr[parsed], &expr[parsed]);
                        goto error;
                    }
                    tok_len += ncname_len;
                }
                /* remove old flag to prevent ambiguities */
                prev_function_check = 0;
                tok_type = LYXP_TOKEN_NAMETEST;
            } else {
                /* there is no prefix so it can still be NodeType/FunctionName, we can't finally decide now */
                prev_function_check = 1;
                tok_type = LYXP_TOKEN_NAMETEST;
            }
        }

        /* store the token, move on to the next one */
        if (exp_add_token(ret, tok_type, parsed, tok_len)) {
            goto error;
        }
        parsed += tok_len;
        while (is_xmlws(expr[parsed])) {
            ++parsed;
        }

    } while (expr[parsed]);

    /* prealloc repeat */
    ret->repeat = calloc(ret->size, sizeof *ret->repeat);
    LY_CHECK_ERR_GOTO(!ret->repeat, LOGMEM(ctx), error);

    return ret;

error:
    lyxp_expr_free(ret);
    return NULL;
}

/*
 * warn functions
 *
 * Warn functions check specific reasonable conditions for schema XPath
 * and print a warning if they are not satisfied.
 */

/**
 * @brief Get the last-added schema node that is currently in the context.
 *
 * @param[in] set Set to search in.
 *
 * @return Last-added schema context node, NULL if no node is in context.
 */
static struct lys_node *
warn_get_snode_in_ctx(struct lyxp_set *set)
{
    uint32_t i;

    if (!set || (set->type != LYXP_SET_SNODE_SET)) {
        return NULL;
    }

    i = set->used;
    do {
        --i;
        if (set->val.snodes[i].in_ctx == 1) {
            /* if there are more, simply return the first found (last added) */
            return set->val.snodes[i].snode;
        }
    } while (i);

    return NULL;
}

/**
 * @brief Test whether a type is numeric - integer type or decimal64.
 *
 * @return 1 if numeric, 0 otherwise.
 */
static int
warn_is_numeric_type(struct lys_type *type)
{
    struct lys_node *node;
    struct lys_type *t = NULL;
    int found = 0, ret;

    switch (type->base) {
    case LY_TYPE_DEC64:
    case LY_TYPE_INT8:
    case LY_TYPE_UINT8:
    case LY_TYPE_INT16:
    case LY_TYPE_UINT16:
    case LY_TYPE_INT32:
    case LY_TYPE_UINT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
        return 1;
    case LY_TYPE_UNION:
        while ((t = lyp_get_next_union_type(type, t, &found))) {
            found = 0;
            ret = warn_is_numeric_type(t);
            if (ret) {
                /* found a suitable type */
                return 1;
            }
        }
        /* did not find any suitable type */
        return 0;
    case LY_TYPE_LEAFREF:
        if (!type->info.lref.target) {
            /* we may be in a grouping (and not directly in a typedef) */
            assert(&((struct lys_node_leaf *)type->parent)->type == type);
            for (node = ((struct lys_node *)type->parent); node && (node->nodetype != LYS_GROUPING); node = node->parent);
            if (!node) {
                LOGINT(((struct lys_node *)type->parent)->module->ctx);
            }
            return 0;
        }
        return warn_is_numeric_type(&type->info.lref.target->type);
    default:
        return 0;
    }
}

/**
 * @brief Test whether a type is string-like - no integers, decimal64 or binary.
 *
 * @return 1 if string, 0 otherwise.
 */
static int
warn_is_string_type(struct lys_type *type)
{
    struct lys_type *t = NULL;
    int found = 0, ret;

    switch (type->base) {
    case LY_TYPE_BITS:
    case LY_TYPE_ENUM:
    case LY_TYPE_IDENT:
    case LY_TYPE_INST:
    case LY_TYPE_STRING:
        return 1;
    case LY_TYPE_UNION:
        while ((t = lyp_get_next_union_type(type, t, &found))) {
            found = 0;
            ret = warn_is_string_type(t);
            if (ret) {
                /* found a suitable type */
                return 1;
            }
        }
        /* did not find any suitable type */
        return 0;
    case LY_TYPE_LEAFREF:
        if (!type->info.lref.target) {
            /* we are in a grouping */
            return 0;
        }
        return warn_is_string_type(&type->info.lref.target->type);
    default:
        return 0;
    }
}

/**
 * @brief Test whether a type is one specific type.
 *
 * @return 1 if it is, 0 otherwise.
 */
static int
warn_is_specific_type(struct lys_type *type, LY_DATA_TYPE base)
{
    struct lys_type *t = NULL;
    int found = 0, ret;

    if (type->base == base) {
        return 1;
    } else if (type->base == LY_TYPE_UNION) {
        while ((t = lyp_get_next_union_type(type, t, &found))) {
            found = 0;
            ret = warn_is_specific_type(t, base);
            if (ret) {
                /* found a suitable type */
                return 1;
            }
        }
        /* did not find any suitable type */
        return 0;
    } else if (type->base == LY_TYPE_LEAFREF) {
        if (!type->info.lref.target) {
            /* we are in a grouping */
            return 1;
        }
        return warn_is_specific_type(&type->info.lref.target->type, base);
    }

    return 0;
}

static struct lys_type *
warn_is_equal_type_next_type(struct lys_type *type, struct lys_type *prev_type)
{
    int found = 0;

    switch (type->base) {
    case LY_TYPE_UNION:
        /* this can, unfortunately, return leafref */
        return lyp_get_next_union_type(type, prev_type, &found);
    case LY_TYPE_LEAFREF:
        if (!type->info.lref.target) {
            /* we are in a grouping */
            return type;
        }
        return warn_is_equal_type_next_type(&type->info.lref.target->type, prev_type);
    default:
        if (prev_type) {
            assert(type == prev_type);
            return NULL;
        } else {
            return type;
        }
    }
}

/**
 * @brief Test whether 2 types have a common type.
 *
 * @return 1 if they do, 0 otherwise.
 */
static int
warn_is_equal_type(struct lys_type *type1, struct lys_type *type2)
{
    struct lys_type *t1, *t2;

    t1 = NULL;
    while ((t1 = warn_is_equal_type_next_type(type1, t1))) {
        if (t1->base == LY_TYPE_LEAFREF) {
            /* we do not check unions with leafrefs, that is just too much... */
            return 1;
        }

        t2 = NULL;
        while ((t2 = warn_is_equal_type_next_type(type2, t2))) {
            if (t2->base == LY_TYPE_LEAFREF) {
                return 1;
            }

            if (t2->base == t1->base) {
                /* match found */
                return 1;
            }
        }
    }

    return 0;
}

/**
 * @brief Check both operands of comparison operators.
 *
 * @param[in] ctx Context for errors.
 * @param[in] set1 First operand set.
 * @param[in] set2 Second operand set.
 * @param[in] numbers_only Whether accept only numbers or other types are fine too (for '=' and '!=').
 * @param[in] expr Start of the expression to print with the warning.
 */
static void
warn_operands(struct ly_ctx *ctx, struct lyxp_set *set1, struct lyxp_set *set2, int numbers_only, const char *expr, uint16_t expr_pos)
{
    struct lys_node_leaf *node1, *node2;
    int leaves = 1, warning = 0;

    node1 = (struct lys_node_leaf *)warn_get_snode_in_ctx(set1);
    node2 = (struct lys_node_leaf *)warn_get_snode_in_ctx(set2);

    if (!node1 && !node2) {
        /* no node-sets involved, nothing to do */
        return;
    }

    if (node1) {
        if (!(node1->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            LOGWRN(ctx, "Node type %s \"%s\" used as operand.", strnodetype(node1->nodetype), node1->name);
            warning = 1;
            leaves = 0;
        } else if (numbers_only && !warn_is_numeric_type(&node1->type)) {
            LOGWRN(ctx, "Node \"%s\" is not of a numeric type, but used where it was expected.", node1->name);
            warning = 1;
        }
    }

    if (node2) {
        if (!(node2->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            LOGWRN(ctx, "Node type %s \"%s\" used as operand.", strnodetype(node2->nodetype), node2->name);
            warning = 1;
            leaves = 0;
        } else if (numbers_only && !warn_is_numeric_type(&node2->type)) {
            LOGWRN(ctx, "Node \"%s\" is not of a numeric type, but used where it was expected.", node2->name);
            warning = 1;
        }
    }

    if (node1 && node2 && leaves && !numbers_only) {
        if ((warn_is_numeric_type(&node1->type) && !warn_is_numeric_type(&node2->type))
                || (!warn_is_numeric_type(&node1->type) && warn_is_numeric_type(&node2->type))
                || (!warn_is_numeric_type(&node1->type) && !warn_is_numeric_type(&node2->type)
                && !warn_is_equal_type(&node1->type, &node2->type))) {
            LOGWRN(ctx, "Incompatible types of operands \"%s\" and \"%s\" for comparison.", node1->name, node2->name);
            warning = 1;
        }
    }

    if (warning) {
        LOGWRN(ctx, "Previous warning generated by XPath subexpression[%u] \"%.20s\".", expr_pos, expr + expr_pos);
    }
}

/**
 * @brief Check that a value is valid for a leaf. If not applicable, does nothing.
 *
 * @param[in] ctx Context for errors.
 * @param[in] exp Parsed XPath expression.
 * @param[in] set Set with the leaf/leaf-list.
 * @param[in] val_exp Index of the value (literal/number) in \p exp.
 * @param[in] equal_exp Index of the start of the equality expression in \p exp.
 * @param[in] last_equal_exp Index of the end of the equality expression in \p exp.
 */
static void
warn_equality_value(struct ly_ctx *ctx, struct lyxp_expr *exp, struct lyxp_set *set, uint16_t val_exp, uint16_t equal_exp,
                    uint16_t last_equal_exp)
{
    struct lys_node *snode;
    char *value;
    int ret;
    enum int_log_opts prev_ilo;

    if ((snode = warn_get_snode_in_ctx(set)) && (snode->nodetype & (LYS_LEAF | LYS_LEAFLIST))
            && ((exp->tokens[val_exp] == LYXP_TOKEN_LITERAL) || (exp->tokens[val_exp] == LYXP_TOKEN_NUMBER))) {
        /* check that the node can have the specified value */
        if (exp->tokens[val_exp] == LYXP_TOKEN_LITERAL) {
            value = strndup(exp->expr + exp->expr_pos[val_exp] + 1, exp->tok_len[val_exp] - 2);
        } else {
            value = strndup(exp->expr + exp->expr_pos[val_exp], exp->tok_len[val_exp]);
        }
        if (!value) {
            LOGMEM(ctx);
            return;
        }

        if ((((struct lys_node_leaf *)snode)->type.base == LY_TYPE_IDENT) && !strchr(value, ':')) {
            LOGWRN(ctx, "Identityref \"%s\" comparison with identity \"%s\" without prefix, consider adding"
                   " a prefix or best using \"derived-from(-or-self)()\" functions.", snode->name, value);
            LOGWRN(ctx, "Previous warning generated by XPath subexpression[%u] \"%.*s\".", exp->expr_pos[equal_exp],
                   (exp->expr_pos[last_equal_exp] - exp->expr_pos[equal_exp]) + exp->tok_len[last_equal_exp],
                   exp->expr + exp->expr_pos[equal_exp]);
        }

        /* we are unable to check identityref validity if this module (and any required imports) are not implemented */
        if ((((struct lys_node_leaf *)snode)->type.base != LY_TYPE_IDENT) || lys_node_module(snode)->implemented) {
            /* we want to print our message and more importantly a warning, not an error */
            ly_ilo_change(NULL, ILO_ERR2WRN, &prev_ilo, NULL);
            ret = lyd_validate_value(snode, value);
            ly_ilo_restore(NULL, prev_ilo, NULL, 0);
            if (ret) {
                LOGWRN(ctx, "Previous warning generated by XPath subexpression[%u] \"%.*s\".", exp->expr_pos[equal_exp],
                    (exp->expr_pos[last_equal_exp] - exp->expr_pos[equal_exp]) + exp->tok_len[last_equal_exp],
                    exp->expr + exp->expr_pos[equal_exp]);
            }
        }
        free(value);
    }
}

/*
 * XPath functions
 */

/**
 * @brief Execute the YANG 1.1 bit-is-set(node-set, string) function. Returns LYXP_SET_BOOLEAN
 *        depending on whether the first node bit value from the second argument is set.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_bit_is_set(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *cur_node, struct lys_module *local_mod,
                 struct lyxp_set *set, int options)
{
    struct lyd_node_leaf_list *leaf;
    struct lys_node_leaf *sleaf;
    int i, bits_count, ret = EXIT_SUCCESS;

    if (options & LYXP_SNODE_ALL) {
        if ((args[0]->type != LYXP_SET_SNODE_SET) || !(sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[0]))) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s not a node-set as expected.", __func__);
            ret = EXIT_FAILURE;
        } else if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
            ret = EXIT_FAILURE;
        } else if (!warn_is_specific_type(&sleaf->type, LY_TYPE_BITS)) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of type \"bits\".", __func__, sleaf->name);
            ret = EXIT_FAILURE;
        }

        if ((args[1]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if ((args[0]->type != LYXP_SET_NODE_SET) && (args[0]->type != LYXP_SET_EMPTY)) {
        LOGVAL(local_mod->ctx, LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "bit-is-set(node-set, string)");
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }

    set_fill_boolean(set, 0);
    if (args[0]->type == LYXP_SET_NODE_SET) {
        leaf = (struct lyd_node_leaf_list *)args[0]->val.nodes[0].node;
        if ((leaf->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))
                && (((struct lys_node_leaf *)leaf->schema)->type.base == LY_TYPE_BITS)) {
            bits_count = ((struct lys_node_leaf *)leaf->schema)->type.info.bits.count;
            for (i = 0; i < bits_count; ++i) {
                if (leaf->value.bit[i] && ly_strequal(leaf->value.bit[i]->name, args[1]->val.str, 0)) {
                    set_fill_boolean(set, 1);
                    break;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath boolean(object) function. Returns LYXP_SET_BOOLEAN
 *        with the argument converted to boolean.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_boolean(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *cur_node, struct lys_module *local_mod,
              struct lyxp_set *set, int options)
{
    if (options & LYXP_SNODE_ALL) {
        set_snode_clear_ctx(set);
        return EXIT_SUCCESS;
    }

    lyxp_set_cast(args[0], LYXP_SET_BOOLEAN, cur_node, local_mod, options);
    set_fill_set(set, args[0]);

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath ceiling(number) function. Returns LYXP_SET_NUMBER
 *        with the first argument rounded up to the nearest integer.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_ceiling(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *cur_node, struct lys_module *local_mod,
              struct lyxp_set *set, int options)
{
    struct lys_node_leaf *sleaf;
    int ret = EXIT_SUCCESS;

    if (options & LYXP_SNODE_ALL) {
        if ((args[0]->type != LYXP_SET_SNODE_SET) || !(sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[0]))) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s not a node-set as expected.", __func__);
            ret = EXIT_FAILURE;
        } else if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
            ret = EXIT_FAILURE;
        } else if (!warn_is_specific_type(&sleaf->type, LY_TYPE_DEC64)) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of type \"decimal64\".", __func__, sleaf->name);
            ret = EXIT_FAILURE;
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_NUMBER, cur_node, local_mod, options)) {
        return -1;
    }
    if ((long long)args[0]->val.num != args[0]->val.num) {
        set_fill_number(set, ((long long)args[0]->val.num) + 1);
    } else {
        set_fill_number(set, args[0]->val.num);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath concat(string, string, string*) function.
 *        Returns LYXP_SET_STRING with the concatenation of all the arguments.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_concat(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lys_module *local_mod,
             struct lyxp_set *set, int options)
{
    uint16_t i;
    char *str = NULL;
    size_t used = 1;
    int ret = EXIT_SUCCESS;
    struct lys_node_leaf *sleaf;

    if (options & LYXP_SNODE_ALL) {
        for (i = 0; i < arg_count; ++i) {
            if ((args[i]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[i]))) {
                if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                    LOGWRN(local_mod->ctx, "Argument #%u of %s is a %s node \"%s\".",
                           i + 1, __func__, strnodetype(sleaf->nodetype), sleaf->name);
                    ret = EXIT_FAILURE;
                } else if (!warn_is_string_type(&sleaf->type)) {
                    LOGWRN(local_mod->ctx, "Argument #%u of %s is node \"%s\", not of string-type.", __func__, i + 1, sleaf->name);
                    ret = EXIT_FAILURE;
                }
            }
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    for (i = 0; i < arg_count; ++i) {
        if (lyxp_set_cast(args[i], LYXP_SET_STRING, cur_node, local_mod, options)) {
            free(str);
            return -1;
        }

        str = ly_realloc(str, (used + strlen(args[i]->val.str)) * sizeof(char));
        LY_CHECK_ERR_RETURN(!str, LOGMEM(local_mod->ctx), -1);
        strcpy(str + used - 1, args[i]->val.str);
        used += strlen(args[i]->val.str);
    }

    /* free, kind of */
    lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, local_mod, options);
    set->type = LYXP_SET_STRING;
    set->val.str = str;

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath contains(string, string) function.
 *        Returns LYXP_SET_BOOLEAN whether the second argument can
 *        be found in the first or not.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_contains(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *cur_node, struct lys_module *local_mod,
               struct lyxp_set *set, int options)
{
    struct lys_node_leaf *sleaf;
    int ret = EXIT_SUCCESS;

    if (options & LYXP_SNODE_ALL) {
        if ((args[0]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }

        if ((args[1]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }

    if (strstr(args[0]->val.str, args[1]->val.str)) {
        set_fill_boolean(set, 1);
    } else {
        set_fill_boolean(set, 0);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath count(node-set) function. Returns LYXP_SET_NUMBER
 *        with the size of the node-set from the argument.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_count(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *UNUSED(cur_node),
            struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    struct lys_node *snode = NULL;
    int ret = EXIT_SUCCESS;

    if (options & LYXP_SNODE_ALL) {
        if ((args[0]->type != LYXP_SET_SNODE_SET) || !(snode = warn_get_snode_in_ctx(args[0]))) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s not a node-set as expected.", __func__);
            ret = EXIT_FAILURE;
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if (args[0]->type == LYXP_SET_EMPTY) {
        set_fill_number(set, 0);
        return EXIT_SUCCESS;
    }

    if (args[0]->type != LYXP_SET_NODE_SET) {
        LOGVAL(local_mod->ctx, LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "count(node-set)");
        return -1;
    }

    set_fill_number(set, args[0]->used);
    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath current() function. Returns LYXP_SET_NODE_SET
 *        with the context with the intial node.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_current(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lys_module *local_mod,
              struct lyxp_set *set, int options)
{
    if (arg_count || args) {
        LOGVAL(local_mod->ctx, LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "current()");
        return -1;
    }

    if (options & LYXP_SNODE_ALL) {
        set_snode_clear_ctx(set);

        set_snode_insert_node(set, (struct lys_node *)cur_node, LYXP_NODE_ELEM);
    } else {
        lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, local_mod, options);

        /* position is filled later */
        set_insert_node(set, cur_node, 0, LYXP_NODE_ELEM, 0);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the YANG 1.1 deref(node-set) function. Returns LYXP_SET_NODE_SET with either
 *        leafref or instance-identifier target node(s).
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_deref(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *cur_node, struct lys_module *local_mod,
            struct lyxp_set *set, int options)
{
    struct lyd_node_leaf_list *leaf;
    struct lys_node_leaf *sleaf;
    int ret = EXIT_SUCCESS;

    if (options & LYXP_SNODE_ALL) {
        if ((args[0]->type != LYXP_SET_SNODE_SET) || !(sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[0]))) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s not a node-set as expected.", __func__);
            ret = EXIT_FAILURE;
        } else if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
            ret = EXIT_FAILURE;
        } else if (!warn_is_specific_type(&sleaf->type, LY_TYPE_LEAFREF) && !warn_is_specific_type(&sleaf->type, LY_TYPE_INST)) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of type \"leafref\" neither \"instance-identifier\".",
                   __func__, sleaf->name);
            ret = EXIT_FAILURE;
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if ((args[0]->type != LYXP_SET_NODE_SET) && (args[0]->type != LYXP_SET_EMPTY)) {
        LOGVAL(local_mod->ctx, LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "deref(node-set)");
        return -1;
    }

    lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, local_mod, options);
    if (args[0]->type != LYXP_SET_EMPTY) {
        leaf = (struct lyd_node_leaf_list *)args[0]->val.nodes[0].node;
        sleaf = (struct lys_node_leaf *)leaf->schema;
        if ((sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))
                && ((sleaf->type.base == LY_TYPE_LEAFREF) || (sleaf->type.base == LY_TYPE_INST))) {
            if (leaf->value_flags & LY_VALUE_UNRES) {
                /* this is bad */
                LOGVAL(local_mod->ctx, LYE_SPEC, LY_VLOG_LYD, args[0]->val.nodes[0].node,
                       "Trying to dereference an unresolved leafref or instance-identifier.");
                return -1;
            }
            /* works for both leafref and instid */
            set_insert_node(set, leaf->value.leafref, 0, LYXP_NODE_ELEM, 0);
        }
    }

    return EXIT_SUCCESS;
}

/* return 0 - match, 1 - mismatch */
static int
xpath_derived_from_ident_cmp(struct lys_ident *ident, const char *ident_str)
{
    const char *ptr;
    int len;

    ptr = strchr(ident_str, ':');
    if (ptr) {
        len = ptr - ident_str;
        if (strncmp(ident->module->name, ident_str, len)
                || ident->module->name[len]) {
            /* module name mismatch BUG we expect JSON format prefix, but if the 2nd argument was
             * not a literal, we may easily be mistaken */
            return 1;
        }
        ++ptr;
    } else {
        ptr = ident_str;
    }

    len = strlen(ptr);
    if (strncmp(ident->name, ptr, len) || ident->name[len]) {
        /* name mismatch */
        return 1;
    }

    return 0;
}

/**
 * @brief Execute the YANG 1.1 derived-from(node-set, string) function. Returns LYXP_SET_BOOLEAN depending
 *        on whether the first argument nodes contain a node of an identity derived from the second
 *        argument identity.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_derived_from(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *cur_node, struct lys_module *local_mod,
                   struct lyxp_set *set, int options)
{
    uint16_t i, j;
    struct lyd_node_leaf_list *leaf;
    struct lys_node_leaf *sleaf;
    int ret = EXIT_SUCCESS;

    if (options & LYXP_SNODE_ALL) {
        if ((args[0]->type != LYXP_SET_SNODE_SET) || !(sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[0]))) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s not a node-set as expected.", __func__);
            ret = EXIT_FAILURE;
        } else if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
            ret = EXIT_FAILURE;
        } else if (!warn_is_specific_type(&sleaf->type, LY_TYPE_IDENT)) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of type \"identityref\".", __func__, sleaf->name);
            ret = EXIT_FAILURE;
        }

        if ((args[1]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if ((args[0]->type != LYXP_SET_NODE_SET) && (args[0]->type != LYXP_SET_EMPTY)) {
        LOGVAL(local_mod->ctx, LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "derived-from(node-set, string)");
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }

    set_fill_boolean(set, 0);
    if (args[0]->type != LYXP_SET_EMPTY) {
        for (i = 0; i < args[0]->used; ++i) {
            leaf = (struct lyd_node_leaf_list *)args[0]->val.nodes[i].node;
            sleaf = (struct lys_node_leaf *)leaf->schema;
            if ((sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST)) && (sleaf->type.base == LY_TYPE_IDENT)) {
                for (j = 0; j < leaf->value.ident->base_size; ++j) {
                    if (!xpath_derived_from_ident_cmp(leaf->value.ident->base[j], args[1]->val.str)) {
                        set_fill_boolean(set, 1);
                        break;
                    }
                }

                if (j < leaf->value.ident->base_size) {
                    break;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the YANG 1.1 derived-from-or-self(node-set, string) function. Returns LYXP_SET_BOOLEAN depending
 *        on whether the first argument nodes contain a node of an identity that either is or is derived from
 *        the second argument identity.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_derived_from_or_self(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *cur_node,
                           struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    uint16_t i, j;
    struct lyd_node_leaf_list *leaf;
    struct lys_node_leaf *sleaf;
    int ret = EXIT_SUCCESS;

    if (options & LYXP_SNODE_ALL) {
        if ((args[0]->type != LYXP_SET_SNODE_SET) || !(sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[0]))) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s not a node-set as expected.", __func__);
            ret = EXIT_FAILURE;
        } else if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
            ret = EXIT_FAILURE;
        } else if (!warn_is_specific_type(&sleaf->type, LY_TYPE_IDENT)) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of type \"identityref\".", __func__, sleaf->name);
            ret = EXIT_FAILURE;
        }

        if ((args[1]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if ((args[0]->type != LYXP_SET_NODE_SET) && (args[0]->type != LYXP_SET_EMPTY)) {
        LOGVAL(local_mod->ctx, LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "derived-from-or-self(node-set, string)");
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }

    set_fill_boolean(set, 0);
    if (args[0]->type != LYXP_SET_EMPTY) {
        for (i = 0; i < args[0]->used; ++i) {
            leaf = (struct lyd_node_leaf_list *)args[0]->val.nodes[i].node;
            sleaf = (struct lys_node_leaf *)leaf->schema;
            if ((sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST)) && (sleaf->type.base == LY_TYPE_IDENT)) {
                if (!xpath_derived_from_ident_cmp(leaf->value.ident, args[1]->val.str)) {
                    set_fill_boolean(set, 1);
                    break;
                }

                for (j = 0; j < leaf->value.ident->base_size; ++j) {
                    if (!xpath_derived_from_ident_cmp(leaf->value.ident->base[j], args[1]->val.str)) {
                        set_fill_boolean(set, 1);
                        break;
                    }
                }

                if (j < leaf->value.ident->base_size) {
                    break;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the YANG 1.1 enum-value(node-set) function. Returns LYXP_SET_NUMBER
 *        with the integer value of the first node's enum value, otherwise NaN.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_enum_value(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *UNUSED(cur_node),
                 struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    struct lyd_node_leaf_list *leaf;
    struct lys_node_leaf *sleaf;
    int ret = EXIT_SUCCESS;

    if (options & LYXP_SNODE_ALL) {
        if ((args[0]->type != LYXP_SET_SNODE_SET) || !(sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[0]))) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s not a node-set as expected.", __func__);
            ret = EXIT_FAILURE;
        } else if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
            ret = EXIT_FAILURE;
        } else if (!warn_is_specific_type(&sleaf->type, LY_TYPE_ENUM)) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of type \"enumeration\".", __func__, sleaf->name);
            ret = EXIT_FAILURE;
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if ((args[0]->type != LYXP_SET_NODE_SET) && (args[0]->type != LYXP_SET_EMPTY)) {
        LOGVAL(local_mod->ctx, LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "enum-value(node-set)");
        return -1;
    }

    set_fill_number(set, NAN);
    if (args[0]->type == LYXP_SET_NODE_SET) {
        leaf = (struct lyd_node_leaf_list *)args[0]->val.nodes[0].node;
        if ((leaf->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))
                && (((struct lys_node_leaf *)leaf->schema)->type.base == LY_TYPE_ENUM)) {
            set_fill_number(set, leaf->value.enm->value);
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath false() function. Returns LYXP_SET_BOOLEAN
 *        with false value.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_false(struct lyxp_set **UNUSED(args), uint16_t UNUSED(arg_count), struct lyd_node *UNUSED(cur_node),
            struct lys_module *UNUSED(local_mod), struct lyxp_set *set, int options)
{
    if (options & LYXP_SNODE_ALL) {
        set_snode_clear_ctx(set);
        return EXIT_SUCCESS;
    }

    set_fill_boolean(set, 0);
    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath floor(number) function. Returns LYXP_SET_NUMBER
 *        with the first argument floored (truncated).
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_floor(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *cur_node, struct lys_module *local_mod,
            struct lyxp_set *set, int options)
{
    if (lyxp_set_cast(args[0], LYXP_SET_NUMBER, cur_node, local_mod, options)) {
        return -1;
    }
    if (isfinite(args[0]->val.num)) {
        set_fill_number(set, (long long)args[0]->val.num);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath lang(string) function. Returns LYXP_SET_BOOLEAN
 *        whether the language of the text matches the one from the argument.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_lang(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *cur_node, struct lys_module *local_mod,
           struct lyxp_set *set, int options)
{
    const struct lyd_node *node, *root;
    struct lys_node_leaf *sleaf;
    struct lyd_attr *attr = NULL;
    int i, ret = EXIT_SUCCESS;

    if (options & LYXP_SNODE_ALL) {
        if ((args[0]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }

    if (set->type == LYXP_SET_EMPTY) {
        set_fill_boolean(set, 0);
        return EXIT_SUCCESS;
    }
    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(local_mod->ctx, LYE_XPATH_INCTX, LY_VLOG_NONE, NULL, print_set_type(set), "lang(string)");
        return -1;
    }

    switch (set->val.nodes[0].type) {
    case LYXP_NODE_ELEM:
    case LYXP_NODE_TEXT:
        node = set->val.nodes[0].node;
        break;
    case LYXP_NODE_ATTR:
        root = moveto_get_root(cur_node, options, NULL);
        node = lyd_attr_parent(root, set->val.attrs[0].attr);
        break;
    default:
        /* nothing to do with roots */
        set_fill_boolean(set, 0);
        return EXIT_SUCCESS;
    }

    /* find lang attribute */
    for (; node; node = node->parent) {
        for (attr = node->attr; attr; attr = attr->next) {
            if (attr->name && !strcmp(attr->name, "lang") && !strcmp(attr->annotation->module->name, "xml")) {
                break;
            }
        }

        if (attr) {
            break;
        }
    }

    /* compare languages */
    if (!attr) {
        set_fill_boolean(set, 0);
    } else {
        for (i = 0; args[0]->val.str[i]; ++i) {
            if (tolower(args[0]->val.str[i]) != tolower(attr->value_str[i])) {
                set_fill_boolean(set, 0);
                break;
            }
        }
        if (!args[0]->val.str[i]) {
            if (!attr->value_str[i] || (attr->value_str[i] == '-')) {
                set_fill_boolean(set, 1);
            } else {
                set_fill_boolean(set, 0);
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath last() function. Returns LYXP_SET_NUMBER
 *        with the context size.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_last(struct lyxp_set **UNUSED(args), uint16_t UNUSED(arg_count), struct lyd_node *UNUSED(cur_node),
           struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    if (options & LYXP_SNODE_ALL) {
        set_snode_clear_ctx(set);
        return EXIT_SUCCESS;
    }

    if (set->type == LYXP_SET_EMPTY) {
        set_fill_number(set, 0);
        return EXIT_SUCCESS;
    }
    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(local_mod->ctx, LYE_XPATH_INCTX, LY_VLOG_NONE, NULL, print_set_type(set), "last()");
        return -1;
    }

    set_fill_number(set, set->ctx_size);
    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath local-name(node-set?) function. Returns LYXP_SET_STRING
 *        with the node name without namespace from the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_local_name(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lys_module *local_mod,
                 struct lyxp_set *set, int options)
{
    struct lyxp_set_node *item;
    /* suppress unused variable warning */
    (void)cur_node;
    (void)options;

    if (options & LYXP_SNODE_ALL) {
        set_snode_clear_ctx(set);
        return EXIT_SUCCESS;
    }

    if (arg_count) {
        if (args[0]->type == LYXP_SET_EMPTY) {
            set_fill_string(set, "", 0);
            return EXIT_SUCCESS;
        }
        if (args[0]->type != LYXP_SET_NODE_SET) {
            LOGVAL(local_mod->ctx, LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "local-name(node-set?)");
            return -1;
        }

        /* we need the set sorted, it affects the result */
        assert(!set_sort(args[0], cur_node, options));

        item = &args[0]->val.nodes[0];
    } else {
        if (set->type == LYXP_SET_EMPTY) {
            set_fill_string(set, "", 0);
            return EXIT_SUCCESS;
        }
        if (set->type != LYXP_SET_NODE_SET) {
            LOGVAL(local_mod->ctx, LYE_XPATH_INCTX, LY_VLOG_NONE, NULL, print_set_type(set), "local-name(node-set?)");
            return -1;
        }

        /* we need the set sorted, it affects the result */
        assert(!set_sort(set, cur_node, options));

        item = &set->val.nodes[0];
    }

    switch (item->type) {
    case LYXP_NODE_ROOT:
    case LYXP_NODE_ROOT_CONFIG:
    case LYXP_NODE_TEXT:
        set_fill_string(set, "", 0);
        break;
    case LYXP_NODE_ELEM:
        set_fill_string(set, item->node->schema->name, strlen(item->node->schema->name));
        break;
    case LYXP_NODE_ATTR:
        set_fill_string(set, ((struct lyd_attr *)item->node)->name, strlen(((struct lyd_attr *)item->node)->name));
        break;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath name(node-set?) function. Returns LYXP_SET_STRING
 *        with the node name fully qualified (with namespace) from the argument or the context.
 *        !! This function does not follow its definition and actually copies what local-name()
 *           function does, for the ietf-ipfix-psamp module that uses it incorrectly. !!
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_name(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lys_module *local_mod,
           struct lyxp_set *set, int options)
{
    return xpath_local_name(args, arg_count, cur_node, local_mod, set, options);
}

/**
 * @brief Execute the XPath namespace-uri(node-set?) function. Returns LYXP_SET_STRING
 *        with the namespace of the node from the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_namespace_uri(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lys_module *local_mod,
                    struct lyxp_set *set, int options)
{
    struct lyxp_set_node *item;
    struct lys_module *module;
    /* suppress unused variable warning */
    (void)cur_node;
    (void)options;

    if (options & LYXP_SNODE_ALL) {
        set_snode_clear_ctx(set);
        return EXIT_SUCCESS;
    }

    if (arg_count) {
        if (args[0]->type == LYXP_SET_EMPTY) {
            set_fill_string(set, "", 0);
            return EXIT_SUCCESS;
        }
        if (args[0]->type != LYXP_SET_NODE_SET) {
            LOGVAL(local_mod->ctx, LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "namespace-uri(node-set?)");
            return -1;
        }

        /* we need the set sorted, it affects the result */
        assert(!set_sort(args[0], cur_node, options));

        item = &args[0]->val.nodes[0];
    } else {
        if (set->type == LYXP_SET_EMPTY) {
            set_fill_string(set, "", 0);
            return EXIT_SUCCESS;
        }
        if (set->type != LYXP_SET_NODE_SET) {
            LOGVAL(local_mod->ctx, LYE_XPATH_INCTX, LY_VLOG_NONE, NULL, print_set_type(set), "namespace-uri(node-set?)");
            return -1;
        }

        /* we need the set sorted, it affects the result */
        assert(!set_sort(set, cur_node, options));

        item = &set->val.nodes[0];
    }

    switch (item->type) {
    case LYXP_NODE_ROOT:
    case LYXP_NODE_ROOT_CONFIG:
    case LYXP_NODE_TEXT:
        set_fill_string(set, "", 0);
        break;
    case LYXP_NODE_ELEM:
    case LYXP_NODE_ATTR:
        if (item->type == LYXP_NODE_ELEM) {
            module =  item->node->schema->module;
        } else { /* LYXP_NODE_ATTR */
            module = ((struct lyd_attr *)item->node)->annotation->module;
        }

        module = lys_main_module(module);

        set_fill_string(set, module->ns, strlen(module->ns));
        break;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath node() function (node type). Returns LYXP_SET_NODE_SET
 *        with only nodes from the context. In practice it either leaves the context
 *        as it is or returns an empty node set.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_node(struct lyxp_set **UNUSED(args), uint16_t UNUSED(arg_count), struct lyd_node *cur_node,
           struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    if (options & LYXP_SNODE_ALL) {
        set_snode_clear_ctx(set);
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, local_mod, options);
    }
    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath normalize-space(string?) function. Returns LYXP_SET_STRING
 *        with normalized value (no leading, trailing, double white spaces) of the node
 *        from the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_normalize_space(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lys_module *local_mod,
                      struct lyxp_set *set, int options)
{
    uint16_t i, new_used;
    char *new;
    int have_spaces = 0, space_before = 0, ret = EXIT_SUCCESS;
    struct lys_node_leaf *sleaf;

    if (options & LYXP_SNODE_ALL) {
        if (arg_count && (args[0]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if (arg_count) {
        set_fill_set(set, args[0]);
    }
    if (lyxp_set_cast(set, LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }

    /* is there any normalization necessary? */
    for (i = 0; set->val.str[i]; ++i) {
        if (is_xmlws(set->val.str[i])) {
            if ((i == 0) || space_before || (!set->val.str[i + 1])) {
                have_spaces = 1;
                break;
            }
            space_before = 1;
        } else {
            space_before = 0;
        }
    }

    /* yep, there is */
    if (have_spaces) {
        /* it's enough, at least one character will go, makes space for ending '\0' */
        new = malloc(strlen(set->val.str) * sizeof(char));
        LY_CHECK_ERR_RETURN(!new, LOGMEM(local_mod->ctx), -1);
        new_used = 0;

        space_before = 0;
        for (i = 0; set->val.str[i]; ++i) {
            if (is_xmlws(set->val.str[i])) {
                if ((i == 0) || space_before) {
                    space_before = 1;
                    continue;
                } else {
                    space_before = 1;
                }
            } else {
                space_before = 0;
            }

            new[new_used] = (space_before ? ' ' : set->val.str[i]);
            ++new_used;
        }

        /* at worst there is one trailing space now */
        if (new_used && is_xmlws(new[new_used - 1])) {
            --new_used;
        }

        new = ly_realloc(new, (new_used + 1) * sizeof(char));
        LY_CHECK_ERR_RETURN(!new, LOGMEM(local_mod->ctx), -1);
        new[new_used] = '\0';

        free(set->val.str);
        set->val.str = new;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath not(boolean) function. Returns LYXP_SET_BOOLEAN
 *        with the argument converted to boolean and logically inverted.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_not(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *cur_node, struct lys_module *local_mod,
          struct lyxp_set *set, int options)
{
    if (options & LYXP_SNODE_ALL) {
        set_snode_clear_ctx(set);
        return EXIT_SUCCESS;
    }

    lyxp_set_cast(args[0], LYXP_SET_BOOLEAN, cur_node, local_mod, options);
    if (args[0]->val.bool) {
        set_fill_boolean(set, 0);
    } else {
        set_fill_boolean(set, 1);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath number(object?) function. Returns LYXP_SET_NUMBER
 *        with the number representation of either the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_number(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lys_module *local_mod,
             struct lyxp_set *set, int options)
{
    if (options & LYXP_SNODE_ALL) {
        set_snode_clear_ctx(set);
        return EXIT_SUCCESS;
    }

    if (arg_count) {
        if (lyxp_set_cast(args[0], LYXP_SET_NUMBER, cur_node, local_mod, options)) {
            return -1;
        }
        set_fill_set(set, args[0]);
    } else {
        if (lyxp_set_cast(set, LYXP_SET_NUMBER, cur_node, local_mod, options)) {
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath position() function. Returns LYXP_SET_NUMBER
 *        with the context position.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_position(struct lyxp_set **UNUSED(args), uint16_t UNUSED(arg_count), struct lyd_node *UNUSED(cur_node),
               struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    if (options & LYXP_SNODE_ALL) {
        set_snode_clear_ctx(set);
        return EXIT_SUCCESS;
    }

    if (set->type == LYXP_SET_EMPTY) {
        set_fill_number(set, 0);
        return EXIT_SUCCESS;
    }
    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(local_mod->ctx, LYE_XPATH_INCTX, LY_VLOG_NONE, NULL, print_set_type(set), "position()");
        return -1;
    }

    set_fill_number(set, set->ctx_pos);

    /* UNUSED in 'Release' build type */
    (void)options;
    return EXIT_SUCCESS;
}

/**
 * @brief Execute the YANG 1.1 re-match(string, string) function. Returns LYXP_SET_BOOLEAN
 *        depending on whether the second argument regex matches the first argument string. For details refer to
 *        YANG 1.1 RFC section 10.2.1.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_re_match(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *cur_node, struct lys_module *local_mod,
               struct lyxp_set *set, int options)
{
    pcre *precomp;
    struct lys_node_leaf *sleaf;
    int ret = EXIT_SUCCESS;

    if (options & LYXP_SNODE_ALL) {
        if ((args[0]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }

        if ((args[1]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }

    if (lyp_check_pattern(local_mod->ctx, args[1]->val.str, &precomp)) {
        return -1;
    }
    if (pcre_exec(precomp, NULL, args[0]->val.str, strlen(args[0]->val.str), 0, 0, NULL, 0)) {
        set_fill_boolean(set, 0);
    } else {
        set_fill_boolean(set, 1);
    }
    free(precomp);

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath round(number) function. Returns LYXP_SET_NUMBER
 *        with the rounded first argument. For details refer to
 *        http://www.w3.org/TR/1999/REC-xpath-19991116/#function-round.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_round(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *cur_node, struct lys_module *local_mod,
            struct lyxp_set *set, int options)
{
    struct lys_node_leaf *sleaf;
    int ret = EXIT_SUCCESS;

    if (options & LYXP_SNODE_ALL) {
        if ((args[0]->type != LYXP_SET_SNODE_SET) || !(sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[0]))) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s not a node-set as expected.", __func__);
            ret = EXIT_FAILURE;
        } else if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
            ret = EXIT_FAILURE;
        } else if (!warn_is_specific_type(&sleaf->type, LY_TYPE_DEC64)) {
            LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of type \"decimal64\".", __func__, sleaf->name);
            ret = EXIT_FAILURE;
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_NUMBER, cur_node, local_mod, options)) {
        return -1;
    }

    /* cover only the cases where floor can't be used */
    if ((args[0]->val.num == -0.0f) || ((args[0]->val.num < 0) && (args[0]->val.num >= -0.5))) {
        set_fill_number(set, -0.0f);
    } else {
        args[0]->val.num += 0.5;
        if (xpath_floor(args, 1, cur_node, local_mod, args[0], options)) {
            return -1;
        }
        set_fill_number(set, args[0]->val.num);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath starts-with(string, string) function.
 *        Returns LYXP_SET_BOOLEAN whether the second argument is
 *        the prefix of the first or not.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_starts_with(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *cur_node, struct lys_module *local_mod,
                  struct lyxp_set *set, int options)
{
    struct lys_node_leaf *sleaf;
    int ret = EXIT_SUCCESS;

    if (options & LYXP_SNODE_ALL) {
        if ((args[0]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }

        if ((args[1]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }

    if (strncmp(args[0]->val.str, args[1]->val.str, strlen(args[1]->val.str))) {
        set_fill_boolean(set, 0);
    } else {
        set_fill_boolean(set, 1);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath string(object?) function. Returns LYXP_SET_STRING
 *        with the string representation of either the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_string(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lys_module *local_mod,
             struct lyxp_set *set, int options)
{
    if (options & LYXP_SNODE_ALL) {
        set_snode_clear_ctx(set);
        return EXIT_SUCCESS;
    }

    if (arg_count) {
        if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, local_mod, options)) {
            return -1;
        }
        set_fill_set(set, args[0]);
    } else {
        if (lyxp_set_cast(set, LYXP_SET_STRING, cur_node, local_mod, options)) {
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath string-length(string?) function. Returns LYXP_SET_NUMBER
 *        with the length of the string in either the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_string_length(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lys_module *local_mod,
                    struct lyxp_set *set, int options)
{
    struct lys_node_leaf *sleaf;
    int ret = EXIT_SUCCESS;

    if (options & LYXP_SNODE_ALL) {
        if (arg_count && (args[0]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }
        if (!arg_count && (set->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(set))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #0 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #0 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if (arg_count) {
        if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, local_mod, options)) {
            return -1;
        }
        set_fill_number(set, strlen(args[0]->val.str));
    } else {
        if (lyxp_set_cast(set, LYXP_SET_STRING, cur_node, local_mod, options)) {
            return -1;
        }
        set_fill_number(set, strlen(set->val.str));
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath substring(string, number, number?) function.
 *        Returns LYXP_SET_STRING substring of the first argument starting
 *        on the second argument index ending on the third argument index,
 *        indexed from 1. For exact definition refer to
 *        http://www.w3.org/TR/1999/REC-xpath-19991116/#function-substring.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_substring(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lys_module *local_mod,
                struct lyxp_set *set, int options)
{
    int start, len, ret = EXIT_SUCCESS;
    uint16_t str_start, str_len, pos;
    struct lys_node_leaf *sleaf;

    if (options & LYXP_SNODE_ALL) {
        if ((args[0]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }

        if ((args[1]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_numeric_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is node \"%s\", not of numeric type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }

        if ((arg_count == 3) && (args[2]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[2]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #3 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_numeric_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #3 of %s is node \"%s\", not of numeric type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }

    /* start */
    if (xpath_round(&args[1], 1, cur_node, local_mod, args[1], options)) {
        return -1;
    }
    if (isfinite(args[1]->val.num)) {
        start = args[1]->val.num - 1;
    } else if (isinf(args[1]->val.num) && signbit(args[1]->val.num)) {
        start = INT_MIN;
    } else {
        start = INT_MAX;
    }

    /* len */
    if (arg_count == 3) {
        if (xpath_round(&args[2], 1, cur_node, local_mod, args[2], options)) {
            return -1;
        }
        if (isfinite(args[2]->val.num)) {
            len = args[2]->val.num;
        } else if (isnan(args[2]->val.num) || signbit(args[2]->val.num)) {
            len = 0;
        } else {
            len = INT_MAX;
        }
    } else {
        len = INT_MAX;
    }

    /* find matching character positions */
    str_start = 0;
    str_len = 0;
    for (pos = 0; args[0]->val.str[pos]; ++pos) {
        if (pos < start) {
            ++str_start;
        } else if (pos < start + len) {
            ++str_len;
        } else {
            break;
        }
    }

    set_fill_string(set, args[0]->val.str + str_start, str_len);
    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath substring-after(string, string) function.
 *        Returns LYXP_SET_STRING with the string succeeding the occurance
 *        of the second argument in the first or an empty string.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_substring_after(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *cur_node,
                      struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    char *ptr;
    struct lys_node_leaf *sleaf;
    int ret = EXIT_SUCCESS;

    if (options & LYXP_SNODE_ALL) {
        if ((args[0]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }

        if ((args[1]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }

    ptr = strstr(args[0]->val.str, args[1]->val.str);
    if (ptr) {
        set_fill_string(set, ptr + strlen(args[1]->val.str), strlen(ptr + strlen(args[1]->val.str)));
    } else {
        set_fill_string(set, "", 0);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath substring-before(string, string) function.
 *        Returns LYXP_SET_STRING with the string preceding the occurance
 *        of the second argument in the first or an empty string.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_substring_before(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *cur_node,
                       struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    char *ptr;
    struct lys_node_leaf *sleaf;
    int ret = EXIT_SUCCESS;

    if (options & LYXP_SNODE_ALL) {
        if ((args[0]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }

        if ((args[1]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }

    ptr = strstr(args[0]->val.str, args[1]->val.str);
    if (ptr) {
        set_fill_string(set, args[0]->val.str, ptr - args[0]->val.str);
    } else {
        set_fill_string(set, "", 0);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath sum(node-set) function. Returns LYXP_SET_NUMBER
 *        with the sum of all the nodes in the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_sum(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *cur_node, struct lys_module *local_mod,
          struct lyxp_set *set, int options)
{
    long double num;
    char *str;
    uint16_t i;
    struct lyxp_set set_item;
    struct lys_node_leaf *sleaf;
    int ret = EXIT_SUCCESS;

    if (options & LYXP_SNODE_ALL) {
        if (args[0]->type == LYXP_SET_SNODE_SET) {
            for (i = 0; i < args[0]->used; ++i) {
                if (args[0]->val.snodes[i].in_ctx == 1) {
                    sleaf = (struct lys_node_leaf *)args[0]->val.snodes[i].snode;
                    if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                        LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                        ret = EXIT_FAILURE;
                    } else if (!warn_is_numeric_type(&sleaf->type)) {
                        LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of numeric type.", __func__, sleaf->name);
                        ret = EXIT_FAILURE;
                    }
                }
            }
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    set_fill_number(set, 0);
    if (args[0]->type == LYXP_SET_EMPTY) {
        return EXIT_SUCCESS;
    }

    if (args[0]->type != LYXP_SET_NODE_SET) {
        LOGVAL(local_mod->ctx, LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "sum(node-set)");
        return -1;
    }

    set_item.type = LYXP_SET_NODE_SET;
    set_item.val.nodes = malloc(sizeof *set_item.val.nodes);
    LY_CHECK_ERR_RETURN(!set_item.val.nodes, LOGMEM(local_mod->ctx), -1);

    set_item.used = 1;
    set_item.size = 1;

    for (i = 0; i < args[0]->used; ++i) {
        set_item.val.nodes[0] = args[0]->val.nodes[i];

        str = cast_node_set_to_string(&set_item, cur_node, local_mod, options);
        if (!str) {
            return -1;
        }
        num = cast_string_to_number(str);
        free(str);
        set->val.num += num;
    }

    free(set_item.val.nodes);

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath text() function (node type). Returns LYXP_SET_NODE_SET
 *        with the text content of the nodes in the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_text(struct lyxp_set **UNUSED(args), uint16_t UNUSED(arg_count), struct lyd_node *UNUSED(cur_node),
           struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    uint32_t i;

    if (options & LYXP_SNODE_ALL) {
        set_snode_clear_ctx(set);
        return EXIT_SUCCESS;
    }

    if (set->type == LYXP_SET_EMPTY) {
        return EXIT_SUCCESS;
    }
    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(local_mod->ctx, LYE_XPATH_INCTX, LY_VLOG_NONE, NULL, print_set_type(set), "text()");
        return -1;
    }

    for (i = 0; i < set->used;) {
        switch (set->val.nodes[i].type) {
        case LYXP_NODE_ELEM:
            if (set->val.nodes[i].node->validity & LYD_VAL_INUSE) {
                LOGVAL(local_mod->ctx, LYE_XPATH_DUMMY, LY_VLOG_LYD, set->val.nodes[i].node, set->val.nodes[i].node->schema->name);
                return -1;
            }
            if ((set->val.nodes[i].node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))
                    && ((struct lyd_node_leaf_list *)set->val.nodes[i].node)->value_str) {
                set->val.nodes[i].type = LYXP_NODE_TEXT;
                ++i;
                break;
            }
            /* fall through */
        case LYXP_NODE_ROOT:
        case LYXP_NODE_ROOT_CONFIG:
        case LYXP_NODE_TEXT:
        case LYXP_NODE_ATTR:
            set_remove_node(set, i);
            break;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath translate(string, string, string) function.
 *        Returns LYXP_SET_STRING with the first argument with the characters
 *        from the second argument replaced by those on the corresponding
 *        positions in the third argument.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_translate(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyd_node *cur_node,
                struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    uint16_t i, j, new_used;
    char *new;
    int found, have_removed;
    struct lys_node_leaf *sleaf;
    int ret = EXIT_SUCCESS;

    if (options & LYXP_SNODE_ALL) {
        if ((args[0]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }

        if ((args[1]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #2 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }

        if ((args[2]->type == LYXP_SET_SNODE_SET) && (sleaf = (struct lys_node_leaf *)warn_get_snode_in_ctx(args[2]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(local_mod->ctx, "Argument #3 of %s is a %s node \"%s\".", __func__, strnodetype(sleaf->nodetype), sleaf->name);
                ret = EXIT_FAILURE;
            } else if (!warn_is_string_type(&sleaf->type)) {
                LOGWRN(local_mod->ctx, "Argument #3 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
                ret = EXIT_FAILURE;
            }
        }
        set_snode_clear_ctx(set);
        return ret;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }
    if (lyxp_set_cast(args[2], LYXP_SET_STRING, cur_node, local_mod, options)) {
        return -1;
    }

    new = malloc((strlen(args[0]->val.str) + 1) * sizeof(char));
    LY_CHECK_ERR_RETURN(!new, LOGMEM(local_mod->ctx), -1);
    new_used = 0;

    have_removed = 0;
    for (i = 0; args[0]->val.str[i]; ++i) {
        found = 0;

        for (j = 0; args[1]->val.str[j]; ++j) {
            if (args[0]->val.str[i] == args[1]->val.str[j]) {
                /* removing this char */
                if (j >= strlen(args[2]->val.str)) {
                    have_removed = 1;
                    found = 1;
                    break;
                }
                /* replacing this char */
                new[new_used] = args[2]->val.str[j];
                ++new_used;
                found = 1;
                break;
            }
        }

        /* copying this char */
        if (!found) {
            new[new_used] = args[0]->val.str[i];
            ++new_used;
        }
    }

    if (have_removed) {
        new = ly_realloc(new, (new_used + 1) * sizeof(char));
        LY_CHECK_ERR_RETURN(!new, LOGMEM(local_mod->ctx), -1);
    }
    new[new_used] = '\0';

    lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, local_mod, options);
    set->type = LYXP_SET_STRING;
    set->val.str = new;

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath true() function. Returns LYXP_SET_BOOLEAN
 *        with true value.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_true(struct lyxp_set **UNUSED(args), uint16_t UNUSED(arg_count), struct lyd_node *UNUSED(cur_node),
           struct lys_module *UNUSED(local_mod), struct lyxp_set *set, int options)
{
    if (options & LYXP_SNODE_ALL) {
        set_snode_clear_ctx(set);
        return EXIT_SUCCESS;
    }

    set_fill_boolean(set, 1);
    return EXIT_SUCCESS;
}

/*
 * moveto functions
 *
 * They and only they actually change the context (set).
 */

/**
 * @brief Resolve and find a specific model. Does not log.
 *
 * \p cur_snode is required in 2 quite specific cases concerning
 * XPath on schema. Problem is when we are parsing a submodule
 * and referencing something in the main module or parsing
 * a module importing another module that references back
 * the original module. Then the target module is still being
 * parsed and it not yet in the context - it fails to resolve.
 * In these cases we can find the module using \p cur_snode.
 *
 * @param[in] mod_name_ns Either module name or namespace.
 * @param[in] mon_nam_ns_len Length of \p mod_name_ns.
 * @param[in] ctx libyang context.
 * @param[in] cur_snode Current schema node, on data XPath leave NULL.
 * @param[in] is_name Whether \p mod_name_ns is module name (1) or namespace (0).
 *
 * @return Corresponding module or NULL on error.
 */
static struct lys_module *
moveto_resolve_model(const char *mod_name_ns, uint16_t mod_nam_ns_len, struct ly_ctx *ctx, struct lys_node *cur_snode,
                     int is_name, int import_and_disabled_model)
{
    uint16_t i;
    const char *str;
    struct lys_module *mod, *mainmod;

    if (cur_snode) {
        /* detect if the XPath is used in augment - in such a case the module of the context node (cur_snode)
         * differs from the currently processed module. Then, we have to use the currently processed module
         * for searching for the module/namespace instead of the module of the context node */
        if (ctx->models.parsing_sub_modules_count &&
                cur_snode->module != ctx->models.parsing_sub_modules[ctx->models.parsing_sub_modules_count - 1]) {
            mod = ctx->models.parsing_sub_modules[ctx->models.parsing_sub_modules_count - 1];
        } else {
            mod = cur_snode->module;
        }
        mainmod = lys_main_module(mod);

        str = (is_name ? mainmod->name : mainmod->ns);
        if (!strncmp(str, mod_name_ns, mod_nam_ns_len) && !str[mod_nam_ns_len]) {
            return mainmod;
        }

        for (i = 0; i < mod->imp_size; ++i) {
            str = (is_name ? mod->imp[i].module->name : mod->imp[i].module->ns);
            if (!strncmp(str, mod_name_ns, mod_nam_ns_len) && !str[mod_nam_ns_len]) {
                return mod->imp[i].module;
            }
        }
    }

    for (i = 0; i < ctx->models.used; ++i) {
        if (!import_and_disabled_model && (!ctx->models.list[i]->implemented || ctx->models.list[i]->disabled)) {
            /* skip not implemented or disabled modules */
            continue;
        }
        str = (is_name ? ctx->models.list[i]->name : ctx->models.list[i]->ns);
        if (!strncmp(str, mod_name_ns, mod_nam_ns_len) && !str[mod_nam_ns_len]) {
            return ctx->models.list[i];
        }
    }

    return NULL;
}

/**
 * @brief Get the context root.
 *
 * @param[in] cur_node Original context node.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[out] root_type Root type, differs only in when, must evaluation.
 *
 * @return Context root.
 */
static const struct lyd_node *
moveto_get_root(const struct lyd_node *cur_node, int options, enum lyxp_node_type *root_type)
{
    const struct lyd_node *root;

    if (!cur_node) {
        return NULL;
    }

    if (!options) {
        /* special kind of root that can access everything */
        for (root = cur_node; root->parent; root = root->parent);
        for (; root->prev->next; root = root->prev);
        *root_type = LYXP_NODE_ROOT;
        return root;
    }

    if (cur_node->schema->flags & LYS_CONFIG_W) {
        *root_type = LYXP_NODE_ROOT_CONFIG;
    } else {
        *root_type = LYXP_NODE_ROOT;
    }

    for (root = cur_node; root->parent; root = root->parent);
    for (; root->prev->next; root = root->prev);

    return root;
}

static const struct lys_node *
moveto_snode_get_root(const struct lys_node *cur_node, int options, enum lyxp_node_type *root_type)
{
    const struct lys_node *root;

    assert(cur_node && root_type);

    if (options & LYXP_SNODE) {
        /* general root that can access everything */
        *root_type = LYXP_NODE_ROOT;
    } else if (cur_node->flags & LYS_CONFIG_W) {
        *root_type = LYXP_NODE_ROOT_CONFIG;
    } else {
        *root_type = LYXP_NODE_ROOT;
    }

    root = lys_getnext(NULL, NULL, lys_node_module(cur_node), LYS_GETNEXT_NOSTATECHECK);

    return root;
}

/**
 * @brief Move context \p set to the root. Handles absolute path.
 *        Result is LYXP_SET_NODE_SET.
 *
 * @param[in,out] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 */
static void
moveto_root(struct lyxp_set *set, struct lyd_node *cur_node, int options)
{
    const struct lyd_node *root;
    enum lyxp_node_type root_type;

    if (!set) {
        return;
    }

    root = moveto_get_root(cur_node, options, &root_type);

    lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, NULL, options);
    if (root) {
        set_insert_node(set, root, 0, root_type, 0);
    }
}

static void
moveto_snode_root(struct lyxp_set *set, struct lys_node *cur_node, int options)
{
    const struct lys_node *root;
    enum lyxp_node_type root_type;

    if (!set) {
        return;
    }

    if (!cur_node) {
        LOGINT(NULL);
        return;
    }

    root = moveto_snode_get_root(cur_node, options, &root_type);
    set_snode_clear_ctx(set);
    set_snode_insert_node(set, root, root_type);
}

/**
 * @brief Check \p node as a part of NameTest processing.
 *
 * @param[in] node Node to check.
 * @param[in] root_type XPath root node type.
 * @param[in] node_name Node name to move to. Must be in the dictionary!
 * @param[in] moveto_mod Expected module of the node.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
moveto_node_check(struct lyd_node *node, enum lyxp_node_type root_type, const char *node_name,
                  struct lys_module *moveto_mod, int options)
{
    /* module check */
    if (moveto_mod && (lyd_node_module(node) != moveto_mod)) {
        return -1;
    }

    /* context check */
    if ((root_type == LYXP_NODE_ROOT_CONFIG) && (node->schema->flags & LYS_CONFIG_R)) {
        return -1;
    }

    /* name check */
    if (strcmp(node_name, "*") && !ly_strequal(node->schema->name, node_name, 1)) {
        return -1;
    }

    /* when check */
    if ((options & LYXP_WHEN) && !LYD_WHEN_DONE(node->when_status)) {
        return EXIT_FAILURE;
    }

    /* match */
    return EXIT_SUCCESS;
}

static int
moveto_snode_check(const struct lys_node *node, enum lyxp_node_type root_type, const char *node_name,
                   struct lys_module *moveto_mod, int options)
{
    struct lys_node *parent;

    /* RPC input/output check */
    for (parent = lys_parent(node); parent && (parent->nodetype == LYS_USES); parent = lys_parent(parent));
    if (options & LYXP_SNODE_OUTPUT) {
        if (parent && (parent->nodetype == LYS_INPUT)) {
            return -1;
        }
    } else {
        if (parent && (parent->nodetype == LYS_OUTPUT)) {
            return -1;
        }
    }

    /* module check */
    if (strcmp(node_name, "*") && (lys_node_module(node) != moveto_mod)) {
        return -1;
    }

    /* context check */
    if ((root_type == LYXP_NODE_ROOT_CONFIG) && (node->flags & LYS_CONFIG_R)) {
        return -1;
    }

    /* name check */
    if (strcmp(node_name, "*") && !ly_strequal(node->name, node_name, 1)) {
        return -1;
    }

    /* match */
    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to a node. Handles '/' and '*', 'NAME', 'PREFIX:*', or 'PREFIX:NAME'.
 *        Result is LYXP_SET_NODE_SET (or LYXP_SET_EMPTY). Context position aware.
 *
 * @param[in,out] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] qname Qualified node name to move to.
 * @param[in] qname_len Length of \p qname.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
moveto_node(struct lyxp_set *set, struct lyd_node *cur_node, const char *qname, uint16_t qname_len, int options)
{
    uint32_t i;
    int replaced, pref_len, ret;
    const char *ptr, *name_dict = NULL; /* optimalization - so we can do (==) instead (!strncmp(...)) in moveto_node_check() */
    struct lys_module *moveto_mod;
    struct lyd_node *sub;
    struct ly_ctx *ctx;
    enum lyxp_node_type root_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    assert(cur_node);
    ctx = cur_node->schema->module->ctx;

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(ctx, LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    moveto_get_root(cur_node, options, &root_type);

    /* prefix */
    if ((ptr = strnchr(qname, ':', qname_len))) {
        /* specific module */
        pref_len = ptr - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, ctx, NULL, 1, 0);
        if (!moveto_mod) {
            LOGVAL(ctx, LYE_XPATH_INMOD, LY_VLOG_NONE, NULL, pref_len, qname);
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else if ((qname[0] == '*') && (qname_len == 1)) {
        /* all modules - special case */
        moveto_mod = NULL;
    } else {
        /* content node module */
        moveto_mod = lyd_node_module(cur_node);
    }

    /* name */
    name_dict = lydict_insert(ctx, qname, qname_len);

    for (i = 0; i < set->used; ) {
        replaced = 0;

        if ((set->val.nodes[i].type == LYXP_NODE_ROOT_CONFIG) || (set->val.nodes[i].type == LYXP_NODE_ROOT)) {
            LY_TREE_FOR(set->val.nodes[i].node, sub) {
                ret = moveto_node_check(sub, root_type, name_dict, moveto_mod, options);
                if (!ret) {
                    /* pos filled later */
                    if (!replaced) {
                        set_replace_node(set, sub, 0, LYXP_NODE_ELEM, i);
                        replaced = 1;
                    } else {
                        set_insert_node(set, sub, 0, LYXP_NODE_ELEM, i);
                    }
                    ++i;
                } else if (ret == EXIT_FAILURE) {
                    lydict_remove(ctx, name_dict);
                    return EXIT_FAILURE;
                }
            }

        /* skip nodes without children - leaves, leaflists, anyxmls, and dummy nodes (ouput root will eval to true) */
        } else if (!(set->val.nodes[i].node->validity & LYD_VAL_INUSE)
                && !(set->val.nodes[i].node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA))) {

            LY_TREE_FOR(set->val.nodes[i].node->child, sub) {
                ret = moveto_node_check(sub, root_type, name_dict, moveto_mod, options);
                if (!ret) {
                    if (!replaced) {
                        set_replace_node(set, sub, 0, LYXP_NODE_ELEM, i);
                        replaced = 1;
                    } else {
                        set_insert_node(set, sub, 0, LYXP_NODE_ELEM, i);
                    }
                    ++i;
                } else if (ret == EXIT_FAILURE) {
                    lydict_remove(ctx, name_dict);
                    return EXIT_FAILURE;
                }
            }
        }

        if (!replaced) {
            /* no match */
            set_remove_node(set, i);
        }
    }
    lydict_remove(ctx, name_dict);

    return EXIT_SUCCESS;
}

static int
moveto_snode(struct lyxp_set *set, struct lys_node *cur_node, const char *qname, uint16_t qname_len, int options)
{
    int i, orig_used, pref_len, idx, temp_ctx = 0;
    const char *ptr, *name_dict = NULL; /* optimalization - so we can do (==) instead (!strncmp(...)) in moveto_node_check() */
    struct lys_module *moveto_mod, *tmp_mod;
    const struct lys_node *sub, *start_parent;
    struct lys_node_augment *last_aug;
    struct ly_ctx *ctx;
    enum lyxp_node_type root_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    ctx = cur_node->module->ctx;

    if (set->type != LYXP_SET_SNODE_SET) {
        LOGVAL(ctx, LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    moveto_snode_get_root(cur_node, options, &root_type);

    /* prefix */
    if ((ptr = strnchr(qname, ':', qname_len))) {
        pref_len = ptr - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, ctx, cur_node, 1, 1);
        if (!moveto_mod) {
            LOGVAL(ctx, LYE_XPATH_INMOD, LY_VLOG_NONE, NULL, pref_len, qname);
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        moveto_mod = NULL;
    }

    /* name */
    name_dict = lydict_insert(ctx, qname, qname_len);

    orig_used = set->used;
    for (i = 0; i < orig_used; ++i) {
        if (set->val.snodes[i].in_ctx != 1) {
            continue;
        }
        set->val.snodes[i].in_ctx = 0;

        start_parent = set->val.snodes[i].snode;

        if ((set->val.snodes[i].type == LYXP_NODE_ROOT_CONFIG) || (set->val.snodes[i].type == LYXP_NODE_ROOT)) {
            /* it can actually be in any module, it's all <running>, but we know it's moveto_mod (if set),
             * so use it directly (root node itself is useless in this case) */
            sub = NULL;
            while ((sub = lys_getnext(sub, NULL, (moveto_mod ? moveto_mod : lys_node_module(cur_node)), LYS_GETNEXT_NOSTATECHECK))) {
                if (!moveto_snode_check(sub, root_type, name_dict, (moveto_mod ? moveto_mod : lys_node_module(cur_node)), options)) {
                    idx = set_snode_insert_node(set, sub, LYXP_NODE_ELEM);
                    /* we need to prevent these nodes to be considered in this moveto */
                    if ((idx < orig_used) && (idx > i)) {
                        set->val.snodes[idx].in_ctx = 2;
                        temp_ctx = 1;
                    }
                }
            }

        /* skip nodes without children - leaves, leaflists, and anyxmls (ouput root will eval to true) */
        } else if (!(start_parent->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA))) {
            /* the target may be from an augment that was not connected */
            last_aug = NULL;
            tmp_mod = NULL;
            if ((moveto_mod && !moveto_mod->implemented) || (!moveto_mod && !lys_node_module(cur_node)->implemented)) {
                if (moveto_mod) {
                    tmp_mod = moveto_mod;
                } else {
                    tmp_mod = lys_node_module(cur_node);
                }

get_next_augment:
                last_aug = lys_getnext_target_aug(last_aug, tmp_mod, start_parent);
            }

            sub = NULL;
            while ((sub = lys_getnext(sub, (last_aug ? (struct lys_node *)last_aug : start_parent), NULL, LYS_GETNEXT_NOSTATECHECK))) {
                if (!moveto_snode_check(sub, root_type, name_dict, (moveto_mod ? moveto_mod : lys_node_module(cur_node)), options)) {
                    idx = set_snode_insert_node(set, sub, LYXP_NODE_ELEM);
                    if ((idx < orig_used) && (idx > i)) {
                        set->val.snodes[idx].in_ctx = 2;
                        temp_ctx = 1;
                    }
                }
            }

            if (last_aug) {
                /* try also other augments */
                goto get_next_augment;
            }
        }
    }
    lydict_remove(ctx, name_dict);

    /* correct temporary in_ctx values */
    if (temp_ctx) {
        for (i = 0; i < orig_used; ++i) {
            if (set->val.snodes[i].in_ctx == 2) {
                set->val.snodes[i].in_ctx = 1;
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to a node and all its descendants. Handles '//' and '*', 'NAME',
 *        'PREFIX:*', or 'PREFIX:NAME'. Result is LYXP_SET_NODE_SET (or LYXP_SET_EMPTY).
 *        Context position aware.
 *
 * @param[in] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] qname Qualified node name to move to.
 * @param[in] qname_len Length of \p qname.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, ECIT_FAILURE on unresolved when, -1 on error.
 */
static int
moveto_node_alldesc(struct lyxp_set *set, struct lyd_node *cur_node, const char *qname, uint16_t qname_len,
                    int options)
{
    uint32_t i;
    int pref_len, all = 0, match, ret;
    struct lyd_node *next, *elem, *start;
    struct lys_module *moveto_mod;
    enum lyxp_node_type root_type;
    struct lyxp_set ret_set;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(cur_node->schema->module->ctx, LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    moveto_get_root(cur_node, options, &root_type);

    /* prefix */
    if (strnchr(qname, ':', qname_len) && cur_node) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, cur_node->schema->module->ctx, NULL, 1, 0);
        if (!moveto_mod) {
            LOGVAL(cur_node->schema->module->ctx, LYE_XPATH_INMOD, LY_VLOG_NONE, NULL, pref_len, qname);
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        moveto_mod = NULL;
    }

    /* replace the original nodes (and throws away all text and attr nodes, root is replaced by a child) */
    ret = moveto_node(set, cur_node, "*", 1, options);
    if (ret) {
        return ret;
    }

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    /* this loop traverses all the nodes in the set and addds/keeps only
     * those that match qname */
    memset(&ret_set, 0, sizeof ret_set);
    for (i = 0; i < set->used; ++i) {

        /* TREE DFS */
        start = set->val.nodes[i].node;
        for (elem = next = start; elem; elem = next) {

            /* when check */
            if ((options & LYXP_WHEN) && !LYD_WHEN_DONE(elem->when_status)) {
                return EXIT_FAILURE;
            }

            /* dummy and context check */
            if ((elem->validity & LYD_VAL_INUSE) || ((root_type == LYXP_NODE_ROOT_CONFIG) && (elem->schema->flags & LYS_CONFIG_R))) {
                goto skip_children;
            }

            match = 1;

            /* module check */
            if (!all) {
                if (moveto_mod && (lys_node_module(elem->schema) != moveto_mod)) {
                    match = 0;
                } else if (!moveto_mod && (lys_node_module(elem->schema) != lyd_node_module(cur_node))) {
                    match = 0;
                }
            }

            /* name check */
            if (match && !all && (strncmp(elem->schema->name, qname, qname_len) || elem->schema->name[qname_len])) {
                match = 0;
            }

            if (match) {
                /* add matching node into result set */
                set_insert_node(&ret_set, elem, 0, LYXP_NODE_ELEM, ret_set.used);
                if (set_dup_node_check(set, elem, LYXP_NODE_ELEM, i)) {
                    /* the node is a duplicate, we'll process it later in the set */
                    goto skip_children;
                }
            }

            /* TREE DFS NEXT ELEM */
            /* select element for the next run - children first */
            if (elem->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                next = NULL;
            } else {
                next = elem->child;
            }
            if (!next) {
skip_children:
                /* no children, so try siblings, but only if it's not the start,
                 * that is considered to be the root and it's siblings are not traversed */
                if (elem != start) {
                    next = elem->next;
                } else {
                    break;
                }
            }
            while (!next) {
                /* no siblings, go back through the parents */
                if (elem->parent == start) {
                    /* we are done, no next element to process */
                    break;
                }
                /* parent is already processed, go to its sibling */
                elem = elem->parent;
                next = elem->next;
            }
        }
    }

    /* make the temporary set the current one */
    ret_set.ctx_pos = set->ctx_pos;
    ret_set.ctx_size = set->ctx_size;
    set_free_content(set);
    memcpy(set, &ret_set, sizeof *set);

    return EXIT_SUCCESS;
}

static int
moveto_snode_alldesc(struct lyxp_set *set, struct lys_node *cur_node, const char *qname, uint16_t qname_len,
                     int options)
{
    int i, orig_used, pref_len, all = 0, match, idx;
    struct lys_node *next, *elem, *start;
    struct lys_module *moveto_mod;
    struct ly_ctx *ctx;
    enum lyxp_node_type root_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    ctx = cur_node->module->ctx;

    if (set->type != LYXP_SET_SNODE_SET) {
        LOGVAL(ctx, LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    moveto_snode_get_root(cur_node, options, &root_type);

    /* add all matching direct descendant nodes */
    idx = moveto_snode(set, cur_node, qname, qname_len, options);
    if (idx) {
        return idx;
    }

    /* prefix */
    if (strnchr(qname, ':', qname_len)) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, ctx, cur_node, 1, 1);
        if (!moveto_mod) {
            LOGVAL(ctx, LYE_XPATH_INMOD, LY_VLOG_NONE, NULL, pref_len, qname);
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        moveto_mod = NULL;
    }

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    orig_used = set->used;
    for (i = 0; i < orig_used; ++i) {
        if (set->val.snodes[i].in_ctx != 1) {
            continue;
        }

        /* TREE DFS */
        start = set->val.snodes[i].snode;
        for (elem = next = start; elem; elem = next) {

            /* context/nodetype check */
            if ((root_type == LYXP_NODE_ROOT_CONFIG) && (elem->flags & LYS_CONFIG_R)) {
                /* valid node, but it is hidden in this context */
                goto skip_children;
            }
            switch (elem->nodetype) {
            case LYS_USES:
            case LYS_CHOICE:
            case LYS_CASE:
                /* schema-only nodes */
                goto next_iter;
            case LYS_INPUT:
                if (options & LYXP_SNODE_OUTPUT) {
                    goto skip_children;
                }
                goto next_iter;
            case LYS_OUTPUT:
                if (!(options & LYXP_SNODE_OUTPUT)) {
                    goto skip_children;
                }
                goto next_iter;
            case LYS_GROUPING:
                goto skip_children;
            default:
                break;
            }

            match = 1;

            /* module check */
            if (!all) {
                if (moveto_mod && (lys_node_module(elem) != moveto_mod)) {
                    match = 0;
                } else if (!moveto_mod && (lys_node_module(elem) != lys_node_module(cur_node))) {
                    match = 0;
                }
            }

            /* name check */
            if (!all && (strncmp(elem->name, qname, qname_len) || elem->name[qname_len])) {
                match = 0;
            }

            if (match && (elem != start)) {
                if ((idx = set_snode_dup_node_check(set, elem, LYXP_NODE_ELEM, i)) > -1) {
                    set->val.snodes[idx].in_ctx = 1;
                    if (idx > i) {
                        /* we will process it later in the set */
                        goto skip_children;
                    }
                } else {
                    set_snode_insert_node(set, elem, LYXP_NODE_ELEM);
                }
            } else if (!match && (elem == start)) {
                /* start node must match! */
                LOGINT(ctx);
            }

next_iter:
            /* TREE DFS NEXT ELEM */
            /* select element for the next run - children first */
            next = elem->child;
            if (elem->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                next = NULL;
            }
            if (!next) {
skip_children:
                /* no children, so try siblings, but only if it's not the start,
                 * that is considered to be the root and it's siblings are not traversed */
                if (elem != start) {
                    next = elem->next;
                } else {
                    break;
                }
            }
            while (!next) {
                /* no siblings, go back through the parents */
                if (lys_parent(elem) == start) {
                    /* we are done, no next element to process */
                    break;
                }
                /* parent is already processed, go to its sibling */
                elem = lys_parent(elem);
                next = elem->next;
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to an attribute. Handles '/' and '@*', '@NAME', '@PREFIX:*',
 *        or '@PREFIX:NAME'. Result is LYXP_SET_NODE_SET (or LYXP_SET_EMPTY).
 *        Indirectly context position aware.
 *
 * @param[in,out] set Set to use.
 * @param[in] qname Qualified node name to move to.
 * @param[in] qname_len Length of \p qname.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
moveto_attr(struct lyxp_set *set, struct lyd_node *cur_node, const char *qname, uint16_t qname_len, int UNUSED(options))
{
    uint32_t i;
    int replaced, all = 0, pref_len;
    struct lys_module *moveto_mod;
    struct lyd_attr *sub;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(cur_node->schema->module->ctx, LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    /* prefix */
    if (strnchr(qname, ':', qname_len) && cur_node) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, cur_node->schema->module->ctx, NULL, 1, 0);
        if (!moveto_mod) {
            LOGVAL(cur_node->schema->module->ctx, LYE_XPATH_INMOD, LY_VLOG_NONE, NULL, pref_len, qname);
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        moveto_mod = NULL;
    }

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    for (i = 0; i < set->used; ) {
        replaced = 0;

        /* only attributes of an elem (not dummy) can be in the result, skip all the rest;
         * our attributes are always qualified */
        if ((set->val.nodes[i].type == LYXP_NODE_ELEM) && !(set->val.nodes[i].node->validity & LYD_VAL_INUSE)) {
            LY_TREE_FOR(set->val.nodes[i].node->attr, sub) {

                /* check "namespace" */
                if (moveto_mod && (sub->annotation->module != moveto_mod)) {
                    /* no match */
                    continue;
                }

                if (all || (!strncmp(sub->name, qname, qname_len) && !sub->name[qname_len])) {
                    /* match */
                    if (!replaced) {
                        set->val.attrs[i].attr = sub;
                        set->val.attrs[i].type = LYXP_NODE_ATTR;
                        /* pos does not change */
                        replaced = 1;
                    } else {
                        set_insert_node(set, (struct lyd_node *)sub, set->val.nodes[i].pos, LYXP_NODE_ATTR, i + 1);
                    }
                    ++i;
                }
            }
        }

        if (!replaced) {
            /* no match */
            set_remove_node(set, i);
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set1 to union with \p set2. \p set2 is emptied afterwards.
 *        Result is LYXP_SET_NODE_SET (or LYXP_SET_EMPTY). Context position aware.
 *
 * @param[in,out] set1 Set to use for the result.
 * @param[in] set2 Set that is copied to \p set1.
 * @param[in] cur_node Original context node.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
moveto_union(struct lyxp_set *set1, struct lyxp_set *set2, struct lyd_node *cur_node, int options)
{
    struct ly_ctx *ctx = (options & LYXP_SNODE) ? ((struct lys_node *)cur_node)->module->ctx : cur_node->schema->module->ctx;

    if (((set1->type != LYXP_SET_NODE_SET) && (set1->type != LYXP_SET_EMPTY))
            || ((set2->type != LYXP_SET_NODE_SET) && (set2->type != LYXP_SET_EMPTY))) {
        LOGVAL(ctx, LYE_XPATH_INOP_2, LY_VLOG_NONE, NULL, "union", print_set_type(set1), print_set_type(set2));
        return -1;
    }

    /* set2 is empty or both set1 and set2 */
    if (set2->type == LYXP_SET_EMPTY) {
        return EXIT_SUCCESS;
    }

    if (set1->type == LYXP_SET_EMPTY) {
        memcpy(set1, set2, sizeof *set1);
        /* dynamic memory belongs to set1 now, do not free */
        set2->type = LYXP_SET_EMPTY;
        return EXIT_SUCCESS;
    }

    /* we assume sets are sorted */
    assert(!set_sort(set1, cur_node, options) && !set_sort(set2, cur_node, options));

    /* sort, remove duplicates */
    if (set_sorted_merge(set1, set2, cur_node, options)) {
        return -1;
    }

    /* final set must be sorted */
    assert(!set_sort(set1, cur_node, options));

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to an attribute in any of the descendants. Handles '//' and '@*',
 *        '@NAME', '@PREFIX:*', or '@PREFIX:NAME'. Result is LYXP_SET_NODE_SET (or LYXP_SET_EMPTY).
 *        Context position aware.
 *
 * @param[in,out] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] qname Qualified node name to move to.
 * @param[in] qname_len Length of \p qname.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
moveto_attr_alldesc(struct lyxp_set *set, struct lyd_node *cur_node, const char *qname, uint16_t qname_len,
                    int options)
{
    uint32_t i;
    int pref_len, replaced, all = 0, ret;
    struct lyd_attr *sub;
    struct lys_module *moveto_mod;
    struct lyxp_set *set_all_desc = NULL;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(cur_node->schema->module->ctx, LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    /* prefix */
    if (strnchr(qname, ':', qname_len)) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, cur_node->schema->module->ctx, NULL, 1, 0);
        if (!moveto_mod) {
            LOGVAL(cur_node->schema->module->ctx, LYE_XPATH_INMOD, LY_VLOG_NONE, NULL, pref_len, qname);
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        moveto_mod = NULL;
    }

    /* can be optimized similarly to moveto_node_alldesc() and save considerable amount of memory,
     * but it likely won't be used much, so it's a waste of time */
    /* copy the context */
    set_all_desc = set_copy(set);
    /* get all descendant nodes (the original context nodes are removed) */
    ret = moveto_node_alldesc(set_all_desc, cur_node, "*", 1, options);
    if (ret) {
        lyxp_set_free(set_all_desc);
        return ret;
    }
    /* prepend the original context nodes */
    if (moveto_union(set, set_all_desc, cur_node, options)) {
        lyxp_set_free(set_all_desc);
        return -1;
    }
    lyxp_set_free(set_all_desc);

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    for (i = 0; i < set->used; ) {
        replaced = 0;

        /* only attributes of an elem can be in the result, skip all the rest,
         * we have all attributes qualified in lyd tree */
        if (set->val.nodes[i].type == LYXP_NODE_ELEM) {
            LY_TREE_FOR(set->val.nodes[i].node->attr, sub) {
                /* check "namespace" */
                if (moveto_mod && (sub->annotation->module != moveto_mod)) {
                    /* no match */
                    continue;
                }

                if (all || (!strncmp(sub->name, qname, qname_len) && !sub->name[qname_len])) {
                    /* match */
                    if (!replaced) {
                        set->val.attrs[i].attr = sub;
                        set->val.attrs[i].type = LYXP_NODE_ATTR;
                        /* pos does not change */
                        replaced = 1;
                    } else {
                        set_insert_node(set, (struct lyd_node *)sub, set->val.attrs[i].pos, LYXP_NODE_ATTR, i + 1);
                    }
                    ++i;
                }
            }
        }

        if (!replaced) {
            /* no match */
            set_remove_node(set, i);
        }
    }

    return EXIT_SUCCESS;
}

static int
moveto_self_add_children_r(const struct lyd_node *parent, uint32_t parent_pos, struct lyxp_set *to_set,
                           const struct lyxp_set *dup_check_set, enum lyxp_node_type root_type, int options)
{
    struct lyd_node *sub;
    int ret;

    /* add all the children ... */
    if (!(parent->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        LY_TREE_FOR(parent->child, sub) {
            /* context check */
            if ((root_type == LYXP_NODE_ROOT_CONFIG) && (sub->schema->flags & LYS_CONFIG_R)) {
                continue;
            }

            /* when check */
            if ((options & LYXP_WHEN) && !LYD_WHEN_DONE(sub->when_status)) {
                return EXIT_FAILURE;
            }

            if (!set_dup_node_check(dup_check_set, sub, LYXP_NODE_ELEM, -1)) {
                set_insert_node(to_set, sub, 0, LYXP_NODE_ELEM, to_set->used);

                /* skip anydata/anyxml and dummy nodes */
                if ((sub->schema->nodetype & LYS_ANYDATA) || (sub->validity & LYD_VAL_INUSE)) {
                    continue;
                }

                /* also add all the children of this node, recursively */
                ret = moveto_self_add_children_r(sub, 0, to_set, dup_check_set, root_type, options);
                if (ret) {
                    return ret;
                }
            }
        }

    /* ... or add their text node, ... */
    } else {
        /* ... but only non-empty */
        if (((struct lyd_node_leaf_list *)parent)->value_str) {
            if (!set_dup_node_check(dup_check_set, parent, LYXP_NODE_TEXT, -1)) {
                set_insert_node(to_set, parent, parent_pos, LYXP_NODE_TEXT, to_set->used);
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to self. Handles '/' or '//' and '.'. Result is LYXP_SET_NODE_SET
 *        (or LYXP_SET_EMPTY). Context position aware.
 *
 * @param[in,out] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] all_desc Whether to go to all descendants ('//') or not ('/').
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
moveto_self(struct lyxp_set *set, struct lyd_node *cur_node, int all_desc, int options)
{
    uint32_t i;
    enum lyxp_node_type root_type;
    struct lyxp_set ret_set;
    int ret;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(cur_node->schema->module->ctx, LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    /* nothing to do */
    if (!all_desc) {
        return EXIT_SUCCESS;
    }

    moveto_get_root(cur_node, options, &root_type);

    /* add all the children, they get added recursively */
    memset(&ret_set, 0, sizeof ret_set);
    for (i = 0; i < set->used; ++i) {
        /* copy the current node to tmp */
        set_insert_node(&ret_set, set->val.nodes[i].node, set->val.nodes[i].pos, set->val.nodes[i].type, ret_set.used);

        /* do not touch attributes and text nodes */
        if ((set->val.nodes[i].type == LYXP_NODE_TEXT) || (set->val.nodes[i].type == LYXP_NODE_ATTR)) {
            continue;
        }

        /* skip anydata/anyxml and dummy nodes */
        if ((set->val.nodes[i].node->schema->nodetype & LYS_ANYDATA) || (set->val.nodes[i].node->validity & LYD_VAL_INUSE)) {
            continue;
        }

        /* add all the children */
        ret = moveto_self_add_children_r(set->val.nodes[i].node, set->val.nodes[i].pos, &ret_set, set, root_type, options);
        if (ret) {
            set_free_content(&ret_set);
            return ret;
        }
    }

    /* use the temporary set as the current one */
    ret_set.ctx_pos = set->ctx_pos;
    ret_set.ctx_size = set->ctx_size;
    set_free_content(set);
    memcpy(set, &ret_set, sizeof *set);

    return EXIT_SUCCESS;
}

static int
moveto_snode_self(struct lyxp_set *set, struct lys_node *cur_node, int all_desc, int options)
{
    const struct lys_node *sub;
    uint32_t i;
    enum lyxp_node_type root_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_SNODE_SET) {
        LOGVAL(cur_node->module->ctx, LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    /* nothing to do */
    if (!all_desc) {
        return EXIT_SUCCESS;
    }

    moveto_snode_get_root(cur_node, options, &root_type);

    /* add all the children, they get added recursively */
    for (i = 0; i < set->used; ++i) {
        if (set->val.snodes[i].in_ctx != 1) {
            continue;
        }

        /* add all the children */
        if (set->val.snodes[i].snode->nodetype & (LYS_LIST | LYS_CONTAINER)) {
            sub = NULL;
            while ((sub = lys_getnext(sub, set->val.snodes[i].snode, NULL, LYS_GETNEXT_NOSTATECHECK))) {
                /* RPC input/output check */
                if (options & LYXP_SNODE_OUTPUT) {
                    if (lys_parent(sub)->nodetype == LYS_INPUT) {
                        continue;
                    }
                } else {
                    if (lys_parent(sub)->nodetype == LYS_OUTPUT) {
                        continue;
                    }
                }

                /* context check */
                if ((root_type == LYXP_NODE_ROOT_CONFIG) && (sub->flags & LYS_CONFIG_R)) {
                    continue;
                }

                set_snode_insert_node(set, sub, LYXP_NODE_ELEM);
                /* throw away the insert index, we want to consider that node again, recursively */
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to parent. Handles '/' or '//' and '..'. Result is LYXP_SET_NODE_SET
 *        (or LYXP_SET_EMPTY). Context position aware.
 *
 * @param[in] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] all_desc Whether to go to all descendants ('//') or not ('/').
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
moveto_parent(struct lyxp_set *set, struct lyd_node *cur_node, int all_desc, int options)
{
    struct ly_ctx *ctx = cur_node->schema->module->ctx;
    int ret;
    uint32_t i;
    struct lyd_node *node, *new_node;
    const struct lyd_node *root;
    enum lyxp_node_type root_type, new_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(ctx, LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    if (all_desc) {
        /* <path>//.. == <path>//./.. */
        ret = moveto_self(set, cur_node, 1, options);
        if (ret) {
            return ret;
        }
    }

    root = moveto_get_root(cur_node, options, &root_type);

    for (i = 0; i < set->used; ) {
        node = set->val.nodes[i].node;

        if (set->val.nodes[i].type == LYXP_NODE_ELEM) {
            new_node = node->parent;
        } else if (set->val.nodes[i].type == LYXP_NODE_TEXT) {
            new_node = node;
        } else if (set->val.nodes[i].type == LYXP_NODE_ATTR) {
            new_node = (struct lyd_node *)lyd_attr_parent(root, set->val.attrs[i].attr);
            if (!new_node) {
                LOGINT(ctx);
                return -1;
            }
        } else {
            /* root does not have a parent */
            set_remove_node(set, i);
            continue;
        }

        /* when check */
        if ((options & LYXP_WHEN) && new_node && !LYD_WHEN_DONE(new_node->when_status)) {
            return EXIT_FAILURE;
        }

        /* node already there can also be the root */
        if (root == node) {
            if (options && (cur_node->schema->flags & LYS_CONFIG_W)) {
                new_type = LYXP_NODE_ROOT_CONFIG;
            } else {
                new_type = LYXP_NODE_ROOT;
            }
            new_node = node;

        /* node has no parent */
        } else if (!new_node) {
            if (options && (cur_node->schema->flags & LYS_CONFIG_W)) {
                new_type = LYXP_NODE_ROOT_CONFIG;
            } else {
                new_type = LYXP_NODE_ROOT;
            }
#ifndef NDEBUG
            for (; node->prev->next; node = node->prev);
            if (node != root) {
                LOGINT(ctx);
            }
#endif
            new_node = (struct lyd_node *)root;

        /* node has a standard parent (it can equal the root, it's not the root yet since they are fake) */
        } else {
            new_type = LYXP_NODE_ELEM;
        }

        assert((new_type == LYXP_NODE_ELEM) || ((new_type == root_type) && (new_node == root)));

        if (set_dup_node_check(set, new_node, new_type, -1)) {
            set_remove_node(set, i);
        } else {
            set_replace_node(set, new_node, 0, new_type, i);
            ++i;
        }
    }

    assert(!set_sort(set, cur_node, options) && !set_sorted_dup_node_clean(set));

    return EXIT_SUCCESS;
}

static int
moveto_snode_parent(struct lyxp_set *set, struct lys_node *cur_node, int all_desc, int options)
{
    int idx, i, orig_used, temp_ctx = 0;
    struct lys_node *node, *new_node;
    const struct lys_node *root;
    enum lyxp_node_type root_type, new_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_SNODE_SET) {
        LOGVAL(cur_node->module->ctx, LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    if (all_desc) {
        /* <path>//.. == <path>//./.. */
        idx = moveto_snode_self(set, cur_node, 1, options);
        if (idx) {
            return idx;
        }
    }

    root = moveto_snode_get_root(cur_node, options, &root_type);

    orig_used = set->used;
    for (i = 0; i < orig_used; ++i) {
        if (set->val.snodes[i].in_ctx != 1) {
            continue;
        }
        set->val.snodes[i].in_ctx = 0;

        node = set->val.snodes[i].snode;

        if (set->val.snodes[i].type == LYXP_NODE_ELEM) {
            for (new_node = lys_parent(node);
                 new_node && (new_node->nodetype & (LYS_USES | LYS_CHOICE | LYS_CASE | LYS_INPUT | LYS_OUTPUT));
                 new_node = lys_parent(new_node));
        } else {
            /* root does not have a parent */
            continue;
        }

        /* node already there can also be the root */
        if (root == node) {
            if ((options & (LYXP_SNODE_MUST | LYXP_SNODE_WHEN)) && (cur_node->flags & LYS_CONFIG_W)) {
                new_type = LYXP_NODE_ROOT_CONFIG;
            } else {
                new_type = LYXP_NODE_ROOT;
            }
            new_node = node;

        /* node has no parent */
        } else if (!new_node) {
            if ((options & (LYXP_SNODE_MUST | LYXP_SNODE_WHEN)) && (cur_node->flags & LYS_CONFIG_W)) {
                new_type = LYXP_NODE_ROOT_CONFIG;
            } else {
                new_type = LYXP_NODE_ROOT;
            }
#ifndef NDEBUG
            node = (struct lys_node *)lys_getnext(NULL, NULL, lys_node_module(node), LYS_GETNEXT_NOSTATECHECK);
            if (node != root) {
                LOGINT(cur_node->module->ctx);
            }
#endif
            new_node = (struct lys_node *)root;

        /* node has a standard parent (it can equal the root, it's not the root yet since they are fake) */
        } else {
            new_type = LYXP_NODE_ELEM;
        }

        assert((new_type == LYXP_NODE_ELEM) || ((new_type == root_type) && (new_node == root)));

        idx = set_snode_insert_node(set, new_node, new_type);
        if ((idx < orig_used) && (idx > i)) {
            set->val.snodes[idx].in_ctx = 2;
            temp_ctx = 1;
        }
    }

    if (temp_ctx) {
        for (i = 0; i < orig_used; ++i) {
            if (set->val.snodes[i].in_ctx == 2) {
                set->val.snodes[i].in_ctx = 1;
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to the result of a comparison. Handles '=', '!=', '<=', '<', '>=', or '>'.
 *        Result is LYXP_SET_BOOLEAN. Indirectly context position aware.
 *
 * @param[in,out] set1 Set to use for the result.
 * @param[in] set2 Set acting as the second operand for \p op.
 * @param[in] op Comparison operator to process.
 * @param[in] cur_node Original context node.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
moveto_op_comp(struct lyxp_set *set1, struct lyxp_set *set2, const char *op, struct lyd_node *cur_node,
               struct lys_module *local_mod, int options)
{
    /*
     * NODE SET + NODE SET = NODE SET + STRING /(1 NODE SET) 2 STRING
     * NODE SET + STRING = STRING + STRING     /1 STRING (2 STRING)
     * NODE SET + NUMBER = NUMBER + NUMBER     /1 NUMBER (2 NUMBER)
     * NODE SET + BOOLEAN = BOOLEAN + BOOLEAN  /1 BOOLEAN (2 BOOLEAN)
     * STRING + NODE SET = STRING + STRING     /(1 STRING) 2 STRING
     * NUMBER + NODE SET = NUMBER + NUMBER     /(1 NUMBER) 2 NUMBER
     * BOOLEAN + NODE SET = BOOLEAN + BOOLEAN  /(1 BOOLEAN) 2 BOOLEAN
     *
     * '=' or '!='
     * BOOLEAN + BOOLEAN
     * BOOLEAN + STRING = BOOLEAN + BOOLEAN    /(1 BOOLEAN) 2 BOOLEAN
     * BOOLEAN + NUMBER = BOOLEAN + BOOLEAN    /(1 BOOLEAN) 2 BOOLEAN
     * STRING + BOOLEAN = BOOLEAN + BOOLEAN    /1 BOOLEAN (2 BOOLEAN)
     * NUMBER + BOOLEAN = BOOLEAN + BOOLEAN    /1 BOOLEAN (2 BOOLEAN)
     * NUMBER + NUMBER
     * NUMBER + STRING = NUMBER + NUMBER       /(1 NUMBER) 2 NUMBER
     * STRING + NUMBER = NUMBER + NUMBER       /1 NUMBER (2 NUMBER)
     * STRING + STRING
     *
     * '<=', '<', '>=', '>'
     * NUMBER + NUMBER
     * BOOLEAN + BOOLEAN = NUMBER + NUMBER     /1 NUMBER, 2 NUMBER
     * BOOLEAN + NUMBER = NUMBER + NUMBER      /1 NUMBER (2 NUMBER)
     * BOOLEAN + STRING = NUMBER + NUMBER      /1 NUMBER, 2 NUMBER
     * NUMBER + STRING = NUMBER + NUMBER       /(1 NUMBER) 2 NUMBER
     * STRING + STRING = NUMBER + NUMBER       /1 NUMBER, 2 NUMBER
     * STRING + NUMBER = NUMBER + NUMBER       /1 NUMBER (2 NUMBER)
     * NUMBER + BOOLEAN = NUMBER + NUMBER      /(1 NUMBER) 2 NUMBER
     * STRING + BOOLEAN = NUMBER + NUMBER      /(1 NUMBER) 2 NUMBER
     */
    struct lyxp_set iter1, iter2;
    int result;
    int64_t i;

    iter1.type = LYXP_SET_EMPTY;

    /* empty node-sets are always false */
    if ((set1->type == LYXP_SET_EMPTY) || (set2->type == LYXP_SET_EMPTY)) {
        set_fill_boolean(set1, 0);
        return EXIT_SUCCESS;
    }

    /* iterative evaluation with node-sets */
    if ((set1->type == LYXP_SET_NODE_SET) || (set2->type == LYXP_SET_NODE_SET)) {
        if (set1->type == LYXP_SET_NODE_SET) {
            for (i = 0; i < set1->used; ++i) {
                switch (set2->type) {
                case LYXP_SET_NUMBER:
                    if (set_comp_cast(&iter1, set1, LYXP_SET_NUMBER, cur_node, local_mod, i, options)) {
                        return -1;
                    }
                    break;
                case LYXP_SET_BOOLEAN:
                    if (set_comp_cast(&iter1, set1, LYXP_SET_BOOLEAN, cur_node, local_mod, i, options)) {
                        return -1;
                    }
                    break;
                default:
                    if (set_comp_cast(&iter1, set1, LYXP_SET_STRING, cur_node, local_mod, i, options)) {
                        return -1;
                    }
                    break;
                }

                if (moveto_op_comp(&iter1, set2, op, cur_node, local_mod, options)) {
                    set_free_content(&iter1);
                    return -1;
                }

                /* lazy evaluation until true */
                if (iter1.val.bool) {
                    set_fill_boolean(set1, 1);
                    return EXIT_SUCCESS;
                }
            }
        } else {
            for (i = 0; i < set2->used; ++i) {
                switch (set1->type) {
                    case LYXP_SET_NUMBER:
                        if (set_comp_cast(&iter2, set2, LYXP_SET_NUMBER, cur_node, local_mod, i, options)) {
                            return -1;
                        }
                        break;
                    case LYXP_SET_BOOLEAN:
                        if (set_comp_cast(&iter2, set2, LYXP_SET_BOOLEAN, cur_node, local_mod, i, options)) {
                            return -1;
                        }
                        break;
                    default:
                        if (set_comp_cast(&iter2, set2, LYXP_SET_STRING, cur_node, local_mod, i, options)) {
                            return -1;
                        }
                        break;
                }

                set_fill_set(&iter1, set1);

                if (moveto_op_comp(&iter1, &iter2, op, cur_node, local_mod, options)) {
                    set_free_content(&iter1);
                    set_free_content(&iter2);
                    return -1;
                }
                set_free_content(&iter2);

                /* lazy evaluation until true */
                if (iter1.val.bool) {
                    set_fill_boolean(set1, 1);
                    return EXIT_SUCCESS;
                }
            }
        }

        /* false for all nodes */
        set_fill_boolean(set1, 0);
        return EXIT_SUCCESS;
    }

    /* first convert properly */
    if ((op[0] == '=') || (op[0] == '!')) {
        if ((set1->type == LYXP_SET_BOOLEAN) || (set2->type == LYXP_SET_BOOLEAN)) {
            lyxp_set_cast(set1, LYXP_SET_BOOLEAN, cur_node, local_mod, options);
            lyxp_set_cast(set2, LYXP_SET_BOOLEAN, cur_node, local_mod, options);
        } else if ((set1->type == LYXP_SET_NUMBER) || (set2->type == LYXP_SET_NUMBER)) {
            if (lyxp_set_cast(set1, LYXP_SET_NUMBER, cur_node, local_mod, options)) {
                return -1;
            }
            if (lyxp_set_cast(set2, LYXP_SET_NUMBER, cur_node, local_mod, options)) {
                return -1;
            }
        } /* else we have 2 strings */
    } else {
        if (lyxp_set_cast(set1, LYXP_SET_NUMBER, cur_node, local_mod, options)) {
            return -1;
        }
        if (lyxp_set_cast(set2, LYXP_SET_NUMBER, cur_node, local_mod, options)) {
            return -1;
        }
    }

    assert(set1->type == set2->type);

    /* compute result */
    if (op[0] == '=') {
        if (set1->type == LYXP_SET_BOOLEAN) {
            result = (set1->val.bool == set2->val.bool);
        } else if (set1->type == LYXP_SET_NUMBER) {
            result = (set1->val.num == set2->val.num);
        } else {
            assert(set1->type == LYXP_SET_STRING);
            result = (ly_strequal(set1->val.str, set2->val.str, 0));
        }
    } else if (op[0] == '!') {
        if (set1->type == LYXP_SET_BOOLEAN) {
            result = (set1->val.bool != set2->val.bool);
        } else if (set1->type == LYXP_SET_NUMBER) {
            result = (set1->val.num != set2->val.num);
        } else {
            assert(set1->type == LYXP_SET_STRING);
            result = (!ly_strequal(set1->val.str, set2->val.str, 0));
        }
    } else {
        assert(set1->type == LYXP_SET_NUMBER);
        if (op[0] == '<') {
            if (op[1] == '=') {
                result = (set1->val.num <= set2->val.num);
            } else {
                result = (set1->val.num < set2->val.num);
            }
        } else {
            if (op[1] == '=') {
                result = (set1->val.num >= set2->val.num);
            } else {
                result = (set1->val.num > set2->val.num);
            }
        }
    }

    /* assign result */
    if (result) {
        set_fill_boolean(set1, 1);
    } else {
        set_fill_boolean(set1, 0);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to the result of a basic operation. Handles '+', '-', unary '-', '*', 'div',
 *        or 'mod'. Result is LYXP_SET_NUMBER. Indirectly context position aware.
 *
 * @param[in,out] set1 Set to use for the result.
 * @param[in] set2 Set acting as the second operand for \p op.
 * @param[in] op Operator to process.
 * @param[in] cur_node Original context node.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
moveto_op_math(struct lyxp_set *set1, struct lyxp_set *set2, const char *op, struct lyd_node *cur_node,
               struct lys_module *local_mod, int options)
{
    /* unary '-' */
    if (!set2 && (op[0] == '-')) {
        if (lyxp_set_cast(set1, LYXP_SET_NUMBER, cur_node, local_mod, options)) {
            return -1;
        }
        set1->val.num *= -1;
        lyxp_set_free(set2);
        return EXIT_SUCCESS;
    }

    assert(set1 && set2);

    if (lyxp_set_cast(set1, LYXP_SET_NUMBER, cur_node, local_mod, options)) {
        return -1;
    }
    if (lyxp_set_cast(set2, LYXP_SET_NUMBER, cur_node, local_mod, options)) {
        return -1;
    }

    switch (op[0]) {
    /* '+' */
    case '+':
        set1->val.num += set2->val.num;
        break;

    /* '-' */
    case '-':
        set1->val.num -= set2->val.num;
        break;

    /* '*' */
    case '*':
        set1->val.num *= set2->val.num;
        break;

    /* 'div' */
    case 'd':
        set1->val.num /= set2->val.num;
        break;

    /* 'mod' */
    case 'm':
        set1->val.num = ((long long)set1->val.num) % ((long long)set2->val.num);
        break;

    default:
        LOGINT(local_mod->ctx);
        return -1;
    }

    return EXIT_SUCCESS;
}

/*
 * eval functions
 *
 * They execute a parsed XPath expression on some data subtree.
 */

/**
 * @brief Evaluate Literal. Logs directly on error.
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static void
eval_literal(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyxp_set *set)
{
    if (set) {
        if (exp->tok_len[*exp_idx] == 2) {
            set_fill_string(set, "", 0);
        } else {
            set_fill_string(set, &exp->expr[exp->expr_pos[*exp_idx] + 1], exp->tok_len[*exp_idx] - 2);
        }
    }
    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
    ++(*exp_idx);
}

/**
 * @brief Evaluate NodeTest. Logs directly on error.
 *
 * [6] NodeTest ::= NameTest | NodeType '(' ')'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in] attr_axis Whether to search attributes or standard nodes.
 * @param[in] all_desc Whether to search all the descendants or children only.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_node_test(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lys_module *local_mod,
               int attr_axis, int all_desc, struct lyxp_set *set, int options)
{
    int i, rc = 0;
    char *path;

    switch (exp->tokens[*exp_idx]) {
    case LYXP_TOKEN_NAMETEST:
        if (attr_axis) {
            if (set && (options & LYXP_SNODE_ALL)) {
                set_snode_clear_ctx(set);
            } else {
                if (all_desc) {
                    rc = moveto_attr_alldesc(set, cur_node, &exp->expr[exp->expr_pos[*exp_idx]],
                                             exp->tok_len[*exp_idx], options);
                } else {
                    rc = moveto_attr(set, cur_node, &exp->expr[exp->expr_pos[*exp_idx]], exp->tok_len[*exp_idx],
                                     options);
                }
            }
        } else {
            if (all_desc) {
                if (set && (options & LYXP_SNODE_ALL)) {
                    rc = moveto_snode_alldesc(set, (struct lys_node *)cur_node, &exp->expr[exp->expr_pos[*exp_idx]],
                                              exp->tok_len[*exp_idx], options);
                } else {
                    rc = moveto_node_alldesc(set, cur_node, &exp->expr[exp->expr_pos[*exp_idx]],
                                             exp->tok_len[*exp_idx], options);
                }
            } else {
                if (set && (options & LYXP_SNODE_ALL)) {
                    rc = moveto_snode(set, (struct lys_node *)cur_node, &exp->expr[exp->expr_pos[*exp_idx]],
                                      exp->tok_len[*exp_idx], options);
                } else {
                    rc = moveto_node(set, cur_node, &exp->expr[exp->expr_pos[*exp_idx]], exp->tok_len[*exp_idx],
                                     options);
                }
            }

            if (!rc && set && (options & LYXP_SNODE_ALL)) {
                for (i = set->used - 1; i > -1; --i) {
                    if (set->val.snodes[i].in_ctx) {
                        break;
                    }
                }
                if (i == -1) {
                    path = lys_path((struct lys_node *)cur_node, LYS_PATH_FIRST_PREFIX);
                    LOGWRN(local_mod->ctx, "Schema node \"%.*s\" not found (%.*s) with context node \"%s\".",
                           exp->tok_len[*exp_idx], &exp->expr[exp->expr_pos[*exp_idx]],
                           exp->expr_pos[*exp_idx] + exp->tok_len[*exp_idx], exp->expr, path);
                    free(path);
                }
            }
        }
        if (rc) {
            return rc;
        }

        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);
        break;

    case LYXP_TOKEN_NODETYPE:
        if (set) {
            assert(exp->tok_len[*exp_idx] == 4);
            if (set->type == LYXP_SET_SNODE_SET) {
                set_snode_clear_ctx(set);
                /* just for the debug message underneath */
                set = NULL;
            } else {
                if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "node", 4)) {
                    if (xpath_node(NULL, 0, cur_node, local_mod, set, options)) {
                        return -1;
                    }
                } else {
                    assert(!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "text", 4));
                    if (xpath_text(NULL, 0, cur_node, local_mod, set, options)) {
                        return -1;
                    }
                }
            }
        }
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        /* '(' */
        assert(exp->tokens[*exp_idx] == LYXP_TOKEN_PAR1);
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        /* ')' */
        assert(exp->tokens[*exp_idx] == LYXP_TOKEN_PAR2);
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);
        break;

    default:
        LOGINT(local_mod->ctx);
        return -1;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate Predicate. Logs directly on error.
 *
 * [7] Predicate ::= '[' Expr ']'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_predicate(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lys_module *local_mod,
               struct lyxp_set *set, int options, int parent_pos_pred)
{
    int ret;
    uint16_t i, orig_exp, brack2_exp, open_brack;
    uint32_t orig_pos, orig_size, pred_in_ctx;
    struct lyxp_set set2;
    struct lyd_node *orig_parent;

    /* '[' */
    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
           print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
    ++(*exp_idx);

    if (!set) {
only_parse:
        ret = eval_expr_select(exp, exp_idx, 0, cur_node, local_mod, NULL, options);
        if (ret == -1 || ret == EXIT_FAILURE) {
            return ret;
        }
    } else if (set->type == LYXP_SET_NODE_SET) {
        /* we (possibly) need the set sorted, it can affect the result (if the predicate result is a number) */
        assert(!set_sort(set, cur_node, options));

        /* empty set, nothing to evaluate */
        if (!set->used) {
            goto only_parse;
        }

        orig_exp = *exp_idx;

        /* find the predicate end */
        open_brack = 0;
        for (brack2_exp = orig_exp; open_brack || (exp->tokens[brack2_exp] != LYXP_TOKEN_BRACK2); ++brack2_exp) {
            if (exp->tokens[brack2_exp] == LYXP_TOKEN_BRACK1) {
                ++open_brack;
            } else if (exp->tokens[brack2_exp] == LYXP_TOKEN_BRACK2) {
                --open_brack;
            }
        }

        orig_pos = 0;
        orig_size = set->used;
        orig_parent = NULL;
        for (i = 0; i < set->used; ) {
            memset(&set2, 0, sizeof set2);
            set_insert_node(&set2, set->val.nodes[i].node, set->val.nodes[i].pos, set->val.nodes[i].type, 0);
            /* remember the node context position for position() and context size for last(),
             * predicates should always be evaluated with respect to the child axis (since we do
             * not support explicit axes) so we assign positions based on their parents */
            if (parent_pos_pred && (set->val.nodes[i].node->parent != orig_parent)) {
                orig_parent = set->val.nodes[i].node->parent;
                orig_pos = 1;
            } else {
                ++orig_pos;
            }

            set2.ctx_pos = orig_pos;
            set2.ctx_size = orig_size;
            *exp_idx = orig_exp;

            ret = eval_expr_select(exp, exp_idx, 0, cur_node, local_mod, &set2, options);
            if (ret == -1 || ret == EXIT_FAILURE) {
                lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, local_mod, options);
                return ret;
            }

            /* number is a position */
            if (set2.type == LYXP_SET_NUMBER) {
                if ((long long)set2.val.num == orig_pos) {
                    set2.val.num = 1;
                } else {
                    set2.val.num = 0;
                }
            }
            lyxp_set_cast(&set2, LYXP_SET_BOOLEAN, cur_node, local_mod, options);

            /* predicate satisfied or not? */
            if (set2.val.bool) {
                ++i;
            } else {
                set_remove_node(set, i);
            }
        }

    } else if (set->type == LYXP_SET_SNODE_SET) {
        for (i = 0; i < set->used; ++i) {
            if (set->val.snodes[i].in_ctx == 1) {
                /* there is a currently-valid node */
                break;
            }
        }
        /* empty set, nothing to evaluate */
        if (i == set->used) {
            goto only_parse;
        }

        orig_exp = *exp_idx;

        /* find the predicate end */
        open_brack = 0;
        for (brack2_exp = orig_exp; open_brack || (exp->tokens[brack2_exp] != LYXP_TOKEN_BRACK2); ++brack2_exp) {
            if (exp->tokens[brack2_exp] == LYXP_TOKEN_BRACK1) {
                ++open_brack;
            } else if (exp->tokens[brack2_exp] == LYXP_TOKEN_BRACK2) {
                --open_brack;
            }
        }

        /* set special in_ctx to all the valid snodes */
        pred_in_ctx = set_snode_new_in_ctx(set);

        /* use the valid snodes one-by-one */
        for (i = 0; i < set->used; ++i) {
            if (set->val.snodes[i].in_ctx != pred_in_ctx) {
                continue;
            }
            set->val.snodes[i].in_ctx = 1;

            *exp_idx = orig_exp;

            ret = eval_expr_select(exp, exp_idx, 0, cur_node, local_mod, set, options);
            if (ret == -1 || ret == EXIT_FAILURE) {
                return ret;
            }

            set->val.snodes[i].in_ctx = pred_in_ctx;
        }

        /* restore the state as it was before the predicate */
        for (i = 0; i < set->used; ++i) {
            if (set->val.snodes[i].in_ctx == 1) {
                set->val.snodes[i].in_ctx = 0;
            } else if (set->val.snodes[i].in_ctx == pred_in_ctx) {
                set->val.snodes[i].in_ctx = 1;
            }
        }

    } else {
        set2.type = LYXP_SET_EMPTY;
        set_fill_set(&set2, set);

        ret = eval_expr_select(exp, exp_idx, 0, cur_node, local_mod, &set2, options);
        if (ret == -1 || ret == EXIT_FAILURE) {
            lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, local_mod, options);
            return ret;
        }

        lyxp_set_cast(&set2, LYXP_SET_BOOLEAN, cur_node, local_mod, options);
        if (!set2.val.bool) {
            lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, local_mod, options);
        }
        lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, local_mod, options);
    }

    /* ']' */
    assert(exp->tokens[*exp_idx] == LYXP_TOKEN_BRACK2);
    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
           print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
    ++(*exp_idx);

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate RelativeLocationPath. Logs directly on error.
 *
 * [4] RelativeLocationPath ::= Step | RelativeLocationPath '/' Step | RelativeLocationPath '//' Step
 * [5] Step ::= '@'? NodeTest Predicate* | '.' | '..'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in] all_desc Whether to search all the descendants or children only.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_relative_location_path(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lys_module *local_mod,
                            int all_desc, struct lyxp_set *set, int options)
{
    int attr_axis, ret;

    goto step;
    do {
        /* evaluate '/' or '//' */
        if (exp->tok_len[*exp_idx] == 1) {
            all_desc = 0;
        } else {
            assert(exp->tok_len[*exp_idx] == 2);
            all_desc = 1;
        }
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

step:
        /* Step */
        attr_axis = 0;
        switch (exp->tokens[*exp_idx]) {
        case LYXP_TOKEN_DOT:
            /* evaluate '.' */
            if (set && (options & LYXP_SNODE_ALL)) {
                ret = moveto_snode_self(set, (struct lys_node *)cur_node, all_desc, options);
            } else {
                ret = moveto_self(set, cur_node, all_desc, options);
            }
            if (ret) {
                return ret;
            }
            LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
            ++(*exp_idx);
            break;

        case LYXP_TOKEN_DDOT:
            /* evaluate '..' */
            if (set && (options & LYXP_SNODE_ALL)) {
                ret = moveto_snode_parent(set, (struct lys_node *)cur_node, all_desc, options);
            } else {
                ret = moveto_parent(set, cur_node, all_desc, options);
            }
            if (ret) {
                return ret;
            }
            LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
            ++(*exp_idx);
            break;

        case LYXP_TOKEN_AT:
            /* evaluate '@' */
            attr_axis = 1;
            LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
            ++(*exp_idx);

            /* fall through */
        case LYXP_TOKEN_NAMETEST:
        case LYXP_TOKEN_NODETYPE:
            ret = eval_node_test(exp, exp_idx, cur_node, local_mod, attr_axis, all_desc, set, options);
            if (ret) {
                return ret;
            }

            while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_BRACK1)) {
                ret = eval_predicate(exp, exp_idx, cur_node, local_mod, set, options, 1);
                if (ret) {
                    return ret;
                }
            }
            break;

        default:
            LOGINT(local_mod->ctx);
            return -1;
        }
    } while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_OPERATOR_PATH));

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate AbsoluteLocationPath. Logs directly on error.
 *
 * [3] AbsoluteLocationPath ::= '/' RelativeLocationPath? | '//' RelativeLocationPath
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_absolute_location_path(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lys_module *local_mod,
                            struct lyxp_set *set, int options)
{
    int all_desc, ret;

    if (set) {
        /* no matter what tokens follow, we need to be at the root */
        if (options & LYXP_SNODE_ALL) {
            moveto_snode_root(set, (struct lys_node *)cur_node, options);
        } else {
            moveto_root(set, cur_node, options);
        }
    }

    /* '/' RelativeLocationPath? */
    if (exp->tok_len[*exp_idx] == 1) {
        /* evaluate '/' - deferred */
        all_desc = 0;
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        if (exp_check_token(local_mod->ctx, exp, *exp_idx, LYXP_TOKEN_NONE, 0)) {
            return EXIT_SUCCESS;
        }
        switch (exp->tokens[*exp_idx]) {
        case LYXP_TOKEN_DOT:
        case LYXP_TOKEN_DDOT:
        case LYXP_TOKEN_AT:
        case LYXP_TOKEN_NAMETEST:
        case LYXP_TOKEN_NODETYPE:
            ret = eval_relative_location_path(exp, exp_idx, cur_node, local_mod, all_desc, set, options);
            if (ret) {
                return ret;
            }
            break;
        default:
            break;
        }

    /* '//' RelativeLocationPath */
    } else {
        /* evaluate '//' - deferred so as not to waste memory by remembering all the nodes */
        all_desc = 1;
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        ret =  eval_relative_location_path(exp, exp_idx, cur_node, local_mod, all_desc, set, options);
        if (ret) {
            return ret;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate FunctionCall. Logs directly on error.
 *
 * [9] FunctionCall ::= FunctionName '(' ( Expr ( ',' Expr )* )? ')'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_function_call(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lys_module *local_mod,
                   struct lyxp_set *set, int options)
{
    int rc = EXIT_FAILURE;
    int (*xpath_func)(struct lyxp_set **, uint16_t, struct lyd_node *, struct lys_module *, struct lyxp_set *, int) = NULL;
    uint16_t arg_count = 0, i, func_exp = *exp_idx;
    struct lyxp_set **args = NULL, **args_aux;

    if (set) {
        /* FunctionName */
        switch (exp->tok_len[*exp_idx]) {
        case 3:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "not", 3)) {
                xpath_func = &xpath_not;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "sum", 3)) {
                xpath_func = &xpath_sum;
            }
            break;
        case 4:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "lang", 4)) {
                xpath_func = &xpath_lang;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "last", 4)) {
                xpath_func = &xpath_last;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "name", 4)) {
                xpath_func = &xpath_name;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "true", 4)) {
                xpath_func = &xpath_true;
            }
            break;
        case 5:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "count", 5)) {
                xpath_func = &xpath_count;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "false", 5)) {
                xpath_func = &xpath_false;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "floor", 5)) {
                xpath_func = &xpath_floor;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "round", 5)) {
                xpath_func = &xpath_round;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "deref", 5)) {
                xpath_func = &xpath_deref;
            }
            break;
        case 6:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "concat", 6)) {
                xpath_func = &xpath_concat;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "number", 6)) {
                xpath_func = &xpath_number;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "string", 6)) {
                xpath_func = &xpath_string;
            }
            break;
        case 7:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "boolean", 7)) {
                xpath_func = &xpath_boolean;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "ceiling", 7)) {
                xpath_func = &xpath_ceiling;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "current", 7)) {
                xpath_func = &xpath_current;
            }
            break;
        case 8:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "contains", 8)) {
                xpath_func = &xpath_contains;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "position", 8)) {
                xpath_func = &xpath_position;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "re-match", 8)) {
                xpath_func = &xpath_re_match;
            }
            break;
        case 9:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "substring", 9)) {
                xpath_func = &xpath_substring;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "translate", 9)) {
                xpath_func = &xpath_translate;
            }
            break;
        case 10:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "local-name", 10)) {
                xpath_func = &xpath_local_name;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "enum-value", 10)) {
                xpath_func = &xpath_enum_value;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "bit-is-set", 10)) {
                xpath_func = &xpath_bit_is_set;
            }
            break;
        case 11:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "starts-with", 11)) {
                xpath_func = &xpath_starts_with;
            }
            break;
        case 12:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "derived-from", 12)) {
                xpath_func = &xpath_derived_from;
            }
            break;
        case 13:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "namespace-uri", 13)) {
                xpath_func = &xpath_namespace_uri;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "string-length", 13)) {
                xpath_func = &xpath_string_length;
            }
            break;
        case 15:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "normalize-space", 15)) {
                xpath_func = &xpath_normalize_space;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "substring-after", 15)) {
                xpath_func = &xpath_substring_after;
            }
            break;
        case 16:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "substring-before", 16)) {
                xpath_func = &xpath_substring_before;
            }
            break;
        case 20:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "derived-from-or-self", 20)) {
                xpath_func = &xpath_derived_from_or_self;
            }
            break;
        }

        if (!xpath_func) {
            LOGVAL(local_mod->ctx, LYE_XPATH_INTOK, LY_VLOG_NONE, NULL, "Unknown", &exp->expr[exp->expr_pos[*exp_idx]]);
            LOGVAL(local_mod->ctx, LYE_SPEC, LY_VLOG_NONE, NULL,
                   "Unknown XPath function \"%.*s\".", exp->tok_len[*exp_idx], &exp->expr[exp->expr_pos[*exp_idx]]);
            return -1;
        }
    }

    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
    ++(*exp_idx);

    /* '(' */
    assert(exp->tokens[*exp_idx] == LYXP_TOKEN_PAR1);
    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
    ++(*exp_idx);

    /* ( Expr ( ',' Expr )* )? */
    if (exp->tokens[*exp_idx] != LYXP_TOKEN_PAR2) {
        if (set) {
            args = malloc(sizeof *args);
            LY_CHECK_ERR_GOTO(!args, LOGMEM(local_mod->ctx), cleanup);
            arg_count = 1;
            args[0] = set_copy(set);
            if (!args[0]) {
                goto cleanup;
            }

            rc = eval_expr_select(exp, exp_idx, 0, cur_node, local_mod, args[0], options);
            if (rc == -1 || rc == EXIT_FAILURE) {
                goto cleanup;
            }
        } else {
            rc = eval_expr_select(exp, exp_idx, 0, cur_node, local_mod, NULL, options);
            if (rc == -1 || rc == EXIT_FAILURE) {
                goto cleanup;
            }
        }
    }
    while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_COMMA)) {
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        if (set) {
            ++arg_count;
            args_aux = realloc(args, arg_count * sizeof *args);
            LY_CHECK_ERR_GOTO(!args_aux, arg_count--; LOGMEM(local_mod->ctx), cleanup);
            args = args_aux;
            args[arg_count - 1] = set_copy(set);
            if (!args[arg_count - 1]) {
                goto cleanup;
            }

            rc = eval_expr_select(exp, exp_idx, 0, cur_node, local_mod, args[arg_count - 1], options);
            if (rc == -1 || rc == EXIT_FAILURE) {
                goto cleanup;
            }
        } else {
            rc = eval_expr_select(exp, exp_idx, 0, cur_node, local_mod, NULL, options);
            if (rc == -1 || rc == EXIT_FAILURE) {
                goto cleanup;
            }
        }
    }

    /* ')' */
    assert(exp->tokens[*exp_idx] == LYXP_TOKEN_PAR2);
    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
    ++(*exp_idx);

    if (set) {
        /* evaluate function */
        rc = xpath_func(args, arg_count, cur_node, local_mod, set, options);

        if (options & LYXP_SNODE_ALL) {
            if (rc == EXIT_FAILURE) {
                /* some validation warning */
                LOGWRN(local_mod->ctx, "Previous warning generated by XPath function \"%.*s\".",
                       (exp->expr_pos[*exp_idx - 1] - exp->expr_pos[func_exp]) + 1, &exp->expr[exp->expr_pos[func_exp]]);
                rc = EXIT_SUCCESS;
            }

            /* merge all nodes from arg evaluations */
            for (i = 0; i < arg_count; ++i) {
                set_snode_clear_ctx(args[i]);
                set_snode_merge(set, args[i]);
            }
        }
    } else {
        rc = EXIT_SUCCESS;
    }

cleanup:
    for (i = 0; i < arg_count; ++i) {
        lyxp_set_free(args[i]);
    }
    free(args);

    return rc;
}

/**
 * @brief Evaluate Number. Logs directly on error.
 *
 * @param[in] ctx Context for errors.
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_number(struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *exp_idx, struct lyxp_set *set)
{
    long double num;
    char *endptr;

    if (set) {
        errno = 0;
        num = strtold(&exp->expr[exp->expr_pos[*exp_idx]], &endptr);
        if (errno) {
            LOGVAL(ctx, LYE_XPATH_INTOK, LY_VLOG_NONE, NULL, "Unknown", &exp->expr[exp->expr_pos[*exp_idx]]);
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Failed to convert \"%.*s\" into a long double (%s).",
                   exp->tok_len[*exp_idx], &exp->expr[exp->expr_pos[*exp_idx]], strerror(errno));
            return -1;
        } else if (endptr - &exp->expr[exp->expr_pos[*exp_idx]] != exp->tok_len[*exp_idx]) {
            LOGVAL(ctx, LYE_XPATH_INTOK, LY_VLOG_NONE, NULL, "Unknown", &exp->expr[exp->expr_pos[*exp_idx]]);
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Failed to convert \"%.*s\" into a long double.",
                   exp->tok_len[*exp_idx], &exp->expr[exp->expr_pos[*exp_idx]]);
            return -1;
        }

        set_fill_number(set, num);
    }

    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
    ++(*exp_idx);
    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate PathExpr. Logs directly on error.
 *
 * [10] PathExpr ::= LocationPath | PrimaryExpr Predicate*
 *                 | PrimaryExpr Predicate* '/' RelativeLocationPath
 *                 | PrimaryExpr Predicate* '//' RelativeLocationPath
 * [2] LocationPath ::= RelativeLocationPath | AbsoluteLocationPath
 * [8] PrimaryExpr ::= '(' Expr ')' | Literal | Number | FunctionCall
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_path_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lys_module *local_mod,
               struct lyxp_set *set, int options)
{
    int all_desc, ret, parent_pos_pred;

    switch (exp->tokens[*exp_idx]) {
    case LYXP_TOKEN_PAR1:
        /* '(' Expr ')' */

        /* '(' */
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        /* Expr */
        ret = eval_expr_select(exp, exp_idx, 0, cur_node, local_mod, set, options);
        if (ret == -1 || ret == EXIT_FAILURE) {
            return ret;
        }

        /* ')' */
        assert(exp->tokens[*exp_idx] == LYXP_TOKEN_PAR2);
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        parent_pos_pred = 0;
        goto predicate;

    case LYXP_TOKEN_DOT:
    case LYXP_TOKEN_DDOT:
    case LYXP_TOKEN_AT:
    case LYXP_TOKEN_NAMETEST:
    case LYXP_TOKEN_NODETYPE:
        /* RelativeLocationPath */
        ret = eval_relative_location_path(exp, exp_idx, cur_node, local_mod, 0, set, options);
        if (ret) {
            return ret;
        }
        break;

    case LYXP_TOKEN_FUNCNAME:
        /* FunctionCall */
        if (!set) {
            ret = eval_function_call(exp, exp_idx, cur_node, local_mod, NULL, options);
        } else {
            ret = eval_function_call(exp, exp_idx, cur_node, local_mod, set, options);
        }
        if (ret) {
            return ret;
        }

        parent_pos_pred = 1;
        goto predicate;

    case LYXP_TOKEN_OPERATOR_PATH:
        /* AbsoluteLocationPath */
        ret = eval_absolute_location_path(exp, exp_idx, cur_node, local_mod, set, options);
        if (ret) {
            return ret;
        }
        break;

    case LYXP_TOKEN_LITERAL:
        /* Literal */
        if (!set || (options & LYXP_SNODE_ALL)) {
            if (set) {
                set_snode_clear_ctx(set);
            }
            eval_literal(exp, exp_idx, NULL);
        } else {
            eval_literal(exp, exp_idx, set);
        }

        parent_pos_pred = 1;
        goto predicate;

    case LYXP_TOKEN_NUMBER:
        /* Number */
        if (!set || (options & LYXP_SNODE_ALL)) {
            if (set) {
                set_snode_clear_ctx(set);
            }
            ret = eval_number(local_mod->ctx, exp, exp_idx, NULL);
        } else {
            ret = eval_number(local_mod->ctx, exp, exp_idx, set);
        }
        if (ret) {
            return ret;
        }

        parent_pos_pred = 1;
        goto predicate;

    default:
        LOGVAL(local_mod->ctx, LYE_XPATH_INTOK, LY_VLOG_NONE, NULL,
               print_token(exp->tokens[*exp_idx]), &exp->expr[exp->expr_pos[*exp_idx]]);
        return -1;
    }

    return EXIT_SUCCESS;

predicate:
    /* Predicate* */
    while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_BRACK1)) {
        ret = eval_predicate(exp, exp_idx, cur_node, local_mod, set, options, parent_pos_pred);
        if (ret) {
            return ret;
        }
    }

    /* ('/' or '//') RelativeLocationPath */
    if ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_OPERATOR_PATH)) {

        /* evaluate '/' or '//' */
        if (exp->tok_len[*exp_idx] == 1) {
            all_desc = 0;
        } else {
            assert(exp->tok_len[*exp_idx] == 2);
            all_desc = 1;
        }

        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        ret = eval_relative_location_path(exp, exp_idx, cur_node, local_mod, all_desc, set, options);
        if (ret) {
            return ret;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate UnionExpr. Logs directly on error.
 *
 * [18] UnionExpr ::= PathExpr | UnionExpr '|' PathExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_union_expr(struct lyxp_expr *exp, uint16_t *exp_idx, uint16_t repeat, struct lyd_node *cur_node,
                struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    int ret;
    struct lyxp_set orig_set, set2;
    uint16_t i;

    assert(repeat);

    memset(&orig_set, 0, sizeof orig_set);
    memset(&set2, 0, sizeof set2);

    set_fill_set(&orig_set, set);

    ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_UNION, cur_node, local_mod, set, options);
    if (ret) {
        goto finish;
    }

    /* ('|' PathExpr)* */
    for (i = 0; i < repeat; ++i) {
        assert(exp->tokens[*exp_idx] == LYXP_TOKEN_OPERATOR_UNI);
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        if (!set) {
            ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_UNION, cur_node, local_mod, NULL, options);
            if (ret) {
                goto finish;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set);
        ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_UNION, cur_node, local_mod, &set2, options);
        if (ret) {
            goto finish;
        }

        /* eval */
        if (options & LYXP_SNODE_ALL) {
            set_snode_merge(set, &set2);
        } else if (moveto_union(set, &set2, cur_node, options)) {
            ret = -1;
            goto finish;
        }
    }

finish:
    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, local_mod, options);
    lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, local_mod, options);
    return ret;
}

/**
 * @brief Evaluate UnaryExpr. Logs directly on error.
 *
 * [17] UnaryExpr ::= UnionExpr | '-' UnaryExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_unary_expr(struct lyxp_expr *exp, uint16_t *exp_idx, uint16_t repeat, struct lyd_node *cur_node,
                struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    int ret;
    uint16_t this_op, i;

    assert(repeat);

    /* ('-')+ */
    this_op = *exp_idx;
    for (i = 0; i < repeat; ++i) {
        assert(!exp_check_token(local_mod->ctx, exp, *exp_idx, LYXP_TOKEN_OPERATOR_MATH, 0) && (exp->expr[exp->expr_pos[*exp_idx]] == '-'));

        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);
    }

    ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_UNARY, cur_node, local_mod, set, options);
    if (ret) {
        return ret;
    }

    if (set && (repeat % 2)) {
        if (options & LYXP_SNODE_ALL) {
            warn_operands(local_mod->ctx, set, NULL, 1, exp->expr, exp->expr_pos[this_op]);
        } else {
            if (moveto_op_math(set, NULL, &exp->expr[exp->expr_pos[this_op]], cur_node, local_mod, options)) {
                return -1;
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate MultiplicativeExpr. Logs directly on error.
 *
 * [16] MultiplicativeExpr ::= UnaryExpr
 *                     | MultiplicativeExpr '*' UnaryExpr
 *                     | MultiplicativeExpr 'div' UnaryExpr
 *                     | MultiplicativeExpr 'mod' UnaryExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_multiplicative_expr(struct lyxp_expr *exp, uint16_t *exp_idx, uint16_t repeat, struct lyd_node *cur_node,
                         struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    int ret;
    uint16_t this_op;
    struct lyxp_set orig_set, set2;
    uint16_t i;

    assert(repeat);

    memset(&orig_set, 0, sizeof orig_set);
    memset(&set2, 0, sizeof set2);

    set_fill_set(&orig_set, set);

    ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_MULTIPLICATIVE, cur_node, local_mod, set, options);
    if (ret) {
        goto finish;
    }

    /* ('*' / 'div' / 'mod' UnaryExpr)* */
    for (i = 0; i < repeat; ++i) {
        this_op = *exp_idx;

        assert(exp->tokens[*exp_idx] == LYXP_TOKEN_OPERATOR_MATH);
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        if (!set) {
            ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_MULTIPLICATIVE, cur_node, local_mod, NULL, options);
            if (ret) {
                goto finish;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set);
        ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_MULTIPLICATIVE, cur_node, local_mod, &set2, options);
        if (ret) {
            goto finish;
        }

        /* eval */
        if (options & LYXP_SNODE_ALL) {
            warn_operands(local_mod->ctx, set, &set2, 1, exp->expr, exp->expr_pos[this_op - 1]);
            set_snode_merge(set, &set2);
            set_snode_clear_ctx(set);
        } else {
            if (moveto_op_math(set, &set2, &exp->expr[exp->expr_pos[this_op]], cur_node, local_mod, options)) {
                ret = -1;
                goto finish;
            }
        }
    }

finish:
    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, local_mod, options);
    lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, local_mod, options);
    return ret;
}

/**
 * @brief Evaluate AdditiveExpr. Logs directly on error.
 *
 * [15] AdditiveExpr ::= MultiplicativeExpr
 *                     | AdditiveExpr '+' MultiplicativeExpr
 *                     | AdditiveExpr '-' MultiplicativeExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_additive_expr(struct lyxp_expr *exp, uint16_t *exp_idx, uint16_t repeat, struct lyd_node *cur_node,
                   struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    int ret;
    uint16_t this_op;
    struct lyxp_set orig_set, set2;
    uint16_t i;

    assert(repeat);

    memset(&orig_set, 0, sizeof orig_set);
    memset(&set2, 0, sizeof set2);

    set_fill_set(&orig_set, set);

    ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_ADDITIVE, cur_node, local_mod, set, options);
    if (ret) {
        goto finish;
    }

    /* ('+' / '-' MultiplicativeExpr)* */
    for (i = 0; i < repeat; ++i) {
        this_op = *exp_idx;

        assert(exp->tokens[*exp_idx] == LYXP_TOKEN_OPERATOR_MATH);
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        if (!set) {
            ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_ADDITIVE, cur_node, local_mod, NULL, options);
            if (ret) {
                goto finish;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set);
        ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_ADDITIVE, cur_node, local_mod, &set2, options);
        if (ret) {
            goto finish;
        }

        /* eval */
        if (options & LYXP_SNODE_ALL) {
            warn_operands(local_mod->ctx, set, &set2, 1, exp->expr, exp->expr_pos[this_op - 1]);
            set_snode_merge(set, &set2);
            set_snode_clear_ctx(set);
        } else {
            if (moveto_op_math(set, &set2, &exp->expr[exp->expr_pos[this_op]], cur_node, local_mod, options)) {
                goto finish;
            }
        }
    }

finish:
    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, local_mod, options);
    lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, local_mod, options);
    return ret;
}

/**
 * @brief Evaluate RelationalExpr. Logs directly on error.
 *
 * [14] RelationalExpr ::= AdditiveExpr
 *                       | RelationalExpr '<' AdditiveExpr
 *                       | RelationalExpr '>' AdditiveExpr
 *                       | RelationalExpr '<=' AdditiveExpr
 *                       | RelationalExpr '>=' AdditiveExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_relational_expr(struct lyxp_expr *exp, uint16_t *exp_idx, uint16_t repeat, struct lyd_node *cur_node,
                     struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    int ret;
    uint16_t this_op;
    struct lyxp_set orig_set, set2;
    uint16_t i;

    assert(repeat);

    memset(&orig_set, 0, sizeof orig_set);
    memset(&set2, 0, sizeof set2);

    set_fill_set(&orig_set, set);

    ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_RELATIONAL, cur_node, local_mod, set, options);
    if (ret) {
        goto finish;
    }

    /* ('<' / '>' / '<=' / '>=' AdditiveExpr)* */
    for (i = 0; i < repeat; ++i) {
        this_op = *exp_idx;

        assert(exp->tokens[*exp_idx] == LYXP_TOKEN_OPERATOR_COMP);
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        if (!set) {
            ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_RELATIONAL, cur_node, local_mod, NULL, options);
            if (ret) {
                goto finish;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set);
        ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_RELATIONAL, cur_node, local_mod, &set2, options);
        if (ret) {
            goto finish;
        }

        /* eval */
        if (options & LYXP_SNODE_ALL) {
            warn_operands(local_mod->ctx, set, &set2, 1, exp->expr, exp->expr_pos[this_op - 1]);
            set_snode_merge(set, &set2);
            set_snode_clear_ctx(set);
        } else {
            if (moveto_op_comp(set, &set2, &exp->expr[exp->expr_pos[this_op]], cur_node, local_mod, options)) {
                ret = -1;
                goto finish;
            }
        }
    }

finish:
    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, local_mod, options);
    lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, local_mod, options);
    return ret;
}

/**
 * @brief Evaluate EqualityExpr. Logs directly on error.
 *
 * [13] EqualityExpr ::= RelationalExpr | EqualityExpr '=' RelationalExpr
 *                     | EqualityExpr '!=' RelationalExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_equality_expr(struct lyxp_expr *exp, uint16_t *exp_idx, uint16_t repeat, struct lyd_node *cur_node,
                   struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    int ret;
    uint16_t this_op;
    struct lyxp_set orig_set, set2;
    uint16_t i;

    assert(repeat);

    memset(&orig_set, 0, sizeof orig_set);
    memset(&set2, 0, sizeof set2);

    set_fill_set(&orig_set, set);

    ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_EQUALITY, cur_node, local_mod, set, options);
    if (ret) {
        goto finish;
    }

    /* ('=' / '!=' RelationalExpr)* */
    for (i = 0; i < repeat; ++i) {
        this_op = *exp_idx;

        assert(exp->tokens[*exp_idx] == LYXP_TOKEN_OPERATOR_COMP);
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        if (!set) {
            ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_EQUALITY, cur_node, local_mod, NULL, options);
            if (ret) {
                return ret;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set);
        ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_EQUALITY, cur_node, local_mod, &set2, options);
        if (ret) {
            goto finish;
        }

        /* eval */
        if (options & LYXP_SNODE_ALL) {
            warn_operands(local_mod->ctx, set, &set2, 0, exp->expr, exp->expr_pos[this_op - 1]);
            warn_equality_value(local_mod->ctx, exp, set, *exp_idx - 1, this_op - 1, *exp_idx - 1);
            warn_equality_value(local_mod->ctx, exp, &set2, this_op - 1, this_op - 1, *exp_idx - 1);
            set_snode_merge(set, &set2);
            set_snode_clear_ctx(set);
        } else {
            /* special handling of evaluations of identityref comparisons, always compare prefixed identites */
            if ((set->type == LYXP_SET_NODE_SET) && (set->val.nodes[0].node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))
                    && (((struct lys_node_leaf *)set->val.nodes[0].node->schema)->type.base == LY_TYPE_IDENT)) {
                /* left operand is identityref */
                if ((set2.type == LYXP_SET_STRING) && !strchr(set2.val.str, ':')) {
                    /* missing prefix in the right operand */
                    set2.val.str = ly_realloc(set2.val.str, strlen(local_mod->name) + 1 + strlen(set2.val.str) + 1);
                    if (!set2.val.str) {
                        goto finish;
                    }

                    memmove(set2.val.str + strlen(local_mod->name) + 1, set2.val.str, strlen(set2.val.str) + 1);
                    memcpy(set2.val.str, local_mod->name, strlen(local_mod->name));
                    set2.val.str[strlen(local_mod->name)] = ':';
                }
            }

            if (moveto_op_comp(set, &set2, &exp->expr[exp->expr_pos[this_op]], cur_node, local_mod, options)) {
                ret = -1;
                goto finish;
            }
        }
    }

finish:
    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, local_mod, options);
    lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, local_mod, options);
    return ret;
}

/**
 * @brief Evaluate AndExpr. Logs directly on error.
 *
 * [12] AndExpr ::= EqualityExpr | AndExpr 'and' EqualityExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_and_expr(struct lyxp_expr *exp, uint16_t *exp_idx, uint16_t repeat, struct lyd_node *cur_node,
              struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    int ret;
    struct lyxp_set orig_set, set2;
    uint16_t i;

    assert(repeat);

    memset(&orig_set, 0, sizeof orig_set);
    memset(&set2, 0, sizeof set2);

    set_fill_set(&orig_set, set);

    ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_AND, cur_node, local_mod, set, options);
    if (ret) {
        goto finish;
    }

    /* cast to boolean, we know that will be the final result */
    if (set && (options & LYXP_SNODE_ALL)) {
        set_snode_clear_ctx(set);
    } else {
        lyxp_set_cast(set, LYXP_SET_BOOLEAN, cur_node, local_mod, options);
    }

    /* ('and' EqualityExpr)* */
    for (i = 0; i < repeat; ++i) {
        assert(exp->tokens[*exp_idx] == LYXP_TOKEN_OPERATOR_LOG);
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (!set || !set->val.bool ? "skipped" : "parsed"),
            print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        /* lazy evaluation */
        if (!set || ((set->type == LYXP_SET_BOOLEAN) && !set->val.bool)) {
            ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_AND, cur_node, local_mod, NULL, options);
            if (ret) {
                goto finish;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set);
        ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_AND, cur_node, local_mod, &set2, options);
        if (ret) {
            goto finish;
        }

        /* eval - just get boolean value actually */
        if (set->type == LYXP_SET_SNODE_SET) {
            set_snode_clear_ctx(&set2);
            set_snode_merge(set, &set2);
        } else {
            lyxp_set_cast(&set2, LYXP_SET_BOOLEAN, cur_node, local_mod, options);
            set_fill_set(set, &set2);
        }
    }

finish:
    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, local_mod, options);
    lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, local_mod, options);
    return ret;
}

/**
 * @brief Evaluate OrExpr. Logs directly on error.
 *
 * [11] OrExpr ::= AndExpr | OrExpr 'or' AndExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_or_expr(struct lyxp_expr *exp, uint16_t *exp_idx, uint16_t repeat, struct lyd_node *cur_node,
             struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    int ret;
    struct lyxp_set orig_set, set2;
    uint16_t i;

    assert(repeat);

    memset(&orig_set, 0, sizeof orig_set);
    memset(&set2, 0, sizeof set2);

    set_fill_set(&orig_set, set);

    ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_OR, cur_node, local_mod, set, options);
    if (ret) {
        goto finish;
    }

    /* cast to boolean, we know that will be the final result */
    if (set && (options & LYXP_SNODE_ALL)) {
        set_snode_clear_ctx(set);
    } else {
        lyxp_set_cast(set, LYXP_SET_BOOLEAN, cur_node, local_mod, options);
    }

    /* ('or' AndExpr)* */
    for (i = 0; i < repeat; ++i) {
        assert(exp->tokens[*exp_idx] == LYXP_TOKEN_OPERATOR_LOG);
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (!set || set->val.bool ? "skipped" : "parsed"),
            print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        /* lazy evaluation */
        if (!set || ((set->type == LYXP_SET_BOOLEAN) && set->val.bool)) {
            ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_OR, cur_node, local_mod, NULL, options);
            if (ret) {
                goto finish;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set);
        /* expr_type cound have been LYXP_EXPR_NONE in all these later calls (except for the first one),
         * but it does not matter */
        ret = eval_expr_select(exp, exp_idx, LYXP_EXPR_OR, cur_node, local_mod, &set2, options);
        if (ret) {
            goto finish;
        }

        /* eval - just get boolean value actually */
        if (set->type == LYXP_SET_SNODE_SET) {
            set_snode_clear_ctx(&set2);
            set_snode_merge(set, &set2);
        } else {
            lyxp_set_cast(&set2, LYXP_SET_BOOLEAN, cur_node, local_mod, options);
            set_fill_set(set, &set2);
        }
    }

finish:
    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, local_mod, options);
    lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, local_mod, options);
    return ret;
}

/**
 * @brief Decide what expression is at the pointer \p exp_idx and evaluate it accordingly.
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] repeat_used How many values were already used from the current token repeat array.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in] local_mod Module considered to be local.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_expr_select(struct lyxp_expr *exp, uint16_t *exp_idx, enum lyxp_expr_type etype, struct lyd_node *cur_node,
                 struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    int ret;
    uint16_t i, count;
    enum lyxp_expr_type next_etype;

    /* process operator repeats */
    if (!exp->repeat[*exp_idx]) {
        next_etype = LYXP_EXPR_NONE;
    } else {
        /* find etype repeat */
        for (i = 0; exp->repeat[*exp_idx][i] > etype; ++i);

        /* select one-priority lower because etype expression called us */
        if (i) {
            next_etype = exp->repeat[*exp_idx][i - 1];
            /* count repeats for that expression */
            for (count = 0; i && exp->repeat[*exp_idx][i - 1] == next_etype; ++count, --i);
        } else {
            next_etype = LYXP_EXPR_NONE;
        }
    }

    /* decide what expression are we parsing based on the repeat */
    switch (next_etype) {
    case LYXP_EXPR_OR:
        ret = eval_or_expr(exp, exp_idx, count, cur_node, local_mod, set, options);
        break;
    case LYXP_EXPR_AND:
        ret = eval_and_expr(exp, exp_idx, count, cur_node, local_mod, set, options);
        break;
    case LYXP_EXPR_EQUALITY:
        ret = eval_equality_expr(exp, exp_idx, count, cur_node, local_mod, set, options);
        break;
    case LYXP_EXPR_RELATIONAL:
        ret = eval_relational_expr(exp, exp_idx, count, cur_node, local_mod, set, options);
        break;
    case LYXP_EXPR_ADDITIVE:
        ret = eval_additive_expr(exp, exp_idx, count, cur_node, local_mod, set, options);
        break;
    case LYXP_EXPR_MULTIPLICATIVE:
        ret = eval_multiplicative_expr(exp, exp_idx, count, cur_node, local_mod, set, options);
        break;
    case LYXP_EXPR_UNARY:
        ret = eval_unary_expr(exp, exp_idx, count, cur_node, local_mod, set, options);
        break;
    case LYXP_EXPR_UNION:
        ret = eval_union_expr(exp, exp_idx, count, cur_node, local_mod, set, options);
        break;
    case LYXP_EXPR_NONE:
        ret = eval_path_expr(exp, exp_idx, cur_node, local_mod, set, options);
        break;
    default:
        ret = -1;
        LOGINT(local_mod ? local_mod->ctx : NULL);
        break;
    }

    return ret;
}

int
lyxp_eval(const char *expr, const struct lyd_node *cur_node, enum lyxp_node_type cur_node_type,
          const struct lys_module *local_mod, struct lyxp_set *set, int options)
{
    struct ly_ctx *ctx = local_mod ? local_mod->ctx : NULL;
    struct lyxp_expr *exp;
    uint16_t exp_idx = 0;
    int rc = -1;

    if (!expr || !set) {
        LOGARG;
        return EXIT_FAILURE;
    }

    exp = lyxp_parse_expr(ctx, expr);
    if (!exp) {
        rc = -1;
        goto finish;
    }

    rc = reparse_or_expr(ctx, exp, &exp_idx);
    if (rc) {
        goto finish;
    } else if (exp->used > exp_idx) {
        LOGVAL(ctx, LYE_XPATH_INTOK, LY_VLOG_NONE, NULL, "Unknown", &exp->expr[exp->expr_pos[exp_idx]]);
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Unparsed characters \"%s\" left at the end of an XPath expression.",
               &exp->expr[exp->expr_pos[exp_idx]]);
        rc = -1;
        goto finish;
    }

    print_expr_struct_debug(exp);

    exp_idx = 0;
    memset(set, 0, sizeof *set);
    if (cur_node) {
        set_insert_node(set, (struct lyd_node *)cur_node, 0, cur_node_type, 0);
    }

    rc = eval_expr_select(exp, &exp_idx, 0, (struct lyd_node *)cur_node, (struct lys_module *)local_mod, set, options);
    if (rc == 2) {
        rc = EXIT_SUCCESS;
    }
    if ((rc == -1) && cur_node) {
        LOGPATH(ctx, LY_VLOG_LYD, cur_node);
    }

finish:
    lyxp_expr_free(exp);
    return rc;
}

#if 0

/* full xml printing of set elements, not used currently */

void
lyxp_set_print_xml(FILE *f, struct lyxp_set *set)
{
    uint32_t i;
    char *str_num;
    struct lyout out;

    memset(&out, 0, sizeof out);

    out.type = LYOUT_STREAM;
    out.method.f = f;

    switch (set->type) {
    case LYXP_SET_EMPTY:
        ly_print(&out, "Empty XPath set\n\n");
        break;
    case LYXP_SET_BOOLEAN:
        ly_print(&out, "Boolean XPath set:\n");
        ly_print(&out, "%s\n\n", set->value.bool ? "true" : "false");
        break;
    case LYXP_SET_STRING:
        ly_print(&out, "String XPath set:\n");
        ly_print(&out, "\"%s\"\n\n", set->value.str);
        break;
    case LYXP_SET_NUMBER:
        ly_print(&out, "Number XPath set:\n");

        if (isnan(set->value.num)) {
            str_num = strdup("NaN");
        } else if ((set->value.num == 0) || (set->value.num == -0.0f)) {
            str_num = strdup("0");
        } else if (isinf(set->value.num) && !signbit(set->value.num)) {
            str_num = strdup("Infinity");
        } else if (isinf(set->value.num) && signbit(set->value.num)) {
            str_num = strdup("-Infinity");
        } else if ((long long)set->value.num == set->value.num) {
            if (asprintf(&str_num, "%lld", (long long)set->value.num) == -1) {
                str_num = NULL;
            }
        } else {
            if (asprintf(&str_num, "%03.1Lf", set->value.num) == -1) {
                str_num = NULL;
            }
        }
        if (!str_num) {
            LOGMEM;
            return;
        }
        ly_print(&out, "%s\n\n", str_num);
        free(str_num);
        break;
    case LYXP_SET_NODE_SET:
        ly_print(&out, "Node XPath set:\n");

        for (i = 0; i < set->used; ++i) {
            ly_print(&out, "%d. ", i + 1);
            switch (set->node_type[i]) {
            case LYXP_NODE_ROOT_ALL:
                ly_print(&out, "ROOT all\n\n");
                break;
            case LYXP_NODE_ROOT_CONFIG:
                ly_print(&out, "ROOT config\n\n");
                break;
            case LYXP_NODE_ROOT_STATE:
                ly_print(&out, "ROOT state\n\n");
                break;
            case LYXP_NODE_ROOT_NOTIF:
                ly_print(&out, "ROOT notification \"%s\"\n\n", set->value.nodes[i]->schema->name);
                break;
            case LYXP_NODE_ROOT_RPC:
                ly_print(&out, "ROOT rpc \"%s\"\n\n", set->value.nodes[i]->schema->name);
                break;
            case LYXP_NODE_ROOT_OUTPUT:
                ly_print(&out, "ROOT output \"%s\"\n\n", set->value.nodes[i]->schema->name);
                break;
            case LYXP_NODE_ELEM:
                ly_print(&out, "ELEM \"%s\"\n", set->value.nodes[i]->schema->name);
                xml_print_node(&out, 1, set->value.nodes[i], 1, LYP_FORMAT);
                ly_print(&out, "\n");
                break;
            case LYXP_NODE_TEXT:
                ly_print(&out, "TEXT \"%s\"\n\n", ((struct lyd_node_leaf_list *)set->value.nodes[i])->value_str);
                break;
            case LYXP_NODE_ATTR:
                ly_print(&out, "ATTR \"%s\" = \"%s\"\n\n", set->value.attrs[i]->name, set->value.attrs[i]->value);
                break;
            }
        }
        break;
    }
}

#endif

int
lyxp_set_cast(struct lyxp_set *set, enum lyxp_set_type target, const struct lyd_node *cur_node,
              const struct lys_module *local_mod, int options)
{
    long double num;
    char *str;

    if (!set || (set->type == target)) {
        return EXIT_SUCCESS;
    }

    /* it's not possible to convert anything into a node set */
    assert((target != LYXP_SET_NODE_SET) && ((set->type != LYXP_SET_SNODE_SET) || (target == LYXP_SET_EMPTY)));

    if (set->type == LYXP_SET_SNODE_SET) {
        set_free_content(set);
        return -1;
    }

    /* to STRING */
    if ((target == LYXP_SET_STRING) || ((target == LYXP_SET_NUMBER)
            && ((set->type == LYXP_SET_NODE_SET) || (set->type == LYXP_SET_EMPTY)))) {
        switch (set->type) {
        case LYXP_SET_NUMBER:
            if (isnan(set->val.num)) {
                set->val.str = strdup("NaN");
                LY_CHECK_ERR_RETURN(!set->val.str, LOGMEM(local_mod->ctx), -1);
            } else if ((set->val.num == 0) || (set->val.num == -0.0f)) {
                set->val.str = strdup("0");
                LY_CHECK_ERR_RETURN(!set->val.str, LOGMEM(local_mod->ctx), -1);
            } else if (isinf(set->val.num) && !signbit(set->val.num)) {
                set->val.str = strdup("Infinity");
                LY_CHECK_ERR_RETURN(!set->val.str, LOGMEM(local_mod->ctx), -1);
            } else if (isinf(set->val.num) && signbit(set->val.num)) {
                set->val.str = strdup("-Infinity");
                LY_CHECK_ERR_RETURN(!set->val.str, LOGMEM(local_mod->ctx), -1);
            } else if ((long long)set->val.num == set->val.num) {
                if (asprintf(&str, "%lld", (long long)set->val.num) == -1) {
                    LOGMEM(local_mod->ctx);
                    return -1;
                }
                set->val.str = str;
            } else {
                if (asprintf(&str, "%03.1Lf", set->val.num) == -1) {
                    LOGMEM(local_mod->ctx);
                    return -1;
                }
                set->val.str = str;
            }
            break;
        case LYXP_SET_BOOLEAN:
            if (set->val.bool) {
                set->val.str = strdup("true");
            } else {
                set->val.str = strdup("false");
            }
            LY_CHECK_ERR_RETURN(!set->val.str, LOGMEM(local_mod->ctx), -1);
            break;
        case LYXP_SET_NODE_SET:
            assert(set->used);

            /* we need the set sorted, it affects the result */
            assert(!set_sort(set, cur_node, options));

            str = cast_node_set_to_string(set, (struct lyd_node *)cur_node, (struct lys_module *)local_mod, options);
            if (!str) {
                return -1;
            }
            set_free_content(set);
            set->val.str = str;
            break;
        case LYXP_SET_EMPTY:
            set->val.str = strdup("");
            LY_CHECK_ERR_RETURN(!set->val.str, LOGMEM(local_mod->ctx), -1);
            break;
        default:
            LOGINT(local_mod->ctx);
            return -1;
        }
        set->type = LYXP_SET_STRING;
    }

    /* to NUMBER */
    if (target == LYXP_SET_NUMBER) {
        switch (set->type) {
        case LYXP_SET_STRING:
            num = cast_string_to_number(set->val.str);
            set_free_content(set);
            set->val.num = num;
            break;
        case LYXP_SET_BOOLEAN:
            if (set->val.bool) {
                set->val.num = 1;
            } else {
                set->val.num = 0;
            }
            break;
        default:
            LOGINT(local_mod->ctx);
            return -1;
        }
        set->type = LYXP_SET_NUMBER;
    }

    /* to BOOLEAN */
    if (target == LYXP_SET_BOOLEAN) {
        switch (set->type) {
        case LYXP_SET_NUMBER:
            if ((set->val.num == 0) || (set->val.num == -0.0f) || isnan(set->val.num)) {
                set->val.bool = 0;
            } else {
                set->val.bool = 1;
            }
            break;
        case LYXP_SET_STRING:
            if (set->val.str[0]) {
                set_free_content(set);
                set->val.bool = 1;
            } else {
                set_free_content(set);
                set->val.bool = 0;
            }
            break;
        case LYXP_SET_NODE_SET:
            set_free_content(set);

            assert(set->used);
            set->val.bool = 1;
            break;
        case LYXP_SET_EMPTY:
            set->val.bool = 0;
            break;
        default:
            LOGINT(local_mod->ctx);
            return -1;
        }
        set->type = LYXP_SET_BOOLEAN;
    }

    /* to EMPTY */
    if (target == LYXP_SET_EMPTY) {
        set_free_content(set);
        set->type = LYXP_SET_EMPTY;
    }

    return EXIT_SUCCESS;
}

int
lyxp_atomize(const char *expr, const struct lys_node *cur_snode, enum lyxp_node_type cur_snode_type,
             struct lyxp_set *set, int options, const struct lys_node **ctx_snode)
{
    struct lys_node *_ctx_snode;
    enum lyxp_node_type ctx_snode_type;
    struct lyxp_expr *exp;
    uint16_t exp_idx = 0;
    int rc = -1;

    exp = lyxp_parse_expr(cur_snode->module->ctx, expr);
    if (!exp) {
        rc = -1;
        goto finish;
    }

    rc = reparse_or_expr(cur_snode->module->ctx, exp, &exp_idx);
    if (rc) {
        goto finish;
    } else if (exp->used > exp_idx) {
        LOGVAL(cur_snode->module->ctx, LYE_XPATH_INTOK, LY_VLOG_NONE, NULL, "Unknown", &exp->expr[exp->expr_pos[exp_idx]]);
        LOGVAL(cur_snode->module->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Unparsed characters \"%s\" left at the end of an XPath expression.",
               &exp->expr[exp->expr_pos[exp_idx]]);
        rc = -1;
        goto finish;
    }

    print_expr_struct_debug(exp);

    if (options & LYXP_SNODE_WHEN) {
        /* for when the context node may need to be changed */
        resolve_when_ctx_snode(cur_snode, &_ctx_snode, &ctx_snode_type);
    } else {
        _ctx_snode = (struct lys_node *)cur_snode;
        ctx_snode_type = cur_snode_type;
    }

    if (ctx_snode) {
        *ctx_snode = _ctx_snode;
    }

    exp_idx = 0;
    memset(set, 0, sizeof *set);
    set->type = LYXP_SET_SNODE_SET;
    set_snode_insert_node(set, _ctx_snode, ctx_snode_type);

    rc = eval_expr_select(exp, &exp_idx, 0, (struct lyd_node *)_ctx_snode, lys_node_module(_ctx_snode), set, options);
    if (rc == 2) {
        rc = EXIT_SUCCESS;
    }

finish:
    lyxp_expr_free(exp);
    return rc;
}

int
lyxp_node_atomize(const struct lys_node *node, struct lyxp_set *set, int set_ext_dep_flags)
{
    struct lys_node *parent, *elem;
    const struct lys_node *ctx_snode = NULL;
    struct lyxp_set tmp_set;
    uint8_t must_size = 0;
    uint32_t i, j;
    int opts, ret = EXIT_SUCCESS;
    struct lys_when *when = NULL;
    struct lys_restr *must = NULL;
    char *path = NULL;

    memset(&tmp_set, 0, sizeof tmp_set);
    memset(set, 0, sizeof *set);

    /* check if we will be traversing RPC output */
    opts = 0;
    for (parent = (struct lys_node *)node; parent && (parent->nodetype != LYS_OUTPUT); parent = lys_parent(parent));
    if (parent) {
        opts |= LYXP_SNODE_OUTPUT;
    }

    switch (node->nodetype) {
    case LYS_CONTAINER:
        when = ((struct lys_node_container *)node)->when;
        must = ((struct lys_node_container *)node)->must;
        must_size = ((struct lys_node_container *)node)->must_size;
        break;
    case LYS_CHOICE:
        when = ((struct lys_node_choice *)node)->when;
        break;
    case LYS_LEAF:
        when = ((struct lys_node_leaf *)node)->when;
        must = ((struct lys_node_leaf *)node)->must;
        must_size = ((struct lys_node_leaf *)node)->must_size;
        break;
    case LYS_LEAFLIST:
        when = ((struct lys_node_leaflist *)node)->when;
        must = ((struct lys_node_leaflist *)node)->must;
        must_size = ((struct lys_node_leaflist *)node)->must_size;
        break;
    case LYS_LIST:
        when = ((struct lys_node_list *)node)->when;
        must = ((struct lys_node_list *)node)->must;
        must_size = ((struct lys_node_list *)node)->must_size;
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        when = ((struct lys_node_anydata *)node)->when;
        must = ((struct lys_node_anydata *)node)->must;
        must_size = ((struct lys_node_anydata *)node)->must_size;
        break;
    case LYS_CASE:
        when = ((struct lys_node_case *)node)->when;
        break;
    case LYS_NOTIF:
        must = ((struct lys_node_notif *)node)->must;
        must_size = ((struct lys_node_notif *)node)->must_size;
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        must = ((struct lys_node_inout *)node)->must;
        must_size = ((struct lys_node_inout *)node)->must_size;
        break;
    case LYS_USES:
        when = ((struct lys_node_uses *)node)->when;
        break;
    case LYS_AUGMENT:
        when = ((struct lys_node_augment *)node)->when;
        break;
    default:
        /* nothing to check */
        break;
    }

    if (set_ext_dep_flags) {
        /* find operation if in one, used later */
        for (parent = (struct lys_node *)node;
             parent && !(parent->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF));
             parent = lys_parent(parent));
    }

    /* check "when" */
    if (when) {
        if (lyxp_atomize(when->cond, node, LYXP_NODE_ELEM, &tmp_set, LYXP_SNODE_WHEN | opts, &ctx_snode)) {
            free(tmp_set.val.snodes);
            if (ctx_snode) {
                path = lys_path(ctx_snode, LYS_PATH_FIRST_PREFIX);
                LOGVAL(node->module->ctx, LYE_SPEC, LY_VLOG_LYS, node,
                       "Invalid when condition \"%s\" with context node \"%s\".", when->cond, path);
            } else {
                LOGVAL(node->module->ctx, LYE_SPEC, LY_VLOG_LYS, node, "Invalid when condition \"%s\".", when->cond);
            }
            ret = -1;
            goto finish;
        } else {
            if (set_ext_dep_flags) {
                for (j = 0; j < tmp_set.used; ++j) {
                    /* skip roots'n'stuff */
                    if (tmp_set.val.snodes[j].type == LYXP_NODE_ELEM) {
                        /* XPath expression cannot reference "lower" status than the node that has the definition */
                        if (lyp_check_status(node->flags, lys_node_module(node), node->name, tmp_set.val.snodes[j].snode->flags,
                                lys_node_module(tmp_set.val.snodes[j].snode), tmp_set.val.snodes[j].snode->name, node)) {
                            ret = -1;
                            goto finish;
                        }

                        if (parent) {
                            for (elem = tmp_set.val.snodes[j].snode; elem && (elem != parent); elem = lys_parent(elem));
                            if (!elem) {
                                /* not in node's RPC or notification subtree, set the correct dep flag */
                                if (tmp_set.val.snodes[j].snode->flags & LYS_CONFIG_W) {
                                    when->flags |= LYS_XPCONF_DEP;
                                    ((struct lys_node *)node)->flags |= LYS_XPCONF_DEP;
                                } else {
                                    assert(tmp_set.val.snodes[j].snode->flags & LYS_CONFIG_R);
                                    when->flags |= LYS_XPSTATE_DEP;
                                    ((struct lys_node *)node)->flags |= LYS_XPSTATE_DEP;
                                }
                            }
                        }
                    }
                }
            }
            set_snode_merge(set, &tmp_set);
            memset(&tmp_set, 0, sizeof tmp_set);
        }
    }

    /* check "must" */
    for (i = 0; i < must_size; ++i) {
        if (lyxp_atomize(must[i].expr, node, LYXP_NODE_ELEM, &tmp_set, LYXP_SNODE_MUST | opts, &ctx_snode)) {
            free(tmp_set.val.snodes);
            if (ctx_snode) {
                path = lys_path(ctx_snode, LYS_PATH_FIRST_PREFIX);
                LOGVAL(node->module->ctx, LYE_SPEC, LY_VLOG_LYS, node,
                       "Invalid must restriction \"%s\" with context node \"%s\".", must[i].expr, path);
            } else {
                LOGVAL(node->module->ctx, LYE_SPEC, LY_VLOG_LYS, node, "Invalid must restriction \"%s\".", must[i].expr);
            }
            ret = -1;
            goto finish;
        } else {
            if (set_ext_dep_flags) {
                for (j = 0; j < tmp_set.used; ++j) {
                    /* skip roots'n'stuff */
                    if (tmp_set.val.snodes[j].type == LYXP_NODE_ELEM) {
                        /* XPath expression cannot reference "lower" status than the node that has the definition */
                        if (lyp_check_status(node->flags, lys_node_module(node), node->name, tmp_set.val.snodes[j].snode->flags,
                                lys_node_module(tmp_set.val.snodes[j].snode), tmp_set.val.snodes[j].snode->name, node)) {
                            ret = -1;
                            goto finish;
                        }

                        if (parent) {
                            for (elem = tmp_set.val.snodes[j].snode; elem && (elem != parent); elem = lys_parent(elem));
                            if (!elem) {
                                /* not in node's RPC or notification subtree, set the correct dep flag */
                                if (tmp_set.val.snodes[j].snode->flags & LYS_CONFIG_W) {
                                    must[i].flags |= LYS_XPCONF_DEP;
                                    ((struct lys_node *)node)->flags |= LYS_XPCONF_DEP;
                                } else {
                                    assert(tmp_set.val.snodes[j].snode->flags & LYS_CONFIG_R);
                                    must[i].flags |= LYS_XPSTATE_DEP;
                                    ((struct lys_node *)node)->flags |= LYS_XPSTATE_DEP;
                                }
                            }
                        }
                    }
                }
            }
            set_snode_merge(set, &tmp_set);
            memset(&tmp_set, 0, sizeof tmp_set);
        }
    }

finish:
    if (ret) {
        free(set->val.snodes);
        memset(set, 0, sizeof *set);
    }
    free(path);
    return ret;
}

int
lyxp_node_check_syntax(const struct lys_node *node)
{
    uint8_t must_size = 0;
    uint16_t exp_idx;
    uint32_t i;
    struct lys_when *when = NULL;
    struct lys_restr *must = NULL;
    struct lyxp_expr *expr;

    switch (node->nodetype) {
    case LYS_CONTAINER:
        when = ((struct lys_node_container *)node)->when;
        must = ((struct lys_node_container *)node)->must;
        must_size = ((struct lys_node_container *)node)->must_size;
        break;
    case LYS_CHOICE:
        when = ((struct lys_node_choice *)node)->when;
        break;
    case LYS_LEAF:
        when = ((struct lys_node_leaf *)node)->when;
        must = ((struct lys_node_leaf *)node)->must;
        must_size = ((struct lys_node_leaf *)node)->must_size;
        break;
    case LYS_LEAFLIST:
        when = ((struct lys_node_leaflist *)node)->when;
        must = ((struct lys_node_leaflist *)node)->must;
        must_size = ((struct lys_node_leaflist *)node)->must_size;
        break;
    case LYS_LIST:
        when = ((struct lys_node_list *)node)->when;
        must = ((struct lys_node_list *)node)->must;
        must_size = ((struct lys_node_list *)node)->must_size;
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        when = ((struct lys_node_anydata *)node)->when;
        must = ((struct lys_node_anydata *)node)->must;
        must_size = ((struct lys_node_anydata *)node)->must_size;
        break;
    case LYS_CASE:
        when = ((struct lys_node_case *)node)->when;
        break;
    case LYS_NOTIF:
        must = ((struct lys_node_notif *)node)->must;
        must_size = ((struct lys_node_notif *)node)->must_size;
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        must = ((struct lys_node_inout *)node)->must;
        must_size = ((struct lys_node_inout *)node)->must_size;
        break;
    case LYS_USES:
        when = ((struct lys_node_uses *)node)->when;
        break;
    case LYS_AUGMENT:
        when = ((struct lys_node_augment *)node)->when;
        break;
    default:
        /* nothing to check */
        break;
    }

    /* check "when" */
    if (when) {
        expr = lyxp_parse_expr(node->module->ctx, when->cond);
        if (!expr) {
            return -1;
        }

        exp_idx = 0;
        if (reparse_or_expr(node->module->ctx, expr, &exp_idx)) {
            lyxp_expr_free(expr);
            return -1;
        } else if (exp_idx != expr->used) {
            LOGVAL(node->module->ctx, LYE_XPATH_INTOK, LY_VLOG_NONE, NULL,
                   print_token(expr->tokens[exp_idx]), &expr->expr[expr->expr_pos[exp_idx]]);
            lyxp_expr_free(expr);
            return -1;
        }
        lyxp_expr_free(expr);
    }

    /* check "must" */
    for (i = 0; i < must_size; ++i) {
        expr = lyxp_parse_expr(node->module->ctx, must[i].expr);
        if (!expr) {
            return -1;
        }

        exp_idx = 0;
        if (reparse_or_expr(node->module->ctx, expr, &exp_idx)) {
            lyxp_expr_free(expr);
            return -1;
        } else if (exp_idx != expr->used) {
            LOGVAL(node->module->ctx, LYE_XPATH_INTOK, LY_VLOG_NONE, NULL,
                   print_token(expr->tokens[exp_idx]), &expr->expr[expr->expr_pos[exp_idx]]);
            lyxp_expr_free(expr);
            return -1;
        }
        lyxp_expr_free(expr);
    }

    return 0;
}
