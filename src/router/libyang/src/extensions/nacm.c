/**
 * @file nacm.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang extension plugin - NACM (RFC 6536)
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

#include <stdlib.h>

#include "../extensions.h"

/**
 * @brief Storage for ID used to check plugin API version compatibility.
 */
LYEXT_VERSION_CHECK

/**
 * @brief Callback to check that the NACM extension can be instantiated inside the provided node
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
int nacm_position(const void *parent, LYEXT_PAR parent_type, LYEXT_SUBSTMT UNUSED(substmt_type))
{
    if (parent_type != LYEXT_PAR_NODE) {
        return 1;
    }

    if (((struct lys_node*)parent)->nodetype &
            (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_CHOICE | LYS_ANYDATA | LYS_AUGMENT | LYS_CASE |
             LYS_USES | LYS_RPC | LYS_ACTION | LYS_NOTIF )) {
        return 0;
    } else {
        return 1;
    }
}

/**
 * @brief Callback to decide whether the extension will be inherited into the provided schema node. The extension
 * instance is always from some of the node's parents.
 *
 * @param[in] ext Extension instance to be inherited.
 * @param[in] node Schema node where the node is supposed to be inherited.
 * @return 0 - yes
 *         1 - no (do not process the node's children)
 *         2 - no, but continue with children
 */
int nacm_inherit(struct lys_ext_instance *UNUSED(ext), struct lys_node *node)
{
    /* libyang already checks if there is explicit instance of the extension already present,
     * in such a case the extension is never inherited and we don't need to check it here */

    /* inherit into all the schema nodes that can be instantiated in data trees */
    if (node->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_ACTION | LYS_NOTIF )) {
        return 0;
    } else {
        return 2;
    }
}

/**
 * @brief Callback to check that the extension instance is correct - have
 * the valid argument, cardinality, etc.
 *
 * In NACM case, we are checking only the cardinality.
 *
 * @param[in] ext Extension instance to be checked.
 * @return 0 - ok
 *         1 - error
 */
int
nacm_cardinality(struct lys_ext_instance *ext)
{
    struct lys_ext_instance **extlist;
    uint8_t extsize, i, c;
    char *path;

    if (ext->flags & LYEXT_OPT_PLUGIN1) {
        /* already checked */
        ext->flags &= ~LYEXT_OPT_PLUGIN1;
        return 0;
    }

    extlist = ((struct lys_node *)ext->parent)->ext;
    extsize = ((struct lys_node *)ext->parent)->ext_size;

    for (i = c = 0; i < extsize; i++) {
        if (extlist[i]->def == ext->def) {
            /* note, that it is not necessary to check also ext->insubstmt since
             * nacm_position() ensures that NACM's extension instances are placed only
             * in schema nodes */
            if (extlist[i] != ext) {
                /* do not mark the instance being checked */
                extlist[i]->flags |= LYEXT_OPT_PLUGIN1;
            }
            c++;
        }
    }

    if (c > 1) {
        path = lys_path((struct lys_node *)(ext->parent), LYS_PATH_FIRST_PREFIX);
        LYEXT_LOG(ext->module->ctx, LY_LLERR, "NACM", "Extension nacm:%s can appear only once, but %d instances found in %s.",
                  ext->def->name, c, path);
        free(path);
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Plugin for the NACM's default-deny-write extension
 */
struct lyext_plugin nacm_deny_write = {
    .type = LYEXT_FLAG,
    .flags = LYEXT_OPT_INHERIT,
    .check_position = &nacm_position,
    .check_result = &nacm_cardinality,
    .check_inherit = &nacm_inherit
};

/**
 * @brief Plugin for the NACM's default-deny-all extension
 */
struct lyext_plugin nacm_deny_all = {
    .type = LYEXT_FLAG,
    .flags = LYEXT_OPT_INHERIT,
    .check_position = &nacm_position,
    .check_result = &nacm_cardinality,
    .check_inherit = &nacm_inherit
};

/**
 * @brief list of all extension plugins implemented here
 *
 * MANDATORY object for all libyang extension plugins, the name must match the <name>.so
 */
struct lyext_plugin_list nacm[] = {
    {"ietf-netconf-acm", "2012-02-22", "default-deny-write", &nacm_deny_write},
    {"ietf-netconf-acm", "2012-02-22", "default-deny-all", &nacm_deny_all},
    {NULL, NULL, NULL, NULL} /* terminating item */
};
