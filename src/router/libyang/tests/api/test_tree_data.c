/*
 * @file test_tree_data.c
 * @author: Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief unit tests for functions from tree_data.h header
 *
 * Copyright (C) 2016 Deutsche Telekom AG.
 *
 * Author: Mislav Novakovic <mislav.novakovic@sartura.hr>
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
#include "../../src/tree_data.h"
#include "../../src/tree_schema.h"

#define TMP_TEMPLATE "/tmp/libyang-XXXXXX"

struct ly_ctx *ctx = NULL;
struct lyd_node *root = NULL;

const char *a_data_xml = "\
<x xmlns=\"urn:a\">\n\
  <bubba>test</bubba>\n\
  </x>\n";

const char *lys_module_a = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>           \
<module name=\"a\"                                    \
        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"   \
        xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\"\
        xmlns:a=\"urn:a\">                            \
  <namespace uri=\"urn:a\"/>                          \
  <prefix value=\"a_mod\"/>                           \
  <include module=\"asub\"/>                          \
  <include module=\"atop\"/>                          \
  <import module=\"ietf-yang-metadata\">              \
    <prefix value=\"md\"/>                            \
  </import>                                           \
  <feature name=\"foo\"/>                             \
  <grouping name=\"gg\">                              \
    <leaf name=\"bar-gggg\">                          \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </grouping>                                         \
  <md:annotation name=\"test\">                       \
    <type name=\"string\"/>                           \
  </md:annotation>                                    \
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
    <leaf name=\"number32\">                          \
      <type name=\"int32\"/>                          \
    </leaf>                                           \
    <leaf name=\"number64\">                          \
      <type name=\"int64\"/>                          \
    </leaf>                                           \
    <leaf name=\"def-leaf\">                          \
      <type name=\"string\"/>                         \
      <default value=\"def\"/>                        \
    </leaf>                                           \
  </container>                                        \
  <container name=\"z\">                              \
    <leaf name=\"number-z\">                          \
      <type name=\"int64\"/>                          \
    </leaf>                                           \
  </container>                                        \
  <leaf name=\"y\"><type name=\"string\"/></leaf>     \
  <anyxml name=\"any\"/>                              \
  <augment target-node=\"/x\">                        \
    <if-feature name=\"bar\"/>                        \
    <container name=\"bar-y\"/>                       \
  </augment>                                          \
  <rpc name=\"bar-rpc\">                              \
    <if-feature name=\"bar\"/>                        \
  </rpc>                                              \
  <rpc name=\"foo-rpc\">                              \
    <if-feature name=\"foo\"/>                        \
  </rpc>                                              \
  <rpc name=\"rpc1\">                                 \
    <input>                                           \
      <leaf name=\"input-leaf1\">                     \
        <type name=\"string\"/>                       \
      </leaf>                                         \
      <container name=\"x\">                          \
        <leaf name=\"input-leaf2\">                   \
          <type name=\"string\"/>                     \
        </leaf>                                       \
      </container>                                    \
    </input>                                          \
    <output>                                          \
      <leaf name=\"output-leaf1\">                    \
        <type name=\"string\"/>                       \
      </leaf>                                         \
      <leaf name=\"output-leaf2\">                    \
        <type name=\"string\"/>                       \
      </leaf>                                         \
      <container name=\"rpc-container\">              \
        <leaf name=\"output-leaf3\">                  \
          <type name=\"string\"/>                     \
        </leaf>                                       \
      </container>                                    \
    </output>                                         \
  </rpc>                                              \
  <list name=\"l\">                                   \
    <key value=\"key1 key2\"/>                        \
    <leaf name=\"key1\">                              \
      <type name=\"uint8\"/>                          \
    </leaf>                                           \
    <leaf name=\"key2\">                              \
      <type name=\"uint8\"/>                          \
    </leaf>                                           \
    <leaf name=\"value\">                             \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </list>                                             \
</module>                                             \
";

const char *result_xml = "<x xmlns=\"urn:a\"><bubba>test</bubba></x>";

const char *result_xml_format ="\
<x xmlns=\"urn:a\">\n\
  <bubba>test</bubba>\n\
</x>\n\
";

const char *result_json = "\
{\n\
  \"a:x\": {\n\
    \"bubba\": \"test\"\n\
  }\n\
}\n\
";

int
generic_init(char *config_file, const char *module, char *yang_folder)
{
    LYS_INFORMAT yang_format;
    LYD_FORMAT in_format;
    char *config = NULL;
    struct stat sb_config;
    int fd = -1;

    if (!yang_folder) {
        goto error;
    }

    yang_format = LYS_IN_YIN;
    in_format = LYD_XML;

    ctx = ly_ctx_new(yang_folder, 0);
    if (!ctx) {
        goto error;
    }

    if (module && !lys_parse_mem(ctx, module, yang_format)) {
        goto error;
    }

    if (config_file) {
        fd = open(config_file, O_RDONLY);
        if (fd == -1 || fstat(fd, &sb_config) == -1 || !S_ISREG(sb_config.st_mode)) {
            goto error;
        }

        config = mmap(NULL, sb_config.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);
        fd = -1;

        root = lyd_parse_mem(ctx, config, in_format, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        if (!root) {
            goto error;
        }

        /* cleanup */
        munmap(config, sb_config.st_size);
    } else {
        root = NULL;
    }

    return 0;

error:
    if (config) {
        munmap(config, sb_config.st_size);
    }
    if (fd != -1) {
        close(fd);
    }

    return -1;
}

static int
setup_f2(void **state)
{
    *state = ly_ctx_new(NULL, 0);
    if (!*state) {
        return -1;
    }

    return 0;
}

static int
teardown_f2(void **state)
{
    ly_ctx_destroy((struct ly_ctx *)(*state), NULL);
    return 0;
}

static int
setup_f(void **state)
{
    (void) state; /* unused */
    char *config_file = TESTS_DIR"/api/files/a.xml";
    char *yang_folder = TESTS_DIR"/api/files";
    int rc;

    rc = generic_init(config_file, lys_module_a, yang_folder);

    if (rc) {
        return -1;
    }

    return 0;
}

static int
teardown_f(void **state)
{
    (void) state; /* unused */
    if (root)
        lyd_free_withsiblings(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    return 0;
}

static void
test_lyd_parse_mem(void **state)
{
    (void) state; /* unused */
    char *yang_folder = TESTS_DIR"/api/files";
    LYD_FORMAT in_format = LYD_XML;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct ly_ctx *ctx = NULL;
    struct lyd_node *root = NULL;

    ctx = ly_ctx_new(yang_folder, 0);
    if (!ctx) {
        goto error;
    }

    if (!lys_parse_mem(ctx, lys_module_a, yang_format)) {
        goto error;
    }

    root = lyd_parse_mem(ctx, a_data_xml, in_format, LYD_OPT_NOSIBLINGS | LYD_OPT_STRICT);
    if (!root) {
        goto error;
    }

    assert_string_equal("x", root->schema->name);

    if (root)
        lyd_free(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    return;
error:

    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    fail();
}

static void
test_lyd_parse_fd(void **state)
{
    (void) state; /* unused */
    char *yang_folder = TESTS_DIR"/api/files";
    char *config_file = TESTS_DIR"/api/files/a.xml";
    LYD_FORMAT in_format = LYD_XML;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct ly_ctx *ctx = NULL;
    struct lyd_node *root = NULL;
    struct stat sb;
    int fd = -1;

    ctx = ly_ctx_new(yang_folder, 0);
    if (!ctx) {
        goto error;
    }

    if (!lys_parse_mem(ctx, lys_module_a, yang_format)) {
        goto error;
    }

    fd = open(config_file, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    root = lyd_parse_fd(ctx, fd, in_format, LYD_OPT_NOSIBLINGS | LYD_OPT_STRICT);
    if (!root) {
        goto error;
    }

    assert_string_equal("x", root->schema->name);

    if (root)
        lyd_free(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);
    if (fd > 0)
        close(fd);

    return;
error:

    if (ctx)
        ly_ctx_destroy(ctx, NULL);
    if (fd > 0)
        close(fd);
    fail();
}

static void
test_lyd_parse_path(void **state)
{
    (void) state; /* unused */
    char *yang_folder = TESTS_DIR"/api/files";
    char *config_file = TESTS_DIR"/api/files/a.xml";
    LYD_FORMAT in_format = LYD_XML;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct ly_ctx *ctx = NULL;
    struct lyd_node *root = NULL;

    ctx = ly_ctx_new(yang_folder, 0);
    if (!ctx) {
        goto error;
    }

    if (!lys_parse_mem(ctx, lys_module_a, yang_format)) {
        goto error;
    }

    root = lyd_parse_path(ctx, config_file, in_format, LYD_OPT_NOSIBLINGS | LYD_OPT_STRICT);
    if (!root) {
        goto error;
    }

    assert_string_equal("x", root->schema->name);

    if (root)
        lyd_free(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    return;
error:

    if (ctx)
        ly_ctx_destroy(ctx, NULL);
    fail();
}

static void
test_lyd_parse_xml(void **state)
{
    (void) state; /* unused */
    char *yang_folder = TESTS_DIR"/api/files";
    struct lyd_node *node = NULL;
    const struct lys_module *module = NULL;
    struct lyxml_elem *root_xml = NULL;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct ly_ctx *ctx = NULL;

    ctx = ly_ctx_new(yang_folder, 0);
    if (!ctx) {
        goto error;
    }

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        goto error;
    }

    root_xml = lyxml_parse_mem(ctx, a_data_xml, 1);

    node = lyd_parse_xml(ctx, &root_xml, LYD_OPT_NOSIBLINGS | LYD_OPT_DATA);
    if (!node) {
        goto error;
    }

    assert_string_equal("x", node->schema->name);

    if (node)
        lyd_free(node);
    if (root_xml)
        lyxml_free(ctx, root_xml);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    return;
error:

    if (root_xml)
        lyxml_free(ctx, root_xml);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);
    fail();
}

static void
test_lyd_new(void **state)
{
    (void) state; /* unused */
    struct lyd_node *new = NULL;

    new = lyd_new(root, root->child->schema->module, "bar-y");
    if (!new) {
        fail();
    }

    assert_string_equal("bar-y", new->schema->name);
}

static void
test_lyd_new_leaf(void **state)
{
    (void) state; /* unused */
    struct lyd_node *new = NULL;
    struct lyd_node_leaf_list *result;

    new = lyd_new_leaf(root, root->schema->module, "number32", "100");
    if (!new) {
        fail();
    }

    result = (struct lyd_node_leaf_list *) new;
    assert_string_equal("100", result->value_str);
}

static void
test_lyd_change_leaf(void **state)
{
    (void) state; /* unused */
    struct lyd_node_leaf_list *result = NULL;
    int rc;

    result = (struct lyd_node_leaf_list *) root->child;
    assert_string_equal("test", result->value_str);

    rc = lyd_change_leaf(result, "new_test");
    if (rc) {
    fail();
    }

    assert_string_equal("new_test", result->value_str);

}

static void
test_lyd_output_new_leaf(void **state)
{
    (void) state; /* unused */
    struct lyd_node *node = NULL;
    struct lyd_node_leaf_list *result = NULL;

    node = lyd_new_output(NULL, root->schema->module, "rpc1");
    if (!node) {
        fail();
    }

    result = (struct lyd_node_leaf_list *)lyd_new_output_leaf(node, NULL, "output-leaf1", "test");
    if (!result) {
        fail();
    }

    assert_string_equal("test", result->value_str);

    lyd_free(node);
}

static void
test_lyd_new_path(void **state)
{
    (void) state; /* unused */
    const struct lys_module *mod;
    struct lyd_node *node, *root;
    char *str;
    struct lyxml_elem *xml;

    mod = ly_ctx_get_module(ctx, "a", NULL, 1);
    assert_non_null(mod);
    lys_features_enable(mod, "bar");

    root = lyd_new_path(NULL, ctx, "/a:x/bar-gggg", "a", 0, 0);
    assert_non_null(root);
    assert_string_equal(root->schema->name, "x");
    assert_string_equal(root->child->schema->name, "bar-gggg");

    /* create a default node first, then implicitly rewrite it, then fail to rewrite it again */
    node = lyd_new_path(root, NULL, "def-leaf", "def", 0, LYD_PATH_OPT_DFLT);
    assert_non_null(node);
    assert_string_equal(node->schema->name, "def-leaf");
    assert_int_equal(node->dflt, 1);

    node = lyd_new_path(root, NULL, "def-leaf", "def", 0, 0);
    assert_non_null(node);
    assert_int_equal(node->dflt, 0);

    node = lyd_new_path(root, NULL, "def-leaf", "def", 0, 0);
    assert_null(node);
    assert_int_equal(ly_errno, LY_EVALID);
    ly_errno = 0;

    node = lyd_new_path(root, NULL, "bubba", "b", 0, 0);
    assert_non_null(node);
    assert_string_equal(node->schema->name, "bubba");

    node = lyd_new_path(root, NULL, "/a:x/number32", "3", 0, 0);
    assert_non_null(node);
    assert_string_equal(node->schema->name, "number32");

    node = lyd_new_path(root, NULL, "a:number64", "64", 0, 0);
    assert_non_null(node);
    assert_string_equal(node->schema->name, "number64");

    node = lyd_new_path(root, NULL, "/a:l[key1='111'][key2='222']", NULL, 0, 0);
    assert_non_null(node);
    assert_string_equal(node->schema->name, "l");
    assert_ptr_not_equal(root->prev, root);

    lyd_free_withsiblings(root);

    root = lyd_new_path(NULL, ctx, "/a:l[key1='1'][key2='2']", NULL, 0, 0);
    assert_non_null(root);
    assert_string_equal(root->schema->name, "l");
    assert_string_equal(root->child->schema->name, "key1");
    assert_string_equal(root->child->next->schema->name, "key2");

    node = lyd_new_path(root, NULL, "/a:l[key1='11'][key2='22']/value", "val", 0, 0);
    assert_non_null(node);
    assert_ptr_not_equal(root->prev, root);
    assert_string_equal(node->schema->name, "l");
    assert_string_equal(node->child->schema->name, "key1");
    assert_string_equal(node->child->next->schema->name, "key2");
    assert_string_equal(node->child->next->next->schema->name, "value");

    node = lyd_new_path(root, NULL, "/a:l[key1='1'][key2='2']/value", "val2", 0, 0);
    assert_non_null(node);
    assert_string_equal(node->schema->name, "value");

    lyd_free_withsiblings(root);

    root = lyd_new_path(NULL, ctx, "/a:any", "test <&>\"", LYD_ANYDATA_CONSTSTRING, 0);
    assert_non_null(root);
    str = NULL;
    lyd_print_mem(&str, root, LYD_XML, 0);
    assert_non_null(root);
    assert_string_equal(str, "<any xmlns=\"urn:a\">test &lt;&amp;&gt;\"</any>");
    free(str);
    lyd_free(root);

    xml = lyxml_parse_mem(ctx, "<test>&lt;</test>", 0);
    assert_non_null(xml);
    root = lyd_new_path(NULL, ctx, "/a:any", xml, LYD_ANYDATA_XML, 0);
    xml = NULL;
    assert_non_null(root);
    lyd_print_mem(&str, root, LYD_XML, 0);
    assert_non_null(root);
    assert_string_equal(str, "<any xmlns=\"urn:a\"><test>&lt;</test></any>");
    free(str);

    node = root;
    root = lyd_new_path(NULL, ctx, "/a:any", node, LYD_ANYDATA_DATATREE, 0);
    assert_non_null(root);
    str = NULL;
    lyd_print_mem(&str, root, LYD_XML, 0);
    assert_non_null(root);
    assert_string_equal(str, "<any xmlns=\"urn:a\"><any xmlns=\"urn:a\"><test>&lt;</test></any></any>");
    free(str);
    lyd_free(root);

    root = lyd_new_path(NULL, ctx, "/a:rpc1/x/input-leaf2", "dudu", 0, 0);
    assert_non_null(root);
    assert_string_equal(root->schema->name, "rpc1");
    assert_string_equal(root->child->schema->name, "x");
    assert_string_equal(root->child->child->schema->name, "input-leaf2");

    node = lyd_new_path(root, NULL, "/a:rpc1/input-leaf1", "bubu", 0, 0);
    assert_non_null(node);
    assert_string_equal(node->schema->name, "input-leaf1");
    assert_string_equal(root->child->schema->name, "input-leaf1");
    assert_string_equal(root->child->next->schema->name, "x");

    lyd_free(root);

    root = lyd_new_path(NULL, ctx, "/a:rpc1/rpc-container", NULL, 0, LYD_PATH_OPT_OUTPUT);
    assert_non_null(root);
    assert_string_equal(root->schema->name, "rpc1");

    node = lyd_new_path(root->child, NULL, "output-leaf3", "cc", 0, LYD_PATH_OPT_OUTPUT);
    assert_non_null(node);
    assert_string_equal(node->schema->name, "output-leaf3");

    node = lyd_new_path(root, NULL, "/a:rpc1/output-leaf1", "aa", 0, LYD_PATH_OPT_OUTPUT);
    assert_non_null(node);
    assert_string_equal(node->schema->name, "output-leaf1");
    assert_ptr_equal(node, root->child);

    node = lyd_new_path(root, NULL, "/a:rpc1/output-leaf2", "bb", 0, LYD_PATH_OPT_OUTPUT);
    assert_non_null(node);
    assert_string_equal(node->schema->name, "output-leaf2");
    assert_string_equal(node->prev->schema->name, "output-leaf1");
    assert_string_equal(node->next->schema->name, "rpc-container");

    lyd_free(root);

    root = lyd_new_path(NULL, ctx, "/a:l", NULL, 0, 0);
    assert_non_null(root);
    assert_string_equal(root->schema->name, "l");

    node = lyd_new_path(root, NULL, "/a:l[key1='11'][key2='22']/value", "val", 0, 0);
    assert_non_null(node);
    assert_ptr_not_equal(root->prev, root);
    assert_string_equal(node->schema->name, "l");
    assert_string_equal(node->child->schema->name, "key1");

    node = lyd_new_path(root, NULL, "/a:l[1]/key1", "1", 0, 0);
    assert_non_null(node);
    assert_string_equal(node->schema->name, "key1");

    node = lyd_new_path(root, NULL, "/a:l[1]/key2", "2", 0, 0);
    assert_non_null(node);
    assert_string_equal(node->schema->name, "key2");

    node = lyd_new_path(root, NULL, "/a:l[1]/value", "vall", 0, 0);
    assert_non_null(node);
    assert_string_equal(node->schema->name, "value");

    node = lyd_new_path(root, NULL, "/a:l[2]/value", "vall", 0, 0);
    assert_null(node);

    node = lyd_new_path(root, NULL, "/a:l[3]/value", "vall", 0, 0);
    assert_non_null(node);
    assert_string_equal(node->schema->name, "l");

    node = lyd_new_path(root, NULL, "/a:l[0]", NULL, 0, 0);
    assert_null(node);

    lyd_free_withsiblings(root);
}

static void
test_lyd_dup(void **state)
{
    (void) state; /* unused */
    struct lyd_node *copy = NULL;

    copy = lyd_dup(root->child, 0);
    if (!copy) {
        fail();
    }

    assert_string_equal("bubba", copy->schema->name);

    lyd_free(copy);
}

static void
test_lyd_insert(void **state)
{
    (void) state; /* unused */
    struct lyd_node *new = NULL;
    struct lyd_node *node = root;
    struct lyd_node_leaf_list *result;
    int rc;

    result = (struct lyd_node_leaf_list *) root->child;
    assert_string_equal("test", result->value_str);

    new = lyd_new_leaf(root, node->schema->module, "number32", "100");
    if (!new) {
        fail();
    }

    rc = lyd_insert(node, new);
    if (rc) {
        fail();
    }

    result = (struct lyd_node_leaf_list *) root->child->prev;
    assert_string_equal("100", result->value_str);

    /* test inserting an empty container that is already present */
    node = NULL;
    assert_int_equal(lyd_validate(&node, LYD_OPT_CONFIG, ctx), 0);
    assert_ptr_not_equal(node, NULL);
    assert_string_equal(node->schema->name, "top");
    assert_ptr_not_equal(node->child, NULL);
    assert_string_equal(node->child->schema->name, "bar-sub2");
    assert_int_equal(node->child->dflt, 1);

    new = lyd_new_path(NULL, ctx, "/a:top/bar-sub2", NULL, 0, 0);
    assert_ptr_not_equal(new, NULL);
    assert_string_equal(new->schema->name, "top");
    assert_int_equal(lyd_insert(node, new->child), 0);
    assert_int_not_equal(node->child->dflt, 1);

    lyd_free_withsiblings(node);
    lyd_free_withsiblings(new);
}

static void
test_lyd_insert_sibling(void **state)
{
    (void) state; /* unused */
    struct lyd_node *new = NULL, *last;

    new = lyd_new_leaf(NULL, root->schema->module, "y", "test");
    assert_ptr_not_equal(new, NULL);
    last = root->prev;

    assert_int_equal(lyd_insert_sibling(&root, new), 0);
    assert_ptr_not_equal(last, root->prev);
    assert_string_equal(root->prev->schema->name, "y");
    assert_string_equal(((struct lyd_node_leaf_list *)root->prev)->value_str, "test");
}

static void
test_lyd_insert_before(void **state)
{
    (void) state; /* unused */
    struct lyd_node *new = NULL;
    struct lyd_node_leaf_list *result;
    int rc;

    result = (struct lyd_node_leaf_list *) root->child;
    assert_string_equal("test", result->value_str);

    new = lyd_new_leaf(root, root->child->schema->module, "number32", "1");
    if (!new) {
        fail();
    }

    rc = lyd_insert(root, new);
    if (rc) {
        fail();
    }

    new = lyd_new_leaf(root, root->child->schema->module, "number64", "1000");
    if (!new) {
        fail();
    }

    rc = lyd_insert_before(root->child->next, new);
    if (rc) {
        fail();
    }

    result = (struct lyd_node_leaf_list *) root->child->prev;
    assert_string_equal("1", result->value_str);
}

static void
test_lyd_insert_after(void **state)
{
    (void) state; /* unused */
    struct lyd_node *new = NULL, *node;
    struct lyd_node_leaf_list *result;
    int rc;

    result = (struct lyd_node_leaf_list *) root->child;
    assert_string_equal("test", result->value_str);

    new = lyd_new_leaf(root, root->child->schema->module, "number32", "1");
    if (!new) {
        fail();
    }

    rc = lyd_insert(root, new);
    if (rc) {
        fail();
    }

    new = lyd_new_leaf(root, root->child->schema->module, "number64", "1000");
    if (!new) {
        fail();
    }

    rc = lyd_insert_after(root->child->next, new);
    if (rc) {
        fail();
    }

    result = (struct lyd_node_leaf_list *) root->child->next->next;
    assert_string_equal("1000", result->value_str);

    /* test user-ordered lists */
    lyd_free_withsiblings(root->child);
    rc = lyd_validate(&root, LYD_OPT_CONFIG, NULL);
    assert_int_equal(rc, 0);

    new = lyd_new(NULL, lyd_node_module(root), "l");
    assert_non_null(new);
    node = lyd_new_leaf(new, NULL, "key1", "1");
    assert_non_null(node);
    node = lyd_new_leaf(new, NULL, "key2", "1");
    assert_non_null(node);
    node = lyd_new_leaf(new, NULL, "value", "one");
    assert_non_null(node);
    rc = lyd_insert_after(root->prev, new);
    assert_int_equal(rc, 0);

    new = lyd_new(NULL, lyd_node_module(root), "l");
    assert_non_null(new);
    node = lyd_new_leaf(new, NULL, "key1", "2");
    assert_non_null(node);
    node = lyd_new_leaf(new, NULL, "key2", "2");
    assert_non_null(node);
    node = lyd_new_leaf(new, NULL, "value", "two");
    assert_non_null(node);
    rc = lyd_insert_after(root->prev, new);
    assert_int_equal(rc, 0);

    rc = lyd_insert_after(root->next, root->next->next);
    assert_int_equal(rc, 0);
}

static void
test_lyd_schema_sort(void **state)
{
    (void) state; /* unused */
    const struct lys_module *module;
    struct lyd_node *root, *node, *node2;

    module = ly_ctx_get_module(ctx, "a", NULL, 0);
    assert_non_null(module);
    lys_features_enable(module, "bar");

    root = lyd_new(NULL, module, "l");
    assert_non_null(root);
    node = lyd_new_leaf(root, NULL, "key1", "1");
    assert_non_null(node);
    node = lyd_new_leaf(root, NULL, "key2", "2");
    assert_non_null(node);

    node = lyd_new(NULL, module, "x");
    assert_non_null(node);
    assert_int_equal(lyd_insert_after(root, node), 0);

    node2 = lyd_new_leaf(node, NULL, "bubba", "a");
    assert_non_null(node2);
    node2 = lyd_new_leaf(node, NULL, "bar-gggg", "b");
    assert_non_null(node2);
    node2 = lyd_new_leaf(node, NULL, "number64", "64");
    assert_non_null(node2);
    node2 = lyd_new_leaf(node, NULL, "number32", "32");
    assert_non_null(node2);

    assert_int_equal(lyd_schema_sort(root, 1), 0);

    root = node;
    assert_string_equal(root->schema->name, "x");
    assert_string_equal(root->next->schema->name, "l");

    assert_string_equal(root->child->schema->name, "bar-gggg");
    assert_string_equal(root->child->next->schema->name, "bubba");
    assert_string_equal(root->child->next->next->schema->name, "number32");
    assert_string_equal(root->child->next->next->next->schema->name, "number64");

    lyd_free_withsiblings(root);

    root = lyd_new_output(NULL, module, "rpc1");
    assert_non_null(root);
    node = lyd_new_output(root, NULL, "rpc-container");
    assert_non_null(node);
    node2 = lyd_new_output_leaf(node, NULL, "output-leaf3", "ff");
    assert_non_null(node2);

    node = lyd_new_output_leaf(root, NULL, "output-leaf1", "gg");
    assert_non_null(node);

    node = lyd_new_output_leaf(root, NULL, "output-leaf2", "hh");
    assert_non_null(node);

    assert_int_equal(lyd_schema_sort(root, 1), 0);

    node = root->child;
    assert_string_equal(node->schema->name, "output-leaf1");
    assert_string_equal(node->next->schema->name, "output-leaf2");
    assert_string_equal(node->next->next->schema->name, "rpc-container");

    lyd_free_withsiblings(root);
}

static void
test_lyd_find_path(void **state)
{
    (void) state; /* unused */
    struct ly_set *set = NULL;
    struct lyd_node_leaf_list *result;

    set = lyd_find_path(root->child, "/a:x/bubba");

    struct lyd_node *node = *set->set.d;
    result = (struct lyd_node_leaf_list *) node;
    assert_string_equal("test", result->value_str);

    ly_set_free(set);
}

static void
test_lyd_find_instance(void **state)
{
    (void) state; /* unused */
    struct ly_set *set = NULL;
    struct lyd_node_leaf_list *result;

    set = lyd_find_instance(root->child, root->child->schema);
    if (!set) {
        fail();
    }

    struct lyd_node *node = *set->set.d;
    result = (struct lyd_node_leaf_list *) node;
    assert_string_equal("test", result->value_str);

    ly_set_free(set);
}

static void
test_lyd_validate(void **state)
{
    (void) state; /* unused */
    struct lyd_node *new = NULL;
    struct lyd_node *node = root;
    int rc;

    rc = lyd_validate(&root, LYD_OPT_CONFIG, NULL);
    if (rc) {
        fail();
    }

    if (root->child->next->next) {
        fail();
    }

    new = lyd_new_leaf(root, node->schema->module, "number32", "1");
    if (!new) {
        fail();
    }

    rc = lyd_insert(root, new);
    if (rc) {
        fail();
    }

    rc = lyd_validate(&root, LYD_OPT_CONFIG, NULL);
    if (rc) {
        fail();
    }
}

static void
test_lyd_unlink(void **state)
{
    (void) state; /* unused */
    struct lyd_node *new = NULL;
    struct lyd_node *node = root->child;
    struct lyd_node_leaf_list *result;
    int rc;

    new = lyd_new_leaf(root, node->schema->module, "number32", "1");
    if (!new) {
        fail();
    }

    rc = lyd_insert(root, new);
    if (rc) {
        fail();
    }

    result = (struct lyd_node_leaf_list *) node->prev;
    assert_string_equal("1", result->value_str);

    rc = lyd_unlink(node->prev);
    if (rc) {
        fail();
    }

    assert_string_not_equal("1", ((struct lyd_node_leaf_list *)node->prev)->value_str);

    lyd_free(new);
}

static void
test_lyd_free(void **state)
{
    (void) state; /* unused */
    struct lyd_node *copy = NULL;

    copy = lyd_dup(root->child, 0);
    if (!copy) {
        fail();
    }

    lyd_free(copy);
}

static void
test_lyd_free_withsiblings(void **state)
{
    (void) state; /* unused */
    struct lyd_node *copy = NULL;
    struct lyd_node *new = NULL;
    struct lyd_node *node = root->child;
    int rc;

    copy = lyd_dup(root->child, 0);
    if (!copy) {
        fail();
    }

    new = lyd_new_leaf(root, node->schema->module, "number32", "1");
    if (!new) {
        fail();
    }

    rc = lyd_insert(copy, new);
    if (!rc) {
        fail();
    }

    lyd_free_withsiblings(copy);
}

static void
test_lyd_insert_attr(void **state)
{
    (void) state; /* unused */
    struct lyd_attr *attr = NULL;
    struct lyd_node *node = root->child;

    attr = lyd_insert_attr(node, NULL, "test", "test");
    if (!attr) {
        fail();
    }

    assert_string_equal("test", node->attr->name);
}

static void
test_lyd_free_attr(void **state)
{
    (void) state; /* unused */
    struct lyd_attr *attr = NULL;
    struct lyd_node *node = root->child;

    attr = lyd_insert_attr(node, NULL, "test", "test");

    if (!node->attr) {
        fail();
    }

    lyd_free_attr(ctx, node, attr, 0);

    if (node->attr) {
        fail();
    }

}

static void
test_lyd_print_mem_xml(void **state)
{
    (void) state; /* unused */
    char *result = NULL;
    int rc;

    rc = lyd_print_mem(&result, root, LYD_XML, 0);
    if (rc) {
        fail();
    }

    assert_string_equal(result_xml, result);
    free(result);
}

static void
test_lyd_print_mem_xml_format(void **state)
{
    (void) state; /* unused */
    char *result = NULL;
    int rc;

    rc = lyd_print_mem(&result, root, LYD_XML, LYP_FORMAT);
    if (rc) {
        fail();
    }

    assert_string_equal(result_xml_format, result);
    free(result);
}

static void
test_lyd_print_mem_json(void **state)
{
    (void) state; /* unused */
    char *result = NULL;
    int rc;

    rc = lyd_print_mem(&result, root, LYD_JSON, LYP_FORMAT);
    if (rc) {
        fail();
    }

    assert_string_equal(result_json, result);
    free(result);
}

static void
test_lyd_print_fd_xml(void **state)
{
    (void) state; /* unused */
    char *result = NULL;
    struct stat sb;
    char file_name[20];
    int rc;
    int fd;

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, TMP_TEMPLATE, sizeof(file_name));

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }

    rc = lyd_print_fd(fd, root, LYD_XML, 0);
    if (rc) {
        goto error;
    }

    if (fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_xml, result);

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
test_lyd_print_fd_xml_format(void **state)
{
    (void) state; /* unused */
    char *result = NULL;
    struct stat sb;
    char file_name[20];
    int rc;
    int fd;

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, TMP_TEMPLATE, sizeof(file_name));

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }

    rc = lyd_print_fd(fd, root, LYD_XML, LYP_FORMAT);
    if (rc) {
        goto error;
    }

    if (fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_xml_format, result);

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
test_lyd_print_fd_json(void **state)
{
    (void) state; /* unused */
    char *result = NULL;
    struct stat sb;
    char file_name[20];
    int rc;
    int fd;

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, TMP_TEMPLATE, sizeof(file_name));

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }

    rc = lyd_print_fd(fd, root, LYD_JSON, LYP_FORMAT);
    if (rc) {
        goto error;
    }

    if (fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_json, result);

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
test_lyd_print_file_xml(void **state)
{
    (void) state; /* unused */
    struct stat sb;
    char file_name[20];
    char *result;
    FILE *f = NULL;
    int rc;
    int fd;

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

    rc = lyd_print_file(f, root, LYD_XML, 0);
    if (rc) {
        goto error;
    }

    fclose(f);

    fd = open(file_name, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_xml, result);

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
test_lyd_print_file_xml_format(void **state)
{
    (void) state; /* unused */
    struct stat sb;
    char file_name[20];
    char *result;
    FILE *f = NULL;
    int rc;
    int fd;

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

    rc = lyd_print_file(f, root, LYD_XML, LYP_FORMAT);
    if (rc) {
        goto error;
    }

    fclose(f);

    fd = open(file_name, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_xml_format, result);

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
test_lyd_print_file_json(void **state)
{
    (void) state; /* unused */
    struct stat sb;
    char file_name[20];
    char *result;
    FILE *f = NULL;
    int rc;
    int fd = -1;

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

    rc = lyd_print_file(f, root, LYD_JSON, LYP_FORMAT);
    if (rc) {
        goto error;
    }

    fclose(f);

    fd = open(file_name, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_json, result);

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

struct buff {
    int len;
    const char *cmp;
};

ssize_t custom_lyd_print_clb(void *arg, const void *buf, size_t count) {
    int rc;
    int len;
    struct buff *pos = arg;

    len = pos->len + count;

    const char *chunk = &pos->cmp[pos->len];

    rc = strncmp(chunk, buf, count);
    if (rc) {
        fail();
    }

    pos->len = len;
    return count;
}

static void
test_lyd_print_clb_xml(void **state)
{
    (void) state; /* unused */
    int rc;

    struct buff *buf = calloc(1, sizeof(struct buff));
    if (!buf) {
        fail();
    }

    buf->len = 0;
    buf->cmp = result_xml;
    void *arg = buf;

    rc = lyd_print_clb(custom_lyd_print_clb, arg, root, LYD_XML, 0);
    if (rc) {
        fail();
        free(buf);
    }

    free(buf);
}

static void
test_lyd_print_clb_xml_format(void **state)
{
    (void) state; /* unused */
    int rc;

    struct buff *buf = calloc(1, sizeof(struct buff));
    if (!buf) {
        fail();
    }

    buf->len = 0;
    buf->cmp = result_xml_format;
    void *arg = buf;

    rc = lyd_print_clb(custom_lyd_print_clb, arg, root, LYD_XML, LYP_FORMAT);
    if (rc) {
        fail();
        free(buf);
    }

    free(buf);
}

static void
test_lyd_print_clb_json(void **state)
{
    (void) state; /* unused */
    int rc;

    struct buff *buf = calloc(1, sizeof(struct buff));
    if (!buf) {
        fail();
    }

    buf->len = 0;
    buf->cmp = result_json;
    void *arg = buf;

    rc = lyd_print_clb(custom_lyd_print_clb, arg, root, LYD_JSON, LYP_FORMAT);
    if (rc) {
        fail();
        free(buf);
    }

    free(buf);
}

static void
test_lyd_path(void **state)
{
    (void) state; /* unused */
    char *str;

    str = lyd_path(root);
    assert_ptr_not_equal(str, NULL);
    assert_string_equal(str, "/a:x");
    free(str);

    str = lyd_path(root->child);
    assert_ptr_not_equal(str, NULL);
    assert_string_equal(str, "/a:x/bubba");
    free(str);
}

static void
test_lyd_leaf_type(void **state)
{
    struct ly_ctx *ctx = (struct ly_ctx *)*state;
    const char *yang = "module x {"
"  namespace urn:x;"
"  prefix x;"
"  yang-version 1.1;"
"  container x {"
"    leaf str { type string; }"
"    leaf e { type enumeration { enum ftp; enum http; } }"
"    leaf u { type union {"
"               type leafref { path \"../e\"; }"
"               type leafref { path \"../str\";}"
"           }            }"
"} }";
    const char *xml1 = "<x xmlns=\"urn:x\"><str>http</str><e>ftp</e><u>http</u></x>";
    const char *xml2 = "<x xmlns=\"urn:x\"><str>http</str><e>ftp</e><u>ftp</u></x>";
    struct lyd_node *data;

    assert_ptr_not_equal(lys_parse_mem(ctx, yang, LYS_IN_YANG), 0);

    data = lyd_parse_mem(ctx, xml1, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(data, NULL);
    assert_int_equal(lyd_leaf_type((struct lyd_node_leaf_list *)data->child->prev)->base, LY_TYPE_STRING);
    lyd_free_withsiblings(data);

    data = lyd_parse_mem(ctx, xml2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(data, NULL);
    assert_int_equal(lyd_leaf_type((struct lyd_node_leaf_list *)data->child->prev)->base, LY_TYPE_ENUM);
    lyd_free_withsiblings(data);

    data = lyd_parse_mem(ctx, xml2, LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(data, NULL);
    lyd_change_leaf((struct lyd_node_leaf_list *)data->child->prev, "ssh");
    assert_int_equal(lyd_leaf_type((struct lyd_node_leaf_list *)data->child->prev), NULL);
    lyd_free_withsiblings(data);
}

static void
test_lyd_validation_dflt_empty_containers(void **state)
{
    (void) state; /* unused */
    struct lyd_node *new = NULL;
    struct lyd_node *old = root;
    struct lyd_node *node = root;

    new = lyd_new(NULL, old->schema->module, "z");
    lyd_insert_before(old, new);
    node = new;

    assert_int_equal(lyd_validate(&node, LYD_OPT_CONFIG, ctx), 0);
    assert_ptr_equal(node->dflt, 1);
}

void
test_lyd_diff(void **state)
{
    (void) state;
    struct lyd_node *first = root;
    struct lyd_node *second = root;
    struct lyd_difflist *difflist = NULL;

    /* Check whether the trees are equal */
    if (lyd_diff(first, second, LYD_DIFFOPT_NOSIBLINGS)) {
        fail();
    }

    second = lyd_new(NULL, root->schema->module, "z");

    if (!second) {
        fail();
    }

    difflist = lyd_diff(first, second, LYD_DIFFOPT_NOSIBLINGS);

    lyd_free_withsiblings(second);

    /* Checking whether the trees are different */
    if (difflist) {
        fail();
    }

    lyd_free_diff(difflist);
}

void
test_lyd_free_diff(void **state)
{
    (void) state;
    struct lyd_node *first = NULL;
    struct lyd_node *second = NULL;
    struct lyd_difflist *difflist;

    first = lyd_dup(root->child, 0);

    difflist = lyd_diff(first, second, LYD_DIFFOPT_NOSIBLINGS);

    /* Check if there are any differences in trees */
    if (!difflist) {
        fail();
    }

    /* Freeing list and nodes */
    lyd_free_diff(difflist);
    lyd_free(first);
    lyd_free(second);
}

void
test_lyd_new_output(void **state)
{
    (void) state;
    struct lyd_node *node = NULL;

    node = lyd_new_output(NULL, root->schema->module, "rpc1");

    /* Checking if node is assigned */
    if (!node) {
        fail();
    }

    lyd_free(node);
}

void
test_lyd_list_pos(void **state)
{
    (void) state;
    struct lyd_node *node = NULL;
    unsigned int pos;

    node = lyd_new_output(NULL, root->schema->module, "rpc1");
    pos = lyd_list_pos(node);

    /* Checking if the position has been assigned */
    if (pos) {
        fail();
    }
}

void
test_lyd_dup_withsiblings(void **state)
{
    (void) state;
    struct lyd_node *node = NULL;
    struct lyd_node *new_node = NULL;

    node = lyd_new_output(NULL, root->schema->module, "rpc1");

    /* Check if the sibling of our node exists  */
    if (!node->prev) {
        fail();
    }

    new_node = lyd_dup_withsiblings(node, 0);

    /* Check if the sibling of our duplicated node exists */
    if (!new_node->prev) {
        fail();
    }

    /* Freeing nodes */
    lyd_free(node);
    lyd_free(new_node);
}

void
test_lyd_dup_to_ctx(void **state)
{
    (void) state;
    struct ly_ctx *new_ctx = NULL;
    struct lyd_node *node = NULL;
    struct lyd_node *new_node = NULL;
    char *yang_folder = TESTS_DIR "/api/files";

    new_ctx = ly_ctx_new(yang_folder, 0);
    lys_parse_mem(new_ctx, lys_module_a, LYS_IN_YIN);

    node = lyd_new_path(NULL, ctx, "/a:top/bar-sub2", NULL, 0, 0);

    /* Check if the node exist */
    if (!node) {
        fail();
    }

    new_node = lyd_dup_to_ctx(node, 0, new_ctx);

    /* Check if the node has been successfully duplicated to context */
    if (!node) {
        fail();
    }

    /* Freeing up memory */
    lyd_free(node);
    lyd_free(new_node);
    ly_ctx_clean(new_ctx, NULL);
    ly_ctx_destroy(new_ctx, NULL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_lyd_parse_mem),
        cmocka_unit_test(test_lyd_parse_fd),
        cmocka_unit_test(test_lyd_parse_path),
        cmocka_unit_test(test_lyd_parse_xml),
        cmocka_unit_test_setup_teardown(test_lyd_new, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_new_leaf, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_change_leaf, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_output_new_leaf, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_new_path, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_dup, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_insert, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_insert_sibling, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_insert_before, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_insert_after, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_schema_sort, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_find_path, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_find_instance, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_validate, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_unlink, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_free, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_free_withsiblings, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_insert_attr, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_free_attr, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_print_mem_xml, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_print_mem_xml_format, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_print_mem_json, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_print_fd_xml, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_print_fd_xml_format, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_print_fd_json, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_print_file_xml, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_print_file_xml_format, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_print_file_json, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_print_clb_xml, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_print_clb_xml_format, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_print_clb_json, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_path, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_leaf_type, setup_f2, teardown_f2),
        cmocka_unit_test_setup_teardown(test_lyd_validation_dflt_empty_containers, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_diff, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_free_diff, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_new_output, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_dup_withsiblings, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_dup_to_ctx, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
