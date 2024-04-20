/**
 * @file schema_compile.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Schema compilation.
 *
 * Copyright (c) 2015 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "schema_compile.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "in.h"
#include "log.h"
#include "parser_schema.h"
#include "path.h"
#include "plugins.h"
#include "plugins_exts.h"
#include "plugins_internal.h"
#include "plugins_types.h"
#include "schema_compile_amend.h"
#include "schema_compile_node.h"
#include "schema_features.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_schema.h"
#include "tree_schema_free.h"
#include "tree_schema_internal.h"
#include "xpath.h"

void
lysc_update_path(struct lysc_ctx *ctx, const struct lys_module *parent_module, const char *name)
{
    int len;
    uint8_t nextlevel = 0; /* 0 - no starttag, 1 - '/' starttag, 2 - '=' starttag + '}' endtag */

    if (!name) {
        /* removing last path segment */
        if (ctx->path[ctx->path_len - 1] == '}') {
            for ( ; ctx->path[ctx->path_len] != '=' && ctx->path[ctx->path_len] != '{'; --ctx->path_len) {}
            if (ctx->path[ctx->path_len] == '=') {
                ctx->path[ctx->path_len++] = '}';
            } else {
                /* not a top-level special tag, remove also preceiding '/' */
                goto remove_nodelevel;
            }
        } else {
remove_nodelevel:
            for ( ; ctx->path[ctx->path_len] != '/'; --ctx->path_len) {}
            if (ctx->path_len == 0) {
                /* top-level (last segment) */
                ctx->path_len = 1;
            }
        }
        /* set new terminating NULL-byte */
        ctx->path[ctx->path_len] = '\0';
    } else {
        if (ctx->path_len > 1) {
            if (!parent_module && (ctx->path[ctx->path_len - 1] == '}') && (ctx->path[ctx->path_len - 2] != '\'')) {
                /* extension of the special tag */
                nextlevel = 2;
                --ctx->path_len;
            } else {
                /* there is already some path, so add next level */
                nextlevel = 1;
            }
        } /* else the path is just initiated with '/', so do not add additional slash in case of top-level nodes */

        if (nextlevel != 2) {
            if ((parent_module && (parent_module == ctx->cur_mod)) || (!parent_module && (ctx->path_len > 1) && (name[0] == '{'))) {
                /* module not changed, print the name unprefixed */
                len = snprintf(&ctx->path[ctx->path_len], LYSC_CTX_BUFSIZE - ctx->path_len, "%s%s", nextlevel ? "/" : "", name);
            } else {
                len = snprintf(&ctx->path[ctx->path_len], LYSC_CTX_BUFSIZE - ctx->path_len, "%s%s:%s", nextlevel ? "/" : "", ctx->cur_mod->name, name);
            }
        } else {
            len = snprintf(&ctx->path[ctx->path_len], LYSC_CTX_BUFSIZE - ctx->path_len, "='%s'}", name);
        }
        if (len >= LYSC_CTX_BUFSIZE - (int)ctx->path_len) {
            /* output truncated */
            ctx->path_len = LYSC_CTX_BUFSIZE - 1;
        } else {
            ctx->path_len += len;
        }
    }

    LOG_LOCBACK(0, 0, 1, 0);
    LOG_LOCSET(NULL, NULL, ctx->path, NULL);
}

/**
 * @brief Fill in the prepared compiled extensions definition structure according to the parsed extension definition.
 *
 * @param[in] ctx Compile context.
 * @param[in] extp Parsed extension instance.
 * @param[out] ext Compiled extension definition.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_extension(struct lysc_ctx *ctx, struct lysp_ext_instance *extp, struct lysc_ext **ext)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysp_ext *ep = extp->def;

    if (!ep->compiled) {
        lysc_update_path(ctx, NULL, "{extension}");
        lysc_update_path(ctx, NULL, ep->name);

        /* compile the extension definition */
        *ext = ep->compiled = calloc(1, sizeof **ext);
        (*ext)->refcount = 1;
        DUP_STRING_GOTO(ctx->ctx, ep->name, (*ext)->name, ret, cleanup);
        DUP_STRING_GOTO(ctx->ctx, ep->argname, (*ext)->argname, ret, cleanup);
        LY_CHECK_GOTO(ret = lysp_ext_find_definition(ctx->ctx, extp, (const struct lys_module **)&(*ext)->module, NULL),
                cleanup);

        /* compile nested extensions */
        COMPILE_EXTS_GOTO(ctx, ep->exts, (*ext)->exts, *ext, ret, cleanup);

        lysc_update_path(ctx, NULL, NULL);
        lysc_update_path(ctx, NULL, NULL);

        /* find extension definition plugin */
        (*ext)->plugin = extp->record ? (struct lyplg_ext *)&extp->record->plugin : NULL;
    }

    *ext = ep->compiled;

cleanup:
    if (ret) {
        lysc_update_path(ctx, NULL, NULL);
        lysc_update_path(ctx, NULL, NULL);
    }
    return ret;
}

LY_ERR
lys_compile_ext(struct lysc_ctx *ctx, struct lysp_ext_instance *extp, struct lysc_ext_instance *ext, void *parent)
{
    LY_ERR ret = LY_SUCCESS;

    DUP_STRING_GOTO(ctx->ctx, extp->argument, ext->argument, ret, cleanup);
    ext->module = ctx->cur_mod;
    ext->parent = parent;
    ext->parent_stmt = extp->parent_stmt;
    ext->parent_stmt_index = extp->parent_stmt_index;

    lysc_update_path(ctx, (ext->parent_stmt & LY_STMT_NODE_MASK) ? ((struct lysc_node *)ext->parent)->module : NULL,
            "{extension}");
    lysc_update_path(ctx, NULL, extp->name);

    /* compile extension if not already */
    LY_CHECK_GOTO(ret = lys_compile_extension(ctx, extp, &ext->def), cleanup);

    /* compile */
    if (ext->def->plugin && ext->def->plugin->compile) {
        if (ext->argument) {
            lysc_update_path(ctx, ext->module, ext->argument);
        }
        ret = ext->def->plugin->compile(ctx, extp, ext);
        if (ret == LY_ENOT) {
            lysc_ext_instance_free(&ctx->free_ctx, ext);
        }
        if (ext->argument) {
            lysc_update_path(ctx, NULL, NULL);
        }
        LY_CHECK_GOTO(ret, cleanup);
    }

cleanup:
    lysc_update_path(ctx, NULL, NULL);
    lysc_update_path(ctx, NULL, NULL);
    return ret;
}

static void
lysc_unres_must_free(struct lysc_unres_must *m)
{
    LY_ARRAY_FREE(m->local_mods);
    free(m);
}

static void
lysc_unres_dflt_free(const struct ly_ctx *ctx, struct lysc_unres_dflt *r)
{
    assert(!r->dflt || !r->dflts);
    if (r->dflt) {
        lysp_qname_free((struct ly_ctx *)ctx, r->dflt);
        free(r->dflt);
    } else {
        FREE_ARRAY((struct ly_ctx *)ctx, r->dflts, lysp_qname_free);
    }
    free(r);
}

LY_ERR
lys_identity_precompile(struct lysc_ctx *ctx_sc, struct ly_ctx *ctx, struct lysp_module *parsed_mod,
        const struct lysp_ident *identities_p, struct lysc_ident **identities)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_ctx cctx;
    struct lysc_ident *ident;
    LY_ERR ret = LY_SUCCESS;

    assert(ctx_sc || ctx);

    if (!ctx_sc) {
        if (parsed_mod) {
            LYSC_CTX_INIT_PMOD(cctx, parsed_mod, NULL);
        } else {
            LYSC_CTX_INIT_CTX(cctx, ctx);
        }
        ctx_sc = &cctx;
    }

    if (!identities_p) {
        return LY_SUCCESS;
    }

    lysc_update_path(ctx_sc, NULL, "{identity}");
    LY_ARRAY_FOR(identities_p, u) {
        lysc_update_path(ctx_sc, NULL, identities_p[u].name);

        /* add new compiled identity */
        LY_ARRAY_NEW_GOTO(ctx_sc->ctx, *identities, ident, ret, done);

        DUP_STRING_GOTO(ctx_sc->ctx, identities_p[u].name, ident->name, ret, done);
        DUP_STRING_GOTO(ctx_sc->ctx, identities_p[u].dsc, ident->dsc, ret, done);
        DUP_STRING_GOTO(ctx_sc->ctx, identities_p[u].ref, ident->ref, ret, done);
        ident->module = ctx_sc->cur_mod;
        /* backlinks (derived) can be added no sooner than when all the identities in the current module are present */
        COMPILE_EXTS_GOTO(ctx_sc, identities_p[u].exts, ident->exts, ident, ret, done);
        ident->flags = identities_p[u].flags;

        lysc_update_path(ctx_sc, NULL, NULL);
    }
    lysc_update_path(ctx_sc, NULL, NULL);

done:
    if (ret) {
        lysc_update_path(ctx_sc, NULL, NULL);
        lysc_update_path(ctx_sc, NULL, NULL);
    }
    return ret;
}

/**
 * @brief Check circular dependency of identities - identity MUST NOT reference itself (via their base statement).
 *
 * The function works in the same way as lys_compile_feature_circular_check() with different structures and error messages.
 *
 * @param[in] ctx Compile context for logging.
 * @param[in] ident The base identity (its derived list is being extended by the identity being currently processed).
 * @param[in] derived The list of derived identities of the identity being currently processed (not the one provided as @p ident)
 * @return LY_SUCCESS if everything is ok.
 * @return LY_EVALID if the identity is derived from itself.
 */
static LY_ERR
lys_compile_identity_circular_check(struct lysc_ctx *ctx, struct lysc_ident *ident, struct lysc_ident **derived)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    struct ly_set recursion = {0};
    struct lysc_ident *drv;

    if (!derived) {
        return LY_SUCCESS;
    }

    for (u = 0; u < LY_ARRAY_COUNT(derived); ++u) {
        if (ident == derived[u]) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE,
                    "Identity \"%s\" is indirectly derived from itself.", ident->name);
            ret = LY_EVALID;
            goto cleanup;
        }
        ret = ly_set_add(&recursion, derived[u], 0, NULL);
        LY_CHECK_GOTO(ret, cleanup);
    }

    for (v = 0; v < recursion.count; ++v) {
        drv = recursion.objs[v];
        for (u = 0; u < LY_ARRAY_COUNT(drv->derived); ++u) {
            if (ident == drv->derived[u]) {
                LOGVAL(ctx->ctx, LYVE_REFERENCE,
                        "Identity \"%s\" is indirectly derived from itself.", ident->name);
                ret = LY_EVALID;
                goto cleanup;
            }
            ret = ly_set_add(&recursion, drv->derived[u], 0, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    ly_set_erase(&recursion, NULL);
    return ret;
}

LY_ERR
lys_compile_identity_bases(struct lysc_ctx *ctx, const struct lysp_module *base_pmod, const char **bases_p,
        struct lysc_ident *ident, struct lysc_ident ***bases)
{
    LY_ARRAY_COUNT_TYPE u, v;
    const char *s, *name;
    const struct lys_module *mod;
    struct lysc_ident **idref;

    assert(ident || bases);

    if ((LY_ARRAY_COUNT(bases_p) > 1) && (ctx->pmod->version < LYS_VERSION_1_1)) {
        LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                "Multiple bases in %s are allowed only in YANG 1.1 modules.", ident ? "identity" : "identityref type");
        return LY_EVALID;
    }

    LY_ARRAY_FOR(bases_p, u) {
        s = strchr(bases_p[u], ':');
        if (s) {
            /* prefixed identity */
            name = &s[1];
            mod = ly_resolve_prefix(ctx->ctx, bases_p[u], s - bases_p[u], LY_VALUE_SCHEMA, (void *)base_pmod);
        } else {
            name = bases_p[u];
            mod = base_pmod->mod;
        }
        if (!mod) {
            if (ident) {
                LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                        "Invalid prefix used for base (%s) of identity \"%s\".", bases_p[u], ident->name);
            } else {
                LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                        "Invalid prefix used for base (%s) of identityref.", bases_p[u]);
            }
            return LY_EVALID;
        }

        idref = NULL;
        LY_ARRAY_FOR(mod->identities, v) {
            if (!strcmp(name, mod->identities[v].name)) {
                if (ident) {
                    if (ident == &mod->identities[v]) {
                        LOGVAL(ctx->ctx, LYVE_REFERENCE,
                                "Identity \"%s\" is derived from itself.", ident->name);
                        return LY_EVALID;
                    }
                    LY_CHECK_RET(lys_compile_identity_circular_check(ctx, &mod->identities[v], ident->derived));
                    /* we have match! store the backlink */
                    LY_ARRAY_NEW_RET(ctx->ctx, mod->identities[v].derived, idref, LY_EMEM);
                    *idref = ident;
                } else {
                    /* we have match! store the found identity */
                    LY_ARRAY_NEW_RET(ctx->ctx, *bases, idref, LY_EMEM);
                    *idref = &mod->identities[v];
                }
                break;
            }
        }
        if (!idref) {
            if (ident) {
                LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                        "Unable to find base (%s) of identity \"%s\".", bases_p[u], ident->name);
            } else {
                LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                        "Unable to find base (%s) of identityref.", bases_p[u]);
            }
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief For the given array of identities, set the backlinks from all their base identities.
 *
 * @param[in] ctx Compile context, not only for logging but also to get the current module to resolve prefixes.
 * @param[in] idents_p Array of identities definitions from the parsed schema structure.
 * @param[in,out] idents Array of referencing identities to which the backlinks are supposed to be set.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_identities_derived(struct lysc_ctx *ctx, struct lysp_ident *idents_p, struct lysc_ident **idents)
{
    LY_ARRAY_COUNT_TYPE u, v;

    lysc_update_path(ctx, NULL, "{identity}");

    for (u = 0; u < LY_ARRAY_COUNT(*idents); ++u) {
        /* find matching parsed identity */
        for (v = 0; v < LY_ARRAY_COUNT(idents_p); ++v) {
            if (idents_p[v].name == (*idents)[u].name) {
                break;
            }
        }

        if ((v == LY_ARRAY_COUNT(idents_p)) || !idents_p[v].bases) {
            /* identity not found (it may be from a submodule) or identity without bases */
            continue;
        }

        lysc_update_path(ctx, NULL, (*idents)[u].name);
        LY_CHECK_RET(lys_compile_identity_bases(ctx, ctx->pmod, idents_p[v].bases, &(*idents)[u], NULL));
        lysc_update_path(ctx, NULL, NULL);
    }

    lysc_update_path(ctx, NULL, NULL);
    return LY_SUCCESS;
}

LY_ERR
lys_compile_expr_implement(const struct ly_ctx *ctx, const struct lyxp_expr *expr, LY_VALUE_FORMAT format,
        void *prefix_data, ly_bool implement, struct lys_glob_unres *unres, const struct lys_module **mod_p)
{
    uint32_t i;
    const char *ptr, *start, **imp_f, *all_f[] = {"*", NULL};
    const struct lys_module *mod;

    assert(implement || mod_p);

    if (mod_p) {
        *mod_p = NULL;
    }

    for (i = 0; i < expr->used; ++i) {
        if ((expr->tokens[i] != LYXP_TOKEN_NAMETEST) && (expr->tokens[i] != LYXP_TOKEN_LITERAL)) {
            /* token cannot have a prefix */
            continue;
        }

        start = expr->expr + expr->tok_pos[i];
        if (!(ptr = ly_strnchr(start, ':', expr->tok_len[i]))) {
            /* token without a prefix */
            continue;
        }

        if (!(mod = ly_resolve_prefix(ctx, start, ptr - start, format, prefix_data))) {
            /* unknown prefix, do not care right now */
            continue;
        }

        /* unimplemented module found */
        if (!mod->implemented && !implement) {
            /* should not be implemented now */
            *mod_p = mod;
            break;
        }

        if (!mod->implemented) {
            /* implement if not implemented */
            imp_f = (ctx->flags & LY_CTX_ENABLE_IMP_FEATURES) ? all_f : NULL;
            LY_CHECK_RET(lys_implement((struct lys_module *)mod, imp_f, unres));
        }
        if (!mod->compiled) {
            /* compile if not implemented before or only marked for compilation */
            LY_CHECK_RET(lys_compile((struct lys_module *)mod, &unres->ds_unres));
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Check when for cyclic dependencies.
 *
 * @param[in] set Set with all the referenced nodes.
 * @param[in] node Node whose "when" referenced nodes are in @p set.
 * @return LY_ERR value
 */
static LY_ERR
lys_compile_unres_when_cyclic(struct lyxp_set *set, const struct lysc_node *node)
{
    struct lyxp_set tmp_set;
    struct lyxp_set_scnode *xp_scnode;
    uint32_t i, j, idx;
    LY_ARRAY_COUNT_TYPE u;
    LY_ERR ret = LY_SUCCESS;

    memset(&tmp_set, 0, sizeof tmp_set);

    /* prepare in_ctx of the set */
    for (i = 0; i < set->used; ++i) {
        xp_scnode = &set->val.scnodes[i];

        if (xp_scnode->in_ctx != LYXP_SET_SCNODE_START_USED) {
            /* check node when, skip the context node (it was just checked) */
            xp_scnode->in_ctx = LYXP_SET_SCNODE_ATOM_CTX;
        }
    }

    for (i = 0; i < set->used; ++i) {
        xp_scnode = &set->val.scnodes[i];
        if (xp_scnode->in_ctx != LYXP_SET_SCNODE_ATOM_CTX) {
            /* already checked */
            continue;
        }

        if ((xp_scnode->type != LYXP_NODE_ELEM) || !lysc_node_when(xp_scnode->scnode)) {
            /* no when to check */
            xp_scnode->in_ctx = LYXP_SET_SCNODE_ATOM_NODE;
            continue;
        }

        node = xp_scnode->scnode;
        do {
            struct lysc_when **when_list, *when;

            LOG_LOCSET(node, NULL, NULL, NULL);
            when_list = lysc_node_when(node);
            LY_ARRAY_FOR(when_list, u) {
                when = when_list[u];
                ret = lyxp_atomize(set->ctx, when->cond, node->module, LY_VALUE_SCHEMA_RESOLVED, when->prefixes,
                        when->context, when->context, &tmp_set, LYXP_SCNODE_SCHEMA);
                if (ret != LY_SUCCESS) {
                    LOGVAL(set->ctx, LYVE_SEMANTICS, "Invalid when condition \"%s\".", when->cond->expr);
                    LOG_LOCBACK(1, 0, 0, 0);
                    goto cleanup;
                }

                for (j = 0; j < tmp_set.used; ++j) {
                    if (tmp_set.val.scnodes[j].type != LYXP_NODE_ELEM) {
                        /* skip roots'n'stuff, no when, nothing to check */
                        tmp_set.val.scnodes[j].in_ctx = LYXP_SET_SCNODE_ATOM_NODE;
                        continue;
                    }

                    /* try to find this node in our set */
                    if (lyxp_set_scnode_contains(set, tmp_set.val.scnodes[j].scnode, LYXP_NODE_ELEM, -1, &idx) &&
                            (set->val.scnodes[idx].in_ctx == LYXP_SET_SCNODE_START_USED)) {
                        LOGVAL(set->ctx, LYVE_SEMANTICS, "When condition cyclic dependency on the node \"%s\".",
                                tmp_set.val.scnodes[j].scnode->name);
                        ret = LY_EVALID;
                        LOG_LOCBACK(1, 0, 0, 0);
                        goto cleanup;
                    }

                    /* needs to be checked, if in both sets, will be ignored */
                    tmp_set.val.scnodes[j].in_ctx = LYXP_SET_SCNODE_ATOM_CTX;
                }

                if (when->context != node) {
                    /* node actually depends on this "when", not the context node */
                    assert(tmp_set.val.scnodes[0].scnode == when->context);
                    if (tmp_set.val.scnodes[0].in_ctx == LYXP_SET_SCNODE_START_USED) {
                        /* replace the non-traversed context node with the dependent node */
                        tmp_set.val.scnodes[0].scnode = (struct lysc_node *)node;
                    } else {
                        /* context node was traversed, so just add the dependent node */
                        ret = lyxp_set_scnode_insert_node(&tmp_set, node, LYXP_SET_SCNODE_START_USED, LYXP_AXIS_CHILD, NULL);
                        LY_CHECK_ERR_GOTO(ret, LOG_LOCBACK(1, 0, 0, 0), cleanup);
                    }
                }

                /* merge this set into the global when set */
                lyxp_set_scnode_merge(set, &tmp_set);
            }
            LOG_LOCBACK(1, 0, 0, 0);

            /* check when of non-data parents as well */
            node = node->parent;
        } while (node && (node->nodetype & (LYS_CASE | LYS_CHOICE)));

        /* this node when was checked (xp_scnode could have been reallocd) */
        set->val.scnodes[i].in_ctx = LYXP_SET_SCNODE_ATOM_NODE;
    }

cleanup:
    lyxp_set_free_content(&tmp_set);
    return ret;
}

LY_ERR
lysc_check_status(struct lysc_ctx *ctx, uint16_t flags1, void *mod1, const char *name1, uint16_t flags2, void *mod2,
        const char *name2)
{
    uint16_t flg1, flg2;

    flg1 = (flags1 & LYS_STATUS_MASK) ? (flags1 & LYS_STATUS_MASK) : LYS_STATUS_CURR;
    flg2 = (flags2 & LYS_STATUS_MASK) ? (flags2 & LYS_STATUS_MASK) : LYS_STATUS_CURR;

    if ((flg1 < flg2) && (mod1 == mod2)) {
        if (ctx) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE, "A %s definition \"%s\" is not allowed to reference %s definition \"%s\".",
                    flg1 == LYS_STATUS_CURR ? "current" : "deprecated", name1,
                    flg2 == LYS_STATUS_OBSLT ? "obsolete" : "deprecated", name2);
        }
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Check when expressions of a node on a complete compiled schema tree.
 *
 * @param[in] ctx Compile context.
 * @param[in] when When to check.
 * @param[in] node Node with @p when.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_unres_when(struct lysc_ctx *ctx, const struct lysc_when *when, const struct lysc_node *node)
{
    struct lyxp_set tmp_set = {0};
    uint32_t i, opts;
    struct lysc_node *schema;
    LY_ERR ret = LY_SUCCESS;

    opts = LYXP_SCNODE_SCHEMA | ((node->flags & LYS_IS_OUTPUT) ? LYXP_SCNODE_OUTPUT : 0);

    /* check "when" */
    ret = lyxp_atomize(ctx->ctx, when->cond, node->module, LY_VALUE_SCHEMA_RESOLVED, when->prefixes, when->context,
            when->context, &tmp_set, opts);
    if (ret) {
        LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Invalid when condition \"%s\".", when->cond->expr);
        goto cleanup;
    }

    ctx->path[0] = '\0';
    lysc_path(node, LYSC_PATH_LOG, ctx->path, LYSC_CTX_BUFSIZE);
    for (i = 0; i < tmp_set.used; ++i) {
        if (tmp_set.val.scnodes[i].type != LYXP_NODE_ELEM) {
            /* skip roots'n'stuff */
            continue;
        } else if (tmp_set.val.scnodes[i].in_ctx == LYXP_SET_SCNODE_START_USED) {
            /* context node not actually traversed */
            continue;
        }

        schema = tmp_set.val.scnodes[i].scnode;

        /* XPath expression cannot reference "lower" status than the node that has the definition */
        if (lysc_check_status(NULL, when->flags, node->module, node->name, schema->flags, schema->module,
                schema->name)) {
            LOGWRN(ctx->ctx, "When condition \"%s\" may be referencing %s node \"%s\".", when->cond->expr,
                    (schema->flags == LYS_STATUS_OBSLT) ? "obsolete" : "deprecated", schema->name);
        }

        /* check dummy node children/value accessing */
        if (lysc_data_parent(schema) == node) {
            LOGVAL(ctx->ctx, LYVE_SEMANTICS, "When condition is accessing its own conditional node children.");
            ret = LY_EVALID;
            goto cleanup;
        } else if ((schema == node) && (tmp_set.val.scnodes[i].in_ctx == LYXP_SET_SCNODE_ATOM_VAL)) {
            LOGVAL(ctx->ctx, LYVE_SEMANTICS, "When condition is accessing its own conditional node value.");
            ret = LY_EVALID;
            goto cleanup;
        }
    }

    if (when->context != node) {
        /* node actually depends on this "when", not the context node */
        assert(tmp_set.val.scnodes[0].scnode == when->context);
        if (tmp_set.val.scnodes[0].in_ctx == LYXP_SET_SCNODE_START_USED) {
            /* replace the non-traversed context node with the dependent node */
            tmp_set.val.scnodes[0].scnode = (struct lysc_node *)node;
        } else {
            /* context node was traversed, so just add the dependent node */
            ret = lyxp_set_scnode_insert_node(&tmp_set, node, LYXP_SET_SCNODE_START_USED, LYXP_AXIS_CHILD, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

    /* check cyclic dependencies */
    ret = lys_compile_unres_when_cyclic(&tmp_set, node);
    LY_CHECK_GOTO(ret, cleanup);

cleanup:
    lyxp_set_free_content(&tmp_set);
    return ret;
}

/**
 * @brief Check must expressions of a node on a complete compiled schema tree.
 *
 * @param[in] ctx Compile context.
 * @param[in] node Node to check.
 * @param[in] local_mods Sized array of local modules for musts of @p node at the same index.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_unres_must(struct lysc_ctx *ctx, const struct lysc_node *node, const struct lysp_module **local_mods)
{
    struct lyxp_set tmp_set;
    uint32_t i, opts;
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_must *musts;
    LY_ERR ret = LY_SUCCESS;
    uint16_t flg;

    LOG_LOCSET(node, NULL, NULL, NULL);

    memset(&tmp_set, 0, sizeof tmp_set);
    opts = LYXP_SCNODE_SCHEMA | ((node->flags & LYS_IS_OUTPUT) ? LYXP_SCNODE_OUTPUT : 0);

    musts = lysc_node_musts(node);
    LY_ARRAY_FOR(musts, u) {
        /* check "must" */
        ret = lyxp_atomize(ctx->ctx, musts[u].cond, node->module, LY_VALUE_SCHEMA_RESOLVED, musts[u].prefixes, node,
                node, &tmp_set, opts);
        if (ret) {
            LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Invalid must condition \"%s\".", musts[u].cond->expr);
            goto cleanup;
        }

        ctx->path[0] = '\0';
        lysc_path(node, LYSC_PATH_LOG, ctx->path, LYSC_CTX_BUFSIZE);
        for (i = 0; i < tmp_set.used; ++i) {
            /* skip roots'n'stuff */
            if (tmp_set.val.scnodes[i].type == LYXP_NODE_ELEM) {
                struct lysc_node *schema = tmp_set.val.scnodes[i].scnode;

                /* XPath expression cannot reference "lower" status than the node that has the definition */
                if (local_mods[u]->mod == node->module) {
                    /* use flags of the context node since the definition is local */
                    flg = node->flags;
                } else {
                    /* definition is foreign (deviation, refine), always current */
                    flg = LYS_STATUS_CURR;
                }
                if (lysc_check_status(NULL, flg, local_mods[u]->mod, node->name, schema->flags, schema->module,
                        schema->name)) {
                    LOGWRN(ctx->ctx, "Must condition \"%s\" may be referencing %s node \"%s\".", musts[u].cond->expr,
                            (schema->flags == LYS_STATUS_OBSLT) ? "obsolete" : "deprecated", schema->name);
                    break;
                }
            }
        }

        lyxp_set_free_content(&tmp_set);
    }

cleanup:
    lyxp_set_free_content(&tmp_set);
    LOG_LOCBACK(1, 0, 0, 0);
    return ret;
}

/**
 * @brief Remove all disabled bits/enums from a sized array.
 *
 * @param[in] ctx Context with the dictionary.
 * @param[in] items Sized array of bits/enums.
 */
static void
lys_compile_unres_disabled_bitenum_remove(struct lysf_ctx *ctx, struct lysc_type_bitenum_item *items)
{
    LY_ARRAY_COUNT_TYPE u = 0, last_u;

    while (u < LY_ARRAY_COUNT(items)) {
        if (items[u].flags & LYS_DISABLED) {
            /* free the disabled item */
            lysc_enum_item_free(ctx, &items[u]);

            /* replace it with the following items */
            last_u = LY_ARRAY_COUNT(items) - 1;
            if (u < last_u) {
                memmove(items + u, items + u + 1, (last_u - u) * sizeof *items);
            }

            /* one item less */
            LY_ARRAY_DECREMENT(items);
            continue;
        }

        ++u;
    }
}

/**
 * @brief Find and remove all disabled bits/enums in a leaf/leaf-list type.
 *
 * @param[in] ctx Compile context.
 * @param[in] leaf Leaf/leaf-list to check.
 * @return LY_ERR value
 */
static LY_ERR
lys_compile_unres_disabled_bitenum(struct lysc_ctx *ctx, struct lysc_node_leaf *leaf)
{
    struct lysc_type **t;
    LY_ARRAY_COUNT_TYPE u, count;
    struct lysc_type_enum *ent;
    ly_bool has_value = 0;

    if (leaf->type->basetype == LY_TYPE_UNION) {
        t = ((struct lysc_type_union *)leaf->type)->types;
        count = LY_ARRAY_COUNT(t);
    } else {
        t = &leaf->type;
        count = 1;
    }
    for (u = 0; u < count; ++u) {
        if ((t[u]->basetype == LY_TYPE_BITS) || (t[u]->basetype == LY_TYPE_ENUM)) {
            /* remove all disabled items */
            ent = (struct lysc_type_enum *)(t[u]);
            lys_compile_unres_disabled_bitenum_remove(&ctx->free_ctx, ent->enums);

            if (LY_ARRAY_COUNT(ent->enums)) {
                has_value = 1;
            }
        } else {
            has_value = 1;
        }
    }

    if (!has_value) {
        LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Node \"%s\" without any (or all disabled) valid values.", leaf->name);
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Check leafref for its target existence on a complete compiled schema tree.
 *
 * @param[in] ctx Compile context.
 * @param[in] node Context node for the leafref.
 * @param[in] lref Leafref to check/resolve.
 * @param[in] local_mod Local module for the leafref type.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_unres_leafref(struct lysc_ctx *ctx, const struct lysc_node *node, struct lysc_type_leafref *lref,
        const struct lysp_module *local_mod)
{
    const struct lysc_node *target = NULL;
    struct ly_path *p;
    struct lysc_type *type;
    uint16_t flg;

    assert(node->nodetype & (LYS_LEAF | LYS_LEAFLIST));

    if (lref->realtype) {
        /* already resolved, may happen (shared union typedef with a leafref) */
        return LY_SUCCESS;
    }

    /* try to find the target, current module is that of the context node (RFC 7950 6.4.1 second bullet) */
    LY_CHECK_RET(ly_path_compile_leafref(ctx->ctx, node, ctx->ext, lref->path,
            (node->flags & LYS_IS_OUTPUT) ? LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT, LY_PATH_TARGET_MANY,
            LY_VALUE_SCHEMA_RESOLVED, lref->prefixes, &p));

    /* get the target node */
    target = p[LY_ARRAY_COUNT(p) - 1].node;
    ly_path_free(node->module->ctx, p);

    if (!(target->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid leafref path \"%s\" - target node is %s instead of leaf or leaf-list.",
                lref->path->expr, lys_nodetype2str(target->nodetype));
        return LY_EVALID;
    }

    /* check status */
    ctx->path[0] = '\0';
    lysc_path(node, LYSC_PATH_LOG, ctx->path, LYSC_CTX_BUFSIZE);
    ctx->path_len = strlen(ctx->path);
    if (node->module == local_mod->mod) {
        /* use flags of the context node since the definition is local */
        flg = node->flags;
    } else {
        /* definition is foreign (deviation), always current */
        flg = LYS_STATUS_CURR;
    }
    if (lysc_check_status(ctx, flg, local_mod->mod, node->name, target->flags, target->module, target->name)) {
        return LY_EVALID;
    }
    ctx->path_len = 1;
    ctx->path[1] = '\0';

    /* check config */
    if (lref->require_instance) {
        if ((node->flags & LYS_CONFIG_W) && (target->flags & LYS_CONFIG_R)) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid leafref path \"%s\" - target is supposed"
                    " to represent configuration data (as the leafref does), but it does not.", lref->path->expr);
            return LY_EVALID;
        }
    }

    /* check for circular chain of leafrefs */
    for (type = ((struct lysc_node_leaf *)target)->type;
            type && (type->basetype == LY_TYPE_LEAFREF);
            type = ((struct lysc_type_leafref *)type)->realtype) {
        if (type == (struct lysc_type *)lref) {
            /* circular chain detected */
            LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid leafref path \"%s\" - circular chain of leafrefs detected.",
                    lref->path->expr);
            return LY_EVALID;
        }
    }

    /* store the type */
    lref->realtype = ((struct lysc_node_leaf *)target)->type;
    ++lref->realtype->refcount;
    return LY_SUCCESS;
}

/**
 * @brief Compile default value(s) for leaf or leaf-list expecting a complete compiled schema tree.
 *
 * @param[in] ctx Compile context.
 * @param[in] node Leaf or leaf-list to compile the default value(s) for.
 * @param[in] type Type of the default value.
 * @param[in] dflt Default value.
 * @param[in] dflt_pmod Parsed module of the @p dflt to resolve possible prefixes.
 * @param[in,out] storage Storage for the compiled default value.
 * @param[in,out] unres Global unres structure for newly implemented modules.
 * @return LY_ERECOMPILE if the whole dep set needs to be recompiled for the value to be checked.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_unres_dflt(struct lysc_ctx *ctx, struct lysc_node *node, struct lysc_type *type, const char *dflt,
        const struct lysp_module *dflt_pmod, struct lyd_value *storage, struct lys_glob_unres *unres)
{
    LY_ERR ret;
    uint32_t options;
    struct ly_err_item *err = NULL;

    options = (ctx->ctx->flags & LY_CTX_REF_IMPLEMENTED) ? LYPLG_TYPE_STORE_IMPLEMENT : 0;
    ret = type->plugin->store(ctx->ctx, type, dflt, strlen(dflt), options, LY_VALUE_SCHEMA, (void *)dflt_pmod,
            LYD_HINT_SCHEMA, node, storage, unres, &err);
    if (ret == LY_ERECOMPILE) {
        /* fine, but we need to recompile */
        return LY_ERECOMPILE;
    } else if (ret == LY_EINCOMPLETE) {
        /* we have no data so we will not be resolving it */
        ret = LY_SUCCESS;
    }

    if (ret) {
        LOG_LOCSET(node, NULL, NULL, NULL);
        if (err) {
            LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Invalid default - value does not fit the type (%s).", err->msg);
            ly_err_free(err);
        } else {
            LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Invalid default - value does not fit the type.");
        }
        LOG_LOCBACK(1, 0, 0, 0);
        return ret;
    }

    LY_ATOMIC_INC_BARRIER(((struct lysc_type *)storage->realtype)->refcount);
    return LY_SUCCESS;
}

/**
 * @brief Compile default value of a leaf expecting a complete compiled schema tree.
 *
 * @param[in] ctx Compile context.
 * @param[in] leaf Leaf that the default value is for.
 * @param[in] dflt Default value to compile.
 * @param[in,out] unres Global unres structure for newly implemented modules.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_unres_leaf_dlft(struct lysc_ctx *ctx, struct lysc_node_leaf *leaf, struct lysp_qname *dflt,
        struct lys_glob_unres *unres)
{
    LY_ERR ret;

    assert(!leaf->dflt);

    if (leaf->flags & (LYS_MAND_TRUE | LYS_KEY)) {
        /* ignore default values for keys and mandatory leaves */
        return LY_SUCCESS;
    }

    /* allocate the default value */
    leaf->dflt = calloc(1, sizeof *leaf->dflt);
    LY_CHECK_ERR_RET(!leaf->dflt, LOGMEM(ctx->ctx), LY_EMEM);

    /* store the default value */
    ret = lys_compile_unres_dflt(ctx, &leaf->node, leaf->type, dflt->str, dflt->mod, leaf->dflt, unres);
    if (ret) {
        free(leaf->dflt);
        leaf->dflt = NULL;
    }

    return ret;
}

/**
 * @brief Compile default values of a leaf-list expecting a complete compiled schema tree.
 *
 * @param[in] ctx Compile context.
 * @param[in] llist Leaf-list that the default value(s) are for.
 * @param[in] dflt Default value to compile, in case of a single value.
 * @param[in] dflts Sized array of default values, in case of more values.
 * @param[in,out] unres Global unres structure for newly implemented modules.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_unres_llist_dflts(struct lysc_ctx *ctx, struct lysc_node_leaflist *llist, struct lysp_qname *dflt,
        struct lysp_qname *dflts, struct lys_glob_unres *unres)
{
    LY_ERR ret;
    LY_ARRAY_COUNT_TYPE orig_count, u, v;

    assert(dflt || dflts);

    /* in case there were already some defaults and we are adding new by deviations */
    orig_count = LY_ARRAY_COUNT(llist->dflts);

    /* allocate new items */
    LY_ARRAY_CREATE_RET(ctx->ctx, llist->dflts, orig_count + (dflts ? LY_ARRAY_COUNT(dflts) : 1), LY_EMEM);

    /* fill each new default value */
    if (dflts) {
        LY_ARRAY_FOR(dflts, u) {
            llist->dflts[orig_count + u] = calloc(1, sizeof **llist->dflts);
            ret = lys_compile_unres_dflt(ctx, &llist->node, llist->type, dflts[u].str, dflts[u].mod,
                    llist->dflts[orig_count + u], unres);
            LY_CHECK_ERR_RET(ret, free(llist->dflts[orig_count + u]), ret);
            LY_ARRAY_INCREMENT(llist->dflts);
        }
    } else {
        llist->dflts[orig_count] = calloc(1, sizeof **llist->dflts);
        ret = lys_compile_unres_dflt(ctx, &llist->node, llist->type, dflt->str, dflt->mod,
                llist->dflts[orig_count], unres);
        LY_CHECK_ERR_RET(ret, free(llist->dflts[orig_count]), ret);
        LY_ARRAY_INCREMENT(llist->dflts);
    }

    /* check default value uniqueness */
    if (llist->flags & LYS_CONFIG_W) {
        /* configuration data values must be unique - so check the default values */
        for (u = orig_count; u < LY_ARRAY_COUNT(llist->dflts); ++u) {
            for (v = 0; v < u; ++v) {
                if (!llist->dflts[u]->realtype->plugin->compare(llist->dflts[u], llist->dflts[v])) {
                    lysc_update_path(ctx, llist->parent ? llist->parent->module : NULL, llist->name);
                    LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Configuration leaf-list has multiple defaults of the same value \"%s\".",
                            (char *)llist->dflts[u]->realtype->plugin->print(ctx->ctx, llist->dflts[u], LY_VALUE_CANON,
                            NULL, NULL, NULL));
                    lysc_update_path(ctx, NULL, NULL);
                    return LY_EVALID;
                }
            }
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Iteratively get all leafrefs from @p node
 * if the node is of type union, otherwise just return the leafref.
 *
 * @param[in] node Node that may contain the leafref.
 * @param[in,out] index Value that is passed between function calls.
 * For each new node, initialize value of the @p index to 0, otherwise
 * do not modify the value between calls.
 * @return Pointer to the leafref or next leafref, otherwise NULL.
 */
static struct lysc_type_leafref *
lys_type_leafref_next(const struct lysc_node *node, uint64_t *index)
{
    struct lysc_type_leafref *ret = NULL;
    struct lysc_type_union *uni;
    struct lysc_type *leaf_type;

    assert(node->nodetype & LYD_NODE_TERM);

    leaf_type = ((struct lysc_node_leaf *)node)->type;
    if (leaf_type->basetype == LY_TYPE_UNION) {
        uni = (struct lysc_type_union *)leaf_type;

        /* find next union leafref */
        while (*index < LY_ARRAY_COUNT(uni->types)) {
            if (uni->types[*index]->basetype == LY_TYPE_LEAFREF) {
                ret = (struct lysc_type_leafref *)uni->types[*index];
                ++(*index);
                break;
            }

            ++(*index);
        }
    } else {
        /* return just the single leafref */
        if (*index == 0) {
            ++(*index);
            assert(leaf_type->basetype == LY_TYPE_LEAFREF);
            ret = (struct lysc_type_leafref *)leaf_type;
        }
    }

    return ret;
}

/**
 * @brief Implement all referenced modules by leafrefs, when and must conditions.
 *
 * @param[in] ctx libyang context.
 * @param[in] unres Global unres structure with the sets to resolve.
 * @return LY_SUCCESS on success.
 * @return LY_ERECOMPILE if the whole dep set needs to be recompiled with the new implemented modules.
 * @return LY_ERR value on error.
 */
static LY_ERR
lys_compile_unres_depset_implement(struct ly_ctx *ctx, struct lys_glob_unres *unres)
{
    struct lys_depset_unres *ds_unres = &unres->ds_unres;
    struct lysc_type_leafref *lref;
    const struct lys_module *mod;
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_unres_leafref *l;
    struct lysc_unres_when *w;
    struct lysc_unres_must *m;
    struct lysc_must *musts;
    ly_bool not_implemented;
    uint32_t di = 0, li = 0, wi = 0, mi = 0;

implement_all:
    /* disabled leafrefs - even those because we need to check their target exists */
    while (di < ds_unres->disabled_leafrefs.count) {
        l = ds_unres->disabled_leafrefs.objs[di];

        u = 0;
        while ((lref = lys_type_leafref_next(l->node, &u))) {
            LY_CHECK_RET(lys_compile_expr_implement(ctx, lref->path, LY_VALUE_SCHEMA_RESOLVED, lref->prefixes, 1, unres, NULL));
        }

        ++di;
    }

    /* leafrefs */
    while (li < ds_unres->leafrefs.count) {
        l = ds_unres->leafrefs.objs[li];

        u = 0;
        while ((lref = lys_type_leafref_next(l->node, &u))) {
            LY_CHECK_RET(lys_compile_expr_implement(ctx, lref->path, LY_VALUE_SCHEMA_RESOLVED, lref->prefixes, 1, unres, NULL));
        }

        ++li;
    }

    /* when conditions */
    while (wi < ds_unres->whens.count) {
        w = ds_unres->whens.objs[wi];

        LY_CHECK_RET(lys_compile_expr_implement(ctx, w->when->cond, LY_VALUE_SCHEMA_RESOLVED, w->when->prefixes,
                ctx->flags & LY_CTX_REF_IMPLEMENTED, unres, &mod));
        if (mod) {
            LOGWRN(ctx, "When condition \"%s\" check skipped because referenced module \"%s\" is not implemented.",
                    w->when->cond->expr, mod->name);

            /* remove from the set to skip the check */
            ly_set_rm_index(&ds_unres->whens, wi, free);
            continue;
        }

        ++wi;
    }

    /* must conditions */
    while (mi < ds_unres->musts.count) {
        m = ds_unres->musts.objs[mi];

        not_implemented = 0;
        musts = lysc_node_musts(m->node);
        LY_ARRAY_FOR(musts, u) {
            LY_CHECK_RET(lys_compile_expr_implement(ctx, musts[u].cond, LY_VALUE_SCHEMA_RESOLVED, musts[u].prefixes,
                    ctx->flags & LY_CTX_REF_IMPLEMENTED, unres, &mod));
            if (mod) {
                LOGWRN(ctx, "Must condition \"%s\" check skipped because referenced module \"%s\" is not implemented.",
                        musts[u].cond->expr, mod->name);

                /* need to implement modules from all the expressions */
                not_implemented = 1;
            }
        }

        if (not_implemented) {
            /* remove from the set to skip the check */
            lysc_unres_must_free(m);
            ly_set_rm_index(&ds_unres->musts, mi, NULL);
            continue;
        }

        ++mi;
    }

    if ((di < ds_unres->disabled_leafrefs.count) || (li < ds_unres->leafrefs.count) || (wi < ds_unres->whens.count)) {
        /* new items in the sets */
        goto implement_all;
    }

    return LY_SUCCESS;
}

/**
 * @brief Finish dependency set compilation by resolving all the unres sets.
 *
 * @param[in] ctx libyang context.
 * @param[in] unres Global unres structure with the sets to resolve.
 * @return LY_SUCCESS on success.
 * @return LY_ERECOMPILE if the dep set needs to be recompiled.
 * @return LY_ERR value on error.
 */
static LY_ERR
lys_compile_unres_depset(struct ly_ctx *ctx, struct lys_glob_unres *unres)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_node *node;
    struct lysc_type *typeiter;
    struct lysc_type_leafref *lref;
    struct lysc_ctx cctx = {0};
    struct lys_depset_unres *ds_unres = &unres->ds_unres;
    struct ly_path *path;
    LY_ARRAY_COUNT_TYPE v;
    struct lysc_unres_leafref *l;
    struct lysc_unres_when *w;
    struct lysc_unres_must *m;
    struct lysc_unres_dflt *d;
    uint32_t i, processed_leafrefs = 0;

resolve_all:
    /* implement all referenced modules to get final ds_unres set */
    if ((ret = lys_compile_unres_depset_implement(ctx, unres))) {
        goto cleanup;
    }

    /* check disabled leafrefs */
    while (ds_unres->disabled_leafrefs.count) {
        /* remember index, it can change before we get to free this item */
        i = ds_unres->disabled_leafrefs.count - 1;
        l = ds_unres->disabled_leafrefs.objs[i];
        LYSC_CTX_INIT_PMOD(cctx, l->node->module->parsed, l->ext);

        LOG_LOCSET(l->node, NULL, NULL, NULL);
        v = 0;
        while ((ret == LY_SUCCESS) && (lref = lys_type_leafref_next(l->node, &v))) {
            ret = lys_compile_unres_leafref(&cctx, l->node, lref, l->local_mod);
        }
        LOG_LOCBACK(1, 0, 0, 0);
        LY_CHECK_GOTO(ret, cleanup);

        ly_set_rm_index(&ds_unres->disabled_leafrefs, i, free);
    }

    /* for leafref, we need 2 rounds - first detects circular chain by storing the first referred type (which
     * can be also leafref, in case it is already resolved, go through the chain and check that it does not
     * point to the starting leafref type). The second round stores the first non-leafref type for later data validation.
     * Also do the same check for set of the disabled leafrefs, but without the second round. */
    for (i = processed_leafrefs; i < ds_unres->leafrefs.count; ++i) {
        l = ds_unres->leafrefs.objs[i];
        LYSC_CTX_INIT_PMOD(cctx, l->node->module->parsed, l->ext);

        LOG_LOCSET(l->node, NULL, NULL, NULL);
        v = 0;
        while ((ret == LY_SUCCESS) && (lref = lys_type_leafref_next(l->node, &v))) {
            ret = lys_compile_unres_leafref(&cctx, l->node, lref, l->local_mod);
        }
        LOG_LOCBACK(1, 0, 0, 0);
        LY_CHECK_GOTO(ret, cleanup);
    }
    for (i = processed_leafrefs; i < ds_unres->leafrefs.count; ++i) {
        l = ds_unres->leafrefs.objs[i];

        /* store pointer to the real type */
        v = 0;
        while ((lref = lys_type_leafref_next(l->node, &v))) {
            for (typeiter = lref->realtype;
                    typeiter->basetype == LY_TYPE_LEAFREF;
                    typeiter = ((struct lysc_type_leafref *)typeiter)->realtype) {}

            lysc_type_free(&cctx.free_ctx, lref->realtype);
            lref->realtype = typeiter;
            ++lref->realtype->refcount;
        }

        /* if 'goto' will be used on the 'resolve_all' label, then the current leafref will not be processed again */
        processed_leafrefs++;
    }

    /* check when, the referenced modules must be implemented now */
    while (ds_unres->whens.count) {
        i = ds_unres->whens.count - 1;
        w = ds_unres->whens.objs[i];
        LYSC_CTX_INIT_PMOD(cctx, w->node->module->parsed, NULL);

        LOG_LOCSET(w->node, NULL, NULL, NULL);
        ret = lys_compile_unres_when(&cctx, w->when, w->node);
        LOG_LOCBACK(w->node ? 1 : 0, 0, 0, 0);
        LY_CHECK_GOTO(ret, cleanup);

        free(w);
        ly_set_rm_index(&ds_unres->whens, i, NULL);
    }

    /* check must */
    while (ds_unres->musts.count) {
        i = ds_unres->musts.count - 1;
        m = ds_unres->musts.objs[i];
        LYSC_CTX_INIT_PMOD(cctx, m->node->module->parsed, m->ext);

        LOG_LOCSET(m->node, NULL, NULL, NULL);
        ret = lys_compile_unres_must(&cctx, m->node, m->local_mods);
        LOG_LOCBACK(1, 0, 0, 0);
        LY_CHECK_GOTO(ret, cleanup);

        lysc_unres_must_free(m);
        ly_set_rm_index(&ds_unres->musts, i, NULL);
    }

    /* remove disabled enums/bits */
    while (ds_unres->disabled_bitenums.count) {
        i = ds_unres->disabled_bitenums.count - 1;
        node = ds_unres->disabled_bitenums.objs[i];
        LYSC_CTX_INIT_PMOD(cctx, node->module->parsed, NULL);

        LOG_LOCSET(node, NULL, NULL, NULL);
        ret = lys_compile_unres_disabled_bitenum(&cctx, (struct lysc_node_leaf *)node);
        LOG_LOCBACK(1, 0, 0, 0);
        LY_CHECK_GOTO(ret, cleanup);

        ly_set_rm_index(&ds_unres->disabled_bitenums, i, NULL);
    }

    /* finish incomplete default values compilation */
    while (ds_unres->dflts.count) {
        i = ds_unres->dflts.count - 1;
        d = ds_unres->dflts.objs[i];
        LYSC_CTX_INIT_PMOD(cctx, d->leaf->module->parsed, NULL);

        LOG_LOCSET(&d->leaf->node, NULL, NULL, NULL);
        if (d->leaf->nodetype == LYS_LEAF) {
            ret = lys_compile_unres_leaf_dlft(&cctx, d->leaf, d->dflt, unres);
        } else {
            ret = lys_compile_unres_llist_dflts(&cctx, d->llist, d->dflt, d->dflts, unres);
        }
        LOG_LOCBACK(1, 0, 0, 0);
        LY_CHECK_GOTO(ret, cleanup);

        lysc_unres_dflt_free(ctx, d);
        ly_set_rm_index(&ds_unres->dflts, i, NULL);
    }

    /* some unres items may have been added by the default values */
    if ((processed_leafrefs != ds_unres->leafrefs.count) || ds_unres->disabled_leafrefs.count ||
            ds_unres->whens.count || ds_unres->musts.count || ds_unres->dflts.count) {
        goto resolve_all;
    }

    /* finally, remove all disabled nodes */
    for (i = 0; i < ds_unres->disabled.count; ++i) {
        node = ds_unres->disabled.snodes[i];
        if (node->flags & LYS_KEY) {
            LOG_LOCSET(node, NULL, NULL, NULL);
            LOGVAL(ctx, LYVE_REFERENCE, "Key \"%s\" is disabled.", node->name);
            LOG_LOCBACK(1, 0, 0, 0);
            ret = LY_EVALID;
            goto cleanup;
        }
        LYSC_CTX_INIT_PMOD(cctx, node->module->parsed, NULL);

        lysc_node_free(&cctx.free_ctx, node, 1);
    }

    /* also check if the leafref target has not been disabled */
    for (i = 0; i < ds_unres->leafrefs.count; ++i) {
        l = ds_unres->leafrefs.objs[i];
        LYSC_CTX_INIT_PMOD(cctx, l->node->module->parsed, l->ext);

        v = 0;
        while ((lref = lys_type_leafref_next(l->node, &v))) {
            ret = ly_path_compile_leafref(cctx.ctx, l->node, cctx.ext, lref->path,
                    (l->node->flags & LYS_IS_OUTPUT) ? LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT, LY_PATH_TARGET_MANY,
                    LY_VALUE_SCHEMA_RESOLVED, lref->prefixes, &path);
            ly_path_free(l->node->module->ctx, path);

            assert(ret != LY_ERECOMPILE);
            if (ret) {
                LOG_LOCSET(l->node, NULL, NULL, NULL);
                LOGVAL(ctx, LYVE_REFERENCE, "Target of leafref \"%s\" cannot be referenced because it is disabled.",
                        l->node->name);
                LOG_LOCBACK(1, 0, 0, 0);
                ret = LY_EVALID;
                goto cleanup;
            }
        }
    }

cleanup:
    lysf_ctx_erase(&cctx.free_ctx);
    return ret;
}

/**
 * @brief Erase dep set unres.
 *
 * @param[in] ctx libyang context.
 * @param[in] unres Global unres structure with the sets to resolve.
 */
static void
lys_compile_unres_depset_erase(const struct ly_ctx *ctx, struct lys_glob_unres *unres)
{
    uint32_t i;

    ly_set_erase(&unres->ds_unres.whens, free);
    for (i = 0; i < unres->ds_unres.musts.count; ++i) {
        lysc_unres_must_free(unres->ds_unres.musts.objs[i]);
    }
    ly_set_erase(&unres->ds_unres.musts, NULL);
    ly_set_erase(&unres->ds_unres.leafrefs, free);
    for (i = 0; i < unres->ds_unres.dflts.count; ++i) {
        lysc_unres_dflt_free(ctx, unres->ds_unres.dflts.objs[i]);
    }
    ly_set_erase(&unres->ds_unres.dflts, NULL);
    ly_set_erase(&unres->ds_unres.disabled, NULL);
    ly_set_erase(&unres->ds_unres.disabled_leafrefs, free);
    ly_set_erase(&unres->ds_unres.disabled_bitenums, NULL);
}

/**
 * @brief Compile all flagged modules in a dependency set, recursively if recompilation is needed.
 *
 * @param[in] ctx libyang context.
 * @param[in] dep_set Dependency set to compile.
 * @param[in,out] unres Global unres to use.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_depset_r(struct ly_ctx *ctx, struct ly_set *dep_set, struct lys_glob_unres *unres)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysf_ctx fctx = {.ctx = ctx};
    struct lys_module *mod;
    uint32_t i;

    for (i = 0; i < dep_set->count; ++i) {
        mod = dep_set->objs[i];
        if (!mod->to_compile) {
            /* skip */
            continue;
        }
        assert(mod->implemented);

        /* free the compiled module, if any */
        lysc_module_free(&fctx, mod->compiled);
        mod->compiled = NULL;

        /* (re)compile the module */
        LY_CHECK_GOTO(ret = lys_compile(mod, &unres->ds_unres), cleanup);
    }

    /* resolve dep set unres */
    ret = lys_compile_unres_depset(ctx, unres);
    if (ret == LY_ERECOMPILE) {
        /* new module is implemented, discard current dep set unres and recompile the whole dep set */
        lys_compile_unres_depset_erase(ctx, unres);
        return lys_compile_depset_r(ctx, dep_set, unres);
    } else if (ret) {
        /* error */
        goto cleanup;
    }

    /* success, unset the flags of all the modules in the dep set */
    for (i = 0; i < dep_set->count; ++i) {
        mod = dep_set->objs[i];
        mod->to_compile = 0;
    }

cleanup:
    lys_compile_unres_depset_erase(ctx, unres);
    lysf_ctx_erase(&fctx);
    return ret;
}

/**
 * @brief Check if-feature of all features of all modules in a dep set.
 *
 * @param[in] dep_set Dep set to check.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_depset_check_features(struct ly_set *dep_set)
{
    struct lys_module *mod;
    uint32_t i;

    for (i = 0; i < dep_set->count; ++i) {
        mod = dep_set->objs[i];
        if (!mod->to_compile) {
            /* skip */
            continue;
        }

        /* check features of this module */
        LY_CHECK_RET(lys_check_features(mod->parsed));
    }

    return LY_SUCCESS;
}

LY_ERR
lys_compile_depset_all(struct ly_ctx *ctx, struct lys_glob_unres *unres)
{
    uint32_t i;

    for (i = 0; i < unres->dep_sets.count; ++i) {
        LY_CHECK_RET(lys_compile_depset_check_features(unres->dep_sets.objs[i]));
        LY_CHECK_RET(lys_compile_depset_r(ctx, unres->dep_sets.objs[i], unres));
    }

    return LY_SUCCESS;
}

/**
 * @brief Finish compilation of all the module unres sets in a compile context.
 *
 * @param[in] ctx Compile context with unres sets.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_unres_mod(struct lysc_ctx *ctx)
{
    struct lysc_augment *aug;
    struct lysc_deviation *dev;
    struct lys_module *orig_mod = ctx->cur_mod;
    uint32_t i;

    /* check that all augments were applied */
    for (i = 0; i < ctx->augs.count; ++i) {
        aug = ctx->augs.objs[i];
        ctx->cur_mod = aug->aug_pmod->mod;
        if (aug->ext) {
            lysc_update_path(ctx, NULL, "{extension}");
            lysc_update_path(ctx, NULL, aug->ext->name);
        }
        lysc_update_path(ctx, NULL, "{augment}");
        lysc_update_path(ctx, NULL, aug->nodeid->expr);
        LOGVAL(ctx->ctx, LYVE_REFERENCE, "Augment%s target node \"%s\" from module \"%s\" was not found.",
                aug->ext ? " extension" : "", aug->nodeid->expr, LYSP_MODULE_NAME(aug->aug_pmod));
        ctx->cur_mod = orig_mod;
        lysc_update_path(ctx, NULL, NULL);
        lysc_update_path(ctx, NULL, NULL);
        if (aug->ext) {
            lysc_update_path(ctx, NULL, NULL);
            lysc_update_path(ctx, NULL, NULL);
        }
    }
    if (ctx->augs.count) {
        return LY_ENOTFOUND;
    }

    /* check that all deviations were applied */
    for (i = 0; i < ctx->devs.count; ++i) {
        dev = ctx->devs.objs[i];
        lysc_update_path(ctx, NULL, "{deviation}");
        lysc_update_path(ctx, NULL, dev->nodeid->expr);
        LOGVAL(ctx->ctx, LYVE_REFERENCE, "Deviation(s) target node \"%s\" from module \"%s\" was not found.",
                dev->nodeid->expr, LYSP_MODULE_NAME(dev->dev_pmods[0]));
        lysc_update_path(ctx, NULL, NULL);
        lysc_update_path(ctx, NULL, NULL);
    }
    if (ctx->devs.count) {
        return LY_ENOTFOUND;
    }

    return LY_SUCCESS;
}

/**
 * @brief Erase all the module unres sets in a compile context.
 *
 * @param[in] ctx Compile context with unres sets.
 * @param[in] error Whether the compilation finished with an error or not.
 */
static void
lys_compile_unres_mod_erase(struct lysc_ctx *ctx, ly_bool error)
{
    uint32_t i;

    ly_set_erase(&ctx->groupings, NULL);
    ly_set_erase(&ctx->tpdf_chain, NULL);

    if (!error) {
        /* there can be no leftover deviations or augments */
        LY_CHECK_ERR_RET(ctx->augs.count, LOGINT(ctx->ctx), );
        LY_CHECK_ERR_RET(ctx->devs.count, LOGINT(ctx->ctx), );

        ly_set_erase(&ctx->augs, NULL);
        ly_set_erase(&ctx->devs, NULL);
        ly_set_erase(&ctx->uses_augs, NULL);
        ly_set_erase(&ctx->uses_rfns, NULL);
    } else {
        for (i = 0; i < ctx->augs.count; ++i) {
            lysc_augment_free(ctx->ctx, ctx->augs.objs[i]);
        }
        ly_set_erase(&ctx->augs, NULL);
        for (i = 0; i < ctx->devs.count; ++i) {
            lysc_deviation_free(ctx->ctx, ctx->devs.objs[i]);
        }
        ly_set_erase(&ctx->devs, NULL);
        for (i = 0; i < ctx->uses_augs.count; ++i) {
            lysc_augment_free(ctx->ctx, ctx->uses_augs.objs[i]);
        }
        ly_set_erase(&ctx->uses_augs, NULL);
        for (i = 0; i < ctx->uses_rfns.count; ++i) {
            lysc_refine_free(ctx->ctx, ctx->uses_rfns.objs[i]);
        }
        ly_set_erase(&ctx->uses_rfns, NULL);
    }
}

LY_ERR
lys_compile(struct lys_module *mod, struct lys_depset_unres *unres)
{
    struct lysc_ctx ctx;
    struct lysc_module *mod_c = NULL;
    struct lysp_module *sp;
    struct lysp_submodule *submod;
    struct lysp_node *pnode;
    struct lysp_node_grp *grp;
    LY_ARRAY_COUNT_TYPE u;
    LY_ERR ret = LY_SUCCESS;

    LY_CHECK_ARG_RET(NULL, mod, mod->parsed, !mod->compiled, mod->ctx, LY_EINVAL);

    assert(mod->implemented && mod->to_compile);

    sp = mod->parsed;
    LYSC_CTX_INIT_PMOD(ctx, sp, NULL);
    ctx.unres = unres;

    ++mod->ctx->change_count;
    mod->compiled = mod_c = calloc(1, sizeof *mod_c);
    LY_CHECK_ERR_RET(!mod_c, LOGMEM(mod->ctx), LY_EMEM);
    mod_c->mod = mod;

    /* compile augments and deviations of our module from other modules so they can be applied during compilation */
    LY_CHECK_GOTO(ret = lys_precompile_own_augments(&ctx), cleanup);
    LY_CHECK_GOTO(ret = lys_precompile_own_deviations(&ctx), cleanup);

    /* data nodes */
    LY_LIST_FOR(sp->data, pnode) {
        LY_CHECK_GOTO(ret = lys_compile_node(&ctx, pnode, NULL, 0, NULL), cleanup);
    }

    /* top-level RPCs */
    LY_LIST_FOR((struct lysp_node *)sp->rpcs, pnode) {
        LY_CHECK_GOTO(ret = lys_compile_node(&ctx, pnode, NULL, 0, NULL), cleanup);
    }

    /* top-level notifications */
    LY_LIST_FOR((struct lysp_node *)sp->notifs, pnode) {
        LY_CHECK_GOTO(ret = lys_compile_node(&ctx, pnode, NULL, 0, NULL), cleanup);
    }

    /* module extension instances */
    COMPILE_EXTS_GOTO(&ctx, sp->exts, mod_c->exts, mod_c, ret, cleanup);

    /* the same for submodules */
    LY_ARRAY_FOR(sp->includes, u) {
        submod = sp->includes[u].submodule;
        ctx.pmod = (struct lysp_module *)submod;

        LY_LIST_FOR(submod->data, pnode) {
            ret = lys_compile_node(&ctx, pnode, NULL, 0, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }

        LY_LIST_FOR((struct lysp_node *)submod->rpcs, pnode) {
            ret = lys_compile_node(&ctx, pnode, NULL, 0, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }

        LY_LIST_FOR((struct lysp_node *)submod->notifs, pnode) {
            ret = lys_compile_node(&ctx, pnode, NULL, 0, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }

        COMPILE_EXTS_GOTO(&ctx, submod->exts, mod_c->exts, mod_c, ret, cleanup);
    }
    ctx.pmod = sp;

    /* validate non-instantiated groupings from the parsed schema,
     * without it we would accept even the schemas with invalid grouping specification */
    ctx.compile_opts |= LYS_COMPILE_GROUPING;
    LY_LIST_FOR(sp->groupings, grp) {
        if (!(grp->flags & LYS_USED_GRP)) {
            LY_CHECK_GOTO(ret = lys_compile_grouping(&ctx, NULL, grp), cleanup);
        }
    }
    LY_LIST_FOR(sp->data, pnode) {
        LY_LIST_FOR((struct lysp_node_grp *)lysp_node_groupings(pnode), grp) {
            if (!(grp->flags & LYS_USED_GRP)) {
                LY_CHECK_GOTO(ret = lys_compile_grouping(&ctx, pnode, grp), cleanup);
            }
        }
    }
    LY_ARRAY_FOR(sp->includes, u) {
        submod = sp->includes[u].submodule;
        ctx.pmod = (struct lysp_module *)submod;

        LY_LIST_FOR(submod->groupings, grp) {
            if (!(grp->flags & LYS_USED_GRP)) {
                LY_CHECK_GOTO(ret = lys_compile_grouping(&ctx, NULL, grp), cleanup);
            }
        }
        LY_LIST_FOR(submod->data, pnode) {
            LY_LIST_FOR((struct lysp_node_grp *)lysp_node_groupings(pnode), grp) {
                if (!(grp->flags & LYS_USED_GRP)) {
                    LY_CHECK_GOTO(ret = lys_compile_grouping(&ctx, pnode, grp), cleanup);
                }
            }
        }
    }
    ctx.pmod = sp;

    LOG_LOCBACK(0, 0, 1, 0);

    /* finish compilation for all unresolved module items in the context */
    LY_CHECK_GOTO(ret = lys_compile_unres_mod(&ctx), cleanup);

cleanup:
    LOG_LOCBACK(0, 0, 1, 0);
    lys_compile_unres_mod_erase(&ctx, ret);
    if (ret) {
        lysc_module_free(&ctx.free_ctx, mod_c);
        mod->compiled = NULL;
    }
    return ret;
}

LY_ERR
lys_compile_identities(struct lys_module *mod)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysc_ctx ctx;
    struct lysp_submodule *submod;
    LY_ARRAY_COUNT_TYPE u;

    /* pre-compile identities of the module and any submodules */
    rc = lys_identity_precompile(NULL, mod->ctx, mod->parsed, mod->parsed->identities, &mod->identities);
    LY_CHECK_GOTO(rc, cleanup);
    LY_ARRAY_FOR(mod->parsed->includes, u) {
        submod = mod->parsed->includes[u].submodule;
        rc = lys_identity_precompile(NULL, mod->ctx, (struct lysp_module *)submod, submod->identities, &mod->identities);
        LY_CHECK_GOTO(rc, cleanup);
    }

    /* prepare context */
    LYSC_CTX_INIT_PMOD(ctx, mod->parsed, NULL);

    if (mod->parsed->identities) {
        rc = lys_compile_identities_derived(&ctx, mod->parsed->identities, &mod->identities);
        LY_CHECK_GOTO(rc, cleanup);
    }
    lysc_update_path(&ctx, NULL, "{submodule}");
    LY_ARRAY_FOR(mod->parsed->includes, u) {
        submod = mod->parsed->includes[u].submodule;
        if (submod->identities) {
            ctx.pmod = (struct lysp_module *)submod;
            lysc_update_path(&ctx, NULL, submod->name);
            rc = lys_compile_identities_derived(&ctx, submod->identities, &mod->identities);
            lysc_update_path(&ctx, NULL, NULL);
        }

        if (rc) {
            break;
        }
    }
    lysc_update_path(&ctx, NULL, NULL);

cleanup:
    /* always needed when using lysc_update_path() */
    LOG_LOCBACK(0, 0, 1, 0);
    return rc;
}

/**
 * @brief Check whether a module does not have any (recursive) compiled import.
 *
 * @param[in] mod Module to examine.
 * @return LY_SUCCESS on success.
 * @return LY_ERECOMPILE on required recompilation of the dep set.
 * @return LY_ERR on error.
 */
static LY_ERR
lys_has_compiled_import_r(struct lys_module *mod)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lys_module *m;

    LY_ARRAY_FOR(mod->parsed->imports, u) {
        m = mod->parsed->imports[u].module;
        if (!m->implemented) {
            continue;
        }

        if (!m->to_compile) {
            /* module was not/will not be compiled in this compilation (so disabled nodes are not present) */
            m->to_compile = 1;
            return LY_ERECOMPILE;
        }

        /* recursive */
        LY_CHECK_RET(lys_has_compiled_import_r(m));
    }

    return LY_SUCCESS;
}

LY_ERR
lys_implement(struct lys_module *mod, const char **features, struct lys_glob_unres *unres)
{
    LY_ERR r;
    struct lys_module *m;

    assert(!mod->implemented);

    /* check collision with other implemented revision */
    m = ly_ctx_get_module_implemented(mod->ctx, mod->name);
    if (m) {
        assert(m != mod);
        LOGERR(mod->ctx, LY_EDENIED, "Module \"%s@%s\" is already implemented in revision \"%s\".",
                mod->name, mod->revision ? mod->revision : "<none>", m->revision ? m->revision : "<none>");
        return LY_EDENIED;
    }

    /* set features */
    r = lys_set_features(mod->parsed, features);
    if (r && (r != LY_EEXIST)) {
        return r;
    }

    /*
     * mark the module implemented, which means
     * 1) to (re)compile it only ::lys_compile() call is needed
     * 2) its compilation will never cause new modules to be implemented (::lys_compile() does not return ::LY_ERECOMPILE)
     *    but there can be some unres items added that do
     */
    mod->implemented = 1;

    /* this module is compiled in this compilation */
    mod->to_compile = 1;

    /* add the module into newly implemented module set */
    LY_CHECK_RET(ly_set_add(&unres->implementing, mod, 1, NULL));

    /* mark target modules with our augments and deviations */
    LY_CHECK_RET(lys_precompile_augments_deviations(mod, unres));

    /* check whether this module may reference any modules compiled previously */
    LY_CHECK_RET(lys_has_compiled_import_r(mod));

    return LY_SUCCESS;
}
