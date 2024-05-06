#include "usr/argp/wargp/denylist4.h"

#include "usr/argp/log.h"
#include "usr/argp/requirements.h"
#include "usr/argp/userspace-types.h"
#include "usr/argp/wargp.h"
#include "usr/argp/xlator_type.h"
#include "usr/nl/core.h"
#include "usr/nl/denylist4.h"
#include "usr/util/str_utils.h"

struct display_args {
	struct wargp_bool no_headers;
	struct wargp_bool csv;
};

static struct wargp_option display_opts[] = {
	WARGP_NO_HEADERS(struct display_args, no_headers),
	WARGP_CSV(struct display_args, csv),
	{ 0 },
};

static void print_separator(void)
{
	print_table_separator(0, 18, 0);
}

static struct jool_result print_entry(struct ipv4_prefix const *prefix, void *args)
{
	struct display_args *dargs = args;
	char *prefix_str = inet_ntoa(prefix->addr);

	if (dargs->csv.value)
		printf("%s/%u\n", prefix_str, prefix->len);
	else
		printf("| %15s/%-2u |\n", prefix_str, prefix->len);

	return result_success();
}

int handle_denylist4_display(char *iname, int argc, char **argv, void const *arg)
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

	if (!dargs.no_headers.value) {
		char *th1 = "IPv4 Prefix";
		if (dargs.csv.value)
			printf("%s\n", th1);
		else {
			print_separator();
			printf("| %18s |\n", th1);
			print_separator();
		}
	}

	result = joolnl_denylist4_foreach(&sk, iname, print_entry, &dargs);

	joolnl_teardown(&sk);

	if (result.error)
		return pr_result(&result);

	if (!dargs.csv.value)
		print_separator();
	return 0;
}

int handle_blacklist4_display(char *iname, int argc, char **argv, void const *arg)
{
	fprintf(stderr, "Warning: blacklist4 is deprecated. Use 'denylist4' instead.\n");
	return handle_denylist4_display(iname, argc, argv, arg);
}

void autocomplete_denylist4_display(void const *args)
{
	print_wargp_opts(display_opts);
}

struct add_args {
	bool force;
	struct wargp_prefix4 prefix;
};

static struct wargp_option add_opts[] = {
	WARGP_FORCE(struct add_args, force),
	{
		.name = "Prefixes",
		.key = ARGP_KEY_ARG,
		.doc = "Prefixes (or addresses) that will shape the new EAMT entry",
		.offset = offsetof(struct add_args, prefix),
		.type = &wt_prefix4,
	},
	{ 0 },
};

int handle_denylist4_add(char *iname, int argc, char **argv, void const *arg)
{
	struct add_args aargs = { 0 };
	struct joolnl_socket sk;
	struct jool_result result;

	result.error = wargp_parse(add_opts, argc, argv, &aargs);
	if (result.error)
		return result.error;

	if (!aargs.prefix.set) {
		struct requirement reqs[] = {
				{ false, "an IPv4 prefix" },
				{ 0 },
		};
		return requirement_print(reqs);
	}

	result = joolnl_setup(&sk, xt_get());
	if (result.error)
		return pr_result(&result);

	result = joolnl_denylist4_add(&sk, iname, &aargs.prefix.prefix, aargs.force);

	joolnl_teardown(&sk);
	return pr_result(&result);
}

int handle_blacklist4_add(char *iname, int argc, char **argv, void const *arg)
{
	fprintf(stderr, "Warning: blacklist4 is deprecated. Use 'denylist4' instead.\n");
	return handle_denylist4_add(iname, argc, argv, arg);
}

void autocomplete_denylist4_add(void const *args)
{
	print_wargp_opts(add_opts);
}

struct rm_args {
	struct wargp_prefix4 prefix;
};

static struct wargp_option remove_opts[] = {
	{
		.name = "Prefixes",
		.key = ARGP_KEY_ARG,
		.doc = "Prefixes (or addresses) that shape the EAMT entry you want to remove",
		.offset = offsetof(struct rm_args, prefix),
		.type = &wt_prefix4,
	},
	{ 0 },
};

int handle_denylist4_remove(char *iname, int argc, char **argv, void const *arg)
{
	struct rm_args rargs = { 0 };
	struct joolnl_socket sk;
	struct jool_result result;

	result.error = wargp_parse(remove_opts, argc, argv, &rargs);
	if (result.error)
		return result.error;

	if (!rargs.prefix.set) {
		struct requirement reqs[] = {
				{ false, "an IPv4 prefix" },
				{ 0 },
		};
		return requirement_print(reqs);
	}

	result = joolnl_setup(&sk, xt_get());
	if (result.error)
		return pr_result(&result);

	result = joolnl_denylist4_rm(&sk, iname, &rargs.prefix.prefix);

	joolnl_teardown(&sk);
	return pr_result(&result);
}

int handle_blacklist4_remove(char *iname, int argc, char **argv, void const *arg)
{
	fprintf(stderr, "Warning: blacklist4 is deprecated. Use 'denylist4' instead.\n");
	return handle_denylist4_remove(iname, argc, argv, arg);
}

void autocomplete_denylist4_remove(void const *args)
{
	print_wargp_opts(remove_opts);
}

int handle_denylist4_flush(char *iname, int argc, char **argv, void const *arg)
{
	struct joolnl_socket sk;
	struct jool_result result;

	result.error = wargp_parse(NULL, argc, argv, NULL);
	if (result.error)
		return result.error;

	result = joolnl_setup(&sk, xt_get());
	if (result.error)
		return pr_result(&result);

	result = joolnl_denylist4_flush(&sk, iname);

	joolnl_teardown(&sk);
	return pr_result(&result);
}

int handle_blacklist4_flush(char *iname, int argc, char **argv, void const *arg)
{
	fprintf(stderr, "Warning: blacklist4 is deprecated. Use 'denylist4' instead.\n");
	return handle_denylist4_flush(iname, argc, argv, arg);
}

void autocomplete_denylist4_flush(void const *args)
{
	/* Nothing needed here. */
}
