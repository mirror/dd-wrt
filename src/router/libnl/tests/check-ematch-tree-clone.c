#include <netlink-private/types.h>
#include <netlink/route/cls/ematch.h>

#include <linux/netlink.h>

#include <stdio.h>
#include <time.h>

#include <check.h>
#include "util.h"

#define MAX_DEPTH		6
#define MAX_CHILDREN		5

static int current_depth = 0;
static int id = 1;
static long long array_size = 0;

static int *src_result = NULL, *dst_result = NULL;

static long long my_pow(long long x, long long y)
{
	int ret = x;

	if (y == 0)
		return 1;

	if (y < 0 || x == 0)
		return 0;

	while(--y) {
		ret *= x;
	}

	return ret;
}

static long int generate_random(long int max)
{
	srandom(time(NULL) + id);
	return (random() % max);
}

static int build_children(struct nl_list_head *parent)
{
	int i, num = 0;
	struct rtnl_ematch *child = NULL;

	if (!parent)
		return 0;

	if (++current_depth > MAX_DEPTH) {
		--current_depth;
		return 0;
	}

	num = generate_random(MAX_CHILDREN + 1);
	for (i = 0; i < num; ++i) {
		child = rtnl_ematch_alloc();
		if (!child) {
			printf("Mem alloc error\n");
			exit(1);
		}
		build_children(&child->e_childs);
		child->e_id = id++;
		nl_list_add_tail(&child->e_list, parent);
	}

	--current_depth;
	return 0;
}

static void build_src_cgroup(struct rtnl_ematch_tree *tree)
{
	build_children(&tree->et_list);
}

static void dump_ematch_list(struct nl_list_head *head, int *result, int *index)
{
	struct rtnl_ematch *pos = NULL;

	nl_list_for_each_entry(pos, head, e_list) {
		if (!nl_list_empty(&pos->e_childs))
			dump_ematch_list(&pos->e_childs, result, index);
		result[*index] = pos->e_id;
		(*index)++;
	}
}

static void dump_ematch_tree(struct rtnl_ematch_tree *tree, int *result, int *index)
{
	if (!tree)
		return;

	dump_ematch_list(&tree->et_list, result, index);
}

static int compare(int *r1, int *r2, int len)
{
	int i = 0;
	for (i = 0; i < len; ++i) {
		if (r1[i] != r2[i])
			return -1;
	}
	return 0;
}

START_TEST(ematch_tree_clone)
{
	struct rtnl_ematch_tree *src = NULL, *dst = NULL;
	int i = 0, j = 0;

	array_size = (MAX_DEPTH * my_pow(MAX_CHILDREN, MAX_DEPTH)) / 2;
	src_result = calloc(4, array_size);
	dst_result = calloc(4, array_size);

	src = rtnl_ematch_tree_alloc(2);

	build_src_cgroup(src);
	dump_ematch_tree(src, src_result, &i);

	dst = rtnl_ematch_tree_clone(src);
	dump_ematch_tree(dst, dst_result, &j);

	fail_if(!dst);
	fail_if(i != j);
	fail_if(compare(src_result, dst_result, i));

	free(src_result);
	free(dst_result);
}
END_TEST

Suite *make_nl_ematch_tree_clone_suite(void)
{
	Suite *suite = suite_create("Clone ematch tree");

	TCase *ematch_tree = tcase_create("Core");
	tcase_add_test(ematch_tree, ematch_tree_clone);
	suite_add_tcase(suite, ematch_tree);

	return suite;
}
