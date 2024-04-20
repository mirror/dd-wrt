/**
 * @file test_context.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from context.c
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
#include "context.h"
#include "in.h"
#include "schema_compile.h"
#include "tests_config.h"
#include "tree_schema_internal.h"
#ifdef _WIN32
static void
slashes_to_backslashes(char *path)
{
    while ((path = strchr(path, '/'))) {
        *path++ = '\\';
    }
}

static void
test_searchdirs(void **state)
{
    const char * const *list;
    char *path1 = strdup(TESTS_BIN "/utests");
    char *path2 = strdup(TESTS_SRC);

    slashes_to_backslashes(path1);
    slashes_to_backslashes(path2);

    assert_int_equal(LY_EINVAL, ly_ctx_set_searchdir(NULL, NULL));
    CHECK_LOG("Invalid argument ctx (ly_ctx_set_searchdir()).", NULL);
    assert_null(ly_ctx_get_searchdirs(NULL));
    CHECK_LOG("Invalid argument ctx (ly_ctx_get_searchdirs()).", NULL);
    assert_int_equal(LY_EINVAL, ly_ctx_unset_searchdir(NULL, NULL));
    CHECK_LOG("Invalid argument ctx (ly_ctx_unset_searchdir()).", NULL);

    /* correct path */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, path1));
    assert_int_equal(1, UTEST_LYCTX->search_paths.count);
    assert_string_equal(path1, UTEST_LYCTX->search_paths.objs[0]);

    /* duplicated paths */
    assert_int_equal(LY_EEXIST, ly_ctx_set_searchdir(UTEST_LYCTX, path1));
    assert_int_equal(1, UTEST_LYCTX->search_paths.count);
    assert_string_equal(path1, UTEST_LYCTX->search_paths.objs[0]);

    /* another path */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, path2));
    assert_int_equal(2, UTEST_LYCTX->search_paths.count);
    assert_string_equal(path2, UTEST_LYCTX->search_paths.objs[1]);

    /* get searchpaths */
    list = ly_ctx_get_searchdirs(UTEST_LYCTX);
    assert_non_null(list);
    assert_string_equal(path1, list[0]);
    assert_string_equal(path2, list[1]);
    assert_null(list[2]);

    /* removing searchpaths */
    /* nonexisting */
    assert_int_equal(LY_EINVAL, ly_ctx_unset_searchdir(UTEST_LYCTX, "/nonexistingfile"));
    CHECK_LOG_CTX("Invalid argument value (ly_ctx_unset_searchdir()).", NULL);

    /* first */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdir(UTEST_LYCTX, path1));
    assert_int_equal(1, UTEST_LYCTX->search_paths.count);
    assert_string_not_equal(path1, list[0]);

    /* second */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdir(UTEST_LYCTX, path2));
    assert_int_equal(0, UTEST_LYCTX->search_paths.count);

    free(path1);
    free(path2);
}

#else

static void
test_searchdirs(void **state)
{
    const char * const *list;

    /* invalid arguments */
    assert_int_equal(LY_EINVAL, ly_ctx_set_searchdir(NULL, NULL));
    CHECK_LOG("Invalid argument ctx (ly_ctx_set_searchdir()).", NULL);
    assert_null(ly_ctx_get_searchdirs(NULL));
    CHECK_LOG("Invalid argument ctx (ly_ctx_get_searchdirs()).", NULL);
    assert_int_equal(LY_EINVAL, ly_ctx_unset_searchdir(NULL, NULL));
    CHECK_LOG("Invalid argument ctx (ly_ctx_unset_searchdir()).", NULL);

    /* readable and executable, but not a directory */
    assert_int_equal(LY_EINVAL, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_BIN "/utest_context"));
    CHECK_LOG_CTX("Given search directory \""TESTS_BIN "/utest_context\" is not a directory.", NULL);
    /* not existing */
    assert_int_equal(LY_EINVAL, ly_ctx_set_searchdir(UTEST_LYCTX, "/nonexistingfile"));
    CHECK_LOG_CTX("Unable to use search directory \"/nonexistingfile\" (No such file or directory).", NULL);

    /* ly_set_add() fails */
    /* no change */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, NULL));

    /* correct path */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_BIN "/utests"));
    assert_int_equal(1, UTEST_LYCTX->search_paths.count);
    assert_string_equal(TESTS_BIN "/utests", UTEST_LYCTX->search_paths.objs[0]);

    /* duplicated paths */
    assert_int_equal(LY_EEXIST, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_BIN "/utests"));
    assert_int_equal(1, UTEST_LYCTX->search_paths.count);
    assert_string_equal(TESTS_BIN "/utests", UTEST_LYCTX->search_paths.objs[0]);

    /* another paths - add 8 to fill the initial buffer of the searchpaths list */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_BIN "/CMakeFiles"));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_SRC "/../src"));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_SRC "/../CMakeModules"));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_SRC "/../doc"));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_SRC));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_BIN));
    assert_int_equal(7, UTEST_LYCTX->search_paths.count);

    /* get searchpaths */
    list = ly_ctx_get_searchdirs(UTEST_LYCTX);
    assert_non_null(list);
    assert_string_equal(TESTS_BIN "/utests", list[0]);
    assert_string_equal(TESTS_BIN "/CMakeFiles", list[1]);
    assert_string_equal(TESTS_SRC, list[5]);
    assert_string_equal(TESTS_BIN, list[6]);
    assert_null(list[7]);

    /* removing searchpaths */
    /* nonexisting */
    assert_int_equal(LY_EINVAL, ly_ctx_unset_searchdir(UTEST_LYCTX, "/nonexistingfile"));
    CHECK_LOG_CTX("Invalid argument value (ly_ctx_unset_searchdir()).", NULL);
    /* first */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdir(UTEST_LYCTX, TESTS_BIN "/utests"));
    assert_string_not_equal(TESTS_BIN "/utests", list[0]);
    assert_int_equal(6, UTEST_LYCTX->search_paths.count);
    /* middle */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdir(UTEST_LYCTX, TESTS_SRC));
    assert_int_equal(5, UTEST_LYCTX->search_paths.count);
    /* last */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdir(UTEST_LYCTX, TESTS_BIN));
    assert_int_equal(4, UTEST_LYCTX->search_paths.count);
    /* all */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdir(UTEST_LYCTX, NULL));
    assert_int_equal(0, UTEST_LYCTX->search_paths.count);

    /* again - no change */
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_searchdir(UTEST_LYCTX, NULL));

    /* cleanup */
    ly_ctx_destroy(UTEST_LYCTX);

    /* test searchdir list in ly_ctx_new() */
    assert_int_equal(LY_EINVAL, ly_ctx_new("/nonexistingfile", 0, &UTEST_LYCTX));
    CHECK_LOG("Unable to use search directory \"/nonexistingfile\" (No such file or directory).", NULL);
    assert_int_equal(LY_SUCCESS,
            ly_ctx_new(TESTS_SRC PATH_SEPARATOR TESTS_BIN PATH_SEPARATOR TESTS_BIN PATH_SEPARATOR TESTS_SRC,
            LY_CTX_DISABLE_SEARCHDIRS, &UTEST_LYCTX));
    assert_int_equal(2, UTEST_LYCTX->search_paths.count);
    assert_string_equal(TESTS_SRC, UTEST_LYCTX->search_paths.objs[0]);
    assert_string_equal(TESTS_BIN, UTEST_LYCTX->search_paths.objs[1]);
}

#endif

static void
test_options(void **state)
{
    /* use own context with extra flags */
    ly_ctx_destroy(UTEST_LYCTX);

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0xffff, &UTEST_LYCTX));

    /* invalid arguments */
    assert_int_equal(0, ly_ctx_get_options(NULL));
    CHECK_LOG("Invalid argument ctx (ly_ctx_get_options()).", NULL);

    assert_int_equal(LY_EINVAL, ly_ctx_set_options(NULL, 0));
    CHECK_LOG("Invalid argument ctx (ly_ctx_set_options()).", NULL);
    assert_int_equal(LY_EINVAL, ly_ctx_unset_options(NULL, 0));
    CHECK_LOG("Invalid argument ctx (ly_ctx_unset_options()).", NULL);

    /* unset */
    /* LY_CTX_ALL_IMPLEMENTED */
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_ALL_IMPLEMENTED);
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_ALL_IMPLEMENTED));
    assert_int_equal(0, UTEST_LYCTX->flags & LY_CTX_ALL_IMPLEMENTED);

    /* LY_CTX_REF_IMPLEMENTED */
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_REF_IMPLEMENTED);
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_REF_IMPLEMENTED));
    assert_int_equal(0, UTEST_LYCTX->flags & LY_CTX_REF_IMPLEMENTED);

    /* LY_CTX_DISABLE_SEARCHDIRS */
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_DISABLE_SEARCHDIRS);
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_DISABLE_SEARCHDIRS));
    assert_int_equal(0, UTEST_LYCTX->flags & LY_CTX_DISABLE_SEARCHDIRS);

    /* LY_CTX_DISABLE_SEARCHDIR_CWD */
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_DISABLE_SEARCHDIR_CWD);
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_DISABLE_SEARCHDIR_CWD));
    assert_int_equal(0, UTEST_LYCTX->flags & LY_CTX_DISABLE_SEARCHDIR_CWD);

    /* LY_CTX_PREFER_SEARCHDIRS */
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_PREFER_SEARCHDIRS);
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_PREFER_SEARCHDIRS));
    assert_int_equal(0, UTEST_LYCTX->flags & LY_CTX_PREFER_SEARCHDIRS);

    assert_int_equal(UTEST_LYCTX->flags, ly_ctx_get_options(UTEST_LYCTX));

    /* set back */
    /* LY_CTX_ALL_IMPLEMENTED */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_options(UTEST_LYCTX, LY_CTX_ALL_IMPLEMENTED));
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_ALL_IMPLEMENTED);

    /* LY_CTX_REF_IMPLEMENTED */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_options(UTEST_LYCTX, LY_CTX_REF_IMPLEMENTED));
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_REF_IMPLEMENTED);

    /* LY_CTX_DISABLE_SEARCHDIRS */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_options(UTEST_LYCTX, LY_CTX_DISABLE_SEARCHDIRS));
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_DISABLE_SEARCHDIRS);

    /* LY_CTX_DISABLE_SEARCHDIR_CWD */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_options(UTEST_LYCTX, LY_CTX_DISABLE_SEARCHDIR_CWD));
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_DISABLE_SEARCHDIR_CWD);

    /* LY_CTX_PREFER_SEARCHDIRS */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_options(UTEST_LYCTX, LY_CTX_PREFER_SEARCHDIRS));
    assert_int_not_equal(0, UTEST_LYCTX->flags & LY_CTX_PREFER_SEARCHDIRS);

    assert_int_equal(UTEST_LYCTX->flags, ly_ctx_get_options(UTEST_LYCTX));
}

static LY_ERR
test_imp_clb(const char *UNUSED(mod_name), const char *UNUSED(mod_rev), const char *UNUSED(submod_name),
        const char *UNUSED(sub_rev), void *user_data, LYS_INFORMAT *format,
        const char **module_data, void (**free_module_data)(void *model_data, void *user_data))
{
    *module_data = user_data;
    *format = LYS_IN_YANG;
    *free_module_data = NULL;
    return LY_SUCCESS;
}

static void
test_models(void **state)
{
    struct ly_in *in;
    const char *str;
    struct lys_module *mod1, *mod2;
    struct lys_glob_unres unres = {0};

    /* use own context with extra flags */
    ly_ctx_destroy(UTEST_LYCTX);

    /* invalid arguments */
    assert_int_equal(0, ly_ctx_get_change_count(NULL));
    CHECK_LOG("Invalid argument ctx (ly_ctx_get_change_count()).", NULL);

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_DISABLE_SEARCHDIRS, &UTEST_LYCTX));
    assert_int_equal(UTEST_LYCTX->change_count, ly_ctx_get_change_count(UTEST_LYCTX));

    assert_int_equal(LY_SUCCESS, ly_in_new_memory("module x {namespace urn:x;prefix x;}", &in));
    assert_int_equal(LY_EINVAL, lys_parse_in(UTEST_LYCTX, in, 4, NULL, NULL, &unres.creating, &mod1));
    lys_unres_glob_erase(&unres);
    ly_in_free(in, 0);
    CHECK_LOG_CTX("Invalid schema input format.", NULL);

    /* import callback */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, (void *)(str = "test"));
    assert_ptr_equal(test_imp_clb, UTEST_LYCTX->imp_clb);
    assert_ptr_equal(str, UTEST_LYCTX->imp_clb_data);
    assert_ptr_equal(test_imp_clb, ly_ctx_get_module_imp_clb(UTEST_LYCTX, (void **)&str));
    assert_string_equal("test", str);

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, NULL, NULL);
    assert_null(UTEST_LYCTX->imp_clb);
    assert_null(UTEST_LYCTX->imp_clb_data);

    /* name collision of module and submodule */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule y {belongs-to a {prefix a;} revision 2018-10-30;}");
    assert_int_equal(LY_SUCCESS, ly_in_new_memory("module y {namespace urn:y;prefix y;include y;}", &in));
    assert_int_equal(LY_EVALID, lys_parse_in(UTEST_LYCTX, in, LYS_IN_YANG, NULL, NULL, &unres.creating, &mod1));
    lys_unres_glob_erase(&unres);
    ly_in_free(in, 0);
    CHECK_LOG_CTX("Parsing module \"y\" failed.", NULL);
    CHECK_LOG_CTX("Name collision between module and submodule of name \"y\".", "Line number 1.");

    assert_int_equal(LY_SUCCESS, ly_in_new_memory("module a {namespace urn:a;prefix a;include y;revision 2018-10-30; }", &in));
    assert_int_equal(LY_SUCCESS, lys_parse_in(UTEST_LYCTX, in, LYS_IN_YANG, NULL, NULL, &unres.creating, &mod1));
    ly_in_free(in, 0);
    assert_int_equal(LY_SUCCESS, ly_in_new_memory("module y {namespace urn:y;prefix y;}", &in));
    assert_int_equal(LY_EVALID, lys_parse_in(UTEST_LYCTX, in, LYS_IN_YANG, NULL, NULL, &unres.creating, &mod1));
    lys_unres_glob_erase(&unres);
    ly_in_free(in, 0);
    CHECK_LOG_CTX("Parsing module \"y\" failed.", NULL);
    CHECK_LOG_CTX("Name collision between module and submodule of name \"y\".", "Line number 1.");

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule y {belongs-to b {prefix b;}}");
    assert_int_equal(LY_SUCCESS, ly_in_new_memory("module b {namespace urn:b;prefix b;include y;}", &in));
    assert_int_equal(LY_EVALID, lys_parse_in(UTEST_LYCTX, in, LYS_IN_YANG, NULL, NULL, &unres.creating, &mod1));
    lys_unres_glob_revert(UTEST_LYCTX, &unres);
    lys_unres_glob_erase(&unres);
    ly_in_free(in, 0);
    CHECK_LOG_CTX("Parsing module \"b\" failed.", NULL);
    CHECK_LOG_CTX("Including \"y\" submodule into \"b\" failed.", NULL);
    CHECK_LOG_CTX("Parsing submodule failed.", NULL);
    CHECK_LOG_CTX("Name collision between submodules of name \"y\".", "Line number 1.");

    /* selecting correct revision of the submodules */
    ly_ctx_reset_latests(UTEST_LYCTX);
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule y {belongs-to a {prefix a;} revision 2018-10-31;}");
    assert_int_equal(LY_SUCCESS, ly_in_new_memory("module a {namespace urn:a;prefix a;include y; revision 2018-10-31;}", &in));
    assert_int_equal(LY_SUCCESS, lys_parse_in(UTEST_LYCTX, in, LYS_IN_YANG, NULL, NULL, &unres.creating, &mod2));
    lys_unres_glob_erase(&unres);
    ly_in_free(in, 0);
    assert_string_equal("2018-10-31", mod2->parsed->includes[0].submodule->revs[0].date);

    /* reloading module in case only the compiled module resists in the context */
    assert_int_equal(LY_SUCCESS, ly_in_new_memory("module w {namespace urn:w;prefix w;revision 2018-10-24;}", &in));
    assert_int_equal(LY_SUCCESS, lys_parse(UTEST_LYCTX, in, LYS_IN_YANG, NULL, &mod1));
    ly_in_free(in, 0);
    assert_non_null(mod1->compiled);
    assert_non_null(mod1->parsed);

    assert_int_equal(LY_SUCCESS, ly_in_new_memory("module z {namespace urn:z;prefix z;import w {prefix w;revision-date 2018-10-24;}}", &in));
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module w {namespace urn:w;prefix w;revision 2018-10-24;}");
    assert_int_equal(LY_SUCCESS, lys_parse(UTEST_LYCTX, in, LYS_IN_YANG, NULL, &mod2));
    ly_in_free(in, 0);
    assert_non_null(mod2);
    assert_non_null(mod1->parsed);
    assert_string_equal("w", mod1->name);
}

static void
test_imports(void **state)
{
    struct lys_module *mod1, *mod2, *mod3, *import;
    char *str;
    uint16_t ctx_options;

    /* use own context with extra flags */
    ly_ctx_destroy(UTEST_LYCTX);
    ctx_options = LY_CTX_DISABLE_SEARCHDIRS | LY_CTX_NO_YANGLIBRARY;

    /* Import callback provides newer revision of module 'a',
     * however the older revision is implemented soon and therefore it is preferred. */
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, ctx_options, &UTEST_LYCTX));
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module a {namespace urn:a; prefix a; revision 2019-09-17;}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {namespace urn:a;prefix a;revision 2019-09-16;}",
            LYS_IN_YANG, &mod1));
    assert_true(LYS_MOD_LATEST_REV & mod1->latest_revision);
    assert_int_equal(1, mod1->implemented);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b;import a {prefix a;}}",
            LYS_IN_YANG, &mod2));
    assert_ptr_equal(mod1, mod2->parsed->imports[0].module);
    assert_true((LYS_MOD_LATEST_REV | LYS_MOD_IMPORTED_REV) & mod1->latest_revision);
    assert_string_equal("2019-09-16", mod1->revision);
    assert_int_equal(1, mod1->implemented);
    assert_non_null(ly_ctx_get_module(UTEST_LYCTX, "a", "2019-09-16"));
    ly_ctx_destroy(UTEST_LYCTX);

    /* Import callback provides older revision of module 'a' and it is
     * imported by another module, so it is preferred even if newer
     * revision is implemented later. */
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, ctx_options, &UTEST_LYCTX));
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "module a {namespace urn:a; prefix a; revision 2019-09-16;}");
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module b {namespace urn:b;prefix b;import a {prefix a;}}",
            LYS_IN_YANG, &mod2));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {namespace urn:a;prefix a;revision 2019-09-17;}",
            LYS_IN_YANG, &mod1));
    ly_log_level(LY_LLVRB);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module c {namespace urn:c;prefix c;import a {prefix a;}}",
            LYS_IN_YANG, &mod3));
    CHECK_LOG("Implemented module \"a@2019-09-17\" is not used for import, revision \"2019-09-16\" is imported instead.", NULL);
    ly_log_level(LY_LLWRN);
    assert_true(LYS_MOD_LATEST_SEARCHDIRS & mod1->latest_revision);
    assert_int_equal(1, mod1->implemented);
    import = mod2->parsed->imports[0].module;
    assert_true(LYS_MOD_IMPORTED_REV & import->latest_revision);
    assert_string_equal("2019-09-16", import->revision);
    assert_int_equal(0, import->implemented);
    import = mod3->parsed->imports[0].module;
    assert_string_equal("2019-09-16", import->revision);
    assert_non_null(ly_ctx_get_module(UTEST_LYCTX, "a", "2019-09-16"));
    assert_non_null(ly_ctx_get_module(UTEST_LYCTX, "a", "2019-09-17"));
    assert_string_equal("2019-09-17", ly_ctx_get_module_implemented(UTEST_LYCTX, "a")->revision);
    ly_ctx_destroy(UTEST_LYCTX);

    /* check of circular dependency */
    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, ctx_options, &UTEST_LYCTX));
    str = "module a {namespace urn:a; prefix a;"
            "import b {prefix b;}"
            "}";
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, str);
    str = "module b { yang-version 1.1; namespace urn:b; prefix b;"
            "import a {prefix a;}"
            "}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL));
    ly_err_clean(UTEST_LYCTX, NULL);
}

static void
test_get_models(void **state)
{
    struct lys_module *mod, *mod2;
    const char *str0 = "module a {namespace urn:a;prefix a;}";
    const char *str1 = "module a {namespace urn:a;prefix a;revision 2018-10-23;}";
    const char *str2 = "module a {namespace urn:a;prefix a;revision 2018-10-23;revision 2018-10-24;}";
    struct ly_in *in0, *in1, *in2;
    struct lys_glob_unres unres = {0};

    unsigned int index = 0;
    const char *names[] = {
        "ietf-yang-metadata", "yang", "ietf-inet-types", "ietf-yang-types", "ietf-yang-schema-mount",
        "ietf-yang-structure-ext", "ietf-datastores", "ietf-yang-library", "a", "a", "a"
    };

    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str0, &in0));
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str1, &in1));
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str2, &in2));

    /* invalid arguments */
    assert_ptr_equal(NULL, ly_ctx_get_module(NULL, NULL, NULL));
    CHECK_LOG("Invalid argument ctx (ly_ctx_get_module()).", NULL);
    assert_ptr_equal(NULL, ly_ctx_get_module(UTEST_LYCTX, NULL, NULL));
    CHECK_LOG_CTX("Invalid argument name (ly_ctx_get_module()).", NULL);
    assert_ptr_equal(NULL, ly_ctx_get_module_ns(NULL, NULL, NULL));
    CHECK_LOG("Invalid argument ctx (ly_ctx_get_module_ns()).", NULL);
    assert_ptr_equal(NULL, ly_ctx_get_module_ns(UTEST_LYCTX, NULL, NULL));
    CHECK_LOG_CTX("Invalid argument ns (ly_ctx_get_module_ns()).", NULL);
    assert_null(ly_ctx_get_module(UTEST_LYCTX, "nonsence", NULL));

    /* internal modules */
    assert_null(ly_ctx_get_module_implemented(UTEST_LYCTX, "ietf-yang-types"));
    mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "yang");
    assert_non_null(mod);
    assert_non_null(mod->parsed);
    assert_string_equal("yang", mod->name);
    mod2 = ly_ctx_get_module_implemented_ns(UTEST_LYCTX, mod->ns);
    assert_ptr_equal(mod, mod2);
    assert_non_null(ly_ctx_get_module(UTEST_LYCTX, "ietf-yang-metadata", "2016-08-05"));
    assert_non_null(ly_ctx_get_module(UTEST_LYCTX, "ietf-yang-types", "2013-07-15"));
    assert_non_null(ly_ctx_get_module(UTEST_LYCTX, "ietf-inet-types", "2013-07-15"));
    assert_non_null(ly_ctx_get_module_ns(UTEST_LYCTX, "urn:ietf:params:xml:ns:yang:ietf-datastores", "2018-02-14"));

    /* select module by revision */
    assert_int_equal(LY_SUCCESS, lys_parse(UTEST_LYCTX, in1, LYS_IN_YANG, NULL, &mod));
    /* invalid attempts - implementing module of the same name and inserting the same module */
    assert_int_equal(LY_SUCCESS, lys_parse_in(UTEST_LYCTX, in2, LYS_IN_YANG, NULL, NULL, &unres.creating, &mod2));
    assert_int_equal(LY_EDENIED, lys_implement(mod2, NULL, &unres));
    CHECK_LOG_CTX("Module \"a@2018-10-24\" is already implemented in revision \"2018-10-23\".", NULL);
    lys_unres_glob_erase(&unres);
    ly_in_reset(in1);
    /* it is already there, fine */
    assert_int_equal(LY_SUCCESS, lys_parse_in(UTEST_LYCTX, in1, LYS_IN_YANG, NULL, NULL, &unres.creating, NULL));
    /* insert the second module only as imported, not implemented */
    lys_unres_glob_erase(&unres);
    ly_in_reset(in2);
    assert_int_equal(LY_SUCCESS, lys_parse_in(UTEST_LYCTX, in2, LYS_IN_YANG, NULL, NULL, &unres.creating, &mod2));
    lys_unres_glob_erase(&unres);
    assert_non_null(mod2);
    assert_ptr_not_equal(mod, mod2);
    mod = ly_ctx_get_module_latest(UTEST_LYCTX, "a");
    assert_ptr_equal(mod, mod2);
    mod2 = ly_ctx_get_module_latest_ns(UTEST_LYCTX, mod->ns);
    assert_ptr_equal(mod, mod2);
    /* work with module with no revision */
    assert_int_equal(LY_SUCCESS, lys_parse_in(UTEST_LYCTX, in0, LYS_IN_YANG, NULL, NULL, &unres.creating, &mod));
    lys_unres_glob_erase(&unres);
    assert_ptr_equal(mod, ly_ctx_get_module(UTEST_LYCTX, "a", NULL));
    assert_ptr_not_equal(mod, ly_ctx_get_module_latest(UTEST_LYCTX, "a"));

    str1 = "submodule b {belongs-to a {prefix a;}}";
    ly_in_free(in1, 0);
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str1, &in1));
    assert_int_equal(LY_EINVAL, lys_parse_in(UTEST_LYCTX, in1, LYS_IN_YANG, NULL, NULL, &unres.creating, &mod));
    CHECK_LOG_CTX("Input data contains submodule which cannot be parsed directly without its main module.", NULL);
    lys_unres_glob_erase(&unres);

    while ((mod = (struct lys_module *)ly_ctx_get_module_iter(UTEST_LYCTX, &index))) {
        assert_string_equal(names[index - 1], mod->name);
    }
    assert_int_equal(11, index);

    /* cleanup */
    ly_in_free(in0, 0);
    ly_in_free(in1, 0);
    ly_in_free(in2, 0);
}

static void
test_ylmem(void **state)
{
#define DATA_YANG_LIBRARY_START "<yang-library xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"\
    "  <module-set>\n"\
    "    <name>complete</name>\n"\
    "    <module>\n"\
    "      <name>yang</name>\n"\
    "      <revision>2022-06-16</revision>\n"\
    "      <namespace>urn:ietf:params:xml:ns:yang:1</namespace>\n"\
    "    </module>\n"\
    "    <module>\n"\
    "      <name>ietf-yang-library</name>\n"\
    "      <revision>2019-01-04</revision>\n"\
    "      <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>\n"\
    "    </module>\n"

#define DATA_YANG_BASE_IMPORTS     "    <import-only-module>\n"\
    "      <name>ietf-yang-metadata</name>\n"\
    "      <revision>2016-08-05</revision>\n"\
    "      <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-metadata</namespace>\n"\
    "    </import-only-module>\n"\
    "    <import-only-module>\n"\
    "      <name>ietf-inet-types</name>\n"\
    "      <revision>2013-07-15</revision>\n"\
    "      <namespace>urn:ietf:params:xml:ns:yang:ietf-inet-types</namespace>\n"\
    "    </import-only-module>\n"\
    "    <import-only-module>\n"\
    "      <name>ietf-yang-types</name>\n"\
    "      <revision>2013-07-15</revision>\n"\
    "      <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-types</namespace>\n"\
    "    </import-only-module>\n"\
    "    <import-only-module>\n"\
    "      <name>ietf-datastores</name>\n"\
    "      <revision>2018-02-14</revision>\n"\
    "      <namespace>urn:ietf:params:xml:ns:yang:ietf-datastores</namespace>\n"\
    "    </import-only-module>\n"

#define DATA_YANG_SCHEMA_MODULE_STATE         "  </module-set>\n"\
    "  <schema>\n"\
    "    <name>complete</name>\n"\
    "    <module-set>complete</module-set>\n"\
    "  </schema>\n"\
    "  <content-id>9</content-id>\n"\
    "</yang-library>\n"\
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"\
    "  <module-set-id>12</module-set-id>\n"\
    "  <module>\n"\
    "    <name>ietf-yang-metadata</name>\n"\
    "    <revision>2016-08-05</revision>\n"\
    "    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-metadata</namespace>\n"\
    "    <conformance-type>import</conformance-type>\n"\
    "  </module>\n"\
    "  <module>\n"\
    "    <name>yang</name>\n"\
    "    <revision>2022-06-16</revision>\n"\
    "    <namespace>urn:ietf:params:xml:ns:yang:1</namespace>\n"\
    "    <conformance-type>implement</conformance-type>\n"\
    "  </module>\n"\
    "  <module>\n"\
    "    <name>ietf-inet-types</name>\n"\
    "    <revision>2013-07-15</revision>\n"\
    "    <namespace>urn:ietf:params:xml:ns:yang:ietf-inet-types</namespace>\n"\
    "    <conformance-type>import</conformance-type>\n"\
    "  </module>\n"\
    "  <module>\n"\
    "    <name>ietf-yang-types</name>\n"\
    "    <revision>2013-07-15</revision>\n"\
    "    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-types</namespace>\n"\
    "    <conformance-type>import</conformance-type>\n"\
    "  </module>\n"\
    "  <module>\n"\
    "    <name>ietf-yang-library</name>\n"\
    "    <revision>2019-01-04</revision>\n"\
    "    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>\n"\
    "    <conformance-type>implement</conformance-type>\n"\
    "  </module>\n"\
    "  <module>\n"\
    "    <name>ietf-datastores</name>\n"\
    "    <revision>2018-02-14</revision>\n"\
    "    <namespace>urn:ietf:params:xml:ns:yang:ietf-datastores</namespace>\n"\
    "    <conformance-type>import</conformance-type>\n"\
    "  </module>\n"

    const char *yanglibrary_only =
            DATA_YANG_LIBRARY_START
            DATA_YANG_BASE_IMPORTS
            DATA_YANG_SCHEMA_MODULE_STATE
            "</modules-state>\n";

    const char *with_netconf =
            DATA_YANG_LIBRARY_START
            "    <module>\n"
            "      <name>ietf-netconf</name>\n"
            "      <revision>2011-06-01</revision>\n"
            "      <namespace>urn:ietf:params:xml:ns:netconf:base:1.0</namespace>\n"
            "    </module>\n"
            DATA_YANG_BASE_IMPORTS
            "    <import-only-module>\n"
            "      <name>ietf-netconf-acm</name>\n"
            "      <revision>2018-02-14</revision>\n"
            "      <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-acm</namespace>\n"
            "    </import-only-module>\n"
            DATA_YANG_SCHEMA_MODULE_STATE
            "  <module>\n"
            "    <name>ietf-netconf</name>\n"
            "    <revision>2011-06-01</revision>\n"
            "    <namespace>urn:ietf:params:xml:ns:netconf:base:1.0</namespace>\n"
            "    <conformance-type>implement</conformance-type>\n"
            "  </module>\n"
            "  <module>\n"
            "    <name>ietf-netconf-acm</name>\n"
            "    <revision>2018-02-14</revision>\n"
            "    <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-acm</namespace>\n"
            "    <conformance-type>import</conformance-type>\n"
            "  </module>\n"
            "</modules-state>";

    char *with_netconf_features = malloc(8096);

    strcpy(with_netconf_features,
            DATA_YANG_LIBRARY_START
            "    <module>\n"
            "      <name>ietf-netconf</name>\n"
            "      <revision>2011-06-01</revision>\n"
            "      <namespace>urn:ietf:params:xml:ns:netconf:base:1.0</namespace>\n"
            "      <feature>writable-running</feature>\n"
            "      <feature>candidate</feature>\n"
            "      <feature>confirmed-commit</feature>\n"
            "      <feature>rollback-on-error</feature>\n"
            "      <feature>validate</feature>\n"
            "      <feature>startup</feature>\n"
            "      <feature>url</feature>\n"
            "      <feature>xpath</feature>\n"
            "    </module>\n"
            "    <import-only-module>\n"
            "      <name>ietf-yang-metadata</name>\n"
            "      <revision>2016-08-05</revision>\n"
            "      <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-metadata</namespace>\n"
            "    </import-only-module>\n"
            "    <import-only-module>\n"
            "      <name>ietf-inet-types</name>\n"
            "      <revision>2013-07-15</revision>\n"
            "      <namespace>urn:ietf:params:xml:ns:yang:ietf-inet-types</namespace>\n"
            "    </import-only-module>\n"
            "    <import-only-module>\n"
            "      <name>ietf-yang-types</name>\n"
            "      <revision>2013-07-15</revision>\n"
            "      <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-types</namespace>\n"
            "    </import-only-module>\n"
            "    <import-only-module>\n"
            "      <name>ietf-datastores</name>\n"
            "      <revision>2018-02-14</revision>\n"
            "      <namespace>urn:ietf:params:xml:ns:yang:ietf-datastores</namespace>\n"
            "    </import-only-module>\n"
            "    <import-only-module>\n"
            "      <name>ietf-netconf-acm</name>\n"
            "      <revision>2018-02-14</revision>\n"
            "      <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-acm</namespace>\n"
            "    </import-only-module>\n");
    strcpy(with_netconf_features + strlen(with_netconf_features),
            DATA_YANG_SCHEMA_MODULE_STATE
            "  <module>\n"
            "    <name>ietf-netconf</name>\n"
            "    <revision>2011-06-01</revision>\n"
            "    <namespace>urn:ietf:params:xml:ns:netconf:base:1.0</namespace>\n"
            "    <feature>writable-running</feature>\n"
            "    <feature>candidate</feature>\n"
            "    <feature>confirmed-commit</feature>\n"
            "    <feature>rollback-on-error</feature>\n"
            "    <feature>validate</feature>\n"
            "    <feature>startup</feature>\n"
            "    <feature>url</feature>\n"
            "    <feature>xpath</feature>\n"
            "    <conformance-type>implement</conformance-type>\n"
            "  </module>\n"
            "  <module>\n"
            "    <name>ietf-netconf-acm</name>\n"
            "    <revision>2018-02-14</revision>\n"
            "    <namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-acm</namespace>\n"
            "    <conformance-type>import</conformance-type>\n"
            "  </module>\n"
            "</modules-state>");

    const char *garbage_revision =
            "<yang-library xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
            "  <module-set>\n"
            "    <name>complete</name>\n"
            "    <module>\n"
            "      <name>yang</name>\n"
            "      <revision>2022-06-16</revision>\n"
            "      <namespace>urn:ietf:params:xml:ns:yang:1</namespace>\n"
            "    </module>\n"
            "    <module>\n"
            "      <name>ietf-yang-library</name>\n"
            "      <revision>2019-01-01</revision>\n"
            "      <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>\n"
            "    </module>\n"
            DATA_YANG_BASE_IMPORTS
            DATA_YANG_SCHEMA_MODULE_STATE
            "</modules-state>\n";

    const char *no_yanglibrary =
            "<yang-library xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
            "  <module-set>\n"
            "    <name>complete</name>\n"
            "    <module>\n"
            "      <name>yang</name>\n"
            "      <revision>2022-06-16</revision>\n"
            "      <namespace>urn:ietf:params:xml:ns:yang:1</namespace>\n"
            "    </module>\n"
            DATA_YANG_BASE_IMPORTS
            DATA_YANG_SCHEMA_MODULE_STATE
            "</modules-state>\n";

    (void) state;
    /* seperate context to avoid double free during teadown */
    struct ly_ctx *ctx_test = NULL;

    /* test invalid parameters */
    assert_int_equal(LY_EINVAL, ly_ctx_new_ylpath(NULL, NULL, LYD_XML, 0, &ctx_test));
    assert_int_equal(LY_EINVAL, ly_ctx_new_ylpath(NULL, TESTS_SRC, LYD_XML, 0, NULL));
    assert_int_equal(LY_ESYS, ly_ctx_new_ylpath(NULL, TESTS_SRC "garbage", LYD_XML, 0, &ctx_test));

    /* basic test with ietf-yang-library-only */
    assert_int_equal(LY_SUCCESS, ly_ctx_new_ylmem(TESTS_SRC "/modules/yang/", yanglibrary_only, LYD_XML, 0, &ctx_test));
    assert_non_null(ly_ctx_get_module(ctx_test, "ietf-yang-library", "2019-01-04"));
    assert_null(ly_ctx_get_module(ctx_test, "ietf-netconf", "2011-06-01"));
    ly_ctx_destroy(ctx_test);
    ctx_test = NULL;

    /* test loading module, should also import other module */
    assert_int_equal(LY_SUCCESS, ly_ctx_new_ylmem(TESTS_SRC "/modules/yang/", with_netconf, LYD_XML, 0, &ctx_test));
    assert_non_null(ly_ctx_get_module(ctx_test, "ietf-netconf", "2011-06-01"));
    assert_int_equal(1, ly_ctx_get_module(ctx_test, "ietf-netconf", "2011-06-01")->implemented);
    assert_non_null(ly_ctx_get_module(ctx_test, "ietf-netconf-acm", "2018-02-14"));
    assert_int_equal(0, ly_ctx_get_module(ctx_test, "ietf-netconf-acm", "2018-02-14")->implemented);
    assert_int_equal(LY_ENOT, lys_feature_value(ly_ctx_get_module(ctx_test, "ietf-netconf", "2011-06-01"), "url"));
    ly_ctx_destroy(ctx_test);
    ctx_test = NULL;

    /* test loading module with feature if they are present */
    assert_int_equal(LY_SUCCESS, ly_ctx_new_ylmem(TESTS_SRC "/modules/yang/", with_netconf_features, LYD_XML, 0, &ctx_test));
    assert_non_null(ly_ctx_get_module(ctx_test, "ietf-netconf", "2011-06-01"));
    assert_non_null(ly_ctx_get_module(ctx_test, "ietf-netconf-acm", "2018-02-14"));
    assert_int_equal(LY_SUCCESS, lys_feature_value(ly_ctx_get_module(ctx_test, "ietf-netconf", "2011-06-01"), "url"));
    ly_ctx_destroy(ctx_test);
    ctx_test = NULL;

    /* test with not matching revision */
    assert_int_equal(LY_EINVAL, ly_ctx_new_ylmem(TESTS_SRC "/modules/yang/", garbage_revision, LYD_XML, 0, &ctx_test));

    /* test data containing ietf-yang-library which conflicts with the option */
    assert_int_equal(LY_EINVAL, ly_ctx_new_ylmem(TESTS_SRC "/modules/yang/", with_netconf_features, LYD_XML, LY_CTX_NO_YANGLIBRARY, &ctx_test));

    /* test creating without ietf-yang-library */
    assert_int_equal(LY_SUCCESS, ly_ctx_new_ylmem(TESTS_SRC "/modules/yang/", no_yanglibrary, LYD_XML, LY_CTX_NO_YANGLIBRARY, &ctx_test));
    assert_int_equal(NULL, ly_ctx_get_module(ctx_test, "ietf-yang-library", "2019-01-04"));
    ly_ctx_destroy(ctx_test);
    free(with_netconf_features);
}

static LY_ERR
check_node_priv_parsed_is_set(struct lysc_node *node, void *data, ly_bool *UNUSED(dfs_continue))
{
    const struct lysp_node *pnode;
    const char ***iter;

    pnode = (const struct lysp_node *)node->priv;
    CHECK_POINTER(pnode, 1);
    iter = (const char ***)data;
    CHECK_POINTER(**iter, 1);
    CHECK_STRING(pnode->name, **iter);
    (*iter)++;

    return LY_SUCCESS;
}

static LY_ERR
check_node_priv_parsed_not_set(struct lysc_node *node, void *UNUSED(data), ly_bool *UNUSED(dfs_continue))
{
    CHECK_POINTER(node->priv, 0);
    return LY_SUCCESS;
}

static void
check_ext_instance_priv_parsed_is_set(struct lysc_ext_instance *ext)
{
    LY_ARRAY_COUNT_TYPE u, v;
    struct lysc_ext_substmt *substmts;
    struct lysc_node *cnode;
    const char **iter;
    const char *check[] = {
        "tmp_cont", "lf", NULL
    };

    LY_ARRAY_FOR(ext, u) {
        substmts = ext[u].substmts;
        LY_ARRAY_FOR(substmts, v) {
            if (substmts && substmts[v].storage && (substmts[v].stmt & LY_STMT_DATA_NODE_MASK)) {
                cnode = *(struct lysc_node **)substmts[v].storage;
                iter = check;
                assert_int_equal(LY_SUCCESS, lysc_tree_dfs_full(cnode, check_node_priv_parsed_is_set, &iter));
            }
        }
    }
}

static void
check_ext_instance_priv_parsed_not_set(struct lysc_ext_instance *ext)
{
    LY_ARRAY_COUNT_TYPE u, v;
    struct lysc_ext_substmt *substmts;
    struct lysc_node *cnode;

    LY_ARRAY_FOR(ext, u) {
        substmts = ext[u].substmts;
        LY_ARRAY_FOR(substmts, v) {
            if (substmts && substmts[v].storage && (substmts[v].stmt & LY_STMT_DATA_NODE_MASK)) {
                cnode = *(struct lysc_node **)substmts[v].storage;
                if (cnode) {
                    CHECK_POINTER((struct lysp_node *)cnode->priv, 0);
                }
            }
        }
    }
}

/**
 * @brief Testing of LY_CTX_SET_PRIV_PARSED.
 */
static void
test_set_priv_parsed(void **state)
{
    struct lys_module *mod;
    const char *schema_a;
    const char **iter;
    const char *check[] = {
        "cont", "contnotif", "contx", "grpleaf", "augleaf", "l1",
        "l1a", "l1b", "l1c", "foo1", "ll", "any", "l2",
        "l2c", "l2cx", "ch", "cas", "casx", "oper",
        "input", "inparam", "output", "outparam", "n1", NULL
    };

    /* each node must have a unique name. */
    schema_a = "module a {\n"
            "  namespace urn:tests:a;\n"
            "  prefix a;yang-version 1.1;\n"
            "\n"
            "  import ietf-restconf {\n"
            "    prefix rc;\n"
            "    revision-date 2017-01-26;\n"
            "  }\n"
            "\n"
            "  rc:yang-data \"tmp\" {\n"
            "    container tmp_cont {\n"
            "      leaf lf {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "  container cont {\n"
            "    notification contnotif;\n"
            "    leaf-list contx {\n"
            "      type string;\n"
            "    }\n"
            "    uses grp;\n"
            "  }\n"
            "  list l1 {\n"
            "    key \"l1a l1b\";\n"
            "    leaf l1a {\n"
            "      type string;\n"
            "    }\n"
            "    leaf l1b {\n"
            "      type string;\n"
            "    }\n"
            "    leaf l1c {\n"
            "      type string;\n"
            "    }\n"
            "  }\n"
            "  feature f1;\n"
            "  feature f2;\n"
            "  leaf foo1 {\n"
            "    type uint16;\n"
            "    if-feature f1;\n"
            "  }\n"
            "  leaf foo2 {\n"
            "    type uint16;\n"
            "  }\n"
            "  leaf foo3 {\n"
            "    type uint16;\n"
            "    if-feature f2;\n"
            "  }\n"
            "  leaf-list ll {\n"
            "    type string;\n"
            "  }\n"
            "  anydata any {\n"
            "    config false;\n"
            "  }\n"
            "  list l2 {\n"
            "    config false;\n"
            "    container l2c {\n"
            "      leaf l2cx {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "  choice ch {\n"
            "    case cas {\n"
            "      leaf casx {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "  rpc oper {\n"
            "    input {\n"
            "      leaf inparam {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "    output {\n"
            "      leaf outparam {\n"
            "        type int8;\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "  notification n1;\n"
            "  grouping grp {\n"
            "    leaf grpleaf {\n"
            "      type uint16;\n"
            "    }\n"
            "  }\n"
            "  augment /cont {\n"
            "    leaf augleaf {\n"
            "      type uint16;\n"
            "    }\n"
            "  }\n"
            "  deviation /a:foo2 {\n"
            "    deviate not-supported;\n"
            "  }\n"
            "}\n";

    /* use own context with extra flags */
    ly_ctx_destroy(UTEST_LYCTX);
    const char *feats[] = {"f1", NULL};

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_SET_PRIV_PARSED, &UTEST_LYCTX));
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-restconf", "2017-01-26", NULL));
    UTEST_ADD_MODULE(schema_a, LYS_IN_YANG, feats, NULL);

    print_message("[          ] create context\n");
    mod = ly_ctx_get_module(UTEST_LYCTX, "a", NULL);
    iter = check;
    assert_int_equal(LY_SUCCESS, lysc_module_dfs_full(mod, check_node_priv_parsed_is_set, &iter));
    check_ext_instance_priv_parsed_is_set(mod->compiled->exts);

    print_message("[          ] unset option\n");
    assert_int_equal(LY_SUCCESS, ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED));
    mod = ly_ctx_get_module(UTEST_LYCTX, "a", NULL);
    iter = check;
    assert_int_equal(LY_SUCCESS, lysc_module_dfs_full(mod, check_node_priv_parsed_not_set, &iter));
    check_ext_instance_priv_parsed_not_set(mod->compiled->exts);

    print_message("[          ] set option\n");
    assert_int_equal(LY_SUCCESS, ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED));
    mod = ly_ctx_get_module(UTEST_LYCTX, "a", NULL);
    iter = check;
    assert_int_equal(LY_SUCCESS, lysc_module_dfs_full(mod, check_node_priv_parsed_is_set, &iter));
    check_ext_instance_priv_parsed_is_set(mod->compiled->exts);
}

static void
test_explicit_compile(void **state)
{
    uint32_t i;
    struct lys_module *mod;
    const char *schema_a = "module a {\n"
            "  namespace urn:tests:a;\n"
            "  prefix a;yang-version 1.1;\n"
            "  feature f1;\n"
            "  feature f2;\n"
            "  leaf foo1 {\n"
            "    type uint16;\n"
            "    if-feature f1;\n"
            "  }\n"
            "  leaf foo2 {\n"
            "    type uint16;\n"
            "  }\n"
            "  container cont {\n"
            "    leaf foo3 {\n"
            "      type string;\n"
            "    }\n"
            "  }\n"
            "}\n";
    const char *schema_b = "module b {\n"
            "  namespace urn:tests:b;\n"
            "  prefix b;yang-version 1.1;\n"
            "  import a {\n"
            "    prefix a;\n"
            "  }\n"
            "  augment /a:cont {\n"
            "    leaf augleaf {\n"
            "      type uint16;\n"
            "    }\n"
            "  }\n"
            "}\n";
    const char *schema_c = "module c {\n"
            "  namespace urn:tests:c;\n"
            "  prefix c;yang-version 1.1;\n"
            "  import a {\n"
            "    prefix a;\n"
            "  }\n"
            "  deviation /a:foo2 {\n"
            "    deviate not-supported;\n"
            "  }\n"
            "}\n";

    /* use own context with extra flags */
    ly_ctx_destroy(UTEST_LYCTX);
    const char *feats[] = {"f1", NULL};

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, LY_CTX_EXPLICIT_COMPILE, &UTEST_LYCTX));
    UTEST_ADD_MODULE(schema_a, LYS_IN_YANG, NULL, &mod);
    UTEST_ADD_MODULE(schema_b, LYS_IN_YANG, NULL, NULL);
    UTEST_ADD_MODULE(schema_c, LYS_IN_YANG, NULL, NULL);
    assert_int_equal(LY_SUCCESS, lys_set_implemented((struct lys_module *)mod, feats));

    /* none of the modules should be compiled */
    i = 0;
    while ((mod = ly_ctx_get_module_iter(UTEST_LYCTX, &i))) {
        assert_null(mod->compiled);
    }

    assert_int_equal(LY_SUCCESS, ly_ctx_compile(UTEST_LYCTX));

    /* check internal modules */
    mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "yang");
    assert_non_null(mod);
    mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "ietf-datastores");
    assert_non_null(mod);
    mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "ietf-yang-library");
    assert_non_null(mod);

    /* check test modules */
    mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "a");
    assert_non_null(mod);
    mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "b");
    assert_non_null(mod);
    mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "c");
    assert_non_null(mod);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_searchdirs),
        UTEST(test_options),
        UTEST(test_models),
        UTEST(test_imports),
        UTEST(test_get_models),
        UTEST(test_ylmem),
        UTEST(test_set_priv_parsed),
        UTEST(test_explicit_compile),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
