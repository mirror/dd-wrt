/**
 * @file parser_yang.c
 * @author Pavol Vican
 * @brief YANG parser for libyang
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <ctype.h>
#include <assert.h>
#include "parser_yang.h"
#include "parser_yang_lex.h"
#include "parser.h"
#include "xpath.h"

static void yang_free_import(struct ly_ctx *ctx, struct lys_import *imp, uint8_t start, uint8_t size);
static int yang_check_must(struct lys_module *module, struct lys_restr *must, uint size, struct unres_schema *unres);
static void yang_free_include(struct ly_ctx *ctx, struct lys_include *inc, uint8_t start, uint8_t size);
static int yang_check_sub_module(struct lys_module *module, struct unres_schema *unres, struct lys_node *node);
static void free_yang_common(struct lys_module *module, struct lys_node *node);
static int yang_check_nodes(struct lys_module *module, struct lys_node *parent, struct lys_node *nodes,
                            int options, struct unres_schema *unres);
static int yang_fill_ext_substm_index(struct lys_ext_instance_complex *ext, LY_STMT stmt, enum yytokentype keyword);
static void yang_free_nodes(struct ly_ctx *ctx, struct lys_node *node);
void lys_iffeature_free(struct ly_ctx *ctx, struct lys_iffeature *iffeature, uint8_t iffeature_size, int shallow,
                        void (*private_destructor)(const struct lys_node *node, void *priv));

static int
yang_check_string(struct lys_module *module, const char **target, char *what,
                  char *where, char *value, struct lys_node *node)
{
    if (*target) {
        LOGVAL(module->ctx, LYE_TOOMANY, (node) ? LY_VLOG_LYS : LY_VLOG_NONE, node, what, where);
        free(value);
        return 1;
    } else {
        *target = lydict_insert_zc(module->ctx, value);
        return 0;
    }
}

int
yang_read_common(struct lys_module *module, char *value, enum yytokentype type)
{
    int ret = 0;

    switch (type) {
    case MODULE_KEYWORD:
        module->name = lydict_insert_zc(module->ctx, value);
        break;
    case NAMESPACE_KEYWORD:
        ret = yang_check_string(module, &module->ns, "namespace", "module", value, NULL);
        break;
    case ORGANIZATION_KEYWORD:
        ret = yang_check_string(module, &module->org, "organization", "module", value, NULL);
        break;
    case CONTACT_KEYWORD:
        ret = yang_check_string(module, &module->contact, "contact", "module", value, NULL);
        break;
    default:
        free(value);
        LOGINT(module->ctx);
        ret = EXIT_FAILURE;
        break;
    }

    return ret;
}

int
yang_check_version(struct lys_module *module, struct lys_submodule *submodule, char *value, int repeat)
{
    int ret = EXIT_SUCCESS;

    if (repeat) {
        LOGVAL(module->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "yang version", "module");
        ret = EXIT_FAILURE;
    } else {
        if (!strcmp(value, "1")) {
            if (submodule) {
                if (module->version > 1) {
                    LOGVAL(module->ctx, LYE_INVER, LY_VLOG_NONE, NULL);
                    ret = EXIT_FAILURE;
                 }
                submodule->version = 1;
            } else {
                module->version = 1;
            }
        } else if (!strcmp(value, "1.1")) {
            if (submodule) {
                if (module->version != 2) {
                    LOGVAL(module->ctx, LYE_INVER, LY_VLOG_NONE, NULL);
                    ret = EXIT_FAILURE;
                }
                submodule->version = 2;
            } else {
                module->version = 2;
            }
        } else {
            LOGVAL(module->ctx, LYE_INARG, LY_VLOG_NONE, NULL, value, "yang-version");
            ret = EXIT_FAILURE;
        }
    }
    free(value);
    return ret;
}

int
yang_read_prefix(struct lys_module *module, struct lys_import *imp, char *value)
{
    int ret = 0;

    if (!imp && lyp_check_identifier(module->ctx, value, LY_IDENT_PREFIX, module, NULL)) {
        free(value);
        return EXIT_FAILURE;
    }

    if (imp) {
        ret = yang_check_string(module, &imp->prefix, "prefix", "import", value, NULL);
    } else {
        ret = yang_check_string(module, &module->prefix, "prefix", "module", value, NULL);
    }

    return ret;
}

static int
yang_fill_import(struct lys_module *module, struct lys_import *imp_old, struct lys_import *imp_new,
                 char *value, struct unres_schema *unres)
{
    const char *exp;
    int rc;

    if (!imp_old->prefix) {
        LOGVAL(module->ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "prefix", "import");
        goto error;
    } else {
        if (lyp_check_identifier(module->ctx, imp_old->prefix, LY_IDENT_PREFIX, module, NULL)) {
            goto error;
        }
    }
    memcpy(imp_new, imp_old, sizeof *imp_old);
    exp = lydict_insert_zc(module->ctx, value);
    rc = lyp_check_import(module, exp, imp_new);
    lydict_remove(module->ctx, exp);
    module->imp_size++;
    if (rc || yang_check_ext_instance(module, &imp_new->ext, imp_new->ext_size, imp_new, unres)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

error:
    free(value);
    lydict_remove(module->ctx, imp_old->dsc);
    lydict_remove(module->ctx, imp_old->ref);
    lydict_remove(module->ctx, imp_old->prefix);
    lys_extension_instances_free(module->ctx, imp_old->ext, imp_old->ext_size, NULL);
    return EXIT_FAILURE;
}

int
yang_read_description(struct lys_module *module, void *node, char *value, char *where, enum yytokentype type)
{
    int ret;
    char *dsc = "description";

    switch (type) {
    case MODULE_KEYWORD:
        ret = yang_check_string(module, &module->dsc, dsc, "module", value, NULL);
        break;
    case REVISION_KEYWORD:
        ret = yang_check_string(module, &((struct lys_revision *)node)->dsc, dsc, where, value, NULL);
        break;
    case IMPORT_KEYWORD:
        ret = yang_check_string(module, &((struct lys_import *)node)->dsc, dsc, where, value, NULL);
        break;
    case INCLUDE_KEYWORD:
        ret = yang_check_string(module, &((struct lys_include *)node)->dsc, dsc, where, value, NULL);
        break;
    case NODE_PRINT:
        ret = yang_check_string(module, &((struct lys_node *)node)->dsc, dsc, where, value, node);
        break;
    default:
        ret = yang_check_string(module, &((struct lys_node *)node)->dsc, dsc, where, value, NULL);
        break;
    }
    return ret;
}

int
yang_read_reference(struct lys_module *module, void *node, char *value, char *where, enum yytokentype type)
{
    int ret;
    char *ref = "reference";

    switch (type) {
    case MODULE_KEYWORD:
        ret = yang_check_string(module, &module->ref, ref, "module", value, NULL);
        break;
    case REVISION_KEYWORD:
        ret = yang_check_string(module, &((struct lys_revision *)node)->ref, ref, where, value, NULL);
        break;
    case IMPORT_KEYWORD:
        ret = yang_check_string(module, &((struct lys_import *)node)->ref, ref, where, value, NULL);
        break;
    case INCLUDE_KEYWORD:
        ret = yang_check_string(module, &((struct lys_include *)node)->ref, ref, where, value, NULL);
        break;
    case NODE_PRINT:
        ret = yang_check_string(module, &((struct lys_node *)node)->ref, ref, where, value, node);
        break;
    default:
        ret = yang_check_string(module, &((struct lys_node *)node)->ref, ref, where, value, NULL);
        break;
    }
    return ret;
}

int
yang_fill_iffeature(struct lys_module *module, struct lys_iffeature *iffeature, void *parent,
                    char *value, struct unres_schema *unres, int parent_is_feature)
{
    const char *exp;
    int ret;

    if ((module->version != 2) && ((value[0] == '(') || strchr(value, ' '))) {
        LOGVAL(module->ctx, LYE_INARG, LY_VLOG_NONE, NULL, value, "if-feature");
        free(value);
        return EXIT_FAILURE;
    }

    if (!(exp = transform_iffeat_schema2json(module, value))) {
        free(value);
        return EXIT_FAILURE;
    }
    free(value);

    ret = resolve_iffeature_compile(iffeature, exp, (struct lys_node *)parent, parent_is_feature, unres);
    lydict_remove(module->ctx, exp);

    return (ret) ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
yang_read_base(struct lys_module *module, struct lys_ident *ident, char *value, struct unres_schema *unres)
{
    const char *exp;

    exp = transform_schema2json(module, value);
    free(value);
    if (!exp) {
        return EXIT_FAILURE;
    }

    if (unres_schema_add_str(module, unres, ident, UNRES_IDENT, exp) == -1) {
        lydict_remove(module->ctx, exp);
        return EXIT_FAILURE;
    }

    lydict_remove(module->ctx, exp);
    return EXIT_SUCCESS;
}

int
yang_read_message(struct lys_module *module,struct lys_restr *save,char *value, char *what, int message)
{
    int ret;

    if (message == ERROR_APP_TAG_KEYWORD) {
        ret = yang_check_string(module, &save->eapptag, "error_app_tag", what, value, NULL);
    } else {
        ret = yang_check_string(module, &save->emsg, "error_message", what, value, NULL);
    }
    return ret;
}

int
yang_read_presence(struct lys_module *module, struct lys_node_container *cont, char *value)
{
    if (cont->presence) {
        LOGVAL(module->ctx, LYE_TOOMANY, LY_VLOG_LYS, cont, "presence", "container");
        free(value);
        return EXIT_FAILURE;
    } else {
        cont->presence = lydict_insert_zc(module->ctx, value);
        return EXIT_SUCCESS;
    }
}

void *
yang_read_when(struct lys_module *module, struct lys_node *node, enum yytokentype type, char *value)
{
    struct lys_when *retval;

    retval = calloc(1, sizeof *retval);
    LY_CHECK_ERR_RETURN(!retval, LOGMEM(module->ctx); free(value), NULL);
    retval->cond = transform_schema2json(module, value);
    if (!retval->cond) {
        goto error;
    }
    switch (type) {
    case CONTAINER_KEYWORD:
        if (((struct lys_node_container *)node)->when) {
            LOGVAL(module->ctx, LYE_TOOMANY, LY_VLOG_LYS, node, "when", "container");
            goto error;
        }
        ((struct lys_node_container *)node)->when = retval;
        break;
    case ANYDATA_KEYWORD:
    case ANYXML_KEYWORD:
        if (((struct lys_node_anydata *)node)->when) {
            LOGVAL(module->ctx, LYE_TOOMANY, LY_VLOG_LYS, node, "when", (type == ANYXML_KEYWORD) ? "anyxml" : "anydata");
            goto error;
        }
        ((struct lys_node_anydata *)node)->when = retval;
        break;
    case CHOICE_KEYWORD:
        if (((struct lys_node_choice *)node)->when) {
            LOGVAL(module->ctx, LYE_TOOMANY, LY_VLOG_LYS, node, "when", "choice");
            goto error;
        }
        ((struct lys_node_choice *)node)->when = retval;
        break;
    case CASE_KEYWORD:
        if (((struct lys_node_case *)node)->when) {
            LOGVAL(module->ctx, LYE_TOOMANY, LY_VLOG_LYS, node, "when", "case");
            goto error;
        }
        ((struct lys_node_case *)node)->when = retval;
        break;
    case LEAF_KEYWORD:
        if (((struct lys_node_leaf *)node)->when) {
            LOGVAL(module->ctx, LYE_TOOMANY, LY_VLOG_LYS, node, "when", "leaf");
            goto error;
        }
        ((struct lys_node_leaf *)node)->when = retval;
        break;
    case LEAF_LIST_KEYWORD:
        if (((struct lys_node_leaflist *)node)->when) {
            LOGVAL(module->ctx, LYE_TOOMANY, LY_VLOG_LYS, node, "when", "leaflist");
            goto error;
        }
        ((struct lys_node_leaflist *)node)->when = retval;
        break;
    case LIST_KEYWORD:
        if (((struct lys_node_list *)node)->when) {
            LOGVAL(module->ctx, LYE_TOOMANY, LY_VLOG_LYS, node, "when", "list");
            goto error;
        }
        ((struct lys_node_list *)node)->when = retval;
        break;
    case USES_KEYWORD:
        if (((struct lys_node_uses *)node)->when) {
            LOGVAL(module->ctx, LYE_TOOMANY, LY_VLOG_LYS, node, "when", "uses");
            goto error;
        }
        ((struct lys_node_uses *)node)->when = retval;
        break;
    case AUGMENT_KEYWORD:
        if (((struct lys_node_augment *)node)->when) {
            LOGVAL(module->ctx, LYE_TOOMANY, LY_VLOG_LYS, node, "when", "augment");
            goto error;
        }
        ((struct lys_node_augment *)node)->when = retval;
        break;
    case EXTENSION_INSTANCE:
        *(struct lys_when **)node = retval;
        break;
    default:
        goto error;
        break;
    }
    free(value);
    return retval;

error:
    free(value);
    lydict_remove(module->ctx, retval->cond);
    free(retval);
    return NULL;
}

void *
yang_read_node(struct lys_module *module, struct lys_node *parent, struct lys_node **root,
               char *value, int nodetype, int sizeof_struct)
{
    struct lys_node *node, **child;

    node = calloc(1, sizeof_struct);
    LY_CHECK_ERR_RETURN(!node, LOGMEM(module->ctx); free(value), NULL);

    LOGDBG(LY_LDGYANG, "parsing %s statement \"%s\"", strnodetype(nodetype), value);
    node->name = lydict_insert_zc(module->ctx, value);
    node->module = module;
    node->nodetype = nodetype;
    node->parent = parent;

    /* insert the node into the schema tree */
    child = (parent) ? &parent->child : root;
    if (*child) {
        (*child)->prev->next = node;
        (*child)->prev = node;
    } else {
        *child = node;
        node->prev = node;
    }
    return node;
}

int
yang_read_default(struct lys_module *module, void *node, char *value, enum yytokentype type)
{
    int ret;

    switch (type) {
    case LEAF_KEYWORD:
        ret = yang_check_string(module, &((struct lys_node_leaf *) node)->dflt, "default", "leaf", value, node);
        break;
    case TYPEDEF_KEYWORD:
        ret = yang_check_string(module, &((struct lys_tpdf *) node)->dflt, "default", "typedef", value, NULL);
        break;
    default:
        free(value);
        LOGINT(module->ctx);
        ret = EXIT_FAILURE;
        break;
    }
    return ret;
}

int
yang_read_units(struct lys_module *module, void *node, char *value, enum yytokentype type)
{
    int ret;

    switch (type) {
    case LEAF_KEYWORD:
        ret = yang_check_string(module, &((struct lys_node_leaf *) node)->units, "units", "leaf", value, node);
        break;
    case LEAF_LIST_KEYWORD:
        ret = yang_check_string(module, &((struct lys_node_leaflist *) node)->units, "units", "leaflist", value, node);
        break;
    case TYPEDEF_KEYWORD:
        ret = yang_check_string(module, &((struct lys_tpdf *) node)->units, "units", "typedef", value, NULL);
        break;
    case ADD_KEYWORD:
    case REPLACE_KEYWORD:
    case DELETE_KEYWORD:
        ret = yang_check_string(module, &((struct lys_deviate *) node)->units, "units", "deviate", value, NULL);
        break;
    default:
        free(value);
        LOGINT(module->ctx);
        ret = EXIT_FAILURE;
        break;
    }
    return ret;
}

int
yang_read_key(struct lys_module *module, struct lys_node_list *list, struct unres_schema *unres)
{
    char *exp, *value;

    exp = value = (char *) list->keys;
    while ((value = strpbrk(value, " \t\n"))) {
        list->keys_size++;
        while (isspace(*value)) {
            value++;
        }
    }
    list->keys_size++;

    list->keys_str = lydict_insert_zc(module->ctx, exp);
    list->keys = calloc(list->keys_size, sizeof *list->keys);
    LY_CHECK_ERR_RETURN(!list->keys, LOGMEM(module->ctx), EXIT_FAILURE);

    if (unres_schema_add_node(module, unres, list, UNRES_LIST_KEYS, NULL) == -1) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int
yang_fill_unique(struct lys_module *module, struct lys_node_list *list, struct lys_unique *unique, char *value, struct unres_schema *unres)
{
    int i, j;
    char *vaux, c;
    struct unres_list_uniq *unique_info;

    /* count the number of unique leafs in the value */
    vaux = value;
    while ((vaux = strpbrk(vaux, " \t\n"))) {
       unique->expr_size++;
        while (isspace(*vaux)) {
            vaux++;
        }
    }
    unique->expr_size++;
    unique->expr = calloc(unique->expr_size, sizeof *unique->expr);
    LY_CHECK_ERR_GOTO(!unique->expr, LOGMEM(module->ctx), error);

    for (i = 0; i < unique->expr_size; i++) {
        vaux = strpbrk(value, " \t\n");
        if (vaux) {
            c = *vaux;
            *vaux = '\0';
        }

        /* store token into unique structure (includes converting prefix to the module name) */
        unique->expr[i] = transform_schema2json(module, value);
        if (vaux) {
            *vaux = c;
        }

        /* check that the expression does not repeat */
        for (j = 0; j < i; j++) {
            if (ly_strequal(unique->expr[j], unique->expr[i], 1)) {
                LOGVAL(module->ctx, LYE_INARG, LY_VLOG_LYS, list, unique->expr[i], "unique");
                LOGVAL(module->ctx, LYE_SPEC, LY_VLOG_LYS, list, "The identifier is not unique");
                goto error;
            }
        }
        /* try to resolve leaf */
        if (unres) {
            unique_info = malloc(sizeof *unique_info);
            LY_CHECK_ERR_GOTO(!unique_info, LOGMEM(module->ctx), error);
            unique_info->list = (struct lys_node *)list;
            unique_info->expr = unique->expr[i];
            unique_info->trg_type = &unique->trg_type;
            if (unres_schema_add_node(module, unres, unique_info, UNRES_LIST_UNIQ, NULL) == -1) {
                goto error;
            }
        } else {
            if (resolve_unique((struct lys_node *)list, unique->expr[i], &unique->trg_type)) {
                goto error;
            }
        }

        /* move to next token */
        value = vaux;
        while(value && isspace(*value)) {
            value++;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_read_unique(struct lys_module *module, struct lys_node_list *list, struct unres_schema *unres)
{
    uint8_t k;
    char *str;

    for (k = 0; k < list->unique_size; k++) {
        str = (char *)list->unique[k].expr;
        if (yang_fill_unique(module, list, &list->unique[k], str, unres)) {
            goto error;
        }
        free(str);
    }
    return EXIT_SUCCESS;

error:
    free(str);
    return EXIT_FAILURE;
}

int
yang_read_leafref_path(struct lys_module *module, struct yang_type *stype, char *value)
{
    if (stype->base && (stype->base != LY_TYPE_LEAFREF)) {
        LOGVAL(module->ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "require-instance");
        goto error;
    }
    if (stype->type->info.lref.path) {
        LOGVAL(module->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "path", "type");
        goto error;
    }
    stype->type->info.lref.path = lydict_insert_zc(module->ctx, value);
    stype->base = LY_TYPE_LEAFREF;
    return EXIT_SUCCESS;

error:
    free(value);
    return EXIT_FAILURE;
}

int
yang_read_require_instance(struct ly_ctx *ctx, struct yang_type *stype, int req)
{
    if (stype->base && (stype->base != LY_TYPE_LEAFREF)) {
        LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "require-instance");
        return EXIT_FAILURE;
    }
    if (stype->type->info.lref.req) {
        LOGVAL(ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "require-instance", "type");
        return EXIT_FAILURE;
    }
    stype->type->info.lref.req = req;
    stype->base = LY_TYPE_LEAFREF;
    return EXIT_SUCCESS;
}

int
yang_check_type(struct lys_module *module, struct lys_node *parent, struct yang_type *typ, struct lys_type *type, int tpdftype, struct unres_schema *unres)
{
    struct ly_ctx *ctx = module->ctx;
    int rc, ret = -1;
    unsigned int i, j;
    int8_t req;
    const char *name, *value, *module_name = NULL;
    LY_DATA_TYPE base = 0, base_tmp;
    struct lys_node *siter;
    struct lys_type *dertype;
    struct lys_type_enum *enms_sc = NULL;
    struct lys_type_bit *bits_sc = NULL;
    struct lys_type_bit bit_tmp;
    struct yang_type *yang;

    value = transform_schema2json(module, typ->name);
    if (!value) {
        goto error;
    }

    i = parse_identifier(value);
    if (i < 1) {
        LOGVAL(ctx, LYE_INCHAR, LY_VLOG_NONE, NULL, value[-i], &value[-i]);
        lydict_remove(ctx, value);
        goto error;
    }
    /* module name */
    name = value;
    if (value[i]) {
        module_name = lydict_insert(ctx, value, i);
        name += i;
        if ((name[0] != ':') || (parse_identifier(name + 1) < 1)) {
            LOGVAL(ctx, LYE_INCHAR, LY_VLOG_NONE, NULL, name[0], name);
            lydict_remove(ctx, module_name);
            lydict_remove(ctx, value);
            goto error;
        }
        ++name;
    }

    rc = resolve_superior_type(name, module_name, module, parent, &type->der);
    if (rc == -1) {
        LOGVAL(ctx, LYE_INMOD, LY_VLOG_NONE, NULL, module_name);
        lydict_remove(ctx, module_name);
        lydict_remove(ctx, value);
        goto error;

    /* the type could not be resolved or it was resolved to an unresolved typedef or leafref */
    } else if (rc == EXIT_FAILURE) {
        LOGVAL(ctx, LYE_NORESOLV, LY_VLOG_NONE, NULL, "type", name);
        lydict_remove(ctx, module_name);
        lydict_remove(ctx, value);
        ret = EXIT_FAILURE;
        goto error;
    }
    lydict_remove(ctx, module_name);
    lydict_remove(ctx, value);

    if (type->value_flags & LY_VALUE_UNRESGRP) {
        /* resolved type in grouping, decrease the grouping's nacm number to indicate that one less
         * unresolved item left inside the grouping, LYTYPE_GRP used as a flag for types inside a grouping.  */
        for (siter = parent; siter && (siter->nodetype != LYS_GROUPING); siter = lys_parent(siter));
        if (siter) {
            assert(((struct lys_node_grp *)siter)->unres_count);
            ((struct lys_node_grp *)siter)->unres_count--;
        } else {
            LOGINT(ctx);
            goto error;
        }
        type->value_flags &= ~LY_VALUE_UNRESGRP;
    }

    /* check status */
    if (lyp_check_status(type->parent->flags, type->parent->module, type->parent->name,
                         type->der->flags, type->der->module, type->der->name, parent)) {
        goto error;
    }

    base = typ->base;
    base_tmp = type->base;
    type->base = type->der->type.base;
    if (base == 0) {
        base = type->der->type.base;
    }
    switch (base) {
    case LY_TYPE_STRING:
        if (type->base == LY_TYPE_BINARY) {
            if (type->info.str.pat_count) {
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Binary type could not include pattern statement.");
                goto error;
            }
            type->info.binary.length = type->info.str.length;
            if (type->info.binary.length && lyp_check_length_range(ctx, type->info.binary.length->expr, type)) {
                LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, type->info.binary.length->expr, "length");
                goto error;
            }
        } else if (type->base == LY_TYPE_STRING) {
            if (type->info.str.length && lyp_check_length_range(ctx, type->info.str.length->expr, type)) {
                LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, type->info.str.length->expr, "length");
                goto error;
            }
        } else {
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", type->parent->name);
            goto error;
        }
        break;
    case LY_TYPE_DEC64:
        if (type->base == LY_TYPE_DEC64) {
            /* mandatory sub-statement(s) check */
            if (!type->info.dec64.dig && !type->der->type.der) {
                /* decimal64 type directly derived from built-in type requires fraction-digits */
                LOGVAL(ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "fraction-digits", "type");
                goto error;
            }
            if (type->info.dec64.dig && type->der->type.der) {
                /* type is not directly derived from buit-in type and fraction-digits statement is prohibited */
                LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "fraction-digits");
                goto error;
            }

            /* copy fraction-digits specification from parent type for easier internal use */
            if (type->der->type.der) {
                type->info.dec64.dig = type->der->type.info.dec64.dig;
                type->info.dec64.div = type->der->type.info.dec64.div;
            }
            if (type->info.dec64.range && lyp_check_length_range(ctx, type->info.dec64.range->expr, type)) {
                LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, type->info.dec64.range->expr, "range");
                goto error;
            }
        } else if (type->base >= LY_TYPE_INT8 && type->base <=LY_TYPE_UINT64) {
            if (type->info.dec64.dig) {
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Numerical type could not include fraction statement.");
                goto error;
            }
            type->info.num.range = type->info.dec64.range;
            if (type->info.num.range && lyp_check_length_range(ctx, type->info.num.range->expr, type)) {
                LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, type->info.num.range->expr, "range");
                goto error;
            }
        } else {
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", type->parent->name);
            goto error;
        }
        break;
    case LY_TYPE_ENUM:
        if (type->base != LY_TYPE_ENUM) {
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", type->parent->name);
            goto error;
        }
        dertype = &type->der->type;

        if (!dertype->der) {
            if (!type->info.enums.count) {
                /* type is derived directly from buit-in enumeartion type and enum statement is required */
                LOGVAL(ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "enum", "type");
                goto error;
            }
        } else {
            for (; !dertype->info.enums.count; dertype = &dertype->der->type);
            if (module->version < 2 && type->info.enums.count) {
                /* type is not directly derived from built-in enumeration type and enum statement is prohibited
                 * in YANG 1.0, since YANG 1.1 enum statements can be used to restrict the base enumeration type */
                LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "enum");
                goto error;
            }

            /* restricted enumeration type - the name MUST be used in the base type */
            enms_sc = dertype->info.enums.enm;
            for (i = 0; i < type->info.enums.count; i++) {
                for (j = 0; j < dertype->info.enums.count; j++) {
                    if (ly_strequal(enms_sc[j].name, type->info.enums.enm[i].name, 1)) {
                        break;
                    }
                }
                if (j == dertype->info.enums.count) {
                    LOGVAL(ctx, LYE_ENUM_INNAME, LY_VLOG_NONE, NULL, type->info.enums.enm[i].name);
                    goto error;
                }

                if (type->info.enums.enm[i].flags & LYS_AUTOASSIGNED) {
                    /* automatically assign value from base type */
                    type->info.enums.enm[i].value = enms_sc[j].value;
                } else {
                    /* check that the assigned value corresponds to the original
                     * value of the enum in the base type */
                    if (type->info.enums.enm[i].value != enms_sc[j].value) {
                        /* type->info.enums.enm[i].value - assigned value in restricted enum
                         * enms_sc[j].value - value assigned to the corresponding enum (detected above) in base type */
                        LOGVAL(ctx, LYE_ENUM_INVAL, LY_VLOG_NONE, NULL, type->info.enums.enm[i].value,
                               type->info.enums.enm[i].name, enms_sc[j].value);
                        goto error;
                    }
                }
            }
        }
        break;
    case LY_TYPE_BITS:
        if (type->base != LY_TYPE_BITS) {
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", type->parent->name);
            goto error;
        }
        dertype = &type->der->type;

        if (!dertype->der) {
            if (!type->info.bits.count) {
                /* type is derived directly from buit-in bits type and bit statement is required */
                LOGVAL(ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "bit", "type");
                goto error;
            }
        } else {
            for (; !dertype->info.enums.count; dertype = &dertype->der->type);
            if (module->version < 2 && type->info.bits.count) {
                /* type is not directly derived from buit-in bits type and bit statement is prohibited,
                 * since YANG 1.1 the bit statements can be used to restrict the base bits type */
                LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "bit");
                goto error;
            }

            bits_sc = dertype->info.bits.bit;
            for (i = 0; i < type->info.bits.count; i++) {
                for (j = 0; j < dertype->info.bits.count; j++) {
                    if (ly_strequal(bits_sc[j].name, type->info.bits.bit[i].name, 1)) {
                        break;
                    }
                }
                if (j == dertype->info.bits.count) {
                    LOGVAL(ctx, LYE_BITS_INNAME, LY_VLOG_NONE, NULL, type->info.bits.bit[i].name);
                    goto error;
                }

                /* restricted bits type */
                if (type->info.bits.bit[i].flags & LYS_AUTOASSIGNED) {
                    /* automatically assign position from base type */
                    type->info.bits.bit[i].pos = bits_sc[j].pos;
                } else {
                    /* check that the assigned position corresponds to the original
                     * position of the bit in the base type */
                    if (type->info.bits.bit[i].pos != bits_sc[j].pos) {
                        /* type->info.bits.bit[i].pos - assigned position in restricted bits
                         * bits_sc[j].pos - position assigned to the corresponding bit (detected above) in base type */
                        LOGVAL(ctx, LYE_BITS_INVAL, LY_VLOG_NONE, NULL, type->info.bits.bit[i].pos,
                               type->info.bits.bit[i].name, bits_sc[j].pos);
                        goto error;
                    }
                }
            }
        }

        for (i = type->info.bits.count; i > 0; i--) {
            j = i - 1;

            /* keep them ordered by position */
            while (j && type->info.bits.bit[j - 1].pos > type->info.bits.bit[j].pos) {
                /* switch them */
                memcpy(&bit_tmp, &type->info.bits.bit[j], sizeof bit_tmp);
                memcpy(&type->info.bits.bit[j], &type->info.bits.bit[j - 1], sizeof bit_tmp);
                memcpy(&type->info.bits.bit[j - 1], &bit_tmp, sizeof bit_tmp);
                j--;
            }
        }
        break;
    case LY_TYPE_LEAFREF:
        if (type->base == LY_TYPE_INST) {
            if (type->info.lref.path) {
                LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "path");
                goto error;
            }
            if ((req = type->info.lref.req)) {
                type->info.inst.req = req;
            }
        } else if (type->base == LY_TYPE_LEAFREF) {
            /* require-instance only YANG 1.1 */
            if (type->info.lref.req && (module->version < 2)) {
                LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "require-instance");
                goto error;
            }
            /* flag resolving for later use */
            if (!tpdftype) {
                for (siter = parent; siter && siter->nodetype != LYS_GROUPING; siter = lys_parent(siter));
                if (siter) {
                    /* just a flag - do not resolve */
                    tpdftype = 1;
                }
            }

            if (type->info.lref.path) {
                if (type->der->type.der) {
                    LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "path");
                    goto error;
                }
                value = type->info.lref.path;
                /* store in the JSON format */
                type->info.lref.path = transform_schema2json(module, value);
                lydict_remove(ctx, value);
                if (!type->info.lref.path) {
                    goto error;
                }
                /* try to resolve leafref path only when this is instantiated
                 * leaf, so it is not:
                 * - typedef's type,
                 * - in  grouping definition,
                 * - just instantiated in a grouping definition,
                 * because in those cases the nodes referenced in path might not be present
                 * and it is not a bug.  */
                if (!tpdftype && unres_schema_add_node(module, unres, type, UNRES_TYPE_LEAFREF, parent) == -1) {
                    goto error;
                }
            } else if (!type->der->type.der) {
                LOGVAL(ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "path", "type");
                goto error;
            } else {
                /* copy leafref definition into the derived type */
                type->info.lref.path = lydict_insert(ctx, type->der->type.info.lref.path, 0);
                /* and resolve the path at the place we are (if not in grouping/typedef) */
                if (!tpdftype && unres_schema_add_node(module, unres, type, UNRES_TYPE_LEAFREF, parent) == -1) {
                    goto error;
                }
            }
        } else {
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", type->parent->name);
            goto error;
        }
        break;
    case LY_TYPE_IDENT:
        if (type->base != LY_TYPE_IDENT) {
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", type->parent->name);
            goto error;
        }
        if (type->der->type.der) {
            if (type->info.ident.ref) {
                LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "base");
                goto error;
            }
        } else {
            if (!type->info.ident.ref) {
                LOGVAL(ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "base", "type");
                goto error;
            }
        }
        break;
    case LY_TYPE_UNION:
        if (type->base != LY_TYPE_UNION) {
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", type->parent->name);
            goto error;
        }
        if (!type->info.uni.types) {
            if (type->der->type.der) {
                /* this is just a derived type with no additional type specified/required */
                assert(type->der->type.base == LY_TYPE_UNION);
                type->info.uni.has_ptr_type = type->der->type.info.uni.has_ptr_type;
                break;
            }
            LOGVAL(ctx, LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, "type", "(union) type");
            goto error;
        }
        for (i = 0; i < type->info.uni.count; i++) {
            dertype = &type->info.uni.types[i];
            if (dertype->base == LY_TYPE_DER) {
                yang = (struct yang_type *)dertype->der;
                dertype->der = NULL;
                dertype->parent = type->parent;
                if (yang_check_type(module, parent, yang, dertype, tpdftype, unres)) {
                    dertype->der = (struct lys_tpdf *)yang;
                    ret = EXIT_FAILURE;
                    type->base = base_tmp;
                    base = 0;
                    goto error;
                } else {
                    lydict_remove(ctx, yang->name);
                    free(yang);
                }
            }
            if (module->version < 2) {
                if (dertype->base == LY_TYPE_EMPTY) {
                    LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, "empty", typ->name);
                    goto error;
                } else if (dertype->base == LY_TYPE_LEAFREF) {
                    LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, "leafref", typ->name);
                    goto error;
                }
            }
            if ((dertype->base == LY_TYPE_INST) || (dertype->base == LY_TYPE_LEAFREF)
                    || ((dertype->base == LY_TYPE_UNION) && dertype->info.uni.has_ptr_type)) {
                type->info.uni.has_ptr_type = 1;
            }
        }
        break;

    default:
        if (base >= LY_TYPE_BINARY && base <= LY_TYPE_UINT64) {
            if (type->base != base) {
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid restriction in type \"%s\".", type->parent->name);
                goto error;
            }
        } else {
            LOGINT(ctx);
            goto error;
        }
    }

    /* if derived type has extension, which need validate data */
    dertype = &type->der->type;
    while (dertype->der) {
        if (dertype->parent->flags & LYS_VALID_EXT) {
            type->parent->flags |= LYS_VALID_EXT;
        }
        dertype = &dertype->der->type;
    }

    return EXIT_SUCCESS;

error:
    if (base) {
        type->base = base_tmp;
    }
    return ret;
}

void
yang_free_type_union(struct ly_ctx *ctx, struct lys_type *type)
{
    struct lys_type *stype;
    struct yang_type *yang;
    unsigned int i;

    for (i = 0; i < type->info.uni.count; ++i) {
        stype = &type->info.uni.types[i];
        if (stype->base == LY_TYPE_DER) {
            yang = (struct yang_type *)stype->der;
            stype->base = yang->base;
            lydict_remove(ctx, yang->name);
            free(yang);
        } else if (stype->base == LY_TYPE_UNION) {
            yang_free_type_union(ctx, stype);
        }
    }
}

void *
yang_read_type(struct ly_ctx *ctx, void *parent, char *value, enum yytokentype type)
{
    struct yang_type *typ;
    struct lys_deviate *dev;

    typ = calloc(1, sizeof *typ);
    LY_CHECK_ERR_RETURN(!typ, LOGMEM(ctx), NULL);

    typ->flags = LY_YANG_STRUCTURE_FLAG;
    switch (type) {
    case LEAF_KEYWORD:
        if (((struct lys_node_leaf *)parent)->type.der) {
            LOGVAL(ctx, LYE_TOOMANY, LY_VLOG_LYS, parent, "type", "leaf");
            goto error;
        }
        ((struct lys_node_leaf *)parent)->type.der = (struct lys_tpdf *)typ;
        ((struct lys_node_leaf *)parent)->type.parent = (struct lys_tpdf *)parent;
        typ->type = &((struct lys_node_leaf *)parent)->type;
        break;
    case LEAF_LIST_KEYWORD:
        if (((struct lys_node_leaflist *)parent)->type.der) {
            LOGVAL(ctx, LYE_TOOMANY, LY_VLOG_LYS, parent, "type", "leaf-list");
            goto error;
        }
        ((struct lys_node_leaflist *)parent)->type.der = (struct lys_tpdf *)typ;
        ((struct lys_node_leaflist *)parent)->type.parent = (struct lys_tpdf *)parent;
        typ->type = &((struct lys_node_leaflist *)parent)->type;
        break;
    case UNION_KEYWORD:
        ((struct lys_type *)parent)->der = (struct lys_tpdf *)typ;
        typ->type = (struct lys_type *)parent;
        break;
    case TYPEDEF_KEYWORD:
        if (((struct lys_tpdf *)parent)->type.der) {
            LOGVAL(ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "type", "typedef");
            goto error;
        }
        ((struct lys_tpdf *)parent)->type.der = (struct lys_tpdf *)typ;
        typ->type = &((struct lys_tpdf *)parent)->type;
        break;
    case REPLACE_KEYWORD:
        /* deviation replace type*/
        dev = (struct lys_deviate *)parent;
        if (dev->type) {
            LOGVAL(ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "type", "deviation");
            goto error;
        }
        dev->type = calloc(1, sizeof *dev->type);
        LY_CHECK_ERR_GOTO(!dev->type, LOGMEM(ctx), error);
        dev->type->der = (struct lys_tpdf *)typ;
        typ->type = dev->type;
        break;
    case EXTENSION_INSTANCE:
        ((struct lys_type *)parent)->der = (struct lys_tpdf *)typ;
        typ->type = parent;
        break;
    default:
        goto error;
        break;
    }
    typ->name = lydict_insert_zc(ctx, value);
    return typ;

error:
    free(value);
    free(typ);
    return NULL;
}

void *
yang_read_length(struct ly_ctx *ctx, struct yang_type *stype, char *value, int is_ext_instance)
{
    struct lys_restr *length;

    if (is_ext_instance) {
        length = (struct lys_restr *)stype;
    } else {
        if (stype->base == 0 || stype->base == LY_TYPE_STRING) {
            stype->base = LY_TYPE_STRING;
        } else {
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Unexpected length statement.");
            goto error;
        }

        if (stype->type->info.str.length) {
            LOGVAL(ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "length", "type");
            goto error;
        }
        length = calloc(1, sizeof *length);
        LY_CHECK_ERR_GOTO(!length, LOGMEM(ctx), error);
        stype->type->info.str.length = length;
    }
    length->expr = lydict_insert_zc(ctx, value);
    return length;

error:
    free(value);
    return NULL;
}

int
yang_read_pattern(struct ly_ctx *ctx, struct lys_restr *pattern, void **precomp, char *value, char modifier)
{
    char *buf;
    size_t len;

    if (precomp && lyp_precompile_pattern(ctx, value, (pcre**)&precomp[0], (pcre_extra**)&precomp[1])) {
        free(value);
        return EXIT_FAILURE;
    }

    len = strlen(value);
    buf = malloc((len + 2) * sizeof *buf); /* modifier byte + value + terminating NULL byte */
    LY_CHECK_ERR_RETURN(!buf, LOGMEM(ctx); free(value), EXIT_FAILURE);

    buf[0] = modifier;
    strcpy(&buf[1], value);
    free(value);

    pattern->expr = lydict_insert_zc(ctx, buf);
    return EXIT_SUCCESS;
}

void *
yang_read_range(struct ly_ctx *ctx, struct yang_type *stype, char *value, int is_ext_instance)
{
    struct lys_restr * range;

    if (is_ext_instance) {
        range = (struct lys_restr *)stype;
    } else {
        if (stype->base != 0 && stype->base != LY_TYPE_DEC64) {
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Unexpected range statement.");
            goto error;
        }
        stype->base = LY_TYPE_DEC64;
        if (stype->type->info.dec64.range) {
            LOGVAL(ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "range", "type");
            goto error;
        }
        range = calloc(1, sizeof *range);
        LY_CHECK_ERR_GOTO(!range, LOGMEM(ctx), error);
        stype->type->info.dec64.range = range;
    }
    range->expr = lydict_insert_zc(ctx, value);
    return range;

error:
    free(value);
    return NULL;
}

int
yang_read_fraction(struct ly_ctx *ctx, struct yang_type *typ, uint32_t value)
{
    uint32_t i;

    if (typ->base == 0 || typ->base == LY_TYPE_DEC64) {
        typ->base = LY_TYPE_DEC64;
    } else {
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Unexpected fraction-digits statement.");
        goto error;
    }
    if (typ->type->info.dec64.dig) {
        LOGVAL(ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "fraction-digits", "type");
        goto error;
    }
    /* range check */
    if (value < 1 || value > 18) {
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"%s\".", value, "fraction-digits");
        goto error;
    }
    typ->type->info.dec64.dig = value;
    typ->type->info.dec64.div = 10;
    for (i = 1; i < value; i++) {
        typ->type->info.dec64.div *= 10;
    }
    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_read_enum(struct ly_ctx *ctx, struct yang_type *typ, struct lys_type_enum *enm, char *value)
{
    int i, j;

    typ->base = LY_TYPE_ENUM;
    if (!value[0]) {
        LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, value, "enum name");
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Enum name must not be empty.");
        free(value);
        goto error;
    }

    enm->name = lydict_insert_zc(ctx, value);

    /* the assigned name MUST NOT have any leading or trailing whitespace characters */
    if (isspace(enm->name[0]) || isspace(enm->name[strlen(enm->name) - 1])) {
        LOGVAL(ctx, LYE_ENUM_WS, LY_VLOG_NONE, NULL, enm->name);
        goto error;
    }

    j = typ->type->info.enums.count - 1;
    /* check the name uniqueness */
    for (i = 0; i < j; i++) {
        if (ly_strequal(typ->type->info.enums.enm[i].name, enm->name, 1)) {
            LOGVAL(ctx, LYE_ENUM_DUPNAME, LY_VLOG_NONE, NULL, enm->name);
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_check_enum(struct ly_ctx *ctx, struct yang_type *typ, struct lys_type_enum *enm, int64_t *value, int assign)
{
    int i, j;

    if (!assign) {
        /* assign value automatically */
        if (*value > INT32_MAX) {
            LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, "2147483648", "enum/value");
            goto error;
        }
        enm->value = *value;
        enm->flags |= LYS_AUTOASSIGNED;
        (*value)++;
    } else if (typ->type->info.enums.enm == enm) {
        /* change value, which is assigned automatically, if first enum has value. */
        *value = typ->type->info.enums.enm[0].value;
        (*value)++;
    }

    /* check that the value is unique */
    j = typ->type->info.enums.count-1;
    for (i = 0; i < j; i++) {
        if (typ->type->info.enums.enm[i].value == typ->type->info.enums.enm[j].value) {
            LOGVAL(ctx, LYE_ENUM_DUPVAL, LY_VLOG_NONE, NULL,
                   typ->type->info.enums.enm[j].value, typ->type->info.enums.enm[j].name,
                   typ->type->info.enums.enm[i].name);
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_read_bit(struct ly_ctx *ctx, struct yang_type *typ, struct lys_type_bit *bit, char *value)
{
    int i, j;

    typ->base = LY_TYPE_BITS;
    bit->name = lydict_insert_zc(ctx, value);
    if (lyp_check_identifier(ctx, bit->name, LY_IDENT_SIMPLE, NULL, NULL)) {
        goto error;
    }

    j = typ->type->info.bits.count - 1;
    /* check the name uniqueness */
    for (i = 0; i < j; i++) {
        if (ly_strequal(typ->type->info.bits.bit[i].name, bit->name, 1)) {
            LOGVAL(ctx, LYE_BITS_DUPNAME, LY_VLOG_NONE, NULL, bit->name);
            goto error;
        }
    }
    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_check_bit(struct ly_ctx *ctx, struct yang_type *typ, struct lys_type_bit *bit, int64_t *value, int assign)
{
    int i,j;

    if (!assign) {
        /* assign value automatically */
        if (*value > UINT32_MAX) {
            LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, "4294967295", "bit/position");
            goto error;
        }
        bit->pos = (uint32_t)*value;
        bit->flags |= LYS_AUTOASSIGNED;
        (*value)++;
    }

    j = typ->type->info.bits.count - 1;
    /* check that the value is unique */
    for (i = 0; i < j; i++) {
        if (typ->type->info.bits.bit[i].pos == bit->pos) {
            LOGVAL(ctx, LYE_BITS_DUPVAL, LY_VLOG_NONE, NULL, bit->pos, bit->name, typ->type->info.bits.bit[i].name);
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_read_augment(struct lys_module *module, struct lys_node *parent, struct lys_node_augment *aug, char *value)
{
    aug->nodetype = LYS_AUGMENT;
    aug->target_name = transform_schema2json(module, value);
    free(value);
    if (!aug->target_name) {
        return EXIT_FAILURE;
    }
    aug->parent = parent;
    aug->module = module;
    return EXIT_SUCCESS;
}

void *
yang_read_deviate_unsupported(struct ly_ctx *ctx, struct lys_deviation *dev)
{
    if (dev->deviate_size) {
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "\"not-supported\" deviation cannot be combined with any other deviation.");
        return NULL;
    }
    dev->deviate = calloc(1, sizeof *dev->deviate);
    LY_CHECK_ERR_RETURN(!dev->deviate, LOGMEM(ctx), NULL);
    dev->deviate[dev->deviate_size].mod = LY_DEVIATE_NO;
    dev->deviate_size = 1;
    return dev->deviate;
}

void *
yang_read_deviate(struct ly_ctx *ctx, struct lys_deviation *dev, LYS_DEVIATE_TYPE mod)
{
    struct lys_deviate *deviate;

    if (dev->deviate_size && dev->deviate[0].mod == LY_DEVIATE_NO) {
        LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "not-supported");
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "\"not-supported\" deviation cannot be combined with any other deviation.");
        return NULL;
    }
    if (!(dev->deviate_size % LY_YANG_ARRAY_SIZE)) {
        deviate = realloc(dev->deviate, (LY_YANG_ARRAY_SIZE + dev->deviate_size) * sizeof *deviate);
        LY_CHECK_ERR_RETURN(!deviate, LOGMEM(ctx), NULL);
        memset(deviate + dev->deviate_size, 0, LY_YANG_ARRAY_SIZE * sizeof *deviate);
        dev->deviate = deviate;
    }
    dev->deviate[dev->deviate_size].mod = mod;
    return &dev->deviate[dev->deviate_size++];
}

int
yang_read_deviate_units(struct ly_ctx *ctx, struct lys_deviate *deviate, struct lys_node *dev_target)
{
    const char **stritem;
    int j;

    /* check target node type */
    if (dev_target->nodetype == LYS_LEAFLIST) {
        stritem = &((struct lys_node_leaflist *)dev_target)->units;
    } else if (dev_target->nodetype == LYS_LEAF) {
        stritem = &((struct lys_node_leaf *)dev_target)->units;
    } else {
        LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "units");
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"units\" property.");
        goto error;
    }

    if (deviate->mod == LY_DEVIATE_DEL) {
        /* check values */
        if (!ly_strequal(*stritem, deviate->units, 1)) {
            LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, deviate->units, "units");
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Value differs from the target being deleted.");
            goto error;
        }
        /* remove current units value of the target */
        lydict_remove(ctx, *stritem);
        *stritem = NULL;
        /* remove its extensions */
        j = -1;
        while ((j = lys_ext_iter(dev_target->ext, dev_target->ext_size, j + 1, LYEXT_SUBSTMT_UNITS)) != -1) {
            lyp_ext_instance_rm(ctx, &dev_target->ext, &dev_target->ext_size, j);
            --j;
        }
    } else {
        if (deviate->mod == LY_DEVIATE_ADD) {
            /* check that there is no current value */
            if (*stritem) {
                LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "units");
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Adding property that already exists.");
                goto error;
            }
        } else { /* replace */
            if (!*stritem) {
                LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "units");
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Replacing a property that does not exist.");
                goto error;
            }
        }
        /* remove current units value of the target ... */
        lydict_remove(ctx, *stritem);

        /* ... and replace it with the value specified in deviation */
        *stritem = lydict_insert(ctx, deviate->units, 0);
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_read_deviate_unique(struct lys_deviate *deviate, struct lys_node *dev_target)
{
    struct ly_ctx *ctx = dev_target->module->ctx;
    struct lys_node_list *list;
    struct lys_unique *unique;

    /* check target node type */
    if (dev_target->nodetype != LYS_LIST) {
        LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "unique");
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"unique\" property.");
        goto error;
    }

    list = (struct lys_node_list *)dev_target;
    if (deviate->mod == LY_DEVIATE_ADD) {
        /* reallocate the unique array of the target */
        unique = ly_realloc(list->unique, (deviate->unique_size + list->unique_size) * sizeof *unique);
        LY_CHECK_ERR_GOTO(!unique, LOGMEM(ctx), error);
        list->unique = unique;
        memset(unique + list->unique_size, 0, deviate->unique_size * sizeof *unique);
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_fill_deviate_default(struct ly_ctx *ctx, struct lys_deviate *deviate, struct lys_node *dev_target,
                          struct ly_set *dflt_check, const char *value)
{
    struct lys_node *node;
    struct lys_node_choice *choice;
    struct lys_node_leaf *leaf;
    struct lys_node_leaflist *llist;
    int rc, i, j;
    unsigned int u;

    u = strlen(value);
    if (dev_target->nodetype == LYS_CHOICE) {
        choice = (struct lys_node_choice *)dev_target;
        rc = resolve_choice_default_schema_nodeid(value, choice->child, (const struct lys_node **)&node);
        if (rc || !node) {
            LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
            goto error;
        }
        if (deviate->mod == LY_DEVIATE_DEL) {
            if (!choice->dflt || (choice->dflt != node)) {
                LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Value differs from the target being deleted.");
                goto error;
            }
            choice->dflt = NULL;
            /* remove extensions of this default instance from the target node */
            j = -1;
            while ((j = lys_ext_iter(dev_target->ext, dev_target->ext_size, j + 1, LYEXT_SUBSTMT_DEFAULT)) != -1) {
                lyp_ext_instance_rm(ctx, &dev_target->ext, &dev_target->ext_size, j);
                --j;
            }
        } else { /* add or replace */
            choice->dflt = node;
            if (!choice->dflt) {
                /* default branch not found */
                LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                goto error;
            }
        }
    } else if (dev_target->nodetype == LYS_LEAF) {
        leaf = (struct lys_node_leaf *)dev_target;
        if (deviate->mod == LY_DEVIATE_DEL) {
            if (!leaf->dflt || !ly_strequal(leaf->dflt, value, 1)) {
                LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Value differs from the target being deleted.");
                goto error;
            }
            /* remove value */
            lydict_remove(ctx, leaf->dflt);
            leaf->dflt = NULL;
            leaf->flags &= ~LYS_DFLTJSON;
            /* remove extensions of this default instance from the target node */
            j = -1;
            while ((j = lys_ext_iter(dev_target->ext, dev_target->ext_size, j + 1, LYEXT_SUBSTMT_DEFAULT)) != -1) {
                lyp_ext_instance_rm(ctx, &dev_target->ext, &dev_target->ext_size, j);
                --j;
            }
        } else { /* add (already checked) and replace */
            /* remove value */
            lydict_remove(ctx, leaf->dflt);
            leaf->flags &= ~LYS_DFLTJSON;

            /* set new value */
            leaf->dflt = lydict_insert(ctx, value, u);

            /* remember to check it later (it may not fit now, but the type can be deviated too) */
            ly_set_add(dflt_check, dev_target, 0);
        }
    } else { /* LYS_LEAFLIST */
        llist = (struct lys_node_leaflist *)dev_target;
        if (deviate->mod == LY_DEVIATE_DEL) {
            /* find and remove the value in target list */
            for (i = 0; i < llist->dflt_size; i++) {
                if (llist->dflt[i] && ly_strequal(llist->dflt[i], value, 1)) {
                    /* match, remove the value */
                    lydict_remove(llist->module->ctx, llist->dflt[i]);
                    llist->dflt[i] = NULL;
                    /* remove extensions of this default instance from the target node */
                    j = -1;
                    while ((j = lys_ext_iter(dev_target->ext, dev_target->ext_size, j + 1, LYEXT_SUBSTMT_DEFAULT)) != -1) {
                        if (dev_target->ext[j]->insubstmt_index == i) {
                            lyp_ext_instance_rm(ctx, &dev_target->ext, &dev_target->ext_size, j);
                            --j;
                        } else if (dev_target->ext[j]->insubstmt_index > i) {
                            /* decrease the substatement index of the extension because of the changed array of defaults */
                            dev_target->ext[j]->insubstmt_index--;
                        }
                    }
                    break;
                }
            }
            if (i == llist->dflt_size) {
                LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "The default value to delete not found in the target node.");
                goto error;
            }
        } else {
            /* add or replace, anyway we place items into the deviate's list
               which propagates to the target */
            /* we just want to check that the value isn't already in the list */
            for (i = 0; i < llist->dflt_size; i++) {
                if (ly_strequal(llist->dflt[i], value, 1)) {
                    LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
                    LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Duplicated default value \"%s\".", value);
                    goto error;
                }
            }
            /* store it in target node */
            llist->dflt[llist->dflt_size++] = lydict_insert(ctx, value, u);

            /* remember to check it later (it may not fit now, but the type can be deviated too) */
            ly_set_add(dflt_check, dev_target, 0);
            llist->flags &= ~LYS_DFLTJSON;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_read_deviate_default(struct lys_module *module, struct lys_deviate *deviate,
                          struct lys_node *dev_target, struct ly_set * dflt_check)
{
    struct ly_ctx *ctx = module->ctx;
    int i;
    struct lys_node_leaflist *llist;
    const char **dflt;

    /* check target node type */
    if (module->version < 2 && dev_target->nodetype == LYS_LEAFLIST) {
        LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "default");
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"default\" property.");
        goto error;
    } else if (deviate->dflt_size > 1 && dev_target->nodetype != LYS_LEAFLIST) { /* from YANG 1.1 */
        LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "default");
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow multiple \"default\" properties.");
        goto error;
    } else if (!(dev_target->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_CHOICE))) {
        LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "default");
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"default\" property.");
        goto error;
    }

    if (deviate->mod == LY_DEVIATE_ADD) {
        /* check that there is no current value */
        if ((dev_target->nodetype == LYS_LEAF && ((struct lys_node_leaf *)dev_target)->dflt) ||
                (dev_target->nodetype == LYS_CHOICE && ((struct lys_node_choice *)dev_target)->dflt)) {
            LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "default");
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Adding property that already exists.");
            goto error;
        }

        /* check collision with mandatory/min-elements */
        if ((dev_target->flags & LYS_MAND_TRUE) ||
                (dev_target->nodetype == LYS_LEAFLIST && ((struct lys_node_leaflist *)dev_target)->min)) {
            LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "default", "deviation");
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL,
                   "Adding the \"default\" statement is forbidden on %s statement.",
                   (dev_target->flags & LYS_MAND_TRUE) ? "nodes with the \"mandatory\"" : "leaflists with non-zero \"min-elements\"");
            goto error;
        }
    } else if (deviate->mod == LY_DEVIATE_RPL) {
        /* check that there was a value before */
        if (((dev_target->nodetype & (LYS_LEAF | LYS_LEAFLIST)) && !((struct lys_node_leaf *)dev_target)->dflt) ||
                (dev_target->nodetype == LYS_CHOICE && !((struct lys_node_choice *)dev_target)->dflt)) {
            LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "default");
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Replacing a property that does not exist.");
            goto error;
        }
    }

    if (dev_target->nodetype == LYS_LEAFLIST) {
        /* reallocate default list in the target */
        llist = (struct lys_node_leaflist *)dev_target;
        if (deviate->mod == LY_DEVIATE_ADD) {
            /* reallocate (enlarge) the unique array of the target */
            dflt = realloc(llist->dflt, (deviate->dflt_size + llist->dflt_size) * sizeof *dflt);
            LY_CHECK_ERR_GOTO(!dflt, LOGMEM(ctx), error);
            llist->dflt = dflt;
        } else if (deviate->mod == LY_DEVIATE_RPL) {
            /* reallocate (replace) the unique array of the target */
            for (i = 0; i < llist->dflt_size; i++) {
                lydict_remove(ctx, llist->dflt[i]);
            }
            dflt = realloc(llist->dflt, deviate->dflt_size * sizeof *dflt);
            LY_CHECK_ERR_GOTO(!dflt, LOGMEM(ctx), error);
            llist->dflt = dflt;
            llist->dflt_size = 0;
        }
    }

    for (i = 0; i < deviate->dflt_size; ++i) {
        if (yang_fill_deviate_default(ctx, deviate, dev_target, dflt_check, deviate->dflt[i])) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_check_deviate_mandatory(struct lys_deviate *deviate, struct lys_node *dev_target)
{
    struct ly_ctx *ctx = dev_target->module->ctx;
    struct lys_node *parent;

    /* check target node type */
    if (!(dev_target->nodetype & (LYS_LEAF | LYS_CHOICE | LYS_ANYDATA))) {
        LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "mandatory");
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"mandatory\" property.");
        goto error;
    }

    if (deviate->mod == LY_DEVIATE_ADD) {
        /* check that there is no current value */
        if (dev_target->flags & LYS_MAND_MASK) {
            LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "mandatory");
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Adding property that already exists.");
            goto error;
        } else {
            if (dev_target->nodetype == LYS_LEAF && ((struct lys_node_leaf *)dev_target)->dflt) {
                /* RFC 6020, 7.6.4 - default statement must not with mandatory true */
                LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "mandatory", "leaf");
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "The \"mandatory\" statement is forbidden on leaf with \"default\".");
                goto error;
            } else if (dev_target->nodetype == LYS_CHOICE && ((struct lys_node_choice *)dev_target)->dflt) {
                LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "mandatory", "choice");
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "The \"mandatory\" statement is forbidden on choices with \"default\".");
                goto error;
            }
        }
    } else { /* replace */
        if (!(dev_target->flags & LYS_MAND_MASK)) {
            LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "mandatory");
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Replacing a property that does not exist.");
            goto error;
        }
    }

    /* remove current mandatory value of the target ... */
    dev_target->flags &= ~LYS_MAND_MASK;

    /* ... and replace it with the value specified in deviation */
    dev_target->flags |= deviate->flags & LYS_MAND_MASK;

    /* check for mandatory node in default case, first find the closest parent choice to the changed node */
    for (parent = dev_target->parent;
         parent && !(parent->nodetype & (LYS_CHOICE | LYS_GROUPING | LYS_ACTION));
         parent = parent->parent) {
        if (parent->nodetype == LYS_CONTAINER && ((struct lys_node_container *)parent)->presence) {
            /* stop also on presence containers */
            break;
        }
    }
    /* and if it is a choice with the default case, check it for presence of a mandatory node in it */
    if (parent && parent->nodetype == LYS_CHOICE && ((struct lys_node_choice *)parent)->dflt) {
        if (lyp_check_mandatory_choice(parent)) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_read_deviate_minmax(struct lys_deviate *deviate, struct lys_node *dev_target, uint32_t value, int type)
{
    struct ly_ctx *ctx = dev_target->module->ctx;
    uint32_t *ui32val, *min, *max;

    /* check target node type */
    if (dev_target->nodetype == LYS_LEAFLIST) {
        max = &((struct lys_node_leaflist *)dev_target)->max;
        min = &((struct lys_node_leaflist *)dev_target)->min;
    } else if (dev_target->nodetype == LYS_LIST) {
        max = &((struct lys_node_list *)dev_target)->max;
        min = &((struct lys_node_list *)dev_target)->min;
    } else {
        LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, (type) ? "max-elements" : "min-elements");
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"%s\" property.", (type) ? "max-elements" : "min-elements");
        goto error;
    }

    ui32val = (type) ? max : min;
    if (deviate->mod == LY_DEVIATE_ADD) {
        /* check that there is no current value */
        if (*ui32val) {
            LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, (type) ? "max-elements" : "min-elements");
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Adding property that already exists.");
            goto error;
        }
    } else if (deviate->mod == LY_DEVIATE_RPL) {
        /* unfortunately, there is no way to check reliably that there
         * was a value before, it could have been the default */
    }

    /* add (already checked) and replace */
    /* set new value specified in deviation */
    *ui32val = value;

    /* check min-elements is smaller than max-elements */
    if (*max && *min > *max) {
        if (type) {
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"max-elements\".", value);
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "\"max-elements\" is smaller than \"min-elements\".");
        } else {
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"min-elements\".", value);
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "\"min-elements\" is bigger than \"max-elements\".");
        }
        goto error;
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
yang_check_deviate_must(struct lys_module *module, struct unres_schema *unres,
                        struct lys_deviate *deviate, struct lys_node *dev_target)
{
    struct ly_ctx *ctx = module->ctx;
    int i, j, erase_must = 1;
    struct lys_restr **trg_must, *must;
    uint8_t *trg_must_size;

    /* check target node type */
    switch (dev_target->nodetype) {
        case LYS_LEAF:
            trg_must = &((struct lys_node_leaf *)dev_target)->must;
            trg_must_size = &((struct lys_node_leaf *)dev_target)->must_size;
            break;
        case LYS_CONTAINER:
            trg_must = &((struct lys_node_container *)dev_target)->must;
            trg_must_size = &((struct lys_node_container *)dev_target)->must_size;
            break;
        case LYS_LEAFLIST:
            trg_must = &((struct lys_node_leaflist *)dev_target)->must;
            trg_must_size = &((struct lys_node_leaflist *)dev_target)->must_size;
            break;
        case LYS_LIST:
            trg_must = &((struct lys_node_list *)dev_target)->must;
            trg_must_size = &((struct lys_node_list *)dev_target)->must_size;
            break;
        case LYS_ANYXML:
        case LYS_ANYDATA:
            trg_must = &((struct lys_node_anydata *)dev_target)->must;
            trg_must_size = &((struct lys_node_anydata *)dev_target)->must_size;
            break;
        default:
            LOGVAL(ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "must");
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"must\" property.");
            goto error;
    }

    /* flag will be checked again, clear it for now */
    dev_target->flags &= ~(LYS_XPCONF_DEP | LYS_XPSTATE_DEP);

    if (deviate->mod == LY_DEVIATE_ADD) {
        /* reallocate the must array of the target */
        must = ly_realloc(*trg_must, (deviate->must_size + *trg_must_size) * sizeof *must);
        LY_CHECK_ERR_GOTO(!must, LOGMEM(ctx), error);
        *trg_must = must;
        memcpy(&(*trg_must)[*trg_must_size], deviate->must, deviate->must_size * sizeof *must);
        *trg_must_size = *trg_must_size + deviate->must_size;
        erase_must = 0;
    } else if (deviate->mod == LY_DEVIATE_DEL) {
        /* find must to delete, we are ok with just matching conditions */
        for (j = 0; j < deviate->must_size; ++j) {
            for (i = 0; i < *trg_must_size; i++) {
                if (ly_strequal(deviate->must[j].expr, (*trg_must)[i].expr, 1)) {
                    /* we have a match, free the must structure ... */
                    lys_restr_free(module->ctx, &((*trg_must)[i]), NULL);
                    /* ... and maintain the array */
                    (*trg_must_size)--;
                    if (i != *trg_must_size) {
                        memcpy(&(*trg_must)[i], &(*trg_must)[*trg_must_size], sizeof *must);
                    }
                    if (!(*trg_must_size)) {
                        free(*trg_must);
                        *trg_must = NULL;
                    } else {
                        memset(&(*trg_must)[*trg_must_size], 0, sizeof *must);
                    }

                    i = -1; /* set match flag */
                    break;
                }
            }
            if (i != -1) {
                /* no match found */
                LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, deviate->must[j].expr, "must");
                LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Value does not match any must from the target.");
                goto error;
            }
        }
    }

    if (yang_check_must(module, deviate->must, deviate->must_size, unres)) {
        goto error;
    }
    /* check XPath dependencies */
    if (!(module->ctx->models.flags & LY_CTX_TRUSTED) && *trg_must_size
            && (unres_schema_add_node(module, unres, dev_target, UNRES_XPATH, NULL) == -1)) {
        goto error;
    }

    return EXIT_SUCCESS;

error:
    if (deviate->mod == LY_DEVIATE_ADD && erase_must) {
        for (i = 0; i < deviate->must_size; ++i) {
            lys_restr_free(module->ctx, &deviate->must[i], NULL);
        }
        free(deviate->must);
    }
    return EXIT_FAILURE;
}

int
yang_deviate_delete_unique(struct lys_module *module, struct lys_deviate *deviate,
                           struct lys_node_list *list, int index, char * value)
{
    struct ly_ctx *ctx = module->ctx;
    int i, j, k;

    /* find unique structures to delete */
    for (i = 0; i < list->unique_size; i++) {
        if (list->unique[i].expr_size != deviate->unique[index].expr_size) {
            continue;
        }

        for (j = 0; j < deviate->unique[index].expr_size; j++) {
            if (!ly_strequal(list->unique[i].expr[j], deviate->unique[index].expr[j], 1)) {
                break;
            }
        }

        if (j == deviate->unique[index].expr_size) {
            /* we have a match, free the unique structure ... */
            for (j = 0; j < list->unique[i].expr_size; j++) {
                lydict_remove(ctx, list->unique[i].expr[j]);
            }
            free(list->unique[i].expr);
            /* ... and maintain the array */
            list->unique_size--;
            if (i != list->unique_size) {
                list->unique[i].expr_size = list->unique[list->unique_size].expr_size;
                list->unique[i].expr = list->unique[list->unique_size].expr;
            }

            if (!list->unique_size) {
                free(list->unique);
                list->unique = NULL;
            } else {
                list->unique[list->unique_size].expr_size = 0;
                list->unique[list->unique_size].expr = NULL;
            }

            k = i; /* remember index for removing extensions */
            i = -1; /* set match flag */
            break;
        }
    }

    if (i != -1) {
        /* no match found */
        LOGVAL(ctx, LYE_INARG, LY_VLOG_NONE, NULL, value, "unique");
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Value differs from the target being deleted.");
        return EXIT_FAILURE;
    }

    /* remove extensions of this unique instance from the target node */
    j = -1;
    while ((j = lys_ext_iter(list->ext, list->ext_size, j + 1, LYEXT_SUBSTMT_UNIQUE)) != -1) {
        if (list->ext[j]->insubstmt_index == k) {
            lyp_ext_instance_rm(ctx, &list->ext, &list->ext_size, j);
            --j;
        } else if (list->ext[j]->insubstmt_index > k) {
            /* decrease the substatement index of the extension because of the changed array of uniques */
            list->ext[j]->insubstmt_index--;
        }
    }
    return EXIT_SUCCESS;
}

int yang_check_deviate_unique(struct lys_module *module, struct lys_deviate *deviate, struct lys_node *dev_target)
{
    struct lys_node_list *list;
    char *str;
    uint i = 0;
    struct lys_unique *last_unique = NULL;

    if (yang_read_deviate_unique(deviate, dev_target)) {
        goto error;
    }
    list = (struct lys_node_list *)dev_target;
    last_unique = &list->unique[list->unique_size];
    for (i = 0; i < deviate->unique_size; ++i) {
        str = (char *) deviate->unique[i].expr;
        if (deviate->mod == LY_DEVIATE_ADD) {
            if (yang_fill_unique(module, list, &list->unique[list->unique_size], str, NULL)) {
                free(str);
                goto error;
            }
            list->unique_size++;
        } else if (deviate->mod == LY_DEVIATE_DEL) {
            if (yang_fill_unique(module, list, &deviate->unique[i], str, NULL)) {
                free(str);
                goto error;
            }
            if (yang_deviate_delete_unique(module, deviate, list, i, str)) {
                free(str);
                goto error;
            }
        }
        free(str);
    }
    if (deviate->mod == LY_DEVIATE_ADD) {
        free(deviate->unique);
        deviate->unique = last_unique;
    }

    return EXIT_SUCCESS;

error:
    if (deviate->mod == LY_DEVIATE_ADD) {
        for (i = i + 1; i < deviate->unique_size; ++i) {
            free(deviate->unique[i].expr);
        }
        free(deviate->unique);
        deviate->unique = last_unique;

    }
    return EXIT_FAILURE;
}

static int
yang_fill_include(struct lys_module *trg, char *value, struct lys_include *inc,
                  struct unres_schema *unres)
{
    const char *str;
    int rc;
    int ret = 0;

    str = lydict_insert_zc(trg->ctx, value);
    rc = lyp_check_include(trg, str, inc, unres);
    if (!rc) {
        /* success, copy the filled data into the final array */
        memcpy(&trg->inc[trg->inc_size], inc, sizeof *inc);
        if (yang_check_ext_instance(trg, &trg->inc[trg->inc_size].ext, trg->inc[trg->inc_size].ext_size,
                                    &trg->inc[trg->inc_size], unres)) {
            ret = -1;
        }
        trg->inc_size++;
    } else if (rc == -1) {
        lys_extension_instances_free(trg->ctx, inc->ext, inc->ext_size, NULL);
        ret = -1;
    }

    lydict_remove(trg->ctx, str);
    return ret;
}

struct lys_ext_instance *
yang_ext_instance(void *node, enum yytokentype type, int is_ext_instance)
{
    struct lys_ext_instance ***ext, **tmp, *instance = NULL;
    LYEXT_PAR parent_type;
    uint8_t *size;

    switch (type) {
    case MODULE_KEYWORD:
    case SUBMODULE_KEYWORD:
        ext = &((struct lys_module *)node)->ext;
        size = &((struct lys_module *)node)->ext_size;
        parent_type = LYEXT_PAR_MODULE;
        break;
    case BELONGS_TO_KEYWORD:
        if (is_ext_instance) {
            ext = &((struct lys_ext_instance *)node)->ext;
            size = &((struct lys_ext_instance *)node)->ext_size;
            parent_type = LYEXT_PAR_EXTINST;
        } else {
            ext = &((struct lys_module *)node)->ext;
            size = &((struct lys_module *)node)->ext_size;
            parent_type = LYEXT_PAR_MODULE;
        }
        break;
    case IMPORT_KEYWORD:
        ext = &((struct lys_import *)node)->ext;
        size = &((struct lys_import *)node)->ext_size;
        parent_type = LYEXT_PAR_IMPORT;
        break;
    case INCLUDE_KEYWORD:
        ext = &((struct lys_include *)node)->ext;
        size = &((struct lys_include *)node)->ext_size;
        parent_type = LYEXT_PAR_INCLUDE;
        break;
    case REVISION_KEYWORD:
        ext = &((struct lys_revision *)node)->ext;
        size = &((struct lys_revision *)node)->ext_size;
        parent_type = LYEXT_PAR_REVISION;
        break;
    case GROUPING_KEYWORD:
    case CONTAINER_KEYWORD:
    case LEAF_KEYWORD:
    case LEAF_LIST_KEYWORD:
    case LIST_KEYWORD:
    case CHOICE_KEYWORD:
    case CASE_KEYWORD:
    case ANYXML_KEYWORD:
    case ANYDATA_KEYWORD:
    case USES_KEYWORD:
    case AUGMENT_KEYWORD:
    case ACTION_KEYWORD:
    case RPC_KEYWORD:
    case INPUT_KEYWORD:
    case OUTPUT_KEYWORD:
    case NOTIFICATION_KEYWORD:
        ext = &((struct lys_node *)node)->ext;
        size = &((struct lys_node *)node)->ext_size;
        parent_type = LYEXT_PAR_NODE;
        break;
    case ARGUMENT_KEYWORD:
        if (is_ext_instance) {
            ext = &((struct lys_ext_instance *)node)->ext;
            size = &((struct lys_ext_instance *)node)->ext_size;
            parent_type = LYEXT_PAR_EXTINST;
        } else {
            ext = &((struct lys_ext *)node)->ext;
            size = &((struct lys_ext *)node)->ext_size;
            parent_type = LYEXT_PAR_EXT;
        }
        break;
    case EXTENSION_KEYWORD:
        ext = &((struct lys_ext *)node)->ext;
        size = &((struct lys_ext *)node)->ext_size;
        parent_type = LYEXT_PAR_EXT;
        break;
    case FEATURE_KEYWORD:
        ext = &((struct lys_feature *)node)->ext;
        size = &((struct lys_feature *)node)->ext_size;
        parent_type = LYEXT_PAR_FEATURE;
        break;
    case IDENTITY_KEYWORD:
        ext = &((struct lys_ident *)node)->ext;
        size = &((struct lys_ident *)node)->ext_size;
        parent_type = LYEXT_PAR_IDENT;
        break;
    case IF_FEATURE_KEYWORD:
        ext = &((struct lys_iffeature *)node)->ext;
        size = &((struct lys_iffeature *)node)->ext_size;
        parent_type = LYEXT_PAR_IFFEATURE;
        break;
    case TYPEDEF_KEYWORD:
        ext = &((struct lys_tpdf *)node)->ext;
        size = &((struct lys_tpdf *)node)->ext_size;
        parent_type = LYEXT_PAR_TPDF;
        break;
    case TYPE_KEYWORD:
        ext = &((struct yang_type *)node)->type->ext;
        size = &((struct yang_type *)node)->type->ext_size;
        parent_type = LYEXT_PAR_TYPE;
        break;
    case LENGTH_KEYWORD:
    case PATTERN_KEYWORD:
    case RANGE_KEYWORD:
    case MUST_KEYWORD:
        ext = &((struct lys_restr *)node)->ext;
        size = &((struct lys_restr *)node)->ext_size;
        parent_type = LYEXT_PAR_RESTR;
        break;
    case WHEN_KEYWORD:
        ext = &((struct lys_when *)node)->ext;
        size = &((struct lys_when *)node)->ext_size;
        parent_type = LYEXT_PAR_RESTR;
        break;
    case ENUM_KEYWORD:
        ext = &((struct lys_type_enum *)node)->ext;
        size = &((struct lys_type_enum *)node)->ext_size;
        parent_type = LYEXT_PAR_TYPE_ENUM;
        break;
    case BIT_KEYWORD:
        ext = &((struct lys_type_bit *)node)->ext;
        size = &((struct lys_type_bit *)node)->ext_size;
        parent_type = LYEXT_PAR_TYPE_BIT;
        break;
    case REFINE_KEYWORD:
        ext = &((struct lys_type_bit *)node)->ext;
        size = &((struct lys_type_bit *)node)->ext_size;
        parent_type = LYEXT_PAR_REFINE;
        break;
    case DEVIATION_KEYWORD:
        ext = &((struct lys_deviation *)node)->ext;
        size = &((struct lys_deviation *)node)->ext_size;
        parent_type = LYEXT_PAR_DEVIATION;
        break;
    case NOT_SUPPORTED_KEYWORD:
    case ADD_KEYWORD:
    case DELETE_KEYWORD:
    case REPLACE_KEYWORD:
        ext = &((struct lys_deviate *)node)->ext;
        size = &((struct lys_deviate *)node)->ext_size;
        parent_type = LYEXT_PAR_DEVIATE;
        break;
    case EXTENSION_INSTANCE:
        ext = &((struct lys_ext_instance *)node)->ext;
        size = &((struct lys_ext_instance *)node)->ext_size;
        parent_type = LYEXT_PAR_EXTINST;
        break;
    default:
        LOGINT(NULL);
        return NULL;
    }

    instance = calloc(1, sizeof *instance);
    if (!instance) {
        goto error;
    }
    instance->parent_type = parent_type;
    tmp = realloc(*ext, (*size + 1) * sizeof *tmp);
    if (!tmp) {
        goto error;
    }
    tmp[*size] = instance;
    *ext = tmp;
    (*size)++;
    return instance;

error:
    LOGMEM(NULL);
    free(instance);
    return NULL;
}

void *
yang_read_ext(struct lys_module *module, void *actual, char *ext_name, char *ext_arg,
              enum yytokentype actual_type, enum yytokentype backup_type, int is_ext_instance)
{
    struct lys_ext_instance *instance;
    LY_STMT stmt = LY_STMT_UNKNOWN;

    if (backup_type != NODE) {
        instance = yang_ext_instance(actual, backup_type, is_ext_instance);
        if (!instance) {
            return NULL;
        }
        switch (actual_type) {
        case YANG_VERSION_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_VERSION;
            stmt = LY_STMT_VERSION;
            break;
        case NAMESPACE_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_NAMESPACE;
            stmt = LY_STMT_NAMESPACE;
            break;
        case PREFIX_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_PREFIX;
            stmt = LY_STMT_PREFIX;
            break;
        case REVISION_DATE_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_REVISIONDATE;
            stmt = LY_STMT_REVISIONDATE;
            break;
        case DESCRIPTION_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_DESCRIPTION;
            stmt = LY_STMT_DESCRIPTION;
            break;
        case REFERENCE_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_REFERENCE;
            stmt = LY_STMT_REFERENCE;
            break;
        case CONTACT_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_CONTACT;
            stmt = LY_STMT_CONTACT;
            break;
        case ORGANIZATION_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_ORGANIZATION;
            stmt = LY_STMT_ORGANIZATION;
            break;
        case YIN_ELEMENT_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_YINELEM;
            stmt = LY_STMT_YINELEM;
            break;
        case STATUS_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_STATUS;
            stmt = LY_STMT_STATUS;
            break;
        case BASE_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_BASE;
            stmt = LY_STMT_BASE;
            if (backup_type == IDENTITY_KEYWORD) {
                instance->insubstmt_index = ((struct lys_ident *)actual)->base_size;
            } else if (backup_type == TYPE_KEYWORD) {
                instance->insubstmt_index = ((struct yang_type *)actual)->type->info.ident.count;
            }
            break;
        case DEFAULT_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_DEFAULT;
            stmt = LY_STMT_DEFAULT;
            switch (backup_type) {
            case LEAF_LIST_KEYWORD:
                instance->insubstmt_index = ((struct lys_node_leaflist *)actual)->dflt_size;
                break;
            case REFINE_KEYWORD:
                instance->insubstmt_index = ((struct lys_refine *)actual)->dflt_size;
                break;
            case ADD_KEYWORD:
                instance->insubstmt_index = ((struct lys_deviate *)actual)->dflt_size;
                break;
            default:
                /* nothing changes */
                break;
            }
            break;
        case UNITS_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_UNITS;
            stmt = LY_STMT_UNITS;
            break;
        case REQUIRE_INSTANCE_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_REQINSTANCE;
            stmt = LY_STMT_REQINSTANCE;
            break;
        case PATH_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_PATH;
            stmt = LY_STMT_PATH;
            break;
        case ERROR_MESSAGE_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_ERRMSG;
            stmt = LY_STMT_ERRMSG;
            break;
        case ERROR_APP_TAG_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_ERRTAG;
            stmt = LY_STMT_ERRTAG;
            break;
        case MODIFIER_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_MODIFIER;
            stmt = LY_STMT_MODIFIER;
            break;
        case FRACTION_DIGITS_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_DIGITS;
            stmt = LY_STMT_DIGITS;
            break;
        case VALUE_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_VALUE;
            stmt = LY_STMT_VALUE;
            break;
        case POSITION_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_POSITION;
            stmt = LY_STMT_POSITION;
            break;
        case PRESENCE_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_PRESENCE;
            stmt = LY_STMT_PRESENCE;
            break;
        case CONFIG_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_CONFIG;
            stmt = LY_STMT_CONFIG;
            break;
        case MANDATORY_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_MANDATORY;
            stmt = LY_STMT_MANDATORY;
            break;
        case MIN_ELEMENTS_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_MIN;
            stmt = LY_STMT_MIN;
            break;
        case MAX_ELEMENTS_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_MAX;
            stmt = LY_STMT_MAX;
            break;
        case ORDERED_BY_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_ORDEREDBY;
            stmt = LY_STMT_ORDEREDBY;
            break;
        case KEY_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_KEY;
            stmt = LY_STMT_KEY;
            break;
        case UNIQUE_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_UNIQUE;
            stmt = LY_STMT_UNIQUE;
            switch (backup_type) {
            case LIST_KEYWORD:
                instance->insubstmt_index = ((struct lys_node_list *)actual)->unique_size;
                break;
            case ADD_KEYWORD:
            case DELETE_KEYWORD:
            case REPLACE_KEYWORD:
                instance->insubstmt_index = ((struct lys_deviate *)actual)->unique_size;
                break;
            default:
                /* nothing changes */
                break;
            }
            break;
        default:
            LOGINT(module->ctx);
            return NULL;
        }
    } else {
        instance = yang_ext_instance(actual, actual_type, is_ext_instance);
        if (!instance) {
            return NULL;
        }
        instance->insubstmt = LYEXT_SUBSTMT_SELF;
        switch (actual_type) {
        case ARGUMENT_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_ARGUMENT;
            stmt = LY_STMT_ARGUMENT;
            break;
        case BELONGS_TO_KEYWORD:
            instance->insubstmt = LYEXT_SUBSTMT_BELONGSTO;
            stmt = LY_STMT_BELONGSTO;
            break;
        default:
            instance->insubstmt = LYEXT_SUBSTMT_SELF;
            break;
        }
    }
    instance->flags |= LYEXT_OPT_YANG;
    instance->def = (struct lys_ext *)ext_name;    /* hack for UNRES */
    instance->arg_value = lydict_insert_zc(module->ctx, ext_arg);
    if (is_ext_instance && stmt != LY_STMT_UNKNOWN && instance->parent_type == LYEXT_PAR_EXTINST) {
        instance->insubstmt_index = yang_fill_ext_substm_index(actual, stmt, backup_type);
    }
    return instance;
}

static int
check_status_flag(struct lys_node *node, struct lys_node *parent)
{
    struct ly_ctx *ctx = node->module->ctx;
    char *str;

    if (node->nodetype & (LYS_OUTPUT | LYS_INPUT)) {
        return EXIT_SUCCESS;
    }

    if (parent && (parent->flags & (LYS_STATUS_DEPRC | LYS_STATUS_OBSLT))) {
        /* status is not inherited by specification, but it not make sense to have
         * current in deprecated or deprecated in obsolete, so we print warning
         * and fix the schema by inheriting */
        if (!(node->flags & (LYS_STATUS_MASK))) {
            /* status not explicitely specified on the current node -> inherit */
            str = lys_path(node, LYS_PATH_FIRST_PREFIX);
            LOGWRN(ctx, "Missing status in %s subtree (%s), inheriting.",
                   parent->flags & LYS_STATUS_DEPRC ? "deprecated" : "obsolete", str);
            free(str);
            node->flags |= parent->flags & LYS_STATUS_MASK;
        } else if ((parent->flags & LYS_STATUS_MASK) > (node->flags & LYS_STATUS_MASK)) {
            /* invalid combination of statuses */
            switch (node->flags & LYS_STATUS_MASK) {
                case 0:
                case LYS_STATUS_CURR:
                    LOGVAL(ctx, LYE_INSTATUS, LY_VLOG_LYS, parent, "current", strnodetype(node->nodetype), "is child of",
                           parent->flags & LYS_STATUS_DEPRC ? "deprecated" : "obsolete", parent->name);
                    break;
                case LYS_STATUS_DEPRC:
                    LOGVAL(ctx, LYE_INSTATUS, LY_VLOG_LYS, parent, "deprecated", strnodetype(node->nodetype), "is child of",
                           "obsolete", parent->name);
                    break;
            }
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int
store_config_flag(struct lys_node *node, int options)
{
    switch (node->nodetype) {
    case LYS_CONTAINER:
    case LYS_LEAF:
    case LYS_LEAFLIST:
    case LYS_LIST:
    case LYS_CHOICE:
    case LYS_ANYDATA:
    case LYS_ANYXML:
        if (options & LYS_PARSE_OPT_CFG_IGNORE) {
            node->flags |= node->flags & (~(LYS_CONFIG_MASK | LYS_CONFIG_SET));
        } else if (!(options & LYS_PARSE_OPT_CFG_NOINHERIT)) {
            if (!(node->flags & LYS_CONFIG_MASK)) {
                /* get config flag from parent */
                if (node->parent) {
                    node->flags |= node->parent->flags & LYS_CONFIG_MASK;
                } else {
                    /* default config is true */
                    node->flags |= LYS_CONFIG_W;
                }
            }
        }
        break;
    case LYS_CASE:
        if (!(options & (LYS_PARSE_OPT_CFG_IGNORE | LYS_PARSE_OPT_CFG_NOINHERIT))) {
            if (!(node->flags & LYS_CONFIG_MASK)) {
                /* get config flag from parent */
                if (node->parent) {
                    node->flags |= node->parent->flags & LYS_CONFIG_MASK;
                } else {
                    /* default config is true */
                    node->flags |= LYS_CONFIG_W;
                }
            }
        }
        break;
    default:
        break;
    }

    return EXIT_SUCCESS;
}

int
yang_parse_mem(struct lys_module *module, struct lys_submodule *submodule, struct unres_schema *unres,
               const char *data, unsigned int size_data, struct lys_node **node)
{
    unsigned int size;
    YY_BUFFER_STATE bp;
    yyscan_t scanner = NULL;
    int ret = 0;
    struct lys_module *trg;
    struct yang_parameter param;

    size = (size_data) ? size_data : strlen(data) + 2;
    yylex_init(&scanner);
    yyset_extra(module->ctx, scanner);
    bp = yy_scan_buffer((char *)data, size, scanner);
    yy_switch_to_buffer(bp, scanner);
    memset(&param, 0, sizeof param);
    param.module = module;
    param.submodule = submodule;
    param.unres = unres;
    param.node = node;
    param.flags |= YANG_REMOVE_IMPORT;
    if (yyparse(scanner, &param)) {
        if (param.flags & YANG_REMOVE_IMPORT) {
            trg = (submodule) ? (struct lys_module *)submodule : module;
            yang_free_import(trg->ctx, trg->imp, 0, trg->imp_size);
            yang_free_include(trg->ctx, trg->inc, 0, trg->inc_size);
            trg->inc_size = 0;
            trg->imp_size = 0;
        }
        ret = (param.flags & YANG_EXIST_MODULE) ? 1 : -1;
    }
    yy_delete_buffer(bp, scanner);
    yylex_destroy(scanner);
    return ret;
}

int
yang_parse_ext_substatement(struct lys_module *module, struct unres_schema *unres, const char *data,
                            char *ext_name, struct lys_ext_instance_complex *ext)
{
    unsigned int size;
    YY_BUFFER_STATE bp;
    yyscan_t scanner = NULL;
    int ret = 0;
    struct yang_parameter param;
    struct lys_node *node = NULL;

    if (!data) {
        return EXIT_SUCCESS;
    }
    size = strlen(data) + 2;
    yylex_init(&scanner);
    bp = yy_scan_buffer((char *)data, size, scanner);
    yy_switch_to_buffer(bp, scanner);
    memset(&param, 0, sizeof param);
    param.module = module;
    param.unres = unres;
    param.node = &node;
    param.actual_node = (void **)ext;
    param.data_node = (void **)ext_name;
    param.flags |= EXT_INSTANCE_SUBSTMT;
    if (yyparse(scanner, &param)) {
        yang_free_nodes(module->ctx, node);
        ret = -1;
    } else {
        /* success parse, but it needs some sematic controls */
        if (node && yang_check_nodes(module, (struct lys_node *)ext, node, LYS_PARSE_OPT_CFG_NOINHERIT, unres)) {
            ret = -1;
        }
    }
    yy_delete_buffer(bp, scanner);
    yylex_destroy(scanner);
    return ret;
}

struct lys_module *
yang_read_module(struct ly_ctx *ctx, const char* data, unsigned int size, const char *revision, int implement)
{
    struct lys_module *module = NULL, *tmp_mod;
    struct unres_schema *unres = NULL;
    struct lys_node *node = NULL;
    int ret;

    unres = calloc(1, sizeof *unres);
    LY_CHECK_ERR_GOTO(!unres, LOGMEM(ctx), error);

    module = calloc(1, sizeof *module);
    LY_CHECK_ERR_GOTO(!module, LOGMEM(ctx), error);

    /* initiale module */
    module->ctx = ctx;
    module->type = 0;
    module->implemented = (implement ? 1 : 0);

    /* add into the list of processed modules */
    if (lyp_check_circmod_add(module)) {
        goto error;
    }

    ret = yang_parse_mem(module, NULL, unres, data, size, &node);
    if (ret == -1) {
        if (ly_vecode(ctx) == LYVE_SUBMODULE && !module->name) {
            /* Remove this module from the list of processed modules,
               as we're about to free it */
            lyp_check_circmod_pop(ctx);

            free(module);
            module = NULL;
        } else {
            free_yang_common(module, node);
        }
        goto error;
    } else if (ret == 1) {
        assert(!unres->count);
    } else {
        if (yang_check_sub_module(module, unres, node)) {
            goto error;
        }

        if (!implement && module->implemented && lys_make_implemented_r(module, unres)) {
            goto error;
        }

        if (unres->count && resolve_unres_schema(module, unres)) {
            goto error;
        }

        /* check correctness of includes */
        if (lyp_check_include_missing(module)) {
            goto error;
        }
    }

    lyp_sort_revisions(module);

    if (lyp_rfn_apply_ext(module) || lyp_deviation_apply_ext(module)) {
        goto error;
    }

    if (revision) {
        /* check revision of the parsed model */
        if (!module->rev_size || strcmp(revision, module->rev[0].date)) {
            LOGVRB("Module \"%s\" parsed with the wrong revision (\"%s\" instead \"%s\").",
                   module->name, module->rev[0].date, revision);
            goto error;
        }
    }

    /* add into context if not already there */
    if (!ret) {
        if (lyp_ctx_add_module(module)) {
            goto error;
        }

        /* remove our submodules from the parsed submodules list */
        lyp_del_includedup(module, 0);
    } else {
        tmp_mod = module;

        /* get the model from the context */
        module = (struct lys_module *)ly_ctx_get_module(ctx, module->name, revision, 0);
        assert(module);

        /* free what was parsed */
        lys_free(tmp_mod, NULL, 0, 0);
    }

    unres_schema_free(NULL, &unres, 0);
    lyp_check_circmod_pop(ctx);
    LOGVRB("Module \"%s%s%s\" successfully parsed as %s.", module->name, (module->rev_size ? "@" : ""),
           (module->rev_size ? module->rev[0].date : ""), (module->implemented ? "implemented" : "imported"));
    return module;

error:
    /* cleanup */
    unres_schema_free(module, &unres, 1);

    if (!module) {
        if (ly_vecode(ctx) != LYVE_SUBMODULE) {
            LOGERR(ctx, ly_errno, "Module parsing failed.");
        }
        return NULL;
    }

    if (module->name) {
        LOGERR(ctx, ly_errno, "Module \"%s\" parsing failed.", module->name);
    } else {
        LOGERR(ctx, ly_errno, "Module parsing failed.");
    }

    lyp_check_circmod_pop(ctx);
    lys_sub_module_remove_devs_augs(module);
    lyp_del_includedup(module, 1);
    lys_free(module, NULL, 0, 1);
    return NULL;
}

struct lys_submodule *
yang_read_submodule(struct lys_module *module, const char *data, unsigned int size, struct unres_schema *unres)
{
    struct lys_submodule *submodule;
    struct lys_node *node = NULL;

    submodule = calloc(1, sizeof *submodule);
    LY_CHECK_ERR_GOTO(!submodule, LOGMEM(module->ctx), error);

    submodule->ctx = module->ctx;
    submodule->type = 1;
    submodule->implemented = module->implemented;
    submodule->belongsto = module;

    /* add into the list of processed modules */
    if (lyp_check_circmod_add((struct lys_module *)submodule)) {
        goto error;
    }

    /* module cannot be changed in this case and 1 cannot be returned */
    if (yang_parse_mem(module, submodule, unres, data, size, &node)) {
        free_yang_common((struct lys_module *)submodule, node);
        goto error;
    }

    lyp_sort_revisions((struct lys_module *)submodule);

    if (yang_check_sub_module((struct lys_module *)submodule, unres, node)) {
        goto error;
    }

    lyp_check_circmod_pop(module->ctx);

    LOGVRB("Submodule \"%s\" successfully parsed.", submodule->name);
    return submodule;

error:
    /* cleanup */
    if (!submodule || !submodule->name) {
        free(submodule);
        LOGERR(module->ctx, ly_errno, "Submodule parsing failed.");
        return NULL;
    }

    LOGERR(module->ctx, ly_errno, "Submodule \"%s\" parsing failed.", submodule->name);

    unres_schema_free((struct lys_module *)submodule, &unres, 0);
    lyp_check_circmod_pop(module->ctx);
    lys_sub_module_remove_devs_augs((struct lys_module *)submodule);
    lys_submodule_module_data_free(submodule);
    lys_submodule_free(submodule, NULL);
    return NULL;
}

static int
read_indent(const char *input, int indent, int size, int in_index, int *out_index, char *output)
{
    int k = 0, j;

    while (in_index < size) {
        if (input[in_index] == ' ') {
            k++;
        } else if (input[in_index] == '\t') {
            /* RFC 6020 6.1.3 tab character is treated as 8 space characters */
            k += 8;
        } else  if (input[in_index] == '\\' && input[in_index + 1] == 't') {
            /* RFC 6020 6.1.3 tab character is treated as 8 space characters */
            k += 8;
            ++in_index;
        } else {
            break;
        }
        ++in_index;
        if (k >= indent) {
            for (j = k - indent; j > 0; --j) {
                output[*out_index] = ' ';
                if (j > 1) {
                    ++(*out_index);
                }
            }
            break;
        }
    }
    return in_index - 1;
}

char *
yang_read_string(struct ly_ctx *ctx, const char *input, char *output, int size, int offset, int indent)
{
    int i = 0, out_index = offset, space = 0;

    while (i < size) {
        switch (input[i]) {
        case '\n':
            out_index -= space;
            output[out_index] = '\n';
            space = 0;
            i = read_indent(input, indent, size, i + 1, &out_index, output);
            break;
        case ' ':
        case '\t':
            output[out_index] = input[i];
            ++space;
            break;
        case '\\':
            if (input[i + 1] == 'n') {
                out_index -= space;
                output[out_index] = '\n';
                space = 0;
                i = read_indent(input, indent, size, i + 2, &out_index, output);
            } else if (input[i + 1] == 't') {
                output[out_index] = '\t';
                ++i;
                ++space;
            } else if (input[i + 1] == '\\') {
                output[out_index] = '\\';
                ++i;
            } else if ((i + 1) != size && input[i + 1] == '"') {
                output[out_index] = '"';
                ++i;
            } else {
                /* backslash must not be followed by any other character */
                LOGVAL(ctx, LYE_XML_INCHAR, LY_VLOG_NONE, NULL, input + i);
                return NULL;
            }
            break;
        default:
            output[out_index] = input[i];
            space = 0;
            break;
        }
        ++i;
        ++out_index;
    }
    output[out_index] = '\0';
    if (size != out_index) {
        output = realloc(output, out_index + 1);
        LY_CHECK_ERR_RETURN(!output, LOGMEM(ctx), NULL);
    }
    return output;
}

/* free function */

void
yang_type_free(struct ly_ctx *ctx, struct lys_type *type)
{
    struct yang_type *stype = (struct yang_type *)type->der;
    unsigned int i;

    if (!stype) {
        return ;
    }
    if (type->base == LY_TYPE_DER || type->base == LY_TYPE_UNION) {
        lydict_remove(ctx, stype->name);
        if (stype->base == LY_TYPE_IDENT && (!(stype->flags & LYS_NO_ERASE_IDENTITY))) {
            for (i = 0; i < type->info.ident.count; ++i) {
                free(type->info.ident.ref[i]);
            }
        }
        if (stype->base == LY_TYPE_UNION) {
            for (i = 0; i < type->info.uni.count; ++i) {
                yang_type_free(ctx, &type->info.uni.types[i]);
            }
            free(type->info.uni.types);
            type->base = LY_TYPE_DER;
        } else {
            type->base = stype->base;
        }
        free(stype);
        type->der = NULL;
    }
    lys_type_free(ctx, type, NULL);
    memset(type, 0, sizeof (struct lys_type));
}

static void
yang_tpdf_free(struct ly_ctx *ctx, struct lys_tpdf *tpdf, uint16_t start, uint16_t size)
{
    uint8_t i;

    assert(ctx);
    if (!tpdf) {
        return;
    }

    for (i = start; i < size; ++i) {
        lydict_remove(ctx, tpdf[i].name);
        lydict_remove(ctx, tpdf[i].dsc);
        lydict_remove(ctx, tpdf[i].ref);

        yang_type_free(ctx, &tpdf[i].type);

        lydict_remove(ctx, tpdf[i].units);
        lydict_remove(ctx, tpdf[i].dflt);
        lys_extension_instances_free(ctx, tpdf[i].ext, tpdf[i].ext_size, NULL);
    }
}

static void
yang_free_import(struct ly_ctx *ctx, struct lys_import *imp, uint8_t start, uint8_t size)
{
    uint8_t i;

    for (i = start; i < size; ++i){
        free((char *)imp[i].module);
        lydict_remove(ctx, imp[i].prefix);
        lydict_remove(ctx, imp[i].dsc);
        lydict_remove(ctx, imp[i].ref);
        lys_extension_instances_free(ctx, imp[i].ext, imp[i].ext_size, NULL);
    }
}

static void
yang_free_include(struct ly_ctx *ctx, struct lys_include *inc, uint8_t start, uint8_t size)
{
    uint8_t i;

    for (i = start; i < size; ++i){
        free((char *)inc[i].submodule);
        lydict_remove(ctx, inc[i].dsc);
        lydict_remove(ctx, inc[i].ref);
        lys_extension_instances_free(ctx, inc[i].ext, inc[i].ext_size, NULL);
    }
}

static void
yang_free_ident_base(struct lys_ident *ident, uint32_t start, uint32_t size)
{
    uint32_t i;
    uint8_t j;

    /* free base name */
    for (i = start; i < size; ++i) {
        for (j = 0; j < ident[i].base_size; ++j) {
            free(ident[i].base[j]);
        }
    }
}

static void
yang_free_grouping(struct ly_ctx *ctx, struct lys_node_grp * grp)
{
    yang_tpdf_free(ctx, grp->tpdf, 0, grp->tpdf_size);
    free(grp->tpdf);
}

static void
yang_free_container(struct ly_ctx *ctx, struct lys_node_container * cont)
{
    uint8_t i;

    yang_tpdf_free(ctx, cont->tpdf, 0, cont->tpdf_size);
    free(cont->tpdf);
    lydict_remove(ctx, cont->presence);

    for (i = 0; i < cont->must_size; ++i) {
        lys_restr_free(ctx, &cont->must[i], NULL);
    }
    free(cont->must);

    lys_when_free(ctx, cont->when, NULL);
}

static void
yang_free_leaf(struct ly_ctx *ctx, struct lys_node_leaf *leaf)
{
    uint8_t i;

    for (i = 0; i < leaf->must_size; i++) {
        lys_restr_free(ctx, &leaf->must[i], NULL);
    }
    free(leaf->must);

    lys_when_free(ctx, leaf->when, NULL);

    yang_type_free(ctx, &leaf->type);
    lydict_remove(ctx, leaf->units);
    lydict_remove(ctx, leaf->dflt);
}

static void
yang_free_leaflist(struct ly_ctx *ctx, struct lys_node_leaflist *leaflist)
{
    uint8_t i;

    for (i = 0; i < leaflist->must_size; i++) {
        lys_restr_free(ctx, &leaflist->must[i], NULL);
    }
    free(leaflist->must);

    for (i = 0; i < leaflist->dflt_size; i++) {
        lydict_remove(ctx, leaflist->dflt[i]);
    }
    free(leaflist->dflt);

    lys_when_free(ctx, leaflist->when, NULL);

    yang_type_free(ctx, &leaflist->type);
    lydict_remove(ctx, leaflist->units);
}

static void
yang_free_list(struct ly_ctx *ctx, struct lys_node_list *list)
{
    uint8_t i;

    yang_tpdf_free(ctx, list->tpdf, 0, list->tpdf_size);
    free(list->tpdf);

    for (i = 0; i < list->must_size; ++i) {
        lys_restr_free(ctx, &list->must[i], NULL);
    }
    free(list->must);

    lys_when_free(ctx, list->when, NULL);

    for (i = 0; i < list->unique_size; ++i) {
        free(list->unique[i].expr);
    }
    free(list->unique);

    free(list->keys);
}

static void
yang_free_choice(struct ly_ctx *ctx, struct lys_node_choice *choice)
{
    free(choice->dflt);
    lys_when_free(ctx, choice->when, NULL);
}

static void
yang_free_anydata(struct ly_ctx *ctx, struct lys_node_anydata *anydata)
{
    uint8_t i;

    for (i = 0; i < anydata->must_size; ++i) {
        lys_restr_free(ctx, &anydata->must[i], NULL);
    }
    free(anydata->must);

    lys_when_free(ctx, anydata->when, NULL);
}

static void
yang_free_inout(struct ly_ctx *ctx, struct lys_node_inout *inout)
{
    uint8_t i;

    yang_tpdf_free(ctx, inout->tpdf, 0, inout->tpdf_size);
    free(inout->tpdf);

    for (i = 0; i < inout->must_size; ++i) {
        lys_restr_free(ctx, &inout->must[i], NULL);
    }
    free(inout->must);
}

static void
yang_free_notif(struct ly_ctx *ctx, struct lys_node_notif *notif)
{
    uint8_t i;

    yang_tpdf_free(ctx, notif->tpdf, 0, notif->tpdf_size);
    free(notif->tpdf);

    for (i = 0; i < notif->must_size; ++i) {
        lys_restr_free(ctx, &notif->must[i], NULL);
    }
    free(notif->must);
}

static void
yang_free_uses(struct ly_ctx *ctx, struct lys_node_uses *uses)
{
    int i, j;

    for (i = 0; i < uses->refine_size; i++) {
        lydict_remove(ctx, uses->refine[i].target_name);
        lydict_remove(ctx, uses->refine[i].dsc);
        lydict_remove(ctx, uses->refine[i].ref);

        for (j = 0; j < uses->refine[i].must_size; j++) {
            lys_restr_free(ctx, &uses->refine[i].must[j], NULL);
        }
        free(uses->refine[i].must);

        for (j = 0; j < uses->refine[i].dflt_size; j++) {
            lydict_remove(ctx, uses->refine[i].dflt[j]);
        }
        free(uses->refine[i].dflt);

        if (uses->refine[i].target_type & LYS_CONTAINER) {
            lydict_remove(ctx, uses->refine[i].mod.presence);
        }
        lys_extension_instances_free(ctx, uses->refine[i].ext, uses->refine[i].ext_size, NULL);
    }
    free(uses->refine);

    lys_when_free(ctx, uses->when, NULL);
}

static void
yang_free_nodes(struct ly_ctx *ctx, struct lys_node *node)
{
    struct lys_node *tmp, *child, *sibling;

    if (!node) {
        return;
    }
    tmp = node;

    while (tmp) {
        child = tmp->child;
        sibling = tmp->next;
        /* common part */
        lydict_remove(ctx, tmp->name);
        if (!(tmp->nodetype & (LYS_INPUT | LYS_OUTPUT))) {
            lys_iffeature_free(ctx, tmp->iffeature, tmp->iffeature_size, 0, NULL);
            lydict_remove(ctx, tmp->dsc);
            lydict_remove(ctx, tmp->ref);
        }

        switch (tmp->nodetype) {
        case LYS_GROUPING:
        case LYS_RPC:
        case LYS_ACTION:
            yang_free_grouping(ctx, (struct lys_node_grp *)tmp);
            break;
        case LYS_CONTAINER:
            yang_free_container(ctx, (struct lys_node_container *)tmp);
            break;
        case LYS_LEAF:
            yang_free_leaf(ctx, (struct lys_node_leaf *)tmp);
            break;
        case LYS_LEAFLIST:
            yang_free_leaflist(ctx, (struct lys_node_leaflist *)tmp);
            break;
        case LYS_LIST:
            yang_free_list(ctx, (struct lys_node_list *)tmp);
            break;
        case LYS_CHOICE:
            yang_free_choice(ctx, (struct lys_node_choice *)tmp);
            break;
        case LYS_CASE:
            lys_when_free(ctx, ((struct lys_node_case *)tmp)->when, NULL);
            break;
        case LYS_ANYXML:
        case LYS_ANYDATA:
            yang_free_anydata(ctx, (struct lys_node_anydata *)tmp);
            break;
        case LYS_INPUT:
        case LYS_OUTPUT:
            yang_free_inout(ctx, (struct lys_node_inout *)tmp);
            break;
        case LYS_NOTIF:
            yang_free_notif(ctx, (struct lys_node_notif *)tmp);
            break;
        case LYS_USES:
            yang_free_uses(ctx, (struct lys_node_uses *)tmp);
            break;
        default:
            break;
        }
        lys_extension_instances_free(ctx, tmp->ext, tmp->ext_size, NULL);
        yang_free_nodes(ctx, child);
        free(tmp);
        tmp = sibling;
    }
}

static void
yang_free_augment(struct ly_ctx *ctx, struct lys_node_augment *aug)
{
    lydict_remove(ctx, aug->target_name);
    lydict_remove(ctx, aug->dsc);
    lydict_remove(ctx, aug->ref);

    lys_iffeature_free(ctx, aug->iffeature, aug->iffeature_size, 0, NULL);
    lys_when_free(ctx, aug->when, NULL);
    yang_free_nodes(ctx, aug->child);
    lys_extension_instances_free(ctx, aug->ext, aug->ext_size, NULL);
}

static void
yang_free_deviate(struct ly_ctx *ctx, struct lys_deviation *dev, uint index)
{
    uint i, j;

    for (i = index; i < dev->deviate_size; ++i) {
        lydict_remove(ctx, dev->deviate[i].units);

        if (dev->deviate[i].type) {
            yang_type_free(ctx, dev->deviate[i].type);
            free(dev->deviate[i].type);
        }

        for (j = 0; j < dev->deviate[i].dflt_size; ++j) {
            lydict_remove(ctx, dev->deviate[i].dflt[j]);
        }
        free(dev->deviate[i].dflt);

        for (j = 0; j < dev->deviate[i].must_size; ++j) {
            lys_restr_free(ctx, &dev->deviate[i].must[j], NULL);
        }
        free(dev->deviate[i].must);

        for (j = 0; j < dev->deviate[i].unique_size; ++j) {
            free(dev->deviate[i].unique[j].expr);
        }
        free(dev->deviate[i].unique);
        lys_extension_instances_free(ctx, dev->deviate[i].ext, dev->deviate[i].ext_size, NULL);
    }
}

void
yang_free_ext_data(struct yang_ext_substmt *substmt)
{
    int i;

    if (!substmt) {
        return;
    }

    free(substmt->ext_substmt);
    if (substmt->ext_modules) {
        for (i = 0; substmt->ext_modules[i]; ++i) {
            free(substmt->ext_modules[i]);
        }
        free(substmt->ext_modules);
    }
    free(substmt);
}

/* free common item from module and submodule */
static void
free_yang_common(struct lys_module *module, struct lys_node *node)
{
    uint i;
    yang_tpdf_free(module->ctx, module->tpdf, 0, module->tpdf_size);
    module->tpdf_size = 0;
    yang_free_ident_base(module->ident, 0, module->ident_size);
    yang_free_nodes(module->ctx, node);
    for (i = 0; i < module->augment_size; ++i) {
        yang_free_augment(module->ctx, &module->augment[i]);
    }
    module->augment_size = 0;
    for (i = 0; i < module->deviation_size; ++i) {
        yang_free_deviate(module->ctx, &module->deviation[i], 0);
        free(module->deviation[i].deviate);
        lydict_remove(module->ctx, module->deviation[i].target_name);
        lydict_remove(module->ctx, module->deviation[i].dsc);
        lydict_remove(module->ctx, module->deviation[i].ref);
    }
    module->deviation_size = 0;
}

/* check function*/

int
yang_check_ext_instance(struct lys_module *module, struct lys_ext_instance ***ext, uint size,
                        void *parent, struct unres_schema *unres)
{
    struct unres_ext *info;
    uint i;

    for (i = 0; i < size; ++i) {
        info = malloc(sizeof *info);
        LY_CHECK_ERR_RETURN(!info, LOGMEM(module->ctx), EXIT_FAILURE);
        info->data.yang = (*ext)[i]->parent;
        info->datatype = LYS_IN_YANG;
        info->parent = parent;
        info->mod = module;
        info->parent_type = (*ext)[i]->parent_type;
        info->substmt = (*ext)[i]->insubstmt;
        info->substmt_index = (*ext)[i]->insubstmt_index;
        info->ext_index = i;
        if (unres_schema_add_node(module, unres, ext, UNRES_EXT, (struct lys_node *)info) == -1) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int
yang_check_imports(struct lys_module *module, struct unres_schema *unres)
{
    struct lys_import *imp;
    struct lys_include *inc;
    uint8_t imp_size, inc_size, j = 0, i = 0;
    char *s;

    imp = module->imp;
    imp_size = module->imp_size;
    inc = module->inc;
    inc_size = module->inc_size;

    if (imp_size) {
        module->imp = calloc(imp_size, sizeof *module->imp);
        module->imp_size = 0;
        LY_CHECK_ERR_GOTO(!module->imp, LOGMEM(module->ctx), error);
    }

    if (inc_size) {
        module->inc = calloc(inc_size, sizeof *module->inc);
        module->inc_size = 0;
        LY_CHECK_ERR_GOTO(!module->inc, LOGMEM(module->ctx), error);
    }

    for (i = 0; i < imp_size; ++i) {
        s = (char *) imp[i].module;
        imp[i].module = NULL;
        if (yang_fill_import(module, &imp[i], &module->imp[module->imp_size], s, unres)) {
            ++i;
            goto error;
        }
    }
    for (j = 0; j < inc_size; ++j) {
        s = (char *) inc[j].submodule;
        inc[j].submodule = NULL;
        if (yang_fill_include(module, s, &inc[j], unres)) {
            ++j;
            goto error;
        }
    }
    free(inc);
    free(imp);

    return EXIT_SUCCESS;

error:
    yang_free_import(module->ctx, imp, i, imp_size);
    yang_free_include(module->ctx, inc, j, inc_size);
    free(imp);
    free(inc);
    return EXIT_FAILURE;
}

static int
yang_check_iffeatures(struct lys_module *module, void *ptr, void *parent, enum yytokentype type, struct unres_schema *unres)
{
    struct lys_iffeature *iffeature;
    uint8_t *ptr_size, size, i;
    char *s;
    int parent_is_feature = 0;

    switch (type) {
    case FEATURE_KEYWORD:
        iffeature = ((struct lys_feature *)parent)->iffeature;
        size = ((struct lys_feature *)parent)->iffeature_size;
        ptr_size = &((struct lys_feature *)parent)->iffeature_size;
        parent_is_feature = 1;
        break;
    case IDENTITY_KEYWORD:
        iffeature = ((struct lys_ident *)parent)->iffeature;
        size = ((struct lys_ident *)parent)->iffeature_size;
        ptr_size = &((struct lys_ident *)parent)->iffeature_size;
        break;
    case ENUM_KEYWORD:
        iffeature = ((struct lys_type_enum *)ptr)->iffeature;
        size = ((struct lys_type_enum *)ptr)->iffeature_size;
        ptr_size = &((struct lys_type_enum *)ptr)->iffeature_size;
        break;
    case BIT_KEYWORD:
        iffeature = ((struct lys_type_bit *)ptr)->iffeature;
        size = ((struct lys_type_bit *)ptr)->iffeature_size;
        ptr_size = &((struct lys_type_bit *)ptr)->iffeature_size;
        break;
    case REFINE_KEYWORD:
        iffeature = ((struct lys_refine *)ptr)->iffeature;
        size = ((struct lys_refine *)ptr)->iffeature_size;
        ptr_size = &((struct lys_refine *)ptr)->iffeature_size;
        break;
    default:
        iffeature = ((struct lys_node *)parent)->iffeature;
        size = ((struct lys_node *)parent)->iffeature_size;
        ptr_size = &((struct lys_node *)parent)->iffeature_size;
        break;
    }

    *ptr_size = 0;
    for (i = 0; i < size; ++i) {
        s = (char *)iffeature[i].features;
        iffeature[i].features = NULL;
        if (yang_fill_iffeature(module, &iffeature[i], parent, s, unres, parent_is_feature)) {
            *ptr_size = size;
            return EXIT_FAILURE;
        }
        if (yang_check_ext_instance(module, &iffeature[i].ext, iffeature[i].ext_size, &iffeature[i], unres)) {
            *ptr_size = size;
            return EXIT_FAILURE;
        }
        (*ptr_size)++;
    }

    return EXIT_SUCCESS;
}

static int
yang_check_identityref(struct lys_module *module, struct lys_type *type, struct unres_schema *unres)
{
    uint size, i;
    int rc;
    struct lys_ident **ref;
    const char *value;
    char *expr;

    ref = type->info.ident.ref;
    size = type->info.ident.count;
    type->info.ident.count = 0;
    type->info.ident.ref = NULL;
    ((struct yang_type *)type->der)->flags |= LYS_NO_ERASE_IDENTITY;

    for (i = 0; i < size; ++i) {
        expr = (char *)ref[i];
        /* store in the JSON format */
        value = transform_schema2json(module, expr);
        free(expr);

        if (!value) {
            goto error;
        }
        rc = unres_schema_add_str(module, unres, type, UNRES_TYPE_IDENTREF, value);
        lydict_remove(module->ctx, value);

        if (rc == -1) {
            goto error;
        }
    }
    free(ref);

    return EXIT_SUCCESS;
error:
    for (i = i+1; i < size; ++i) {
        free(ref[i]);
    }
    free(ref);
    return EXIT_FAILURE;
}

int
yang_fill_type(struct lys_module *module, struct lys_type *type, struct yang_type *stype,
               void *parent, struct unres_schema *unres)
{
    unsigned int i, j;

    type->parent = parent;
    if (yang_check_ext_instance(module, &type->ext, type->ext_size, type, unres)) {
        return EXIT_FAILURE;
    }
    for (j = 0; j < type->ext_size; ++j) {
        if (type->ext[j]->flags & LYEXT_OPT_VALID) {
            type->parent->flags |= LYS_VALID_EXT;
            break;
        }
    }

    switch (stype->base) {
    case LY_TYPE_ENUM:
        for (i = 0; i < type->info.enums.count; ++i) {
            if (yang_check_iffeatures(module, &type->info.enums.enm[i], parent, ENUM_KEYWORD, unres)) {
                return EXIT_FAILURE;
            }
            if (yang_check_ext_instance(module, &type->info.enums.enm[i].ext, type->info.enums.enm[i].ext_size,
                                        &type->info.enums.enm[i], unres)) {
                return EXIT_FAILURE;
            }
            for (j = 0; j < type->info.enums.enm[i].ext_size; ++j) {
                if (type->info.enums.enm[i].ext[j]->flags & LYEXT_OPT_VALID) {
                    type->parent->flags |= LYS_VALID_EXT;
                    break;
                }
            }
        }
        break;
    case LY_TYPE_BITS:
        for (i = 0; i < type->info.bits.count; ++i) {
            if (yang_check_iffeatures(module, &type->info.bits.bit[i], parent, BIT_KEYWORD, unres)) {
                return EXIT_FAILURE;
            }
            if (yang_check_ext_instance(module, &type->info.bits.bit[i].ext, type->info.bits.bit[i].ext_size,
                                        &type->info.bits.bit[i], unres)) {
                return EXIT_FAILURE;
            }
            for (j = 0; j < type->info.bits.bit[i].ext_size; ++j) {
                if (type->info.bits.bit[i].ext[j]->flags & LYEXT_OPT_VALID) {
                    type->parent->flags |= LYS_VALID_EXT;
                    break;
                }
            }
        }
        break;
    case LY_TYPE_IDENT:
        if (yang_check_identityref(module, type, unres)) {
            return EXIT_FAILURE;
        }
        break;
    case LY_TYPE_STRING:
        if (type->info.str.length) {
            if (yang_check_ext_instance(module, &type->info.str.length->ext,
                                        type->info.str.length->ext_size, type->info.str.length, unres)) {
                return EXIT_FAILURE;
            }
            for (j = 0; j < type->info.str.length->ext_size; ++j) {
                if (type->info.str.length->ext[j]->flags & LYEXT_OPT_VALID) {
                    type->parent->flags |= LYS_VALID_EXT;
                    break;
                }
            }
        }

        for (i = 0; i < type->info.str.pat_count; ++i) {
            if (yang_check_ext_instance(module, &type->info.str.patterns[i].ext, type->info.str.patterns[i].ext_size,
                                        &type->info.str.patterns[i], unres)) {
                return EXIT_FAILURE;
            }
            for (j = 0; j < type->info.str.patterns[i].ext_size; ++j) {
                if (type->info.str.patterns[i].ext[j]->flags & LYEXT_OPT_VALID) {
                    type->parent->flags |= LYS_VALID_EXT;
                    break;
                }
            }
        }
        break;
    case LY_TYPE_DEC64:
        if (type->info.dec64.range) {
            if (yang_check_ext_instance(module, &type->info.dec64.range->ext,
                                        type->info.dec64.range->ext_size, type->info.dec64.range, unres)) {
                return EXIT_FAILURE;
            }
            for (j = 0; j < type->info.dec64.range->ext_size; ++j) {
                if (type->info.dec64.range->ext[j]->flags & LYEXT_OPT_VALID) {
                    type->parent->flags |= LYS_VALID_EXT;
                    break;
                }
            }
        }
        break;
    case LY_TYPE_UNION:
        for (i = 0; i < type->info.uni.count; ++i) {
            if (yang_fill_type(module, &type->info.uni.types[i], (struct yang_type *)type->info.uni.types[i].der,
                               parent, unres)) {
                return EXIT_FAILURE;
            }
        }
        break;
    default:
        /* nothing checks */
        break;
    }
    return EXIT_SUCCESS;
}

int
yang_check_typedef(struct lys_module *module, struct lys_node *parent, struct unres_schema *unres)
{
    struct lys_tpdf *tpdf;
    uint8_t *ptr_tpdf_size = NULL;
    uint16_t j, i, tpdf_size, *ptr_tpdf_size16 = NULL;

    if (!parent) {
        tpdf = module->tpdf;
        //ptr_tpdf_size = &module->tpdf_size;
        ptr_tpdf_size16 = &module->tpdf_size;
    } else {
        switch (parent->nodetype) {
        case LYS_GROUPING:
            tpdf = ((struct lys_node_grp *)parent)->tpdf;
            ptr_tpdf_size16 = &((struct lys_node_grp *)parent)->tpdf_size;
            break;
        case LYS_CONTAINER:
            tpdf = ((struct lys_node_container *)parent)->tpdf;
            ptr_tpdf_size16 = &((struct lys_node_container *)parent)->tpdf_size;
            break;
        case LYS_LIST:
            tpdf = ((struct lys_node_list *)parent)->tpdf;
            ptr_tpdf_size = &((struct lys_node_list *)parent)->tpdf_size;
            break;
        case LYS_RPC:
        case LYS_ACTION:
            tpdf = ((struct lys_node_rpc_action *)parent)->tpdf;
            ptr_tpdf_size16 = &((struct lys_node_rpc_action *)parent)->tpdf_size;
            break;
        case LYS_INPUT:
        case LYS_OUTPUT:
            tpdf = ((struct lys_node_inout *)parent)->tpdf;
            ptr_tpdf_size16 = &((struct lys_node_inout *)parent)->tpdf_size;
            break;
        case LYS_NOTIF:
            tpdf = ((struct lys_node_notif *)parent)->tpdf;
            ptr_tpdf_size16 = &((struct lys_node_notif *)parent)->tpdf_size;
            break;
        default:
            LOGINT(module->ctx);
            return EXIT_FAILURE;
        }
    }

    if (ptr_tpdf_size16) {
        tpdf_size = *ptr_tpdf_size16;
        *ptr_tpdf_size16 = 0;
    } else {
        tpdf_size = *ptr_tpdf_size;
        *ptr_tpdf_size = 0;
    }

    for (i = 0; i < tpdf_size; ++i) {
        if (lyp_check_identifier(module->ctx, tpdf[i].name, LY_IDENT_TYPE, module, parent)) {
            goto error;
        }

        if (yang_fill_type(module, &tpdf[i].type, (struct yang_type *)tpdf[i].type.der, &tpdf[i], unres)) {
            goto error;
        }
        if (yang_check_ext_instance(module, &tpdf[i].ext, tpdf[i].ext_size, &tpdf[i], unres)) {
            goto error;
        }
        for (j = 0; j < tpdf[i].ext_size; ++j) {
            if (tpdf[i].ext[j]->flags & LYEXT_OPT_VALID) {
                tpdf[i].flags |= LYS_VALID_EXT;
                break;
            }
        }
        if (unres_schema_add_node(module, unres, &tpdf[i].type, UNRES_TYPE_DER_TPDF, parent) == -1) {
            goto error;
        }

        if (ptr_tpdf_size16) {
            (*ptr_tpdf_size16)++;
        } else {
            (*ptr_tpdf_size)++;
        }
        /* check default value*/
        if (!(module->ctx->models.flags & LY_CTX_TRUSTED)
                && unres_schema_add_node(module, unres, &tpdf[i].type, UNRES_TYPEDEF_DFLT, (struct lys_node *)(&tpdf[i].dflt)) == -1)  {
            ++i;
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    yang_tpdf_free(module->ctx, tpdf, i, tpdf_size);
    return EXIT_FAILURE;
}

static int
yang_check_identities(struct lys_module *module, struct unres_schema *unres)
{
    uint32_t i, size, base_size;
    uint8_t j;

    size = module->ident_size;
    module->ident_size = 0;
    for (i = 0; i < size; ++i) {
        base_size = module->ident[i].base_size;
        module->ident[i].base_size = 0;
        for (j = 0; j < base_size; ++j) {
            if (yang_read_base(module, &module->ident[i], (char *)module->ident[i].base[j], unres)) {
                ++j;
                module->ident_size = size;
                goto error;
            }
        }
        module->ident_size++;
        if (yang_check_iffeatures(module, NULL, &module->ident[i], IDENTITY_KEYWORD, unres)) {
            goto error;
        }
        if (yang_check_ext_instance(module, &module->ident[i].ext, module->ident[i].ext_size, &module->ident[i], unres)) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    for (; j< module->ident[i].base_size; ++j) {
        free(module->ident[i].base[j]);
    }
    yang_free_ident_base(module->ident, i + 1, size);
    return EXIT_FAILURE;
}

static int
yang_check_must(struct lys_module *module, struct lys_restr *must, uint size, struct unres_schema *unres)
{
    uint i;

    for (i = 0; i < size; ++i) {
        if (yang_check_ext_instance(module, &must[i].ext, must[i].ext_size, &must[i], unres)) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

static int
yang_check_container(struct lys_module *module, struct lys_node_container *cont, struct lys_node **child,
                     int options, struct unres_schema *unres)
{
    if (yang_check_typedef(module, (struct lys_node *)cont, unres)) {
        goto error;
    }

    if (yang_check_iffeatures(module, NULL, cont, CONTAINER_KEYWORD, unres)) {
        goto error;
    }

    if (yang_check_nodes(module, (struct lys_node *)cont, *child, options, unres)) {
        *child = NULL;
        goto error;
    }
    *child = NULL;

    if (cont->when && yang_check_ext_instance(module, &cont->when->ext, cont->when->ext_size, cont->when, unres)) {
        goto error;
    }
    if (yang_check_must(module, cont->must, cont->must_size, unres)) {
        goto error;
    }

    /* check XPath dependencies */
    if (!(module->ctx->models.flags & LY_CTX_TRUSTED) && (cont->when || cont->must_size)) {
        if (options & LYS_PARSE_OPT_INGRP) {
            if (lyxp_node_check_syntax((struct lys_node *)cont)) {
                goto error;
            }
        } else {
            if (unres_schema_add_node(module, unres, cont, UNRES_XPATH, NULL) == -1) {
                goto error;
            }
        }
    }

    return EXIT_SUCCESS;
error:

    return EXIT_FAILURE;
}

static int
yang_check_leaf(struct lys_module *module, struct lys_node_leaf *leaf, int options, struct unres_schema *unres)
{
    if (yang_fill_type(module, &leaf->type, (struct yang_type *)leaf->type.der, leaf, unres)) {
        yang_type_free(module->ctx, &leaf->type);
        goto error;
    }
    if (yang_check_iffeatures(module, NULL, leaf, LEAF_KEYWORD, unres)) {
        yang_type_free(module->ctx, &leaf->type);
        goto error;
    }

    if (unres_schema_add_node(module, unres, &leaf->type, UNRES_TYPE_DER, (struct lys_node *)leaf) == -1) {
        yang_type_free(module->ctx, &leaf->type);
        goto error;
    }

    if (!(module->ctx->models.flags & LY_CTX_TRUSTED) &&
            (unres_schema_add_node(module, unres, &leaf->type, UNRES_TYPE_DFLT, (struct lys_node *)&leaf->dflt) == -1)) {
        goto error;
    }

    if (leaf->when && yang_check_ext_instance(module, &leaf->when->ext, leaf->when->ext_size, leaf->when, unres)) {
        goto error;
    }
    if (yang_check_must(module, leaf->must, leaf->must_size, unres)) {
        goto error;
    }

    /* check XPath dependencies */
    if (!(module->ctx->models.flags & LY_CTX_TRUSTED) && (leaf->when || leaf->must_size)) {
        if (options & LYS_PARSE_OPT_INGRP) {
            if (lyxp_node_check_syntax((struct lys_node *)leaf)) {
                goto error;
            }
        } else {
            if (unres_schema_add_node(module, unres, leaf, UNRES_XPATH, NULL) == -1) {
                goto error;
            }
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

static int
yang_check_leaflist(struct lys_module *module, struct lys_node_leaflist *leaflist, int options,
                    struct unres_schema *unres)
{
    int i, j;

    if (yang_fill_type(module, &leaflist->type, (struct yang_type *)leaflist->type.der, leaflist, unres)) {
        yang_type_free(module->ctx, &leaflist->type);
        goto error;
    }
    if (yang_check_iffeatures(module, NULL, leaflist, LEAF_LIST_KEYWORD, unres)) {
        yang_type_free(module->ctx, &leaflist->type);
        goto error;
    }

    if (unres_schema_add_node(module, unres, &leaflist->type, UNRES_TYPE_DER, (struct lys_node *)leaflist) == -1) {
        yang_type_free(module->ctx, &leaflist->type);
        goto error;
    }

    for (i = 0; i < leaflist->dflt_size; ++i) {
        /* check for duplicity in case of configuration data,
         * in case of status data duplicities are allowed */
        if (leaflist->flags & LYS_CONFIG_W) {
            for (j = i +1; j < leaflist->dflt_size; ++j) {
                if (ly_strequal(leaflist->dflt[i], leaflist->dflt[j], 1)) {
                    LOGVAL(module->ctx, LYE_INARG, LY_VLOG_LYS, leaflist, leaflist->dflt[i], "default");
                    LOGVAL(module->ctx, LYE_SPEC, LY_VLOG_LYS, leaflist, "Duplicated default value \"%s\".", leaflist->dflt[i]);
                    goto error;
                }
            }
        }
        /* check default value (if not defined, there still could be some restrictions
         * that need to be checked against a default value from a derived type) */
        if (!(module->ctx->models.flags & LY_CTX_TRUSTED) &&
                (unres_schema_add_node(module, unres, &leaflist->type, UNRES_TYPE_DFLT,
                                       (struct lys_node *)(&leaflist->dflt[i])) == -1)) {
            goto error;
        }
    }

    if (leaflist->when && yang_check_ext_instance(module, &leaflist->when->ext, leaflist->when->ext_size, leaflist->when, unres)) {
        goto error;
    }
    if (yang_check_must(module, leaflist->must, leaflist->must_size, unres)) {
        goto error;
    }

    /* check XPath dependencies */
    if (!(module->ctx->models.flags & LY_CTX_TRUSTED) && (leaflist->when || leaflist->must_size)) {
        if (options & LYS_PARSE_OPT_INGRP) {
            if (lyxp_node_check_syntax((struct lys_node *)leaflist)) {
                goto error;
            }
        } else {
            if (unres_schema_add_node(module, unres, leaflist, UNRES_XPATH, NULL) == -1) {
                goto error;
            }
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

static int
yang_check_list(struct lys_module *module, struct lys_node_list *list, struct lys_node **child,
                int options, struct unres_schema *unres)
{
    struct lys_node *node;

    if (yang_check_typedef(module, (struct lys_node *)list, unres)) {
        goto error;
    }

    if (yang_check_iffeatures(module, NULL, list, LIST_KEYWORD, unres)) {
        goto error;
    }

    if (list->flags & LYS_CONFIG_R) {
        /* RFC 6020, 7.7.5 - ignore ordering when the list represents state data
         * ignore oredering MASK - 0x7F
         */
        list->flags &= 0x7F;
    }
    /* check - if list is configuration, key statement is mandatory
     * (but only if we are not in a grouping or augment, then the check is deferred) */
    for (node = (struct lys_node *)list; node && !(node->nodetype & (LYS_GROUPING | LYS_AUGMENT | LYS_EXT)); node = node->parent);
    if (!node && (list->flags & LYS_CONFIG_W) && !list->keys) {
        LOGVAL(module->ctx, LYE_MISSCHILDSTMT, LY_VLOG_LYS, list, "key", "list");
        goto error;
    }

    if (yang_check_nodes(module, (struct lys_node *)list, *child, options, unres)) {
        *child = NULL;
        goto error;
    }
    *child = NULL;

    if (list->keys && yang_read_key(module, list, unres)) {
        goto error;
    }

    if (yang_read_unique(module, list, unres)) {
        goto error;
    }

    if (list->when && yang_check_ext_instance(module, &list->when->ext, list->when->ext_size, list->when, unres)) {
        goto error;
    }
    if (yang_check_must(module, list->must, list->must_size, unres)) {
        goto error;
    }

    /* check XPath dependencies */
    if (!(module->ctx->models.flags & LY_CTX_TRUSTED) && (list->when || list->must_size)) {
        if (options & LYS_PARSE_OPT_INGRP) {
            if (lyxp_node_check_syntax((struct lys_node *)list)) {
                goto error;
            }
        } else {
            if (unres_schema_add_node(module, unres, list, UNRES_XPATH, NULL) == -1) {
                goto error;
            }
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

static int
yang_check_choice(struct lys_module *module, struct lys_node_choice *choice, struct lys_node **child,
                  int options, struct unres_schema *unres)
{
    char *value;

    if (yang_check_iffeatures(module, NULL, choice, CHOICE_KEYWORD, unres)) {
        free(choice->dflt);
        choice->dflt = NULL;
        goto error;
    }

    if (yang_check_nodes(module, (struct lys_node *)choice, *child, options, unres)) {
        *child = NULL;
        free(choice->dflt);
        choice->dflt = NULL;
        goto error;
    }
    *child = NULL;

    if (choice->dflt) {
        value = (char *)choice->dflt;
        choice->dflt = NULL;
        if (unres_schema_add_str(module, unres, choice, UNRES_CHOICE_DFLT, value) == -1) {
            free(value);
            goto error;
        }
        free(value);
    }

    if (choice->when && yang_check_ext_instance(module, &choice->when->ext, choice->when->ext_size, choice->when, unres)) {
        goto error;
    }

    /* check XPath dependencies */
    if (!(module->ctx->models.flags & LY_CTX_TRUSTED) && choice->when) {
        if (options & LYS_PARSE_OPT_INGRP) {
            if (lyxp_node_check_syntax((struct lys_node *)choice)) {
                goto error;
            }
        } else {
            if (unres_schema_add_node(module, unres, choice, UNRES_XPATH, NULL) == -1) {
                goto error;
            }
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

static int
yang_check_rpc_action(struct lys_module *module, struct lys_node_rpc_action *rpc, struct lys_node **child,
                      int options, struct unres_schema *unres)
{
    struct lys_node *node;

    if (rpc->nodetype == LYS_ACTION) {
        for (node = rpc->parent; node; node = lys_parent(node)) {
            if ((node->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF))
                    || ((node->nodetype == LYS_LIST) && !((struct lys_node_list *)node)->keys)) {
                LOGVAL(module->ctx, LYE_INPAR, LY_VLOG_LYS, rpc->parent, strnodetype(node->nodetype), "action");
                goto error;
            }
        }
    }
    if (yang_check_typedef(module, (struct lys_node *)rpc, unres)) {
        goto error;
    }

    if (yang_check_iffeatures(module, NULL, rpc, RPC_KEYWORD, unres)) {
        goto error;
    }

    if (yang_check_nodes(module, (struct lys_node *)rpc, *child, options | LYS_PARSE_OPT_CFG_IGNORE, unres)) {
        *child = NULL;
        goto error;
    }
    *child = NULL;

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

static int
yang_check_notif(struct lys_module *module, struct lys_node_notif *notif, struct lys_node **child,
                 int options, struct unres_schema *unres)
{
    if (yang_check_typedef(module, (struct lys_node *)notif, unres)) {
        goto error;
    }

    if (yang_check_iffeatures(module, NULL, notif, NOTIFICATION_KEYWORD, unres)) {
        goto error;
    }

    if (yang_check_nodes(module, (struct lys_node *)notif, *child, options | LYS_PARSE_OPT_CFG_IGNORE, unres)) {
        *child = NULL;
        goto error;
    }
    *child = NULL;

    if (yang_check_must(module, notif->must, notif->must_size, unres)) {
        goto error;
    }

    /* check XPath dependencies */
    if (!(module->ctx->models.flags & LY_CTX_TRUSTED) && notif->must_size) {
        if (options & LYS_PARSE_OPT_INGRP) {
            if (lyxp_node_check_syntax((struct lys_node *)notif)) {
                goto error;
            }
        } else {
            if (unres_schema_add_node(module, unres, notif, UNRES_XPATH, NULL) == -1) {
                goto error;
            }
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

static int
yang_check_augment(struct lys_module *module, struct lys_node_augment *augment, int options, struct unres_schema *unres)
{
    struct lys_node *child;

    child = augment->child;
    augment->child = NULL;

    if (yang_check_iffeatures(module, NULL, augment, AUGMENT_KEYWORD, unres)) {
        yang_free_nodes(module->ctx, child);
        goto error;
    }

    if (yang_check_nodes(module, (struct lys_node *)augment, child, options, unres)) {
        goto error;
    }

    if (yang_check_ext_instance(module, &augment->ext, augment->ext_size, augment, unres)) {
        goto error;
    }

    if (augment->when && yang_check_ext_instance(module, &augment->when->ext, augment->when->ext_size, augment->when, unres)) {
        goto error;
    }

    /* check XPath dependencies */
    if (!(module->ctx->models.flags & LY_CTX_TRUSTED) && augment->when) {
        if (options & LYS_PARSE_OPT_INGRP) {
            if (lyxp_node_check_syntax((struct lys_node *)augment)) {
                goto error;
            }
        } else {
            if (unres_schema_add_node(module, unres, augment, UNRES_XPATH, NULL) == -1) {
                goto error;
            }
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

static int
yang_check_uses(struct lys_module *module, struct lys_node_uses *uses, int options, struct unres_schema *unres)
{
    uint i, size;

    size = uses->augment_size;
    uses->augment_size = 0;

    if (yang_check_iffeatures(module, NULL, uses, USES_KEYWORD, unres)) {
        goto error;
    }

    for (i = 0; i < uses->refine_size; ++i) {
        if (yang_check_iffeatures(module, &uses->refine[i], uses, REFINE_KEYWORD, unres)) {
            goto error;
        }
        if (yang_check_must(module, uses->refine[i].must, uses->refine[i].must_size, unres)) {
            goto error;
        }
        if (yang_check_ext_instance(module, &uses->refine[i].ext, uses->refine[i].ext_size, &uses->refine[i], unres)) {
            goto error;
        }
    }

    for (i = 0; i < size; ++i) {
        uses->augment_size++;
        if (yang_check_augment(module, &uses->augment[i], options, unres)) {
            goto error;
        }
    }

    if (unres_schema_add_node(module, unres, uses, UNRES_USES, NULL) == -1) {
        goto error;
    }

    if (uses->when && yang_check_ext_instance(module, &uses->when->ext, uses->when->ext_size, uses->when, unres)) {
        goto error;
    }

    /* check XPath dependencies */
    if (!(module->ctx->models.flags & LY_CTX_TRUSTED) && uses->when) {
        if (options & LYS_PARSE_OPT_INGRP) {
            if (lyxp_node_check_syntax((struct lys_node *)uses)) {
                goto error;
            }
        } else {
            if (unres_schema_add_node(module, unres, uses, UNRES_XPATH, NULL) == -1) {
                goto error;
            }
        }
    }

    return EXIT_SUCCESS;

error:
    for (i = uses->augment_size; i < size; ++i) {
        yang_free_augment(module->ctx, &uses->augment[i]);
    }
    return EXIT_FAILURE;
}

static int
yang_check_anydata(struct lys_module *module, struct lys_node_anydata *anydata, struct lys_node **child,
                   int options, struct unres_schema *unres)
{
    if (yang_check_iffeatures(module, NULL, anydata, ANYDATA_KEYWORD, unres)) {
        goto error;
    }

    if (yang_check_nodes(module, (struct lys_node *)anydata, *child, options, unres)) {
        *child = NULL;
        goto error;
    }
    *child = NULL;

    if (anydata->when && yang_check_ext_instance(module, &anydata->when->ext, anydata->when->ext_size, anydata->when, unres)) {
        goto error;
    }
    if (yang_check_must(module, anydata->must, anydata->must_size, unres)) {
        goto error;
    }

    /* check XPath dependencies */
    if (!(module->ctx->models.flags & LY_CTX_TRUSTED) && (anydata->when || anydata->must_size)) {
        if (options & LYS_PARSE_OPT_INGRP) {
            if (lyxp_node_check_syntax((struct lys_node *)anydata)) {
                goto error;
            }
        } else {
            if (unres_schema_add_node(module, unres, anydata, UNRES_XPATH, NULL) == -1) {
                goto error;
            }
        }
    }

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

static int
yang_check_nodes(struct lys_module *module, struct lys_node *parent, struct lys_node *nodes,
                 int options, struct unres_schema *unres)
{
    struct lys_node *node = nodes, *sibling, *child;
    int i;

    while (node) {
        sibling = node->next;
        child = node->child;
        node->next = NULL;
        node->child = NULL;
        node->parent = NULL;
        node->prev = node;

        if (lys_node_addchild(parent, module->type ? ((struct lys_submodule *)module)->belongsto: module, node, 0) ||
            check_status_flag(node, parent)) {
            lys_node_unlink(node);
            yang_free_nodes(module->ctx, node);
            goto error;
        }
        if (node->parent != parent) {
            assert(node->parent->parent == parent);
            assert((node->parent->nodetype == LYS_CASE) && (node->parent->flags & LYS_IMPLICIT));
            store_config_flag(node->parent, options);
        }
        store_config_flag(node, options);
        if (yang_check_ext_instance(module, &node->ext, node->ext_size, node, unres)) {
            goto error;
        }
        for (i = 0; i < node->ext_size; ++i) {
            if (node->ext[i]->flags & LYEXT_OPT_VALID) {
                node->flags |= LYS_VALID_EXT;
                break;
            }
        }

        switch (node->nodetype) {
        case LYS_GROUPING:
            if (yang_check_typedef(module, node, unres)) {
                goto error;
            }
            if (yang_check_iffeatures(module, NULL, node, GROUPING_KEYWORD, unres)) {
                goto error;
            }
            if (yang_check_nodes(module, node, child, options | LYS_PARSE_OPT_INGRP, unres)) {
                child = NULL;
                goto error;
            }
            break;
        case LYS_CONTAINER:
            if (yang_check_container(module, (struct lys_node_container *)node, &child, options, unres)) {
                goto error;
            }
            break;
        case LYS_LEAF:
            if (yang_check_leaf(module, (struct lys_node_leaf *)node, options, unres)) {
                child = NULL;
                goto error;
            }
            break;
        case LYS_LEAFLIST:
            if (yang_check_leaflist(module, (struct lys_node_leaflist *)node, options, unres)) {
                child = NULL;
                goto error;
            }
            break;
        case LYS_LIST:
            if (yang_check_list(module, (struct lys_node_list *)node, &child, options, unres)) {
                goto error;
            }
            break;
        case LYS_CHOICE:
            if (yang_check_choice(module, (struct lys_node_choice *)node, &child, options, unres)) {
                goto error;
            }
            break;
        case LYS_CASE:
            if (yang_check_iffeatures(module, NULL, node, CASE_KEYWORD, unres)) {
                goto error;
            }
            if (yang_check_nodes(module, node, child, options, unres)) {
                child = NULL;
                goto error;
            }
            if (((struct lys_node_case *)node)->when) {
                if (yang_check_ext_instance(module, &((struct lys_node_case *)node)->when->ext,
                        ((struct lys_node_case *)node)->when->ext_size, ((struct lys_node_case *)node)->when, unres)) {
                    goto error;
                }
                /* check XPath dependencies */
                if (!(module->ctx->models.flags & LY_CTX_TRUSTED) && (options & LYS_PARSE_OPT_INGRP)) {
                    if (lyxp_node_check_syntax(node)) {
                        goto error;
                    }
                } else {
                    if (unres_schema_add_node(module, unres, node, UNRES_XPATH, NULL) == -1) {
                        goto error;
                    }
                }
            }
            break;
        case LYS_ANYDATA:
        case LYS_ANYXML:
            if (yang_check_anydata(module, (struct lys_node_anydata *)node, &child, options, unres)) {
                goto error;
            }
            break;
        case LYS_RPC:
        case LYS_ACTION:
            if (yang_check_rpc_action(module, (struct lys_node_rpc_action *)node, &child, options, unres)){
                goto error;
            }
            break;
        case LYS_INPUT:
        case LYS_OUTPUT:
            if (yang_check_typedef(module, node, unres)) {
                goto error;
            }
            if (yang_check_nodes(module, node, child, options, unres)) {
                child = NULL;
                goto error;
            }
            if (((struct lys_node_inout *)node)->must_size) {
                if (yang_check_must(module, ((struct lys_node_inout *)node)->must, ((struct lys_node_inout *)node)->must_size, unres)) {
                    goto error;
                }
                /* check XPath dependencies */
                if (!(module->ctx->models.flags & LY_CTX_TRUSTED) && (options & LYS_PARSE_OPT_INGRP)) {
                    if (lyxp_node_check_syntax(node)) {
                        goto error;
                    }
                } else {
                    if (unres_schema_add_node(module, unres, node, UNRES_XPATH, NULL) == -1) {
                        goto error;
                    }
                }
            }
            break;
        case LYS_NOTIF:
            if (yang_check_notif(module, (struct lys_node_notif *)node, &child, options, unres)) {
                goto error;
            }
            break;
        case LYS_USES:
            if (yang_check_uses(module, (struct lys_node_uses *)node, options, unres)) {
                child = NULL;
                goto error;
            }
            break;
        default:
            LOGINT(module->ctx);
            goto error;
        }
        node = sibling;
    }

    return EXIT_SUCCESS;

error:
    yang_free_nodes(module->ctx, sibling);
    yang_free_nodes(module->ctx, child);
    return EXIT_FAILURE;
}

static int
yang_check_deviate(struct lys_module *module, struct unres_schema *unres, struct lys_deviate *deviate,
                   struct lys_node *dev_target, struct ly_set *dflt_check)
{
    struct lys_node_leaflist *llist;
    struct lys_type *type;
    struct lys_tpdf *tmp_parent;
    int i, j;

    if (yang_check_ext_instance(module, &deviate->ext, deviate->ext_size, deviate, unres)) {
        goto error;
    }
    if (deviate->must_size && yang_check_deviate_must(module, unres, deviate, dev_target)) {
        goto error;
    }
    if (deviate->unique && yang_check_deviate_unique(module, deviate, dev_target)) {
        goto error;
    }
    if (deviate->dflt_size) {
        if (yang_read_deviate_default(module, deviate, dev_target, dflt_check)) {
            goto error;
        }
        if (dev_target->nodetype == LYS_LEAFLIST && deviate->mod == LY_DEVIATE_DEL) {
            /* consolidate the final list in the target after removing items from it */
            llist = (struct lys_node_leaflist *)dev_target;
            for (i = j = 0; j < llist->dflt_size; j++) {
                llist->dflt[i] = llist->dflt[j];
                if (llist->dflt[i]) {
                    i++;
                }
            }
            llist->dflt_size = i + 1;
        }
    }

    if (deviate->max_set && yang_read_deviate_minmax(deviate, dev_target, deviate->max, 1)) {
        goto error;
    }

    if (deviate->min_set && yang_read_deviate_minmax(deviate, dev_target, deviate->min, 0)) {
        goto error;
    }

    if (deviate->units && yang_read_deviate_units(module->ctx, deviate, dev_target)) {
        goto error;
    }

    if ((deviate->flags & LYS_CONFIG_MASK)) {
        /* add and replace are the same in this case */
        /* remove current config value of the target ... */
        dev_target->flags &= ~LYS_CONFIG_MASK;

        /* ... and replace it with the value specified in deviation */
        dev_target->flags |= deviate->flags & LYS_CONFIG_MASK;
    }

    if ((deviate->flags & LYS_MAND_MASK) && yang_check_deviate_mandatory(deviate, dev_target)) {
        goto error;
    }

    if (deviate->type) {
        /* check target node type */
        if (dev_target->nodetype == LYS_LEAF) {
            type = &((struct lys_node_leaf *)dev_target)->type;
        } else if (dev_target->nodetype == LYS_LEAFLIST) {
            type = &((struct lys_node_leaflist *)dev_target)->type;
        } else {
            LOGVAL(module->ctx, LYE_INSTMT, LY_VLOG_NONE, NULL, "type");
            LOGVAL(module->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Target node does not allow \"type\" property.");
            goto error;
        }
        /* remove type and initialize it */
        tmp_parent = type->parent;
        lys_type_free(module->ctx, type, NULL);
        memcpy(type, deviate->type, sizeof *deviate->type);
        free(deviate->type);
        deviate->type = type;
        deviate->type->parent = tmp_parent;
        if (yang_fill_type(module, type, (struct yang_type *)type->der, tmp_parent, unres)) {
            goto error;
        }

        if (unres_schema_add_node(module, unres, deviate->type, UNRES_TYPE_DER, dev_target) == -1) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    if (deviate->type) {
        yang_type_free(module->ctx, deviate->type);
        deviate->type = NULL;
    }
    return EXIT_FAILURE;
}

static int
yang_check_deviation(struct lys_module *module, struct unres_schema *unres, struct lys_deviation *dev)
{
    int rc;
    uint i;
    struct lys_node *dev_target = NULL, *parent;
    struct ly_set *dflt_check = ly_set_new(), *set;
    unsigned int u;
    const char *value, *target_name;
    struct lys_node_leaflist *llist;
    struct lys_node_leaf *leaf;
    struct lys_node_inout *inout;
    struct unres_schema tmp_unres;
    struct lys_module *mod;

    /* resolve target node */
    rc = resolve_schema_nodeid(dev->target_name, NULL, module, &set, 0, 1);
    if (rc == -1) {
        LOGVAL(module->ctx, LYE_INARG, LY_VLOG_NONE, NULL, dev->target_name, "deviation");
        ly_set_free(set);
        i = 0;
        goto free_type_error;
    }
    dev_target = set->set.s[0];
    ly_set_free(set);

    if (dev_target->module == lys_main_module(module)) {
        LOGVAL(module->ctx, LYE_INARG, LY_VLOG_NONE, NULL, dev->target_name, "deviation");
        LOGVAL(module->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Deviating own module is not allowed.");
        i = 0;
        goto free_type_error;
    }

    if (!dflt_check) {
        LOGMEM(module->ctx);
        i = 0;
        goto free_type_error;
    }

    if (dev->deviate[0].mod == LY_DEVIATE_NO) {
        /* you cannot remove a key leaf */
        if ((dev_target->nodetype == LYS_LEAF) && dev_target->parent && (dev_target->parent->nodetype == LYS_LIST)) {
            for (i = 0; i < ((struct lys_node_list *)dev_target->parent)->keys_size; ++i) {
                if (((struct lys_node_list *)dev_target->parent)->keys[i] == (struct lys_node_leaf *)dev_target) {
                    LOGVAL(module->ctx, LYE_INARG, LY_VLOG_NONE, NULL, "not-supported", "deviation");
                    LOGVAL(module->ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "\"not-supported\" deviation cannot remove a list key.");
                    i = 0;
                    goto free_type_error;
                }
            }
        }
        /* unlink and store the original node */
        parent = dev_target->parent;
        lys_node_unlink(dev_target);
        if (parent) {
            if (parent->nodetype & (LYS_AUGMENT | LYS_USES)) {
                /* hack for augment, because when the original will be sometime reconnected back, we actually need
                 * to reconnect it to both - the augment and its target (which is deduced from the deviations target
                 * path), so we need to remember the augment as an addition */
                /* remember uses parent so we can reconnect to it */
                dev_target->parent = parent;
            } else if (parent->nodetype & (LYS_RPC | LYS_ACTION)) {
                /* re-create implicit node */
                inout = calloc(1, sizeof *inout);
                LY_CHECK_ERR_GOTO(!inout, LOGMEM(module->ctx), error);

                inout->nodetype = dev_target->nodetype;
                inout->name = lydict_insert(module->ctx, (inout->nodetype == LYS_INPUT) ? "input" : "output", 0);
                inout->module = dev_target->module;
                inout->flags = LYS_IMPLICIT;

                /* insert it manually */
                assert(parent->child && !parent->child->next
                    && (parent->child->nodetype == (inout->nodetype == LYS_INPUT ? LYS_OUTPUT : LYS_INPUT)));
                parent->child->next = (struct lys_node *)inout;
                inout->prev = parent->child;
                parent->child->prev = (struct lys_node *)inout;
                inout->parent = parent;
            }
        }
        dev->orig_node = dev_target;
    } else {
        /* store a shallow copy of the original node */
        memset(&tmp_unres, 0, sizeof tmp_unres);
        dev->orig_node = lys_node_dup(dev_target->module, NULL, dev_target, &tmp_unres, 1);
        /* just to be safe */
        if (tmp_unres.count) {
            LOGINT(module->ctx);
            i = 0;
            goto free_type_error;
        }
    }

    if (yang_check_ext_instance(module, &dev->ext, dev->ext_size, dev, unres)) {
        i = 0;
        goto free_type_error;
    }

    for (i = 0; i < dev->deviate_size; ++i) {
        if (yang_check_deviate(module, unres, &dev->deviate[i], dev_target, dflt_check)) {
            yang_free_deviate(module->ctx, dev, i + 1);
            dev->deviate_size = i + 1;
            goto free_type_error;
        }
    }
    /* now check whether default value, if any, matches the type */
    for (u = 0; u < dflt_check->number; ++u) {
        value = NULL;
        rc = EXIT_SUCCESS;
        if (dflt_check->set.s[u]->nodetype == LYS_LEAF) {
            leaf = (struct lys_node_leaf *)dflt_check->set.s[u];
            target_name = leaf->name;
            value = leaf->dflt;
            rc = unres_schema_add_node(module, unres, &leaf->type, UNRES_TYPE_DFLT, (struct lys_node *)(&leaf->dflt));
        } else { /* LYS_LEAFLIST */
            llist = (struct lys_node_leaflist *)dflt_check->set.s[u];
            target_name = llist->name;
            for (i = 0; i < llist->dflt_size; i++) {
                rc = unres_schema_add_node(module, unres, &llist->type, UNRES_TYPE_DFLT,
                                           (struct lys_node *)(&llist->dflt[i]));
                if (rc == -1) {
                    value = llist->dflt[i];
                    break;
                }
            }
        }
        if (rc == -1) {
            LOGVAL(module->ctx, LYE_INARG, LY_VLOG_NONE, NULL, value, "default");
            LOGVAL(module->ctx, LYE_SPEC, LY_VLOG_NONE, NULL,
                "The default value \"%s\" of the deviated node \"%s\"no longer matches its type.",
                target_name);
            goto error;
        }
    }
    ly_set_free(dflt_check);
    dflt_check = NULL;

    /* mark all the affected modules as deviated and implemented */
    for (parent = dev_target; parent; parent = lys_parent(parent)) {
        mod = lys_node_module(parent);
        if (module != mod) {
            mod->deviated = 1;            /* main module */
            parent->module->deviated = 1; /* possible submodule */
            if (!mod->implemented) {
                mod->implemented = 1;
                if (unres_schema_add_node(mod, unres, NULL, UNRES_MOD_IMPLEMENT, NULL) == -1) {
                    goto error;
                }
            }
        }
    }

    return EXIT_SUCCESS;

free_type_error:
    /* we need to free types because they are for now allocated dynamically (use i as it is now, is set correctly) */
    for (; i < dev->deviate_size; ++i) {
        if (dev->deviate[i].type) {
            yang_type_free(module->ctx, dev->deviate[i].type);
            free(dev->deviate[i].type);
            dev->deviate[i].type = NULL;
        }
    }
error:
    ly_set_free(dflt_check);
    return EXIT_FAILURE;
}

static int
yang_check_sub_module(struct lys_module *module, struct unres_schema *unres, struct lys_node *node)
{
    uint i, erase_identities = 1, erase_nodes = 1, aug_size, dev_size = 0;

    aug_size = module->augment_size;
    module->augment_size = 0;
    dev_size = module->deviation_size;
    module->deviation_size = 0;

    if (yang_check_typedef(module, NULL, unres)) {
        goto error;
    }

    if (yang_check_ext_instance(module, &module->ext, module->ext_size, module, unres)) {
        goto error;
    }

    /* check extension in revision */
    for (i = 0; i < module->rev_size; ++i) {
        if (yang_check_ext_instance(module, &module->rev[i].ext, module->rev[i].ext_size, &module->rev[i], unres)) {
            goto error;
        }
    }

    /* check extension in definition of extension */
    for (i = 0; i < module->extensions_size; ++i) {
        if (yang_check_ext_instance(module, &module->extensions[i].ext, module->extensions[i].ext_size, &module->extensions[i], unres)) {
            goto error;
        }
    }

    /* check features */
    for (i = 0; i < module->features_size; ++i) {
        if (yang_check_iffeatures(module, NULL, &module->features[i], FEATURE_KEYWORD, unres)) {
            goto error;
        }
        if (yang_check_ext_instance(module, &module->features[i].ext, module->features[i].ext_size, &module->features[i], unres)) {
            goto error;
        }

        /* check for circular dependencies */
        if (module->features[i].iffeature_size && (unres_schema_add_node(module, unres, &module->features[i], UNRES_FEATURE, NULL) == -1)) {
            goto error;
        }
    }
    erase_identities = 0;
    if (yang_check_identities(module, unres)) {
        goto error;
    }
    erase_nodes = 0;
    if (yang_check_nodes(module, NULL, node, 0, unres)) {
        goto error;
    }

    /* check deviation */
    for (i = 0; i < dev_size; ++i) {
        module->deviation_size++;
        if (yang_check_deviation(module, unres, &module->deviation[i])) {
            goto error;
        }
    }

    /* check augments */
    for (i = 0; i < aug_size; ++i) {
        module->augment_size++;
        if (yang_check_augment(module, &module->augment[i], 0, unres)) {
            goto error;
        }
        if (unres_schema_add_node(module, unres, &module->augment[i], UNRES_AUGMENT, NULL) == -1) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:
    if (erase_identities) {
        yang_free_ident_base(module->ident, 0, module->ident_size);
    }
    if (erase_nodes) {
        yang_free_nodes(module->ctx, node);
    }
    for (i = module->augment_size; i < aug_size; ++i) {
        yang_free_augment(module->ctx, &module->augment[i]);
    }
    for (i = module->deviation_size; i < dev_size; ++i) {
        yang_free_deviate(module->ctx, &module->deviation[i], 0);
        free(module->deviation[i].deviate);
        lydict_remove(module->ctx, module->deviation[i].target_name);
        lydict_remove(module->ctx, module->deviation[i].dsc);
        lydict_remove(module->ctx, module->deviation[i].ref);
    }
    return EXIT_FAILURE;
}

int
yang_read_extcomplex_str(struct lys_module *module, struct lys_ext_instance_complex *ext, const char *arg_name,
                         const char *parent_name, char *value, int parent_stmt, LY_STMT stmt)
{
    int c;
    const char **str, ***p = NULL;
    void *reallocated;
    struct lyext_substmt *info;

    c = 0;
    if (stmt == LY_STMT_PREFIX && parent_stmt == LY_STMT_BELONGSTO) {
        /* str contains no NULL value */
        str = lys_ext_complex_get_substmt(LY_STMT_BELONGSTO, ext, &info);
        if (info->cardinality < LY_STMT_CARD_SOME) {
            str++;
        } else {
           /* get the index in the array to add new item */
            p = (const char ***)str;
            for (c = 0; p[0][c + 1]; c++);
            str = p[1];
        }
        str[c] = lydict_insert_zc(module->ctx, value);
    }  else {
        str = lys_ext_complex_get_substmt(stmt, ext, &info);
        if (!str) {
            LOGVAL(module->ctx, LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, arg_name, parent_name);
            goto error;
        }
        if (info->cardinality < LY_STMT_CARD_SOME && *str) {
            LOGVAL(module->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, arg_name, parent_name);
            goto error;
        }

        if (info->cardinality >= LY_STMT_CARD_SOME) {
            /* there can be multiple instances, str is actually const char *** */
            p = (const char ***)str;
            if (!p[0]) {
                /* allocate initial array */
                p[0] = calloc(2, sizeof(const char *));
                LY_CHECK_ERR_GOTO(!p[0], LOGMEM(module->ctx), error);
                if (stmt == LY_STMT_BELONGSTO) {
                    /* allocate another array for the belongs-to's prefixes */
                    p[1] = calloc(2, sizeof(const char *));
                    LY_CHECK_ERR_GOTO(!p[1], LOGMEM(module->ctx), error);
                } else if (stmt == LY_STMT_ARGUMENT) {
                    /* allocate another array for the yin element */
                    ((uint8_t **)p)[1] = calloc(2, sizeof(uint8_t));
                    LY_CHECK_ERR_GOTO(!p[1], LOGMEM(module->ctx), error);
                    /* default value of yin element */
                    ((uint8_t *)p[1])[0] = 2;
                }
            } else {
                /* get the index in the array to add new item */
                for (c = 0; p[0][c]; c++);
            }
            str = p[0];
        }

        str[c] = lydict_insert_zc(module->ctx, value);
        value = NULL;

        if (c) {
            /* enlarge the array(s) */
            reallocated = realloc(p[0], (c + 2) * sizeof(const char *));
            if (!reallocated) {
                LOGMEM(module->ctx);
                lydict_remove(module->ctx, p[0][c]);
                p[0][c] = NULL;
                return EXIT_FAILURE;
            }
            p[0] = reallocated;
            p[0][c + 1] = NULL;

            if (stmt == LY_STMT_BELONGSTO) {
                /* enlarge the second belongs-to's array with prefixes */
                reallocated = realloc(p[1], (c + 2) * sizeof(const char *));
                if (!reallocated) {
                    LOGMEM(module->ctx);
                    lydict_remove(module->ctx, p[1][c]);
                    p[1][c] = NULL;
                    return EXIT_FAILURE;
                }
                p[1] = reallocated;
                p[1][c + 1] = NULL;
            } else if (stmt == LY_STMT_ARGUMENT) {
                /* enlarge the second argument's array with yin element */
                reallocated = realloc(p[1], (c + 2) * sizeof(uint8_t));
                if (!reallocated) {
                    LOGMEM(module->ctx);
                    ((uint8_t *)p[1])[c] = 0;
                    return EXIT_FAILURE;
                }
                p[1] = reallocated;
                ((uint8_t *)p[1])[c + 1] = 0;
            }
        }
    }

    return EXIT_SUCCESS;

error:
    free(value);
    return EXIT_FAILURE;
}

static int
yang_fill_ext_substm_index(struct lys_ext_instance_complex *ext, LY_STMT stmt, enum yytokentype keyword)
{
    int c = 0, decrement = 0;
    const char **str, ***p = NULL;
    struct lyext_substmt *info;


    if (keyword == BELONGS_TO_KEYWORD || stmt == LY_STMT_BELONGSTO) {
        stmt = LY_STMT_BELONGSTO;
        decrement = -1;
    } else if (keyword == ARGUMENT_KEYWORD || stmt == LY_STMT_ARGUMENT) {
        stmt = LY_STMT_ARGUMENT;
        decrement = -1;
    }

    str = lys_ext_complex_get_substmt(stmt, ext, &info);
    if (!str || info->cardinality < LY_STMT_CARD_SOME || !((const char ***)str)[0]) {
        return 0;
    } else {
        p = (const char ***)str;
        /* get the index in the array */
        for (c = 0; p[0][c]; c++);
        return c + decrement;
    }
}

void **
yang_getplace_for_extcomplex_struct(struct lys_ext_instance_complex *ext, int *index,
                                    char *parent_name, char *node_name, LY_STMT stmt)
{
    struct ly_ctx *ctx = ext->module->ctx;
    int c;
    void **data, ***p = NULL;
    void *reallocated;
    struct lyext_substmt *info;

    data = lys_ext_complex_get_substmt(stmt, ext, &info);
    if (!data) {
        LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, node_name, parent_name);
        return NULL;
    }
    if (info->cardinality < LY_STMT_CARD_SOME && *data) {
        LOGVAL(ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, node_name, parent_name);
        return NULL;
    }

    c = 0;
    if (info->cardinality >= LY_STMT_CARD_SOME) {
        /* there can be multiple instances, so instead of pointer to array,
         * we have in data pointer to pointer to array */
        p = (void ***)data;
        data = *p;
        if (!data) {
            /* allocate initial array */
            *p = data = calloc(2, sizeof(void *));
            LY_CHECK_ERR_RETURN(!data, LOGMEM(ctx), NULL);
        } else {
            for (c = 0; *data; data++, c++);
        }
    }

    if (c) {
        /* enlarge the array */
        reallocated = realloc(*p, (c + 2) * sizeof(void *));
        LY_CHECK_ERR_RETURN(!reallocated, LOGMEM(ctx), NULL);
        *p = reallocated;
        data = *p;
        data[c + 1] = NULL;
    }

    if (index) {
        *index = c;
        return data;
    } else {
        return &data[c];
    }
}

int
yang_fill_extcomplex_flags(struct lys_ext_instance_complex *ext, char *parent_name, char *node_name,
                           LY_STMT stmt, uint16_t value, uint16_t mask)
{
    uint16_t *data;
    struct lyext_substmt *info;

    data = lys_ext_complex_get_substmt(stmt, ext, &info);
    if (!data) {
        LOGVAL(ext->module->ctx, LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, node_name, parent_name);
        return EXIT_FAILURE;
    }
    if (info->cardinality < LY_STMT_CARD_SOME && (*data & mask)) {
        LOGVAL(ext->module->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, node_name, parent_name);
        return EXIT_FAILURE;
    }

    *data |= value;
    return EXIT_SUCCESS;
}

int
yang_fill_extcomplex_uint8(struct lys_ext_instance_complex *ext, char *parent_name, char *node_name,
                           LY_STMT stmt, uint8_t value)
{
    struct ly_ctx *ctx = ext->module->ctx;
    uint8_t *val, **pp = NULL, *reallocated;
    struct lyext_substmt *info;
    int i = 0;

    val = lys_ext_complex_get_substmt(stmt, ext, &info);
    if (!val) {
        LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, node_name, parent_name);
        return EXIT_FAILURE;
    }
    if (stmt == LY_STMT_DIGITS) {
        if (info->cardinality < LY_STMT_CARD_SOME && *val) {
            LOGVAL(ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, node_name, parent_name);
            return EXIT_FAILURE;
        }

        if (info->cardinality >= LY_STMT_CARD_SOME) {
            /* there can be multiple instances */
            pp = (uint8_t**)val;
            if (!(*pp)) {
                *pp = calloc(2, sizeof(uint8_t)); /* allocate initial array */
                LY_CHECK_ERR_RETURN(!*pp, LOGMEM(ctx), EXIT_FAILURE);
            } else {
                for (i = 0; (*pp)[i]; i++);
            }
            val = &(*pp)[i];
        }

        /* stored value */
        *val = value;

        if (i) {
            /* enlarge the array */
            reallocated = realloc(*pp, (i + 2) * sizeof *val);
            LY_CHECK_ERR_RETURN(!reallocated, LOGMEM(ctx), EXIT_FAILURE);
            *pp = reallocated;
            (*pp)[i + 1] = 0;
        }
    } else {
        if (*val) {
            LOGVAL(ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, node_name, parent_name);
            return EXIT_FAILURE;
        }

        if (stmt == LY_STMT_REQINSTANCE) {
            *val = (value == 1) ? 1 : 2;
        } else if (stmt == LY_STMT_MODIFIER) {
            *val =  1;
        } else {
            LOGINT(ctx);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int
yang_extcomplex_node(struct lys_ext_instance_complex *ext, char *parent_name, char *node_name,
                     struct lys_node *node, LY_STMT stmt)
{
    struct lyext_substmt *info;
    struct lys_node **snode, *siter;

    snode = lys_ext_complex_get_substmt(stmt, ext, &info);
    if (!snode) {
        LOGVAL(ext->module->ctx, LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, node_name, parent_name);
        return EXIT_FAILURE;
    }
    if (info->cardinality < LY_STMT_CARD_SOME) {
        LY_TREE_FOR(node, siter) {
            if (stmt == lys_snode2stmt(siter->nodetype)) {
                LOGVAL(ext->module->ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, node_name, parent_name);
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

int
yang_fill_extcomplex_module(struct ly_ctx *ctx, struct lys_ext_instance_complex *ext,
                            char *parent_name, char **values, int implemented)
{
    int c, i;
    struct lys_module **modules, ***p, *reallocated, **pp;
    struct lyext_substmt *info;

    if (!values) {
        return EXIT_SUCCESS;
    }
    pp = modules = lys_ext_complex_get_substmt(LY_STMT_MODULE, ext, &info);
    if (!modules) {
        LOGVAL(ctx, LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "module", parent_name);
        return EXIT_FAILURE;
    }

    for (i = 0; values[i]; ++i) {
        c = 0;
        if (info->cardinality < LY_STMT_CARD_SOME && *modules) {
            LOGVAL(ctx, LYE_TOOMANY, LY_VLOG_NONE, NULL, "module", parent_name);
            return EXIT_FAILURE;
        }
        if (info->cardinality >= LY_STMT_CARD_SOME) {
            /* there can be multiple instances, so instead of pointer to array,
             * we have in modules pointer to pointer to array */
            p = (struct lys_module ***)pp;
            modules = *p;
            if (!modules) {
                /* allocate initial array */
                *p = modules = calloc(2, sizeof(struct lys_module *));
                LY_CHECK_ERR_RETURN(!*p, LOGMEM(ctx), EXIT_FAILURE);
            } else {
                for (c = 0; *modules; modules++, c++);
            }
        }

        if (c) {
            /* enlarge the array */
            reallocated = realloc(*p, (c + 2) * sizeof(struct lys_module *));
            LY_CHECK_ERR_RETURN(!reallocated, LOGMEM(ctx), EXIT_FAILURE);
            *p = (struct lys_module **)reallocated;
            modules = *p;
            modules[c + 1] = NULL;
        }

        modules[c] = yang_read_module(ctx, values[i], 0, NULL, implemented);
        if (!modules[c]) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
