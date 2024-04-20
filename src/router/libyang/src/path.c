/**
 * @file path.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Path functions
 *
 * Copyright (c) 2020 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "path.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "log.h"
#include "ly_common.h"
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
        uint32_t *tok_idx, uint16_t prefix, uint16_t pred)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_set *set = NULL;
    uint32_t i;
    const char *name;
    size_t name_len;

    if (cur_node) {
        LOG_LOCSET(cur_node, NULL);
    }

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
                    LOGVAL(ctx, LYVE_XPATH, "Prefix missing for \"%.*s\" in path.", (int)exp->tok_len[*tok_idx],
                            exp->expr + exp->tok_pos[*tok_idx]);
                    goto token_error;
                } else if ((prefix == LY_PATH_PREFIX_STRICT_INHERIT) && name) {
                    LOGVAL(ctx, LYVE_XPATH, "Redundant prefix for \"%.*s\" in path.", (int)exp->tok_len[*tok_idx],
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
                    if (!strncmp(set->objs[i], name, name_len) &&
                            lysp_check_identifierchar(NULL, ((char *)set->objs[i])[name_len], 0, NULL)) {
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

                /* fill repeat */
                exp->repeat[*tok_idx - 2] = calloc(2, sizeof *exp->repeat[*tok_idx]);
                LY_CHECK_ERR_GOTO(!exp->repeat[*tok_idx - 2], LOGMEM(NULL); ret = LY_EMEM, cleanup);
                exp->repeat[*tok_idx - 2][0] = LYXP_EXPR_EQUALITY;

                /* Literal, Number, or VariableReference */
                if (lyxp_next_token(NULL, exp, tok_idx, LYXP_TOKEN_LITERAL) &&
                        lyxp_next_token(NULL, exp, tok_idx, LYXP_TOKEN_NUMBER) &&
                        lyxp_next_token(NULL, exp, tok_idx, LYXP_TOKEN_VARREF)) {
                    /* error */
                    lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_LITERAL);
                    goto token_error;
                }

                /* ']' */
                LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_BRACK2), token_error);

                /* '[' */
            } while (!lyxp_next_token(NULL, exp, tok_idx, LYXP_TOKEN_BRACK1));

        } else if ((pred == LY_PATH_PRED_SIMPLE) && !lyxp_next_token(NULL, exp, tok_idx, LYXP_TOKEN_DOT)) {
            /* '.' */

            /* '=' */
            LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_OPER_EQUAL), token_error);

            /* fill repeat */
            exp->repeat[*tok_idx - 2] = calloc(2, sizeof *exp->repeat[*tok_idx]);
            LY_CHECK_ERR_GOTO(!exp->repeat[*tok_idx - 2], LOGMEM(NULL); ret = LY_EMEM, cleanup);
            exp->repeat[*tok_idx - 2][0] = LYXP_EXPR_EQUALITY;

            /* Literal or Number */
            LY_CHECK_GOTO(lyxp_next_token2(ctx, exp, tok_idx, LYXP_TOKEN_LITERAL, LYXP_TOKEN_NUMBER), token_error);

            /* ']' */
            LY_CHECK_GOTO(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_BRACK2), token_error);

        } else if ((pred == LY_PATH_PRED_SIMPLE) && !lyxp_next_token(NULL, exp, tok_idx, LYXP_TOKEN_NUMBER)) {
            /* Number */

            /* check for index 0 */
            if (!atoi(exp->expr + exp->tok_pos[*tok_idx - 1])) {
                LOGVAL(ctx, LYVE_XPATH, "Invalid positional predicate \"%.*s\".", (int)exp->tok_len[*tok_idx - 1],
                        exp->expr + exp->tok_pos[*tok_idx - 1]);
                goto token_error;
            }

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
                    if (!strncmp(set->objs[i], name, name_len) &&
                            lysp_check_identifierchar(NULL, ((char *)set->objs[i])[name_len], 0, NULL)) {
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

                /* fill repeat */
                exp->repeat[*tok_idx - 2] = calloc(2, sizeof *exp->repeat[*tok_idx]);
                LY_CHECK_ERR_GOTO(!exp->repeat[*tok_idx - 2], LOGMEM(NULL); ret = LY_EMEM, cleanup);
                exp->repeat[*tok_idx - 2][0] = LYXP_EXPR_EQUALITY;

                /* FuncName */
                LY_CHECK_GOTO(lyxp_check_token(ctx, exp, *tok_idx, LYXP_TOKEN_FUNCNAME), token_error);
                if ((exp->tok_len[*tok_idx] != ly_strlen_const("current")) ||
                        strncmp(exp->expr + exp->tok_pos[*tok_idx], "current", ly_strlen_const("current"))) {
                    LOGVAL(ctx, LYVE_XPATH, "Invalid function \"%.*s\" invocation in path.",
                            (int)exp->tok_len[*tok_idx], exp->expr + exp->tok_pos[*tok_idx]);
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

        } else if (lyxp_check_token(ctx, exp, *tok_idx, 0)) {
            /* unexpected EOF */
            goto token_error;
        } else {
            /* invalid token */
            LOGVAL(ctx, LY_VCODE_XP_INTOK, lyxp_token2str(exp->tokens[*tok_idx]), exp->expr + exp->tok_pos[*tok_idx]);
            goto token_error;
        }
    }

cleanup:
    LOG_LOCBACK(cur_node ? 1 : 0, 0);
    ly_set_free(set, NULL);
    return ret;

token_error:
    LOG_LOCBACK(cur_node ? 1 : 0, 0);
    ly_set_free(set, NULL);
    return LY_EVALID;
}

/**
 * @brief Parse deref XPath function and perform all additional checks.
 *
 * @param[in] ctx libyang context.
 * @param[in] ctx_node Optional context node, used for logging.
 * @param[in] exp Parsed path.
 * @param[in,out] tok_idx Index in @p exp, is adjusted.
 * @return LY_ERR value.
 */
static LY_ERR
ly_path_parse_deref(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const struct lyxp_expr *exp,
        uint32_t *tok_idx)
{
    size_t arg_len;
    uint32_t begin_token, end_token;
    struct lyxp_expr *arg_expr = NULL;

    /* mandatory FunctionName */
    LY_CHECK_RET(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_FUNCNAME), LY_EVALID);
    if (!strncmp(&exp->expr[exp->tok_pos[*tok_idx]], "deref", 5)) {
        LOGVAL(ctx, LYVE_XPATH, "Unexpected XPath function \"%.*s\" in path, expected \"deref(...)\"",
                (int)exp->tok_len[*tok_idx], exp->expr + exp->tok_pos[*tok_idx]);
        return LY_EVALID;
    }

    /* mandatory '(' */
    LY_CHECK_RET(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_PAR1), LY_EVALID);
    begin_token = *tok_idx;

    /* count tokens till ')' */
    while (lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_PAR2) && *tok_idx < exp->used) {
        /* emebedded functions are not allowed */
        if (!lyxp_check_token(NULL, exp, *tok_idx, LYXP_TOKEN_FUNCNAME)) {
            LOGVAL(ctx, LYVE_XPATH, "Embedded function XPath function inside deref function within the path"
                    "is not allowed");
            return LY_EVALID;
        }

        (*tok_idx)++;
    }

    /* mandatory ')' */
    LY_CHECK_RET(lyxp_next_token(ctx, exp, tok_idx, LYXP_TOKEN_PAR2), LY_EVALID);
    end_token = *tok_idx - 1;

    /* parse the path of deref argument */
    arg_len = exp->tok_pos[end_token] - exp->tok_pos[begin_token];
    LY_CHECK_RET(ly_path_parse(ctx, ctx_node, &exp->expr[exp->tok_pos[begin_token]], arg_len, 1,
            LY_PATH_BEGIN_EITHER, LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_LEAFREF, &arg_expr), LY_EVALID);
    lyxp_expr_free(ctx, arg_expr);

    return LY_SUCCESS;
}

LY_ERR
ly_path_parse(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *str_path, size_t path_len,
        ly_bool lref, uint16_t begin, uint16_t prefix, uint16_t pred, struct lyxp_expr **expr)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_expr *exp = NULL;
    uint32_t tok_idx, cur_len;
    const char *cur_node, *prev_prefix = NULL, *ptr;
    ly_bool is_abs;

    assert((begin == LY_PATH_BEGIN_ABSOLUTE) || (begin == LY_PATH_BEGIN_EITHER));
    assert((prefix == LY_PATH_PREFIX_OPTIONAL) || (prefix == LY_PATH_PREFIX_MANDATORY) ||
            (prefix == LY_PATH_PREFIX_FIRST) || (prefix == LY_PATH_PREFIX_STRICT_INHERIT));
    assert((pred == LY_PATH_PRED_KEYS) || (pred == LY_PATH_PRED_SIMPLE) || (pred == LY_PATH_PRED_LEAFREF));

    if (ctx_node) {
        LOG_LOCSET(ctx_node, NULL);
    }

    /* parse as a generic XPath expression, reparse is performed manually */
    LY_CHECK_GOTO(ret = lyxp_expr_parse(ctx, str_path, path_len, 0, &exp), error);
    tok_idx = 0;

    /* alloc empty repeat (only '=', filled manually) */
    exp->repeat = calloc(exp->size, sizeof *exp->repeat);
    LY_CHECK_ERR_GOTO(!exp->repeat, LOGMEM(ctx); ret = LY_EMEM, error);

    if (begin == LY_PATH_BEGIN_EITHER) {
        /* is the path relative? */
        if (lyxp_next_token(NULL, exp, &tok_idx, LYXP_TOKEN_OPER_PATH)) {
            /* relative path check specific to leafref */
            if (lref) {
                /* optional function 'deref..' */
                if ((ly_ctx_get_options(ctx) & LY_CTX_LEAFREF_EXTENDED) &&
                        !lyxp_check_token(NULL, exp, tok_idx, LYXP_TOKEN_FUNCNAME)) {
                    LY_CHECK_ERR_GOTO(ly_path_parse_deref(ctx, ctx_node, exp, &tok_idx), ret = LY_EVALID, error);

                    /* '/' */
                    LY_CHECK_ERR_GOTO(lyxp_next_token(ctx, exp, &tok_idx, LYXP_TOKEN_OPER_PATH), ret = LY_EVALID,
                            error);
                }

                /* mandatory '..' */
                LY_CHECK_ERR_GOTO(lyxp_next_token(ctx, exp, &tok_idx, LYXP_TOKEN_DDOT), ret = LY_EVALID, error);

                do {
                    /* '/' */
                    LY_CHECK_ERR_GOTO(lyxp_next_token(ctx, exp, &tok_idx, LYXP_TOKEN_OPER_PATH), ret = LY_EVALID,
                            error);

                    /* optional '..' */
                } while (!lyxp_next_token(NULL, exp, &tok_idx, LYXP_TOKEN_DDOT));
            }

            is_abs = 0;
        } else {
            is_abs = 1;
        }
    } else {
        /* '/' */
        LY_CHECK_ERR_GOTO(lyxp_next_token(ctx, exp, &tok_idx, LYXP_TOKEN_OPER_PATH), ret = LY_EVALID, error);

        is_abs = 1;
    }

    do {
        /* NameTest */
        LY_CHECK_ERR_GOTO(lyxp_check_token(ctx, exp, tok_idx, LYXP_TOKEN_NAMETEST), ret = LY_EVALID, error);

        /* check prefix based on the options */
        cur_node = exp->expr + exp->tok_pos[tok_idx];
        cur_len = exp->tok_len[tok_idx];
        if (prefix == LY_PATH_PREFIX_MANDATORY) {
            if (!strnstr(cur_node, ":", cur_len)) {
                LOGVAL(ctx, LYVE_XPATH, "Prefix missing for \"%.*s\" in path.", (int)cur_len, cur_node);
                ret = LY_EVALID;
                goto error;
            }
        } else if ((prefix == LY_PATH_PREFIX_FIRST) || (prefix == LY_PATH_PREFIX_STRICT_INHERIT)) {
            if (!prev_prefix && is_abs) {
                /* the first node must have a prefix */
                if (!strnstr(cur_node, ":", cur_len)) {
                    LOGVAL(ctx, LYVE_XPATH, "Prefix missing for \"%.*s\" in path.", (int)cur_len, cur_node);
                    ret = LY_EVALID;
                    goto error;
                }

                /* remember the first prefix */
                prev_prefix = cur_node;
            } else if (prev_prefix && (prefix == LY_PATH_PREFIX_STRICT_INHERIT)) {
                /* the prefix must be different, if any */
                ptr = strnstr(cur_node, ":", cur_len);
                if (ptr) {
                    if (!strncmp(prev_prefix, cur_node, ptr - cur_node) && (prev_prefix[ptr - cur_node] == ':')) {
                        LOGVAL(ctx, LYVE_XPATH, "Duplicate prefix for \"%.*s\" in path.", (int)cur_len, cur_node);
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

    LOG_LOCBACK(ctx_node ? 1 : 0, 0);
    return LY_SUCCESS;

error:
    lyxp_expr_free(ctx, exp);
    LOG_LOCBACK(ctx_node ? 1 : 0, 0);
    return ret;
}

LY_ERR
ly_path_parse_predicate(const struct ly_ctx *ctx, const struct lysc_node *cur_node, const char *str_path,
        size_t path_len, uint16_t prefix, uint16_t pred, struct lyxp_expr **expr)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_expr *exp = NULL;
    uint32_t tok_idx;

    assert((prefix == LY_PATH_PREFIX_OPTIONAL) || (prefix == LY_PATH_PREFIX_MANDATORY));
    assert((pred == LY_PATH_PRED_KEYS) || (pred == LY_PATH_PRED_SIMPLE) || (pred == LY_PATH_PRED_LEAFREF));

    if (cur_node) {
        LOG_LOCSET(cur_node, NULL);
    }

    /* parse as a generic XPath expression, reparse is performed manually */
    LY_CHECK_GOTO(ret = lyxp_expr_parse(ctx, str_path, path_len, 0, &exp), error);
    tok_idx = 0;

    /* alloc empty repeat (only '=', filled manually) */
    exp->repeat = calloc(exp->size, sizeof *exp->repeat);
    LY_CHECK_ERR_GOTO(!exp->repeat, LOGMEM(ctx); ret = LY_EMEM, error);

    LY_CHECK_GOTO(ret = ly_path_check_predicate(ctx, cur_node, exp, &tok_idx, prefix, pred), error);

    /* trailing token check */
    if (exp->used > tok_idx) {
        LOGVAL(ctx, LYVE_XPATH, "Unparsed characters \"%s\" left at the end of predicate.",
                exp->expr + exp->tok_pos[tok_idx]);
        ret = LY_EVALID;
        goto error;
    }

    *expr = exp;

    LOG_LOCBACK(cur_node ? 1 : 0, 0);
    return LY_SUCCESS;

error:
    lyxp_expr_free(ctx, exp);
    LOG_LOCBACK(cur_node ? 1 : 0, 0);
    return ret;
}

/**
 * @brief Parse NameTest and get the corresponding schema node.
 *
 * @param[in] ctx libyang context.
 * @param[in] cur_node Optional current (original context) node.
 * @param[in] cur_mod Current module of the path (where the path is "instantiated"). Needed for ::LY_VALUE_SCHEMA
 * and ::LY_VALUE_SCHEMA_RESOLVED.
 * @param[in] prev_ctx_node Previous context node.
 * @param[in] expr Parsed path.
 * @param[in] tok_idx Index in @p expr.
 * @param[in] format Format of the path.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[in] top_ext Optional top-level extension to use for searching the schema node.
 * @param[in] getnext_opts Options to be used for ::lys_getnext() calls.
 * @param[out] snode Resolved schema node.
 * @param[out] ext Optional extension instance of @p snode, if any.
 * @return LY_ERR value.
 */
static LY_ERR
ly_path_compile_snode(const struct ly_ctx *ctx, const struct lysc_node *cur_node, const struct lys_module *cur_mod,
        const struct lysc_node *prev_ctx_node, const struct lyxp_expr *expr, uint32_t tok_idx, LY_VALUE_FORMAT format,
        void *prefix_data, const struct lysc_ext_instance *top_ext, uint32_t getnext_opts, const struct lysc_node **snode,
        struct lysc_ext_instance **ext)
{
    LY_ERR ret;
    const struct lys_module *mod = NULL;
    struct lysc_ext_instance *e = NULL;
    const char *pref, *name;
    size_t len, name_len;

    assert(expr->tokens[tok_idx] == LYXP_TOKEN_NAMETEST);

    *snode = NULL;
    if (ext) {
        *ext = NULL;
    }

    /* get prefix */
    if ((pref = strnstr(expr->expr + expr->tok_pos[tok_idx], ":", expr->tok_len[tok_idx]))) {
        len = pref - (expr->expr + expr->tok_pos[tok_idx]);
        pref = expr->expr + expr->tok_pos[tok_idx];
    } else {
        len = 0;
    }

    /* set name */
    if (pref) {
        name = pref + len + 1;
        name_len = expr->tok_len[tok_idx] - len - 1;
    } else {
        name = expr->expr + expr->tok_pos[tok_idx];
        name_len = expr->tok_len[tok_idx];
    }

    /* find node module */
    if (pref) {
        if (cur_node) {
            LOG_LOCSET(cur_node, NULL);
        }

        mod = ly_resolve_prefix(prev_ctx_node ? prev_ctx_node->module->ctx : ctx, pref, len, format, prefix_data);
        if ((!mod || !mod->implemented) && prev_ctx_node) {
            /* check for nested ext data */
            ret = ly_nested_ext_schema(NULL, prev_ctx_node, pref, len, format, prefix_data, name, name_len, snode, &e);
            if (!ret) {
                goto success;
            } else if (ret != LY_ENOT) {
                goto error;
            }
        }

        if (!mod) {
            LOGVAL(ctx, LYVE_XPATH, "No module connected with the prefix \"%.*s\" found (prefix format %s).",
                    (int)len, pref, ly_format2str(format));
            ret = LY_EVALID;
            goto error;
        } else if (!mod->implemented) {
            LOGVAL(ctx, LYVE_XPATH, "Not implemented module \"%s\" in path.", mod->name);
            ret = LY_EVALID;
            goto error;
        }

        LOG_LOCBACK(cur_node ? 1 : 0, 0);
    } else {
        switch (format) {
        case LY_VALUE_SCHEMA:
        case LY_VALUE_SCHEMA_RESOLVED:
            if (!cur_mod) {
                LOGINT_RET(ctx);
            }
            /* use current module */
            mod = cur_mod;
            break;
        case LY_VALUE_JSON:
        case LY_VALUE_LYB:
            if (!prev_ctx_node) {
                LOGINT_RET(ctx);
            }
            /* inherit module of the previous node */
            mod = prev_ctx_node->module;
            break;
        case LY_VALUE_CANON:
        case LY_VALUE_XML:
        case LY_VALUE_STR_NS:
            /* not really defined or accepted */
            LOGINT_RET(ctx);
        }
    }

    /* find schema node */
    if (!prev_ctx_node && top_ext) {
        *snode = lysc_ext_find_node(top_ext, mod, name, name_len, 0, getnext_opts);
    } else {
        *snode = lys_find_child(prev_ctx_node, mod, name, name_len, 0, getnext_opts);
        if (!(*snode) && prev_ctx_node) {
            ret = ly_nested_ext_schema(NULL, prev_ctx_node, pref, len, format, prefix_data, name, name_len, snode, &e);
            LY_CHECK_RET(ret && (ret != LY_ENOT), ret);
        }
    }
    if (!(*snode)) {
        LOGVAL(ctx, LYVE_XPATH, "Not found node \"%.*s\" in path.", (int)name_len, name);
        return LY_ENOTFOUND;
    }

success:
    if (ext) {
        *ext = e;
    }
    return LY_SUCCESS;

error:
    LOG_LOCBACK(cur_node ? 1 : 0, 0);
    return ret;
}

LY_ERR
ly_path_compile_predicate(const struct ly_ctx *ctx, const struct lysc_node *cur_node, const struct lys_module *cur_mod,
        const struct lysc_node *ctx_node, const struct lyxp_expr *expr, uint32_t *tok_idx, LY_VALUE_FORMAT format,
        void *prefix_data, struct ly_path_predicate **predicates)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_path_predicate *p;
    const struct lysc_node *key;
    const char *val;
    size_t val_len, key_count;

    assert(ctx && ctx_node);

    if (cur_node) {
        LOG_LOCSET(cur_node, NULL);
    }

    *predicates = NULL;

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
            LY_CHECK_RET(ly_path_compile_snode(ctx, cur_node, cur_mod, ctx_node, expr, *tok_idx, format, prefix_data,
                    NULL, 0, &key, NULL));
            if ((key->nodetype != LYS_LEAF) || !(key->flags & LYS_KEY)) {
                LOGVAL(ctx, LYVE_XPATH, "Key expected instead of %s \"%s\" in path.", lys_nodetype2str(key->nodetype),
                        key->name);
                ret = LY_EVALID;
                goto cleanup;
            }
            ++(*tok_idx);

            /* new predicate */
            LY_ARRAY_NEW_GOTO(ctx, *predicates, p, ret, cleanup);
            p->key = key;

            /* '=' */
            assert(expr->tokens[*tok_idx] == LYXP_TOKEN_OPER_EQUAL);
            ++(*tok_idx);

            /* Literal, Number, or VariableReference */
            if (expr->tokens[*tok_idx] == LYXP_TOKEN_VARREF) {
                /* store the variable name */
                p->variable = strndup(expr->expr + expr->tok_pos[*tok_idx], expr->tok_len[*tok_idx]);
                LY_CHECK_ERR_GOTO(!p->variable, LOGMEM(ctx); ret = LY_EMEM, cleanup);

                p->type = LY_PATH_PREDTYPE_LIST_VAR;
                ++(*tok_idx);
            } else {
                if (expr->tokens[*tok_idx] == LYXP_TOKEN_LITERAL) {
                    /* skip quotes */
                    val = expr->expr + expr->tok_pos[*tok_idx] + 1;
                    val_len = expr->tok_len[*tok_idx] - 2;
                } else {
                    assert(expr->tokens[*tok_idx] == LYXP_TOKEN_NUMBER);
                    val = expr->expr + expr->tok_pos[*tok_idx];
                    val_len = expr->tok_len[*tok_idx];
                }

                /* store the value */
                LOG_LOCSET(key, NULL);
                ret = lyd_value_store(ctx, &p->value, ((struct lysc_node_leaf *)key)->type, val, val_len, 0, 0,
                        NULL, format, prefix_data, LYD_HINT_DATA, key, NULL);
                LOG_LOCBACK(1, 0);
                LY_CHECK_ERR_GOTO(ret, p->value.realtype = NULL, cleanup);

                /* "allocate" the type to avoid problems when freeing the value after the type was freed */
                LY_ATOMIC_INC_BARRIER(((struct lysc_type *)p->value.realtype)->refcount);

                p->type = LY_PATH_PREDTYPE_LIST;
                ++(*tok_idx);
            }

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
            ly_path_predicates_free(ctx, *predicates);
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
        LY_ARRAY_NEW_GOTO(ctx, *predicates, p, ret, cleanup);
        p->type = LY_PATH_PREDTYPE_LEAFLIST;

        /* '=' */
        assert(expr->tokens[*tok_idx] == LYXP_TOKEN_OPER_EQUAL);
        ++(*tok_idx);

        /* Literal or Number */
        assert((expr->tokens[*tok_idx] == LYXP_TOKEN_LITERAL) || (expr->tokens[*tok_idx] == LYXP_TOKEN_NUMBER));
        if (expr->tokens[*tok_idx] == LYXP_TOKEN_LITERAL) {
            /* skip quotes */
            val = expr->expr + expr->tok_pos[*tok_idx] + 1;
            val_len = expr->tok_len[*tok_idx] - 2;
        } else {
            val = expr->expr + expr->tok_pos[*tok_idx];
            val_len = expr->tok_len[*tok_idx];
        }

        /* store the value */
        if (ctx_node) {
            LOG_LOCSET(ctx_node, NULL);
        }
        ret = lyd_value_store(ctx, &p->value, ((struct lysc_node_leaflist *)ctx_node)->type, val, val_len, 0, 0,
                NULL, format, prefix_data, LYD_HINT_DATA, ctx_node, NULL);
        LOG_LOCBACK(ctx_node ? 1 : 0, 0);
        LY_CHECK_ERR_GOTO(ret, p->value.realtype = NULL, cleanup);
        ++(*tok_idx);

        /* "allocate" the type to avoid problems when freeing the value after the type was freed */
        LY_ATOMIC_INC_BARRIER(((struct lysc_type *)p->value.realtype)->refcount);

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
        LY_ARRAY_NEW_GOTO(ctx, *predicates, p, ret, cleanup);
        p->type = LY_PATH_PREDTYPE_POSITION;

        /* syntax was already checked */
        p->position = strtoull(expr->expr + expr->tok_pos[*tok_idx], (char **)&val, LY_BASE_DEC);
        ++(*tok_idx);

        /* ']' */
        assert(expr->tokens[*tok_idx] == LYXP_TOKEN_BRACK2);
        ++(*tok_idx);
    }

cleanup:
    LOG_LOCBACK(cur_node ? 1 : 0, 0);
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
 * @return LY_ERR value.
 */
static LY_ERR
ly_path_compile_predicate_leafref(const struct lysc_node *ctx_node, const struct lysc_node *cur_node,
        const struct lyxp_expr *expr, uint32_t *tok_idx, LY_VALUE_FORMAT format, void *prefix_data)
{
    LY_ERR ret = LY_SUCCESS;
    const struct lysc_node *key, *node, *node2;
    struct ly_ctx *ctx = cur_node->module->ctx;

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
        ret = ly_path_compile_snode(ctx, cur_node, cur_node->module, ctx_node, expr, *tok_idx, format, prefix_data,
                NULL, 0, &key, NULL);
        LY_CHECK_GOTO(ret, cleanup);
        if ((key->nodetype != LYS_LEAF) || !(key->flags & LYS_KEY)) {
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
            LY_CHECK_RET(ly_path_compile_snode(ctx, cur_node, cur_node->module, node, expr, *tok_idx, format,
                    prefix_data, NULL, 0, &node2, NULL));
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
    return (ret == LY_ENOTFOUND) ? LY_EVALID : ret;
}

/**
 * @brief Duplicate ly_path_predicate structure.
 *
 * @param[in] ctx libyang context.
 * @param[in] pred The array of path predicates.
 * @param[out] dup Duplicated predicates.
 * @return LY_ERR value.
 */
static LY_ERR
ly_path_dup_predicates(const struct ly_ctx *ctx, const struct ly_path_predicate *pred, struct ly_path_predicate **dup)
{
    LY_ARRAY_COUNT_TYPE u;

    if (!pred) {
        return LY_SUCCESS;
    }

    LY_ARRAY_CREATE_RET(ctx, *dup, LY_ARRAY_COUNT(pred), LY_EMEM);
    LY_ARRAY_FOR(pred, u) {
        LY_ARRAY_INCREMENT(*dup);
        (*dup)[u].type = pred->type;

        switch (pred[u].type) {
        case LY_PATH_PREDTYPE_POSITION:
            /* position-predicate */
            (*dup)[u].position = pred[u].position;
            break;
        case LY_PATH_PREDTYPE_LIST:
        case LY_PATH_PREDTYPE_LEAFLIST:
            /* key-predicate or leaf-list-predicate */
            (*dup)[u].key = pred[u].key;
            pred[u].value.realtype->plugin->duplicate(ctx, &pred[u].value, &(*dup)[u].value);
            LY_ATOMIC_INC_BARRIER(((struct lysc_type *)pred[u].value.realtype)->refcount);
            break;
        case LY_PATH_PREDTYPE_LIST_VAR:
            /* key-predicate with a variable */
            (*dup)[u].key = pred[u].key;
            (*dup)[u].variable = strdup(pred[u].variable);
            break;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Appends path elements from source to destination array
 *
 * @param[in] ctx libyang context.
 * @param[in] src The source path
 * @param[in,out] dst The destination path
 * @return LY_ERR value.
 */
static LY_ERR
ly_path_append(const struct ly_ctx *ctx, const struct ly_path *src, struct ly_path **dst)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    struct ly_path *p;

    if (!src) {
        return LY_SUCCESS;
    }

    LY_ARRAY_CREATE_RET(ctx, *dst, LY_ARRAY_COUNT(src), LY_EMEM);
    LY_ARRAY_FOR(src, u) {
        LY_ARRAY_NEW_GOTO(ctx, *dst, p, ret, cleanup);
        p->node = src[u].node;
        p->ext = src[u].ext;
        LY_CHECK_GOTO(ret = ly_path_dup_predicates(ctx, src[u].predicates, &p->predicates), cleanup);
    }

cleanup:
    return ret;
}

/**
 * @brief Compile deref XPath function into ly_path structure.
 *
 * @param[in] ctx libyang context.
 * @param[in] ctx_node Optional context node, mandatory of @p lref.
 * @param[in] top_ext Extension instance containing the definition of the data being created. It is used to find
 * the top-level node inside the extension instance instead of a module. Note that this is the case not only if
 * the @p ctx_node is NULL, but also if the relative path starting in @p ctx_node reaches the document root
 * via double dots.
 * @param[in] expr Parsed path.
 * @param[in] oper Oper option (@ref path_oper_options).
 * @param[in] target Target option (@ref path_target_options).
 * @param[in] format Format of the path.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[in,out] tok_idx Index in @p exp, is adjusted.
 * @param[out] path Compiled path.
 * @return LY_ERR value.
 */
static LY_ERR
ly_path_compile_deref(const struct ly_ctx *ctx, const struct lysc_node *ctx_node,
        const struct lysc_ext_instance *top_ext, const struct lyxp_expr *expr, uint16_t oper, uint16_t target,
        LY_VALUE_FORMAT format, void *prefix_data, uint32_t *tok_idx, struct ly_path **path)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_expr expr2;
    struct ly_path *path2 = NULL;
    const struct lysc_node *node2;
    const struct lysc_node_leaf *deref_leaf_node;
    const struct lysc_type_leafref *lref;
    uint32_t begin_token;

    *path = NULL;

    /* properly parsed path must always starts with 'deref' and '(' */
    assert(!lyxp_check_token(NULL, expr, *tok_idx, LYXP_TOKEN_FUNCNAME));
    assert(!strncmp(&expr->expr[expr->tok_pos[*tok_idx]], "deref", 5));
    (*tok_idx)++;
    assert(!lyxp_check_token(NULL, expr, *tok_idx, LYXP_TOKEN_PAR1));
    (*tok_idx)++;
    begin_token = *tok_idx;

    /* emebedded functions were already identified count tokens till ')' */
    while (lyxp_check_token(NULL, expr, *tok_idx, LYXP_TOKEN_PAR2) && (*tok_idx < expr->used)) {
        (*tok_idx)++;
    }

    /* properly parsed path must have ')' within the tokens */
    assert(!lyxp_check_token(NULL, expr, *tok_idx, LYXP_TOKEN_PAR2));

    /* prepare expr representing just deref arg */
    expr2.tokens = &expr->tokens[begin_token];
    expr2.tok_pos = &expr->tok_pos[begin_token];
    expr2.tok_len = &expr->tok_len[begin_token];
    expr2.repeat = &expr->repeat[begin_token];
    expr2.used = *tok_idx - begin_token;
    expr2.size = expr->size - begin_token;
    expr2.expr = expr->expr;

    /* compile just deref arg, append it to the path and find dereferenced lref for next operations */
    LY_CHECK_GOTO(ret = ly_path_compile_leafref(ctx, ctx_node, top_ext, &expr2, oper, target, format, prefix_data,
            &path2), cleanup);
    node2 = path2[LY_ARRAY_COUNT(path2) - 1].node;
    if ((node2->nodetype != LYS_LEAF) && (node2->nodetype != LYS_LEAFLIST)) {
        LOGVAL(ctx, LYVE_XPATH, "The deref function target node \"%s\" is not leaf nor leaflist", node2->name);
        ret = LY_EVALID;
        goto cleanup;
    }
    deref_leaf_node = (const struct lysc_node_leaf *)node2;
    if (deref_leaf_node->type->basetype != LY_TYPE_LEAFREF) {
        LOGVAL(ctx, LYVE_XPATH, "The deref function target node \"%s\" is not leafref", node2->name);
        ret = LY_EVALID;
        goto cleanup;
    }
    lref = (const struct lysc_type_leafref *)deref_leaf_node->type;
    LY_CHECK_GOTO(ret = ly_path_append(ctx, path2, path), cleanup);
    ly_path_free(ctx, path2);
    path2 = NULL;

    /* compile dereferenced leafref expression and append it to the path */
    LY_CHECK_GOTO(ret = ly_path_compile_leafref(ctx, node2, top_ext, lref->path, oper, target, format, prefix_data,
            &path2), cleanup);
    node2 = path2[LY_ARRAY_COUNT(path2) - 1].node;
    LY_CHECK_GOTO(ret = ly_path_append(ctx, path2, path), cleanup);
    ly_path_free(ctx, path2);
    path2 = NULL;

    /* properly parsed path must always continue with ')' and '/' */
    assert(!lyxp_check_token(NULL, expr, *tok_idx, LYXP_TOKEN_PAR2));
    (*tok_idx)++;
    assert(!lyxp_check_token(NULL, expr, *tok_idx, LYXP_TOKEN_OPER_PATH));
    (*tok_idx)++;

    /* prepare expr representing rest of the path after deref */
    expr2.tokens = &expr->tokens[*tok_idx];
    expr2.tok_pos = &expr->tok_pos[*tok_idx];
    expr2.tok_len = &expr->tok_len[*tok_idx];
    expr2.repeat = &expr->repeat[*tok_idx];
    expr2.used = expr->used - *tok_idx;
    expr2.size = expr->size - *tok_idx;
    expr2.expr = expr->expr;

    /* compile rest of the path and append it to the path */
    LY_CHECK_GOTO(ret = ly_path_compile_leafref(ctx, node2, top_ext, &expr2, oper, target, format, prefix_data, &path2),
            cleanup);
    LY_CHECK_GOTO(ret = ly_path_append(ctx, path2, path), cleanup);

cleanup:
    ly_path_free(ctx, path2);
    if (ret) {
        ly_path_free(ctx, *path);
        *path = NULL;
    }
    return ret;
}

/**
 * @brief Compile path into ly_path structure. Any predicates of a leafref are only checked, not compiled.
 *
 * @param[in] ctx libyang context.
 * @param[in] cur_mod Current module of the path (where it was "instantiated"), ignored of @p lref. Used for nodes
 * without a prefix for ::LY_VALUE_SCHEMA and ::LY_VALUE_SCHEMA_RESOLVED format.
 * @param[in] ctx_node Optional context node, mandatory of @p lref.
 * @param[in] top_ext Extension instance containing the definition of the data being created. It is used to find the top-level
 * node inside the extension instance instead of a module. Note that this is the case not only if the @p ctx_node is NULL,
 * but also if the relative path starting in @p ctx_node reaches the document root via double dots.
 * @param[in] expr Parsed path.
 * @param[in] lref Whether leafref is being compiled or not.
 * @param[in] oper Oper option (@ref path_oper_options).
 * @param[in] target Target option (@ref path_target_options).
 * @param[in] limit_access_tree Whether to limit accessible tree as described in
 * [XPath context](https://datatracker.ietf.org/doc/html/rfc7950#section-6.4.1).
 * @param[in] format Format of the path.
 * @param[in] prefix_data Format-specific data for resolving any prefixes (see ::ly_resolve_prefix).
 * @param[out] path Compiled path.
 * @return LY_ERECOMPILE, only if @p lref.
 * @return LY_ERR value.
 */
static LY_ERR
_ly_path_compile(const struct ly_ctx *ctx, const struct lys_module *cur_mod, const struct lysc_node *ctx_node,
        const struct lysc_ext_instance *top_ext, const struct lyxp_expr *expr, ly_bool lref, uint16_t oper, uint16_t target,
        ly_bool limit_access_tree, LY_VALUE_FORMAT format, void *prefix_data, struct ly_path **path)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t tok_idx = 0, getnext_opts;
    const struct lysc_node *node2, *cur_node, *op;
    struct ly_path *p = NULL;
    struct lysc_ext_instance *ext = NULL;

    assert(ctx);
    assert(!lref || ctx_node);
    assert((oper == LY_PATH_OPER_INPUT) || (oper == LY_PATH_OPER_OUTPUT));
    assert((target == LY_PATH_TARGET_SINGLE) || (target == LY_PATH_TARGET_MANY));

    if (!limit_access_tree) {
        op = NULL;
    } else {
        /* find operation, if we are in any */
        for (op = ctx_node; op && !(op->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)); op = op->parent) {}
    }

    *path = NULL;

    /* remember original context node */
    cur_node = ctx_node;
    if (cur_node) {
        LOG_LOCSET(cur_node, NULL);
    }

    if (oper == LY_PATH_OPER_OUTPUT) {
        getnext_opts = LYS_GETNEXT_OUTPUT;
    } else {
        getnext_opts = 0;
    }

    if (lref && (ly_ctx_get_options(ctx) & LY_CTX_LEAFREF_EXTENDED) &&
            (expr->tokens[tok_idx] == LYXP_TOKEN_FUNCNAME)) {
        /* deref function */
        ret = ly_path_compile_deref(ctx, ctx_node, top_ext, expr, oper, target, format, prefix_data, &tok_idx, path);
        goto cleanup;
    } else if (expr->tokens[tok_idx] == LYXP_TOKEN_OPER_PATH) {
        /* absolute path */
        ctx_node = NULL;

        ++tok_idx;
    } else {
        /* relative path */
        if (!ctx_node) {
            LOGVAL(ctx, LYVE_XPATH, "No initial schema parent for a relative path.");
            ret = LY_EVALID;
            goto cleanup;
        }

        /* go up the parents for leafref */
        while (lref && (expr->tokens[tok_idx] == LYXP_TOKEN_DDOT)) {
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
    }

    do {
        /* check last compiled inner node, whether it is uniquely identified (even key-less list) */
        if (p && !lref && (target == LY_PATH_TARGET_SINGLE) && (p->node->nodetype == LYS_LIST) && !p->predicates) {
            LOGVAL(ctx, LYVE_XPATH, "Predicate missing for %s \"%s\" in path.",
                    lys_nodetype2str(p->node->nodetype), p->node->name);
            ret = LY_EVALID;
            goto cleanup;
        }

        /* NameTest */
        LY_CHECK_ERR_GOTO(lyxp_check_token(ctx, expr, tok_idx, LYXP_TOKEN_NAMETEST), ret = LY_EVALID, cleanup);

        /* get schema node */
        LY_CHECK_GOTO(ret = ly_path_compile_snode(ctx, cur_node, cur_mod, ctx_node, expr, tok_idx, format, prefix_data,
                top_ext, getnext_opts, &node2, &ext), cleanup);
        ++tok_idx;
        if ((op && (node2->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) && (node2 != op))) {
            LOGVAL(ctx, LYVE_XPATH, "Not found node \"%s\" in path.", node2->name);
            ret = LY_EVALID;
            goto cleanup;
        }
        ctx_node = node2;

        /* new path segment */
        LY_ARRAY_NEW_GOTO(ctx, *path, p, ret, cleanup);
        p->node = ctx_node;
        p->ext = ext;

        /* compile any predicates */
        if (lref) {
            ret = ly_path_compile_predicate_leafref(ctx_node, cur_node, expr, &tok_idx, format, prefix_data);
        } else {
            ret = ly_path_compile_predicate(ctx, cur_node, cur_mod, ctx_node, expr, &tok_idx, format, prefix_data,
                    &p->predicates);
        }
        LY_CHECK_GOTO(ret, cleanup);
    } while (!lyxp_next_token(NULL, expr, &tok_idx, LYXP_TOKEN_OPER_PATH));

    /* check leftover tokens */
    if (tok_idx < expr->used) {
        LOGVAL(ctx, LY_VCODE_XP_INTOK, lyxp_token2str(expr->tokens[tok_idx]), &expr->expr[expr->tok_pos[tok_idx]]);
        ret = LY_EVALID;
        goto cleanup;
    }

    /* check last compiled node */
    if (!lref && (target == LY_PATH_TARGET_SINGLE) && (p->node->nodetype & (LYS_LIST | LYS_LEAFLIST)) && !p->predicates) {
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
    LOG_LOCBACK(cur_node ? 1 : 0, 0);
    return (ret == LY_ENOTFOUND) ? LY_EVALID : ret;
}

LY_ERR
ly_path_compile(const struct ly_ctx *ctx, const struct lys_module *cur_mod, const struct lysc_node *ctx_node,
        const struct lysc_ext_instance *top_ext, const struct lyxp_expr *expr, uint16_t oper, uint16_t target,
        ly_bool limit_access_tree, LY_VALUE_FORMAT format, void *prefix_data, struct ly_path **path)
{
    return _ly_path_compile(ctx, cur_mod, ctx_node, top_ext, expr, 0, oper, target, limit_access_tree, format,
            prefix_data, path);
}

LY_ERR
ly_path_compile_leafref(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const struct lysc_ext_instance *top_ext,
        const struct lyxp_expr *expr, uint16_t oper, uint16_t target, LY_VALUE_FORMAT format, void *prefix_data,
        struct ly_path **path)
{
    return _ly_path_compile(ctx, ctx_node->module, ctx_node, top_ext, expr, 1, oper, target, 1, format, prefix_data, path);
}

LY_ERR
ly_path_eval_partial(const struct ly_path *path, const struct lyd_node *start, const struct lyxp_var *vars,
        ly_bool with_opaq, LY_ARRAY_COUNT_TYPE *path_idx, struct lyd_node **match)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lyd_node *prev_node = NULL, *elem, *node = NULL, *target;
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
        if (path[u].predicates) {
            switch (path[u].predicates[0].type) {
            case LY_PATH_PREDTYPE_POSITION:
                /* we cannot use hashes and want an instance on a specific position */
                pos = 1;
                node = NULL;
                LYD_LIST_FOR_INST(start, path[u].node, elem) {
                    if (pos == path[u].predicates[0].position) {
                        node = elem;
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
            case LY_PATH_PREDTYPE_LIST_VAR:
            case LY_PATH_PREDTYPE_LIST:
                /* we will use hashes to find one list instance */
                LY_CHECK_RET(lyd_create_list(path[u].node, path[u].predicates, vars, 1, &target));
                lyd_find_sibling_first(start, target, &node);
                lyd_free_tree(target);
                break;
            }
        } else {
            /* we will use hashes to find one any/container/leaf instance */
            if (lyd_find_sibling_val(start, path[u].node, NULL, 0, &node) && with_opaq) {
                if (!lyd_find_sibling_opaq_next(start, path[u].node->name, &node) &&
                        (lyd_node_module(node) != path[u].node->module)) {
                    /* non-matching opaque node */
                    node = NULL;
                }
            }
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
ly_path_eval(const struct ly_path *path, const struct lyd_node *start, const struct lyxp_var *vars, struct lyd_node **match)
{
    LY_ERR ret;
    struct lyd_node *m;

    ret = ly_path_eval_partial(path, start, vars, 0, NULL, &m);

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
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;

    if (!path) {
        return LY_SUCCESS;
    }

    LY_ARRAY_CREATE_RET(ctx, *dup, LY_ARRAY_COUNT(path), LY_EMEM);
    LY_ARRAY_FOR(path, u) {
        LY_ARRAY_INCREMENT(*dup);
        (*dup)[u].node = path[u].node;
        (*dup)[u].ext = path[u].ext;
        LY_CHECK_RET(ret = ly_path_dup_predicates(ctx, path[u].predicates, &(*dup)[u].predicates), ret);
    }

    return LY_SUCCESS;
}

void
ly_path_predicates_free(const struct ly_ctx *ctx, struct ly_path_predicate *predicates)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysf_ctx fctx = {.ctx = (struct ly_ctx *)ctx};

    if (!predicates) {
        return;
    }

    LY_ARRAY_FOR(predicates, u) {
        switch (predicates[u].type) {
        case LY_PATH_PREDTYPE_POSITION:
            /* nothing to free */
            break;
        case LY_PATH_PREDTYPE_LIST:
        case LY_PATH_PREDTYPE_LEAFLIST:
            if (predicates[u].value.realtype) {
                predicates[u].value.realtype->plugin->free(ctx, &predicates[u].value);
                lysc_type_free(&fctx, (struct lysc_type *)predicates[u].value.realtype);
            }
            break;
        case LY_PATH_PREDTYPE_LIST_VAR:
            free(predicates[u].variable);
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
        ly_path_predicates_free(ctx, path[u].predicates);
    }
    LY_ARRAY_FREE(path);
}
