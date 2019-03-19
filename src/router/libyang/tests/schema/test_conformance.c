/**
 * \file test_conformance.c
 * \author Radek Krejci <rkrejci@cesnet.cz>
 * \brief libyang tests - setting up modules as imported or implemented
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
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cmocka.h>

#include "libyang.h"
#include <context.h>
#include "tests/config.h"

#define SCHEMA_FOLDER_YIN TESTS_DIR"/schema/yin/conformance"
#define SCHEMA_FOLDER_YANG TESTS_DIR"/schema/yang/conformance"

static int
setup_ctx(void **state)
{
    *state = ly_ctx_new(NULL, 0);
    if (!*state) {
        return -1;
    }
    return 0;
}

static int
teardown_ctx(void **state)
{
    ly_ctx_destroy(*state, NULL);
    return 0;
}

static void
test_implemented1_yin(void **state)
{
    struct ly_ctx *ctx = *state;
    const struct lys_module *a, *b, *b2, *c, *c2;

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YIN);

    /* loads a.yin (impl), b@2015-01-01.yin (impl by augment) and c@2015-03-03.yin (imp) */
    a = lys_parse_path(ctx, SCHEMA_FOLDER_YIN"/a.yin", LYS_IN_YIN);
    assert_ptr_not_equal(a, NULL);
    assert_int_equal(a->implemented, 1);

    b = ly_ctx_get_module(ctx, "b", NULL, 1);
    assert_ptr_not_equal(b, NULL);
    assert_int_equal(b->implemented, 1);

    c = ly_ctx_get_module(ctx, "c", NULL, 0);
    assert_ptr_not_equal(c, NULL);
    assert_int_equal(c->implemented, 0);

    /* another b cannot be loaded, since it is already implemented */
    b2 = lys_parse_path(ctx, SCHEMA_FOLDER_YIN"/b@2015-04-04.yin", LYS_IN_YIN);
    assert_ptr_equal(b2, NULL);
    assert_int_equal(ly_errno, LY_EINVAL);
    assert_string_equal(ly_errmsg(ctx), "Module \"b\" parsing failed.");

    /* older c can be loaded and it will be marked as implemented */
    c2 = lys_parse_path(ctx, SCHEMA_FOLDER_YIN"/c@2015-01-01.yin", LYS_IN_YIN);
    assert_ptr_not_equal(c2, NULL);
    assert_int_equal(c2->implemented, 1);
    assert_ptr_not_equal(c, c2);
}

static void
test_implemented1_yang(void **state)
{
    struct ly_ctx *ctx = *state;
    const struct lys_module *a, *b, *b2, *c, *c2;

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YANG);

    /* loads a.yang (impl), b@2015-01-01.yang (impl by augment) and c@2015-03-03.yang (imp) */
    a = lys_parse_path(ctx, SCHEMA_FOLDER_YANG"/a.yang", LYS_IN_YANG);
    assert_ptr_not_equal(a, NULL);
    assert_int_equal(a->implemented, 1);

    b = ly_ctx_get_module(ctx, "b", NULL, 1);
    assert_ptr_not_equal(b, NULL);
    assert_int_equal(b->implemented, 1);

    c = ly_ctx_get_module(ctx, "c", NULL, 0);
    assert_ptr_not_equal(c, NULL);
    assert_int_equal(c->implemented, 0);

    /* another b cannot be loaded, since it is already implemented */
    b2 = lys_parse_path(ctx, SCHEMA_FOLDER_YANG"/b@2015-04-04.yang", LYS_IN_YANG);
    assert_ptr_equal(b2, NULL);
    assert_int_equal(ly_errno, LY_EINVAL);
    assert_string_equal(ly_errmsg(ctx), "Module \"b\" parsing failed.");

    /* older c can be loaded and it will be marked as implemented */
    c2 = lys_parse_path(ctx, SCHEMA_FOLDER_YANG"/c@2015-01-01.yang", LYS_IN_YANG);
    assert_ptr_not_equal(c2, NULL);
    assert_int_equal(c2->implemented, 1);
    assert_ptr_not_equal(c, c2);
}

static void
test_implemented2_yin(void **state)
{
    struct ly_ctx *ctx = *state;
    const struct lys_module *a, *b2;

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YIN);

    /* load the newest b first, it is implemented */
    b2 = ly_ctx_load_module(ctx, "b", "2015-04-04");
    assert_ptr_not_equal(b2, NULL);
    assert_int_equal(b2->implemented, 1);

    /* loads a.yin (impl), b@2015-04-04 is augmented by a, but cannot be implemented */
    a = lys_parse_path(ctx, SCHEMA_FOLDER_YIN"/a.yin", LYS_IN_YIN);
    assert_ptr_equal(a, NULL);
}

static void
test_implemented2_yang(void **state)
{
    struct ly_ctx *ctx = *state;
    const struct lys_module *a, *b2;

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YANG);

    /* load the newest b first, it is implemented */
    b2 = ly_ctx_load_module(ctx, "b", "2015-04-04");
    assert_ptr_not_equal(b2, NULL);
    assert_int_equal(b2->implemented, 1);

    /* loads a.yin (impl), b@2015-04-04 is augmented by a, but cannot be implemented */
    a = lys_parse_path(ctx, SCHEMA_FOLDER_YANG"/a.yang", LYS_IN_YANG);
    assert_ptr_equal(a, NULL);
}

static void
test_implemented_info_yin(void **state)
{
    struct ly_ctx *ctx = *state;
    struct lyd_node *info;
    const struct lys_module *a;
    char *data;
    const char *template = "<yang-library xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
"  <module-set>\n"
"    <name>complete</name>\n"
"    <checksum>10</checksum>\n"
"    <import-only-module>\n"
"      <name>ietf-yang-metadata</name>\n"
"      <revision>2016-08-05</revision>\n"
"      <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-metadata</namespace>\n"
"    </import-only-module>\n"
"    <module>\n"
"      <name>yang</name>\n"
"      <revision>2017-02-20</revision>\n"
"      <namespace>urn:ietf:params:xml:ns:yang:1</namespace>\n"
"    </module>\n"
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
"      <revision>2017-08-17</revision>\n"
"      <namespace>urn:ietf:params:xml:ns:yang:ietf-datastores</namespace>\n"
"    </import-only-module>\n"
"    <module>\n"
"      <name>ietf-yang-library</name>\n"
"      <revision>2018-01-17</revision>\n"
"      <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>\n"
"    </module>\n"
"    <module>\n"
"      <name>b</name>\n"
"      <revision>2015-01-01</revision>\n"
"      <namespace>urn:example:b</namespace>\n"
"      <location>file://"SCHEMA_FOLDER_YIN"/b@2015-01-01.yin</location>\n"
"    </module>\n"
"    <import-only-module>\n"
"      <name>c</name>\n"
"      <revision>2015-03-03</revision>\n"
"      <namespace>urn:example:c</namespace>\n"
"      <location>file://"SCHEMA_FOLDER_YIN"/c@2015-03-03.yin</location>\n"
"    </import-only-module>\n"
"    <module>\n"
"      <name>a</name>\n"
"      <revision>2015-01-01</revision>\n"
"      <namespace>urn:example:a</namespace>\n"
"      <location>file://"SCHEMA_FOLDER_YIN"/a.yin</location>\n"
"      <feature>\n"
"        <name>foo</name>\n"
"      </feature>\n"
"    </module>\n"
"  </module-set>\n"
"  <checksum>10</checksum>\n"
"</yang-library>\n"
"<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
"  <module>\n"
"    <name>ietf-yang-metadata</name>\n"
"    <revision>2016-08-05</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-metadata</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>yang</name>\n"
"    <revision>2017-02-20</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:1</namespace>\n"
"    <conformance-type>implement</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>ietf-inet-types</name>\n"
"    <revision>2013-07-15</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-inet-types</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>ietf-yang-types</name>\n"
"    <revision>2013-07-15</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-types</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>ietf-datastores</name>\n"
"    <revision>2017-08-17</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-datastores</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>ietf-yang-library</name>\n"
"    <revision>2018-01-17</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>\n"
"    <conformance-type>implement</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>b</name>\n"
"    <revision>2015-01-01</revision>\n"
"    <schema>file://"SCHEMA_FOLDER_YIN"/b@2015-01-01.yin</schema>\n"
"    <namespace>urn:example:b</namespace>\n"
"    <conformance-type>implement</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>c</name>\n"
"    <revision>2015-03-03</revision>\n"
"    <schema>file://"SCHEMA_FOLDER_YIN"/c@2015-03-03.yin</schema>\n"
"    <namespace>urn:example:c</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>a</name>\n"
"    <revision>2015-01-01</revision>\n"
"    <schema>file://"SCHEMA_FOLDER_YIN"/a.yin</schema>\n"
"    <namespace>urn:example:a</namespace>\n"
"    <feature>foo</feature>\n"
"    <conformance-type>implement</conformance-type>\n"
"  </module>\n"
"  <module-set-id>10</module-set-id>\n"
"</modules-state>\n";

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YIN);

    /* loads a.yin (impl), b@2015-01-01.yin (impl by augment) and c@2015-03-03.yin (imp) */
    assert_ptr_not_equal((a = lys_parse_path(ctx, SCHEMA_FOLDER_YIN"/a.yin", LYS_IN_YIN)), NULL);
    assert_int_equal(lys_features_enable(a, "foo"), 0);

    /* get yang-library data */
    info = ly_ctx_info(ctx);
    assert_ptr_not_equal(info, NULL);

    lyd_print_mem(&data, info, LYD_XML, LYP_FORMAT | LYP_WITHSIBLINGS);
    lyd_free_withsiblings(info);
    assert_string_equal(data, template);
    free(data);
}

static void
test_implemented_info_yang(void **state)
{
    struct ly_ctx *ctx = *state;
    struct lyd_node *info;
    const struct lys_module *a;
    char *data;
    const char *template = "<yang-library xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
"  <module-set>\n"
"    <name>complete</name>\n"
"    <checksum>10</checksum>\n"
"    <import-only-module>\n"
"      <name>ietf-yang-metadata</name>\n"
"      <revision>2016-08-05</revision>\n"
"      <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-metadata</namespace>\n"
"    </import-only-module>\n"
"    <module>\n"
"      <name>yang</name>\n"
"      <revision>2017-02-20</revision>\n"
"      <namespace>urn:ietf:params:xml:ns:yang:1</namespace>\n"
"    </module>\n"
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
"      <revision>2017-08-17</revision>\n"
"      <namespace>urn:ietf:params:xml:ns:yang:ietf-datastores</namespace>\n"
"    </import-only-module>\n"
"    <module>\n"
"      <name>ietf-yang-library</name>\n"
"      <revision>2018-01-17</revision>\n"
"      <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>\n"
"    </module>\n"
"    <module>\n"
"      <name>b</name>\n"
"      <revision>2015-01-01</revision>\n"
"      <namespace>urn:example:b</namespace>\n"
"      <location>file://"SCHEMA_FOLDER_YANG"/b@2015-01-01.yang</location>\n"
"    </module>\n"
"    <import-only-module>\n"
"      <name>c</name>\n"
"      <revision>2015-03-03</revision>\n"
"      <namespace>urn:example:c</namespace>\n"
"      <location>file://"SCHEMA_FOLDER_YANG"/c@2015-03-03.yang</location>\n"
"    </import-only-module>\n"
"    <module>\n"
"      <name>a</name>\n"
"      <revision>2015-01-01</revision>\n"
"      <namespace>urn:example:a</namespace>\n"
"      <location>file://"SCHEMA_FOLDER_YANG"/a.yang</location>\n"
"      <feature>\n"
"        <name>foo</name>\n"
"      </feature>\n"
"    </module>\n"
"  </module-set>\n"
"  <checksum>10</checksum>\n"
"</yang-library>\n"
"<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n"
"  <module>\n"
"    <name>ietf-yang-metadata</name>\n"
"    <revision>2016-08-05</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-metadata</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>yang</name>\n"
"    <revision>2017-02-20</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:1</namespace>\n"
"    <conformance-type>implement</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>ietf-inet-types</name>\n"
"    <revision>2013-07-15</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-inet-types</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>ietf-yang-types</name>\n"
"    <revision>2013-07-15</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-types</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>ietf-datastores</name>\n"
"    <revision>2017-08-17</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-datastores</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>ietf-yang-library</name>\n"
"    <revision>2018-01-17</revision>\n"
"    <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>\n"
"    <conformance-type>implement</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>b</name>\n"
"    <revision>2015-01-01</revision>\n"
"    <schema>file://"SCHEMA_FOLDER_YANG"/b@2015-01-01.yang</schema>\n"
"    <namespace>urn:example:b</namespace>\n"
"    <conformance-type>implement</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>c</name>\n"
"    <revision>2015-03-03</revision>\n"
"    <schema>file://"SCHEMA_FOLDER_YANG"/c@2015-03-03.yang</schema>\n"
"    <namespace>urn:example:c</namespace>\n"
"    <conformance-type>import</conformance-type>\n"
"  </module>\n"
"  <module>\n"
"    <name>a</name>\n"
"    <revision>2015-01-01</revision>\n"
"    <schema>file://"SCHEMA_FOLDER_YANG"/a.yang</schema>\n"
"    <namespace>urn:example:a</namespace>\n"
"    <feature>foo</feature>\n"
"    <conformance-type>implement</conformance-type>\n"
"  </module>\n"
"  <module-set-id>10</module-set-id>\n"
"</modules-state>\n";

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER_YANG);

    /* loads a.yang (impl), b@2015-01-01.yang (impl by augment) and c@2015-03-03.yang (imp) */
    assert_ptr_not_equal((a = lys_parse_path(ctx, SCHEMA_FOLDER_YANG"/a.yang", LYS_IN_YANG)), NULL);
    assert_int_equal(lys_features_enable(a, "foo"), 0);

    /* get yang-library data */
    info = ly_ctx_info(ctx);
    assert_ptr_not_equal(info, NULL);

    lyd_print_mem(&data, info, LYD_XML, LYP_FORMAT | LYP_WITHSIBLINGS);
    lyd_free_withsiblings(info);
    assert_string_equal(data, template);
    free(data);
}

static void
test_revision_date_yin(void **state)
{
    struct ly_ctx *ctx = *state;
    const char *yin1 = "<module name=\"x\""
          "xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\""
          "xmlns:x=\"urn:cesnet:x\">"
        "<namespace uri=\"urn:cesnet:x\"/>"
        "<prefix value=\"x\"/>"
        "<revision date=\"2018-02-29\"/>"
        "</module>";
    const char *yin2 = "<module name=\"x\""
          "xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\""
          "xmlns:x=\"urn:cesnet:x\">"
        "<namespace uri=\"urn:cesnet:x\"/>"
        "<prefix value=\"x\"/>"
        "<revision date=\"18-02-28\"/>"
        "</module>";
    const char *yin3 = "<module name=\"x\""
          "xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\""
          "xmlns:x=\"urn:cesnet:x\">"
        "<namespace uri=\"urn:cesnet:x\"/>"
        "<prefix value=\"x\"/>"
        "<revision date=\"today\"/>"
        "</module>";
    const char *yin4 = "<module name=\"x\""
          "xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\""
          "xmlns:x=\"urn:cesnet:x\">"
        "<namespace uri=\"urn:cesnet:x\"/>"
        "<prefix value=\"x\"/>"
        "<revision date=\"2018-02-28\"/>"
        "</module>";
    const char *yin5 = "<module name=\"y\""
          "xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\""
          "xmlns:y=\"urn:cesnet:y\">"
        "<namespace uri=\"urn:cesnet:y\"/>"
        "<prefix value=\"y\"/>"
        "<revision date=\"2016-02-29\"/>"
        "</module>";
    const char *yin6 = "<module name=\"z\""
          "xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\""
          "xmlns:z=\"urn:cesnet:z\">"
        "<namespace uri=\"urn:cesnet:z\"/>"
        "<prefix value=\"z\"/>"
        "<revision date=\"2000-02-29\"/>"
        "</module>";

    /* invalid dates */
    assert_ptr_equal(lys_parse_mem(ctx, yin1, LYS_IN_YIN), NULL);
    assert_int_equal(ly_vecode(ctx), LYVE_INDATE);

    assert_ptr_equal(lys_parse_mem(ctx, yin2, LYS_IN_YIN), NULL);
    assert_int_equal(ly_vecode(ctx), LYVE_INDATE);

    assert_ptr_equal(lys_parse_mem(ctx, yin3, LYS_IN_YIN), NULL);
    assert_int_equal(ly_vecode(ctx), LYVE_INDATE);

    /* valid dates */
    assert_ptr_not_equal(lys_parse_mem(ctx, yin4, LYS_IN_YIN), NULL);
    assert_ptr_not_equal(lys_parse_mem(ctx, yin5, LYS_IN_YIN), NULL);
    assert_ptr_not_equal(lys_parse_mem(ctx, yin6, LYS_IN_YIN), NULL);
}

static void
test_revision_date_yang(void **state)
{
    struct ly_ctx *ctx = *state;
    const char *yang1 = "module x {"
          "namespace urn:cesnet:x;"
          "prefix x;"
          "revision \"2018-02-29\";}";
    const char *yang2 = "module x {"
          "namespace urn:cesnet:x;"
          "prefix x;"
          "revision \"18-02-28\";}";
    const char *yang3 = "module x {"
          "namespace urn:cesnet:x;"
          "prefix x;"
          "revision \"today\";}";
    const char *yang4 = "module x {"
          "namespace urn:cesnet:x;"
          "prefix x;"
          "revision \"2018-02-28\";}";
    const char *yang5 = "module y {"
          "namespace urn:cesnet:y;"
          "prefix y;"
          "revision \"2016-02-29\";}";
    const char *yang6 = "module z {"
          "namespace urn:cesnet:z;"
          "prefix z;"
          "revision \"2000-02-29\";}";

    /* invalid dates */
    assert_ptr_equal(lys_parse_mem(ctx, yang1, LYS_IN_YANG), NULL);
    assert_int_equal(ly_vecode(ctx), LYVE_INDATE);

    assert_ptr_equal(lys_parse_mem(ctx, yang2, LYS_IN_YANG), NULL);
    assert_int_equal(ly_vecode(ctx), LYVE_INDATE);

    assert_ptr_equal(lys_parse_mem(ctx, yang3, LYS_IN_YANG), NULL);
    assert_int_equal(ly_vecode(ctx), LYVE_INDATE);

    /* valid dates */
    assert_ptr_not_equal(lys_parse_mem(ctx, yang4, LYS_IN_YANG), NULL);
    assert_ptr_not_equal(lys_parse_mem(ctx, yang5, LYS_IN_YANG), NULL);
    assert_ptr_not_equal(lys_parse_mem(ctx, yang6, LYS_IN_YANG), NULL);
}


const struct lys_module *
_my_data_clb(struct ly_ctx *ctx, const char *name, const char *ns, int options, void *user_data)
{
    char filepath[256] = {0};
    const struct lys_module *ly_module = NULL;

    fprintf(stderr, "%s:%i %s() name:%s ns:%s \n", __FILE__, __LINE__, __FUNCTION__, name, ns);

    snprintf(filepath, sizeof(filepath), "%s/%s.yang", SCHEMA_FOLDER_YANG, name);
    ly_module = lys_parse_path(ctx, filepath, LYS_IN_YANG);
    if ( !ly_module ) {
        fprintf(stderr, "%s:%i %s() lys_parse_path(%s) failed (%d %p)\n", __FILE__, __LINE__, __FUNCTION__, filepath, options, user_data);
    }

    return ly_module;
}

static void
test_issue_already_implemented(void **state) {
    struct ly_ctx *ctx = *state;
    const struct lys_module *a = NULL;
    char* search_paths[] = {SCHEMA_FOLDER_YANG, NULL};

    ctx->models.search_paths = search_paths;
    ctx->data_clb = _my_data_clb;

    a = lys_parse_path(ctx, SCHEMA_FOLDER_YANG "/ident-aug-must-issue-apst.yang", LYS_IN_YANG);

    ctx->data_clb = NULL;
    ctx->models.search_paths = NULL;

    assert_ptr_not_equal(a, NULL);

}


int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_implemented1_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_implemented1_yang, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_implemented2_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_implemented2_yang, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_implemented_info_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_implemented_info_yang, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_revision_date_yin, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_revision_date_yang, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_issue_already_implemented, setup_ctx, teardown_ctx)
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
