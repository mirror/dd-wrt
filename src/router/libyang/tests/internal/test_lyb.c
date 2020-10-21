/**
 * @file test_lyb.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Cmocka tests for LYB binary data format.
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>
#include <inttypes.h>

#include "tests/config.h"
#include "libyang.h"
#include "tree_internal.h"
#include "hash_table.h"

struct state {
    struct ly_ctx *ctx;
    struct lyd_node *dt1, *dt2;
    char *mem;
};

static void
check_data_tree_next(struct lyd_node **start, struct lyd_node **next, struct lyd_node **elem)
{
    if (*elem) {
        goto loop_next;
    }

loop_begin:
    LY_TREE_DFS_BEGIN(*start, *next, *elem) {
        return;
loop_next:
        LY_TREE_DFS_END(*start, *next, *elem);
    }

    if (!*next) {
        /* top-level siblings */
        *start = (*start)->next;
        if (!(*start)) {
            *elem = NULL;
            return;
        }
        goto loop_begin;
    }

    return;
}

static void
check_data_tree(struct lyd_node *root1, struct lyd_node *root2)
{
    struct lyd_node *next1, *next2, *elem1 = NULL, *elem2 = NULL, *iter;
    struct lyd_attr *attr1, *attr2;
    struct lyd_node_leaf_list *leaf1, *leaf2;
    struct lyd_node_anydata *any1, *any2;
#ifdef LY_ENABLED_CACHE
    uint32_t i1, i2;
#endif

    for (check_data_tree_next(&root1, &next1, &elem1), check_data_tree_next(&root2, &next2, &elem2);
         elem1 && elem2;
         check_data_tree_next(&root1, &next1, &elem1), check_data_tree_next(&root2, &next2, &elem2)) {

        if (elem1->schema != elem2->schema) {
            fprintf(stderr, "Schema mismatch (\"%s\" and \"%s\").\n", elem1->schema->name, elem2->schema->name);
            fail();
        }

        /* check common data node attributes */
        if (elem1->validity != elem2->validity) {
            fprintf(stderr, "\"%s\": validity flags mismatch (\"%u\" and \"%u\").\n", elem1->schema->name, elem1->validity, elem2->validity);
            fail();
        } else if (elem1->dflt != elem2->dflt) {
            fprintf(stderr, "\"%s\": dflt flag mismatch (\"%u\" and \"%u\").\n", elem1->schema->name, elem1->dflt, elem2->dflt);
            fail();
        } else if (elem1->when_status != elem2->when_status) {
            fprintf(stderr, "\"%s\": when flags mismatch (\"%u\" and \"%u\").\n", elem1->schema->name, elem1->when_status, elem2->when_status);
            fail();
        }

        /* check data node attributes */
        for (attr1 = elem1->attr, attr2 = elem2->attr; attr1 && attr2; attr1 = attr1->next, attr2 = attr2->next) {
            if (attr1->annotation != attr2->annotation) {
                fprintf(stderr, "\"%s\": attr annotation mismatch.\n", elem1->schema->name);
                fail();
            }
            if (strcmp(attr1->name, attr2->name)) {
                fprintf(stderr, "\"%s\": attr name mismatch (\"%s\" and \"%s\").\n", elem1->schema->name, attr1->name, attr2->name);
                fail();
            }
            if (strcmp(attr1->value_str, attr2->value_str)) {
                fprintf(stderr, "\"%s\": attr value_str mismatch (\"%s\" and \"%s\").\n", elem1->schema->name, attr1->value_str, attr2->value_str);
                fail();
            }
            switch (attr1->value_type) {
            case LY_TYPE_BITS:
            case LY_TYPE_INST:
            case LY_TYPE_LEAFREF:
                /* do not compare pointers */
                break;
            default:
                if ((attr1->value.uint64 != attr2->value.uint64) && !(attr1->value_flags & LY_VALUE_USER)) {
                    fprintf(stderr, "\"%s\": attr value mismatch (\"%" PRIu64 "\" and \"%" PRIu64 "\").\n", elem1->schema->name, attr1->value.uint64, attr2->value.uint64);
                    fail();
                }
                break;
            }
            if (attr1->value_type != attr2->value_type) {
                fprintf(stderr, "\"%s\": attr value_type mismatch (\"%d\" and \"%d\").\n", elem1->schema->name, attr1->value_type, attr2->value_type);
                fail();
            }
            if (attr1->value_flags != attr2->value_flags) {
                fprintf(stderr, "\"%s\": attr value_flags mismatch (\"%d\" and \"%d\").\n", elem1->schema->name, attr1->value_flags, attr2->value_flags);
                fail();
            }
        }
        if (attr1) {
            fprintf(stderr, "\"%s\": attr mismatch (\"%s\" and \"NULL\").\n", elem1->schema->name, attr1->name);
            fail();
        }
        if (attr2) {
            fprintf(stderr, "\"%s\": attr mismatch (\"NULL\" and \"%s\").\n", elem1->schema->name, attr2->name);
            fail();
        }

        /* check specific data node attributes */
        switch (elem1->schema->nodetype) {
        case LYS_CONTAINER:
        case LYS_LIST:
        case LYS_RPC:
        case LYS_ACTION:
        case LYS_NOTIF:
#ifdef LY_ENABLED_CACHE
            i1 = 0;
            LY_TREE_FOR(elem1->child, iter) {
                ++i1;
            }

            i2 = 0;
            LY_TREE_FOR(elem2->child, iter) {
                ++i2;
            }

            if (i1 != i2) {
                fprintf(stderr, "\"%s\": child count mismatch (%u and %u).\n", elem1->schema->name, i1, i2);
                fail();
            }

            if (i1 >= LY_CACHE_HT_MIN_CHILDREN) {
                if (!elem1->ht || !elem2->ht) {
                    fprintf(stderr, "\"%s\": missing hash table (%p and %p).\n", elem1->schema->name, elem1->ht, elem2->ht);
                    fail();
                }

                LY_TREE_FOR(elem1->child, iter) {
                    if (lyht_find(elem1->ht, &iter, iter->hash, NULL)) {
                        fprintf(stderr, "\"%s\": missing child \"%s\" in the hash table 1.\n", elem1->schema->name, iter->schema->name);
                        fail();
                    }
                }
                LY_TREE_FOR(elem2->child, iter) {
                    if (lyht_find(elem2->ht, &iter, iter->hash, NULL)) {
                        fprintf(stderr, "\"%s\": missing child \"%s\" in the hash table 2.\n", elem1->schema->name, iter->schema->name);
                        fail();
                    }
                }
            }
#endif
            break;
        case LYS_LEAF:
        case LYS_LEAFLIST:
            leaf1 = (struct lyd_node_leaf_list *)elem1;
            leaf2 = (struct lyd_node_leaf_list *)elem2;

            /* both should be in the same dictionary */
            if (leaf1->value_str != leaf2->value_str) {
                fprintf(stderr, "\"%s\": value_str mismatch (\"%s\" and \"%s\").\n", elem1->schema->name, leaf1->value_str, leaf2->value_str);
                fail();
            }
            switch (leaf1->value_type) {
            case LY_TYPE_BITS:
            case LY_TYPE_INST:
            case LY_TYPE_LEAFREF:
                /* do not compare pointers */
                break;
            default:
                if ((leaf1->value.uint64 != leaf2->value.uint64) && !(leaf1->value_flags & LY_VALUE_USER)) {
                    fprintf(stderr, "\"%s\": value mismatch (\"%" PRIu64 "\" and \"%" PRIu64 "\").\n", elem1->schema->name, leaf1->value.uint64, leaf2->value.uint64);
                    fail();
                }
                break;
            }
            if (leaf1->value_type != leaf2->value_type) {
                fprintf(stderr, "\"%s\": value_type mismatch (\"%d\" and \"%d\").\n", elem1->schema->name, leaf1->value_type, leaf2->value_type);
                fail();
            }
            if (leaf1->value_flags != leaf2->value_flags) {
                fprintf(stderr, "\"%s\": attr value_flags mismatch (\"%d\" and \"%d\").\n", elem1->schema->name, leaf1->value_flags, leaf2->value_flags);
                fail();
            }
            break;
        case LYS_ANYDATA:
        case LYS_ANYXML:
            any1 = (struct lyd_node_anydata *)elem1;
            any2 = (struct lyd_node_anydata *)elem2;

            /* if we had to do conversion from XML, skip it, assume it was done correctly */
            if ((any1->value_type != LYD_ANYDATA_XML) && (any2->value_type != LYD_ANYDATA_XML)) {
                if (any1->value_type != any2->value_type) {
                    fprintf(stderr, "\"%s\": value_type mismatch (\"%d\" and \"%d\").\n", elem1->schema->name, any1->value_type, any2->value_type);
                    fail();
                }
                if (any1->value_type == LYD_ANYDATA_DATATREE) {
                    check_data_tree(any1->value.tree, any2->value.tree);
                } else if (strcmp(any1->value.str, any2->value.str)) {
                    fprintf(stderr, "\"%s\": value mismatch\n\"\"%s\"\"\nand\n\"\"%s\"\"\n", elem1->schema->name, any1->value.str, any2->value.str);
                    fail();
                }
            }
            break;
        default:
            fprintf(stderr, "Unexpected data node type.\n");
            fail();
        }

#ifdef LY_ENABLED_CACHE
        if (!elem1->hash) {
            fprintf(stderr, "\"%s\": hash not calculated.\n", elem1->schema->name);
            fail();
        }
        if (elem1->hash != elem2->hash) {
            fprintf(stderr, "\"%s\": hashes do not match (%u and %u).\n", elem1->schema->name, elem1->hash, elem2->hash);
            fail();
        }
#endif
    }

    if (elem1) {
        fprintf(stderr, "Schema mismatch (\"%s\" and \"NULL\").\n", elem1->schema->name);
        fail();
    }
    if (elem2) {
        fprintf(stderr, "Schema mismatch (\"NULL\" and \"%s\").\n", elem2->schema->name);
        fail();
    }
}

static int
setup_f(void **state)
{
    struct state *st;

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(TESTS_DIR"/schema/yang/ietf/", 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        return -1;
    }

    return 0;
}

static int
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free_withsiblings(st->dt1);
    lyd_free_withsiblings(st->dt2);
    ly_ctx_destroy(st->ctx, NULL);
    free(st->mem);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_ietf_interfaces(void **state)
{
    struct state *st = (*state);
    int ret;

    assert_non_null(ly_ctx_load_module(st->ctx, "ietf-ip", NULL));
    assert_non_null(ly_ctx_load_module(st->ctx, "iana-if-type", NULL));

    st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/ietf-interfaces.json", LYD_JSON, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt1, NULL);

    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_not_equal(st->dt2, NULL);

    check_data_tree(st->dt1, st->dt2);
}

static void
test_origin(void **state)
{
    struct state *st = (*state);
    int ret;
    const char *test_origin =
    "module test-origin {"
    "   namespace \"urn:test-origin\";"
    "   prefix to;"
    "   import ietf-origin {"
    "       prefix or;"
    "   }"
    ""
    "   container cont {"
    "       leaf leaf1 {"
    "           type string;"
    "       }"
    "       leaf leaf2 {"
    "           type string;"
    "       }"
    "       leaf leaf3 {"
    "           type uint8;"
    "       }"
    "   }"
    "}";

    assert_non_null(lys_parse_mem(st->ctx, test_origin, LYS_YANG));
    lys_set_implemented(ly_ctx_get_module(st->ctx, "ietf-origin", NULL, 0));

    st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/test-origin.json", LYD_JSON, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt1, NULL);

    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_not_equal(st->dt2, NULL);

    check_data_tree(st->dt1, st->dt2);
}

static void
test_statements(void **state)
{
    struct state *st = (*state);
    int ret;

    ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
    assert_non_null(ly_ctx_load_module(st->ctx, "statements", NULL));

    st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/statements.xml", LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt1, NULL);

    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_not_equal(st->dt2, NULL);

    check_data_tree(st->dt1, st->dt2);
}

static void
test_types(void **state)
{
    struct state *st = (*state);
    int ret;

    ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
    assert_non_null(ly_ctx_load_module(st->ctx, "types", NULL));

    st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/types.xml", LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt1, NULL);

    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_not_equal(st->dt2, NULL);

    check_data_tree(st->dt1, st->dt2);
}

static void
test_annotations(void **state)
{
    struct state *st = (*state);
    int ret;

    ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
    assert_non_null(ly_ctx_load_module(st->ctx, "annotations", NULL));

    st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/annotations.xml", LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt1, NULL);

    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_not_equal(st->dt2, NULL);

    check_data_tree(st->dt1, st->dt2);
}

static void
test_similar_annot_names(void **state)
{
    struct state *st = (*state);
    int ret;

    ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
    assert_non_null(ly_ctx_load_module(st->ctx, "annotations", NULL));

    st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/similar-annot-names.xml", LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt1, NULL);

    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_not_equal(st->dt2, NULL);

    check_data_tree(st->dt1, st->dt2);
}

static void
test_many_child_annot(void **state)
{
    struct state *st = (*state);
    int ret;

    ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
    assert_non_null(ly_ctx_load_module(st->ctx, "annotations", NULL));

    st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/many-childs-annot.xml", LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt1, NULL);

    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_not_equal(st->dt2, NULL);

    check_data_tree(st->dt1, st->dt2);
}

static void
test_union(void **state)
{
    struct state *st = (*state);
    int ret;

    ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
    assert_non_null(ly_ctx_load_module(st->ctx, "union", NULL));

    st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/union.xml", LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt1, NULL);

    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_not_equal(st->dt2, NULL);

    check_data_tree(st->dt1, st->dt2);
}

static void
test_union2(void **state)
{
    struct state *st = (*state);
    int ret;

    ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
    assert_non_null(ly_ctx_load_module(st->ctx, "statements", NULL));

    st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/union2.xml", LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt1, NULL);

    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_not_equal(st->dt2, NULL);

    check_data_tree(st->dt1, st->dt2);
}

static void
test_collisions(void **state)
{
    struct state *st = (*state);
    int ret;

    ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
    assert_non_null(ly_ctx_load_module(st->ctx, "annotations", NULL));

    st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/collisions.xml", LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt1, NULL);

    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_not_equal(st->dt2, NULL);

    check_data_tree(st->dt1, st->dt2);
}

static void
test_anydata(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    int ret;
    const char *test_anydata =
    "module test-anydata {"
    "   namespace \"urn:test-anydata\";"
    "   prefix ya;"
    ""
    "   container cont {"
    "       anydata ntf;"
    "   }"
    "}";

    assert_non_null(ly_ctx_load_module(st->ctx, "ietf-netconf-notifications", NULL));

    st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/ietf-netconf-notifications.json", LYD_JSON, LYD_OPT_NOTIF | LYD_OPT_TRUSTED, NULL);
    assert_ptr_not_equal(st->dt1, NULL);

    /* get notification in LYB format to set as anydata content */
    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    lyd_free_withsiblings(st->dt1);
    st->dt1 = NULL;

    /* now comes the real test, test anydata */
    mod = lys_parse_mem(st->ctx, test_anydata, LYS_YANG);
    assert_non_null(mod);

    st->dt1 = lyd_new(NULL, mod, "cont");
    assert_non_null(st->dt1);

    assert_non_null(lyd_new_anydata(st->dt1, NULL, "ntf", st->mem, LYD_ANYDATA_LYBD));
    st->mem = NULL;

    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    ret = lyd_validate(&st->dt1, LYD_OPT_CONFIG, NULL);
    assert_int_equal(ret, 0);

    st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_not_equal(st->dt2, NULL);

    check_data_tree(st->dt1, st->dt2);

    /* and also test the embedded notification itself */
    free(st->mem);
    ret = lyd_lyb_data_length(((struct lyd_node_anydata *)st->dt1->child)->value.mem);
    st->mem = malloc(ret);
    memcpy(st->mem, ((struct lyd_node_anydata *)st->dt1->child)->value.mem, ret);

    lyd_free_withsiblings(st->dt2);
    st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_NOTIF | LYD_OPT_STRICT | LYD_OPT_NOEXTDEPS, NULL);
    assert_ptr_not_equal(st->dt2, NULL);

    /* parse the JSON again for this comparison */
    lyd_free_withsiblings(st->dt1);
    st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/ietf-netconf-notifications.json", LYD_JSON, LYD_OPT_NOTIF | LYD_OPT_TRUSTED, NULL);
    assert_ptr_not_equal(st->dt1, NULL);

    check_data_tree(st->dt1, st->dt2);
}

static void
test_submodule_feature(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    int ret;

    ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
    mod = ly_ctx_load_module(st->ctx, "feature-submodule-main", NULL);
    assert_non_null(mod);
    assert_int_equal(lys_features_enable(mod, "test-submodule-feature"), 0);

    st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/test-submodule-feature.json", LYD_JSON, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt1, NULL);

    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_not_equal(st->dt2, NULL);

    check_data_tree(st->dt1, st->dt2);
}

static void
test_coliding_augments(void **state)
{
    struct state *st = (*state);
    int ret;

    ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
    assert_non_null(ly_ctx_load_module(st->ctx, "augment-target", NULL));
    assert_non_null(ly_ctx_load_module(st->ctx, "augment0", NULL));
    assert_non_null(ly_ctx_load_module(st->ctx, "augment1", NULL));

    st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/augment.xml", LYD_XML, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt1, NULL);

    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_not_equal(st->dt2, NULL);

    check_data_tree(st->dt1, st->dt2);
}

static void
test_leafrefs(void **state)
{
    struct state *st = (*state);
    int ret;

    ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
    assert_non_null(ly_ctx_load_module(st->ctx, "leafrefs2", NULL));

    st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/leafrefs2.json", LYD_JSON, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_not_equal(st->dt1, NULL);

    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    assert_ptr_not_equal(st->dt2, NULL);

    check_data_tree(st->dt1, st->dt2);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_ietf_interfaces, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_origin, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_statements, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_types, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_annotations, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_similar_annot_names, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_many_child_annot, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_union, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_union2, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_collisions, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_anydata, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_submodule_feature, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_coliding_augments, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_leafrefs, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
