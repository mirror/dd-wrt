/**
 * @file test_metadata.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for Metadata extension (annotation) support
 *
 * Copyright (c) 2019 - 2022 CESNET, z.s.p.o.
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
#include "plugins_exts.h"
#include "plugins_exts/metadata.h"

static void
test_yang(void **state)
{
    struct lys_module *mod;
    struct lysc_ext_instance *e;
    const char *units;

    const char *data = "module a {yang-version 1.1; namespace urn:tests:extensions:metadata:a; prefix a;"
            "import ietf-yang-metadata {prefix md;}"
            "feature f;"
            "md:annotation x {"
            "  description \"test\";"
            "  if-feature f;"
            "  reference \"test\";"
            "  status \"current\";"
            "  type uint8;"
            "  units meters;"
            "}}";
    const char *feats[] = {"f", NULL};

    UTEST_ADD_MODULE(data, LYS_IN_YANG, feats, &mod);
    assert_int_equal(1, LY_ARRAY_COUNT(mod->compiled->exts));
    e = &mod->compiled->exts[0];
    assert_non_null(e->compiled);
    assert_non_null(e->substmts);
    lyplg_ext_get_storage(e, LY_STMT_UNITS, sizeof units, (const void **)&units);
    assert_string_equal("meters", units);

    /* invalid */
    /* missing mandatory type substatement */
    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:metadata:aa; prefix aa;"
            "import ietf-yang-metadata {prefix md;}"
            "md:annotation aa;}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 metadata v1\": Missing mandatory keyword \"type\" as a child of \"md:annotation aa\".",
            "Path \"/aa:{extension='md:annotation'}/aa\".");

    /* not allowed substatement */
    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:metadata:aa; prefix aa;"
            "import ietf-yang-metadata {prefix md;}"
            "md:annotation aa {default x;}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Invalid keyword \"default\" as a child of \"md:annotation aa\" extension instance.",
            "Path \"/aa:{extension='md:annotation'}/aa\".");

    /* invalid cardinality of units substatement */
    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:metadata:aa; prefix aa;"
            "import ietf-yang-metadata {prefix md;}"
            "md:annotation aa {type string; units x; units y;}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate keyword \"units\".",
            "Path \"/aa:{extension='md:annotation'}/aa\".");

    /* invalid cardinality of status substatement */
    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:metadata:aa; prefix aa;"
            "import ietf-yang-metadata {prefix md;}"
            "md:annotation aa {type string; status current; status obsolete;}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate keyword \"status\".",
            "Path \"/aa:{extension='md:annotation'}/aa\".");

    /* invalid cardinality of status substatement */
    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:metadata:aa; prefix aa;"
            "import ietf-yang-metadata {prefix md;}"
            "md:annotation aa {type string; type uint8;}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Duplicate keyword \"type\".",
            "Path \"/aa:{extension='md:annotation'}/aa\".");

    /* duplication of the same annotation */
    data = "module aa {yang-version 1.1; namespace urn:tests:extensions:metadata:aa; prefix aa;"
            "import ietf-yang-metadata {prefix md;}"
            "md:annotation aa {type string;} md:annotation aa {type uint8;}}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 metadata v1\": Extension md:annotation is instantiated multiple times.",
            "Path \"/aa:{extension='md:annotation'}/aa\".");
}

static void
test_yin(void **state)
{
    struct lys_module *mod;
    struct lysc_ext_instance *e;
    const char *data, *units;

    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\" name=\"a\">\n"
            "<yang-version value=\"1.1\"/><namespace uri=\"urn:tests:extensions:metadata:a\"/><prefix value=\"a\"/>\n"
            "<import module=\"ietf-yang-metadata\"><prefix value=\"md\"/></import>\n"
            "<feature name=\"f\"/>\n"
            "<md:annotation name=\"x\">\n"
            "  <description><text>test</text></description>\n"
            "  <reference><text>test</text></reference>\n"
            "  <if-feature name=\"f\"/>\n"
            "  <status value=\"current\"/>\n"
            "  <type name=\"uint8\"/>\n"
            "  <units name=\"meters\"/>\n"
            "</md:annotation></module>";
    const char *feats[] = {"f", NULL};

    UTEST_ADD_MODULE(data, LYS_IN_YIN, feats, &mod);
    assert_int_equal(1, LY_ARRAY_COUNT(mod->compiled->exts));
    e = &mod->compiled->exts[0];
    assert_non_null(e->compiled);
    assert_non_null(e->substmts);
    lyplg_ext_get_storage(e, LY_STMT_UNITS, sizeof units, (const void **)&units);
    assert_string_equal("meters", units);

    /* invalid */
    /* missing mandatory type substatement */
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\" name=\"aa\">\n"
            "<yang-version value=\"1.1\"/><namespace uri=\"urn:tests:extensions:metadata:aa\"/><prefix value=\"aa\"/>\n"
            "<import module=\"ietf-yang-metadata\"><prefix value=\"md\"/></import>\n"
            "<md:annotation name=\"aa\"/>\n"
            "</module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 metadata v1\": Missing mandatory keyword \"type\" as a child of \"md:annotation aa\".",
            "Path \"/aa:{extension='md:annotation'}/aa\".");

    /* not allowed substatement */
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\" name=\"aa\">\n"
            "<yang-version value=\"1.1\"/><namespace uri=\"urn:tests:extensions:metadata:aa\"/><prefix value=\"aa\"/>\n"
            "<import module=\"ietf-yang-metadata\"><prefix value=\"md\"/></import>\n"
            "<md:annotation name=\"aa\">\n"
            "  <default value=\"x\"/>\n"
            "</md:annotation></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Invalid keyword \"default\" as a child of \"md:annotation aa\" extension instance.",
            "Path \"/aa:{extension='md:annotation'}/aa\".");

    /* invalid cardinality of units substatement */
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\" name=\"aa\">\n"
            "<yang-version value=\"1.1\"/><namespace uri=\"urn:tests:extensions:metadata:aa\"/><prefix value=\"aa\"/>\n"
            "<import module=\"ietf-yang-metadata\"><prefix value=\"md\"/></import>\n"
            "<md:annotation name=\"aa\">\n"
            "  <type name=\"string\"/>\n"
            "  <units name=\"x\"/>\n"
            "  <units name=\"y\"/>\n"
            "</md:annotation></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Duplicate keyword \"units\".",
            "Path \"/aa:{extension='md:annotation'}/aa\".");

    /* invalid cardinality of status substatement */
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\" name=\"aa\">\n"
            "<yang-version value=\"1.1\"/><namespace uri=\"urn:tests:extensions:metadata:aa\"/><prefix value=\"aa\"/>\n"
            "<import module=\"ietf-yang-metadata\"><prefix value=\"md\"/></import>\n"
            "<md:annotation name=\"aa\">\n"
            "  <type name=\"string\"/>\n"
            "  <status value=\"current\"/>\n"
            "  <status value=\"obsolete\"/>\n"
            "</md:annotation></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Duplicate keyword \"status\".",
            "Path \"/aa:{extension='md:annotation'}/aa\".");

    /* invalid cardinality of status substatement */
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\" name=\"aa\">\n"
            "<yang-version value=\"1.1\"/><namespace uri=\"urn:tests:extensions:metadata:aa\"/><prefix value=\"aa\"/>\n"
            "<import module=\"ietf-yang-metadata\"><prefix value=\"md\"/></import>\n"
            "<md:annotation name=\"aa\">\n"
            "  <type name=\"string\"/>\n"
            "  <type name=\"uint8\"/>\n"
            "</md:annotation></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Duplicate keyword \"type\".",
            "Path \"/aa:{extension='md:annotation'}/aa\".");

    /* duplication of the same annotation */
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\" name=\"aa\">\n"
            "<yang-version value=\"1.1\"/><namespace uri=\"urn:tests:extensions:metadata:aa\"/><prefix value=\"aa\"/>\n"
            "<import module=\"ietf-yang-metadata\"><prefix value=\"md\"/></import>\n"
            "<md:annotation name=\"aa\">\n"
            "  <type name=\"string\"/>\n"
            "</md:annotation><md:annotation name=\"aa\">\n"
            "  <type name=\"uint8\"/>\n"
            "</md:annotation></module>";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, data, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Ext plugin \"ly2 metadata v1\": Extension md:annotation is instantiated multiple times.",
            "Path \"/aa:{extension='md:annotation'}/aa\".");
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_yang),
        UTEST(test_yin),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
