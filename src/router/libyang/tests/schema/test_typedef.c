/**
 * \file test_typedef.c
 * \author Michal Vasko <mvasko@cesnet.cz>
 * \brief libyang tests - typedefs and their resolution
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>
#include <cmocka.h>

#include "libyang.h"
#include "tests/config.h"

struct state {
    struct ly_ctx *ctx;
    int fd;
    char *str1;
    char *str2;
};

static int
setup_ctx(void **state)
{
    struct state *st;
    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(NULL, 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    return 0;

error:
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return -1;
}

static int
teardown_ctx(void **state)
{
    struct state *st = (*state);

    ly_ctx_destroy(st->ctx, NULL);
    if (st->fd > 0) {
        close(st->fd);
    }
    free(st->str1);
    free(st->str2);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_typedef_yin(void **state)
{
    const char *schema = TESTS_DIR"/schema/yin/files/f.yin";
    struct state *st = (*state);
    struct stat s;
    const struct lys_module *mod;

    mod = lys_parse_path(st->ctx, schema, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    st->fd = open(schema, O_RDONLY);
    fstat(st->fd, &s);
    st->str1 = malloc(s.st_size + 1);
    assert_ptr_not_equal(st->str1, NULL);
    assert_int_equal(read(st->fd, st->str1, s.st_size), s.st_size);
    st->str1[s.st_size] = '\0';

    lys_print_mem(&(st->str2), mod, LYS_OUT_YIN, NULL, 0, 0);

    assert_string_equal(st->str1, st->str2);
}

static void
test_typedef_yang(void **state)
{
    const char *schema = TESTS_DIR"/schema/yang/files/f.yang";
    struct state *st = (*state);
    struct stat s;
    const struct lys_module *mod;

    mod = lys_parse_path(st->ctx, schema, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    st->fd = open(schema, O_RDONLY);
    fstat(st->fd, &s);
    st->str1 = malloc(s.st_size + 1);
    assert_ptr_not_equal(st->str1, NULL);
    assert_int_equal(read(st->fd, st->str1, s.st_size), s.st_size);
    st->str1[s.st_size] = '\0';

    lys_print_mem(&(st->str2), mod, LYS_OUT_YANG, NULL, 0, 0);

    assert_string_equal(st->str1, st->str2);
}

static void
test_typedef_11in10(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin_enums = "<module name=\"x1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <namespace uri=\"urn:x1\"/><prefix value=\"x1\"/>"
"  <typedef name=\"e1\"><type name=\"enumeration\">"
"    <enum name=\"one\"/><enum name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"e1\">"
"    <enum name=\"one\"/>"
"  </type></leaf>"
"</module>";

    const char *yang_enums = "module x2 {"
"  namespace \"urn:x2\"; prefix x2;"
"  typedef e1 { type enumeration { enum one; enum two; } default one; }"
"  leaf l { type e1 { enum one; } } }";

    const char *yin_bits = "<module name=\"y1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <namespace uri=\"urn:y1\"/><prefix value=\"y1\"/>"
"  <typedef name=\"b1\"><type name=\"bits\">"
"    <bit name=\"one\"/><bit name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"b1\">"
"    <bit name=\"one\"/>"
"  </type></leaf>"
"</module>";

    const char *yang_bits = "module y2 {"
"  namespace \"urn:y2\"; prefix y2;"
"  typedef b1 { type bits { bit one; bit two; } default one; }"
"  leaf l { type b1 { bit one; } } }";

    const char *yin_union1 = "<module name=\"x1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <namespace uri=\"urn:x1\"/><prefix value=\"x1\"/>"
"  <typedef name=\"un\"><type name=\"union\">"
"    <type name=\"string\"/>"
"    <type name=\"leafref\"><path value=\"../name\"/></type>"
"  </type></typedef>"
"</module>";

    const char *yang_union1 = "module x1 {"
"  namespace \"urn:x1\"; prefix \"x1\";"
"  typedef un { type \"union\" {"
"    type string;"
"    type \"leafref\" { path \"../name\"; }"
"  }}}";

    const char *yin_union2 = "<module name=\"x1\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <namespace uri=\"urn:x1\"/><prefix value=\"x1\"/>"
"  <typedef name=\"un\"><type name=\"union\">"
"    <type name=\"string\"/>"
"    <type name=\"empty\"/>"
"  </type></typedef>"
"</module>";

    const char *yang_union2 = "module x1 {"
"  namespace \"urn:x1\"; prefix \"x1\";"
"  typedef \"un\" { type union {"
"    type string; type empty; }}"
"  }";

    mod = lys_parse_mem(st->ctx, yin_enums, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INSTMT);

    mod = lys_parse_mem(st->ctx, yin_bits, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INSTMT);

    mod = lys_parse_mem(st->ctx, yin_union1, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INARG);

    mod = lys_parse_mem(st->ctx, yin_union2, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INARG);

    mod = lys_parse_mem(st->ctx, yang_enums, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INSTMT);

    mod = lys_parse_mem(st->ctx, yang_bits, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INSTMT);

    mod = lys_parse_mem(st->ctx, yang_union1, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INARG);

    mod = lys_parse_mem(st->ctx, yang_union2, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INARG);
}

static void
test_typedef_11_multidents_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    struct lyd_node *root;
    const char *schema = "module x {"
"  yang-version 1.1;"
"  namespace \"urn:x\";"
"  prefix x;"
"  identity des3 { base des; }"
"  identity des { base crypto-alg; base symmetric-key; }"
"  identity rsa { base crypto-alg; base public-key; }"
"  identity crypto-alg;"
"  identity symmetric-key;"
"  identity public-key;"
"  leaf l1 { type identityref { base crypto-alg; } }"
"  leaf l2 { type identityref { base public-key; } } }";

    const char *data1 = "<l1 xmlns=\"urn:x\">des</l1><l2 xmlns=\"urn:x\">des</l2>";
    const char *data2 = "<l1 xmlns=\"urn:x\">des3</l1><l2 xmlns=\"urn:x\">des3</l2>";
    const char *data3 = "<l1 xmlns=\"urn:x\">rsa</l1><l2 xmlns=\"urn:x\">rsa</l2>";

    mod = lys_parse_mem(st->ctx, schema, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_string_equal(ly_errmsg(st->ctx), "Failed to resolve identityref \"des\".");
    assert_string_equal(ly_errpath(st->ctx), "/x:l2");

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_string_equal(ly_errmsg(st->ctx), "Failed to resolve identityref \"des3\".");
    assert_string_equal(ly_errpath(st->ctx), "/x:l2");

    root = lyd_parse_mem(st->ctx, data3, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    lyd_free_withsiblings(root);
}

static void
test_typedef_11_multidents_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    struct lyd_node *root;
    const char *schema = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/><prefix value=\"x\"/>"
"  <identity name=\"des3\"><base name=\"des\"/></identity>"
"  <identity name=\"des\"><base name=\"crypto-alg\"/><base name=\"symmetric-key\"/></identity>"
"  <identity name=\"rsa\"><base name=\"crypto-alg\"/><base name=\"public-key\"/></identity>"
"  <identity name=\"crypto-alg\"/>"
"  <identity name=\"symmetric-key\"/>"
"  <identity name=\"public-key\"/>"
"  <leaf name=\"l1\"><type name=\"identityref\"><base name=\"crypto-alg\"/></type></leaf>"
"  <leaf name=\"l2\"><type name=\"identityref\"><base name=\"public-key\"/></type></leaf></module>";

    const char *data1 = "<l1 xmlns=\"urn:x\">des</l1><l2 xmlns=\"urn:x\">des</l2>";
    const char *data2 = "<l1 xmlns=\"urn:x\">des3</l1><l2 xmlns=\"urn:x\">des3</l2>";
    const char *data3 = "<l1 xmlns=\"urn:x\">rsa</l1><l2 xmlns=\"urn:x\">rsa</l2>";

    mod = lys_parse_mem(st->ctx, schema, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_string_equal(ly_errmsg(st->ctx), "Failed to resolve identityref \"des\".");
    assert_string_equal(ly_errpath(st->ctx), "/x:l2");

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_string_equal(ly_errmsg(st->ctx), "Failed to resolve identityref \"des3\".");
    assert_string_equal(ly_errpath(st->ctx), "/x:l2");

    root = lyd_parse_mem(st->ctx, data3, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    lyd_free_withsiblings(root);
}

static void
test_typedef_11_enums_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *enums1 = "module x {"
"  yang-version 1.1;"
"  namespace \"urn:x\";"
"  prefix x;"
"  typedef e1 { type enumeration { enum one; enum two; } default one; }"
"  leaf l { type e1 { enum two; } } }";

    const char *enums2 = "module x {"
"  yang-version 1.1;"
"  namespace \"urn:x\";"
"  prefix x;"
"  typedef e1 { type \"enumeration\" { enum \"one\"; enum two; } default \"one\"; }"
"  leaf l { type e1 { enum one { value 1; } } } }";

    const char *enums3 = "module x {"
"  yang-version 1.1;"
"  namespace \"urn:x\";"
"  prefix \"x\";"
"  typedef e1 { type enumeration { enum one; enum two; } default one; }"
"  leaf l { type e1 { enum three; } } }";

    const char *enums4 = "module x {"
"  yang-version 1.1;"
"  namespace \"urn:x\";"
"  prefix x;"
"  typedef e1 { type enumeration { enum one; enum two; } default one; }"
"  leaf l { type e1 { enum one; } } }";

    mod = lys_parse_mem(st->ctx, enums1, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    mod = lys_parse_mem(st->ctx, enums2, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_ENUM_INVAL);

    mod = lys_parse_mem(st->ctx, enums3, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_ENUM_INNAME);

    mod = lys_parse_mem(st->ctx, enums4, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);
}

static void
test_typedef_11_enums_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *enums1 = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/><prefix value=\"x\"/>"
"  <typedef name=\"e1\"><type name=\"enumeration\">"
"    <enum name=\"one\"/><enum name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"e1\">"
"    <enum name=\"two\"/>"
"  </type></leaf>"
"</module>";

    const char *enums2 = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/><prefix value=\"x\"/>"
"  <typedef name=\"e1\"><type name=\"enumeration\">"
"    <enum name=\"one\"/><enum name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"e1\">"
"    <enum name=\"one\"><value value=\"1\"/></enum>"
"  </type></leaf>"
"</module>";

    const char *enums3 = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/><prefix value=\"x\"/>"
"  <typedef name=\"e1\"><type name=\"enumeration\">"
"    <enum name=\"one\"/><enum name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"e1\">"
"    <enum name=\"three\"/>"
"  </type></leaf>"
"</module>";

    const char *enums4 = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/><prefix value=\"x\"/>"
"  <typedef name=\"e1\"><type name=\"enumeration\">"
"    <enum name=\"one\"/><enum name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"e1\">"
"    <enum name=\"one\"/>"
"  </type></leaf>"
"</module>";

    mod = lys_parse_mem(st->ctx, enums1, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    mod = lys_parse_mem(st->ctx, enums2, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_ENUM_INVAL);

    mod = lys_parse_mem(st->ctx, enums3, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_ENUM_INNAME);

    mod = lys_parse_mem(st->ctx, enums4, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);
}

static void
test_typedef_11_bits_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    const char *bits1 = "module y {"
"  yang-version 1.1;"
"  namespace \"urn:y\";"
"  prefix \"y\";"
"  typedef b1 { type \"bits\" { bit \"one\"; bit two; } default one; }"
"  leaf l { type b1 { bit two; } } }";

    const char *bits2 = "module y {"
"  yang-version 1.1;"
"  namespace \"urn:y\";"
"  prefix y;"
"  typedef b1 { type bits { bit one; bit two; } default one; }"
"  leaf l { type b1 { bit one { position 1; } } } }";

    const char *bits3 = "module y {"
"  yang-version 1.1;"
"  namespace \"urn:y\";"
"  prefix y;"
"  typedef b1 { type bits { bit one; bit two; } default \"one\"; }"
"  leaf l { type b1 { bit three; } } }";

    const char *bits4 = "module y {"
"  yang-version 1.1;"
"  namespace \"urn:y\";"
"  prefix y;"
"  typedef b1 { type bits { bit one; bit two; } default one; }"
"  leaf l { type b1 { bit one; } } }";

    mod = lys_parse_mem(st->ctx, bits1, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    mod = lys_parse_mem(st->ctx, bits2, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_BITS_INVAL);

    mod = lys_parse_mem(st->ctx, bits3, LYS_IN_YANG);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_BITS_INNAME);

    mod = lys_parse_mem(st->ctx, bits4, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);
}

static void
test_typedef_11_bits_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    const char *bits1 = "<module name=\"y\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:y\"/><prefix value=\"y\"/>"
"  <typedef name=\"b1\"><type name=\"bits\">"
"    <bit name=\"one\"/><bit name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"b1\">"
"    <bit name=\"two\"/>"
"  </type></leaf>"
"</module>";

    const char *bits2 = "<module name=\"y\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:y\"/><prefix value=\"y\"/>"
"  <typedef name=\"b1\"><type name=\"bits\">"
"    <bit name=\"one\"/><bit name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"b1\">"
"    <bit name=\"one\"><position value=\"1\"/></bit>"
"  </type></leaf>"
"</module>";

    const char *bits3 = "<module name=\"y\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:y\"/><prefix value=\"y\"/>"
"  <typedef name=\"b1\"><type name=\"bits\">"
"    <bit name=\"one\"/><bit name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"b1\">"
"    <bit name=\"three\"/>"
"  </type></leaf>"
"</module>";

    const char *bits4 = "<module name=\"y\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:y\"/><prefix value=\"y\"/>"
"  <typedef name=\"b1\"><type name=\"bits\">"
"    <bit name=\"one\"/><bit name=\"two\"/>"
"  </type><default value=\"one\"/></typedef>"
"  <leaf name=\"l\"><type name=\"b1\">"
"    <bit name=\"one\"/>"
"  </type></leaf>"
"</module>";

    mod = lys_parse_mem(st->ctx, bits1, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    mod = lys_parse_mem(st->ctx, bits2, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_BITS_INVAL);

    mod = lys_parse_mem(st->ctx, bits3, LYS_IN_YIN);
    assert_ptr_equal(mod, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_BITS_INNAME);

    mod = lys_parse_mem(st->ctx, bits4, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);
}

static void
test_typedef_11_iff_ident_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    const char *idents = "module x {"
"  yang-version 1.1;"
"  namespace \"urn:x\";"
"  prefix x;"
"  feature x;"
"  identity ibase;"
"  identity one { base ibase; if-feature x; }"
"  identity \"two\" { base \"ibase\"; }"
"  leaf l { type identityref { base ibase; } } }";

    struct lyd_node *root;
    const char *data1 = "<l xmlns=\"urn:x\">one</l>";
    const char *data2 = "<l xmlns=\"urn:x\">two</l>";

    mod = lys_parse_mem(st->ctx, idents, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    lyd_free_withsiblings(root);

    lys_features_enable(mod, "x");
    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);

    lys_features_disable(mod, "x");
    assert_int_not_equal(lyd_validate(&root, LYD_OPT_CONFIG, NULL), 0);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    lyd_free_withsiblings(root);
}

static void
test_typedef_11_iff_ident_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    const char *idents = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/><prefix value=\"x\"/><feature name=\"x\"/>"
"  <identity name=\"ibase\"/>"
"  <identity name=\"one\"><base name=\"ibase\"/><if-feature name=\"x\"/></identity>"
"  <identity name=\"two\"><base name=\"ibase\"/></identity>"
"  <leaf name=\"l\"><type name=\"identityref\"><base name=\"ibase\"/></type></leaf></module>";

    struct lyd_node *root;
    const char *data1 = "<l xmlns=\"urn:x\">one</l>";
    const char *data2 = "<l xmlns=\"urn:x\">two</l>";

    mod = lys_parse_mem(st->ctx, idents, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    lyd_free_withsiblings(root);

    lys_features_enable(mod, "x");
    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);

    lys_features_disable(mod, "x");
    assert_int_not_equal(lyd_validate(&root, LYD_OPT_CONFIG, NULL), 0);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    lyd_free_withsiblings(root);
}

static void
test_typedef_11_iff_enums_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    const char *idents = "module x {"
"  yang-version 1.1;"
"  namespace \"urn:x\";"
"  prefix x;"
"  feature x;"
"  typedef myenum { type enumeration { enum one { if-feature x; } enum two; } }"
"  leaf l { type myenum; } }";

    struct lyd_node *root;
    const char *data1 = "<l xmlns=\"urn:x\">one</l>";
    const char *data2 = "<l xmlns=\"urn:x\">two</l>";

    mod = lys_parse_mem(st->ctx, idents, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    lyd_free_withsiblings(root);

    lys_features_enable(mod, "x");
    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);

    lys_features_disable(mod, "x");
    assert_int_not_equal(lyd_validate(&root, LYD_OPT_CONFIG, NULL), 0);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    lyd_free_withsiblings(root);
}

static void
test_typedef_11_iff_enums_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    const char *idents = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/><prefix value=\"x\"/><feature name=\"x\"/>"
"  <typedef name=\"myenum\"><type name=\"enumeration\">"
"    <enum name=\"one\"><if-feature name=\"x\"/></enum><enum name=\"two\"/>"
"  </type></typedef>"
"  <leaf name=\"l\"><type name=\"myenum\"/></leaf></module>";

    struct lyd_node *root;
    const char *data1 = "<l xmlns=\"urn:x\">one</l>";
    const char *data2 = "<l xmlns=\"urn:x\">two</l>";

    mod = lys_parse_mem(st->ctx, idents, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    lyd_free_withsiblings(root);

    lys_features_enable(mod, "x");
    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);

    lys_features_disable(mod, "x");
    assert_int_not_equal(lyd_validate(&root, LYD_OPT_CONFIG, NULL), 0);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    lyd_free_withsiblings(root);
}

static void
test_typedef_11_iff_bits_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    const char *idents = "module x {"
"  yang-version 1.1;"
"  namespace \"urn:x\";"
"  prefix x;"
"  feature x;"
"  typedef mybits { type bits { bit one { if-feature x;} bit two; } }"
"  leaf l { type mybits; } }";

    struct lyd_node *root;
    const char *data1 = "<l xmlns=\"urn:x\">one</l>";
    const char *data2 = "<l xmlns=\"urn:x\">two</l>";

    mod = lys_parse_mem(st->ctx, idents, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    lyd_free_withsiblings(root);

    lys_features_enable(mod, "x");
    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);

    lys_features_disable(mod, "x");
    assert_int_not_equal(lyd_validate(&root, LYD_OPT_CONFIG, NULL), 0);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    lyd_free_withsiblings(root);
}

static void
test_typedef_11_iff_bits_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    const char *idents = "<module name=\"x\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/><prefix value=\"x\"/><feature name=\"x\"/>"
"  <typedef name=\"mybits\"><type name=\"bits\">"
"    <bit name=\"one\"><if-feature name=\"x\"/></bit><bit name=\"two\"/>"
"  </type></typedef>"
"  <leaf name=\"l\"><type name=\"mybits\"/></leaf></module>";

    struct lyd_node *root;
    const char *data1 = "<l xmlns=\"urn:x\">one</l>";
    const char *data2 = "<l xmlns=\"urn:x\">two</l>";

    mod = lys_parse_mem(st->ctx, idents, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    lyd_free_withsiblings(root);

    lys_features_enable(mod, "x");
    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);

    lys_features_disable(mod, "x");
    assert_int_not_equal(lyd_validate(&root, LYD_OPT_CONFIG, NULL), 0);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    lyd_free_withsiblings(root);
}

static void
test_typedef_11_pattern_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    const char *modstr = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<module name=\"x\"\n"
"        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
"        xmlns:x=\"urn:x\">\n"
"  <yang-version value=\"1.1\"/>\n"
"  <namespace uri=\"urn:x\"/>\n  <prefix value=\"x\"/>\n"
"  <leaf name=\"l\">\n    <type name=\"string\">\n"
"      <pattern value=\"[a-zA-Z_][a-zA-Z0-9\\-_.]*\"/>\n"
"      <pattern value=\"[nN][oO][tT].*\">\n        <modifier value=\"invert-match\"/>\n      </pattern>\n"
"    </type>\n  </leaf>\n</module>\n";
    char *printed;

    struct lyd_node *root;
    const char *data1 = "<l xmlns=\"urn:x\">enabled</l>"; /* legal */
    const char *data2 = "<l xmlns=\"urn:x\">10</l>";      /* ilegal, starts with number */
    const char *data3 = "<l xmlns=\"urn:x\">notoric</l>"; /* ilegal, starts with not */

    mod = lys_parse_mem(st->ctx, modstr, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&printed, mod, LYS_OUT_YIN, NULL, 0, 0);
    assert_ptr_not_equal(printed, NULL);
    assert_string_equal(printed, modstr);
    free(printed);

    root = lyd_parse_mem(st->ctx, data3, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_NOCONSTR);

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_NOCONSTR);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    lyd_free_withsiblings(root);
}

static void
test_typedef_11_pattern_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    const char *modstr = "module x {\n"
"  yang-version 1.1;\n"
"  namespace \"urn:x\";\n  prefix x;\n\n"
"  leaf l {\n    type string {\n"
"      pattern \"[a-zA-Z_][a-zA-Z0-9\\\\-_.]*\";\n"
"      pattern \"[nN][oO][tT].*\" {\n        modifier invert-match;\n      }\n"
"    }\n"
"  }\n"
"}\n";
    char *printed;

    struct lyd_node *root;
    const char *data1 = "<l xmlns=\"urn:x\">enabled</l>"; /* legal */
    const char *data2 = "<l xmlns=\"urn:x\">10</l>";      /* ilegal, starts with number */
    const char *data3 = "<l xmlns=\"urn:x\">notoric</l>"; /* ilegal, starts with not */

    mod = lys_parse_mem(st->ctx, modstr, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);
    lys_print_mem(&printed, mod, LYS_OUT_YANG, NULL, 0, 0);
    assert_ptr_not_equal(printed, NULL);
    assert_string_equal(printed, modstr);
    free(printed);

    root = lyd_parse_mem(st->ctx, data3, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_NOCONSTR);

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_NOCONSTR);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    lyd_free_withsiblings(root);
}

/* this test follows RFC 7950, sec. 9.12.4 */
static void
test_typedef_11_union_leafref_yin(void **state)
{
    struct state *st = (*state);
    const char *modstr = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<module name=\"x\""
"        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\""
"        xmlns:x=\"urn:x\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/><prefix value=\"x\"/>"
"  <typedef name=\"mytype\">"
"    <type name=\"union\">"
"      <type name=\"int32\"/>"
"      <type name=\"enumeration\">"
"        <enum name=\"unbounded\"/>"
"      </type>"
"    </type>"
"  </typedef>"
"  <list name=\"filter\">"
"    <key value=\"name\"/>"
"    <leaf name=\"name\">"
"      <type name=\"string\"/>"
"    </leaf>"
"  </list>"
"  <leaf name=\"filter2\">"
"    <type name=\"union\">"
"      <type name=\"leafref\">"
"        <path value=\"/filter/name\"/>"
"      </type>"
"      <type name=\"enumeration\">"
"        <enum name=\"default-filter\"/>"
"      </type>"
"    </type>"
"  </leaf>"
"  <leaf name=\"filter3\">"
"    <type name=\"union\">"
"      <type name=\"leafref\">"
"        <path value=\"/filter/name\"/>"
"      </type>"
"      <type name=\"string\"/>"
"    </type>"
"  </leaf>"
"  <leaf name=\"filter4\">"
"    <type name=\"union\">"
"      <type name=\"leafref\">"
"        <path value=\"/filter/name\"/>"
"        <require-instance value=\"false\"/>"
"      </type>"
"      <type name=\"string\"/>"
"    </type>"
"  </leaf>"
"</module>";
    struct lyd_node *root;
    const char *data1 = "<filter xmlns=\"urn:x\"><name>http</name></filter>"
                        "<filter2 xmlns=\"urn:x\">http</filter2>"; /* legal */
    const char *data2 = "<filter2 xmlns=\"urn:x\">http</filter2>"; /* illegal, leafref nor enumeration does not match */
    const char *data3 = "<filter3 xmlns=\"urn:x\">http</filter3>"; /* legal, leafref does not match, but string does */
    const char *data4 = "<filter4 xmlns=\"urn:x\">http</filter4>"; /* legal, leafref does not need to match */

    assert_ptr_not_equal(lys_parse_mem(st->ctx, modstr, LYS_IN_YIN), NULL);

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    assert_int_equal(((struct lyd_node_leaf_list *)root->next)->value_type, LY_TYPE_LEAFREF);
    assert_ptr_equal(root->child, ((struct lyd_node_leaf_list *)root->next)->value.leafref);
    lyd_free_withsiblings(root);

    root = lyd_parse_mem(st->ctx, data3, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    assert_int_equal(((struct lyd_node_leaf_list *)root)->value_type, LY_TYPE_STRING);
    lyd_free_withsiblings(root);

    root = lyd_parse_mem(st->ctx, data4, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    assert_int_equal(((struct lyd_node_leaf_list *)root)->value_type, LY_TYPE_STRING);
    assert_int_equal(((struct lyd_node_leaf_list *)root)->value_flags, LY_VALUE_UNRES);
    assert_string_equal("http", ((struct lyd_node_leaf_list *)root)->value.string);
    lyd_free_withsiblings(root);
}

static void
test_typedef_11_union_leafref_yang(void **state)
{
    struct state *st = (*state);
    const char *modstr = "module \"x\"{"
"  yang-version 1.1;"
"  namespace \"urn:x\"; prefix \"x\";"
"  typedef mytype {"
"    type \"union\" {"
"      type int32;"
"      type enumeration {"
"        enum \"unbounded\";"
"      }"
"    }"
"  }"
"  list filter {"
"    key \"name\";"
"    leaf name {"
"      type string;"
"    }"
"  }"
"  leaf filter2 {"
"    type union {"
"      type leafref {"
"        path \"/filter/name\";"
"      }"
"      type \"enumeration\" {"
"        enum \"default-filter\";"
"      }"
"    }"
"  }"
"  leaf \"filter3\" {"
"    type union {"
"      type \"leafref\" {"
"        path \"/filter/name\";"
"      }"
"      type string;"
"    }"
"  }"
"  leaf filter4 {"
"    type union {"
"      type \"leafref\" {"
"        path \"/filter/name\";"
"        require-instance false;"
"      }"
"      type string;"
"    }"
"  }"
"}";
    struct lyd_node *root;
    const char *data1 = "<filter xmlns=\"urn:x\"><name>http</name></filter>"
                        "<filter2 xmlns=\"urn:x\">http</filter2>"; /* legal */
    const char *data2 = "<filter2 xmlns=\"urn:x\">http</filter2>"; /* illegal, leafref nor enumeration does not match */
    const char *data3 = "<filter3 xmlns=\"urn:x\">http</filter3>"; /* legal, leafref does not match, but string does */
    const char *data4 = "<filter4 xmlns=\"urn:x\">http</filter4>"; /* legal, leafref does not need to match */

    assert_ptr_not_equal(lys_parse_mem(st->ctx, modstr, LYS_IN_YANG), NULL);

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    assert_int_equal(((struct lyd_node_leaf_list *)root->next)->value_type, LY_TYPE_LEAFREF);
    assert_ptr_equal(root->child, ((struct lyd_node_leaf_list *)root->next)->value.leafref);
    lyd_free_withsiblings(root);

    root = lyd_parse_mem(st->ctx, data3, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    assert_int_equal(((struct lyd_node_leaf_list *)root)->value_type, LY_TYPE_STRING);
    lyd_free_withsiblings(root);

    root = lyd_parse_mem(st->ctx, data4, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    assert_int_equal(((struct lyd_node_leaf_list *)root)->value_type, LY_TYPE_STRING);
    assert_int_equal(((struct lyd_node_leaf_list *)root)->value_flags, LY_VALUE_UNRES);
    assert_string_equal("http", ((struct lyd_node_leaf_list *)root)->value.string);
    lyd_free_withsiblings(root);
}

static void
test_typedef_11_union_empty_yin(void **state)
{
    struct state *st = (*state);
    const char *modstr = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<module name=\"x\""
"        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\""
"        xmlns:x=\"urn:x\">"
"  <yang-version value=\"1.1\"/>"
"  <namespace uri=\"urn:x\"/><prefix value=\"x\"/>"
"  <typedef name=\"mytype1\">"
"    <type name=\"union\">"
"      <type name=\"int32\"/>"
"      <type name=\"empty\"/>"
"    </type>"
"  </typedef>"
"  <typedef name=\"mytype2\">"
"    <type name=\"union\">"
"      <type name=\"int8\"/>"
"      <type name=\"int16\"/>"
"    </type>"
"  </typedef>"
"  <leaf name=\"value\">"
"    <type name=\"mytype1\"/>"
"  </leaf>"
"  <leaf name=\"integer\">"
"    <type name=\"mytype2\"/>"
"  </leaf>"
"</module>";
    struct lyd_node *root;
    const char *data1 = "<integer xmlns=\"urn:x\"/>"; /* illegal */
    const char *data2 = "<value xmlns=\"urn:x\">xxx</value>"; /* illegal */
    const char *data3 = "<value xmlns=\"urn:x\">11</value>"; /* legal, int32 */
    const char *data4 = "<value xmlns=\"urn:x\"/>"; /* legal, empty */

    assert_ptr_not_equal(lys_parse_mem(st->ctx, modstr, LYS_IN_YIN), NULL);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    root = lyd_parse_mem(st->ctx, data3, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    assert_int_equal(((struct lyd_node_leaf_list *)root)->value_type, LY_TYPE_INT32);
    lyd_free_withsiblings(root);

    root = lyd_parse_mem(st->ctx, data4, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    assert_int_equal(((struct lyd_node_leaf_list *)root)->value_type, LY_TYPE_EMPTY);
    lyd_free_withsiblings(root);
}

static void
test_typedef_11_union_empty_yang(void **state)
{
    struct state *st = (*state);
    const char *modstr = "module x {"
"  yang-version 1.1;"
"  namespace \"urn:x\"; prefix \"x\";"
"  typedef mytype1 {"
"    type union {"
"      type int32;"
"      type empty;"
"    }"
"  }"
"  typedef \"mytype2\" {"
"    type union {"
"      type int8;"
"      type int16;"
"    }"
"  }"
"  leaf value {"
"    type \"mytype1\";"
"  }"
"  leaf integer {"
"    type mytype2;"
"  }"
"}";
    struct lyd_node *root;
    const char *data1 = "<integer xmlns=\"urn:x\"/>"; /* illegal */
    const char *data2 = "<value xmlns=\"urn:x\">xxx</value>"; /* illegal */
    const char *data3 = "<value xmlns=\"urn:x\">11</value>"; /* legal, int32 */
    const char *data4 = "<value xmlns=\"urn:x\"/>"; /* legal, empty */

    assert_ptr_not_equal(lys_parse_mem(st->ctx, modstr, LYS_IN_YANG), NULL);

    root = lyd_parse_mem(st->ctx, data1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    root = lyd_parse_mem(st->ctx, data2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_equal(root, NULL);
    assert_int_equal(ly_vecode(st->ctx), LYVE_INVAL);

    root = lyd_parse_mem(st->ctx, data3, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    assert_int_equal(((struct lyd_node_leaf_list *)root)->value_type, LY_TYPE_INT32);
    lyd_free_withsiblings(root);

    root = lyd_parse_mem(st->ctx, data4, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(root, NULL);
    assert_int_equal(((struct lyd_node_leaf_list *)root)->value_type, LY_TYPE_EMPTY);
    lyd_free_withsiblings(root);
}

static void
test_typedef_patterns_optimizations_schema(struct state *st, const struct lys_module *mod)
{
    const char *valid = "<a xmlns=\"urn:libyang:tests:patterns\">a</a>"
                        "<b xmlns=\"urn:libyang:tests:patterns\">2</b>"
                        "<c xmlns=\"urn:libyang:tests:patterns\">C</c>";
    const char *invalid1 = "<a xmlns=\"urn:libyang:tests:patterns\">1</a>";
    const char *invalid2 = "<b xmlns=\"urn:libyang:tests:patterns\">b</b>";
    const char *invalid3 = "<c xmlns=\"urn:libyang:tests:patterns\">c</c>";
    struct lys_node_grp *grp = NULL;
    struct lys_node_leaf *leaf = NULL;
    struct lys_node *iter;
    struct lyd_node *data;

    /* check optimizations */
    /* 1. module's typedef has PCRE data */
    assert_int_equal(mod->tpdf_size, 1);
    assert_ptr_not_equal(mod->tpdf, NULL);
    assert_int_equal(mod->tpdf[0].type.base, LY_TYPE_STRING);
    assert_int_equal(mod->tpdf[0].type.info.str.pat_count, 1);
#ifdef LY_ENABLED_CACHE
    assert_ptr_not_equal(mod->tpdf[0].type.info.str.patterns_pcre, NULL);
#endif

    /* 2. grouping's typedef has PCRE data */
    LY_TREE_FOR(mod->data, iter) {
        if (iter->nodetype == LYS_GROUPING && !strcmp(iter->name, "a")) {
            grp = (struct lys_node_grp*)iter;
            break;
        }
    }
    assert_ptr_not_equal(grp, NULL);
    assert_int_equal(grp->tpdf_size, 1);
    assert_ptr_not_equal(grp->tpdf, NULL);
    assert_int_equal(grp->tpdf[0].type.base, LY_TYPE_STRING);
    assert_int_equal(grp->tpdf[0].type.info.str.pat_count, 1);
#ifdef LY_ENABLED_CACHE
    assert_ptr_not_equal(grp->tpdf[0].type.info.str.patterns_pcre, NULL);
#endif

    /* 3. grouping's leaf does not have PCRE data */
    LY_TREE_FOR(mod->data, iter) {
        if (iter->nodetype == LYS_GROUPING && !strcmp(iter->name, "b")) {
            leaf = (struct lys_node_leaf*)iter->child;
            break;
        }
    }
    assert_ptr_not_equal(leaf, NULL);
    assert_int_equal(leaf->type.base, LY_TYPE_STRING);
    assert_int_equal(leaf->type.info.str.pat_count, 1);
#ifdef LY_ENABLED_CACHE
    assert_ptr_equal(leaf->type.info.str.patterns_pcre, NULL);
#endif
    leaf = NULL;

    /* 4. but it's instantiated copy does have PCRE data */
    LY_TREE_FOR(mod->data, iter) {
        if (iter->nodetype == LYS_USES && !strcmp(iter->name, "b")) {
            leaf = (struct lys_node_leaf*)iter->child;
            break;
        }
    }
    assert_ptr_not_equal(leaf, NULL);
    assert_int_equal(leaf->type.base, LY_TYPE_STRING);
    assert_int_equal(leaf->type.info.str.pat_count, 1);
#ifdef LY_ENABLED_CACHE
    assert_ptr_not_equal(leaf->type.info.str.patterns_pcre, NULL);
#endif

    /* check data */
    data = lyd_parse_mem(st->ctx, valid, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(data, NULL);
    lyd_free_withsiblings(data);

    assert_ptr_equal(lyd_parse_mem(st->ctx, invalid1, LYD_XML, LYD_OPT_CONFIG), NULL);
    assert_ptr_equal(lyd_parse_mem(st->ctx, invalid2, LYD_XML, LYD_OPT_CONFIG), NULL);
    assert_ptr_equal(lyd_parse_mem(st->ctx, invalid3, LYD_XML, LYD_OPT_CONFIG), NULL);
}

static void
test_typedef_patterns_optimizations_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    mod = lys_parse_path(st->ctx, TESTS_DIR"/schema/yang/files/patterns.yang", LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    test_typedef_patterns_optimizations_schema(st, mod);
}

static void
test_typedef_patterns_optimizations_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;

    mod = lys_parse_path(st->ctx, TESTS_DIR"/schema/yin/files/patterns.yin", LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    test_typedef_patterns_optimizations_schema(st, mod);
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_typedef_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_yang, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11in10, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_enums_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_enums_yang, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_bits_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_bits_yang, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_iff_ident_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_iff_ident_yang, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_iff_enums_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_iff_enums_yang, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_iff_bits_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_iff_bits_yang, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_pattern_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_pattern_yang, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_multidents_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_multidents_yang, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_union_leafref_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_union_leafref_yang, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_union_empty_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_11_union_empty_yang, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_patterns_optimizations_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_typedef_patterns_optimizations_yang, setup_ctx, teardown_ctx),
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
