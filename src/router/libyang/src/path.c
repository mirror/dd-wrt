/**
 * @file path.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Path functions
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _ISOC99_SOURCE /* strtoull */

#include "path.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "log.h"
#include "plugins_types.h"
#include "schema_compile.h"
#include "set.h"
#include "tree.h"
#include "tree_data_internal.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xpath.h"

#define LOGVAL_P(CTX, CUR_NODE, CODE, ...) ly_vlog(CTX, (CUR_NODE) ? LY_VLOG_LYSC : LY_VLOG_NONE, CUR_NODE, CODE, ##__VA_ARGS__)

/**
 * @brief Check predicate syntax.
 *
 * @param[in] ctx libyang context.
 * @param[in] cur_node Current (original context) node.
 * @param[in] exp Parsed predicate.
 * @param[in,out] tok_idx Index in @p exp, is adjusted.
 * @param[in] prefix Prefix option.
 * @param[in] pred Predicate option.
 * @return LY_ERR value.
 */
static LY_ERR
ly_path_check_predicate(const struct ly_ctx *ctx, const struct lysc_node *cur_node, const struct lyxp_expr *exp,
        uint16_t *tok_idx, uint8_t prefix, uint8_t pred)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_set *set = NULL;
    uint32_t i;
    const char *name;
    size_t name_len;

    LOG_LOCSET(cur_node, NULL, NULL, NULL);

    if (!lyxp_next_token(NULL, exp, tok_idx, LYXP_TOKEN_BRACK1)) {
        /* '[' */

        if (((pred == LY_PATH_PRED_SIMPLE) || (pred == LY_PATH_PRED_KEYS)) &&
                !lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_NAMETEST)) {
            ret = ly_set_new(&set);
            LY_CHECK_GOTO(ret, cleanup);

            do {
                /* NameTest is always expected here */
                LY_CHECK_GOTO(lyxp_check_token(ctx, exp, *tok_idx, LYXP_TOKEN_NAMETEST), token_error);

                /* check prefix based on the options */
                name = strnstr(exp->expr + exp->tok_pos[*tok_idx], ":", exp->tok_len[*tok_idx]);
                if ((prefix == LY_PATH_PREFIX_MANDATORY) && !name) {
                    LOGVAL(ctx, LYVE_XPATH, "Prefix missing for \"%.*s\" in path.", exp->tok_len[*tok_idx],
                            exp->expr + exp->tok_pos[*tok_idx]);
                    goto token_error;
                } else if ((prefix == LY_PATH_PREFIX_STRICT_INHERIT) && name) {
                    LOGVAL(ctx, LYVE_XPATH, "Redundant prefix for \"%.*s\" in path.", exp->tok_len[*tok_idx],
                            exp->expr + exp->tok_pos[*tok_idx]);
                    goto token_error;
                }
                if (!name) {
                    name = exp->expr + exp->tok_pos[*tok_idx];
                    name_len = exp->tok_len[*tok_idx];
                } else {
                    ++name;
                    name_len = exp->tok_len[*tok_idx] - (name - (exp->expr + exp->tok_pos[*tok_idx]));
                }

                /* check whether it was not already specified */
                for (i = 0; i < set->count; ++i) {
                    /* all the keys must be from the same module so this comparison should be fine */
                    if (!strncmp(set->objs[i], name, name_len) && !isalpha(((char *)set->objs[i])[name_len])) {
                        LOGVAL(ctx, LYVE_XPATH, "Duplicate predicate key \"%.*s\" in path.", (int)name_len, name);
                        goto token_error;
                    }
                }

                /* add it into the set */
                ret = ly_set_add(set, (void *)name, 1, NULL);
                LY_CHECK_GOTO(ret, cleanup);

                /* NameTest */
                ++(*tok_idx);

                /* '=' */
                LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_OPER_EQUAL), token_error);

                /* Literal */
                LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_LITERAL), token_error);

                /* ']' */
                LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_BRACK2), token_error);

                /* '[' */
            } while (!lyxp_next_token(NULL, exp, tok_idx, LYXP_TOKEN_BRACK1));

        } else if ((pred == LY_PATH_PRED_SIMPLE) && !lyxp_next_token(NULL, exp, tok_idx, LYXP_TOKEN_DOT)) {
            /* '.' */

            /* '=' */
            LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_OPER_EQUAL), token_error);

            /* Literal */
            LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_LITERAL), token_error);

            /* ']' */
            LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_BRACK2), token_error);

        } else if ((pred == LY_PATH_PRED_SIMPLE) && !lyxp_next_token(NULL, exp, tok_idx, LYXP_TOKEN_NUMBER)) {
            /* Number */

            /* ']' */
            LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_BRACK2), token_error);

        } else if ((pred == LY_PATH_PRED_LEAFREF) && !lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_NAMETEST)) {
            assert(prefix == LY_PATH_PREFIX_OPTIONAL);
            ret = ly_set_new(&set);
            LY_CHECK_GOTO(ret, cleanup);

            do {
                /* NameTest is always expected here */
                LY_CHECK_GOTO(lyxp_check_token(ctx, exp, *tok_idx, LYXP_TOKEN_NAMETEST), token_error);

                name = strnstr(exp->expr + exp->tok_pos[*tok_idx], ":", exp->tok_len[*tok_idx]);
                if (!name) {
                    name = exp->expr + exp->tok_pos[*tok_idx];
                    name_len = exp->tok_len[*tok_idx];
                } else {
                    ++name;
                    name_len = exp->tok_len[*tok_idx] - (name - (exp->expr + exp->tok_pos[*tok_idx]));
                }

                /* check whether it was not already specified */
                for (i = 0; i < set->count; ++i) {
                    /* all the keys must be from the same module so this comparison should be fine */
                    if (!strncmp(set->objs[i], name, name_len) && !isalpha(((char *)set->objs[i])[name_len])) {
                        LOGVAL(ctx, LYVE_XPATH, "Duplicate predicate key \"%.*s\" in path.", (int)name_len, name);
                        goto token_error;
                    }
                }

                /* add it into the set */
                ret = ly_set_add(set, (void *)name, 1, NULL);
                LY_CHECK_GOTO(ret, cleanup);

                /* NameTest */
                ++(*tok_idx);

                /* '=' */
                LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_OPER_EQUAL), token_error);

                /* FuncName */
                LY_CHECK_GOTO(lyxp_check_token(ctx, exp, *tok_idx, LYXP_TOKEN_FUNCNAME), token_error);
                if ((exp->tok_len[*tok_idx] != ly_strlen_const("current")) ||
                        strncmp(exp->expr + exp->tok_pos[*tok_idx], "current", ly_strlen_const("current"))) {
                    LOGVAL(ctx, LYVE_XPATH, "Invalid function \"%.*s\" invocation in path.",
                            exp->tok_len[*tok_idx], exp->expr + exp->tok_pos[*tok_idx]);
                    goto token_error;
                }
                ++(*tok_idx);

                /* '(' */
                LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_PAR1), token_error);

                /* ')' */
                LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_PAR2), token_error);

                /* '/' */
                LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_OPER_PATH), token_error);

                /* '..' */
                LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_DDOT), token_error);
                do {
                    /* '/' */
                    LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_OPER_PATH), token_error);
                } while (!lyxp_next_token(NULL, exp, tok_idx, LYXP_TOKEN_DDOT));

                /* NameTest */
                LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_NAMETEST), token_error);

                /* '/' */
                while (!lyxp_next_token(NULL, exp, tok_idx, LYXP_TOKEN_OPER_PATH)) {
                    /* NameTest */
                    LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_NAMETEST), token_error);
                }

                /* ']' */
                LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_BRACK2), token_error);

                /* '[' */
            } while (!lyxp_next_token(NULL, exp, tok_idx, LYXP_TOKEN_BRACK1));

        } else {
            LOGVAL(ctx, LY_VCODE_XP_INTOK, lyxp_print_token(exp->tokens[*tok_idx]), exp->expr + exp->tok_pos[*tok_idx]);
            goto token_error;
        }
    }

cleanup:
    LOG_LOCBACK(cur_node ? 1 : 0, 0, 0, 0);
    ly_set_free(set, NULL);
    return ret;

token_error:
    LOG_LOCBACK(cur_node ? 1 : 0, 0, 0, 0);
    ly_set_free(set, NULL);
    return LY_EVALID;
}

LY_ERR
ly_path_parse(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *str_path, size_t path_len,
        uint8_t begin, uint8_t lref, uint8_t prefix, uint8_t pred, struct lyxp_expr **expr)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_expr *exp = NULL;
    uint16_t tok_idx, cur_len;
    const char *cur_node, *prev_prefix = NULL, *ptr;

    assert((begin == LY_PATH_BEGIN_ABSOLUTE) || (begin == LY_PATH_BEGIN_EITHER));
    assert((lref == LY_PATH_LREF_TRUE) || (lref == LY_PATH_LREF_FALSE));
    assert((prefix == LY_PATH_PREFIX_OPTIONAL) || (prefix == LY_PATH_PREFIX_MANDATORY) ||
            (prefix == LY_PATH_PREFIX_STRICT_INHERIT));
    assert((pred == LY_PATH_PRED_KEYS) || (pred == LY_PATH_PRED_SIMPLE) || (pred == LY_PATH_PRED_LEAFREF));

    LOG_LOCSET(ctx_node, NULL, NULL, NULL);

    /* parse as a generic XPath expression */
    LY_CHECK_GOTO(ret = lyxp_expr_parse(ctx, str_path, path_len, 1, &exp), error);
    tok_idx = 0;

    if (begin == LY_PATH_BEGIN_EITHER) {
        /* is the path relative? */
        if (lyxp_next_token(NULL, exp, &tok_idx, LYXP_TOKEN_OPER_PATH)) {
            /* relative path check specific to leafref */
            if (lref == LY_PATH_LREF_TRUE) {
                /* mandatory '..' */
                LY_CHECK_ERR_GOTO(lyxp_next_token(ctx, exp, &tok_idx, LYXP_TOKEN_DDOT), ret = LY_EVALID, error);

                do {
                    /* '/' */
                    LY_CHECK_ERR_GOTO(lyxp_next_token(ctx, exp, &tok_idx, LYXP_TOKEN_OPER_PATH), ret = LY_EVALID, error);

                    /* optional '..' */
                } while (!lyxp_next_token(NULL, exp, &tok_idx, LYXP_TOKEN_DDOT));
            }
        }
    } else {
        /* '/' */
        LY_CHECK_ERR_GOTO(lyxp_next_token(ctx, exp, &tok_idx, LYXP_TOKEN_OPER_PATH), ret = LY_EVALID, error);
    }

    do {
        /* NameTest */
        LY_CHECK_ERR_GOTO(lyxp_check_token(ctx, exp, tok_idx, LYXP_TOKEN_NAMETEST), ret = LY_EVALID, error);

        /* check prefix based on the options */
        cur_node = exp->expr + exp->tok_pos[tok_idx];
        cur_len = exp->tok_len[tok_idx];
        if (prefix == LY_PATH_PREFIX_MANDATORY) {
            if (!strnstr(cur_node, ":", cur_len)) {
                LOGVAL(ctx, LYVE_XPATH, "Prefix missing for \"%.*s\" in path.", cur_len, cur_node);
                ret = LY_EVALID;
                goto error;
            }
        } else if (prefix == LY_PATH_PREFIX_STRICT_INHERIT) {
            if (!prev_prefix) {
                /* the first node must have a prefix */
                if (!strnstr(cur_node, ":", cur_len)) {
                    LOGVAL(ctx, LYVE_XPATH, "Prefix missing for \"%.*s\" in path.", cur_len, cur_node);
                    ret = LY_EVALID;
                    goto error;
                }

                /* remember the first prefix */
                prev_prefix = cur_node;
            } else {
                /* the prefix must be different, if any */
                ptr = strnstr(cur_node, ":", cur_len);
                if (ptr) {
                    if (!strncmp(prev_prefix, cur_node, ptr - cur_node) && (prev_prefix[ptr - cur_node] == ':')) {
                        LOGVAL(ctx, LYVE_XPATH, "Duplicate prefix for \"%.*s\" in path.", cur_len, cur_node);
                        ret = LY_EVALID;
                        goto error;
                    }

                    /* remember this next prefix */
                    prev_prefix = cur_node;
                }
            }
        }

        ++tok_idx;

        /* Predicate* */
        LY_CHECK_GOTO(ret = ly_path_check_predicate(ctx, ctx_node, exp, &tok_idx, prefix, pred), error);

        /* '/' */
    } while (!lyxp_next_token(NULL, exp, &tok_idx, LYXP_TOKEN_OPER_PATH));

    /* trailing token check */
    if (exp->used > tok_idx) {
        LOGVAL(ctx, LYVE_XPATH, "Unparsed characters \"%s\" left at the end of path.", exp->expr + exp->tok_pos[tok_idx]);
        ret = LY_EVALID;
        goto error;
    }

    *expr = exp;

    LOG_LOCBACK(ctx_node ? 1 : 0, 0, 0, 0);
    return LY_SUCCESS;

error:
    lyxp_expr_free(ctx, exp);
    LOG_LOCBACK(ctx_node ? 1 : 0, 0, 0, 0);
    return ret;
}

LY_ERR
ly_path_parse_predicate(const struct ly_ctx *ctx, const struct lysc_node *cur_node, const char *str_path,
        size_t path_len, uint8_t prefix, uint8_t pred, struct lyxp_expr **expr)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_expr *exp = NULL;
    uint16_t tok_idx;

    assert((prefix == LY_PATH_PREFIX_OPTIONAL) || (prefix == LY_PATH_PREFIX_MANDATORY));
    assert((pred == LY_PATH_PRED_KEYS) || (pred == LY_PATH_PRED_SIMPLE) || (pred == LY_PATH_PRED_LEAFREF));

    LOG_LOCSET(cur_node, NULL, NULL, NULL);

    /* parse as a generic XPath expression */
    LY_CHECK_GOTO(ret = lyxp_expr_parse(ctx, str_path, path_len, 0, &exp), error);
    tok_idx = 0;

    LY_CHECK_GOTO(ret = ly_path_check_predicate(ctx, cur_node, exp, &tok_idx, prefix, pred), error);

    /* trailing token check */
    if (exp->used > tok_idx) {
        LOGVAL(ctx, LYVE_XPATH, "Unparsed characters \"%s\" left at the end of predicate.",
                exp->expr + exp->tok_pos[tok_idx]);
        ret = LY_EVALID;
        goto error;
    }

    *expr = exp;

    LOG_LOCBACK(cur_node ? 1 : 0, 0, 0, 0);
    return LY_SUCCESS;

error:
    lyxp_expr_free(ctx, exp);
    LOG_LOCBACK(cur_node ? 1 : 0, 0, 0, 0);
    return ret;
}

/**
 * @brief Parse prefix from a NameTest, if any, and node name, and return expected module of the node.
 *
 * @param[in] ctx libyang context.
 * @param[in] cur_node Optional current (original context) node.
 * @param[in] cur_mod Current module of the path (where the path is "instantiated"). Needed for ::LY_VALUE_SCHEMA*.
 * @param[in] prev_ctx_node Previous context node. Needed for ::LY_VALUE_JSON.
 * @param[in] expr Parsed path.
 * @param[in] tok_idx Index in @p expr.
 * @param[in] lref Lref option.
 * @param[in] format Format of the path.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[out] mod Resolved module.
 * @param[out] name Parsed node name.
 * @param[out] name_len Length of @p name.
 * @return LY_ERR value.
 */
static LY_ERR
ly_path_compile_prefix(const struct ly_ctx *ctx, const struct lysc_node *cur_node, const struct lys_module *cur_mod,
        const struct lysc_node *prev_ctx_node, const struct lyxp_expr *expr, uint16_t tok_idx, uint8_t lref,
        LY_VALUE_FORMAT format, void *prefix_data, struct lys_glob_unres *unres, const struct lys_module **mod,
        const char **name, size_t *name_len)
{
    LY_ERR ret;
    const char *pref;
    size_t len;

    assert(expr->tokens[tok_idx] == LYXP_TOKEN_NAMETEST);

    /* get prefix */
    if ((pref = strnstr(expr->expr + expr->tok_pos[tok_idx], ":", expr->tok_len[tok_idx]))) {
        len = pref - (expr->expr + expr->tok_pos[tok_idx]);
        pref = expr->expr + expr->tok_pos[tok_idx];
    } else {
        len = 0;
    }

    /* find next node module */
    if (pref) {
        LOG_LOCSET(cur_node, NULL, NULL, NULL);

        *mod = ly_resolve_prefix(ctx, pref, len, format, prefix_data);
        if (!*mod) {
            LOGVAL(ctx, LYVE_XPATH, "No module connected with the prefix \"%.*s\" found (prefix format %s).",
                    (int)len, pref, ly_format2str(format));
            ret = LY_EVALID;
            goto error;
        } else if (!(*mod)->implemented) {
            if (lref == LY_PATH_LREF_FALSE) {
                LOGVAL(ctx, LYVE_XPATH, "Not implemented module \"%s\" in path.", (*mod)->name);
                ret = LY_EVALID;
                goto error;
            }

            assert(unres);
            LY_CHECK_GOTO(ret = lys_set_implemented_r((struct lys_module *)*mod, NULL, unres), error);
            if (unres->recompile) {
                return LY_ERECOMPILE;
            }
        }

        LOG_LOCBACK(cur_node ? 1 : 0, 0, 0, 0);
    } else {
        switch (format) {
        case LY_VALUE_SCHEMA:
        case LY_VALUE_SCHEMA_RESOLVED:
            if (!cur_mod) {
                LOGINT_RET(ctx);
            }
            /* use current module */
            *mod = cur_mod;
            break;
        case LY_VALUE_JSON:
            if (!prev_ctx_node) {
                LOGINT_RET(ctx);
            }
            /* inherit module of the previous node */
            *mod = prev_ctx_node->module;
            break;
        case LY_VALUE_CANON:
        case LY_VALUE_XML:
        case LY_VALUE_LYB:
            /* not really defined or accepted */
            LOGINT_RET(ctx);
        }
    }

    /* set name */
    if (pref) {
        *name = pref + len + 1;
        *name_len = expr->tok_len[tok_idx] - len - 1;
    } else {
        *name = expr->expr + expr->tok_pos[tok_idx];
        *name_len = expr->tok_len[tok_idx];
    }

    return LY_SUCCESS;

error:
    LOG_LOCBACK(cur_node ? 1 : 0, 0, 0, 0);
    return ret;
}

LY_ERR
ly_path_compile_predicate(const struct ly_ctx *ctx, const struct lysc_node *cur_node, const struct lys_module *cur_mod,
        const struct lysc_node *ctx_node, const struct lyxp_expr *expr, uint16_t *tok_idx, LY_VALUE_FORMAT format,
        void *prefix_data, struct ly_path_predicate **predicates, enum ly_path_pred_type *pred_type)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_path_predicate *p;
    const struct lysc_node *key;
    const struct lys_module *mod = NULL;
    const char *name;
    size_t name_len, key_count;

    assert(ctx && ctx_node);

    LOG_LOCSET(cur_node, NULL, NULL, NULL);

    *pred_type = 0;

    if (lyxp_next_token(NULL, expr, tok_idx, LYXP_TOKEN_BRACK1)) {
        /* '[', no predicate */
        goto cleanup; /* LY_SUCCESS */
    }

    if (expr->tokens[*tok_idx] == LYXP_TOKEN_NAMETEST) {
        if (ctx_node->nodetype != LYS_LIST) {
            LOGVAL(ctx, LYVE_XPATH, "List predicate defined for %s \"%s\" in path.",
                    lys_nodetype2str(ctx_node->nodetype), ctx_node->name);
            ret = LY_EVALID;
            goto cleanup;
        } else if (ctx_node->flags & LYS_KEYLESS) {
            LOGVAL(ctx, LYVE_XPATH, "List predicate defined for keyless %s \"%s\" in path.",
                    lys_nodetype2str(ctx_node->nodetype), ctx_node->name);
            ret = LY_EVALID;
            goto cleanup;
        }

        do {
            /* NameTest, find the key */
            LY_CHECK_RET(ly_path_compile_prefix(ctx, cur_node, cur_mod, ctx_node, expr, *tok_idx, LY_PATH_LREF_FALSE,
                    format, prefix_data, NULL, &mod, &name, &name_len));
            key = lys_find_child(ctx_node, mod, name, name_len, 0, 0);
            if (!key) {
                LOGVAL(ctx, LYVE_XPATH, "Not found node \"%.*s\" in path.", (int)name_len, name);
                ret = LY_ENOTFOUND;
                goto cleanup;
            } else if ((key->nodetype != LYS_LEAF) || !(key->flags & LYS_KEY)) {
                LOGVAL(ctx, LYVE_XPATH, "Key expected instead of %s \"%s\" in path.", lys_nodetype2str(key->nodetype),
                        key->name);
                ret = LY_EVALID;
                goto cleanup;
            }
            ++(*tok_idx);

            if (!*pred_type) {
                /* new predicate */
                *pred_type = LY_PATH_PREDTYPE_LIST;
            }
            assert(*pred_type == LY_PATH_PREDTYPE_LIST);
            LY_ARRAY_NEW_GOTO(ctx, *predicates, p, ret, cleanup);
            p->key = key;

            /* '=' */
            assert(expr->tokens[*tok_idx] == LYXP_TOKEN_OPER_EQUAL);
            ++(*tok_idx);

            /* Literal */
            assert(expr->tokens[*tok_idx] == LYXP_TOKEN_LITERAL);
            LOG_LOCSET(key, NULL, NULL, NULL);
            ret = lyd_value_store(ctx, &p->value, ((struct lysc_node_leaf *)key)->type,
                    expr->expr + expr->tok_pos[*tok_idx] + 1, expr->tok_len[*tok_idx] - 2, NULL, format, prefix_data,
                    LYD_HINT_DATA, key, NULL);
            LOG_LOCBACK(key ? 1 : 0, 0, 0, 0);
            LY_CHECK_ERR_GOTO(ret, p->value.realtype = NULL, cleanup);
            ++(*tok_idx);

            /* "allocate" the type to avoid problems when freeing the value after the type was freed */
            ++((struct lysc_type *)p->value.realtype)->refcount;

            /* ']' */
            assert(expr->tokens[*tok_idx] == LYXP_TOKEN_BRACK2);
            ++(*tok_idx);

            /* another predicate follows? */
        } while (!lyxp_next_token(NULL, expr, tok_idx, LYXP_TOKEN_BRACK1));

        /* check that all keys were set */
        key_count = 0;
        for (key = lysc_node_child(ctx_node); key && (key->flags & LYS_KEY); key = key->next) {
            ++key_count;
        }
        if (LY_ARRAY_COUNT(*predicates) != key_count) {
            /* names (keys) are unique - it was checked when parsing */
            LOGVAL(ctx, LYVE_XPATH, "Predicate missing for a key of %s \"%s\" in path.",
                    lys_nodetype2str(ctx_node->nodetype), ctx_node->name);
            ly_path_predicates_free(ctx, LY_PATH_PREDTYPE_LIST, *predicates);
            *predicates = NULL;
            ret = LY_EVALID;
            goto cleanup;
        }

    } else if (expr->tokens[*tok_idx] == LYXP_TOKEN_DOT) {
        if (ctx_node->nodetype != LYS_LEAFLIST) {
            LOGVAL(ctx, LYVE_XPATH, "Leaf-list predicate defined for %s \"%s\" in path.",
                    lys_nodetype2str(ctx_node->nodetype), ctx_node->name);
            ret = LY_EVALID;
            goto cleanup;
        }
        ++(*tok_idx);

        /* new predicate */
        *pred_type = LY_PATH_PREDTYPE_LEAFLIST;
        LY_ARRAY_NEW_GOTO(ctx, *predicates, p, ret, cleanup);

        /* '=' */
        assert(expr->tokens[*tok_idx] == LYXP_TOKEN_OPER_EQUAL);
        ++(*tok_idx);

        assert(expr->tokens[*tok_idx] == LYXP_TOKEN_LITERAL);
        /* store the value */
        LOG_LOCSET(ctx_node, NULL, NULL, NULL);
        ret = lyd_value_store(ctx, &p->value, ((struct lysc_node_leaflist *)ctx_node)->type,
                expr->expr + expr->tok_pos[*tok_idx] + 1, expr->tok_len[*tok_idx] - 2, NULL, format, prefix_data,
                LYD_HINT_DATA, ctx_node, NULL);
        LOG_LOCBACK(ctx_node ? 1 : 0, 0, 0, 0);
        LY_CHECK_ERR_GOTO(ret, p->value.realtype = NULL, cleanup);
        ++(*tok_idx);

        /* "allocate" the type to avoid problems when freeing the value after the type was freed */
        ++((struct lysc_type *)p->value.realtype)->refcount;

        /* ']' */
        assert(expr->tokens[*tok_idx] == LYXP_TOKEN_BRACK2);
        ++(*tok_idx);
    } else {
        assert(expr->tokens[*tok_idx] == LYXP_TOKEN_NUMBER);
        if (!(ctx_node->nodetype & (LYS_LEAFLIST | LYS_LIST))) {
            ret = LY_EVALID;
            LOGVAL(ctx, LYVE_XPATH, "Positional predicate defined for %s \"%s\" in path.",
                    lys_nodetype2str(ctx_node->nodetype), ctx_node->name);
            goto cleanup;
        } else if (ctx_node->flags & LYS_CONFIG_W) {
            ret = LY_EVALID;
            LOGVAL(ctx, LYVE_XPATH, "Positional predicate defined for configuration %s \"%s\" in path.",
                    lys_nodetype2str(ctx_node->nodetype), ctx_node->name);
            goto cleanup;
        }

        /* new predicate */
        *pred_type = LY_PATH_PREDTYPE_POSITION;
        LY_ARRAY_NEW_GOTO(ctx, *predicates, p, ret, cleanup);

        /* syntax was already checked */
        p->position = strtoull(expr->expr + expr->tok_pos[*tok_idx], (char **)&name, LY_BASE_DEC);
        ++(*tok_idx);

        /* ']' */
        assert(expr->tokens[*tok_idx] == LYXP_TOKEN_BRACK2);
        ++(*tok_idx);
    }

cleanup:
    LOG_LOCBACK(cur_node ? 1 : 0, 0, 0, 0);
    return ret;
}

/**
 * @brief Compile leafref predicate. Actually, it is only checked.
 *
 * @param[in] ctx_node Context node, node for which the predicate is defined.
 * @param[in] cur_node Current (original context) node.
 * @param[in] expr Parsed path.
 * @param[in,out] tok_idx Index in @p expr, is adjusted for parsed tokens.
 * @param[in] format Format of the path.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[in,out] unres Global unres structure for newly implemented modules.
 * @return LY_ERR value.
 */
static LY_ERR
ly_path_compile_predicate_leafref(const struct lysc_node *ctx_node, const struct lysc_node *cur_node,
        const struct lyxp_expr *expr, uint16_t *tok_idx, LY_VALUE_FORMAT format, void *prefix_data,
        struct lys_glob_unres *unres)
{
    LY_ERR ret = LY_SUCCESS;
    const struct lysc_node *key, *node, *node2;
    const struct lys_module *mod;
    const char *name;
    size_t name_len;
    struct ly_ctx *ctx = cur_node->module->ctx;

    LOG_LOCSET(cur_node, NULL, NULL, NULL);

    if (lyxp_next_token(NULL, expr, tok_idx, LYXP_TOKEN_BRACK1)) {
        /* '[', no predicate */
        goto cleanup; /* LY_SUCCESS */
    }

    if (ctx_node->nodetype != LYS_LIST) {
        LOGVAL(ctx, LYVE_XPATH, "List predicate defined for %s \"%s\" in path.",
                lys_nodetype2str(ctx_node->nodetype), ctx_node->name);
        ret = LY_EVALID;
        goto cleanup;
    } else if (ctx_node->flags & LYS_KEYLESS) {
        LOGVAL(ctx, LYVE_XPATH, "List predicate defined for keyless %s \"%s\" in path.",
                lys_nodetype2str(ctx_node->nodetype), ctx_node->name);
        ret = LY_EVALID;
        goto cleanup;
    }

    do {
        /* NameTest, find the key */
        ret = ly_path_compile_prefix(ctx, cur_node, cur_node->module, ctx_node, expr, *tok_idx,
                LY_PATH_LREF_TRUE, format, prefix_data, unres, &mod, &name, &name_len);
        LY_CHECK_GOTO(ret, cleanup);
        key = lys_find_child(ctx_node, mod, name, name_len, 0, 0);
        if (!key) {
            LOGVAL(ctx, LYVE_XPATH, "Not found node \"%.*s\" in path.", (int)name_len, name);
            ret = LY_EVALID;
            goto cleanup;
        } else if ((key->nodetype != LYS_LEAF) || !(key->flags & LYS_KEY)) {
            LOGVAL(ctx, LYVE_XPATH, "Key expected instead of %s \"%s\" in path.",
                    lys_nodetype2str(key->nodetype), key->name);
            ret = LY_EVALID;
            goto cleanup;
        }
        ++(*tok_idx);

        /* we are not actually compiling, throw the key away */
        (void)key;

        /* '=' */
        assert(expr->tokens[*tok_idx] == LYXP_TOKEN_OPER_EQUAL);
        ++(*tok_idx);

        /* FuncName */
        assert(expr->tokens[*tok_idx] == LYXP_TOKEN_FUNCNAME);
        ++(*tok_idx);

        /* evaluating from the "current()" node */
        node = cur_node;

        /* '(' */
        assert(expr->tokens[*tok_idx] == LYXP_TOKEN_PAR1);
        ++(*tok_idx);

        /* ')' */
        assert(expr->tokens[*tok_idx] == LYXP_TOKEN_PAR2);
        ++(*tok_idx);

        do {
            /* '/' */
            assert(expr->tokens[*tok_idx] == LYXP_TOKEN_OPER_PATH);
            ++(*tok_idx);

            /* go to parent */
            if (!node) {
                LOGVAL(ctx, LYVE_XPATH, "Too many parent references in path.");
                ret = LY_EVALID;
                goto cleanup;
            }
            node = lysc_data_parent(node);

            /* '..' */
            assert(expr->tokens[*tok_idx] == LYXP_TOKEN_DDOT);
            ++(*tok_idx);
        } while (expr->tokens[*tok_idx + 1] == LYXP_TOKEN_DDOT);

        do {
            /* '/' */
            assert(expr->tokens[*tok_idx] == LYXP_TOKEN_OPER_PATH);
            ++(*tok_idx);

            /* NameTest */
            assert(expr->tokens[*tok_idx] == LYXP_TOKEN_NAMETEST);
            LY_CHECK_RET(ly_path_compile_prefix(ctx, cur_node, cur_node->module, node, expr, *tok_idx,
                    LY_PATH_LREF_TRUE, format, prefix_data, unres, &mod, &name, &name_len));
            node2 = lys_find_child(node, mod, name, name_len, 0, 0);
            if (!node2) {
                LOGVAL(ctx, LYVE_XPATH, "Not found node \"%.*s\" in path.", (int)name_len, name);
                ret = LY_EVALID;
                goto cleanup;
            }
            node = node2;
            ++(*tok_idx);
        } while ((*tok_idx + 1 < expr->used) && (expr->tokens[*tok_idx + 1] == LYXP_TOKEN_NAMETEST));

        /* check the last target node */
        if (node->nodetype != LYS_LEAF) {
            LOGVAL(ctx, LYVE_XPATH, "Leaf expected instead of %s \"%s\" in leafref predicate in path.",
                    lys_nodetype2str(node->nodetype), node->name);
            ret = LY_EVALID;
            goto cleanup;
        }

        /* we are not actually compiling, throw the rightside node away */
        (void)node;

        /* ']' */
        assert(expr->tokens[*tok_idx] == LYXP_TOKEN_BRACK2);
        ++(*tok_idx);

        /* another predicate follows? */
    } while (!lyxp_next_token(NULL, expr, tok_idx, LYXP_TOKEN_BRACK1));

cleanup:
    LOG_LOCBACK(1, 0, 0, 0);
    return ret;
}

LY_ERR
ly_path_compile(const struct ly_ctx *ctx, const struct lys_module *cur_mod, const struct lysc_node *ctx_node,
        const struct lysc_ext_instance *ext, const struct lyxp_expr *expr, uint8_t lref, uint8_t oper, uint8_t target,
        LY_VALUE_FORMAT format, void *prefix_data, struct lys_glob_unres *unres, struct ly_path **path)
{
    LY_ERR ret = LY_SUCCESS;
    uint16_t tok_idx = 0;
    const struct lys_module *mod = NULL;
    const struct lysc_node *node2, *cur_node, *op;
    struct ly_path *p = NULL;
    uint32_t getnext_opts;
    const char *name;
    size_t name_len;

    assert(ctx);
    assert((lref == LY_PATH_LREF_FALSE) || ctx_node);
    assert((lref == LY_PATH_LREF_TRUE) || (lref == LY_PATH_LREF_FALSE));
    assert((oper == LY_PATH_OPER_INPUT) || (oper == LY_PATH_OPER_OUTPUT));
    assert((target == LY_PATH_TARGET_SINGLE) || (target == LY_PATH_TARGET_MANY));

    /* find operation, if we are in any */
    for (op = ctx_node; op && !(op->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)); op = op->parent) {}

    *path = NULL;

    /* remember original context node */
    cur_node = ctx_node;
    LOG_LOCINIT(cur_node, NULL, NULL, NULL);

    if (oper == LY_PATH_OPER_OUTPUT) {
        getnext_opts = LYS_GETNEXT_OUTPUT;
    } else {
        getnext_opts = 0;
    }

    if (expr->tokens[tok_idx] == LYXP_TOKEN_OPER_PATH) {
        /* absolute path */
        ctx_node = NULL;

        ++tok_idx;
    } else {
        /* relative path */
        while ((lref == LY_PATH_LREF_TRUE) && (expr->tokens[tok_idx] == LYXP_TOKEN_DDOT)) {
            if (!ctx_node) {
                LOGVAL(ctx, LYVE_XPATH, "Too many parent references in path.");
                ret = LY_EVALID;
                goto cleanup;
            }

            /* get parent */
            ctx_node = lysc_data_parent(ctx_node);

            ++tok_idx;

            assert(expr->tokens[tok_idx] == LYXP_TOKEN_OPER_PATH);
            ++tok_idx;
        }

        /* we are not storing the parent */
        (void)ctx_node;
    }

    do {
        /* check last compiled inner node, whether it is uniquely identified (even key-less list) */
        if (p && (lref == LY_PATH_LREF_FALSE) && (target == LY_PATH_TARGET_SINGLE) &&
                (p->node->nodetype == LYS_LIST) && !p->predicates) {
            LOGVAL(ctx, LYVE_XPATH, "Predicate missing for %s \"%s\" in path.",
                    lys_nodetype2str(p->node->nodetype), p->node->name);
            ret = LY_EVALID;
            goto cleanup;
        }

        /* NameTest */
        LY_CHECK_ERR_GOTO(lyxp_check_token(ctx, expr, tok_idx, LYXP_TOKEN_NAMETEST), ret = LY_EVALID, cleanup);

        /* get module and node name */
        LY_CHECK_GOTO(ret = ly_path_compile_prefix(ctx, cur_node, cur_mod, ctx_node, expr, tok_idx, lref, format,
                prefix_data, unres, &mod, &name, &name_len), cleanup);
        ++tok_idx;

        /* find the next node */
        if (!ctx_node && ext) {
            node2 = lysc_ext_find_node(ext, mod, name, name_len, 0, getnext_opts);
        } else {
            node2 = lys_find_child(ctx_node, mod, name, name_len, 0, getnext_opts);
        }
        if (!node2 || (op && (node2->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) && (node2 != op))) {
            LOGVAL(ctx, LYVE_XPATH, "Not found node \"%.*s\" in path.", (int)name_len, name);
            ret = LY_EVALID;
            goto cleanup;
        }
        ctx_node = node2;

        /* new path segment */
        LY_ARRAY_NEW_GOTO(ctx, *path, p, ret, cleanup);
        p->node = ctx_node;

        /* compile any predicates */
        if (lref == LY_PATH_LREF_TRUE) {
            ret = ly_path_compile_predicate_leafref(ctx_node, cur_node, expr, &tok_idx, format, prefix_data, unres);
        } else {
            ret = ly_path_compile_predicate(ctx, cur_node, cur_mod, ctx_node, expr, &tok_idx, format, prefix_data,
                    &p->predicates, &p->pred_type);
        }
        LY_CHECK_GOTO(ret, cleanup);
    } while (!lyxp_next_token(NULL, expr, &tok_idx, LYXP_TOKEN_OPER_PATH));

    /* check leftover tokens */
    if (tok_idx < expr->used) {
        LOGVAL(ctx, LY_VCODE_XP_INTOK, lyxp_print_token(expr->tokens[tok_idx]), &expr->expr[expr->tok_pos[tok_idx]]);
        ret = LY_EVALID;
        goto cleanup;
    }

    /* check last compiled node */
    if ((lref == LY_PATH_LREF_FALSE) && (target == LY_PATH_TARGET_SINGLE) &&
            (p->node->nodetype & (LYS_LIST | LYS_LEAFLIST)) && !p->predicates) {
        LOGVAL(ctx, LYVE_XPATH, "Predicate missing for %s \"%s\" in path.",
                lys_nodetype2str(p->node->nodetype), p->node->name);
        ret = LY_EVALID;
        goto cleanup;
    }

cleanup:
    if (ret) {
        ly_path_free(ctx, *path);
        *path = NULL;
    }
    LOG_LOCBACK(1, 0, 0, 0);
    return ret;
}

LY_ERR
ly_path_eval_partial(const struct ly_path *path, const struct lyd_node *start, LY_ARRAY_COUNT_TYPE *path_idx,
        struct lyd_node **match)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lyd_node *prev_node = NULL, *node = NULL, *target;
    uint64_t pos;

    assert(path && start);

    if (lysc_data_parent(path[0].node)) {
        /* relative path, start from the parent children */
        start = lyd_child(start);
    } else {
        /* absolute path, start from the first top-level sibling */
        while (start->parent) {
            start = lyd_parent(start);
        }
        while (start->prev->next) {
            start = start->prev;
        }
    }

    LY_ARRAY_FOR(path, u) {
        switch (path[u].pred_type) {
        case LY_PATH_PREDTYPE_POSITION:
            /* we cannot use hashes and want an instance on a specific position */
            pos = 1;
            LYD_LIST_FOR_INST(start, path[u].node, node) {
                if (pos == path[u].predicates[0].position) {
                    break;
                }
                ++pos;
            }
            break;
        case LY_PATH_PREDTYPE_LEAFLIST:
            /* we will use hashes to find one leaf-list instance */
            LY_CHECK_RET(lyd_create_term2(path[u].node, &path[u].predicates[0].value, &target));
            lyd_find_sibling_first(start, target, &node);
            lyd_free_tree(target);
            break;
        case LY_PATH_PREDTYPE_LIST:
            /* we will use hashes to find one list instance */
            LY_CHECK_RET(lyd_create_list(path[u].node, path[u].predicates, &target));
            lyd_find_sibling_first(start, target, &node);
            lyd_free_tree(target);
            break;
        case LY_PATH_PREDTYPE_NONE:
            /* we will use hashes to find one any/container/leaf instance */
            lyd_find_sibling_val(start, path[u].node, NULL, 0, &node);
            break;
        }

        if (!node) {
            /* no matching nodes */
            break;
        }

        /* rememeber previous node */
        prev_node = node;

        /* next path segment, if any */
        start = lyd_child(node);
    }

    if (node) {
        /* we have found the full path */
        if (path_idx) {
            *path_idx = u;
        }
        if (match) {
            *match = node;
        }
        return LY_SUCCESS;

    } else if (prev_node) {
        /* we have found only some partial match */
        if (path_idx) {
            *path_idx = u - 1;
        }
        if (match) {
            *match = prev_node;
        }
        return LY_EINCOMPLETE;
    }

    /* we have not found any nodes */
    if (path_idx) {
        *path_idx = 0;
    }
    if (match) {
        *match = NULL;
    }
    return LY_ENOTFOUND;
}

LY_ERR
ly_path_eval(const struct ly_path *path, const struct lyd_node *start, struct lyd_node **match)
{
    LY_ERR ret;
    struct lyd_node *m;

    ret = ly_path_eval_partial(path, start, NULL, &m);

    if (ret == LY_SUCCESS) {
        /* last node was found */
        if (match) {
            *match = m;
        }
        return LY_SUCCESS;
    }

    /* not a full match */
    if (match) {
        *match = NULL;
    }
    return LY_ENOTFOUND;
}

LY_ERR
ly_path_dup(const struct ly_ctx *ctx, const struct ly_path *path, struct ly_path **dup)
{
    LY_ARRAY_COUNT_TYPE u, v;

    if (!path) {
        return LY_SUCCESS;
    }

    LY_ARRAY_CREATE_RET(ctx, *dup, LY_ARRAY_COUNT(path), LY_EMEM);
    LY_ARRAY_FOR(path, u) {
        LY_ARRAY_INCREMENT(*dup);
        (*dup)[u].node = path[u].node;
        if (path[u].predicates) {
            LY_ARRAY_CREATE_RET(ctx, (*dup)[u].predicates, LY_ARRAY_COUNT(path[u].predicates), LY_EMEM);
            (*dup)[u].pred_type = path[u].pred_type;
            LY_ARRAY_FOR(path[u].predicates, v) {
                struct ly_path_predicate *pred = &path[u].predicates[v];

                LY_ARRAY_INCREMENT((*dup)[u].predicates);
                switch (path[u].pred_type) {
                case LY_PATH_PREDTYPE_POSITION:
                    /* position-predicate */
                    (*dup)[u].predicates[v].position = pred->position;
                    break;
                case LY_PATH_PREDTYPE_LIST:
                case LY_PATH_PREDTYPE_LEAFLIST:
                    /* key-predicate or leaf-list-predicate */
                    (*dup)[u].predicates[v].key = pred->key;
                    pred->value.realtype->plugin->duplicate(ctx, &pred->value, &(*dup)[u].predicates[v].value);
                    ++((struct lysc_type *)pred->value.realtype)->refcount;
                    break;
                case LY_PATH_PREDTYPE_NONE:
                    break;
                }
            }
        }
    }

    return LY_SUCCESS;
}

void
ly_path_predicates_free(const struct ly_ctx *ctx, enum ly_path_pred_type pred_type, struct ly_path_predicate *predicates)
{
    LY_ARRAY_COUNT_TYPE u;

    if (!predicates) {
        return;
    }

    LY_ARRAY_FOR(predicates, u) {
        switch (pred_type) {
        case LY_PATH_PREDTYPE_POSITION:
        case LY_PATH_PREDTYPE_NONE:
            /* nothing to free */
            break;
        case LY_PATH_PREDTYPE_LIST:
        case LY_PATH_PREDTYPE_LEAFLIST:
            if (predicates[u].value.realtype) {
                predicates[u].value.realtype->plugin->free(ctx, &predicates[u].value);
                lysc_type_free((struct ly_ctx *)ctx, (struct lysc_type *)predicates[u].value.realtype);
            }
            break;
        }
    }
    LY_ARRAY_FREE(predicates);
}

void
ly_path_free(const struct ly_ctx *ctx, struct ly_path *path)
{
    LY_ARRAY_COUNT_TYPE u;

    if (!path) {
        return;
    }

    LY_ARRAY_FOR(path, u) {
        ly_path_predicates_free(ctx, path[u].pred_type, path[u].predicates);
    }
    LY_ARRAY_FREE(path);
}
