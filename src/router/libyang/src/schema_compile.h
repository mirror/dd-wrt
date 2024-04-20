/**
 * @file schema_compile.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Header for schema compilation.
 *
 * Copyright (c) 2015 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_SCHEMA_COMPILE_H_
#define LY_SCHEMA_COMPILE_H_

#include <stddef.h>
#include <stdint.h>

#include "log.h"
#include "plugins_exts.h"
#include "set.h"
#include "tree.h"
#include "tree_schema.h"
#include "tree_schema_free.h"

struct lyxp_expr;

/**
 * @brief YANG schema compilation context.
 */
struct lysc_ctx {
    struct ly_ctx *ctx;         /**< libyang context */
    struct lys_module *cur_mod; /**< module currently being compiled,
                                     - identifier/path - used as the current module for unprefixed nodes
                                     - augment - module where the augment is defined
                                     - deviation - module where the deviation is defined
                                     - uses - module where the uses is defined */
    struct lysp_module *pmod;   /**< parsed module being processed,
                                     - identifier/path - used for searching imports to resolve prefixed nodes
                                     - augment - module where the augment is defined
                                     - deviation - module where the deviation is defined
                                     - uses - module where the grouping is defined */
    struct lysc_ext_instance *ext; /**< extension instance being processed and serving as a source for its substatements
                                     instead of the module itself */
    struct ly_set groupings;    /**< stack for groupings circular check */
    struct ly_set tpdf_chain;   /**< stack for typedefs circular check */
    struct ly_set augs;         /**< set of compiled non-applied top-level augments (stored ::lysc_augment *) */
    struct ly_set devs;         /**< set of compiled non-applied deviations (stored ::lysc_deviation *) */
    struct ly_set uses_augs;    /**< set of compiled non-applied uses augments (stored ::lysc_augment *) */
    struct ly_set uses_rfns;    /**< set of compiled non-applied uses refines (stored ::lysc_refine *) */
    struct lys_depset_unres *unres; /**< dependency set unres sets */
    uint32_t path_len;          /**< number of path bytes used */
    uint32_t compile_opts;      /**< various @ref scflags. */
    struct lysf_ctx free_ctx;   /**< freeing context for errors/recompilation */

#define LYSC_CTX_BUFSIZE 4078
    char path[LYSC_CTX_BUFSIZE];/**< Path identifying the schema node currently being processed */
};

/**
 * @brief Initalize local compilation context using libyang context.
 *
 * @param[out] CCTX Compile context.
 * @param[in] CTX libyang context.
 */
#define LYSC_CTX_INIT_CTX(CCTX, CTX) \
    memset(&(CCTX), 0, sizeof (CCTX)); \
    (CCTX).ctx = (CTX); \
    (CCTX).path_len = 1; \
    (CCTX).path[0] = '/'; \
    (CCTX).free_ctx.ctx = (CTX)

/**
 * @brief Initalize local compilation context using a parsed module.
 *
 * @param[out] CCTX Compile context.
 * @param[in] PMOD Parsed module.
 * @param[in] EXT Ancestor extension instance.
 */
#define LYSC_CTX_INIT_PMOD(CCTX, PMOD, EXT) \
    memset(&(CCTX), 0, sizeof (CCTX)); \
    (CCTX).ctx = (PMOD)->mod->ctx; \
    (CCTX).cur_mod = (PMOD)->mod; \
    (CCTX).pmod = (PMOD); \
    (CCTX).ext = (EXT); \
    (CCTX).path_len = 1; \
    (CCTX).path[0] = '/'; \
    (CCTX).free_ctx.ctx = (PMOD)->mod->ctx

/**
 * @brief Structure for unresolved items that may depend on any implemented module data in the dependency set
 * so their resolution can only be performed after the whole dep set compilation is done.
 */
struct lys_depset_unres {
    struct ly_set whens;                /**< nodes with when to check */
    struct ly_set musts;                /**< set of musts to check */
    struct ly_set leafrefs;             /**< to validate target of leafrefs */
    struct ly_set dflts;                /**< set of incomplete default values */
    struct ly_set disabled;             /**< set of compiled nodes whose if-feature(s) was not satisfied
                                             (stored ::lysc_node *) */
    struct ly_set disabled_leafrefs;    /**< subset of the lys_depset_unres.disabled to validate target of disabled leafrefs */
    struct ly_set disabled_bitenums;    /**< set of enumation/bits leaves/leaf-lists with bits/enums to disable
                                             (stored ::lysc_node_leaf *) */
};

/**
 * @brief Unres structure global for compilation.
 */
struct lys_glob_unres {
    struct ly_set dep_sets;     /**< set of dependency sets of modules, see ::lys_compile_depset_all() */
    struct ly_set implementing; /**< set of YANG schemas being atomically implemented (compiled); the first added
                                    module is always the explicitly implemented module, the other ones are dependencies */
    struct ly_set creating;     /**< set of YANG schemas being atomically created (parsed); it is a subset of implemented
                                    and all these modules are freed if any error occurs */
    struct lys_depset_unres ds_unres;   /**< unres specific for the current dependency set */
};

/**
 * @brief Structure for storing schema node with a when expression.
 */
struct lysc_unres_when {
    struct lysc_node *node;                 /**< node with the when expression */
    struct lysc_when *when;                 /**< one when expression of the node */
};

/**
 * @brief Structure for storing schema nodes with must expressions and local module for each of them.
 */
struct lysc_unres_must {
    struct lysc_node *node;                 /**< node with the must expression(s) */
    const struct lysp_module **local_mods;  /**< sized array of local modules for must(s) */
    struct lysc_ext_instance *ext;          /**< ancestor extension instance of the must(s) */
};

/**
 * @brief Structure for storing leafref node and its local module.
 */
struct lysc_unres_leafref {
    struct lysc_node *node;                 /**< leaf/leaf-list node with leafref type */
    const struct lysp_module *local_mod;    /**< local module of the leafref type */
    struct lysc_ext_instance *ext;          /**< ancestor extension instance of the leafref */
};

/**
 * @brief Structure for remembering default values of leaves and leaf-lists. They are resolved at schema compilation
 * end when the whole schema tree is available.
 */
struct lysc_unres_dflt {
    union {
        struct lysc_node_leaf *leaf;
        struct lysc_node_leaflist *llist;
    };
    struct lysp_qname *dflt;
    struct lysp_qname *dflts;           /**< this is a sized array */
};

/**
 * @brief Duplicate string into dictionary
 * @param[in] CTX libyang context of the dictionary.
 * @param[in] ORIG String to duplicate.
 * @param[out] DUP Where to store the result.
 * @param[out] RET Where to store the return code.
 */
#define DUP_STRING(CTX, ORIG, DUP, RET) RET = lydict_insert(CTX, ORIG, 0, &(DUP))
#define DUP_STRING_RET(CTX, ORIG, DUP) LY_CHECK_RET(lydict_insert(CTX, ORIG, 0, &(DUP)))
#define DUP_STRING_GOTO(CTX, ORIG, DUP, RET, GOTO) LY_CHECK_GOTO(RET = lydict_insert(CTX, ORIG, 0, &(DUP)), GOTO)

#define DUP_ARRAY(CTX, ORIG_ARRAY, NEW_ARRAY, DUP_FUNC) \
    if (ORIG_ARRAY) { \
        LY_ARRAY_COUNT_TYPE __u; \
        LY_ARRAY_CREATE_RET(CTX, NEW_ARRAY, LY_ARRAY_COUNT(ORIG_ARRAY), LY_EMEM); \
        LY_ARRAY_FOR(ORIG_ARRAY, __u) { \
            LY_ARRAY_INCREMENT(NEW_ARRAY); \
            LY_CHECK_RET(DUP_FUNC(CTX, &(ORIG_ARRAY)[__u], &(NEW_ARRAY)[__u])); \
        } \
    }

#define DUP_ARRAY2(CTX, PMOD, ORIG_ARRAY, NEW_ARRAY, DUP_FUNC) \
    if (ORIG_ARRAY) { \
        LY_ARRAY_COUNT_TYPE __u; \
        LY_ARRAY_CREATE_RET(CTX, NEW_ARRAY, LY_ARRAY_COUNT(ORIG_ARRAY), LY_EMEM); \
        LY_ARRAY_FOR(ORIG_ARRAY, __u) { \
            LY_ARRAY_INCREMENT(NEW_ARRAY); \
            LY_CHECK_RET(DUP_FUNC(CTX, PMOD, &(ORIG_ARRAY)[__u], &(NEW_ARRAY)[__u])); \
        } \
    }

#define DUP_EXTS(CTX, PMOD, PARENT, PARENT_STMT, ORIG_ARRAY, NEW_ARRAY, DUP_FUNC) \
    if (ORIG_ARRAY) { \
        LY_ARRAY_COUNT_TYPE __u, __new_start; \
        __new_start = LY_ARRAY_COUNT(NEW_ARRAY); \
        LY_ARRAY_CREATE_RET(CTX, NEW_ARRAY, LY_ARRAY_COUNT(ORIG_ARRAY), LY_EMEM); \
        LY_ARRAY_FOR(ORIG_ARRAY, __u) { \
            LY_ARRAY_INCREMENT(NEW_ARRAY); \
            LY_CHECK_RET(DUP_FUNC(CTX, PMOD, PARENT, PARENT_STMT, &(ORIG_ARRAY)[__u], &(NEW_ARRAY)[__new_start + __u])); \
        } \
    }

#define COMPILE_OP_ARRAY_GOTO(CTX, ARRAY_P, ARRAY_C, PARENT, FUNC, USES_STATUS, RET, GOTO) \
    if (ARRAY_P) { \
        LY_ARRAY_COUNT_TYPE __u = (ARRAY_C) ? LY_ARRAY_COUNT(ARRAY_C) : 0; \
        LY_ARRAY_CREATE_GOTO((CTX)->ctx, ARRAY_C, __u + LY_ARRAY_COUNT(ARRAY_P), RET, GOTO); \
        LY_ARRAY_FOR(ARRAY_P, __u) { \
            LY_ARRAY_INCREMENT(ARRAY_C); \
            RET = FUNC(CTX, &(ARRAY_P)[__u], PARENT, &(ARRAY_C)[LY_ARRAY_COUNT(ARRAY_C) - 1], USES_STATUS); \
            if (RET == LY_EDENIED) { \
                LY_ARRAY_DECREMENT(ARRAY_C); \
                RET = LY_SUCCESS; \
            } else if (RET) { \
                goto GOTO; \
            } \
        } \
    }

#define COMPILE_ARRAY_GOTO(CTX, ARRAY_P, ARRAY_C, FUNC, RET, GOTO) \
    if (ARRAY_P) { \
        LY_ARRAY_COUNT_TYPE __u = (ARRAY_C) ? LY_ARRAY_COUNT(ARRAY_C) : 0; \
        LY_ARRAY_CREATE_GOTO((CTX)->ctx, ARRAY_C, __u + LY_ARRAY_COUNT(ARRAY_P), RET, GOTO); \
        LY_ARRAY_FOR(ARRAY_P, __u) { \
            LY_ARRAY_INCREMENT(ARRAY_C); \
            RET = FUNC(CTX, &(ARRAY_P)[__u], &(ARRAY_C)[LY_ARRAY_COUNT(ARRAY_C) - 1]); \
            LY_CHECK_GOTO(RET, GOTO); \
        } \
    }

#define COMPILE_EXTS_GOTO(CTX, EXTS_P, EXT_C, PARENT, RET, GOTO) \
    if (EXTS_P) { \
        LY_ARRAY_COUNT_TYPE __u = (EXT_C) ? LY_ARRAY_COUNT(EXT_C) : 0; \
        LY_ARRAY_CREATE_GOTO((CTX)->ctx, EXT_C, __u + LY_ARRAY_COUNT(EXTS_P), RET, GOTO); \
        LY_ARRAY_FOR(EXTS_P, __u) { \
            LY_ARRAY_INCREMENT(EXT_C); \
            RET = lys_compile_ext(CTX, &(EXTS_P)[__u], &(EXT_C)[LY_ARRAY_COUNT(EXT_C) - 1], PARENT); \
            if (RET == LY_ENOT) { \
                LY_ARRAY_DECREMENT(EXT_C); \
                RET = LY_SUCCESS; \
            } else if (RET) { \
                goto GOTO; \
            } \
        } \
    }

/**
 * @brief Update path in the compile context, which is used for logging where the compilation failed.
 *
 * @param[in] ctx Compile context with the path.
 * @param[in] parent_module Module of the current node's parent to check difference with the currently processed module
 * (taken from @p ctx).
 * @param[in] name Name of the node to update path with. If NULL, the last segment is removed. If the format is
 * `{keyword}`, the following call updates the segment to the form `{keyword='name'}` (to remove this compound segment,
 * 2 calls with NULL @p name must be used).
 */
void lysc_update_path(struct lysc_ctx *ctx, const struct lys_module *parent_module, const char *name);

/**
 * @brief Fill in the prepared compiled extension instance structure according to the parsed extension instance.
 *
 * @param[in] ctx Compilation context.
 * @param[in] extp Parsed extension instance.
 * @param[in,out] ext Prepared compiled extension instance.
 * @param[in] parent Extension instance parent.
 * @return LY_SUCCESS on success.
 * @return LY_ENOT if the extension is disabled and should be ignored.
 * @return LY_ERR on error.
 */
LY_ERR lys_compile_ext(struct lysc_ctx *ctx, struct lysp_ext_instance *extp, struct lysc_ext_instance *ext, void *parent);

/**
 * @brief Compile information from the identity statement
 *
 * The backlinks to the identities derived from this one are supposed to be filled later via ::lys_compile_identity_bases().
 *
 * @param[in] ctx_sc Compile context - alternative to the combination of @p ctx and @p parsed_mod.
 * @param[in] ctx libyang context.
 * @param[in] parsed_mod Module with the identities.
 * @param[in] identities_p Array of the parsed identity definitions to precompile.
 * @param[in,out] identities Pointer to the storage of the (pre)compiled identities array where the new identities are
 * supposed to be added. The storage is supposed to be initiated to NULL when the first parsed identities are going
 * to be processed.
 * @return LY_ERR value.
 */
LY_ERR lys_identity_precompile(struct lysc_ctx *ctx_sc, struct ly_ctx *ctx, struct lysp_module *parsed_mod,
        const struct lysp_ident *identities_p, struct lysc_ident **identities);

/**
 * @brief Find and process the referenced base identities from another identity or identityref
 *
 * For bases in identity set backlinks to them from the base identities. For identityref, store
 * the array of pointers to the base identities. So one of the ident or bases parameter must be set
 * to distinguish these two use cases.
 *
 * @param[in] ctx Compile context, not only for logging but also to get the current module to resolve prefixes.
 * @param[in] base_pmod Module where to resolve @p bases_p prefixes.
 * @param[in] bases_p Array of names (including prefix if necessary) of base identities.
 * @param[in] ident Referencing identity to work with, NULL for identityref.
 * @param[in] bases Array of bases of identityref to fill in.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_identity_bases(struct lysc_ctx *ctx, const struct lysp_module *base_pmod, const char **bases_p,
        struct lysc_ident *ident, struct lysc_ident ***bases);

/**
 * @brief Perform a complet compilation of identites in a module and all its submodules.
 *
 * @param[in] mod Module to process.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_identities(struct lys_module *mod);

/**
 * @brief Compile schema into a validated schema linking all the references. Must have been implemented before.
 *
 * @param[in] mod Pointer to the schema structure holding pointers to both schema structure types. The ::lys_module#parsed
 * member is used as input and ::lys_module#compiled is used to hold the result of the compilation.
 * @param[in,out] unres Dep set unres structure to add to.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
LY_ERR lys_compile(struct lys_module *mod, struct lys_depset_unres *unres);

/**
 * @brief Check statement's status for invalid combination.
 *
 * The modX parameters are used just to determine if both flags are in the same module,
 * so any of the schema module structure can be used, but both modules must be provided
 * in the same type.
 *
 * @param[in] ctx Compile context for logging.
 * @param[in] flags1 Flags of the referencing node.
 * @param[in] mod1 Module of the referencing node,
 * @param[in] name1 Schema node name of the referencing node.
 * @param[in] flags2 Flags of the referenced node.
 * @param[in] mod2 Module of the referenced node,
 * @param[in] name2 Schema node name of the referenced node.
 * @return LY_ERR value
 */
LY_ERR lysc_check_status(struct lysc_ctx *ctx, uint16_t flags1, void *mod1, const char *name1, uint16_t flags2,
        void *mod2, const char *name2);

/**
 * @brief Check parsed expression for any prefixes of unimplemented modules.
 *
 * @param[in] ctx libyang context.
 * @param[in] expr Parsed expression.
 * @param[in] format Prefix format.
 * @param[in] prefix_data Format-specific data (see ::ly_resolve_prefix()).
 * @param[in] implement Whether all the non-implemented modules should are implemented or the first
 * non-implemented module, if any, returned in @p mod_p.
 * @param[in,out] unres Global unres structure of newly implemented modules.
 * @param[out] mod_p Module that is not implemented.
 * @return LY_SUCCESS on success.
 * @return LY_ERECOMPILE if @p implement is set.
 * @return LY_ERR on error.
 */
LY_ERR lys_compile_expr_implement(const struct ly_ctx *ctx, const struct lyxp_expr *expr, LY_VALUE_FORMAT format,
        void *prefix_data, ly_bool implement, struct lys_glob_unres *unres, const struct lys_module **mod_p);

/**
 * @brief Compile all flagged modules in a dependency set, recursively if recompilation is needed.
 *
 * Steps taken when adding a new module (::ly_ctx_load_module(), ::lys_parse()):
 *
 * 1) parse module and add it into context with all imports and includes also parsed and in context
 *    (::lys_parse_load(), ::lys_parse_in(), lys_parse_localfile() - static)
 * 2) implement it (perform one-time compilation tasks - compile identities and add reference to augment/deviation
 *    target modules, implement those as well, ::_lys_set_implemented())
 * 3) create dep set of the module (::lys_unres_dep_sets_create())
 * 4) (re)compile all the modules in the dep set and collect unres (::lys_compile_dep_set_r())
 * 5) resolve unres (lys_compile_unres_depset() - static), new modules may be implemented like in 2) and if
 *    require recompilation, free all compiled modules and do 4)
 * 6) all modules that needed to be (re)compiled are now, with all their dependencies
 *
 * What can cause new modules to be implemented when resolving unres in 5):
 * - leafref
 * - when, must
 * - identityref, instance-identifier default value
 * - new implemented module augments, deviations
 *
 * @param[in] ctx libyang context.
 * @param[in,out] unres Global unres to use.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_depset_all(struct ly_ctx *ctx, struct lys_glob_unres *unres);

/**
 * @brief Implement a single module. Does not actually compile, only marks to_compile!
 *
 * @param[in] mod Module to implement.
 * @param[in] features Features to set, see ::lys_set_features().
 * @param[in,out] unres Global unres to use.
 * @return LY_ERR value.
 */
LY_ERR lys_implement(struct lys_module *mod, const char **features, struct lys_glob_unres *unres);

#endif /* LY_SCHEMA_COMPILE_H_ */
