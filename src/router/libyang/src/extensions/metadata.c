/**
 * @file metadata.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang extension plugin - YANG Metadata (annotations) (RFC 7952)
 *
 * Copyright (c) 2016-2017 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#include "../extensions.h"

/**
 * @brief Storage for ID used to check plugin API version compatibility.
 */
LYEXT_VERSION_CHECK

/**
 * @brief Callback to check that the annotation can be instantiated inside the provided node
 *
 * @param[in] parent The parent of the instantiated extension.
 * @param[in] parent_type The type of the structure provided as \p parent.
 * @param[in] substmt_type libyang does not store all the extension instances in the structures where they are
 *                         instantiated in the module. In some cases (see #LYEXT_SUBSTMT) they are stored in parent
 *                         structure and marked with flag to know in which substatement of the parent the extension
 *                         was originally instantiated.
 * @return 0 - ok
 *         1 - error
 */
int annotation_position(const void * UNUSED(parent), LYEXT_PAR parent_type, LYEXT_SUBSTMT UNUSED(substmt_type))
{
    /* annotations can appear only at the top level of a YANG module or submodule */
    if (parent_type == LYEXT_PAR_MODULE) {
        return 0;
    } else {
        return 1;
    }
}

/**
 * @brief Callback to check that the extension instance is correct - have
 * the valid argument, cardinality, etc.
 *
 * In Metadata case, we are checking for the annotation names duplication.
 *
 * @param[in] ext Extension instance to be checked.
 * @return 0 - ok
 *         1 - error
 */
int
annotation_final_check(struct lys_ext_instance *ext)
{
    uint8_t  i, j, c;
    struct lys_module *mod;
    struct lys_submodule *submod;
    struct lys_type *type;

    /*
     * check type - leafref is not allowed
     */
    type = *(struct lys_type**)lys_ext_complex_get_substmt(LY_STMT_TYPE, (struct lys_ext_instance_complex *)ext, NULL);
    if (type->base == LY_TYPE_LEAFREF) {
        LYEXT_LOG(ext->module->ctx, LY_LLERR, "Annotations", "The leafref type is not supported for annotations (annotation %s).",
                  ext->arg_value);
        return 1;
    }

    /*
     * check duplication
     */
    if (ext->flags & LYEXT_OPT_PLUGIN1) {
        /* already checked */
        ext->flags &= ~LYEXT_OPT_PLUGIN1;
        return 0;
    }

    mod = lys_main_module((struct lys_module *)ext->parent);

    for (i = c = 0; i < mod->ext_size; i++) {
        /* note, that it is not necessary to check also ext->insubstmt since
         * annotation_position() ensures that annotation instances are placed only
         * in module or submodule */
        if (mod->ext[i]->def == ext->def && mod->ext[i]->arg_value == ext->arg_value) {
            if (mod->ext[i] != ext) {
                /* do not mark the instance being checked */
                mod->ext[i]->flags |= LYEXT_OPT_PLUGIN1;
            }
            c++;
        }
    }
    /* similarly, go through the all extensions in the main module submodules */
    for (j = 0; j < mod->inc_size; j++) {
        submod = mod->inc[j].submodule; /* shortcut */
        for (i = 0; i < submod->ext_size; i++) {
            if (submod->ext[i]->def == ext->def && submod->ext[i]->arg_value == ext->arg_value) {
                if (submod->ext[i] != ext) {
                    /* do not mark the instance being checked */
                    submod->ext[i]->flags |= LYEXT_OPT_PLUGIN1;
                }
                c++;
            }
        }
    }

    if (c > 1) {
        LYEXT_LOG(ext->module->ctx, LY_LLERR, "Annotations",
                  "Annotation instance %s is not unique, there are %d instances with the same name in module %s.",
                  ext->arg_value, c, ((struct lys_module *)ext->parent)->name);
        return 1;
    } else {
        return 0;
    }
}
/**
 * extension instance's content:
 * struct lys_type *type;
 * const char* dsc;
 * const char* ref;
 * const char* units;
 * struct lys_iffeature **iff;
 * uint16_t status;
 *
 * - placement in the structure is specified via offsets
 * - the order in lyext_substmt structure specified the canonical order in which the items are printed
 */
struct lyext_substmt annotation_substmt[] = {
    {LY_STMT_IFFEATURE, 4 * sizeof(void *),  LY_STMT_CARD_ANY},
    {LY_STMT_TYPE, 0, LY_STMT_CARD_MAND},
    {LY_STMT_UNITS, 3 * sizeof(void *),  LY_STMT_CARD_OPT},
    {LY_STMT_STATUS, 5 * sizeof(void *),  LY_STMT_CARD_OPT},
    {LY_STMT_DESCRIPTION, 1 * sizeof(void *),  LY_STMT_CARD_OPT},
    {LY_STMT_REFERENCE, 2 * sizeof(void *),  LY_STMT_CARD_OPT},
    {0, 0, 0} /* terminating item */
};

/**
 * @brief Plugin for the RFC 7952's annotation extension
 */
struct lyext_plugin_complex annotation = {
    .type = LYEXT_COMPLEX,
    .flags = 0,
    .check_position = &annotation_position,
    .check_result = &annotation_final_check,
    .check_inherit = NULL,

    /* specification of allowed substatements of the extension instance */
    .substmt = annotation_substmt,

    /* final size of the extension instance structure with the space for storing the substatements */
    .instance_size = (sizeof(struct lys_ext_instance_complex) - 1) + 5 * sizeof(void*) + sizeof(uint16_t)
};

/**
 * @brief list of all extension plugins implemented here
 *
 * MANDATORY object for all libyang extension plugins, the name must match the <name>.so
 */
struct lyext_plugin_list metadata[] = {
    {"ietf-yang-metadata", "2016-08-05", "annotation", (struct lyext_plugin*)&annotation},
    {NULL, NULL, NULL, NULL} /* terminating item */
};
