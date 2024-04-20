/**
 * @file schema_compile_amend.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Schema compilation of augments, deviations, and refines.
 *
 * Copyright (c) 2015 - 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "schema_compile_amend.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "dict.h"
#include "log.h"
#include "schema_compile.h"
#include "schema_compile_node.h"
#include "schema_features.h"
#include "set.h"
#include "tree.h"
#include "tree_data_internal.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xpath.h"

/**
 * @brief Get module of a single nodeid node name test.
 *
 * @param[in] ctx libyang context.
 * @param[in] nametest Nametest with an optional prefix.
 * @param[in] nametest_len Length of @p nametest.
 * @param[in] mod Both current and prefix module for resolving prefixes and to return in case of no prefix.
 * @param[out] name Optional pointer to the name test without the prefix.
 * @param[out] name_len Length of @p name.
 * @return Resolved module.
 */
static const struct lys_module *
lys_schema_node_get_module(const struct ly_ctx *ctx, const char *nametest, size_t nametest_len,
        const struct lysp_module *mod, const char **name, size_t *name_len)
{
    const struct lys_module *target_mod;
    const char *ptr;

    ptr = ly_strnchr(nametest, ':', nametest_len);
    if (ptr) {
        target_mod = ly_resolve_prefix(ctx, nametest, ptr - nametest, LY_VALUE_SCHEMA, (void *)mod);
        if (!target_mod) {
            LOGVAL(ctx, LYVE_REFERENCE,
                    "Invalid absolute-schema-nodeid nametest \"%.*s\" - prefix \"%.*s\" not defined in module \"%s\".",
                    (int)nametest_len, nametest, (int)(ptr - nametest), nametest, LYSP_MODULE_NAME(mod));
            return NULL;
        }

        if (name) {
            *name = ptr + 1;
            *name_len = nametest_len - ((ptr - nametest) + 1);
        }
    } else {
        target_mod = mod->mod;
        if (name) {
            *name = nametest;
            *name_len = nametest_len;
        }
    }

    return target_mod;
}

/**
 * @brief Check the syntax of a node-id and collect all the referenced modules.
 *
 * @param[in] ctx Compile context.
 * @param[in] nodeid Node-id to check.
 * @param[in] abs Whether @p nodeid is absolute.
 * @param[in,out] mod_set Set to add referenced modules into.
 * @param[out] expr Optional node-id parsed into an expression.
 * @param[out] target_mod Optional target module of the node-id.
 * @return LY_ERR value.
 */
static LY_ERR
lys_nodeid_mod_check(struct lysc_ctx *ctx, const char *nodeid, ly_bool abs, struct ly_set *mod_set,
        struct lyxp_expr **expr, struct lys_module **target_mod)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_expr *e = NULL;
    struct lys_module *tmod = NULL, *mod;
    const char *nodeid_type = abs ? "absolute-schema-nodeid" : "descendant-schema-nodeid";
    uint32_t i;

    /* parse */
    ret = lyxp_expr_parse(ctx->ctx, nodeid, strlen(nodeid), 0, &e);
    if (ret) {
        LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG, "Invalid %s value \"%s\" - invalid syntax.",
                nodeid_type, nodeid);
        ret = LY_EVALID;
        goto cleanup;
    }

    if (abs) {
        /* absolute schema nodeid */
        i = 0;
    } else {
        /* descendant schema nodeid */
        if (e->tokens[0] != LYXP_TOKEN_NAMETEST) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid %s value \"%s\" - name test expected instead of \"%.*s\".",
                    nodeid_type, nodeid, e->tok_len[0], e->expr + e->tok_pos[0]);
            ret = LY_EVALID;
            goto cleanup;
        }
        i = 1;
    }

    /* check all the tokens */
    for ( ; i < e->used; i += 2) {
        if (e->tokens[i] != LYXP_TOKEN_OPER_PATH) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid %s value \"%s\" - \"/\" expected instead of \"%.*s\".",
                    nodeid_type, nodeid, e->tok_len[i], e->expr + e->tok_pos[i]);
            ret = LY_EVALID;
            goto cleanup;
        } else if (e->used == i + 1) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE,
                    "Invalid %s value \"%s\" - unexpected end of expression.", nodeid_type, e->expr);
            ret = LY_EVALID;
            goto cleanup;
        } else if (e->tokens[i + 1] != LYXP_TOKEN_NAMETEST) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid %s value \"%s\" - name test expected instead of \"%.*s\".",
                    nodeid_type, nodeid, e->tok_len[i + 1], e->expr + e->tok_pos[i + 1]);
            ret = LY_EVALID;
            goto cleanup;
        } else if (abs) {
            mod = (struct lys_module *)lys_schema_node_get_module(ctx->ctx, e->expr + e->tok_pos[i + 1],
                    e->tok_len[i + 1], ctx->pmod, NULL, NULL);
            LY_CHECK_ERR_GOTO(!mod, ret = LY_EVALID, cleanup);

            /* only keep the first module */
            if (!tmod) {
                tmod = mod;
            }

            /* store the referenced module */
            LY_CHECK_GOTO(ret = ly_set_add(mod_set, mod, 0, NULL), cleanup);
        }
    }

cleanup:
    if (ret || !expr) {
        lyxp_expr_free(ctx->ctx, e);
        e = NULL;
    }
    if (expr) {
        *expr = ret ? NULL : e;
    }
    if (target_mod) {
        *target_mod = ret ? NULL : tmod;
    }
    return ret;
}

/**
 * @brief Check whether 2 schema nodeids match.
 *
 * @param[in] ctx libyang context.
 * @param[in] exp1 First schema nodeid.
 * @param[in] exp1p_mod Module of @p exp1 nodes without any prefix.
 * @param[in] exp2 Second schema nodeid.
 * @param[in] exp2_pmod Module of @p exp2 nodes without any prefix.
 * @return Whether the schema nodeids match or not.
 */
static ly_bool
lys_abs_schema_nodeid_match(const struct ly_ctx *ctx, const struct lyxp_expr *exp1, const struct lysp_module *exp1_pmod,
        const struct lyxp_expr *exp2, const struct lysp_module *exp2_pmod)
{
    uint32_t i;
    const struct lys_module *mod1, *mod2;
    const char *name1 = NULL, *name2 = NULL;
    size_t name1_len = 0, name2_len = 0;

    if (exp1->used != exp2->used) {
        return 0;
    }

    for (i = 0; i < exp1->used; ++i) {
        assert(exp1->tokens[i] == exp2->tokens[i]);

        if (exp1->tokens[i] == LYXP_TOKEN_NAMETEST) {
            /* check modules of all the nodes in the node ID */
            mod1 = lys_schema_node_get_module(ctx, exp1->expr + exp1->tok_pos[i], exp1->tok_len[i], exp1_pmod,
                    &name1, &name1_len);
            assert(mod1);
            mod2 = lys_schema_node_get_module(ctx, exp2->expr + exp2->tok_pos[i], exp2->tok_len[i], exp2_pmod,
                    &name2, &name2_len);
            assert(mod2);

            /* compare modules */
            if (mod1 != mod2) {
                return 0;
            }

            /* compare names */
            if ((name1_len != name2_len) || strncmp(name1, name2, name1_len)) {
                return 0;
            }
        }
    }

    return 1;
}

LY_ERR
lys_precompile_uses_augments_refines(struct lysc_ctx *ctx, struct lysp_node_uses *uses_p, const struct lysc_node *ctx_node)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_expr *exp = NULL;
    struct lysc_augment *aug;
    struct lysp_node_augment *aug_p;
    struct lysc_refine *rfn;
    struct lysp_refine **new_rfn;
    LY_ARRAY_COUNT_TYPE u;
    uint32_t i;
    struct ly_set mod_set = {0};

    LY_LIST_FOR(uses_p->augments, aug_p) {
        lysc_update_path(ctx, NULL, "{augment}");
        lysc_update_path(ctx, NULL, aug_p->nodeid);

        /* parse the nodeid */
        LY_CHECK_GOTO(ret = lys_nodeid_mod_check(ctx, aug_p->nodeid, 0, &mod_set, &exp, NULL), cleanup);

        /* allocate new compiled augment and store it in the set */
        aug = calloc(1, sizeof *aug);
        LY_CHECK_ERR_GOTO(!aug, LOGMEM(ctx->ctx); ret = LY_EMEM, cleanup);
        LY_CHECK_GOTO(ret = ly_set_add(&ctx->uses_augs, aug, 1, NULL), cleanup);

        aug->nodeid = exp;
        exp = NULL;
        aug->aug_pmod = ctx->pmod;
        aug->nodeid_ctx_node = ctx_node;
        aug->aug_p = aug_p;

        lysc_update_path(ctx, NULL, NULL);
        lysc_update_path(ctx, NULL, NULL);
    }

    LY_ARRAY_FOR(uses_p->refines, u) {
        lysc_update_path(ctx, NULL, "{refine}");
        lysc_update_path(ctx, NULL, uses_p->refines[u].nodeid);

        /* parse the nodeid */
        LY_CHECK_GOTO(ret = lys_nodeid_mod_check(ctx, uses_p->refines[u].nodeid, 0, &mod_set, &exp, NULL), cleanup);

        /* try to find the node in already compiled refines */
        rfn = NULL;
        for (i = 0; i < ctx->uses_rfns.count; ++i) {
            if (lys_abs_schema_nodeid_match(ctx->ctx, exp, ctx->pmod, ((struct lysc_refine *)ctx->uses_rfns.objs[i])->nodeid,
                    ctx->pmod)) {
                rfn = ctx->uses_rfns.objs[i];
                break;
            }
        }

        if (!rfn) {
            /* allocate new compiled refine */
            rfn = calloc(1, sizeof *rfn);
            LY_CHECK_ERR_GOTO(!rfn, LOGMEM(ctx->ctx); ret = LY_EMEM, cleanup);
            LY_CHECK_GOTO(ret = ly_set_add(&ctx->uses_rfns, rfn, 1, NULL), cleanup);

            rfn->nodeid = exp;
            exp = NULL;
            rfn->nodeid_pmod = ctx->cur_mod->parsed;
            rfn->nodeid_ctx_node = ctx_node;
            rfn->uses_p = uses_p;
        } else {
            /* just free exp */
            lyxp_expr_free(ctx->ctx, exp);
            exp = NULL;
        }

        /* add new parsed refine structure */
        LY_ARRAY_NEW_GOTO(ctx->ctx, rfn->rfns, new_rfn, ret, cleanup);
        *new_rfn = &uses_p->refines[u];

        lysc_update_path(ctx, NULL, NULL);
        lysc_update_path(ctx, NULL, NULL);
    }

cleanup:
    if (ret) {
        lysc_update_path(ctx, NULL, NULL);
        lysc_update_path(ctx, NULL, NULL);
    }
    /* should include only this module, will fail later if not */
    ly_set_erase(&mod_set, NULL);
    lyxp_expr_free(ctx->ctx, exp);
    return ret;
}

/**
 * @brief Duplicate parsed extension children, recursively.
 *
 * @param[in] ctx Context.
 * @param[in] orig_child First original child to duplicate.
 * @param[in,out] child Duplicated children to add to.
 * @return LY_ERR value.
 */
static LY_ERR
lysp_ext_children_dup(const struct ly_ctx *ctx, const struct lysp_stmt *orig_child, struct lysp_stmt **child)
{
    struct lysp_stmt *ch = NULL;

    assert(!*child);

    LY_LIST_FOR(orig_child, orig_child) {
        /* new child */
        if (!*child) {
            *child = ch = calloc(1, sizeof *ch);
            LY_CHECK_ERR_RET(!ch, LOGMEM(ctx), LY_EMEM);
        } else {
            ch->next = calloc(1, sizeof *ch);
            LY_CHECK_ERR_RET(!ch->next, LOGMEM(ctx), LY_EMEM);
            ch = ch->next;
        }

        /* fill */
        DUP_STRING_RET(ctx, orig_child->stmt, ch->stmt);
        ch->flags = orig_child->flags;
        DUP_STRING_RET(ctx, orig_child->arg, ch->arg);
        ch->format = orig_child->format;
        LY_CHECK_RET(ly_dup_prefix_data(ctx, orig_child->format, orig_child->prefix_data, &(ch->prefix_data)));
        ch->kw = orig_child->kw;

        /* recursive children */
        LY_CHECK_RET(lysp_ext_children_dup(ctx, orig_child->child, &ch->child));
    }

    return LY_SUCCESS;
}

/**
 * @brief Duplicate parsed extension instance.
 *
 * @param[in] ctx Context.
 * @param[in] pmod Current parsed module.
 * @param[in] parent Parent of the duplicated ext instance.
 * @param[in] parent_stmt Parent statement of the duplicated ext instance (should be @p parent).
 * @param[out] ext Duplicated ext instance to fill.
 * @return LY_ERR value.
 */
static LY_ERR
lysp_ext_dup(const struct ly_ctx *ctx, const struct lysp_module *pmod, void *parent, enum ly_stmt parent_stmt,
        const struct lysp_ext_instance *orig_ext, struct lysp_ext_instance *ext)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_set pmods = {0};
    struct lysp_ctx pctx = {.parsed_mods = &pmods};

    DUP_STRING_GOTO(ctx, orig_ext->name, ext->name, ret, cleanup);
    DUP_STRING_GOTO(ctx, orig_ext->argument, ext->argument, ret, cleanup);
    ext->format = orig_ext->format;
    LY_CHECK_GOTO(ret = ly_dup_prefix_data(ctx, orig_ext->format, orig_ext->prefix_data, &ext->prefix_data), cleanup);
    ext->def = orig_ext->def;

    ext->parent = parent;
    ext->parent_stmt = parent_stmt;
    ext->parent_stmt_index = orig_ext->parent_stmt_index;
    ext->flags = orig_ext->flags;
    ext->record = orig_ext->record;

    LY_CHECK_GOTO(ret = lysp_ext_children_dup(ctx, orig_ext->child, &ext->child), cleanup);
    if (ext->record && ext->record->plugin.parse) {
        /* parse again */
        LY_CHECK_GOTO(ret = ly_set_add(&pmods, pmod, 1, NULL), cleanup);
        LY_CHECK_GOTO(ret = ext->record->plugin.parse(&pctx, ext), cleanup);
    }

cleanup:
    ly_set_erase(&pmods, NULL);
    return ret;
}

static LY_ERR
lysp_restr_dup(const struct ly_ctx *ctx, const struct lysp_module *pmod, const struct lysp_restr *orig_restr,
        struct lysp_restr *restr)
{
    LY_ERR ret = LY_SUCCESS;

    if (orig_restr) {
        DUP_STRING(ctx, orig_restr->arg.str, restr->arg.str, ret);
        restr->arg.mod = orig_restr->arg.mod;
        DUP_STRING(ctx, orig_restr->emsg, restr->emsg, ret);
        DUP_STRING(ctx, orig_restr->eapptag, restr->eapptag, ret);
        DUP_STRING(ctx, orig_restr->dsc, restr->dsc, ret);
        DUP_STRING(ctx, orig_restr->ref, restr->ref, ret);
        DUP_EXTS(ctx, pmod, restr, LY_STMT_MUST, orig_restr->exts, restr->exts, lysp_ext_dup);
    }

    return ret;
}

static LY_ERR
lysp_string_dup(const struct ly_ctx *ctx, const char **orig_str, const char **str)
{
    LY_ERR ret = LY_SUCCESS;

    DUP_STRING(ctx, *orig_str, *str, ret);

    return ret;
}

LY_ERR
lysp_qname_dup(const struct ly_ctx *ctx, const struct lysp_qname *orig_qname, struct lysp_qname *qname)
{
    LY_ERR ret = LY_SUCCESS;

    if (!orig_qname->str) {
        return LY_SUCCESS;
    }

    DUP_STRING(ctx, orig_qname->str, qname->str, ret);
    assert(orig_qname->mod);
    qname->mod = orig_qname->mod;

    return ret;
}

static LY_ERR
lysp_type_enum_dup(const struct ly_ctx *ctx, const struct lysp_module *pmod, const struct lysp_type_enum *orig_enm,
        struct lysp_type_enum *enm)
{
    LY_ERR ret = LY_SUCCESS;

    DUP_STRING(ctx, orig_enm->name, enm->name, ret);
    DUP_STRING(ctx, orig_enm->dsc, enm->dsc, ret);
    DUP_STRING(ctx, orig_enm->ref, enm->ref, ret);
    enm->value = orig_enm->value;
    DUP_ARRAY(ctx, orig_enm->iffeatures, enm->iffeatures, lysp_qname_dup);
    DUP_EXTS(ctx, pmod, enm, LY_STMT_ENUM, orig_enm->exts, enm->exts, lysp_ext_dup);
    enm->flags = orig_enm->flags;

    return ret;
}

static LY_ERR
lysp_type_dup(const struct ly_ctx *ctx, const struct lysp_module *pmod, const struct lysp_type *orig_type,
        struct lysp_type *type)
{
    LY_ERR ret = LY_SUCCESS;

    /* array macros read previous data so we must zero it */
    memset(type, 0, sizeof *type);

    DUP_STRING_GOTO(ctx, orig_type->name, type->name, ret, done);

    if (orig_type->range) {
        type->range = calloc(1, sizeof *type->range);
        LY_CHECK_ERR_RET(!type->range, LOGMEM(ctx), LY_EMEM);
        LY_CHECK_RET(lysp_restr_dup(ctx, pmod, orig_type->range, type->range));
    }

    if (orig_type->length) {
        type->length = calloc(1, sizeof *type->length);
        LY_CHECK_ERR_RET(!type->length, LOGMEM(ctx), LY_EMEM);
        LY_CHECK_RET(lysp_restr_dup(ctx, pmod, orig_type->length, type->length));
    }

    DUP_ARRAY2(ctx, pmod, orig_type->patterns, type->patterns, lysp_restr_dup);
    DUP_ARRAY2(ctx, pmod, orig_type->enums, type->enums, lysp_type_enum_dup);
    DUP_ARRAY2(ctx, pmod, orig_type->bits, type->bits, lysp_type_enum_dup);
    LY_CHECK_GOTO(ret = lyxp_expr_dup(ctx, orig_type->path, 0, 0, &type->path), done);
    DUP_ARRAY(ctx, orig_type->bases, type->bases, lysp_string_dup);
    DUP_ARRAY2(ctx, pmod, orig_type->types, type->types, lysp_type_dup);
    DUP_EXTS(ctx, pmod, type, LY_STMT_TYPE, orig_type->exts, type->exts, lysp_ext_dup);

    type->pmod = orig_type->pmod;
    type->compiled = orig_type->compiled;

    type->fraction_digits = orig_type->fraction_digits;
    type->require_instance = orig_type->require_instance;
    type->flags = orig_type->flags;

done:
    return ret;
}

static LY_ERR
lysp_when_dup(const struct ly_ctx *ctx, const struct lysp_module *pmod, struct lysp_when *when,
        const struct lysp_when *orig_when)
{
    LY_ERR ret = LY_SUCCESS;

    DUP_STRING(ctx, orig_when->cond, when->cond, ret);
    DUP_STRING(ctx, orig_when->dsc, when->dsc, ret);
    DUP_STRING(ctx, orig_when->ref, when->ref, ret);
    DUP_EXTS(ctx, pmod, when, LY_STMT_WHEN, orig_when->exts, when->exts, lysp_ext_dup);

    return ret;
}

static LY_ERR
lysp_node_common_dup(const struct ly_ctx *ctx, const struct lysp_module *pmod, struct lysp_node *node,
        const struct lysp_node *orig)
{
    LY_ERR ret = LY_SUCCESS;

    node->parent = NULL;
    node->nodetype = orig->nodetype;
    node->flags = orig->flags;
    node->next = NULL;
    DUP_STRING(ctx, orig->name, node->name, ret);
    DUP_STRING(ctx, orig->dsc, node->dsc, ret);
    DUP_STRING(ctx, orig->ref, node->ref, ret);
    DUP_ARRAY(ctx, orig->iffeatures, node->iffeatures, lysp_qname_dup);
    DUP_EXTS(ctx, pmod, node, lyplg_ext_nodetype2stmt(node->nodetype), orig->exts, node->exts, lysp_ext_dup);

    return ret;
}

#define DUP_PWHEN(CTX, PMOD, ORIG, NEW) \
    if (ORIG) { \
        NEW = calloc(1, sizeof *NEW); \
        LY_CHECK_ERR_RET(!NEW, LOGMEM(CTX), LY_EMEM); \
        LY_CHECK_RET(lysp_when_dup(CTX, PMOD, NEW, ORIG)); \
    }

static LY_ERR
lysp_node_dup(const struct ly_ctx *ctx, const struct lysp_module *pmod, struct lysp_node *node,
        const struct lysp_node *orig)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysp_node_container *cont;
    const struct lysp_node_container *orig_cont;
    struct lysp_node_leaf *leaf;
    const struct lysp_node_leaf *orig_leaf;
    struct lysp_node_leaflist *llist;
    const struct lysp_node_leaflist *orig_llist;
    struct lysp_node_list *list;
    const struct lysp_node_list *orig_list;
    struct lysp_node_choice *choice;
    const struct lysp_node_choice *orig_choice;
    struct lysp_node_case *cas;
    const struct lysp_node_case *orig_cas;
    struct lysp_node_anydata *any;
    const struct lysp_node_anydata *orig_any;
    struct lysp_node_action *action;
    const struct lysp_node_action *orig_action;
    struct lysp_node_action_inout *action_inout;
    const struct lysp_node_action_inout *orig_action_inout;
    struct lysp_node_notif *notif;
    const struct lysp_node_notif *orig_notif;

    assert(orig->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_CHOICE | LYS_CASE | LYS_ANYDATA |
            LYS_RPC | LYS_ACTION | LYS_NOTIF));

    /* common part */
    LY_CHECK_RET(lysp_node_common_dup(ctx, pmod, node, orig));

    /* specific part */
    switch (node->nodetype) {
    case LYS_CONTAINER:
        cont = (struct lysp_node_container *)node;
        orig_cont = (const struct lysp_node_container *)orig;

        DUP_PWHEN(ctx, pmod, orig_cont->when, cont->when);
        DUP_ARRAY2(ctx, pmod, orig_cont->musts, cont->musts, lysp_restr_dup);
        DUP_STRING(ctx, orig_cont->presence, cont->presence, ret);
        /* we do not need the rest */
        break;
    case LYS_LEAF:
        leaf = (struct lysp_node_leaf *)node;
        orig_leaf = (const struct lysp_node_leaf *)orig;

        DUP_PWHEN(ctx, pmod, orig_leaf->when, leaf->when);
        DUP_ARRAY2(ctx, pmod, orig_leaf->musts, leaf->musts, lysp_restr_dup);
        LY_CHECK_RET(lysp_type_dup(ctx, pmod, &orig_leaf->type, &leaf->type));
        DUP_STRING(ctx, orig_leaf->units, leaf->units, ret);
        LY_CHECK_RET(lysp_qname_dup(ctx, &orig_leaf->dflt, &leaf->dflt));
        break;
    case LYS_LEAFLIST:
        llist = (struct lysp_node_leaflist *)node;
        orig_llist = (const struct lysp_node_leaflist *)orig;

        DUP_PWHEN(ctx, pmod, orig_llist->when, llist->when);
        DUP_ARRAY2(ctx, pmod, orig_llist->musts, llist->musts, lysp_restr_dup);
        LY_CHECK_RET(lysp_type_dup(ctx, pmod, &orig_llist->type, &llist->type));
        DUP_STRING(ctx, orig_llist->units, llist->units, ret);
        DUP_ARRAY(ctx, orig_llist->dflts, llist->dflts, lysp_qname_dup);
        llist->min = orig_llist->min;
        llist->max = orig_llist->max;
        break;
    case LYS_LIST:
        list = (struct lysp_node_list *)node;
        orig_list = (const struct lysp_node_list *)orig;

        DUP_PWHEN(ctx, pmod, orig_list->when, list->when);
        DUP_ARRAY2(ctx, pmod, orig_list->musts, list->musts, lysp_restr_dup);
        DUP_STRING(ctx, orig_list->key, list->key, ret);
        /* we do not need these arrays */
        DUP_ARRAY(ctx, orig_list->uniques, list->uniques, lysp_qname_dup);
        list->min = orig_list->min;
        list->max = orig_list->max;
        break;
    case LYS_CHOICE:
        choice = (struct lysp_node_choice *)node;
        orig_choice = (const struct lysp_node_choice *)orig;

        DUP_PWHEN(ctx, pmod, orig_choice->when, choice->when);
        /* we do not need children */
        LY_CHECK_RET(lysp_qname_dup(ctx, &orig_choice->dflt, &choice->dflt));
        break;
    case LYS_CASE:
        cas = (struct lysp_node_case *)node;
        orig_cas = (const struct lysp_node_case *)orig;

        DUP_PWHEN(ctx, pmod, orig_cas->when, cas->when);
        /* we do not need children */
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        any = (struct lysp_node_anydata *)node;
        orig_any = (const struct lysp_node_anydata *)orig;

        DUP_PWHEN(ctx, pmod, orig_any->when, any->when);
        DUP_ARRAY2(ctx, pmod, orig_any->musts, any->musts, lysp_restr_dup);
        break;
    case LYS_RPC:
    case LYS_ACTION:
        action = (struct lysp_node_action *)node;
        orig_action = (const struct lysp_node_action *)orig;

        action->input.nodetype = orig_action->input.nodetype;
        action->output.nodetype = orig_action->output.nodetype;
        /* we do not need the rest */
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        action_inout = (struct lysp_node_action_inout *)node;
        orig_action_inout = (const struct lysp_node_action_inout *)orig;

        DUP_ARRAY2(ctx, pmod, orig_action_inout->musts, action_inout->musts, lysp_restr_dup);
        /* we do not need the rest */
        break;
    case LYS_NOTIF:
        notif = (struct lysp_node_notif *)node;
        orig_notif = (const struct lysp_node_notif *)orig;

        DUP_ARRAY2(ctx, pmod, orig_notif->musts, notif->musts, lysp_restr_dup);
        /* we do not need the rest */
        break;
    default:
        LOGINT_RET(ctx);
    }

    return ret;
}

/**
 * @brief Duplicate a single parsed node. Only attributes that are used in compilation are copied.
 *
 * @param[in] ctx libyang context.
 * @param[in] pmod Current parsed module.
 * @param[in] pnode Node to duplicate.
 * @param[in] with_links Whether to also copy any links (child, parent pointers).
 * @param[out] dup_p Duplicated parsed node.
 * @return LY_ERR value.
 */
static LY_ERR
lysp_dup_single(struct lysc_ctx *cctx, const struct lysp_node *pnode, ly_bool with_links, struct lysp_node **dup_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysp_node *dup = NULL;

    if (!pnode) {
        *dup_p = NULL;
        return LY_SUCCESS;
    }

    switch (pnode->nodetype) {
    case LYS_CONTAINER:
        dup = calloc(1, sizeof(struct lysp_node_container));
        break;
    case LYS_LEAF:
        dup = calloc(1, sizeof(struct lysp_node_leaf));
        break;
    case LYS_LEAFLIST:
        dup = calloc(1, sizeof(struct lysp_node_leaflist));
        break;
    case LYS_LIST:
        dup = calloc(1, sizeof(struct lysp_node_list));
        break;
    case LYS_CHOICE:
        dup = calloc(1, sizeof(struct lysp_node_choice));
        break;
    case LYS_CASE:
        dup = calloc(1, sizeof(struct lysp_node_case));
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        dup = calloc(1, sizeof(struct lysp_node_anydata));
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        dup = calloc(1, sizeof(struct lysp_node_action_inout));
        break;
    case LYS_ACTION:
    case LYS_RPC:
        dup = calloc(1, sizeof(struct lysp_node_action));
        break;
    case LYS_NOTIF:
        dup = calloc(1, sizeof(struct lysp_node_notif));
        break;
    default:
        LOGINT_RET(cctx->ctx);
    }
    LY_CHECK_ERR_GOTO(!dup, LOGMEM(cctx->ctx); ret = LY_EMEM, cleanup);
    LY_CHECK_GOTO(ret = lysp_node_dup(cctx->ctx, cctx->pmod, dup, pnode), cleanup);

    if (with_links) {
        /* copy also parent, child, action, and notification pointers */
        dup->parent = pnode->parent;
        switch (pnode->nodetype) {
        case LYS_CONTAINER:
            ((struct lysp_node_container *)dup)->child = ((struct lysp_node_container *)pnode)->child;
            ((struct lysp_node_container *)dup)->actions = ((struct lysp_node_container *)pnode)->actions;
            ((struct lysp_node_container *)dup)->notifs = ((struct lysp_node_container *)pnode)->notifs;
            break;
        case LYS_LIST:
            ((struct lysp_node_list *)dup)->child = ((struct lysp_node_list *)pnode)->child;
            ((struct lysp_node_list *)dup)->actions = ((struct lysp_node_list *)pnode)->actions;
            ((struct lysp_node_list *)dup)->notifs = ((struct lysp_node_list *)pnode)->notifs;
            break;
        case LYS_CHOICE:
            ((struct lysp_node_choice *)dup)->child = ((struct lysp_node_choice *)pnode)->child;
            break;
        case LYS_CASE:
            ((struct lysp_node_case *)dup)->child = ((struct lysp_node_case *)pnode)->child;
            break;
        default:
            break;
        }
    }

cleanup:
    if (ret) {
        lysp_dev_node_free(cctx, dup);
    } else {
        *dup_p = dup;
    }
    return ret;
}

#define AMEND_WRONG_NODETYPE(AMEND_STR, OP_STR, PROPERTY) \
    LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid %s of %s node - it is not possible to %s \"%s\" property.", \
            AMEND_STR, lys_nodetype2str(target->nodetype), OP_STR, PROPERTY);\
    ret = LY_EVALID; \
    goto cleanup;

#define AMEND_CHECK_CARDINALITY(ARRAY, MAX, AMEND_STR, PROPERTY) \
    if (LY_ARRAY_COUNT(ARRAY) > MAX) { \
        LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Invalid %s of %s with too many (%"LY_PRI_ARRAY_COUNT_TYPE") %s properties.", \
               AMEND_STR, lys_nodetype2str(target->nodetype), LY_ARRAY_COUNT(ARRAY), PROPERTY); \
        ret = LY_EVALID; \
        goto cleanup; \
    }

/**
 * @brief Apply refine.
 *
 * @param[in] ctx Compile context.
 * @param[in] rfn Refine to apply.
 * @param[in] rfn_pmod Local module fo the refine.
 * @param[in,out] target Refine target.
 * @return LY_ERR value.
 */
static LY_ERR
lys_apply_refine(struct lysc_ctx *ctx, struct lysp_refine *rfn, const struct lysp_module *rfn_pmod, struct lysp_node *target)
{
    LY_ERR ret = LY_SUCCESS;
    struct lys_module *orig_mod = ctx->cur_mod;
    struct lysp_module *orig_pmod = ctx->pmod;
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_qname *qname;
    struct lysp_restr **musts, *must;
    uint32_t *num;

    /* use module from the refine */
    ctx->cur_mod = rfn_pmod->mod;
    ctx->pmod = (struct lysp_module *)rfn_pmod;

    /* keep the current path and add to it */
    lysc_update_path(ctx, NULL, "{refine}");
    lysc_update_path(ctx, NULL, rfn->nodeid);

    /* default value */
    if (rfn->dflts) {
        switch (target->nodetype) {
        case LYS_LEAF:
            AMEND_CHECK_CARDINALITY(rfn->dflts, 1, "refine", "default");

            lydict_remove(ctx->ctx, ((struct lysp_node_leaf *)target)->dflt.str);
            LY_CHECK_GOTO(ret = lysp_qname_dup(ctx->ctx, &rfn->dflts[0], &((struct lysp_node_leaf *)target)->dflt), cleanup);
            break;
        case LYS_LEAFLIST:
            if (rfn->dflts[0].mod->version < LYS_VERSION_1_1) {
                LOGVAL(ctx->ctx, LYVE_SEMANTICS,
                        "Invalid refine of default in leaf-list - the default statement is allowed only in YANG 1.1 modules.");
                ret = LY_EVALID;
                goto cleanup;
            }

            FREE_ARRAY(ctx->ctx, ((struct lysp_node_leaflist *)target)->dflts, lysp_qname_free);
            ((struct lysp_node_leaflist *)target)->dflts = NULL;
            LY_ARRAY_FOR(rfn->dflts, u) {
                LY_ARRAY_NEW_GOTO(ctx->ctx, ((struct lysp_node_leaflist *)target)->dflts, qname, ret, cleanup);
                LY_CHECK_GOTO(ret = lysp_qname_dup(ctx->ctx, &rfn->dflts[u], qname), cleanup);
            }
            break;
        case LYS_CHOICE:
            AMEND_CHECK_CARDINALITY(rfn->dflts, 1, "refine", "default");

            lydict_remove(ctx->ctx, ((struct lysp_node_choice *)target)->dflt.str);
            LY_CHECK_GOTO(ret = lysp_qname_dup(ctx->ctx, &rfn->dflts[0], &((struct lysp_node_choice *)target)->dflt), cleanup);
            break;
        default:
            AMEND_WRONG_NODETYPE("refine", "replace", "default");
        }
    }

    /* description */
    if (rfn->dsc) {
        lydict_remove(ctx->ctx, target->dsc);
        DUP_STRING_GOTO(ctx->ctx, rfn->dsc, target->dsc, ret, cleanup);
    }

    /* reference */
    if (rfn->ref) {
        lydict_remove(ctx->ctx, target->ref);
        DUP_STRING_GOTO(ctx->ctx, rfn->ref, target->ref, ret, cleanup);
    }

    /* config */
    if (rfn->flags & LYS_CONFIG_MASK) {
        if (ctx->compile_opts & LYS_COMPILE_NO_CONFIG) {
            LOGWRN(ctx->ctx, "Refining config inside %s has no effect (%s).",
                    (ctx->compile_opts & (LYS_IS_INPUT | LYS_IS_OUTPUT)) ? "RPC/action" :
                    ctx->compile_opts & LYS_IS_NOTIF ? "notification" : "a subtree ignoring config", ctx->path);
        } else {
            target->flags &= ~LYS_CONFIG_MASK;
            target->flags |= rfn->flags & LYS_CONFIG_MASK;
        }
    }

    /* mandatory */
    if (rfn->flags & LYS_MAND_MASK) {
        switch (target->nodetype) {
        case LYS_LEAF:
        case LYS_CHOICE:
        case LYS_ANYDATA:
        case LYS_ANYXML:
            break;
        default:
            AMEND_WRONG_NODETYPE("refine", "replace", "mandatory");
        }

        target->flags &= ~LYS_MAND_MASK;
        target->flags |= rfn->flags & LYS_MAND_MASK;
    }

    /* presence */
    if (rfn->presence) {
        if (target->nodetype != LYS_CONTAINER) {
            AMEND_WRONG_NODETYPE("refine", "replace", "presence");
        }

        lydict_remove(ctx->ctx, ((struct lysp_node_container *)target)->presence);
        DUP_STRING_GOTO(ctx->ctx, rfn->presence, ((struct lysp_node_container *)target)->presence, ret, cleanup);
    }

    /* must */
    if (rfn->musts) {
        switch (target->nodetype) {
        case LYS_CONTAINER:
        case LYS_LIST:
        case LYS_LEAF:
        case LYS_LEAFLIST:
        case LYS_ANYDATA:
        case LYS_ANYXML:
            musts = &((struct lysp_node_container *)target)->musts;
            break;
        default:
            AMEND_WRONG_NODETYPE("refine", "add", "must");
        }

        LY_ARRAY_FOR(rfn->musts, u) {
            LY_ARRAY_NEW_GOTO(ctx->ctx, *musts, must, ret, cleanup);
            LY_CHECK_GOTO(ret = lysp_restr_dup(ctx->ctx, rfn_pmod, &rfn->musts[u], must), cleanup);
        }
    }

    /* min-elements */
    if (rfn->flags & LYS_SET_MIN) {
        switch (target->nodetype) {
        case LYS_LEAFLIST:
            num = &((struct lysp_node_leaflist *)target)->min;
            break;
        case LYS_LIST:
            num = &((struct lysp_node_list *)target)->min;
            break;
        default:
            AMEND_WRONG_NODETYPE("refine", "replace", "min-elements");
        }

        *num = rfn->min;
    }

    /* max-elements */
    if (rfn->flags & LYS_SET_MAX) {
        switch (target->nodetype) {
        case LYS_LEAFLIST:
            num = &((struct lysp_node_leaflist *)target)->max;
            break;
        case LYS_LIST:
            num = &((struct lysp_node_list *)target)->max;
            break;
        default:
            AMEND_WRONG_NODETYPE("refine", "replace", "max-elements");
        }

        *num = rfn->max;
    }

    /* if-feature */
    if (rfn->iffeatures) {
        switch (target->nodetype) {
        case LYS_LEAF:
        case LYS_LEAFLIST:
        case LYS_LIST:
        case LYS_CONTAINER:
        case LYS_CHOICE:
        case LYS_CASE:
        case LYS_ANYDATA:
        case LYS_ANYXML:
            break;
        default:
            AMEND_WRONG_NODETYPE("refine", "add", "if-feature");
        }

        LY_ARRAY_FOR(rfn->iffeatures, u) {
            LY_ARRAY_NEW_GOTO(ctx->ctx, target->iffeatures, qname, ret, cleanup);
            LY_CHECK_GOTO(ret = lysp_qname_dup(ctx->ctx, &rfn->iffeatures[u], qname), cleanup);
        }
    }

    /* extension instances */
    DUP_EXTS(ctx->ctx, rfn_pmod, target, lyplg_ext_nodetype2stmt(target->nodetype), rfn->exts, target->exts, lysp_ext_dup);

cleanup:
    ctx->cur_mod = orig_mod;
    ctx->pmod = orig_pmod;

    lysc_update_path(ctx, NULL, NULL);
    lysc_update_path(ctx, NULL, NULL);
    return ret;
}

/**
 * @brief Apply deviate add.
 *
 * @param[in] ctx Compile context.
 * @param[in] d Deviate add to apply.
 * @param[in,out] target Deviation target.
 * @return LY_ERR value.
 */
static LY_ERR
lys_apply_deviate_add(struct lysc_ctx *ctx, struct lysp_deviate_add *d, struct lysp_node *target)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_qname *qname;
    uint32_t *num;
    struct lysp_restr **musts, *must;

#define DEV_CHECK_NONPRESENCE(TYPE, MEMBER, PROPERTY, VALUEMEMBER) \
    if (((TYPE)target)->MEMBER) { \
        LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid deviation adding \"%s\" property which already exists (with value \"%s\").", \
                PROPERTY, ((TYPE)target)->VALUEMEMBER); \
        ret = LY_EVALID; \
        goto cleanup; \
    }

    /* [units-stmt] */
    if (d->units) {
        switch (target->nodetype) {
        case LYS_LEAF:
        case LYS_LEAFLIST:
            break;
        default:
            AMEND_WRONG_NODETYPE("deviation", "add", "units");
        }

        DEV_CHECK_NONPRESENCE(struct lysp_node_leaf *, units, "units", units);
        DUP_STRING_GOTO(ctx->ctx, d->units, ((struct lysp_node_leaf *)target)->units, ret, cleanup);
    }

    /* *must-stmt */
    if (d->musts) {
        musts = lysp_node_musts_p(target);
        if (!musts) {
            AMEND_WRONG_NODETYPE("deviation", "add", "must");
        }

        LY_ARRAY_FOR(d->musts, u) {
            LY_ARRAY_NEW_GOTO(ctx->ctx, *musts, must, ret, cleanup);
            LY_CHECK_GOTO(ret = lysp_restr_dup(ctx->ctx, ctx->pmod, &d->musts[u], must), cleanup);
        }
    }

    /* *unique-stmt */
    if (d->uniques) {
        if (target->nodetype != LYS_LIST) {
            AMEND_WRONG_NODETYPE("deviation", "add", "unique");
        }

        LY_ARRAY_FOR(d->uniques, u) {
            LY_ARRAY_NEW_GOTO(ctx->ctx, ((struct lysp_node_list *)target)->uniques, qname, ret, cleanup);
            LY_CHECK_GOTO(ret = lysp_qname_dup(ctx->ctx, &d->uniques[u], qname), cleanup);
        }
    }

    /* *default-stmt */
    if (d->dflts) {
        switch (target->nodetype) {
        case LYS_LEAF:
            AMEND_CHECK_CARDINALITY(d->dflts, 1, "deviation", "default");
            DEV_CHECK_NONPRESENCE(struct lysp_node_leaf *, dflt.str, "default", dflt.str);

            LY_CHECK_GOTO(ret = lysp_qname_dup(ctx->ctx, &d->dflts[0], &((struct lysp_node_leaf *)target)->dflt), cleanup);
            break;
        case LYS_LEAFLIST:
            LY_ARRAY_FOR(d->dflts, u) {
                LY_ARRAY_NEW_GOTO(ctx->ctx, ((struct lysp_node_leaflist *)target)->dflts, qname, ret, cleanup);
                LY_CHECK_GOTO(ret = lysp_qname_dup(ctx->ctx, &d->dflts[u], qname), cleanup);
            }
            break;
        case LYS_CHOICE:
            AMEND_CHECK_CARDINALITY(d->dflts, 1, "deviation", "default");
            DEV_CHECK_NONPRESENCE(struct lysp_node_choice *, dflt.str, "default", dflt.str);

            LY_CHECK_GOTO(ret = lysp_qname_dup(ctx->ctx, &d->dflts[0], &((struct lysp_node_choice *)target)->dflt), cleanup);
            break;
        default:
            AMEND_WRONG_NODETYPE("deviation", "add", "default");
        }
    }

    /* [config-stmt] */
    if (d->flags & LYS_CONFIG_MASK) {
        switch (target->nodetype) {
        case LYS_CONTAINER:
        case LYS_LEAF:
        case LYS_LEAFLIST:
        case LYS_LIST:
        case LYS_CHOICE:
        case LYS_ANYDATA:
        case LYS_ANYXML:
            break;
        default:
            AMEND_WRONG_NODETYPE("deviation", "add", "config");
        }

        if (target->flags & LYS_CONFIG_MASK) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE,
                    "Invalid deviation adding \"config\" property which already exists (with value \"config %s\").",
                    target->flags & LYS_CONFIG_W ? "true" : "false");
            ret = LY_EVALID;
            goto cleanup;
        }

        target->flags |= d->flags & LYS_CONFIG_MASK;
    }

    /* [mandatory-stmt] */
    if (d->flags & LYS_MAND_MASK) {
        switch (target->nodetype) {
        case LYS_LEAF:
        case LYS_CHOICE:
        case LYS_ANYDATA:
        case LYS_ANYXML:
            break;
        default:
            AMEND_WRONG_NODETYPE("deviation", "add", "mandatory");
        }

        if (target->flags & LYS_MAND_MASK) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE,
                    "Invalid deviation adding \"mandatory\" property which already exists (with value \"mandatory %s\").",
                    target->flags & LYS_MAND_TRUE ? "true" : "false");
            ret = LY_EVALID;
            goto cleanup;
        }

        target->flags |= d->flags & LYS_MAND_MASK;
    }

    /* [min-elements-stmt] */
    if (d->flags & LYS_SET_MIN) {
        switch (target->nodetype) {
        case LYS_LEAFLIST:
            num = &((struct lysp_node_leaflist *)target)->min;
            break;
        case LYS_LIST:
            num = &((struct lysp_node_list *)target)->min;
            break;
        default:
            AMEND_WRONG_NODETYPE("deviation", "add", "min-elements");
        }

        if (target->flags & LYS_SET_MIN) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE,
                    "Invalid deviation adding \"min-elements\" property which already exists (with value \"%u\").", *num);
            ret = LY_EVALID;
            goto cleanup;
        }

        *num = d->min;
    }

    /* [max-elements-stmt] */
    if (d->flags & LYS_SET_MAX) {
        switch (target->nodetype) {
        case LYS_LEAFLIST:
            num = &((struct lysp_node_leaflist *)target)->max;
            break;
        case LYS_LIST:
            num = &((struct lysp_node_list *)target)->max;
            break;
        default:
            AMEND_WRONG_NODETYPE("deviation", "add", "max-elements");
        }

        if (target->flags & LYS_SET_MAX) {
            if (*num) {
                LOGVAL(ctx->ctx, LYVE_REFERENCE,
                        "Invalid deviation adding \"max-elements\" property which already exists (with value \"%u\").",
                        *num);
            } else {
                LOGVAL(ctx->ctx, LYVE_REFERENCE,
                        "Invalid deviation adding \"max-elements\" property which already exists (with value \"unbounded\").");
            }
            ret = LY_EVALID;
            goto cleanup;
        }

        *num = d->max;
    }

cleanup:
    return ret;
}

/**
 * @brief Apply deviate delete.
 *
 * @param[in] ctx Compile context.
 * @param[in] d Deviate delete to apply.
 * @param[in,out] target Deviation target.
 * @return LY_ERR value.
 */
static LY_ERR
lys_apply_deviate_delete(struct lysc_ctx *ctx, struct lysp_deviate_del *d, struct lysp_node *target)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysp_restr **musts;
    LY_ARRAY_COUNT_TYPE u, v;
    struct lysp_qname **uniques, **dflts;

#define DEV_DEL_ARRAY(DEV_ARRAY, ORIG_ARRAY, DEV_MEMBER, ORIG_MEMBER, FREE_FUNC, FREE_CTX, PROPERTY) \
    LY_ARRAY_FOR(d->DEV_ARRAY, u) { \
        int found = 0; \
        LY_ARRAY_FOR(ORIG_ARRAY, v) { \
            if (!strcmp(d->DEV_ARRAY[u]DEV_MEMBER, (ORIG_ARRAY)[v]ORIG_MEMBER)) { \
                found = 1; \
                break; \
            } \
        } \
        if (!found) { \
            LOGVAL(ctx->ctx, LYVE_REFERENCE, \
                    "Invalid deviation deleting \"%s\" property \"%s\" which does not match any of the target's property values.", \
                    PROPERTY, d->DEV_ARRAY[u]DEV_MEMBER); \
            ret = LY_EVALID; \
            goto cleanup; \
        } \
        LY_ARRAY_DECREMENT(ORIG_ARRAY); \
        FREE_FUNC(FREE_CTX, &(ORIG_ARRAY)[v]); \
        if (v < LY_ARRAY_COUNT(ORIG_ARRAY)) { \
            memmove(&(ORIG_ARRAY)[v], &(ORIG_ARRAY)[v + 1], (LY_ARRAY_COUNT(ORIG_ARRAY) - v) * sizeof *(ORIG_ARRAY)); \
        } \
    } \
    if (!LY_ARRAY_COUNT(ORIG_ARRAY)) { \
        LY_ARRAY_FREE(ORIG_ARRAY); \
        ORIG_ARRAY = NULL; \
    }

#define DEV_CHECK_PRESENCE_VALUE(TYPE, MEMBER, DEVTYPE, PROPERTY, VALUE) \
    if (!((TYPE)target)->MEMBER) { \
        LOGVAL(ctx->ctx, LY_VCODE_DEV_NOT_PRESENT, DEVTYPE, PROPERTY, VALUE); \
        ret = LY_EVALID; \
        goto cleanup; \
    } else if (strcmp(((TYPE)target)->MEMBER, VALUE)) { \
        LOGVAL(ctx->ctx, LYVE_REFERENCE, \
                "Invalid deviation deleting \"%s\" property \"%s\" which does not match the target's property value \"%s\".", \
                PROPERTY, VALUE, ((TYPE)target)->MEMBER); \
        ret = LY_EVALID; \
        goto cleanup; \
    }

    /* [units-stmt] */
    if (d->units) {
        switch (target->nodetype) {
        case LYS_LEAF:
        case LYS_LEAFLIST:
            break;
        default:
            AMEND_WRONG_NODETYPE("deviation", "delete", "units");
        }

        DEV_CHECK_PRESENCE_VALUE(struct lysp_node_leaf *, units, "deleting", "units", d->units);
        lydict_remove(ctx->ctx, ((struct lysp_node_leaf *)target)->units);
        ((struct lysp_node_leaf *)target)->units = NULL;
    }

    /* *must-stmt */
    if (d->musts) {
        musts = lysp_node_musts_p(target);
        if (!musts) {
            AMEND_WRONG_NODETYPE("deviation", "delete", "must");
        }

        DEV_DEL_ARRAY(musts, *musts, .arg.str, .arg.str, lysp_restr_free, &ctx->free_ctx, "must");
    }

    /* *unique-stmt */
    if (d->uniques) {
        if (target->nodetype != LYS_LIST) {
            AMEND_WRONG_NODETYPE("deviation", "delete", "unique");
        }

        uniques = &((struct lysp_node_list *)target)->uniques;
        DEV_DEL_ARRAY(uniques, *uniques, .str, .str, lysp_qname_free, ctx->ctx, "unique");
    }

    /* *default-stmt */
    if (d->dflts) {
        switch (target->nodetype) {
        case LYS_LEAF:
            AMEND_CHECK_CARDINALITY(d->dflts, 1, "deviation", "default");
            DEV_CHECK_PRESENCE_VALUE(struct lysp_node_leaf *, dflt.str, "deleting", "default", d->dflts[0].str);

            lydict_remove(ctx->ctx, ((struct lysp_node_leaf *)target)->dflt.str);
            ((struct lysp_node_leaf *)target)->dflt.str = NULL;
            break;
        case LYS_LEAFLIST:
            dflts = &((struct lysp_node_leaflist *)target)->dflts;
            DEV_DEL_ARRAY(dflts, *dflts, .str, .str, lysp_qname_free, ctx->ctx, "default");
            break;
        case LYS_CHOICE:
            AMEND_CHECK_CARDINALITY(d->dflts, 1, "deviation", "default");
            DEV_CHECK_PRESENCE_VALUE(struct lysp_node_choice *, dflt.str, "deleting", "default", d->dflts[0].str);

            lydict_remove(ctx->ctx, ((struct lysp_node_choice *)target)->dflt.str);
            ((struct lysp_node_choice *)target)->dflt.str = NULL;
            break;
        default:
            AMEND_WRONG_NODETYPE("deviation", "delete", "default");
        }
    }

cleanup:
    return ret;
}

/**
 * @brief Apply deviate replace.
 *
 * @param[in] ctx Compile context.
 * @param[in] d Deviate replace to apply.
 * @param[in,out] target Deviation target.
 * @return LY_ERR value.
 */
static LY_ERR
lys_apply_deviate_replace(struct lysc_ctx *ctx, struct lysp_deviate_rpl *d, struct lysp_node *target)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t *num;

#define DEV_CHECK_PRESENCE(TYPE, MEMBER, DEVTYPE, PROPERTY, VALUE) \
    if (!((TYPE)target)->MEMBER) { \
        LOGVAL(ctx->ctx, LY_VCODE_DEV_NOT_PRESENT, DEVTYPE, PROPERTY, VALUE); \
        ret = LY_EVALID; \
        goto cleanup; \
    }

    /* [type-stmt] */
    if (d->type) {
        switch (target->nodetype) {
        case LYS_LEAF:
        case LYS_LEAFLIST:
            break;
        default:
            AMEND_WRONG_NODETYPE("deviation", "replace", "type");
        }

        lysp_type_free(&ctx->free_ctx, &((struct lysp_node_leaf *)target)->type);
        lysp_type_dup(ctx->ctx, ctx->pmod, d->type, &((struct lysp_node_leaf *)target)->type);
    }

    /* [units-stmt] */
    if (d->units) {
        switch (target->nodetype) {
        case LYS_LEAF:
        case LYS_LEAFLIST:
            break;
        default:
            AMEND_WRONG_NODETYPE("deviation", "replace", "units");
        }

        DEV_CHECK_PRESENCE(struct lysp_node_leaf *, units, "replacing", "units", d->units);
        lydict_remove(ctx->ctx, ((struct lysp_node_leaf *)target)->units);
        DUP_STRING_GOTO(ctx->ctx, d->units, ((struct lysp_node_leaf *)target)->units, ret, cleanup);
    }

    /* [default-stmt] */
    if (d->dflt.str) {
        switch (target->nodetype) {
        case LYS_LEAF:
            DEV_CHECK_PRESENCE(struct lysp_node_leaf *, dflt.str, "replacing", "default", d->dflt.str);

            lydict_remove(ctx->ctx, ((struct lysp_node_leaf *)target)->dflt.str);
            LY_CHECK_GOTO(ret = lysp_qname_dup(ctx->ctx, &d->dflt, &((struct lysp_node_leaf *)target)->dflt), cleanup);
            break;
        case LYS_CHOICE:
            DEV_CHECK_PRESENCE(struct lysp_node_choice *, dflt.str, "replacing", "default", d->dflt.str);

            lydict_remove(ctx->ctx, ((struct lysp_node_choice *)target)->dflt.str);
            LY_CHECK_GOTO(ret = lysp_qname_dup(ctx->ctx, &d->dflt, &((struct lysp_node_choice *)target)->dflt), cleanup);
            break;
        default:
            AMEND_WRONG_NODETYPE("deviation", "replace", "default");
        }
    }

    /* [config-stmt] */
    if (d->flags & LYS_CONFIG_MASK) {
        switch (target->nodetype) {
        case LYS_CONTAINER:
        case LYS_LEAF:
        case LYS_LEAFLIST:
        case LYS_LIST:
        case LYS_CHOICE:
        case LYS_ANYDATA:
        case LYS_ANYXML:
            break;
        default:
            AMEND_WRONG_NODETYPE("deviation", "replace", "config");
        }

        target->flags &= ~LYS_CONFIG_MASK;
        target->flags |= d->flags & LYS_CONFIG_MASK;
    }

    /* [mandatory-stmt] */
    if (d->flags & LYS_MAND_MASK) {
        switch (target->nodetype) {
        case LYS_LEAF:
        case LYS_CHOICE:
        case LYS_ANYDATA:
        case LYS_ANYXML:
            break;
        default:
            AMEND_WRONG_NODETYPE("deviation", "replace", "mandatory");
        }

        if (!(target->flags & LYS_MAND_MASK)) {
            LOGVAL(ctx->ctx, LY_VCODE_DEV_NOT_PRESENT, "replacing", "mandatory",
                    d->flags & LYS_MAND_TRUE ? "mandatory true" : "mandatory false");
            ret = LY_EVALID;
            goto cleanup;
        }

        target->flags &= ~LYS_MAND_MASK;
        target->flags |= d->flags & LYS_MAND_MASK;
    }

    /* [min-elements-stmt] */
    if (d->flags & LYS_SET_MIN) {
        switch (target->nodetype) {
        case LYS_LEAFLIST:
            num = &((struct lysp_node_leaflist *)target)->min;
            break;
        case LYS_LIST:
            num = &((struct lysp_node_list *)target)->min;
            break;
        default:
            AMEND_WRONG_NODETYPE("deviation", "replace", "min-elements");
        }

        if (!(target->flags & LYS_SET_MIN)) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid deviation replacing \"min-elements\" property which is not present.");
            ret = LY_EVALID;
            goto cleanup;
        }

        *num = d->min;
    }

    /* [max-elements-stmt] */
    if (d->flags & LYS_SET_MAX) {
        switch (target->nodetype) {
        case LYS_LEAFLIST:
            num = &((struct lysp_node_leaflist *)target)->max;
            break;
        case LYS_LIST:
            num = &((struct lysp_node_list *)target)->max;
            break;
        default:
            AMEND_WRONG_NODETYPE("deviation", "replace", "max-elements");
        }

        if (!(target->flags & LYS_SET_MAX)) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid deviation replacing \"max-elements\" property which is not present.");
            ret = LY_EVALID;
            goto cleanup;
        }

        *num = d->max;
    }

cleanup:
    return ret;
}

/**
 * @brief Apply deviation with all its deviates.
 *
 * @param[in] ctx Compile context.
 * @param[in] dev Deviation to apply.
 * @param[in] dev_pmod Local module of the deviation.
 * @param[in,out] target Deviation target.
 * @return LY_ERR value.
 */
static LY_ERR
lys_apply_deviation(struct lysc_ctx *ctx, struct lysp_deviation *dev, const struct lysp_module *dev_pmod,
        struct lysp_node *target)
{
    LY_ERR ret = LY_SUCCESS;
    struct lys_module *orig_mod = ctx->cur_mod;
    struct lysp_module *orig_pmod = ctx->pmod;
    char orig_path[LYSC_CTX_BUFSIZE];
    struct lysp_deviate *d;

    /* clear path and set modules */
    strcpy(orig_path, ctx->path);
    ctx->path_len = 1;
    ctx->cur_mod = dev_pmod->mod;
    ctx->pmod = (struct lysp_module *)dev_pmod;

    /* generate correct path */
    lysc_update_path(ctx, NULL, "{deviation}");
    lysc_update_path(ctx, NULL, dev->nodeid);

    LY_LIST_FOR(dev->deviates, d) {
        switch (d->mod) {
        case LYS_DEV_ADD:
            ret = lys_apply_deviate_add(ctx, (struct lysp_deviate_add *)d, target);
            break;
        case LYS_DEV_DELETE:
            ret = lys_apply_deviate_delete(ctx, (struct lysp_deviate_del *)d, target);
            break;
        case LYS_DEV_REPLACE:
            ret = lys_apply_deviate_replace(ctx, (struct lysp_deviate_rpl *)d, target);
            break;
        default:
            LOGINT(ctx->ctx);
            ret = LY_EINT;
        }
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* deviation extension instances */
    DUP_EXTS(ctx->ctx, dev_pmod, target, lyplg_ext_nodetype2stmt(target->nodetype), dev->exts, target->exts, lysp_ext_dup);

cleanup:
    ctx->cur_mod = orig_mod;
    ctx->pmod = orig_pmod;

    strcpy(ctx->path, orig_path);
    ctx->path_len = strlen(ctx->path);
    return ret;
}

/**
 * @brief Check whether a compiled node matches a single schema nodeid name test.
 *
 * @param[in,out] node Compiled node to consider. On a match it is moved to its parent.
 * @param[in] mod Expected module.
 * @param[in] name Expected name.
 * @param[in] name_len Length of @p name.
 * @return Whether it is a match or not.
 */
static ly_bool
lysp_schema_nodeid_match_node(const struct lysc_node **node, const struct lys_module *mod, const char *name,
        size_t name_len)
{
    /* compare with the module of the node */
    if ((*node)->module != mod) {
        return 0;
    }

    /* compare names */
    if (ly_strncmp((*node)->name, name, name_len)) {
        return 0;
    }

    /* move to next parent */
    *node = (*node)->parent;

    return 1;
}

/**
 * @brief Check whether a compiled ext instance matches a single schema nodeid name test.
 *
 * @param[in,out] ext Compiled ext instance to consider. On a match it is zeroed to not match again.
 * @param[in] mod Expected module.
 * @param[in] name Expected name.
 * @param[in] name_len Length of @p name.
 * @return Whether it is a match or not.
 */
static ly_bool
lysp_schema_nodeid_match_ext(const struct lysc_ext_instance **ext, const struct lys_module *mod, const char *name,
        size_t name_len)
{
    /* compare with the module */
    if ((*ext)->module != mod) {
        return 0;
    }

    /* compare names (argument) */
    if (ly_strncmp((*ext)->argument, name, name_len)) {
        return 0;
    }

    /* zero */
    *ext = NULL;

    return 1;
}

/**
 * @brief Check whether a node matches specific schema nodeid.
 *
 * @param[in] exp Parsed nodeid to match.
 * @param[in] exp_pmod Module to use for nodes in @p exp without a prefix.
 * @param[in] exp_ext Extension instance in which @p exp is defined, it means it targets an extension instance.
 * @param[in] ctx_node Initial context node that should match, only for descendant paths.
 * @param[in] parent First compiled parent to consider. If @p pnode is NULL, it is condered the node to be matched.
 * @param[in] pnode Parsed node to be matched. May be NULL if the target node was already compiled.
 * @param[in] pnode_mod Compiled @p pnode to-be module.
 * @param[in] pnode_ext Extension instance in which @p pnode is defined.
 * @return Whether it is a match or not.
 */
static ly_bool
lysp_schema_nodeid_match(const struct lyxp_expr *exp, const struct lysp_module *exp_pmod,
        const struct lysp_ext_instance *exp_ext, const struct lysc_node *ctx_node, const struct lysc_node *parent,
        const struct lysp_node *pnode, const struct lys_module *pnode_mod, const struct lysc_ext_instance *pnode_ext)
{
    uint32_t i;
    const struct lys_module *mod;
    const char *name = NULL;
    size_t name_len = 0;

    if (exp_ext && !pnode_ext) {
        /* extension instance augment and standard node, will never match */
        return 0;
    } else if (!exp_ext && pnode_ext) {
        /* standard augment and extension instance node, will never match */
        return 0;
    }

    /* compare last node in the node ID */
    i = exp->used - 1;

    /* get exp node ID module */
    mod = lys_schema_node_get_module(exp_pmod->mod->ctx, exp->expr + exp->tok_pos[i], exp->tok_len[i], exp_pmod, &name, &name_len);
    assert(mod);

    if (pnode) {
        /* compare on the last parsed-only node */
        if ((pnode_mod != mod) || ly_strncmp(pnode->name, name, name_len)) {
            return 0;
        }
    } else {
        /* using parent directly */
        if (!lysp_schema_nodeid_match_node(&parent, mod, name, name_len)) {
            return 0;
        }
    }

    /* now compare all the compiled parents */
    while (i > 1) {
        i -= 2;
        assert(exp->tokens[i] == LYXP_TOKEN_NAMETEST);

        if (!parent && !pnode_ext) {
            /* no more parents but path continues */
            return 0;
        }

        /* get exp node ID module */
        mod = lys_schema_node_get_module(exp_pmod->mod->ctx, exp->expr + exp->tok_pos[i], exp->tok_len[i], exp_pmod, &name,
                &name_len);
        assert(mod);

        if (parent) {
            /* compare with the parent */
            if (!lysp_schema_nodeid_match_node(&parent, mod, name, name_len)) {
                return 0;
            }
        } else {
            /* compare with the ext instance */
            if (!lysp_schema_nodeid_match_ext(&pnode_ext, mod, name, name_len)) {
                return 0;
            }
        }
    }

    if (ctx_node && (ctx_node != parent)) {
        /* descendant path has not finished in the context node */
        return 0;
    } else if (!ctx_node && (parent || pnode_ext)) {
        /* some parent/extension was not matched */
        return 0;
    }

    return 1;
}

void
lysc_augment_free(const struct ly_ctx *ctx, struct lysc_augment *aug)
{
    if (aug) {
        lyxp_expr_free(ctx, aug->nodeid);

        free(aug);
    }
}

void
lysc_deviation_free(const struct ly_ctx *ctx, struct lysc_deviation *dev)
{
    if (dev) {
        lyxp_expr_free(ctx, dev->nodeid);
        LY_ARRAY_FREE(dev->devs);
        LY_ARRAY_FREE(dev->dev_pmods);

        free(dev);
    }
}

void
lysc_refine_free(const struct ly_ctx *ctx, struct lysc_refine *rfn)
{
    if (rfn) {
        lyxp_expr_free(ctx, rfn->nodeid);
        LY_ARRAY_FREE(rfn->rfns);

        free(rfn);
    }
}

void
lysp_dev_node_free(struct lysc_ctx *cctx, struct lysp_node *dev_pnode)
{
    if (!dev_pnode) {
        return;
    }

    switch (dev_pnode->nodetype) {
    case LYS_CONTAINER:
        ((struct lysp_node_container *)dev_pnode)->child = NULL;
        ((struct lysp_node_container *)dev_pnode)->actions = NULL;
        ((struct lysp_node_container *)dev_pnode)->notifs = NULL;
        break;
    case LYS_LIST:
        ((struct lysp_node_list *)dev_pnode)->child = NULL;
        ((struct lysp_node_list *)dev_pnode)->actions = NULL;
        ((struct lysp_node_list *)dev_pnode)->notifs = NULL;
        break;
    case LYS_CHOICE:
        ((struct lysp_node_choice *)dev_pnode)->child = NULL;
        break;
    case LYS_CASE:
        ((struct lysp_node_case *)dev_pnode)->child = NULL;
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
    case LYS_ANYXML:
    case LYS_ANYDATA:
        /* no children */
        break;
    case LYS_NOTIF:
        ((struct lysp_node_notif *)dev_pnode)->child = NULL;
        break;
    case LYS_RPC:
    case LYS_ACTION:
        ((struct lysp_node_action *)dev_pnode)->input.child = NULL;
        ((struct lysp_node_action *)dev_pnode)->output.child = NULL;
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        ((struct lysp_node_action_inout *)dev_pnode)->child = NULL;
        lysp_node_free(&cctx->free_ctx, dev_pnode);
        free(dev_pnode);
        return;
    default:
        LOGINT(cctx->ctx);
        return;
    }

    lysp_node_free(&cctx->free_ctx, dev_pnode);
}

LY_ERR
lys_compile_node_deviations_refines(struct lysc_ctx *ctx, const struct lysp_node *pnode, const struct lysc_node *parent,
        struct lysp_node **dev_pnode, ly_bool *not_supported)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t i;
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_refine *rfn;
    struct lysc_deviation *dev;

    *dev_pnode = NULL;
    *not_supported = 0;

    for (i = 0; i < ctx->uses_rfns.count; ) {
        rfn = ctx->uses_rfns.objs[i];

        if (!lysp_schema_nodeid_match(rfn->nodeid, rfn->nodeid_pmod, NULL, rfn->nodeid_ctx_node, parent, pnode,
                ctx->cur_mod, ctx->ext)) {
            /* not our target node */
            ++i;
            continue;
        }

        if (!*dev_pnode) {
            /* first refine on this node, create a copy first */
            LY_CHECK_GOTO(ret = lysp_dup_single(ctx, pnode, 1, dev_pnode), cleanup);
        }

        /* apply all the refines by changing (the copy of) the parsed node */
        LY_ARRAY_FOR(rfn->rfns, u) {
            LY_CHECK_GOTO(ret = lys_apply_refine(ctx, rfn->rfns[u], rfn->nodeid_pmod, *dev_pnode), cleanup);
        }

        /* refine was applied, remove it */
        lysc_refine_free(ctx->ctx, rfn);
        ly_set_rm_index(&ctx->uses_rfns, i, NULL);

        /* refines use relative paths so more may apply to a single node */
    }

    for (i = 0; i < ctx->devs.count; ++i) {
        dev = ctx->devs.objs[i];

        if (!lysp_schema_nodeid_match(dev->nodeid, dev->dev_pmods[0], NULL, NULL, parent, pnode, ctx->cur_mod, ctx->ext)) {
            /* not our target node */
            continue;
        }

        if (dev->not_supported) {
            /* it is not supported, no more deviations */
            *not_supported = 1;
            goto dev_applied;
        }

        if (!*dev_pnode) {
            /* first deviation on this node, create a copy first */
            LY_CHECK_GOTO(ret = lysp_dup_single(ctx, pnode, 1, dev_pnode), cleanup);
        }

        /* apply all the deviates by changing (the copy of) the parsed node */
        LY_ARRAY_FOR(dev->devs, u) {
            LY_CHECK_GOTO(ret = lys_apply_deviation(ctx, dev->devs[u], dev->dev_pmods[u], *dev_pnode), cleanup);
        }

dev_applied:
        /* deviation was applied, remove it */
        lysc_deviation_free(ctx->ctx, dev);
        ly_set_rm_index(&ctx->devs, i, NULL);

        /* all the deviations for one target node are in one structure, we are done */
        break;
    }

cleanup:
    if (ret) {
        lysp_dev_node_free(ctx, *dev_pnode);
        *dev_pnode = NULL;
        *not_supported = 0;
    }
    return ret;
}

/**
 * @brief Compile augment children.
 *
 * @param[in] ctx Compile context.
 * @param[in] aug_when Parsed augment when to inherit.
 * @param[in] aug_flags Parsed augment flags.
 * @param[in] child First augment child to compile.
 * @param[in] target Target node of the augment.
 * @param[in] child_unres_disabled Whether the children are to be put into unres disabled set or not.
 * @return LY_SUCCESS on success.
 * @return LY_EVALID on failure.
 */
static LY_ERR
lys_compile_augment_children(struct lysc_ctx *ctx, struct lysp_when *aug_when, uint16_t aug_flags, struct lysp_node *child,
        struct lysc_node *target, ly_bool child_unres_disabled)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysp_node *pnode;
    struct lysc_node *node;
    struct lysc_when *when_shared = NULL;
    ly_bool enabled, allow_mand = 0;
    struct ly_set child_set = {0};
    uint32_t i, opt_prev = ctx->compile_opts;

    /* check for mandatory nodes
     * - new cases augmenting some choice can have mandatory nodes
     * - mandatory nodes are allowed only in case the augmentation is made conditional with a when statement
     */
    if (aug_when || (target->nodetype == LYS_CHOICE) || (ctx->cur_mod == target->module)) {
        allow_mand = 1;
    }

    LY_LIST_FOR(child, pnode) {
        /* check if the subnode can be connected to the found target (e.g. case cannot be inserted into container) */
        if (((pnode->nodetype == LYS_CASE) && (target->nodetype != LYS_CHOICE)) ||
                ((pnode->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) && !(target->nodetype & (LYS_CONTAINER | LYS_LIST))) ||
                ((pnode->nodetype == LYS_USES) && (target->nodetype == LYS_CHOICE))) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE,
                    "Invalid augment of %s node which is not allowed to contain %s node \"%s\".",
                    lys_nodetype2str(target->nodetype), lys_nodetype2str(pnode->nodetype), pnode->name);
            rc = LY_EVALID;
            goto cleanup;
        }

        /* compile the children */
        if (target->nodetype == LYS_CHOICE) {
            LY_CHECK_GOTO(rc = lys_compile_node_choice_child(ctx, pnode, target, &child_set), cleanup);
        } else if (target->nodetype & (LYS_INPUT | LYS_OUTPUT)) {
            if (target->nodetype == LYS_INPUT) {
                ctx->compile_opts |= LYS_COMPILE_RPC_INPUT;
            } else {
                ctx->compile_opts |= LYS_COMPILE_RPC_OUTPUT;
            }
            LY_CHECK_GOTO(rc = lys_compile_node(ctx, pnode, target, aug_flags, &child_set), cleanup);
        } else {
            LY_CHECK_GOTO(rc = lys_compile_node(ctx, pnode, target, aug_flags, &child_set), cleanup);
        }

        /* eval if-features again for the rest of this node processing */
        LY_CHECK_GOTO(rc = lys_eval_iffeatures(ctx->ctx, pnode->iffeatures, &enabled), cleanup);
        if (!enabled && !(ctx->compile_opts & (LYS_COMPILE_NO_DISABLED | LYS_COMPILE_DISABLED | LYS_COMPILE_GROUPING))) {
            ctx->compile_opts |= LYS_COMPILE_DISABLED;
        }

        /* since the augment node is not present in the compiled tree, we need to pass some of its
         * statements to all its children */
        for (i = 0; i < child_set.count; ++i) {
            node = child_set.snodes[i];
            if (!allow_mand && (node->flags & LYS_CONFIG_W) && (node->flags & LYS_MAND_TRUE)) {
                node->flags &= ~LYS_MAND_TRUE;
                lys_compile_mandatory_parents(target, 0);
                LOGVAL(ctx->ctx, LYVE_SEMANTICS,
                        "Invalid augment adding mandatory node \"%s\" without making it conditional via when statement.",
                        node->name);
                rc = LY_EVALID;
                goto cleanup;
            }

            if (aug_when) {
                /* pass augment's when to all the children */
                rc = lys_compile_when(ctx, aug_when, aug_flags, target, lysc_data_node(target), node, &when_shared);
                LY_CHECK_GOTO(rc, cleanup);
            }

            if (child_unres_disabled) {
                /* child is disabled by the augment if-features */
                ly_set_add(&ctx->unres->disabled, node, 1, NULL);
            }
        }

        /* next iter */
        ly_set_erase(&child_set, NULL);
        ctx->compile_opts = opt_prev;
    }

cleanup:
    ly_set_erase(&child_set, NULL);
    ctx->compile_opts = opt_prev;
    return rc;
}

/**
 * @brief Compile the parsed augment connecting it into its target.
 *
 * It is expected that all the data referenced in path are present - augments are ordered so that augment B
 * targeting data from augment A is being compiled after augment A. Also the modules referenced in the path
 * are already implemented and compiled.
 *
 * @param[in] ctx Compile context.
 * @param[in] aug_p Parsed augment to compile.
 * @param[in] target Target node of the augment.
 * @return LY_SUCCESS on success.
 * @return LY_EVALID on failure.
 */
static LY_ERR
lys_compile_augment(struct lysc_ctx *ctx, struct lysp_node_augment *aug_p, struct lysc_node *target)
{
    LY_ERR rc = LY_SUCCESS;
    ly_bool enabled, child_unres_disabled = 0;
    uint32_t opt_prev = ctx->compile_opts;

    assert(target->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_CHOICE | LYS_CASE | LYS_INPUT | LYS_OUTPUT | LYS_NOTIF));

    /* nodetype checks */
    if (aug_p->actions && !lysc_node_actions_p(target)) {
        LOGVAL(ctx->ctx, LYVE_REFERENCE,
                "Invalid augment of %s node which is not allowed to contain RPC/action node \"%s\".",
                lys_nodetype2str(target->nodetype), aug_p->actions->name);
        rc = LY_EVALID;
        goto cleanup;
    }
    if (aug_p->notifs && !lysc_node_notifs_p(target)) {
        LOGVAL(ctx->ctx, LYVE_REFERENCE,
                "Invalid augment of %s node which is not allowed to contain notification node \"%s\".",
                lys_nodetype2str(target->nodetype), aug_p->notifs->name);
        rc = LY_EVALID;
        goto cleanup;
    }

    /* augment if-features */
    LY_CHECK_GOTO(rc = lys_eval_iffeatures(ctx->ctx, aug_p->iffeatures, &enabled), cleanup);
    if (!enabled && !(ctx->compile_opts & (LYS_COMPILE_NO_DISABLED | LYS_COMPILE_DISABLED | LYS_COMPILE_GROUPING))) {
        ctx->compile_opts |= LYS_COMPILE_DISABLED;
        child_unres_disabled = 1;
    }

    /* augment children */
    rc = lys_compile_augment_children(ctx, aug_p->when, aug_p->flags, aug_p->child, target, child_unres_disabled);
    LY_CHECK_GOTO(rc, cleanup);

    /* augment actions */
    rc = lys_compile_augment_children(ctx, aug_p->when, aug_p->flags, (struct lysp_node *)aug_p->actions, target,
            child_unres_disabled);
    LY_CHECK_GOTO(rc, cleanup);

    /* augment notifications */
    rc = lys_compile_augment_children(ctx, aug_p->when, aug_p->flags, (struct lysp_node *)aug_p->notifs, target,
            child_unres_disabled);
    LY_CHECK_GOTO(rc, cleanup);

    /* compile extensions into the target */
    COMPILE_EXTS_GOTO(ctx, aug_p->exts, target->exts, target, rc, cleanup);

cleanup:
    ctx->compile_opts = opt_prev;
    return rc;
}

LY_ERR
lys_compile_node_augments(struct lysc_ctx *ctx, struct lysc_node *node)
{
    LY_ERR ret = LY_SUCCESS;
    struct lys_module *orig_mod = ctx->cur_mod;
    struct lysp_module *orig_pmod = ctx->pmod;
    uint32_t i;
    char orig_path[LYSC_CTX_BUFSIZE];
    struct lysc_augment *aug;

    /* uses augments */
    for (i = 0; i < ctx->uses_augs.count; ) {
        aug = ctx->uses_augs.objs[i];

        if (!lysp_schema_nodeid_match(aug->nodeid, orig_mod->parsed, aug->ext, aug->nodeid_ctx_node, node, NULL, NULL,
                ctx->ext)) {
            /* not our target node */
            ++i;
            continue;
        }

        /* use the path and modules from the augment */
        lysc_update_path(ctx, NULL, "{augment}");
        lysc_update_path(ctx, NULL, aug->aug_p->nodeid);
        ctx->pmod = (struct lysp_module *)aug->aug_pmod;

        /* apply augment, restore the path */
        ret = lys_compile_augment(ctx, aug->aug_p, node);
        lysc_update_path(ctx, NULL, NULL);
        lysc_update_path(ctx, NULL, NULL);
        LY_CHECK_GOTO(ret, cleanup);

        /* augment was applied, remove it (index and the whole set may have changed because other augments
         * could have been applied) */
        ly_set_rm(&ctx->uses_augs, aug, NULL);
        lysc_augment_free(ctx->ctx, aug);
        i = 0;
    }

    /* top-level augments */
    for (i = 0; i < ctx->augs.count; ) {
        aug = ctx->augs.objs[i];

        if (!lysp_schema_nodeid_match(aug->nodeid, aug->aug_pmod, aug->ext, NULL, node, NULL, NULL, ctx->ext)) {
            /* not our target node */
            ++i;
            continue;
        }

        /* use the path and modules from the augment */
        strcpy(orig_path, ctx->path);
        ctx->path_len = 1;
        ctx->cur_mod = aug->aug_pmod->mod;
        ctx->pmod = (struct lysp_module *)aug->aug_pmod;
        lysc_update_path(ctx, NULL, "{augment}");
        lysc_update_path(ctx, NULL, aug->aug_p->nodeid);

        /* apply augment, restore the path */
        ret = lys_compile_augment(ctx, aug->aug_p, node);
        strcpy(ctx->path, orig_path);
        ctx->path_len = strlen(ctx->path);
        LY_CHECK_GOTO(ret, cleanup);

        /* augment was applied, remove it */
        ly_set_rm(&ctx->augs, aug, NULL);
        lysc_augment_free(ctx->ctx, aug);
        i = 0;
    }

cleanup:
    ctx->cur_mod = orig_mod;
    ctx->pmod = orig_pmod;
    return ret;
}

/**
 * @brief Prepare an absolute-nodeid augment to be applied during data nodes compilation.
 *
 * @param[in] ctx Compile context.
 * @param[in] aug_p Parsed augment to be applied.
 * @param[in] pmod Both current and prefix module for @p aug_p.
 * @param[in] ext Extension instance in case @p aug_p is defined in one.
 * @return LY_ERR value.
 */
static LY_ERR
lys_precompile_own_augment(struct lysc_ctx *ctx, struct lysp_node_augment *aug_p, const struct lysp_module *pmod,
        const struct lysp_ext_instance *ext)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_expr *exp = NULL;
    struct lysc_augment *aug;
    const struct lys_module *mod;

    /* parse its target, it was already parsed and fully checked (except for the existence of the nodes) */
    ret = lyxp_expr_parse(ctx->ctx, aug_p->nodeid, strlen(aug_p->nodeid), 0, &exp);
    LY_CHECK_GOTO(ret, cleanup);

    mod = lys_schema_node_get_module(ctx->ctx, exp->expr + exp->tok_pos[1], exp->tok_len[1], pmod, NULL, NULL);
    LY_CHECK_ERR_GOTO(!mod, LOGINT(ctx->ctx); ret = LY_EINT, cleanup);
    if (mod != ctx->cur_mod) {
        /* augment for another module, ignore */
        goto cleanup;
    }

    /* allocate new compiled augment and store it in the set */
    aug = calloc(1, sizeof *aug);
    LY_CHECK_ERR_GOTO(!aug, LOGMEM(ctx->ctx); ret = LY_EMEM, cleanup);
    LY_CHECK_GOTO(ret = ly_set_add(&ctx->augs, aug, 1, NULL), cleanup);

    aug->nodeid = exp;
    exp = NULL;
    aug->aug_pmod = pmod;
    aug->ext = ext;
    aug->aug_p = aug_p;

cleanup:
    lyxp_expr_free(ctx->ctx, exp);
    return ret;
}

/**
 * @brief Prepare all top-level augments and extension instance augments to be applied during data nodes compilation.
 *
 * @param[in] ctx Compile context.
 * @param[in] pmod Parsed mod to use.
 * @return LY_ERR value.
 */
static LY_ERR
lys_precompile_own_augments_mod(struct lysc_ctx *ctx, const struct lysp_module *pmod)
{
    LY_ARRAY_COUNT_TYPE u, v;
    struct lysp_node_augment *aug_p;

    /* module */
    LY_LIST_FOR(pmod->augments, aug_p) {
        LY_CHECK_RET(lys_precompile_own_augment(ctx, aug_p, pmod, NULL));
    }

    /* parsed extension instances */
    LY_ARRAY_FOR(pmod->exts, u) {
        aug_p = NULL;
        LY_ARRAY_FOR(pmod->exts[u].substmts, v) {
            if (pmod->exts[u].substmts[v].stmt == LY_STMT_AUGMENT) {
                aug_p = *(struct lysp_node_augment **)pmod->exts[u].substmts[v].storage;
                break;
            }
        }
        if (!aug_p) {
            continue;
        }

        LY_CHECK_RET(lys_precompile_own_augment(ctx, aug_p, pmod, &pmod->exts[u]));
    }

    return LY_SUCCESS;
}

LY_ERR
lys_precompile_own_augments(struct lysc_ctx *ctx)
{
    LY_ARRAY_COUNT_TYPE u, v;
    const struct lys_module *aug_mod;
    const struct lysp_module *submod;

    LY_ARRAY_FOR(ctx->cur_mod->augmented_by, u) {
        aug_mod = ctx->cur_mod->augmented_by[u];

        /* collect all module augments */
        LY_CHECK_RET(lys_precompile_own_augments_mod(ctx, aug_mod->parsed));

        /* collect all submodules augments */
        LY_ARRAY_FOR(aug_mod->parsed->includes, v) {
            submod = (struct lysp_module *)aug_mod->parsed->includes[v].submodule;

            LY_CHECK_RET(lys_precompile_own_augments_mod(ctx, submod));
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Prepare a deviation to be applied during data nodes compilation.
 *
 * @param[in] ctx Compile context.
 * @param[in] dev_p Parsed deviation to be applied.
 * @param[in] pmod Both current and prefix module for @p dev_p.
 * @return LY_ERR value.
 */
static LY_ERR
lys_precompile_own_deviation(struct lysc_ctx *ctx, struct lysp_deviation *dev_p, const struct lysp_module *pmod)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_deviation *dev = NULL;
    struct lyxp_expr *exp = NULL;
    struct lysp_deviation **new_dev;
    const struct lys_module *mod;
    const struct lysp_module **new_dev_pmod;
    uint32_t i;

    /* parse its target, it was already parsed and fully checked (except for the existence of the nodes) */
    ret = lyxp_expr_parse(ctx->ctx, dev_p->nodeid, strlen(dev_p->nodeid), 0, &exp);
    LY_CHECK_GOTO(ret, cleanup);

    mod = lys_schema_node_get_module(ctx->ctx, exp->expr + exp->tok_pos[1], exp->tok_len[1], pmod, NULL, NULL);
    LY_CHECK_ERR_GOTO(!mod, LOGINT(ctx->ctx); ret = LY_EINT, cleanup);
    if (mod != ctx->cur_mod) {
        /* deviation for another module, ignore */
        goto cleanup;
    }

    /* try to find the node in already compiled deviations */
    for (i = 0; i < ctx->devs.count; ++i) {
        if (lys_abs_schema_nodeid_match(ctx->ctx, exp, pmod, ((struct lysc_deviation *)ctx->devs.objs[i])->nodeid,
                ((struct lysc_deviation *)ctx->devs.objs[i])->dev_pmods[0])) {
            dev = ctx->devs.objs[i];
            break;
        }
    }

    if (!dev) {
        /* allocate new compiled deviation */
        dev = calloc(1, sizeof *dev);
        LY_CHECK_ERR_GOTO(!dev, LOGMEM(ctx->ctx); ret = LY_EMEM, cleanup);
        LY_CHECK_GOTO(ret = ly_set_add(&ctx->devs, dev, 1, NULL), cleanup);

        dev->nodeid = exp;
        exp = NULL;
    }

    /* add new parsed deviation structure */
    LY_ARRAY_NEW_GOTO(ctx->ctx, dev->devs, new_dev, ret, cleanup);
    *new_dev = dev_p;
    LY_ARRAY_NEW_GOTO(ctx->ctx, dev->dev_pmods, new_dev_pmod, ret, cleanup);
    *new_dev_pmod = pmod;

cleanup:
    lyxp_expr_free(ctx->ctx, exp);
    return ret;
}

LY_ERR
lys_precompile_own_deviations(struct lysc_ctx *ctx)
{
    LY_ARRAY_COUNT_TYPE u, v, w;
    struct lys_module *orig_cur_mod;
    const struct lys_module *dev_mod;
    struct lysc_deviation *dev;
    struct lysp_deviate *d;
    int not_supported;
    uint32_t i;

    LY_ARRAY_FOR(ctx->cur_mod->deviated_by, u) {
        dev_mod = ctx->cur_mod->deviated_by[u];

        /* compile all module deviations */
        LY_ARRAY_FOR(dev_mod->parsed->deviations, v) {
            LY_CHECK_RET(lys_precompile_own_deviation(ctx, &dev_mod->parsed->deviations[v], dev_mod->parsed));
        }

        /* compile all submodules deviations */
        LY_ARRAY_FOR(dev_mod->parsed->includes, v) {
            LY_ARRAY_FOR(dev_mod->parsed->includes[v].submodule->deviations, w) {
                LY_CHECK_RET(lys_precompile_own_deviation(ctx, &dev_mod->parsed->includes[v].submodule->deviations[w],
                        (struct lysp_module *)dev_mod->parsed->includes[v].submodule));
            }
        }
    }

    /* set not-supported flags for all the deviations */
    for (i = 0; i < ctx->devs.count; ++i) {
        dev = ctx->devs.objs[i];
        not_supported = 0;

        LY_ARRAY_FOR(dev->devs, u) {
            LY_LIST_FOR(dev->devs[u]->deviates, d) {
                if (d->mod == LYS_DEV_NOT_SUPPORTED) {
                    not_supported = 1;
                    break;
                }
            }
            if (not_supported) {
                break;
            }
        }
        if (not_supported && (LY_ARRAY_COUNT(dev->devs) > 1)) {
            orig_cur_mod = ctx->cur_mod;
            ctx->cur_mod = dev->dev_pmods[u]->mod;
            lysc_update_path(ctx, NULL, "{deviation}");
            lysc_update_path(ctx, NULL, dev->nodeid->expr);
            LOGVAL(ctx->ctx, LYVE_SEMANTICS,
                    "Multiple deviations of \"%s\" with one of them being \"not-supported\".", dev->nodeid->expr);
            lysc_update_path(ctx, NULL, NULL);
            lysc_update_path(ctx, NULL, NULL);
            ctx->cur_mod = orig_cur_mod;
            return LY_EVALID;
        }

        dev->not_supported = not_supported;
    }

    return LY_SUCCESS;
}

/**
 * @brief Add a module reference into an array, checks for duplicities.
 *
 * @param[in] ctx Compile context.
 * @param[in] mod Module reference to add.
 * @param[in,out] mod_array Module sized array to add to.
 * @return LY_ERR value.
 */
static LY_ERR
lys_array_add_mod_ref(struct lysc_ctx *ctx, struct lys_module *mod, struct lys_module ***mod_array)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lys_module **new_mod;

    LY_ARRAY_FOR(*mod_array, u) {
        if ((*mod_array)[u] == mod) {
            /* already there */
            return LY_EEXIST;
        }
    }

    /* add the new module ref */
    LY_ARRAY_NEW_RET(ctx->ctx, *mod_array, new_mod, LY_EMEM);
    *new_mod = mod;

    return LY_SUCCESS;
}

/**
 * @brief Check whether all modules in a set are implemented.
 *
 * @param[in] mod_set Module set to check.
 * @return Whether all modules are implemented or not.
 */
static ly_bool
lys_precompile_mod_set_is_all_implemented(const struct ly_set *mod_set)
{
    uint32_t i;
    const struct lys_module *mod;

    for (i = 0; i < mod_set->count; ++i) {
        mod = mod_set->objs[i];
        if (!mod->implemented) {
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Add references to target modules of top-level augments, deviations, and augments in extension instances
 * in a module and all its submodules.
 *
 * @param[in] pmod Module to process.
 * @param[in,out] mod_set Module set to add referenced modules into.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
static LY_ERR
lys_precompile_mod_augments_deviations(struct lysp_module *pmod, struct ly_set *mod_set)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    struct lysc_ctx ctx;
    struct lys_module *m;
    struct lysp_node_augment *aug;
    struct ly_set set = {0};

    LYSC_CTX_INIT_PMOD(ctx, pmod, NULL);

    LY_LIST_FOR(pmod->augments, aug) {
        /* get target module */
        lysc_update_path(&ctx, NULL, "{augment}");
        lysc_update_path(&ctx, NULL, aug->nodeid);
        ret = lys_nodeid_mod_check(&ctx, aug->nodeid, 1, &set, NULL, &m);
        lysc_update_path(&ctx, NULL, NULL);
        lysc_update_path(&ctx, NULL, NULL);
        LY_CHECK_GOTO(ret, cleanup);

        /* add this module into the target module augmented_by, if not there and implemented */
        if ((lys_array_add_mod_ref(&ctx, pmod->mod, &m->augmented_by) != LY_EEXIST) ||
                !lys_precompile_mod_set_is_all_implemented(&set)) {
            LY_CHECK_GOTO(ret = ly_set_merge(mod_set, &set, 0, NULL), cleanup);
        }
        ly_set_erase(&set, NULL);
    }

    LY_ARRAY_FOR(pmod->deviations, u) {
        /* get target module */
        lysc_update_path(&ctx, NULL, "{deviation}");
        lysc_update_path(&ctx, NULL, pmod->deviations[u].nodeid);
        ret = lys_nodeid_mod_check(&ctx, pmod->deviations[u].nodeid, 1, &set, NULL, &m);
        lysc_update_path(&ctx, NULL, NULL);
        lysc_update_path(&ctx, NULL, NULL);
        LY_CHECK_GOTO(ret, cleanup);

        /* add this module into the target module deviated_by, if not there and implemented */
        if ((lys_array_add_mod_ref(&ctx, pmod->mod, &m->deviated_by) != LY_EEXIST) ||
                !lys_precompile_mod_set_is_all_implemented(&set)) {
            LY_CHECK_GOTO(ret = ly_set_merge(mod_set, &set, 0, NULL), cleanup);
        }
        ly_set_erase(&set, NULL);
    }

    LY_ARRAY_FOR(pmod->exts, u) {
        aug = NULL;
        LY_ARRAY_FOR(pmod->exts[u].substmts, v) {
            if (pmod->exts[u].substmts[v].stmt == LY_STMT_AUGMENT) {
                aug = *(struct lysp_node_augment **)pmod->exts[u].substmts[v].storage;
                break;
            }
        }
        if (!aug) {
            continue;
        }

        /* get target module */
        lysc_update_path(&ctx, NULL, "{ext-augment}");
        lysc_update_path(&ctx, NULL, aug->nodeid);
        ret = lys_nodeid_mod_check(&ctx, aug->nodeid, 1, &set, NULL, &m);
        lysc_update_path(&ctx, NULL, NULL);
        lysc_update_path(&ctx, NULL, NULL);
        LY_CHECK_GOTO(ret, cleanup);

        /* add this module into the target module augmented_by, if not there and implemented */
        if ((lys_array_add_mod_ref(&ctx, pmod->mod, &m->augmented_by) != LY_EEXIST) ||
                !lys_precompile_mod_set_is_all_implemented(&set)) {
            LY_CHECK_GOTO(ret = ly_set_merge(mod_set, &set, 0, NULL), cleanup);
        }
        ly_set_erase(&set, NULL);
    }

cleanup:
    ly_set_erase(&set, NULL);
    return ret;
}

LY_ERR
lys_precompile_augments_deviations(struct lys_module *mod, struct lys_glob_unres *unres)
{
    LY_ERR ret = LY_SUCCESS, r;
    LY_ARRAY_COUNT_TYPE u;
    struct lys_module *m;
    struct lysp_module *submod;
    const char **imp_f, *all_f[] = {"*", NULL};
    uint32_t i;
    struct ly_set mod_set = {0};

    /* module */
    LY_CHECK_GOTO(ret = lys_precompile_mod_augments_deviations(mod->parsed, &mod_set), cleanup);

    /* submodules */
    LY_ARRAY_FOR(mod->parsed->includes, u) {
        submod = (struct lysp_module *)mod->parsed->includes[u].submodule;
        LY_CHECK_GOTO(ret = lys_precompile_mod_augments_deviations(submod, &mod_set), cleanup);
    }

    for (i = 0; i < mod_set.count; ++i) {
        m = mod_set.objs[i];

        if (m == mod) {
            /* will be applied normally later */
            continue;
        }

        /* we do not actually need the target modules compiled with out amends, they just need to be implemented
         * not compiled yet and marked for compilation */

        if (!m->implemented) {
            /* implement the target module */
            imp_f = (mod->ctx->flags & LY_CTX_ENABLE_IMP_FEATURES) ? all_f : NULL;
            r = lys_implement(m, imp_f, unres);
            if (r == LY_ERECOMPILE) {
                /* implement all the modules right away to save possible later recompilation */
                ret = r;
                continue;
            } else if (r) {
                /* error */
                ret = r;
                goto cleanup;
            }
        } else if (m->compiled) {
            /* target module was already compiled without our amends (augment/deviation), we need to recompile it */
            m->to_compile = 1;
            ret = LY_ERECOMPILE;
            continue;
        }
    }

cleanup:
    ly_set_erase(&mod_set, NULL);
    return ret;
}

void
lys_precompile_augments_deviations_revert(struct ly_ctx *ctx, const struct lys_module *mod)
{
    uint32_t i;
    LY_ARRAY_COUNT_TYPE u, count;
    struct lys_module *m;

    for (i = 0; i < ctx->list.count; ++i) {
        m = ctx->list.objs[i];

        if (m->augmented_by) {
            count = LY_ARRAY_COUNT(m->augmented_by);
            for (u = 0; u < count; ++u) {
                if (m->augmented_by[u] == mod) {
                    /* keep the order */
                    if (u < count - 1) {
                        memmove(m->augmented_by + u, m->augmented_by + u + 1, (count - u - 1) * sizeof *m->augmented_by);
                    }
                    LY_ARRAY_DECREMENT(m->augmented_by);
                    break;
                }
            }
            if (!LY_ARRAY_COUNT(m->augmented_by)) {
                LY_ARRAY_FREE(m->augmented_by);
                m->augmented_by = NULL;
            }
        }

        if (m->deviated_by) {
            count = LY_ARRAY_COUNT(m->deviated_by);
            for (u = 0; u < count; ++u) {
                if (m->deviated_by[u] == mod) {
                    /* keep the order */
                    if (u < count - 1) {
                        memmove(m->deviated_by + u, m->deviated_by + u + 1, (count - u - 1) * sizeof *m->deviated_by);
                    }
                    LY_ARRAY_DECREMENT(m->deviated_by);
                    break;
                }
            }
            if (!LY_ARRAY_COUNT(m->deviated_by)) {
                LY_ARRAY_FREE(m->deviated_by);
                m->deviated_by = NULL;
            }
        }
    }
}
