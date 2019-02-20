/*
 * Copyright (C) 2010 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "units.h"
#include "device_mapper/all.h"

static void *_mem_init(void)
{
	struct dm_pool *mem = dm_pool_create("config test", 1024);
	if (!mem) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	return mem;
}

static void _mem_exit(void *mem)
{
	dm_pool_destroy(mem);
}

static const char *conf =
	"id = \"yada-yada\"\n"
	"seqno = 15\n"
	"status = [\"READ\", \"WRITE\"]\n"
	"flags = []\n"
	"extent_size = 8192\n"
	"physical_volumes {\n"
	"    pv0 {\n"
	"        id = \"abcd-efgh\"\n"
	"    }\n"
	"    pv1 {\n"
	"        id = \"bbcd-efgh\"\n"
	"    }\n"
	"    pv2 {\n"
	"        id = \"cbcd-efgh\"\n"
	"    }\n"
	"}\n";

static const char *overlay =
	"id = \"yoda-soda\"\n"
	"flags = [\"FOO\"]\n"
	"physical_volumes {\n"
	"    pv1 {\n"
	"        id = \"hgfe-dcba\"\n"
	"    }\n"
	"    pv3 {\n"
	"        id = \"dbcd-efgh\"\n"
	"    }\n"
	"}\n";

static void test_parse(void *fixture)
{
	struct dm_config_tree *tree = dm_config_from_string(conf);
	const struct dm_config_value *value;

	T_ASSERT((long) tree);
	T_ASSERT(dm_config_has_node(tree->root, "id"));
	T_ASSERT(dm_config_has_node(tree->root, "physical_volumes"));
	T_ASSERT(dm_config_has_node(tree->root, "physical_volumes/pv0"));
	T_ASSERT(dm_config_has_node(tree->root, "physical_volumes/pv0/id"));

	T_ASSERT(!strcmp(dm_config_find_str(tree->root, "id", "foo"), "yada-yada"));
	T_ASSERT(!strcmp(dm_config_find_str(tree->root, "idt", "foo"), "foo"));

	T_ASSERT(!strcmp(dm_config_find_str(tree->root, "physical_volumes/pv0/bb", "foo"), "foo"));
	T_ASSERT(!strcmp(dm_config_find_str(tree->root, "physical_volumes/pv0/id", "foo"), "abcd-efgh"));

	T_ASSERT(!dm_config_get_uint32(tree->root, "id", NULL));
	T_ASSERT(dm_config_get_uint32(tree->root, "extent_size", NULL));

	/* FIXME: Currently everything parses as a list, even if it's not */
	// T_ASSERT(!dm_config_get_list(tree->root, "id", NULL));
	// T_ASSERT(!dm_config_get_list(tree->root, "extent_size", NULL));

	T_ASSERT(dm_config_get_list(tree->root, "flags", &value));
	T_ASSERT(value->next == NULL); /* an empty list */
	T_ASSERT(dm_config_get_list(tree->root, "status", &value));
	T_ASSERT(value->next != NULL); /* a non-empty list */

	dm_config_destroy(tree);
}

static void test_clone(void *fixture)
{
	struct dm_config_tree *tree = dm_config_from_string(conf);
	struct dm_config_node *n = dm_config_clone_node(tree, tree->root, 1);
	const struct dm_config_value *value;

	/* Check that the nodes are actually distinct. */
	T_ASSERT(n != tree->root);
	T_ASSERT(n->sib != tree->root->sib);
	T_ASSERT(dm_config_find_node(n, "physical_volumes") != NULL);
	T_ASSERT(dm_config_find_node(tree->root, "physical_volumes") != NULL);
	T_ASSERT(dm_config_find_node(n, "physical_volumes") != dm_config_find_node(tree->root, "physical_volumes"));

	T_ASSERT(dm_config_has_node(n, "id"));
	T_ASSERT(dm_config_has_node(n, "physical_volumes"));
	T_ASSERT(dm_config_has_node(n, "physical_volumes/pv0"));
	T_ASSERT(dm_config_has_node(n, "physical_volumes/pv0/id"));

	T_ASSERT(!strcmp(dm_config_find_str(n, "id", "foo"), "yada-yada"));
	T_ASSERT(!strcmp(dm_config_find_str(n, "idt", "foo"), "foo"));

	T_ASSERT(!strcmp(dm_config_find_str(n, "physical_volumes/pv0/bb", "foo"), "foo"));
	T_ASSERT(!strcmp(dm_config_find_str(n, "physical_volumes/pv0/id", "foo"), "abcd-efgh"));

	T_ASSERT(!dm_config_get_uint32(n, "id", NULL));
	T_ASSERT(dm_config_get_uint32(n, "extent_size", NULL));

	/* FIXME: Currently everything parses as a list, even if it's not */
	// T_ASSERT(!dm_config_get_list(tree->root, "id", NULL));
	// T_ASSERT(!dm_config_get_list(tree->root, "extent_size", NULL));

	T_ASSERT(dm_config_get_list(n, "flags", &value));
	T_ASSERT(value->next == NULL); /* an empty list */
	T_ASSERT(dm_config_get_list(n, "status", &value));
	T_ASSERT(value->next != NULL); /* a non-empty list */

	dm_config_destroy(tree);
}

static void test_cascade(void *fixture)
{
	struct dm_config_tree *t1 = dm_config_from_string(conf),
		              *t2 = dm_config_from_string(overlay),
		              *tree = dm_config_insert_cascaded_tree(t2, t1);

	T_ASSERT(!strcmp(dm_config_tree_find_str(tree, "id", "foo"), "yoda-soda"));
	T_ASSERT(!strcmp(dm_config_tree_find_str(tree, "idt", "foo"), "foo"));

	T_ASSERT(!strcmp(dm_config_tree_find_str(tree, "physical_volumes/pv0/bb", "foo"), "foo"));
	T_ASSERT(!strcmp(dm_config_tree_find_str(tree, "physical_volumes/pv1/id", "foo"), "hgfe-dcba"));
	T_ASSERT(!strcmp(dm_config_tree_find_str(tree, "physical_volumes/pv3/id", "foo"), "dbcd-efgh"));

	dm_config_destroy(t1);
	dm_config_destroy(t2);
}

#define T(path, desc, fn) register_test(ts, "/metadata/config/" path, desc, fn)

void config_tests(struct dm_list *all_tests)
{
	struct test_suite *ts = test_suite_create(_mem_init, _mem_exit);
	if (!ts) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	T("parse", "parsing various", test_parse);
	T("clone", "duplicating a config tree", test_clone);
	T("cascade", "cascade", test_cascade);

	dm_list_add(all_tests, &ts->list);
};
