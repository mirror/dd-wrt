/*
 * @file test_yangdata.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for yang-data extensions support
 *
 * Copyright (c) 2019-2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include "libyang.h"

static int
setup(void **state)
{
    UTEST_SETUP;

    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-restconf", "2017-01-26", NULL));

    return 0;
}

static void
test_schema(void **state)
{
    const struct lys_module *mod;
    struct lysc_ext_instance *e;
    char *printed = NULL;
    const char *data = "module a {yang-version 1.1; namespace urn:tests:extensions:yangdata:a; prefix self;"
            "import ietf-restconf {revision-date 2017-01-26; prefix rc;}"
            "feature x;"
            "rc:yang-data template { container x { list l { leaf x { type string;}} leaf y {if-feature x; type string; config false;}}}}";
    const char *info = "module a {\n"
            "  namespace \"urn:tests:extensions:yangdata:a\";\n"
            "  prefix self;\n\n"
            "  ietf-restconf:yang-data \"template\" {\n"
            "    container x {\n"
            "      status current;\n"
            "      list l {\n" /* no key */
            "        min-elements 0;\n"
            "        max-elements 4294967295;\n"
            "        ordered-by user;\n"
            "        status current;\n"
            "        leaf x {\n"
            "          type string;\n"
            "          status current;\n"
            "        }\n"
            "      }\n"
            "      leaf y {\n" /* config and if-feature are ignored */
            "        type string;\n"
            "        status current;\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "}\n";

    /* valid data */
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, &mod));
    assert_non_null(e = mod->compiled->exts);
    assert_int_equal(LY_ARRAY_COUNT(mod->compiled->exts), 1);
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG_COMPILED, 0));
    assert_string_equal(printed, info);
    free(printed);

    data = "module c {yang-version 1.1; namespace urn:tests:extensions:yangdata:c; prefix self;"
            "import ietf-restconf {revision-date 2017-01-26; prefix rc;}"
            "grouping g { choice ch { container a {presence a; config false;} container b {presence b; config true;}}}"
            "rc:yang-data template { uses g;}}";
    info = "module c {\n"
            "  namespace \"urn:tests:extensions:yangdata:c\";\n"
            "  prefix self;\n\n"
            "  ietf-restconf:yang-data \"template\" {\n"
            "    choice ch {\n"
            "      status current;\n"
            "      case a {\n"
            "        status current;\n"
            "        container a {\n"
            "          presence \"true\";\n"
            "          status current;\n"
            "        }\n"
            "      }\n"
            "      case b {\n"
            "        status current;\n"
            "        container b {\n"
            "          presence \"true\";\n"
            "          status current;\n"
            "        }\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "}\n";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, &mod));
    assert_non_null(e = mod->compiled->exts);
    assert_int_equal(LY_ARRAY_COUNT(mod->compiled->exts), 1);
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG_COMPILED, 0));
    assert_string_equal(printed, info);
    free(printed);

    /* ignored - valid with warning */
    data = "module b {yang-version 1.1; namespace urn:tests:extensions:yangdata:b; prefix self;"
            "import ietf-restconf {revision-date 2017-01-26; prefix rc;}"
            "container b { rc:yang-data template { container x { leaf x {type string;}}}}}";
    info = "module b {\n"
            "  namespace \"urn:tests:extensions:yangdata:b\";\n"
            "  prefix self;\n"
            "  container b {\n"
            "    config true;\n"
            "    status current;\n"
            "  }\n"
            "}\n";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, &mod));
    assert_null(mod->compiled->exts);
    CHECK_LOG_CTX("Extension plugin \"libyang 2 - yang-data, version 1\": "
            "Extension rc:yang-data is ignored since it appears as a non top-level statement in \"container\" statement.",
            "/b:b/{extension='rc:yang-data'}/template");
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG_COMPILED, 0));
    assert_string_equal(printed, info);
    free(printed);

    /* sama data nodes name, but not conflicting */
    data = "module d {yang-version 1.1; namespace urn:tests:extensions:yangdata:d; prefix self;"
            "import ietf-restconf {revision-date 2017-01-26; prefix rc;}"
            "leaf d { type string;}"
            "rc:yang-data template1 { container d {presence d;}}"
            "rc:yang-data template2 { container d {presence d;}}}";
    info = "module d {\n"
            "  namespace \"urn:tests:extensions:yangdata:d\";\n"
            "  prefix self;\n\n"
            "  ietf-restconf:yang-data \"template1\" {\n"
            "    container d {\n"
            "      presence \"true\";\n"
            "      status current;\n"
            "    }\n"
            "  }\n"
            "  ietf-restconf:yang-data \"template2\" {\n"
            "    container d {\n"
            "      presence \"true\";\n"
            "      status current;\n"
            "    }\n"
            "  }\n"
            "  leaf d {\n"
            "    type string;\n"
            "    config true;\n"
            "    status current;\n"
            "  }\n"
            "}\n";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, &mod));
    assert_non_null(e = mod->compiled->exts);
    assert_int_equal(LY_ARRAY_COUNT(mod->compiled->exts), 2);
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG_COMPILED, 0));
    assert_string_equal(printed, info);
    free(printed);
}

static void
test_schema_invalid(void **state)
{
    const char *data = "module a {yang-version 1.1; namespace urn:tests:extensions:yangdata:a; prefix self;"
            "import ietf-restconf {revision-date 2017-01-26; prefix rc;}"
            "rc:yang-data template { leaf x {type string;}}}";

    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid keyword \"leaf\" as a child of \"rc:yang-data template\" extension instance.",
            "/a:{extension='rc:yang-data'}/template");

    data = "module a {yang-version 1.1; namespace urn:tests:extensions:yangdata:a; prefix self;"
            "import ietf-restconf {revision-date 2017-01-26; prefix rc;}"
            "rc:yang-data template { choice x { leaf x {type string;}}}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Extension plugin \"libyang 2 - yang-data, version 1\": "
            "Extension rc:yang-data is instantiated with leaf top level data node (inside a choice), "
            "but only a single container data node is allowed.",
            "/a:{extension='rc:yang-data'}/template");

    data = "module a {yang-version 1.1; namespace urn:tests:extensions:yangdata:a; prefix self;"
            "import ietf-restconf {revision-date 2017-01-26; prefix rc;}"
            "rc:yang-data template { choice x { case x { container z {presence ppp;} leaf x {type string;}}}}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Extension plugin \"libyang 2 - yang-data, version 1\": "
            "Extension rc:yang-data is instantiated with multiple top level data nodes (inside a single choice's case), "
            "but only a single container data node is allowed.",
            "/a:{extension='rc:yang-data'}/template");

    data = "module a {yang-version 1.1; namespace urn:tests:extensions:yangdata:a; prefix self;"
            "import ietf-restconf {revision-date 2017-01-26; prefix rc;}"
            "rc:yang-data template { container x { leaf x {type string;}} container y { leaf y {type string;}}}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Extension plugin \"libyang 2 - yang-data, version 1\": "
            "Extension rc:yang-data is instantiated with multiple top level data nodes, "
            "but only a single container data node is allowed.",
            "/a:{extension='rc:yang-data'}/template");

    data = "module a {yang-version 1.1; namespace urn:tests:extensions:yangdata:a; prefix self;"
            "import ietf-restconf {revision-date 2017-01-26; prefix rc;}"
            "rc:yang-data template;}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Extension plugin \"libyang 2 - yang-data, version 1\": "
            "Extension rc:yang-data is instantiated without any top level data node, "
            "but exactly one container data node is expected.",
            "/a:{extension='rc:yang-data'}/template");

    data = "module a {yang-version 1.1; namespace urn:tests:extensions:yangdata:a; prefix self;"
            "import ietf-restconf {revision-date 2017-01-26; prefix rc;}"
            "rc:yang-data { container x { leaf x {type string;}}}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Extension instance \"rc:yang-data\" misses argument element \"name\".",
            "/a:{extension='rc:yang-data'}");

    data = "module a {yang-version 1.1; namespace urn:tests:extensions:yangdata:a; prefix self;"
            "import ietf-restconf {revision-date 2017-01-26; prefix rc;}"
            "rc:yang-data template { container x { leaf x {type string;}}}"
            "rc:yang-data template { container y { leaf y {type string;}}}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Extension plugin \"libyang 2 - yang-data, version 1\": "
            "Extension rc:yang-data is instantiated multiple times.",
            "/a:{extension='rc:yang-data'}/template");

    data = "module a {yang-version 1.1; namespace urn:tests:extensions:yangdata:a; prefix self;"
            "import ietf-restconf {revision-date 2017-01-26; prefix rc;}"
            "grouping t { leaf-list x {type string;}}"
            "rc:yang-data template { uses t;}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Extension plugin \"libyang 2 - yang-data, version 1\": "
            "Extension rc:yang-data is instantiated with leaf-list top level data node, "
            "but only a single container data node is allowed.",
            "/a:{extension='rc:yang-data'}/template");
}

static void
test_data(void **state)
{
    const struct lys_module *mod;
    struct lysc_ext_instance *e;
    struct lyd_node *tree = NULL;
    const char *schema = "module a {yang-version 1.1; namespace urn:tests:extensions:yangdata:a; prefix self;"
            "import ietf-restconf {revision-date 2017-01-26; prefix rc;}"
            "rc:yang-data template { container x { leaf x { type string;}}}}";
    const char *xml = "<x xmlns=\"urn:tests:extensions:yangdata:a\"><x>test</x></x>";
    const char *json = "{\"a:x\":{\"x\":\"test\"}}";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, schema, LYS_IN_YANG, &mod));
    assert_non_null(e = mod->compiled->exts);

    assert_int_equal(LY_SUCCESS, ly_in_new_memory(xml, &UTEST_IN));
    assert_int_equal(LY_SUCCESS, lyd_parse_ext_data(e, NULL, UTEST_IN, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LYD_STRING_PARAM(tree, xml, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS);
    lyd_free_all(tree);

    ly_in_memory(UTEST_IN, json);
    assert_int_equal(LY_SUCCESS, lyd_parse_ext_data(e, NULL, UTEST_IN, LYD_JSON, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LYD_STRING_PARAM(tree, json, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS);

    lyd_free_all(tree);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_schema, setup),
        UTEST(test_schema_invalid, setup),
        UTEST(test_data, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
