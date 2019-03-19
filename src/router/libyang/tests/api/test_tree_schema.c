/**
 * @file test_data_initialization.c
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Cmocka data test initialization.
 *
 * Copyright (c) 2015 Sartura d.o.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include "tests/config.h"
#include "libyang.h"

#define TMP_TEMPLATE "/tmp/libyang-XXXXXX"

struct ly_ctx *ctx = NULL;
struct lyd_node *root = NULL;

const char *lys_module_a = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>           \
<module name=\"a\"                                    \
        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"   \
        xmlns:a=\"urn:a\">                            \
  <namespace uri=\"urn:a\"/>                          \
  <prefix value=\"a_mod\"/>                           \
  <include module=\"asub\"/>                          \
  <include module=\"atop\"/>                          \
  <revision date=\"2015-01-01\">                      \
    <description>                                     \
      <text>version 1</text>                          \
    </description>                                    \
    <reference>                                       \
      <text>RFC XXXX</text>                           \
    </reference>                                      \
  </revision>                                         \
  <feature name=\"foo\"/>                             \
  <grouping name=\"gg\">                              \
    <leaf name=\"bar-gggg\">                          \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </grouping>                                         \
  <container name=\"x\">                              \
    <leaf name=\"bar-leaf\">                          \
      <if-feature name=\"bar\"/>                      \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <uses name=\"gg\">                                \
      <if-feature name=\"bar\"/>                      \
    </uses>                                           \
    <leaf name=\"baz\">                               \
      <if-feature name=\"foo\"/>                      \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <leaf name=\"bubba\">                             \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </container>                                        \
  <augment target-node=\"/x\">                        \
    <if-feature name=\"bar\"/>                        \
    <container name=\"bar-y\">                        \
      <leaf name=\"ll\">                              \
        <type name=\"string\"/>                       \
      </leaf>                                         \
    </container>                                      \
  </augment>                                          \
  <rpc name=\"bar-rpc\">                              \
    <if-feature name=\"bar\"/>                        \
  </rpc>                                              \
  <rpc name=\"foo-rpc\">                              \
    <if-feature name=\"foo\"/>                        \
  </rpc>                                              \
</module>                                             \
";

char lys_module_b[] =
"module b {\
    namespace \"urn:b\";\
    prefix b_mod;\
    include bsub;\
    include btop;\
    feature foo;\
    grouping gg {\
        leaf bar-gggg {\
            type string;\
        }\
    }\
    container x {\
        leaf bar-leaf {\
            if-feature \"bar\";\
            type string;\
        }\
        uses gg {\
            if-feature \"bar\";\
        }\
        leaf baz {\
            if-feature \"foo\";\
            type string;\
        }\
        leaf bubba {\
            type string;\
        }\
    }\
    augment \"/x\" {\
        if-feature \"bar\";\
        container bar-y;\
    }\
    rpc bar-rpc {\
        if-feature \"bar\";\
    }\
    rpc foo-rpc {\
        if-feature \"foo\";\
    }\
}";

const char *lys_module_a_with_typo = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>           \
<module_typo name=\"a\"                                    \
        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"   \
        xmlns:a=\"urn:a\">                            \
  <namespace uri=\"urn:a\"/>                          \
  <prefix value=\"a_mod\"/>                           \
  <include module=\"asub\"/>                          \
  <include module=\"atop\"/>                          \
  <feature name=\"foo\"/>                             \
  <grouping name=\"gg\">                              \
    <leaf name=\"bar-gggg\">                          \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </grouping>                                         \
  <container name=\"x\">                              \
    <leaf name=\"bar-leaf\">                          \
      <if-feature name=\"bar\"/>                      \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <uses name=\"gg\">                                \
      <if-feature name=\"bar\"/>                      \
    </uses>                                           \
    <leaf name=\"baz\">                               \
      <if-feature name=\"foo\"/>                      \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <leaf name=\"bubba\">                             \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </container>                                        \
  <augment target-node=\"/x\">                        \
    <if-feature name=\"bar\"/>                        \
    <container name=\"bar-y\">                        \
      <leaf name=\"ll\">                              \
        <type name=\"string\"/>                       \
      </leaf>                                         \
    </container>                                      \
  </augment>                                          \
  <rpc name=\"bar-rpc\">                              \
    <if-feature name=\"bar\"/>                        \
  </rpc>                                              \
  <rpc name=\"foo-rpc\">                              \
    <if-feature name=\"foo\"/>                        \
  </rpc>                                              \
</module>                                             \
";

char *result_tree = "\
module: a\n\
  +--rw top\n\
  |  +--rw bar-sub2\n\
  +--rw x\n\
     +--rw bubba?      string\n";

char *result_yang = "\
module a {\n\
  namespace \"urn:a\";\n\
  prefix a_mod;\n\
\n\
  include \"asub\";\n\
\n\
  include \"atop\";\n\
\n\
  revision 2015-01-01 {\n\
    description\n\
      \"version 1\";\n\
    reference\n\
      \"RFC XXXX\";\n\
  }\n\
\n\
  feature foo;\n\
\n\
  grouping gg {\n\
    leaf bar-gggg {\n\
      type string;\n\
    }\n\
  }\n\
\n\
  container x {\n\
    leaf bar-leaf {\n\
      if-feature \"bar\";\n\
      type string;\n\
    }\n\n\
    uses gg {\n\
      if-feature \"bar\";\n\
    }\n\n\
    leaf baz {\n\
      if-feature \"foo\";\n\
      type string;\n\
    }\n\n\
    leaf bubba {\n\
      type string;\n\
    }\n\
  }\n\
\n\
  augment \"/x\" {\n\
    if-feature \"bar\";\n\
    container bar-y {\n\
      leaf ll {\n\
        type string;\n\
      }\n\
    }\n\
  }\n\
\n\
  rpc bar-rpc {\n\
    if-feature \"bar\";\n\
  }\n\
\n\
  rpc foo-rpc {\n\
    if-feature \"foo\";\n\
  }\n\
}\n";

char *result_yin = "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<module name=\"a\"\n\
        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n\
        xmlns:a_mod=\"urn:a\">\n\
  <namespace uri=\"urn:a\"/>\n\
  <prefix value=\"a_mod\"/>\n\
  <include module=\"asub\"/>\n\
  <include module=\"atop\"/>\n\
  <revision date=\"2015-01-01\">\n\
    <description>\n\
      <text>version 1</text>\n\
    </description>\n\
    <reference>\n\
      <text>RFC XXXX</text>\n\
    </reference>\n\
  </revision>\n\
  <feature name=\"foo\"/>\n\
  <grouping name=\"gg\">\n\
    <leaf name=\"bar-gggg\">\n\
      <type name=\"string\"/>\n\
    </leaf>\n\
  </grouping>\n\
  <container name=\"x\">\n\
    <leaf name=\"bar-leaf\">\n\
      <if-feature name=\"bar\"/>\n\
      <type name=\"string\"/>\n\
    </leaf>\n\
    <uses name=\"gg\">\n\
      <if-feature name=\"bar\"/>\n\
    </uses>\n\
    <leaf name=\"baz\">\n\
      <if-feature name=\"foo\"/>\n\
      <type name=\"string\"/>\n\
    </leaf>\n\
    <leaf name=\"bubba\">\n\
      <type name=\"string\"/>\n\
    </leaf>\n\
  </container>\n\
  <augment target-node=\"/x\">\n\
    <if-feature name=\"bar\"/>\n\
    <container name=\"bar-y\">\n\
      <leaf name=\"ll\">\n\
        <type name=\"string\"/>\n\
      </leaf>\n\
    </container>\n\
  </augment>\n\
  <rpc name=\"bar-rpc\">\n\
    <if-feature name=\"bar\"/>\n\
  </rpc>\n\
  <rpc name=\"foo-rpc\">\n\
    <if-feature name=\"foo\"/>\n\
  </rpc>\n\
</module>\n";

char *result_info ="\
Feature:   foo\n\
Module:    a\n\
Desc:      \n\
Reference: \n\
Status:    current\n\
Enabled:   no\n\
If-feats:  \n";

char *result_jsons = "{\"a\":{"
  "\"namespace\":\"urn:a\","
  "\"prefix\":\"a_mod\","
  "\"yang-version\":{\"value\":\"1.0\"},"
  "\"revision\":{\"2015-01-01\":{\"description\":{\"text\":\"version 1\"},\"reference\":{\"text\":\"RFC XXXX\"}}},"
  "\"include\":{\"asub\":{},\"atop\":{}},"
  "\"features\":[\"foo\"],"
  "\"augment\":{\"/x\":{"
    "\"if-features\":[\"bar\"],"
    "\"data\":{\"bar-y\":{\"nodetype\":\"container\"}}}},"
  "\"groupings\":[\"gg\"],"
  "\"data\":{\"top\":{\"nodetype\":\"container\",\"included-from\":\"atop\"},"
    "\"x\":{\"nodetype\":\"container\"}},"
  "\"rpcs\":[\"bar-rpc\",\"foo-rpc\"],"
  "\"notifications\":[\"bar-notif\",\"fox-notif\"]}}";

char *result_jsons_grouping = "{\"gg\":{\"module\":\"a\",\"data\":{\"bar-gggg\":{\"nodetype\":\"leaf\"}}}}";

static int
setup_f(void **state)
{
    (void) state; /* unused */
    char *yang_folder = TESTS_DIR"/api/files";

    ctx = ly_ctx_new(yang_folder, 0);
    if (!ctx) {
        return -1;
    }

    return 0;
}

static int
teardown_f(void **state)
{
    (void) state; /* unused */
    if (root)
        lyd_free(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    return 0;
}

static void
test_lys_parse_mem(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    char *yang_folder = TESTS_DIR"/api/files";

    LYS_INFORMAT yang_format = LYS_IN_YIN;

    module = NULL;
    ctx = NULL;
    ctx = ly_ctx_new(yang_folder, 0);
    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        fail();
    }

    assert_string_equal("a", module->name);

    module = NULL;
    module = lys_parse_mem(ctx, lys_module_a_with_typo, yang_format);
    if (module) {
        fail();
    }

    module = NULL;
    module = lys_parse_mem(ctx, lys_module_b, LYS_IN_YANG);
    if (!module) {
        fail();
    }

    assert_string_equal("b", module->name);

    module = NULL;
    ly_ctx_destroy(ctx, NULL);
    ctx = NULL;
}

static void
test_lys_parse_fd(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    char *yang_folder = TESTS_DIR"/api/files";
    char *yin_file = TESTS_DIR"/api/files/a.yin";
    char *yang_file = TESTS_DIR"/api/files/b.yang";
    int fd = -1;

    ctx = ly_ctx_new(yang_folder, 0);

    fd = open(yin_file, O_RDONLY);
    if (fd == -1) {
        fail();
    }

    module = lys_parse_fd(ctx, fd, LYS_IN_YIN);
    if (!module) {
        fail();
    }

    close(fd);
    assert_string_equal("a", module->name);

    fd = open(yang_file, O_RDONLY);
    if (fd == -1) {
        fail();
    }

    module = lys_parse_fd(ctx, fd, LYS_IN_YANG);
    if (!module) {
        fail();
    }

    close(fd);
    assert_string_equal("b", module->name);

    module = lys_parse_mem(ctx, lys_module_a, LYS_IN_YIN);
    if (module) {
        fail();
    }

    ly_ctx_destroy(ctx, NULL);
    ctx = NULL;
}

static void
test_lys_parse_path(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    char *yang_folder = TESTS_DIR"/api/files";
    char *yin_file = TESTS_DIR"/api/files/a.yin";
    char *yang_file = TESTS_DIR"/api/files/b.yang";
    int fd = -1;

    fd = open(yin_file, O_RDONLY);
    if (fd == -1) {
        fail();
    }
    close(fd);

    fd = open(yang_file, O_RDONLY);
    if (fd == -1) {
        fail();
    }
    close(fd);

    ctx = ly_ctx_new(yang_folder, 0);
    module = lys_parse_path(ctx, yin_file, LYS_IN_YIN);
    if (!module) {
        fail();
    }

    assert_string_equal("a", module->name);

    module = lys_parse_path(ctx, yang_file, LYS_IN_YANG);
    if (!module) {
        fail();
    }

    assert_string_equal("b", module->name);

    ly_ctx_destroy(ctx, NULL);
    ctx = NULL;
}

static void
test_lys_features_list(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    const char **result;
    uint8_t st = 1;
    uint8_t *states = &st;

    module = lys_parse_mem(ctx, lys_module_a, LYS_IN_YIN);
    if (!module) {
        fail();
    }
    result = lys_features_list(module, &states);

    assert_string_equal("foo", *result);

    free(result);
    free(states);

    module = lys_parse_mem(ctx, lys_module_b, LYS_IN_YANG);
    if (!module) {
        fail();
    }
    result = lys_features_list(module, &states);

    assert_string_equal("foo", *result);

    free(result);
    free(states);
}

static void
test_lys_features_enable(void **state)
{
    (void) state; /* unused */
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    const struct lys_module *module;
    int rc;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        fail();
    }

    assert_string_equal("foo", module->features->name);
    assert_int_equal(0x0, module->features->flags);

    rc = lys_features_enable(module, "*");
    if (rc) {
        fail();
    }


    assert_int_equal(LYS_FENABLED, module->features->flags);
}

static void
test_lys_features_disable(void **state)
{
    (void) state; /* unused */
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    const struct lys_module *module;
    int rc;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        fail();
    }

    assert_string_equal("foo", module->features->name);
    assert_int_equal(0x0, module->features->flags);

    rc = lys_features_enable(module, "*");
    if (rc) {
        fail();
    }


    assert_int_equal(LYS_FENABLED, module->features->flags);

    rc = lys_features_disable(module, "*");
    if (rc) {
        fail();
    }


    assert_int_equal(0x0, module->features->flags);
}

static void
test_lys_features_state(void **state)
{
    (void) state; /* unused */
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    const struct lys_module *module;
    int feature_state;
    int rc;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        fail();
    }

    assert_string_equal("foo", module->features->name);

    feature_state = lys_features_state(module, "foo");
    assert_int_equal(0x0, feature_state);

    rc = lys_features_enable(module, "foo");
    if (rc) {
        fail();
    }


    feature_state = lys_features_state(module, "foo");
    assert_int_equal(0x1, feature_state);

    rc = lys_features_disable(module, "foo");
    if (rc) {
        fail();
    }


    feature_state = lys_features_state(module, "foo");
    assert_int_equal(0x0, feature_state);
}

static void
test_lys_is_disabled(void **state)
{
    (void) state; /* unused */
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    const struct lys_module *module;
    const struct lys_node *node = NULL;
    int rc;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        fail();
    }

    node = lys_is_disabled(module->data->child, 2);
    if (!node) {
        fail();
    }

    assert_string_equal("/a:top", node->name);

    rc = lys_features_enable(module, "bar");
    if (rc) {
        fail();
    }

    node = lys_is_disabled(module->data->child, 2);
    if (node) {
        fail();
    }
}

static void
test_lys_getnext2(const struct lys_module *module)
{
    const struct lys_node *node;
    const struct lys_node *node_parent;
    const struct lys_node *node_child;

    assert_string_equal("top", module->data->name);

    node_parent = module->data;
    node_child = module->data->child;

    assert_string_equal("bar-sub", node_child->name);

    node = lys_getnext(node_child, node_parent, module, LYS_GETNEXT_WITHINOUT);

    assert_string_equal("bar-sub2", node->name);
}

static void
test_lys_getnext(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;

    module = lys_parse_mem(ctx, lys_module_a, LYS_IN_YIN);
    if (!module) {
        fail();
    }
    test_lys_getnext2(module);

    module = lys_parse_mem(ctx, lys_module_b, LYS_IN_YANG);
    if (!module) {
        fail();
    }
    test_lys_getnext2(module);
}

static void
test_lys_parent(void **state)
{
    (void) state; /* unused */
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    const struct lys_node *node;
    const struct lys_module *module;
    char *str = "node";

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        fail();
    }

    assert_string_equal("top", module->data->name);

    node = module->data;

    lys_set_private(node, str);

    assert_string_equal("node", node->priv);
}

static void
test_lys_set_private(void **state)
{
    (void) state; /* unused */
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    const struct lys_node *node_parent;
    const struct lys_node *node_child;
    const struct lys_module *module;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        fail();
    }

    assert_string_equal("top", module->data->name);

    node_parent = module->data;
    node_child = module->data->child;

    assert_string_equal("bar-sub", node_child->name);

    node_parent = lys_parent(node_child);

    assert_string_equal("top", node_parent->name);
}

static void
test_lys_print_mem_tree(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    char *result = NULL;
    int rc;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        fail();
    }

    rc = lys_print_mem(&result, module, LYS_OUT_TREE, NULL, 0, 0);
    if (rc) {
        fail();
    }

    assert_string_equal(result_tree, result);
    free(result);
}

static void
test_lys_print_mem_yang(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    char *result = NULL;
    int rc;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        fail();
    }

    rc = lys_print_mem(&result, module, LYS_OUT_YANG, NULL, 0, 0);
    if (rc) {
        fail();
    }

    assert_string_equal(result_yang, result);
    free(result);
}

static void
test_lys_print_mem_yin(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    char *result = NULL;
    int rc;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        fail();
    }

    rc = lys_print_mem(&result, module, LYS_OUT_YIN, NULL, 0, 0);
    if (rc) {
        fail();
    }

    assert_string_equal(result_yin, result);
    free(result);
}

static void
test_lys_print_mem_info(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    char *target = "feature/foo";
    char *result = NULL;
    int rc;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        fail();
    }

    rc = lys_print_mem(&result, module, LYS_OUT_INFO, target, 0, 0);
    if (rc) {
        fail();
    }

    assert_string_equal(result_info, result);
    free(result);
}

static void
test_lys_print_mem_jsons(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    const char *target = "grouping/gg";
    char *result = NULL;
    int rc;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        fail();
    }

    rc = lys_print_mem(&result, module, LYS_OUT_JSON, NULL, 0, 0);
    if (rc) {
        fail();
    }

    assert_string_equal(result_jsons, result);
    free(result);

    rc = lys_print_mem(&result, module, LYS_OUT_JSON, target, 0, 0);
    if (rc) {
        fail();
    }

    assert_string_equal(result_jsons_grouping, result);
    free(result);
}

static void
test_lys_print_fd_tree(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct stat sb;
    char file_name[20];
    char *result;
    int rc;
    int fd = -1;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        goto error;
    }

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, TMP_TEMPLATE, sizeof(file_name));

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }

    rc = lys_print_fd(fd, module, LYS_OUT_TREE, NULL, 0, 0);
    if (rc) {
        goto error;
    }

    if (fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_tree, result);

    close(fd);
    unlink(file_name);

    return;
error:
    if (fd > 0) {
        close(fd);
        unlink(file_name);
    }
    fail();
}

static void
test_lys_print_fd_yang(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct stat sb;
    char file_name[20];
    char *result;
    int rc;
    int fd = -1;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        goto error;
    }

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, TMP_TEMPLATE, sizeof(file_name));

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }

    rc = lys_print_fd(fd, module, LYS_OUT_YANG, NULL, 0, 0);
    if (rc) {
        goto error;
    }

    if (fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_yang, result);

    close(fd);
    unlink(file_name);

    return;
error:
    if (fd > 0) {
        close(fd);
        unlink(file_name);
    }
    fail();
}

static void
test_lys_print_fd_yin(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct stat sb;
    char file_name[20];
    char *result;
    int rc;
    int fd = -1;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        goto error;
    }

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, TMP_TEMPLATE, sizeof(file_name));

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }

    rc = lys_print_fd(fd, module, LYS_OUT_YIN, NULL, 0, 0);
    if (rc) {
        goto error;
    }

    if (fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_yin, result);

    close(fd);
    unlink(file_name);

    return;
error:
    if (fd > 0) {
        close(fd);
        unlink(file_name);
    }
    fail();
}

static void
test_lys_print_fd_info(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct stat sb;
    char *target = "feature/foo";
    char file_name[20];
    char *result;
    int rc;
    int fd = -1;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        goto error;
    }

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, TMP_TEMPLATE, sizeof(file_name));

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }

    rc = lys_print_fd(fd, module, LYS_OUT_INFO, target, 0, 0);
    if (rc) {
        goto error;
    }

    if (fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_info, result);

    close(fd);
    unlink(file_name);

    return;
error:
    if (fd > 0) {
        close(fd);
        unlink(file_name);
    }
    fail();
}

static void
test_lys_print_fd_jsons(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct stat sb;
    char *target = "grouping/gg";
    char file_name1[20];
    char file_name2[20];
    char *result;
    int rc;
    int fd1 = -1, fd2 = -1;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        goto error;
    }

    memset(file_name1, 0, sizeof(file_name1));
    memset(file_name2, 0, sizeof(file_name2));
    strncpy(file_name1, TMP_TEMPLATE, sizeof(file_name1));
    strncpy(file_name2, TMP_TEMPLATE, sizeof(file_name2));

    fd1 = mkstemp(file_name1);
    fd2 = mkstemp(file_name2);
    if (fd1 < 1 || fd2 < 1) {
        goto error;
    }

    /* module */
    rc = lys_print_fd(fd1, module, LYS_OUT_JSON, NULL, 0, 0);
    if (rc) {
        goto error;
    }

    if (fstat(fd1, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd1, 0);
    assert_string_equal(result_jsons, result);

    /* grouping */
    rc = lys_print_fd(fd2, module, LYS_OUT_JSON, target, 0, 0);
    if (rc) {
        goto error;
    }

    if (fstat(fd2, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd2, 0);
    assert_string_equal(result_jsons_grouping, result);

    close(fd1);
    close(fd2);
    unlink(file_name1);
    unlink(file_name2);

    return;
error:
    if (fd1 > 0) {
        close(fd1);
        unlink(file_name1);
    }
    if (fd2 > 0) {
        close(fd2);
        unlink(file_name2);
    }
    fail();
}

static void
test_lys_print_file_tree(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct stat sb;
    char file_name[20];
    char *result;
    FILE *f = NULL;
    int rc;
    int fd = -1;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        goto error;
    }

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, TMP_TEMPLATE, sizeof(file_name));

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }
    close(fd);

    f = (fopen(file_name,"r+"));
    if (f == NULL) {
        goto error;
    }

    rc = lys_print_file(f, module, LYS_OUT_TREE, NULL, 0, 0);
    if (rc) {
        goto error;
    }

    fclose(f);

    fd = open(file_name, O_RDONLY);
    if (fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_tree, result);

    close(fd);
    unlink(file_name);

    return;
error:
    if (f)
        fclose(f);
    if (fd > 0) {
        unlink(file_name);
        close(fd);
    }
    fail();
}

static void
test_lys_print_file_yin(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct stat sb;
    char file_name[20];
    char *result;
    FILE *f = NULL;
    int rc;
    int fd = -1;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        goto error;
    }

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, TMP_TEMPLATE, sizeof(file_name));

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }
    close(fd);

    f = (fopen(file_name,"r+"));
    if (f == NULL) {
        goto error;
    }

    rc = lys_print_file(f, module, LYS_OUT_YIN, NULL, 0, 0);
    if (rc) {
        goto error;
    }

    fclose(f);

    fd = open(file_name, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_yin, result);

    close(fd);
    unlink(file_name);

    return;
error:
    if (f)
        fclose(f);
    if (fd > 0) {
        unlink(file_name);
        close(fd);
    }
    fail();
}

static void
test_lys_print_file_yang(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct stat sb;
    char file_name[20];
    char *result;
    FILE *f = NULL;
    int rc;
    int fd = -1;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        goto error;
    }

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, TMP_TEMPLATE, sizeof(file_name));

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }
    close(fd);

    f = (fopen(file_name,"r+"));
    if (f == NULL) {
        goto error;
    }

    rc = lys_print_file(f, module, LYS_OUT_YANG, NULL, 0, 0);
    if (rc) {
        goto error;
    }

    fclose(f);

    fd = open(file_name, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_yang, result);

    close(fd);
    unlink(file_name);

    return;
error:
    if (f)
        fclose(f);
    if (fd > 0) {
        unlink(file_name);
        close(fd);
    }
    fail();
}

static void
test_lys_print_file_info(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct stat sb;
    char *target = "feature/foo";
    char file_name[20];
    char *result;
    FILE *f = NULL;
    int rc;
    int fd = -1;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        goto error;
    }

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, TMP_TEMPLATE, sizeof(file_name));

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }
    close(fd);

    f = (fopen(file_name,"r+"));
    if (f == NULL) {
        goto error;
    }

    rc = lys_print_file(f, module, LYS_OUT_INFO, target, 0, 0);
    if (rc) {
        goto error;
    }

    fclose(f);

    fd = open(file_name, O_RDONLY);
    if (fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_info, result);

    close(fd);
    unlink(file_name);

    return;
error:
    if (f)
        fclose(f);
    if (fd > 0) {
        unlink(file_name);
        close(fd);
    }
    fail();
}

static void
test_lys_print_file_jsons(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct stat sb;
    char *target = "grouping/gg";
    char file_name1[20];
    char file_name2[20];
    char *result;
    FILE *f1 = NULL, *f2 = NULL;
    int rc;
    int fd1 = -1, fd2 = -1;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        goto error;
    }

    memset(file_name1, 0, sizeof(file_name1));
    memset(file_name2, 0, sizeof(file_name2));
    strncpy(file_name1, TMP_TEMPLATE, sizeof(file_name1));
    strncpy(file_name2, TMP_TEMPLATE, sizeof(file_name2));

    fd1 = mkstemp(file_name1);
    fd2 = mkstemp(file_name2);
    if (fd1 < 1 || fd2 < 1) {
        goto error;
    }
    close(fd1);
    close(fd2);

    f1 = (fopen(file_name1,"r+"));
    f2 = (fopen(file_name2,"r+"));
    if (!f1 || !f2) {
        goto error;
    }

    /* module */
    rc = lys_print_file(f1, module, LYS_OUT_JSON, NULL, 0, 0);
    if (rc) {
        goto error;
    }

    fclose(f1); f1 = NULL;

    fd1 = open(file_name1, O_RDONLY);
    if (fstat(fd1, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd1, 0);
    assert_string_equal(result_jsons, result);

    /* grouping */
    rc = lys_print_file(f2, module, LYS_OUT_JSON, target, 0, 0);
    if (rc) {
        goto error;
    }

    fclose(f2); f2 = NULL;

    fd2 = open(file_name2, O_RDONLY);
    if (fstat(fd2, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd2, 0);
    assert_string_equal(result_jsons_grouping, result);

    close(fd1);
    close(fd2);
    unlink(file_name1);
    unlink(file_name2);

    return;
error:
    if (f1)
        fclose(f1);
    if (fd1 > 0) {
        unlink(file_name1);
        close(fd1);
    }
    if (f2)
        fclose(f2);
    if (fd2 > 0) {
        unlink(file_name2);
        close(fd2);
    }
    fail();
}

static void
test_lys_find_path(void **state)
{
    (void) state; /* unused */
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    const struct lys_module *module;
    struct ly_set *set;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    assert_ptr_not_equal(module, NULL);

    set = lys_find_path(module, NULL, "/x/*");
    assert_ptr_not_equal(set, NULL);
    assert_int_equal(set->number, 5);
    ly_set_free(set);

    set = lys_find_path(module, NULL, "/x//*");
    assert_ptr_not_equal(set, NULL);
    assert_int_equal(set->number, 6);
    ly_set_free(set);

    set = lys_find_path(module, NULL, "/x//.");
    assert_ptr_not_equal(set, NULL);
    assert_int_equal(set->number, 7);
    ly_set_free(set);
}

static void
test_lys_xpath_atomize(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    struct ly_set *set;
    const char *schema =
    "module a {"
        "namespace \"urn:a\";"
        "prefix \"a\";"
        "grouping g {"
            "list b {"
                "key \"name type\";"
                "leaf name {"
                    "type leafref {"
                        "path \"/a/name\";"
                    "}"
                "}"
                "leaf type {"
                    "type leafref {"
                        "path \"/a[name=current()/../name]/type\";"
                    "}"
                "}"
            "}"
        "}"
        "list a {"
            "key \"name type\";"
            "leaf name {"
                "type string;"
            "}"
            "leaf type {"
                "type string;"
            "}"
        "}"
        "uses g;"
    "}";

    module = lys_parse_mem(ctx, schema, LYS_IN_YANG);
    assert_non_null(module);

    /* should not crash */
    set = lys_xpath_atomize(module->data->child->child->next, LYXP_NODE_ELEM, "/a[name=current()/../name]/type", 0);
    ly_set_free(set);
}

static void
test_lys_path(void **state)
{
    (void) state; /* unused */
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    const struct lys_node *node;
    const struct lys_module *module;
    char *path;
    struct ly_set *set;
    const char *template;

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    assert_ptr_not_equal(module, NULL);

    template = "/a:x/bar-gggg";
    set = lys_find_path(module, NULL, template);
    assert_ptr_not_equal(set, NULL);
    node = set->set.s[0];
    ly_set_free(set);
    path = lys_path(node, 1);
    assert_string_equal(template, path);
    free(path);

    template = "/a:x/a:bar-gggg";
    set = lys_find_path(module, NULL, template);
    assert_ptr_not_equal(set, NULL);
    node = set->set.s[0];
    ly_set_free(set);
    path = lys_path(node, 0);
    assert_string_equal(template, path);
    free(path);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_lys_parse_mem),
        cmocka_unit_test(test_lys_parse_fd),
        cmocka_unit_test(test_lys_parse_path),
        cmocka_unit_test_setup_teardown(test_lys_features_list, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_features_enable, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_features_disable, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_features_state, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_is_disabled, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_getnext, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_parent, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_set_private, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_print_mem_tree, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_print_mem_yang, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_print_mem_yin, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_print_mem_info, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_print_mem_jsons, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_print_fd_tree, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_print_fd_yang, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_print_fd_yin, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_print_fd_info, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_print_fd_jsons, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_print_file_tree, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_print_file_yin, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_print_file_yang, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_print_file_info, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_print_file_jsons, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_find_path, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_xpath_atomize, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_path, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
