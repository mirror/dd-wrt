/**
 * @file schema_compile.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Header for schema compilation.
 *
 * Copyright (c) 2015 - 2020 CESNET, z.s.p.o.
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

#include "common.h"
#include "dict.h"
#include "log.h"
#include "set.h"
#include "tree.h"
#include "tree_edit.h"
#include "tree_schema.h"

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
    struct lys_glob_unres *unres;  /**< global unres sets */
    uint32_t path_len;          /**< number of path bytes used */
    uint32_t options;           /**< various @ref scflags. */
#define LYSC_CTX_BUFSIZE 4078
    char path[LYSC_CTX_BUFSIZE];/**< Path identifying the schema node currently being processed */
};

/**
 * @brief Structure for unresolved items that may depend on any implemented module data so their resolution
 * can only be performed after all module basic compilation is done.
 */
struct lys_glob_unres {
    struct ly_set implementing; /**< set of YANG schemas being atomically implemented (compiled); the first added
                                    module is always the explcitly implemented module, the other ones are dependencies */
    struct ly_set creating;     /**< set of YANG schemas being atomically created (parsed); it is a subset of implemented
                                    and all these modules are freed if any error occurs */
    struct ly_set xpath;        /**< when/must to check */
    struct ly_set leafrefs;     /**< to validate leafref's targets */
    struct ly_set dflts;        /**< set of incomplete default values */
    struct ly_set disabled;     /**< set of compiled nodes whose if-feature(s) was not satisfied (stored ::lysc_node *) */
    ly_bool full_compilation;   /**< flag marking that all the currently implemented modules were compiled in this
                                    compilation (meaning that all their disabled nodes are still present) */
    ly_bool recompile;          /**< flag marking that a module needs to be recompiled for this compilation to succeed */
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
 */
#define DUP_STRING(CTX, ORIG, DUP, RET) if (ORIG) {RET = lydict_insert(CTX, ORIG, 0, &DUP);}
#define DUP_STRING_RET(CTX, ORIG, DUP) if (ORIG) {LY_ERR __ret = lydict_insert(CTX, ORIG, 0, &DUP); LY_CHECK_RET(__ret);}
#define DUP_STRING_GOTO(CTX, ORIG, DUP, RET, GOTO) if (ORIG) {LY_CHECK_GOTO(RET = lydict_insert(CTX, ORIG, 0, &DUP), GOTO);}

#define DUP_ARRAY(CTX, ORIG_ARRAY, NEW_ARRAY, DUP_FUNC) \
    if (ORIG_ARRAY) { \
        LY_ARRAY_COUNT_TYPE __u; \
        LY_ARRAY_CREATE_RET(CTX, NEW_ARRAY, LY_ARRAY_COUNT(ORIG_ARRAY), LY_EMEM); \
        LY_ARRAY_FOR(ORIG_ARRAY, __u) { \
            LY_ARRAY_INCREMENT(NEW_ARRAY); \
            LY_CHECK_RET(DUP_FUNC(CTX, &(NEW_ARRAY)[__u], &(ORIG_ARRAY)[__u])); \
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
            RET = lys_compile_ext(CTX, &(EXTS_P)[__u], &(EXT_C)[LY_ARRAY_COUNT(EXT_C) - 1], PARENT, NULL); \
            if (RET == LY_ENOT) { \
                LY_ARRAY_DECREMENT(EXT_C); \
                RET = LY_SUCCESS; \
            } else if (RET) { \
                goto GOTO; \
            } \
        } \
    }

/**
 * @brief Fill in the prepared compiled extension instance structure according to the parsed extension instance.
 *
 * @param[in] ctx Compilation context.
 * @param[in] ext_p Parsed extension instance.
 * @param[in,out] ext Prepared compiled extension instance.
 * @param[in] parent Extension instance parent.
 * @param[in] ext_mod Optional module with the extension instance extension definition, set only for internal annotations.
 * @return LY_SUCCESS on success.
 * @return LY_ENOT if the extension is disabled and should be ignored.
 * @return LY_ERR on error.
 */
LY_ERR lys_compile_ext(struct lysc_ctx *ctx, struct lysp_ext_instance *ext_p, struct lysc_ext_instance *ext, void *parent,
        const struct lys_module *ext_mod);

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
        struct lysp_ident *identities_p, struct lysc_ident **identities);

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
 * @param[in] enabled Whether the base is disabled, must be set if @p ident is set.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_identity_bases(struct lysc_ctx *ctx, const struct lysp_module *base_pmod, const char **bases_p,
        struct lysc_ident *ident, struct lysc_ident ***bases, ly_bool *enabled);

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
 * @brief Finish compilation of all the global unres sets.
 * Will always finish the compilation (never return @p unres with `recompile` set).
 *
 * @param[in] ctx libyang context.
 * @param[in] unres Global unres structure with the sets to resolve.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_unres_glob(struct ly_ctx *ctx, struct lys_glob_unres *unres);

/**
 * @brief Revert a failed compilation (free new modules, unimplement newly implemented modules).
 *
 * @param[in] ctx libyang context.
 * @param[in] unres Global unres set with newly implemented modules.
 */
void lys_compile_unres_glob_revert(struct ly_ctx *ctx, struct lys_glob_unres *unres);

/**
 * @brief Erase all the global unres sets.
 *
 * @param[in] ctx libyang context.
 * @param[in] unres Global unres structure with the sets.
 */
void lys_compile_unres_glob_erase(const struct ly_ctx *ctx, struct lys_glob_unres *unres);

/**
 * @brief Recompile the whole context based on the current flags.
 *
 * @param[in] ctx Context to recompile.
 * @param[in] log Whether to log all the errors.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
LY_ERR lys_recompile(struct ly_ctx *ctx, ly_bool log);

/**
 * @brief Compile printable schema into a validated schema linking all the references.
 *
 * @param[in] mod Pointer to the schema structure holding pointers to both schema structure types. The ::lys_module#parsed
 * member is used as input and ::lys_module#compiled is used to hold the result of the compilation.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @param[in,out] unres Global unres structure for newly implemented modules. If `recompile` was set, @p mod
 * was actually not compiled because there is at least one compiled imported module that must be recompiled first.
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
LY_ERR lys_compile(struct lys_module *mod, uint32_t options, struct lys_glob_unres *unres);

#endif /* LY_SCHEMA_COMPILE_H_ */
