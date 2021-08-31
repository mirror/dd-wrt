/*
 * @file test_tree_schema_compile.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from parser_yang.c
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include "common.h"
#include "in.h"
#include "parser_internal.h"
#include "path.h"
#include "plugins_types.h"
#include "schema_compile.h"
#include "xpath.h"

static int
setup(void **state)
{
    UTEST_SETUP;

    assert_int_equal(LY_SUCCESS, ly_ctx_set_options(UTEST_LYCTX, LY_CTX_DISABLE_SEARCHDIRS));

    return 0;
}

static void
test_imp_free_data(void *model_data, void *UNUSED(user_data))
{
    free(model_data);
}

static LY_ERR
test_imp_clb(const char *mod_name, const char *UNUSED(mod_rev), const char *UNUSED(submod_name),
        const char *UNUSED(sub_rev), void *user_data, LYS_INFORMAT *format,
        const char **module_data, void (**free_module_data)(void *model_data, void *user_data))
{
    char *nl;

    if ((nl = strchr(user_data, '\n'))) {
        /* more modules */
        if (!strncmp(user_data + 7, mod_name, strlen(mod_name))) {
            *module_data = strndup(user_data, nl - (char *)user_data);
            *format = LYS_IN_YANG;
            *free_module_data = test_imp_free_data;
        } else {
            *module_data = strdup(nl + 1);
            *format = LYS_IN_YANG;
            *free_module_data = test_imp_free_data;
        }
    } else {
        *module_data = user_data;
        *format = LYS_IN_YANG;
        *free_module_data = NULL;
    }
    return LY_SUCCESS;
}

static void
test_module(void **state)
{
    const char *str;
    struct ly_in *in;
    struct lys_module *mod = NULL;
    struct lysp_feature *f;
    struct lysc_iffeature *iff;
    struct lys_glob_unres unres = {0};

    str = "module test {namespace urn:test; prefix t;"
            "feature f1;feature f2 {if-feature f1;}}";
    assert_int_equal(LY_EINVAL, lys_compile(NULL, 0, NULL));
    CHECK_LOG("Invalid argument mod (lys_compile()).", NULL);
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_SUCCESS, lys_create_module(UTEST_LYCTX, in, LYS_IN_YANG, 0, NULL, NULL, NULL, &unres, &mod));
    assert_int_equal(LY_SUCCESS, lys_compile_unres_glob(UTEST_LYCTX, &unres));
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    ly_in_free(in, 0);
    assert_int_equal(0, mod->implemented);
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0, &unres));
    assert_int_equal(LY_SUCCESS, lys_compile_unres_glob(UTEST_LYCTX, &unres));
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    assert_null(mod->compiled);
    mod->implemented = 1;
    assert_int_equal(LY_SUCCESS, lys_compile(mod, 0, &unres));
    assert_int_equal(LY_SUCCESS, lys_compile_unres_glob(UTEST_LYCTX, &unres));
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    assert_non_null(mod->compiled);
    assert_string_equal("test", mod->name);
    assert_string_equal("urn:test", mod->ns);
    assert_string_equal("t", mod->prefix);
    /* features */
    assert_non_null(mod->parsed->features);
    assert_int_equal(2, LY_ARRAY_COUNT(mod->parsed->features));
    f = &mod->parsed->features[1];
    assert_non_null(f->iffeatures);
    assert_int_equal(1, LY_ARRAY_COUNT(f->iffeatures));
    iff = &f->iffeatures_c[0];
    assert_non_null(iff->expr);
    assert_non_null(iff->features);
    assert_int_equal(1, LY_ARRAY_COUNT(iff->features));
    assert_ptr_equal(&mod->parsed->features[0], iff->features[0]);

    /* submodules cannot be compiled directly */
    str = "submodule test {belongs-to xxx {prefix x;}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_EINVAL, lys_create_module(UTEST_LYCTX, in, LYS_IN_YANG, 1, NULL, NULL, NULL, &unres, NULL));
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    ly_in_free(in, 0);
    CHECK_LOG_CTX("Input data contains submodule which cannot be parsed directly without its main module.", NULL);

    /* data definition name collision in top level */
    str = "module aa {namespace urn:aa;prefix aa; leaf a {type string;} container a{presence x;}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str, &in));
    assert_int_equal(LY_EEXIST, lys_create_module(UTEST_LYCTX, in, LYS_IN_YANG, 1, NULL, NULL, NULL, &unres, &mod));
    CHECK_LOG_CTX("Duplicate identifier \"a\" of data definition/RPC/action/notification statement.", "/aa:a");
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    ly_in_free(in, 0);
}

static void
test_name_collisions(void **state)
{
    const char *yang_data;

    /* top-level */
    yang_data = "module a {namespace urn:a;prefix a;"
            "  container c;"
            "  leaf a {type empty;}"
            "  leaf c {type empty;}"
            "}";
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, yang_data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"c\" of data definition/RPC/action/notification statement.", "/a:c");
    UTEST_LOG_CLEAN;

    yang_data = "module a {namespace urn:a;prefix a;"
            "  container c;"
            "  leaf a {type empty;}"
            "  notification c;"
            "}";
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, yang_data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"c\" of data definition/RPC/action/notification statement.", "/a:c");
    UTEST_LOG_CLEAN;

    yang_data = "module a {namespace urn:a;prefix a;"
            "  container c;"
            "  leaf a {type empty;}"
            "  rpc c;"
            "}";
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, yang_data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"c\" of data definition/RPC/action/notification statement.", "/a:c");
    UTEST_LOG_CLEAN;

    yang_data = "module a {namespace urn:a;prefix a;"
            "  container c;"
            "  leaf a {type empty;}"
            "  choice ch {"
            "    leaf c {type string;}"
            "    case c2 {"
            "      leaf aa {type empty;}"
            "    }"
            "  }"
            "}";
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, yang_data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"c\" of data definition/RPC/action/notification statement.", "/a:ch/c/c");
    UTEST_LOG_CLEAN;

    /* nested */
    yang_data = "module a {namespace urn:a;prefix a;container c { list l {key \"k\"; leaf k {type string;}"
            "leaf-list a {type string;}"
            "container a;"
            "}}}";
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, yang_data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"a\" of data definition/RPC/action/notification statement.", "/a:c/l/a");
    UTEST_LOG_CLEAN;

    yang_data = "module a {yang-version 1.1;namespace urn:a;prefix a;container c { list l {key \"k\"; leaf k {type string;}"
            "leaf-list a {type string;}"
            "notification a;"
            "}}}";
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, yang_data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"a\" of data definition/RPC/action/notification statement.", "/a:c/l/a");
    UTEST_LOG_CLEAN;

    yang_data = "module a {yang-version 1.1;namespace urn:a;prefix a;container c { list l {key \"k\"; leaf k {type string;}"
            "leaf-list a {type string;}"
            "action a;"
            "}}}";
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, yang_data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"a\" of data definition/RPC/action/notification statement.", "/a:c/l/a");
    UTEST_LOG_CLEAN;

    /* grouping */
}

static void
test_node_container(void **state)
{
    const struct lys_module *mod;
    struct lysc_node_container *cont;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {namespace urn:a;prefix a;container c;}", LYS_IN_YANG, &mod));
    assert_non_null(mod->compiled);
    assert_non_null((cont = (struct lysc_node_container *)mod->compiled->data));
    assert_int_equal(LYS_CONTAINER, cont->nodetype);
    assert_string_equal("c", cont->name);
    assert_true(cont->flags & LYS_CONFIG_W);
    assert_true(cont->flags & LYS_STATUS_CURR);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b;container c {config false; status deprecated; container child;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Missing explicit \"deprecated\" status that was already specified in parent, inheriting.", NULL);
    assert_non_null(mod->compiled);
    assert_non_null((cont = (struct lysc_node_container *)mod->compiled->data));
    assert_true(cont->flags & LYS_CONFIG_R);
    assert_true(cont->flags & LYS_STATUS_DEPRC);
    assert_non_null((cont = (struct lysc_node_container *)cont->child));
    assert_int_equal(LYS_CONTAINER, cont->nodetype);
    assert_true(cont->flags & LYS_CONFIG_R);
    assert_true(cont->flags & LYS_STATUS_DEPRC);
    assert_string_equal("child", cont->name);
}

static void
test_node_leaflist(void **state)
{
    const struct lys_module *mod;
    struct lysc_type *type;
    struct lysc_node_leaflist *ll;
    struct lysc_node_leaf *l;
    const char *dflt;
    uint8_t dynamic;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {namespace urn:a;prefix a;"
            "typedef mytype {type union {type leafref {path ../target;} type string;}}"
            "leaf-list ll1 {type union {type decimal64 {fraction-digits 2;} type mytype;}}"
            "leaf-list ll2 {type leafref {path ../target;}}"
            "leaf target {type int8;}}",
            LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_UNION, type->basetype);
    assert_non_null(((struct lysc_type_union *)type)->types);
    assert_int_equal(3, LY_ARRAY_COUNT(((struct lysc_type_union *)type)->types));
    assert_int_equal(LY_TYPE_DEC64, ((struct lysc_type_union *)type)->types[0]->basetype);
    assert_int_equal(LY_TYPE_LEAFREF, ((struct lysc_type_union *)type)->types[1]->basetype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_union *)type)->types[2]->basetype);
    assert_non_null(((struct lysc_type_leafref *)((struct lysc_type_union *)type)->types[1])->realtype);
    assert_int_equal(LY_TYPE_INT8, ((struct lysc_type_leafref *)((struct lysc_type_union *)type)->types[1])->realtype->basetype);
    type = ((struct lysc_node_leaf *)mod->compiled->data->next)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_non_null(((struct lysc_type_leafref *)type)->realtype);
    assert_int_equal(LY_TYPE_INT8, ((struct lysc_type_leafref *)type)->realtype->basetype);

    /* now test for string type is in file ./tests/utests/types/string.c */
#if 0
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b;leaf-list ll {type string;}}", LYS_IN_YANG, &mod));
    assert_non_null(mod->compiled);
    assert_non_null((ll = (struct lysc_node_leaflist *)mod->compiled->data));
    assert_int_equal(0, ll->min);
    assert_int_equal((uint32_t)-1, ll->max);
#endif

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module c {yang-version 1.1;namespace urn:c;prefix c;typedef mytype {type int8;default 10;}"
            "leaf-list ll1 {type mytype;default 1; default 1; config false;}"
            "leaf-list ll2 {type mytype; ordered-by user;}}", LYS_IN_YANG, &mod));
    assert_non_null(mod->compiled);
    assert_non_null((ll = (struct lysc_node_leaflist *)mod->compiled->data));
    assert_non_null(ll->dflts);
    assert_int_equal(6, ll->type->refcount); /* 3x type's reference, 3x default value's reference (typedef's default does not reference own type) */
    assert_int_equal(2, LY_ARRAY_COUNT(ll->dflts));
    assert_string_equal("1", dflt = ll->dflts[0]->realtype->plugin->print(UTEST_LYCTX, ll->dflts[0], LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_string_equal("1", dflt = ll->dflts[1]->realtype->plugin->print(UTEST_LYCTX, ll->dflts[1], LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR | LYS_ORDBY_USER | LYS_SET_DFLT | LYS_SET_CONFIG, ll->flags);
    assert_non_null((ll = (struct lysc_node_leaflist *)mod->compiled->data->next));
    assert_non_null(ll->dflts);
    assert_int_equal(6, ll->type->refcount); /* 3x type's reference, 3x default value's reference */
    assert_int_equal(1, LY_ARRAY_COUNT(ll->dflts));
    assert_string_equal("10", dflt = ll->dflts[0]->realtype->plugin->print(UTEST_LYCTX, ll->dflts[0], LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_int_equal(LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_USER, ll->flags);

    /* ordered-by is ignored (with verbose message) for state data, RPC/action output parameters and notification content */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module d {yang-version 1.1;namespace urn:d;prefix d;"
            "leaf-list ll {config false; type string; ordered-by user;}}", LYS_IN_YANG, &mod));
    assert_non_null(mod->compiled);
    assert_non_null((ll = (struct lysc_node_leaflist *)mod->compiled->data));
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR | LYS_ORDBY_USER | LYS_SET_CONFIG, ll->flags);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module e {yang-version 1.1;namespace urn:e;prefix e;"
            "rpc oper {output {leaf-list ll {type string; ordered-by user;}}}}", LYS_IN_YANG, &mod));

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module f {yang-version 1.1;namespace urn:f;prefix f;"
            "notification event {leaf-list ll {type string; ordered-by user;}}}", LYS_IN_YANG, &mod));

    /* forward reference in default */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module g {yang-version 1.1; namespace urn:g;prefix g;"
            "leaf ref {type instance-identifier {require-instance true;} default \"/g:g[.='val']\";}"
            "leaf-list g {type string;}}", LYS_IN_YANG, &mod));
    assert_non_null(l = (struct lysc_node_leaf *)mod->compiled->data);
    assert_string_equal("ref", l->name);
    assert_non_null(l->dflt);

    /* invalid */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa;leaf-list ll {type empty;}}",
            LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Leaf-list of type \"empty\" is allowed only in YANG 1.1 modules.", "/aa:ll");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module bb {yang-version 1.1;namespace urn:bb;prefix bb;leaf-list ll {type empty; default x;}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Invalid empty value length 1.).", "Schema location /bb:ll.");

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module cc {yang-version 1.1;namespace urn:cc;prefix cc;"
            "leaf-list ll {config false;type string; default one;default two;default one;}}", LYS_IN_YANG, &mod));
    assert_non_null(mod->compiled);
    assert_non_null((ll = (struct lysc_node_leaflist *)mod->compiled->data));
    assert_non_null(ll->dflts);
    assert_int_equal(3, LY_ARRAY_COUNT(ll->dflts));
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module dd {yang-version 1.1;namespace urn:dd;prefix dd;"
            "leaf-list ll {type string; default one;default two;default one;}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Configuration leaf-list has multiple defaults of the same value \"one\".", "/dd:ll");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ee {yang-version 1.1; namespace urn:ee;prefix ee;"
            "leaf ref {type instance-identifier {require-instance true;} default \"/ee:g\";}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid default - value does not fit the type "
            "(Invalid instance-identifier \"/ee:g\" value - semantic error.).", "Schema location /ee:ref.");
}

static void
test_node_list(void **state)
{
    const struct lys_module *mod;
    struct lysc_node_list *list;
    struct lysc_node *child;
    struct ly_in *in;
    const char *data =
            "module a {namespace urn:a;prefix a;feature f;"
            "list l1 {key \"x y\"; ordered-by user; leaf y{type string;if-feature f;} leaf x {type string; when 1;}}"
            "list l2 {config false;leaf value {type string;}}}";
    const char *feats[] = {"f", NULL};

    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lys_parse(UTEST_LYCTX, in, LYS_IN_YANG, feats, &mod));
    ly_in_free(in, 0);
    list = (struct lysc_node_list *)mod->compiled->data;
    assert_non_null(list);
    assert_non_null(list->child);
    assert_string_equal("x", list->child->name);
    assert_true(list->child->flags & LYS_KEY);
    assert_string_equal("y", list->child->next->name);
    assert_true(list->child->next->flags & LYS_KEY);
    assert_non_null(list->child);
    assert_int_equal(LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_USER, list->flags);
    assert_true(list->child->flags & LYS_KEY);
    assert_true(list->child->next->flags & LYS_KEY);
    list = (struct lysc_node_list *)mod->compiled->data->next;
    assert_non_null(list);
    assert_non_null(list->child);
    assert_false(list->child->flags & LYS_KEY);
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR | LYS_ORDBY_USER | LYS_SET_CONFIG | LYS_KEYLESS, list->flags);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b;"
            "list l {key a; unique \"a c/b:b\"; unique \"c/e d\";"
            "leaf a {type string; default x;} leaf d {type string;config false;}"
            "container c {leaf b {type string;}leaf e{type string;config false;}}}}",
            LYS_IN_YANG, &mod));
    list = (struct lysc_node_list *)mod->compiled->data;
    assert_non_null(list);
    assert_string_equal("l", list->name);
    assert_string_equal("a", list->child->name);
    assert_true(list->child->flags & LYS_KEY);
    assert_null(((struct lysc_node_leaf *)list->child)->dflt);
    assert_non_null(list->uniques);
    assert_int_equal(2, LY_ARRAY_COUNT(list->uniques));
    assert_int_equal(2, LY_ARRAY_COUNT(list->uniques[0]));
    assert_string_equal("a", list->uniques[0][0]->name);
    assert_true(list->uniques[0][0]->flags & LYS_UNIQUE);
    assert_string_equal("b", list->uniques[0][1]->name);
    assert_true(list->uniques[0][1]->flags & LYS_UNIQUE);
    assert_int_equal(2, LY_ARRAY_COUNT(list->uniques[1]));
    assert_string_equal("e", list->uniques[1][0]->name);
    assert_true(list->uniques[1][0]->flags & LYS_UNIQUE);
    assert_string_equal("d", list->uniques[1][1]->name);
    assert_true(list->uniques[1][1]->flags & LYS_UNIQUE);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module c {yang-version 1.1;namespace urn:c;prefix c;"
            "list l {key a;leaf a {type empty;}}}", LYS_IN_YANG, &mod));
    list = (struct lysc_node_list *)mod->compiled->data;
    assert_non_null(list);
    assert_string_equal("l", list->name);
    assert_string_equal("a", list->child->name);
    assert_true(list->child->flags & LYS_KEY);
    assert_int_equal(LY_TYPE_EMPTY, ((struct lysc_node_leaf *)list->child)->type->basetype);

    /* keys order */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module d {yang-version 1.1;namespace urn:d;prefix d;"
            "list l {key \"d b c\";leaf a {type string;} leaf b {type string;} leaf c {type string;} leaf d {type string;}}}", LYS_IN_YANG, &mod));
    list = (struct lysc_node_list *)mod->compiled->data;
    assert_non_null(list);
    assert_string_equal("l", list->name);
    assert_non_null(child = list->child);
    assert_string_equal("d", child->name);
    assert_true(child->flags & LYS_KEY);
    assert_non_null(child = child->next);
    assert_string_equal("b", child->name);
    assert_true(child->flags & LYS_KEY);
    assert_non_null(child = child->next);
    assert_string_equal("c", child->name);
    assert_true(child->flags & LYS_KEY);
    assert_non_null(child = child->next);
    assert_string_equal("a", child->name);
    assert_false(child->flags & LYS_KEY);

    /* invalid */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa;list l;}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Missing key in list representing configuration data.", "/aa:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module bb {yang-version 1.1; namespace urn:bb;prefix bb;"
            "list l {key x; leaf x {type string; when 1;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("List's key must not have any \"when\" statement.", "/bb:l/x");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module cc {yang-version 1.1;namespace urn:cc;prefix cc;feature f;"
            "list l {key x; leaf x {type string; if-feature f;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Key \"x\" is disabled by its if-features.", "Schema location /cc:l/x.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module dd {namespace urn:dd;prefix dd;"
            "list l {key x; leaf x {type string; config false;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Key of the configuration list must not be status leaf.", "/dd:l/x");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ee {namespace urn:ee;prefix ee;"
            "list l {config false;key x; leaf x {type string; config true;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Configuration node cannot be child of any state data node.", "/ee:l/x");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ff {namespace urn:ff;prefix ff;"
            "list l {key x; leaf-list x {type string;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("The list's key \"x\" not found.", "/ff:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module gg {namespace urn:gg;prefix gg;"
            "list l {key x; unique y;leaf x {type string;} leaf-list y {type string;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Unique's descendant-schema-nodeid \"y\" refers to leaf-list node instead of a leaf.", "/gg:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module hh {namespace urn:hh;prefix hh;"
            "list l {key x; unique \"x y\";leaf x {type string;} leaf y {config false; type string;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Unique statement \"x y\" refers to leaves with different config type.", "/hh:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ii {namespace urn:ii;prefix ii;"
            "list l {key x; unique a:x;leaf x {type string;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid descendant-schema-nodeid value \"a:x\" - prefix \"a\" not defined in module \"ii\".", "/ii:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module jj {namespace urn:jj;prefix jj;"
            "list l {key x; unique c/x;leaf x {type string;}container c {leaf y {type string;}}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid descendant-schema-nodeid value \"c/x\" - target node not found.", "/jj:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module kk {namespace urn:kk;prefix kk;"
            "list l {key x; unique c^y;leaf x {type string;}container c {leaf y {type string;}}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid descendant-schema-nodeid value \"c^\" - missing \"/\" as node-identifier separator.", "/kk:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ll {namespace urn:ll;prefix ll;"
            "list l {key \"x y x\";leaf x {type string;}leaf y {type string;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicated key identifier \"x\".", "/ll:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module mm {namespace urn:mm;prefix mm;"
            "list l {key x;leaf x {type empty;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("List's key cannot be of \"empty\" type until it is in YANG 1.1 module.", "/mm:l/x");
}

static void
test_node_choice(void **state)
{
    const struct lys_module *mod;
    struct lysc_node_choice *ch;
    struct lysc_node_case *cs;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {namespace urn:a;prefix a;feature f;"
            "choice ch {default a:b; when \"true()\"; case a {leaf a1 {type string;}leaf a2 {type string;}}"
            "leaf b {type string;}}}", LYS_IN_YANG, &mod));
    ch = (struct lysc_node_choice *)mod->compiled->data;
    assert_non_null(ch);
    assert_int_equal(LYS_CONFIG_W | LYS_STATUS_CURR, ch->flags);
    assert_int_equal(1, LY_ARRAY_COUNT(ch->when));
    assert_null(ch->when[0]->context);
    cs = ch->cases;
    assert_non_null(cs);
    assert_string_equal("a", cs->name);
    assert_ptr_equal(ch, cs->parent);
    assert_non_null(cs->child);
    assert_string_equal("a1", cs->child->name);
    assert_non_null(cs->child->next);
    assert_string_equal("a2", cs->child->next->name);
    assert_ptr_equal(cs, cs->child->parent);
    cs = (struct lysc_node_case *)cs->next;
    assert_non_null(cs);
    assert_string_equal("b", cs->name);
    assert_int_equal(LYS_STATUS_CURR | LYS_SET_DFLT | LYS_CONFIG_W, cs->flags);
    assert_ptr_equal(ch, cs->parent);
    assert_non_null(cs->child);
    assert_string_equal("b", cs->child->name);
    assert_ptr_equal(cs, cs->child->parent);
    assert_ptr_equal(ch->dflt, cs);

    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa;"
            "choice ch {case a {leaf x {type string;}}leaf x {type string;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"x\" of data definition/RPC/action/notification statement.", "/aa:ch/x/x");
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module aa2 {namespace urn:aa2;prefix aa;"
            "choice ch {case a {leaf y {type string;}}case b {leaf y {type string;}}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"y\" of data definition/RPC/action/notification statement.", "/aa2:ch/b/y");
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module bb {namespace urn:bb;prefix bb;"
            "choice ch {case a {leaf x {type string;}}leaf a {type string;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"a\" of case statement.", "/bb:ch/a");
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module bb2 {namespace urn:bb2;prefix bb;"
            "choice ch {case b {leaf x {type string;}}case b {leaf y {type string;}}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"b\" of case statement.", "/bb2:ch/b");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ca {namespace urn:ca;prefix ca;"
            "choice ch {default c;case a {leaf x {type string;}}case b {leaf y {type string;}}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Default case \"c\" not found.", "/ca:ch");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module cb {namespace urn:cb;prefix cb; import a {prefix a;}"
            "choice ch {default a:a;case a {leaf x {type string;}}case b {leaf y {type string;}}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Default case \"a:a\" not found.", "/cb:ch");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module cc {namespace urn:cc;prefix cc;"
            "choice ch {default a;case a {leaf x {mandatory true;type string;}}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Mandatory node \"x\" under the default case \"a\".", "/cc:ch");
    /* TODO check with mandatory nodes from augment placed into the case */
}

static void
test_node_anydata(void **state)
{
    const struct lys_module *mod;
    struct lysc_node_anydata *any;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {yang-version 1.1;namespace urn:a;prefix a;"
            "anydata any {config false;mandatory true;}}", LYS_IN_YANG, &mod));
    any = (struct lysc_node_anydata *)mod->compiled->data;
    assert_non_null(any);
    assert_int_equal(LYS_ANYDATA, any->nodetype);
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR | LYS_MAND_TRUE | LYS_SET_CONFIG, any->flags);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b;"
            "anyxml any;}", LYS_IN_YANG, &mod));
    any = (struct lysc_node_anydata *)mod->compiled->data;
    assert_non_null(any);
    assert_int_equal(LYS_ANYXML, any->nodetype);
    assert_int_equal(LYS_CONFIG_W | LYS_STATUS_CURR, any->flags);

    /* invalid */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa;anydata any;}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid keyword \"anydata\" as a child of \"module\" - the statement is allowed only in YANG 1.1 modules.", "Line number 1.");
}

static void
test_action(void **state)
{
    const struct lys_module *mod;
    const struct lysc_node_action *rpc;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {namespace urn:a;prefix a;"
            "rpc a {input {leaf x {type int8;} leaf y {type int8;}} output {leaf result {type int16;}}}}", LYS_IN_YANG, &mod));
    rpc = mod->compiled->rpcs;
    assert_non_null(rpc);
    assert_null(rpc->next);
    assert_int_equal(LYS_RPC, rpc->nodetype);
    assert_int_equal(LYS_STATUS_CURR, rpc->flags);
    assert_string_equal("a", rpc->name);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {yang-version 1.1; namespace urn:b;prefix b; container top {"
            "action b {input {leaf x {type int8;} leaf y {type int8;}}"
            "output {must \"result > 25\"; must \"/top\"; leaf result {type int16;}}}}"
            "augment /top/b/output {leaf result2 {type string;}}}", LYS_IN_YANG, &mod));
    rpc = lysc_node_actions(mod->compiled->data);
    assert_non_null(rpc);
    assert_null(rpc->next);
    assert_int_equal(LYS_ACTION, rpc->nodetype);
    assert_int_equal(LYS_STATUS_CURR, rpc->flags);
    assert_string_equal("b", rpc->name);
    assert_null(rpc->input.musts);
    assert_int_equal(2, LY_ARRAY_COUNT(rpc->output.musts));

    /* invalid */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa;container top {action x;}}",
            LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid keyword \"action\" as a child of \"container\" - the statement is allowed only in YANG 1.1 modules.", "Line number 1.");

    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module bb {namespace urn:bb;prefix bb;leaf x{type string;} rpc x;}",
            LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"x\" of data definition/RPC/action/notification statement.", "/bb:x");
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module cc {yang-version 1.1; namespace urn:cc;prefix cc;container c {leaf y {type string;} action y;}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"y\" of data definition/RPC/action/notification statement.", "/cc:c/y");
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module dd {yang-version 1.1; namespace urn:dd;prefix dd;container c {action z; action z;}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"z\" of data definition/RPC/action/notification statement.", "/dd:c/z");
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule eesub {belongs-to ee {prefix ee;} notification w;}");
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module ee {yang-version 1.1; namespace urn:ee;prefix ee;include eesub; rpc w;}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"w\" of data definition/RPC/action/notification statement.", "/ee:w");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ff {yang-version 1.1; namespace urn:ff;prefix ff; rpc test {input {container a {leaf b {type string;}}}}"
            "augment /test/input/a {action invalid {input {leaf x {type string;}}}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Action \"invalid\" is placed inside another RPC/action.", "/ff:{augment='/test/input/a'}/invalid");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module gg {yang-version 1.1; namespace urn:gg;prefix gg; notification test {container a {leaf b {type string;}}}"
            "augment /test/a {action invalid {input {leaf x {type string;}}}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Action \"invalid\" is placed inside notification.", "/gg:{augment='/test/a'}/invalid");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module hh {yang-version 1.1; namespace urn:hh;prefix hh; notification test {container a {uses grp;}}"
            "grouping grp {action invalid {input {leaf x {type string;}}}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Action \"invalid\" is placed inside notification.", "/hh:test/a/{uses='grp'}/invalid");
}

static void
test_notification(void **state)
{
    const struct lys_module *mod;
    const struct lysc_node_notif *notif;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {namespace urn:a;prefix a;"
            "notification a1 {leaf x {type int8;}} notification a2;}", LYS_IN_YANG, &mod));
    notif = mod->compiled->notifs;
    assert_non_null(notif);
    assert_non_null(notif->next);
    assert_null(notif->next->next);
    assert_int_equal(LYS_NOTIF, notif->nodetype);
    assert_int_equal(LYS_STATUS_CURR, notif->flags);
    assert_string_equal("a1", notif->name);
    assert_non_null(notif->child);
    assert_string_equal("x", notif->child->name);
    notif = notif->next;
    assert_int_equal(LYS_NOTIF, notif->nodetype);
    assert_int_equal(LYS_STATUS_CURR, notif->flags);
    assert_string_equal("a2", notif->name);
    assert_null(notif->child);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {yang-version 1.1; namespace urn:b;prefix b; container top {"
            "notification b1 {leaf x {type int8;}} notification b2 {must \"/top\";}}}", LYS_IN_YANG, &mod));
    notif = lysc_node_notifs(mod->compiled->data);
    assert_non_null(notif);
    assert_non_null(notif->next);
    assert_null(notif->next->next);
    assert_int_equal(LYS_NOTIF, notif->nodetype);
    assert_int_equal(LYS_STATUS_CURR, notif->flags);
    assert_string_equal("b1", notif->name);
    assert_non_null(notif->child);
    assert_string_equal("x", notif->child->name);
    notif = notif->next;
    assert_int_equal(LYS_NOTIF, notif->nodetype);
    assert_int_equal(LYS_STATUS_CURR, notif->flags);
    assert_string_equal("b2", notif->name);
    assert_null(notif->child);
    assert_int_equal(1, LY_ARRAY_COUNT(notif->musts));

    /* invalid */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa;container top {notification x;}}",
            LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid keyword \"notification\" as a child of \"container\" - the statement is allowed only in YANG 1.1 modules.", "Line number 1.");

    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module bb {namespace urn:bb;prefix bb;leaf x{type string;} notification x;}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"x\" of data definition/RPC/action/notification statement.", "/bb:x");
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module cc {yang-version 1.1; namespace urn:cc;prefix cc;container c {leaf y {type string;} notification y;}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"y\" of data definition/RPC/action/notification statement.", "/cc:c/y");
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module dd {yang-version 1.1; namespace urn:dd;prefix dd;container c {notification z; notification z;}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"z\" of data definition/RPC/action/notification statement.", "/dd:c/z");
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule eesub {belongs-to ee {prefix ee;} rpc w;}");
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module ee {yang-version 1.1; namespace urn:ee;prefix ee;include eesub; notification w;}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate identifier \"w\" of data definition/RPC/action/notification statement.", "/ee:w");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ff {yang-version 1.1; namespace urn:ff;prefix ff; rpc test {input {container a {leaf b {type string;}}}}"
            "augment /test/input/a {notification invalid {leaf x {type string;}}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Notification \"invalid\" is placed inside RPC/action.", "/ff:{augment='/test/input/a'}/invalid");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module gg {yang-version 1.1; namespace urn:gg;prefix gg; notification test {container a {leaf b {type string;}}}"
            "augment /test/a {notification invalid {leaf x {type string;}}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Notification \"invalid\" is placed inside another notification.", "/gg:{augment='/test/a'}/invalid");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module hh {yang-version 1.1; namespace urn:hh;prefix hh; rpc test {input {container a {uses grp;}}}"
            "grouping grp {notification invalid {leaf x {type string;}}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Notification \"invalid\" is placed inside RPC/action.", "/hh:test/input/a/{uses='grp'}/invalid");
}

/**
 * actually the same as length restriction (tested in test_type_length()), so just check the correct handling in appropriate types,
 * do not test the expression itself
 */
static void
test_type_range(void **state)
{
    const struct lys_module *mod;
    struct lysc_type *type;

#if 0
    /*test about int8 should be in tests/utests/types/int8.c*/
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {namespace urn:a;prefix a;leaf l {type int8 {range min..10|max;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_INT8, type->basetype);
    assert_non_null(((struct lysc_type_num *)type)->range);
    assert_non_null(((struct lysc_type_num *)type)->range->parts);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_num *)type)->range->parts));
    assert_int_equal(-128, ((struct lysc_type_num *)type)->range->parts[0].min_64);
    assert_int_equal(10, ((struct lysc_type_num *)type)->range->parts[0].max_64);
    assert_int_equal(127, ((struct lysc_type_num *)type)->range->parts[1].min_64);
    assert_int_equal(127, ((struct lysc_type_num *)type)->range->parts[1].max_64);
#endif

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b;leaf l {type int16 {range min..10|max;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_INT16, type->basetype);
    assert_non_null(((struct lysc_type_num *)type)->range);
    assert_non_null(((struct lysc_type_num *)type)->range->parts);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_num *)type)->range->parts));
    assert_int_equal(-32768, ((struct lysc_type_num *)type)->range->parts[0].min_64);
    assert_int_equal(10, ((struct lysc_type_num *)type)->range->parts[0].max_64);
    assert_int_equal(32767, ((struct lysc_type_num *)type)->range->parts[1].min_64);
    assert_int_equal(32767, ((struct lysc_type_num *)type)->range->parts[1].max_64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module c {namespace urn:c;prefix c;leaf l {type int32 {range min..10|max;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_INT32, type->basetype);
    assert_non_null(((struct lysc_type_num *)type)->range);
    assert_non_null(((struct lysc_type_num *)type)->range->parts);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_num *)type)->range->parts));
    assert_int_equal(INT64_C(-2147483648), ((struct lysc_type_num *)type)->range->parts[0].min_64);
    assert_int_equal(10, ((struct lysc_type_num *)type)->range->parts[0].max_64);
    assert_int_equal(INT64_C(2147483647), ((struct lysc_type_num *)type)->range->parts[1].min_64);
    assert_int_equal(INT64_C(2147483647), ((struct lysc_type_num *)type)->range->parts[1].max_64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module d {namespace urn:d;prefix d;leaf l {type int64 {range min..10|max;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_INT64, type->basetype);
    assert_non_null(((struct lysc_type_num *)type)->range);
    assert_non_null(((struct lysc_type_num *)type)->range->parts);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_num *)type)->range->parts));
    assert_int_equal(INT64_C(-9223372036854775807) - INT64_C(1), ((struct lysc_type_num *)type)->range->parts[0].min_64);
    assert_int_equal(10, ((struct lysc_type_num *)type)->range->parts[0].max_64);
    assert_int_equal(INT64_C(9223372036854775807), ((struct lysc_type_num *)type)->range->parts[1].min_64);
    assert_int_equal(INT64_C(9223372036854775807), ((struct lysc_type_num *)type)->range->parts[1].max_64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module e {namespace urn:e;prefix e;leaf l {type uint8 {range min..10|max;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_UINT8, type->basetype);
    assert_non_null(((struct lysc_type_num *)type)->range);
    assert_non_null(((struct lysc_type_num *)type)->range->parts);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_num *)type)->range->parts));
    assert_int_equal(0, ((struct lysc_type_num *)type)->range->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_num *)type)->range->parts[0].max_u64);
    assert_int_equal(255, ((struct lysc_type_num *)type)->range->parts[1].min_u64);
    assert_int_equal(255, ((struct lysc_type_num *)type)->range->parts[1].max_u64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module f {namespace urn:f;prefix f;leaf l {type uint16 {range min..10|max;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_UINT16, type->basetype);
    assert_non_null(((struct lysc_type_num *)type)->range);
    assert_non_null(((struct lysc_type_num *)type)->range->parts);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_num *)type)->range->parts));
    assert_int_equal(0, ((struct lysc_type_num *)type)->range->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_num *)type)->range->parts[0].max_u64);
    assert_int_equal(65535, ((struct lysc_type_num *)type)->range->parts[1].min_u64);
    assert_int_equal(65535, ((struct lysc_type_num *)type)->range->parts[1].max_u64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module g {namespace urn:g;prefix g;leaf l {type uint32 {range min..10|max;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_UINT32, type->basetype);
    assert_non_null(((struct lysc_type_num *)type)->range);
    assert_non_null(((struct lysc_type_num *)type)->range->parts);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_num *)type)->range->parts));
    assert_int_equal(0, ((struct lysc_type_num *)type)->range->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_num *)type)->range->parts[0].max_u64);
    assert_int_equal(UINT64_C(4294967295), ((struct lysc_type_num *)type)->range->parts[1].min_u64);
    assert_int_equal(UINT64_C(4294967295), ((struct lysc_type_num *)type)->range->parts[1].max_u64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module h {namespace urn:h;prefix h;leaf l {type uint64 {range min..10|max;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_UINT64, type->basetype);
    assert_non_null(((struct lysc_type_num *)type)->range);
    assert_non_null(((struct lysc_type_num *)type)->range->parts);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_num *)type)->range->parts));
    assert_int_equal(0, ((struct lysc_type_num *)type)->range->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_num *)type)->range->parts[0].max_u64);
    assert_int_equal(UINT64_C(18446744073709551615), ((struct lysc_type_num *)type)->range->parts[1].min_u64);
    assert_int_equal(UINT64_C(18446744073709551615), ((struct lysc_type_num *)type)->range->parts[1].max_u64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module i {namespace urn:i;prefix i;typedef mytype {type uint8 {range 10..100;}}"
            "typedef mytype2 {type mytype;} leaf l {type mytype2;}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(3, type->refcount);
    assert_int_equal(LY_TYPE_UINT8, type->basetype);
    assert_non_null(((struct lysc_type_num *)type)->range);
    assert_non_null(((struct lysc_type_num *)type)->range->parts);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_num *)type)->range->parts));

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module j {namespace urn:j;prefix j;"
            "typedef mytype {type uint8 {range 1..100{description \"one to hundred\";reference A;}}}"
            "leaf l {type mytype {range 1..10 {description \"one to ten\";reference B;}}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_UINT8, type->basetype);
    assert_non_null(((struct lysc_type_num *)type)->range);
    assert_string_equal("one to ten", ((struct lysc_type_num *)type)->range->dsc);
    assert_string_equal("B", ((struct lysc_type_num *)type)->range->ref);
    assert_non_null(((struct lysc_type_num *)type)->range->parts);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_num *)type)->range->parts));
    assert_int_equal(1, ((struct lysc_type_num *)type)->range->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_num *)type)->range->parts[0].max_u64);
}

static void
test_type_length(void **state)
{
    const struct lys_module *mod;
    struct lysc_type *type;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {namespace urn:a;prefix a;leaf l {type binary {length min {error-app-tag errortag;error-message error;}}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin *)type)->length);
    assert_non_null(((struct lysc_type_bin *)type)->length->parts);
    assert_string_equal("errortag", ((struct lysc_type_bin *)type)->length->eapptag);
    assert_string_equal("error", ((struct lysc_type_bin *)type)->length->emsg);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_bin *)type)->length->parts));
    assert_int_equal(0, ((struct lysc_type_bin *)type)->length->parts[0].min_u64);
    assert_int_equal(0, ((struct lysc_type_bin *)type)->length->parts[0].max_u64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b;leaf l {type binary {length max;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin *)type)->length);
    assert_non_null(((struct lysc_type_bin *)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_bin *)type)->length->parts));
    assert_int_equal(UINT64_C(18446744073709551615), ((struct lysc_type_bin *)type)->length->parts[0].min_u64);
    assert_int_equal(UINT64_C(18446744073709551615), ((struct lysc_type_bin *)type)->length->parts[0].max_u64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module c {namespace urn:c;prefix c;leaf l {type binary {length min..max;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin *)type)->length);
    assert_non_null(((struct lysc_type_bin *)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_bin *)type)->length->parts));
    assert_int_equal(0, ((struct lysc_type_bin *)type)->length->parts[0].min_u64);
    assert_int_equal(UINT64_C(18446744073709551615), ((struct lysc_type_bin *)type)->length->parts[0].max_u64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module d {namespace urn:d;prefix d;leaf l {type binary {length 5;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin *)type)->length);
    assert_non_null(((struct lysc_type_bin *)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_bin *)type)->length->parts));
    assert_int_equal(5, ((struct lysc_type_bin *)type)->length->parts[0].min_u64);
    assert_int_equal(5, ((struct lysc_type_bin *)type)->length->parts[0].max_u64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module e {namespace urn:e;prefix e;leaf l {type binary {length 1..10;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin *)type)->length);
    assert_non_null(((struct lysc_type_bin *)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_bin *)type)->length->parts));
    assert_int_equal(1, ((struct lysc_type_bin *)type)->length->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_bin *)type)->length->parts[0].max_u64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module f {namespace urn:f;prefix f;leaf l {type binary {length 1..10|20..30;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin *)type)->length);
    assert_non_null(((struct lysc_type_bin *)type)->length->parts);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_bin *)type)->length->parts));
    assert_int_equal(1, ((struct lysc_type_bin *)type)->length->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_bin *)type)->length->parts[0].max_u64);
    assert_int_equal(20, ((struct lysc_type_bin *)type)->length->parts[1].min_u64);
    assert_int_equal(30, ((struct lysc_type_bin *)type)->length->parts[1].max_u64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module g {namespace urn:g;prefix g;leaf l {type binary {length \"16 | 32\";}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin *)type)->length);
    assert_non_null(((struct lysc_type_bin *)type)->length->parts);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_bin *)type)->length->parts));
    assert_int_equal(16, ((struct lysc_type_bin *)type)->length->parts[0].min_u64);
    assert_int_equal(16, ((struct lysc_type_bin *)type)->length->parts[0].max_u64);
    assert_int_equal(32, ((struct lysc_type_bin *)type)->length->parts[1].min_u64);
    assert_int_equal(32, ((struct lysc_type_bin *)type)->length->parts[1].max_u64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module h {namespace urn:h;prefix h;typedef mytype {type binary {length 10;}}"
            "leaf l {type mytype {length \"10\";}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin *)type)->length);
    assert_non_null(((struct lysc_type_bin *)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_bin *)type)->length->parts));
    assert_int_equal(10, ((struct lysc_type_bin *)type)->length->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_bin *)type)->length->parts[0].max_u64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module i {namespace urn:i;prefix i;typedef mytype {type binary {length 10..100;}}"
            "leaf l {type mytype {length \"50\";}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin *)type)->length);
    assert_non_null(((struct lysc_type_bin *)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_bin *)type)->length->parts));
    assert_int_equal(50, ((struct lysc_type_bin *)type)->length->parts[0].min_u64);
    assert_int_equal(50, ((struct lysc_type_bin *)type)->length->parts[0].max_u64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module j {namespace urn:j;prefix j;typedef mytype {type binary {length 10..100;}}"
            "leaf l {type mytype {length \"10..30|60..100\";}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_bin *)type)->length);
    assert_non_null(((struct lysc_type_bin *)type)->length->parts);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_bin *)type)->length->parts));
    assert_int_equal(10, ((struct lysc_type_bin *)type)->length->parts[0].min_u64);
    assert_int_equal(30, ((struct lysc_type_bin *)type)->length->parts[0].max_u64);
    assert_int_equal(60, ((struct lysc_type_bin *)type)->length->parts[1].min_u64);
    assert_int_equal(100, ((struct lysc_type_bin *)type)->length->parts[1].max_u64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module k {namespace urn:k;prefix k;typedef mytype {type binary {length 10..100;}}"
            "leaf l {type mytype {length \"10..80\";}}leaf ll {type mytype;}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_non_null(((struct lysc_type_bin *)type)->length);
    assert_non_null(((struct lysc_type_bin *)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_bin *)type)->length->parts));
    assert_int_equal(10, ((struct lysc_type_bin *)type)->length->parts[0].min_u64);
    assert_int_equal(80, ((struct lysc_type_bin *)type)->length->parts[0].max_u64);
    type = ((struct lysc_node_leaf *)mod->compiled->data->next)->type;
    assert_non_null(type);
    assert_int_equal(2, type->refcount);
    assert_non_null(((struct lysc_type_bin *)type)->length);
    assert_non_null(((struct lysc_type_bin *)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_bin *)type)->length->parts));
    assert_int_equal(10, ((struct lysc_type_bin *)type)->length->parts[0].min_u64);
    assert_int_equal(100, ((struct lysc_type_bin *)type)->length->parts[0].max_u64);

    /* new string is tested in file ./tests/utests/types/string.c */
#if 0
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module l {namespace urn:l;prefix l;typedef mytype {type string {length 10..100;}}"
            "typedef mytype2 {type mytype {pattern '[0-9]*';}} leaf l {type mytype2 {pattern '[0-4]*';}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_int_equal(1, type->refcount);
    assert_non_null(((struct lysc_type_str *)type)->length);
    assert_non_null(((struct lysc_type_str *)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_str *)type)->length->parts));
    assert_int_equal(10, ((struct lysc_type_str *)type)->length->parts[0].min_u64);
    assert_int_equal(100, ((struct lysc_type_str *)type)->length->parts[0].max_u64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module m {namespace urn:m;prefix m;typedef mytype {type string {length 10;}}"
            "leaf l {type mytype {length min..max;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_int_equal(1, type->refcount);
    assert_non_null(((struct lysc_type_str *)type)->length);
    assert_non_null(((struct lysc_type_str *)type)->length->parts);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_str *)type)->length->parts));
    assert_int_equal(10, ((struct lysc_type_str *)type)->length->parts[0].min_u64);
    assert_int_equal(10, ((struct lysc_type_str *)type)->length->parts[0].max_u64);
#endif

    /* invalid values */
    assert_int_equal(LY_EDENIED, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa;leaf l {type binary {length -10;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - value \"-10\" does not fit the type limitations.", "/aa:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module bb {namespace urn:bb;prefix bb;leaf l {type binary {length 18446744073709551616;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - invalid value \"18446744073709551616\".", "/bb:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module cc {namespace urn:cc;prefix cc;leaf l {type binary {length \"max .. 10\";}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - unexpected data after max keyword (.. 10).", "/cc:l");
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module dd {namespace urn:dd;prefix dd;leaf l {type binary {length 50..10;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - values are not in ascending order (10).", "/dd:l");
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module ee {namespace urn:ee;prefix ee;leaf l {type binary {length \"50 | 10\";}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - values are not in ascending order (10).", "/ee:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ff {namespace urn:ff;prefix ff;leaf l {type binary {length \"x\";}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - unexpected data (x).", "/ff:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module gg {namespace urn:gg;prefix gg;leaf l {type binary {length \"50 | min\";}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - unexpected data before min keyword (50 | ).", "/gg:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module hh {namespace urn:hh;prefix hh;leaf l {type binary {length \"| 50\";}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - unexpected beginning of the expression (| 50).", "/hh:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ii {namespace urn:ii;prefix ii;leaf l {type binary {length \"10 ..\";}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - unexpected end of the expression after \"..\" (10 ..).", "/ii:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module jj {namespace urn:jj;prefix jj;leaf l {type binary {length \".. 10\";}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - unexpected \"..\" without a lower bound.", "/jj:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module kk {namespace urn:kk;prefix kk;leaf l {type binary {length \"10 |\";}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - unexpected end of the expression (10 |).", "/kk:l");
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module kl {namespace urn:kl;prefix kl;leaf l {type binary {length \"10..20 | 15..30\";}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - values are not in ascending order (15).", "/kl:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ll {namespace urn:ll;prefix ll;typedef mytype {type binary {length 10;}}"
            "leaf l {type mytype {length 11;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - the derived restriction (11) is not equally or more limiting.", "/ll:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module mm {namespace urn:mm;prefix mm;typedef mytype {type binary {length 10..100;}}"
            "leaf l {type mytype {length 1..11;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - the derived restriction (1..11) is not equally or more limiting.", "/mm:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module nn {namespace urn:nn;prefix nn;typedef mytype {type binary {length 10..100;}}"
            "leaf l {type mytype {length 20..110;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - the derived restriction (20..110) is not equally or more limiting.", "/nn:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module oo {namespace urn:oo;prefix oo;typedef mytype {type binary {length 10..100;}}"
            "leaf l {type mytype {length 20..30|110..120;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - the derived restriction (20..30|110..120) is not equally or more limiting.", "/oo:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module pp {namespace urn:pp;prefix pp;typedef mytype {type binary {length 10..11;}}"
            "leaf l {type mytype {length 15;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - the derived restriction (15) is not equally or more limiting.", "/pp:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module qq {namespace urn:qq;prefix qq;typedef mytype {type binary {length 10..20|30..40;}}"
            "leaf l {type mytype {length 15..35;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - the derived restriction (15..35) is not equally or more limiting.", "/qq:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module rr {namespace urn:rr;prefix rr;typedef mytype {type binary {length 10;}}"
            "leaf l {type mytype {length 10..35;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid length restriction - the derived restriction (10..35) is not equally or more limiting.", "/rr:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ss {namespace urn:ss;prefix ss;leaf l {type binary {pattern '[0-9]*';}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid type restrictions for binary type.", "/ss:l");
}

static void
test_type_pattern(void **state)
{
    const struct lys_module *mod;
    struct lysc_type *type;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {yang-version 1.1; namespace urn:a;prefix a;leaf l {type string {"
            "pattern .* {error-app-tag errortag;error-message error;}"
            "pattern [0-9].*[0-9] {modifier invert-match;}}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_str *)type)->patterns);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_str *)type)->patterns));
    assert_string_equal("errortag", ((struct lysc_type_str *)type)->patterns[0]->eapptag);
    assert_string_equal("error", ((struct lysc_type_str *)type)->patterns[0]->emsg);
    assert_string_equal(".*", ((struct lysc_type_str *)type)->patterns[0]->expr);
    assert_int_equal(0, ((struct lysc_type_str *)type)->patterns[0]->inverted);
    assert_null(((struct lysc_type_str *)type)->patterns[1]->eapptag);
    assert_null(((struct lysc_type_str *)type)->patterns[1]->emsg);
    assert_string_equal("[0-9].*[0-9]", ((struct lysc_type_str *)type)->patterns[1]->expr);
    assert_int_equal(1, ((struct lysc_type_str *)type)->patterns[1]->inverted);

    /* new string is tested in file ./tests/utests/types/string.c */
#if 0
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b;typedef mytype {type string {pattern '[0-9]*';}}"
            "typedef mytype2 {type mytype {length 10;}} leaf l {type mytype2 {pattern '[0-4]*';}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_int_equal(1, type->refcount);
    assert_non_null(((struct lysc_type_str *)type)->patterns);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_str *)type)->patterns));
    assert_string_equal("[0-9]*", ((struct lysc_type_str *)type)->patterns[0]->expr);
    assert_int_equal(3, ((struct lysc_type_str *)type)->patterns[0]->refcount);
    assert_string_equal("[0-4]*", ((struct lysc_type_str *)type)->patterns[1]->expr);
    assert_int_equal(1, ((struct lysc_type_str *)type)->patterns[1]->refcount);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module c {namespace urn:c;prefix c;typedef mytype {type string {pattern '[0-9]*';}}"
            "leaf l {type mytype {length 10;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_int_equal(1, type->refcount);
    assert_non_null(((struct lysc_type_str *)type)->patterns);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_str *)type)->patterns));
    assert_string_equal("[0-9]*", ((struct lysc_type_str *)type)->patterns[0]->expr);
    assert_int_equal(2, ((struct lysc_type_str *)type)->patterns[0]->refcount);

    /* test substitutions */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module d {namespace urn:d;prefix d;leaf l {type string {"
            "pattern '^\\p{IsLatinExtended-A}$';}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_non_null(((struct lysc_type_str *)type)->patterns);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_str *)type)->patterns));
    assert_string_equal("^\\p{IsLatinExtended-A}$", ((struct lysc_type_str *)type)->patterns[0]->expr);
#endif

    /* TODO check some data "^ř$" */
}

static void
test_type_enum(void **state)
{
    const struct lys_module *mod;
    struct lysc_type *type;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {yang-version 1.1; namespace urn:a;prefix a;feature f; leaf l {type enumeration {"
            "enum automin; enum min {value -2147483648;}enum one {if-feature f; value 1;}"
            "enum two; enum seven {value 7;}enum eight;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_ENUM, type->basetype);
    assert_non_null(((struct lysc_type_enum *)type)->enums);
    assert_int_equal(5, LY_ARRAY_COUNT(((struct lysc_type_enum *)type)->enums));
    assert_string_equal("automin", ((struct lysc_type_enum *)type)->enums[0].name);
    assert_int_equal(0, ((struct lysc_type_enum *)type)->enums[0].value);
    assert_string_equal("min", ((struct lysc_type_enum *)type)->enums[1].name);
    assert_int_equal(-2147483648, ((struct lysc_type_enum *)type)->enums[1].value);
    assert_string_equal("two", ((struct lysc_type_enum *)type)->enums[2].name);
    assert_int_equal(2, ((struct lysc_type_enum *)type)->enums[2].value);
    assert_string_equal("seven", ((struct lysc_type_enum *)type)->enums[3].name);
    assert_int_equal(7, ((struct lysc_type_enum *)type)->enums[3].value);
    assert_string_equal("eight", ((struct lysc_type_enum *)type)->enums[4].name);
    assert_int_equal(8, ((struct lysc_type_enum *)type)->enums[4].value);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {yang-version 1.1; namespace urn:b;prefix b;feature f; typedef mytype {type enumeration {"
            "enum 11; enum min {value -2147483648;}enum x$&;"
            "enum two; enum seven {value 7;}enum eight;}} leaf l { type mytype {enum seven;enum eight;}}}",
            LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_ENUM, type->basetype);
    assert_non_null(((struct lysc_type_enum *)type)->enums);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_enum *)type)->enums));
    assert_string_equal("seven", ((struct lysc_type_enum *)type)->enums[0].name);
    assert_int_equal(7, ((struct lysc_type_enum *)type)->enums[0].value);
    assert_string_equal("eight", ((struct lysc_type_enum *)type)->enums[1].name);
    assert_int_equal(8, ((struct lysc_type_enum *)type)->enums[1].value);

    const char *new_module = "module moc_c {yang-version 1.1; namespace urn:moc_c;prefix moc_c;feature f; typedef mytype {type enumeration {"
            "enum first{value -270;} enum second; enum third {value -400;} enum fourth;}} leaf l { type mytype;}}";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, new_module, LYS_IN_YANG, &mod));

    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_ENUM, type->basetype);
    assert_non_null(((struct lysc_type_enum *)type)->enums);
    assert_int_equal(4, LY_ARRAY_COUNT(((struct lysc_type_enum *)type)->enums));
    assert_string_equal("first", ((struct lysc_type_enum *)type)->enums[0].name);
    assert_int_equal(-270, ((struct lysc_type_enum *)type)->enums[0].value);
    assert_string_equal("second", ((struct lysc_type_enum *)type)->enums[1].name);
    assert_int_equal(-269, ((struct lysc_type_enum *)type)->enums[1].value);
    assert_string_equal("third", ((struct lysc_type_enum *)type)->enums[2].name);
    assert_int_equal(-400, ((struct lysc_type_enum *)type)->enums[2].value);
    assert_string_equal("fourth", ((struct lysc_type_enum *)type)->enums[3].name);
    assert_int_equal(-268, ((struct lysc_type_enum *)type)->enums[3].value);

    new_module = "module moc_d {yang-version 1.1; namespace urn:moc_d;prefix moc_d;feature f; typedef mytype {type enumeration {"
            "enum first; enum second;}} leaf l { type mytype;}}";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, new_module, LYS_IN_YANG, &mod));

    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_ENUM, type->basetype);
    assert_non_null(((struct lysc_type_enum *)type)->enums);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_enum *)type)->enums));
    assert_string_equal("first", ((struct lysc_type_enum *)type)->enums[0].name);
    assert_int_equal(0, ((struct lysc_type_enum *)type)->enums[0].value);
    assert_string_equal("second", ((struct lysc_type_enum *)type)->enums[1].name);
    assert_int_equal(1, ((struct lysc_type_enum *)type)->enums[1].value);

    /* invalid cases */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; feature f; leaf l {type enumeration {"
            "enum one {if-feature f;}}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid keyword \"if-feature\" as a child of \"enum\" - the statement is allowed only in YANG 1.1 modules.", "Line number 1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type enumeration {"
            "enum one {value -2147483649;}}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid value \"-2147483649\" of \"value\".", "Line number 1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type enumeration {"
            "enum one {value 2147483648;}}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid value \"2147483648\" of \"value\".", "Line number 1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type enumeration {"
            "enum one; enum one;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Duplicate identifier \"one\" of enum statement.", "Line number 1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type enumeration {"
            "enum '';}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Enum name must not be zero-length.", "Line number 1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type enumeration {"
            "enum ' x';}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Enum name must not have any leading or trailing whitespaces (\" x\").", "Line number 1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type enumeration {"
            "enum 'x ';}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Enum name must not have any leading or trailing whitespaces (\"x \").", "Line number 1.");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type enumeration {"
            "enum 'inva\nlid';}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Control characters in enum name should be avoided (\"inva\nlid\", character number 5).", NULL);

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module bb {namespace urn:bb;prefix bb; leaf l {type enumeration;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Missing enum substatement for enumeration type.", "/bb:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module cc {yang-version 1.1;namespace urn:cc;prefix cc;typedef mytype {type enumeration {enum one;}}"
            "leaf l {type mytype {enum two;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid enumeration - derived type adds new item \"two\".", "/cc:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module dd {yang-version 1.1;namespace urn:dd;prefix dd;typedef mytype {type enumeration {enum one;}}"
            "leaf l {type mytype {enum one {value 1;}}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid enumeration - value of the item \"one\" has changed from 0 to 1 in the derived type.", "/dd:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ee {namespace urn:ee;prefix ee;leaf l {type enumeration {enum x {value 2147483647;}enum y;}}}",
            LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid enumeration - it is not possible to auto-assign enum value for \"y\" since the highest value is already 2147483647.", "/ee:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ff {namespace urn:ff;prefix ff;leaf l {type enumeration {enum x {value 1;}enum y {value 1;}}}}",
            LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid enumeration - value 1 collide in items \"y\" and \"x\".", "/ff:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module gg {namespace urn:gg;prefix gg;typedef mytype {type enumeration;}"
            "leaf l {type mytype {enum one;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Missing enum substatement for enumeration type mytype.", "/gg:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module hh {namespace urn:hh;prefix hh; typedef mytype {type enumeration {enum one;}}"
            "leaf l {type mytype {enum one;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Enumeration type can be subtyped only in YANG 1.1 modules.", "/hh:l");
}

static void
test_type_bits(void **state)
{
    const struct lys_module *mod;
    struct lysc_type *type;

    /* type bits is now tested in file type/bits.c */
#if 0
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {yang-version 1.1; namespace urn:a;prefix a;feature f; leaf l {type bits {"
            "bit automin; bit one {if-feature f; position 1;}"
            "bit two; bit seven {position 7;} bit five {position 5;} bit eight;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_BITS, type->basetype);
    assert_non_null(((struct lysc_type_bits *)type)->bits);
    assert_int_equal(5, LY_ARRAY_COUNT(((struct lysc_type_bits *)type)->bits));
    assert_string_equal("automin", ((struct lysc_type_bits *)type)->bits[0].name);
    assert_int_equal(0, ((struct lysc_type_bits *)type)->bits[0].position);
    assert_string_equal("two", ((struct lysc_type_bits *)type)->bits[1].name);
    assert_int_equal(2, ((struct lysc_type_bits *)type)->bits[1].position);
    assert_string_equal("seven", ((struct lysc_type_bits *)type)->bits[2].name);
    assert_int_equal(7, ((struct lysc_type_bits *)type)->bits[2].position);
    assert_string_equal("five", ((struct lysc_type_bits *)type)->bits[3].name);
    assert_int_equal(5, ((struct lysc_type_bits *)type)->bits[3].position);
    assert_string_equal("eight", ((struct lysc_type_bits *)type)->bits[4].name);
    assert_int_equal(8, ((struct lysc_type_bits *)type)->bits[4].position);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {yang-version 1.1;namespace urn:b;prefix b;feature f; typedef mytype {type bits {"
            "bit automin; bit one;bit two; bit seven {position 7;}bit eight;}} leaf l { type mytype {bit eight;bit seven;bit automin;}}}",
            LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_BITS, type->basetype);
    assert_non_null(((struct lysc_type_bits *)type)->bits);
    assert_int_equal(3, LY_ARRAY_COUNT(((struct lysc_type_bits *)type)->bits));
    assert_string_equal("automin", ((struct lysc_type_bits *)type)->bits[0].name);
    assert_int_equal(0, ((struct lysc_type_bits *)type)->bits[0].position);
    assert_string_equal("seven", ((struct lysc_type_bits *)type)->bits[1].name);
    assert_int_equal(7, ((struct lysc_type_bits *)type)->bits[1].position);
    assert_string_equal("eight", ((struct lysc_type_bits *)type)->bits[2].name);
    assert_int_equal(8, ((struct lysc_type_bits *)type)->bits[2].position);
#endif

    /* invalid cases */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; feature f; leaf l {type bits {"
            "bit one {if-feature f;}}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid keyword \"if-feature\" as a child of \"bit\" - the statement is allowed only in YANG 1.1 modules.", "Line number 1.");

#if 0
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type bits {"
            "bit one {position -1;}}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid value \"-1\" of \"position\".", "Line number 1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type bits {"
            "bit one {position 4294967296;}}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid value \"4294967296\" of \"position\".", "Line number 1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type bits {"
            "bit one; bit one;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Duplicate identifier \"one\" of bit statement.", "Line number 1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type bits {"
            "bit '11';}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid identifier first character '1' (0x0031).", "Line number 1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type bits {"
            "bit 'x1$1';}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid identifier character '$' (0x0024).", "Line number 1.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module bb {namespace urn:bb;prefix bb; leaf l {type bits;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Missing bit substatement for bits type.", "/bb:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module cc {yang-version 1.1;namespace urn:cc;prefix cc;typedef mytype {type bits {bit one;}}"
            "leaf l {type mytype {bit two;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid bits - derived type adds new item \"two\".", "/cc:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module dd {yang-version 1.1;namespace urn:dd;prefix dd;typedef mytype {type bits {bit one;}}"
            "leaf l {type mytype {bit one {position 1;}}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid bits - position of the item \"one\" has changed from 0 to 1 in the derived type.", "/dd:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ee {namespace urn:ee;prefix ee;leaf l {type bits {bit x {position 4294967295;}bit y;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid bits - it is not possible to auto-assign bit position for \"y\" since the highest value is already 4294967295.", "/ee:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ff {namespace urn:ff;prefix ff;leaf l {type bits {bit x {position 1;}bit y {position 1;}}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid bits - position 1 collide in items \"y\" and \"x\".", "/ff:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module gg {namespace urn:gg;prefix gg;typedef mytype {type bits;}"
            "leaf l {type mytype {bit one;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Missing bit substatement for bits type mytype.", "/gg:l");
#endif

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module hh {namespace urn:hh;prefix hh; typedef mytype {type bits {bit one;}}"
            "leaf l {type mytype {bit one;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Bits type can be subtyped only in YANG 1.1 modules.", "/hh:l");

}

static void
test_type_dec64(void **state)
{
    const struct lys_module *mod;
    struct lysc_type *type;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {namespace urn:a;prefix a;leaf l {type decimal64 {"
            "fraction-digits 2;range min..max;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_DEC64, type->basetype);
    assert_int_equal(2, ((struct lysc_type_dec *)type)->fraction_digits);
    assert_non_null(((struct lysc_type_dec *)type)->range);
    assert_non_null(((struct lysc_type_dec *)type)->range->parts);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_dec *)type)->range->parts));
    assert_int_equal(INT64_C(-9223372036854775807) - INT64_C(1), ((struct lysc_type_dec *)type)->range->parts[0].min_64);
    assert_int_equal(INT64_C(9223372036854775807), ((struct lysc_type_dec *)type)->range->parts[0].max_64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b;typedef mytype {type decimal64 {"
            "fraction-digits 2;range '3.14 | 5.1 | 10';}}leaf l {type mytype;}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_DEC64, type->basetype);
    assert_int_equal(2, ((struct lysc_type_dec *)type)->fraction_digits);
    assert_non_null(((struct lysc_type_dec *)type)->range);
    assert_non_null(((struct lysc_type_dec *)type)->range->parts);
    assert_int_equal(3, LY_ARRAY_COUNT(((struct lysc_type_dec *)type)->range->parts));
    assert_int_equal(314, ((struct lysc_type_dec *)type)->range->parts[0].min_64);
    assert_int_equal(314, ((struct lysc_type_dec *)type)->range->parts[0].max_64);
    assert_int_equal(510, ((struct lysc_type_dec *)type)->range->parts[1].min_64);
    assert_int_equal(510, ((struct lysc_type_dec *)type)->range->parts[1].max_64);
    assert_int_equal(1000, ((struct lysc_type_dec *)type)->range->parts[2].min_64);
    assert_int_equal(1000, ((struct lysc_type_dec *)type)->range->parts[2].max_64);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module c {namespace urn:c;prefix c;typedef mytype {type decimal64 {"
            "fraction-digits 2;range '1 .. 65535';}}leaf l {type mytype;}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_int_equal(LY_TYPE_DEC64, type->basetype);
    assert_int_equal(2, ((struct lysc_type_dec *)type)->fraction_digits);
    assert_non_null(((struct lysc_type_dec *)type)->range);
    assert_non_null(((struct lysc_type_dec *)type)->range->parts);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_dec *)type)->range->parts));
    assert_int_equal(100, ((struct lysc_type_dec *)type)->range->parts[0].min_64);
    assert_int_equal(6553500, ((struct lysc_type_dec *)type)->range->parts[0].max_64);

    /* invalid cases */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type decimal64 {fraction-digits 0;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid value \"0\" of \"fraction-digits\".", "Line number 1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type decimal64 {fraction-digits -1;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid value \"-1\" of \"fraction-digits\".", "Line number 1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type decimal64 {fraction-digits 19;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Value \"19\" is out of \"fraction-digits\" bounds.", "Line number 1.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type decimal64;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Missing fraction-digits substatement for decimal64 type.", "/aa:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ab {namespace urn:ab;prefix ab; typedef mytype {type decimal64;}leaf l {type mytype;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Missing fraction-digits substatement for decimal64 type mytype.", "/ab:l");

    assert_int_equal(LY_EINVAL, lys_parse_mem(UTEST_LYCTX, "module bb {namespace urn:bb;prefix bb; leaf l {type decimal64 {fraction-digits 2;"
            "range '3.142';}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Range boundary \"3.142\" of decimal64 type exceeds defined number (2) of fraction digits.", "/bb:l");

    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module cc {namespace urn:cc;prefix cc; leaf l {type decimal64 {fraction-digits 2;"
            "range '4 | 3.14';}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid range restriction - values are not in ascending order (3.14).", "/cc:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module dd {namespace urn:dd;prefix dd; typedef mytype {type decimal64 {fraction-digits 2;}}"
            "leaf l {type mytype {fraction-digits 3;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid fraction-digits substatement for type not directly derived from decimal64 built-in type.", "/dd:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module de {namespace urn:de;prefix de; typedef mytype {type decimal64 {fraction-digits 2;}}"
            "typedef mytype2 {type mytype {fraction-digits 3;}}leaf l {type mytype2;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid fraction-digits substatement for type \"mytype2\" not directly derived from decimal64 built-in type.", "/de:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ee {namespace urn:c;prefix c;typedef mytype {type decimal64 {"
            "fraction-digits 18;range '-10 .. 0';}}leaf l {type mytype;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid range restriction - invalid value \"-10000000000000000000\".", "/ee:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ee {namespace urn:c;prefix c;typedef mytype {type decimal64 {"
            "fraction-digits 18;range '0 .. 10';}}leaf l {type mytype;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid range restriction - invalid value \"10000000000000000000\".", "/ee:l");
}

static void
test_type_instanceid(void **state)
{
    const struct lys_module *mod;
    struct lysc_type *type;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {namespace urn:a;prefix a;typedef mytype {type instance-identifier {require-instance false;}}"
            "leaf l1 {type instance-identifier {require-instance true;}}"
            "leaf l2 {type mytype;} leaf l3 {type instance-identifier;}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_INST, type->basetype);
    assert_int_equal(1, ((struct lysc_type_instanceid *)type)->require_instance);

    type = ((struct lysc_node_leaf *)mod->compiled->data->next)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_INST, type->basetype);
    assert_int_equal(0, ((struct lysc_type_instanceid *)type)->require_instance);

    type = ((struct lysc_node_leaf *)mod->compiled->data->next->next)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_INST, type->basetype);
    assert_int_equal(1, ((struct lysc_type_instanceid *)type)->require_instance);

    /* invalid cases */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type instance-identifier {require-instance yes;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid value \"yes\" of \"require-instance\".", "Line number 1.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type instance-identifier {fraction-digits 1;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid type restrictions for instance-identifier type.", "/aa:l");
}

static void
test_type_identityref(void **state)
{
    const struct lys_module *mod;
    struct lysc_type *type;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {yang-version 1.1;namespace urn:a;prefix a;identity i; identity j; identity k {base i;}"
            "typedef mytype {type identityref {base i;}}"
            "leaf l1 {type mytype;} leaf l2 {type identityref {base a:k; base j;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_IDENT, type->basetype);
    assert_non_null(((struct lysc_type_identityref *)type)->bases);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_type_identityref *)type)->bases));
    assert_string_equal("i", ((struct lysc_type_identityref *)type)->bases[0]->name);

    type = ((struct lysc_node_leaf *)mod->compiled->data->next)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_IDENT, type->basetype);
    assert_non_null(((struct lysc_type_identityref *)type)->bases);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_identityref *)type)->bases));
    assert_string_equal("k", ((struct lysc_type_identityref *)type)->bases[0]->name);
    assert_string_equal("j", ((struct lysc_type_identityref *)type)->bases[1]->name);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {yang-version 1.1;namespace urn:b;prefix b;import a {prefix a;}"
            "leaf l {type identityref {base a:k; base a:j;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_IDENT, type->basetype);
    assert_non_null(((struct lysc_type_identityref *)type)->bases);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_identityref *)type)->bases));
    assert_string_equal("k", ((struct lysc_type_identityref *)type)->bases[0]->name);
    assert_string_equal("j", ((struct lysc_type_identityref *)type)->bases[1]->name);

    /* invalid cases */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; leaf l {type identityref;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Missing base substatement for identityref type.", "/aa:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module bb {namespace urn:bb;prefix bb; typedef mytype {type identityref;}"
            "leaf l {type mytype;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Missing base substatement for identityref type mytype.", "/bb:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module cc {namespace urn:cc;prefix cc; identity i; typedef mytype {type identityref {base i;}}"
            "leaf l {type mytype {base i;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid base substatement for the type not directly derived from identityref built-in type.", "/cc:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module dd {namespace urn:dd;prefix dd; identity i; typedef mytype {type identityref {base i;}}"
            "typedef mytype2 {type mytype {base i;}}leaf l {type mytype2;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid base substatement for the type \"mytype2\" not directly derived from identityref built-in type.", "/dd:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ee {namespace urn:ee;prefix ee; identity i; identity j;"
            "leaf l {type identityref {base i;base j;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Multiple bases in identityref type are allowed only in YANG 1.1 modules.", "/ee:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ff {namespace urn:ff;prefix ff; identity i;leaf l {type identityref {base j;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Unable to find base (j) of identityref.", "/ff:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module gg {namespace urn:gg;prefix gg;leaf l {type identityref {base x:j;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid prefix used for base (x:j) of identityref.", "/gg:l");
}

static void
test_type_leafref(void **state)
{
    const struct lys_module *mod;
    struct lysc_type *type;
    const char *path;
    struct lyxp_expr *expr;

    /* lys_path_parse() */
    path = "invalid_path";
    assert_int_equal(LY_EVALID, ly_path_parse(UTEST_LYCTX, NULL, path, strlen(path), LY_PATH_BEGIN_EITHER, LY_PATH_LREF_TRUE,
            LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_LEAFREF, &expr));
    path = "..";
    assert_int_equal(LY_EVALID, ly_path_parse(UTEST_LYCTX, NULL, path, strlen(path), LY_PATH_BEGIN_EITHER, LY_PATH_LREF_TRUE,
            LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_LEAFREF, &expr));
    path = "..[";
    assert_int_equal(LY_EVALID, ly_path_parse(UTEST_LYCTX, NULL, path, strlen(path), LY_PATH_BEGIN_EITHER, LY_PATH_LREF_TRUE,
            LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_LEAFREF, &expr));
    path = "../";
    assert_int_equal(LY_EVALID, ly_path_parse(UTEST_LYCTX, NULL, path, strlen(path), LY_PATH_BEGIN_EITHER, LY_PATH_LREF_TRUE,
            LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_LEAFREF, &expr));
    path = "/";
    assert_int_equal(LY_EVALID, ly_path_parse(UTEST_LYCTX, NULL, path, strlen(path), LY_PATH_BEGIN_EITHER, LY_PATH_LREF_TRUE,
            LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_LEAFREF, &expr));

    path = "../../pref:id/xxx[predicate]/invalid!!!";
    assert_int_equal(LY_EVALID, ly_path_parse(UTEST_LYCTX, NULL, path, strlen(path), LY_PATH_BEGIN_EITHER, LY_PATH_LREF_TRUE,
            LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_LEAFREF, &expr));
    CHECK_LOG_CTX("Invalid character 0x21 ('!'), perhaps \"invalid\" is supposed to be a function call.", NULL);

    path = "/absolute/prefix:path";
    assert_int_equal(LY_SUCCESS, ly_path_parse(UTEST_LYCTX, NULL, path, strlen(path), LY_PATH_BEGIN_EITHER, LY_PATH_LREF_TRUE,
            LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_LEAFREF, &expr));
    assert_int_equal(4, expr->used);
    assert_int_equal(LYXP_TOKEN_OPER_PATH, expr->tokens[0]);
    assert_int_equal(LYXP_TOKEN_NAMETEST, expr->tokens[1]);
    assert_int_equal(LYXP_TOKEN_OPER_PATH, expr->tokens[2]);
    assert_int_equal(LYXP_TOKEN_NAMETEST, expr->tokens[3]);
    lyxp_expr_free(UTEST_LYCTX, expr);

    /* complete leafref paths */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {yang-version 1.1;namespace urn:a;prefix a;"
            "leaf ref1 {type leafref {path /a:target1;}} leaf ref2 {type leafref {path /a/target2; require-instance false;}}"
            "leaf target1 {type string;}container a {leaf target2 {type uint8;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("/a:target1", ((struct lysc_type_leafref *)type)->path->expr);
    assert_ptr_equal(mod, ly_resolve_prefix(UTEST_LYCTX, "a", 1, LY_VALUE_SCHEMA_RESOLVED, ((struct lysc_type_leafref *)type)->prefixes));
    assert_non_null(((struct lysc_type_leafref *)type)->realtype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_leafref *)type)->realtype->basetype);
    assert_int_equal(1, ((struct lysc_type_leafref *)type)->require_instance);
    type = ((struct lysc_node_leaf *)mod->compiled->data->next)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("/a/target2", ((struct lysc_type_leafref *)type)->path->expr);
    assert_int_equal(0, LY_ARRAY_COUNT(((struct lysc_type_leafref *)type)->prefixes));
    assert_non_null(((struct lysc_type_leafref *)type)->realtype);
    assert_int_equal(LY_TYPE_UINT8, ((struct lysc_type_leafref *)type)->realtype->basetype);
    assert_int_equal(0, ((struct lysc_type_leafref *)type)->require_instance);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b; typedef mytype {type leafref {path /b:target;}}"
            "typedef mytype2 {type mytype;} typedef mytype3 {type leafref {path /target;}} leaf ref {type mytype2;}"
            "leaf target {type leafref {path ../realtarget;}} leaf realtarget {type string;}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("/b:target", ((struct lysc_type_leafref *)type)->path->expr);
    assert_ptr_equal(mod, ly_resolve_prefix(UTEST_LYCTX, "b", 1, LY_VALUE_SCHEMA_RESOLVED, ((struct lysc_type_leafref *)type)->prefixes));
    assert_non_null(((struct lysc_type_leafref *)type)->realtype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_leafref *)type)->realtype->basetype);
    assert_int_equal(1, ((struct lysc_type_leafref *)type)->require_instance);

    /* prefixes are reversed to check using correct context of the path! */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module c {yang-version 1.1;namespace urn:c;prefix b; import b {prefix c;}"
            "typedef mytype3 {type c:mytype {require-instance false;}}"
            "leaf ref1 {type b:mytype3;}leaf ref2 {type c:mytype2;}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("/b:target", ((struct lysc_type_leafref *)type)->path->expr);
    assert_ptr_not_equal(mod, ly_resolve_prefix(UTEST_LYCTX, "b", 1, LY_VALUE_SCHEMA_RESOLVED, ((struct lysc_type_leafref *)type)->prefixes));
    assert_non_null(((struct lysc_type_leafref *)type)->realtype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_leafref *)type)->realtype->basetype);
    assert_int_equal(0, ((struct lysc_type_leafref *)type)->require_instance);
    type = ((struct lysc_node_leaf *)mod->compiled->data->next)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("/b:target", ((struct lysc_type_leafref *)type)->path->expr);
    assert_ptr_not_equal(mod, ly_resolve_prefix(UTEST_LYCTX, "b", 1, LY_VALUE_SCHEMA_RESOLVED, ((struct lysc_type_leafref *)type)->prefixes));
    assert_int_equal(1, ((struct lysc_type_leafref *)type)->require_instance);

    /* non-prefixed nodes in path are supposed to be from the module where the leafref type is instantiated */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module d {namespace urn:d;prefix d; import b {prefix b;}"
            "leaf ref {type b:mytype3;}leaf target {type int8;}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("/target", ((struct lysc_type_leafref *)type)->path->expr);
    assert_int_equal(0, LY_ARRAY_COUNT(((struct lysc_type_leafref *)type)->prefixes));
    assert_non_null(((struct lysc_type_leafref *)type)->realtype);
    assert_int_equal(LY_TYPE_INT8, ((struct lysc_type_leafref *)type)->realtype->basetype);
    assert_int_equal(1, ((struct lysc_type_leafref *)type)->require_instance);

    /* conditional leafrefs */
    /*assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module e {yang-version 1.1;namespace urn:e;prefix e;feature f1; feature f2;"
                                        "leaf ref1 {if-feature 'f1 and f2';type leafref {path /target;}}"
                                        "leaf target {if-feature f1; type boolean;}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf*)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("/target", ((struct lysc_type_leafref* )type)->path->expr);
    assert_int_equal(0, LY_ARRAY_COUNT(((struct lysc_type_leafref*)type)->prefixes));
    assert_non_null(((struct lysc_type_leafref*)type)->realtype);
    assert_int_equal(LY_TYPE_BOOL, ((struct lysc_type_leafref*)type)->realtype->basetype);*/

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module f {namespace urn:f;prefix f;"
            "list interface{key name;leaf name{type string;}list address {key ip;leaf ip {type string;}}}"
            "container default-address{leaf ifname{type leafref{ path \"../../interface/name\";}}"
            "leaf address {type leafref{ path \"../../interface[  name = current()/../ifname ]/address/ip\";}}}}",
            LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)(*lysc_node_child_p(mod->compiled->data->prev))->prev)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("../../interface[  name = current()/../ifname ]/address/ip",
            ((struct lysc_type_leafref *)type)->path->expr);
    assert_int_equal(0, LY_ARRAY_COUNT(((struct lysc_type_leafref *)type)->prefixes));
    assert_non_null(((struct lysc_type_leafref *)type)->realtype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_leafref *)type)->realtype->basetype);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module g {namespace urn:g;prefix g;"
            "leaf source {type leafref {path \"/endpoint-parent[id=current()/../field]/endpoint/name\";}}"
            "leaf field {type int32;}list endpoint-parent {key id;leaf id {type int32;}"
            "list endpoint {key name;leaf name {type string;}}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("/endpoint-parent[id=current()/../field]/endpoint/name", ((struct lysc_type_leafref *)type)->path->expr);
    assert_int_equal(0, LY_ARRAY_COUNT(((struct lysc_type_leafref *)type)->prefixes));
    assert_non_null(((struct lysc_type_leafref *)type)->realtype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_leafref *)type)->realtype->basetype);

    /* leafref to imported (not yet implemented) module */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module h-imp {namespace urn:h-imp;prefix h-imp;"
            "leaf l {type string;}}", LYS_IN_YANG, NULL));
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module h {namespace urn:h;prefix h;import h-imp {prefix hi;}"
            "leaf h {type uint16;}}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module i {namespace urn:i;prefix i;import h {prefix h;}"
            "leaf i {type leafref {path /h:h;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_non_null(((struct lysc_type_leafref *)type)->realtype);
    assert_int_equal(LY_TYPE_UINT16, ((struct lysc_type_leafref *)type)->realtype->basetype);
    assert_non_null(mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "h"));
    assert_int_equal(1, mod->implemented);
    assert_non_null(mod->compiled->data);
    assert_string_equal("h", mod->compiled->data->name);

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module j {namespace urn:j;prefix j; leaf j  {type string;}}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module k {namespace urn:k;prefix k;import j {prefix j;}"
            "leaf i {type leafref {path \"/ilist[name = current()/../j:j]/value\";}}"
            "list ilist {key name; leaf name {type string;} leaf value {type uint16;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_non_null(((struct lysc_type_leafref *)type)->realtype);
    assert_int_equal(LY_TYPE_UINT16, ((struct lysc_type_leafref *)type)->realtype->basetype);
    assert_non_null(mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "j"));
    assert_int_equal(1, mod->implemented);
    assert_non_null(mod->compiled->data);
    assert_string_equal("j", mod->compiled->data->name);

    /* leafref with a default value */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module l {namespace urn:l;prefix l;"
            "leaf source {type leafref {path \"../target\";}default true;}"
            "leaf target {type boolean;}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_LEAFREF, type->basetype);
    assert_string_equal("../target", ((struct lysc_type_leafref *)type)->path->expr);
    assert_non_null(((struct lysc_type_leafref *)type)->realtype);
    assert_int_equal(LY_TYPE_BOOL, ((struct lysc_type_leafref *)type)->realtype->basetype);
    assert_non_null(((struct lysc_node_leaf *)mod->compiled->data)->dflt);
    assert_int_equal(LY_TYPE_BOOL, ((struct lysc_node_leaf *)mod->compiled->data)->dflt->realtype->basetype);
    assert_int_equal(1, ((struct lysc_node_leaf *)mod->compiled->data)->dflt->boolean);

    /* invalid paths */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa;container a {leaf target2 {type uint8;}}"
            "leaf ref1 {type leafref {path ../a/invalid;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Not found node \"invalid\" in path.", "Schema location /aa:ref1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module bb {namespace urn:bb;prefix bb;container a {leaf target2 {type uint8;}}"
            "leaf ref1 {type leafref {path ../../toohigh;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Too many parent references in path.", "Schema location /bb:ref1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module cc {namespace urn:cc;prefix cc;container a {leaf target2 {type uint8;}}"
            "leaf ref1 {type leafref {path /a:invalid;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("No module connected with the prefix \"a\" found (prefix format schema stored mapping).", "Schema location /cc:ref1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module dd {namespace urn:dd;prefix dd;leaf target1 {type string;}"
            "container a {leaf target2 {type uint8;}} leaf ref1 {type leafref {"
            "path '/a[target2 = current()/../target1]/target2';}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("List predicate defined for container \"a\" in path.", "Schema location /dd:ref1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ee {namespace urn:ee;prefix ee;\n  container a {leaf target2 {type uint8;}}\n"
            "leaf ref1 {type leafref {path /a!invalid;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid character 0x21 ('!'), perhaps \"a\" is supposed to be a function call.", "Line number 3.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ff {namespace urn:ff;prefix ff;container a {leaf target2 {type uint8;}}"
            "leaf ref1 {type leafref {path /a;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid leafref path \"/a\" - target node is container instead of leaf or leaf-list.", "Schema location /ff:ref1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module gg {namespace urn:gg;prefix gg;container a {leaf target2 {type uint8;"
            "status deprecated;}} leaf ref1 {type leafref {path /a/target2;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("A current definition \"ref1\" is not allowed to reference deprecated definition \"target2\".", "Schema location /gg:ref1.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module hh {namespace urn:hh;prefix hh;"
            "leaf ref1 {type leafref;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Missing path substatement for leafref type.", "/hh:ref1");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ii {namespace urn:ii;prefix ii;typedef mytype {type leafref;}"
            "leaf ref1 {type mytype;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Missing path substatement for leafref type mytype.", "/ii:ref1");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module kk {namespace urn:kk;prefix kk;"
            "leaf ref {type leafref {path /target;}}leaf target {type string;config false;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid leafref path \"/target\" - target is supposed to represent configuration data (as the leafref does), but it does not.", "Schema location /kk:ref.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ll {namespace urn:ll;prefix ll;"
            "leaf ref {type leafref {path /target; require-instance true;}}leaf target {type string;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Leafref type can be restricted by require-instance statement only in YANG 1.1 modules.", "/ll:ref");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module mm {namespace urn:mm;prefix mm;typedef mytype {type leafref {path /target;require-instance false;}}"
            "leaf ref {type mytype;}leaf target {type string;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Leafref type \"mytype\" can be restricted by require-instance statement only in YANG 1.1 modules.", "/mm:ref");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module nn {namespace urn:nn;prefix nn;\n"
            "list interface{key name;leaf name{type string;}leaf ip {type string;}}\n"
            "leaf ifname{type leafref{ path \"../interface/name\";}}\n"
            "leaf address {type leafref{\n path \"/interface[name is current()/../ifname]/ip\";}}}",
            LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid character 0x69 ('i'), perhaps \"name\" is supposed to be a function call.", "Line number 5.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module oo {namespace urn:oo;prefix oo;\n"
            "list interface{key name;leaf name{type string;}leaf ip {type string;}}\n"
            "leaf ifname{type leafref{ path \"../interface/name\";}}\n"
            "leaf address {type leafref{\n path \"/interface[name=current()/../ifname/ip\";}}}",
            LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Unexpected XPath expression end.", "Line number 5.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module pp {namespace urn:pp;prefix pp;"
            "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
            "leaf ifname{type leafref{ path \"../interface/name\";}}"
            "leaf address {type leafref{ path \"/interface[x:name=current()/../ifname]/ip\";}}}",
            LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("No module connected with the prefix \"x\" found (prefix format schema stored mapping).", "Schema location /pp:address.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module qq {namespace urn:qq;prefix qq;"
            "list interface{key name;leaf name{type string;}leaf ip {type string;}}"
            "leaf ifname{type leafref{ path \"../interface/name\";}}"
            "leaf address {type leafref{ path \"/interface[id=current()/../ifname]/ip\";}}}",
            LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Not found node \"id\" in path.", "Schema location /qq:address.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module rr {namespace urn:rr;prefix rr;\n"
            "list interface{key name;leaf name{type string;}leaf ip {type string;}}\n"
            "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}\n"
            "leaf address {type leafref{ path \"/interface[name=current() /  .. / ifname][name=current()/../test]/ip\";}}}",
            LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Duplicate predicate key \"name\" in path.", "Line number 4.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ss {namespace urn:ss;prefix ss;\n"
            "list interface{key name;leaf name{type string;}leaf ip {type string;}}\n"
            "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}\n"
            "leaf address {type leafref{ path \"/interface[name = ../ifname]/ip\";}}}",
            LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Unexpected XPath token \"..\" (\"../ifname]/ip\"), expected \"FunctionName\".", "Line number 4.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module tt {namespace urn:tt;prefix tt;\n"
            "list interface{key name;leaf name{type string;}leaf ip {type string;}}\n"
            "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}\n"
            "leaf address {type leafref{ path \"/interface[name = current()../ifname]/ip\";}}}",
            LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Unexpected XPath token \"..\" (\"../ifname]/ip\"), expected \"]\".", "Line number 4.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module uu {namespace urn:uu;prefix uu;\n"
            "list interface{key name;leaf name{type string;}leaf ip {type string;}}\n"
            "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}\n"
            "leaf address {type leafref{ path \"/interface[name = current()/..ifname]/ip\";}}}",
            LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid character 'i'[31] of expression '/interface[name = current()/..ifname]/ip'.", "Line number 4.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module vv {namespace urn:vv;prefix vv;\n"
            "list interface{key name;leaf name{type string;}leaf ip {type string;}}\n"
            "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}\n"
            "leaf address {type leafref{ path \"/interface[name = current()/ifname]/ip\";}}}",
            LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Unexpected XPath token \"NameTest\" (\"ifname]/ip\"), expected \"..\".", "Line number 4.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ww {namespace urn:ww;prefix ww;\n"
            "list interface{key name;leaf name{type string;}leaf ip {type string;}}\n"
            "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}\n"
            "leaf address {type leafref{ path \"/interface[name = current()/../]/ip\";}}}",
            LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Unexpected XPath token \"]\" (\"]/ip\").", "Line number 4.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module xx {namespace urn:xx;prefix xx;\n"
            "list interface{key name;leaf name{type string;}leaf ip {type string;}}\n"
            "leaf ifname{type leafref{ path \"../interface/name\";}}leaf test{type string;}\n"
            "leaf address {type leafref{ path \"/interface[name = current()/../$node]/ip\";}}}",
            LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid character 0x24 ('$'), perhaps \"\" is supposed to be a function call.", "Line number 4.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module yy {namespace urn:yy;prefix yy;\n"
            "list interface{key name;leaf name{type string;}leaf ip {type string;}}\n"
            "leaf ifname{type leafref{ path \"../interface/name\";}}\n"
            "leaf address {type leafref{ path \"/interface[name=current()/../x:ifname]/ip\";}}}",
            LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("No module connected with the prefix \"x\" found (prefix format schema stored mapping).", "Schema location /yy:address.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module zz {namespace urn:zz;prefix zz;\n"
            "list interface{key name;leaf name{type string;}leaf ip {type string;}}\n"
            "leaf ifname{type leafref{ path \"../interface/name\";}}\n"
            "leaf address {type leafref{ path \"/interface[name=current()/../xxx]/ip\";}}}",
            LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Not found node \"xxx\" in path.", "Schema location /zz:address.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module zza {namespace urn:zza;prefix zza;\n"
            "list interface{key name;leaf name{type string;}leaf ip {type string;}}\n"
            "leaf ifname{type leafref{ path \"../interface/name\";}}container c;\n"
            "leaf address {type leafref{ path \"/interface[name=current()/../c]/ip\";}}}",
            LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Leaf expected instead of container \"c\" in leafref predicate in path.", "Schema location /zza:address.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module zzb {namespace urn:zzb;prefix zzb;\n"
            "list interface{key name;leaf name{type string;}leaf ip {type string;}container c;}\n"
            "leaf ifname{type leafref{ path \"../interface/name\";}}\n"
            "leaf address {type leafref{ path \"/interface[c=current()/../ifname]/ip\";}}}",
            LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Key expected instead of container \"c\" in path.", "Schema location /zzb:address.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module zzc {namespace urn:zzc;prefix zzc;\n"
            "leaf source {type leafref {path \"../target\";}default true;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Not found node \"target\" in path.", "Schema location /zzc:source.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module zzd {namespace urn:zzd;prefix zzd;\n"
            "leaf source {type leafref {path \"../target\";}default true;}\n"
            "leaf target {type uint8;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Invalid uint8 value \"true\".).", "Schema location /zzd:source.");

    /* circular chain */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aaa {namespace urn:aaa;prefix aaa;\n"
            "leaf ref1 {type leafref {path /ref2;}}\n"
            "leaf ref2 {type leafref {path /ref3;}}\n"
            "leaf ref3 {type leafref {path /ref4;}}\n"
            "leaf ref4 {type leafref {path /ref1;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid leafref path \"/ref1\" - circular chain of leafrefs detected.", "Schema location /aaa:ref4.");
}

static void
test_type_empty(void **state)
{

    /* invalid */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa;"
            "leaf l {type empty; default x;}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Invalid empty value length 1.).", "Schema location /aa:l.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module bb {namespace urn:bb;prefix bb;typedef mytype {type empty; default x;}"
            "leaf l {type mytype;}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid type \"mytype\" - \"empty\" type must not have a default value (x).", "/bb:l");
}

static void
test_type_union(void **state)
{
    const struct lys_module *mod;
    struct lysc_type *type;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {yang-version 1.1;namespace urn:a;prefix a; typedef mybasetype {type string;}"
            "typedef mytype {type union {type int8; type mybasetype;}}"
            "leaf l {type mytype;}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(2, type->refcount);
    assert_int_equal(LY_TYPE_UNION, type->basetype);
    assert_non_null(((struct lysc_type_union *)type)->types);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_union *)type)->types));
    assert_int_equal(LY_TYPE_INT8, ((struct lysc_type_union *)type)->types[0]->basetype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_union *)type)->types[1]->basetype);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {yang-version 1.1;namespace urn:b;prefix b; typedef mybasetype {type string;}"
            "typedef mytype {type union {type int8; type mybasetype;}}"
            "leaf l {type union {type decimal64 {fraction-digits 2;} type mytype;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_UNION, type->basetype);
    assert_non_null(((struct lysc_type_union *)type)->types);
    assert_int_equal(3, LY_ARRAY_COUNT(((struct lysc_type_union *)type)->types));
    assert_int_equal(LY_TYPE_DEC64, ((struct lysc_type_union *)type)->types[0]->basetype);
    assert_int_equal(LY_TYPE_INT8, ((struct lysc_type_union *)type)->types[1]->basetype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_union *)type)->types[2]->basetype);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module c {yang-version 1.1;namespace urn:c;prefix c; typedef mybasetype {type string;}"
            "typedef mytype {type union {type leafref {path ../target;} type mybasetype;}}"
            "leaf l {type union {type decimal64 {fraction-digits 2;} type mytype;}}"
            "leaf target {type leafref {path ../realtarget;}} leaf realtarget {type int8;}}",
            LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_UNION, type->basetype);
    assert_non_null(((struct lysc_type_union *)type)->types);
    assert_int_equal(3, LY_ARRAY_COUNT(((struct lysc_type_union *)type)->types));
    assert_int_equal(LY_TYPE_DEC64, ((struct lysc_type_union *)type)->types[0]->basetype);
    assert_int_equal(LY_TYPE_LEAFREF, ((struct lysc_type_union *)type)->types[1]->basetype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_union *)type)->types[2]->basetype);
    assert_non_null(((struct lysc_type_leafref *)((struct lysc_type_union *)type)->types[1])->realtype);
    assert_int_equal(LY_TYPE_INT8, ((struct lysc_type_leafref *)((struct lysc_type_union *)type)->types[1])->realtype->basetype);

    /* invalid unions */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa;typedef mytype {type union;}"
            "leaf l {type mytype;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Missing type substatement for union type mytype.", "/aa:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module bb {namespace urn:bb;prefix bb;leaf l {type union;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Missing type substatement for union type.", "/bb:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module cc {namespace urn:cc;prefix cc;typedef mytype {type union{type int8; type string;}}"
            "leaf l {type mytype {type string;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid type substatement for the type not directly derived from union built-in type.", "/cc:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module dd {namespace urn:dd;prefix dd;typedef mytype {type union{type int8; type string;}}"
            "typedef mytype2 {type mytype {type string;}}leaf l {type mytype2;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid type substatement for the type \"mytype2\" not directly derived from union built-in type.", "/dd:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ee {namespace urn:ee;prefix ee;typedef mytype {type union{type mytype; type string;}}"
            "leaf l {type mytype;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid \"mytype\" type reference - circular chain of types detected.", "/ee:l");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ef {namespace urn:ef;prefix ef;typedef mytype {type mytype2;}"
            "typedef mytype2 {type mytype;} leaf l {type mytype;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid \"mytype\" type reference - circular chain of types detected.", "/ef:l");
}

static void
test_type_dflt(void **state)
{
    const struct lys_module *mod;
    struct lysc_type *type;
    struct lysc_node_leaf *leaf;
    uint8_t dynamic;

    /* default is not inherited from union's types */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {namespace urn:a;prefix a; typedef mybasetype {type string;default hello;units xxx;}"
            "leaf l {type union {type decimal64 {fraction-digits 2;} type mybasetype;}}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(1, type->refcount);
    assert_int_equal(LY_TYPE_UNION, type->basetype);
    assert_non_null(((struct lysc_type_union *)type)->types);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_type_union *)type)->types));
    assert_int_equal(LY_TYPE_DEC64, ((struct lysc_type_union *)type)->types[0]->basetype);
    assert_int_equal(LY_TYPE_STRING, ((struct lysc_type_union *)type)->types[1]->basetype);
    assert_null(((struct lysc_node_leaf *)mod->compiled->data)->dflt);
    assert_null(((struct lysc_node_leaf *)mod->compiled->data)->units);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b; typedef mybasetype {type string;default hello;units xxx;}"
            "leaf l {type mybasetype;}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(3, type->refcount);     /* 2x type reference, 1x default value's reference (typedf's default does not reference own type)*/
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_non_null(leaf = (struct lysc_node_leaf *)mod->compiled->data);
    assert_string_equal("hello", leaf->dflt->realtype->plugin->print(UTEST_LYCTX, leaf->dflt, LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_string_equal("xxx", leaf->units);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module c {namespace urn:c;prefix c; typedef mybasetype {type string;default hello;units xxx;}"
            "leaf l {type mybasetype; default goodbye;units yyy;}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(3, type->refcount);     /* 2x type reference, 1x default value's reference */
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    leaf = (struct lysc_node_leaf *)mod->compiled->data;
    assert_string_equal("goodbye", leaf->dflt->realtype->plugin->print(UTEST_LYCTX, leaf->dflt, LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_string_equal("yyy", leaf->units);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module d {namespace urn:d;prefix d; typedef mybasetype {type string;default hello;units xxx;}"
            "typedef mytype {type mybasetype;}leaf l1 {type mytype; default goodbye;units yyy;}"
            "leaf l2 {type mytype;}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(6, type->refcount);     /* 4x type reference, 2x default value's reference (1 shared compiled type of typedefs which default does not reference own type) */
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    leaf = (struct lysc_node_leaf *)mod->compiled->data;
    assert_string_equal("goodbye", leaf->dflt->realtype->plugin->print(UTEST_LYCTX, leaf->dflt, LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_string_equal("yyy", leaf->units);
    type = ((struct lysc_node_leaf *)mod->compiled->data->next)->type;
    assert_non_null(type);
    assert_int_equal(6, type->refcount);     /* 4x type reference, 2x default value's reference (1 shared compiled type of typedefs which default does not reference own type) */
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    leaf = (struct lysc_node_leaf *)mod->compiled->data->next;
    assert_string_equal("hello", leaf->dflt->realtype->plugin->print(UTEST_LYCTX, leaf->dflt, LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_string_equal("xxx", leaf->units);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module e {namespace urn:e;prefix e; typedef mybasetype {type string;}"
            "typedef mytype {type mybasetype; default hello;units xxx;}leaf l {type mytype;}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(4, type->refcount);     /* 3x type reference, 1x default value's reference (typedef's default does not reference own type) */
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    leaf = (struct lysc_node_leaf *)mod->compiled->data;
    assert_string_equal("hello", leaf->dflt->realtype->plugin->print(UTEST_LYCTX, leaf->dflt, LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_string_equal("xxx", leaf->units);

    /* mandatory leaf does not takes default value from type */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module f {namespace urn:f;prefix f;typedef mytype {type string; default hello;units xxx;}"
            "leaf l {type mytype; mandatory true;}}", LYS_IN_YANG, &mod));
    type = ((struct lysc_node_leaf *)mod->compiled->data)->type;
    assert_non_null(type);
    assert_int_equal(LY_TYPE_STRING, type->basetype);
    assert_null(((struct lysc_node_leaf *)mod->compiled->data)->dflt);
    assert_string_equal("xxx", ((struct lysc_node_leaf *)mod->compiled->data)->units);
}

static void
test_status(void **state)
{

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa;"
            "container c {status deprecated; leaf l {status current; type string;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("A \"current\" status is in conflict with the parent's \"deprecated\" status.", "/aa:c/l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module bb {namespace urn:bb;prefix bb;"
            "container c {status obsolete; leaf l {status current; type string;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("A \"current\" status is in conflict with the parent's \"obsolete\" status.", "/bb:c/l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module cc {namespace urn:cc;prefix cc;"
            "container c {status obsolete; leaf l {status deprecated; type string;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("A \"deprecated\" status is in conflict with the parent's \"obsolete\" status.", "/cc:c/l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module cc {namespace urn:dd;prefix d;"
            "container c {leaf l {status obsolete; type string;}}"
            "container d {leaf m {when \"../../c/l\"; type string;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("A current definition \"m\" is not allowed to reference obsolete definition \"l\".", "Schema location /cc:d/m.");
}

static void
test_grouping(void **state)
{

    /* result ok, but a warning about not used locally scoped grouping printed */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {namespace urn:a;prefix a; grouping grp1 {leaf a1 {type string;}}"
            "container a {leaf x {type string;} grouping grp2 {leaf a2 {type string;}}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Locally scoped grouping \"grp2\" not used.", NULL);
    UTEST_LOG_CLEAN;

    /* result ok - when statement or leafref target must be checked only at the place where the grouping is really instantiated */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b; grouping grp {"
            "leaf ref {type leafref {path \"../name\";}}"
            "leaf cond {type string; when \"../name = 'specialone'\";}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX(NULL, NULL);

    /* invalid - error in a non-instantiated grouping */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa;"
            "grouping grp {leaf x {type leafref;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Missing path substatement for leafref type.", "/aa:{grouping='grp'}/x");
    UTEST_LOG_CLEAN;
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa;"
            "container a {grouping grp {leaf x {type leafref;}}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Missing path substatement for leafref type.", "/aa:a/{grouping='grp'}/x");
}

static void
test_uses(void **state)
{
    const struct lys_module *mod;
    const struct lysc_node *parent, *child;
    const struct lysc_node_container *cont;
    const struct lysc_node_leaf *leaf;
    const struct lysc_node_choice *choice;
    const struct lysc_node_case *cs;

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module grp {namespace urn:grp;prefix g; typedef mytype {type string;} feature f;"
            "grouping grp {leaf x {type mytype;} leaf y {type string; if-feature f;}}}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {namespace urn:a;prefix a;import grp {prefix g;}"
            "grouping grp_a_top {leaf a1 {type int8;}}"
            "container a {uses grp_a; uses grp_a_top; uses g:grp; grouping grp_a {leaf a2 {type uint8;}}}}", LYS_IN_YANG, &mod));
    assert_non_null((parent = mod->compiled->data));
    assert_int_equal(LYS_CONTAINER, parent->nodetype);
    assert_non_null((child = ((struct lysc_node_container *)parent)->child));
    assert_string_equal("a2", child->name);
    assert_ptr_equal(mod, child->module);
    assert_non_null((child = child->next));
    assert_string_equal("a1", child->name);
    assert_ptr_equal(mod, child->module);
    assert_non_null((child = child->next));
    assert_string_equal("x", child->name);
    assert_ptr_equal(mod, child->module);
    assert_null((child = child->next));

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule bsub {belongs-to b {prefix b;} grouping grp {leaf b {when 1; type string;} leaf c {type string;}}}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b;include bsub;uses grp {when 2;}}", LYS_IN_YANG, &mod));
    assert_non_null(mod->compiled->data);

    leaf = (struct lysc_node_leaf *)mod->compiled->data;
    assert_int_equal(LYS_LEAF, leaf->nodetype);
    assert_string_equal("b", leaf->name);
    assert_int_equal(2, LY_ARRAY_COUNT(leaf->when));
    assert_int_equal(1, leaf->when[0]->refcount);
    assert_non_null(leaf->when[0]->context);
    assert_string_equal("b", leaf->when[0]->context->name);
    assert_int_equal(2, leaf->when[1]->refcount);
    assert_null(leaf->when[1]->context);

    leaf = (struct lysc_node_leaf *)leaf->next;
    assert_int_equal(LYS_LEAF, leaf->nodetype);
    assert_string_equal("c", leaf->name);
    assert_int_equal(1, LY_ARRAY_COUNT(leaf->when));
    assert_int_equal(2, leaf->when[0]->refcount);
    assert_null(leaf->when[0]->context);

    UTEST_LOG_CLEAN;
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module c {namespace urn:ii;prefix ii;"
            "grouping grp {leaf l {type string;}leaf k {type string; status obsolete;}}"
            "uses grp {status deprecated;}}", LYS_IN_YANG, &mod));
    assert_int_equal(LYS_LEAF, mod->compiled->data->nodetype);
    assert_string_equal("l", mod->compiled->data->name);
    assert_true(LYS_STATUS_DEPRC & mod->compiled->data->flags);
    assert_int_equal(LYS_LEAF, mod->compiled->data->next->nodetype);
    assert_string_equal("k", mod->compiled->data->next->name);
    assert_true(LYS_STATUS_OBSLT & mod->compiled->data->next->flags);
    CHECK_LOG(NULL, NULL);     /* no warning about inheriting deprecated flag from uses */

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module d {namespace urn:d;prefix d; grouping grp {container g;}"
            "container top {uses grp {augment g {leaf x {type int8;}}}}}", LYS_IN_YANG, &mod));
    assert_non_null(mod->compiled->data);
    assert_non_null(child = lysc_node_child(mod->compiled->data));
    assert_string_equal("g", child->name);
    assert_non_null(child = lysc_node_child(child));
    assert_string_equal("x", child->name);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module e {yang-version 1.1;namespace urn:e;prefix e; grouping grp {action g { description \"super g\";}}"
            "container top {action e; uses grp {refine g {description \"ultra g\";}}}}", LYS_IN_YANG, &mod));
    assert_non_null(mod->compiled->data);
    cont = (const struct lysc_node_container *)mod->compiled->data;
    assert_non_null(cont->actions);
    assert_non_null(cont->actions->next);
    assert_null(cont->actions->next->next);
    assert_string_equal("e", cont->actions->next->name);
    assert_string_equal("g", cont->actions->name);
    assert_string_equal("ultra g", cont->actions->dsc);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module f {yang-version 1.1;namespace urn:f;prefix f; grouping grp {notification g { description \"super g\";}}"
            "container top {notification f; uses grp {refine g {description \"ultra g\";}}}}", LYS_IN_YANG, &mod));
    assert_non_null(mod->compiled->data);
    cont = (const struct lysc_node_container *)mod->compiled->data;
    assert_non_null(cont->notifs);
    assert_non_null(cont->notifs->next);
    assert_null(cont->notifs->next->next);
    assert_string_equal("f", cont->notifs->next->name);
    assert_string_equal("g", cont->notifs->name);
    assert_string_equal("ultra g", cont->notifs->dsc);

    /* empty grouping */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module g {namespace urn:g;prefix g; grouping grp; uses grp;}", LYS_IN_YANG, &mod));
    assert_null(mod->compiled->data);

    /* choice in uses */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module h {yang-version 1.1;namespace urn:h;prefix h; grouping grp {choice gch {case gc1 { leaf y { type string;}} case gc2 {leaf z {type string;}}}}"
            "choice ch {case one { leaf x {type string;}} case two { uses grp;}}}", LYS_IN_YANG, &mod));
    assert_non_null(mod->compiled->data);
    choice = (const struct lysc_node_choice *)mod->compiled->data;
    assert_string_equal("ch", choice->name);
    cs = choice->cases;
    assert_non_null(cs);
    assert_string_equal("one", cs->name);
    assert_non_null(cs->child);
    assert_string_equal("x", cs->child->name);

    cs = (struct lysc_node_case *)cs->next;
    assert_non_null(cs);
    assert_string_equal("two", cs->name);
    assert_non_null(cs->child);
    assert_string_equal("gch", cs->child->name);

    /* top-level uses with augment and refine */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module i {namespace urn:i;prefix i; grouping grp {container g;}"
            "uses grp {augment g {leaf x {type int8;}} refine g {description \"dsc\";}}}",
            LYS_IN_YANG, &mod));
    assert_non_null(mod->compiled->data);
    child = mod->compiled->data;
    assert_string_equal("g", child->name);
    cont = (const struct lysc_node_container *)child;
    assert_string_equal("dsc", cont->dsc);
    assert_non_null(child = lysc_node_child(child));
    assert_string_equal("x", child->name);

    /* invalid */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa;uses missinggrp;}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Grouping \"missinggrp\" referenced by a uses statement not found.", "/aa:{uses='missinggrp'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module bb {namespace urn:bb;prefix bb;uses grp;"
            "grouping grp {leaf a{type string;}uses grp1;}"
            "grouping grp1 {leaf b {type string;}uses grp2;}"
            "grouping grp2 {leaf c {type string;}uses grp;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Grouping \"grp\" references itself through a uses statement.", "/bb:{uses='grp'}/{uses='grp1'}/{uses='grp2'}/{uses='grp'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module cc {namespace urn:cc;prefix cc;uses a:missingprefix;}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid prefix used for grouping reference.", "/cc:{uses='a:missingprefix'}");

    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module dd {namespace urn:dd;prefix dd;grouping grp{leaf a{type string;}}"
            "leaf a {type string;}uses grp;}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Duplicate identifier \"a\" of data definition/RPC/action/notification statement.", "/dd:{uses='grp'}/dd:a");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ee {namespace urn:ee;prefix ee;grouping grp {leaf l {type string; status deprecated;}}"
            "uses grp {status obsolete;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("A \"deprecated\" status is in conflict with the parent's \"obsolete\" status.", "/ee:{uses='grp'}/ee:l");

    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module ff {namespace urn:ff;prefix ff;grouping grp {leaf l {type string;}}"
            "leaf l {type int8;}uses grp;}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Duplicate identifier \"l\" of data definition/RPC/action/notification statement.", "/ff:{uses='grp'}/ff:l");
    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module fg {namespace urn:fg;prefix fg;grouping grp {leaf m {type string;}}"
            "uses grp;leaf m {type int8;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Duplicate identifier \"m\" of data definition/RPC/action/notification statement.", "/fg:m");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module gg {namespace urn:gg;prefix gg; grouping grp {container g;}"
            "leaf g {type string;}"
            "container top {uses grp {augment /g {leaf x {type int8;}}}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid descendant-schema-nodeid value \"/g\" - name test expected instead of \"/\".", "/gg:top/{uses='grp'}/{augment='/g'}");

    assert_int_equal(LY_ENOTFOUND, lys_parse_mem(UTEST_LYCTX, "module hh {yang-version 1.1;namespace urn:hh;prefix hh;"
            "grouping grp {notification g { description \"super g\";}}"
            "container top {notification h; uses grp {refine h {description \"ultra h\";}}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Refine(s) target node \"h\" in grouping \"grp\" was not found.", "/hh:top/{uses='grp'}");

    assert_int_equal(LY_ENOTFOUND, lys_parse_mem(UTEST_LYCTX, "module ii {yang-version 1.1;namespace urn:ii;prefix ii;"
            "grouping grp {action g { description \"super g\";}}"
            "container top {action i; uses grp {refine i {description \"ultra i\";}}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Refine(s) target node \"i\" in grouping \"grp\" was not found.", "/ii:top/{uses='grp'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module jj {yang-version 1.1;namespace urn:jj;prefix jj;"
            "grouping grp {leaf j { when \"1\"; type invalid;}}"
            "container top {uses grp;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Referenced type \"invalid\" not found.", "/jj:top/{uses='grp'}/j");
}

static void
test_refine(void **state)
{
    const struct lys_module *mod;
    struct lysc_node *parent, *child;
    struct lysc_node_leaf *leaf;
    struct lysc_node_leaflist *llist;
    uint8_t dynamic;
    struct ly_in *in;
    const char *data, *feats1[] = {"f", NULL}, *feats2[] = {"fa", NULL};

    data = "module grp {yang-version 1.1;namespace urn:grp;prefix g; feature f;typedef mytype {type string; default cheers!;}"
            "grouping grp {container c {leaf l {type mytype; default goodbye;}"
            "leaf-list ll {type mytype; default goodbye; max-elements 6;}"
            "choice ch {default ca; leaf ca {type int8;}leaf cb{type uint8;}}"
            "leaf x {type mytype; mandatory true; must 1;}"
            "anydata a {mandatory false; if-feature f; description original; reference original;}"
            "container c {config false; leaf l {type string;}}}}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lys_parse(UTEST_LYCTX, in, LYS_IN_YANG, feats1, NULL));

    data = "module a {yang-version 1.1;namespace urn:a;prefix a;import grp {prefix g;}feature fa;"
            "uses g:grp {refine c/l {default hello; config false;}"
            "refine c/ll {default hello;default world;}"
            "refine c/ch {default cb;config true; if-feature fa;}"
            "refine c/x {mandatory false; must ../ll;description refined; reference refined;}"
            "refine c/a {mandatory true; must 1; description refined; reference refined;}"
            "refine c/ll {max-elements 5;}"
            "refine c/c {config true;presence indispensable;}}}";
    ly_in_memory(in, data);
    assert_int_equal(LY_SUCCESS, lys_parse(UTEST_LYCTX, in, LYS_IN_YANG, feats2, &mod));
    ly_in_free(in, 0);

    assert_non_null((parent = mod->compiled->data));
    assert_int_equal(LYS_CONTAINER, parent->nodetype);
    assert_string_equal("c", parent->name);
    assert_non_null((leaf = (struct lysc_node_leaf *)((struct lysc_node_container *)parent)->child));
    assert_int_equal(LYS_LEAF, leaf->nodetype);
    assert_string_equal("l", leaf->name);
    assert_string_equal("hello", leaf->dflt->realtype->plugin->print(UTEST_LYCTX, leaf->dflt, LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_int_equal(LYS_CONFIG_R, leaf->flags & LYS_CONFIG_MASK);
    assert_non_null(llist = (struct lysc_node_leaflist *)leaf->next);
    assert_int_equal(LYS_LEAFLIST, llist->nodetype);
    assert_string_equal("ll", llist->name);
    assert_int_equal(2, LY_ARRAY_COUNT(llist->dflts));
    assert_string_equal("hello", llist->dflts[0]->realtype->plugin->print(UTEST_LYCTX, llist->dflts[0], LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_string_equal("world", llist->dflts[1]->realtype->plugin->print(UTEST_LYCTX, llist->dflts[1], LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_int_equal(5, llist->max);
    assert_non_null(child = llist->next);
    assert_int_equal(LYS_CHOICE, child->nodetype);
    assert_string_equal("ch", child->name);
    assert_string_equal("cb", ((struct lysc_node_choice *)child)->dflt->name);
    assert_true(LYS_SET_DFLT & ((struct lysc_node_choice *)child)->dflt->flags);
    assert_false(LYS_SET_DFLT & ((struct lysc_node_choice *)child)->cases[0].flags);
    assert_non_null(leaf = (struct lysc_node_leaf *)child->next);
    assert_int_equal(LYS_LEAF, leaf->nodetype);
    assert_string_equal("x", leaf->name);
    assert_false(LYS_MAND_TRUE & leaf->flags);
    assert_string_equal("cheers!", leaf->dflt->realtype->plugin->print(UTEST_LYCTX, leaf->dflt, LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_non_null(leaf->musts);
    assert_int_equal(2, LY_ARRAY_COUNT(leaf->musts));
    assert_string_equal("refined", leaf->dsc);
    assert_string_equal("refined", leaf->ref);
    assert_non_null(child = leaf->next);
    assert_int_equal(LYS_ANYDATA, child->nodetype);
    assert_string_equal("a", child->name);
    assert_true(LYS_MAND_TRUE & child->flags);
    assert_non_null(((struct lysc_node_anydata *)child)->musts);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_node_anydata *)child)->musts));
    assert_string_equal("refined", child->dsc);
    assert_string_equal("refined", child->ref);
    assert_non_null(child = child->next);
    assert_int_equal(LYS_CONTAINER, child->nodetype);
    assert_string_equal("c", child->name);
    assert_true(LYS_PRESENCE & child->flags);
    assert_true(LYS_CONFIG_W & child->flags);
    assert_true(LYS_CONFIG_W & ((struct lysc_node_container *)child)->child->flags);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {yang-version 1.1;namespace urn:b;prefix b;import grp {prefix g;}"
            "uses g:grp {status deprecated; refine c/x {default hello; mandatory false;}}}", LYS_IN_YANG, &mod));
    assert_non_null((leaf = (struct lysc_node_leaf *)((struct lysc_node_container *)mod->compiled->data)->child->prev->prev->prev));
    assert_int_equal(LYS_LEAF, leaf->nodetype);
    assert_string_equal("x", leaf->name);
    assert_false(LYS_MAND_TRUE & leaf->flags);
    assert_string_equal("hello", leaf->dflt->realtype->plugin->print(UTEST_LYCTX, leaf->dflt, LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);

    /* invalid */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa;import grp {prefix g;}"
            "uses g:grp {refine c {default hello;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid refine of container node - it is not possible to replace \"default\" property.", "/aa:{uses='g:grp'}/aa:c/{refine='c'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module bb {namespace urn:bb;prefix bb;import grp {prefix g;}"
            "uses g:grp {refine c/l {default hello; default world;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid refine of leaf with too many (2) default properties.", "/bb:{uses='g:grp'}/bb:c/l/{refine='c/l'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module cc {namespace urn:cc;prefix cc;import grp {prefix g;}"
            "uses g:grp {refine c/ll {default hello; default world;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid refine of default in leaf-list - the default statement is allowed only in YANG 1.1 modules.", "/cc:{uses='g:grp'}/cc:c/ll/{refine='c/ll'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module dd {namespace urn:dd;prefix dd;import grp {prefix g;}"
            "uses g:grp {refine c/ll {mandatory true;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid refine of leaf-list node - it is not possible to replace \"mandatory\" property.", "/dd:{uses='g:grp'}/dd:c/ll/{refine='c/ll'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ee {namespace urn:ee;prefix ee;import grp {prefix g;}"
            "uses g:grp {refine c/l {mandatory true;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/ee:{uses='g:grp'}/ee:c/l",
            "Invalid mandatory leaf with a default value.", "/ee:{uses='g:grp'}/ee:c/l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ef {namespace urn:ef;prefix ef;import grp {prefix g;}"
            "uses g:grp {refine c/ch {mandatory true;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/ef:{uses='g:grp'}/ef:c/ch",
            "Invalid mandatory choice with a default case.", "/ef:{uses='g:grp'}/ef:c/ch");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ff {namespace urn:ff;prefix ff;import grp {prefix g;}"
            "uses g:grp {refine c/ch/ca/ca {mandatory true;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Mandatory node \"ca\" under the default case \"ca\".", "/ff:{uses='g:grp'}/ff:c/ch");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module gg {namespace urn:gg;prefix gg;import grp {prefix g;}"
            "uses g:grp {refine c/x {default hello;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/gg:{uses='g:grp'}/gg:c/x",
            "Invalid mandatory leaf with a default value.", "/gg:{uses='g:grp'}/gg:c/x");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module hh {namespace urn:hh;prefix hh;import grp {prefix g;}"
            "uses g:grp {refine c/c/l {config true;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/hh:{uses='g:grp'}/hh:c/c/l",
            "Configuration node cannot be child of any state data node.", "/hh:{uses='g:grp'}/hh:c/c/l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ii {namespace urn:ii;prefix ii;grouping grp {leaf l {type string; status deprecated;}}"
            "uses grp {status obsolete;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("A \"deprecated\" status is in conflict with the parent's \"obsolete\" status.", "/ii:{uses='grp'}/ii:l");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module jj {namespace urn:jj;prefix jj;import grp {prefix g;}"
            "uses g:grp {refine c/x {presence nonsence;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid refine of leaf node - it is not possible to replace \"presence\" property.", "/jj:{uses='g:grp'}/jj:c/x/{refine='c/x'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module kk {namespace urn:kk;prefix kk;import grp {prefix g;}"
            "uses g:grp {refine c/ch {must 1;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid refine of choice node - it is not possible to add \"must\" property.", "/kk:{uses='g:grp'}/kk:c/ch/{refine='c/ch'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ll {namespace urn:ll;prefix ll;import grp {prefix g;}"
            "uses g:grp {refine c/x {min-elements 1;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid refine of leaf node - it is not possible to replace \"min-elements\" property.", "/ll:{uses='g:grp'}/ll:c/x/{refine='c/x'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module mm {namespace urn:mm;prefix mm;import grp {prefix g;}"
            "uses g:grp {refine c/ll {min-elements 1;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/mm:{uses='g:grp'}/mm:c/ll",
            "The default statement is present on leaf-list with a nonzero min-elements.", "/mm:{uses='g:grp'}/mm:c/ll");
}

static void
test_augment(void **state)
{
    const struct lys_module *mod;
    const struct lysc_node *node;
    const struct lysc_node_choice *ch;
    const struct lysc_node_case *c;
    const struct lysc_node_container *cont;
    const struct lysc_node_action *rpc;

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module a {namespace urn:a;prefix a; typedef atype {type string;}"
            "container top {leaf a {type string;}}}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b;import a {prefix a;}"
            "leaf b {type a:atype;}}", LYS_IN_YANG, &mod));
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module c {namespace urn:c;prefix c; import a {prefix a;}"
            "augment /a:top { container c {leaf c {type a:atype;}}}}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module d {namespace urn:d;prefix d;import a {prefix a;} import c {prefix c;}"
            "augment /a:top/c:c { leaf d {type a:atype;} leaf c {type string;}}}", LYS_IN_YANG, &mod));
    assert_non_null((mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "a")));
    assert_non_null(ly_ctx_get_module_implemented(UTEST_LYCTX, "b"));
    assert_non_null(ly_ctx_get_module_implemented(UTEST_LYCTX, "c"));
    assert_non_null(ly_ctx_get_module_implemented(UTEST_LYCTX, "d"));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal(node->name, "top");
    assert_non_null(node = lysc_node_child(node));
    assert_string_equal(node->name, "a");
    assert_non_null(node = node->next);
    assert_string_equal(node->name, "c");
    assert_non_null(node = lysc_node_child(node));
    assert_string_equal(node->name, "c");
    assert_non_null(node = node->next);
    assert_string_equal(node->name, "d");
    assert_non_null(node = node->next);
    assert_string_equal(node->name, "c");

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module e {namespace urn:e;prefix e;choice ch {leaf a {type string;}}"
            "augment /ch/c {when 1; leaf lc2 {type uint16;}}"
            "augment /ch { when 1; leaf b {type int8;} case c {leaf lc1 {type uint8;}}}}", LYS_IN_YANG, &mod));
    assert_non_null((ch = (const struct lysc_node_choice *)mod->compiled->data));
    assert_null(mod->compiled->data->next);
    assert_string_equal("ch", ch->name);

    assert_non_null(c = ch->cases);
    assert_string_equal("b", c->name);
    assert_non_null(c->when);
    assert_string_equal("b", c->child->name);

    assert_non_null(c = (const struct lysc_node_case *)c->next);
    assert_string_equal("c", c->name);
    assert_non_null(c->when);
    assert_string_equal("lc2", ((const struct lysc_node_case *)c)->child->name);
    assert_non_null(lysc_node_when(((const struct lysc_node_case *)c)->child));
    assert_string_equal("lc1", ((const struct lysc_node_case *)c)->child->next->name);
    assert_null(lysc_node_when(((const struct lysc_node_case *)c)->child->next));

    assert_non_null(c = (const struct lysc_node_case *)c->next);
    assert_string_equal("a", c->name);
    assert_null(c->when);
    assert_string_equal("a", c->child->name);
    assert_null(c->next);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module f {namespace urn:f;prefix f;grouping g {leaf a {type string;}}"
            "container c;"
            "augment /c {uses g;}}", LYS_IN_YANG, &mod));
    assert_non_null(node = lysc_node_child(mod->compiled->data));
    assert_string_equal(node->name, "a");

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule gsub {belongs-to g {prefix g;}"
            "augment /c {container sub;}}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module g {namespace urn:g;prefix g;include gsub; container c;"
            "augment /c/sub {leaf main {type string;}}}", LYS_IN_YANG, &mod));
    assert_non_null(mod->compiled->data);
    assert_string_equal("c", mod->compiled->data->name);
    assert_non_null(node = ((struct lysc_node_container *)mod->compiled->data)->child);
    assert_string_equal("sub", node->name);
    assert_non_null(node = ((struct lysc_node_container *)node)->child);
    assert_string_equal("main", node->name);

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module himp {namespace urn:hi;prefix hi;container top; rpc func;}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module h {namespace urn:h;prefix h;import himp {prefix hi;}container top;"
            "augment /hi:top {container p {presence XXX; leaf x {mandatory true;type string;}}}"
            "augment /hi:top {list ll {key x;leaf x {type string;}leaf y {mandatory true; type string;}}}"
            "augment /hi:top {leaf l {type string; mandatory true; config false;}}"
            "augment /top {leaf l {type string; mandatory true;}}}", LYS_IN_YANG, &mod));
    assert_non_null(node = mod->compiled->data);
    assert_non_null(node = ((struct lysc_node_container *)node)->child);
    assert_string_equal("l", node->name);
    assert_true(node->flags & LYS_MAND_TRUE);
    assert_non_null(mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "himp"));
    assert_non_null(node = mod->compiled->data);
    assert_non_null(node = ((struct lysc_node_container *)node)->child);
    assert_string_equal("p", node->name);
    assert_non_null(node = node->next);
    assert_string_equal("l", node->name);
    assert_true(node->flags & LYS_CONFIG_R);
    assert_non_null(node = node->next);
    assert_string_equal("ll", node->name);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module i {namespace urn:i;prefix i;import himp {prefix hi;}"
            "augment /hi:func/hi:input {leaf x {type string;}}"
            "augment /hi:func/hi:output {leaf y {type string;}}}", LYS_IN_YANG, NULL));
    assert_non_null(mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "himp"));
    assert_non_null(rpc = mod->compiled->rpcs);
    assert_null(rpc->next);
    assert_non_null(rpc->input.child);
    assert_string_equal("x", rpc->input.child->name);
    assert_null(rpc->input.child->next);
    assert_non_null(rpc->output.child);
    assert_string_equal("y", rpc->output.child->name);
    assert_null(rpc->output.child->next);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module j {namespace urn:j;prefix j;yang-version 1.1; container root;"
            "grouping grp {notification grp-notif;}"
            "augment /root {uses grp;}}", LYS_IN_YANG, &mod));
    assert_non_null(cont = (const struct lysc_node_container *)mod->compiled->data);
    assert_null(cont->child);
    assert_non_null(cont->notifs);
    assert_null(cont->notifs->next);

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, NULL, NULL);
    UTEST_LOG_CLEAN;
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module k {namespace urn:k; prefix k;yang-version 1.1;"
            "feature f;"
            "container c {if-feature f; leaf a {type string;}}}", LYS_IN_YANG, &mod));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module l {namespace urn:l; prefix l; yang-version 1.1;"
            "import k {prefix k;}"
            "augment /k:c {leaf b {type string;}}"
            "leaf c {when \"/k:c/l:b\"; type string;}}", LYS_IN_YANG, NULL));
    /* no xpath warning expected */
    CHECK_LOG(NULL, NULL);
    assert_null(mod->compiled->data);

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; container c {leaf a {type string;}}"
            "augment /x/ {leaf a {type int8;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid absolute-schema-nodeid value \"/x/\" - unexpected end of expression.", "/aa:{augment='/x/'}");

    assert_int_equal(LY_ENOTFOUND, lys_parse_mem(UTEST_LYCTX, "module aa {namespace urn:aa;prefix aa; container c {leaf a {type string;}}"
            "augment /x {leaf a {type int8;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Augment target node \"/x\" from module \"aa\" was not found.", "/aa:{augment='/x'}");

    assert_int_equal(LY_EEXIST, lys_parse_mem(UTEST_LYCTX, "module bb {namespace urn:bb;prefix bb; container c {leaf a {type string;}}"
            "augment /c {leaf a {type int8;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Duplicate identifier \"a\" of data definition/RPC/action/notification statement.", "/bb:c/a");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module cc {namespace urn:cc;prefix cc; container c {leaf a {type string;}}"
            "augment /c/a {leaf a {type int8;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Augment's absolute-schema-nodeid \"/c/a\" refers to a leaf node which is not an allowed augment's target.", "/cc:{augment='/c/a'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module dd {namespace urn:dd;prefix dd; container c {leaf a {type string;}}"
            "augment /c {case b {leaf d {type int8;}}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid augment of container node which is not allowed to contain case node \"b\".", "/dd:{augment='/c'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ee {namespace urn:ee;prefix ee; import himp {prefix hi;}"
            "augment /hi:top {container c {leaf d {mandatory true; type int8;}}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid augment adding mandatory node \"c\" without making it conditional via when statement.", "/ee:{augment='/hi:top'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ff {namespace urn:ff;prefix ff; container top;"
            "augment ../top {leaf x {type int8;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid absolute-schema-nodeid value \"../top\" - \"/\" expected instead of \"..\".", "/ff:{augment='../top'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module gg {namespace urn:gg;prefix gg; rpc func;"
            "augment /func {leaf x {type int8;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Augment's absolute-schema-nodeid \"/func\" refers to a RPC node which is not an allowed augment's target.", "/gg:{augment='/func'}");

    assert_int_equal(LY_ENOTFOUND, lys_parse_mem(UTEST_LYCTX, "module hh {namespace urn:i;prefix i;import himp {prefix hi;}"
            "augment /hi:func/input {leaf x {type string;}}"
            "augment /hi:func/output {leaf y {type string;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Augment target node \"/hi:func/input\" from module \"hh\" was not found.", "/hh:{augment='/hi:func/input'}");
}

static void
test_deviation(void **state)
{
    const struct lys_module *mod;
    const struct lysc_node *node;
    const struct lysc_node_list *list;
    const struct lysc_node_leaflist *llist;
    const struct lysc_node_leaf *leaf;
    const char *str;
    uint8_t dynamic;

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module a {namespace urn:a;prefix a;"
            "container top {leaf a {type string;} leaf b {type string;} leaf c {type string;}}"
            "choice ch {default c; case b {leaf b{type string;}} case a {leaf a{type string;} leaf x {type string;}}"
            " case c {leaf c{type string;}}}"
            "rpc func1 { input { leaf x {type int8;}} output {leaf y {type int8;}}}"
            "rpc func2;}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b;import a {prefix a;}"
            "deviation /a:top/a:b {deviate not-supported;}"
            "deviation /a:ch/a:a/a:x {deviate not-supported;}"
            "deviation /a:ch/a:c {deviate not-supported;}"
            "deviation /a:ch/a:b {deviate not-supported;}"
            "deviation /a:ch/a:a/a:a {deviate not-supported;}"
            "deviation /a:ch {deviate replace {default a;}}"
            "deviation /a:func1/a:input {deviate not-supported;}"
            "deviation /a:func1/a:output {deviate not-supported;}"
            "deviation /a:func2 {deviate not-supported;}}", LYS_IN_YANG, NULL));
    assert_non_null((mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "a")));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal(node->name, "top");
    assert_non_null(node = lysc_node_child(node));
    assert_string_equal(node->name, "a");
    assert_non_null(node = node->next);
    assert_string_equal(node->name, "c");
    assert_null(node = node->next);
    assert_non_null(node = mod->compiled->data->next);
    assert_string_equal("ch", node->name);
    assert_non_null(((struct lysc_node_choice *)node)->dflt);
    assert_non_null(((struct lysc_node_choice *)node)->cases);
    assert_null(((struct lysc_node_choice *)node)->cases->next);
    assert_non_null(mod->compiled->rpcs);
    assert_null(mod->compiled->rpcs->next);
    assert_null(mod->compiled->rpcs->input.child);
    assert_null(mod->compiled->rpcs->output.child);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module c {namespace urn:c;prefix c; typedef mytype {type string; units kilometers;}"
            "leaf c1 {type mytype;} leaf c2 {type mytype; units meters;} leaf c3 {type mytype; units meters;}"
            "deviation /c1 {deviate add {units meters;}}"
            "deviation /c2 {deviate delete {units meters;}}"
            "deviation /c3 {deviate replace {units centimeters;}}}", LYS_IN_YANG, &mod));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal("c1", node->name);
    assert_string_equal("meters", ((struct lysc_node_leaf *)node)->units);
    assert_non_null(node = node->next);
    assert_string_equal("c2", node->name);
    assert_string_equal("kilometers", ((struct lysc_node_leaf *)node)->units);
    assert_non_null(node = node->next);
    assert_string_equal("c3", node->name);
    assert_string_equal("centimeters", ((struct lysc_node_leaf *)node)->units);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module d {namespace urn:d;prefix d; leaf c1 {type string; must 1;}"
            "container c2 {presence yes; must 1; must 2;} leaf c3 {type string; must 1; must 3;}"
            "deviation /c1 {deviate add {must 3;}}"
            "deviation /c2 {deviate delete {must 2;}}"
            "deviation /c3 {deviate delete {must 3; must 1;}}}", LYS_IN_YANG, &mod));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal("c1", node->name);
    assert_int_equal(2, LY_ARRAY_COUNT(((struct lysc_node_leaf *)node)->musts));
    assert_string_equal("3", ((struct lysc_node_leaf *)node)->musts[1].cond->expr);
    assert_non_null(node = node->next);
    assert_string_equal("c2", node->name);
    assert_int_equal(1, LY_ARRAY_COUNT(((struct lysc_node_container *)node)->musts));
    assert_string_equal("1", ((struct lysc_node_container *)node)->musts[0].cond->expr);
    assert_non_null(node = node->next);
    assert_string_equal("c3", node->name);
    assert_null(((struct lysc_node_leaf *)node)->musts);

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module e {yang-version 1.1; namespace urn:e;prefix e; typedef mytype {type string; default nothing;}"
            "choice a {default aa;leaf aa {type string;} leaf ab {type string;} leaf ac {type string; mandatory true;}}"
            "choice b {default ba;leaf ba {type string;} leaf bb {type string;}}"
            "leaf c {default hello; type string;}"
            "leaf-list d {default hello; default world; type string;}"
            "leaf c2 {type mytype;} leaf-list d2 {type mytype;}}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module f {yang-version 1.1; namespace urn:f;prefix f;import e {prefix x;}"
            "deviation /x:a {deviate delete {default aa;}}"
            "deviation /x:b {deviate delete {default ba;}}"
            "deviation /x:c {deviate delete {default hello;}}"
            "deviation /x:d {deviate delete {default world;}}}", LYS_IN_YANG, NULL));
    assert_non_null((mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "e")));
    assert_non_null(node = mod->compiled->data);
    assert_null(((struct lysc_node_choice *)node)->dflt);
    assert_non_null(node = node->next);
    assert_null(((struct lysc_node_choice *)node)->dflt);
    assert_non_null(leaf = (struct lysc_node_leaf *)node->next);
    assert_null(leaf->dflt);
    assert_non_null(llist = (struct lysc_node_leaflist *)leaf->next);
    assert_int_equal(1, LY_ARRAY_COUNT(llist->dflts));
    assert_string_equal("hello", llist->dflts[0]->realtype->plugin->print(UTEST_LYCTX, llist->dflts[0], LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_non_null(leaf = (struct lysc_node_leaf *)llist->next);
    assert_string_equal("nothing", leaf->dflt->realtype->plugin->print(UTEST_LYCTX, leaf->dflt, LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_int_equal(5, leaf->dflt->realtype->refcount);     /* 3x type reference, 2x default value reference (typedef's default does not reference own type) */
    assert_non_null(llist = (struct lysc_node_leaflist *)leaf->next);
    assert_int_equal(1, LY_ARRAY_COUNT(llist->dflts));
    assert_string_equal("nothing", llist->dflts[0]->realtype->plugin->print(UTEST_LYCTX, llist->dflts[0], LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module g {yang-version 1.1; namespace urn:g;prefix g;import e {prefix x;}"
            "deviation /x:b {deviate add {default x:ba;}}"
            "deviation /x:c {deviate add {default bye;}}"
            "deviation /x:d {deviate add {default all; default people;}}"
            "deviation /x:c2 {deviate add {default hi; must 1;}}"
            "deviation /x:d2 {deviate add {default hi; default all;}}}", LYS_IN_YANG, NULL));
    assert_non_null((mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "e")));
    assert_non_null(node = mod->compiled->data);
    assert_null(((struct lysc_node_choice *)node)->dflt);
    assert_non_null(node = node->next);
    assert_non_null(((struct lysc_node_choice *)node)->dflt);
    assert_string_equal("ba", ((struct lysc_node_choice *)node)->dflt->name);
    assert_non_null(leaf = (struct lysc_node_leaf *)node->next);
    assert_non_null(leaf->dflt);
    assert_string_equal("bye", leaf->dflt->realtype->plugin->print(UTEST_LYCTX, leaf->dflt, LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_non_null(llist = (struct lysc_node_leaflist *)leaf->next);
    assert_int_equal(3, LY_ARRAY_COUNT(llist->dflts));
    assert_string_equal("hello", llist->dflts[0]->realtype->plugin->print(UTEST_LYCTX, llist->dflts[0], LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_string_equal("all", llist->dflts[1]->realtype->plugin->print(UTEST_LYCTX, llist->dflts[1], LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_string_equal("people", llist->dflts[2]->realtype->plugin->print(UTEST_LYCTX, llist->dflts[2], LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_non_null(leaf = (struct lysc_node_leaf *)llist->next);
    assert_non_null(leaf->dflt);
    assert_string_equal("hi", leaf->dflt->realtype->plugin->print(UTEST_LYCTX, leaf->dflt, LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_int_equal(6, leaf->dflt->realtype->refcount);     /* 3x type reference, 3x default value reference
    - previous type's default values were replaced by node's default values where d2 now has 2 default values */
    assert_int_equal(1, LY_ARRAY_COUNT(leaf->musts));
    assert_int_equal(0, LY_ARRAY_COUNT(leaf->musts[0].prefixes));
    assert_non_null(llist = (struct lysc_node_leaflist *)leaf->next);
    assert_int_equal(2, LY_ARRAY_COUNT(llist->dflts));
    assert_string_equal("hi", llist->dflts[0]->realtype->plugin->print(UTEST_LYCTX, llist->dflts[0], LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_string_equal("all", llist->dflts[1]->realtype->plugin->print(UTEST_LYCTX, llist->dflts[1], LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module h {yang-version 1.1; namespace urn:h;prefix h;import e {prefix x;}"
            "deviation /x:b {deviate replace {default x:ba;}}"
            "deviation /x:c {deviate replace {default hello;}}}", LYS_IN_YANG, NULL));
    assert_non_null((mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "e")));
    assert_non_null(node = mod->compiled->data);
    assert_null(((struct lysc_node_choice *)node)->dflt);
    assert_non_null(node = node->next);
    assert_non_null(((struct lysc_node_choice *)node)->dflt);
    assert_string_equal("ba", ((struct lysc_node_choice *)node)->dflt->name);
    assert_non_null(leaf = (struct lysc_node_leaf *)node->next);
    assert_non_null(leaf->dflt);
    assert_string_equal("hello", leaf->dflt->realtype->plugin->print(UTEST_LYCTX, leaf->dflt, LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module i {namespace urn:i;prefix i;"
            "list l1 {key a; leaf a {type string;} leaf b {type string;} leaf c {type string;}}"
            "list l2 {key a; unique \"b c\"; unique \"d\"; leaf a {type string;} leaf b {type string;}"
            "         leaf c {type string;} leaf d {type string;}}}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module j {namespace urn:j;prefix j;import i {prefix i;}"
            "augment /i:l1 {leaf j_c {type string;}}"
            "deviation /i:l1 {deviate add {unique \"i:b j_c\"; }}"
            "deviation /i:l1 {deviate add {unique \"i:c\";}}"
            "deviation /i:l2 {deviate delete {unique \"d\"; unique \"b c\";}}}", LYS_IN_YANG, NULL));
    assert_non_null((mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "i")));
    assert_non_null(list = (struct lysc_node_list *)mod->compiled->data);
    assert_string_equal("l1", list->name);
    assert_int_equal(2, LY_ARRAY_COUNT(list->uniques));
    assert_int_equal(2, LY_ARRAY_COUNT(list->uniques[0]));
    assert_string_equal("b", list->uniques[0][0]->name);
    assert_string_equal("j_c", list->uniques[0][1]->name);
    assert_int_equal(1, LY_ARRAY_COUNT(list->uniques[1]));
    assert_string_equal("c", list->uniques[1][0]->name);
    assert_non_null(list = (struct lysc_node_list *)list->next);
    assert_string_equal("l2", list->name);
    assert_null(list->uniques);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module k {namespace urn:k;prefix k; leaf a {type string;}"
            "container top {leaf x {type string;} leaf y {type string; config false;}}"
            "deviation /a {deviate add {config false; }}"
            "deviation /top {deviate add {config false;}}}", LYS_IN_YANG, &mod));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal("a", node->name);
    assert_true(node->flags & LYS_CONFIG_R);
    assert_non_null(node = node->next);
    assert_string_equal("top", node->name);
    assert_true(node->flags & LYS_CONFIG_R);
    assert_non_null(node = lysc_node_child(node));
    assert_string_equal("x", node->name);
    assert_true(node->flags & LYS_CONFIG_R);
    assert_non_null(node = node->next);
    assert_string_equal("y", node->name);
    assert_true(node->flags & LYS_CONFIG_R);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module l {namespace urn:l;prefix l; leaf a {config false; type string;}"
            "container top {config false; leaf x {type string;}}"
            "deviation /a {deviate replace {config true;}}"
            "deviation /top {deviate replace {config true;}}}", LYS_IN_YANG, &mod));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal("a", node->name);
    assert_true(node->flags & LYS_CONFIG_W);
    assert_non_null(node = node->next);
    assert_string_equal("top", node->name);
    assert_true(node->flags & LYS_CONFIG_W);
    assert_non_null(node = lysc_node_child(node));
    assert_string_equal("x", node->name);
    assert_true(node->flags & LYS_CONFIG_W);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module m {namespace urn:m;prefix m;"
            "container a {leaf a {type string;}}"
            "container b {leaf b {mandatory true; type string;}}"
            "deviation /a/a {deviate add {mandatory true;}}"
            "deviation /b/b {deviate replace {mandatory false;}}}", LYS_IN_YANG, &mod));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal("a", node->name);
    assert_true((node->flags & LYS_MAND_MASK) == LYS_MAND_TRUE);
    assert_true((lysc_node_child(node)->flags & LYS_MAND_MASK) == LYS_MAND_TRUE);
    assert_non_null(node = node->next);
    assert_string_equal("b", node->name);
    assert_false(node->flags & LYS_MAND_MASK);     /* just unset on container */
    assert_true((lysc_node_child(node)->flags & LYS_MAND_MASK) == LYS_MAND_FALSE);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module n {yang-version 1.1; namespace urn:n;prefix n;"
            "leaf a {default test; type string;}"
            "leaf b {mandatory true; type string;}"
            "deviation /a {deviate add {mandatory true;} deviate delete {default test;}}"
            "deviation /b {deviate add {default test;} deviate replace {mandatory false;}}}", LYS_IN_YANG, &mod));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal("a", node->name);
    assert_null(((struct lysc_node_leaf *)node)->dflt);
    assert_true((node->flags & LYS_MAND_MASK) == LYS_MAND_TRUE);
    assert_non_null(node = node->next);
    assert_string_equal("b", node->name);
    assert_non_null(((struct lysc_node_leaf *)node)->dflt);
    assert_true((node->flags & LYS_MAND_MASK) == LYS_MAND_FALSE);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module o {namespace urn:o;prefix o;"
            "leaf-list a {type string;}"
            "list b {config false;}"
            "leaf-list c {min-elements 1; max-elements 10; type string;}"
            "list d {min-elements 10; max-elements 100; config false;}"
            "deviation /a {deviate add {min-elements 1; max-elements 10;}}"
            "deviation /b {deviate add {min-elements 10; max-elements 100;}}"
            "deviation /c {deviate replace {min-elements 10; max-elements 100;}}"
            "deviation /d {deviate replace {min-elements 1; max-elements 10;}}}", LYS_IN_YANG, &mod));
    assert_non_null(node = mod->compiled->data);
    assert_string_equal("a", node->name);
    assert_int_equal(1, ((struct lysc_node_leaflist *)node)->min);
    assert_int_equal(10, ((struct lysc_node_leaflist *)node)->max);
    assert_non_null(node = node->next);
    assert_string_equal("b", node->name);
    assert_int_equal(10, ((struct lysc_node_list *)node)->min);
    assert_int_equal(100, ((struct lysc_node_list *)node)->max);
    assert_non_null(node = node->next);
    assert_string_equal("c", node->name);
    assert_int_equal(10, ((struct lysc_node_leaflist *)node)->min);
    assert_int_equal(100, ((struct lysc_node_leaflist *)node)->max);
    assert_non_null(node = node->next);
    assert_string_equal("d", node->name);
    assert_int_equal(1, ((struct lysc_node_list *)node)->min);
    assert_int_equal(10, ((struct lysc_node_list *)node)->max);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module p {yang-version 1.1; namespace urn:p;prefix p; typedef mytype {type int8; default 1;}"
            "leaf a {type string; default 10;} leaf-list b {type string;}"
            "deviation /a {deviate replace {type mytype;}}"
            "deviation /b {deviate replace {type mytype;}}}", LYS_IN_YANG, &mod));
    assert_non_null(leaf = (struct lysc_node_leaf *)mod->compiled->data);
    assert_string_equal("a", leaf->name);
    assert_int_equal(LY_TYPE_INT8, leaf->type->basetype);
    assert_string_equal("10", leaf->dflt->realtype->plugin->print(UTEST_LYCTX, leaf->dflt, LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_int_equal(10, leaf->dflt->uint8);
    assert_non_null(llist = (struct lysc_node_leaflist *)leaf->next);
    assert_string_equal("b", llist->name);
    assert_int_equal(LY_TYPE_INT8, llist->type->basetype);
    assert_int_equal(1, LY_ARRAY_COUNT(llist->dflts));
    assert_string_equal("1", llist->dflts[0]->realtype->plugin->print(UTEST_LYCTX, llist->dflts[0], LY_VALUE_SCHEMA, NULL, &dynamic, NULL));
    assert_int_equal(0, dynamic);
    assert_int_equal(1, llist->dflts[0]->uint8);

    /* instance-identifiers with NULL canonical are changed to string types with a canonical value equal to the original value */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module q {yang-version 1.1; namespace urn:q;prefix q; import e {prefix e;}"
            "leaf q {type instance-identifier; default \"/e:d2[.='a']\";}"
            "leaf-list ql {type instance-identifier; default \"/e:d[.='b']\"; default \"/e:d2[.='c']\";}}", LYS_IN_YANG, &mod));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module qdev {yang-version 1.1; namespace urn:qdev;prefix qd; import q {prefix q;}"
            "deviation /q:q { deviate replace {type string;}}"
            "deviation /q:ql { deviate replace {type string;}}}", LYS_IN_YANG, NULL));
    assert_non_null(leaf = (struct lysc_node_leaf *)mod->compiled->data);
    assert_int_equal(LY_TYPE_STRING, leaf->dflt->realtype->basetype);
    assert_non_null(leaf->dflt->realtype->plugin->print(UTEST_LYCTX, leaf->dflt, LY_VALUE_CANON, NULL, NULL, NULL));
    assert_string_equal("/e:d2[.='a']", leaf->dflt->_canonical);
    assert_non_null(llist = (struct lysc_node_leaflist *)leaf->next);
    assert_int_equal(2, LY_ARRAY_COUNT(llist->dflts));
    assert_int_equal(LY_TYPE_STRING, llist->dflts[0]->realtype->basetype);
    assert_string_equal("/e:d[.='b']", llist->dflts[0]->_canonical);
    assert_int_equal(LY_TYPE_STRING, llist->dflts[0]->realtype->basetype);
    assert_string_equal("/e:d2[.='c']", llist->dflts[1]->_canonical);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module r {yang-version 1.1; namespace urn:r;prefix r;"
            "typedef mytype {type uint8; default 200;}"
            "leaf r {type mytype;} leaf-list lr {type mytype;}"
            "deviation /r:r {deviate replace {type string;}}"
            "deviation /r:lr {deviate replace {type string;}}}", LYS_IN_YANG, &mod));
    assert_non_null(leaf = (struct lysc_node_leaf *)mod->compiled->data);
    assert_string_equal("r", leaf->name);
    assert_null(leaf->dflt);
    assert_non_null(llist = (struct lysc_node_leaflist *)leaf->next);
    assert_string_equal("lr", llist->name);
    assert_null(llist->dflts);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module s {yang-version 1.1; namespace urn:s;prefix s;"
            "leaf s {type instance-identifier {require-instance true;} default /s:x;}"
            "leaf x {type string;} leaf y {type string;}"
            "deviation /s:s {deviate replace {default /s:y;}}}", LYS_IN_YANG, &mod));
    assert_non_null(leaf = (struct lysc_node_leaf *)mod->compiled->data);
    assert_string_equal("s", leaf->name);
    assert_non_null(leaf->dflt);
    assert_non_null(str = leaf->dflt->realtype->plugin->print(UTEST_LYCTX, leaf->dflt, LY_VALUE_SCHEMA, mod->parsed, &dynamic, NULL));
    assert_string_equal("/s:y", str);
    if (dynamic) {
        free((char *)str);
    }

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module t {namespace urn:t;prefix t;"
            "leaf l {type string;}}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module u {namespace urn:u;prefix u;import t {prefix t;}"
            "identity ident;"
            "deviation /t:l {deviate replace {type identityref {base ident;}}}"
            "}", LYS_IN_YANG, NULL));
    assert_non_null((mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "t")));
    assert_non_null(leaf = (struct lysc_node_leaf *)mod->compiled->data);
    assert_string_equal("l", leaf->name);
    assert_int_equal(LY_TYPE_IDENT, leaf->type->basetype);
    assert_string_equal("ident", ((struct lysc_type_identityref *)leaf->type)->bases[0]->name);

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module v {namespace urn:v;prefix v;"
            "identity ident; identity ident2 { base ident; }"
            "}", LYS_IN_YANG, NULL));
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule w-sub { belongs-to w { prefix w; }"
            "import v { prefix v_pref; }"
            "leaf l { type string; default \"v_pref:ident2\"; }"
            "}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module w {namespace urn:w;prefix w;"
            "include w-sub;"
            "}", LYS_IN_YANG, &mod));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module x {namespace urn:x;prefix x;"
            "import w { prefix w_pref; } import v { prefix v_pref; }"
            "deviation /w_pref:l { deviate replace { type identityref { base v_pref:ident; } } }"
            "}", LYS_IN_YANG, NULL));
    assert_non_null(leaf = (struct lysc_node_leaf *)mod->compiled->data);
    assert_string_equal("l", leaf->name);
    assert_int_equal(LY_TYPE_IDENT, leaf->type->basetype);

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module y {namespace urn:y;prefix y;"
            "container cont {leaf l {type string;}}"
            "leaf bl2 {type string;}"
            "}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module z {namespace urn:z;prefix z;"
            "import y {prefix y;}"
            "deviation \"/y:cont/y:l\" {deviate replace {type leafref {path \"/al\";}}}"
            "leaf al {type string;}"
            "leaf al2 {type leafref {path \"/y:bl2\";}}"
            "}", LYS_IN_YANG, NULL));
    assert_non_null((mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "y")));
    assert_non_null(leaf = (struct lysc_node_leaf *)lysc_node_child(mod->compiled->data));
    assert_string_equal("l", leaf->name);
    assert_int_equal(LY_TYPE_LEAFREF, leaf->type->basetype);

    /* complex dependencies */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module m-base {namespace urn:m-base;prefix mb;"
            "container cont {leaf l {type string;} leaf l2 {type string;}}}", LYS_IN_YANG, NULL));
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module m-base-aug {namespace urn:m-base-aug;prefix mba;"
            "import m-base {prefix mb;}"
            "augment /mb:cont {leaf l {type string;} leaf l2 {type string;}}"
            "container cont2 {leaf l {type string;}}}"
            "\n"
            "module m-base-aug2 {namespace urn:m-base-aug2;prefix mba2;"
            "import m-base {prefix mb;} import m-base-aug {prefix mba;}"
            "augment /mb:cont {leaf augl1 {type string;}}"
            "augment /mba:cont2 {leaf augl2 {type string;}}}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module m-dev {namespace urn:m-dev;prefix md;"
            "import m-base-aug {prefix mba;} import m-base-aug2 {prefix mba2;}"
            "deviation /mba:cont2/mba2:augl2 {deviate not-supported;}}", LYS_IN_YANG, NULL));
    assert_non_null((mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "m-base-aug")));
    node = mod->compiled->data;
    assert_string_equal(node->name, "cont2");
    assert_non_null(node = lysc_node_child(node));
    assert_string_equal(node->name, "l");
    assert_null(node->next);

    assert_int_equal(LY_ENOTFOUND, lys_parse_mem(UTEST_LYCTX, "module aa1 {namespace urn:aa1;prefix aa1;import a {prefix a;}"
            "deviation /a:top/a:z {deviate not-supported;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Deviation(s) target node \"/a:top/a:z\" from module \"aa1\" was not found.", "/a:{deviation='/a:top/a:z'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module aa2 {namespace urn:aa2;prefix aa2;import a {prefix a;}"
            "deviation /a:top/a:a {deviate not-supported;}"
            "deviation /a:top/a:a {deviate add {default error;}}}", LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Multiple deviations of \"/a:top/a:a\" with one of them being \"not-supported\".", "/");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module bb {namespace urn:bb;prefix bb;import a {prefix a;}"
            "deviation a:top/a:a {deviate not-supported;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid absolute-schema-nodeid value \"a:top/a:a\" - \"/\" expected instead of \"a:top\".", "/bb:{deviation='a:top/a:a'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module cc {namespace urn:cc;prefix cc; container c;"
            "deviation /c {deviate add {units meters;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation of container node - it is not possible to add \"units\" property.", "/cc:{deviation='/c'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module cd {namespace urn:cd;prefix cd; leaf c {type string; units centimeters;}"
            "deviation /c {deviate add {units meters;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation adding \"units\" property which already exists (with value \"centimeters\").", "/cd:{deviation='/c'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module dd1 {namespace urn:dd1;prefix dd1; container c;"
            "deviation /c {deviate delete {units meters;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation of container node - it is not possible to delete \"units\" property.", "/dd1:{deviation='/c'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module dd2 {namespace urn:dd2;prefix dd2; leaf c {type string;}"
            "deviation /c {deviate delete {units meters;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation deleting \"units\" property \"meters\" which is not present.", "/dd2:{deviation='/c'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module dd3 {namespace urn:dd3;prefix dd3; leaf c {type string; units centimeters;}"
            "deviation /c {deviate delete {units meters;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation deleting \"units\" property \"meters\" which does not match the target's property value \"centimeters\".",
            "/dd3:{deviation='/c'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ee1 {namespace urn:ee1;prefix ee1; container c;"
            "deviation /c {deviate replace {units meters;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation of container node - it is not possible to replace \"units\" property.", "/ee1:{deviation='/c'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ee2 {namespace urn:ee2;prefix ee2; leaf c {type string;}"
            "deviation /c {deviate replace {units meters;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation replacing \"units\" property \"meters\" which is not present.", "/ee2:{deviation='/c'}");

    /* the default is already deleted in /e:a byt module f */
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ff1 {namespace urn:ff1;prefix ff1; import e {prefix e;}"
            "deviation /e:a {deviate delete {default x:aa;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation deleting \"default\" property \"x:aa\" which is not present.", "/ff1:{deviation='/e:a'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ff3 {namespace urn:ff3;prefix ff3; import e {prefix e;}"
            "deviation /e:b {deviate delete {default e:b;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation deleting \"default\" property \"e:b\" which does not match the target's property value \"x:ba\".", "/ff3:{deviation='/e:b'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ff5 {namespace urn:ff5;prefix ff5; anyxml a;"
            "deviation /a {deviate delete {default x;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation of anyxml node - it is not possible to delete \"default\" property.", "/ff5:{deviation='/a'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ff6 {namespace urn:ff6;prefix ff6; import e {prefix e;}"
            "deviation /e:c {deviate delete {default hi;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation deleting \"default\" property \"hi\" which does not match the target's property value \"hello\".", "/ff6:{deviation='/e:c'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ff7 {namespace urn:ff7;prefix ff7; import e {prefix e;}"
            "deviation /e:d {deviate delete {default hi;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation deleting \"default\" property \"hi\" which does not match any of the target's property values.", "/ff7:{deviation='/e:d'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module gg1 {namespace urn:gg1;prefix gg1; import e {prefix e;}"
            "deviation /e:b {deviate add {default e:a;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation adding \"default\" property which already exists (with value \"x:ba\").", "/gg1:{deviation='/e:b'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module gg2 {namespace urn:gg2;prefix gg2; import e {prefix e;}"
            "deviation /e:a {deviate add {default x:a;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/e:a",
            "Default case prefix \"x\" not found in imports of \"gg2\".", "/e:a");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module gg3 {namespace urn:gg3;prefix gg3; import e {prefix e;}"
            "deviation /e:a {deviate add {default a;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/e:a",
            "Default case \"a\" not found.", "/e:a");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module gg4 {namespace urn:gg4;prefix gg4; import e {prefix e;}"
            "deviation /e:c {deviate add {default hi;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation adding \"default\" property which already exists (with value \"hello\").", "/gg4:{deviation='/e:c'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module gg4 {namespace urn:gg4;prefix gg4; import e {prefix e;}"
            "deviation /e:a {deviate add {default e:ac;}}}", LYS_IN_YANG, &mod));
    /*CHECK_LOG_CTX("Invalid deviation adding \"default\" property \"e:ac\" of choice - mandatory node \"ac\" under the default case.", "/gg4:{deviation='/e:a'}");*/
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/e:a");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module gg5 {namespace urn:gg5;prefix gg5; leaf x {type string; mandatory true;}"
            "deviation /x {deviate add {default error;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/gg5:{deviation='/x'}",
            "Invalid mandatory leaf with a default value.", "/gg5:{deviation='/x'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module hh1 {yang-version 1.1; namespace urn:hh1;prefix hh1; import e {prefix e;}"
            "deviation /e:d {deviate replace {default hi;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation of leaf-list node - it is not possible to replace \"default\" property.", "/hh1:{deviation='/e:d'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ii1 {namespace urn:ii1;prefix ii1; import i {prefix i;}"
            "deviation /i:l1 {deviate delete {unique x;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation deleting \"unique\" property \"x\" which does not match any of the target's property values.", "/ii1:{deviation='/i:l1'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ii2 {namespace urn:ii2;prefix ii2; import i {prefix i;} leaf x { type string;}"
            "deviation /i:l2 {deviate delete {unique d;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation deleting \"unique\" property \"d\" which does not match any of the target's property values.", "/ii2:{deviation='/i:l2'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ii3 {namespace urn:ii3;prefix ii3; leaf x { type string;}"
            "deviation /x {deviate delete {unique d;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation of leaf node - it is not possible to delete \"unique\" property.", "/ii3:{deviation='/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ii4 {namespace urn:ii4;prefix ii4; leaf x { type string;}"
            "deviation /x {deviate add {unique d;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation of leaf node - it is not possible to add \"unique\" property.", "/ii4:{deviation='/x'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module jj1 {namespace urn:jj1;prefix jj1; choice ch {case a {leaf a{type string;}}}"
            "deviation /ch/a {deviate add {config false;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation of case node - it is not possible to add \"config\" property.", "/jj1:{deviation='/ch/a'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module jj2 {namespace urn:jj2;prefix jj2; container top {config false; leaf x {type string;}}"
            "deviation /top/x {deviate add {config true;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/jj2:{deviation='/top/x'}",
            "Configuration node cannot be child of any state data node.", "/jj2:{deviation='/top/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module jj3 {namespace urn:jj3;prefix jj3; container top {leaf x {type string;}}"
            "deviation /top/x {deviate replace {config false;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation replacing \"config\" property \"config false\" which is not present.", "/jj3:{deviation='/top/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module jj4 {namespace urn:jj4;prefix jj4; choice ch {case a {leaf a{type string;}}}"
            "deviation /ch/a {deviate replace {config false;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation of case node - it is not possible to replace \"config\" property.", "/jj4:{deviation='/ch/a'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module jj5 {namespace urn:jj5;prefix jj5; container top {leaf x {type string; config true;}}"
            "deviation /top {deviate add {config false;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/jj5:top",
            "Configuration node cannot be child of any state data node.", "/jj5:top/x");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module jj6 {namespace urn:jj6;prefix jj6; leaf x {config false; type string;}"
            "deviation /x {deviate add {config true;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation adding \"config\" property which already exists (with value \"config false\").", "/jj6:{deviation='/x'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module kk1 {namespace urn:kk1;prefix kk1; container top {leaf a{type string;}}"
            "deviation /top {deviate add {mandatory true;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation of container node - it is not possible to add \"mandatory\" property.", "/kk1:{deviation='/top'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module kk2 {namespace urn:kk2;prefix kk2; container top {leaf a{type string;}}"
            "deviation /top {deviate replace {mandatory true;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation of container node - it is not possible to replace \"mandatory\" property.", "/kk2:{deviation='/top'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module kk3 {namespace urn:kk3;prefix kk3; container top {leaf x {type string;}}"
            "deviation /top/x {deviate replace {mandatory true;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation replacing \"mandatory\" property \"mandatory true\" which is not present.", "/kk3:{deviation='/top/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module kk4 {namespace urn:kk4;prefix kk4; leaf x {mandatory true; type string;}"
            "deviation /x {deviate add {mandatory false;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation adding \"mandatory\" property which already exists (with value \"mandatory true\").", "/kk4:{deviation='/x'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ll1 {namespace urn:ll1;prefix ll1; leaf x {default test; type string;}"
            "deviation /x {deviate add {mandatory true;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/ll1:{deviation='/x'}",
            "Invalid mandatory leaf with a default value.", "/ll1:{deviation='/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ll2 {yang-version 1.1; namespace urn:ll2;prefix ll2; leaf-list x {default test; type string;}"
            "deviation /x {deviate add {min-elements 1;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/ll2:{deviation='/x'}",
            "The default statement is present on leaf-list with a nonzero min-elements.", "/ll2:{deviation='/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module ll2 {namespace urn:ll2;prefix ll2; choice ch {default a; leaf a {type string;} leaf b {type string;}}"
            "deviation /ch {deviate add {mandatory true;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/ll2:ch",
            "Invalid mandatory choice with a default case.", "/ll2:ch");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module mm1 {namespace urn:mm1;prefix mm1; leaf-list x {min-elements 10; type string;}"
            "deviation /x {deviate add {max-elements 5;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/mm1:{deviation='/x'}",
            "Leaf-list min-elements 10 is bigger than max-elements 5.", "/mm1:{deviation='/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module mm2 {namespace urn:mm2;prefix mm2; leaf-list x {max-elements 10; type string;}"
            "deviation /x {deviate add {min-elements 20;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/mm2:{deviation='/x'}",
            "Leaf-list min-elements 20 is bigger than max-elements 10.", "/mm2:{deviation='/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module mm3 {namespace urn:mm3;prefix mm3; list x {min-elements 5; max-elements 10; config false;}"
            "deviation /x {deviate replace {max-elements 1;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/mm3:{deviation='/x'}",
            "List min-elements 5 is bigger than max-elements 1.", "/mm3:{deviation='/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module mm4 {namespace urn:mm4;prefix mm4; list x {min-elements 5; max-elements 10; config false;}"
            "deviation /x {deviate replace {min-elements 20;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/mm4:{deviation='/x'}",
            "List min-elements 20 is bigger than max-elements 10.", "/mm4:{deviation='/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module mm5 {namespace urn:mm5;prefix mm5; leaf-list x {type string; min-elements 5;}"
            "deviation /x {deviate add {min-elements 1;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation adding \"min-elements\" property which already exists (with value \"5\").", "/mm5:{deviation='/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module mm6 {namespace urn:mm6;prefix mm6; list x {config false; min-elements 5;}"
            "deviation /x {deviate add {min-elements 1;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation adding \"min-elements\" property which already exists (with value \"5\").", "/mm6:{deviation='/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module mm7 {namespace urn:mm7;prefix mm7; leaf-list x {type string; max-elements 5;}"
            "deviation /x {deviate add {max-elements 1;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation adding \"max-elements\" property which already exists (with value \"5\").", "/mm7:{deviation='/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module mm8 {namespace urn:mm8;prefix mm8; list x {config false; max-elements 5;}"
            "deviation /x {deviate add {max-elements 1;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation adding \"max-elements\" property which already exists (with value \"5\").", "/mm8:{deviation='/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module mm9 {namespace urn:mm9;prefix mm9; leaf-list x {type string;}"
            "deviation /x {deviate replace {min-elements 1;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation replacing \"min-elements\" property which is not present.", "/mm9:{deviation='/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module mm10 {namespace urn:mm10;prefix mm10; list x {config false;}"
            "deviation /x {deviate replace {min-elements 1;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation replacing \"min-elements\" property which is not present.", "/mm10:{deviation='/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module mm11 {namespace urn:mm11;prefix mm11; leaf-list x {type string;}"
            "deviation /x {deviate replace {max-elements 1;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation replacing \"max-elements\" property which is not present.", "/mm11:{deviation='/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module mm12 {namespace urn:mm12;prefix mm12; list x {config false; }"
            "deviation /x {deviate replace {max-elements 1;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation replacing \"max-elements\" property which is not present.", "/mm12:{deviation='/x'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module nn1 {namespace urn:nn1;prefix nn1; anyxml x;"
            "deviation /x {deviate replace {type string;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid deviation of anyxml node - it is not possible to replace \"type\" property.", "/nn1:{deviation='/x'}");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module nn2 {namespace urn:nn2;prefix nn2; leaf-list x {type string;}"
            "deviation /x {deviate replace {type empty;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Compilation of a deviated and/or refined node failed.", "/nn2:{deviation='/x'}",
            "Leaf-list of type \"empty\" is allowed only in YANG 1.1 modules.", "/nn2:{deviation='/x'}");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module oo1 {namespace urn:oo1;prefix oo1; leaf x {type uint16; default 300;}"
            "deviation /x {deviate replace {type uint8;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid default - value does not fit the type "
            "(Value \"300\" is out of uint8's min/max bounds.).", "Schema location /oo1:x.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module oo2 {yang-version 1.1;namespace urn:oo2;prefix oo2; leaf-list x {type uint16; default 10; default 300;}"
            "deviation /x {deviate replace {type uint8;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid default - value does not fit the type "
            "(Value \"300\" is out of uint8's min/max bounds.).", "Schema location /oo2:x.");
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module oo3 {namespace urn:oo3;prefix oo3; leaf x {type uint8;}"
            "deviation /x {deviate add {default 300;}}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Invalid default - value does not fit the type "
            "(Value \"300\" is out of uint8's min/max bounds.).", "Schema location /oo3:x.");

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module pp {namespace urn:pp;prefix pp; leaf l { type leafref {path /c/x;}}"
            "container c {leaf x {type string;} leaf y {type string;}}}", LYS_IN_YANG, &mod));
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, "module pp1 {namespace urn:pp1;prefix pp1; import pp {prefix pp;}"
            "deviation /pp:c/pp:x {deviate not-supported;}}", LYS_IN_YANG, &mod));
    CHECK_LOG_CTX("Not found node \"x\" in path.", "Schema location /pp:l.");
}

static void
test_when(void **state)
{
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX,
            "module a {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    container cont {\n"
            "        leaf l {\n"
            "            when \"/cont/lst[val='25']\";\n"
            "            type empty;\n"
            "        }\n"
            "        list lst {\n"
            "            key \"k\";\n"
            "            leaf k {\n"
            "                type uint8;\n"
            "            }\n"
            "            leaf val {\n"
            "                when /cont2;\n"
            "                type int32;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    container cont2 {\n"
            "        presence \"a\";\n"
            "        when ../cont/l;\n"
            "    }\n"
            "}",
            LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("When condition includes a self-reference.", "Schema location /a:cont/lst/val.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX,
            "module a {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    container cont {\n"
            "        leaf l {\n"
            "            when \"/cont/lst[val='25']\";\n"
            "            type empty;\n"
            "        }\n"
            "        list lst {\n"
            "            key \"k\";\n"
            "            leaf k {\n"
            "                type uint8;\n"
            "            }\n"
            "            leaf val {\n"
            "                when /cont2;\n"
            "                type int32;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    container cont2 {\n"
            "        presence \"a\";\n"
            "        when ../cont/lst/val;\n"
            "    }\n"
            "}",
            LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("When condition includes a self-reference.", "Schema location /a:cont/lst/val.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX,
            "module a {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    leaf val {\n"
            "        type int64;\n"
            "        when \"../val='25'\";\n"
            "    }\n"
            "}",
            LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("When condition is accessing its own conditional node value.", "Schema location /a:val.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX,
            "module a {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    grouping grp {\n"
            "        leaf val {\n"
            "            type int64;\n"
            "        }\n"
            "    }\n"
            "    uses grp {\n"
            "        when \"val='25'\";\n"
            "    }\n"
            "}",
            LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("When condition is accessing its own conditional node value.", "Schema location /a:val.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX,
            "module a {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    augment /cont {\n"
            "        when \"val='25'\";\n"
            "        leaf val {\n"
            "            type int64;\n"
            "        }\n"
            "    }\n"
            "    container cont;\n"
            "}",
            LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("When condition is accessing its own conditional node value.", "Schema location /a:cont/val.");

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX,
            "module a {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    augment /cont {\n"
            "        when \"aug-cont/aug-l\";\n"
            "        container aug-cont {\n"
            "            leaf aug-l {\n"
            "                type int64;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    container cont;\n"
            "}",
            LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("When condition is accessing its own conditional node children.", "Schema location /a:cont/aug-cont.");

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX,
            "module b {\n"
            "    namespace urn:b;\n"
            "    prefix b;\n"
            "    container c {\n"
            "        list l {\n"
            "            key name;\n"
            "            leaf name {\n"
            "                type string;\n"
            "            }\n"
            "\n"
            "            container cond-data {\n"
            "                when \"/c/l2[name = current()/../name]/val = 'value'\";\n"
            "                leaf cond-leaf {\n"
            "                    type string;\n"
            "                    default \"default_val\";\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "        list l2 {\n"
            "            key name;\n"
            "            leaf name {\n"
            "                type string;\n"
            "            }\n"
            "\n"
            "            container c2 {\n"
            "                leaf val {\n"
            "                    type string;\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}",
            LYS_IN_YANG, NULL));
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_module, setup),
        UTEST(test_name_collisions, setup),
        UTEST(test_type_length, setup),
        UTEST(test_type_range, setup),
        UTEST(test_type_pattern, setup),
        UTEST(test_type_enum, setup),
        UTEST(test_type_bits, setup),
        UTEST(test_type_dec64, setup),
        UTEST(test_type_instanceid, setup),
        UTEST(test_type_identityref, setup),
        UTEST(test_type_leafref, setup),
        UTEST(test_type_empty, setup),
        UTEST(test_type_union, setup),
        UTEST(test_type_dflt, setup),
        UTEST(test_status, setup),
        UTEST(test_node_container, setup),
        UTEST(test_node_leaflist, setup),
        UTEST(test_node_list, setup),
        UTEST(test_node_choice, setup),
        UTEST(test_node_anydata, setup),
        UTEST(test_action, setup),
        UTEST(test_notification, setup),
        UTEST(test_grouping, setup),
        UTEST(test_uses, setup),
        UTEST(test_refine, setup),
        UTEST(test_augment, setup),
        UTEST(test_deviation, setup),
        UTEST(test_when, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
