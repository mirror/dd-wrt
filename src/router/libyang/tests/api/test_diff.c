/**
 * @file test_diff.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Cmocka tests for lyd_diff().
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <cmocka.h>

#include "tests/config.h"
#include "libyang.h"

struct state {
    struct ly_ctx *ctx;
    const struct lys_module *mod;
    struct lyd_node *first;
    struct lyd_node *second;
    char *xml;
};

static int
setup_f(void **state)
{
    struct state *st;
    const char *schemafile = TESTS_DIR"/data/files/defaults.yin";

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

    /* schemas */
    st->mod = lys_parse_path(st->ctx, schemafile, LYS_IN_YIN);
    if (!st->mod) {
        fprintf(stderr, "Failed to load data model \"%s\".\n", schemafile);
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
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free_withsiblings(st->first);
    lyd_free_withsiblings(st->second);
    ly_ctx_destroy(st->ctx, NULL);
    free(st->xml);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_same(void **state)
{
    struct state *st = (*state);
    const char *xml = "<nacm xmlns=\"urn:ietf:params:xml:ns:yang:ietf-netconf-acm\">"
                        "<enable-nacm>true</enable-nacm>"
                        "<read-default>permit</read-default>"
                        "<write-default>deny</write-default>"
                        "<exec-default>permit</exec-default>"
                        "<enable-external-groups>true</enable-external-groups>"
                      "</nacm><df xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";
    struct lyd_difflist *diff;

    assert_ptr_not_equal((st->first = lyd_parse_mem(st->ctx, xml, LYD_XML, LYD_OPT_CONFIG)), NULL);
    assert_ptr_not_equal((st->second = lyd_parse_mem(st->ctx, xml, LYD_XML, LYD_OPT_CONFIG)), NULL);

    assert_ptr_not_equal((diff = lyd_diff(st->first, st->second, 0)), NULL);
    assert_ptr_not_equal(diff->type, NULL);
    assert_int_equal(diff->type[0], LYD_DIFF_END);

    lyd_free_diff(diff);
}

static void
test_empty1(void **state)
{
    struct state *st = (*state);
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";
    struct lyd_difflist *diff;

    assert_ptr_not_equal((st->first = lyd_parse_mem(st->ctx, xml, LYD_XML, LYD_OPT_CONFIG)), NULL);

    assert_ptr_not_equal((diff = lyd_diff(NULL, st->first, 0)), NULL);
    assert_ptr_not_equal(diff->type, NULL);

    assert_int_equal(diff->type[0], LYD_DIFF_CREATED);
    assert_ptr_equal(diff->first[0], NULL);
    assert_ptr_equal(diff->second[0], st->first);

    assert_int_equal(diff->type[1], LYD_DIFF_CREATED);
    assert_ptr_equal(diff->first[1], NULL);
    assert_ptr_equal(diff->second[1], st->first->next);

    assert_int_equal(diff->type[2], LYD_DIFF_END);
    lyd_free_diff(diff);
}

static void
test_empty2(void **state)
{
    struct state *st = (*state);
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><b1_1>42</b1_1>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";
    struct lyd_difflist *diff;

    assert_ptr_not_equal((st->first = lyd_parse_mem(st->ctx, xml, LYD_XML, LYD_OPT_CONFIG)), NULL);

    assert_ptr_not_equal((diff = lyd_diff(st->first, NULL, 0)), NULL);
    assert_ptr_not_equal(diff->type, NULL);

    assert_int_equal(diff->type[0], LYD_DIFF_DELETED);
    assert_ptr_equal(diff->first[0], st->first);
    assert_ptr_equal(diff->second[0], NULL);

    assert_int_equal(diff->type[1], LYD_DIFF_DELETED);
    assert_ptr_equal(diff->first[1], st->first->next);
    assert_ptr_equal(diff->second[1], NULL);

    assert_int_equal(diff->type[2], LYD_DIFF_END);
    lyd_free_diff(diff);
}

static void
test_empty3(void **state)
{
    struct state *st = (*state);
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\"><foo>42</foo></df>";
    struct lyd_difflist *diff;

    assert_ptr_not_equal((st->first = lyd_parse_mem(st->ctx, xml, LYD_XML, LYD_OPT_CONFIG)), NULL);

    assert_ptr_not_equal((diff = lyd_diff(NULL, NULL, 0)), NULL);
    assert_ptr_not_equal(diff->type, NULL);
    assert_int_equal(diff->type[0], LYD_DIFF_END);
    lyd_free_diff(diff);

    assert_ptr_equal((diff = lyd_diff(NULL, st->first->child, 0)), NULL);
    assert_int_equal(ly_errno, LY_EINVAL);

    assert_ptr_not_equal((diff = lyd_diff(st->first->child, NULL, 0)), NULL);
    assert_ptr_not_equal(diff->type, NULL);

    assert_int_equal(diff->type[0], LYD_DIFF_DELETED);
    assert_ptr_equal(diff->first[0], st->first->child);
    assert_ptr_equal(diff->second[0], NULL);

    assert_int_equal(diff->type[1], LYD_DIFF_END);
    lyd_free_diff(diff);
}

static void
test_diff1(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo>"
                      "</df><hidden xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>42</foo><baz>42</baz></hidden>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>41</foo><b1_1>42</b1_1>"
                      "</df>";
    char *str;
    struct lyd_difflist *diff;

    assert_ptr_not_equal((st->first = lyd_parse_mem(st->ctx, xml1, LYD_XML, LYD_OPT_CONFIG)), NULL);
    assert_ptr_not_equal((st->second = lyd_parse_mem(st->ctx, xml2, LYD_XML, LYD_OPT_CONFIG)), NULL);

    assert_ptr_not_equal((diff = lyd_diff(st->first, st->second, 0)), NULL);
    assert_ptr_not_equal(diff->type, NULL);

    assert_int_equal(diff->type[0], LYD_DIFF_CHANGED);
    assert_ptr_not_equal(diff->first[0], NULL);
    assert_string_equal((str = lyd_path(diff->first[0])), "/defaults:df/foo");
    free(str);
    assert_ptr_not_equal(diff->second[0], NULL);
    assert_string_equal((str = lyd_path(diff->second[0])), "/defaults:df/foo");
    free(str);

    assert_int_equal(diff->type[1], LYD_DIFF_DELETED);
    assert_ptr_not_equal(diff->first[1], NULL);
    assert_string_equal((str = lyd_path(diff->first[1])), "/defaults:hidden");
    free(str);
    assert_ptr_equal(diff->second[1], NULL);

    assert_int_equal(diff->type[2], LYD_DIFF_CREATED);
    assert_ptr_not_equal(diff->first[2], NULL);
    assert_string_equal((str = lyd_path(diff->first[2])), "/defaults:df");
    free(str);
    assert_ptr_not_equal(diff->second[2], NULL);
    assert_string_equal((str = lyd_path(diff->second[2])), "/defaults:df/b1_1");
    free(str);

    assert_int_equal(diff->type[3], LYD_DIFF_END);

    lyd_free_diff(diff);
}

static void
test_diff2(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<list><name>a</name><value>1</value></list>"
                         "<list><name>b</name><value>2</value></list>"
                       "</df>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<list><name>b</name><value>-2</value></list>"
                         "<list><name>c</name><value>3</value></list>"
                       "</df>";
    char *str;
    struct lyd_difflist *diff;

    assert_ptr_not_equal((st->first = lyd_parse_mem(st->ctx, xml1, LYD_XML, LYD_OPT_CONFIG)), NULL);
    assert_ptr_not_equal((st->second = lyd_parse_mem(st->ctx, xml2, LYD_XML, LYD_OPT_CONFIG)), NULL);

    assert_ptr_not_equal((diff = lyd_diff(st->first, st->second, 0)), NULL);
    assert_ptr_not_equal(diff->type, NULL);

    assert_int_equal(diff->type[0], LYD_DIFF_CHANGED);
    assert_ptr_not_equal(diff->first[0], NULL);
    assert_string_equal((str = lyd_path(diff->first[0])), "/defaults:df/list[name=\'b\']/value");
    free(str);
    assert_ptr_not_equal(diff->second[0], NULL);
    assert_string_equal((str = lyd_path(diff->second[0])), "/defaults:df/list[name=\'b\']/value");
    free(str);

    assert_int_equal(diff->type[1], LYD_DIFF_DELETED);
    assert_ptr_not_equal(diff->first[1], NULL);
    assert_string_equal((str = lyd_path(diff->first[1])), "/defaults:df/list[name=\'a\']");
    free(str);
    assert_ptr_equal(diff->second[1], NULL);

    assert_int_equal(diff->type[2], LYD_DIFF_CREATED);
    assert_ptr_not_equal(diff->first[2], NULL);
    assert_string_equal((str = lyd_path(diff->first[2])), "/defaults:df");
    free(str);
    assert_ptr_not_equal(diff->second[2], NULL);
    assert_string_equal((str = lyd_path(diff->second[2])), "/defaults:df/list[name=\'c\']");
    free(str);

    assert_int_equal(diff->type[3], LYD_DIFF_END);

    lyd_free_diff(diff);
}

static void
test_move1(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<llist>2</llist>"
                       "</df>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>2</llist>"
                         "<llist>1</llist>"
                       "</df>";
    char *str;
    struct lyd_difflist *diff;

    assert_ptr_not_equal((st->first = lyd_parse_mem(st->ctx, xml1, LYD_XML, LYD_OPT_CONFIG)), NULL);
    assert_ptr_not_equal((st->second = lyd_parse_mem(st->ctx, xml2, LYD_XML, LYD_OPT_CONFIG)), NULL);

    assert_ptr_not_equal((diff = lyd_diff(st->first, st->second, 0)), NULL);
    assert_ptr_not_equal(diff->type, NULL);

    assert_int_equal(diff->type[0], LYD_DIFF_MOVEDAFTER1);
    assert_ptr_not_equal(diff->first[0], NULL);
    assert_string_equal((str = lyd_path(diff->first[0])), "/defaults:df/llist[.='1']");
    free(str);
    assert_ptr_not_equal(diff->second[0], NULL);
    assert_string_equal((str = lyd_path(diff->second[0])), "/defaults:df/llist[.='2']");
    free(str);

    assert_int_equal(diff->type[1], LYD_DIFF_END);

    lyd_free_diff(diff);
}

static void
test_move2(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<llist>2</llist>"
                         "<llist>3</llist>"
                         "<llist>4</llist>"
                         "<llist>5</llist>"
                       "</df>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<llist>4</llist>"
                         "<llist>3</llist>"
                         "<llist>2</llist>"
                         "<llist>5</llist>"
                       "</df>";
    char *str;
    struct lyd_difflist *diff;

    assert_ptr_not_equal((st->first = lyd_parse_mem(st->ctx, xml1, LYD_XML, LYD_OPT_CONFIG)), NULL);
    assert_ptr_not_equal((st->second = lyd_parse_mem(st->ctx, xml2, LYD_XML, LYD_OPT_CONFIG)), NULL);

    assert_ptr_not_equal((diff = lyd_diff(st->first, st->second, 0)), NULL);
    assert_ptr_not_equal(diff->type, NULL);

    assert_int_equal(diff->type[0], LYD_DIFF_MOVEDAFTER1);
    assert_ptr_not_equal(diff->first[0], NULL);
    assert_string_equal((str = lyd_path(diff->first[0])), "/defaults:df/llist[.='2']");
    free(str);
    assert_ptr_not_equal(diff->second[0], NULL);
    assert_string_equal((str = lyd_path(diff->second[0])), "/defaults:df/llist[.='4']");
    free(str);

    assert_int_equal(diff->type[1], LYD_DIFF_MOVEDAFTER1);
    assert_ptr_not_equal(diff->first[1], NULL);
    assert_string_equal((str = lyd_path(diff->first[1])), "/defaults:df/llist[.='4']");
    free(str);
    assert_ptr_not_equal(diff->second[1], NULL);
    assert_string_equal((str = lyd_path(diff->second[1])), "/defaults:df/llist[.='1']");
    free(str);

    assert_int_equal(diff->type[2], LYD_DIFF_END);

    lyd_free_diff(diff);
}

static void
test_move3(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<list><name>a</name><value>1</value></list>"
                         "<llist>2</llist>"
                         "<llist>3</llist>"
                         "<llist>4</llist>"
                       "</df>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<list><name>a</name><value>1</value></list>"
                         "<llist>2</llist>"
                         "<llist>4</llist>"
                         "<llist>3</llist>"
                       "</df>";
    char *str;
    struct lyd_difflist *diff;

    assert_ptr_not_equal((st->first = lyd_parse_mem(st->ctx, xml1, LYD_XML, LYD_OPT_CONFIG)), NULL);
    assert_ptr_not_equal((st->second = lyd_parse_mem(st->ctx, xml2, LYD_XML, LYD_OPT_CONFIG)), NULL);

    assert_ptr_not_equal((diff = lyd_diff(st->first, st->second, 0)), NULL);
    assert_ptr_not_equal(diff->type, NULL);

    assert_int_equal(diff->type[0], LYD_DIFF_MOVEDAFTER1);
    assert_ptr_not_equal(diff->first[0], NULL);
    assert_string_equal((str = lyd_path(diff->first[0])), "/defaults:df/llist[.='3']");
    free(str);
    assert_ptr_not_equal(diff->second[0], NULL);
    assert_string_equal((str = lyd_path(diff->second[0])), "/defaults:df/llist[.='4']");
    free(str);

    assert_int_equal(diff->type[1], LYD_DIFF_END);

    lyd_free_diff(diff);
}

static void
test_mix1(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<llist>2</llist>"
                         "<llist>3</llist>"
                       "</df>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>3</llist>"
                         "<llist>1</llist>"
                       "</df>";
    char *str;
    struct lyd_difflist *diff;

    assert_ptr_not_equal((st->first = lyd_parse_mem(st->ctx, xml1, LYD_XML, LYD_OPT_CONFIG)), NULL);
    assert_ptr_not_equal((st->second = lyd_parse_mem(st->ctx, xml2, LYD_XML, LYD_OPT_CONFIG)), NULL);

    assert_ptr_not_equal((diff = lyd_diff(st->first, st->second, 0)), NULL);
    assert_ptr_not_equal(diff->type, NULL);

    assert_int_equal(diff->type[0], LYD_DIFF_DELETED);
    assert_ptr_not_equal(diff->first[0], NULL);
    assert_string_equal((str = lyd_path(diff->first[0])), "/defaults:df/llist[.='2']");
    free(str);
    assert_ptr_equal(diff->second[0], NULL);

    assert_int_equal(diff->type[1], LYD_DIFF_MOVEDAFTER1);
    assert_ptr_not_equal(diff->first[1], NULL);
    assert_string_equal((str = lyd_path(diff->first[1])), "/defaults:df/llist[.='1']");
    free(str);
    assert_ptr_not_equal(diff->second[1], NULL);
    assert_string_equal((str = lyd_path(diff->second[1])), "/defaults:df/llist[.='3']");
    free(str);

    assert_int_equal(diff->type[2], LYD_DIFF_END);

    lyd_free_diff(diff);
}

static void
test_mix2(void **state)
{
    struct state *st = (*state);
    const char *xml1 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>1</llist>"
                         "<llist>2</llist>"
                         "<llist>3</llist>"
                       "</df>";
    const char *xml2 = "<df xmlns=\"urn:libyang:tests:defaults\">"
                         "<llist>4</llist>"
                         "<llist>3</llist>"
                         "<llist>1</llist>"
                       "</df>";
    char *str;
    struct lyd_difflist *diff;

    assert_ptr_not_equal((st->first = lyd_parse_mem(st->ctx, xml1, LYD_XML, LYD_OPT_CONFIG)), NULL);
    assert_ptr_not_equal((st->second = lyd_parse_mem(st->ctx, xml2, LYD_XML, LYD_OPT_CONFIG)), NULL);

    assert_ptr_not_equal((diff = lyd_diff(st->first, st->second, 0)), NULL);
    assert_ptr_not_equal(diff->type, NULL);

    assert_int_equal(diff->type[0], LYD_DIFF_DELETED);
    assert_ptr_not_equal(diff->first[0], NULL);
    assert_string_equal((str = lyd_path(diff->first[0])), "/defaults:df/llist[.='2']");
    free(str);
    assert_ptr_equal(diff->second[0], NULL);

    assert_int_equal(diff->type[1], LYD_DIFF_MOVEDAFTER1);
    assert_ptr_not_equal(diff->first[1], NULL);
    assert_string_equal((str = lyd_path(diff->first[1])), "/defaults:df/llist[.='1']");
    free(str);
    assert_ptr_not_equal(diff->second[1], NULL);
    assert_string_equal((str = lyd_path(diff->second[1])), "/defaults:df/llist[.='3']");
    free(str);

    assert_int_equal(diff->type[2], LYD_DIFF_CREATED);
    assert_ptr_not_equal(diff->first[2], NULL);
    assert_string_equal((str = lyd_path(diff->first[2])), "/defaults:df");
    free(str);
    assert_ptr_not_equal(diff->second[2], NULL);
    assert_string_equal((str = lyd_path(diff->second[2])), "/defaults:df/llist[.='4']");
    free(str);

    assert_int_equal(diff->type[3], LYD_DIFF_MOVEDAFTER2);
    assert_ptr_equal(diff->first[3], NULL);
    assert_ptr_not_equal(diff->second[3], NULL);
    assert_string_equal((str = lyd_path(diff->second[3])), "/defaults:df/llist[.='4']");
    free(str);

    assert_int_equal(diff->type[4], LYD_DIFF_END);

    lyd_free_diff(diff);
}

static void
test_wd1(void **state)
{
    struct state *st = (*state);
    const char *xml = "<df xmlns=\"urn:libyang:tests:defaults\">"
                        "<foo>41</foo><dllist>4</dllist>"
                      "</df>";
    char *str;
    struct lyd_difflist *diff;

    st->first = NULL;
    lyd_validate(&st->first, LYD_OPT_CONFIG, st->ctx);
    assert_ptr_not_equal(st->first, NULL);
    assert_ptr_not_equal((st->second = lyd_parse_mem(st->ctx, xml, LYD_XML, LYD_OPT_CONFIG)), NULL);

    assert_ptr_not_equal((diff = lyd_diff(st->first, st->second, LYD_DIFFOPT_WITHDEFAULTS)), NULL);
    assert_ptr_not_equal(diff->type, NULL);

    assert_int_equal(diff->type[0], LYD_DIFF_CHANGED);
    assert_ptr_not_equal(diff->first[0], NULL);
    assert_string_equal((str = lyd_path(diff->first[0])), "/defaults:df/foo");
    assert_int_equal(((struct lyd_node_leaf_list*)diff->first[0])->value.int32, 42);
    free(str);
    assert_ptr_not_equal(diff->second[0], NULL);
    assert_string_equal((str = lyd_path(diff->second[0])), "/defaults:df/foo");
    assert_int_equal(((struct lyd_node_leaf_list*)diff->second[0])->value.int32, 41);
    free(str);

    assert_int_equal(diff->type[1], LYD_DIFF_DELETED);
    assert_ptr_not_equal(diff->first[1], NULL);
    assert_string_equal((str = lyd_path(diff->first[1])), "/defaults:df/dllist[.='1']");
    free(str);
    assert_ptr_equal(diff->second[1], NULL);

    assert_int_equal(diff->type[2], LYD_DIFF_DELETED);
    assert_ptr_not_equal(diff->first[2], NULL);
    assert_string_equal((str = lyd_path(diff->first[2])), "/defaults:df/dllist[.='2']");
    free(str);
    assert_ptr_equal(diff->second[2], NULL);

    assert_int_equal(diff->type[3], LYD_DIFF_DELETED);
    assert_ptr_not_equal(diff->first[3], NULL);
    assert_string_equal((str = lyd_path(diff->first[3])), "/defaults:df/dllist[.='3']");
    free(str);
    assert_ptr_equal(diff->second[3], NULL);

    assert_int_equal(diff->type[4], LYD_DIFF_CREATED);
    assert_ptr_not_equal(diff->first[4], NULL);
    assert_string_equal((str = lyd_path(diff->first[4])), "/defaults:df");
    free(str);
    assert_ptr_not_equal(diff->second[4], NULL);
    assert_string_equal((str = lyd_path(diff->second[4])), "/defaults:df/dllist[.='4']");
    free(str);

    assert_int_equal(diff->type[5], LYD_DIFF_END);

    lyd_free_diff(diff);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_same, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_empty1, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_empty2, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_empty3, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_diff1, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_diff2, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_move1, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_move2, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_move3, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_mix1, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_mix2, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_wd1, setup_f, teardown_f), };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
