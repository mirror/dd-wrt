#include "framework.h"

/*----------------------------------------------------------------
 * Assertions
 *--------------------------------------------------------------*/

jmp_buf test_k;
#define TEST_FAILED 1

void test_fail(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");

	longjmp(test_k, TEST_FAILED);
}

struct test_suite *test_suite_create(void *(*fixture_init)(void),
				     void (*fixture_exit)(void *))
{
	struct test_suite *ts = malloc(sizeof(*ts));
	if (ts) {
		ts->fixture_init = fixture_init;
		ts->fixture_exit = fixture_exit;
		dm_list_init(&ts->tests);
	}

	return ts;
}

void test_suite_destroy(struct test_suite *ts)
{
	struct test_details *td, *tmp;

	dm_list_iterate_items_safe (td, tmp, &ts->tests) {
		dm_list_del(&td->list);
		free(td);
	}

	free(ts);
}

bool register_test(struct test_suite *ts,
		   const char *path, const char *desc,
		   void (*fn)(void *))
{
	struct test_details *t = malloc(sizeof(*t));
	if (!t) {
		fprintf(stderr, "out of memory\n");
		return false;
	}

	t->parent = ts;
	t->path = path;
	t->desc = desc;
	t->fn = fn;
	dm_list_add(&ts->tests, &t->list);

	return true;
}

//-----------------------------------------------------------------
