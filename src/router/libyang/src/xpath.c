/**
 * @file xpath.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief YANG XPath evaluation functions
 *
 * Copyright (c) 2015 - 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE /* asprintf, strdup */
#include <sys/cdefs.h>

#include "xpath.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "hash_table.h"
#include "out.h"
#include "parser_data.h"
#include "path.h"
#include "plugins_types.h"
#include "printer_data.h"
#include "schema_compile_node.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_edit.h"
#include "tree_schema_internal.h"
#include "xml.h"

static LY_ERR reparse_or_expr(const struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *tok_idx);
static LY_ERR eval_expr_select(const struct lyxp_expr *exp, uint16_t *tok_idx, enum lyxp_expr_type etype,
        struct lyxp_set *set, uint32_t options);
static LY_ERR moveto_resolve_model(const char **qname, uint16_t *qname_len, const struct lyxp_set *set,
        const struct lysc_node *ctx_scnode, const struct lys_module **moveto_mod);

/**
 * @brief Print the type of an XPath \p set.
 *
 * @param[in] set Set to use.
 * @return Set type string.
 */
static const char *
print_set_type(struct lyxp_set *set)
{
    switch (set->type) {
    case LYXP_SET_NODE_SET:
        return "node set";
    case LYXP_SET_SCNODE_SET:
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

const char *
lyxp_print_token(enum lyxp_token tok)
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
    case LYXP_TOKEN_OPER_LOG:
        return "Operator(Logic)";
    case LYXP_TOKEN_OPER_EQUAL:
        return "Operator(Equal)";
    case LYXP_TOKEN_OPER_NEQUAL:
        return "Operator(Non-equal)";
    case LYXP_TOKEN_OPER_COMP:
        return "Operator(Comparison)";
    case LYXP_TOKEN_OPER_MATH:
        return "Operator(Math)";
    case LYXP_TOKEN_OPER_UNI:
        return "Operator(Union)";
    case LYXP_TOKEN_OPER_PATH:
        return "Operator(Path)";
    case LYXP_TOKEN_OPER_RPATH:
        return "Operator(Recursive Path)";
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
print_expr_struct_debug(const struct lyxp_expr *exp)
{
#define MSG_BUFFER_SIZE 128
    char tmp[MSG_BUFFER_SIZE];
    uint32_t i, j;

    if (!exp || (ly_ll < LY_LLDBG)) {
        return;
    }

    LOGDBG(LY_LDGXPATH, "expression \"%s\":", exp->expr);
    for (i = 0; i < exp->used; ++i) {
        sprintf(tmp, "\ttoken %s, in expression \"%.*s\"", lyxp_print_token(exp->tokens[i]), exp->tok_len[i],
                &exp->expr[exp->tok_pos[i]]);
        if (exp->repeat && exp->repeat[i]) {
            sprintf(tmp + strlen(tmp), " (repeat %d", exp->repeat[i][0]);
            for (j = 1; exp->repeat[i][j]; ++j) {
                sprintf(tmp + strlen(tmp), ", %d", exp->repeat[i][j]);
            }
            strcat(tmp, ")");
        }
        LOGDBG(LY_LDGXPATH, tmp);
    }
#undef MSG_BUFFER_SIZE
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
    char *str;
    struct lyxp_set_node *item;
    struct lyxp_set_scnode *sitem;

    if (ly_ll < LY_LLDBG) {
        return;
    }

    switch (set->type) {
    case LYXP_SET_NODE_SET:
        LOGDBG(LY_LDGXPATH, "set NODE SET:");
        for (i = 0; i < set->used; ++i) {
            item = &set->val.nodes[i];

            switch (item->type) {
            case LYXP_NODE_NONE:
                LOGDBG(LY_LDGXPATH, "\t%d (pos %u): NONE", i + 1, item->pos);
                break;
            case LYXP_NODE_ROOT:
                LOGDBG(LY_LDGXPATH, "\t%d (pos %u): ROOT", i + 1, item->pos);
                break;
            case LYXP_NODE_ROOT_CONFIG:
                LOGDBG(LY_LDGXPATH, "\t%d (pos %u): ROOT CONFIG", i + 1, item->pos);
                break;
            case LYXP_NODE_ELEM:
                if ((item->node->schema->nodetype == LYS_LIST) && (lyd_child(item->node)->schema->nodetype == LYS_LEAF)) {
                    LOGDBG(LY_LDGXPATH, "\t%d (pos %u): ELEM %s (1st child val: %s)", i + 1, item->pos,
                            item->node->schema->name, lyd_get_value(lyd_child(item->node)));
                } else if (item->node->schema->nodetype == LYS_LEAFLIST) {
                    LOGDBG(LY_LDGXPATH, "\t%d (pos %u): ELEM %s (val: %s)", i + 1, item->pos,
                            item->node->schema->name, lyd_get_value(item->node));
                } else {
                    LOGDBG(LY_LDGXPATH, "\t%d (pos %u): ELEM %s", i + 1, item->pos, item->node->schema->name);
                }
                break;
            case LYXP_NODE_TEXT:
                if (item->node->schema->nodetype & LYS_ANYDATA) {
                    LOGDBG(LY_LDGXPATH, "\t%d (pos %u): TEXT <%s>", i + 1, item->pos,
                            item->node->schema->nodetype == LYS_ANYXML ? "anyxml" : "anydata");
                } else {
                    LOGDBG(LY_LDGXPATH, "\t%d (pos %u): TEXT %s", i + 1, item->pos, lyd_get_value(item->node));
                }
                break;
            case LYXP_NODE_META:
                LOGDBG(LY_LDGXPATH, "\t%d (pos %u): META %s = %s", i + 1, item->pos, set->val.meta[i].meta->name,
                        set->val.meta[i].meta->value);
                break;
            }
        }
        break;

    case LYXP_SET_SCNODE_SET:
        LOGDBG(LY_LDGXPATH, "set SCNODE SET:");
        for (i = 0; i < set->used; ++i) {
            sitem = &set->val.scnodes[i];

            switch (sitem->type) {
            case LYXP_NODE_ROOT:
                LOGDBG(LY_LDGXPATH, "\t%d (%u): ROOT", i + 1, sitem->in_ctx);
                break;
            case LYXP_NODE_ROOT_CONFIG:
                LOGDBG(LY_LDGXPATH, "\t%d (%u): ROOT CONFIG", i + 1, sitem->in_ctx);
                break;
            case LYXP_NODE_ELEM:
                LOGDBG(LY_LDGXPATH, "\t%d (%u): ELEM %s", i + 1, sitem->in_ctx, sitem->scnode->name);
                break;
            default:
                LOGINT(NULL);
                break;
            }
        }
        break;

    case LYXP_SET_BOOLEAN:
        LOGDBG(LY_LDGXPATH, "set BOOLEAN");
        LOGDBG(LY_LDGXPATH, "\t%s", (set->val.bln ? "true" : "false"));
        break;

    case LYXP_SET_STRING:
        LOGDBG(LY_LDGXPATH, "set STRING");
        LOGDBG(LY_LDGXPATH, "\t%s", set->val.str);
        break;

    case LYXP_SET_NUMBER:
        LOGDBG(LY_LDGXPATH, "set NUMBER");

        if (isnan(set->val.num)) {
            str = strdup("NaN");
        } else if ((set->val.num == 0) || (set->val.num == -0.0f)) {
            str = strdup("0");
        } else if (isinf(set->val.num) && !signbit(set->val.num)) {
            str = strdup("Infinity");
        } else if (isinf(set->val.num) && signbit(set->val.num)) {
            str = strdup("-Infinity");
        } else if ((long long)set->val.num == set->val.num) {
            if (asprintf(&str, "%lld", (long long)set->val.num) == -1) {
                str = NULL;
            }
        } else {
            if (asprintf(&str, "%03.1Lf", set->val.num) == -1) {
                str = NULL;
            }
        }
        LY_CHECK_ERR_RET(!str, LOGMEM(NULL), );

        LOGDBG(LY_LDGXPATH, "\t%s", str);
        free(str);
    }
}

#endif

/**
 * @brief Realloc the string \p str.
 *
 * @param[in] ctx libyang context for logging.
 * @param[in] needed How much free space is required.
 * @param[in,out] str Pointer to the string to use.
 * @param[in,out] used Used bytes in \p str.
 * @param[in,out] size Allocated bytes in \p str.
 * @return LY_ERR
 */
static LY_ERR
cast_string_realloc(const struct ly_ctx *ctx, uint16_t needed, char **str, uint16_t *used, uint16_t *size)
{
    if (*size - *used < needed) {
        do {
            if ((UINT16_MAX - *size) < LYXP_STRING_CAST_SIZE_STEP) {
                LOGERR(ctx, LY_EINVAL, "XPath string length limit (%u) reached.", UINT16_MAX);
                return LY_EINVAL;
            }
            *size += LYXP_STRING_CAST_SIZE_STEP;
        } while (*size - *used < needed);
        *str = ly_realloc(*str, *size * sizeof(char));
        LY_CHECK_ERR_RET(!(*str), LOGMEM(ctx), LY_EMEM);
    }

    return LY_SUCCESS;
}

/**
 * @brief Cast nodes recursively to one string @p str.
 *
 * @param[in] node Node to cast.
 * @param[in] fake_cont Whether to put the data into a "fake" container.
 * @param[in] root_type Type of the XPath root.
 * @param[in] indent Current indent.
 * @param[in,out] str Resulting string.
 * @param[in,out] used Used bytes in @p str.
 * @param[in,out] size Allocated bytes in @p str.
 * @return LY_ERR
 */
static LY_ERR
cast_string_recursive(const struct lyd_node *node, ly_bool fake_cont, enum lyxp_node_type root_type, uint16_t indent, char **str,
        uint16_t *used, uint16_t *size)
{
    char *buf, *line, *ptr = NULL;
    const char *value_str;
    const struct lyd_node *child;
    struct lyd_node *tree;
    struct lyd_node_any *any;
    LY_ERR rc;

    if ((root_type == LYXP_NODE_ROOT_CONFIG) && (node->schema->flags & LYS_CONFIG_R)) {
        return LY_SUCCESS;
    }

    if (fake_cont) {
        rc = cast_string_realloc(LYD_CTX(node), 1, str, used, size);
        LY_CHECK_RET(rc);
        strcpy(*str + (*used - 1), "\n");
        ++(*used);

        ++indent;
    }

    switch (node->schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_RPC:
    case LYS_NOTIF:
        rc = cast_string_realloc(LYD_CTX(node), 1, str, used, size);
        LY_CHECK_RET(rc);
        strcpy(*str + (*used - 1), "\n");
        ++(*used);

        for (child = lyd_child(node); child; child = child->next) {
            rc = cast_string_recursive(child, 0, root_type, indent + 1, str, used, size);
            LY_CHECK_RET(rc);
        }

        break;

    case LYS_LEAF:
    case LYS_LEAFLIST:
        value_str = lyd_get_value(node);

        /* print indent */
        LY_CHECK_RET(cast_string_realloc(LYD_CTX(node), indent * 2 + strlen(value_str) + 1, str, used, size));
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
        any = (struct lyd_node_any *)node;
        if (!(void *)any->value.tree) {
            /* no content */
            buf = strdup("");
            LY_CHECK_ERR_RET(!buf, LOGMEM(LYD_CTX(node)), LY_EMEM);
        } else {
            struct ly_out *out;

            if (any->value_type == LYD_ANYDATA_LYB) {
                /* try to parse it into a data tree */
                if (lyd_parse_data_mem((struct ly_ctx *)LYD_CTX(node), any->value.mem, LYD_LYB, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, &tree) == LY_SUCCESS) {
                    /* successfully parsed */
                    free(any->value.mem);
                    any->value.tree = tree;
                    any->value_type = LYD_ANYDATA_DATATREE;
                }
                /* error is covered by the following switch where LYD_ANYDATA_LYB causes failure */
            }

            switch (any->value_type) {
            case LYD_ANYDATA_STRING:
            case LYD_ANYDATA_XML:
            case LYD_ANYDATA_JSON:
                buf = strdup(any->value.json);
                LY_CHECK_ERR_RET(!buf, LOGMEM(LYD_CTX(node)), LY_EMEM);
                break;
            case LYD_ANYDATA_DATATREE:
                LY_CHECK_RET(ly_out_new_memory(&buf, 0, &out));
                rc = lyd_print_all(out, any->value.tree, LYD_XML, 0);
                ly_out_free(out, NULL, 0);
                LY_CHECK_RET(rc < 0, -rc);
                break;
            case LYD_ANYDATA_LYB:
                LOGERR(LYD_CTX(node), LY_EINVAL, "Cannot convert LYB anydata into string.");
                return LY_EINVAL;
            }
        }

        line = strtok_r(buf, "\n", &ptr);
        do {
            rc = cast_string_realloc(LYD_CTX(node), indent * 2 + strlen(line) + 1, str, used, size);
            if (rc != LY_SUCCESS) {
                free(buf);
                return rc;
            }
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
        LOGINT_RET(LYD_CTX(node));
    }

    if (fake_cont) {
        rc = cast_string_realloc(LYD_CTX(node), 1, str, used, size);
        LY_CHECK_RET(rc);
        strcpy(*str + (*used - 1), "\n");
        ++(*used);

        --indent;
    }

    return LY_SUCCESS;
}

/**
 * @brief Cast an element into a string.
 *
 * @param[in] node Node to cast.
 * @param[in] fake_cont Whether to put the data into a "fake" container.
 * @param[in] root_type Type of the XPath root.
 * @param[out] str Element cast to dynamically-allocated string.
 * @return LY_ERR
 */
static LY_ERR
cast_string_elem(struct lyd_node *node, ly_bool fake_cont, enum lyxp_node_type root_type, char **str)
{
    uint16_t used, size;
    LY_ERR rc;

    *str = malloc(LYXP_STRING_CAST_SIZE_START * sizeof(char));
    LY_CHECK_ERR_RET(!*str, LOGMEM(LYD_CTX(node)), LY_EMEM);
    (*str)[0] = '\0';
    used = 1;
    size = LYXP_STRING_CAST_SIZE_START;

    rc = cast_string_recursive(node, fake_cont, root_type, 0, str, &used, &size);
    if (rc != LY_SUCCESS) {
        free(*str);
        return rc;
    }

    if (size > used) {
        *str = ly_realloc(*str, used * sizeof(char));
        LY_CHECK_ERR_RET(!*str, LOGMEM(LYD_CTX(node)), LY_EMEM);
    }
    return LY_SUCCESS;
}

/**
 * @brief Cast a LYXP_SET_NODE_SET set into a string.
 *        Context position aware.
 *
 * @param[in] set Set to cast.
 * @param[out] str Cast dynamically-allocated string.
 * @return LY_ERR
 */
static LY_ERR
cast_node_set_to_string(struct lyxp_set *set, char **str)
{
    if (!set->used) {
        *str = strdup("");
        if (!*str) {
            LOGMEM_RET(set->ctx);
        }
        return LY_SUCCESS;
    }

    switch (set->val.nodes[0].type) {
    case LYXP_NODE_NONE:
        /* invalid */
        LOGINT_RET(set->ctx);
    case LYXP_NODE_ROOT:
    case LYXP_NODE_ROOT_CONFIG:
        return cast_string_elem(set->val.nodes[0].node, 1, set->root_type, str);
    case LYXP_NODE_ELEM:
    case LYXP_NODE_TEXT:
        return cast_string_elem(set->val.nodes[0].node, 0, set->root_type, str);
    case LYXP_NODE_META:
        *str = strdup(lyd_get_meta_value(set->val.meta[0].meta));
        if (!*str) {
            LOGMEM_RET(set->ctx);
        }
        return LY_SUCCESS;
    }

    LOGINT_RET(set->ctx);
}

/**
 * @brief Cast a string into an XPath number.
 *
 * @param[in] str String to use.
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

/**
 * @brief Callback for checking value equality.
 *
 * Implementation of ::lyht_value_equal_cb.
 *
 * @param[in] val1_p First value.
 * @param[in] val2_p Second value.
 * @param[in] mod Whether hash table is being modified.
 * @param[in] cb_data Callback data.
 * @return Boolean value whether values are equal or not.
 */
static ly_bool
set_values_equal_cb(void *val1_p, void *val2_p, ly_bool UNUSED(mod), void *UNUSED(cb_data))
{
    struct lyxp_set_hash_node *val1, *val2;

    val1 = (struct lyxp_set_hash_node *)val1_p;
    val2 = (struct lyxp_set_hash_node *)val2_p;

    if ((val1->node == val2->node) && (val1->type == val2->type)) {
        return 1;
    }

    return 0;
}

/**
 * @brief Insert node and its hash into set.
 *
 * @param[in] set et to insert to.
 * @param[in] node Node with hash.
 * @param[in] type Node type.
 */
static void
set_insert_node_hash(struct lyxp_set *set, struct lyd_node *node, enum lyxp_node_type type)
{
    LY_ERR r;
    uint32_t i, hash;
    struct lyxp_set_hash_node hnode;

    if (!set->ht && (set->used >= LYD_HT_MIN_ITEMS)) {
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

            if (hnode.node == node) {
                /* it was just added, do not add it twice */
                node = NULL;
            }
        }
    }

    if (set->ht && node) {
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

/**
 * @brief Remove node and its hash from set.
 *
 * @param[in] set Set to remove from.
 * @param[in] node Node to remove.
 * @param[in] type Node type.
 */
static void
set_remove_node_hash(struct lyxp_set *set, struct lyd_node *node, enum lyxp_node_type type)
{
    LY_ERR r;
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

/**
 * @brief Check whether node is in set based on its hash.
 *
 * @param[in] set Set to search in.
 * @param[in] node Node to search for.
 * @param[in] type Node type.
 * @param[in] skip_idx Index in @p set to skip.
 * @return LY_ERR
 */
static LY_ERR
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
                return LY_SUCCESS;
            }
        }

        return LY_EEXIST;
    }

    /* not found */
    return LY_SUCCESS;
}

void
lyxp_set_free_content(struct lyxp_set *set)
{
    if (!set) {
        return;
    }

    if (set->type == LYXP_SET_NODE_SET) {
        free(set->val.nodes);
        lyht_free(set->ht);
    } else if (set->type == LYXP_SET_SCNODE_SET) {
        free(set->val.scnodes);
        lyht_free(set->ht);
    } else {
        if (set->type == LYXP_SET_STRING) {
            free(set->val.str);
        }
        set->type = LYXP_SET_NODE_SET;
    }

    set->val.nodes = NULL;
    set->used = 0;
    set->size = 0;
    set->ht = NULL;
    set->ctx_pos = 0;
    set->ctx_pos = 0;
}

/**
 * @brief Free dynamically-allocated set.
 *
 * @param[in] set Set to free.
 */
static void
lyxp_set_free(struct lyxp_set *set)
{
    if (!set) {
        return;
    }

    lyxp_set_free_content(set);
    free(set);
}

/**
 * @brief Initialize set context.
 *
 * @param[in] new Set to initialize.
 * @param[in] set Arbitrary initialized set.
 */
static void
set_init(struct lyxp_set *new, const struct lyxp_set *set)
{
    memset(new, 0, sizeof *new);
    if (set) {
        new->ctx = set->ctx;
        new->cur_node = set->cur_node;
        new->root_type = set->root_type;
        new->context_op = set->context_op;
        new->tree = set->tree;
        new->cur_mod = set->cur_mod;
        new->format = set->format;
        new->prefix_data = set->prefix_data;
    }
}

/**
 * @brief Create a deep copy of a set.
 *
 * @param[in] set Set to copy.
 * @return Copy of @p set.
 */
static struct lyxp_set *
set_copy(struct lyxp_set *set)
{
    struct lyxp_set *ret;
    uint32_t i;

    if (!set) {
        return NULL;
    }

    ret = malloc(sizeof *ret);
    LY_CHECK_ERR_RET(!ret, LOGMEM(set->ctx), NULL);
    set_init(ret, set);

    if (set->type == LYXP_SET_SCNODE_SET) {
        ret->type = set->type;

        for (i = 0; i < set->used; ++i) {
            if ((set->val.scnodes[i].in_ctx == LYXP_SET_SCNODE_ATOM_CTX) ||
                    (set->val.scnodes[i].in_ctx == LYXP_SET_SCNODE_START)) {
                uint32_t idx;
                LY_CHECK_ERR_RET(lyxp_set_scnode_insert_node(ret, set->val.scnodes[i].scnode, set->val.scnodes[i].type, &idx),
                        lyxp_set_free(ret), NULL);
                /* coverity seems to think scnodes can be NULL */
                if (!ret->val.scnodes) {
                    lyxp_set_free(ret);
                    return NULL;
                }
                ret->val.scnodes[idx].in_ctx = set->val.scnodes[i].in_ctx;
            }
        }
    } else if (set->type == LYXP_SET_NODE_SET) {
        ret->type = set->type;
        ret->val.nodes = malloc(set->used * sizeof *ret->val.nodes);
        LY_CHECK_ERR_RET(!ret->val.nodes, LOGMEM(set->ctx); free(ret), NULL);
        memcpy(ret->val.nodes, set->val.nodes, set->used * sizeof *ret->val.nodes);

        ret->used = ret->size = set->used;
        ret->ctx_pos = set->ctx_pos;
        ret->ctx_size = set->ctx_size;
        if (set->ht) {
            ret->ht = lyht_dup(set->ht);
        }
    } else {
        memcpy(ret, set, sizeof *ret);
        if (set->type == LYXP_SET_STRING) {
            ret->val.str = strdup(set->val.str);
            LY_CHECK_ERR_RET(!ret->val.str, LOGMEM(set->ctx); free(ret), NULL);
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
 */
static void
set_fill_string(struct lyxp_set *set, const char *string, uint16_t str_len)
{
    lyxp_set_free_content(set);

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
    lyxp_set_free_content(set);

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
set_fill_boolean(struct lyxp_set *set, ly_bool boolean)
{
    lyxp_set_free_content(set);

    set->type = LYXP_SET_BOOLEAN;
    set->val.bln = boolean;
}

/**
 * @brief Fill XPath set with the value from another set (deep assign).
 *        Any current data are disposed of.
 *
 * @param[in] trg Set to fill.
 * @param[in] src Source set to copy into \p trg.
 */
static void
set_fill_set(struct lyxp_set *trg, const struct lyxp_set *src)
{
    if (!trg || !src) {
        return;
    }

    if (trg->type == LYXP_SET_NODE_SET) {
        free(trg->val.nodes);
    } else if (trg->type == LYXP_SET_STRING) {
        free(trg->val.str);
    }
    set_init(trg, src);

    if (src->type == LYXP_SET_SCNODE_SET) {
        trg->type = LYXP_SET_SCNODE_SET;
        trg->used = src->used;
        trg->size = src->used;

        trg->val.scnodes = ly_realloc(trg->val.scnodes, trg->size * sizeof *trg->val.scnodes);
        LY_CHECK_ERR_RET(!trg->val.scnodes, LOGMEM(src->ctx); memset(trg, 0, sizeof *trg), );
        memcpy(trg->val.scnodes, src->val.scnodes, src->used * sizeof *src->val.scnodes);
    } else if (src->type == LYXP_SET_BOOLEAN) {
        set_fill_boolean(trg, src->val.bln);
    } else if (src->type == LYXP_SET_NUMBER) {
        set_fill_number(trg, src->val.num);
    } else if (src->type == LYXP_SET_STRING) {
        set_fill_string(trg, src->val.str, strlen(src->val.str));
    } else {
        if (trg->type == LYXP_SET_NODE_SET) {
            free(trg->val.nodes);
        } else if (trg->type == LYXP_SET_STRING) {
            free(trg->val.str);
        }

        assert(src->type == LYXP_SET_NODE_SET);

        trg->type = LYXP_SET_NODE_SET;
        trg->used = src->used;
        trg->size = src->used;
        trg->ctx_pos = src->ctx_pos;
        trg->ctx_size = src->ctx_size;

        trg->val.nodes = malloc(trg->used * sizeof *trg->val.nodes);
        LY_CHECK_ERR_RET(!trg->val.nodes, LOGMEM(src->ctx); memset(trg, 0, sizeof *trg), );
        memcpy(trg->val.nodes, src->val.nodes, src->used * sizeof *src->val.nodes);
        if (src->ht) {
            trg->ht = lyht_dup(src->ht);
        } else {
            trg->ht = NULL;
        }
    }
}

/**
 * @brief Clear context of all schema nodes.
 *
 * @param[in] set Set to clear.
 * @param[in] new_ctx New context state for all the nodes currently in the context.
 */
static void
set_scnode_clear_ctx(struct lyxp_set *set, int32_t new_ctx)
{
    uint32_t i;

    for (i = 0; i < set->used; ++i) {
        if (set->val.scnodes[i].in_ctx == LYXP_SET_SCNODE_ATOM_CTX) {
            set->val.scnodes[i].in_ctx = new_ctx;
        } else if (set->val.scnodes[i].in_ctx == LYXP_SET_SCNODE_START) {
            set->val.scnodes[i].in_ctx = LYXP_SET_SCNODE_START_USED;
        }
    }
}

/**
 * @brief Remove a node from a set. Removing last node changes
 *        set into LYXP_SET_EMPTY. Context position aware.
 *
 * @param[in] set Set to use.
 * @param[in] idx Index from @p set of the node to be removed.
 */
static void
set_remove_node(struct lyxp_set *set, uint32_t idx)
{
    assert(set && (set->type == LYXP_SET_NODE_SET));
    assert(idx < set->used);

    set_remove_node_hash(set, set->val.nodes[idx].node, set->val.nodes[idx].type);

    --set->used;
    if (set->used) {
        memmove(&set->val.nodes[idx], &set->val.nodes[idx + 1],
                (set->used - idx) * sizeof *set->val.nodes);
    } else {
        lyxp_set_free_content(set);
    }
}

/**
 * @brief Remove a node from a set by setting its type to LYXP_NODE_NONE.
 *
 * @param[in] set Set to use.
 * @param[in] idx Index from @p set of the node to be removed.
 */
static void
set_remove_node_none(struct lyxp_set *set, uint32_t idx)
{
    assert(set && (set->type == LYXP_SET_NODE_SET));
    assert(idx < set->used);

    if (set->val.nodes[idx].type == LYXP_NODE_ELEM) {
        set_remove_node_hash(set, set->val.nodes[idx].node, set->val.nodes[idx].type);
    }
    set->val.nodes[idx].type = LYXP_NODE_NONE;
}

/**
 * @brief Remove all LYXP_NODE_NONE nodes from a set. Removing last node changes
 *        set into LYXP_SET_EMPTY. Context position aware.
 *
 * @param[in] set Set to consolidate.
 */
static void
set_remove_nodes_none(struct lyxp_set *set)
{
    uint32_t i, orig_used, end = 0;
    int64_t start;

    assert(set);

    orig_used = set->used;
    set->used = 0;
    for (i = 0; i < orig_used; ) {
        start = -1;
        do {
            if ((set->val.nodes[i].type != LYXP_NODE_NONE) && (start == -1)) {
                start = i;
            } else if ((start > -1) && (set->val.nodes[i].type == LYXP_NODE_NONE)) {
                end = i;
                ++i;
                break;
            }

            ++i;
            if (i == orig_used) {
                end = i;
            }
        } while (i < orig_used);

        if (start > -1) {
            /* move the whole chunk of valid nodes together */
            if (set->used != (unsigned)start) {
                memmove(&set->val.nodes[set->used], &set->val.nodes[start], (end - start) * sizeof *set->val.nodes);
            }
            set->used += end - start;
        }
    }
}

/**
 * @brief Check for duplicates in a node set.
 *
 * @param[in] set Set to check.
 * @param[in] node Node to look for in @p set.
 * @param[in] node_type Type of @p node.
 * @param[in] skip_idx Index from @p set to skip.
 * @return LY_ERR
 */
static LY_ERR
set_dup_node_check(const struct lyxp_set *set, const struct lyd_node *node, enum lyxp_node_type node_type, int skip_idx)
{
    uint32_t i;

    if (set->ht && node) {
        return set_dup_node_hash_check(set, (struct lyd_node *)node, node_type, skip_idx);
    }

    for (i = 0; i < set->used; ++i) {
        if ((skip_idx > -1) && (i == (unsigned)skip_idx)) {
            continue;
        }

        if ((set->val.nodes[i].node == node) && (set->val.nodes[i].type == node_type)) {
            return LY_EEXIST;
        }
    }

    return LY_SUCCESS;
}

ly_bool
lyxp_set_scnode_contains(struct lyxp_set *set, const struct lysc_node *node, enum lyxp_node_type node_type, int skip_idx,
        uint32_t *index_p)
{
    uint32_t i;

    for (i = 0; i < set->used; ++i) {
        if ((skip_idx > -1) && (i == (unsigned)skip_idx)) {
            continue;
        }

        if ((set->val.scnodes[i].scnode == node) && (set->val.scnodes[i].type == node_type)) {
            if (index_p) {
                *index_p = i;
            }
            return 1;
        }
    }

    return 0;
}

void
lyxp_set_scnode_merge(struct lyxp_set *set1, struct lyxp_set *set2)
{
    uint32_t orig_used, i, j;

    assert((set1->type == LYXP_SET_SCNODE_SET) && (set2->type == LYXP_SET_SCNODE_SET));

    if (!set2->used) {
        return;
    }

    if (!set1->used) {
        memcpy(set1, set2, sizeof *set1);
        return;
    }

    if (set1->used + set2->used > set1->size) {
        set1->size = set1->used + set2->used;
        set1->val.scnodes = ly_realloc(set1->val.scnodes, set1->size * sizeof *set1->val.scnodes);
        LY_CHECK_ERR_RET(!set1->val.scnodes, LOGMEM(set1->ctx), );
    }

    orig_used = set1->used;

    for (i = 0; i < set2->used; ++i) {
        for (j = 0; j < orig_used; ++j) {
            /* detect duplicities */
            if (set1->val.scnodes[j].scnode == set2->val.scnodes[i].scnode) {
                break;
            }
        }

        if (j < orig_used) {
            /* node is there, but update its status if needed */
            if (set1->val.scnodes[j].in_ctx == LYXP_SET_SCNODE_START_USED) {
                set1->val.scnodes[j].in_ctx = set2->val.scnodes[i].in_ctx;
            } else if ((set1->val.scnodes[j].in_ctx == LYXP_SET_SCNODE_ATOM_NODE) &&
                    (set2->val.scnodes[i].in_ctx == LYXP_SET_SCNODE_ATOM_VAL)) {
                set1->val.scnodes[j].in_ctx = set2->val.scnodes[i].in_ctx;
            }
        } else {
            memcpy(&set1->val.scnodes[set1->used], &set2->val.scnodes[i], sizeof *set2->val.scnodes);
            ++set1->used;
        }
    }

    lyxp_set_free_content(set2);
    set2->type = LYXP_SET_SCNODE_SET;
}

/**
 * @brief Insert a node into a set. Context position aware.
 *
 * @param[in] set Set to use.
 * @param[in] node Node to insert to @p set.
 * @param[in] pos Sort position of @p node. If left 0, it is filled just before sorting.
 * @param[in] node_type Node type of @p node.
 * @param[in] idx Index in @p set to insert into.
 */
static void
set_insert_node(struct lyxp_set *set, const struct lyd_node *node, uint32_t pos, enum lyxp_node_type node_type, uint32_t idx)
{
    assert(set && (set->type == LYXP_SET_NODE_SET));

    if (!set->size) {
        /* first item */
        if (idx) {
            /* no real harm done, but it is a bug */
            LOGINT(set->ctx);
            idx = 0;
        }
        set->val.nodes = malloc(LYXP_SET_SIZE_START * sizeof *set->val.nodes);
        LY_CHECK_ERR_RET(!set->val.nodes, LOGMEM(set->ctx), );
        set->type = LYXP_SET_NODE_SET;
        set->used = 0;
        set->size = LYXP_SET_SIZE_START;
        set->ctx_pos = 1;
        set->ctx_size = 1;
        set->ht = NULL;
    } else {
        /* not an empty set */
        if (set->used == set->size) {

            /* set is full */
            set->val.nodes = ly_realloc(set->val.nodes, (set->size + LYXP_SET_SIZE_STEP) * sizeof *set->val.nodes);
            LY_CHECK_ERR_RET(!set->val.nodes, LOGMEM(set->ctx), );
            set->size += LYXP_SET_SIZE_STEP;
        }

        if (idx > set->used) {
            LOGINT(set->ctx);
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

    if (set->val.nodes[idx].type == LYXP_NODE_ELEM) {
        set_insert_node_hash(set, (struct lyd_node *)node, node_type);
    }
}

LY_ERR
lyxp_set_scnode_insert_node(struct lyxp_set *set, const struct lysc_node *node, enum lyxp_node_type node_type, uint32_t *index_p)
{
    uint32_t index;

    assert(set->type == LYXP_SET_SCNODE_SET);

    if (lyxp_set_scnode_contains(set, node, node_type, -1, &index)) {
        set->val.scnodes[index].in_ctx = LYXP_SET_SCNODE_ATOM_CTX;
    } else {
        if (set->used == set->size) {
            set->val.scnodes = ly_realloc(set->val.scnodes, (set->size + LYXP_SET_SIZE_STEP) * sizeof *set->val.scnodes);
            LY_CHECK_ERR_RET(!set->val.scnodes, LOGMEM(set->ctx), LY_EMEM);
            set->size += LYXP_SET_SIZE_STEP;
        }

        index = set->used;
        set->val.scnodes[index].scnode = (struct lysc_node *)node;
        set->val.scnodes[index].type = node_type;
        set->val.scnodes[index].in_ctx = LYXP_SET_SCNODE_ATOM_CTX;
        ++set->used;
    }

    if (index_p) {
        *index_p = index;
    }

    return LY_SUCCESS;
}

/**
 * @brief Replace a node in a set with another. Context position aware.
 *
 * @param[in] set Set to use.
 * @param[in] node Node to insert to @p set.
 * @param[in] pos Sort position of @p node. If left 0, it is filled just before sorting.
 * @param[in] node_type Node type of @p node.
 * @param[in] idx Index in @p set of the node to replace.
 */
static void
set_replace_node(struct lyxp_set *set, const struct lyd_node *node, uint32_t pos, enum lyxp_node_type node_type, uint32_t idx)
{
    assert(set && (idx < set->used));

    if (set->val.nodes[idx].type == LYXP_NODE_ELEM) {
        set_remove_node_hash(set, set->val.nodes[idx].node, set->val.nodes[idx].type);
    }
    set->val.nodes[idx].node = (struct lyd_node *)node;
    set->val.nodes[idx].type = node_type;
    set->val.nodes[idx].pos = pos;
    if (set->val.nodes[idx].type == LYXP_NODE_ELEM) {
        set_insert_node_hash(set, set->val.nodes[idx].node, set->val.nodes[idx].type);
    }
}

/**
 * @brief Set all nodes with ctx 1 to a new unique context value.
 *
 * @param[in] set Set to modify.
 * @return New context value.
 */
static int32_t
set_scnode_new_in_ctx(struct lyxp_set *set)
{
    uint32_t i;
    int32_t ret_ctx;

    assert(set->type == LYXP_SET_SCNODE_SET);

    ret_ctx = LYXP_SET_SCNODE_ATOM_PRED_CTX;
retry:
    for (i = 0; i < set->used; ++i) {
        if (set->val.scnodes[i].in_ctx >= ret_ctx) {
            ret_ctx = set->val.scnodes[i].in_ctx + 1;
            goto retry;
        }
    }
    for (i = 0; i < set->used; ++i) {
        if (set->val.scnodes[i].in_ctx == LYXP_SET_SCNODE_ATOM_CTX) {
            set->val.scnodes[i].in_ctx = ret_ctx;
        }
    }

    return ret_ctx;
}

/**
 * @brief Get unique @p node position in the data.
 *
 * @param[in] node Node to find.
 * @param[in] node_type Node type of @p node.
 * @param[in] root Root node.
 * @param[in] root_type Type of the XPath @p root node.
 * @param[in] prev Node that we think is before @p node in DFS from @p root. Can optionally
 * be used to increase efficiency and start the DFS from this node.
 * @param[in] prev_pos Node @p prev position. Optional, but must be set if @p prev is set.
 * @return Node position.
 */
static uint32_t
get_node_pos(const struct lyd_node *node, enum lyxp_node_type node_type, const struct lyd_node *root,
        enum lyxp_node_type root_type, const struct lyd_node **prev, uint32_t *prev_pos)
{
    const struct lyd_node *elem = NULL, *top_sibling;
    uint32_t pos = 1;
    ly_bool found = 0;

    assert(prev && prev_pos && !root->prev->next);

    if ((node_type == LYXP_NODE_ROOT) || (node_type == LYXP_NODE_ROOT_CONFIG)) {
        return 0;
    }

    if (*prev) {
        /* start from the previous element instead from the root */
        pos = *prev_pos;
        for (top_sibling = *prev; top_sibling->parent; top_sibling = lyd_parent(top_sibling)) {}
        goto dfs_search;
    }

    LY_LIST_FOR(root, top_sibling) {
        LYD_TREE_DFS_BEGIN(top_sibling, elem) {
dfs_search:
            if (*prev && !elem) {
                /* resume previous DFS */
                elem = LYD_TREE_DFS_next = (struct lyd_node *)*prev;
                LYD_TREE_DFS_continue = 0;
            }

            if ((root_type == LYXP_NODE_ROOT_CONFIG) && (elem->schema->flags & LYS_CONFIG_R)) {
                /* skip */
                LYD_TREE_DFS_continue = 1;
            } else {
                if (elem == node) {
                    found = 1;
                    break;
                }
                ++pos;
            }

            LYD_TREE_DFS_END(top_sibling, elem);
        }

        /* node found */
        if (found) {
            break;
        }
    }

    if (!found) {
        if (!(*prev)) {
            /* we went from root and failed to find it, cannot be */
            LOGINT(LYD_CTX(node));
            return 0;
        } else {
            /* start the search again from the beginning */
            *prev = root;

            top_sibling = root;
            pos = 1;
            goto dfs_search;
        }
    }

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
 * @return LY_ERR
 */
static LY_ERR
set_assign_pos(struct lyxp_set *set, const struct lyd_node *root, enum lyxp_node_type root_type)
{
    const struct lyd_node *prev = NULL, *tmp_node;
    uint32_t i, tmp_pos = 0;

    for (i = 0; i < set->used; ++i) {
        if (!set->val.nodes[i].pos) {
            tmp_node = NULL;
            switch (set->val.nodes[i].type) {
            case LYXP_NODE_META:
                tmp_node = set->val.meta[i].meta->parent;
                if (!tmp_node) {
                    LOGINT_RET(root->schema->module->ctx);
                }
            /* fall through */
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

    return LY_SUCCESS;
}

/**
 * @brief Get unique @p meta position in the parent metadata.
 *
 * @param[in] meta Metadata to use.
 * @return Metadata position.
 */
static uint16_t
get_meta_pos(struct lyd_meta *meta)
{
    uint16_t pos = 0;
    struct lyd_meta *meta2;

    for (meta2 = meta->parent->meta; meta2 && (meta2 != meta); meta2 = meta2->next) {
        ++pos;
    }

    assert(meta2);
    return pos;
}

/**
 * @brief Compare 2 nodes in respect to XPath document order.
 *
 * @param[in] item1 1st node.
 * @param[in] item2 2nd node.
 * @return If 1st > 2nd returns 1, 1st == 2nd returns 0, and 1st < 2nd returns -1.
 */
static int
set_sort_compare(struct lyxp_set_node *item1, struct lyxp_set_node *item2)
{
    uint32_t meta_pos1 = 0, meta_pos2 = 0;

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

    /* we need meta positions now */
    if (item1->type == LYXP_NODE_META) {
        meta_pos1 = get_meta_pos((struct lyd_meta *)item1->node);
    }
    if (item2->type == LYXP_NODE_META) {
        meta_pos2 = get_meta_pos((struct lyd_meta *)item2->node);
    }

    /* 1st ROOT - 2nd ROOT, 1st ELEM - 2nd ELEM, 1st TEXT - 2nd TEXT, 1st META - =pos= - 2nd META */
    /* check for duplicates */
    if (item1->node == item2->node) {
        assert((item1->type == item2->type) && ((item1->type != LYXP_NODE_META) || (meta_pos1 == meta_pos2)));
        return 0;
    }

    /* 1st ELEM - 2nd TEXT, 1st ELEM - any pos - 2nd META */
    /* elem is always first, 2nd node is after it */
    if (item1->type == LYXP_NODE_ELEM) {
        assert(item2->type != LYXP_NODE_ELEM);
        return -1;
    }

    /* 1st TEXT - 2nd ELEM, 1st TEXT - any pos - 2nd META, 1st META - any pos - 2nd ELEM, 1st META - >pos> - 2nd META */
    /* 2nd is before 1st */
    if (((item1->type == LYXP_NODE_TEXT) &&
            ((item2->type == LYXP_NODE_ELEM) || (item2->type == LYXP_NODE_META))) ||
            ((item1->type == LYXP_NODE_META) && (item2->type == LYXP_NODE_ELEM)) ||
            (((item1->type == LYXP_NODE_META) && (item2->type == LYXP_NODE_META)) &&
            (meta_pos1 > meta_pos2))) {
        return 1;
    }

    /* 1st META - any pos - 2nd TEXT, 1st META <pos< - 2nd META */
    /* 2nd is after 1st */
    return -1;
}

/**
 * @brief Set cast for comparisons.
 *
 * @param[in] trg Target set to cast source into.
 * @param[in] src Source set.
 * @param[in] type Target set type.
 * @param[in] src_idx Source set node index.
 * @return LY_ERR
 */
static LY_ERR
set_comp_cast(struct lyxp_set *trg, struct lyxp_set *src, enum lyxp_set_type type, uint32_t src_idx)
{
    assert(src->type == LYXP_SET_NODE_SET);

    set_init(trg, src);

    /* insert node into target set */
    set_insert_node(trg, src->val.nodes[src_idx].node, src->val.nodes[src_idx].pos, src->val.nodes[src_idx].type, 0);

    /* cast target set appropriately */
    return lyxp_set_cast(trg, type);
}

/**
 * @brief Set content canonization for comparisons.
 *
 * @param[in] trg Target set to put the canononized source into.
 * @param[in] src Source set.
 * @param[in] xp_node Source XPath node/meta to use for canonization.
 * @return LY_ERR
 */
static LY_ERR
set_comp_canonize(struct lyxp_set *trg, const struct lyxp_set *src, const struct lyxp_set_node *xp_node)
{
    const struct lysc_type *type = NULL;
    struct lyd_value val;
    struct ly_err_item *err = NULL;
    char *str, *ptr;
    LY_ERR rc;

    /* is there anything to canonize even? */
    if ((src->type == LYXP_SET_NUMBER) || (src->type == LYXP_SET_STRING)) {
        /* do we have a type to use for canonization? */
        if ((xp_node->type == LYXP_NODE_ELEM) && (xp_node->node->schema->nodetype & LYD_NODE_TERM)) {
            type = ((struct lyd_node_term *)xp_node->node)->value.realtype;
        } else if (xp_node->type == LYXP_NODE_META) {
            type = ((struct lyd_meta *)xp_node->node)->value.realtype;
        }
    }
    if (!type) {
        goto fill;
    }

    if (src->type == LYXP_SET_NUMBER) {
        /* canonize number */
        if (asprintf(&str, "%Lf", src->val.num) == -1) {
            LOGMEM(src->ctx);
            return LY_EMEM;
        }
    } else {
        /* canonize string */
        str = strdup(src->val.str);
    }

    /* ignore errors, the value may not satisfy schema constraints */
    rc = type->plugin->store(src->ctx, type, str, strlen(str), LYPLG_TYPE_STORE_DYNAMIC, src->format, src->prefix_data,
            LYD_HINT_DATA, xp_node->node->schema, &val, NULL, &err);
    ly_err_free(err);
    if (rc) {
        /* invalid value */
        /* function store automaticaly dealloc value when fail */
        goto fill;
    }

    /* use the canonized value */
    set_init(trg, src);
    trg->type = src->type;
    if (src->type == LYXP_SET_NUMBER) {
        trg->val.num = strtold(type->plugin->print(src->ctx, &val, LY_VALUE_CANON, NULL, NULL, NULL), &ptr);
        LY_CHECK_ERR_RET(ptr[0], LOGINT(src->ctx), LY_EINT);
    } else {
        trg->val.str = strdup(type->plugin->print(src->ctx, &val, LY_VALUE_CANON, NULL, NULL, NULL));
    }
    type->plugin->free(src->ctx, &val);
    return LY_SUCCESS;

fill:
    /* no canonization needed/possible */
    set_fill_set(trg, src);
    return LY_SUCCESS;
}

#ifndef NDEBUG

/**
 * @brief Bubble sort @p set into XPath document order.
 *        Context position aware. Unused in the 'Release' build target.
 *
 * @param[in] set Set to sort.
 * @return How many times the whole set was traversed - 1 (if set was sorted, returns 0).
 */
static int
set_sort(struct lyxp_set *set)
{
    uint32_t i, j;
    int ret = 0, cmp;
    ly_bool inverted, change;
    const struct lyd_node *root;
    struct lyxp_set_node item;
    struct lyxp_set_hash_node hnode;
    uint64_t hash;

    if ((set->type != LYXP_SET_NODE_SET) || (set->used < 2)) {
        return 0;
    }

    /* find first top-level node to be used as anchor for positions */
    for (root = set->cur_node; root->parent; root = lyd_parent(root)) {}
    for ( ; root->prev->next; root = root->prev) {}

    /* fill positions */
    if (set_assign_pos(set, root, set->root_type)) {
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
                cmp = set_sort_compare(&set->val.nodes[j], &set->val.nodes[j - 1]);
            } else {
                cmp = set_sort_compare(&set->val.nodes[j - 1], &set->val.nodes[j]);
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

    /* check node hashes */
    if (set->used >= LYD_HT_MIN_ITEMS) {
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

    return ret - 1;
}

/**
 * @brief Remove duplicate entries in a sorted node set.
 *
 * @param[in] set Sorted set to check.
 * @return LY_ERR (LY_EEXIST if some duplicates are found)
 */
static LY_ERR
set_sorted_dup_node_clean(struct lyxp_set *set)
{
    uint32_t i = 0;
    LY_ERR ret = LY_SUCCESS;

    if (set->used > 1) {
        while (i < set->used - 1) {
            if ((set->val.nodes[i].node == set->val.nodes[i + 1].node) &&
                    (set->val.nodes[i].type == set->val.nodes[i + 1].type)) {
                set_remove_node_none(set, i + 1);
                ret = LY_EEXIST;
            }
            ++i;
        }
    }

    set_remove_nodes_none(set);
    return ret;
}

#endif

/**
 * @brief Merge 2 sorted sets into one.
 *
 * @param[in,out] trg Set to merge into. Duplicates are removed.
 * @param[in] src Set to be merged into @p trg. It is cast to #LYXP_SET_EMPTY on success.
 * @return LY_ERR
 */
static LY_ERR
set_sorted_merge(struct lyxp_set *trg, struct lyxp_set *src)
{
    uint32_t i, j, k, count, dup_count;
    int cmp;
    const struct lyd_node *root;

    if ((trg->type != LYXP_SET_NODE_SET) || (src->type != LYXP_SET_NODE_SET)) {
        return LY_EINVAL;
    }

    if (!src->used) {
        return LY_SUCCESS;
    } else if (!trg->used) {
        set_fill_set(trg, src);
        lyxp_set_free_content(src);
        return LY_SUCCESS;
    }

    /* find first top-level node to be used as anchor for positions */
    for (root = trg->cur_node; root->parent; root = lyd_parent(root)) {}
    for ( ; root->prev->next; root = root->prev) {}

    /* fill positions */
    if (set_assign_pos(trg, root, trg->root_type) || set_assign_pos(src, root, src->root_type)) {
        return LY_EINT;
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
        LY_CHECK_ERR_RET(!trg->val.nodes, LOGMEM(src->ctx), LY_EMEM);
    }

    i = 0;
    j = 0;
    count = 0;
    dup_count = 0;
    do {
        cmp = set_sort_compare(&src->val.nodes[i], &trg->val.nodes[j]);
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

            /* insert the hash now */
            set_insert_node_hash(trg, src->val.nodes[i - 1].node, src->val.nodes[i - 1].type);
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
        /* insert all the hashes first */
        for (k = i; k < src->used; ++k) {
            set_insert_node_hash(trg, src->val.nodes[k].node, src->val.nodes[k].type);
        }

        /* loop ended, but we need to copy something at trg end */
        count += src->used - i;
        i = src->used;
        goto copy_nodes;
    }

    /* we are inserting hashes before the actual node insert, which causes
     * situations when there were initially not enough items for a hash table,
     * but even after some were inserted, hash table was not created (during
     * insertion the number of items is not updated yet) */
    if (!trg->ht && (trg->used >= LYD_HT_MIN_ITEMS)) {
        set_insert_node_hash(trg, NULL, 0);
    }

#ifndef NDEBUG
    LOGDBG(LY_LDGXPATH, "MERGE result");
    print_set_debug(trg);
#endif

    lyxp_set_free_content(src);
    return LY_SUCCESS;
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

LY_ERR
lyxp_check_token(const struct ly_ctx *ctx, const struct lyxp_expr *exp, uint16_t tok_idx, enum lyxp_token want_tok)
{
    if (exp->used == tok_idx) {
        if (ctx) {
            LOGVAL(ctx, LY_VCODE_XP_EOF);
        }
        return LY_EINCOMPLETE;
    }

    if (want_tok && (exp->tokens[tok_idx] != want_tok)) {
        if (ctx) {
            LOGVAL(ctx, LY_VCODE_XP_INTOK2, lyxp_print_token(exp->tokens[tok_idx]),
                    &exp->expr[exp->tok_pos[tok_idx]], lyxp_print_token(want_tok));
        }
        return LY_ENOT;
    }

    return LY_SUCCESS;
}

LY_ERR
lyxp_next_token(const struct ly_ctx *ctx, const struct lyxp_expr *exp, uint16_t *tok_idx, enum lyxp_token want_tok)
{
    LY_CHECK_RET(lyxp_check_token(ctx, exp, *tok_idx, want_tok));

    /* skip the token */
    ++(*tok_idx);

    return LY_SUCCESS;
}

/* just like lyxp_check_token() but tests for 2 tokens */
static LY_ERR
exp_check_token2(const struct ly_ctx *ctx, const struct lyxp_expr *exp, uint16_t tok_idx, enum lyxp_token want_tok1,
        enum lyxp_token want_tok2)
{
    if (exp->used == tok_idx) {
        if (ctx) {
            LOGVAL(ctx, LY_VCODE_XP_EOF);
        }
        return LY_EINCOMPLETE;
    }

    if ((exp->tokens[tok_idx] != want_tok1) && (exp->tokens[tok_idx] != want_tok2)) {
        if (ctx) {
            LOGVAL(ctx, LY_VCODE_XP_INTOK, lyxp_print_token(exp->tokens[tok_idx]),
                    &exp->expr[exp->tok_pos[tok_idx]]);
        }
        return LY_ENOT;
    }

    return LY_SUCCESS;
}

/**
 * @brief Stack operation push on the repeat array.
 *
 * @param[in] exp Expression to use.
 * @param[in] tok_idx Position in the expresion \p exp.
 * @param[in] repeat_op_idx Index from \p exp of the operator token. This value is pushed.
 */
static void
exp_repeat_push(struct lyxp_expr *exp, uint16_t tok_idx, uint16_t repeat_op_idx)
{
    uint16_t i;

    if (exp->repeat[tok_idx]) {
        for (i = 0; exp->repeat[tok_idx][i]; ++i) {}
        exp->repeat[tok_idx] = realloc(exp->repeat[tok_idx], (i + 2) * sizeof *exp->repeat[tok_idx]);
        LY_CHECK_ERR_RET(!exp->repeat[tok_idx], LOGMEM(NULL), );
        exp->repeat[tok_idx][i] = repeat_op_idx;
        exp->repeat[tok_idx][i + 1] = 0;
    } else {
        exp->repeat[tok_idx] = calloc(2, sizeof *exp->repeat[tok_idx]);
        LY_CHECK_ERR_RET(!exp->repeat[tok_idx], LOGMEM(NULL), );
        exp->repeat[tok_idx][0] = repeat_op_idx;
    }
}

/**
 * @brief Reparse Predicate. Logs directly on error.
 *
 * [7] Predicate ::= '[' Expr ']'
 *
 * @param[in] ctx Context for logging.
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @return LY_ERR
 */
static LY_ERR
reparse_predicate(const struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *tok_idx)
{
    LY_ERR rc;

    rc = lyxp_check_token(ctx, exp, *tok_idx, LYXP_TOKEN_BRACK1);
    LY_CHECK_RET(rc);
    ++(*tok_idx);

    rc = reparse_or_expr(ctx, exp, tok_idx);
    LY_CHECK_RET(rc);

    rc = lyxp_check_token(ctx, exp, *tok_idx, LYXP_TOKEN_BRACK2);
    LY_CHECK_RET(rc);
    ++(*tok_idx);

    return LY_SUCCESS;
}

/**
 * @brief Reparse RelativeLocationPath. Logs directly on error.
 *
 * [4] RelativeLocationPath ::= Step | RelativeLocationPath '/' Step | RelativeLocationPath '//' Step
 * [5] Step ::= '@'? NodeTest Predicate* | '.' | '..'
 * [6] NodeTest ::= NameTest | NodeType '(' ')'
 *
 * @param[in] ctx Context for logging.
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression \p exp.
 * @return LY_ERR (LY_EINCOMPLETE on forward reference)
 */
static LY_ERR
reparse_relative_location_path(const struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *tok_idx)
{
    LY_ERR rc;

    rc = lyxp_check_token(ctx, exp, *tok_idx, LYXP_TOKEN_NONE);
    LY_CHECK_RET(rc);

    goto step;
    do {
        /* '/' or '//' */
        ++(*tok_idx);

        rc = lyxp_check_token(ctx, exp, *tok_idx, LYXP_TOKEN_NONE);
        LY_CHECK_RET(rc);
step:
        /* Step */
        switch (exp->tokens[*tok_idx]) {
        case LYXP_TOKEN_DOT:
            ++(*tok_idx);
            break;

        case LYXP_TOKEN_DDOT:
            ++(*tok_idx);
            break;

        case LYXP_TOKEN_AT:
            ++(*tok_idx);

            rc = lyxp_check_token(ctx, exp, *tok_idx, LYXP_TOKEN_NONE);
            LY_CHECK_RET(rc);
            if ((exp->tokens[*tok_idx] != LYXP_TOKEN_NAMETEST) && (exp->tokens[*tok_idx] != LYXP_TOKEN_NODETYPE)) {
                LOGVAL(ctx, LY_VCODE_XP_INTOK, lyxp_print_token(exp->tokens[*tok_idx]), &exp->expr[exp->tok_pos[*tok_idx]]);
                return LY_EVALID;
            }
        /* fall through */
        case LYXP_TOKEN_NAMETEST:
            ++(*tok_idx);
            goto reparse_predicate;
            break;

        case LYXP_TOKEN_NODETYPE:
            ++(*tok_idx);

            /* '(' */
            rc = lyxp_check_token(ctx, exp, *tok_idx, LYXP_TOKEN_PAR1);
            LY_CHECK_RET(rc);
            ++(*tok_idx);

            /* ')' */
            rc = lyxp_check_token(ctx, exp, *tok_idx, LYXP_TOKEN_PAR2);
            LY_CHECK_RET(rc);
            ++(*tok_idx);

reparse_predicate:
            /* Predicate* */
            while (!lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_BRACK1)) {
                rc = reparse_predicate(ctx, exp, tok_idx);
                LY_CHECK_RET(rc);
            }
            break;
        default:
            LOGVAL(ctx, LY_VCODE_XP_INTOK, lyxp_print_token(exp->tokens[*tok_idx]), &exp->expr[exp->tok_pos[*tok_idx]]);
            return LY_EVALID;
        }
    } while (!exp_check_token2(NULL, exp, *tok_idx, LYXP_TOKEN_OPER_PATH, LYXP_TOKEN_OPER_RPATH));

    return LY_SUCCESS;
}

/**
 * @brief Reparse AbsoluteLocationPath. Logs directly on error.
 *
 * [3] AbsoluteLocationPath ::= '/' RelativeLocationPath? | '//' RelativeLocationPath
 *
 * @param[in] ctx Context for logging.
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression \p exp.
 * @return LY_ERR
 */
static LY_ERR
reparse_absolute_location_path(const struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *tok_idx)
{
    LY_ERR rc;

    LY_CHECK_RET(exp_check_token2(ctx, exp, *tok_idx, LYXP_TOKEN_OPER_PATH, LYXP_TOKEN_OPER_RPATH));

    /* '/' RelativeLocationPath? */
    if (exp->tokens[*tok_idx] == LYXP_TOKEN_OPER_PATH) {
        /* '/' */
        ++(*tok_idx);

        if (lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_NONE)) {
            return LY_SUCCESS;
        }
        switch (exp->tokens[*tok_idx]) {
        case LYXP_TOKEN_DOT:
        case LYXP_TOKEN_DDOT:
        case LYXP_TOKEN_AT:
        case LYXP_TOKEN_NAMETEST:
        case LYXP_TOKEN_NODETYPE:
            rc = reparse_relative_location_path(ctx, exp, tok_idx);
            LY_CHECK_RET(rc);
        /* fall through */
        default:
            break;
        }

    } else {
        /* '//' RelativeLocationPath */
        ++(*tok_idx);

        rc = reparse_relative_location_path(ctx, exp, tok_idx);
        LY_CHECK_RET(rc);
    }

    return LY_SUCCESS;
}

/**
 * @brief Reparse FunctionCall. Logs directly on error.
 *
 * [9] FunctionCall ::= FunctionName '(' ( Expr ( ',' Expr )* )? ')'
 *
 * @param[in] ctx Context for logging.
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @return LY_ERR
 */
static LY_ERR
reparse_function_call(const struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *tok_idx)
{
    int8_t min_arg_count = -1;
    uint32_t arg_count, max_arg_count = 0;
    uint16_t func_tok_idx;
    LY_ERR rc;

    rc = lyxp_check_token(ctx, exp, *tok_idx, LYXP_TOKEN_FUNCNAME);
    LY_CHECK_RET(rc);
    func_tok_idx = *tok_idx;
    switch (exp->tok_len[*tok_idx]) {
    case 3:
        if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "not", 3)) {
            min_arg_count = 1;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "sum", 3)) {
            min_arg_count = 1;
            max_arg_count = 1;
        }
        break;
    case 4:
        if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "lang", 4)) {
            min_arg_count = 1;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "last", 4)) {
            min_arg_count = 0;
            max_arg_count = 0;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "name", 4)) {
            min_arg_count = 0;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "true", 4)) {
            min_arg_count = 0;
            max_arg_count = 0;
        }
        break;
    case 5:
        if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "count", 5)) {
            min_arg_count = 1;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "false", 5)) {
            min_arg_count = 0;
            max_arg_count = 0;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "floor", 5)) {
            min_arg_count = 1;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "round", 5)) {
            min_arg_count = 1;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "deref", 5)) {
            min_arg_count = 1;
            max_arg_count = 1;
        }
        break;
    case 6:
        if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "concat", 6)) {
            min_arg_count = 2;
            max_arg_count = UINT32_MAX;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "number", 6)) {
            min_arg_count = 0;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "string", 6)) {
            min_arg_count = 0;
            max_arg_count = 1;
        }
        break;
    case 7:
        if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "boolean", 7)) {
            min_arg_count = 1;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "ceiling", 7)) {
            min_arg_count = 1;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "current", 7)) {
            min_arg_count = 0;
            max_arg_count = 0;
        }
        break;
    case 8:
        if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "contains", 8)) {
            min_arg_count = 2;
            max_arg_count = 2;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "position", 8)) {
            min_arg_count = 0;
            max_arg_count = 0;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "re-match", 8)) {
            min_arg_count = 2;
            max_arg_count = 2;
        }
        break;
    case 9:
        if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "substring", 9)) {
            min_arg_count = 2;
            max_arg_count = 3;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "translate", 9)) {
            min_arg_count = 3;
            max_arg_count = 3;
        }
        break;
    case 10:
        if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "local-name", 10)) {
            min_arg_count = 0;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "enum-value", 10)) {
            min_arg_count = 1;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "bit-is-set", 10)) {
            min_arg_count = 2;
            max_arg_count = 2;
        }
        break;
    case 11:
        if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "starts-with", 11)) {
            min_arg_count = 2;
            max_arg_count = 2;
        }
        break;
    case 12:
        if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "derived-from", 12)) {
            min_arg_count = 2;
            max_arg_count = 2;
        }
        break;
    case 13:
        if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "namespace-uri", 13)) {
            min_arg_count = 0;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "string-length", 13)) {
            min_arg_count = 0;
            max_arg_count = 1;
        }
        break;
    case 15:
        if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "normalize-space", 15)) {
            min_arg_count = 0;
            max_arg_count = 1;
        } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "substring-after", 15)) {
            min_arg_count = 2;
            max_arg_count = 2;
        }
        break;
    case 16:
        if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "substring-before", 16)) {
            min_arg_count = 2;
            max_arg_count = 2;
        }
        break;
    case 20:
        if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "derived-from-or-self", 20)) {
            min_arg_count = 2;
            max_arg_count = 2;
        }
        break;
    }
    if (min_arg_count == -1) {
        LOGVAL(ctx, LY_VCODE_XP_INFUNC, exp->tok_len[*tok_idx], &exp->expr[exp->tok_pos[*tok_idx]]);
        return LY_EINVAL;
    }
    ++(*tok_idx);

    /* '(' */
    rc = lyxp_check_token(ctx, exp, *tok_idx, LYXP_TOKEN_PAR1);
    LY_CHECK_RET(rc);
    ++(*tok_idx);

    /* ( Expr ( ',' Expr )* )? */
    arg_count = 0;
    rc = lyxp_check_token(ctx, exp, *tok_idx, LYXP_TOKEN_NONE);
    LY_CHECK_RET(rc);
    if (exp->tokens[*tok_idx] != LYXP_TOKEN_PAR2) {
        ++arg_count;
        rc = reparse_or_expr(ctx, exp, tok_idx);
        LY_CHECK_RET(rc);
    }
    while (!lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_COMMA)) {
        ++(*tok_idx);

        ++arg_count;
        rc = reparse_or_expr(ctx, exp, tok_idx);
        LY_CHECK_RET(rc);
    }

    /* ')' */
    rc = lyxp_check_token(ctx, exp, *tok_idx, LYXP_TOKEN_PAR2);
    LY_CHECK_RET(rc);
    ++(*tok_idx);

    if ((arg_count < (uint32_t)min_arg_count) || (arg_count > max_arg_count)) {
        LOGVAL(ctx, LY_VCODE_XP_INARGCOUNT, arg_count, exp->tok_len[func_tok_idx], &exp->expr[exp->tok_pos[func_tok_idx]]);
        return LY_EVALID;
    }

    return LY_SUCCESS;
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
 * @param[in] ctx Context for logging.
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @return LY_ERR
 */
static LY_ERR
reparse_path_expr(const struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *tok_idx)
{
    LY_ERR rc;

    if (lyxp_check_token(ctx, exp, *tok_idx, LYXP_TOKEN_NONE)) {
        return LY_EVALID;
    }

    switch (exp->tokens[*tok_idx]) {
    case LYXP_TOKEN_PAR1:
        /* '(' Expr ')' Predicate* */
        ++(*tok_idx);

        rc = reparse_or_expr(ctx, exp, tok_idx);
        LY_CHECK_RET(rc);

        rc = lyxp_check_token(ctx, exp, *tok_idx, LYXP_TOKEN_PAR2);
        LY_CHECK_RET(rc);
        ++(*tok_idx);
        goto predicate;
        break;
    case LYXP_TOKEN_DOT:
    case LYXP_TOKEN_DDOT:
    case LYXP_TOKEN_AT:
    case LYXP_TOKEN_NAMETEST:
    case LYXP_TOKEN_NODETYPE:
        /* RelativeLocationPath */
        rc = reparse_relative_location_path(ctx, exp, tok_idx);
        LY_CHECK_RET(rc);
        break;
    case LYXP_TOKEN_FUNCNAME:
        /* FunctionCall */
        rc = reparse_function_call(ctx, exp, tok_idx);
        LY_CHECK_RET(rc);
        goto predicate;
        break;
    case LYXP_TOKEN_OPER_PATH:
    case LYXP_TOKEN_OPER_RPATH:
        /* AbsoluteLocationPath */
        rc = reparse_absolute_location_path(ctx, exp, tok_idx);
        LY_CHECK_RET(rc);
        break;
    case LYXP_TOKEN_LITERAL:
        /* Literal */
        ++(*tok_idx);
        goto predicate;
        break;
    case LYXP_TOKEN_NUMBER:
        /* Number */
        ++(*tok_idx);
        goto predicate;
        break;
    default:
        LOGVAL(ctx, LY_VCODE_XP_INTOK, lyxp_print_token(exp->tokens[*tok_idx]), &exp->expr[exp->tok_pos[*tok_idx]]);
        return LY_EVALID;
    }

    return LY_SUCCESS;

predicate:
    /* Predicate* */
    while (!lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_BRACK1)) {
        rc = reparse_predicate(ctx, exp, tok_idx);
        LY_CHECK_RET(rc);
    }

    /* ('/' or '//') RelativeLocationPath */
    if (!exp_check_token2(NULL, exp, *tok_idx, LYXP_TOKEN_OPER_PATH, LYXP_TOKEN_OPER_RPATH)) {

        /* '/' or '//' */
        ++(*tok_idx);

        rc = reparse_relative_location_path(ctx, exp, tok_idx);
        LY_CHECK_RET(rc);
    }

    return LY_SUCCESS;
}

/**
 * @brief Reparse UnaryExpr. Logs directly on error.
 *
 * [17] UnaryExpr ::= UnionExpr | '-' UnaryExpr
 * [18] UnionExpr ::= PathExpr | UnionExpr '|' PathExpr
 *
 * @param[in] ctx Context for logging.
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @return LY_ERR
 */
static LY_ERR
reparse_unary_expr(const struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *tok_idx)
{
    uint16_t prev_exp;
    LY_ERR rc;

    /* ('-')* */
    prev_exp = *tok_idx;
    while (!lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_OPER_MATH) &&
            (exp->expr[exp->tok_pos[*tok_idx]] == '-')) {
        exp_repeat_push(exp, prev_exp, LYXP_EXPR_UNARY);
        ++(*tok_idx);
    }

    /* PathExpr */
    prev_exp = *tok_idx;
    rc = reparse_path_expr(ctx, exp, tok_idx);
    LY_CHECK_RET(rc);

    /* ('|' PathExpr)* */
    while (!lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_OPER_UNI)) {
        exp_repeat_push(exp, prev_exp, LYXP_EXPR_UNION);
        ++(*tok_idx);

        rc = reparse_path_expr(ctx, exp, tok_idx);
        LY_CHECK_RET(rc);
    }

    return LY_SUCCESS;
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
 * @param[in] ctx Context for logging.
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @return LY_ERR
 */
static LY_ERR
reparse_additive_expr(const struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *tok_idx)
{
    uint16_t prev_add_exp, prev_mul_exp;
    LY_ERR rc;

    prev_add_exp = *tok_idx;
    goto reparse_multiplicative_expr;

    /* ('+' / '-' MultiplicativeExpr)* */
    while (!lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_OPER_MATH) &&
            ((exp->expr[exp->tok_pos[*tok_idx]] == '+') || (exp->expr[exp->tok_pos[*tok_idx]] == '-'))) {
        exp_repeat_push(exp, prev_add_exp, LYXP_EXPR_ADDITIVE);
        ++(*tok_idx);

reparse_multiplicative_expr:
        /* UnaryExpr */
        prev_mul_exp = *tok_idx;
        rc = reparse_unary_expr(ctx, exp, tok_idx);
        LY_CHECK_RET(rc);

        /* ('*' / 'div' / 'mod' UnaryExpr)* */
        while (!lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_OPER_MATH) &&
                ((exp->expr[exp->tok_pos[*tok_idx]] == '*') || (exp->tok_len[*tok_idx] == 3))) {
            exp_repeat_push(exp, prev_mul_exp, LYXP_EXPR_MULTIPLICATIVE);
            ++(*tok_idx);

            rc = reparse_unary_expr(ctx, exp, tok_idx);
            LY_CHECK_RET(rc);
        }
    }

    return LY_SUCCESS;
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
 * @param[in] ctx Context for logging.
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @return LY_ERR
 */
static LY_ERR
reparse_equality_expr(const struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *tok_idx)
{
    uint16_t prev_eq_exp, prev_rel_exp;
    LY_ERR rc;

    prev_eq_exp = *tok_idx;
    goto reparse_additive_expr;

    /* ('=' / '!=' RelationalExpr)* */
    while (!exp_check_token2(NULL, exp, *tok_idx, LYXP_TOKEN_OPER_EQUAL, LYXP_TOKEN_OPER_NEQUAL)) {
        exp_repeat_push(exp, prev_eq_exp, LYXP_EXPR_EQUALITY);
        ++(*tok_idx);

reparse_additive_expr:
        /* AdditiveExpr */
        prev_rel_exp = *tok_idx;
        rc = reparse_additive_expr(ctx, exp, tok_idx);
        LY_CHECK_RET(rc);

        /* ('<' / '>' / '<=' / '>=' AdditiveExpr)* */
        while (!lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_OPER_COMP)) {
            exp_repeat_push(exp, prev_rel_exp, LYXP_EXPR_RELATIONAL);
            ++(*tok_idx);

            rc = reparse_additive_expr(ctx, exp, tok_idx);
            LY_CHECK_RET(rc);
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Reparse OrExpr. Logs directly on error.
 *
 * [11] OrExpr ::= AndExpr | OrExpr 'or' AndExpr
 * [12] AndExpr ::= EqualityExpr | AndExpr 'and' EqualityExpr
 *
 * @param[in] ctx Context for logging.
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @return LY_ERR
 */
static LY_ERR
reparse_or_expr(const struct ly_ctx *ctx, struct lyxp_expr *exp, uint16_t *tok_idx)
{
    uint16_t prev_or_exp, prev_and_exp;
    LY_ERR rc;

    prev_or_exp = *tok_idx;
    goto reparse_equality_expr;

    /* ('or' AndExpr)* */
    while (!lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_OPER_LOG) && (exp->tok_len[*tok_idx] == 2)) {
        exp_repeat_push(exp, prev_or_exp, LYXP_EXPR_OR);
        ++(*tok_idx);

reparse_equality_expr:
        /* EqualityExpr */
        prev_and_exp = *tok_idx;
        rc = reparse_equality_expr(ctx, exp, tok_idx);
        LY_CHECK_RET(rc);

        /* ('and' EqualityExpr)* */
        while (!lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_OPER_LOG) && (exp->tok_len[*tok_idx] == 3)) {
            exp_repeat_push(exp, prev_and_exp, LYXP_EXPR_AND);
            ++(*tok_idx);

            rc = reparse_equality_expr(ctx, exp, tok_idx);
            LY_CHECK_RET(rc);
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse NCName.
 *
 * @param[in] ncname Name to parse.
 * @return Length of @p ncname valid bytes.
 */
static long int
parse_ncname(const char *ncname)
{
    uint32_t uc;
    size_t size;
    long int len = 0;

    LY_CHECK_RET(ly_getutf8(&ncname, &uc, &size), 0);
    if (!is_xmlqnamestartchar(uc) || (uc == ':')) {
        return len;
    }

    do {
        len += size;
        if (!*ncname) {
            break;
        }
        LY_CHECK_RET(ly_getutf8(&ncname, &uc, &size), -len);
    } while (is_xmlqnamechar(uc) && (uc != ':'));

    return len;
}

/**
 * @brief Add @p token into the expression @p exp.
 *
 * @param[in] ctx Context for logging.
 * @param[in] exp Expression to use.
 * @param[in] token Token to add.
 * @param[in] tok_pos Token position in the XPath expression.
 * @param[in] tok_len Token length in the XPath expression.
 * @return LY_ERR
 */
static LY_ERR
exp_add_token(const struct ly_ctx *ctx, struct lyxp_expr *exp, enum lyxp_token token, uint16_t tok_pos, uint16_t tok_len)
{
    uint32_t prev;

    if (exp->used == exp->size) {
        prev = exp->size;
        exp->size += LYXP_EXPR_SIZE_STEP;
        if (prev > exp->size) {
            LOGINT(ctx);
            return LY_EINT;
        }

        exp->tokens = ly_realloc(exp->tokens, exp->size * sizeof *exp->tokens);
        LY_CHECK_ERR_RET(!exp->tokens, LOGMEM(ctx), LY_EMEM);
        exp->tok_pos = ly_realloc(exp->tok_pos, exp->size * sizeof *exp->tok_pos);
        LY_CHECK_ERR_RET(!exp->tok_pos, LOGMEM(ctx), LY_EMEM);
        exp->tok_len = ly_realloc(exp->tok_len, exp->size * sizeof *exp->tok_len);
        LY_CHECK_ERR_RET(!exp->tok_len, LOGMEM(ctx), LY_EMEM);
    }

    exp->tokens[exp->used] = token;
    exp->tok_pos[exp->used] = tok_pos;
    exp->tok_len[exp->used] = tok_len;
    ++exp->used;
    return LY_SUCCESS;
}

void
lyxp_expr_free(const struct ly_ctx *ctx, struct lyxp_expr *expr)
{
    uint16_t i;

    if (!expr) {
        return;
    }

    lydict_remove(ctx, expr->expr);
    free(expr->tokens);
    free(expr->tok_pos);
    free(expr->tok_len);
    if (expr->repeat) {
        for (i = 0; i < expr->used; ++i) {
            free(expr->repeat[i]);
        }
    }
    free(expr->repeat);
    free(expr);
}

LY_ERR
lyxp_expr_parse(const struct ly_ctx *ctx, const char *expr_str, size_t expr_len, ly_bool reparse, struct lyxp_expr **expr_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_expr *expr;
    size_t parsed = 0, tok_len;
    enum lyxp_token tok_type;
    ly_bool prev_function_check = 0;
    uint16_t tok_idx = 0;

    assert(expr_p);

    if (!expr_str[0]) {
        LOGVAL(ctx, LY_VCODE_XP_EOF);
        return LY_EVALID;
    }

    if (!expr_len) {
        expr_len = strlen(expr_str);
    }
    if (expr_len > UINT16_MAX) {
        LOGVAL(ctx, LYVE_XPATH, "XPath expression cannot be longer than %u characters.", UINT16_MAX);
        return LY_EVALID;
    }

    /* init lyxp_expr structure */
    expr = calloc(1, sizeof *expr);
    LY_CHECK_ERR_GOTO(!expr, LOGMEM(ctx); ret = LY_EMEM, error);
    LY_CHECK_GOTO(ret = lydict_insert(ctx, expr_str, expr_len, &expr->expr), error);
    expr->used = 0;
    expr->size = LYXP_EXPR_SIZE_START;
    expr->tokens = malloc(expr->size * sizeof *expr->tokens);
    LY_CHECK_ERR_GOTO(!expr->tokens, LOGMEM(ctx); ret = LY_EMEM, error);

    expr->tok_pos = malloc(expr->size * sizeof *expr->tok_pos);
    LY_CHECK_ERR_GOTO(!expr->tok_pos, LOGMEM(ctx); ret = LY_EMEM, error);

    expr->tok_len = malloc(expr->size * sizeof *expr->tok_len);
    LY_CHECK_ERR_GOTO(!expr->tok_len, LOGMEM(ctx); ret = LY_EMEM, error);

    /* make expr 0-terminated */
    expr_str = expr->expr;

    while (is_xmlws(expr_str[parsed])) {
        ++parsed;
    }

    do {
        if (expr_str[parsed] == '(') {

            /* '(' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_PAR1;

            if (prev_function_check && expr->used && (expr->tokens[expr->used - 1] == LYXP_TOKEN_NAMETEST)) {
                /* it is a NodeType/FunctionName after all */
                if (((expr->tok_len[expr->used - 1] == 4) &&
                        (!strncmp(&expr_str[expr->tok_pos[expr->used - 1]], "node", 4) ||
                        !strncmp(&expr_str[expr->tok_pos[expr->used - 1]], "text", 4))) ||
                        ((expr->tok_len[expr->used - 1] == 7) &&
                        !strncmp(&expr_str[expr->tok_pos[expr->used - 1]], "comment", 7))) {
                    expr->tokens[expr->used - 1] = LYXP_TOKEN_NODETYPE;
                } else {
                    expr->tokens[expr->used - 1] = LYXP_TOKEN_FUNCNAME;
                }
                prev_function_check = 0;
            }

        } else if (expr_str[parsed] == ')') {

            /* ')' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_PAR2;

        } else if (expr_str[parsed] == '[') {

            /* '[' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_BRACK1;

        } else if (expr_str[parsed] == ']') {

            /* ']' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_BRACK2;

        } else if (!strncmp(&expr_str[parsed], "..", 2)) {

            /* '..' */
            tok_len = 2;
            tok_type = LYXP_TOKEN_DDOT;

        } else if ((expr_str[parsed] == '.') && (!isdigit(expr_str[parsed + 1]))) {

            /* '.' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_DOT;

        } else if (expr_str[parsed] == '@') {

            /* '@' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_AT;

        } else if (expr_str[parsed] == ',') {

            /* ',' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_COMMA;

        } else if (expr_str[parsed] == '\'') {

            /* Literal with ' */
            for (tok_len = 1; (expr_str[parsed + tok_len] != '\0') && (expr_str[parsed + tok_len] != '\''); ++tok_len) {}
            LY_CHECK_ERR_GOTO(expr_str[parsed + tok_len] == '\0',
                    LOGVAL(ctx, LY_VCODE_XP_EOE, expr_str[parsed], &expr_str[parsed]); ret = LY_EVALID,
                    error);
            ++tok_len;
            tok_type = LYXP_TOKEN_LITERAL;

        } else if (expr_str[parsed] == '\"') {

            /* Literal with " */
            for (tok_len = 1; (expr_str[parsed + tok_len] != '\0') && (expr_str[parsed + tok_len] != '\"'); ++tok_len) {}
            LY_CHECK_ERR_GOTO(expr_str[parsed + tok_len] == '\0',
                    LOGVAL(ctx, LY_VCODE_XP_EOE, expr_str[parsed], &expr_str[parsed]); ret = LY_EVALID,
                    error);
            ++tok_len;
            tok_type = LYXP_TOKEN_LITERAL;

        } else if ((expr_str[parsed] == '.') || (isdigit(expr_str[parsed]))) {

            /* Number */
            for (tok_len = 0; isdigit(expr_str[parsed + tok_len]); ++tok_len) {}
            if (expr_str[parsed + tok_len] == '.') {
                ++tok_len;
                for ( ; isdigit(expr_str[parsed + tok_len]); ++tok_len) {}
            }
            tok_type = LYXP_TOKEN_NUMBER;

        } else if (expr_str[parsed] == '/') {

            /* Operator '/', '//' */
            if (!strncmp(&expr_str[parsed], "//", 2)) {
                tok_len = 2;
                tok_type = LYXP_TOKEN_OPER_RPATH;
            } else {
                tok_len = 1;
                tok_type = LYXP_TOKEN_OPER_PATH;
            }

        } else if (!strncmp(&expr_str[parsed], "!=", 2)) {

            /* Operator '!=' */
            tok_len = 2;
            tok_type = LYXP_TOKEN_OPER_NEQUAL;

        } else if (!strncmp(&expr_str[parsed], "<=", 2) || !strncmp(&expr_str[parsed], ">=", 2)) {

            /* Operator '<=', '>=' */
            tok_len = 2;
            tok_type = LYXP_TOKEN_OPER_COMP;

        } else if (expr_str[parsed] == '|') {

            /* Operator '|' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_OPER_UNI;

        } else if ((expr_str[parsed] == '+') || (expr_str[parsed] == '-')) {

            /* Operator '+', '-' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_OPER_MATH;

        } else if (expr_str[parsed] == '=') {

            /* Operator '=' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_OPER_EQUAL;

        } else if ((expr_str[parsed] == '<') || (expr_str[parsed] == '>')) {

            /* Operator '<', '>' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_OPER_COMP;

        } else if (expr->used && (expr->tokens[expr->used - 1] != LYXP_TOKEN_AT) &&
                (expr->tokens[expr->used - 1] != LYXP_TOKEN_PAR1) &&
                (expr->tokens[expr->used - 1] != LYXP_TOKEN_BRACK1) &&
                (expr->tokens[expr->used - 1] != LYXP_TOKEN_COMMA) &&
                (expr->tokens[expr->used - 1] != LYXP_TOKEN_OPER_LOG) &&
                (expr->tokens[expr->used - 1] != LYXP_TOKEN_OPER_EQUAL) &&
                (expr->tokens[expr->used - 1] != LYXP_TOKEN_OPER_NEQUAL) &&
                (expr->tokens[expr->used - 1] != LYXP_TOKEN_OPER_COMP) &&
                (expr->tokens[expr->used - 1] != LYXP_TOKEN_OPER_MATH) &&
                (expr->tokens[expr->used - 1] != LYXP_TOKEN_OPER_UNI) &&
                (expr->tokens[expr->used - 1] != LYXP_TOKEN_OPER_PATH) &&
                (expr->tokens[expr->used - 1] != LYXP_TOKEN_OPER_RPATH)) {

            /* Operator '*', 'or', 'and', 'mod', or 'div' */
            if (expr_str[parsed] == '*') {
                tok_len = 1;
                tok_type = LYXP_TOKEN_OPER_MATH;

            } else if (!strncmp(&expr_str[parsed], "or", 2)) {
                tok_len = 2;
                tok_type = LYXP_TOKEN_OPER_LOG;

            } else if (!strncmp(&expr_str[parsed], "and", 3)) {
                tok_len = 3;
                tok_type = LYXP_TOKEN_OPER_LOG;

            } else if (!strncmp(&expr_str[parsed], "mod", 3) || !strncmp(&expr_str[parsed], "div", 3)) {
                tok_len = 3;
                tok_type = LYXP_TOKEN_OPER_MATH;

            } else if (prev_function_check) {
                LOGVAL(ctx, LYVE_XPATH, "Invalid character 0x%x ('%c'), perhaps \"%.*s\" is supposed to be a function call.",
                        expr_str[parsed], expr_str[parsed], expr->tok_len[expr->used - 1], &expr->expr[expr->tok_pos[expr->used - 1]]);
                ret = LY_EVALID;
                goto error;
            } else {
                LOGVAL(ctx, LY_VCODE_XP_INEXPR, expr_str[parsed], parsed + 1, expr_str);
                ret = LY_EVALID;
                goto error;
            }
        } else if (expr_str[parsed] == '*') {

            /* NameTest '*' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_NAMETEST;

        } else {

            /* NameTest (NCName ':' '*' | QName) or NodeType/FunctionName */
            long int ncname_len = parse_ncname(&expr_str[parsed]);
            LY_CHECK_ERR_GOTO(ncname_len < 0, LOGVAL(ctx, LY_VCODE_XP_INEXPR, expr_str[parsed - ncname_len],
                    parsed - ncname_len + 1, expr_str); ret = LY_EVALID, error);
            tok_len = ncname_len;

            if (expr_str[parsed + tok_len] == ':') {
                ++tok_len;
                if (expr_str[parsed + tok_len] == '*') {
                    ++tok_len;
                } else {
                    ncname_len = parse_ncname(&expr_str[parsed + tok_len]);
                    LY_CHECK_ERR_GOTO(ncname_len < 0, LOGVAL(ctx, LY_VCODE_XP_INEXPR, expr_str[parsed - ncname_len],
                            parsed - ncname_len + 1, expr_str); ret = LY_EVALID, error);
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
        LY_CHECK_GOTO(ret = exp_add_token(ctx, expr, tok_type, parsed, tok_len), error);
        parsed += tok_len;
        while (is_xmlws(expr_str[parsed])) {
            ++parsed;
        }

    } while (expr_str[parsed]);

    if (reparse) {
        /* prealloc repeat */
        expr->repeat = calloc(expr->size, sizeof *expr->repeat);
        LY_CHECK_ERR_GOTO(!expr->repeat, LOGMEM(ctx); ret = LY_EMEM, error);

        /* fill repeat */
        LY_CHECK_ERR_GOTO(reparse_or_expr(ctx, expr, &tok_idx), ret = LY_EVALID, error);
        if (expr->used > tok_idx) {
            LOGVAL(ctx, LYVE_XPATH, "Unparsed characters \"%s\" left at the end of an XPath expression.",
                    &expr->expr[expr->tok_pos[tok_idx]]);
            ret = LY_EVALID;
            goto error;
        }
    }

    print_expr_struct_debug(expr);
    *expr_p = expr;
    return LY_SUCCESS;

error:
    lyxp_expr_free(ctx, expr);
    return ret;
}

LY_ERR
lyxp_expr_dup(const struct ly_ctx *ctx, const struct lyxp_expr *exp, struct lyxp_expr **dup_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_expr *dup = NULL;
    uint32_t i, j;

    if (!exp) {
        goto cleanup;
    }

    dup = calloc(1, sizeof *dup);
    LY_CHECK_ERR_GOTO(!dup, LOGMEM(ctx); ret = LY_EMEM, cleanup);

    dup->tokens = malloc(exp->used * sizeof *dup->tokens);
    LY_CHECK_ERR_GOTO(!dup->tokens, LOGMEM(ctx); ret = LY_EMEM, cleanup);
    memcpy(dup->tokens, exp->tokens, exp->used * sizeof *dup->tokens);

    dup->tok_pos = malloc(exp->used * sizeof *dup->tok_pos);
    LY_CHECK_ERR_GOTO(!dup->tok_pos, LOGMEM(ctx); ret = LY_EMEM, cleanup);
    memcpy(dup->tok_pos, exp->tok_pos, exp->used * sizeof *dup->tok_pos);

    dup->tok_len = malloc(exp->used * sizeof *dup->tok_len);
    LY_CHECK_ERR_GOTO(!dup->tok_len, LOGMEM(ctx); ret = LY_EMEM, cleanup);
    memcpy(dup->tok_len, exp->tok_len, exp->used * sizeof *dup->tok_len);

    dup->repeat = malloc(exp->used * sizeof *dup->repeat);
    LY_CHECK_ERR_GOTO(!dup->repeat, LOGMEM(ctx); ret = LY_EMEM, cleanup);
    for (i = 0; i < exp->used; ++i) {
        if (!exp->repeat[i]) {
            dup->repeat[i] = NULL;
        } else {
            for (j = 0; exp->repeat[i][j]; ++j) {}
            /* the ending 0 as well */
            ++j;

            dup->repeat[i] = malloc(j * sizeof **dup->repeat);
            LY_CHECK_ERR_GOTO(!dup->repeat[i], LOGMEM(ctx); ret = LY_EMEM, cleanup);
            memcpy(dup->repeat[i], exp->repeat[i], j * sizeof **dup->repeat);
            dup->repeat[i][j - 1] = 0;
        }
    }

    dup->used = exp->used;
    dup->size = exp->used;
    LY_CHECK_GOTO(ret = lydict_insert(ctx, exp->expr, 0, &dup->expr), cleanup);

cleanup:
    if (ret) {
        lyxp_expr_free(ctx, dup);
    } else {
        *dup_p = dup;
    }
    return ret;
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
 * @return Last-added schema context node, NULL if no node is in context.
 */
static struct lysc_node *
warn_get_scnode_in_ctx(struct lyxp_set *set)
{
    uint32_t i;

    if (!set || (set->type != LYXP_SET_SCNODE_SET)) {
        return NULL;
    }

    i = set->used;
    do {
        --i;
        if (set->val.scnodes[i].in_ctx == LYXP_SET_SCNODE_ATOM_CTX) {
            /* if there are more, simply return the first found (last added) */
            return set->val.scnodes[i].scnode;
        }
    } while (i);

    return NULL;
}

/**
 * @brief Test whether a type is numeric - integer type or decimal64.
 *
 * @param[in] type Type to test.
 * @return Boolean value whether @p type is numeric type or not.
 */
static ly_bool
warn_is_numeric_type(struct lysc_type *type)
{
    struct lysc_type_union *uni;
    ly_bool ret;
    LY_ARRAY_COUNT_TYPE u;

    switch (type->basetype) {
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
        uni = (struct lysc_type_union *)type;
        LY_ARRAY_FOR(uni->types, u) {
            ret = warn_is_numeric_type(uni->types[u]);
            if (ret) {
                /* found a suitable type */
                return ret;
            }
        }
        /* did not find any suitable type */
        return 0;
    case LY_TYPE_LEAFREF:
        return warn_is_numeric_type(((struct lysc_type_leafref *)type)->realtype);
    default:
        return 0;
    }
}

/**
 * @brief Test whether a type is string-like - no integers, decimal64 or binary.
 *
 * @param[in] type Type to test.
 * @return Boolean value whether @p type's basetype is string type or not.
 */
static ly_bool
warn_is_string_type(struct lysc_type *type)
{
    struct lysc_type_union *uni;
    ly_bool ret;
    LY_ARRAY_COUNT_TYPE u;

    switch (type->basetype) {
    case LY_TYPE_BITS:
    case LY_TYPE_ENUM:
    case LY_TYPE_IDENT:
    case LY_TYPE_INST:
    case LY_TYPE_STRING:
        return 1;
    case LY_TYPE_UNION:
        uni = (struct lysc_type_union *)type;
        LY_ARRAY_FOR(uni->types, u) {
            ret = warn_is_string_type(uni->types[u]);
            if (ret) {
                /* found a suitable type */
                return ret;
            }
        }
        /* did not find any suitable type */
        return 0;
    case LY_TYPE_LEAFREF:
        return warn_is_string_type(((struct lysc_type_leafref *)type)->realtype);
    default:
        return 0;
    }
}

/**
 * @brief Test whether a type is one specific type.
 *
 * @param[in] type Type to test.
 * @param[in] base Expected type.
 * @return Boolean value whether the given @p type is of the specific basetype @p base.
 */
static ly_bool
warn_is_specific_type(struct lysc_type *type, LY_DATA_TYPE base)
{
    struct lysc_type_union *uni;
    ly_bool ret;
    LY_ARRAY_COUNT_TYPE u;

    if (type->basetype == base) {
        return 1;
    } else if (type->basetype == LY_TYPE_UNION) {
        uni = (struct lysc_type_union *)type;
        LY_ARRAY_FOR(uni->types, u) {
            ret = warn_is_specific_type(uni->types[u], base);
            if (ret) {
                /* found a suitable type */
                return ret;
            }
        }
        /* did not find any suitable type */
        return 0;
    } else if (type->basetype == LY_TYPE_LEAFREF) {
        return warn_is_specific_type(((struct lysc_type_leafref *)type)->realtype, base);
    }

    return 0;
}

/**
 * @brief Get next type of a (union) type.
 *
 * @param[in] type Base type.
 * @param[in] prev_type Previously returned type.
 * @return Next type or NULL.
 */
static struct lysc_type *
warn_is_equal_type_next_type(struct lysc_type *type, struct lysc_type *prev_type)
{
    struct lysc_type_union *uni;
    ly_bool found = 0;
    LY_ARRAY_COUNT_TYPE u;

    switch (type->basetype) {
    case LY_TYPE_UNION:
        uni = (struct lysc_type_union *)type;
        if (!prev_type) {
            return uni->types[0];
        }
        LY_ARRAY_FOR(uni->types, u) {
            if (found) {
                return uni->types[u];
            }
            if (prev_type == uni->types[u]) {
                found = 1;
            }
        }
        return NULL;
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
 * @param[in] type1 First type.
 * @param[in] type2 Second type.
 * @return 1 if they do, 0 otherwise.
 */
static int
warn_is_equal_type(struct lysc_type *type1, struct lysc_type *type2)
{
    struct lysc_type *t1, *rt1, *t2, *rt2;

    t1 = NULL;
    while ((t1 = warn_is_equal_type_next_type(type1, t1))) {
        if (t1->basetype == LY_TYPE_LEAFREF) {
            rt1 = ((struct lysc_type_leafref *)t1)->realtype;
        } else {
            rt1 = t1;
        }

        t2 = NULL;
        while ((t2 = warn_is_equal_type_next_type(type2, t2))) {
            if (t2->basetype == LY_TYPE_LEAFREF) {
                rt2 = ((struct lysc_type_leafref *)t2)->realtype;
            } else {
                rt2 = t2;
            }

            if (rt2->basetype == rt1->basetype) {
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
 * @param[in] tok_pos Token position.
 */
static void
warn_operands(struct ly_ctx *ctx, struct lyxp_set *set1, struct lyxp_set *set2, ly_bool numbers_only, const char *expr, uint16_t tok_pos)
{
    struct lysc_node_leaf *node1, *node2;
    ly_bool leaves = 1, warning = 0;

    node1 = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(set1);
    node2 = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(set2);

    if (!node1 && !node2) {
        /* no node-sets involved, nothing to do */
        return;
    }

    if (node1) {
        if (!(node1->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            LOGWRN(ctx, "Node type %s \"%s\" used as operand.", lys_nodetype2str(node1->nodetype), node1->name);
            warning = 1;
            leaves = 0;
        } else if (numbers_only && !warn_is_numeric_type(node1->type)) {
            LOGWRN(ctx, "Node \"%s\" is not of a numeric type, but used where it was expected.", node1->name);
            warning = 1;
        }
    }

    if (node2) {
        if (!(node2->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            LOGWRN(ctx, "Node type %s \"%s\" used as operand.", lys_nodetype2str(node2->nodetype), node2->name);
            warning = 1;
            leaves = 0;
        } else if (numbers_only && !warn_is_numeric_type(node2->type)) {
            LOGWRN(ctx, "Node \"%s\" is not of a numeric type, but used where it was expected.", node2->name);
            warning = 1;
        }
    }

    if (node1 && node2 && leaves && !numbers_only) {
        if ((warn_is_numeric_type(node1->type) && !warn_is_numeric_type(node2->type)) ||
                (!warn_is_numeric_type(node1->type) && warn_is_numeric_type(node2->type)) ||
                (!warn_is_numeric_type(node1->type) && !warn_is_numeric_type(node2->type) &&
                !warn_is_equal_type(node1->type, node2->type))) {
            LOGWRN(ctx, "Incompatible types of operands \"%s\" and \"%s\" for comparison.", node1->name, node2->name);
            warning = 1;
        }
    }

    if (warning) {
        LOGWRN(ctx, "Previous warning generated by XPath subexpression[%u] \"%.20s\".", tok_pos, expr + tok_pos);
    }
}

/**
 * @brief Check that a value is valid for a leaf. If not applicable, does nothing.
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] set Set with the leaf/leaf-list.
 * @param[in] val_exp Index of the value (literal/number) in @p exp.
 * @param[in] equal_exp Index of the start of the equality expression in @p exp.
 * @param[in] last_equal_exp Index of the end of the equality expression in @p exp.
 */
static void
warn_equality_value(const struct lyxp_expr *exp, struct lyxp_set *set, uint16_t val_exp, uint16_t equal_exp,
        uint16_t last_equal_exp)
{
    struct lysc_node *scnode;
    struct lysc_type *type;
    char *value;
    struct lyd_value storage;
    LY_ERR rc;
    struct ly_err_item *err = NULL;

    if ((scnode = warn_get_scnode_in_ctx(set)) && (scnode->nodetype & (LYS_LEAF | LYS_LEAFLIST)) &&
            ((exp->tokens[val_exp] == LYXP_TOKEN_LITERAL) || (exp->tokens[val_exp] == LYXP_TOKEN_NUMBER))) {
        /* check that the node can have the specified value */
        if (exp->tokens[val_exp] == LYXP_TOKEN_LITERAL) {
            value = strndup(exp->expr + exp->tok_pos[val_exp] + 1, exp->tok_len[val_exp] - 2);
        } else {
            value = strndup(exp->expr + exp->tok_pos[val_exp], exp->tok_len[val_exp]);
        }
        if (!value) {
            LOGMEM(set->ctx);
            return;
        }

        if ((((struct lysc_node_leaf *)scnode)->type->basetype == LY_TYPE_IDENT) && !strchr(value, ':')) {
            LOGWRN(set->ctx, "Identityref \"%s\" comparison with identity \"%s\" without prefix, consider adding"
                    " a prefix or best using \"derived-from(-or-self)()\" functions.", scnode->name, value);
            LOGWRN(set->ctx, "Previous warning generated by XPath subexpression[%u] \"%.*s\".", exp->tok_pos[equal_exp],
                    (exp->tok_pos[last_equal_exp] - exp->tok_pos[equal_exp]) + exp->tok_len[last_equal_exp],
                    exp->expr + exp->tok_pos[equal_exp]);
        }

        type = ((struct lysc_node_leaf *)scnode)->type;
        if (type->basetype != LY_TYPE_IDENT) {
            rc = type->plugin->store(set->ctx, type, value, strlen(value), 0, set->format, set->prefix_data,
                    LYD_HINT_DATA, scnode, &storage, NULL, &err);
            if (rc == LY_EINCOMPLETE) {
                rc = LY_SUCCESS;
            }

            if (err) {
                LOGWRN(set->ctx, "Invalid value \"%s\" which does not fit the type (%s).", value, err->msg);
                ly_err_free(err);
            } else if (rc != LY_SUCCESS) {
                LOGWRN(set->ctx, "Invalid value \"%s\" which does not fit the type.", value);
            }
            if (rc != LY_SUCCESS) {
                LOGWRN(set->ctx, "Previous warning generated by XPath subexpression[%u] \"%.*s\".", exp->tok_pos[equal_exp],
                        (exp->tok_pos[last_equal_exp] - exp->tok_pos[equal_exp]) + exp->tok_len[last_equal_exp],
                        exp->expr + exp->tok_pos[equal_exp]);
            } else {
                type->plugin->free(set->ctx, &storage);
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
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_bit_is_set(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    struct lyd_node_term *leaf;
    struct lysc_node_leaf *sleaf;
    struct lysc_type_bits *bits;
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;

    if (options & LYXP_SCNODE_ALL) {
        if (args[0]->type != LYXP_SET_SCNODE_SET) {
            LOGWRN(set->ctx, "Argument #1 of %s not a node-set as expected.", __func__);
        } else if ((sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype),
                        sleaf->name);
            } else if (!warn_is_specific_type(sleaf->type, LY_TYPE_BITS)) {
                LOGWRN(set->ctx, "Argument #1 of %s is node \"%s\", not of type \"bits\".", __func__, sleaf->name);
            }
        }

        if ((args[1]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype),
                        sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #2 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return rc;
    }

    if (args[0]->type != LYXP_SET_NODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INARGTYPE, 1, print_set_type(args[0]), "bit-is-set(node-set, string)");
        return LY_EVALID;
    }
    rc = lyxp_set_cast(args[1], LYXP_SET_STRING);
    LY_CHECK_RET(rc);

    set_fill_boolean(set, 0);
    if (args[0]->used) {
        leaf = (struct lyd_node_term *)args[0]->val.nodes[0].node;
        if ((leaf->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) &&
                (((struct lysc_node_leaf *)leaf->schema)->type->basetype == LY_TYPE_BITS)) {
            bits = (struct lysc_type_bits *)((struct lysc_node_leaf *)leaf->schema)->type;
            LY_ARRAY_FOR(bits->bits, u) {
                if (!strcmp(bits->bits[u].name, args[1]->val.str)) {
                    set_fill_boolean(set, 1);
                    break;
                }
            }
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath boolean(object) function. Returns LYXP_SET_BOOLEAN
 *        with the argument converted to boolean.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_boolean(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    LY_ERR rc;

    if (options & LYXP_SCNODE_ALL) {
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_NODE);
        return LY_SUCCESS;
    }

    rc = lyxp_set_cast(args[0], LYXP_SET_BOOLEAN);
    LY_CHECK_RET(rc);
    set_fill_set(set, args[0]);

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath ceiling(number) function. Returns LYXP_SET_NUMBER
 *        with the first argument rounded up to the nearest integer.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_ceiling(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    struct lysc_node_leaf *sleaf;
    LY_ERR rc = LY_SUCCESS;

    if (options & LYXP_SCNODE_ALL) {
        if (args[0]->type != LYXP_SET_SCNODE_SET) {
            LOGWRN(set->ctx, "Argument #1 of %s not a node-set as expected.", __func__);
        } else if ((sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype),
                        sleaf->name);
            } else if (!warn_is_specific_type(sleaf->type, LY_TYPE_DEC64)) {
                LOGWRN(set->ctx, "Argument #1 of %s is node \"%s\", not of type \"decimal64\".", __func__, sleaf->name);
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return rc;
    }

    rc = lyxp_set_cast(args[0], LYXP_SET_NUMBER);
    LY_CHECK_RET(rc);
    if ((long long)args[0]->val.num != args[0]->val.num) {
        set_fill_number(set, ((long long)args[0]->val.num) + 1);
    } else {
        set_fill_number(set, args[0]->val.num);
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath concat(string, string, string*) function.
 *        Returns LYXP_SET_STRING with the concatenation of all the arguments.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_concat(struct lyxp_set **args, uint16_t arg_count, struct lyxp_set *set, uint32_t options)
{
    uint16_t i;
    char *str = NULL;
    size_t used = 1;
    LY_ERR rc = LY_SUCCESS;
    struct lysc_node_leaf *sleaf;

    if (options & LYXP_SCNODE_ALL) {
        for (i = 0; i < arg_count; ++i) {
            if ((args[i]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[i]))) {
                if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                    LOGWRN(set->ctx, "Argument #%u of %s is a %s node \"%s\".",
                            i + 1, __func__, lys_nodetype2str(sleaf->nodetype), sleaf->name);
                } else if (!warn_is_string_type(sleaf->type)) {
                    LOGWRN(set->ctx, "Argument #%u of %s is node \"%s\", not of string-type.", i + 1, __func__, sleaf->name);
                }
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return rc;
    }

    for (i = 0; i < arg_count; ++i) {
        rc = lyxp_set_cast(args[i], LYXP_SET_STRING);
        if (rc != LY_SUCCESS) {
            free(str);
            return rc;
        }

        str = ly_realloc(str, (used + strlen(args[i]->val.str)) * sizeof(char));
        LY_CHECK_ERR_RET(!str, LOGMEM(set->ctx), LY_EMEM);
        strcpy(str + used - 1, args[i]->val.str);
        used += strlen(args[i]->val.str);
    }

    /* free, kind of */
    lyxp_set_free_content(set);
    set->type = LYXP_SET_STRING;
    set->val.str = str;

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath contains(string, string) function.
 *        Returns LYXP_SET_BOOLEAN whether the second argument can
 *        be found in the first or not.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_contains(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    struct lysc_node_leaf *sleaf;
    LY_ERR rc = LY_SUCCESS;

    if (options & LYXP_SCNODE_ALL) {
        if ((args[0]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype),
                        sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }

        if ((args[1]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype),
                        sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #2 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return rc;
    }

    rc = lyxp_set_cast(args[0], LYXP_SET_STRING);
    LY_CHECK_RET(rc);
    rc = lyxp_set_cast(args[1], LYXP_SET_STRING);
    LY_CHECK_RET(rc);

    if (strstr(args[0]->val.str, args[1]->val.str)) {
        set_fill_boolean(set, 1);
    } else {
        set_fill_boolean(set, 0);
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath count(node-set) function. Returns LYXP_SET_NUMBER
 *        with the size of the node-set from the argument.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_count(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    LY_ERR rc = LY_SUCCESS;

    if (options & LYXP_SCNODE_ALL) {
        if (args[0]->type != LYXP_SET_SCNODE_SET) {
            LOGWRN(set->ctx, "Argument #1 of %s not a node-set as expected.", __func__);
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_NODE);
        return rc;
    }

    if (args[0]->type != LYXP_SET_NODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INARGTYPE, 1, print_set_type(args[0]), "count(node-set)");
        return LY_EVALID;
    }

    set_fill_number(set, args[0]->used);
    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath current() function. Returns LYXP_SET_NODE_SET
 *        with the context with the intial node.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_current(struct lyxp_set **args, uint16_t arg_count, struct lyxp_set *set, uint32_t options)
{
    if (arg_count || args) {
        LOGVAL(set->ctx, LY_VCODE_XP_INARGCOUNT, arg_count, "current()");
        return LY_EVALID;
    }

    if (options & LYXP_SCNODE_ALL) {
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_NODE);

        LY_CHECK_RET(lyxp_set_scnode_insert_node(set, set->cur_scnode, LYXP_NODE_ELEM, NULL));
    } else {
        lyxp_set_free_content(set);

        /* position is filled later */
        set_insert_node(set, set->cur_node, 0, LYXP_NODE_ELEM, 0);
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the YANG 1.1 deref(node-set) function. Returns LYXP_SET_NODE_SET with either
 *        leafref or instance-identifier target node(s).
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_deref(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    struct lyd_node_term *leaf;
    struct lysc_node_leaf *sleaf = NULL;
    struct lysc_type_leafref *lref;
    const struct lysc_node *target;
    struct ly_path *p;
    struct lyd_node *node;
    char *errmsg = NULL;
    uint8_t oper;
    LY_ERR rc = LY_SUCCESS;

    if (options & LYXP_SCNODE_ALL) {
        if (args[0]->type != LYXP_SET_SCNODE_SET) {
            LOGWRN(set->ctx, "Argument #1 of %s not a node-set as expected.", __func__);
        } else if ((sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype),
                        sleaf->name);
            } else if (!warn_is_specific_type(sleaf->type, LY_TYPE_LEAFREF) && !warn_is_specific_type(sleaf->type, LY_TYPE_INST)) {
                LOGWRN(set->ctx, "Argument #1 of %s is node \"%s\", not of type \"leafref\" nor \"instance-identifier\".",
                        __func__, sleaf->name);
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        if (sleaf && (sleaf->type->basetype == LY_TYPE_LEAFREF)) {
            lref = (struct lysc_type_leafref *)sleaf->type;
            oper = (sleaf->flags & LYS_IS_OUTPUT) ? LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT;

            /* it was already evaluated on schema, it must succeed */
            rc = ly_path_compile(set->ctx, lref->cur_mod, &sleaf->node, NULL, lref->path, LY_PATH_LREF_TRUE, oper,
                    LY_PATH_TARGET_MANY, LY_VALUE_SCHEMA_RESOLVED, lref->prefixes, NULL, &p);
            assert(!rc);

            /* get the target node */
            target = p[LY_ARRAY_COUNT(p) - 1].node;
            ly_path_free(set->ctx, p);

            LY_CHECK_RET(lyxp_set_scnode_insert_node(set, target, LYXP_NODE_ELEM, NULL));
        }

        return rc;
    }

    if (args[0]->type != LYXP_SET_NODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INARGTYPE, 1, print_set_type(args[0]), "deref(node-set)");
        return LY_EVALID;
    }

    lyxp_set_free_content(set);
    if (args[0]->used) {
        leaf = (struct lyd_node_term *)args[0]->val.nodes[0].node;
        sleaf = (struct lysc_node_leaf *)leaf->schema;
        if (sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
            if (sleaf->type->basetype == LY_TYPE_LEAFREF) {
                /* find leafref target */
                if (lyplg_type_resolve_leafref((struct lysc_type_leafref *)sleaf->type, &leaf->node, &leaf->value, set->tree,
                        &node, &errmsg)) {
                    LOGERR(set->ctx, LY_EVALID, errmsg);
                    free(errmsg);
                    return LY_EVALID;
                }
            } else {
                assert(sleaf->type->basetype == LY_TYPE_INST);
                if (ly_path_eval(leaf->value.target, set->tree, &node)) {
                    LOGERR(set->ctx, LY_EVALID, "Invalid instance-identifier \"%s\" value - required instance not found.",
                            lyd_get_value(&leaf->node));
                    return LY_EVALID;
                }
            }

            /* insert it */
            set_insert_node(set, node, 0, LYXP_NODE_ELEM, 0);
        }
    }

    return LY_SUCCESS;
}

static LY_ERR
xpath_derived_(struct lyxp_set **args, struct lyxp_set *set, uint32_t options, ly_bool self_match, const char *func)
{
    uint32_t i;
    LY_ARRAY_COUNT_TYPE u;
    struct lyd_node_term *leaf;
    struct lysc_node_leaf *sleaf;
    struct lyd_meta *meta;
    struct lyd_value *val;
    const struct lys_module *mod;
    const char *id_name;
    uint16_t id_len;
    struct lysc_ident *id;
    LY_ERR rc = LY_SUCCESS;
    ly_bool found;

    if (options & LYXP_SCNODE_ALL) {
        if (args[0]->type != LYXP_SET_SCNODE_SET) {
            LOGWRN(set->ctx, "Argument #1 of %s not a node-set as expected.", func);
        } else if ((sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #1 of %s is a %s node \"%s\".", func, lys_nodetype2str(sleaf->nodetype),
                        sleaf->name);
            } else if (!warn_is_specific_type(sleaf->type, LY_TYPE_IDENT)) {
                LOGWRN(set->ctx, "Argument #1 of %s is node \"%s\", not of type \"identityref\".", func, sleaf->name);
            }
        }

        if ((args[1]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #2 of %s is a %s node \"%s\".", func, lys_nodetype2str(sleaf->nodetype),
                        sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #2 of %s is node \"%s\", not of string-type.", func, sleaf->name);
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return rc;
    }

    if (args[0]->type != LYXP_SET_NODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INARGTYPE, 1, print_set_type(args[0]), "derived-from(-or-self)(node-set, string)");
        return LY_EVALID;
    }
    rc = lyxp_set_cast(args[1], LYXP_SET_STRING);
    LY_CHECK_RET(rc);

    /* parse the identity */
    id_name = args[1]->val.str;
    id_len = strlen(id_name);
    rc = moveto_resolve_model(&id_name, &id_len, set, set->cur_node ? set->cur_node->schema : NULL, &mod);
    LY_CHECK_RET(rc);
    if (!mod) {
        LOGVAL(set->ctx, LYVE_XPATH, "Identity \"%.*s\" without a prefix.", (int)id_len, id_name);
        return LY_EVALID;
    }

    /* find the identity */
    found = 0;
    LY_ARRAY_FOR(mod->identities, u) {
        if (!ly_strncmp(mod->identities[u].name, id_name, id_len)) {
            /* we have match */
            found = 1;
            break;
        }
    }
    if (!found) {
        LOGVAL(set->ctx, LYVE_XPATH, "Identity \"%.*s\" not found in module \"%s\".", (int)id_len, id_name, mod->name);
        return LY_EVALID;
    }
    id = &mod->identities[u];

    set_fill_boolean(set, 0);
    found = 0;
    for (i = 0; i < args[0]->used; ++i) {
        if ((args[0]->val.nodes[i].type != LYXP_NODE_ELEM) && (args[0]->val.nodes[i].type != LYXP_NODE_META)) {
            continue;
        }

        if (args[0]->val.nodes[i].type == LYXP_NODE_ELEM) {
            leaf = (struct lyd_node_term *)args[0]->val.nodes[i].node;
            sleaf = (struct lysc_node_leaf *)leaf->schema;
            val = &leaf->value;
            if (!(sleaf->nodetype & LYD_NODE_TERM) || (leaf->value.realtype->basetype != LY_TYPE_IDENT)) {
                /* uninteresting */
                continue;
            }
        } else {
            meta = args[0]->val.meta[i].meta;
            val = &meta->value;
            if (val->realtype->basetype != LY_TYPE_IDENT) {
                /* uninteresting */
                continue;
            }
        }

        /* check the identity itself */
        if (self_match && (id == val->ident)) {
            set_fill_boolean(set, 1);
            found = 1;
        }
        if (!found && !lyplg_type_identity_isderived(id, val->ident)) {
            set_fill_boolean(set, 1);
            found = 1;
        }

        if (found) {
            break;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the YANG 1.1 derived-from(node-set, string) function. Returns LYXP_SET_BOOLEAN depending
 *        on whether the first argument nodes contain a node of an identity derived from the second
 *        argument identity.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_derived_from(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    return xpath_derived_(args, set, options, 0, __func__);
}

/**
 * @brief Execute the YANG 1.1 derived-from-or-self(node-set, string) function. Returns LYXP_SET_BOOLEAN depending
 *        on whether the first argument nodes contain a node of an identity that either is or is derived from
 *        the second argument identity.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_derived_from_or_self(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    return xpath_derived_(args, set, options, 1, __func__);
}

/**
 * @brief Execute the YANG 1.1 enum-value(node-set) function. Returns LYXP_SET_NUMBER
 *        with the integer value of the first node's enum value, otherwise NaN.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_enum_value(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    struct lyd_node_term *leaf;
    struct lysc_node_leaf *sleaf;
    LY_ERR rc = LY_SUCCESS;

    if (options & LYXP_SCNODE_ALL) {
        if (args[0]->type != LYXP_SET_SCNODE_SET) {
            LOGWRN(set->ctx, "Argument #1 of %s not a node-set as expected.", __func__);
        } else if ((sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype),
                        sleaf->name);
            } else if (!warn_is_specific_type(sleaf->type, LY_TYPE_ENUM)) {
                LOGWRN(set->ctx, "Argument #1 of %s is node \"%s\", not of type \"enumeration\".", __func__, sleaf->name);
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return rc;
    }

    if (args[0]->type != LYXP_SET_NODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INARGTYPE, 1, print_set_type(args[0]), "enum-value(node-set)");
        return LY_EVALID;
    }

    set_fill_number(set, NAN);
    if (args[0]->used) {
        leaf = (struct lyd_node_term *)args[0]->val.nodes[0].node;
        sleaf = (struct lysc_node_leaf *)leaf->schema;
        if ((sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST)) && (sleaf->type->basetype == LY_TYPE_ENUM)) {
            set_fill_number(set, leaf->value.enum_item->value);
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath false() function. Returns LYXP_SET_BOOLEAN
 *        with false value.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_false(struct lyxp_set **UNUSED(args), uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    if (options & LYXP_SCNODE_ALL) {
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_NODE);
        return LY_SUCCESS;
    }

    set_fill_boolean(set, 0);
    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath floor(number) function. Returns LYXP_SET_NUMBER
 *        with the first argument floored (truncated).
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_floor(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t UNUSED(options))
{
    LY_ERR rc;

    rc = lyxp_set_cast(args[0], LYXP_SET_NUMBER);
    LY_CHECK_RET(rc);
    if (isfinite(args[0]->val.num)) {
        set_fill_number(set, (long long)args[0]->val.num);
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath lang(string) function. Returns LYXP_SET_BOOLEAN
 *        whether the language of the text matches the one from the argument.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_lang(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    const struct lyd_node *node;
    struct lysc_node_leaf *sleaf;
    struct lyd_meta *meta = NULL;
    const char *val;
    LY_ERR rc = LY_SUCCESS;

    if (options & LYXP_SCNODE_ALL) {
        if ((args[0]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype),
                        sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return rc;
    }

    rc = lyxp_set_cast(args[0], LYXP_SET_STRING);
    LY_CHECK_RET(rc);

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INCTX, print_set_type(set), "lang(string)");
        return LY_EVALID;
    } else if (!set->used) {
        set_fill_boolean(set, 0);
        return LY_SUCCESS;
    }

    switch (set->val.nodes[0].type) {
    case LYXP_NODE_ELEM:
    case LYXP_NODE_TEXT:
        node = set->val.nodes[0].node;
        break;
    case LYXP_NODE_META:
        node = set->val.meta[0].meta->parent;
        break;
    default:
        /* nothing to do with roots */
        set_fill_boolean(set, 0);
        return LY_SUCCESS;
    }

    /* find lang metadata */
    for ( ; node; node = lyd_parent(node)) {
        for (meta = node->meta; meta; meta = meta->next) {
            /* annotations */
            if (meta->name && !strcmp(meta->name, "lang") && !strcmp(meta->annotation->module->name, "xml")) {
                break;
            }
        }

        if (meta) {
            break;
        }
    }

    /* compare languages */
    if (!meta) {
        set_fill_boolean(set, 0);
    } else {
        uint64_t i;

        val = lyd_get_meta_value(meta);
        for (i = 0; args[0]->val.str[i]; ++i) {
            if (tolower(args[0]->val.str[i]) != tolower(val[i])) {
                set_fill_boolean(set, 0);
                break;
            }
        }
        if (!args[0]->val.str[i]) {
            if (!val[i] || (val[i] == '-')) {
                set_fill_boolean(set, 1);
            } else {
                set_fill_boolean(set, 0);
            }
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath last() function. Returns LYXP_SET_NUMBER
 *        with the context size.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_last(struct lyxp_set **UNUSED(args), uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    if (options & LYXP_SCNODE_ALL) {
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_NODE);
        return LY_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INCTX, print_set_type(set), "last()");
        return LY_EVALID;
    } else if (!set->used) {
        set_fill_number(set, 0);
        return LY_SUCCESS;
    }

    set_fill_number(set, set->ctx_size);
    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath local-name(node-set?) function. Returns LYXP_SET_STRING
 *        with the node name without namespace from the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_local_name(struct lyxp_set **args, uint16_t arg_count, struct lyxp_set *set, uint32_t options)
{
    struct lyxp_set_node *item;

    /* suppress unused variable warning */
    (void)options;

    if (options & LYXP_SCNODE_ALL) {
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_NODE);
        return LY_SUCCESS;
    }

    if (arg_count) {
        if (args[0]->type != LYXP_SET_NODE_SET) {
            LOGVAL(set->ctx, LY_VCODE_XP_INARGTYPE, 1, print_set_type(args[0]),
                    "local-name(node-set?)");
            return LY_EVALID;
        } else if (!args[0]->used) {
            set_fill_string(set, "", 0);
            return LY_SUCCESS;
        }

        /* we need the set sorted, it affects the result */
        assert(!set_sort(args[0]));

        item = &args[0]->val.nodes[0];
    } else {
        if (set->type != LYXP_SET_NODE_SET) {
            LOGVAL(set->ctx, LY_VCODE_XP_INCTX, print_set_type(set), "local-name(node-set?)");
            return LY_EVALID;
        } else if (!set->used) {
            set_fill_string(set, "", 0);
            return LY_SUCCESS;
        }

        /* we need the set sorted, it affects the result */
        assert(!set_sort(set));

        item = &set->val.nodes[0];
    }

    switch (item->type) {
    case LYXP_NODE_NONE:
        LOGINT_RET(set->ctx);
    case LYXP_NODE_ROOT:
    case LYXP_NODE_ROOT_CONFIG:
    case LYXP_NODE_TEXT:
        set_fill_string(set, "", 0);
        break;
    case LYXP_NODE_ELEM:
        set_fill_string(set, item->node->schema->name, strlen(item->node->schema->name));
        break;
    case LYXP_NODE_META:
        set_fill_string(set, ((struct lyd_meta *)item->node)->name, strlen(((struct lyd_meta *)item->node)->name));
        break;
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath name(node-set?) function. Returns LYXP_SET_STRING
 *        with the node name fully qualified (with namespace) from the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_name(struct lyxp_set **args, uint16_t arg_count, struct lyxp_set *set, uint32_t options)
{
    struct lyxp_set_node *item;
    struct lys_module *mod = NULL;
    char *str;
    const char *name = NULL;

    if (options & LYXP_SCNODE_ALL) {
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_NODE);
        return LY_SUCCESS;
    }

    if (arg_count) {
        if (args[0]->type != LYXP_SET_NODE_SET) {
            LOGVAL(set->ctx, LY_VCODE_XP_INARGTYPE, 1, print_set_type(args[0]), "name(node-set?)");
            return LY_EVALID;
        } else if (!args[0]->used) {
            set_fill_string(set, "", 0);
            return LY_SUCCESS;
        }

        /* we need the set sorted, it affects the result */
        assert(!set_sort(args[0]));

        item = &args[0]->val.nodes[0];
    } else {
        if (set->type != LYXP_SET_NODE_SET) {
            LOGVAL(set->ctx, LY_VCODE_XP_INCTX, print_set_type(set), "name(node-set?)");
            return LY_EVALID;
        } else if (!set->used) {
            set_fill_string(set, "", 0);
            return LY_SUCCESS;
        }

        /* we need the set sorted, it affects the result */
        assert(!set_sort(set));

        item = &set->val.nodes[0];
    }

    switch (item->type) {
    case LYXP_NODE_NONE:
        LOGINT_RET(set->ctx);
    case LYXP_NODE_ROOT:
    case LYXP_NODE_ROOT_CONFIG:
    case LYXP_NODE_TEXT:
        /* keep NULL */
        break;
    case LYXP_NODE_ELEM:
        mod = item->node->schema->module;
        name = item->node->schema->name;
        break;
    case LYXP_NODE_META:
        mod = ((struct lyd_meta *)item->node)->annotation->module;
        name = ((struct lyd_meta *)item->node)->name;
        break;
    }

    if (mod && name) {
        int rc = asprintf(&str, "%s:%s", ly_get_prefix(mod, set->format, set->prefix_data), name);
        LY_CHECK_ERR_RET(rc == -1, LOGMEM(set->ctx), LY_EMEM);
        set_fill_string(set, str, strlen(str));
        free(str);
    } else {
        set_fill_string(set, "", 0);
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath namespace-uri(node-set?) function. Returns LYXP_SET_STRING
 *        with the namespace of the node from the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINVAL for wrong arguments on schema)
 */
static LY_ERR
xpath_namespace_uri(struct lyxp_set **args, uint16_t arg_count, struct lyxp_set *set, uint32_t options)
{
    struct lyxp_set_node *item;
    struct lys_module *mod;

    /* suppress unused variable warning */
    (void)options;

    if (options & LYXP_SCNODE_ALL) {
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return LY_SUCCESS;
    }

    if (arg_count) {
        if (args[0]->type != LYXP_SET_NODE_SET) {
            LOGVAL(set->ctx, LY_VCODE_XP_INARGTYPE, 1, print_set_type(args[0]),
                    "namespace-uri(node-set?)");
            return LY_EVALID;
        } else if (!args[0]->used) {
            set_fill_string(set, "", 0);
            return LY_SUCCESS;
        }

        /* we need the set sorted, it affects the result */
        assert(!set_sort(args[0]));

        item = &args[0]->val.nodes[0];
    } else {
        if (set->type != LYXP_SET_NODE_SET) {
            LOGVAL(set->ctx, LY_VCODE_XP_INCTX, print_set_type(set), "namespace-uri(node-set?)");
            return LY_EVALID;
        } else if (!set->used) {
            set_fill_string(set, "", 0);
            return LY_SUCCESS;
        }

        /* we need the set sorted, it affects the result */
        assert(!set_sort(set));

        item = &set->val.nodes[0];
    }

    switch (item->type) {
    case LYXP_NODE_NONE:
        LOGINT_RET(set->ctx);
    case LYXP_NODE_ROOT:
    case LYXP_NODE_ROOT_CONFIG:
    case LYXP_NODE_TEXT:
        set_fill_string(set, "", 0);
        break;
    case LYXP_NODE_ELEM:
    case LYXP_NODE_META:
        if (item->type == LYXP_NODE_ELEM) {
            mod = item->node->schema->module;
        } else { /* LYXP_NODE_META */
            /* annotations */
            mod = ((struct lyd_meta *)item->node)->annotation->module;
        }

        set_fill_string(set, mod->ns, strlen(mod->ns));
        break;
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath node() function (node type). Returns LYXP_SET_NODE_SET
 *        with only nodes from the context. In practice it either leaves the context
 *        as it is or returns an empty node set.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_node(struct lyxp_set **UNUSED(args), uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    if (options & LYXP_SCNODE_ALL) {
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_NODE);
        return LY_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        lyxp_set_free_content(set);
    }
    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath normalize-space(string?) function. Returns LYXP_SET_STRING
 *        with normalized value (no leading, trailing, double white spaces) of the node
 *        from the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_normalize_space(struct lyxp_set **args, uint16_t arg_count, struct lyxp_set *set, uint32_t options)
{
    uint16_t i, new_used;
    char *new;
    ly_bool have_spaces = 0, space_before = 0;
    struct lysc_node_leaf *sleaf;
    LY_ERR rc = LY_SUCCESS;

    if (options & LYXP_SCNODE_ALL) {
        if (arg_count && (args[0]->type == LYXP_SET_SCNODE_SET) &&
                (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype),
                        sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return rc;
    }

    if (arg_count) {
        set_fill_set(set, args[0]);
    }
    rc = lyxp_set_cast(set, LYXP_SET_STRING);
    LY_CHECK_RET(rc);

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
        LY_CHECK_ERR_RET(!new, LOGMEM(set->ctx), LY_EMEM);
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
        LY_CHECK_ERR_RET(!new, LOGMEM(set->ctx), LY_EMEM);
        new[new_used] = '\0';

        free(set->val.str);
        set->val.str = new;
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath not(boolean) function. Returns LYXP_SET_BOOLEAN
 *        with the argument converted to boolean and logically inverted.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_not(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    if (options & LYXP_SCNODE_ALL) {
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_NODE);
        return LY_SUCCESS;
    }

    lyxp_set_cast(args[0], LYXP_SET_BOOLEAN);
    if (args[0]->val.bln) {
        set_fill_boolean(set, 0);
    } else {
        set_fill_boolean(set, 1);
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath number(object?) function. Returns LYXP_SET_NUMBER
 *        with the number representation of either the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_number(struct lyxp_set **args, uint16_t arg_count, struct lyxp_set *set, uint32_t options)
{
    LY_ERR rc;

    if (options & LYXP_SCNODE_ALL) {
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return LY_SUCCESS;
    }

    if (arg_count) {
        rc = lyxp_set_cast(args[0], LYXP_SET_NUMBER);
        LY_CHECK_RET(rc);
        set_fill_set(set, args[0]);
    } else {
        rc = lyxp_set_cast(set, LYXP_SET_NUMBER);
        LY_CHECK_RET(rc);
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath position() function. Returns LYXP_SET_NUMBER
 *        with the context position.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_position(struct lyxp_set **UNUSED(args), uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    if (options & LYXP_SCNODE_ALL) {
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_NODE);
        return LY_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INCTX, print_set_type(set), "position()");
        return LY_EVALID;
    } else if (!set->used) {
        set_fill_number(set, 0);
        return LY_SUCCESS;
    }

    set_fill_number(set, set->ctx_pos);

    /* UNUSED in 'Release' build type */
    (void)options;
    return LY_SUCCESS;
}

/**
 * @brief Execute the YANG 1.1 re-match(string, string) function. Returns LYXP_SET_BOOLEAN
 *        depending on whether the second argument regex matches the first argument string. For details refer to
 *        YANG 1.1 RFC section 10.2.1.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_re_match(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    struct lysc_pattern **patterns = NULL, **pattern;
    struct lysc_node_leaf *sleaf;
    LY_ERR rc = LY_SUCCESS;
    struct ly_err_item *err;

    if (options & LYXP_SCNODE_ALL) {
        if ((args[0]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype), sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }

        if ((args[1]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype), sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #2 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return rc;
    }

    rc = lyxp_set_cast(args[0], LYXP_SET_STRING);
    LY_CHECK_RET(rc);
    rc = lyxp_set_cast(args[1], LYXP_SET_STRING);
    LY_CHECK_RET(rc);

    LY_ARRAY_NEW_RET(set->ctx, patterns, pattern, LY_EMEM);
    *pattern = calloc(1, sizeof **pattern);
    LOG_LOCSET(NULL, set->cur_node, NULL, NULL);
    rc = lys_compile_type_pattern_check(set->ctx, args[1]->val.str, &(*pattern)->code);
    LOG_LOCBACK(0, 1, 0, 0);
    if (rc != LY_SUCCESS) {
        LY_ARRAY_FREE(patterns);
        return rc;
    }

    rc = lyplg_type_validate_patterns(patterns, args[0]->val.str, strlen(args[0]->val.str), &err);
    pcre2_code_free((*pattern)->code);
    free(*pattern);
    LY_ARRAY_FREE(patterns);
    if (rc && (rc != LY_EVALID)) {
        ly_err_print(set->ctx, err);
        ly_err_free(err);
        return rc;
    }

    if (rc == LY_EVALID) {
        ly_err_free(err);
        set_fill_boolean(set, 0);
    } else {
        set_fill_boolean(set, 1);
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath round(number) function. Returns LYXP_SET_NUMBER
 *        with the rounded first argument. For details refer to
 *        http://www.w3.org/TR/1999/REC-xpath-19991116/#function-round.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_round(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    struct lysc_node_leaf *sleaf;
    LY_ERR rc = LY_SUCCESS;

    if (options & LYXP_SCNODE_ALL) {
        if (args[0]->type != LYXP_SET_SCNODE_SET) {
            LOGWRN(set->ctx, "Argument #1 of %s not a node-set as expected.", __func__);
        } else if ((sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype),
                        sleaf->name);
            } else if (!warn_is_specific_type(sleaf->type, LY_TYPE_DEC64)) {
                LOGWRN(set->ctx, "Argument #1 of %s is node \"%s\", not of type \"decimal64\".", __func__, sleaf->name);
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return rc;
    }

    rc = lyxp_set_cast(args[0], LYXP_SET_NUMBER);
    LY_CHECK_RET(rc);

    /* cover only the cases where floor can't be used */
    if ((args[0]->val.num == -0.0f) || ((args[0]->val.num < 0) && (args[0]->val.num >= -0.5))) {
        set_fill_number(set, -0.0f);
    } else {
        args[0]->val.num += 0.5;
        rc = xpath_floor(args, 1, args[0], options);
        LY_CHECK_RET(rc);
        set_fill_number(set, args[0]->val.num);
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath starts-with(string, string) function.
 *        Returns LYXP_SET_BOOLEAN whether the second argument is
 *        the prefix of the first or not.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_starts_with(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    struct lysc_node_leaf *sleaf;
    LY_ERR rc = LY_SUCCESS;

    if (options & LYXP_SCNODE_ALL) {
        if ((args[0]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype), sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }

        if ((args[1]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype), sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #2 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return rc;
    }

    rc = lyxp_set_cast(args[0], LYXP_SET_STRING);
    LY_CHECK_RET(rc);
    rc = lyxp_set_cast(args[1], LYXP_SET_STRING);
    LY_CHECK_RET(rc);

    if (strncmp(args[0]->val.str, args[1]->val.str, strlen(args[1]->val.str))) {
        set_fill_boolean(set, 0);
    } else {
        set_fill_boolean(set, 1);
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath string(object?) function. Returns LYXP_SET_STRING
 *        with the string representation of either the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_string(struct lyxp_set **args, uint16_t arg_count, struct lyxp_set *set, uint32_t options)
{
    LY_ERR rc;

    if (options & LYXP_SCNODE_ALL) {
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return LY_SUCCESS;
    }

    if (arg_count) {
        rc = lyxp_set_cast(args[0], LYXP_SET_STRING);
        LY_CHECK_RET(rc);
        set_fill_set(set, args[0]);
    } else {
        rc = lyxp_set_cast(set, LYXP_SET_STRING);
        LY_CHECK_RET(rc);
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath string-length(string?) function. Returns LYXP_SET_NUMBER
 *        with the length of the string in either the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_string_length(struct lyxp_set **args, uint16_t arg_count, struct lyxp_set *set, uint32_t options)
{
    struct lysc_node_leaf *sleaf;
    LY_ERR rc = LY_SUCCESS;

    if (options & LYXP_SCNODE_ALL) {
        if (arg_count && (args[0]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype), sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }
        if (!arg_count && (set->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(set))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #0 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype), sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #0 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return rc;
    }

    if (arg_count) {
        rc = lyxp_set_cast(args[0], LYXP_SET_STRING);
        LY_CHECK_RET(rc);
        set_fill_number(set, strlen(args[0]->val.str));
    } else {
        rc = lyxp_set_cast(set, LYXP_SET_STRING);
        LY_CHECK_RET(rc);
        set_fill_number(set, strlen(set->val.str));
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath substring(string, number, number?) function.
 *        Returns LYXP_SET_STRING substring of the first argument starting
 *        on the second argument index ending on the third argument index,
 *        indexed from 1. For exact definition refer to
 *        http://www.w3.org/TR/1999/REC-xpath-19991116/#function-substring.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_substring(struct lyxp_set **args, uint16_t arg_count, struct lyxp_set *set, uint32_t options)
{
    int32_t start, len;
    uint16_t str_start, str_len, pos;
    struct lysc_node_leaf *sleaf;
    LY_ERR rc = LY_SUCCESS;

    if (options & LYXP_SCNODE_ALL) {
        if ((args[0]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype), sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }

        if ((args[1]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype), sleaf->name);
            } else if (!warn_is_numeric_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #2 of %s is node \"%s\", not of numeric type.", __func__, sleaf->name);
            }
        }

        if ((arg_count == 3) && (args[2]->type == LYXP_SET_SCNODE_SET) &&
                (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[2]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #3 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype), sleaf->name);
            } else if (!warn_is_numeric_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #3 of %s is node \"%s\", not of numeric type.", __func__, sleaf->name);
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return rc;
    }

    rc = lyxp_set_cast(args[0], LYXP_SET_STRING);
    LY_CHECK_RET(rc);

    /* start */
    if (xpath_round(&args[1], 1, args[1], options)) {
        return -1;
    }
    if (isfinite(args[1]->val.num)) {
        start = args[1]->val.num - 1;
    } else if (isinf(args[1]->val.num) && signbit(args[1]->val.num)) {
        start = INT32_MIN;
    } else {
        start = INT32_MAX;
    }

    /* len */
    if (arg_count == 3) {
        rc = xpath_round(&args[2], 1, args[2], options);
        LY_CHECK_RET(rc);
        if (isnan(args[2]->val.num) || signbit(args[2]->val.num)) {
            len = 0;
        } else if (isfinite(args[2]->val.num)) {
            len = args[2]->val.num;
        } else {
            len = INT32_MAX;
        }
    } else {
        len = INT32_MAX;
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
    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath substring-after(string, string) function.
 *        Returns LYXP_SET_STRING with the string succeeding the occurance
 *        of the second argument in the first or an empty string.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_substring_after(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    char *ptr;
    struct lysc_node_leaf *sleaf;
    LY_ERR rc = LY_SUCCESS;

    if (options & LYXP_SCNODE_ALL) {
        if ((args[0]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype), sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }

        if ((args[1]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype), sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #2 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return rc;
    }

    rc = lyxp_set_cast(args[0], LYXP_SET_STRING);
    LY_CHECK_RET(rc);
    rc = lyxp_set_cast(args[1], LYXP_SET_STRING);
    LY_CHECK_RET(rc);

    ptr = strstr(args[0]->val.str, args[1]->val.str);
    if (ptr) {
        set_fill_string(set, ptr + strlen(args[1]->val.str), strlen(ptr + strlen(args[1]->val.str)));
    } else {
        set_fill_string(set, "", 0);
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath substring-before(string, string) function.
 *        Returns LYXP_SET_STRING with the string preceding the occurance
 *        of the second argument in the first or an empty string.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_substring_before(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    char *ptr;
    struct lysc_node_leaf *sleaf;
    LY_ERR rc = LY_SUCCESS;

    if (options & LYXP_SCNODE_ALL) {
        if ((args[0]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype), sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }

        if ((args[1]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype), sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #2 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return rc;
    }

    rc = lyxp_set_cast(args[0], LYXP_SET_STRING);
    LY_CHECK_RET(rc);
    rc = lyxp_set_cast(args[1], LYXP_SET_STRING);
    LY_CHECK_RET(rc);

    ptr = strstr(args[0]->val.str, args[1]->val.str);
    if (ptr) {
        set_fill_string(set, args[0]->val.str, ptr - args[0]->val.str);
    } else {
        set_fill_string(set, "", 0);
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath sum(node-set) function. Returns LYXP_SET_NUMBER
 *        with the sum of all the nodes in the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_sum(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    long double num;
    char *str;
    uint32_t i;
    struct lyxp_set set_item;
    struct lysc_node_leaf *sleaf;
    LY_ERR rc = LY_SUCCESS;

    if (options & LYXP_SCNODE_ALL) {
        if (args[0]->type == LYXP_SET_SCNODE_SET) {
            for (i = 0; i < args[0]->used; ++i) {
                if (args[0]->val.scnodes[i].in_ctx == LYXP_SET_SCNODE_ATOM_CTX) {
                    sleaf = (struct lysc_node_leaf *)args[0]->val.scnodes[i].scnode;
                    if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                        LOGWRN(set->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__,
                                lys_nodetype2str(sleaf->nodetype), sleaf->name);
                    } else if (!warn_is_numeric_type(sleaf->type)) {
                        LOGWRN(set->ctx, "Argument #1 of %s is node \"%s\", not of numeric type.", __func__, sleaf->name);
                    }
                }
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return rc;
    }

    set_fill_number(set, 0);

    if (args[0]->type != LYXP_SET_NODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INARGTYPE, 1, print_set_type(args[0]), "sum(node-set)");
        return LY_EVALID;
    } else if (!args[0]->used) {
        return LY_SUCCESS;
    }

    set_init(&set_item, set);

    set_item.type = LYXP_SET_NODE_SET;
    set_item.val.nodes = malloc(sizeof *set_item.val.nodes);
    LY_CHECK_ERR_RET(!set_item.val.nodes, LOGMEM(set->ctx), LY_EMEM);

    set_item.used = 1;
    set_item.size = 1;

    for (i = 0; i < args[0]->used; ++i) {
        set_item.val.nodes[0] = args[0]->val.nodes[i];

        rc = cast_node_set_to_string(&set_item, &str);
        LY_CHECK_RET(rc);
        num = cast_string_to_number(str);
        free(str);
        set->val.num += num;
    }

    free(set_item.val.nodes);

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath text() function (node type). Returns LYXP_SET_NODE_SET
 *        with the text content of the nodes in the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_text(struct lyxp_set **UNUSED(args), uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    uint32_t i;

    if (options & LYXP_SCNODE_ALL) {
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return LY_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INCTX, print_set_type(set), "text()");
        return LY_EVALID;
    }

    for (i = 0; i < set->used; ) {
        switch (set->val.nodes[i].type) {
        case LYXP_NODE_NONE:
            LOGINT_RET(set->ctx);
        case LYXP_NODE_ELEM:
            if (set->val.nodes[i].node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
                set->val.nodes[i].type = LYXP_NODE_TEXT;
                ++i;
                break;
            }
        /* fall through */
        case LYXP_NODE_ROOT:
        case LYXP_NODE_ROOT_CONFIG:
        case LYXP_NODE_TEXT:
        case LYXP_NODE_META:
            set_remove_node(set, i);
            break;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath translate(string, string, string) function.
 *        Returns LYXP_SET_STRING with the first argument with the characters
 *        from the second argument replaced by those on the corresponding
 *        positions in the third argument.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_translate(struct lyxp_set **args, uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    uint16_t i, j, new_used;
    char *new;
    ly_bool have_removed;
    struct lysc_node_leaf *sleaf;
    LY_ERR rc = LY_SUCCESS;

    if (options & LYXP_SCNODE_ALL) {
        if ((args[0]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[0]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #1 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype), sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #1 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }

        if ((args[1]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[1]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #2 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype), sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #2 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }

        if ((args[2]->type == LYXP_SET_SCNODE_SET) && (sleaf = (struct lysc_node_leaf *)warn_get_scnode_in_ctx(args[2]))) {
            if (!(sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                LOGWRN(set->ctx, "Argument #3 of %s is a %s node \"%s\".", __func__, lys_nodetype2str(sleaf->nodetype), sleaf->name);
            } else if (!warn_is_string_type(sleaf->type)) {
                LOGWRN(set->ctx, "Argument #3 of %s is node \"%s\", not of string-type.", __func__, sleaf->name);
            }
        }
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        return rc;
    }

    rc = lyxp_set_cast(args[0], LYXP_SET_STRING);
    LY_CHECK_RET(rc);
    rc = lyxp_set_cast(args[1], LYXP_SET_STRING);
    LY_CHECK_RET(rc);
    rc = lyxp_set_cast(args[2], LYXP_SET_STRING);
    LY_CHECK_RET(rc);

    new = malloc((strlen(args[0]->val.str) + 1) * sizeof(char));
    LY_CHECK_ERR_RET(!new, LOGMEM(set->ctx), LY_EMEM);
    new_used = 0;

    have_removed = 0;
    for (i = 0; args[0]->val.str[i]; ++i) {
        ly_bool found = 0;

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
        LY_CHECK_ERR_RET(!new, LOGMEM(set->ctx), LY_EMEM);
    }
    new[new_used] = '\0';

    lyxp_set_free_content(set);
    set->type = LYXP_SET_STRING;
    set->val.str = new;

    return LY_SUCCESS;
}

/**
 * @brief Execute the XPath true() function. Returns LYXP_SET_BOOLEAN
 *        with true value.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in @p args.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
xpath_true(struct lyxp_set **UNUSED(args), uint16_t UNUSED(arg_count), struct lyxp_set *set, uint32_t options)
{
    if (options & LYXP_SCNODE_ALL) {
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_NODE);
        return LY_SUCCESS;
    }

    set_fill_boolean(set, 1);
    return LY_SUCCESS;
}

/*
 * moveto functions
 *
 * They and only they actually change the context (set).
 */

/**
 * @brief Skip prefix and return corresponding model if there is a prefix. Logs directly.
 *
 * XPath @p set is expected to be a (sc)node set!
 *
 * @param[in,out] qname Qualified node name. If includes prefix, it is skipped.
 * @param[in,out] qname_len Length of @p qname, is updated accordingly.
 * @param[in] set Set with general XPath context.
 * @param[in] ctx_scnode Context node to inherit module for unprefixed node for ::LY_PREF_JSON.
 * @param[out] moveto_mod Expected module of a matching node.
 * @return LY_ERR
 */
static LY_ERR
moveto_resolve_model(const char **qname, uint16_t *qname_len, const struct lyxp_set *set,
        const struct lysc_node *ctx_scnode, const struct lys_module **moveto_mod)
{
    const struct lys_module *mod = NULL;
    const char *ptr;
    size_t pref_len;

    assert((set->type == LYXP_SET_NODE_SET) || (set->type == LYXP_SET_SCNODE_SET));

    if ((ptr = ly_strnchr(*qname, ':', *qname_len))) {
        /* specific module */
        pref_len = ptr - *qname;
        mod = ly_resolve_prefix(set->ctx, *qname, pref_len, set->format, set->prefix_data);

        /* check for errors and non-implemented modules, as they are not valid */
        if (!mod || !mod->implemented) {
            LOGVAL(set->ctx, LY_VCODE_XP_INMOD, pref_len, *qname);
            return LY_EVALID;
        }

        *qname += pref_len + 1;
        *qname_len -= pref_len + 1;
    } else if (((*qname)[0] == '*') && (*qname_len == 1)) {
        /* all modules - special case */
        mod = NULL;
    } else {
        switch (set->format) {
        case LY_VALUE_SCHEMA:
        case LY_VALUE_SCHEMA_RESOLVED:
            /* current module */
            mod = set->cur_mod;
            break;
        case LY_VALUE_CANON:
        case LY_VALUE_JSON:
        case LY_VALUE_LYB:
            /* inherit parent (context node) module */
            if (ctx_scnode) {
                mod = ctx_scnode->module;
            } else {
                mod = NULL;
            }
            break;
        case LY_VALUE_XML:
            /* all nodes need to be prefixed */
            LOGVAL(set->ctx, LYVE_DATA, "Non-prefixed node \"%.*s\" in XML xpath found.", *qname_len, *qname);
            return LY_EVALID;
        }
    }

    *moveto_mod = mod;
    return LY_SUCCESS;
}

/**
 * @brief Move context @p set to the root. Handles absolute path.
 *        Result is LYXP_SET_NODE_SET.
 *
 * @param[in,out] set Set to use.
 * @param[in] options Xpath options.
 * @return LY_ERR value.
 */
static LY_ERR
moveto_root(struct lyxp_set *set, uint32_t options)
{
    if (!set) {
        return LY_SUCCESS;
    }

    if (options & LYXP_SCNODE_ALL) {
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_NODE);
        LY_CHECK_RET(lyxp_set_scnode_insert_node(set, NULL, set->root_type, NULL));
    } else {
        set->type = LYXP_SET_NODE_SET;
        set->used = 0;
        set_insert_node(set, NULL, 0, set->root_type, 0);
    }

    return LY_SUCCESS;
}

/**
 * @brief Check @p node as a part of NameTest processing.
 *
 * @param[in] node Node to check.
 * @param[in] ctx_node Context node.
 * @param[in] set Set to read general context from.
 * @param[in] node_name Node name in the dictionary to move to, NULL for any node.
 * @param[in] moveto_mod Expected module of the node, NULL for no prefix.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_ENOT if node does not match, LY_EINCOMPLETE on unresolved when,
 * LY_EINVAL if netither node nor any children match)
 */
static LY_ERR
moveto_node_check(const struct lyd_node *node, const struct lyd_node *ctx_node, const struct lyxp_set *set,
        const char *node_name, const struct lys_module *moveto_mod, uint32_t options)
{
    if (!moveto_mod && (!node_name || strcmp(node_name, "*"))) {
        switch (set->format) {
        case LY_VALUE_SCHEMA:
        case LY_VALUE_SCHEMA_RESOLVED:
            /* use current module */
            moveto_mod = set->cur_mod;
            break;
        case LY_VALUE_JSON:
        case LY_VALUE_LYB:
            /* inherit module of the context node, if any */
            if (ctx_node) {
                moveto_mod = ctx_node->schema->module;
            }
            break;
        case LY_VALUE_CANON:
        case LY_VALUE_XML:
            /* not defined */
            LOGINT(set->ctx);
            return LY_EINVAL;
        }
    }

    /* module check */
    if (moveto_mod && (node->schema->module != moveto_mod)) {
        return LY_ENOT;
    }

    /* context check */
    if ((set->root_type == LYXP_NODE_ROOT_CONFIG) && (node->schema->flags & LYS_CONFIG_R)) {
        return LY_EINVAL;
    } else if (set->context_op && (node->schema->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) &&
            (node->schema != set->context_op)) {
        return LY_EINVAL;
    }

    /* name check */
    if (node_name && strcmp(node_name, "*") && (node->schema->name != node_name)) {
        return LY_ENOT;
    }

    /* when check */
    if (!(options & LYXP_IGNORE_WHEN) && lysc_has_when(node->schema) && !(node->flags & LYD_WHEN_TRUE)) {
        return LY_EINCOMPLETE;
    }

    /* match */
    return LY_SUCCESS;
}

/**
 * @brief Check @p node as a part of schema NameTest processing.
 *
 * @param[in] node Schema node to check.
 * @param[in] ctx_scnode Context node.
 * @param[in] set Set to read general context from.
 * @param[in] node_name Node name in the dictionary to move to, NULL for any nodes.
 * @param[in] moveto_mod Expected module of the node, NULL for no prefix.
 * @return LY_ERR (LY_ENOT if node does not match, LY_EINVAL if neither node nor any children match)
 */
static LY_ERR
moveto_scnode_check(const struct lysc_node *node, const struct lysc_node *ctx_scnode, const struct lyxp_set *set,
        const char *node_name, const struct lys_module *moveto_mod)
{
    if (!moveto_mod && (!node_name || strcmp(node_name, "*"))) {
        switch (set->format) {
        case LY_VALUE_SCHEMA:
        case LY_VALUE_SCHEMA_RESOLVED:
            /* use current module */
            moveto_mod = set->cur_mod;
            break;
        case LY_VALUE_JSON:
        case LY_VALUE_LYB:
            /* inherit module of the context node, if any */
            if (ctx_scnode) {
                moveto_mod = ctx_scnode->module;
            }
            break;
        case LY_VALUE_CANON:
        case LY_VALUE_XML:
            /* not defined */
            LOGINT(set->ctx);
            return LY_EINVAL;
        }
    }

    /* module check */
    if (moveto_mod && (node->module != moveto_mod)) {
        return LY_ENOT;
    }

    /* context check */
    if ((set->root_type == LYXP_NODE_ROOT_CONFIG) && (node->flags & LYS_CONFIG_R)) {
        return LY_EINVAL;
    } else if (set->context_op && (node->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) && (node != set->context_op)) {
        return LY_EINVAL;
    }

    /* name check */
    if (node_name && strcmp(node_name, "*") && (node->name != node_name)) {
        return LY_ENOT;
    }

    /* match */
    return LY_SUCCESS;
}

/**
 * @brief Move context @p set to a node. Result is LYXP_SET_NODE_SET (or LYXP_SET_EMPTY). Context position aware.
 *
 * @param[in,out] set Set to use.
 * @param[in] moveto_mod Matching node module, NULL for no prefix.
 * @param[in] ncname Matching node name in the dictionary, NULL for any.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
moveto_node(struct lyxp_set *set, const struct lys_module *moveto_mod, const char *ncname, uint32_t options)
{
    uint32_t i;
    const struct lyd_node *siblings, *sub, *ctx_node;
    LY_ERR rc;

    if (!set) {
        return LY_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INOP_1, "path operator", print_set_type(set));
        return LY_EVALID;
    }

    for (i = 0; i < set->used; ) {
        ly_bool replaced = 0;

        if ((set->val.nodes[i].type == LYXP_NODE_ROOT_CONFIG) || (set->val.nodes[i].type == LYXP_NODE_ROOT)) {
            assert(!set->val.nodes[i].node);

            /* search in all the trees */
            ctx_node = NULL;
            siblings = set->tree;
        } else {
            /* search in children */
            ctx_node = set->val.nodes[i].node;
            siblings = lyd_child(ctx_node);
        }

        for (sub = siblings; sub; sub = sub->next) {
            rc = moveto_node_check(sub, ctx_node, set, ncname, moveto_mod, options);
            if (rc == LY_SUCCESS) {
                if (!replaced) {
                    set_replace_node(set, sub, 0, LYXP_NODE_ELEM, i);
                    replaced = 1;
                } else {
                    set_insert_node(set, sub, 0, LYXP_NODE_ELEM, i);
                }
                ++i;
            } else if (rc == LY_EINCOMPLETE) {
                return rc;
            }
        }

        if (!replaced) {
            /* no match */
            set_remove_node(set, i);
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Move context @p set to a node using hashes. Result is LYXP_SET_NODE_SET (or LYXP_SET_EMPTY).
 * Context position aware.
 *
 * @param[in,out] set Set to use.
 * @param[in] scnode Matching node schema.
 * @param[in] predicates If @p scnode is ::LYS_LIST or ::LYS_LEAFLIST, the predicates specifying a single instance.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
moveto_node_hash(struct lyxp_set *set, const struct lysc_node *scnode, const struct ly_path_predicate *predicates,
        uint32_t options)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t i;
    const struct lyd_node *siblings;
    struct lyd_node *sub, *inst = NULL;

    assert(scnode && (!(scnode->nodetype & (LYS_LIST | LYS_LEAFLIST)) || predicates));

    if (!set) {
        goto cleanup;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INOP_1, "path operator", print_set_type(set));
        ret = LY_EVALID;
        goto cleanup;
    }

    /* context check for all the nodes since we have the schema node */
    if ((set->root_type == LYXP_NODE_ROOT_CONFIG) && (scnode->flags & LYS_CONFIG_R)) {
        lyxp_set_free_content(set);
        goto cleanup;
    } else if (set->context_op && (scnode->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) &&
            (scnode != set->context_op)) {
        lyxp_set_free_content(set);
        goto cleanup;
    }

    /* create specific data instance if needed */
    if (scnode->nodetype == LYS_LIST) {
        LY_CHECK_GOTO(ret = lyd_create_list(scnode, predicates, &inst), cleanup);
    } else if (scnode->nodetype == LYS_LEAFLIST) {
        LY_CHECK_GOTO(ret = lyd_create_term2(scnode, &predicates[0].value, &inst), cleanup);
    }

    for (i = 0; i < set->used; ) {
        siblings = NULL;

        if ((set->val.nodes[i].type == LYXP_NODE_ROOT_CONFIG) || (set->val.nodes[i].type == LYXP_NODE_ROOT)) {
            assert(!set->val.nodes[i].node);

            /* search in all the trees */
            siblings = set->tree;
        } else if (set->val.nodes[i].type == LYXP_NODE_ELEM) {
            /* search in children */
            siblings = lyd_child(set->val.nodes[i].node);
        }

        /* find the node using hashes */
        if (inst) {
            lyd_find_sibling_first(siblings, inst, &sub);
        } else {
            lyd_find_sibling_val(siblings, scnode, NULL, 0, &sub);
        }

        /* when check */
        if (!(options & LYXP_IGNORE_WHEN) && sub && lysc_has_when(sub->schema) && !(sub->flags & LYD_WHEN_TRUE)) {
            ret = LY_EINCOMPLETE;
            goto cleanup;
        }

        if (sub) {
            /* pos filled later */
            set_replace_node(set, sub, 0, LYXP_NODE_ELEM, i);
            ++i;
        } else {
            /* no match */
            set_remove_node(set, i);
        }
    }

cleanup:
    lyd_free_tree(inst);
    return ret;
}

/**
 * @brief Move context @p set to a schema node. Result is LYXP_SET_SCNODE_SET (or LYXP_SET_EMPTY).
 *
 * @param[in,out] set Set to use.
 * @param[in] moveto_mod Matching node module, NULL for no prefix.
 * @param[in] ncname Matching node name in the dictionary, NULL for any.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
moveto_scnode(struct lyxp_set *set, const struct lys_module *moveto_mod, const char *ncname, uint32_t options)
{
    ly_bool temp_ctx = 0;
    uint32_t getnext_opts;
    uint32_t orig_used, i;
    uint32_t mod_idx;
    const struct lysc_node *iter, *start_parent;
    const struct lys_module *mod;

    if (!set) {
        return LY_SUCCESS;
    }

    if (set->type != LYXP_SET_SCNODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INOP_1, "path operator", print_set_type(set));
        return LY_EVALID;
    }

    /* getnext opts */
    getnext_opts = 0;
    if (options & LYXP_SCNODE_OUTPUT) {
        getnext_opts |= LYS_GETNEXT_OUTPUT;
    }

    orig_used = set->used;
    for (i = 0; i < orig_used; ++i) {
        uint32_t idx;

        if (set->val.scnodes[i].in_ctx != LYXP_SET_SCNODE_ATOM_CTX) {
            if (set->val.scnodes[i].in_ctx != LYXP_SET_SCNODE_START) {
                continue;
            }

            /* remember context node */
            set->val.scnodes[i].in_ctx = LYXP_SET_SCNODE_START_USED;
        } else {
            set->val.scnodes[i].in_ctx = LYXP_SET_SCNODE_ATOM_NODE;
        }

        start_parent = set->val.scnodes[i].scnode;

        if ((set->val.scnodes[i].type == LYXP_NODE_ROOT_CONFIG) || (set->val.scnodes[i].type == LYXP_NODE_ROOT)) {
            /* it can actually be in any module, it's all <running>, and even if it's moveto_mod (if set),
             * it can be in a top-level augment (the root node itself is useless in this case) */
            mod_idx = 0;
            while ((mod = (struct lys_module *)ly_ctx_get_module_iter(set->ctx, &mod_idx))) {
                iter = NULL;
                /* module may not be implemented */
                while (mod->implemented && (iter = lys_getnext(iter, NULL, mod->compiled, getnext_opts))) {
                    if (!moveto_scnode_check(iter, NULL, set, ncname, moveto_mod)) {
                        LY_CHECK_RET(lyxp_set_scnode_insert_node(set, iter, LYXP_NODE_ELEM, &idx));

                        /* we need to prevent these nodes from being considered in this moveto */
                        if ((idx < orig_used) && (idx > i)) {
                            set->val.scnodes[idx].in_ctx = LYXP_SET_SCNODE_ATOM_NEW_CTX;
                            temp_ctx = 1;
                        }
                    }
                }
            }

        } else if (set->val.scnodes[i].type == LYXP_NODE_ELEM) {
            iter = NULL;
            while ((iter = lys_getnext(iter, start_parent, NULL, getnext_opts))) {
                if (!moveto_scnode_check(iter, start_parent, set, ncname, moveto_mod)) {
                    LY_CHECK_RET(lyxp_set_scnode_insert_node(set, iter, LYXP_NODE_ELEM, &idx));

                    if ((idx < orig_used) && (idx > i)) {
                        set->val.scnodes[idx].in_ctx = LYXP_SET_SCNODE_ATOM_NEW_CTX;
                        temp_ctx = 1;
                    }
                }
            }
        }
    }

    /* correct temporary in_ctx values */
    if (temp_ctx) {
        for (i = 0; i < orig_used; ++i) {
            if (set->val.scnodes[i].in_ctx == LYXP_SET_SCNODE_ATOM_NEW_CTX) {
                set->val.scnodes[i].in_ctx = LYXP_SET_SCNODE_ATOM_CTX;
            }
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Move context @p set to a node and all its descendants. Result is LYXP_SET_NODE_SET (or LYXP_SET_EMPTY).
 *        Context position aware.
 *
 * @param[in] set Set to use.
 * @param[in] moveto_mod Matching node module, NULL for no prefix.
 * @param[in] ncname Matching node name in the dictionary, NULL for any.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
moveto_node_alldesc(struct lyxp_set *set, const struct lys_module *moveto_mod, const char *ncname, uint32_t options)
{
    uint32_t i;
    const struct lyd_node *next, *elem, *start;
    struct lyxp_set ret_set;
    LY_ERR rc;

    if (!set) {
        return LY_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INOP_1, "path operator", print_set_type(set));
        return LY_EVALID;
    }

    /* replace the original nodes (and throws away all text and meta nodes, root is replaced by a child) */
    rc = moveto_node(set, NULL, NULL, options);
    LY_CHECK_RET(rc);

    /* this loop traverses all the nodes in the set and adds/keeps only those that match qname */
    set_init(&ret_set, set);
    for (i = 0; i < set->used; ++i) {

        /* TREE DFS */
        start = set->val.nodes[i].node;
        for (elem = next = start; elem; elem = next) {
            rc = moveto_node_check(elem, start, set, ncname, moveto_mod, options);
            if (!rc) {
                /* add matching node into result set */
                set_insert_node(&ret_set, elem, 0, LYXP_NODE_ELEM, ret_set.used);
                if (set_dup_node_check(set, elem, LYXP_NODE_ELEM, i)) {
                    /* the node is a duplicate, we'll process it later in the set */
                    goto skip_children;
                }
            } else if (rc == LY_EINCOMPLETE) {
                return rc;
            } else if (rc == LY_EINVAL) {
                goto skip_children;
            }

            /* TREE DFS NEXT ELEM */
            /* select element for the next run - children first */
            next = lyd_child(elem);
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
                if (lyd_parent(elem) == start) {
                    /* we are done, no next element to process */
                    break;
                }
                /* parent is already processed, go to its sibling */
                elem = lyd_parent(elem);
                next = elem->next;
            }
        }
    }

    /* make the temporary set the current one */
    ret_set.ctx_pos = set->ctx_pos;
    ret_set.ctx_size = set->ctx_size;
    lyxp_set_free_content(set);
    memcpy(set, &ret_set, sizeof *set);

    return LY_SUCCESS;
}

/**
 * @brief Move context @p set to a schema node and all its descendants. Result is LYXP_SET_NODE_SET.
 *
 * @param[in] set Set to use.
 * @param[in] moveto_mod Matching node module, NULL for no prefix.
 * @param[in] ncname Matching node name in the dictionary, NULL for any.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
moveto_scnode_alldesc(struct lyxp_set *set, const struct lys_module *moveto_mod, const char *ncname, uint32_t options)
{
    uint32_t i, orig_used;
    const struct lysc_node *next, *elem, *start;
    LY_ERR rc;

    if (!set) {
        return LY_SUCCESS;
    }

    if (set->type != LYXP_SET_SCNODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INOP_1, "path operator", print_set_type(set));
        return LY_EVALID;
    }

    orig_used = set->used;
    for (i = 0; i < orig_used; ++i) {
        if (set->val.scnodes[i].in_ctx != LYXP_SET_SCNODE_ATOM_CTX) {
            if (set->val.scnodes[i].in_ctx != LYXP_SET_SCNODE_START) {
                continue;
            }

            /* remember context node */
            set->val.scnodes[i].in_ctx = LYXP_SET_SCNODE_START_USED;
        } else {
            set->val.scnodes[i].in_ctx = LYXP_SET_SCNODE_ATOM_NODE;
        }

        /* TREE DFS */
        start = set->val.scnodes[i].scnode;
        for (elem = next = start; elem; elem = next) {
            if ((elem == start) || (elem->nodetype & (LYS_CHOICE | LYS_CASE))) {
                /* schema-only nodes, skip root */
                goto next_iter;
            }

            rc = moveto_scnode_check(elem, start, set, ncname, moveto_mod);
            if (!rc) {
                uint32_t idx;

                if (lyxp_set_scnode_contains(set, elem, LYXP_NODE_ELEM, i, &idx)) {
                    set->val.scnodes[idx].in_ctx = LYXP_SET_SCNODE_ATOM_CTX;
                    if ((uint32_t)idx > i) {
                        /* we will process it later in the set */
                        goto skip_children;
                    }
                } else {
                    LY_CHECK_RET(lyxp_set_scnode_insert_node(set, elem, LYXP_NODE_ELEM, NULL));
                }
            } else if (rc == LY_EINVAL) {
                goto skip_children;
            }

next_iter:
            /* TREE DFS NEXT ELEM */
            /* select element for the next run - children first */
            next = lysc_node_child(elem);
            if (next && (next->nodetype == LYS_INPUT) && (options & LYXP_SCNODE_OUTPUT)) {
                next = next->next;
            } else if (next && (next->nodetype == LYS_OUTPUT) && !(options & LYXP_SCNODE_OUTPUT)) {
                next = next->next;
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

    return LY_SUCCESS;
}

/**
 * @brief Move context @p set to an attribute. Result is LYXP_SET_NODE_SET.
 *        Indirectly context position aware.
 *
 * @param[in,out] set Set to use.
 * @param[in] mod Matching metadata module, NULL for any.
 * @param[in] ncname Matching metadata name in the dictionary, NULL for any.
 * @return LY_ERR
 */
static LY_ERR
moveto_attr(struct lyxp_set *set, const struct lys_module *mod, const char *ncname)
{
    struct lyd_meta *sub;

    if (!set) {
        return LY_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INOP_1, "path operator", print_set_type(set));
        return LY_EVALID;
    }

    for (uint32_t i = 0; i < set->used; ) {
        ly_bool replaced = 0;

        /* only attributes of an elem (not dummy) can be in the result, skip all the rest;
         * our attributes are always qualified */
        if (set->val.nodes[i].type == LYXP_NODE_ELEM) {
            for (sub = set->val.nodes[i].node->meta; sub; sub = sub->next) {

                /* check "namespace" */
                if (mod && (sub->annotation->module != mod)) {
                    continue;
                }

                if (!ncname || (sub->name == ncname)) {
                    /* match */
                    if (!replaced) {
                        set->val.meta[i].meta = sub;
                        set->val.meta[i].type = LYXP_NODE_META;
                        /* pos does not change */
                        replaced = 1;
                    } else {
                        set_insert_node(set, (struct lyd_node *)sub, set->val.nodes[i].pos, LYXP_NODE_META, i + 1);
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

    return LY_SUCCESS;
}

/**
 * @brief Move context @p set1 to union with @p set2. @p set2 is emptied afterwards.
 *        Result is LYXP_SET_NODE_SET. Context position aware.
 *
 * @param[in,out] set1 Set to use for the result.
 * @param[in] set2 Set that is copied to @p set1.
 * @return LY_ERR
 */
static LY_ERR
moveto_union(struct lyxp_set *set1, struct lyxp_set *set2)
{
    LY_ERR rc;

    if ((set1->type != LYXP_SET_NODE_SET) || (set2->type != LYXP_SET_NODE_SET)) {
        LOGVAL(set1->ctx, LY_VCODE_XP_INOP_2, "union", print_set_type(set1), print_set_type(set2));
        return LY_EVALID;
    }

    /* set2 is empty or both set1 and set2 */
    if (!set2->used) {
        return LY_SUCCESS;
    }

    if (!set1->used) {
        memcpy(set1, set2, sizeof *set1);
        /* dynamic memory belongs to set1 now, do not free */
        memset(set2, 0, sizeof *set2);
        return LY_SUCCESS;
    }

    /* we assume sets are sorted */
    assert(!set_sort(set1) && !set_sort(set2));

    /* sort, remove duplicates */
    rc = set_sorted_merge(set1, set2);
    LY_CHECK_RET(rc);

    /* final set must be sorted */
    assert(!set_sort(set1));

    return LY_SUCCESS;
}

/**
 * @brief Move context @p set to an attribute in any of the descendants. Result is LYXP_SET_NODE_SET.
 *        Context position aware.
 *
 * @param[in,out] set Set to use.
 * @param[in] mod Matching metadata module, NULL for any.
 * @param[in] ncname Matching metadata name in the dictionary, NULL for any.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static int
moveto_attr_alldesc(struct lyxp_set *set, const struct lys_module *mod, const char *ncname, uint32_t options)
{
    struct lyd_meta *sub;
    struct lyxp_set *set_all_desc = NULL;
    LY_ERR rc;

    if (!set) {
        return LY_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INOP_1, "path operator", print_set_type(set));
        return LY_EVALID;
    }

    /* can be optimized similarly to moveto_node_alldesc() and save considerable amount of memory,
     * but it likely won't be used much, so it's a waste of time */
    /* copy the context */
    set_all_desc = set_copy(set);
    /* get all descendant nodes (the original context nodes are removed) */
    rc = moveto_node_alldesc(set_all_desc, NULL, NULL, options);
    if (rc != LY_SUCCESS) {
        lyxp_set_free(set_all_desc);
        return rc;
    }
    /* prepend the original context nodes */
    rc = moveto_union(set, set_all_desc);
    if (rc != LY_SUCCESS) {
        lyxp_set_free(set_all_desc);
        return rc;
    }
    lyxp_set_free(set_all_desc);

    for (uint32_t i = 0; i < set->used; ) {
        ly_bool replaced = 0;

        /* only attributes of an elem can be in the result, skip all the rest,
         * we have all attributes qualified in lyd tree */
        if (set->val.nodes[i].type == LYXP_NODE_ELEM) {
            for (sub = set->val.nodes[i].node->meta; sub; sub = sub->next) {
                /* check "namespace" */
                if (mod && (sub->annotation->module != mod)) {
                    continue;
                }

                if (!ncname || (sub->name == ncname)) {
                    /* match */
                    if (!replaced) {
                        set->val.meta[i].meta = sub;
                        set->val.meta[i].type = LYXP_NODE_META;
                        /* pos does not change */
                        replaced = 1;
                    } else {
                        set_insert_node(set, (struct lyd_node *)sub, set->val.meta[i].pos, LYXP_NODE_META, i + 1);
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

    return LY_SUCCESS;
}

/**
 * @brief Move context @p set to self and al chilren, recursively. Handles '/' or '//' and '.'. Result is LYXP_SET_NODE_SET.
 *        Context position aware.
 *
 * @param[in] parent Current parent.
 * @param[in] parent_pos Position of @p parent.
 * @param[in] parent_type Node type of @p parent.
 * @param[in,out] to_set Set to use.
 * @param[in] dup_check_set Set for checking duplicities.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
moveto_self_add_children_r(const struct lyd_node *parent, uint32_t parent_pos, enum lyxp_node_type parent_type,
        struct lyxp_set *to_set, const struct lyxp_set *dup_check_set, uint32_t options)
{
    const struct lyd_node *iter, *first;
    LY_ERR rc;

    switch (parent_type) {
    case LYXP_NODE_ROOT:
    case LYXP_NODE_ROOT_CONFIG:
        assert(!parent);

        /* add all top-level nodes as elements */
        first = to_set->tree;
        break;
    case LYXP_NODE_ELEM:
        /* add just the text node of this term element node */
        if (parent->schema->nodetype & (LYD_NODE_TERM | LYD_NODE_ANY)) {
            if (!set_dup_node_check(dup_check_set, parent, LYXP_NODE_TEXT, -1)) {
                set_insert_node(to_set, parent, parent_pos, LYXP_NODE_TEXT, to_set->used);
            }
            return LY_SUCCESS;
        }

        /* add all the children of this node */
        first = lyd_child(parent);
        break;
    default:
        LOGINT_RET(parent->schema->module->ctx);
    }

    /* add all top-level nodes as elements */
    LY_LIST_FOR(first, iter) {
        /* context check */
        if ((parent_type == LYXP_NODE_ROOT_CONFIG) && (iter->schema->flags & LYS_CONFIG_R)) {
            continue;
        }

        /* when check */
        if (!(options & LYXP_IGNORE_WHEN) && lysc_has_when(iter->schema) && !(iter->flags & LYD_WHEN_TRUE)) {
            return LY_EINCOMPLETE;
        }

        if (!set_dup_node_check(dup_check_set, iter, LYXP_NODE_ELEM, -1)) {
            set_insert_node(to_set, iter, 0, LYXP_NODE_ELEM, to_set->used);

            /* also add all the children of this node, recursively */
            rc = moveto_self_add_children_r(iter, 0, LYXP_NODE_ELEM, to_set, dup_check_set, options);
            LY_CHECK_RET(rc);
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Move context @p set to self. Handles '/' or '//' and '.'. Result is LYXP_SET_NODE_SET
 *        (or LYXP_SET_EMPTY). Context position aware.
 *
 * @param[in,out] set Set to use.
 * @param[in] all_desc Whether to go to all descendants ('//') or not ('/').
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
moveto_self(struct lyxp_set *set, ly_bool all_desc, uint32_t options)
{
    struct lyxp_set ret_set;
    LY_ERR rc;

    if (!set) {
        return LY_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INOP_1, "path operator", print_set_type(set));
        return LY_EVALID;
    }

    /* nothing to do */
    if (!all_desc) {
        return LY_SUCCESS;
    }

    /* add all the children, they get added recursively */
    set_init(&ret_set, set);
    for (uint32_t i = 0; i < set->used; ++i) {
        /* copy the current node to tmp */
        set_insert_node(&ret_set, set->val.nodes[i].node, set->val.nodes[i].pos, set->val.nodes[i].type, ret_set.used);

        /* do not touch attributes and text nodes */
        if ((set->val.nodes[i].type == LYXP_NODE_TEXT) || (set->val.nodes[i].type == LYXP_NODE_META)) {
            continue;
        }

        /* add all the children */
        rc = moveto_self_add_children_r(set->val.nodes[i].node, set->val.nodes[i].pos, set->val.nodes[i].type, &ret_set,
                set, options);
        if (rc != LY_SUCCESS) {
            lyxp_set_free_content(&ret_set);
            return rc;
        }
    }

    /* use the temporary set as the current one */
    ret_set.ctx_pos = set->ctx_pos;
    ret_set.ctx_size = set->ctx_size;
    lyxp_set_free_content(set);
    memcpy(set, &ret_set, sizeof *set);

    return LY_SUCCESS;
}

/**
 * @brief Move context schema @p set to self. Handles '/' or '//' and '.'. Result is LYXP_SET_SCNODE_SET
 *        (or LYXP_SET_EMPTY).
 *
 * @param[in,out] set Set to use.
 * @param[in] all_desc Whether to go to all descendants ('//') or not ('/').
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
moveto_scnode_self(struct lyxp_set *set, ly_bool all_desc, uint32_t options)
{
    uint32_t getnext_opts;
    uint32_t mod_idx;
    const struct lysc_node *iter, *start_parent;
    const struct lys_module *mod;

    if (!set) {
        return LY_SUCCESS;
    }

    if (set->type != LYXP_SET_SCNODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INOP_1, "path operator", print_set_type(set));
        return LY_EVALID;
    }

    /* getnext opts */
    getnext_opts = 0;
    if (options & LYXP_SCNODE_OUTPUT) {
        getnext_opts |= LYS_GETNEXT_OUTPUT;
    }

    /* add all the children, recursively as they are being added into the same set */
    for (uint32_t i = 0; i < set->used; ++i) {
        if (!all_desc) {
            /* traverse the start node */
            if (set->val.scnodes[i].in_ctx == LYXP_SET_SCNODE_START) {
                set->val.scnodes[i].in_ctx = LYXP_SET_SCNODE_ATOM_CTX;
            }
            continue;
        }

        if (set->val.scnodes[i].in_ctx != LYXP_SET_SCNODE_ATOM_CTX) {
            if (set->val.scnodes[i].in_ctx != LYXP_SET_SCNODE_START) {
                continue;
            }

            /* remember context node */
            set->val.scnodes[i].in_ctx = LYXP_SET_SCNODE_START_USED;
        } else {
            set->val.scnodes[i].in_ctx = LYXP_SET_SCNODE_ATOM_NODE;
        }

        start_parent = set->val.scnodes[i].scnode;

        if ((set->val.scnodes[i].type == LYXP_NODE_ROOT_CONFIG) || (set->val.scnodes[i].type == LYXP_NODE_ROOT)) {
            /* it can actually be in any module, it's all <running> */
            mod_idx = 0;
            while ((mod = (struct lys_module *)ly_ctx_get_module_iter(set->ctx, &mod_idx))) {
                iter = NULL;
                /* module may not be implemented */
                while (mod->implemented && (iter = lys_getnext(iter, NULL, mod->compiled, getnext_opts))) {
                    /* context check */
                    if ((set->root_type == LYXP_NODE_ROOT_CONFIG) && (iter->flags & LYS_CONFIG_R)) {
                        continue;
                    }

                    LY_CHECK_RET(lyxp_set_scnode_insert_node(set, iter, LYXP_NODE_ELEM, NULL));
                    /* throw away the insert index, we want to consider that node again, recursively */
                }
            }

        } else if (set->val.scnodes[i].type == LYXP_NODE_ELEM) {
            iter = NULL;
            while ((iter = lys_getnext(iter, start_parent, NULL, getnext_opts))) {
                /* context check */
                if ((set->root_type == LYXP_NODE_ROOT_CONFIG) && (iter->flags & LYS_CONFIG_R)) {
                    continue;
                }

                LY_CHECK_RET(lyxp_set_scnode_insert_node(set, iter, LYXP_NODE_ELEM, NULL));
            }
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Move context @p set to parent. Handles '/' or '//' and '..'. Result is LYXP_SET_NODE_SET
 *        (or LYXP_SET_EMPTY). Context position aware.
 *
 * @param[in] set Set to use.
 * @param[in] all_desc Whether to go to all descendants ('//') or not ('/').
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
moveto_parent(struct lyxp_set *set, ly_bool all_desc, uint32_t options)
{
    LY_ERR rc;
    struct lyd_node *node, *new_node;
    enum lyxp_node_type new_type;

    if (!set) {
        return LY_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INOP_1, "path operator", print_set_type(set));
        return LY_EVALID;
    }

    if (all_desc) {
        /* <path>//.. == <path>//./.. */
        rc = moveto_self(set, 1, options);
        LY_CHECK_RET(rc);
    }

    for (uint32_t i = 0; i < set->used; ++i) {
        node = set->val.nodes[i].node;

        if (set->val.nodes[i].type == LYXP_NODE_ELEM) {
            new_node = lyd_parent(node);
        } else if (set->val.nodes[i].type == LYXP_NODE_TEXT) {
            new_node = node;
        } else if (set->val.nodes[i].type == LYXP_NODE_META) {
            new_node = set->val.meta[i].meta->parent;
            if (!new_node) {
                LOGINT_RET(set->ctx);
            }
        } else {
            /* root does not have a parent */
            set_remove_node_none(set, i);
            continue;
        }

        /* when check */
        if (!(options & LYXP_IGNORE_WHEN) && new_node && lysc_has_when(new_node->schema) && !(new_node->flags & LYD_WHEN_TRUE)) {
            return LY_EINCOMPLETE;
        }

        if (!new_node) {
            /* node already there can also be the root */
            new_type = set->root_type;

        } else {
            /* node has a standard parent (it can equal the root, it's not the root yet since they are fake) */
            new_type = LYXP_NODE_ELEM;
        }

        if (set_dup_node_check(set, new_node, new_type, -1)) {
            set_remove_node_none(set, i);
        } else {
            set_replace_node(set, new_node, 0, new_type, i);
        }
    }

    set_remove_nodes_none(set);
    assert(!set_sort(set) && !set_sorted_dup_node_clean(set));

    return LY_SUCCESS;
}

/**
 * @brief Move context schema @p set to parent. Handles '/' or '//' and '..'. Result is LYXP_SET_SCNODE_SET
 *        (or LYXP_SET_EMPTY).
 *
 * @param[in] set Set to use.
 * @param[in] all_desc Whether to go to all descendants ('//') or not ('/').
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
moveto_scnode_parent(struct lyxp_set *set, ly_bool all_desc, uint32_t options)
{
    uint32_t i, orig_used, idx;
    ly_bool temp_ctx = 0;
    const struct lysc_node *node, *new_node;
    enum lyxp_node_type new_type;

    if (!set) {
        return LY_SUCCESS;
    }

    if (set->type != LYXP_SET_SCNODE_SET) {
        LOGVAL(set->ctx, LY_VCODE_XP_INOP_1, "path operator", print_set_type(set));
        return LY_EVALID;
    }

    if (all_desc) {
        /* <path>//.. == <path>//./.. */
        LY_CHECK_RET(moveto_scnode_self(set, 1, options));
    }

    orig_used = set->used;
    for (i = 0; i < orig_used; ++i) {
        if (set->val.scnodes[i].in_ctx != LYXP_SET_SCNODE_ATOM_CTX) {
            if (set->val.scnodes[i].in_ctx != LYXP_SET_SCNODE_START) {
                continue;
            }

            /* remember context node */
            set->val.scnodes[i].in_ctx = LYXP_SET_SCNODE_START_USED;
        } else {
            set->val.scnodes[i].in_ctx = LYXP_SET_SCNODE_ATOM_NODE;
        }

        node = set->val.scnodes[i].scnode;

        if (set->val.scnodes[i].type == LYXP_NODE_ELEM) {
            new_node = lysc_data_parent(node);
        } else {
            /* root does not have a parent */
            continue;
        }

        if (!new_node) {
            /* node has no parent */
            new_type = set->root_type;

        } else {
            /* node has a standard parent (it can equal the root, it's not the root yet since they are fake) */
            new_type = LYXP_NODE_ELEM;
        }

        LY_CHECK_RET(lyxp_set_scnode_insert_node(set, new_node, new_type, &idx));
        if ((idx < orig_used) && (idx > i)) {
            set->val.scnodes[idx].in_ctx = LYXP_SET_SCNODE_ATOM_NEW_CTX;
            temp_ctx = 1;
        }
    }

    if (temp_ctx) {
        for (i = 0; i < orig_used; ++i) {
            if (set->val.scnodes[i].in_ctx == LYXP_SET_SCNODE_ATOM_NEW_CTX) {
                set->val.scnodes[i].in_ctx = LYXP_SET_SCNODE_ATOM_CTX;
            }
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Move context @p set to the result of a comparison. Handles '=', '!=', '<=', '<', '>=', or '>'.
 *        Result is LYXP_SET_BOOLEAN. Indirectly context position aware.
 *
 * @param[in,out] set1 Set to use for the result.
 * @param[in] set2 Set acting as the second operand for @p op.
 * @param[in] op Comparison operator to process.
 * @param[in] options XPath options.
 * @return LY_ERR
 */
static LY_ERR
moveto_op_comp(struct lyxp_set *set1, struct lyxp_set *set2, const char *op, uint32_t options)
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
    LY_ERR rc;

    memset(&iter1, 0, sizeof iter1);
    memset(&iter2, 0, sizeof iter2);

    /* iterative evaluation with node-sets */
    if ((set1->type == LYXP_SET_NODE_SET) || (set2->type == LYXP_SET_NODE_SET)) {
        if (set1->type == LYXP_SET_NODE_SET) {
            for (i = 0; i < set1->used; ++i) {
                /* cast set1 */
                switch (set2->type) {
                case LYXP_SET_NUMBER:
                    rc = set_comp_cast(&iter1, set1, LYXP_SET_NUMBER, i);
                    break;
                case LYXP_SET_BOOLEAN:
                    rc = set_comp_cast(&iter1, set1, LYXP_SET_BOOLEAN, i);
                    break;
                default:
                    rc = set_comp_cast(&iter1, set1, LYXP_SET_STRING, i);
                    break;
                }
                LY_CHECK_RET(rc);

                /* canonize set2 */
                LY_CHECK_ERR_RET(rc = set_comp_canonize(&iter2, set2, &set1->val.nodes[i]), lyxp_set_free_content(&iter1), rc);

                /* compare recursively */
                rc = moveto_op_comp(&iter1, &iter2, op, options);
                lyxp_set_free_content(&iter2);
                LY_CHECK_ERR_RET(rc, lyxp_set_free_content(&iter1), rc);

                /* lazy evaluation until true */
                if (iter1.val.bln) {
                    set_fill_boolean(set1, 1);
                    return LY_SUCCESS;
                }
            }
        } else {
            for (i = 0; i < set2->used; ++i) {
                /* set set2 */
                switch (set1->type) {
                case LYXP_SET_NUMBER:
                    rc = set_comp_cast(&iter2, set2, LYXP_SET_NUMBER, i);
                    break;
                case LYXP_SET_BOOLEAN:
                    rc = set_comp_cast(&iter2, set2, LYXP_SET_BOOLEAN, i);
                    break;
                default:
                    rc = set_comp_cast(&iter2, set2, LYXP_SET_STRING, i);
                    break;
                }
                LY_CHECK_RET(rc);

                /* canonize set1 */
                LY_CHECK_ERR_RET(rc = set_comp_canonize(&iter1, set1, &set2->val.nodes[i]), lyxp_set_free_content(&iter2), rc);

                /* compare recursively */
                rc = moveto_op_comp(&iter1, &iter2, op, options);
                lyxp_set_free_content(&iter2);
                LY_CHECK_ERR_RET(rc, lyxp_set_free_content(&iter1), rc);

                /* lazy evaluation until true */
                if (iter1.val.bln) {
                    set_fill_boolean(set1, 1);
                    return LY_SUCCESS;
                }
            }
        }

        /* false for all nodes */
        set_fill_boolean(set1, 0);
        return LY_SUCCESS;
    }

    /* first convert properly */
    if ((op[0] == '=') || (op[0] == '!')) {
        if ((set1->type == LYXP_SET_BOOLEAN) || (set2->type == LYXP_SET_BOOLEAN)) {
            lyxp_set_cast(set1, LYXP_SET_BOOLEAN);
            lyxp_set_cast(set2, LYXP_SET_BOOLEAN);
        } else if ((set1->type == LYXP_SET_NUMBER) || (set2->type == LYXP_SET_NUMBER)) {
            rc = lyxp_set_cast(set1, LYXP_SET_NUMBER);
            LY_CHECK_RET(rc);
            rc = lyxp_set_cast(set2, LYXP_SET_NUMBER);
            LY_CHECK_RET(rc);
        } /* else we have 2 strings */
    } else {
        rc = lyxp_set_cast(set1, LYXP_SET_NUMBER);
        LY_CHECK_RET(rc);
        rc = lyxp_set_cast(set2, LYXP_SET_NUMBER);
        LY_CHECK_RET(rc);
    }

    assert(set1->type == set2->type);

    /* compute result */
    if (op[0] == '=') {
        if (set1->type == LYXP_SET_BOOLEAN) {
            result = (set1->val.bln == set2->val.bln);
        } else if (set1->type == LYXP_SET_NUMBER) {
            result = (set1->val.num == set2->val.num);
        } else {
            assert(set1->type == LYXP_SET_STRING);
            result = !strcmp(set1->val.str, set2->val.str);
        }
    } else if (op[0] == '!') {
        if (set1->type == LYXP_SET_BOOLEAN) {
            result = (set1->val.bln != set2->val.bln);
        } else if (set1->type == LYXP_SET_NUMBER) {
            result = (set1->val.num != set2->val.num);
        } else {
            assert(set1->type == LYXP_SET_STRING);
            result = strcmp(set1->val.str, set2->val.str);
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

    return LY_SUCCESS;
}

/**
 * @brief Move context @p set to the result of a basic operation. Handles '+', '-', unary '-', '*', 'div',
 *        or 'mod'. Result is LYXP_SET_NUMBER. Indirectly context position aware.
 *
 * @param[in,out] set1 Set to use for the result.
 * @param[in] set2 Set acting as the second operand for @p op.
 * @param[in] op Operator to process.
 * @return LY_ERR
 */
static LY_ERR
moveto_op_math(struct lyxp_set *set1, struct lyxp_set *set2, const char *op)
{
    LY_ERR rc;

    /* unary '-' */
    if (!set2 && (op[0] == '-')) {
        rc = lyxp_set_cast(set1, LYXP_SET_NUMBER);
        LY_CHECK_RET(rc);
        set1->val.num *= -1;
        lyxp_set_free(set2);
        return LY_SUCCESS;
    }

    assert(set1 && set2);

    rc = lyxp_set_cast(set1, LYXP_SET_NUMBER);
    LY_CHECK_RET(rc);
    rc = lyxp_set_cast(set2, LYXP_SET_NUMBER);
    LY_CHECK_RET(rc);

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
        LOGINT_RET(set1->ctx);
    }

    return LY_SUCCESS;
}

/*
 * eval functions
 *
 * They execute a parsed XPath expression on some data subtree.
 */

/**
 * @brief Evaluate Predicate. Logs directly on error.
 *
 * [9] Predicate ::= '[' Expr ']'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options XPath options.
 * @param[in] parent_pos_pred Whether parent predicate was a positional one.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
eval_predicate(const struct lyxp_expr *exp, uint16_t *tok_idx, struct lyxp_set *set, uint32_t options, ly_bool parent_pos_pred)
{
    LY_ERR rc;
    uint16_t orig_exp;
    uint32_t i, orig_pos, orig_size;
    int32_t pred_in_ctx;
    struct lyxp_set set2;
    struct lyd_node *orig_parent;

    /* '[' */
    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
            lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
    ++(*tok_idx);

    if (!set) {
only_parse:
        rc = eval_expr_select(exp, tok_idx, 0, NULL, options);
        LY_CHECK_RET(rc);
    } else if (set->type == LYXP_SET_NODE_SET) {
        /* we (possibly) need the set sorted, it can affect the result (if the predicate result is a number) */
        assert(!set_sort(set));

        /* empty set, nothing to evaluate */
        if (!set->used) {
            goto only_parse;
        }

        orig_exp = *tok_idx;
        orig_pos = 0;
        orig_size = set->used;
        orig_parent = NULL;
        for (i = 0; i < set->used; ++i) {
            set_init(&set2, set);
            set_insert_node(&set2, set->val.nodes[i].node, set->val.nodes[i].pos, set->val.nodes[i].type, 0);
            /* remember the node context position for position() and context size for last(),
             * predicates should always be evaluated with respect to the child axis (since we do
             * not support explicit axes) so we assign positions based on their parents */
            if (parent_pos_pred && (lyd_parent(set->val.nodes[i].node) != orig_parent)) {
                orig_parent = lyd_parent(set->val.nodes[i].node);
                orig_pos = 1;
            } else {
                ++orig_pos;
            }

            set2.ctx_pos = orig_pos;
            set2.ctx_size = orig_size;
            *tok_idx = orig_exp;

            rc = eval_expr_select(exp, tok_idx, 0, &set2, options);
            if (rc != LY_SUCCESS) {
                lyxp_set_free_content(&set2);
                return rc;
            }

            /* number is a position */
            if (set2.type == LYXP_SET_NUMBER) {
                if ((long long)set2.val.num == orig_pos) {
                    set2.val.num = 1;
                } else {
                    set2.val.num = 0;
                }
            }
            lyxp_set_cast(&set2, LYXP_SET_BOOLEAN);

            /* predicate satisfied or not? */
            if (!set2.val.bln) {
                set_remove_node_none(set, i);
            }
        }
        set_remove_nodes_none(set);

    } else if (set->type == LYXP_SET_SCNODE_SET) {
        for (i = 0; i < set->used; ++i) {
            if (set->val.scnodes[i].in_ctx == LYXP_SET_SCNODE_ATOM_CTX) {
                /* there is a currently-valid node */
                break;
            }
        }
        /* empty set, nothing to evaluate */
        if (i == set->used) {
            goto only_parse;
        }

        orig_exp = *tok_idx;

        /* set special in_ctx to all the valid snodes */
        pred_in_ctx = set_scnode_new_in_ctx(set);

        /* use the valid snodes one-by-one */
        for (i = 0; i < set->used; ++i) {
            if (set->val.scnodes[i].in_ctx != pred_in_ctx) {
                continue;
            }
            set->val.scnodes[i].in_ctx = LYXP_SET_SCNODE_ATOM_CTX;

            *tok_idx = orig_exp;

            rc = eval_expr_select(exp, tok_idx, 0, set, options);
            LY_CHECK_RET(rc);

            set->val.scnodes[i].in_ctx = pred_in_ctx;
        }

        /* restore the state as it was before the predicate */
        for (i = 0; i < set->used; ++i) {
            if (set->val.scnodes[i].in_ctx == LYXP_SET_SCNODE_ATOM_CTX) {
                set->val.scnodes[i].in_ctx = LYXP_SET_SCNODE_ATOM_NODE;
            } else if (set->val.scnodes[i].in_ctx == pred_in_ctx) {
                set->val.scnodes[i].in_ctx = LYXP_SET_SCNODE_ATOM_CTX;
            }
        }

    } else {
        set2.type = LYXP_SET_NODE_SET;
        set_fill_set(&set2, set);

        rc = eval_expr_select(exp, tok_idx, 0, &set2, options);
        if (rc != LY_SUCCESS) {
            lyxp_set_free_content(&set2);
            return rc;
        }

        lyxp_set_cast(&set2, LYXP_SET_BOOLEAN);
        if (!set2.val.bln) {
            lyxp_set_free_content(set);
        }
        lyxp_set_free_content(&set2);
    }

    /* ']' */
    assert(exp->tokens[*tok_idx] == LYXP_TOKEN_BRACK2);
    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
            lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
    ++(*tok_idx);

    return LY_SUCCESS;
}

/**
 * @brief Evaluate Literal. Logs directly on error.
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 */
static void
eval_literal(const struct lyxp_expr *exp, uint16_t *tok_idx, struct lyxp_set *set)
{
    if (set) {
        if (exp->tok_len[*tok_idx] == 2) {
            set_fill_string(set, "", 0);
        } else {
            set_fill_string(set, &exp->expr[exp->tok_pos[*tok_idx] + 1], exp->tok_len[*tok_idx] - 2);
        }
    }
    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
            lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
    ++(*tok_idx);
}

/**
 * @brief Try to compile list or leaf-list predicate in the known format to be used for hash-based instance search.
 *
 * @param[in] exp Full parsed XPath expression.
 * @param[in,out] tok_idx Index in @p exp at the beginning of the predicate, is updated on success.
 * @param[in] ctx_node Found schema node as the context for the predicate.
 * @param[in] cur_mod Current module for the expression.
 * @param[in] cur_node Current (original context) node.
 * @param[in] format Format of any prefixes in key names/values.
 * @param[in] prefix_data Format-specific prefix data (see ::ly_resolve_prefix).
 * @param[out] predicates Parsed predicates.
 * @param[out] pred_type Type of @p predicates.
 * @return LY_SUCCESS on success,
 * @return LY_ERR on any error.
 */
static LY_ERR
eval_name_test_try_compile_predicates(const struct lyxp_expr *exp, uint16_t *tok_idx, const struct lysc_node *ctx_node,
        const struct lys_module *cur_mod, const struct lysc_node *cur_node, LY_VALUE_FORMAT format, void *prefix_data,
        struct ly_path_predicate **predicates, enum ly_path_pred_type *pred_type)
{
    LY_ERR ret = LY_SUCCESS;
    uint16_t key_count, e_idx, pred_idx = 0;
    const struct lysc_node *key;
    size_t pred_len;
    uint32_t prev_lo;
    struct lyxp_expr *exp2 = NULL;

    assert(ctx_node->nodetype & (LYS_LIST | LYS_LEAFLIST));

    if (ctx_node->nodetype == LYS_LIST) {
        /* get key count */
        if (ctx_node->flags & LYS_KEYLESS) {
            return LY_EINVAL;
        }
        for (key_count = 0, key = lysc_node_child(ctx_node); key && (key->flags & LYS_KEY); key = key->next, ++key_count) {}
        assert(key_count);

        /* learn where the predicates end */
        e_idx = *tok_idx;
        while (key_count) {
            /* '[' */
            if (lyxp_check_token(NULL, exp, e_idx, LYXP_TOKEN_BRACK1)) {
                return LY_EINVAL;
            }
            ++e_idx;

            if (lyxp_check_token(NULL, exp, e_idx, LYXP_TOKEN_NAMETEST)) {
                /* definitely not a key */
                return LY_EINVAL;
            }

            /* ']' */
            while (lyxp_check_token(NULL, exp, e_idx, LYXP_TOKEN_BRACK2)) {
                ++e_idx;
            }
            ++e_idx;

            /* another presumably key predicate parsed */
            --key_count;
        }
    } else {
        /* learn just where this single predicate ends */
        e_idx = *tok_idx;

        /* '[' */
        if (lyxp_check_token(NULL, exp, e_idx, LYXP_TOKEN_BRACK1)) {
            return LY_EINVAL;
        }
        ++e_idx;

        if (lyxp_check_token(NULL, exp, e_idx, LYXP_TOKEN_DOT)) {
            /* definitely not the value */
            return LY_EINVAL;
        }

        /* ']' */
        while (lyxp_check_token(NULL, exp, e_idx, LYXP_TOKEN_BRACK2)) {
            ++e_idx;
        }
        ++e_idx;
    }

    /* get the joined length of all the keys predicates/of the single leaf-list predicate */
    pred_len = (exp->tok_pos[e_idx - 1] + exp->tok_len[e_idx - 1]) - exp->tok_pos[*tok_idx];

    /* turn logging off */
    prev_lo = ly_log_options(0);

    /* parse the predicate(s) */
    LY_CHECK_GOTO(ret = ly_path_parse_predicate(ctx_node->module->ctx, ctx_node, exp->expr + exp->tok_pos[*tok_idx],
            pred_len, LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_SIMPLE, &exp2), cleanup);

    /* compile */
    ret = ly_path_compile_predicate(ctx_node->module->ctx, cur_node, cur_mod, ctx_node, exp2, &pred_idx, format,
            prefix_data, predicates, pred_type);
    LY_CHECK_GOTO(ret, cleanup);

    /* success, the predicate must include all the needed information for hash-based search */
    *tok_idx = e_idx;

cleanup:
    ly_log_options(prev_lo);
    lyxp_expr_free(ctx_node->module->ctx, exp2);
    return ret;
}

/**
 * @brief Search for/check the next schema node that could be the only matching schema node meaning the
 * data node(s) could be found using a single hash-based search.
 *
 * @param[in] ctx libyang context.
 * @param[in] node Next context node to check.
 * @param[in] name Expected node name.
 * @param[in] name_len Length of @p name.
 * @param[in] moveto_mod Expected node module, can be NULL for JSON format with no prefix.
 * @param[in] root_type XPath root type.
 * @param[in] format Prefix format.
 * @param[in,out] found Previously found node, is updated.
 * @return LY_SUCCESS on success,
 * @return LY_ENOT if the whole check failed and hashes cannot be used.
 */
static LY_ERR
eval_name_test_with_predicate_get_scnode(const struct ly_ctx *ctx, const struct lyd_node *node, const char *name,
        uint16_t name_len, const struct lys_module *moveto_mod, enum lyxp_node_type root_type, LY_VALUE_FORMAT format,
        const struct lysc_node **found)
{
    const struct lysc_node *scnode;
    const struct lys_module *mod;
    uint32_t idx = 0;

    assert((format == LY_VALUE_JSON) || moveto_mod);

continue_search:
    scnode = NULL;
    if (!node) {
        if ((format == LY_VALUE_JSON) && !moveto_mod) {
            /* search all modules for a single match */
            while ((mod = ly_ctx_get_module_iter(ctx, &idx))) {
                scnode = lys_find_child(NULL, mod, name, name_len, 0, 0);
                if (scnode) {
                    /* we have found a match */
                    break;
                }
            }

            if (!scnode) {
                /* all modules searched */
                idx = 0;
            }
        } else {
            /* search in top-level */
            scnode = lys_find_child(NULL, moveto_mod, name, name_len, 0, 0);
        }
    } else if (!*found || (lysc_data_parent(*found) != node->schema)) {
        if ((format == LY_VALUE_JSON) && !moveto_mod) {
            /* we must adjust the module to inherit the one from the context node */
            moveto_mod = node->schema->module;
        }

        /* search in children, do not repeat the same search */
        scnode = lys_find_child(node->schema, moveto_mod, name, name_len, 0, 0);
    } /* else skip redundant search */

    /* additional context check */
    if (scnode && (root_type == LYXP_NODE_ROOT_CONFIG) && (scnode->flags & LYS_CONFIG_R)) {
        scnode = NULL;
    }

    if (scnode) {
        if (*found) {
            /* we found a schema node with the same name but at different level, give up, too complicated
             * (more hash-based searches would be required, not supported) */
            return LY_ENOT;
        } else {
            /* remember the found schema node and continue to make sure it can be used */
            *found = scnode;
        }
    }

    if (idx) {
        /* continue searching all the following models */
        goto continue_search;
    }

    return LY_SUCCESS;
}

/**
 * @brief Evaluate NameTest and any following Predicates. Logs directly on error.
 *
 * [5] Step ::= '@'? NodeTest Predicate* | '.' | '..'
 * [6] NodeTest ::= NameTest | NodeType '(' ')'
 * [7] NameTest ::= '*' | NCName ':' '*' | QName
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in] attr_axis Whether to search attributes or standard nodes.
 * @param[in] all_desc Whether to search all the descendants or children only.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
eval_name_test_with_predicate(const struct lyxp_expr *exp, uint16_t *tok_idx, ly_bool attr_axis, ly_bool all_desc,
        struct lyxp_set *set, uint32_t options)
{
    char *path;
    const char *ncname, *ncname_dict = NULL;
    uint16_t ncname_len;
    const struct lys_module *moveto_mod = NULL;
    const struct lysc_node *scnode = NULL;
    struct ly_path_predicate *predicates = NULL;
    enum ly_path_pred_type pred_type = 0;
    LY_ERR rc = LY_SUCCESS;

    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
            lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
    ++(*tok_idx);

    if (!set) {
        goto moveto;
    }

    ncname = &exp->expr[exp->tok_pos[*tok_idx - 1]];
    ncname_len = exp->tok_len[*tok_idx - 1];

    if ((ncname[0] == '*') && (ncname_len == 1)) {
        /* all nodes will match */
        goto moveto;
    }

    /* parse (and skip) module name */
    rc = moveto_resolve_model(&ncname, &ncname_len, set, NULL, &moveto_mod);
    LY_CHECK_GOTO(rc, cleanup);

    if (((set->format == LY_VALUE_JSON) || moveto_mod) && !attr_axis && !all_desc && (set->type == LYXP_SET_NODE_SET)) {
        /* find the matching schema node in some parent in the context */
        for (uint32_t i = 0; i < set->used; ++i) {
            if (eval_name_test_with_predicate_get_scnode(set->ctx, set->val.nodes[i].node, ncname, ncname_len,
                    moveto_mod, set->root_type, set->format, &scnode)) {
                /* check failed */
                scnode = NULL;
                break;
            }
        }

        if (scnode && (scnode->nodetype & (LYS_LIST | LYS_LEAFLIST))) {
            /* try to create the predicates */
            if (eval_name_test_try_compile_predicates(exp, tok_idx, scnode, set->cur_mod, set->cur_node ?
                    set->cur_node->schema : NULL, set->format, set->prefix_data, &predicates, &pred_type)) {
                /* hashes cannot be used */
                scnode = NULL;
            }
        }
    }

    if (!scnode) {
        /* we are not able to match based on a schema node and not all the modules match ("*"),
         * use dictionary for efficient comparison */
        LY_CHECK_GOTO(rc = lydict_insert(set->ctx, ncname, ncname_len, &ncname_dict), cleanup);
    }

moveto:
    /* move to the attribute(s), data node(s), or schema node(s) */
    if (attr_axis) {
        if (set && (options & LYXP_SCNODE_ALL)) {
            set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_NODE);
        } else {
            if (all_desc) {
                rc = moveto_attr_alldesc(set, moveto_mod, ncname_dict, options);
            } else {
                rc = moveto_attr(set, moveto_mod, ncname_dict);
            }
            LY_CHECK_GOTO(rc, cleanup);
        }
    } else {
        if (set && (options & LYXP_SCNODE_ALL)) {
            int64_t i;

            if (all_desc) {
                rc = moveto_scnode_alldesc(set, moveto_mod, ncname_dict, options);
            } else {
                rc = moveto_scnode(set, moveto_mod, ncname_dict, options);
            }
            LY_CHECK_GOTO(rc, cleanup);

            for (i = set->used - 1; i > -1; --i) {
                if (set->val.scnodes[i].in_ctx > LYXP_SET_SCNODE_ATOM_NODE) {
                    break;
                }
            }
            if (i == -1) {
                path = lysc_path(set->cur_scnode, LYSC_PATH_LOG, NULL, 0);
                LOGWRN(set->ctx, "Schema node \"%.*s\" not found (\"%.*s\") with context node \"%s\".",
                        ncname_len, ncname, (ncname - exp->expr) + ncname_len, exp->expr, path);
                free(path);
            }
        } else {
            if (all_desc) {
                rc = moveto_node_alldesc(set, moveto_mod, ncname_dict, options);
            } else {
                if (scnode) {
                    /* we can find the nodes using hashes */
                    rc = moveto_node_hash(set, scnode, predicates, options);
                } else {
                    rc = moveto_node(set, moveto_mod, ncname_dict, options);
                }
            }
            LY_CHECK_GOTO(rc, cleanup);
        }
    }

    /* Predicate* */
    while (!lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_BRACK1)) {
        rc = eval_predicate(exp, tok_idx, set, options, 1);
        LY_CHECK_RET(rc);
    }

cleanup:
    if (set) {
        lydict_remove(set->ctx, ncname_dict);
        ly_path_predicates_free(set->ctx, pred_type, predicates);
    }
    return rc;
}

/**
 * @brief Evaluate NodeType and any following Predicates. Logs directly on error.
 *
 * [5] Step ::= '@'? NodeTest Predicate* | '.' | '..'
 * [6] NodeTest ::= NameTest | NodeType '(' ')'
 * [8] NodeType ::= 'text' | 'node'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in] attr_axis Whether to search attributes or standard nodes.
 * @param[in] all_desc Whether to search all the descendants or children only.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
eval_node_type_with_predicate(const struct lyxp_expr *exp, uint16_t *tok_idx, ly_bool attr_axis, ly_bool all_desc,
        struct lyxp_set *set, uint32_t options)
{
    LY_ERR rc;

    /* TODO */
    (void)attr_axis;
    (void)all_desc;

    if (set) {
        assert(exp->tok_len[*tok_idx] == 4);
        if (set->type == LYXP_SET_SCNODE_SET) {
            set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_NODE);
            /* just for the debug message below */
            set = NULL;
        } else {
            if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "node", 4)) {
                rc = xpath_node(NULL, 0, set, options);
            } else {
                assert(!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "text", 4));
                rc = xpath_text(NULL, 0, set, options);
            }
            LY_CHECK_RET(rc);
        }
    }
    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
            lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
    ++(*tok_idx);

    /* '(' */
    assert(exp->tokens[*tok_idx] == LYXP_TOKEN_PAR1);
    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
            lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
    ++(*tok_idx);

    /* ')' */
    assert(exp->tokens[*tok_idx] == LYXP_TOKEN_PAR2);
    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
            lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
    ++(*tok_idx);

    /* Predicate* */
    while (!lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_BRACK1)) {
        rc = eval_predicate(exp, tok_idx, set, options, 1);
        LY_CHECK_RET(rc);
    }

    return LY_SUCCESS;
}

/**
 * @brief Evaluate RelativeLocationPath. Logs directly on error.
 *
 * [4] RelativeLocationPath ::= Step | RelativeLocationPath '/' Step | RelativeLocationPath '//' Step
 * [5] Step ::= '@'? NodeTest Predicate* | '.' | '..'
 * [6] NodeTest ::= NameTest | NodeType '(' ')'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in] all_desc Whether to search all the descendants or children only.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options XPath options.
 * @return LY_ERR (YL_EINCOMPLETE on unresolved when)
 */
static LY_ERR
eval_relative_location_path(const struct lyxp_expr *exp, uint16_t *tok_idx, ly_bool all_desc, struct lyxp_set *set,
        uint32_t options)
{
    ly_bool attr_axis;
    LY_ERR rc;

    goto step;
    do {
        /* evaluate '/' or '//' */
        if (exp->tok_len[*tok_idx] == 1) {
            all_desc = 0;
        } else {
            assert(exp->tok_len[*tok_idx] == 2);
            all_desc = 1;
        }
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
                lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
        ++(*tok_idx);

step:
        /* evaluate abbreviated axis '@'? if any */
        if (exp->tokens[*tok_idx] == LYXP_TOKEN_AT) {
            attr_axis = 1;

            LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
                    lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
            ++(*tok_idx);
        } else {
            attr_axis = 0;
        }

        /* Step */
        switch (exp->tokens[*tok_idx]) {
        case LYXP_TOKEN_DOT:
            /* evaluate '.' */
            if (set && (options & LYXP_SCNODE_ALL)) {
                rc = moveto_scnode_self(set, all_desc, options);
            } else {
                rc = moveto_self(set, all_desc, options);
            }
            LY_CHECK_RET(rc);
            LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
                    lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
            ++(*tok_idx);
            break;

        case LYXP_TOKEN_DDOT:
            /* evaluate '..' */
            if (set && (options & LYXP_SCNODE_ALL)) {
                rc = moveto_scnode_parent(set, all_desc, options);
            } else {
                rc = moveto_parent(set, all_desc, options);
            }
            LY_CHECK_RET(rc);
            LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
                    lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
            ++(*tok_idx);
            break;

        case LYXP_TOKEN_NAMETEST:
            /* evaluate NameTest Predicate* */
            rc = eval_name_test_with_predicate(exp, tok_idx, attr_axis, all_desc, set, options);
            LY_CHECK_RET(rc);
            break;

        case LYXP_TOKEN_NODETYPE:
            /* evaluate NodeType Predicate* */
            rc = eval_node_type_with_predicate(exp, tok_idx, attr_axis, all_desc, set, options);
            LY_CHECK_RET(rc);
            break;

        default:
            LOGINT_RET(set ? set->ctx : NULL);
        }
    } while (!exp_check_token2(NULL, exp, *tok_idx, LYXP_TOKEN_OPER_PATH, LYXP_TOKEN_OPER_RPATH));

    return LY_SUCCESS;
}

/**
 * @brief Evaluate AbsoluteLocationPath. Logs directly on error.
 *
 * [3] AbsoluteLocationPath ::= '/' RelativeLocationPath? | '//' RelativeLocationPath
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
eval_absolute_location_path(const struct lyxp_expr *exp, uint16_t *tok_idx, struct lyxp_set *set, uint32_t options)
{
    ly_bool all_desc;

    if (set) {
        /* no matter what tokens follow, we need to be at the root */
        LY_CHECK_RET(moveto_root(set, options));
    }

    /* '/' RelativeLocationPath? */
    if (exp->tok_len[*tok_idx] == 1) {
        /* evaluate '/' - deferred */
        all_desc = 0;
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
                lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
        ++(*tok_idx);

        if (lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_NONE)) {
            return LY_SUCCESS;
        }
        switch (exp->tokens[*tok_idx]) {
        case LYXP_TOKEN_DOT:
        case LYXP_TOKEN_DDOT:
        case LYXP_TOKEN_AT:
        case LYXP_TOKEN_NAMETEST:
        case LYXP_TOKEN_NODETYPE:
            LY_CHECK_RET(eval_relative_location_path(exp, tok_idx, all_desc, set, options));
            break;
        default:
            break;
        }

    } else {
        /* '//' RelativeLocationPath */
        /* evaluate '//' - deferred so as not to waste memory by remembering all the nodes */
        all_desc = 1;
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
                lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
        ++(*tok_idx);

        LY_CHECK_RET(eval_relative_location_path(exp, tok_idx, all_desc, set, options));
    }

    return LY_SUCCESS;
}

/**
 * @brief Evaluate FunctionCall. Logs directly on error.
 *
 * [11] FunctionCall ::= FunctionName '(' ( Expr ( ',' Expr )* )? ')'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
eval_function_call(const struct lyxp_expr *exp, uint16_t *tok_idx, struct lyxp_set *set, uint32_t options)
{
    LY_ERR rc;

    LY_ERR (*xpath_func)(struct lyxp_set **, uint16_t, struct lyxp_set *, uint32_t) = NULL;
    uint16_t arg_count = 0, i;
    struct lyxp_set **args = NULL, **args_aux;

    if (set) {
        /* FunctionName */
        switch (exp->tok_len[*tok_idx]) {
        case 3:
            if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "not", 3)) {
                xpath_func = &xpath_not;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "sum", 3)) {
                xpath_func = &xpath_sum;
            }
            break;
        case 4:
            if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "lang", 4)) {
                xpath_func = &xpath_lang;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "last", 4)) {
                xpath_func = &xpath_last;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "name", 4)) {
                xpath_func = &xpath_name;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "true", 4)) {
                xpath_func = &xpath_true;
            }
            break;
        case 5:
            if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "count", 5)) {
                xpath_func = &xpath_count;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "false", 5)) {
                xpath_func = &xpath_false;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "floor", 5)) {
                xpath_func = &xpath_floor;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "round", 5)) {
                xpath_func = &xpath_round;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "deref", 5)) {
                xpath_func = &xpath_deref;
            }
            break;
        case 6:
            if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "concat", 6)) {
                xpath_func = &xpath_concat;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "number", 6)) {
                xpath_func = &xpath_number;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "string", 6)) {
                xpath_func = &xpath_string;
            }
            break;
        case 7:
            if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "boolean", 7)) {
                xpath_func = &xpath_boolean;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "ceiling", 7)) {
                xpath_func = &xpath_ceiling;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "current", 7)) {
                xpath_func = &xpath_current;
            }
            break;
        case 8:
            if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "contains", 8)) {
                xpath_func = &xpath_contains;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "position", 8)) {
                xpath_func = &xpath_position;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "re-match", 8)) {
                xpath_func = &xpath_re_match;
            }
            break;
        case 9:
            if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "substring", 9)) {
                xpath_func = &xpath_substring;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "translate", 9)) {
                xpath_func = &xpath_translate;
            }
            break;
        case 10:
            if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "local-name", 10)) {
                xpath_func = &xpath_local_name;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "enum-value", 10)) {
                xpath_func = &xpath_enum_value;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "bit-is-set", 10)) {
                xpath_func = &xpath_bit_is_set;
            }
            break;
        case 11:
            if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "starts-with", 11)) {
                xpath_func = &xpath_starts_with;
            }
            break;
        case 12:
            if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "derived-from", 12)) {
                xpath_func = &xpath_derived_from;
            }
            break;
        case 13:
            if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "namespace-uri", 13)) {
                xpath_func = &xpath_namespace_uri;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "string-length", 13)) {
                xpath_func = &xpath_string_length;
            }
            break;
        case 15:
            if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "normalize-space", 15)) {
                xpath_func = &xpath_normalize_space;
            } else if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "substring-after", 15)) {
                xpath_func = &xpath_substring_after;
            }
            break;
        case 16:
            if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "substring-before", 16)) {
                xpath_func = &xpath_substring_before;
            }
            break;
        case 20:
            if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "derived-from-or-self", 20)) {
                xpath_func = &xpath_derived_from_or_self;
            }
            break;
        }

        if (!xpath_func) {
            LOGVAL(set->ctx, LY_VCODE_XP_INFUNC, exp->tok_len[*tok_idx], &exp->expr[exp->tok_pos[*tok_idx]]);
            return LY_EVALID;
        }
    }

    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
            lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
    ++(*tok_idx);

    /* '(' */
    assert(exp->tokens[*tok_idx] == LYXP_TOKEN_PAR1);
    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
            lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
    ++(*tok_idx);

    /* ( Expr ( ',' Expr )* )? */
    if (exp->tokens[*tok_idx] != LYXP_TOKEN_PAR2) {
        if (set) {
            args = malloc(sizeof *args);
            LY_CHECK_ERR_GOTO(!args, LOGMEM(set->ctx); rc = LY_EMEM, cleanup);
            arg_count = 1;
            args[0] = set_copy(set);
            if (!args[0]) {
                rc = LY_EMEM;
                goto cleanup;
            }

            rc = eval_expr_select(exp, tok_idx, 0, args[0], options);
            LY_CHECK_GOTO(rc, cleanup);
        } else {
            rc = eval_expr_select(exp, tok_idx, 0, NULL, options);
            LY_CHECK_GOTO(rc, cleanup);
        }
    }
    while (!lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_COMMA)) {
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
                lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
        ++(*tok_idx);

        if (set) {
            ++arg_count;
            args_aux = realloc(args, arg_count * sizeof *args);
            LY_CHECK_ERR_GOTO(!args_aux, arg_count--; LOGMEM(set->ctx); rc = LY_EMEM, cleanup);
            args = args_aux;
            args[arg_count - 1] = set_copy(set);
            if (!args[arg_count - 1]) {
                rc = LY_EMEM;
                goto cleanup;
            }

            rc = eval_expr_select(exp, tok_idx, 0, args[arg_count - 1], options);
            LY_CHECK_GOTO(rc, cleanup);
        } else {
            rc = eval_expr_select(exp, tok_idx, 0, NULL, options);
            LY_CHECK_GOTO(rc, cleanup);
        }
    }

    /* ')' */
    assert(exp->tokens[*tok_idx] == LYXP_TOKEN_PAR2);
    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
            lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
    ++(*tok_idx);

    if (set) {
        /* evaluate function */
        rc = xpath_func(args, arg_count, set, options);

        if (options & LYXP_SCNODE_ALL) {
            /* merge all nodes from arg evaluations */
            for (i = 0; i < arg_count; ++i) {
                set_scnode_clear_ctx(args[i], LYXP_SET_SCNODE_ATOM_NODE);
                lyxp_set_scnode_merge(set, args[i]);
            }
        }
    } else {
        rc = LY_SUCCESS;
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
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @return LY_ERR
 */
static LY_ERR
eval_number(struct ly_ctx *ctx, const struct lyxp_expr *exp, uint16_t *tok_idx, struct lyxp_set *set)
{
    long double num;
    char *endptr;

    if (set) {
        errno = 0;
        num = strtold(&exp->expr[exp->tok_pos[*tok_idx]], &endptr);
        if (errno) {
            LOGVAL(ctx, LY_VCODE_XP_INTOK, "Unknown", &exp->expr[exp->tok_pos[*tok_idx]]);
            LOGVAL(ctx, LYVE_XPATH, "Failed to convert \"%.*s\" into a long double (%s).",
                    exp->tok_len[*tok_idx], &exp->expr[exp->tok_pos[*tok_idx]], strerror(errno));
            return LY_EVALID;
        } else if (endptr - &exp->expr[exp->tok_pos[*tok_idx]] != exp->tok_len[*tok_idx]) {
            LOGVAL(ctx, LY_VCODE_XP_INTOK, "Unknown", &exp->expr[exp->tok_pos[*tok_idx]]);
            LOGVAL(ctx, LYVE_XPATH, "Failed to convert \"%.*s\" into a long double.",
                    exp->tok_len[*tok_idx], &exp->expr[exp->tok_pos[*tok_idx]]);
            return LY_EVALID;
        }

        set_fill_number(set, num);
    }

    LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
            lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
    ++(*tok_idx);
    return LY_SUCCESS;
}

/**
 * @brief Evaluate PathExpr. Logs directly on error.
 *
 * [12] PathExpr ::= LocationPath | PrimaryExpr Predicate*
 *                 | PrimaryExpr Predicate* '/' RelativeLocationPath
 *                 | PrimaryExpr Predicate* '//' RelativeLocationPath
 * [2] LocationPath ::= RelativeLocationPath | AbsoluteLocationPath
 * [10] PrimaryExpr ::= '(' Expr ')' | Literal | Number | FunctionCall
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
eval_path_expr(const struct lyxp_expr *exp, uint16_t *tok_idx, struct lyxp_set *set, uint32_t options)
{
    ly_bool all_desc, parent_pos_pred;
    LY_ERR rc;

    switch (exp->tokens[*tok_idx]) {
    case LYXP_TOKEN_PAR1:
        /* '(' Expr ')' */

        /* '(' */
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
                lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
        ++(*tok_idx);

        /* Expr */
        rc = eval_expr_select(exp, tok_idx, 0, set, options);
        LY_CHECK_RET(rc);

        /* ')' */
        assert(exp->tokens[*tok_idx] == LYXP_TOKEN_PAR2);
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
                lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
        ++(*tok_idx);

        parent_pos_pred = 0;
        goto predicate;

    case LYXP_TOKEN_DOT:
    case LYXP_TOKEN_DDOT:
    case LYXP_TOKEN_AT:
    case LYXP_TOKEN_NAMETEST:
    case LYXP_TOKEN_NODETYPE:
        /* RelativeLocationPath */
        rc = eval_relative_location_path(exp, tok_idx, 0, set, options);
        LY_CHECK_RET(rc);
        break;

    case LYXP_TOKEN_FUNCNAME:
        /* FunctionCall */
        if (!set) {
            rc = eval_function_call(exp, tok_idx, NULL, options);
        } else {
            rc = eval_function_call(exp, tok_idx, set, options);
        }
        LY_CHECK_RET(rc);

        parent_pos_pred = 1;
        goto predicate;

    case LYXP_TOKEN_OPER_PATH:
    case LYXP_TOKEN_OPER_RPATH:
        /* AbsoluteLocationPath */
        rc = eval_absolute_location_path(exp, tok_idx, set, options);
        LY_CHECK_RET(rc);
        break;

    case LYXP_TOKEN_LITERAL:
        /* Literal */
        if (!set || (options & LYXP_SCNODE_ALL)) {
            if (set) {
                set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
            }
            eval_literal(exp, tok_idx, NULL);
        } else {
            eval_literal(exp, tok_idx, set);
        }

        parent_pos_pred = 1;
        goto predicate;

    case LYXP_TOKEN_NUMBER:
        /* Number */
        if (!set || (options & LYXP_SCNODE_ALL)) {
            if (set) {
                set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
            }
            rc = eval_number(NULL, exp, tok_idx, NULL);
        } else {
            rc = eval_number(set->ctx, exp, tok_idx, set);
        }
        LY_CHECK_RET(rc);

        parent_pos_pred = 1;
        goto predicate;

    default:
        LOGVAL(set->ctx, LY_VCODE_XP_INTOK, lyxp_print_token(exp->tokens[*tok_idx]), &exp->expr[exp->tok_pos[*tok_idx]]);
        return LY_EVALID;
    }

    return LY_SUCCESS;

predicate:
    /* Predicate* */
    while (!lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_BRACK1)) {
        rc = eval_predicate(exp, tok_idx, set, options, parent_pos_pred);
        LY_CHECK_RET(rc);
    }

    /* ('/' or '//') RelativeLocationPath */
    if (!exp_check_token2(NULL, exp, *tok_idx, LYXP_TOKEN_OPER_PATH, LYXP_TOKEN_OPER_RPATH)) {

        /* evaluate '/' or '//' */
        if (exp->tokens[*tok_idx] == LYXP_TOKEN_OPER_PATH) {
            all_desc = 0;
        } else {
            all_desc = 1;
        }

        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
                lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
        ++(*tok_idx);

        rc = eval_relative_location_path(exp, tok_idx, all_desc, set, options);
        LY_CHECK_RET(rc);
    }

    return LY_SUCCESS;
}

/**
 * @brief Evaluate UnionExpr. Logs directly on error.
 *
 * [20] UnionExpr ::= PathExpr | UnionExpr '|' PathExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in] repeat How many times this expression is repeated.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
eval_union_expr(const struct lyxp_expr *exp, uint16_t *tok_idx, uint16_t repeat, struct lyxp_set *set, uint32_t options)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyxp_set orig_set, set2;
    uint16_t i;

    assert(repeat);

    set_init(&orig_set, set);
    set_init(&set2, set);

    set_fill_set(&orig_set, set);

    rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_UNION, set, options);
    LY_CHECK_GOTO(rc, cleanup);

    /* ('|' PathExpr)* */
    for (i = 0; i < repeat; ++i) {
        assert(exp->tokens[*tok_idx] == LYXP_TOKEN_OPER_UNI);
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
                lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
        ++(*tok_idx);

        if (!set) {
            rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_UNION, NULL, options);
            LY_CHECK_GOTO(rc, cleanup);
            continue;
        }

        set_fill_set(&set2, &orig_set);
        rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_UNION, &set2, options);
        LY_CHECK_GOTO(rc, cleanup);

        /* eval */
        if (options & LYXP_SCNODE_ALL) {
            lyxp_set_scnode_merge(set, &set2);
        } else {
            rc = moveto_union(set, &set2);
            LY_CHECK_GOTO(rc, cleanup);
        }
    }

cleanup:
    lyxp_set_free_content(&orig_set);
    lyxp_set_free_content(&set2);
    return rc;
}

/**
 * @brief Evaluate UnaryExpr. Logs directly on error.
 *
 * [19] UnaryExpr ::= UnionExpr | '-' UnaryExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in] repeat How many times this expression is repeated.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
eval_unary_expr(const struct lyxp_expr *exp, uint16_t *tok_idx, uint16_t repeat, struct lyxp_set *set, uint32_t options)
{
    LY_ERR rc;
    uint16_t this_op, i;

    assert(repeat);

    /* ('-')+ */
    this_op = *tok_idx;
    for (i = 0; i < repeat; ++i) {
        assert(!lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_OPER_MATH) && (exp->expr[exp->tok_pos[*tok_idx]] == '-'));

        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
                lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
        ++(*tok_idx);
    }

    rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_UNARY, set, options);
    LY_CHECK_RET(rc);

    if (set && (repeat % 2)) {
        if (options & LYXP_SCNODE_ALL) {
            warn_operands(set->ctx, set, NULL, 1, exp->expr, exp->tok_pos[this_op]);
        } else {
            rc = moveto_op_math(set, NULL, &exp->expr[exp->tok_pos[this_op]]);
            LY_CHECK_RET(rc);
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Evaluate MultiplicativeExpr. Logs directly on error.
 *
 * [18] MultiplicativeExpr ::= UnaryExpr
 *                     | MultiplicativeExpr '*' UnaryExpr
 *                     | MultiplicativeExpr 'div' UnaryExpr
 *                     | MultiplicativeExpr 'mod' UnaryExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in] repeat How many times this expression is repeated.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
eval_multiplicative_expr(const struct lyxp_expr *exp, uint16_t *tok_idx, uint16_t repeat, struct lyxp_set *set,
        uint32_t options)
{
    LY_ERR rc;
    uint16_t this_op;
    struct lyxp_set orig_set, set2;
    uint16_t i;

    assert(repeat);

    set_init(&orig_set, set);
    set_init(&set2, set);

    set_fill_set(&orig_set, set);

    rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_MULTIPLICATIVE, set, options);
    LY_CHECK_GOTO(rc, cleanup);

    /* ('*' / 'div' / 'mod' UnaryExpr)* */
    for (i = 0; i < repeat; ++i) {
        this_op = *tok_idx;

        assert(exp->tokens[*tok_idx] == LYXP_TOKEN_OPER_MATH);
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
                lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
        ++(*tok_idx);

        if (!set) {
            rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_MULTIPLICATIVE, NULL, options);
            LY_CHECK_GOTO(rc, cleanup);
            continue;
        }

        set_fill_set(&set2, &orig_set);
        rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_MULTIPLICATIVE, &set2, options);
        LY_CHECK_GOTO(rc, cleanup);

        /* eval */
        if (options & LYXP_SCNODE_ALL) {
            warn_operands(set->ctx, set, &set2, 1, exp->expr, exp->tok_pos[this_op - 1]);
            lyxp_set_scnode_merge(set, &set2);
            set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        } else {
            rc = moveto_op_math(set, &set2, &exp->expr[exp->tok_pos[this_op]]);
            LY_CHECK_GOTO(rc, cleanup);
        }
    }

cleanup:
    lyxp_set_free_content(&orig_set);
    lyxp_set_free_content(&set2);
    return rc;
}

/**
 * @brief Evaluate AdditiveExpr. Logs directly on error.
 *
 * [17] AdditiveExpr ::= MultiplicativeExpr
 *                     | AdditiveExpr '+' MultiplicativeExpr
 *                     | AdditiveExpr '-' MultiplicativeExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in] repeat How many times this expression is repeated.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
eval_additive_expr(const struct lyxp_expr *exp, uint16_t *tok_idx, uint16_t repeat, struct lyxp_set *set, uint32_t options)
{
    LY_ERR rc;
    uint16_t this_op;
    struct lyxp_set orig_set, set2;
    uint16_t i;

    assert(repeat);

    set_init(&orig_set, set);
    set_init(&set2, set);

    set_fill_set(&orig_set, set);

    rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_ADDITIVE, set, options);
    LY_CHECK_GOTO(rc, cleanup);

    /* ('+' / '-' MultiplicativeExpr)* */
    for (i = 0; i < repeat; ++i) {
        this_op = *tok_idx;

        assert(exp->tokens[*tok_idx] == LYXP_TOKEN_OPER_MATH);
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
                lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
        ++(*tok_idx);

        if (!set) {
            rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_ADDITIVE, NULL, options);
            LY_CHECK_GOTO(rc, cleanup);
            continue;
        }

        set_fill_set(&set2, &orig_set);
        rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_ADDITIVE, &set2, options);
        LY_CHECK_GOTO(rc, cleanup);

        /* eval */
        if (options & LYXP_SCNODE_ALL) {
            warn_operands(set->ctx, set, &set2, 1, exp->expr, exp->tok_pos[this_op - 1]);
            lyxp_set_scnode_merge(set, &set2);
            set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        } else {
            rc = moveto_op_math(set, &set2, &exp->expr[exp->tok_pos[this_op]]);
            LY_CHECK_GOTO(rc, cleanup);
        }
    }

cleanup:
    lyxp_set_free_content(&orig_set);
    lyxp_set_free_content(&set2);
    return rc;
}

/**
 * @brief Evaluate RelationalExpr. Logs directly on error.
 *
 * [16] RelationalExpr ::= AdditiveExpr
 *                       | RelationalExpr '<' AdditiveExpr
 *                       | RelationalExpr '>' AdditiveExpr
 *                       | RelationalExpr '<=' AdditiveExpr
 *                       | RelationalExpr '>=' AdditiveExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in] repeat How many times this expression is repeated.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
eval_relational_expr(const struct lyxp_expr *exp, uint16_t *tok_idx, uint16_t repeat, struct lyxp_set *set, uint32_t options)
{
    LY_ERR rc;
    uint16_t this_op;
    struct lyxp_set orig_set, set2;
    uint16_t i;

    assert(repeat);

    set_init(&orig_set, set);
    set_init(&set2, set);

    set_fill_set(&orig_set, set);

    rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_RELATIONAL, set, options);
    LY_CHECK_GOTO(rc, cleanup);

    /* ('<' / '>' / '<=' / '>=' AdditiveExpr)* */
    for (i = 0; i < repeat; ++i) {
        this_op = *tok_idx;

        assert(exp->tokens[*tok_idx] == LYXP_TOKEN_OPER_COMP);
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
                lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
        ++(*tok_idx);

        if (!set) {
            rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_RELATIONAL, NULL, options);
            LY_CHECK_GOTO(rc, cleanup);
            continue;
        }

        set_fill_set(&set2, &orig_set);
        rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_RELATIONAL, &set2, options);
        LY_CHECK_GOTO(rc, cleanup);

        /* eval */
        if (options & LYXP_SCNODE_ALL) {
            warn_operands(set->ctx, set, &set2, 1, exp->expr, exp->tok_pos[this_op - 1]);
            lyxp_set_scnode_merge(set, &set2);
            set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        } else {
            rc = moveto_op_comp(set, &set2, &exp->expr[exp->tok_pos[this_op]], options);
            LY_CHECK_GOTO(rc, cleanup);
        }
    }

cleanup:
    lyxp_set_free_content(&orig_set);
    lyxp_set_free_content(&set2);
    return rc;
}

/**
 * @brief Evaluate EqualityExpr. Logs directly on error.
 *
 * [15] EqualityExpr ::= RelationalExpr | EqualityExpr '=' RelationalExpr
 *                     | EqualityExpr '!=' RelationalExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in] repeat How many times this expression is repeated.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
eval_equality_expr(const struct lyxp_expr *exp, uint16_t *tok_idx, uint16_t repeat, struct lyxp_set *set, uint32_t options)
{
    LY_ERR rc;
    uint16_t this_op;
    struct lyxp_set orig_set, set2;
    uint16_t i;

    assert(repeat);

    set_init(&orig_set, set);
    set_init(&set2, set);

    set_fill_set(&orig_set, set);

    rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_EQUALITY, set, options);
    LY_CHECK_GOTO(rc, cleanup);

    /* ('=' / '!=' RelationalExpr)* */
    for (i = 0; i < repeat; ++i) {
        this_op = *tok_idx;

        assert((exp->tokens[*tok_idx] == LYXP_TOKEN_OPER_EQUAL) || (exp->tokens[*tok_idx] == LYXP_TOKEN_OPER_NEQUAL));
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
                lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
        ++(*tok_idx);

        if (!set) {
            rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_EQUALITY, NULL, options);
            LY_CHECK_GOTO(rc, cleanup);
            continue;
        }

        set_fill_set(&set2, &orig_set);
        rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_EQUALITY, &set2, options);
        LY_CHECK_GOTO(rc, cleanup);

        /* eval */
        if (options & LYXP_SCNODE_ALL) {
            warn_operands(set->ctx, set, &set2, 0, exp->expr, exp->tok_pos[this_op - 1]);
            warn_equality_value(exp, set, *tok_idx - 1, this_op - 1, *tok_idx - 1);
            warn_equality_value(exp, &set2, this_op - 1, this_op - 1, *tok_idx - 1);
            lyxp_set_scnode_merge(set, &set2);
            set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_VAL);
        } else {
            rc = moveto_op_comp(set, &set2, &exp->expr[exp->tok_pos[this_op]], options);
            LY_CHECK_GOTO(rc, cleanup);
        }
    }

cleanup:
    lyxp_set_free_content(&orig_set);
    lyxp_set_free_content(&set2);
    return rc;
}

/**
 * @brief Evaluate AndExpr. Logs directly on error.
 *
 * [14] AndExpr ::= EqualityExpr | AndExpr 'and' EqualityExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in] repeat How many times this expression is repeated.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
eval_and_expr(const struct lyxp_expr *exp, uint16_t *tok_idx, uint16_t repeat, struct lyxp_set *set, uint32_t options)
{
    LY_ERR rc;
    struct lyxp_set orig_set, set2;
    uint16_t i;

    assert(repeat);

    set_init(&orig_set, set);
    set_init(&set2, set);

    set_fill_set(&orig_set, set);

    rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_AND, set, options);
    LY_CHECK_GOTO(rc, cleanup);

    /* cast to boolean, we know that will be the final result */
    if (set && (options & LYXP_SCNODE_ALL)) {
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_NODE);
    } else {
        lyxp_set_cast(set, LYXP_SET_BOOLEAN);
    }

    /* ('and' EqualityExpr)* */
    for (i = 0; i < repeat; ++i) {
        assert(exp->tokens[*tok_idx] == LYXP_TOKEN_OPER_LOG);
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (!set || !set->val.bln ? "skipped" : "parsed"),
                lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
        ++(*tok_idx);

        /* lazy evaluation */
        if (!set || ((set->type == LYXP_SET_BOOLEAN) && !set->val.bln)) {
            rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_AND, NULL, options);
            LY_CHECK_GOTO(rc, cleanup);
            continue;
        }

        set_fill_set(&set2, &orig_set);
        rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_AND, &set2, options);
        LY_CHECK_GOTO(rc, cleanup);

        /* eval - just get boolean value actually */
        if (set->type == LYXP_SET_SCNODE_SET) {
            set_scnode_clear_ctx(&set2, LYXP_SET_SCNODE_ATOM_NODE);
            lyxp_set_scnode_merge(set, &set2);
        } else {
            lyxp_set_cast(&set2, LYXP_SET_BOOLEAN);
            set_fill_set(set, &set2);
        }
    }

cleanup:
    lyxp_set_free_content(&orig_set);
    lyxp_set_free_content(&set2);
    return rc;
}

/**
 * @brief Evaluate OrExpr. Logs directly on error.
 *
 * [13] OrExpr ::= AndExpr | OrExpr 'or' AndExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in] repeat How many times this expression is repeated.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
eval_or_expr(const struct lyxp_expr *exp, uint16_t *tok_idx, uint16_t repeat, struct lyxp_set *set, uint32_t options)
{
    LY_ERR rc;
    struct lyxp_set orig_set, set2;
    uint16_t i;

    assert(repeat);

    set_init(&orig_set, set);
    set_init(&set2, set);

    set_fill_set(&orig_set, set);

    rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_OR, set, options);
    LY_CHECK_GOTO(rc, cleanup);

    /* cast to boolean, we know that will be the final result */
    if (set && (options & LYXP_SCNODE_ALL)) {
        set_scnode_clear_ctx(set, LYXP_SET_SCNODE_ATOM_NODE);
    } else {
        lyxp_set_cast(set, LYXP_SET_BOOLEAN);
    }

    /* ('or' AndExpr)* */
    for (i = 0; i < repeat; ++i) {
        assert(exp->tokens[*tok_idx] == LYXP_TOKEN_OPER_LOG);
        LOGDBG(LY_LDGXPATH, "%-27s %s %s[%u]", __func__, (!set || set->val.bln ? "skipped" : "parsed"),
                lyxp_print_token(exp->tokens[*tok_idx]), exp->tok_pos[*tok_idx]);
        ++(*tok_idx);

        /* lazy evaluation */
        if (!set || ((set->type == LYXP_SET_BOOLEAN) && set->val.bln)) {
            rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_OR, NULL, options);
            LY_CHECK_GOTO(rc, cleanup);
            continue;
        }

        set_fill_set(&set2, &orig_set);
        /* expr_type cound have been LYXP_EXPR_NONE in all these later calls (except for the first one),
         * but it does not matter */
        rc = eval_expr_select(exp, tok_idx, LYXP_EXPR_OR, &set2, options);
        LY_CHECK_GOTO(rc, cleanup);

        /* eval - just get boolean value actually */
        if (set->type == LYXP_SET_SCNODE_SET) {
            set_scnode_clear_ctx(&set2, LYXP_SET_SCNODE_ATOM_NODE);
            lyxp_set_scnode_merge(set, &set2);
        } else {
            lyxp_set_cast(&set2, LYXP_SET_BOOLEAN);
            set_fill_set(set, &set2);
        }
    }

cleanup:
    lyxp_set_free_content(&orig_set);
    lyxp_set_free_content(&set2);
    return rc;
}

/**
 * @brief Decide what expression is at the pointer @p tok_idx and evaluate it accordingly.
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] tok_idx Position in the expression @p exp.
 * @param[in] etype Expression type to evaluate.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] options XPath options.
 * @return LY_ERR (LY_EINCOMPLETE on unresolved when)
 */
static LY_ERR
eval_expr_select(const struct lyxp_expr *exp, uint16_t *tok_idx, enum lyxp_expr_type etype, struct lyxp_set *set,
        uint32_t options)
{
    uint16_t i, count;
    enum lyxp_expr_type next_etype;
    LY_ERR rc;

    /* process operator repeats */
    if (!exp->repeat[*tok_idx]) {
        next_etype = LYXP_EXPR_NONE;
    } else {
        /* find etype repeat */
        for (i = 0; exp->repeat[*tok_idx][i] > etype; ++i) {}

        /* select one-priority lower because etype expression called us */
        if (i) {
            next_etype = exp->repeat[*tok_idx][i - 1];
            /* count repeats for that expression */
            for (count = 0; i && exp->repeat[*tok_idx][i - 1] == next_etype; ++count, --i) {}
        } else {
            next_etype = LYXP_EXPR_NONE;
        }
    }

    /* decide what expression are we parsing based on the repeat */
    switch (next_etype) {
    case LYXP_EXPR_OR:
        rc = eval_or_expr(exp, tok_idx, count, set, options);
        break;
    case LYXP_EXPR_AND:
        rc = eval_and_expr(exp, tok_idx, count, set, options);
        break;
    case LYXP_EXPR_EQUALITY:
        rc = eval_equality_expr(exp, tok_idx, count, set, options);
        break;
    case LYXP_EXPR_RELATIONAL:
        rc = eval_relational_expr(exp, tok_idx, count, set, options);
        break;
    case LYXP_EXPR_ADDITIVE:
        rc = eval_additive_expr(exp, tok_idx, count, set, options);
        break;
    case LYXP_EXPR_MULTIPLICATIVE:
        rc = eval_multiplicative_expr(exp, tok_idx, count, set, options);
        break;
    case LYXP_EXPR_UNARY:
        rc = eval_unary_expr(exp, tok_idx, count, set, options);
        break;
    case LYXP_EXPR_UNION:
        rc = eval_union_expr(exp, tok_idx, count, set, options);
        break;
    case LYXP_EXPR_NONE:
        rc = eval_path_expr(exp, tok_idx, set, options);
        break;
    default:
        LOGINT_RET(set->ctx);
    }

    return rc;
}

/**
 * @brief Get root type.
 *
 * @param[in] ctx_node Context node.
 * @param[in] ctx_scnode Schema context node.
 * @param[in] options XPath options.
 * @return Root type.
 */
static enum lyxp_node_type
lyxp_get_root_type(const struct lyd_node *ctx_node, const struct lysc_node *ctx_scnode, uint32_t options)
{
    const struct lysc_node *op;

    if (options & LYXP_SCNODE_ALL) {
        /* schema */
        for (op = ctx_scnode; op && !(op->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)); op = op->parent) {}

        if (op || (options & LYXP_SCNODE)) {
            /* general root that can access everything */
            return LYXP_NODE_ROOT;
        } else if (!ctx_scnode || (ctx_scnode->flags & LYS_CONFIG_W)) {
            /* root context node can access only config data (because we said so, it is unspecified) */
            return LYXP_NODE_ROOT_CONFIG;
        }
        return LYXP_NODE_ROOT;
    }

    /* data */
    op = ctx_node ? ctx_node->schema : NULL;
    for ( ; op && !(op->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)); op = op->parent) {}

    if (op || !(options & LYXP_SCHEMA)) {
        /* general root that can access everything */
        return LYXP_NODE_ROOT;
    } else if (!ctx_node || !ctx_node->schema || (ctx_node->schema->flags & LYS_CONFIG_W)) {
        /* root context node can access only config data (because we said so, it is unspecified) */
        return LYXP_NODE_ROOT_CONFIG;
    }
    return LYXP_NODE_ROOT;
}

LY_ERR
lyxp_eval(const struct ly_ctx *ctx, const struct lyxp_expr *exp, const struct lys_module *cur_mod,
        LY_VALUE_FORMAT format, void *prefix_data, const struct lyd_node *ctx_node, const struct lyd_node *tree,
        struct lyxp_set *set, uint32_t options)
{
    uint16_t tok_idx = 0;
    LY_ERR rc;

    LY_CHECK_ARG_RET(ctx, ctx, exp, set, LY_EINVAL);
    if (!cur_mod && ((format == LY_VALUE_SCHEMA) || (format == LY_VALUE_SCHEMA_RESOLVED))) {
        LOGARG(NULL, "Current module must be set if schema format is used.");
        return LY_EINVAL;
    }

    if (tree) {
        /* adjust the pointer to be the first top-level sibling */
        while (tree->parent) {
            tree = lyd_parent(tree);
        }
        tree = lyd_first_sibling(tree);
    }

    /* prepare set for evaluation */
    memset(set, 0, sizeof *set);
    set->type = LYXP_SET_NODE_SET;
    set->root_type = lyxp_get_root_type(ctx_node, NULL, options);
    set_insert_node(set, (struct lyd_node *)ctx_node, 0, ctx_node ? LYXP_NODE_ELEM : set->root_type, 0);

    set->ctx = (struct ly_ctx *)ctx;
    set->cur_node = ctx_node;
    for (set->context_op = ctx_node ? ctx_node->schema : NULL;
            set->context_op && !(set->context_op->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF));
            set->context_op = set->context_op->parent) {}
    set->tree = tree;
    set->cur_mod = cur_mod;
    set->format = format;
    set->prefix_data = prefix_data;

    LOG_LOCSET(NULL, set->cur_node, NULL, NULL);

    /* evaluate */
    rc = eval_expr_select(exp, &tok_idx, 0, set, options);
    if (rc != LY_SUCCESS) {
        lyxp_set_free_content(set);
    }

    LOG_LOCBACK(0, 1, 0, 0);
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
        ly_print_(&out, "Empty XPath set\n\n");
        break;
    case LYXP_SET_BOOLEAN:
        ly_print_(&out, "Boolean XPath set:\n");
        ly_print_(&out, "%s\n\n", set->value.bool ? "true" : "false");
        break;
    case LYXP_SET_STRING:
        ly_print_(&out, "String XPath set:\n");
        ly_print_(&out, "\"%s\"\n\n", set->value.str);
        break;
    case LYXP_SET_NUMBER:
        ly_print_(&out, "Number XPath set:\n");

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
        ly_print_(&out, "%s\n\n", str_num);
        free(str_num);
        break;
    case LYXP_SET_NODE_SET:
        ly_print_(&out, "Node XPath set:\n");

        for (i = 0; i < set->used; ++i) {
            ly_print_(&out, "%d. ", i + 1);
            switch (set->node_type[i]) {
            case LYXP_NODE_ROOT_ALL:
                ly_print_(&out, "ROOT all\n\n");
                break;
            case LYXP_NODE_ROOT_CONFIG:
                ly_print_(&out, "ROOT config\n\n");
                break;
            case LYXP_NODE_ROOT_STATE:
                ly_print_(&out, "ROOT state\n\n");
                break;
            case LYXP_NODE_ROOT_NOTIF:
                ly_print_(&out, "ROOT notification \"%s\"\n\n", set->value.nodes[i]->schema->name);
                break;
            case LYXP_NODE_ROOT_RPC:
                ly_print_(&out, "ROOT rpc \"%s\"\n\n", set->value.nodes[i]->schema->name);
                break;
            case LYXP_NODE_ROOT_OUTPUT:
                ly_print_(&out, "ROOT output \"%s\"\n\n", set->value.nodes[i]->schema->name);
                break;
            case LYXP_NODE_ELEM:
                ly_print_(&out, "ELEM \"%s\"\n", set->value.nodes[i]->schema->name);
                xml_print_node(&out, 1, set->value.nodes[i], 1, LYP_FORMAT);
                ly_print_(&out, "\n");
                break;
            case LYXP_NODE_TEXT:
                ly_print_(&out, "TEXT \"%s\"\n\n", ((struct lyd_node_leaf_list *)set->value.nodes[i])->value_str);
                break;
            case LYXP_NODE_ATTR:
                ly_print_(&out, "ATTR \"%s\" = \"%s\"\n\n", set->value.attrs[i]->name, set->value.attrs[i]->value);
                break;
            }
        }
        break;
    }
}

#endif

LY_ERR
lyxp_set_cast(struct lyxp_set *set, enum lyxp_set_type target)
{
    long double num;
    char *str;
    LY_ERR rc;

    if (!set || (set->type == target)) {
        return LY_SUCCESS;
    }

    /* it's not possible to convert anything into a node set */
    assert(target != LYXP_SET_NODE_SET);

    if (set->type == LYXP_SET_SCNODE_SET) {
        lyxp_set_free_content(set);
        return LY_EINVAL;
    }

    /* to STRING */
    if ((target == LYXP_SET_STRING) || ((target == LYXP_SET_NUMBER) && (set->type == LYXP_SET_NODE_SET))) {
        switch (set->type) {
        case LYXP_SET_NUMBER:
            if (isnan(set->val.num)) {
                set->val.str = strdup("NaN");
                LY_CHECK_ERR_RET(!set->val.str, LOGMEM(set->ctx), -1);
            } else if ((set->val.num == 0) || (set->val.num == -0.0f)) {
                set->val.str = strdup("0");
                LY_CHECK_ERR_RET(!set->val.str, LOGMEM(set->ctx), -1);
            } else if (isinf(set->val.num) && !signbit(set->val.num)) {
                set->val.str = strdup("Infinity");
                LY_CHECK_ERR_RET(!set->val.str, LOGMEM(set->ctx), -1);
            } else if (isinf(set->val.num) && signbit(set->val.num)) {
                set->val.str = strdup("-Infinity");
                LY_CHECK_ERR_RET(!set->val.str, LOGMEM(set->ctx), -1);
            } else if ((long long)set->val.num == set->val.num) {
                if (asprintf(&str, "%lld", (long long)set->val.num) == -1) {
                    LOGMEM_RET(set->ctx);
                }
                set->val.str = str;
            } else {
                if (asprintf(&str, "%03.1Lf", set->val.num) == -1) {
                    LOGMEM_RET(set->ctx);
                }
                set->val.str = str;
            }
            break;
        case LYXP_SET_BOOLEAN:
            if (set->val.bln) {
                set->val.str = strdup("true");
            } else {
                set->val.str = strdup("false");
            }
            LY_CHECK_ERR_RET(!set->val.str, LOGMEM(set->ctx), LY_EMEM);
            break;
        case LYXP_SET_NODE_SET:
            /* we need the set sorted, it affects the result */
            assert(!set_sort(set));

            rc = cast_node_set_to_string(set, &str);
            LY_CHECK_RET(rc);
            lyxp_set_free_content(set);
            set->val.str = str;
            break;
        default:
            LOGINT_RET(set->ctx);
        }
        set->type = LYXP_SET_STRING;
    }

    /* to NUMBER */
    if (target == LYXP_SET_NUMBER) {
        switch (set->type) {
        case LYXP_SET_STRING:
            num = cast_string_to_number(set->val.str);
            lyxp_set_free_content(set);
            set->val.num = num;
            break;
        case LYXP_SET_BOOLEAN:
            if (set->val.bln) {
                set->val.num = 1;
            } else {
                set->val.num = 0;
            }
            break;
        default:
            LOGINT_RET(set->ctx);
        }
        set->type = LYXP_SET_NUMBER;
    }

    /* to BOOLEAN */
    if (target == LYXP_SET_BOOLEAN) {
        switch (set->type) {
        case LYXP_SET_NUMBER:
            if ((set->val.num == 0) || (set->val.num == -0.0f) || isnan(set->val.num)) {
                set->val.bln = 0;
            } else {
                set->val.bln = 1;
            }
            break;
        case LYXP_SET_STRING:
            if (set->val.str[0]) {
                lyxp_set_free_content(set);
                set->val.bln = 1;
            } else {
                lyxp_set_free_content(set);
                set->val.bln = 0;
            }
            break;
        case LYXP_SET_NODE_SET:
            if (set->used) {
                lyxp_set_free_content(set);
                set->val.bln = 1;
            } else {
                lyxp_set_free_content(set);
                set->val.bln = 0;
            }
            break;
        default:
            LOGINT_RET(set->ctx);
        }
        set->type = LYXP_SET_BOOLEAN;
    }

    return LY_SUCCESS;
}

LY_ERR
lyxp_atomize(const struct ly_ctx *ctx, const struct lyxp_expr *exp, const struct lys_module *cur_mod,
        LY_VALUE_FORMAT format, void *prefix_data, const struct lysc_node *ctx_scnode, struct lyxp_set *set,
        uint32_t options)
{
    LY_ERR ret;
    uint16_t tok_idx = 0;

    LY_CHECK_ARG_RET(ctx, ctx, exp, set, LY_EINVAL);
    if (!cur_mod && ((format == LY_VALUE_SCHEMA) || (format == LY_VALUE_SCHEMA_RESOLVED))) {
        LOGARG(NULL, "Current module must be set if schema format is used.");
        return LY_EINVAL;
    }

    /* prepare set for evaluation */
    memset(set, 0, sizeof *set);
    set->type = LYXP_SET_SCNODE_SET;
    set->root_type = lyxp_get_root_type(NULL, ctx_scnode, options);
    LY_CHECK_RET(lyxp_set_scnode_insert_node(set, ctx_scnode, ctx_scnode ? LYXP_NODE_ELEM : set->root_type, NULL));
    set->val.scnodes[0].in_ctx = LYXP_SET_SCNODE_START;

    set->ctx = (struct ly_ctx *)ctx;
    set->cur_scnode = ctx_scnode;
    for (set->context_op = ctx_scnode;
            set->context_op && !(set->context_op->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF));
            set->context_op = set->context_op->parent) {}
    set->cur_mod = cur_mod;
    set->format = format;
    set->prefix_data = prefix_data;

    LOG_LOCSET(set->cur_scnode, NULL, NULL, NULL);

    /* evaluate */
    ret = eval_expr_select(exp, &tok_idx, 0, set, options);

    LOG_LOCBACK(1, 0, 0, 0);
    return ret;
}

API const char *
lyxp_get_expr(const struct lyxp_expr *path)
{
    if (!path) {
        return NULL;
    }

    return path->expr;
}
