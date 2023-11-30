#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "avl.h"
#include "avl-cmp.h"
#include "utils.h"

#define OUT(fmt, ...) do { \
	fprintf(stdout, "%s: " fmt, __func__, ## __VA_ARGS__); \
} while (0);

struct node {
	struct avl_node avl;
};

static void test_basics()
{
	size_t i;
	struct avl_tree t;
	struct node *temp;
	struct node *elem;
	struct node *last;
	struct node *first;
	const char *vals[] = {
		"zero", "one", "two", "three", "four", "five", "six",
		"seven", "eight", "nine", "ten", "eleven", "twelve"
	};

	avl_init(&t, avl_strcmp, false, NULL);

	OUT("insert: ");
	for (i=0; i<ARRAY_SIZE(vals); i++) {
		struct node *n = malloc(sizeof(struct node));
		n->avl.key = vals[i];

		int r = avl_insert(&t, &n->avl);
		fprintf(stdout, "%d=%s ", r, (char *)n->avl.key);
	}
	fprintf(stdout, "\n");

	OUT("insert duplicate: ");
	for (i=0; i<ARRAY_SIZE(vals); i++) {
		struct node *n = malloc(sizeof(struct node));
		n->avl.key = vals[i];

		int r = avl_insert(&t, &n->avl);
		fprintf(stdout, "%d=%s ", r, (char *)n->avl.key);

		if (r)
			free(n);
	}
	fprintf(stdout, "\n");

	first = avl_first_element(&t, first, avl);
	last = avl_last_element(&t, last, avl);
	OUT("first=%s last=%s\n", (char*)first->avl.key, (char*)last->avl.key);

	OUT("for each element: ");
	avl_for_each_element(&t, elem, avl) {
		fprintf(stdout, "%s ", (char*)elem->avl.key);
	}
	fprintf(stdout, "\n");

	OUT("delete 'one' element\n");
	elem = avl_find_element(&t, "one", elem, avl);
	avl_delete(&t, &elem->avl);
	free(elem);

	OUT("for each element reverse: ");
	avl_for_each_element_reverse(&t, elem, avl) {
		fprintf(stdout, "%s ", (char*)elem->avl.key);
	}
	fprintf(stdout, "\n");

	OUT("delete all elements\n");
	avl_for_each_element_safe(&t, elem, avl, temp) {
		avl_delete(&t, &elem->avl);
		free(elem);
	}
}

int main()
{
	test_basics();
	return 0;
}
