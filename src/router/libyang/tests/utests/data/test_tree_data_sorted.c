/**
 * @file test_tree_data_sorted.c
 * @author Adam Piecek <piecek@cesnet.cz>
 * @brief Unit tests for functions from tree_data_sorted.h
 *
 * Copyright (c) 2018-2023 CESNET, z.s.p.o.
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
#include "ly_common.h"
#include "tree_data_internal.h"
#include "tree_data_sorted.h"

#define META_NAME "lyds_tree"

static void *
get_rbt(struct lyd_meta *meta)
{
    struct lyd_value_lyds_tree *lt;

    if (!meta) {
        return NULL;
    }

    LYD_VALUE_GET(&meta->value, lt);
    return lt ? lt->rbt : NULL;
}

static void
test_insert_top_level_list(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *first, *node;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "list lst {key \"k\"; leaf k {type uint32;}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    assert_int_equal(lyd_new_list(NULL, mod, "lst", 0, &first, "2"), LY_SUCCESS);
    assert_int_equal(lyd_new_list(NULL, mod, "lst", 0, &node, "1"), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(first, node, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_list(NULL, mod, "lst", 0, &node, "3"), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(first, node, NULL), LY_SUCCESS);
    assert_true(first->next && first->prev && first->prev->meta);
    assert_string_equal(first->prev->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(lyd_child(first->prev)), "1");
    assert_string_equal(lyd_get_value(lyd_child(first)), "2");
    assert_string_equal(lyd_get_value(lyd_child(first->next)), "3");
    lyd_free_all(first);
}

static void
test_insert_top_level_leaflist(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *first, *node;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "leaf-list ll {type uint32;}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    assert_int_equal(lyd_new_term(NULL, mod, "ll", "2", 0, &first), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "1", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(first, node, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "3", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(first, node, NULL), LY_SUCCESS);
    assert_true(first->next && first->prev && first->prev->meta);
    assert_string_equal(first->prev->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(first->prev), "1");
    assert_string_equal(lyd_get_value(first), "2");
    assert_string_equal(lyd_get_value(first->next), "3");
    lyd_free_all(first);
}

static void
test_insert_cont_list(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *node;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn { list lst {key \"k\"; leaf k {type uint32;}}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);
    assert_int_equal(lyd_new_list(cont, mod, "lst", 0, NULL, "2"), LY_SUCCESS);
    assert_int_equal(lyd_new_list(cont, mod, "lst", 0, NULL, "1"), LY_SUCCESS);
    assert_int_equal(lyd_new_list(cont, mod, "lst", 0, NULL, "3"), LY_SUCCESS);
    node = lyd_child(cont);
    assert_true(node && node->meta && node->next && node->next->next);
    assert_string_equal(node->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(lyd_child(node)), "1");
    assert_string_equal(lyd_get_value(lyd_child(node->next)), "2");
    assert_string_equal(lyd_get_value(lyd_child(node->next->next)), "3");
    lyd_free_all(cont);
}

static void
test_insert_cont_leaflist(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *node;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn { leaf-list ll {type uint32;}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "2", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "3", 0, NULL), LY_SUCCESS);
    node = lyd_child(cont);
    assert_true(node && node->next && node->next->next && node->meta);
    assert_string_equal(node->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(node), "1");
    assert_string_equal(lyd_get_value(node->next), "2");
    assert_string_equal(lyd_get_value(node->next->next), "3");
    lyd_free_all(cont);
}

static void
test_try_user_order_func(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *sibl, *node;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "leaf-list ll {type uint32;}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    assert_int_equal(lyd_new_term(NULL, mod, "ll", "2", 0, &sibl), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "1", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_before(sibl, node), LY_EINVAL);
    CHECK_LOG_CTX("Can be used only for user-ordered nodes.", NULL, 0);
    assert_int_equal(lyd_insert_after(sibl, node), LY_EINVAL);
    CHECK_LOG_CTX("Can be used only for user-ordered nodes.", NULL, 0);
    lyd_free_all(node);
    lyd_free_all(sibl);
}

static void
test_ordered_by_user(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *node;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn { leaf-list ll {type uint32; ordered-by user;}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);

    assert_int_equal(lyd_new_term(cont, mod, "ll", "2", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, NULL), LY_SUCCESS);
    node = lyd_child(cont);
    assert_true(node && !node->meta);
    assert_string_equal(lyd_get_value(node), "2");
    assert_string_equal(lyd_get_value(node->next), "1");
    lyd_free_all(cont);
}

static void
test_remove(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *node, *deleted;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn { leaf-list ll {type uint32;}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* Remove first */
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "2", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "3", 0, NULL), LY_SUCCESS);
    node = lyd_child(cont);
    assert_true(node && node->next && node->next->next && node->meta);
    assert_string_equal(node->meta->name, META_NAME);
    deleted = node;
    lyd_unlink_tree(deleted);
    lyd_free_tree(deleted);
    node = lyd_child(cont);
    assert_true(node && node->next && node->meta);
    assert_string_equal(node->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(node), "2");
    assert_string_equal(lyd_get_value(node->next), "3");
    lyd_free_all(cont);

    /* Remove middle */
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "2", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "3", 0, NULL), LY_SUCCESS);
    node = lyd_child(cont);
    assert_true(node && node->next && node->next->next && node->meta);
    assert_string_equal(node->meta->name, META_NAME);
    deleted = node->next;
    lyd_unlink_tree(deleted);
    lyd_free_tree(deleted);
    node = lyd_child(cont);
    assert_true(node && node->next && node->meta);
    assert_string_equal(node->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(node), "1");
    assert_string_equal(lyd_get_value(node->next), "3");
    lyd_free_all(cont);

    /* Remove last */
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "2", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "3", 0, NULL), LY_SUCCESS);
    node = lyd_child(cont);
    assert_true(node && node->next && node->next->next && node->meta);
    assert_string_equal(node->meta->name, META_NAME);
    deleted = node->next->next;
    lyd_unlink_tree(deleted);
    lyd_free_tree(deleted);
    node = lyd_child(cont);
    assert_true(node && node->next && node->meta);
    assert_string_equal(node->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(node), "1");
    assert_string_equal(lyd_get_value(node->next), "2");
    lyd_free_all(cont);

    /* Remove all */
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, NULL), LY_SUCCESS);
    node = lyd_child(cont);
    assert_non_null(node);
    deleted = node;
    lyd_unlink_tree(deleted);
    lyd_free_tree(deleted);
    node = lyd_child(cont);
    assert_null(node);
    lyd_free_all(cont);
}

static void
test_remove_then_insert(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *node, *deleted;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn { leaf-list ll {type uint32;}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* remove first */
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "2", 0, NULL), LY_SUCCESS);
    node = lyd_child(cont);
    assert_true(node && node->next && node->meta);
    assert_string_equal(node->meta->name, META_NAME);
    deleted = node;
    lyd_unlink_tree(deleted);
    lyd_free_tree(deleted);
    node = lyd_child(cont);
    assert_non_null(node->meta);
    assert_string_equal(node->meta->name, META_NAME);

    /* insert last */
    assert_int_equal(lyd_new_term(cont, mod, "ll", "3", 0, NULL), LY_SUCCESS);
    node = lyd_child(cont);
    assert_true(node && node->next && node->meta);
    assert_string_equal(node->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(node), "2");
    assert_string_equal(lyd_get_value(node->next), "3");

    /* insert first */
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, NULL), LY_SUCCESS);
    node = lyd_child(cont);
    assert_true(node && node->next && node->next && node->meta);
    assert_string_equal(node->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(node), "1");
    assert_string_equal(lyd_get_value(node->next), "2");
    assert_string_equal(lyd_get_value(node->next->next), "3");
    lyd_free_all(cont);
}

static void
test_unlink_all(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *first, *second;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "leaf-list ll {type uint32;}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* unlink second then first */
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "1", 0, &first), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "2", 0, &second), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(first, second, NULL), LY_SUCCESS);
    lyd_unlink_tree(second);
    assert_non_null(first->meta);
    lyd_unlink_tree(first);
    assert_true(first->meta && !second->meta);

    /* unlink first then second */
    assert_int_equal(lyd_insert_sibling(first, second, NULL), LY_SUCCESS);
    lyd_unlink_tree(first);
    assert_true(!first->meta && second->meta);

    lyd_free_all(first);
    lyd_free_all(second);
}

static void
test_insert_before_anchor(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *node;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn {"
            "  leaf-list llm {"
            "    type string;"
            "  }"
            "  leaf-list lln {"
            "    type string;"
            "  }"
            "}}";

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);

    assert_int_equal(lyd_new_term(cont, mod, "lln", "1", 0, NULL), LY_SUCCESS);
    node = lyd_child(cont);
    assert_true(node && !node->meta);
    assert_string_equal(lyd_get_value(node), "1");

    assert_int_equal(lyd_new_term(cont, mod, "llm", "2", 0, NULL), LY_SUCCESS);
    node = lyd_child(cont);
    assert_true(node && !node->meta && node->next);
    assert_string_equal(lyd_get_value(node), "2");
    assert_string_equal(lyd_get_value(node->next), "1");

    lyd_free_all(cont);
}

static void
test_insert_after_anchor(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *node;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn {"
            "  leaf-list llm {"
            "    type string;"
            "  }"
            "  leaf-list lln {"
            "    type string;"
            "  }"
            "}}";

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);

    assert_int_equal(lyd_new_term(cont, mod, "llm", "1", 0, NULL), LY_SUCCESS);
    node = lyd_child(cont);
    assert_true(node && !node->meta);
    assert_string_equal(lyd_get_value(node), "1");

    assert_int_equal(lyd_new_term(cont, mod, "lln", "2", 0, NULL), LY_SUCCESS);
    node = lyd_child(cont);
    assert_true(node && !node->meta && node->next);
    assert_string_equal(lyd_get_value(node), "1");
    assert_string_equal(lyd_get_value(node->next), "2");

    lyd_free_all(cont);
}

static void
test_insert_same_values_leaflist(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *n1, *n2;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn { leaf-list ll {type uint32;}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);

    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, NULL), LY_SUCCESS);
    n1 = lyd_child(cont);
    assert_true(n1 && !n1->meta);
    assert_string_equal(lyd_get_value(n1), "1");

    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, NULL), LY_SUCCESS);
    n2 = lyd_child(cont);
    assert_true(n2 && n2->meta && n2->next);
    assert_string_equal(n2->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(n2), "1");
    assert_string_equal(lyd_get_value(n2->next), "1");

    assert_ptr_equal(n1, n2);
    lyd_free_all(cont);
}

static void
test_insert_same_values_list(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *n1, *n2;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn { list lst {key \"k\"; leaf k {type uint32;}}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);

    assert_int_equal(lyd_new_list(cont, mod, "lst", 0, NULL, "1"), LY_SUCCESS);
    n1 = lyd_child(cont);
    assert_true(n1 && !n1->meta);
    assert_string_equal(lyd_get_value(lyd_child(n1)), "1");

    assert_int_equal(lyd_new_list(cont, mod, "lst", 0, NULL, "1"), LY_SUCCESS);
    n2 = lyd_child(cont);
    assert_true(n2 && n2->meta);
    assert_string_equal(n2->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(lyd_child(n2)), "1");
    assert_string_equal(lyd_get_value(lyd_child(n2->next)), "1");

    assert_ptr_equal(n1, n2);
    lyd_free_all(cont);
}

static void
test_remove_same_values_leaflist(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *n1, *n2, *n3, *n4, *n5, *child;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn { leaf-list ll {type uint32;}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);

    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, &n1), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, &n2), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, &n3), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, &n4), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, &n5), LY_SUCCESS);

    /* remove second node */
    lyd_free_tree(n2);
    child = lyd_child(cont);
    assert_true((child == n1) && (child->next == n3) && (child->next->next == n4) && (child->next->next->next == n5));

    /* remove first node */
    lyd_free_tree(n1);
    child = lyd_child(cont);
    assert_true((child == n3) && (child->next == n4) && (child->next->next == n5));

    /* remove fifth node */
    lyd_free_tree(n5);
    child = lyd_child(cont);
    assert_true((child == n3) && (child->next == n4));

    /* remove fourth node */
    lyd_free_tree(n4);
    child = lyd_child(cont);
    assert_true(child == n3);

    lyd_free_all(cont);
}

static void
test_insert_keyless_list(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *lst, *node;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn { list lst {config false; leaf lf {type uint32;}}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* lyds tree is not used for keyless list */
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);
    assert_int_equal(lyd_new_list(cont, mod, "lst", 0, &lst), LY_SUCCESS);
    assert_int_equal(lyd_new_term(lst, mod, "lf", "2", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_list(cont, mod, "lst", 0, &lst), LY_SUCCESS);
    assert_int_equal(lyd_new_term(lst, mod, "lf", "1", 0, NULL), LY_SUCCESS);
    node = lyd_child(cont);
    assert_true(node && !node->meta && node->next);
    assert_string_equal(lyd_get_value(lyd_child(node)), "2");
    assert_string_equal(lyd_get_value(lyd_child(node->next)), "1");
    lyd_free_all(cont);
}

static void
test_leaflist_default(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *node;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn { leaf-list ll {default \"1\"; type uint32;}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);

    assert_int_equal(lyd_new_term(cont, mod, "ll", "2", 0, NULL), LY_SUCCESS);
    node = lyd_child(cont);
    assert_true(node && !node->meta);
    assert_string_equal(lyd_get_value(node), "2");
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, NULL), LY_SUCCESS);
    node = lyd_child(cont);
    assert_true(node && node->next && node->meta);
    assert_string_equal(node->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(node), "1");
    lyd_free_all(cont);
}

static void
test_unlink_then_insert(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *first, *second, *third;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn { leaf-list ll {type uint32;}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* unlink first and second */
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "2", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "3", 0, NULL), LY_SUCCESS);
    first = lyd_child(cont);
    lyd_unlink_tree(first);
    second = lyd_child(cont);
    lyd_unlink_tree(second);
    third = lyd_child(cont);
    assert_true(third && third->meta && !first->meta && !second->meta);
    assert_string_equal(third->meta->name, META_NAME);

    /* insert first */
    lyd_insert_child(cont, first);
    assert_true(first && first->meta && !third->meta);
    assert_string_equal(first->meta->name, META_NAME);

    /* insert second */
    lyd_insert_child(cont, second);
    assert_true(first && first->meta && !second->meta && !third->meta);
    assert_string_equal(first->meta->name, META_NAME);

    /* check the order */
    assert_ptr_equal(lyd_child(cont), first);
    first = lyd_child(cont);
    assert_string_equal(lyd_get_value(first), "1");
    assert_string_equal(lyd_get_value(first->next), "2");
    assert_string_equal(lyd_get_value(first->next->next), "3");

    /* unlink all nodes */
    lyd_unlink_tree(lyd_child(cont));
    lyd_unlink_tree(lyd_child(cont));
    lyd_unlink_tree(lyd_child(cont));
    assert_null(lyd_child(cont));
    assert_true(!first->meta && !second->meta && third->meta && get_rbt(third->meta));
    assert_string_equal(third->meta->name, META_NAME);

    /* insert third */
    lyd_insert_child(cont, third);
    assert_non_null(third->meta);
    assert_string_equal(third->meta->name, META_NAME);

    lyd_free_all(cont);
    lyd_free_all(first);
    lyd_free_all(second);
}

static void
test_change_term(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *first, *node;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn { leaf-list ll {type uint32;}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "2", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "3", 0, NULL), LY_SUCCESS);
    first = lyd_child(cont);

    /* change node which has no meta */
    node = lyd_child(cont)->next;
    assert_int_equal(lyd_change_term(node, "5"), LY_SUCCESS);
    assert_string_equal(lyd_get_value(node), "5");
    assert_true(first && first->meta && first->next && first->next->next);
    assert_string_equal(lyd_get_value(first), "1");
    assert_string_equal(lyd_get_value(first->next), "3");
    assert_string_equal(lyd_get_value(first->next->next), "5");

    /* change node which has meta */
    assert_int_equal(lyd_change_term(first, "6"), LY_SUCCESS);
    assert_string_equal(lyd_get_value(first), "6");
    first = lyd_child(cont);
    assert_true(first && first->meta && first->next && first->next->next);
    assert_string_equal(lyd_get_value(first), "3");
    assert_string_equal(lyd_get_value(first->next), "5");
    assert_string_equal(lyd_get_value(first->next->next), "6");

    lyd_free_all(cont);
}

static void
test_change_key(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *first, *node;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn { list lst {key \"k\"; leaf k {type uint32;}}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);
    assert_int_equal(lyd_new_list(cont, mod, "lst", 0, NULL, "1"), LY_SUCCESS);
    assert_int_equal(lyd_new_list(cont, mod, "lst", 0, NULL, "2"), LY_SUCCESS);
    assert_int_equal(lyd_new_list(cont, mod, "lst", 0, NULL, "3"), LY_SUCCESS);
    first = lyd_child(cont);

    /* change node which has no meta */
    node = lyd_child(cont)->next;
    assert_int_equal(lyd_change_term(lyd_child(node), "5"), LY_SUCCESS);
    assert_string_equal(lyd_get_value(lyd_child(node)), "5");
    assert_true(first && first->meta && first->next && first->next->next);
    assert_string_equal(lyd_get_value(lyd_child(first)), "1");
    assert_string_equal(lyd_get_value(lyd_child(first->next)), "3");
    assert_string_equal(lyd_get_value(lyd_child(first->next->next)), "5");

    /* change node which has meta */
    assert_int_equal(lyd_change_term(lyd_child(first), "6"), LY_SUCCESS);
    assert_string_equal(lyd_get_value(lyd_child(first)), "6");
    first = lyd_child(cont);
    assert_true(first && first->meta && first->next && first->next->next);
    assert_string_equal(lyd_get_value(lyd_child(first)), "3");
    assert_string_equal(lyd_get_value(lyd_child(first->next)), "5");
    assert_string_equal(lyd_get_value(lyd_child(first->next->next)), "6");

    lyd_free_all(cont);
}

static void
test_lyd_dup_meta(void **state)
{
    const char *schema;
    struct lys_module *mod, *mod2;
    struct lyd_node *node, *sibl, *par, *par2;
    struct lyd_meta *meta, *meta2;
    struct ly_ctx *ctx2;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "leaf-list ll {type uint32;}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* metadata duplication in the same context */
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "1", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "2", 0, &sibl), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(sibl, node, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "3", 0, &par), LY_SUCCESS);
    assert_int_equal(lyd_dup_meta_single(node->meta, par, &meta), LY_SUCCESS);
    assert_non_null(node->meta);
    assert_string_equal(node->meta->name, META_NAME);
    assert_ptr_equal(meta->annotation, node->meta->annotation);
    assert_true(get_rbt(node->meta) && !get_rbt(meta));
    lyd_free_meta_single(meta);
    lyd_free_all(par);

    /* metadata duplication where parameters are from different contexts */
    assert_int_equal(ly_ctx_new(NULL, 0, &ctx2), LY_SUCCESS);
    assert_int_equal(ly_in_new_memory(schema, &UTEST_IN), LY_SUCCESS);
    assert_int_equal(lys_parse(ctx2, UTEST_IN, LYS_IN_YANG, NULL, &mod2), LY_SUCCESS);
    ly_in_free(UTEST_IN, 0), UTEST_IN = NULL;
    assert_int_equal(lyd_new_term(NULL, mod2, "ll", "1", 0, &par2), LY_SUCCESS);
    assert_int_equal(lyd_dup_meta_single_to_ctx(ctx2, node->meta, par2, &meta2), LY_SUCCESS);
    assert_ptr_not_equal(node->meta->annotation, meta2->annotation);
    assert_null(get_rbt(meta2));
    lyd_free_all(par2);
    ly_ctx_destroy(ctx2);

    lyd_free_all(node);
}

static void
test_insert_into_duplicate(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *node, *dup;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn { leaf-list ll {type uint32;}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "3", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, NULL), LY_SUCCESS);
    /* create duplicate */
    assert_int_equal(lyd_dup_single(cont, NULL, LYD_DUP_RECURSIVE, &dup), LY_SUCCESS);
    node = lyd_child(dup);
    assert_true(node && node->next && !get_rbt(node->meta));
    assert_string_equal(node->meta->name, META_NAME);
    /* insert into duplicate */
    assert_int_equal(lyd_new_term(dup, mod, "ll", "2", 0, NULL), LY_SUCCESS);
    assert_non_null(node->next->next);
    assert_string_equal(lyd_get_value(node), "1");
    assert_string_equal(lyd_get_value(node->next), "2");
    assert_string_equal(lyd_get_value(node->next->next), "3");
    assert_non_null(get_rbt(node->meta));
    lyd_free_all(cont);
    lyd_free_all(dup);
}

static void
test_option_dup_no_meta(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *dup, *node;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn { leaf-list ll {type uint32;}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "2", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "3", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_dup_siblings(cont, NULL, LYD_DUP_NO_META | LYD_DUP_RECURSIVE, &dup), LY_SUCCESS);
    node = lyd_child(dup);
    assert_true(node && !node->meta);
    assert_int_equal(lyd_new_term(dup, mod, "ll", "1", 0, NULL), LY_SUCCESS);
    node = lyd_child(dup);
    assert_non_null(node->meta && get_rbt(node->meta));
    assert_string_equal(node->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(node), "1");

    lyd_free_all(cont);
    lyd_free_all(dup);
}

static void
test_free_meta_single(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *first, *second, *third;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "leaf-list ll {type uint32;}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* user free metadata with lyds tree but it will created again */
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "7", 0, &first), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "8", 0, &second), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(first, second, NULL), LY_SUCCESS);
    assert_non_null(first->meta);
    assert_string_equal(first->meta->name, META_NAME);
    lyd_free_meta_single(first->meta);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "6", 0, &third), LY_SUCCESS);
    lyd_insert_sibling(first, third, NULL);
    assert_string_equal(lyd_get_value(first->prev), "6");
    assert_string_equal(lyd_get_value(first), "7");
    assert_string_equal(lyd_get_value(first->next), "8");
    assert_non_null(first->prev->meta);
    assert_string_equal(first->prev->meta->name, META_NAME);
    lyd_free_all(first->prev);
}

static void
test_no_metadata_remains(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *f1, *node, *f2, *dup;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "leaf-list ll {type uint32;}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* setup */
    /* create one node with metadata which contains the lyds tree */
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "1", 0, &f1), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "2", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(f1, node, NULL), LY_SUCCESS);
    lyd_unlink_tree(node);
    lyd_free_all(node);
    assert_non_null(get_rbt(f1->meta));
    assert_string_equal(f1->meta->name, META_NAME);
    /* do it again with another data tree */
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "3", 0, &f2), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "4", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(f2, node, NULL), LY_SUCCESS);
    lyd_unlink_tree(node);
    lyd_free_all(node);
    assert_non_null(get_rbt(f2->meta));
    assert_string_equal(f2->meta->name, META_NAME);
    /* also create a duplicate */
    lyd_dup_single(f2, NULL, 0, &dup);
    assert_true(dup->meta && !get_rbt(dup->meta));
    assert_string_equal(dup->meta->name, META_NAME);

    /* test: insert node which also has metadata */
    assert_int_equal(lyd_insert_sibling(f1, f2, NULL), LY_SUCCESS);
    /* inserted node no longer has metadata */
    assert_true(f1->next && !f1->next->meta);
    lyd_unlink_tree(f2);
    lyd_free_all(f2);

    /* test: insert duplicate node which also has metadata but no lyds_tree */
    assert_int_equal(lyd_insert_sibling(f1, dup, NULL), LY_SUCCESS);
    /* inserted node no longer has metadata */
    assert_true(f1->next && !f1->next->meta);
    lyd_unlink_tree(dup);
    lyd_free_all(dup);

    /* teardown */
    lyd_free_all(f1);
}

static void
test_insert_multiple_keys(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *node, *key;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn { list lst {key \"k1 k2 k3\";"
            "leaf k1 {type uint32;} leaf k2 {type uint32;} leaf k3 {type uint32;}}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);
    assert_int_equal(lyd_new_list(cont, mod, "lst", 0, NULL, "2", "0", "0"), LY_SUCCESS);
    assert_int_equal(lyd_new_list(cont, mod, "lst", 0, NULL, "1", "5", "0"), LY_SUCCESS);
    assert_int_equal(lyd_new_list(cont, mod, "lst", 0, NULL, "1", "5", "1"), LY_SUCCESS);
    assert_int_equal(lyd_new_list(cont, mod, "lst", 0, NULL, "1", "6", "0"), LY_SUCCESS);
    node = lyd_child(cont);
    assert_true(node && node->meta && node->next && node->next->next && node->next->next->next);
    assert_string_equal(node->meta->name, META_NAME);
    key = lyd_child(node);
    assert_string_equal(lyd_get_value(key), "1");
    assert_string_equal(lyd_get_value(key->next), "5");
    assert_string_equal(lyd_get_value(key->next->next), "0");
    key = lyd_child(node->next);
    assert_string_equal(lyd_get_value(key), "1");
    assert_string_equal(lyd_get_value(key->next), "5");
    assert_string_equal(lyd_get_value(key->next->next), "1");
    key = lyd_child(node->next->next);
    assert_string_equal(lyd_get_value(key), "1");
    assert_string_equal(lyd_get_value(key->next), "6");
    assert_string_equal(lyd_get_value(key->next->next), "0");
    key = lyd_child(node->next->next->next);
    assert_string_equal(lyd_get_value(key), "2");
    assert_string_equal(lyd_get_value(key->next), "0");
    assert_string_equal(lyd_get_value(key->next->next), "0");
    lyd_free_all(cont);
}

static void
test_merge_siblings(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *f1, *f2, *s1, *s2;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "leaf-list ll {type uint32;}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* both source and target trees have metadata */
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "1", 0, &f1), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "2", 0, &s1), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(f1, s1, NULL), LY_SUCCESS);
    assert_non_null(f1->meta);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "21", 0, &f2), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "22", 0, &s2), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(f2, s2, NULL), LY_SUCCESS);
    assert_non_null(f2->meta);
    assert_int_equal(lyd_merge_siblings(&f2, f1, 0), LY_SUCCESS);
    assert_true(f2->meta && f2->next && f2->next->next && f2->next->next->next);
    assert_string_equal(f2->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(f2), "1");
    assert_string_equal(lyd_get_value(f2->next), "2");
    assert_string_equal(lyd_get_value(f2->next->next), "21");
    assert_string_equal(lyd_get_value(f2->next->next->next), "22");
    lyd_free_all(f1);
    lyd_free_all(f2);

    /* only target tree have metadata */
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "1", 0, &f1), LY_SUCCESS);
    assert_null(f1->meta);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "21", 0, &f2), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "22", 0, &s2), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(f2, s2, NULL), LY_SUCCESS);
    assert_non_null(f2->meta);
    assert_int_equal(lyd_merge_siblings(&f2, f1, 0), LY_SUCCESS);
    assert_true(f2->meta && f2->next);
    assert_string_equal(f2->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(f2), "1");
    assert_string_equal(lyd_get_value(f2->next), "21");
    assert_string_equal(lyd_get_value(f2->next->next), "22");
    lyd_free_all(f1);
    lyd_free_all(f2);

    /* only source tree have metadata */
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "1", 0, &f1), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "2", 0, &s1), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(f1, s1, NULL), LY_SUCCESS);
    assert_non_null(f1->meta);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "21", 0, &f2), LY_SUCCESS);
    assert_null(f2->meta);
    assert_int_equal(lyd_merge_siblings(&f2, f1, 0), LY_SUCCESS);
    assert_true(f2->meta && f2->next && f2->next->next);
    assert_string_equal(f2->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(f2), "1");
    assert_string_equal(lyd_get_value(f2->next), "2");
    assert_string_equal(lyd_get_value(f2->next->next), "21");
    lyd_free_all(f1);
    lyd_free_all(f2);

    /* none have metadata */
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "1", 0, &f1), LY_SUCCESS);
    assert_null(f1->meta);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "21", 0, &f2), LY_SUCCESS);
    assert_null(f2->meta);
    assert_int_equal(lyd_merge_siblings(&f2, f1, 0), LY_SUCCESS);
    assert_true(f2->meta && f2->next);
    assert_string_equal(f2->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(f2), "1");
    assert_string_equal(lyd_get_value(f2->next), "21");
    lyd_free_all(f1);
    lyd_free_all(f2);
}

static void
test_merge_siblings_destruct(void **state)
{
    const char *schema, *data;
    struct lys_module *mod;
    struct lyd_node *dst, *src, *first;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "leaf-list ll {type uint32;}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* only source have metadata */
    data = "{\"a:ll\":[1,2,3,4]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, LYD_PARSE_ORDERED, LYD_VALIDATE_PRESENT, LY_SUCCESS, dst);
    assert_true(dst && !dst->meta);
    data = "{\"a:ll\":[21,22]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, src);
    assert_true(src && src->meta);
    assert_int_equal(lyd_merge_siblings(&dst, src, LYD_MERGE_DESTRUCT), LY_SUCCESS);
    assert_true(dst->meta && get_rbt(dst->meta));
    assert_string_equal(dst->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(dst), "1");
    assert_string_equal(lyd_get_value(dst->next), "2");
    assert_string_equal(lyd_get_value(dst->next->next), "3");
    assert_string_equal(lyd_get_value(dst->next->next->next), "4");
    assert_string_equal(lyd_get_value(dst->next->next->next->next), "21");
    assert_string_equal(lyd_get_value(dst->next->next->next->next->next), "22");
    lyd_free_all(dst);

    /* both source and target trees have metadata */
    data = "{\"a:ll\":[1,2,3]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, dst);
    assert_true(dst && dst->meta);
    data = "{\"a:ll\":[21,22,23]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, src);
    assert_true(src && src->meta);
    assert_int_equal(lyd_merge_siblings(&dst, src, LYD_MERGE_DESTRUCT), LY_SUCCESS);
    assert_true(dst->meta && get_rbt(dst->meta));
    assert_string_equal(dst->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(dst), "1");
    assert_string_equal(lyd_get_value(dst->next), "2");
    assert_string_equal(lyd_get_value(dst->next->next), "3");
    assert_string_equal(lyd_get_value(dst->next->next->next), "21");
    assert_string_equal(lyd_get_value(dst->next->next->next->next), "22");
    assert_string_equal(lyd_get_value(dst->next->next->next->next->next), "23");
    lyd_free_all(dst);

    schema = "module b {namespace urn:tests:b;prefix b;yang-version 1.1;revision 2014-05-08;"
            "container cont {"
            "  leaf-list ll {type uint32;}}"
            "}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* destination is empty, source have metadata */
    data = "{\"b:cont\": {} }";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, LYD_PARSE_ORDERED, LYD_VALIDATE_PRESENT, LY_SUCCESS, dst);
    assert_true(dst && !dst->meta);
    data = "{\"b:cont\": {\"ll\":[1,2,3]} }";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, src);
    assert_true(src && lyd_child(src)->meta);
    assert_int_equal(lyd_merge_siblings(&dst, src, LYD_MERGE_DESTRUCT), LY_SUCCESS);
    first = lyd_child(dst);
    assert_true(first->meta && get_rbt(first->meta));
    assert_string_equal(first->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(first), "1");
    assert_string_equal(lyd_get_value(first->next), "2");
    assert_string_equal(lyd_get_value(first->next->next), "3");
    lyd_free_all(dst);
}

static void
test_parse_data(void **state)
{
    const char *schema, *data;
    char *lyb_out;
    struct lys_module *mod;
    struct lyd_node *tree, *tree2;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "leaf-list ll {type uint32;}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* json */
    data = "{\"a:ll\":[2,1]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    assert_true(tree && tree->meta && tree->next);
    assert_string_equal(tree->meta->name, META_NAME);
    CHECK_LYD_VALUE(((struct lyd_node_term *)tree)->value, UINT32, "1", 1);
    lyd_free_all(tree);

    /* xml */
    data = "<ll xmlns=\"urn:tests:a\">2</ll>"
            "<ll xmlns=\"urn:tests:a\">1</ll>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    assert_true(tree && tree->meta && tree->next);
    assert_string_equal(tree->meta->name, META_NAME);
    CHECK_LYD_VALUE(((struct lyd_node_term *)tree)->value, UINT32, "1", 1);
    /* data tree is used in the next check */

    /* lyb */
    assert_int_equal(lyd_print_mem(&lyb_out, tree, LYD_LYB, LYD_PRINT_WITHSIBLINGS), 0);
    assert_int_equal(lyd_parse_data_mem(UTEST_LYCTX, lyb_out, LYD_LYB, LYD_PARSE_ONLY | LYD_PARSE_STRICT,
            0, &tree2), LY_SUCCESS);
    assert_true(tree2 && tree2->meta && tree2->next);
    assert_string_equal(tree2->meta->name, META_NAME);
    CHECK_LYD_VALUE(((struct lyd_node_term *)tree2)->value, UINT32, "1", 1);
    free(lyb_out);
    lyd_free_all(tree2);
    lyd_free_all(tree);
}

static void
test_parse_ordered_data(void **state)
{
    const char *schema, *data;
    struct lys_module *mod;
    struct lyd_node *tree, *node;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "leaf-list ll {type uint32;}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    data = "{\"a:ll\":[1,2,3]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, LYD_PARSE_ORDERED, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    assert_true(tree && !tree->meta && tree->next && tree->next->next);
    CHECK_LYD_VALUE(((struct lyd_node_term *)tree)->value, UINT32, "1", 1);

    /* unlink node with value 2 */
    node = tree->next;
    lyd_unlink_tree(node);
    assert_true(tree && !tree->meta && tree->next);
    lyd_free_all(node);

    /* unlink node with value 1 */
    node = tree;
    tree = tree->next;
    assert_null(tree->meta);
    lyd_unlink_tree(node);
    assert_true(tree && !tree->meta && !tree->next);

    /* insert node with value 1 to node with value 3 */
    lyd_insert_sibling(tree, node, NULL);
    tree = tree->prev;
    assert_true(tree && tree->meta && tree->next && !tree->next->meta && get_rbt(tree->meta));
    assert_string_equal(tree->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(tree), "1");
    assert_string_equal(lyd_get_value(tree->next), "3");

    lyd_free_all(tree);
}

static void
test_print_data(void **state)
{
    const char *schema, *exp;
    char *out;
    struct lys_module *mod;
    struct lyd_node *first, *node;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "leaf-list ll {type uint32;}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* lyds metadata must not be printed */

    /* json */
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "2", 0, &first), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "1", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(first, node, NULL), LY_SUCCESS);
    assert_int_equal(lyd_print_mem(&out, node, LYD_JSON, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK), 0);
    exp = "{\"a:ll\":[1,2]}";
    assert_string_equal(out, exp);
    free(out);

    /* xml */
    assert_int_equal(lyd_print_mem(&out, node, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK), 0);
    exp = "<ll xmlns=\"urn:tests:a\">1</ll><ll xmlns=\"urn:tests:a\">2</ll>";
    assert_string_equal(out, exp);
    free(out);

    lyd_free_all(first);
}

static void
test_manipulation_of_many_nodes(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *cont, *iter, *prev, *node;
    uint32_t i;
    char *data[] = {
        "q", "q", "n", "h", "c", "b", "h", "n", "p", "t",
        "p", "j", "c", "h", "n", "k", "n", "q", "p", "p",
        "s", "l", "p", "x", "h", "e", "i", "f", "u", "z",
        "l", "n", "o", "k", "n", "t", "w", "o", "d", "b",
        "k", "w", "w", "q", "e", "b", "x", "a", "g", "w",
        "b", "e", "p", "r", "s", "w", "u", "w", "d", "r",
    };
    uint32_t data_len = sizeof(data) / sizeof(data[0]);

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn { leaf-list ll {type string;}}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);

    /* insert nodes */
    for (i = 0; i < data_len; i++) {
        assert_int_equal(lyd_new_term(cont, mod, "ll", data[i], 0, NULL), LY_SUCCESS);
    }

    /* sort check */
    prev = lyd_child(cont);
    LY_LIST_FOR(prev->next, iter) {
        assert_true(lyd_get_value(prev)[0] <= lyd_get_value(iter)[0]);
        prev = iter;
    }

    /* remove every even node */
    LY_LIST_FOR(lyd_child(cont), iter) {
        node = iter->next;
        lyd_unlink_tree(node);
        lyd_free_tree(node);
    }
    data_len /= 2;

    /* sort check */
    prev = lyd_child(cont);
    i = 1;
    LY_LIST_FOR(prev->next, iter) {
        assert_true(lyd_get_value(prev)[0] <= lyd_get_value(iter)[0]);
        prev = iter;
        i++;
    }
    assert_int_equal(i, data_len);

    lyd_free_all(cont);
}

static void
test_lyds_free_metadata(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *first, *node;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "leaf-list ll {type uint32;}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    assert_int_equal(lyd_new_term(NULL, mod, "ll", "1", 0, &first), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "2", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(first, node, NULL), LY_SUCCESS);
    lyds_free_metadata(first);
    assert_null(first->meta);
    assert_null(node->meta);

    lyd_free_all(first);
}

static void
test_move_whole_list(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *src, *dst, *node, *first;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "leaf-list tll {type string;}"
            "container cn {"
            "  leaf head {type string;}"
            "  leaf-list ll {type string;}"
            "  leaf tail {type string;}"
            "}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* move into the void: top-level, nothing will happen */
    assert_int_equal(lyd_new_term(NULL, mod, "tll", "1", 0, &src), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "tll", "2", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(src, node, NULL), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(NULL, src, &first), LY_SUCCESS);
    assert_true(src && src->meta && get_rbt(src->meta) && src == first);
    assert_string_equal(src->meta->name, META_NAME);
    lyd_free_all(src);

    /* move to childless parent: lyd_insert_child */
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &src), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "1", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "2", 0, &node), LY_SUCCESS);
    first = lyd_child(src);
    assert_int_equal(lyd_unlink_siblings(first), LY_SUCCESS);
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &dst), LY_SUCCESS);
    assert_int_equal(lyd_insert_child(dst, first), LY_SUCCESS);
    first = lyd_child(dst);
    assert_true(first && first->meta && get_rbt(first->meta) && first->next);
    assert_string_equal(first->meta->name, META_NAME);
    assert_true(!lyd_child(src));
    lyd_free_all(src);
    lyd_free_all(dst);

    /* move before the anchor */
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &src), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "1", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "2", 0, &node), LY_SUCCESS);
    first = lyd_child(src);
    assert_int_equal(lyd_unlink_siblings(lyd_child(src)), LY_SUCCESS);
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &dst), LY_SUCCESS);
    assert_int_equal(lyd_new_term(dst, mod, "tail", "a", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(lyd_child(dst), first, NULL), LY_SUCCESS);
    first = lyd_child(dst);
    assert_true(first && first->meta && get_rbt(first->meta) && first->next);
    assert_string_equal(first->meta->name, META_NAME);
    assert_true(!lyd_child(src));
    lyd_free_all(src);
    lyd_free_all(dst);

    /* move to the last position */
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &src), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "1", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "2", 0, &node), LY_SUCCESS);
    first = lyd_child(src);
    assert_int_equal(lyd_unlink_siblings(lyd_child(src)), LY_SUCCESS);
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &dst), LY_SUCCESS);
    assert_int_equal(lyd_new_term(dst, mod, "head", "a", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(lyd_child(dst), first, NULL), LY_SUCCESS);
    first = lyd_child(dst);
    assert_true(first && first->next);
    node = first->next;
    assert_true(node && node->meta && get_rbt(node->meta) && node->next);
    assert_string_equal(node->meta->name, META_NAME);
    assert_true(!lyd_child(src));
    lyd_free_all(src);
    lyd_free_all(dst);
}

static void
test_move_part_list(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyd_node *src, *dst, *node, *first;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "leaf-list tll {type string;}"
            "container cn {"
            "  leaf head {type string;}"
            "  leaf-list ll {type string;}"
            "  leaf tail {type string;}"
            "}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* move into the void: top-level, first node is unlinked */
    assert_int_equal(lyd_new_term(NULL, mod, "tll", "1", 0, &src), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "tll", "2", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(src, node, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(NULL, mod, "tll", "3", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(src, node, NULL), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(NULL, src->next, &first), LY_SUCCESS);
    assert_true(src && src->meta && get_rbt(src->meta));
    assert_string_equal(src->meta->name, META_NAME);
    assert_true(src && src->next);
    assert_string_equal(lyd_get_value(first), "2");
    lyd_free_all(src);
    lyd_free_all(first);

    /* move into the void: inner node, node is unlinked */
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &src), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "1", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "2", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "3", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(NULL, lyd_child(src), &dst), LY_SUCCESS);
    first = lyd_child(src);
    assert_true(first && first->meta && get_rbt(first->meta) && first->next);
    assert_string_equal(first->meta->name, META_NAME);
    assert_true(dst && !dst->meta && !dst->next);
    lyd_free_all(src);
    lyd_free_all(dst);

    /* move to childless parent: lyd_insert_child */
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &src), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "1", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "2", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "3", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &dst), LY_SUCCESS);
    assert_int_equal(lyd_insert_child(dst, lyd_child(src)), LY_SUCCESS);
    first = lyd_child(src);
    assert_true(first && first->meta && get_rbt(first->meta) && first->next);
    assert_string_equal(first->meta->name, META_NAME);
    first = lyd_child(dst);
    assert_true(first && !first->meta);
    lyd_free_all(src);
    lyd_free_all(dst);

    /* move before the anchor */
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &src), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "1", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "2", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "3", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &dst), LY_SUCCESS);
    assert_int_equal(lyd_new_term(dst, mod, "tail", "a", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(lyd_child(dst), lyd_child(src), NULL), LY_SUCCESS);
    first = lyd_child(src);
    assert_true(first && first->meta && get_rbt(first->meta) && first->next);
    assert_string_equal(first->meta->name, META_NAME);
    first = lyd_child(dst);
    assert_true(first && !first->meta && first->next);
    assert_string_equal(lyd_get_value(first), "1");
    lyd_free_all(src);
    lyd_free_all(dst);

    /* move to the last position */
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &src), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "1", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "2", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_new_term(src, mod, "ll", "3", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &dst), LY_SUCCESS);
    assert_int_equal(lyd_new_term(dst, mod, "head", "a", 0, &node), LY_SUCCESS);
    assert_int_equal(lyd_insert_sibling(lyd_child(dst), lyd_child(src), NULL), LY_SUCCESS);
    first = lyd_child(src);
    assert_true(first && first->meta && get_rbt(first->meta) && first->next);
    assert_string_equal(first->meta->name, META_NAME);
    first = lyd_child(dst);
    assert_true(first && first->next);
    node = first->next;
    assert_true(node && !node->meta && !node->next);
    assert_string_equal(lyd_get_value(node), "1");
    lyd_free_all(src);
    lyd_free_all(dst);
}

static void
test_merge_whole_list(void **state)
{
    const char *schema, *data;
    char *check[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
    struct lys_module *mod;
    struct lyd_node *src, *dst, *first, *iter;
    int i;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "leaf-list ll {type uint32;}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* src with LYD_PARSE_ORDERED and dst with LYD_PARSE_ORDERED */
    data = "{\"a:ll\":[1,2,5,6,9,10]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, LYD_PARSE_ORDERED, LYD_VALIDATE_PRESENT, LY_SUCCESS, src);
    assert_true(src && !src->meta);
    data = "{\"a:ll\":[3,4,7,8]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, LYD_PARSE_ORDERED, LYD_VALIDATE_PRESENT, LY_SUCCESS, dst);
    assert_true(dst && !dst->meta);
    assert_int_equal(lyd_insert_sibling(dst, src, &first), LY_SUCCESS);
    assert_true(first->meta && get_rbt(first->meta));
    assert_string_equal(first->meta->name, META_NAME);
    i = 0;
    LY_LIST_FOR(first, iter) {
        assert_string_equal(lyd_get_value(iter), check[i]);
        ++i;
    }
    lyd_free_all(dst);

    /* src with LYD_PARSE_ORDERED dst with lyds_tree */
    data = "{\"a:ll\":[1,2,5,6,9,10]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, LYD_PARSE_ORDERED, LYD_VALIDATE_PRESENT, LY_SUCCESS, src);
    assert_true(src && !src->meta);
    data = "{\"a:ll\":[3,4,7,8]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, dst);
    assert_true(dst && dst->meta);
    assert_string_equal(dst->meta->name, META_NAME);
    assert_int_equal(lyd_insert_sibling(dst, src, &first), LY_SUCCESS);
    assert_true(first->meta && get_rbt(first->meta) && !first->next->next->meta);
    assert_string_equal(first->meta->name, META_NAME);
    i = 0;
    LY_LIST_FOR(first, iter) {
        assert_string_equal(lyd_get_value(iter), check[i]);
        ++i;
    }
    lyd_free_all(dst);

    /* src with lyds_tree and dst with LYD_PARSE_ORDERED */
    data = "{\"a:ll\":[1,2,5,6,9,10]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, src);
    assert_true(src && src->meta);
    assert_string_equal(src->meta->name, META_NAME);
    data = "{\"a:ll\":[3,4,7,8]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, LYD_PARSE_ORDERED, LYD_VALIDATE_PRESENT, LY_SUCCESS, dst);
    assert_true(dst && !dst->meta);
    assert_int_equal(lyd_insert_sibling(dst, src, &first), LY_SUCCESS);
    assert_true(first->meta && get_rbt(first->meta));
    assert_string_equal(first->meta->name, META_NAME);
    i = 0;
    LY_LIST_FOR(first, iter) {
        assert_string_equal(lyd_get_value(iter), check[i]);
        ++i;
    }
    lyd_free_all(dst);

    /* src with lyds_tree and dst with lyds_tree */
    data = "{\"a:ll\":[1,2,5,6,9,10]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, src);
    assert_true(src && src->meta);
    assert_string_equal(src->meta->name, META_NAME);
    data = "{\"a:ll\":[3,4,7,8]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, dst);
    assert_true(dst && dst->meta);
    assert_string_equal(dst->meta->name, META_NAME);
    assert_int_equal(lyd_insert_sibling(dst, src, &first), LY_SUCCESS);
    assert_true(first->meta && get_rbt(first->meta));
    assert_string_equal(first->meta->name, META_NAME);
    i = 0;
    LY_LIST_FOR(first, iter) {
        assert_string_equal(lyd_get_value(iter), check[i]);
        ++i;
    }
    lyd_free_all(dst);
}

static void
test_unlink_siblings(void **state)
{
    const char *schema, *data;
    struct lys_module *mod;
    struct lyd_node *cont, *node, *first;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "container cn {"
            "  leaf-list ll {type uint32;}"
            "  leaf-list tail {type string;}"
            "}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "3", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "2", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "tail", "a", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "tail", "b", 0, NULL), LY_SUCCESS);

    node = lyd_child(cont)->next;
    assert_int_equal(lyd_unlink_siblings(node), LY_SUCCESS);
    first = lyd_child(cont);
    assert_true(first && first->meta && !first->next);
    assert_string_equal(first->meta->name, META_NAME);
    assert_string_equal(lyd_get_value(first), "1");
    assert_true(node && !node->meta && node->next && node->next->next && node->next->next->next);
    assert_string_equal(lyd_get_value(node), "2");
    assert_string_equal(lyd_get_value(node->next), "3");

    lyd_free_all(node);
    lyd_free_all(cont);

    /* call lyds_split but no metadata is set */
    data = "{\"a:cn\": {\"ll\":[1,2,3,4]} }";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, LYD_PARSE_ORDERED, LYD_VALIDATE_PRESENT, LY_SUCCESS, cont);
    first = lyd_child(cont);
    assert_true(!first->meta);
    node = lyd_child(cont)->next;
    lyd_unlink_siblings(node);
    assert_true(node && !node->meta && node->next);
    assert_string_equal(lyd_get_value(node), "2");
    assert_string_equal(lyd_get_value(node->next), "3");
    assert_string_equal(lyd_get_value(node->next->next), "4");
    lyd_free_all(node);
    lyd_free_all(cont);

    /* call lyds_split, metadata is set, two items  */
    assert_int_equal(lyd_new_inner(NULL, mod, "cn", 0, &cont), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "1", 0, NULL), LY_SUCCESS);
    assert_int_equal(lyd_new_term(cont, mod, "ll", "2", 0, NULL), LY_SUCCESS);
    first = lyd_child(cont);
    assert_true(first->meta);
    node = lyd_child(cont)->next;
    lyd_unlink_siblings(node);
    assert_true(node && !node->meta);
    assert_string_equal(lyd_get_value(node), "2");
    lyd_free_all(node);
    lyd_free_all(cont);
}

static void
test_order_violation(void **state)
{
    const char *schema, *data;
    struct lys_module *mod;
    struct lyd_node *tree, *node, *dst, *first;

    schema = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;revision 2014-05-08;"
            "leaf-list ll {type uint32;}}";
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    /* inserting a new node causes the nodes to be sorted */
    data = "{\"a:ll\":[1,8,2]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, LYD_PARSE_ORDERED, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    assert_true(tree && !tree->meta && tree->next && tree->next->next);
    assert_string_equal(lyd_get_value(tree), "1");
    assert_string_equal(lyd_get_value(tree->next), "8");
    assert_string_equal(lyd_get_value(tree->next->next), "2");
#ifndef NDEBUG
    CHECK_LOG_CTX("Data in \"ll\" are not sorted, inserted node should not be added to the end.", NULL, 0);
#endif
    assert_int_equal(lyd_new_term(NULL, mod, "ll", "3", 0, &node), LY_SUCCESS);
    lyd_insert_sibling(tree, node, NULL);
    assert_string_equal(lyd_get_value(tree), "1");
    assert_string_equal(lyd_get_value(tree->next), "2");
    assert_string_equal(lyd_get_value(tree->next->next), "3");
    assert_string_equal(lyd_get_value(tree->next->next->next), "8");
    lyd_free_all(tree);

    /* move unsorted nodes causes the nodes to be sorted */
    data = "{\"a:ll\":[1,8,2]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, LYD_PARSE_ORDERED, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
#ifndef NDEBUG
    CHECK_LOG_CTX("Data in \"ll\" are not sorted, inserted node should not be added to the end.", NULL, 0);
#endif
    data = "{\"a:ll\":[7,5]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, LYD_PARSE_ORDERED, LYD_VALIDATE_PRESENT, LY_SUCCESS, dst);
#ifndef NDEBUG
    CHECK_LOG_CTX("Data in \"ll\" are not sorted, inserted node should not be added to the end.", NULL, 0);
#endif
    lyd_insert_sibling(dst, tree, &first);
    assert_string_equal(lyd_get_value(first), "1");
    assert_string_equal(lyd_get_value(first->next), "2");
    assert_string_equal(lyd_get_value(first->next->next), "5");
    assert_string_equal(lyd_get_value(first->next->next->next), "7");
    assert_string_equal(lyd_get_value(first->next->next->next->next), "8");
    lyd_free_all(first);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_insert_top_level_list),
        UTEST(test_insert_top_level_leaflist),
        UTEST(test_insert_cont_list),
        UTEST(test_insert_cont_leaflist),
        UTEST(test_try_user_order_func),
        UTEST(test_ordered_by_user),
        UTEST(test_remove),
        UTEST(test_remove_then_insert),
        UTEST(test_unlink_all),
        UTEST(test_insert_before_anchor),
        UTEST(test_insert_after_anchor),
        UTEST(test_insert_same_values_leaflist),
        UTEST(test_insert_same_values_list),
        UTEST(test_remove_same_values_leaflist),
        UTEST(test_insert_keyless_list),
        UTEST(test_leaflist_default),
        UTEST(test_unlink_then_insert),
        UTEST(test_change_term),
        UTEST(test_change_key),
        UTEST(test_lyd_dup_meta),
        UTEST(test_insert_into_duplicate),
        UTEST(test_option_dup_no_meta),
        UTEST(test_no_metadata_remains),
        UTEST(test_free_meta_single),
        UTEST(test_insert_multiple_keys),
        UTEST(test_merge_siblings),
        UTEST(test_merge_siblings_destruct),
        UTEST(test_parse_data),
        UTEST(test_parse_ordered_data),
        UTEST(test_print_data),
        UTEST(test_manipulation_of_many_nodes),
        UTEST(test_lyds_free_metadata),
        UTEST(test_move_whole_list),
        UTEST(test_move_part_list),
        UTEST(test_merge_whole_list),
        UTEST(test_unlink_siblings),
        UTEST(test_order_violation),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
