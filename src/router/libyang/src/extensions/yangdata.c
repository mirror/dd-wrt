/**
 * @file yangdata.c
 * @author Pavol Vican <vican.pavol@gmail.com>
 * @brief libyang extension plugin - YANG DATA (RFC 8040)
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
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

int check_node(struct lys_node *node);

/**
 * @brief Callback to check that the yang-data can be instantiated inside the provided node
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
int yang_data_position(const void * UNUSED(parent), LYEXT_PAR parent_type, LYEXT_SUBSTMT UNUSED(substmt_type))
{
    /* yang-data can appear only at the top level of a YANG module or submodule */
    if (parent_type == LYEXT_PAR_MODULE) {
        return 0;
    } else {
        return 1;
    }
}

/* return values - 0 - OK
 *                 1 - Something wrong
 *                -1 - Absolute wrong
 */
int check_choice(struct lys_node *root) {
    struct lys_node *node, *next;
    int result = 1, tmp_result;

    LY_TREE_FOR_SAFE(root->child, next, node) {
        tmp_result = (node->nodetype == LYS_CASE) ? check_node(node->child) : check_node(node);
        if (tmp_result == -1) {
            return -1;
        } else if (tmp_result == 0) {
            result = 0;
        }
    }

    return result;
}

/* return values - 0 - OK
 *                 1 - Something wrong
 *                -1 - Absolute wrong
 */
int check_node(struct lys_node *node) {

    int result = 0;

    if (node == NULL) {
        return 1;
    }

    /* check nodes and find only one container */
    if (node->nodetype == LYS_CHOICE) {
        result = check_choice(node);
    } else if (node->nodetype == LYS_USES) {
        result = check_node(((struct lys_node_uses*)node)->grp->child);
    } else if (node->nodetype != LYS_CONTAINER || (node->next != NULL || node->prev != node)) {
        result = -1;
    }

    return result;
}


void remove_iffeature(struct lys_iffeature **iffeature, uint8_t *iffeature_size, struct ly_ctx *ctx) {

    lys_iffeature_free(ctx, *iffeature, *iffeature_size, 0, NULL);
    *iffeature_size = 0;
    *iffeature = NULL;
}

void remove_iffeature_type(struct lys_type *type, struct ly_ctx *ctx) {
    unsigned int i;

    if (type->base == LY_TYPE_ENUM) {
        for (i = 0; i < type->info.enums.count; ++i) {
            remove_iffeature(&type->info.enums.enm[i].iffeature, &type->info.enums.enm[i].iffeature_size, ctx);
        }
    } else if (type->base == LY_TYPE_BITS) {
        for (i = 0; i < type->info.bits.count; ++i) {
            remove_iffeature(&type->info.bits.bit[i].iffeature, &type->info.bits.bit[i].iffeature_size, ctx);
        }
    }
}

/* fix schema - ignore config flag, iffeature */
void fix_schema(struct lys_node *root, struct ly_ctx *ctx) {
    struct lys_node *node, *next;
    struct lys_node_container *cont;
    struct lys_node_rpc_action *action;
    struct lys_node_grp *grp;
    struct lys_node_uses *uses;
    int i;

    LY_TREE_DFS_BEGIN(root, next, node) {
        /* ignore config flag */
        node->flags = node->flags & (~(LYS_CONFIG_MASK | LYS_CONFIG_SET));
        remove_iffeature(&node->iffeature, &node->iffeature_size, ctx);
        switch (node->nodetype) {
            case LYS_CONTAINER:
                cont = (struct lys_node_container *)node;
                for (i = 0; i < cont->tpdf_size; ++i) {
                    remove_iffeature_type(&cont->tpdf[i].type, ctx);
                }
                break;
            case LYS_LEAF:
                remove_iffeature_type(&((struct lys_node_leaf *)node)->type, ctx);
                break;
            case LYS_LEAFLIST:
                remove_iffeature_type(&((struct lys_node_leaflist *)node)->type, ctx);
                break;
            case LYS_ACTION:
            case LYS_NOTIF:
                action = (struct lys_node_rpc_action *)node;
                for (i = 0; i < action->tpdf_size; ++i) {
                    remove_iffeature_type(&action->tpdf[i].type, ctx);
                }
                break;
            case LYS_GROUPING:
                grp = (struct lys_node_grp *)node;
                for (i = 0; i < grp->tpdf_size; ++i) {
                    remove_iffeature_type(&grp->tpdf[i].type, ctx);
                }
                break;
            case LYS_USES:
                uses = (struct lys_node_uses *)node;
                for (i = 0; i < uses->augment_size; ++i) {
                    remove_iffeature(&uses->augment[i].iffeature, &uses->augment[i].iffeature_size, ctx);
                    fix_schema(uses->augment[i].child, ctx);
                }
                for (i = 0; i < uses->refine_size; ++i) {
                    remove_iffeature(&uses->refine[i].iffeature, &uses->refine[i].iffeature_size, ctx);
                }
                break;
            default:
                break;
        }
        LY_TREE_DFS_END(root, next, node)
    }
}

int yang_data_result(struct lys_ext_instance *ext) {
    struct lys_node **root;

    root = lys_ext_complex_get_substmt(LY_STMT_CONTAINER, (struct lys_ext_instance_complex *)ext, NULL);
    if (!root || !(*root) || (*root)->next != NULL || check_node(*root)) {
        return 1;
    }

    fix_schema(*root, ext->def->module->ctx);
    return 0;
}

struct lyext_substmt yang_data_substmt[] = {
    {LY_STMT_USES,        0, LY_STMT_CARD_OPT},
    {LY_STMT_CONTAINER,   0, LY_STMT_CARD_OPT},
    {LY_STMT_CHOICE,      0, LY_STMT_CARD_OPT},
    {0, 0, 0} /* terminating item */
};

/**
 * @brief Plugin for the RFC 8040 restconf extension
 */
struct lyext_plugin_complex yang_data = {
    .type = LYEXT_COMPLEX,
    .flags = 0,
    .check_position = &yang_data_position,
    .check_result = &yang_data_result,
    .check_inherit = NULL,
    .valid_data = NULL,
    /* specification of allowed substatements of the extension instance */
    .substmt = yang_data_substmt,

    /* final size of the extension instance structure with the space for storing the substatements */
    .instance_size = (sizeof(struct lys_ext_instance_complex) - 1) + 2 * sizeof(void*)
};

/**
 * @brief list of all extension plugins implemented here
 *
 * MANDATORY object for all libyang extension plugins, the name must match the <name>.so
 */
struct lyext_plugin_list yangdata[] = {
    {"ietf-restconf", "2017-01-26", "yang-data", (struct lyext_plugin*)&yang_data},
    {NULL, NULL, NULL, NULL} /* terminating item */
};
