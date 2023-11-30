#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "utils.h"

struct item {
	const char *name;
	struct list_head list;
};

#define OUT(fmt, ...) do { \
	fprintf(stdout, "%s: " fmt, __func__, ## __VA_ARGS__); \
} while (0);

static void init_list(struct list_head *list)
{
	const char *vals[] = {
		"zero", "one", "two", "three", "four", "five", "six",
		"seven", "eight", "nine", "ten", "eleven", "twelve"
	};

	OUT("list_empty: %s\n", list_empty(list) ? "yes" : "no");
	OUT("list_add_tail: ");
	for (size_t i=0; i<ARRAY_SIZE(vals); i++) {
		struct item *e = malloc(sizeof(struct item));
		e->name = vals[i];
		list_add_tail(&e->list, list);
		fprintf(stdout, "%s ", vals[i]);
	}
	fprintf(stdout, "\n");
	OUT("list_empty: %s\n", list_empty(list) ? "yes" : "no");
}

static void test_basics()
{
	struct item *tmp;
	struct item *item;
	struct item *last;
	struct item *first;
	struct list_head test_list = LIST_HEAD_INIT(test_list);

	init_list(&test_list);

	first = list_first_entry(&test_list, struct item, list);
	last = list_last_entry(&test_list, struct item, list);
	OUT("first=%s last=%s\n", first->name, last->name);
	OUT("'zero' is first, %s\n", list_is_first(&first->list, &test_list) ? "yes" : "no");
	OUT("'twelve' is last, %s\n", list_is_last(&last->list, &test_list) ? "yes" : "no");

	OUT("removing 'twelve' and 'zero'\n");
	list_del(&first->list);
	list_del(&last->list);
	free(first);
	free(last);

	first = list_first_entry(&test_list, struct item, list);
	last = list_last_entry(&test_list, struct item, list);

	if (!first || !last)
		return;

	OUT("first=%s last=%s\n", first->name, last->name);
	OUT("'one' is first, %s\n", list_is_first(&first->list, &test_list) ? "yes" : "no");
	OUT("'eleven' is last, %s\n", list_is_last(&last->list, &test_list) ? "yes" : "no");

	OUT("moving 'one' to the tail\n");
	list_move_tail(&first->list, &test_list);
	first = list_first_entry(&test_list, struct item, list);
	last = list_last_entry(&test_list, struct item, list);
	OUT("first=%s last=%s\n", first->name, last->name);
	OUT("'two' is first, %s\n", list_is_first(&first->list, &test_list) ? "yes" : "no");
	OUT("'one' is last, %s\n", list_is_last(&last->list, &test_list) ? "yes" : "no");

	OUT("list_for_each_entry: ");
	list_for_each_entry(item, &test_list, list) {
		fprintf(stdout, "%s ", item->name);
	}
	fprintf(stdout, "\n");

	OUT("list_for_each_entry_reverse: ");
	list_for_each_entry_reverse(item, &test_list, list) {
		fprintf(stdout, "%s ", item->name);
	}
	fprintf(stdout, "\n");

	OUT("delete all entries\n");
	list_for_each_entry_safe(item, tmp, &test_list, list) {
		list_del(&item->list);
		free(item);
	}
	OUT("list_empty: %s\n", list_empty(&test_list) ? "yes" : "no");
}

static void test_while_list_empty()
{
	struct item *first;
	struct list_head test_list = LIST_HEAD_INIT(test_list);

	init_list(&test_list);

	OUT("delete all entries\n");
	while (!list_empty(&test_list)) {
		first = list_first_entry(&test_list, struct item, list);
		list_del(&first->list);
		free(first);
	}
	OUT("list_empty: %s\n", list_empty(&test_list) ? "yes" : "no");
}

int main()
{
	test_basics();
	test_while_list_empty();
	return 0;
}
