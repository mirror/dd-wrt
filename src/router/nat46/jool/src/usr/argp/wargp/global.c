#include "usr/argp/wargp/global.h"

#include "usr/argp/command.h"
#include "usr/argp/log.h"
#include "usr/argp/userspace-types.h"
#include "usr/argp/wargp.h"
#include "usr/argp/xlator_type.h"
#include "usr/nl/core.h"
#include "usr/nl/global.h"

struct display_args {
	struct wargp_bool no_headers;
	struct wargp_bool csv;
};

static struct wargp_option display_opts[] = {
	WARGP_NO_HEADERS(struct display_args, no_headers),
	WARGP_CSV(struct display_args, csv),
	{ 0 },
};

static struct jool_result handle_display_response(
		struct joolnl_global_meta const *metadata,
		void *value, void *args)
{
	struct display_args *dargs = args;

	if (!dargs->csv.value)
		printf("  ");
	printf("%s%s", joolnl_global_meta_name(metadata),
			dargs->csv.value ? "," : ": ");
	joolnl_global_print(metadata, value, dargs->csv.value);
	printf("\n");

	return result_success();
}

int handle_global_display(char *iname, int argc, char **argv, void const *arg)
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

	if (show_csv_header(dargs.no_headers.value, dargs.csv.value))
		printf("Field,Value\n");

	result = joolnl_global_foreach(&sk, iname, handle_display_response,
			&dargs);

	joolnl_teardown(&sk);

	return pr_result(&result);
}

void autocomplete_global_display(void const *args)
{
	print_wargp_opts(display_opts);
}

struct update_args {
	struct wargp_string global_str;
	struct wargp_bool force;
};

static struct wargp_option update_opts[] = {
	WARGP_FORCE(struct update_args, force),
	{
		.name = "Value",
		.key = ARGP_KEY_ARG,
		.doc = "New value the variable should be changed to",
		.offset = offsetof(struct update_args, global_str),
		.type = &wt_string,
	},
	{ 0 },
};

static int handle_global_update(char *iname, int argc, char **argv, void const *field)
{
	struct update_args uargs = { 0 };
	struct joolnl_socket sk;
	struct jool_result result;

	result.error = wargp_parse(update_opts, argc, argv, &uargs);
	if (result.error)
		return result.error;

	if (!uargs.global_str.value) {
		pr_err("Missing value of key %s.", argv[0]);
		return -EINVAL;
	}

	result = joolnl_setup(&sk, xt_get());
	if (result.error)
		return pr_result(&result);
	result = joolnl_global_update(&sk, iname, field, uargs.global_str.value, uargs.force.value);
	joolnl_teardown(&sk);

	return pr_result(&result);
}

void autocomplete_global_update(void const *meta)
{
	printf("%s ", joolnl_global_meta_values(meta));
	print_wargp_opts(update_opts);
}

struct cmd_option *build_global_update_children(void)
{
	struct joolnl_global_meta const *meta;
	struct cmd_option *opts;
	struct cmd_option *opt;

	opts = calloc(joolnl_global_meta_count() + 1, sizeof(struct cmd_option));
	if (!opts)
		return NULL;

	opt = opts;
	joolnl_global_foreach_meta(meta) {
		opt->label = joolnl_global_meta_name(meta);
		opt->xt = joolnl_global_meta_xt(meta);
		opt->handler = handle_global_update;
		opt->handle_autocomplete = autocomplete_global_update;
		opt->args = meta;
		opt++;
	}

	return opts;
}
