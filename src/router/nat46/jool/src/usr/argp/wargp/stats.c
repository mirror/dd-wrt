#include "usr/argp/wargp/stats.h"

#include "usr/nl/core.h"
#include "usr/nl/stats.h"
#include "usr/argp/log.h"
#include "usr/argp/userspace-types.h"
#include "usr/argp/wargp.h"
#include "usr/argp/xlator_type.h"

struct display_args {
	struct wargp_bool all;
	struct wargp_bool explain;
	struct wargp_bool no_headers;
	struct wargp_bool csv;
};

static struct wargp_option display_opts[] = {
	{
		.name = "all",
		.key = 'a',
		.doc = "Do not filter out zero stats",
		.offset = offsetof(struct display_args, all),
		.type = &wt_bool,
	}, {
		.name = "explain",
		.key = 'e',
		.doc = "Print a description of what each stat means",
		.offset = offsetof(struct display_args, explain),
		.type = &wt_bool,
	},
	WARGP_NO_HEADERS(struct display_args, no_headers),
	WARGP_CSV(struct display_args, csv),
	{ 0 },
};

static struct jool_result handle_jstat(struct joolnl_stat const *stat, void *args)
{
	struct display_args *dargs = args;

	if (!dargs->all.value && stat->value == 0)
		return result_success();

	if (dargs->csv.value) {
		printf("%s,%llu", stat->meta.name, stat->value);
		if (dargs->explain.value)
			printf(",\"%s\"", stat->meta.doc);
		printf("\n");
	} else {
		printf("%s: %llu\n", stat->meta.name, stat->value);
		if (dargs->explain.value)
			printf("%s\n\n", stat->meta.doc);
	}

	return result_success();
}

int handle_stats_display(char *iname, int argc, char **argv, void const *arg)
{
	struct display_args dargs = { 0 };
	struct joolnl_socket sk;
	struct jool_result result;

	result.error = wargp_parse(display_opts, argc, argv, &dargs);
	if (result.error)
		return result.error;

	result = joolnl_setup(&sk, xt_get());
	if (result.error)
		return pr_result(&result);

	if (show_csv_header(dargs.no_headers.value, dargs.csv.value)) {
		printf("Stat,Value");
		if (dargs.explain.value)
			printf(",Explanation");
		printf("\n");
	}

	result = joolnl_stats_foreach(&sk, iname, handle_jstat, &dargs);

	joolnl_teardown(&sk);
	return pr_result(&result);
}

void autocomplete_stats_display(void const *args)
{
	print_wargp_opts(display_opts);
}
