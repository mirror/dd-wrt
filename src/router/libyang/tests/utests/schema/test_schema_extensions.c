/*
 * @file test_schema_extensions.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for YANG (YIN) extension statements and their instances in schemas
 *
 * Copyright (c) 2018-2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#include "test_schema.h"

#include <string.h>

#include "context.h"
#include "log.h"
#include "tree_schema.h"

void
test_extension_argument(void **state)
{
    const struct lys_module *mod;
    const char *mod_def_yang = "module a {\n"
            "  namespace \"urn:a\";\n"
            "  prefix a;\n\n"
            "  extension e {\n"
            "    argument name;\n"
            "  }\n\n"
            "  a:e \"aaa\";\n"
            "}\n";
    const char *mod_def_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"a\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:a\"/>\n"
            "  <prefix value=\"a\"/>\n\n"
            "  <extension name=\"e\">\n"
            "    <argument name=\"name\"/>\n"
            "  </extension>\n\n"
            "  <a:e name=\"aaa\"/>\n"
            "</module>\n";
    const char *mod_test_yin, *mod_test_yang;
    char *printed;

    mod_test_yang = "module b {\n"
            "  namespace \"urn:b\";\n"
            "  prefix b;\n\n"
            "  import a {\n"
            "    prefix a;\n"
            "  }\n\n"
            "  a:e \"xxx\";\n"
            "}\n";
    mod_test_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"b\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:b=\"urn:b\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:b\"/>\n"
            "  <prefix value=\"b\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n\n"
            "  <a:e name=\"xxx\"/>\n"
            "</module>\n";

    /* from YANG */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, (void *)mod_def_yang);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, mod_test_yang, LYS_IN_YANG, &mod));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_test_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_test_yin);
    free(printed);

    assert_non_null(mod = ly_ctx_get_module(UTEST_LYCTX, "a", NULL));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_def_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_def_yin);
    free(printed);

    /* context reset */
    ly_ctx_destroy(UTEST_LYCTX);
    ly_ctx_new(NULL, 0, &UTEST_LYCTX);

    /* from YIN */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, (void *)mod_def_yin);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, &mod));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_test_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_test_yin);
    free(printed);

    assert_non_null(mod = ly_ctx_get_module(UTEST_LYCTX, "a", NULL));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_def_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_def_yin);
    free(printed);
}

void
test_extension_argument_element(void **state)
{
    const struct lys_module *mod;
    const char *mod_def_yang = "module a {\n"
            "  namespace \"urn:a\";\n"
            "  prefix a;\n\n"
            "  extension e {\n"
            "    argument name {\n"
            "      yin-element true;\n"
            "    }\n"
            "  }\n\n"
            "  a:e \"aaa\";\n"
            "}\n";
    const char *mod_def_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"a\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:a\"/>\n"
            "  <prefix value=\"a\"/>\n\n"
            "  <extension name=\"e\">\n"
            "    <argument name=\"name\">\n"
            "      <yin-element value=\"true\"/>\n"
            "    </argument>\n"
            "  </extension>\n\n"
            "  <a:e>\n"
            "    <a:name>aaa</a:name>\n"
            "  </a:e>\n"
            "</module>\n";
    const char *mod_test_yin, *mod_test_yang;
    char *printed;

    mod_test_yang = "module b {\n"
            "  namespace \"urn:b\";\n"
            "  prefix b;\n\n"
            "  import a {\n"
            "    prefix a;\n"
            "  }\n\n"
            "  a:e \"xxx\";\n"
            "}\n";
    mod_test_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"b\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:b=\"urn:b\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:b\"/>\n"
            "  <prefix value=\"b\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n\n"
            "  <a:e>\n"
            "    <a:name>xxx</a:name>\n"
            "  </a:e>\n"
            "</module>\n";

    /* from YANG */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, (void *)mod_def_yang);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, mod_test_yang, LYS_IN_YANG, &mod));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_test_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_test_yin);
    free(printed);

    assert_non_null(mod = ly_ctx_get_module(UTEST_LYCTX, "a", NULL));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_def_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_def_yin);
    free(printed);

    /* context reset */
    ly_ctx_destroy(UTEST_LYCTX);
    ly_ctx_new(NULL, 0, &UTEST_LYCTX);

    /* from YIN */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, (void *)mod_def_yin);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, &mod));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_test_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_test_yin);
    free(printed);

    assert_non_null(mod = ly_ctx_get_module(UTEST_LYCTX, "a", NULL));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_def_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_def_yin);
    free(printed);

    /* invalid */
    mod_test_yang = "module x { namespace \"urn:x\"; prefix x; import a { prefix a; } a:e; }";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, mod_test_yang, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Extension instance \"a:e\" misses argument element \"name\".", "/x:{extension='a:e'}");

    mod_test_yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"x\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:x=\"urn:x\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:x\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n\n"
            "  <a:e/>\n"
            "</module>\n";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Extension instance \"a:e\" misses argument element \"name\".", "/x:{extension='a:e'}");

    mod_test_yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"x\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:x=\"urn:x\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:x\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n\n"
            "  <a:e name=\"xxx\"/>\n"
            "</module>\n";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Extension instance \"a:e\" misses argument element \"name\".", "/x:{extension='a:e'}");

    mod_test_yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"x\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:x=\"urn:x\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:x\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n\n"
            "  <a:e>\n"
            "    <x:name>xxx</x:name>\n"
            "  </a:e>\n"
            "</module>\n";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Extension instance \"a:e\" element and its argument element \"name\" are expected in the same namespace, but they differ.",
            "/x:{extension='a:e'}");

    mod_test_yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"x\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:x=\"urn:x\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:x\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n\n"
            "  <a:e>\n"
            "    <a:value>xxx</a:value>\n"
            "  </a:e>\n"
            "</module>\n";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Extension instance \"a:e\" expects argument element \"name\" as its first XML child, but \"value\" element found.",
            "/x:{extension='a:e'}");

}
