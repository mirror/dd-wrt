/**
 * @file schema_features.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Schema feature handling
 *
 * Copyright (c) 2015 - 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "schema_features.h"

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "ly_common.h"
#include "set.h"
#include "tree.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

#define IFF_RECORDS_IN_BYTE 4
#define IFF_RECORD_BITS 2
#define IFF_RECORD_MASK 0x3

uint8_t
lysc_iff_getop(uint8_t *list, size_t pos)
{
    uint8_t *item;
    uint8_t mask = IFF_RECORD_MASK, result;

    item = &list[pos / IFF_RECORDS_IN_BYTE];
    result = (*item) & (mask << IFF_RECORD_BITS * (pos % IFF_RECORDS_IN_BYTE));
    return result >> IFF_RECORD_BITS * (pos % IFF_RECORDS_IN_BYTE);
}

static LY_ERR
lysc_iffeature_value_(const struct lysc_iffeature *iff, size_t *index_e, size_t *index_f)
{
    uint8_t op;
    LY_ERR a, b;

    op = lysc_iff_getop(iff->expr, *index_e);
    (*index_e)++;

    switch (op) {
    case LYS_IFF_F:
        /* resolve feature */
        return (iff->features[(*index_f)++]->flags & LYS_FENABLED) ? LY_SUCCESS : LY_ENOT;
    case LYS_IFF_NOT:
        /* invert result */
        return lysc_iffeature_value_(iff, index_e, index_f) == LY_SUCCESS ? LY_ENOT : LY_SUCCESS;
    case LYS_IFF_AND:
    case LYS_IFF_OR:
        a = lysc_iffeature_value_(iff, index_e, index_f);
        b = lysc_iffeature_value_(iff, index_e, index_f);
        if (op == LYS_IFF_AND) {
            if ((a == LY_SUCCESS) && (b == LY_SUCCESS)) {
                return LY_SUCCESS;
            } else {
                return LY_ENOT;
            }
        } else { /* LYS_IFF_OR */
            if ((a == LY_SUCCESS) || (b == LY_SUCCESS)) {
                return LY_SUCCESS;
            } else {
                return LY_ENOT;
            }
        }
    }

    return LY_ENOT;
}

LIBYANG_API_DEF LY_ERR
lysc_iffeature_value(const struct lysc_iffeature *iff)
{
    size_t index_e = 0, index_f = 0;

    LY_CHECK_ARG_RET(NULL, iff, LY_EINVAL);

    if (iff->expr) {
        return lysc_iffeature_value_(iff, &index_e, &index_f);
    }
    return LY_ENOT;
}

LIBYANG_API_DEF LY_ERR
lys_identity_iffeature_value(const struct lysc_ident *ident)
{
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool enabled;
    const struct lysp_ident *idents_p, *found_ident = NULL;
    struct lysp_include *includes;

    assert(ident);

    /* Search parsed identity in the module. */
    idents_p = ident->module->parsed->identities;
    LY_ARRAY_FOR(idents_p, u) {
        if (idents_p[u].name == ident->name) {
            found_ident = &idents_p[u];
            break;
        }
    }

    if (!found_ident) {
        /* It is not in the module, so it must be in some submodule. */
        includes = ident->module->parsed->includes;
        LY_ARRAY_FOR(includes, u) {
            idents_p = includes[u].submodule->identities;
            LY_ARRAY_FOR(idents_p, v) {
                if (idents_p[v].name == ident->name) {
                    found_ident = &idents_p[v];
                    break;
                }
            }
        }
    }

    assert(found_ident);

    /* Evaluate its if-feature. */
    LY_CHECK_RET(lys_eval_iffeatures(ident->module->ctx, found_ident->iffeatures, &enabled));
    if (!enabled) {
        return LY_ENOT;
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF struct lysp_feature *
lysp_feature_next(const struct lysp_feature *last, const struct lysp_module *pmod, uint32_t *idx)
{
    struct lysp_feature *features;

    if (!*idx) {
        /* module features */
        features = pmod->features;
    } else if ((*idx - 1) < LY_ARRAY_COUNT(pmod->includes)) {
        /* submodule features */
        features = pmod->includes[*idx - 1].submodule->features;
    } else {
        /* no more features */
        return NULL;
    }

    /* get the next feature */
    if (features && (!last || (&features[LY_ARRAY_COUNT(features) - 1] != last))) {
        return !last ? &features[0] : (struct lysp_feature *)last + 1;
    }

    /* no more features in current (sub)module */
    ++(*idx);
    return lysp_feature_next(NULL, pmod, idx);
}

/**
 * @brief Find a feature of the given name and referenced in the given module.
 *
 * @param[in] pmod Module where the feature was referenced (used to resolve prefix of the feature).
 * @param[in] name Name of the feature including possible prefix.
 * @param[in] len Length of the string representing the feature identifier in the name variable (mandatory!).
 * @param[in] prefixed Whether the feature name can be prefixed.
 * @return Pointer to the feature structure if found, NULL otherwise.
 */
static struct lysp_feature *
lysp_feature_find(const struct lysp_module *pmod, const char *name, size_t len, ly_bool prefixed)
{
    const struct lys_module *mod;
    const char *ptr;
    struct lysp_feature *f = NULL;
    uint32_t idx = 0;

    assert(pmod);

    if (prefixed && (ptr = ly_strnchr(name, ':', len))) {
        /* we have a prefixed feature */
        mod = ly_resolve_prefix(pmod->mod->ctx, name, ptr - name, LY_VALUE_SCHEMA, (void *)pmod);
        LY_CHECK_RET(!mod, NULL);

        pmod = mod->parsed;
        len = len - (ptr - name) - 1;
        name = ptr + 1;
    }

    /* feature without prefix, look in main module and all submodules */
    if (pmod->is_submod) {
        pmod = pmod->mod->parsed;
    }

    /* we have the correct module, get the feature */
    while ((f = lysp_feature_next(f, pmod, &idx))) {
        if (!ly_strncmp(f->name, name, len)) {
            return f;
        }
    }

    return NULL;
}

LIBYANG_API_DEF LY_ERR
lys_feature_value(const struct lys_module *module, const char *feature)
{
    const struct lysp_feature *f;

    LY_CHECK_ARG_RET(NULL, module, module->parsed, feature, LY_EINVAL);

    /* search for the specified feature */
    f = lysp_feature_find(module->parsed, feature, strlen(feature), 0);
    LY_CHECK_RET(!f, LY_ENOTFOUND);

    /* feature disabled */
    if (!(f->flags & LYS_FENABLED)) {
        return LY_ENOT;
    }

    /* feature enabled */
    return LY_SUCCESS;
}

/**
 * @brief Stack for processing if-feature expressions.
 */
struct iff_stack {
    size_t size;    /**< number of items in the stack */
    size_t index;   /**< first empty item */
    uint8_t *stack; /**< stack - array of @ref ifftokens to create the if-feature expression in prefix format */
};
#define IFF_STACK_SIZE_STEP 4

/**
 * @brief Add @ref ifftokens into the stack.
 * @param[in] stack The if-feature stack to use.
 * @param[in] value One of the @ref ifftokens to store in the stack.
 * @return LY_EMEM in case of memory allocation error
 * @return LY_ESUCCESS if the value successfully stored.
 */
static LY_ERR
iff_stack_push(struct iff_stack *stack, uint8_t value)
{
    if (stack->index == stack->size) {
        stack->size += IFF_STACK_SIZE_STEP;
        stack->stack = ly_realloc(stack->stack, stack->size * sizeof *stack->stack);
        LY_CHECK_ERR_RET(!stack->stack, LOGMEM(NULL); stack->size = 0, LY_EMEM);
    }
    stack->stack[stack->index++] = value;
    return LY_SUCCESS;
}

/**
 * @brief Get (and remove) the last item form the stack.
 * @param[in] stack The if-feature stack to use.
 * @return The value from the top of the stack.
 */
static uint8_t
iff_stack_pop(struct iff_stack *stack)
{
    assert(stack && stack->index);

    stack->index--;
    return stack->stack[stack->index];
}

/**
 * @brief Clean up the stack.
 * @param[in] stack The if-feature stack to use.
 */
static void
iff_stack_clean(struct iff_stack *stack)
{
    stack->size = 0;
    free(stack->stack);
}

/**
 * @brief Store the @ref ifftokens (@p op) on the given position in the 2bits array
 * (libyang format of the if-feature expression).
 * @param[in,out] list The 2bits array to modify.
 * @param[in] op The operand (@ref ifftokens) to store.
 * @param[in] pos Position (0-based) where to store the given @p op.
 */
static void
iff_setop(uint8_t *list, uint8_t op, size_t pos)
{
    uint8_t *item;
    uint8_t mask = IFF_RECORD_MASK;

    assert(op <= IFF_RECORD_MASK); /* max 2 bits */

    item = &list[pos / IFF_RECORDS_IN_BYTE];
    mask = mask << IFF_RECORD_BITS * (pos % IFF_RECORDS_IN_BYTE);
    *item = (*item) & ~mask;
    *item = (*item) | (op << IFF_RECORD_BITS * (pos % IFF_RECORDS_IN_BYTE));
}

#define LYS_IFF_LP 0x04 /**< Additional, temporary, value of @ref ifftokens: ( */
#define LYS_IFF_RP 0x08 /**< Additional, temporary, value of @ref ifftokens: ) */

static LY_ERR
lys_compile_iffeature(const struct ly_ctx *ctx, const struct lysp_qname *qname, struct lysc_iffeature *iff)
{
    LY_ERR rc = LY_SUCCESS;
    const char *c = qname->str;
    int64_t i, j;
    int8_t op_len, last_not = 0, checkversion = 0;
    LY_ARRAY_COUNT_TYPE f_size = 0, expr_size = 0, f_exp = 1;
    uint8_t op;
    struct iff_stack stack = {0, 0, NULL};
    struct lysp_feature *f;

    assert(c);

    /* pre-parse the expression to get sizes for arrays, also do some syntax checks of the expression */
    for (i = j = 0; c[i]; i++) {
        if (c[i] == '(') {
            j++;
            checkversion = 1;
            continue;
        } else if (c[i] == ')') {
            j--;
            continue;
        } else if (isspace(c[i])) {
            checkversion = 1;
            continue;
        }

        if (!strncmp(&c[i], "not", op_len = ly_strlen_const("not")) ||
                !strncmp(&c[i], "and", op_len = ly_strlen_const("and")) ||
                !strncmp(&c[i], "or", op_len = ly_strlen_const("or"))) {
            uint64_t spaces;

            for (spaces = 0; c[i + op_len + spaces] && isspace(c[i + op_len + spaces]); spaces++) {}
            if (c[i + op_len + spaces] == '\0') {
                LOGVAL(ctx, LYVE_SYNTAX_YANG, "Invalid value \"%s\" of if-feature - unexpected end of expression.", qname->str);
                return LY_EVALID;
            } else if (!isspace(c[i + op_len])) {
                /* feature name starting with the not/and/or */
                last_not = 0;
                f_size++;
            } else if (c[i] == 'n') { /* not operation */
                if (last_not) {
                    /* double not */
                    expr_size = expr_size - 2;
                    last_not = 0;
                } else {
                    last_not = 1;
                }
            } else { /* and, or */
                if (f_exp != f_size) {
                    LOGVAL(ctx, LYVE_SYNTAX_YANG,
                            "Invalid value \"%s\" of if-feature - missing feature/expression before \"%.*s\" operation.",
                            qname->str, op_len, &c[i]);
                    return LY_EVALID;
                }
                f_exp++;

                /* not a not operation */
                last_not = 0;
            }
            i += op_len;
        } else {
            f_size++;
            last_not = 0;
        }
        expr_size++;

        while (!isspace(c[i])) {
            if (!c[i] || (c[i] == ')') || (c[i] == '(')) {
                i--;
                break;
            }
            i++;
        }
    }
    if (j) {
        /* not matching count of ( and ) */
        LOGVAL(ctx, LYVE_SYNTAX_YANG, "Invalid value \"%s\" of if-feature - non-matching opening and closing parentheses.",
                qname->str);
        return LY_EVALID;
    }
    if (f_exp != f_size) {
        /* features do not match the needed arguments for the logical operations */
        LOGVAL(ctx, LYVE_SYNTAX_YANG, "Invalid value \"%s\" of if-feature - number of features in expression does not match "
                "the required number of operands for the operations.", qname->str);
        return LY_EVALID;
    }

    if (checkversion || (expr_size > 1)) {
        /* check that we have 1.1 module */
        if (qname->mod->version != LYS_VERSION_1_1) {
            LOGVAL(ctx, LYVE_SYNTAX_YANG, "Invalid value \"%s\" of if-feature - YANG 1.1 expression in YANG 1.0 module.",
                    qname->str);
            return LY_EVALID;
        }
    }

    /* allocate the memory */
    LY_ARRAY_CREATE_RET(ctx, iff->features, f_size, LY_EMEM);
    iff->expr = calloc((j = (expr_size / IFF_RECORDS_IN_BYTE) + ((expr_size % IFF_RECORDS_IN_BYTE) ? 1 : 0)), sizeof *iff->expr);
    stack.stack = malloc(expr_size * sizeof *stack.stack);
    LY_CHECK_ERR_GOTO(!stack.stack || !iff->expr, LOGMEM(ctx); rc = LY_EMEM, cleanup);

    stack.size = expr_size;
    f_size--; expr_size--; /* used as indexes from now */

    for (i--; i >= 0; i--) {
        if (c[i] == ')') {
            /* push it on stack */
            iff_stack_push(&stack, LYS_IFF_RP);
            continue;
        } else if (c[i] == '(') {
            /* pop from the stack into result all operators until ) */
            while ((op = iff_stack_pop(&stack)) != LYS_IFF_RP) {
                iff_setop(iff->expr, op, expr_size--);
            }
            continue;
        } else if (isspace(c[i])) {
            continue;
        }

        /* end of operator or operand -> find beginning and get what is it */
        j = i + 1;
        while (i >= 0 && !isspace(c[i]) && c[i] != '(') {
            i--;
        }
        i++; /* go back by one step */

        if (!strncmp(&c[i], "not", ly_strlen_const("not")) && isspace(c[i + ly_strlen_const("not")])) {
            if (stack.index && (stack.stack[stack.index - 1] == LYS_IFF_NOT)) {
                /* double not */
                iff_stack_pop(&stack);
            } else {
                /* not has the highest priority, so do not pop from the stack
                 * as in case of AND and OR */
                iff_stack_push(&stack, LYS_IFF_NOT);
            }
        } else if (!strncmp(&c[i], "and", ly_strlen_const("and")) && isspace(c[i + ly_strlen_const("and")])) {
            /* as for OR - pop from the stack all operators with the same or higher
             * priority and store them to the result, then push the AND to the stack */
            while (stack.index && stack.stack[stack.index - 1] <= LYS_IFF_AND) {
                op = iff_stack_pop(&stack);
                iff_setop(iff->expr, op, expr_size--);
            }
            iff_stack_push(&stack, LYS_IFF_AND);
        } else if (!strncmp(&c[i], "or", 2) && isspace(c[i + 2])) {
            while (stack.index && stack.stack[stack.index - 1] <= LYS_IFF_OR) {
                op = iff_stack_pop(&stack);
                iff_setop(iff->expr, op, expr_size--);
            }
            iff_stack_push(&stack, LYS_IFF_OR);
        } else {
            /* feature name, length is j - i */

            /* add it to the expression */
            iff_setop(iff->expr, LYS_IFF_F, expr_size--);

            /* now get the link to the feature definition */
            f = lysp_feature_find(qname->mod, &c[i], j - i, 1);
            if (!f) {
                LOGVAL(ctx, LYVE_SYNTAX_YANG, "Invalid value \"%s\" of if-feature - unable to find feature \"%.*s\".",
                        qname->str, (int)(j - i), &c[i]);
                rc = LY_EVALID;
                goto cleanup;
            }
            iff->features[f_size] = f;
            LY_ARRAY_INCREMENT(iff->features);
            f_size--;
        }
    }
    while (stack.index) {
        op = iff_stack_pop(&stack);
        iff_setop(iff->expr, op, expr_size--);
    }

    if (++expr_size || ++f_size) {
        /* not all expected operators and operands found */
        LOGVAL(ctx, LYVE_SYNTAX_YANG, "Invalid value \"%s\" of if-feature - processing error.", qname->str);
        rc = LY_EINT;
    }

cleanup:
    if (rc) {
        LY_ARRAY_FREE(iff->features);
        iff->features = NULL;
        free(iff->expr);
        iff->expr = NULL;
    }
    iff_stack_clean(&stack);
    return rc;
}

LY_ERR
lys_eval_iffeatures(const struct ly_ctx *ctx, const struct lysp_qname *iffeatures, ly_bool *enabled)
{
    LY_ERR ret;
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_iffeature iff;
    struct lysf_ctx fctx = {.ctx = (struct ly_ctx *)ctx};

    /* enabled by default */
    *enabled = 1;

    if (!iffeatures) {
        return LY_SUCCESS;
    }

    /* evaluate all if-feature conditions or until an unsatisfied one is found */
    LY_ARRAY_FOR(iffeatures, u) {
        memset(&iff, 0, sizeof iff);
        LY_CHECK_RET(lys_compile_iffeature(ctx, &iffeatures[u], &iff));

        ret = lysc_iffeature_value(&iff);
        lysc_iffeature_free(&fctx, &iff);
        if (ret == LY_ENOT) {
            *enabled = 0;
            break;
        } else if (ret) {
            return ret;
        }
    }

    return LY_SUCCESS;
}

LY_ERR
lys_check_features(const struct lysp_module *pmod)
{
    LY_ERR r;
    uint32_t i = 0;
    struct lysp_feature *f = NULL;

    while ((f = lysp_feature_next(f, pmod, &i))) {
        if (!(f->flags & LYS_FENABLED) || !f->iffeatures) {
            /* disabled feature or no if-features to check */
            continue;
        }

        assert(f->iffeatures_c);
        r = lysc_iffeature_value(f->iffeatures_c);
        if (r == LY_ENOT) {
            LOGERR(pmod->mod->ctx, LY_EDENIED, "Feature \"%s\" cannot be enabled because its \"if-feature\" is not satisfied.",
                    f->name);
            return LY_EDENIED;
        } else if (r) {
            return r;
        } /* else if-feature satisfied */
    }

    return LY_SUCCESS;
}

LY_ERR
lys_set_features(struct lysp_module *pmod, const char **features)
{
    uint32_t i = 0, j;
    struct lysp_feature *f = 0;
    ly_bool change = 0;

    if (!features) {
        /* do not touch the features */

    } else if (!features[0]) {
        /* disable all the features */
        while ((f = lysp_feature_next(f, pmod, &i))) {
            if (f->flags & LYS_FENABLED) {
                f->flags &= ~LYS_FENABLED;
                change = 1;
            }
        }
    } else if (!strcmp(features[0], "*")) {
        /* enable all the features */
        while ((f = lysp_feature_next(f, pmod, &i))) {
            if (!(f->flags & LYS_FENABLED)) {
                f->flags |= LYS_FENABLED;
                change = 1;
            }
        }
    } else {
        /* check that all the features exist */
        for (j = 0; features[j]; ++j) {
            if (!lysp_feature_find(pmod, features[j], strlen(features[j]), 0)) {
                LOGERR(pmod->mod->ctx, LY_EINVAL, "Feature \"%s\" not found in module \"%s\".", features[j], pmod->mod->name);
                return LY_EINVAL;
            }
        }

        /* enable specific features, disable the rest */
        while ((f = lysp_feature_next(f, pmod, &i))) {
            for (j = 0; features[j]; ++j) {
                if (!strcmp(f->name, features[j])) {
                    break;
                }
            }

            if (features[j] && !(f->flags & LYS_FENABLED)) {
                /* enable */
                f->flags |= LYS_FENABLED;
                change = 1;
            } else if (!features[j] && (f->flags & LYS_FENABLED)) {
                /* disable */
                f->flags &= ~LYS_FENABLED;
                change = 1;
            }
        }
    }

    if (!change) {
        /* features already set correctly */
        return LY_EEXIST;
    }

    return LY_SUCCESS;
}

/**
 * @brief Check circular dependency of features - feature MUST NOT reference itself (via their if-feature statement).
 *
 * The function works in the same way as lys_compile_identity_circular_check() with different structures and error messages.
 *
 * @param[in] ctx Compile context for logging.
 * @param[in] feature The feature referenced in if-feature statement (its depfeatures list is being extended by the feature
 *            being currently processed).
 * @param[in] depfeatures The list of depending features of the feature being currently processed (not the one provided as @p feature)
 * @return LY_SUCCESS if everything is ok.
 * @return LY_EVALID if the feature references indirectly itself.
 */
static LY_ERR
lys_compile_feature_circular_check(const struct ly_ctx *ctx, struct lysp_feature *feature, struct lysp_feature **depfeatures)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    struct ly_set recursion = {0};
    struct lysp_feature *drv;

    if (!depfeatures) {
        return LY_SUCCESS;
    }

    for (u = 0; u < LY_ARRAY_COUNT(depfeatures); ++u) {
        if (feature == depfeatures[u]) {
            LOGVAL(ctx, LYVE_REFERENCE, "Feature \"%s\" is indirectly referenced from itself.", feature->name);
            ret = LY_EVALID;
            goto cleanup;
        }
        ret = ly_set_add(&recursion, depfeatures[u], 0, NULL);
        LY_CHECK_GOTO(ret, cleanup);
    }

    for (v = 0; v < recursion.count; ++v) {
        drv = recursion.objs[v];
        for (u = 0; u < LY_ARRAY_COUNT(drv->depfeatures); ++u) {
            if (feature == drv->depfeatures[u]) {
                LOGVAL(ctx, LYVE_REFERENCE, "Feature \"%s\" is indirectly referenced from itself.", feature->name);
                ret = LY_EVALID;
                goto cleanup;
            }
            ly_set_add(&recursion, drv->depfeatures[u], 0, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    ly_set_erase(&recursion, NULL);
    return ret;
}

LY_ERR
lys_compile_feature_iffeatures(struct lysp_module *pmod)
{
    LY_ARRAY_COUNT_TYPE u, v;
    struct lysp_feature *f = NULL, **df;
    uint32_t idx = 0;

    while ((f = lysp_feature_next(f, pmod, &idx))) {
        if (!f->iffeatures) {
            continue;
        }

        /* compile if-features */
        LY_ARRAY_CREATE_RET(pmod->mod->ctx, f->iffeatures_c, LY_ARRAY_COUNT(f->iffeatures), LY_EMEM);
        LY_ARRAY_FOR(f->iffeatures, u) {
            LY_ARRAY_INCREMENT(f->iffeatures_c);
            LY_CHECK_RET(lys_compile_iffeature(pmod->mod->ctx, &(f->iffeatures)[u], &(f->iffeatures_c)[u]));
        }
        LY_ARRAY_FOR(f->iffeatures_c, u) {
            LY_ARRAY_FOR(f->iffeatures_c[u].features, v) {
                /* check for circular dependency - direct reference first,... */
                if (f == f->iffeatures_c[u].features[v]) {
                    LOGVAL(pmod->mod->ctx, LYVE_REFERENCE, "Feature \"%s\" is referenced from itself.", f->name);
                    return LY_EVALID;
                }
                /* ... and indirect circular reference */
                LY_CHECK_RET(lys_compile_feature_circular_check(pmod->mod->ctx, f->iffeatures_c[u].features[v], f->depfeatures));

                /* add itself into the dependants list */
                LY_ARRAY_NEW_RET(pmod->mod->ctx, f->iffeatures_c[u].features[v]->depfeatures, df, LY_EMEM);
                *df = f;
            }
        }
    }

    return LY_SUCCESS;
}
