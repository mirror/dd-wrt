#include "units.h"

#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <unistd.h>

//-----------------------------------------------------------------

#define MAX_COMPONENTS 16

struct token {
	const char *b, *e;
};

static bool _pop_component(const char *path, struct token *result)
{
	const char *b, *e;

	while (*path && *path == '/')
		path++;

	b = path;
	while (*path && (*path != '/'))
		path++;
	e = path;

	if (b == e)
		return false;

	result->b = b;
	result->e = e;
	return true;
}

static unsigned _split_components(const char *path, struct token *toks, unsigned len)
{
	unsigned count = 0;
	struct token tok;
	tok.e = path;

	while (len && _pop_component(tok.e, &tok)) {
		*toks = tok;
		toks++;
		count++;
		len--;
	}

	return count;
}

static void _indent(FILE *stream, unsigned count)
{
	unsigned i;

	for (i = 0; i < count; i++)
		fprintf(stream, "  ");
}

static void _print_token(FILE *stream, struct token *t)
{
	const char *ptr;

	for (ptr = t->b; ptr != t->e; ptr++)
		fprintf(stream, "%c", *ptr);
}

static int _char_cmp(char l, char r)
{
	if (l < r)
		return -1;

	else if (r < l)
		return 1;

	else
		return 0;
}

static int _tok_cmp(struct token *lhs, struct token *rhs)
{
	const char *l = lhs->b, *le = lhs->e;
	const char *r = rhs->b, *re = rhs->e;

	while ((l != le) && (r != re) && (*l == *r)) {
		l++;
		r++;
	}

	if ((l != le) && (r != re))
		return _char_cmp(*l, *r);

	else if (r != re)
		return -1;

	else if (l != le)
		return 1;

	else
		return 0;
}

static void _print_path_delta(FILE *stream,
			      struct token *old, unsigned old_len,
			      struct token *new, unsigned new_len,
			      const char *desc)
{
	unsigned i, common_prefix = 0, len, d;
	unsigned max_prefix = old_len < new_len ? old_len : new_len;

	for (i = 0; i < max_prefix; i++) {
		if (_tok_cmp(old + i, new + i))
			break;
		else
			common_prefix++;
	}

	for (; i < new_len; i++) {
		_indent(stream, common_prefix);
		_print_token(stream, new + i);
		common_prefix++;
		if (i < new_len - 1)
			fprintf(stream, "\n");
	}

	len = common_prefix * 2 + (new[new_len - 1].e - new[new_len - 1].b);
	fprintf(stream, "  ");
	for (d = len; d < 60; d++)
		fprintf(stream, ".");
	fprintf(stream, "  ");
	fprintf(stream, "%s", desc);
	fprintf(stream, "\n");
}

typedef struct token comp_t[MAX_COMPONENTS];

static void _list_tests(struct test_details **tests, unsigned nr)
{
	unsigned i, current = 0, current_len, last_len = 0;

	comp_t components[2];

	for (i = 0; i < nr; i++) {
		struct test_details *t = tests[i];
		current_len = _split_components(t->path, components[current], MAX_COMPONENTS);
		_print_path_delta(stderr, components[!current], last_len,
				  components[current], current_len, t->desc);

		last_len = current_len;
		current = !current;
	}
}

static void _destroy_tests(struct dm_list *suites)
{
	struct test_suite *ts, *tmp;

	dm_list_iterate_items_safe (ts, tmp, suites)
		test_suite_destroy(ts);
}

static const char *red(bool c)
{
	return c ? "\x1B[31m" : "";
}

static const char *green(bool c)
{
	return c ? "\x1B[32m" : "";
}

static const char *normal(bool c)
{
	return c ? "\x1B[0m" : "";
}

static void _run_test(struct test_details *t, bool use_colour, unsigned *passed, unsigned *total)
{
	void *fixture;
	struct test_suite *ts = t->parent;
	fprintf(stderr, "[RUN    ] %s\n", t->path);

	(*total)++;
	if (setjmp(test_k))
		fprintf(stderr, "%s[   FAIL]%s %s\n", red(use_colour), normal(use_colour), t->path);
	else {
		if (ts->fixture_init)
			fixture = ts->fixture_init();
		else
			fixture = NULL;

		t->fn(fixture);

		if (ts->fixture_exit)
			ts->fixture_exit(fixture);

		(*passed)++;
		fprintf(stderr, "%s[     OK]%s\n", green(use_colour), normal(use_colour));
	}
}

static bool _run_tests(struct test_details **tests, unsigned nr)
{
	bool use_colour = isatty(fileno(stderr));
	unsigned i, passed = 0, total = 0;

	for (i = 0; i < nr; i++)
		_run_test(tests[i], use_colour, &passed, &total);

	fprintf(stderr, "\n%u/%u tests passed\n", passed, total);

	return passed == total;
}

static void _usage(void)
{
	fprintf(stderr, "Usage: unit-test <list|run> [pattern]\n");
}

static int _cmp_paths(const void *lhs, const void *rhs)
{
	struct test_details *l = *((struct test_details **) lhs);
	struct test_details *r = *((struct test_details **) rhs);

	return strcmp(l->path, r->path);
}

static unsigned _filter(const char *pattern, struct test_details **tests, unsigned nr)
{
	unsigned i, found = 0;
	regex_t rx;

	if (regcomp(&rx, pattern, 0)) {
		fprintf(stderr, "couldn't compile regex '%s'\n", pattern);
		exit(1);
	}

	for (i = 0; i < nr; i++)
		if (!regexec(&rx, tests[i]->path, 0, NULL, 0))
			tests[found++] = tests[i];

	regfree(&rx);

	return found;
}

int main(int argc, char **argv)
{
	int r;
	unsigned i, nr_tests;
	struct test_suite *ts;
	struct test_details *t, **t_array;
	struct dm_list suites;

	dm_list_init(&suites);
	register_all_tests(&suites);

	// count all tests
	nr_tests = 0;
	dm_list_iterate_items (ts, &suites)
		dm_list_iterate_items (t, &ts->tests)
			nr_tests++;

	// stick them in an array
	t_array = malloc(sizeof(*t_array) * nr_tests);
	if (!t_array) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	i = 0;
	dm_list_iterate_items (ts, &suites)
		dm_list_iterate_items (t, &ts->tests)
			t_array[i++] = t;

	// filter
	if (argc == 3)
		nr_tests = _filter(argv[2], t_array, nr_tests);

	// sort
	qsort(t_array, nr_tests, sizeof(*t_array), _cmp_paths);

	// run or list them
	if (argc == 1)
		r = !_run_tests(t_array, nr_tests);
	else {
		const char *cmd = argv[1];
		if (!strcmp(cmd, "run"))
			r = !_run_tests(t_array, nr_tests);

		else if (!strcmp(cmd, "list")) {
			_list_tests(t_array, nr_tests);
			r = 0;

		} else {
			_usage();
			r = 1;
		}
	}

	free(t_array);
	_destroy_tests(&suites);

	return r;
}

//-----------------------------------------------------------------
