/**
 * @file schema_compile_node.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Schema compilation of common nodes.
 *
 * Copyright (c) 2015 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* asprintf, strdup */

#include "schema_compile_node.h"

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "dict.h"
#include "log.h"
#include "ly_common.h"
#include "plugins.h"
#include "plugins_internal.h"
#include "plugins_types.h"
#include "schema_compile.h"
#include "schema_compile_amend.h"
#include "schema_features.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xpath.h"

/**
 * @brief Item for storing typedef chain item.
 */
struct lys_type_item {
    const struct lysp_tpdf *tpdf;
    struct lysp_node *node;
};

/**
 * @brief Add a node with a when to unres.
 *
 * @param[in] ctx Compile context.
 * @param[in] when Specific compiled when to check.
 * @param[in] node Compiled node with when(s).
 * @return LY_ERR value.
 */
static LY_ERR
lysc_unres_when_add(struct lysc_ctx *ctx, struct lysc_when *when, struct lysc_node *node)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysc_unres_when *w = NULL;

    /* do not check must(s) in a grouping */
    if (ctx->compile_opts & LYS_COMPILE_GROUPING) {
        goto cleanup;
    }

    /* add new unres when */
    w = calloc(1, sizeof *w);
    LY_CHECK_ERR_GOTO(!w, LOGMEM(ctx->ctx); rc = LY_EMEM, cleanup);

    w->node = node;
    w->when = when;

    /* add into the unres set */
    LY_CHECK_ERR_GOTO(ly_set_add(&ctx->unres->whens, w, 1, NULL), LOGMEM(ctx->ctx); rc = LY_EMEM, cleanup);
    w = NULL;

cleanup:
    free(w);
    return rc;
}

/**
 * @brief Add a node with must(s) to unres.
 *
 * @param[in] ctx Compile context.
 * @param[in] node Compiled node with must(s).
 * @param[in] pnode Parsed ndoe with must(s).
 * @return LY_ERR value.
 */
static LY_ERR
lysc_unres_must_add(struct lysc_ctx *ctx, struct lysc_node *node, struct lysp_node *pnode)
{
    struct lysc_unres_must *m = NULL;
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_must *musts;
    struct lysp_restr *pmusts;
    LY_ERR ret;

    /* do not check must(s) in a grouping */
    if (ctx->compile_opts & LYS_COMPILE_GROUPING) {
        return LY_SUCCESS;
    }

    musts = lysc_node_musts(node);
    pmusts = lysp_node_musts(pnode);
    assert(LY_ARRAY_COUNT(musts) == LY_ARRAY_COUNT(pmusts));

    if (!musts) {
        /* no must */
        return LY_SUCCESS;
    }

    /* add new unres must */
    m = calloc(1, sizeof *m);
    LY_CHECK_ERR_GOTO(!m, ret = LY_EMEM, error);
    m->node = node;

    /* add must local modules */
    LY_ARRAY_CREATE_GOTO(ctx->ctx, m->local_mods, LY_ARRAY_COUNT(pmusts), ret, error);
    LY_ARRAY_FOR(pmusts, u) {
        m->local_mods[u] = pmusts[u].arg.mod;
        LY_ARRAY_INCREMENT(m->local_mods);
    }

    /* store ext */
    m->ext = ctx->ext;

    LY_CHECK_ERR_GOTO(ly_set_add(&ctx->unres->musts, m, 1, NULL), ret = LY_EMEM, error);

    return LY_SUCCESS;

error:
    if (m) {
        LY_ARRAY_FREE(m->local_mods);
        free(m);
    }
    LOGMEM(ctx->ctx);
    return ret;
}

static LY_ERR
lysc_unres_leafref_add(struct lysc_ctx *ctx, struct lysc_node_leaf *leaf, const struct lysp_module *local_mod)
{
    struct lysc_unres_leafref *l = NULL;
    struct ly_set *leafrefs_set;
    LY_ARRAY_COUNT_TYPE u;
    int is_lref = 0;

    if (ctx->compile_opts & LYS_COMPILE_GROUPING) {
        /* do not check leafrefs in groupings */
        return LY_SUCCESS;
    }

    /* use special set for disabled leafrefs */
    leafrefs_set = ctx->compile_opts & LYS_COMPILE_DISABLED ? &ctx->unres->disabled_leafrefs : &ctx->unres->leafrefs;

    if (leaf->type->basetype == LY_TYPE_LEAFREF) {
        /* leafref */
        is_lref = 1;
    } else if (leaf->type->basetype == LY_TYPE_UNION) {
        /* union with leafrefs */
        LY_ARRAY_FOR(((struct lysc_type_union *)leaf->type)->types, u) {
            if (((struct lysc_type_union *)leaf->type)->types[u]->basetype == LY_TYPE_LEAFREF) {
                is_lref = 1;
                break;
            }
        }
    }

    if (is_lref) {
        /* add new unresolved leafref node */
        l = calloc(1, sizeof *l);
        LY_CHECK_ERR_RET(!l, LOGMEM(ctx->ctx), LY_EMEM);

        l->node = &leaf->node;
        l->local_mod = local_mod;
        l->ext = ctx->ext;

        LY_CHECK_ERR_RET(ly_set_add(leafrefs_set, l, 1, NULL), free(l); LOGMEM(ctx->ctx), LY_EMEM);
    }

    return LY_SUCCESS;
}

/**
 * @brief Add/replace a leaf default value in unres.
 * Can also be used for a single leaf-list default value.
 *
 * @param[in] ctx Compile context.
 * @param[in] leaf Leaf with the default value.
 * @param[in] dflt Default value to use.
 * @return LY_ERR value.
 */
static LY_ERR
lysc_unres_leaf_dflt_add(struct lysc_ctx *ctx, struct lysc_node_leaf *leaf, struct lysp_qname *dflt)
{
    struct lysc_unres_dflt *r = NULL;
    uint32_t i;

    if (ctx->compile_opts & (LYS_COMPILE_DISABLED | LYS_COMPILE_GROUPING)) {
        return LY_SUCCESS;
    }

    for (i = 0; i < ctx->unres->dflts.count; ++i) {
        if (((struct lysc_unres_dflt *)ctx->unres->dflts.objs[i])->leaf == leaf) {
            /* just replace the default */
            r = ctx->unres->dflts.objs[i];
            lysp_qname_free(ctx->ctx, r->dflt);
            free(r->dflt);
            break;
        }
    }
    if (!r) {
        /* add new unres item */
        r = calloc(1, sizeof *r);
        LY_CHECK_ERR_RET(!r, LOGMEM(ctx->ctx), LY_EMEM);
        r->leaf = leaf;

        LY_CHECK_RET(ly_set_add(&ctx->unres->dflts, r, 1, NULL));
    }

    r->dflt = malloc(sizeof *r->dflt);
    LY_CHECK_GOTO(!r->dflt, error);
    LY_CHECK_GOTO(lysp_qname_dup(ctx->ctx, dflt, r->dflt), error);

    return LY_SUCCESS;

error:
    free(r->dflt);
    LOGMEM(ctx->ctx);
    return LY_EMEM;
}

/**
 * @brief Add/replace a leaf-list default value(s) in unres.
 *
 * @param[in] ctx Compile context.
 * @param[in] llist Leaf-list with the default value.
 * @param[in] dflts Sized array of the default values.
 * @return LY_ERR value.
 */
static LY_ERR
lysc_unres_llist_dflts_add(struct lysc_ctx *ctx, struct lysc_node_leaflist *llist, struct lysp_qname *dflts)
{
    struct lysc_unres_dflt *r = NULL;
    uint32_t i;

    if (ctx->compile_opts & (LYS_COMPILE_DISABLED | LYS_COMPILE_GROUPING)) {
        return LY_SUCCESS;
    }

    for (i = 0; i < ctx->unres->dflts.count; ++i) {
        if (((struct lysc_unres_dflt *)ctx->unres->dflts.objs[i])->llist == llist) {
            /* just replace the defaults */
            r = ctx->unres->dflts.objs[i];
            lysp_qname_free(ctx->ctx, r->dflt);
            free(r->dflt);
            r->dflt = NULL;
            FREE_ARRAY(ctx->ctx, r->dflts, lysp_qname_free);
            r->dflts = NULL;
            break;
        }
    }
    if (!r) {
        r = calloc(1, sizeof *r);
        LY_CHECK_ERR_RET(!r, LOGMEM(ctx->ctx), LY_EMEM);
        r->llist = llist;

        LY_CHECK_RET(ly_set_add(&ctx->unres->dflts, r, 1, NULL));
    }

    DUP_ARRAY(ctx->ctx, dflts, r->dflts, lysp_qname_dup);

    return LY_SUCCESS;
}

/**
 * @brief Add a bits/enumeration type to unres.
 *
 * @param[in] ctx Compile context.
 * @param[in] leaf Leaf of type bits/enumeration whose disabled items to free.
 * @return LY_ERR value.
 */
static LY_ERR
lysc_unres_bitenum_add(struct lysc_ctx *ctx, struct lysc_node_leaf *leaf)
{
    if (ctx->compile_opts & (LYS_COMPILE_DISABLED | LYS_COMPILE_GROUPING)) {
        /* skip groupings and redundant for disabled nodes */
        return LY_SUCCESS;
    }

    LY_CHECK_RET(ly_set_add(&ctx->unres->disabled_bitenums, leaf, 1, NULL));

    return LY_SUCCESS;
}

/**
 * @brief Duplicate the compiled pattern structure.
 *
 * Instead of duplicating memory, the reference counter in the @p orig is increased.
 *
 * @param[in] orig The pattern structure to duplicate.
 * @return The duplicated structure to use.
 */
static struct lysc_pattern *
lysc_pattern_dup(struct lysc_pattern *orig)
{
    ++orig->refcount;
    return orig;
}

/**
 * @brief Duplicate the array of compiled patterns.
 *
 * The sized array itself is duplicated, but the pattern structures are just shadowed by increasing their reference counter.
 *
 * @param[in] ctx Libyang context for logging.
 * @param[in] orig The patterns sized array to duplicate.
 * @return New sized array as a copy of @p orig.
 * @return NULL in case of memory allocation error.
 */
static struct lysc_pattern **
lysc_patterns_dup(struct ly_ctx *ctx, struct lysc_pattern **orig)
{
    struct lysc_pattern **dup = NULL;
    LY_ARRAY_COUNT_TYPE u;

    assert(orig);

    LY_ARRAY_CREATE_RET(ctx, dup, LY_ARRAY_COUNT(orig), NULL);
    LY_ARRAY_FOR(orig, u) {
        dup[u] = lysc_pattern_dup(orig[u]);
        LY_ARRAY_INCREMENT(dup);
    }
    return dup;
}

/**
 * @brief Duplicate compiled range structure.
 *
 * @param[in] ctx Compile context.
 * @param[in] orig The range structure to be duplicated.
 * @param[in] tpdf_chain Chain of the used typedefs, traversed backwards.
 * @param[in] tpdf_chain_last Index of the last (backwards) typedef in @p tpdf_chain to use.
 * @return New compiled range structure as a copy of @p orig.
 * @return NULL in case of memory allocation error.
 */
static struct lysc_range *
lysc_range_dup(struct lysc_ctx *ctx, const struct lysc_range *orig, struct ly_set *tpdf_chain, uint32_t tpdf_chain_last)
{
    struct lysc_range *dup;
    LY_ERR ret;
    struct lys_type_item *tpdf_item;
    uint32_t i;

    assert(orig);

    dup = calloc(1, sizeof *dup);
    LY_CHECK_ERR_RET(!dup, LOGMEM(ctx->ctx), NULL);
    if (orig->parts) {
        LY_ARRAY_CREATE_GOTO(ctx->ctx, dup->parts, LY_ARRAY_COUNT(orig->parts), ret, cleanup);
        (*((LY_ARRAY_COUNT_TYPE *)(dup->parts) - 1)) = LY_ARRAY_COUNT(orig->parts);
        memcpy(dup->parts, orig->parts, LY_ARRAY_COUNT(dup->parts) * sizeof *dup->parts);
    }
    DUP_STRING_GOTO(ctx->ctx, orig->eapptag, dup->eapptag, ret, cleanup);
    DUP_STRING_GOTO(ctx->ctx, orig->emsg, dup->emsg, ret, cleanup);

    /* collect all range extensions */
    if (tpdf_chain->count > tpdf_chain_last) {
        i = tpdf_chain->count;
        do {
            --i;
            tpdf_item = tpdf_chain->objs[i];
            if (!tpdf_item->tpdf->type.range) {
                continue;
            }
            COMPILE_EXTS_GOTO(ctx, tpdf_item->tpdf->type.range->exts, dup->exts, dup, ret, cleanup);
        } while (i > tpdf_chain_last);
    }

    return dup;

cleanup:
    free(dup);
    (void) ret; /* set but not used due to the return type */
    return NULL;
}

/**
 * @brief Print status into a string.
 *
 * @param[in] flags Flags with the status to print.
 * @return String status.
 */
static const char *
lys_status2str(uint16_t flags)
{
    flags &= LYS_STATUS_MASK;

    switch (flags) {
    case 0:
    case LYS_STATUS_CURR:
        return "current";
    case LYS_STATUS_DEPRC:
        return "deprecated";
    case LYS_STATUS_OBSLT:
        return "obsolete";
    default:
        LOGINT(NULL);
        return NULL;
    }
}

/**
 * @brief Compile status information of the given statement.
 *
 * To simplify getting status of the node, the flags are set following inheritance rules, so all the nodes
 * has the status correctly set during the compilation.
 *
 * @param[in] ctx Compile context
 * @param[in] parsed_flags Parsed statement flags.
 * @param[in] inherited_flags Parsed inherited flags from a schema-only statement (augment, uses, ext instance, ...).
 * @param[in] parent_flags Compiled parent node flags.
 * @param[in] parent_name Name of the parent node, for logging.
 * @param[in] stmt_name Statement name, for logging.
 * @param[in,out] stmt_flags Statement flags with the correct status set.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_status(struct lysc_ctx *ctx, uint16_t parsed_flags, uint16_t inherited_flags, uint16_t parent_flags,
        const char *parent_name, const char *stmt_name, uint16_t *stmt_flags)
{
    /* normalize to status-only */
    parsed_flags &= LYS_STATUS_MASK;
    inherited_flags &= LYS_STATUS_MASK;
    parent_flags &= LYS_STATUS_MASK;

    /* check for conflicts */
    if (parent_flags && parsed_flags && (parent_flags > parsed_flags)) {
        LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Status \"%s\" of \"%s\" is in conflict with \"%s\" status of parent \"%s\".",
                lys_status2str(parsed_flags), stmt_name, lys_status2str(parent_flags), parent_name);
        return LY_EVALID;
    } else if (inherited_flags && parsed_flags && (inherited_flags > parsed_flags)) {
        LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Inherited schema-only status \"%s\" is in conflict with \"%s\" status of \"%s\".",
                lys_status2str(inherited_flags), lys_status2str(parsed_flags), stmt_name);
        return LY_EVALID;
    } else if (parent_flags && inherited_flags && (parent_flags > inherited_flags)) {
        LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Status \"%s\" of parent \"%s\" is in conflict with inherited schema-only status \"%s\".",
                lys_status2str(parent_flags), parent_name, lys_status2str(inherited_flags));
        return LY_EVALID;
    }

    /* clear */
    (*stmt_flags) &= ~LYS_STATUS_MASK;

    if (parsed_flags) {
        /* explicit status */
        (*stmt_flags) |= parsed_flags;
    } else if (inherited_flags) {
        /* inherited status from a schema-only statement */
        (*stmt_flags) |= inherited_flags;
    } else if (parent_flags) {
        /* inherited status from a parent node */
        (*stmt_flags) |= parent_flags;
    } else {
        /* default status */
        (*stmt_flags) |= LYS_STATUS_CURR;
    }

    return LY_SUCCESS;
}

/**
 * @brief Compile information from the when statement
 *
 * @param[in] ctx Compile context.
 * @param[in] when_p Parsed when structure.
 * @param[in] inherited_flags Inherited flags from a schema-only statement.
 * @param[in] parent Parent node, if any.
 * @param[in] ctx_node Context node for the when statement.
 * @param[out] when Pointer where to store pointer to the created compiled when structure.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_when_(struct lysc_ctx *ctx, const struct lysp_when *when_p, uint16_t inherited_flags,
        const struct lysc_node *parent, const struct lysc_node *ctx_node, struct lysc_when **when)
{
    LY_ERR ret = LY_SUCCESS;
    LY_VALUE_FORMAT format;

    *when = calloc(1, sizeof **when);
    LY_CHECK_ERR_RET(!(*when), LOGMEM(ctx->ctx), LY_EMEM);
    (*when)->refcount = 1;
    LY_CHECK_RET(lyxp_expr_parse(ctx->ctx, when_p->cond, 0, 1, &(*when)->cond));
    LY_CHECK_RET(lyplg_type_prefix_data_new(ctx->ctx, when_p->cond, strlen(when_p->cond),
            LY_VALUE_SCHEMA, ctx->pmod, &format, (void **)&(*when)->prefixes));
    (*when)->context = (struct lysc_node *)ctx_node;
    DUP_STRING_GOTO(ctx->ctx, when_p->dsc, (*when)->dsc, ret, done);
    DUP_STRING_GOTO(ctx->ctx, when_p->ref, (*when)->ref, ret, done);
    COMPILE_EXTS_GOTO(ctx, when_p->exts, (*when)->exts, (*when), ret, done);
    LY_CHECK_RET(lys_compile_status(ctx, 0, inherited_flags, parent ? parent->flags : 0, parent ? parent->name : NULL,
            "when", &(*when)->flags));

done:
    return ret;
}

LY_ERR
lys_compile_when(struct lysc_ctx *ctx, const struct lysp_when *when_p, uint16_t inherited_flags, const struct lysc_node *parent,
        const struct lysc_node *ctx_node, struct lysc_node *node, struct lysc_when **when_c)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysc_when **new_when, ***node_when, *ptr;

    assert(when_p && (node || when_c));

    if (node) {
        /* get the when array */
        node_when = lysc_node_when_p(node);

        /* create new when pointer */
        LY_ARRAY_NEW_GOTO(ctx->ctx, *node_when, new_when, rc, cleanup);
    } else {
        /* individual when */
        new_when = &ptr;
        *new_when = calloc(1, sizeof **new_when);
        LY_CHECK_ERR_GOTO(!*new_when, LOGMEM(ctx->ctx); rc = LY_EMEM, cleanup);
    }

    if (!when_c || !(*when_c)) {
        /* compile when */
        LY_CHECK_GOTO(rc = lys_compile_when_(ctx, when_p, inherited_flags, parent, ctx_node, new_when), cleanup);

        /* remember the compiled when for sharing */
        if (when_c) {
            *when_c = *new_when;
        }
    } else {
        /* use the previously compiled when */
        ++(*when_c)->refcount;
        *new_when = *when_c;
    }

    if (node) {
        /* add when to unres if there is a node for evaluation (not for extension instances) */
        LY_CHECK_GOTO(rc = lysc_unres_when_add(ctx, *new_when, node), cleanup);
    }

cleanup:
    return rc;
}

LY_ERR
lys_compile_must(struct lysc_ctx *ctx, const struct lysp_restr *must_p, struct lysc_must *must)
{
    LY_ERR ret = LY_SUCCESS;
    LY_VALUE_FORMAT format;

    LY_CHECK_RET(lyxp_expr_parse(ctx->ctx, must_p->arg.str, 0, 1, &must->cond));
    LY_CHECK_RET(lyplg_type_prefix_data_new(ctx->ctx, must_p->arg.str, strlen(must_p->arg.str),
            LY_VALUE_SCHEMA, must_p->arg.mod, &format, (void **)&must->prefixes));
    DUP_STRING_GOTO(ctx->ctx, must_p->eapptag, must->eapptag, ret, done);
    DUP_STRING_GOTO(ctx->ctx, must_p->emsg, must->emsg, ret, done);
    DUP_STRING_GOTO(ctx->ctx, must_p->dsc, must->dsc, ret, done);
    DUP_STRING_GOTO(ctx->ctx, must_p->ref, must->ref, ret, done);
    COMPILE_EXTS_GOTO(ctx, must_p->exts, must->exts, must, ret, done);

done:
    return ret;
}

/**
 * @brief Validate and normalize numeric value from a range definition.
 * @param[in] ctx Compile context.
 * @param[in] basetype Base YANG built-in type of the node connected with the range restriction. Actually only LY_TYPE_DEC64 is important to
 * allow processing of the fractions. The fraction point is extracted from the value which is then normalize according to given frdigits into
 * valcopy to allow easy parsing and storing of the value. libyang stores decimal number without the decimal point which is always recovered from
 * the known fraction-digits value. So, with fraction-digits 2, number 3.14 is stored as 314 and number 1 is stored as 100.
 * @param[in] frdigits The fraction-digits of the type in case of LY_TYPE_DEC64.
 * @param[in] value String value of the range boundary.
 * @param[out] len Number of the processed bytes from the value. Processing stops on the first character which is not part of the number boundary.
 * @param[out] valcopy NULL-terminated string with the numeric value to parse and store.
 * @return LY_ERR value - LY_SUCCESS, LY_EMEM, LY_EVALID (no number) or LY_EINVAL (decimal64 not matching fraction-digits value).
 */
static LY_ERR
range_part_check_value_syntax(struct lysc_ctx *ctx, LY_DATA_TYPE basetype, uint8_t frdigits, const char *value,
        size_t *len, char **valcopy)
{
    size_t fraction = 0, size;

    *len = 0;

    assert(value);
    /* parse value */
    if (!isdigit(value[*len]) && (value[*len] != '-') && (value[*len] != '+')) {
        return LY_EVALID;
    }

    if ((value[*len] == '-') || (value[*len] == '+')) {
        ++(*len);
    }

    while (isdigit(value[*len])) {
        ++(*len);
    }

    if ((basetype != LY_TYPE_DEC64) || (value[*len] != '.') || !isdigit(value[*len + 1])) {
        if (basetype == LY_TYPE_DEC64) {
            goto decimal;
        } else {
            *valcopy = strndup(value, *len);
            return LY_SUCCESS;
        }
    }
    fraction = *len;

    ++(*len);
    while (isdigit(value[*len])) {
        ++(*len);
    }

    if (basetype == LY_TYPE_DEC64) {
decimal:
        assert(frdigits);
        if (fraction && (*len - 1 - fraction > frdigits)) {
            LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                    "Range boundary \"%.*s\" of decimal64 type exceeds defined number (%u) of fraction digits.",
                    (int)(*len), value, frdigits);
            return LY_EINVAL;
        }
        if (fraction) {
            size = (*len) + (frdigits - ((*len) - 1 - fraction));
        } else {
            size = (*len) + frdigits + 1;
        }
        *valcopy = malloc(size * sizeof **valcopy);
        LY_CHECK_ERR_RET(!(*valcopy), LOGMEM(ctx->ctx), LY_EMEM);

        (*valcopy)[size - 1] = '\0';
        if (fraction) {
            memcpy(&(*valcopy)[0], &value[0], fraction);
            memcpy(&(*valcopy)[fraction], &value[fraction + 1], (*len) - 1 - (fraction));
            memset(&(*valcopy)[(*len) - 1], '0', frdigits - ((*len) - 1 - fraction));
        } else {
            memcpy(&(*valcopy)[0], &value[0], *len);
            memset(&(*valcopy)[*len], '0', frdigits);
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Check that values in range are in ascendant order.
 * @param[in] unsigned_value Flag to note that we are working with unsigned values.
 * @param[in] max Flag to distinguish if checking min or max value. min value must be strictly higher than previous,
 * max can be also equal.
 * @param[in] value Current value to check.
 * @param[in] prev_value The last seen value.
 * @return LY_SUCCESS or LY_EEXIST for invalid order.
 */
static LY_ERR
range_part_check_ascendancy(ly_bool unsigned_value, ly_bool max, int64_t value, int64_t prev_value)
{
    if (unsigned_value) {
        if ((max && ((uint64_t)prev_value > (uint64_t)value)) || (!max && ((uint64_t)prev_value >= (uint64_t)value))) {
            return LY_EEXIST;
        }
    } else {
        if ((max && (prev_value > value)) || (!max && (prev_value >= value))) {
            return LY_EEXIST;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Set min/max value of the range part.
 * @param[in] ctx Compile context.
 * @param[in] part Range part structure to fill.
 * @param[in] max Flag to distinguish if storing min or max value.
 * @param[in] prev The last seen value to check that all values in range are specified in ascendant order.
 * @param[in] basetype Type of the value to get know implicit min/max values and other checking rules.
 * @param[in] first Flag for the first value of the range to avoid ascendancy order.
 * @param[in] length_restr Flag to distinguish between range and length restrictions. Only for logging.
 * @param[in] frdigits The fraction-digits value in case of LY_TYPE_DEC64 basetype.
 * @param[in] base_range Range from the type from which the current type is derived (if not built-in) to get type's min and max values.
 * @param[in,out] value Numeric range value to be stored, if not provided the type's min/max value is set.
 * @return LY_ERR value - LY_SUCCESS, LY_EDENIED (value brokes type's boundaries), LY_EVALID (not a number),
 * LY_EEXIST (value is smaller than the previous one), LY_EINVAL (decimal64 value does not corresponds with the
 * frdigits value), LY_EMEM.
 */
static LY_ERR
range_part_minmax(struct lysc_ctx *ctx, struct lysc_range_part *part, ly_bool max, int64_t prev, LY_DATA_TYPE basetype,
        ly_bool first, ly_bool length_restr, uint8_t frdigits, struct lysc_range *base_range, const char **value)
{
    LY_ERR ret = LY_SUCCESS;
    char *valcopy = NULL;
    size_t len = 0;

    if (value) {
        ret = range_part_check_value_syntax(ctx, basetype, frdigits, *value, &len, &valcopy);
        LY_CHECK_GOTO(ret, finalize);
    }
    if (!valcopy && base_range) {
        if (max) {
            part->max_64 = base_range->parts[LY_ARRAY_COUNT(base_range->parts) - 1].max_64;
        } else {
            part->min_64 = base_range->parts[0].min_64;
        }
        if (!first) {
            ret = range_part_check_ascendancy(basetype <= LY_TYPE_STRING ? 1 : 0, max, max ? part->max_64 : part->min_64, prev);
        }
        goto finalize;
    }

    switch (basetype) {
    case LY_TYPE_INT8: /* range */
        if (valcopy) {
            ret = ly_parse_int(valcopy, strlen(valcopy), INT64_C(-128), INT64_C(127), LY_BASE_DEC, max ? &part->max_64 : &part->min_64);
        } else if (max) {
            part->max_64 = INT64_C(127);
        } else {
            part->min_64 = INT64_C(-128);
        }
        if (!ret && !first) {
            ret = range_part_check_ascendancy(0, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_INT16: /* range */
        if (valcopy) {
            ret = ly_parse_int(valcopy, strlen(valcopy), INT64_C(-32768), INT64_C(32767), LY_BASE_DEC,
                    max ? &part->max_64 : &part->min_64);
        } else if (max) {
            part->max_64 = INT64_C(32767);
        } else {
            part->min_64 = INT64_C(-32768);
        }
        if (!ret && !first) {
            ret = range_part_check_ascendancy(0, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_INT32: /* range */
        if (valcopy) {
            ret = ly_parse_int(valcopy, strlen(valcopy), INT64_C(-2147483648), INT64_C(2147483647), LY_BASE_DEC,
                    max ? &part->max_64 : &part->min_64);
        } else if (max) {
            part->max_64 = INT64_C(2147483647);
        } else {
            part->min_64 = INT64_C(-2147483648);
        }
        if (!ret && !first) {
            ret = range_part_check_ascendancy(0, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_INT64: /* range */
    case LY_TYPE_DEC64: /* range */
        if (valcopy) {
            ret = ly_parse_int(valcopy, strlen(valcopy), INT64_C(-9223372036854775807) - INT64_C(1), INT64_C(9223372036854775807),
                    LY_BASE_DEC, max ? &part->max_64 : &part->min_64);
        } else if (max) {
            part->max_64 = INT64_C(9223372036854775807);
        } else {
            part->min_64 = INT64_C(-9223372036854775807) - INT64_C(1);
        }
        if (!ret && !first) {
            ret = range_part_check_ascendancy(0, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_UINT8: /* range */
        if (valcopy) {
            ret = ly_parse_uint(valcopy, strlen(valcopy), UINT64_C(255), LY_BASE_DEC, max ? &part->max_u64 : &part->min_u64);
        } else if (max) {
            part->max_u64 = UINT64_C(255);
        } else {
            part->min_u64 = UINT64_C(0);
        }
        if (!ret && !first) {
            ret = range_part_check_ascendancy(1, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_UINT16: /* range */
        if (valcopy) {
            ret = ly_parse_uint(valcopy, strlen(valcopy), UINT64_C(65535), LY_BASE_DEC, max ? &part->max_u64 : &part->min_u64);
        } else if (max) {
            part->max_u64 = UINT64_C(65535);
        } else {
            part->min_u64 = UINT64_C(0);
        }
        if (!ret && !first) {
            ret = range_part_check_ascendancy(1, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_UINT32: /* range */
        if (valcopy) {
            ret = ly_parse_uint(valcopy, strlen(valcopy), UINT64_C(4294967295), LY_BASE_DEC,
                    max ? &part->max_u64 : &part->min_u64);
        } else if (max) {
            part->max_u64 = UINT64_C(4294967295);
        } else {
            part->min_u64 = UINT64_C(0);
        }
        if (!ret && !first) {
            ret = range_part_check_ascendancy(1, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    case LY_TYPE_UINT64: /* range */
    case LY_TYPE_STRING: /* length */
    case LY_TYPE_BINARY: /* length */
        if (valcopy) {
            ret = ly_parse_uint(valcopy, strlen(valcopy), UINT64_C(18446744073709551615), LY_BASE_DEC,
                    max ? &part->max_u64 : &part->min_u64);
        } else if (max) {
            part->max_u64 = UINT64_C(18446744073709551615);
        } else {
            part->min_u64 = UINT64_C(0);
        }
        if (!ret && !first) {
            ret = range_part_check_ascendancy(1, max, max ? part->max_64 : part->min_64, prev);
        }
        break;
    default:
        LOGINT(ctx->ctx);
        ret = LY_EINT;
    }

finalize:
    if (ret == LY_EDENIED) {
        LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                "Invalid %s restriction - value \"%s\" does not fit the type limitations.",
                length_restr ? "length" : "range", valcopy ? valcopy : *value);
    } else if (ret == LY_EVALID) {
        LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                "Invalid %s restriction - invalid value \"%s\".",
                length_restr ? "length" : "range", valcopy ? valcopy : *value);
    } else if (ret == LY_EEXIST) {
        LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                "Invalid %s restriction - values are not in ascending order (%s).",
                length_restr ? "length" : "range",
                (valcopy && basetype != LY_TYPE_DEC64) ? valcopy : value ? *value : max ? "max" : "min");
    } else if (!ret && value) {
        *value = *value + len;
    }
    free(valcopy);
    return ret;
}

LY_ERR
lys_compile_type_range(struct lysc_ctx *ctx, const struct lysp_restr *range_p, LY_DATA_TYPE basetype, ly_bool length_restr,
        uint8_t frdigits, struct lysc_range *base_range, struct lysc_range **range)
{
    LY_ERR ret = LY_SUCCESS;
    const char *expr;
    struct lysc_range_part *parts = NULL, *part;
    ly_bool range_expected = 0, uns;
    LY_ARRAY_COUNT_TYPE parts_done = 0, u, v;

    assert(range);
    assert(range_p);

    expr = range_p->arg.str;
    while (1) {
        if (isspace(*expr)) {
            ++expr;
        } else if (*expr == '\0') {
            if (range_expected) {
                LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                        "Invalid %s restriction - unexpected end of the expression after \"..\" (%s).",
                        length_restr ? "length" : "range", range_p->arg.str);
                ret = LY_EVALID;
                goto cleanup;
            } else if (!parts || (parts_done == LY_ARRAY_COUNT(parts))) {
                LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                        "Invalid %s restriction - unexpected end of the expression (%s).",
                        length_restr ? "length" : "range", range_p->arg.str);
                ret = LY_EVALID;
                goto cleanup;
            }
            parts_done++;
            break;
        } else if (!strncmp(expr, "min", ly_strlen_const("min"))) {
            if (parts) {
                /* min cannot be used elsewhere than in the first part */
                LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                        "Invalid %s restriction - unexpected data before min keyword (%.*s).", length_restr ? "length" : "range",
                        (int)(expr - range_p->arg.str), range_p->arg.str);
                ret = LY_EVALID;
                goto cleanup;
            }
            expr += ly_strlen_const("min");

            LY_ARRAY_NEW_GOTO(ctx->ctx, parts, part, ret, cleanup);
            LY_CHECK_GOTO(ret = range_part_minmax(ctx, part, 0, 0, basetype, 1, length_restr, frdigits, base_range, NULL), cleanup);
            part->max_64 = part->min_64;
        } else if (*expr == '|') {
            if (!parts || range_expected) {
                LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                        "Invalid %s restriction - unexpected beginning of the expression (%s).", length_restr ? "length" : "range", expr);
                ret = LY_EVALID;
                goto cleanup;
            }
            expr++;
            parts_done++;
            /* process next part of the expression */
        } else if (!strncmp(expr, "..", 2)) {
            expr += 2;
            while (isspace(*expr)) {
                expr++;
            }
            if (!parts || (LY_ARRAY_COUNT(parts) == parts_done)) {
                LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                        "Invalid %s restriction - unexpected \"..\" without a lower bound.", length_restr ? "length" : "range");
                ret = LY_EVALID;
                goto cleanup;
            }
            /* continue expecting the upper boundary */
            range_expected = 1;
        } else if (isdigit(*expr) || (*expr == '-') || (*expr == '+')) {
            /* number */
            if (range_expected) {
                part = &parts[LY_ARRAY_COUNT(parts) - 1];
                LY_CHECK_GOTO(ret = range_part_minmax(ctx, part, 1, part->min_64, basetype, 0, length_restr, frdigits, NULL, &expr), cleanup);
                range_expected = 0;
            } else {
                LY_ARRAY_NEW_GOTO(ctx->ctx, parts, part, ret, cleanup);
                LY_CHECK_GOTO(ret = range_part_minmax(ctx, part, 0, parts_done ? parts[LY_ARRAY_COUNT(parts) - 2].max_64 : 0,
                        basetype, parts_done ? 0 : 1, length_restr, frdigits, NULL, &expr), cleanup);
                part->max_64 = part->min_64;
            }

            /* continue with possible another expression part */
        } else if (!strncmp(expr, "max", ly_strlen_const("max"))) {
            expr += ly_strlen_const("max");
            while (isspace(*expr)) {
                expr++;
            }
            if (*expr != '\0') {
                LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG, "Invalid %s restriction - unexpected data after max keyword (%s).",
                        length_restr ? "length" : "range", expr);
                ret = LY_EVALID;
                goto cleanup;
            }
            if (range_expected) {
                part = &parts[LY_ARRAY_COUNT(parts) - 1];
                LY_CHECK_GOTO(ret = range_part_minmax(ctx, part, 1, part->min_64, basetype, 0, length_restr, frdigits, base_range, NULL), cleanup);
                range_expected = 0;
            } else {
                LY_ARRAY_NEW_GOTO(ctx->ctx, parts, part, ret, cleanup);
                LY_CHECK_GOTO(ret = range_part_minmax(ctx, part, 1, parts_done ? parts[LY_ARRAY_COUNT(parts) - 2].max_64 : 0,
                        basetype, parts_done ? 0 : 1, length_restr, frdigits, base_range, NULL), cleanup);
                part->min_64 = part->max_64;
            }
        } else {
            LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG, "Invalid %s restriction - unexpected data (%s).",
                    length_restr ? "length" : "range", expr);
            ret = LY_EVALID;
            goto cleanup;
        }
    }

    /* check with the previous range/length restriction */
    if (base_range) {
        switch (basetype) {
        case LY_TYPE_BINARY:
        case LY_TYPE_UINT8:
        case LY_TYPE_UINT16:
        case LY_TYPE_UINT32:
        case LY_TYPE_UINT64:
        case LY_TYPE_STRING:
            uns = 1;
            break;
        case LY_TYPE_DEC64:
        case LY_TYPE_INT8:
        case LY_TYPE_INT16:
        case LY_TYPE_INT32:
        case LY_TYPE_INT64:
            uns = 0;
            break;
        default:
            LOGINT(ctx->ctx);
            ret = LY_EINT;
            goto cleanup;
        }
        for (u = v = 0; u < parts_done && v < LY_ARRAY_COUNT(base_range->parts); ++u) {
            if ((uns && (parts[u].min_u64 < base_range->parts[v].min_u64)) || (!uns && (parts[u].min_64 < base_range->parts[v].min_64))) {
                goto baseerror;
            }
            /* current lower bound is not lower than the base */
            if (base_range->parts[v].min_64 == base_range->parts[v].max_64) {
                /* base has single value */
                if (base_range->parts[v].min_64 == parts[u].min_64) {
                    /* both lower bounds are the same */
                    if (parts[u].min_64 != parts[u].max_64) {
                        /* current continues with a range */
                        goto baseerror;
                    } else {
                        /* equal single values, move both forward */
                        ++v;
                        continue;
                    }
                } else {
                    /* base is single value lower than current range, so the
                     * value from base range is removed in the current,
                     * move only base and repeat checking */
                    ++v;
                    --u;
                    continue;
                }
            } else {
                /* base is the range */
                if (parts[u].min_64 == parts[u].max_64) {
                    /* current is a single value */
                    if ((uns && (parts[u].max_u64 > base_range->parts[v].max_u64)) || (!uns && (parts[u].max_64 > base_range->parts[v].max_64))) {
                        /* current is behind the base range, so base range is omitted,
                         * move the base and keep the current for further check */
                        ++v;
                        --u;
                    } /* else it is within the base range, so move the current, but keep the base */
                    continue;
                } else {
                    /* both are ranges - check the higher bound, the lower was already checked */
                    if ((uns && (parts[u].max_u64 > base_range->parts[v].max_u64)) || (!uns && (parts[u].max_64 > base_range->parts[v].max_64))) {
                        /* higher bound is higher than the current higher bound */
                        if ((uns && (parts[u].min_u64 > base_range->parts[v].max_u64)) || (!uns && (parts[u].min_64 > base_range->parts[v].max_64))) {
                            /* but the current lower bound is also higher, so the base range is omitted,
                             * continue with the same current, but move the base */
                            --u;
                            ++v;
                            continue;
                        }
                        /* current range starts within the base range but end behind it */
                        goto baseerror;
                    } else {
                        /* current range is smaller than the base,
                         * move current, but stay with the base */
                        continue;
                    }
                }
            }
        }
        if (u != parts_done) {
baseerror:
            LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                    "Invalid %s restriction - the derived restriction (%s) is not equally or more limiting.",
                    length_restr ? "length" : "range", range_p->arg.str);
            ret = LY_EVALID;
            goto cleanup;
        }
    }

    if (!(*range)) {
        *range = calloc(1, sizeof **range);
        LY_CHECK_ERR_RET(!(*range), LOGMEM(ctx->ctx), LY_EMEM);
    }

    /* we rewrite the following values as the types chain is being processed */
    if (range_p->eapptag) {
        lydict_remove(ctx->ctx, (*range)->eapptag);
        LY_CHECK_GOTO(ret = lydict_insert(ctx->ctx, range_p->eapptag, 0, &(*range)->eapptag), cleanup);
    }
    if (range_p->emsg) {
        lydict_remove(ctx->ctx, (*range)->emsg);
        LY_CHECK_GOTO(ret = lydict_insert(ctx->ctx, range_p->emsg, 0, &(*range)->emsg), cleanup);
    }
    if (range_p->dsc) {
        lydict_remove(ctx->ctx, (*range)->dsc);
        LY_CHECK_GOTO(ret = lydict_insert(ctx->ctx, range_p->dsc, 0, &(*range)->dsc), cleanup);
    }
    if (range_p->ref) {
        lydict_remove(ctx->ctx, (*range)->ref);
        LY_CHECK_GOTO(ret = lydict_insert(ctx->ctx, range_p->ref, 0, &(*range)->ref), cleanup);
    }
    /* extensions are taken only from the last range by the caller */

    (*range)->parts = parts;
    parts = NULL;
cleanup:
    LY_ARRAY_FREE(parts);

    return ret;
}

/**
 * @brief Transform characters block in an XML Schema pattern into Perl character ranges.
 *
 * @param[in] ctx libyang context.
 * @param[in] pattern Original pattern.
 * @param[in,out] regex Pattern to modify.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_pattern_chblocks_xmlschema2perl(const struct ly_ctx *ctx, const char *pattern, char **regex)
{
#define URANGE_LEN 19
    char *ublock2urange[][2] = {
        {"BasicLatin", "[\\x{0000}-\\x{007F}]"},
        {"Latin-1Supplement", "[\\x{0080}-\\x{00FF}]"},
        {"LatinExtended-A", "[\\x{0100}-\\x{017F}]"},
        {"LatinExtended-B", "[\\x{0180}-\\x{024F}]"},
        {"IPAExtensions", "[\\x{0250}-\\x{02AF}]"},
        {"SpacingModifierLetters", "[\\x{02B0}-\\x{02FF}]"},
        {"CombiningDiacriticalMarks", "[\\x{0300}-\\x{036F}]"},
        {"Greek", "[\\x{0370}-\\x{03FF}]"},
        {"Cyrillic", "[\\x{0400}-\\x{04FF}]"},
        {"Armenian", "[\\x{0530}-\\x{058F}]"},
        {"Hebrew", "[\\x{0590}-\\x{05FF}]"},
        {"Arabic", "[\\x{0600}-\\x{06FF}]"},
        {"Syriac", "[\\x{0700}-\\x{074F}]"},
        {"Thaana", "[\\x{0780}-\\x{07BF}]"},
        {"Devanagari", "[\\x{0900}-\\x{097F}]"},
        {"Bengali", "[\\x{0980}-\\x{09FF}]"},
        {"Gurmukhi", "[\\x{0A00}-\\x{0A7F}]"},
        {"Gujarati", "[\\x{0A80}-\\x{0AFF}]"},
        {"Oriya", "[\\x{0B00}-\\x{0B7F}]"},
        {"Tamil", "[\\x{0B80}-\\x{0BFF}]"},
        {"Telugu", "[\\x{0C00}-\\x{0C7F}]"},
        {"Kannada", "[\\x{0C80}-\\x{0CFF}]"},
        {"Malayalam", "[\\x{0D00}-\\x{0D7F}]"},
        {"Sinhala", "[\\x{0D80}-\\x{0DFF}]"},
        {"Thai", "[\\x{0E00}-\\x{0E7F}]"},
        {"Lao", "[\\x{0E80}-\\x{0EFF}]"},
        {"Tibetan", "[\\x{0F00}-\\x{0FFF}]"},
        {"Myanmar", "[\\x{1000}-\\x{109F}]"},
        {"Georgian", "[\\x{10A0}-\\x{10FF}]"},
        {"HangulJamo", "[\\x{1100}-\\x{11FF}]"},
        {"Ethiopic", "[\\x{1200}-\\x{137F}]"},
        {"Cherokee", "[\\x{13A0}-\\x{13FF}]"},
        {"UnifiedCanadianAboriginalSyllabics", "[\\x{1400}-\\x{167F}]"},
        {"Ogham", "[\\x{1680}-\\x{169F}]"},
        {"Runic", "[\\x{16A0}-\\x{16FF}]"},
        {"Khmer", "[\\x{1780}-\\x{17FF}]"},
        {"Mongolian", "[\\x{1800}-\\x{18AF}]"},
        {"LatinExtendedAdditional", "[\\x{1E00}-\\x{1EFF}]"},
        {"GreekExtended", "[\\x{1F00}-\\x{1FFF}]"},
        {"GeneralPunctuation", "[\\x{2000}-\\x{206F}]"},
        {"SuperscriptsandSubscripts", "[\\x{2070}-\\x{209F}]"},
        {"CurrencySymbols", "[\\x{20A0}-\\x{20CF}]"},
        {"CombiningMarksforSymbols", "[\\x{20D0}-\\x{20FF}]"},
        {"LetterlikeSymbols", "[\\x{2100}-\\x{214F}]"},
        {"NumberForms", "[\\x{2150}-\\x{218F}]"},
        {"Arrows", "[\\x{2190}-\\x{21FF}]"},
        {"MathematicalOperators", "[\\x{2200}-\\x{22FF}]"},
        {"MiscellaneousTechnical", "[\\x{2300}-\\x{23FF}]"},
        {"ControlPictures", "[\\x{2400}-\\x{243F}]"},
        {"OpticalCharacterRecognition", "[\\x{2440}-\\x{245F}]"},
        {"EnclosedAlphanumerics", "[\\x{2460}-\\x{24FF}]"},
        {"BoxDrawing", "[\\x{2500}-\\x{257F}]"},
        {"BlockElements", "[\\x{2580}-\\x{259F}]"},
        {"GeometricShapes", "[\\x{25A0}-\\x{25FF}]"},
        {"MiscellaneousSymbols", "[\\x{2600}-\\x{26FF}]"},
        {"Dingbats", "[\\x{2700}-\\x{27BF}]"},
        {"BraillePatterns", "[\\x{2800}-\\x{28FF}]"},
        {"CJKRadicalsSupplement", "[\\x{2E80}-\\x{2EFF}]"},
        {"KangxiRadicals", "[\\x{2F00}-\\x{2FDF}]"},
        {"IdeographicDescriptionCharacters", "[\\x{2FF0}-\\x{2FFF}]"},
        {"CJKSymbolsandPunctuation", "[\\x{3000}-\\x{303F}]"},
        {"Hiragana", "[\\x{3040}-\\x{309F}]"},
        {"Katakana", "[\\x{30A0}-\\x{30FF}]"},
        {"Bopomofo", "[\\x{3100}-\\x{312F}]"},
        {"HangulCompatibilityJamo", "[\\x{3130}-\\x{318F}]"},
        {"Kanbun", "[\\x{3190}-\\x{319F}]"},
        {"BopomofoExtended", "[\\x{31A0}-\\x{31BF}]"},
        {"EnclosedCJKLettersandMonths", "[\\x{3200}-\\x{32FF}]"},
        {"CJKCompatibility", "[\\x{3300}-\\x{33FF}]"},
        {"CJKUnifiedIdeographsExtensionA", "[\\x{3400}-\\x{4DB5}]"},
        {"CJKUnifiedIdeographs", "[\\x{4E00}-\\x{9FFF}]"},
        {"YiSyllables", "[\\x{A000}-\\x{A48F}]"},
        {"YiRadicals", "[\\x{A490}-\\x{A4CF}]"},
        {"HangulSyllables", "[\\x{AC00}-\\x{D7A3}]"},
        {"PrivateUse", "[\\x{E000}-\\x{F8FF}]"},
        {"CJKCompatibilityIdeographs", "[\\x{F900}-\\x{FAFF}]"},
        {"AlphabeticPresentationForms", "[\\x{FB00}-\\x{FB4F}]"},
        {"ArabicPresentationForms-A", "[\\x{FB50}-\\x{FDFF}]"},
        {"CombiningHalfMarks", "[\\x{FE20}-\\x{FE2F}]"},
        {"CJKCompatibilityForms", "[\\x{FE30}-\\x{FE4F}]"},
        {"SmallFormVariants", "[\\x{FE50}-\\x{FE6F}]"},
        {"ArabicPresentationForms-B", "[\\x{FE70}-\\x{FEFE}]"},
        {"HalfwidthandFullwidthForms", "[\\x{FF00}-\\x{FFEF}]"},
        {"Specials", "[\\x{FEFF}|\\x{FFF0}-\\x{FFFD}]"},
        {NULL, NULL}
    };

    size_t idx, idx2, start, end;
    char *perl_regex, *ptr;

    perl_regex = *regex;

    /* substitute Unicode Character Blocks with exact Character Ranges */
    while ((ptr = strstr(perl_regex, "\\p{Is"))) {
        start = ptr - perl_regex;

        ptr = strchr(ptr, '}');
        if (!ptr) {
            LOGVAL(ctx, LY_VCODE_INREGEXP, pattern, perl_regex + start + 2, "unterminated character property");
            return LY_EVALID;
        }
        end = (ptr - perl_regex) + 1;

        /* need more space */
        if (end - start < URANGE_LEN) {
            perl_regex = ly_realloc(perl_regex, strlen(perl_regex) + (URANGE_LEN - (end - start)) + 1);
            *regex = perl_regex;
            LY_CHECK_ERR_RET(!perl_regex, LOGMEM(ctx), LY_EMEM);
        }

        /* find our range */
        for (idx = 0; ublock2urange[idx][0]; ++idx) {
            if (!strncmp(perl_regex + start + ly_strlen_const("\\p{Is"),
                    ublock2urange[idx][0], strlen(ublock2urange[idx][0]))) {
                break;
            }
        }
        if (!ublock2urange[idx][0]) {
            LOGVAL(ctx, LY_VCODE_INREGEXP, pattern, perl_regex + start + 5, "unknown block name");
            return LY_EVALID;
        }

        /* make the space in the string and replace the block (but we cannot include brackets if it was already enclosed in them) */
        for (idx2 = 0, idx = 0; idx2 < start; ++idx2) {
            if ((perl_regex[idx2] == '[') && (!idx2 || (perl_regex[idx2 - 1] != '\\'))) {
                ++idx;
            }
            if ((perl_regex[idx2] == ']') && (!idx2 || (perl_regex[idx2 - 1] != '\\'))) {
                --idx;
            }
        }
        if (idx) {
            /* skip brackets */
            memmove(perl_regex + start + (URANGE_LEN - 2), perl_regex + end, strlen(perl_regex + end) + 1);
            memcpy(perl_regex + start, ublock2urange[idx][1] + 1, URANGE_LEN - 2);
        } else {
            memmove(perl_regex + start + URANGE_LEN, perl_regex + end, strlen(perl_regex + end) + 1);
            memcpy(perl_regex + start, ublock2urange[idx][1], URANGE_LEN);
        }
    }

    return LY_SUCCESS;
}

LY_ERR
lys_compile_type_pattern_check(struct ly_ctx *ctx, const char *pattern, pcre2_code **code)
{
    size_t idx, size, brack;
    char *perl_regex;
    int err_code, compile_opts;
    const char *orig_ptr;
    PCRE2_SIZE err_offset;
    pcre2_code *code_local;
    ly_bool escaped;
    LY_ERR r;

    /* adjust the expression to a Perl equivalent
     * http://www.w3.org/TR/2004/REC-xmlschema-2-20041028/#regexs */

    /* allocate space for the transformed pattern */
    size = strlen(pattern) + 1;
    compile_opts = PCRE2_UTF | PCRE2_UCP | PCRE2_ANCHORED | PCRE2_DOLLAR_ENDONLY | PCRE2_NO_AUTO_CAPTURE;
#ifdef PCRE2_ENDANCHORED
    compile_opts |= PCRE2_ENDANCHORED;
#else
    /* add space for trailing $ anchor */
    size++;
#endif
    perl_regex = malloc(size);
    LY_CHECK_ERR_RET(!perl_regex, LOGMEM(ctx), LY_EMEM);
    perl_regex[0] = '\0';

    /* we need to replace all "$" and "^" (that are not in "[]") with "\$" and "\^" */
    brack = 0;
    idx = 0;
    escaped = 0;
    orig_ptr = pattern;
    while (orig_ptr[0]) {
        switch (orig_ptr[0]) {
        case '$':
        case '^':
            if (!brack) {
                /* make space for the extra character */
                ++size;
                perl_regex = ly_realloc(perl_regex, size);
                LY_CHECK_ERR_RET(!perl_regex, LOGMEM(ctx), LY_EMEM);

                /* print escape slash */
                perl_regex[idx] = '\\';
                ++idx;
            }
            break;
        case '\\':
            /*  escape character found or backslash is escaped */
            escaped = !escaped;
            /* copy backslash and continue with the next character */
            perl_regex[idx] = orig_ptr[0];
            ++idx;
            ++orig_ptr;
            continue;
        case '[':
            if (!escaped) {
                ++brack;
            }
            break;
        case ']':
            if (!brack && !escaped) {
                /* If ']' does not terminate a character class expression, then pcre2_compile() implicitly escapes the
                 * ']' character. But this seems to be against the regular expressions rules declared in
                 * "XML schema: Datatypes" and therefore an error is returned. So for example if pattern is '\[a]' then
                 * pcre2 match characters '[a]' literally but in YANG such pattern is not allowed.
                 */
                LOGVAL(ctx, LY_VCODE_INREGEXP, pattern, orig_ptr, "character group doesn't begin with '['");
                free(perl_regex);
                return LY_EVALID;
            } else if (!escaped) {
                --brack;
            }
            break;
        default:
            break;
        }

        /* copy char */
        perl_regex[idx] = orig_ptr[0];

        ++idx;
        ++orig_ptr;
        escaped = 0;
    }
#ifndef PCRE2_ENDANCHORED
    /* anchor match to end of subject */
    perl_regex[idx++] = '$';
#endif
    perl_regex[idx] = '\0';

    /* transform character blocks */
    if ((r = lys_compile_pattern_chblocks_xmlschema2perl(ctx, pattern, &perl_regex))) {
        free(perl_regex);
        return r;
    }

    /* must return 0, already checked during parsing */
    code_local = pcre2_compile((PCRE2_SPTR)perl_regex, PCRE2_ZERO_TERMINATED, compile_opts,
            &err_code, &err_offset, NULL);
    if (!code_local) {
        PCRE2_UCHAR err_msg[LY_PCRE2_MSG_LIMIT] = {0};

        pcre2_get_error_message(err_code, err_msg, LY_PCRE2_MSG_LIMIT);
        LOGVAL(ctx, LY_VCODE_INREGEXP, pattern, perl_regex + err_offset, err_msg);
        free(perl_regex);
        return LY_EVALID;
    }
    free(perl_regex);

    if (code) {
        *code = code_local;
    } else {
        free(code_local);
    }

    return LY_SUCCESS;

#undef URANGE_LEN
}

LY_ERR
lys_compile_type_patterns(struct lysc_ctx *ctx, const struct lysp_restr *patterns_p, struct lysc_pattern **base_patterns,
        struct lysc_pattern ***patterns)
{
    struct lysc_pattern **pattern;
    LY_ARRAY_COUNT_TYPE u;
    LY_ERR ret = LY_SUCCESS;

    /* first, copy the patterns from the base type */
    if (base_patterns) {
        *patterns = lysc_patterns_dup(ctx->ctx, base_patterns);
        LY_CHECK_ERR_RET(!(*patterns), LOGMEM(ctx->ctx), LY_EMEM);
    }

    LY_ARRAY_FOR(patterns_p, u) {
        LY_ARRAY_NEW_RET(ctx->ctx, (*patterns), pattern, LY_EMEM);
        *pattern = calloc(1, sizeof **pattern);
        ++(*pattern)->refcount;

        ret = lys_compile_type_pattern_check(ctx->ctx, &patterns_p[u].arg.str[1], &(*pattern)->code);
        LY_CHECK_RET(ret);

        if (patterns_p[u].arg.str[0] == LYSP_RESTR_PATTERN_NACK) {
            (*pattern)->inverted = 1;
        }
        DUP_STRING_GOTO(ctx->ctx, &patterns_p[u].arg.str[1], (*pattern)->expr, ret, done);
        DUP_STRING_GOTO(ctx->ctx, patterns_p[u].eapptag, (*pattern)->eapptag, ret, done);
        DUP_STRING_GOTO(ctx->ctx, patterns_p[u].emsg, (*pattern)->emsg, ret, done);
        DUP_STRING_GOTO(ctx->ctx, patterns_p[u].dsc, (*pattern)->dsc, ret, done);
        DUP_STRING_GOTO(ctx->ctx, patterns_p[u].ref, (*pattern)->ref, ret, done);
        COMPILE_EXTS_GOTO(ctx, patterns_p[u].exts, (*pattern)->exts, (*pattern), ret, done);
    }
done:
    return ret;
}

/**
 * @brief map of the possible restrictions combination for the specific built-in type.
 */
static uint16_t type_substmt_map[LY_DATA_TYPE_COUNT] = {
    0 /* LY_TYPE_UNKNOWN */,
    LYS_SET_LENGTH /* LY_TYPE_BINARY */,
    LYS_SET_RANGE /* LY_TYPE_UINT8 */,
    LYS_SET_RANGE /* LY_TYPE_UINT16 */,
    LYS_SET_RANGE /* LY_TYPE_UINT32 */,
    LYS_SET_RANGE /* LY_TYPE_UINT64 */,
    LYS_SET_LENGTH | LYS_SET_PATTERN /* LY_TYPE_STRING */,
    LYS_SET_BIT /* LY_TYPE_BITS */,
    0 /* LY_TYPE_BOOL */,
    LYS_SET_FRDIGITS | LYS_SET_RANGE /* LY_TYPE_DEC64 */,
    0 /* LY_TYPE_EMPTY */,
    LYS_SET_ENUM /* LY_TYPE_ENUM */,
    LYS_SET_BASE /* LY_TYPE_IDENT */,
    LYS_SET_REQINST /* LY_TYPE_INST */,
    LYS_SET_REQINST | LYS_SET_PATH /* LY_TYPE_LEAFREF */,
    LYS_SET_TYPE /* LY_TYPE_UNION */,
    LYS_SET_RANGE /* LY_TYPE_INT8 */,
    LYS_SET_RANGE /* LY_TYPE_INT16 */,
    LYS_SET_RANGE /* LY_TYPE_INT32 */,
    LYS_SET_RANGE /* LY_TYPE_INT64 */
};

/**
 * @brief stringification of the YANG built-in data types
 */
const char *ly_data_type2str[LY_DATA_TYPE_COUNT] = {
    LY_TYPE_UNKNOWN_STR,
    LY_TYPE_BINARY_STR,
    LY_TYPE_UINT8_STR,
    LY_TYPE_UINT16_STR,
    LY_TYPE_UINT32_STR,
    LY_TYPE_UINT64_STR,
    LY_TYPE_STRING_STR,
    LY_TYPE_BITS_STR,
    LY_TYPE_BOOL_STR,
    LY_TYPE_DEC64_STR,
    LY_TYPE_EMPTY_STR,
    LY_TYPE_ENUM_STR,
    LY_TYPE_IDENT_STR,
    LY_TYPE_INST_STR,
    LY_TYPE_LEAFREF_STR,
    LY_TYPE_UNION_STR,
    LY_TYPE_INT8_STR,
    LY_TYPE_INT16_STR,
    LY_TYPE_INT32_STR,
    LY_TYPE_INT64_STR
};

LY_ERR
lys_compile_type_enums(struct lysc_ctx *ctx, const struct lysp_type_enum *enums_p, LY_DATA_TYPE basetype,
        struct lysc_type_bitenum_item *base_enums, struct lysc_type_bitenum_item **bitenums)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v, match = 0;
    int32_t highest_value = INT32_MIN, cur_val = INT32_MIN;
    uint32_t highest_position = 0, cur_pos = 0;
    struct lysc_type_bitenum_item *e, storage;
    ly_bool enabled;

    if (base_enums && (ctx->pmod->version < LYS_VERSION_1_1)) {
        LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG, "%s type can be subtyped only in YANG 1.1 modules.",
                basetype == LY_TYPE_ENUM ? "Enumeration" : "Bits");
        return LY_EVALID;
    }

    LY_ARRAY_FOR(enums_p, u) {
        /* perform all checks */
        if (base_enums) {
            /* check the enum/bit presence in the base type - the set of enums/bits in the derived type must be a subset */
            LY_ARRAY_FOR(base_enums, v) {
                if (!strcmp(enums_p[u].name, base_enums[v].name)) {
                    break;
                }
            }
            if (v == LY_ARRAY_COUNT(base_enums)) {
                LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                        "Invalid %s - derived type adds new item \"%s\".",
                        basetype == LY_TYPE_ENUM ? "enumeration" : "bits", enums_p[u].name);
                return LY_EVALID;
            }
            match = v;
        }

        if (basetype == LY_TYPE_ENUM) {
            if (enums_p[u].flags & LYS_SET_VALUE) {
                /* value assigned by model */
                cur_val = (int32_t)enums_p[u].value;
                /* check collision with other values */
                LY_ARRAY_FOR(*bitenums, v) {
                    if (cur_val == (*bitenums)[v].value) {
                        LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                                "Invalid enumeration - value %" PRId32 " collide in items \"%s\" and \"%s\".",
                                cur_val, enums_p[u].name, (*bitenums)[v].name);
                        return LY_EVALID;
                    }
                }
            } else if (base_enums) {
                /* inherit the assigned value */
                cur_val = base_enums[match].value;
            } else {
                /* assign value automatically */
                if (u == 0) {
                    cur_val = 0;
                } else if (highest_value == INT32_MAX) {
                    LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                            "Invalid enumeration - it is not possible to auto-assign enum value for "
                            "\"%s\" since the highest value is already 2147483647.", enums_p[u].name);
                    return LY_EVALID;
                } else {
                    cur_val = highest_value + 1;
                }
            }

            /* save highest value for auto assing */
            if (highest_value < cur_val) {
                highest_value = cur_val;
            }
        } else { /* LY_TYPE_BITS */
            if (enums_p[u].flags & LYS_SET_VALUE) {
                /* value assigned by model */
                cur_pos = (uint32_t)enums_p[u].value;
                /* check collision with other values */
                LY_ARRAY_FOR(*bitenums, v) {
                    if (cur_pos == (*bitenums)[v].position) {
                        LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                                "Invalid bits - position %" PRIu32 " collide in items \"%s\" and \"%s\".",
                                cur_pos, enums_p[u].name, (*bitenums)[v].name);
                        return LY_EVALID;
                    }
                }
            } else if (base_enums) {
                /* inherit the assigned value */
                cur_pos = base_enums[match].position;
            } else {
                /* assign value automatically */
                if (u == 0) {
                    cur_pos = 0;
                } else if (highest_position == UINT32_MAX) {
                    /* counter overflow */
                    LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                            "Invalid bits - it is not possible to auto-assign bit position for "
                            "\"%s\" since the highest value is already 4294967295.", enums_p[u].name);
                    return LY_EVALID;
                } else {
                    cur_pos = highest_position + 1;
                }
            }

            /* save highest position for auto assing */
            if (highest_position < cur_pos) {
                highest_position = cur_pos;
            }
        }

        /* the assigned values must not change from the derived type */
        if (base_enums) {
            if (basetype == LY_TYPE_ENUM) {
                if (cur_val != base_enums[match].value) {
                    LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                            "Invalid enumeration - value of the item \"%s\" has changed from %" PRId32 " to %" PRId32
                            " in the derived type.", enums_p[u].name, base_enums[match].value, cur_val);
                    return LY_EVALID;
                }
            } else {
                if (cur_pos != base_enums[match].position) {
                    LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                            "Invalid bits - position of the item \"%s\" has changed from %" PRIu32 " to %" PRIu32
                            " in the derived type.", enums_p[u].name, base_enums[match].position, cur_pos);
                    return LY_EVALID;
                }
            }
        }

        /* add new enum/bit */
        LY_ARRAY_NEW_RET(ctx->ctx, *bitenums, e, LY_EMEM);
        DUP_STRING_GOTO(ctx->ctx, enums_p[u].name, e->name, ret, done);
        DUP_STRING_GOTO(ctx->ctx, enums_p[u].dsc, e->dsc, ret, done);
        DUP_STRING_GOTO(ctx->ctx, enums_p[u].ref, e->ref, ret, done);
        e->flags = (enums_p[u].flags & LYS_FLAGS_COMPILED_MASK) | (basetype == LY_TYPE_ENUM ? LYS_IS_ENUM : 0);
        if (basetype == LY_TYPE_ENUM) {
            e->value = cur_val;
        } else {
            e->position = cur_pos;
        }
        COMPILE_EXTS_GOTO(ctx, enums_p[u].exts, e->exts, e, ret, done);

        /* evaluate if-ffeatures */
        LY_CHECK_RET(lys_eval_iffeatures(ctx->ctx, enums_p[u].iffeatures, &enabled));
        if (!enabled) {
            /* set only flag, later resolved and removed */
            e->flags |= LYS_DISABLED;
        }

        if (basetype == LY_TYPE_BITS) {
            /* keep bits ordered by position */
            for (v = u; v && (*bitenums)[v - 1].position > e->position; --v) {}
            if (v != u) {
                memcpy(&storage, e, sizeof *e);
                memmove(&(*bitenums)[v + 1], &(*bitenums)[v], (u - v) * sizeof **bitenums);
                memcpy(&(*bitenums)[v], &storage, sizeof storage);
            }
        }
    }

done:
    return ret;
}

/**
 * @brief Compile union type.
 *
 * @param[in] ctx Compile context.
 * @param[in] ptypes Parsed union types.
 * @param[in] context_pnode Schema node where the type/typedef is placed to correctly find the base types.
 * @param[in] context_flags Flags of the context node or the referencing typedef to correctly check status of referencing and referenced objects.
 * @param[in] context_name Name of the context node or referencing typedef for logging.
 * @param[out] utypes_p Array of compiled union types.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_type_union(struct lysc_ctx *ctx, struct lysp_type *ptypes, struct lysp_node *context_pnode, uint16_t context_flags,
        const char *context_name, struct lysc_type ***utypes_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type **utypes = *utypes_p;
    struct lysc_type_union *un_aux = NULL;

    LY_ARRAY_CREATE_GOTO(ctx->ctx, utypes, LY_ARRAY_COUNT(ptypes), ret, error);
    for (LY_ARRAY_COUNT_TYPE u = 0, additional = 0; u < LY_ARRAY_COUNT(ptypes); ++u) {
        ret = lys_compile_type(ctx, context_pnode, context_flags, context_name, &ptypes[u], &utypes[u + additional],
                NULL, NULL);
        LY_CHECK_GOTO(ret, error);
        LY_ATOMIC_INC_BARRIER(utypes[u + additional]->refcount);

        if (utypes[u + additional]->basetype == LY_TYPE_UNION) {
            /* add space for additional types from the union subtype */
            un_aux = (struct lysc_type_union *)utypes[u + additional];
            LY_ARRAY_CREATE_GOTO(ctx->ctx, utypes,
                    LY_ARRAY_COUNT(ptypes) + additional + LY_ARRAY_COUNT(un_aux->types) - LY_ARRAY_COUNT(utypes), ret, error);

            /* copy subtypes of the subtype union */
            for (LY_ARRAY_COUNT_TYPE v = 0; v < LY_ARRAY_COUNT(un_aux->types); ++v) {
                utypes[u + additional] = un_aux->types[v];
                LY_ATOMIC_INC_BARRIER(un_aux->types[v]->refcount);
                ++additional;
                LY_ARRAY_INCREMENT(utypes);
            }
            /* compensate u increment in main loop */
            --additional;

            /* free the replaced union subtype */
            lysc_type_free(&ctx->free_ctx, (struct lysc_type *)un_aux);
            un_aux = NULL;
        } else {
            LY_ARRAY_INCREMENT(utypes);
        }
    }

    *utypes_p = utypes;
    return LY_SUCCESS;

error:
    if (un_aux) {
        lysc_type_free(&ctx->free_ctx, (struct lysc_type *)un_aux);
    }
    *utypes_p = utypes;
    return ret;
}

/**
 * @brief Allocate a new specific type structure according to the basetype.
 *
 * @param[in] ctx Context to use.
 * @param[in] basetype Base type of the new type.
 * @param[in] tpdf_name Optional referenced typedef name, NULL for built-in types.
 * @param[out] type Specific type structure.
 * @return LY_ERR value.
 */
static LY_ERR
lys_new_type(const struct ly_ctx *ctx, LY_DATA_TYPE basetype, const char *tpdf_name, struct lysc_type **type)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysc_type *t = NULL;

    *type = NULL;

    switch (basetype) {
    case LY_TYPE_BINARY:
        t = calloc(1, sizeof(struct lysc_type_bin));
        break;
    case LY_TYPE_BITS:
        t = calloc(1, sizeof(struct lysc_type_bits));
        break;
    case LY_TYPE_DEC64:
        t = calloc(1, sizeof(struct lysc_type_dec));
        break;
    case LY_TYPE_STRING:
        t = calloc(1, sizeof(struct lysc_type_str));
        break;
    case LY_TYPE_ENUM:
        t = calloc(1, sizeof(struct lysc_type_enum));
        break;
    case LY_TYPE_INT8:
    case LY_TYPE_UINT8:
    case LY_TYPE_INT16:
    case LY_TYPE_UINT16:
    case LY_TYPE_INT32:
    case LY_TYPE_UINT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
        t = calloc(1, sizeof(struct lysc_type_num));
        break;
    case LY_TYPE_IDENT:
        t = calloc(1, sizeof(struct lysc_type_identityref));
        break;
    case LY_TYPE_LEAFREF:
        t = calloc(1, sizeof(struct lysc_type_leafref));
        break;
    case LY_TYPE_INST:
        t = calloc(1, sizeof(struct lysc_type_instanceid));
        break;
    case LY_TYPE_UNION:
        t = calloc(1, sizeof(struct lysc_type_union));
        break;
    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
        t = calloc(1, sizeof(struct lysc_type));
        break;
    case LY_TYPE_UNKNOWN:
        break;
    }
    LY_CHECK_ERR_GOTO(!t, LOGMEM(ctx); rc = LY_EMEM, cleanup);

    if (tpdf_name) {
        rc = lydict_insert(ctx, tpdf_name, 0, &t->name);
        LY_CHECK_GOTO(rc, cleanup);
    }

cleanup:
    if (rc) {
        free(t);
    } else {
        *type = t;
    }
    return rc;
}

/**
 * @brief The core of the lys_compile_type() - compile information about the given type (from typedef or leaf/leaf-list).
 *
 * @param[in] ctx Compile context.
 * @param[in] context_pnode Schema node where the type/typedef is placed to correctly find the base types.
 * @param[in] context_flags Flags of the context node or the referencing typedef to correctly check status of
 * referencing and referenced objects.
 * @param[in] context_name Name of the context node or referencing typedef for logging.
 * @param[in] type_p Parsed type to compile.
 * @param[in] basetype Base YANG built-in type of the type to compile.
 * @param[in] tpdfname Name of the type's typedef, serves as a flag - if it is leaf/leaf-list's type, it is NULL.
 * @param[in] base Latest base (compiled) type from which the current type is being derived.
 * @param[in] plugin Type plugin to use.
 * @param[in] tpdf_chain Chain of the used typedefs, traversed backwards.
 * @param[in] tpdf_chain_last Index of the last (backwards) typedef in @p tpdf_chain to use.
 * @param[out] type Compiled type.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_type_(struct lysc_ctx *ctx, struct lysp_node *context_pnode, uint16_t context_flags, const char *context_name,
        const struct lysp_type *type_p, LY_DATA_TYPE basetype, const char *tpdfname, const struct lysc_type *base,
        struct lyplg_type *plugin, struct ly_set *tpdf_chain, uint32_t tpdf_chain_last, struct lysc_type **type)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysc_type_bin *bin;
    struct lysc_type_num *num;
    struct lysc_type_str *str;
    struct lysc_type_bits *bits;
    struct lysc_type_enum *enumeration;
    struct lysc_type_dec *dec;
    struct lysc_type_identityref *idref;
    struct lysc_type_leafref *lref;
    struct lysc_type_union *un;
    struct lys_type_item *tpdf_item;
    const struct lysp_type *base_type_p;
    uint32_t i;

    /* alloc and init */
    rc = lys_new_type(ctx->ctx, basetype, tpdfname, type);
    LY_CHECK_GOTO(rc, cleanup);

    (*type)->basetype = basetype;
    (*type)->plugin = plugin;

    switch (basetype) {
    case LY_TYPE_BINARY:
        bin = (struct lysc_type_bin *)*type;

        /* RFC 7950 9.8.1, 9.4.4 - length, number of octets it contains */
        if (type_p->length) {
            LY_CHECK_GOTO(rc = lys_compile_type_range(ctx, type_p->length, basetype, 1, 0,
                    base ? ((struct lysc_type_bin *)base)->length : NULL, &bin->length), cleanup);
            if (!tpdfname) {
                COMPILE_EXTS_GOTO(ctx, type_p->length->exts, bin->length->exts, bin->length, rc, cleanup);
            }
        }
        break;
    case LY_TYPE_BITS:
        /* RFC 7950 9.7 - bits */
        bits = (struct lysc_type_bits *)*type;
        if (type_p->bits) {
            /* compile bits from this type */
            LY_CHECK_GOTO(rc = lys_compile_type_enums(ctx, type_p->bits, basetype,
                    base ? (struct lysc_type_bitenum_item *)((struct lysc_type_bits *)base)->bits : NULL,
                    (struct lysc_type_bitenum_item **)&bits->bits), cleanup);
        } else if (base) {
            /* recompile bits from the first superior type with bits */
            assert(tpdf_chain->count > tpdf_chain_last);
            base_type_p = NULL;
            i = tpdf_chain->count;
            do {
                --i;
                tpdf_item = tpdf_chain->objs[i];

                if (tpdf_item->tpdf->type.bits) {
                    base_type_p = &tpdf_item->tpdf->type;
                    break;
                }
            } while (i > tpdf_chain_last);
            assert(base_type_p);

            LY_CHECK_GOTO(rc = lys_compile_type_enums(ctx, base_type_p->bits, basetype, NULL,
                    (struct lysc_type_bitenum_item **)&bits->bits), cleanup);
        } else {
            /* type derived from bits built-in type must contain at least one bit */
            if (tpdfname) {
                LOGVAL(ctx->ctx, LY_VCODE_MISSCHILDSTMT, "bit", "bits type ", tpdfname);
            } else {
                LOGVAL(ctx->ctx, LY_VCODE_MISSCHILDSTMT, "bit", "bits type", "");
            }
            rc = LY_EVALID;
            goto cleanup;
        }
        break;
    case LY_TYPE_DEC64:
        dec = (struct lysc_type_dec *)*type;

        /* RFC 7950 9.3.4 - fraction-digits */
        if (!base) {
            if (!type_p->fraction_digits) {
                if (tpdfname) {
                    LOGVAL(ctx->ctx, LY_VCODE_MISSCHILDSTMT, "fraction-digits", "decimal64 type ", tpdfname);
                } else {
                    LOGVAL(ctx->ctx, LY_VCODE_MISSCHILDSTMT, "fraction-digits", "decimal64 type", "");
                }
                rc = LY_EVALID;
                goto cleanup;
            }
            dec->fraction_digits = type_p->fraction_digits;
        } else {
            if (type_p->fraction_digits) {
                /* fraction digits is prohibited in types not directly derived from built-in decimal64 */
                if (tpdfname) {
                    LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                            "Invalid fraction-digits substatement for type \"%s\" not directly derived from decimal64 built-in type.",
                            tpdfname);
                } else {
                    LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                            "Invalid fraction-digits substatement for type not directly derived from decimal64 built-in type.");
                }
                rc = LY_EVALID;
                goto cleanup;
            }
            dec->fraction_digits = ((struct lysc_type_dec *)base)->fraction_digits;
        }

        /* RFC 7950 9.2.4 - range */
        if (type_p->range) {
            LY_CHECK_GOTO(rc = lys_compile_type_range(ctx, type_p->range, basetype, 0, dec->fraction_digits,
                    base ? ((struct lysc_type_dec *)base)->range : NULL, &dec->range), cleanup);
            if (!tpdfname) {
                COMPILE_EXTS_GOTO(ctx, type_p->range->exts, dec->range->exts, dec->range, rc, cleanup);
            }
        }
        break;
    case LY_TYPE_STRING:
        str = (struct lysc_type_str *)*type;

        /* RFC 7950 9.4.4 - length */
        if (type_p->length) {
            LY_CHECK_GOTO(rc = lys_compile_type_range(ctx, type_p->length, basetype, 1, 0,
                    base ? ((struct lysc_type_str *)base)->length : NULL, &str->length), cleanup);
            if (!tpdfname) {
                COMPILE_EXTS_GOTO(ctx, type_p->length->exts, str->length->exts, str->length, rc, cleanup);
            }
        } else if (base && ((struct lysc_type_str *)base)->length) {
            str->length = lysc_range_dup(ctx, ((struct lysc_type_str *)base)->length, tpdf_chain, tpdf_chain_last);
        }

        /* RFC 7950 9.4.5 - pattern */
        if (type_p->patterns) {
            LY_CHECK_GOTO(rc = lys_compile_type_patterns(ctx, type_p->patterns,
                    base ? ((struct lysc_type_str *)base)->patterns : NULL, &str->patterns), cleanup);
        } else if (base && ((struct lysc_type_str *)base)->patterns) {
            str->patterns = lysc_patterns_dup(ctx->ctx, ((struct lysc_type_str *)base)->patterns);
        }
        break;
    case LY_TYPE_ENUM:
        enumeration = (struct lysc_type_enum *)*type;

        /* RFC 7950 9.6 - enum */
        if (type_p->enums) {
            LY_CHECK_GOTO(rc = lys_compile_type_enums(ctx, type_p->enums, basetype,
                    base ? ((struct lysc_type_enum *)base)->enums : NULL, &enumeration->enums), cleanup);
        } else if (base) {
            /* recompile enums from the first superior type with enums */
            assert(tpdf_chain->count > tpdf_chain_last);
            base_type_p = NULL;
            i = tpdf_chain->count;
            do {
                --i;
                tpdf_item = tpdf_chain->objs[i];

                if (tpdf_item->tpdf->type.enums) {
                    base_type_p = &tpdf_item->tpdf->type;
                    break;
                }
            } while (i > tpdf_chain_last);
            assert(base_type_p);

            LY_CHECK_GOTO(rc = lys_compile_type_enums(ctx, base_type_p->enums, basetype, NULL, &enumeration->enums), cleanup);
        } else {
            /* type derived from enumerations built-in type must contain at least one enum */
            if (tpdfname) {
                LOGVAL(ctx->ctx, LY_VCODE_MISSCHILDSTMT, "enum", "enumeration type ", tpdfname);
            } else {
                LOGVAL(ctx->ctx, LY_VCODE_MISSCHILDSTMT, "enum", "enumeration type", "");
            }
            rc = LY_EVALID;
            goto cleanup;
        }
        break;
    case LY_TYPE_INT8:
    case LY_TYPE_UINT8:
    case LY_TYPE_INT16:
    case LY_TYPE_UINT16:
    case LY_TYPE_INT32:
    case LY_TYPE_UINT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
        num = (struct lysc_type_num *)*type;

        /* RFC 6020 9.2.4 - range */
        if (type_p->range) {
            LY_CHECK_GOTO(rc = lys_compile_type_range(ctx, type_p->range, basetype, 0, 0,
                    base ? ((struct lysc_type_num *)base)->range : NULL, &num->range), cleanup);
            if (!tpdfname) {
                COMPILE_EXTS_GOTO(ctx, type_p->range->exts, num->range->exts, num->range, rc, cleanup);
            }
        }
        break;
    case LY_TYPE_IDENT:
        idref = (struct lysc_type_identityref *)*type;

        /* RFC 7950 9.10.2 - base */
        if (type_p->bases) {
            if (base) {
                /* only the directly derived identityrefs can contain base specification */
                if (tpdfname) {
                    LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                            "Invalid base substatement for the type \"%s\" not directly derived from identityref built-in type.",
                            tpdfname);
                } else {
                    LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                            "Invalid base substatement for the type not directly derived from identityref built-in type.");
                }
                rc = LY_EVALID;
                goto cleanup;
            }
            LY_CHECK_GOTO(rc = lys_compile_identity_bases(ctx, type_p->pmod, type_p->bases, NULL, &idref->bases), cleanup);
        } else if (base) {
            /* copy all the bases */
            const struct lysc_type_identityref *idref_base = (struct lysc_type_identityref *)base;
            LY_ARRAY_COUNT_TYPE u;

            LY_ARRAY_CREATE_GOTO(ctx->ctx, idref->bases, LY_ARRAY_COUNT(idref_base->bases), rc, cleanup);
            LY_ARRAY_FOR(idref_base->bases, u) {
                idref->bases[u] = idref_base->bases[u];
                LY_ARRAY_INCREMENT(idref->bases);
            }
        } else {
            /* type derived from identityref built-in type must contain at least one base */
            if (tpdfname) {
                LOGVAL(ctx->ctx, LY_VCODE_MISSCHILDSTMT, "base", "identityref type ", tpdfname);
            } else {
                LOGVAL(ctx->ctx, LY_VCODE_MISSCHILDSTMT, "base", "identityref type", "");
            }
            rc = LY_EVALID;
            goto cleanup;
        }
        break;
    case LY_TYPE_LEAFREF:
        lref = (struct lysc_type_leafref *)*type;

        /* RFC 7950 9.9.3 - require-instance */
        if (type_p->flags & LYS_SET_REQINST) {
            if (type_p->pmod->version < LYS_VERSION_1_1) {
                if (tpdfname) {
                    LOGVAL(ctx->ctx, LYVE_SEMANTICS,
                            "Leafref type \"%s\" can be restricted by require-instance statement only in YANG 1.1 modules.", tpdfname);
                } else {
                    LOGVAL(ctx->ctx, LYVE_SEMANTICS,
                            "Leafref type can be restricted by require-instance statement only in YANG 1.1 modules.");
                }
                rc = LY_EVALID;
                goto cleanup;
            }
            lref->require_instance = type_p->require_instance;
        } else if (base) {
            /* inherit */
            lref->require_instance = ((struct lysc_type_leafref *)base)->require_instance;
        } else {
            /* default is true */
            lref->require_instance = 1;
        }
        if (type_p->path) {
            LY_VALUE_FORMAT format;

            LY_CHECK_GOTO(rc = lyxp_expr_dup(ctx->ctx, type_p->path, 0, 0, &lref->path), cleanup);
            LY_CHECK_GOTO(lyplg_type_prefix_data_new(ctx->ctx, type_p->path->expr, strlen(type_p->path->expr),
                    LY_VALUE_SCHEMA, type_p->pmod, &format, (void **)&lref->prefixes), cleanup);
        } else if (base) {
            LY_CHECK_GOTO(rc = lyxp_expr_dup(ctx->ctx, ((struct lysc_type_leafref *)base)->path, 0, 0, &lref->path), cleanup);
            LY_CHECK_GOTO(rc = lyplg_type_prefix_data_dup(ctx->ctx, LY_VALUE_SCHEMA_RESOLVED,
                    ((struct lysc_type_leafref *)base)->prefixes, (void **)&lref->prefixes), cleanup);
        } else {
            /* type derived from leafref built-in type must contain path */
            if (tpdfname) {
                LOGVAL(ctx->ctx, LY_VCODE_MISSCHILDSTMT, "path", "leafref type ", tpdfname);
            } else {
                LOGVAL(ctx->ctx, LY_VCODE_MISSCHILDSTMT, "path", "leafref type", "");
            }
            rc = LY_EVALID;
            goto cleanup;
        }
        break;
    case LY_TYPE_INST:
        /* RFC 7950 9.9.3 - require-instance */
        if (type_p->flags & LYS_SET_REQINST) {
            ((struct lysc_type_instanceid *)*type)->require_instance = type_p->require_instance;
        } else {
            /* default is true */
            ((struct lysc_type_instanceid *)*type)->require_instance = 1;
        }
        break;
    case LY_TYPE_UNION:
        un = (struct lysc_type_union *)*type;

        /* RFC 7950 7.4 - type */
        if (type_p->types) {
            if (base) {
                /* only the directly derived union can contain types specification */
                if (tpdfname) {
                    LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                            "Invalid type substatement for the type \"%s\" not directly derived from union built-in type.",
                            tpdfname);
                } else {
                    LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG,
                            "Invalid type substatement for the type not directly derived from union built-in type.");
                }
                rc = LY_EVALID;
                goto cleanup;
            }
            /* compile the type */
            LY_CHECK_GOTO(rc = lys_compile_type_union(ctx, type_p->types, context_pnode, context_flags, context_name,
                    &un->types), cleanup);
        } else if (base) {
            /* copy all the types */
            const struct lysc_type_union *un_base = (struct lysc_type_union *)base;
            LY_ARRAY_COUNT_TYPE u;

            LY_ARRAY_CREATE_GOTO(ctx->ctx, un->types, LY_ARRAY_COUNT(un_base->types), rc, cleanup);
            LY_ARRAY_FOR(un_base->types, u) {
                un->types[u] = un_base->types[u];
                LY_ATOMIC_INC_BARRIER(un->types[u]->refcount);
                LY_ARRAY_INCREMENT(un->types);
            }
        } else {
            /* type derived from union built-in type must contain at least one type */
            if (tpdfname) {
                LOGVAL(ctx->ctx, LY_VCODE_MISSCHILDSTMT, "type", "union type ", tpdfname);
            } else {
                LOGVAL(ctx->ctx, LY_VCODE_MISSCHILDSTMT, "type", "union type", "");
            }
            rc = LY_EVALID;
            goto cleanup;
        }
        break;
    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
    case LY_TYPE_UNKNOWN: /* just to complete switch */
        break;
    }

    if (tpdf_chain->count > tpdf_chain_last) {
        i = tpdf_chain->count;
        do {
            --i;
            tpdf_item = tpdf_chain->objs[i];

            /* compile previous typedefs extensions */
            COMPILE_EXTS_GOTO(ctx, tpdf_item->tpdf->type.exts, (*type)->exts, *type, rc, cleanup);
        } while (i > tpdf_chain_last);
    }

    /* compile new parsed extensions */
    COMPILE_EXTS_GOTO(ctx, type_p->exts, (*type)->exts, *type, rc, cleanup);

cleanup:
    if (rc) {
        LY_ATOMIC_INC_BARRIER((*type)->refcount);
        lysc_type_free(&ctx->free_ctx, *type);
        *type = NULL;
    }
    return rc;
}

LY_ERR
lys_compile_type(struct lysc_ctx *ctx, struct lysp_node *context_pnode, uint16_t context_flags, const char *context_name,
        const struct lysp_type *type_p, struct lysc_type **type, const char **units, struct lysp_qname **dflt)
{
    LY_ERR ret = LY_SUCCESS;
    ly_bool dummyloops = 0, has_leafref;
    struct lys_type_item *tctx, *tctx_prev = NULL, *tctx_iter;
    LY_DATA_TYPE basetype = LY_TYPE_UNKNOWN;
    struct lysc_type *base = NULL;
    struct lysc_type_union *base_un;
    LY_ARRAY_COUNT_TYPE u;
    struct ly_set tpdf_chain = {0};
    struct lyplg_type *plugin;

    *type = NULL;
    if (dflt) {
        *dflt = NULL;
    }

    tctx = calloc(1, sizeof *tctx);
    LY_CHECK_ERR_RET(!tctx, LOGMEM(ctx->ctx), LY_EMEM);
    for (ret = lysp_type_find(type_p->name, context_pnode, type_p->pmod, ctx->ext, &basetype, &tctx->tpdf, &tctx->node);
            ret == LY_SUCCESS;
            ret = lysp_type_find(tctx_prev->tpdf->type.name, tctx_prev->node, tctx_prev->tpdf->type.pmod, ctx->ext,
                    &basetype, &tctx->tpdf, &tctx->node)) {
        if (basetype) {
            break;
        }

        /* check status */
        ret = lysc_check_status(ctx, context_flags, (void *)type_p->pmod, context_name, tctx->tpdf->flags,
                (void *)tctx->tpdf->type.pmod, tctx->node ? tctx->node->name : tctx->tpdf->name);
        LY_CHECK_ERR_GOTO(ret, free(tctx), cleanup);

        if (units && !*units) {
            /* inherit units */
            DUP_STRING(ctx->ctx, tctx->tpdf->units, *units, ret);
            LY_CHECK_ERR_GOTO(ret, free(tctx), cleanup);
        }
        if (dflt && !*dflt && tctx->tpdf->dflt.str) {
            /* inherit default */
            *dflt = (struct lysp_qname *)&tctx->tpdf->dflt;
        }
        if (dummyloops && (!units || *units) && dflt && *dflt) {
            basetype = ((struct lys_type_item *)tpdf_chain.objs[tpdf_chain.count - 1])->tpdf->type.compiled->basetype;
            break;
        }

        if (tctx->tpdf->type.compiled && (tctx->tpdf->type.compiled->refcount == 1)) {
            /* context recompilation - everything was freed previously (the only reference is from the parsed type itself)
             * and we need now recompile the type again in the updated context. */
            lysc_type_free(&ctx->free_ctx, tctx->tpdf->type.compiled);
            ((struct lysp_tpdf *)tctx->tpdf)->type.compiled = NULL;
        }

        if (tctx->tpdf->type.compiled) {
            /* it is not necessary to continue, the rest of the chain was already compiled,
             * but we still may need to inherit default and units values, so start dummy loops */
            basetype = tctx->tpdf->type.compiled->basetype;
            ret = ly_set_add(&tpdf_chain, tctx, 1, NULL);
            LY_CHECK_ERR_GOTO(ret, free(tctx), cleanup);

            if ((units && !*units) || (dflt && !*dflt)) {
                dummyloops = 1;
                goto preparenext;
            } else {
                tctx = NULL;
                break;
            }
        }

        /* circular typedef reference detection */
        for (uint32_t u = 0; u < tpdf_chain.count; u++) {
            /* local part */
            tctx_iter = (struct lys_type_item *)tpdf_chain.objs[u];
            if (tctx_iter->tpdf == tctx->tpdf) {
                LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid \"%s\" type reference - circular chain of types detected.",
                        tctx->tpdf->name);
                free(tctx);
                ret = LY_EVALID;
                goto cleanup;
            }
        }
        for (uint32_t u = 0; u < ctx->tpdf_chain.count; u++) {
            /* global part for unions corner case */
            tctx_iter = (struct lys_type_item *)ctx->tpdf_chain.objs[u];
            if (tctx_iter->tpdf == tctx->tpdf) {
                LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid \"%s\" type reference - circular chain of types detected.",
                        tctx->tpdf->name);
                free(tctx);
                ret = LY_EVALID;
                goto cleanup;
            }
        }

        /* store information for the following processing */
        ret = ly_set_add(&tpdf_chain, tctx, 1, NULL);
        LY_CHECK_ERR_GOTO(ret, free(tctx), cleanup);

preparenext:
        /* prepare next loop */
        tctx_prev = tctx;
        tctx = calloc(1, sizeof *tctx);
        LY_CHECK_ERR_RET(!tctx, LOGMEM(ctx->ctx), LY_EMEM);
    }
    free(tctx);

    /* basic checks */
    if (basetype == LY_TYPE_UNKNOWN) {
        LOGVAL(ctx->ctx, LYVE_REFERENCE, "Referenced type \"%s\" not found.",
                tctx_prev ? tctx_prev->tpdf->type.name : type_p->name);
        ret = LY_EVALID;
        goto cleanup;
    }
    if (~type_substmt_map[basetype] & type_p->flags) {
        LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG, "Invalid type restrictions for %s type.", ly_data_type2str[basetype]);
        ret = LY_EVALID;
        goto cleanup;
    }

    /* get restrictions from the referred typedefs */
    for (uint32_t u = tpdf_chain.count - 1; u + 1 > 0; --u) {
        tctx = (struct lys_type_item *)tpdf_chain.objs[u];

        /* remember the typedef context for circular check */
        ret = ly_set_add(&ctx->tpdf_chain, tctx, 1, NULL);
        LY_CHECK_GOTO(ret, cleanup);

        if (tctx->tpdf->type.compiled) {
            /* already compiled */
            base = tctx->tpdf->type.compiled;
            continue;
        }

        /* try to find loaded user type plugins */
        plugin = lyplg_type_plugin_find(ctx->ctx, tctx->tpdf->type.pmod->mod->name, tctx->tpdf->type.pmod->mod->revision,
                tctx->tpdf->name);
        if (!plugin && base) {
            /* use the base type implementation if available */
            plugin = base->plugin;
        }
        if (!plugin) {
            /* use the internal built-in type implementation */
            plugin = lyplg_type_plugin_find(ctx->ctx, "", NULL, ly_data_type2str[basetype]);
        }
        assert(plugin);

        if ((basetype != LY_TYPE_LEAFREF) && (u != tpdf_chain.count - 1) && !tctx->tpdf->type.flags &&
                !tctx->tpdf->type.exts && (plugin == base->plugin)) {
            /* no change, reuse the compiled base */
            ((struct lysp_tpdf *)tctx->tpdf)->type.compiled = base;
            LY_ATOMIC_INC_BARRIER(base->refcount);
            continue;
        }

        if (~type_substmt_map[basetype] & tctx->tpdf->type.flags) {
            LOGVAL(ctx->ctx, LYVE_SYNTAX_YANG, "Invalid type \"%s\" restriction(s) for %s type.",
                    tctx->tpdf->name, ly_data_type2str[basetype]);
            ret = LY_EVALID;
            goto cleanup;
        } else if ((basetype == LY_TYPE_EMPTY) && tctx->tpdf->dflt.str) {
            LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Invalid type \"%s\" - \"empty\" type must not have a default value (%s).",
                    tctx->tpdf->name, tctx->tpdf->dflt.str);
            ret = LY_EVALID;
            goto cleanup;
        }

        /* compile the typedef type */
        ret = lys_compile_type_(ctx, tctx->node, tctx->tpdf->flags, tctx->tpdf->name, &tctx->tpdf->type, basetype,
                tctx->tpdf->name, base, plugin, &tpdf_chain, u + 1, &base);
        LY_CHECK_GOTO(ret, cleanup);

        /* store separately compiled typedef type to be reused */
        ((struct lysp_tpdf *)tctx->tpdf)->type.compiled = base;
        LY_ATOMIC_INC_BARRIER(base->refcount);
    }

    /* remove the processed typedef contexts from the stack for circular check */
    ctx->tpdf_chain.count = ctx->tpdf_chain.count - tpdf_chain.count;

    /* learn whether the type has a leafref, in which case it cannot be shared because it may resolve to a different
     * real type for every instantiation */
    has_leafref = 0;
    if (basetype == LY_TYPE_LEAFREF) {
        /* leafref type */
        has_leafref = 1;
    } else if ((basetype == LY_TYPE_UNION) && base) {
        /* union with a leafref */
        base_un = (struct lysc_type_union *)base;
        LY_ARRAY_FOR(base_un->types, u) {
            if (base_un->types[u]->basetype == LY_TYPE_LEAFREF) {
                has_leafref = 1;
                break;
            }
        }
    }

    /* process the type definition in leaf */
    if (type_p->flags || type_p->exts || !base || has_leafref) {
        /* leaf type has changes that need to be compiled into the type */
        plugin = base ? base->plugin : lyplg_type_plugin_find(ctx->ctx, "", NULL, ly_data_type2str[basetype]);
        ret = lys_compile_type_(ctx, context_pnode, context_flags, context_name, (struct lysp_type *)type_p, basetype,
                NULL, base, plugin, &tpdf_chain, 0, type);
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        /* no changes of the type in the leaf, just use the base compiled type */
        *type = base;
    }

cleanup:
    ly_set_erase(&tpdf_chain, free);
    return ret;
}

/**
 * @brief Check uniqness of the node/action/notification name.
 *
 * Data nodes, actions/RPCs and Notifications are stored separately (in distinguish lists) in the schema
 * structures, but they share the namespace so we need to check their name collisions.
 *
 * @param[in] ctx Compile context.
 * @param[in] parent Parent of the nodes to check, can be NULL.
 * @param[in] name Name of the item to find in the given lists.
 * @param[in] exclude Node that was just added that should be excluded from the name checking.
 * @return LY_SUCCESS in case of unique name, LY_EEXIST otherwise.
 */
static LY_ERR
lys_compile_node_uniqness(struct lysc_ctx *ctx, const struct lysc_node *parent, const char *name,
        const struct lysc_node *exclude)
{
    const struct lysc_node *iter, *iter2, *dup = NULL;
    const struct lysc_node_action *actions;
    const struct lysc_node_notif *notifs;
    uint32_t getnext_flags;
    struct ly_set parent_choices = {0};
    const char *node_type_str = "data definition/RPC/action/notification";
    char *spath;

#define CHECK_NODE(iter, exclude, name) (iter != (void *)exclude && (iter)->module == exclude->module && !strcmp(name, (iter)->name))

    if (exclude->nodetype == LYS_CASE) {
        /* check restricted only to all the cases */
        assert(parent->nodetype == LYS_CHOICE);
        LY_LIST_FOR(lysc_node_child(parent), iter) {
            if (CHECK_NODE(iter, exclude, name)) {
                node_type_str = "case";
                dup = iter;
                goto cleanup;
            }
        }

        return LY_SUCCESS;
    }

    /* no reason for our parent to be choice anymore */
    assert(!parent || (parent->nodetype != LYS_CHOICE));

    if (parent && (parent->nodetype == LYS_CASE)) {
        /* move to the first data definition parent */

        /* but remember the choice nodes on the parents path to avoid believe they collide with our node */
        iter = lysc_data_parent(parent);
        do {
            parent = parent->parent;
            if (parent && (parent->nodetype == LYS_CHOICE)) {
                ly_set_add(&parent_choices, (void *)parent, 1, NULL);
            }
        } while (parent != iter);
    }

    getnext_flags = LYS_GETNEXT_WITHCHOICE;
    if (parent && (parent->nodetype & (LYS_RPC | LYS_ACTION))) {
        /* move to the inout to avoid traversing a not-filled-yet (the other) node */
        if (exclude->flags & LYS_IS_OUTPUT) {
            getnext_flags |= LYS_GETNEXT_OUTPUT;
            parent = lysc_node_child(parent)->next;
        } else {
            parent = lysc_node_child(parent);
        }
    }

    iter = NULL;
    if (!parent && ctx->ext) {
        while ((iter = lys_getnext_ext(iter, parent, ctx->ext, getnext_flags))) {
            if (!ly_set_contains(&parent_choices, (void *)iter, NULL) && CHECK_NODE(iter, exclude, name)) {
                dup = iter;
                goto cleanup;
            }

            /* we must compare with both the choice and all its nested data-definiition nodes (but not recursively) */
            if (iter->nodetype == LYS_CHOICE) {
                iter2 = NULL;
                while ((iter2 = lys_getnext_ext(iter2, iter, NULL, 0))) {
                    if (CHECK_NODE(iter2, exclude, name)) {
                        dup = iter2;
                        goto cleanup;
                    }
                }
            }
        }
    } else {
        while ((iter = lys_getnext(iter, parent, ctx->cur_mod->compiled, getnext_flags))) {
            if (!ly_set_contains(&parent_choices, (void *)iter, NULL) && CHECK_NODE(iter, exclude, name)) {
                dup = iter;
                goto cleanup;
            }

            /* we must compare with both the choice and all its nested data-definiition nodes (but not recursively) */
            if (iter->nodetype == LYS_CHOICE) {
                iter2 = NULL;
                while ((iter2 = lys_getnext(iter2, iter, NULL, 0))) {
                    if (CHECK_NODE(iter2, exclude, name)) {
                        dup = iter2;
                        goto cleanup;
                    }
                }
            }
        }

        actions = parent ? lysc_node_actions(parent) : ctx->cur_mod->compiled->rpcs;
        LY_LIST_FOR((struct lysc_node *)actions, iter) {
            if (CHECK_NODE(iter, exclude, name)) {
                dup = iter;
                goto cleanup;
            }
        }

        notifs = parent ? lysc_node_notifs(parent) : ctx->cur_mod->compiled->notifs;
        LY_LIST_FOR((struct lysc_node *)notifs, iter) {
            if (CHECK_NODE(iter, exclude, name)) {
                dup = iter;
                goto cleanup;
            }
        }
    }

cleanup:
    ly_set_erase(&parent_choices, NULL);
    if (dup) {
        spath = lysc_path(dup, LYSC_PATH_LOG, NULL, 0);
        LOGVAL(ctx->ctx, LY_VCODE_DUPIDENT, spath, node_type_str);
        free(spath);
        return LY_EEXIST;
    }
    return LY_SUCCESS;

#undef CHECK_NODE
}

LY_ERR
lys_compile_node_connect(struct lysc_ctx *ctx, struct lysc_node *parent, struct lysc_node *node)
{
    struct lysc_node **children, *anchor = NULL;
    int insert_after = 0;

    node->parent = parent;

    if (parent) {
        if (node->nodetype == LYS_INPUT) {
            assert(parent->nodetype & (LYS_ACTION | LYS_RPC));
            /* input node is part of the action but link it with output */
            node->next = &((struct lysc_node_action *)parent)->output.node;
            node->prev = node->next;
            return LY_SUCCESS;
        } else if (node->nodetype == LYS_OUTPUT) {
            /* output node is part of the action but link it with input */
            node->next = NULL;
            node->prev = &((struct lysc_node_action *)parent)->input.node;
            return LY_SUCCESS;
        } else if (node->nodetype == LYS_ACTION) {
            children = (struct lysc_node **)lysc_node_actions_p(parent);
        } else if (node->nodetype == LYS_NOTIF) {
            children = (struct lysc_node **)lysc_node_notifs_p(parent);
        } else {
            children = lysc_node_child_p(parent);
        }
        assert(children);

        if (!(*children)) {
            /* first child */
            *children = node;
        } else if (node->flags & LYS_KEY) {
            /* special handling of adding keys */
            assert(node->module == parent->module);
            anchor = *children;
            if (anchor->flags & LYS_KEY) {
                while ((anchor->flags & LYS_KEY) && anchor->next) {
                    anchor = anchor->next;
                }
                /* insert after the last key */
                insert_after = 1;
            } /* else insert before anchor (at the beginning) */
        } else if ((*children)->prev->module == node->module) {
            /* last child is from the same module, keep the order and insert at the end */
            anchor = (*children)->prev;
            insert_after = 1;
        } else if (parent->module == node->module) {
            /* adding module child after some augments were connected */
            for (anchor = *children; anchor->module == node->module; anchor = anchor->next) {}
        } else {
            /* some augments are already connected and we are connecting new ones,
             * keep module name order and insert the node into the children list */
            anchor = *children;
            do {
                anchor = anchor->prev;

                /* check that we have not found the last augment node from our module or
                 * the first augment node from a "smaller" module or
                 * the first node from a local module */
                if ((anchor->module == node->module) || (strcmp(anchor->module->name, node->module->name) < 0) ||
                        (anchor->module == parent->module)) {
                    /* insert after */
                    insert_after = 1;
                    break;
                }

                /* we have traversed all the nodes, insert before anchor (as the first node) */
            } while (anchor->prev->next);
        }

        /* insert */
        if (anchor) {
            if (insert_after) {
                node->next = anchor->next;
                node->prev = anchor;
                anchor->next = node;
                if (node->next) {
                    /* middle node */
                    node->next->prev = node;
                } else {
                    /* last node */
                    (*children)->prev = node;
                }
            } else {
                node->next = anchor;
                node->prev = anchor->prev;
                anchor->prev = node;
                if (anchor == *children) {
                    /* first node */
                    *children = node;
                } else {
                    /* middle node */
                    node->prev->next = node;
                }
            }
        }

        /* check the name uniqueness (even for an only child, it may be in case) */
        if (lys_compile_node_uniqness(ctx, parent, node->name, node)) {
            return LY_EEXIST;
        }
    } else {
        /* top-level element */
        struct lysc_node **list;

        if (ctx->ext) {
            lyplg_ext_get_storage_p(ctx->ext, LY_STMT_DATA_NODE_MASK, (const void ***)&list);
        } else if (node->nodetype == LYS_RPC) {
            list = (struct lysc_node **)&ctx->cur_mod->compiled->rpcs;
        } else if (node->nodetype == LYS_NOTIF) {
            list = (struct lysc_node **)&ctx->cur_mod->compiled->notifs;
        } else {
            list = &ctx->cur_mod->compiled->data;
        }
        if (!(*list)) {
            *list = node;
        } else {
            /* insert at the end of the module's top-level nodes list */
            (*list)->prev->next = node;
            node->prev = (*list)->prev;
            (*list)->prev = node;
        }

        /* check the name uniqueness on top-level */
        if (lys_compile_node_uniqness(ctx, NULL, node->name, node)) {
            return LY_EEXIST;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Set config and operation flags for a node.
 *
 * @param[in] ctx Compile context.
 * @param[in] node Compiled node flags to set.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_config(struct lysc_ctx *ctx, struct lysc_node *node)
{
    /* case never has any explicit config */
    assert((node->nodetype != LYS_CASE) || !(node->flags & LYS_CONFIG_MASK));

    if (ctx->compile_opts & LYS_COMPILE_NO_CONFIG) {
        /* ignore config statements inside Notification/RPC/action/... data */
        node->flags &= ~LYS_CONFIG_MASK;
    } else if (!(node->flags & LYS_CONFIG_MASK)) {
        /* config not explicitly set, inherit it from parent */
        assert(!node->parent || (node->parent->flags & LYS_CONFIG_MASK) || (node->parent->nodetype & LYS_AUGMENT));
        if (node->parent && (node->parent->flags & LYS_CONFIG_MASK)) {
            node->flags |= node->parent->flags & LYS_CONFIG_MASK;
        } else {
            /* default is config true */
            node->flags |= LYS_CONFIG_W;
        }
    } else {
        /* config set explicitly */
        node->flags |= LYS_SET_CONFIG;
    }

    if (node->parent && (node->parent->flags & LYS_CONFIG_R) && (node->flags & LYS_CONFIG_W)) {
        LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Configuration node cannot be child of any state data node.");
        return LY_EVALID;
    }

    return LY_SUCCESS;
}

/**
 * @brief Set various flags of the compiled nodes
 *
 * @param[in] ctx Compile context.
 * @param[in] parsed_flags Parsed node flags.
 * @param[in] inherited_flags Inherited flags from a schema-only statement.
 * @param[in,out] node Compiled node where the flags will be set.
 */
static LY_ERR
lys_compile_node_flags(struct lysc_ctx *ctx, uint16_t parsed_flags, uint16_t inherited_flags, struct lysc_node *node)
{
    uint16_t parent_flags;
    const char *parent_name;

    /* copy flags except for status */
    node->flags = (parsed_flags & LYS_FLAGS_COMPILED_MASK) & ~LYS_STATUS_MASK;

    /* inherit config flags */
    LY_CHECK_RET(lys_compile_config(ctx, node));

    /* compile status */
    parent_flags = node->parent ? node->parent->flags : 0;
    parent_name = node->parent ? node->parent->name : NULL;
    LY_CHECK_RET(lys_compile_status(ctx, parsed_flags, inherited_flags, parent_flags, parent_name, node->name, &node->flags));

    /* other flags */
    if ((ctx->compile_opts & LYS_IS_INPUT) && (node->nodetype != LYS_INPUT)) {
        node->flags |= LYS_IS_INPUT;
    } else if ((ctx->compile_opts & LYS_IS_OUTPUT) && (node->nodetype != LYS_OUTPUT)) {
        node->flags |= LYS_IS_OUTPUT;
    } else if ((ctx->compile_opts & LYS_IS_NOTIF) && (node->nodetype != LYS_NOTIF)) {
        node->flags |= LYS_IS_NOTIF;
    }

    return LY_SUCCESS;
}

static LY_ERR
lys_compile_node_(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *parent, uint16_t inherited_flags,
        LY_ERR (*node_compile_spec)(struct lysc_ctx *, struct lysp_node *, struct lysc_node *),
        struct lysc_node *node, struct ly_set *child_set)
{
    LY_ERR ret = LY_SUCCESS;
    ly_bool not_supported, enabled;
    struct lysp_node *dev_pnode = NULL;
    struct lysp_when *pwhen = NULL;
    uint32_t prev_opts = ctx->compile_opts;

    node->nodetype = pnode->nodetype;
    node->module = ctx->cur_mod;
    node->parent = parent;
    node->prev = node;
    node->priv = ctx->ctx->flags & LY_CTX_SET_PRIV_PARSED ? pnode : NULL;

    /* compile any deviations for this node */
    LY_CHECK_GOTO(ret = lys_compile_node_deviations_refines(ctx, pnode, parent, &dev_pnode, &not_supported), error);
    if (not_supported && !(ctx->compile_opts & (LYS_COMPILE_NO_DISABLED | LYS_COMPILE_DISABLED | LYS_COMPILE_GROUPING))) {
        /* if not supported, keep it just like disabled nodes by if-feature */
        ly_set_add(&ctx->unres->disabled, node, 1, NULL);
        ctx->compile_opts |= LYS_COMPILE_DISABLED;
    }
    if (dev_pnode) {
        pnode = dev_pnode;
    }

    DUP_STRING_GOTO(ctx->ctx, pnode->name, node->name, ret, error);
    DUP_STRING_GOTO(ctx->ctx, pnode->dsc, node->dsc, ret, error);
    DUP_STRING_GOTO(ctx->ctx, pnode->ref, node->ref, ret, error);

    /* if-features */
    LY_CHECK_GOTO(ret = lys_eval_iffeatures(ctx->ctx, pnode->iffeatures, &enabled), error);
    if (!enabled && !(ctx->compile_opts & (LYS_COMPILE_NO_DISABLED | LYS_COMPILE_DISABLED | LYS_COMPILE_GROUPING))) {
        ly_set_add(&ctx->unres->disabled, node, 1, NULL);
        ctx->compile_opts |= LYS_COMPILE_DISABLED;
    }

    /* config, status and other flags */
    LY_CHECK_GOTO(ret = lys_compile_node_flags(ctx, pnode->flags, inherited_flags, node), error);

    /* list ordering */
    if (node->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
        if ((node->flags & (LYS_CONFIG_R | LYS_IS_OUTPUT | LYS_IS_NOTIF)) && (node->flags & LYS_ORDBY_MASK)) {
            node->flags &= ~LYS_ORDBY_MASK;
            LOGVRB("The ordered-by statement is ignored in lists representing %s (%s).",
                    (node->flags & LYS_IS_OUTPUT) ? "RPC/action output parameters" :
                    (ctx->compile_opts & LYS_IS_NOTIF) ? "notification content" : "state data", ctx->path);
        }
        if (node->flags & (LYS_IS_OUTPUT | LYS_IS_NOTIF | LYS_CONFIG_R)) {
            /* it is probably better not to order them */
            node->flags |= LYS_ORDBY_USER;
        } else if (!(node->flags & LYS_ORDBY_MASK)) {
            /* default ordering is system */
            node->flags |= LYS_ORDBY_SYSTEM;
        }
    }

    /* insert into parent's children/compiled module (we can no longer free the node separately on error) */
    LY_CHECK_GOTO(ret = lys_compile_node_connect(ctx, parent, node), cleanup);

    if ((pwhen = lysp_node_when(pnode))) {
        /* compile when */
        ret = lys_compile_when(ctx, pwhen, pnode->flags, node, lysc_data_node(node), node, NULL);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* nodetype-specific part */
    LY_CHECK_GOTO(ret = node_compile_spec(ctx, pnode, node), cleanup);

    /* final compilation tasks that require the node to be connected */
    COMPILE_EXTS_GOTO(ctx, pnode->exts, node->exts, node, ret, cleanup);
    if (node->flags & LYS_MAND_TRUE) {
        /* inherit LYS_MAND_TRUE in parent containers */
        lys_compile_mandatory_parents(parent, 1);
    }

    if (child_set) {
        /* add the new node into set */
        LY_CHECK_GOTO(ret = ly_set_add(child_set, node, 1, NULL), cleanup);
    }

    goto cleanup;

error:
    lysc_node_free(&ctx->free_ctx, node, 0);

cleanup:
    if (ret && dev_pnode) {
        LOGVAL(ctx->ctx, LYVE_OTHER, "Compilation of a deviated and/or refined node failed.");
    }
    ctx->compile_opts = prev_opts;
    lysp_dev_node_free(ctx, dev_pnode);
    return ret;
}

LY_ERR
lys_compile_node_action_inout(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysp_node *child_p;
    uint32_t prev_options = ctx->compile_opts;

    struct lysp_node_action_inout *inout_p = (struct lysp_node_action_inout *)pnode;
    struct lysc_node_action_inout *inout = (struct lysc_node_action_inout *)node;

    COMPILE_ARRAY_GOTO(ctx, inout_p->musts, inout->musts, lys_compile_must, ret, done);
    COMPILE_EXTS_GOTO(ctx, inout_p->exts, inout->exts, inout, ret, done);
    ctx->compile_opts |= (inout_p->nodetype == LYS_INPUT) ? LYS_COMPILE_RPC_INPUT : LYS_COMPILE_RPC_OUTPUT;

    LY_LIST_FOR(inout_p->child, child_p) {
        LY_CHECK_GOTO(ret = lys_compile_node(ctx, child_p, node, 0, NULL), done);
    }

    /* connect any augments */
    LY_CHECK_GOTO(ret = lys_compile_node_augments(ctx, node), done);

    ctx->compile_opts = prev_options;

done:
    return ret;
}

/**
 * @brief Compile parsed action node information.
 *
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed action node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the action-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_action(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    LY_ERR ret;
    struct lysp_node_action *action_p = (struct lysp_node_action *)pnode;
    struct lysc_node_action *action = (struct lysc_node_action *)node;
    struct lysp_node_action_inout *input, implicit_input = {
        .nodetype = LYS_INPUT,
        .name = "input",
        .parent = pnode,
    };
    struct lysp_node_action_inout *output, implicit_output = {
        .nodetype = LYS_OUTPUT,
        .name = "output",
        .parent = pnode,
    };

    /* input */
    lysc_update_path(ctx, action->module, "input");
    if (action_p->input.nodetype == LYS_UNKNOWN) {
        input = &implicit_input;
    } else {
        input = &action_p->input;
    }
    ret = lys_compile_node_(ctx, &input->node, &action->node, 0, lys_compile_node_action_inout, &action->input.node, NULL);
    lysc_update_path(ctx, NULL, NULL);
    LY_CHECK_GOTO(ret, done);

    /* add must(s) to unres */
    ret = lysc_unres_must_add(ctx, &action->input.node, &input->node);
    LY_CHECK_GOTO(ret, done);

    /* output */
    lysc_update_path(ctx, action->module, "output");
    if (action_p->output.nodetype == LYS_UNKNOWN) {
        output = &implicit_output;
    } else {
        output = &action_p->output;
    }
    ret = lys_compile_node_(ctx, &output->node, &action->node, 0, lys_compile_node_action_inout, &action->output.node, NULL);
    lysc_update_path(ctx, NULL, NULL);
    LY_CHECK_GOTO(ret, done);

    /* add must(s) to unres */
    ret = lysc_unres_must_add(ctx, &action->output.node, &output->node);
    LY_CHECK_GOTO(ret, done);

done:
    return ret;
}

/**
 * @brief Compile parsed action node information.
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed action node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the action-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_notif(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysp_node_notif *notif_p = (struct lysp_node_notif *)pnode;
    struct lysc_node_notif *notif = (struct lysc_node_notif *)node;
    struct lysp_node *child_p;

    COMPILE_ARRAY_GOTO(ctx, notif_p->musts, notif->musts, lys_compile_must, ret, done);

    /* add must(s) to unres */
    ret = lysc_unres_must_add(ctx, node, pnode);
    LY_CHECK_GOTO(ret, done);

    LY_LIST_FOR(notif_p->child, child_p) {
        ret = lys_compile_node(ctx, child_p, node, 0, NULL);
        LY_CHECK_GOTO(ret, done);
    }

    /* connect any augments */
    LY_CHECK_GOTO(ret = lys_compile_node_augments(ctx, node), done);

done:
    return ret;
}

/**
 * @brief Compile parsed container node information.
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed container node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the container-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_container(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    struct lysp_node_container *cont_p = (struct lysp_node_container *)pnode;
    struct lysc_node_container *cont = (struct lysc_node_container *)node;
    struct lysp_node *child_p;
    LY_ERR ret = LY_SUCCESS;

    if (cont_p->presence) {
        /* presence container */
        cont->flags |= LYS_PRESENCE;
    }

    /* more cases when the container has meaning but is kept NP for convenience:
     *   - when condition
     *   - direct child action/notification
     */

    LY_LIST_FOR(cont_p->child, child_p) {
        ret = lys_compile_node(ctx, child_p, node, 0, NULL);
        LY_CHECK_GOTO(ret, done);
    }

    COMPILE_ARRAY_GOTO(ctx, cont_p->musts, cont->musts, lys_compile_must, ret, done);

    /* add must(s) to unres */
    ret = lysc_unres_must_add(ctx, node, pnode);
    LY_CHECK_GOTO(ret, done);

    /* connect any augments */
    LY_CHECK_GOTO(ret = lys_compile_node_augments(ctx, node), done);

    LY_LIST_FOR((struct lysp_node *)cont_p->actions, child_p) {
        ret = lys_compile_node(ctx, child_p, node, 0, NULL);
        LY_CHECK_GOTO(ret, done);
    }
    LY_LIST_FOR((struct lysp_node *)cont_p->notifs, child_p) {
        ret = lys_compile_node(ctx, child_p, node, 0, NULL);
        LY_CHECK_GOTO(ret, done);
    }

done:
    return ret;
}

/**
 * @brief Compile type in leaf/leaf-list node and do all the necessary checks.
 * @param[in] ctx Compile context.
 * @param[in] context_node Schema node where the type/typedef is placed to correctly find the base types.
 * @param[in] type_p Parsed type to compile.
 * @param[in,out] leaf Compiled leaf structure (possibly cast leaf-list) to provide node information and to store the compiled type information.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_node_type(struct lysc_ctx *ctx, struct lysp_node *context_node, struct lysp_type *type_p,
        struct lysc_node_leaf *leaf)
{
    struct lysp_qname *dflt;
    struct lysc_type **t;
    LY_ARRAY_COUNT_TYPE u, count;
    ly_bool in_unres = 0;

    LY_CHECK_RET(lys_compile_type(ctx, context_node, leaf->flags, leaf->name, type_p, &leaf->type,
            leaf->units ? NULL : &leaf->units, &dflt));
    LY_ATOMIC_INC_BARRIER(leaf->type->refcount);

    /* store default value, if any */
    if (dflt && !(leaf->flags & LYS_SET_DFLT)) {
        LY_CHECK_RET(lysc_unres_leaf_dflt_add(ctx, leaf, dflt));
    }

    /* store leafref(s) to be resolved */
    LY_CHECK_RET(lysc_unres_leafref_add(ctx, leaf, type_p->pmod));

    /* type-specific checks */
    if (leaf->type->basetype == LY_TYPE_UNION) {
        t = ((struct lysc_type_union *)leaf->type)->types;
        count = LY_ARRAY_COUNT(t);
    } else {
        t = &leaf->type;
        count = 1;
    }
    for (u = 0; u < count; ++u) {
        if (t[u]->basetype == LY_TYPE_EMPTY) {
            if ((leaf->nodetype == LYS_LEAFLIST) && (ctx->pmod->version < LYS_VERSION_1_1)) {
                LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Leaf-list of type \"empty\" is allowed only in YANG 1.1 modules.");
                return LY_EVALID;
            }
        } else if (!in_unres && ((t[u]->basetype == LY_TYPE_BITS) || (t[u]->basetype == LY_TYPE_ENUM))) {
            /* store in unres for all disabled bits/enums to be removed */
            LY_CHECK_RET(lysc_unres_bitenum_add(ctx, leaf));
            in_unres = 1;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Compile parsed leaf node information.
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed leaf node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the leaf-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_leaf(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    struct lysp_node_leaf *leaf_p = (struct lysp_node_leaf *)pnode;
    struct lysc_node_leaf *leaf = (struct lysc_node_leaf *)node;
    LY_ERR ret = LY_SUCCESS;

    COMPILE_ARRAY_GOTO(ctx, leaf_p->musts, leaf->musts, lys_compile_must, ret, done);

    /* add must(s) to unres */
    ret = lysc_unres_must_add(ctx, node, pnode);
    LY_CHECK_GOTO(ret, done);

    if (leaf_p->units) {
        LY_CHECK_GOTO(ret = lydict_insert(ctx->ctx, leaf_p->units, 0, &leaf->units), done);
        leaf->flags |= LYS_SET_UNITS;
    }

    /* compile type */
    ret = lys_compile_node_type(ctx, pnode, &leaf_p->type, leaf);
    LY_CHECK_GOTO(ret, done);

    /* store/update default value */
    if (leaf_p->dflt.str) {
        LY_CHECK_RET(lysc_unres_leaf_dflt_add(ctx, leaf, &leaf_p->dflt));
        leaf->flags |= LYS_SET_DFLT;
    }

    /* checks */
    if ((leaf->flags & LYS_SET_DFLT) && (leaf->flags & LYS_MAND_TRUE)) {
        LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Invalid mandatory leaf with a default value.");
        return LY_EVALID;
    }

done:
    return ret;
}

/**
 * @brief Compile parsed leaf-list node information.
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed leaf-list node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the leaf-list-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_leaflist(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    struct lysp_node_leaflist *llist_p = (struct lysp_node_leaflist *)pnode;
    struct lysc_node_leaflist *llist = (struct lysc_node_leaflist *)node;
    LY_ERR ret = LY_SUCCESS;

    COMPILE_ARRAY_GOTO(ctx, llist_p->musts, llist->musts, lys_compile_must, ret, done);

    /* add must(s) to unres */
    ret = lysc_unres_must_add(ctx, node, pnode);
    LY_CHECK_GOTO(ret, done);

    if (llist_p->units) {
        LY_CHECK_GOTO(ret = lydict_insert(ctx->ctx, llist_p->units, 0, &llist->units), done);
        llist->flags |= LYS_SET_UNITS;
    }

    /* compile type */
    ret = lys_compile_node_type(ctx, pnode, &llist_p->type, (struct lysc_node_leaf *)llist);
    LY_CHECK_GOTO(ret, done);

    /* store/update default values */
    if (llist_p->dflts) {
        if (ctx->pmod->version < LYS_VERSION_1_1) {
            LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Leaf-list default values are allowed only in YANG 1.1 modules.");
            return LY_EVALID;
        }

        LY_CHECK_GOTO(lysc_unres_llist_dflts_add(ctx, llist, llist_p->dflts), done);
        llist->flags |= LYS_SET_DFLT;
    }

    llist->min = llist_p->min;
    if (llist->min) {
        llist->flags |= LYS_MAND_TRUE;
    }
    llist->max = llist_p->max ? llist_p->max : UINT32_MAX;

    if (llist->flags & LYS_CONFIG_R) {
        /* state leaf-list is always ordered-by user */
        llist->flags &= ~LYS_ORDBY_SYSTEM;
        llist->flags |= LYS_ORDBY_USER;
    }

    /* checks */
    if ((llist->flags & LYS_SET_DFLT) && (llist->flags & LYS_MAND_TRUE)) {
        LOGVAL(ctx->ctx, LYVE_SEMANTICS, "The default statement is present on leaf-list with a nonzero min-elements.");
        return LY_EVALID;
    }

    if (llist->min > llist->max) {
        LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Leaf-list min-elements %" PRIu32 " is bigger than max-elements %" PRIu32 ".",
                llist->min, llist->max);
        return LY_EVALID;
    }

done:
    return ret;
}

/**
 * @brief Find the node according to the given descendant/absolute schema nodeid.
 * Used in unique, refine and augment statements.
 *
 * @param[in] ctx Compile context
 * @param[in] nodeid Descendant-schema-nodeid (according to the YANG grammar)
 * @param[in] nodeid_len Length of the given nodeid, if it is not NULL-terminated string.
 * @param[in] ctx_node Context node for a relative nodeid.
 * @param[in] format Format of any prefixes.
 * @param[in] prefix_data Format-specific prefix data (see ::ly_resolve_prefix).
 * @param[in] nodetype Optional (can be 0) restriction for target's nodetype. If target exists, but does not match
 * the given nodetype, LY_EDENIED is returned (and target is provided), but no error message is printed.
 * The value can be even an ORed value to allow multiple nodetypes.
 * @param[out] target Found target node if any.
 * @param[out] result_flag Output parameter to announce if the schema nodeid goes through the action's input/output or a Notification.
 * The LYSC_OPT_RPC_INPUT, LYSC_OPT_RPC_OUTPUT and LYSC_OPT_NOTIFICATION are used as flags.
 * @return LY_ERR values - LY_ENOTFOUND, LY_EVALID, LY_EDENIED or LY_SUCCESS.
 */
static LY_ERR
lysc_resolve_schema_nodeid(struct lysc_ctx *ctx, const char *nodeid, size_t nodeid_len, const struct lysc_node *ctx_node,
        LY_VALUE_FORMAT format, void *prefix_data, uint16_t nodetype, const struct lysc_node **target, uint16_t *result_flag)
{
    LY_ERR ret = LY_EVALID;
    const char *name, *prefix, *id;
    size_t name_len, prefix_len;
    const struct lys_module *mod = NULL;
    const char *nodeid_type;
    uint32_t getnext_extra_flag = 0;
    uint16_t current_nodetype = 0;

    assert(nodeid);
    assert(target);
    assert(result_flag);
    *target = NULL;
    *result_flag = 0;

    id = nodeid;

    if (ctx_node) {
        /* descendant-schema-nodeid */
        nodeid_type = "descendant";

        if (*id == '/') {
            LOGVAL(ctx->ctx, LYVE_REFERENCE,
                    "Invalid descendant-schema-nodeid value \"%.*s\" - absolute-schema-nodeid used.",
                    (int)(nodeid_len ? nodeid_len : strlen(nodeid)), nodeid);
            return LY_EVALID;
        }
    } else {
        /* absolute-schema-nodeid */
        nodeid_type = "absolute";

        if (*id != '/') {
            LOGVAL(ctx->ctx, LYVE_REFERENCE,
                    "Invalid absolute-schema-nodeid value \"%.*s\" - missing starting \"/\".",
                    (int)(nodeid_len ? nodeid_len : strlen(nodeid)), nodeid);
            return LY_EVALID;
        }
        ++id;
    }

    while (*id && (ret = ly_parse_nodeid(&id, &prefix, &prefix_len, &name, &name_len)) == LY_SUCCESS) {
        if (prefix) {
            mod = ly_resolve_prefix(ctx->ctx, prefix, prefix_len, format, prefix_data);
            if (!mod) {
                /* module must always be found */
                assert(prefix);
                LOGVAL(ctx->ctx, LYVE_REFERENCE,
                        "Invalid %s-schema-nodeid value \"%.*s\" - prefix \"%.*s\" not defined in module \"%s\".",
                        nodeid_type, (int)(id - nodeid), nodeid, (int)prefix_len, prefix, LYSP_MODULE_NAME(ctx->pmod));
                return LY_ENOTFOUND;
            }
        } else {
            switch (format) {
            case LY_VALUE_SCHEMA:
            case LY_VALUE_SCHEMA_RESOLVED:
                /* use the current module */
                mod = ctx->cur_mod;
                break;
            case LY_VALUE_JSON:
            case LY_VALUE_LYB:
                if (!ctx_node) {
                    LOGINT_RET(ctx->ctx);
                }
                /* inherit the module of the previous context node */
                mod = ctx_node->module;
                break;
            case LY_VALUE_CANON:
            case LY_VALUE_XML:
            case LY_VALUE_STR_NS:
                /* not really defined */
                LOGINT_RET(ctx->ctx);
            }
        }

        if (ctx_node && (ctx_node->nodetype & (LYS_RPC | LYS_ACTION))) {
            /* move through input/output manually */
            if (mod != ctx_node->module) {
                LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid %s-schema-nodeid value \"%.*s\" - target node not found.",
                        nodeid_type, (int)(id - nodeid), nodeid);
                return LY_ENOTFOUND;
            }
            if (!ly_strncmp("input", name, name_len)) {
                ctx_node = &((struct lysc_node_action *)ctx_node)->input.node;
            } else if (!ly_strncmp("output", name, name_len)) {
                ctx_node = &((struct lysc_node_action *)ctx_node)->output.node;
                getnext_extra_flag = LYS_GETNEXT_OUTPUT;
            } else {
                /* only input or output is valid */
                ctx_node = NULL;
            }
        } else if (ctx->ext && !ctx_node) {
            /* top-level extension nodes */
            ctx_node = lysc_ext_find_node(ctx->ext, mod, name, name_len, 0, LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHCASE);
        } else {
            ctx_node = lys_find_child(ctx_node, mod, name, name_len, 0,
                    getnext_extra_flag | LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHCASE);
            getnext_extra_flag = 0;
        }
        if (!ctx_node) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid %s-schema-nodeid value \"%.*s\" - target node not found.",
                    nodeid_type, (int)(id - nodeid), nodeid);
            return LY_ENOTFOUND;
        }
        current_nodetype = ctx_node->nodetype;

        if (current_nodetype == LYS_NOTIF) {
            (*result_flag) |= LYS_COMPILE_NOTIFICATION;
        } else if (current_nodetype == LYS_INPUT) {
            (*result_flag) |= LYS_COMPILE_RPC_INPUT;
        } else if (current_nodetype == LYS_OUTPUT) {
            (*result_flag) |= LYS_COMPILE_RPC_OUTPUT;
        }

        if (!*id || (nodeid_len && ((size_t)(id - nodeid) >= nodeid_len))) {
            break;
        }
        if (*id != '/') {
            LOGVAL(ctx->ctx, LYVE_REFERENCE,
                    "Invalid %s-schema-nodeid value \"%.*s\" - missing \"/\" as node-identifier separator.",
                    nodeid_type, (int)(id - nodeid + 1), nodeid);
            return LY_EVALID;
        }
        ++id;
    }

    if (ret == LY_SUCCESS) {
        *target = ctx_node;
        if (nodetype && !(current_nodetype & nodetype)) {
            return LY_EDENIED;
        }
    } else {
        LOGVAL(ctx->ctx, LYVE_REFERENCE,
                "Invalid %s-schema-nodeid value \"%.*s\" - unexpected end of expression.",
                nodeid_type, (int)(nodeid_len ? nodeid_len : strlen(nodeid)), nodeid);
    }

    return ret;
}

/**
 * @brief Compile information about list's uniques.
 * @param[in] ctx Compile context.
 * @param[in] uniques Sized array list of unique statements.
 * @param[in] list Compiled list where the uniques are supposed to be resolved and stored.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_node_list_unique(struct lysc_ctx *ctx, struct lysp_qname *uniques, struct lysc_node_list *list)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_node_leaf **key, ***unique;
    struct lysc_node *parent;
    const char *keystr, *delim;
    size_t len;
    LY_ARRAY_COUNT_TYPE v;
    int8_t config; /* -1 - not yet seen; 0 - LYS_CONFIG_R; 1 - LYS_CONFIG_W */
    uint16_t flags;

    LY_ARRAY_FOR(uniques, v) {
        config = -1;
        LY_ARRAY_NEW_RET(ctx->ctx, list->uniques, unique, LY_EMEM);
        keystr = uniques[v].str;
        while (keystr) {
            delim = strpbrk(keystr, " \t\n");
            if (delim) {
                len = delim - keystr;
                while (isspace(*delim)) {
                    ++delim;
                }
            } else {
                len = strlen(keystr);
            }

            /* unique node must be present */
            LY_ARRAY_NEW_RET(ctx->ctx, *unique, key, LY_EMEM);
            ret = lysc_resolve_schema_nodeid(ctx, keystr, len, &list->node, LY_VALUE_SCHEMA, (void *)uniques[v].mod,
                    LYS_LEAF, (const struct lysc_node **)key, &flags);
            if (ret != LY_SUCCESS) {
                if (ret == LY_EDENIED) {
                    LOGVAL(ctx->ctx, LYVE_REFERENCE,
                            "Unique's descendant-schema-nodeid \"%.*s\" refers to %s node instead of a leaf.",
                            (int)len, keystr, lys_nodetype2str((*key)->nodetype));
                }
                return LY_EVALID;
            } else if (flags) {
                LOGVAL(ctx->ctx, LYVE_REFERENCE,
                        "Unique's descendant-schema-nodeid \"%.*s\" refers into %s node.",
                        (int)len, keystr, flags & LYS_IS_NOTIF ? "notification" : "RPC/action");
                return LY_EVALID;
            }

            /* all referenced leafs must be of the same config type */
            if ((config != -1) && ((((*key)->flags & LYS_CONFIG_W) && (config == 0)) ||
                    (((*key)->flags & LYS_CONFIG_R) && (config == 1)))) {
                LOGVAL(ctx->ctx, LYVE_SEMANTICS,
                        "Unique statement \"%s\" refers to leaves with different config type.", uniques[v].str);
                return LY_EVALID;
            } else if ((*key)->flags & LYS_CONFIG_W) {
                config = 1;
            } else { /* LYS_CONFIG_R */
                config = 0;
            }

            /* we forbid referencing nested lists because it is unspecified what instance of such a list to use */
            for (parent = (*key)->parent; parent != (struct lysc_node *)list; parent = parent->parent) {
                if (parent->nodetype == LYS_LIST) {
                    LOGVAL(ctx->ctx, LYVE_SEMANTICS,
                            "Unique statement \"%s\" refers to a leaf in nested list \"%s\".", uniques[v].str, parent->name);
                    return LY_EVALID;
                }
            }

            /* check status */
            LY_CHECK_RET(lysc_check_status(ctx, list->flags, uniques[v].mod->mod, list->name,
                    (*key)->flags, (*key)->module, (*key)->name));

            /* mark leaf as unique */
            (*key)->flags |= LYS_UNIQUE;

            /* next unique value in line */
            keystr = delim;
        }
        /* next unique definition */
    }

    return LY_SUCCESS;
}

/**
 * @brief Compile parsed list node information.
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed list node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the list-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_list(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    struct lysp_node_list *list_p = (struct lysp_node_list *)pnode;
    struct lysc_node_list *list = (struct lysc_node_list *)node;
    struct lysp_node *child_p;
    struct lysc_node *parent;
    struct lysc_node_leaf *key, *prev_key = NULL;
    size_t len;
    const char *keystr, *delim;
    LY_ERR ret = LY_SUCCESS;

    list->min = list_p->min;
    if (list->min) {
        list->flags |= LYS_MAND_TRUE;
    }
    list->max = list_p->max ? list_p->max : (uint32_t)-1;

    LY_LIST_FOR(list_p->child, child_p) {
        LY_CHECK_RET(lys_compile_node(ctx, child_p, node, 0, NULL));
    }

    COMPILE_ARRAY_GOTO(ctx, list_p->musts, list->musts, lys_compile_must, ret, done);

    /* add must(s) to unres */
    ret = lysc_unres_must_add(ctx, node, pnode);
    LY_CHECK_GOTO(ret, done);

    /* keys */
    if (list->flags & LYS_CONFIG_W) {
        parent = node;
        if (ctx->compile_opts & LYS_COMPILE_GROUPING) {
            /* compiling individual grouping, we can check this only if there is an explicit config set */
            while (parent) {
                if (parent->flags & LYS_SET_CONFIG) {
                    break;
                }
                parent = parent->parent;
            }
        }

        if (parent && (!list_p->key || !list_p->key[0])) {
            LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Missing key in list representing configuration data.");
            return LY_EVALID;
        }
    }

    /* find all the keys (must be direct children) */
    keystr = list_p->key;
    if (!keystr) {
        /* keyless list */
        list->flags &= ~LYS_ORDBY_SYSTEM;
        list->flags |= LYS_KEYLESS | LYS_ORDBY_USER;
    }
    while (keystr) {
        delim = strpbrk(keystr, " \t\n");
        if (delim) {
            len = delim - keystr;
            while (isspace(*delim)) {
                ++delim;
            }
        } else {
            len = strlen(keystr);
        }

        /* key node must be present */
        key = (struct lysc_node_leaf *)lys_find_child(node, node->module, keystr, len, LYS_LEAF, LYS_GETNEXT_NOCHOICE);
        if (!key) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE, "The list's key \"%.*s\" not found.", (int)len, keystr);
            return LY_EVALID;
        }
        /* keys must be unique */
        if (key->flags & LYS_KEY) {
            /* the node was already marked as a key */
            LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Duplicated key identifier \"%.*s\".", (int)len, keystr);
            return LY_EVALID;
        }

        lysc_update_path(ctx, list->module, key->name);
        /* key must have the same config flag as the list itself */
        if ((list->flags & LYS_CONFIG_MASK) != (key->flags & LYS_CONFIG_MASK)) {
            LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Key of a configuration list must not be a state leaf.");
            return LY_EVALID;
        }
        if (ctx->pmod->version < LYS_VERSION_1_1) {
            /* YANG 1.0 denies key to be of empty type */
            if (key->type->basetype == LY_TYPE_EMPTY) {
                LOGVAL(ctx->ctx, LYVE_SEMANTICS,
                        "List key of the \"empty\" type is allowed only in YANG 1.1 modules.");
                return LY_EVALID;
            }
        } else {
            /* when and if-feature are illegal on list keys */
            if (key->when) {
                LOGVAL(ctx->ctx, LYVE_SEMANTICS, "List's key must not have any \"when\" statement.");
                return LY_EVALID;
            }
            /* unable to check if-features but compilation would fail if disabled */
        }

        /* check status */
        LY_CHECK_RET(lysc_check_status(ctx, list->flags, list->module, list->name, key->flags, key->module, key->name));

        /* ignore default values of the key */
        if (key->dflt) {
            key->dflt->realtype->plugin->free(ctx->ctx, key->dflt);
            lysc_type_free(&ctx->free_ctx, (struct lysc_type *)key->dflt->realtype);
            free(key->dflt);
            key->dflt = NULL;
        }
        /* mark leaf as key */
        key->flags |= LYS_KEY;

        /* move it to the correct position */
        if ((prev_key && ((struct lysc_node *)prev_key != key->prev)) || (!prev_key && key->prev->next)) {
            /* fix links in closest previous siblings of the key */
            if (key->next) {
                key->next->prev = key->prev;
            } else {
                /* last child */
                list->child->prev = key->prev;
            }
            if (key->prev->next) {
                key->prev->next = key->next;
            }
            /* fix links in the key */
            if (prev_key) {
                key->prev = &prev_key->node;
                key->next = prev_key->next;
            } else {
                key->prev = list->child->prev;
                key->next = list->child;
            }
            /* fix links in closes future siblings of the key */
            if (prev_key) {
                if (prev_key->next) {
                    prev_key->next->prev = &key->node;
                } else {
                    list->child->prev = &key->node;
                }
                prev_key->next = &key->node;
            } else {
                list->child->prev = &key->node;
            }
            /* fix links in parent */
            if (!key->prev->next) {
                list->child = &key->node;
            }
        }

        /* next key value */
        prev_key = key;
        keystr = delim;
        lysc_update_path(ctx, NULL, NULL);
    }

    /* connect any augments */
    LY_CHECK_GOTO(ret = lys_compile_node_augments(ctx, node), done);

    /* uniques */
    if (list_p->uniques) {
        LY_CHECK_RET(lys_compile_node_list_unique(ctx, list_p->uniques, list));
    }

    LY_LIST_FOR((struct lysp_node *)list_p->actions, child_p) {
        ret = lys_compile_node(ctx, child_p, node, 0, NULL);
        LY_CHECK_GOTO(ret, done);
    }
    LY_LIST_FOR((struct lysp_node *)list_p->notifs, child_p) {
        ret = lys_compile_node(ctx, child_p, node, 0, NULL);
        LY_CHECK_GOTO(ret, done);
    }

    /* checks */
    if (list->min > list->max) {
        LOGVAL(ctx->ctx, LYVE_SEMANTICS, "List min-elements %" PRIu32 " is bigger than max-elements %" PRIu32 ".",
                list->min, list->max);
        return LY_EVALID;
    }

done:
    return ret;
}

/**
 * @brief Do some checks and set the default choice's case.
 *
 * Selects (and stores into ::lysc_node_choice#dflt) the default case and set LYS_SET_DFLT flag on it.
 *
 * @param[in] ctx Compile context.
 * @param[in] dflt Name of the default branch. Can even contain a prefix.
 * @param[in,out] ch The compiled choice node, its dflt member is filled to point to the default case node of the choice.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_node_choice_dflt(struct lysc_ctx *ctx, struct lysp_qname *dflt, struct lysc_node_choice *ch)
{
    struct lysc_node *iter;
    const struct lys_module *mod;
    const char *prefix = NULL, *name;
    size_t prefix_len = 0;

    /* could use lys_parse_nodeid(), but it checks syntax which is already done in this case by the parsers */
    name = strchr(dflt->str, ':');
    if (name) {
        prefix = dflt->str;
        prefix_len = name - prefix;
        ++name;
    } else {
        name = dflt->str;
    }
    if (prefix) {
        mod = ly_resolve_prefix(ctx->ctx, prefix, prefix_len, LY_VALUE_SCHEMA, (void *)dflt->mod);
        if (!mod) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE, "Default case prefix \"%.*s\" not found "
                    "in imports of \"%s\".", (int)prefix_len, prefix, LYSP_MODULE_NAME(dflt->mod));
            return LY_EVALID;
        }
    } else {
        mod = ch->module;
    }

    ch->dflt = (struct lysc_node_case *)lys_find_child(&ch->node, mod, name, 0, LYS_CASE, LYS_GETNEXT_WITHCASE);
    if (!ch->dflt) {
        LOGVAL(ctx->ctx, LYVE_SEMANTICS,
                "Default case \"%s\" not found.", dflt->str);
        return LY_EVALID;
    }

    /* no mandatory nodes directly under the default case */
    LY_LIST_FOR(ch->dflt->child, iter) {
        if (iter->parent != (struct lysc_node *)ch->dflt) {
            break;
        }
        if (iter->flags & LYS_MAND_TRUE) {
            LOGVAL(ctx->ctx, LYVE_SEMANTICS,
                    "Mandatory node \"%s\" under the default case \"%s\".", iter->name, dflt->str);
            return LY_EVALID;
        }
    }

    if (ch->flags & LYS_MAND_TRUE) {
        LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Invalid mandatory choice with a default case.");
        return LY_EVALID;
    }

    ch->dflt->flags |= LYS_SET_DFLT;
    return LY_SUCCESS;
}

LY_ERR
lys_compile_node_choice_child(struct lysc_ctx *ctx, struct lysp_node *child_p, struct lysc_node *node,
        struct ly_set *child_set)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysp_node *child_p_next = child_p->next;
    struct lysp_node_case *cs_p;
    struct lysc_node_case *cs_c;

    if (child_p->nodetype == LYS_CASE) {
        /* standard case under choice */
        ret = lys_compile_node(ctx, child_p, node, 0, child_set);
    } else {
        /* we need the implicit case first, so create a fake parsed (shorthand) case */
        cs_p = calloc(1, sizeof *cs_p);
        LY_CHECK_ERR_RET(!cs_p, LOGMEM(ctx->ctx), LY_EMEM);
        cs_p->nodetype = LYS_CASE;
        DUP_STRING_GOTO(ctx->ctx, child_p->name, cs_p->name, ret, revert_sh_case);
        cs_p->child = child_p;

        /* make the child the only case child */
        child_p->next = NULL;

        /* compile it normally */
        LY_CHECK_GOTO(ret = lys_compile_node(ctx, (struct lysp_node *)cs_p, node, 0, child_set), revert_sh_case);

        if (((struct lysc_node_choice *)node)->cases) {
            /* find our case node */
            cs_c = (struct lysc_node_case *)((struct lysc_node_choice *)node)->cases;
            while (cs_c->name != cs_p->name) {
                cs_c = (struct lysc_node_case *)cs_c->next;
                assert(cs_c);
            }

            if (ctx->ctx->flags & LY_CTX_SET_PRIV_PARSED) {
                /* compiled case node cannot point to his corresponding parsed node
                 * because it exists temporarily so it must be set to NULL
                 */
                assert(cs_c->priv == cs_p);
                cs_c->priv = NULL;
            }

            /* status is copied from his child and not from his parent as usual. */
            if (cs_c->child) {
                cs_c->flags &= ~LYS_STATUS_MASK;
                cs_c->flags |= (LYS_STATUS_MASK & cs_c->child->flags);
            }
        } /* else it was removed by a deviation */

revert_sh_case:
        /* free the parsed shorthand case and correct pointers back */
        cs_p->child = NULL;
        lysp_node_free(&ctx->free_ctx, (struct lysp_node *)cs_p);
        child_p->next = child_p_next;
    }

    return ret;
}

/**
 * @brief Compile parsed choice node information.
 *
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed choice node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the choice-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_choice(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    struct lysp_node_choice *ch_p = (struct lysp_node_choice *)pnode;
    struct lysc_node_choice *ch = (struct lysc_node_choice *)node;
    struct lysp_node *child_p;
    LY_ERR ret = LY_SUCCESS;

    assert(node->nodetype == LYS_CHOICE);

    LY_LIST_FOR(ch_p->child, child_p) {
        LY_CHECK_GOTO(ret = lys_compile_node_choice_child(ctx, child_p, node, NULL), done);
    }

    /* connect any augments */
    LY_CHECK_GOTO(ret = lys_compile_node_augments(ctx, node), done);

    /* default branch */
    if (ch_p->dflt.str) {
        LY_CHECK_GOTO(ret = lys_compile_node_choice_dflt(ctx, &ch_p->dflt, ch), done);
    }

done:
    return ret;
}

/**
 * @brief Compile parsed anydata or anyxml node information.
 *
 * @param[in] ctx Compile context
 * @param[in] pnode Parsed anydata or anyxml node.
 * @param[in,out] node Pre-prepared structure from lys_compile_node() with filled generic node information
 * is enriched with the any-specific information.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_node_any(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    struct lysp_node_anydata *any_p = (struct lysp_node_anydata *)pnode;
    struct lysc_node_anydata *any = (struct lysc_node_anydata *)node;
    LY_ERR ret = LY_SUCCESS;

    COMPILE_ARRAY_GOTO(ctx, any_p->musts, any->musts, lys_compile_must, ret, done);

    /* add must(s) to unres */
    ret = lysc_unres_must_add(ctx, node, pnode);
    LY_CHECK_GOTO(ret, done);

    if (any->flags & LYS_CONFIG_W) {
        LOGVRB("Use of %s to define configuration data is not recommended. %s",
                lyplg_ext_stmt2str(any->nodetype == LYS_ANYDATA ? LY_STMT_ANYDATA : LY_STMT_ANYXML), ctx->path);
    }
done:
    return ret;
}

/**
 * @brief Prepare the case structure in choice node for the new data node.
 *
 * It is able to handle implicit as well as explicit cases and the situation when the case has multiple data nodes and the case was already
 * created in the choice when the first child was processed.
 *
 * @param[in] ctx Compile context.
 * @param[in] pnode Node image from the parsed tree. If the case is explicit, it is the LYS_CASE node, but in case of implicit case,
 *                   it is the LYS_CHOICE, LYS_AUGMENT or LYS_GROUPING node.
 * @param[in] ch The compiled choice structure where the new case structures are created (if needed).
 * @param[in] child The new data node being part of a case (no matter if explicit or implicit).
 * @return The case structure where the child node belongs to, NULL in case of error. Note that the child is not connected into the siblings list,
 * it is linked from the case structure only in case it is its first child.
 */
static LY_ERR
lys_compile_node_case(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *node)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysp_node *child_p;
    struct lysp_node_case *cs_p = (struct lysp_node_case *)pnode;

    if (pnode->nodetype & (LYS_CHOICE | LYS_AUGMENT | LYS_GROUPING)) {
        /* we have to add an implicit case node into the parent choice */
    } else if (pnode->nodetype == LYS_CASE) {
        /* explicit parent case */
        LY_LIST_FOR(cs_p->child, child_p) {
            LY_CHECK_GOTO(ret = lys_compile_node(ctx, child_p, node, 0, NULL), done);
        }
    } else {
        LOGINT_RET(ctx->ctx);
    }

    /* connect any augments */
    LY_CHECK_GOTO(ret = lys_compile_node_augments(ctx, node), done);

done:
    return ret;
}

void
lys_compile_mandatory_parents(struct lysc_node *parent, ly_bool add)
{
    const struct lysc_node *iter;

    if (add) { /* set flag */
        for ( ; parent && parent->nodetype == LYS_CONTAINER && !(parent->flags & LYS_MAND_TRUE) && !(parent->flags & LYS_PRESENCE);
                parent = parent->parent) {
            parent->flags |= LYS_MAND_TRUE;
        }
    } else { /* unset flag */
        for ( ; parent && parent->nodetype == LYS_CONTAINER && (parent->flags & LYS_MAND_TRUE); parent = parent->parent) {
            for (iter = lysc_node_child(parent); iter; iter = iter->next) {
                if (iter->flags & LYS_MAND_TRUE) {
                    /* there is another mandatory node */
                    return;
                }
            }
            /* unset mandatory flag - there is no mandatory children in the non-presence container */
            parent->flags &= ~LYS_MAND_TRUE;
        }
    }
}

/**
 * @brief Get the grouping with the specified name from given groupings sized array.
 *
 * @param[in] node Linked list of nodes with groupings.
 * @param[in] name Name of the grouping to find,
 * @return NULL when there is no grouping with the specified name
 * @return Pointer to the grouping of the specified @p name.
 */
static struct lysp_node_grp *
match_grouping(const struct lysp_node_grp *node, const char *name)
{
    LY_LIST_FOR(node, node) {
        if ((node->nodetype == LYS_GROUPING) && !strcmp(node->name, name)) {
            return (struct lysp_node_grp *)node;
        }
    }

    return NULL;
}

/**
 * @brief Find grouping for a uses.
 *
 * @param[in] ctx Compile context.
 * @param[in] uses_p Parsed uses node.
 * @param[out] gpr_p Found grouping on success.
 * @param[out] grp_pmod Module of @p grp_p on success.
 * @return LY_ERR value.
 */
static LY_ERR
lys_compile_uses_find_grouping(struct lysc_ctx *ctx, struct lysp_node_uses *uses_p, struct lysp_node_grp **grp_p,
        struct lysp_module **grp_pmod)
{
    struct lysp_node *pnode;
    struct lysp_node_grp *grp;
    const struct lysp_node_grp *ext_grp;
    LY_ARRAY_COUNT_TYPE u;
    const char *id, *name, *prefix, *local_pref;
    size_t prefix_len, name_len;
    struct lysp_module *pmod, *found = NULL;
    const struct lys_module *mod;

    *grp_p = NULL;
    *grp_pmod = NULL;

    /* search for the grouping definition */
    id = uses_p->name;
    LY_CHECK_RET(ly_parse_nodeid(&id, &prefix, &prefix_len, &name, &name_len), LY_EVALID);
    local_pref = ctx->pmod->is_submod ? ((struct lysp_submodule *)ctx->pmod)->prefix : ctx->pmod->mod->prefix;
    if (!prefix || !ly_strncmp(local_pref, prefix, prefix_len)) {
        /* current module, search local groupings first */
        pmod = ctx->pmod->mod->parsed; /* make sure that we will start in main_module, not submodule */
        for (pnode = uses_p->parent; !found && pnode; pnode = pnode->parent) {
            if ((grp = match_grouping(lysp_node_groupings(pnode), name))) {
                found = ctx->pmod;
                break;
            }
        }

        /* if in an extension, search possible groupings in it */
        if (!found && ctx->ext) {
            lyplg_ext_parsed_get_storage(ctx->ext, LY_STMT_GROUPING, sizeof ext_grp, (const void **)&ext_grp);
            if ((grp = match_grouping(ext_grp, name))) {
                found = ctx->pmod;
            }
        }
    } else {
        /* foreign module, find it first */
        mod = ly_resolve_prefix(ctx->ctx, prefix, prefix_len, LY_VALUE_SCHEMA, ctx->pmod);
        if (!mod) {
            LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid prefix used for grouping \"%s\" reference.", uses_p->name);
            return LY_EVALID;
        }
        pmod = mod->parsed;
    }

    if (!found) {
        /* search in top-level groupings of the main module ... */
        if ((grp = match_grouping(pmod->groupings, name))) {
            found = pmod;
        } else {
            /* ... and all the submodules */
            LY_ARRAY_FOR(pmod->includes, u) {
                if ((grp = match_grouping(pmod->includes[u].submodule->groupings, name))) {
                    found = (struct lysp_module *)pmod->includes[u].submodule;
                    break;
                }
            }
        }
    }
    if (!found) {
        LOGVAL(ctx->ctx, LYVE_SEMANTICS, "Grouping \"%s\" referenced by a uses statement not found.", uses_p->name);
        return LY_EVALID;
    }

    if (!(ctx->compile_opts & LYS_COMPILE_GROUPING)) {
        /* remember that the grouping is instantiated to avoid its standalone validation */
        grp->flags |= LYS_USED_GRP;
    }

    *grp_p = grp;
    *grp_pmod = found;
    return LY_SUCCESS;
}

/**
 * @brief Compile uses grouping children.
 *
 * @param[in] ctx Compile context.
 * @param[in] uses_p Parsed uses.
 * @param[in] inherited_flags Inherited flags from the uses.
 * @param[in] child First grouping child to compile.
 * @param[in] grp_mod Grouping parsed module.
 * @param[in] parent Uses compiled parent, may be NULL if top-level.
 * @param[in,out] child_set Set of all compiled child nodes.
 * @param[in] child_unres_disabled Whether the children are to be put into unres disabled set or not.
 * @return LY_SUCCESS on success.
 * @return LY_EVALID on failure.
 */
static LY_ERR
lys_compile_uses_children(struct lysc_ctx *ctx, struct lysp_node_uses *uses_p, uint16_t inherited_flags,
        struct lysp_node *child, struct lysp_module *grp_mod, struct lysc_node *parent, struct ly_set *child_set,
        ly_bool child_unres_disabled)
{
    LY_ERR rc = LY_SUCCESS;
    struct lysp_module *mod_old = ctx->pmod;
    uint32_t child_i, opt_prev = ctx->compile_opts;
    ly_bool enabled;
    struct lysp_node *pnode;
    struct lysc_node *node;
    struct lysc_when *when_shared = NULL;

    assert(child_set);

    child_i = child_set->count;
    LY_LIST_FOR(child, pnode) {
        /* compile the nodes with their parsed (grouping) module */
        ctx->pmod = grp_mod;
        LY_CHECK_GOTO(rc = lys_compile_node(ctx, pnode, parent, inherited_flags, child_set), cleanup);

        /* eval if-features again for the rest of this node processing */
        LY_CHECK_GOTO(rc = lys_eval_iffeatures(ctx->ctx, pnode->iffeatures, &enabled), cleanup);
        if (!enabled && !(ctx->compile_opts & (LYS_COMPILE_NO_DISABLED | LYS_COMPILE_DISABLED | LYS_COMPILE_GROUPING))) {
            ctx->compile_opts |= LYS_COMPILE_DISABLED;
        }

        /* restore the parsed module */
        ctx->pmod = mod_old;

        /* since the uses node is not present in the compiled tree, we need to pass some of its
         * statements to all its children */
        while (child_i < child_set->count) {
            node = child_set->snodes[child_i];

            if (uses_p->when) {
                /* pass uses when to all the children */
                rc = lys_compile_when(ctx, uses_p->when, inherited_flags, parent, lysc_data_node(parent), node, &when_shared);
                LY_CHECK_GOTO(rc, cleanup);
            }

            if (child_unres_disabled) {
                /* child is disabled by the uses if-features */
                ly_set_add(&ctx->unres->disabled, node, 1, NULL);
            }

            /* child processed */
            ++child_i;
        }

        /* next iter */
        ctx->compile_opts = opt_prev;
    }

cleanup:
    ctx->compile_opts = opt_prev;
    return rc;
}

/**
 * @brief Compile parsed uses statement - resolve target grouping and connect its content into parent.
 * If present, also apply uses's modificators.
 *
 * @param[in] ctx Compile context
 * @param[in] uses_p Parsed uses schema node.
 * @param[in] parent Compiled parent node where the content of the referenced grouping is supposed to be connected. It is
 * NULL for top-level nodes, in such a case the module where the node will be connected is taken from
 * the compile context.
 * @param[in] inherited_flags Inherited flags from a schema-only statement.
 * @param[in] child_set Optional set of all the compiled children.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
static LY_ERR
lys_compile_uses(struct lysc_ctx *ctx, struct lysp_node_uses *uses_p, struct lysc_node *parent, uint16_t inherited_flags,
        struct ly_set *child_set)
{
    LY_ERR rc = LY_SUCCESS;
    ly_bool enabled, child_unres_disabled = 0;
    uint32_t i, grp_stack_count, opt_prev = ctx->compile_opts;
    struct lysp_node_grp *grp = NULL;
    uint16_t uses_flags = 0;
    struct lysp_module *grp_mod;
    struct ly_set uses_child_set = {0};

    /* find the referenced grouping */
    LY_CHECK_RET(lys_compile_uses_find_grouping(ctx, uses_p, &grp, &grp_mod));

    /* grouping must not reference themselves - stack in ctx maintains list of groupings currently being applied */
    grp_stack_count = ctx->groupings.count;
    LY_CHECK_RET(ly_set_add(&ctx->groupings, (void *)grp, 0, NULL));
    if (grp_stack_count == ctx->groupings.count) {
        /* the target grouping is already in the stack, so we are already inside it -> circular dependency */
        LOGVAL(ctx->ctx, LYVE_REFERENCE,
                "Grouping \"%s\" references itself through a uses statement.", grp->name);
        return LY_EVALID;
    }

    /* nodetype checks */
    if (grp->actions && (parent && !lysc_node_actions_p(parent))) {
        LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid child %s \"%s\" of uses parent %s \"%s\" node.",
                grp->actions->name, lys_nodetype2str(grp->actions->nodetype),
                parent->name, lys_nodetype2str(parent->nodetype));
        rc = LY_EVALID;
        goto cleanup;
    }
    if (grp->notifs && (parent && !lysc_node_notifs_p(parent))) {
        LOGVAL(ctx->ctx, LYVE_REFERENCE, "Invalid child %s \"%s\" of uses parent %s \"%s\" node.",
                grp->notifs->name, lys_nodetype2str(grp->notifs->nodetype),
                parent->name, lys_nodetype2str(parent->nodetype));
        rc = LY_EVALID;
        goto cleanup;
    }

    /* check status */
    rc = lysc_check_status(ctx, uses_p->flags, ctx->pmod, uses_p->name, grp->flags, grp_mod, grp->name);
    LY_CHECK_GOTO(rc, cleanup);

    /* compile any augments and refines so they can be applied during the grouping nodes compilation */
    rc = lys_precompile_uses_augments_refines(ctx, uses_p, parent);
    LY_CHECK_GOTO(rc, cleanup);

    /* compile special uses status flags */
    rc = lys_compile_status(ctx, uses_p->flags, inherited_flags, parent ? parent->flags : 0,
            parent ? parent->name : NULL, "<uses>", &uses_flags);
    LY_CHECK_GOTO(rc, cleanup);

    /* uses if-features */
    LY_CHECK_GOTO(rc = lys_eval_iffeatures(ctx->ctx, uses_p->iffeatures, &enabled), cleanup);
    if (!enabled && !(ctx->compile_opts & (LYS_COMPILE_NO_DISABLED | LYS_COMPILE_DISABLED | LYS_COMPILE_GROUPING))) {
        ctx->compile_opts |= LYS_COMPILE_DISABLED;
        child_unres_disabled = 1;
    }

    /* uses grouping children */
    rc = lys_compile_uses_children(ctx, uses_p, uses_flags, grp->child, grp_mod, parent,
            child_set ? child_set : &uses_child_set, child_unres_disabled);
    LY_CHECK_GOTO(rc, cleanup);

    /* uses grouping RPCs/actions */
    rc = lys_compile_uses_children(ctx, uses_p, uses_flags, (struct lysp_node *)grp->actions, grp_mod, parent,
            child_set ? child_set : &uses_child_set, child_unres_disabled);
    LY_CHECK_GOTO(rc, cleanup);

    /* uses grouping notifications */
    rc = lys_compile_uses_children(ctx, uses_p, uses_flags, (struct lysp_node *)grp->notifs, grp_mod, parent,
            child_set ? child_set : &uses_child_set, child_unres_disabled);
    LY_CHECK_GOTO(rc, cleanup);

    /* check that all augments were applied */
    for (i = 0; i < ctx->uses_augs.count; ++i) {
        if (((struct lysc_augment *)ctx->uses_augs.objs[i])->aug_p->parent != (struct lysp_node *)uses_p) {
            /* augment of some parent uses, irrelevant now */
            continue;
        }

        LOGVAL(ctx->ctx, LYVE_REFERENCE, "Augment target node \"%s\" in grouping \"%s\" was not found.",
                ((struct lysc_augment *)ctx->uses_augs.objs[i])->nodeid->str, grp->name);
        rc = LY_ENOTFOUND;
    }
    LY_CHECK_GOTO(rc, cleanup);

    /* check that all refines were applied */
    for (i = 0; i < ctx->uses_rfns.count; ++i) {
        if (((struct lysc_refine *)ctx->uses_rfns.objs[i])->uses_p != uses_p) {
            /* refine of some parent uses, irrelevant now */
            continue;
        }

        LOGVAL(ctx->ctx, LYVE_REFERENCE, "Refine(s) target node \"%s\" in grouping \"%s\" was not found.",
                ((struct lysc_refine *)ctx->uses_rfns.objs[i])->nodeid->str, grp->name);
        rc = LY_ENOTFOUND;
    }
    LY_CHECK_GOTO(rc, cleanup);

    /* compile uses and grouping extensions into the parent */
    COMPILE_EXTS_GOTO(ctx, uses_p->exts, parent->exts, parent, rc, cleanup);
    COMPILE_EXTS_GOTO(ctx, grp->exts, parent->exts, parent, rc, cleanup);

cleanup:
    /* restore previous context */
    ctx->compile_opts = opt_prev;

    /* remove the grouping from the stack for circular groupings dependency check */
    ly_set_rm_index(&ctx->groupings, ctx->groupings.count - 1, NULL);
    assert(ctx->groupings.count == grp_stack_count);

    ly_set_erase(&uses_child_set, NULL);
    return rc;
}

static int
lys_compile_grouping_pathlog(struct lysc_ctx *ctx, struct lysp_node *node, char **path)
{
    struct lysp_node *iter;
    int len = 0;

    *path = NULL;
    for (iter = node; iter && len >= 0; iter = iter->parent) {
        char *s = *path;
        char *id;

        switch (iter->nodetype) {
        case LYS_USES:
            LY_CHECK_RET(asprintf(&id, "{uses='%s'}", iter->name) == -1, -1);
            break;
        case LYS_GROUPING:
            LY_CHECK_RET(asprintf(&id, "{grouping='%s'}", iter->name) == -1, -1);
            break;
        case LYS_AUGMENT:
            LY_CHECK_RET(asprintf(&id, "{augment='%s'}", iter->name) == -1, -1);
            break;
        default:
            id = strdup(iter->name);
            break;
        }

        if (!iter->parent) {
            /* print prefix */
            len = asprintf(path, "/%s:%s%s", ctx->cur_mod->name, id, s ? s : "");
        } else {
            /* prefix is the same as in parent */
            len = asprintf(path, "/%s%s", id, s ? s : "");
        }
        free(s);
        free(id);
    }

    if (len < 0) {
        free(*path);
        *path = NULL;
    } else if (len == 0) {
        *path = strdup("/");
        len = 1;
    }
    return len;
}

LY_ERR
lys_compile_grouping(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysp_node_grp *grp)
{
    LY_ERR rc = LY_SUCCESS;
    char *path;
    int len;

    /* use grouping status to avoid errors */
    struct lysp_node_uses fake_uses = {
        .parent = pnode,
        .nodetype = LYS_USES,
        .flags = grp->flags & LYS_STATUS_MASK, .next = NULL,
        .name = grp->name,
        .dsc = NULL, .ref = NULL, .when = NULL, .iffeatures = NULL, .exts = NULL,
        .refines = NULL, .augments = NULL
    };
    struct lysc_node_container fake_container = {
        .nodetype = LYS_CONTAINER,
        .flags = 0,
        .module = ctx->cur_mod,
        .parent = NULL, .next = NULL,
        .prev = &fake_container.node,
        .name = "fake",
        .dsc = NULL, .ref = NULL, .exts = NULL, .when = NULL,
        .child = NULL, .musts = NULL, .actions = NULL, .notifs = NULL
    };

    /* compile fake container flags */
    LY_CHECK_GOTO(rc = lys_compile_node_flags(ctx, pnode ? pnode->flags : 0, 0, &fake_container.node), cleanup);

    if (grp->parent) {
        LOGWRN(ctx->ctx, "Locally scoped grouping \"%s\" not used.", grp->name);
    }

    len = lys_compile_grouping_pathlog(ctx, grp->parent, &path);
    if (len < 0) {
        LOGMEM(ctx->ctx);
        return LY_EMEM;
    }
    strncpy(ctx->path, path, LYSC_CTX_BUFSIZE - 1);
    ctx->path_len = (uint32_t)len;
    free(path);

    lysc_update_path(ctx, NULL, "{grouping}");
    lysc_update_path(ctx, NULL, grp->name);
    rc = lys_compile_uses(ctx, &fake_uses, &fake_container.node, 0, NULL);
    lysc_update_path(ctx, NULL, NULL);
    lysc_update_path(ctx, NULL, NULL);

    ctx->path_len = 1;
    ctx->path[1] = '\0';

cleanup:
    lysc_node_container_free(&ctx->free_ctx, &fake_container);
    FREE_ARRAY(&ctx->free_ctx, fake_container.exts, lysc_ext_instance_free);
    return rc;
}

LY_ERR
lys_compile_node(struct lysc_ctx *ctx, struct lysp_node *pnode, struct lysc_node *parent, uint16_t inherited_flags,
        struct ly_set *child_set)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_node *node = NULL;
    uint32_t prev_opts = ctx->compile_opts;

    LY_ERR (*node_compile_spec)(struct lysc_ctx *, struct lysp_node *, struct lysc_node *);

    if (pnode->nodetype != LYS_USES) {
        lysc_update_path(ctx, parent ? parent->module : NULL, pnode->name);
    } else {
        lysc_update_path(ctx, NULL, "{uses}");
        lysc_update_path(ctx, NULL, pnode->name);
    }

    switch (pnode->nodetype) {
    case LYS_CONTAINER:
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_container));
        node_compile_spec = lys_compile_node_container;
        break;
    case LYS_LEAF:
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_leaf));
        node_compile_spec = lys_compile_node_leaf;
        break;
    case LYS_LIST:
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_list));
        node_compile_spec = lys_compile_node_list;
        break;
    case LYS_LEAFLIST:
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_leaflist));
        node_compile_spec = lys_compile_node_leaflist;
        break;
    case LYS_CHOICE:
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_choice));
        node_compile_spec = lys_compile_node_choice;
        break;
    case LYS_CASE:
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_case));
        node_compile_spec = lys_compile_node_case;
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_anydata));
        node_compile_spec = lys_compile_node_any;
        break;
    case LYS_RPC:
    case LYS_ACTION:
        if (ctx->compile_opts & (LYS_IS_INPUT | LYS_IS_OUTPUT | LYS_IS_NOTIF)) {
            LOGVAL(ctx->ctx, LYVE_SEMANTICS,
                    "Action \"%s\" is placed inside %s.", pnode->name,
                    (ctx->compile_opts & LYS_IS_NOTIF) ? "notification" : "another RPC/action");
            return LY_EVALID;
        }
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_action));
        node_compile_spec = lys_compile_node_action;
        ctx->compile_opts |= LYS_COMPILE_NO_CONFIG;
        break;
    case LYS_NOTIF:
        if (ctx->compile_opts & (LYS_IS_INPUT | LYS_IS_OUTPUT | LYS_IS_NOTIF)) {
            LOGVAL(ctx->ctx, LYVE_SEMANTICS,
                    "Notification \"%s\" is placed inside %s.", pnode->name,
                    (ctx->compile_opts & LYS_IS_NOTIF) ? "another notification" : "RPC/action");
            return LY_EVALID;
        }
        node = (struct lysc_node *)calloc(1, sizeof(struct lysc_node_notif));
        node_compile_spec = lys_compile_node_notif;
        ctx->compile_opts |= LYS_COMPILE_NOTIFICATION;
        break;
    case LYS_USES:
        ret = lys_compile_uses(ctx, (struct lysp_node_uses *)pnode, parent, inherited_flags, child_set);
        lysc_update_path(ctx, NULL, NULL);
        lysc_update_path(ctx, NULL, NULL);
        return ret;
    default:
        LOGINT(ctx->ctx);
        return LY_EINT;
    }
    LY_CHECK_ERR_RET(!node, LOGMEM(ctx->ctx), LY_EMEM);

    ret = lys_compile_node_(ctx, pnode, parent, inherited_flags, node_compile_spec, node, child_set);

    ctx->compile_opts = prev_opts;
    lysc_update_path(ctx, NULL, NULL);
    return ret;
}
