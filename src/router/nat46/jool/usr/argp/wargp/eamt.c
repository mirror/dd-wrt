#include "usr/argp/wargp/eamt.h"

#include "usr/argp/log.h"
#include "usr/argp/requirements.h"
#include "usr/argp/userspace-types.h"
#include "usr/argp/wargp.h"
#include "usr/argp/xlator_type.h"
#include "usr/nl/core.h"
#include "usr/nl/eamt.h"
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
	print_table_separator(0, 43, 18, 0);
}

static struct jool_result print_entry(struct eamt_entry const *entry, void *args)
{
	struct display_args *dargs = args;
	char ipv6_str[INET6_ADDRSTRLEN];
	char *ipv4_str;

	inet_ntop(AF_INET6, &entry->prefix6.addr, ipv6_str, sizeof(ipv6_str));
	ipv4_str = inet_ntoa(entry->prefix4.addr);

	if (dargs->csv.value) {
		printf("%s/%u,%s/%u\n",
				ipv6_str, entry->prefix6.len,
				ipv4_str, entry->prefix4.len);
	} else {
		printf("| %39s/%-3u | %15s/%-2u |\n",
				ipv6_str, entry->prefix6.len,
				ipv4_str, entry->prefix4.len);
	}

	return result_success();
}

int handle_eamt_display(char *iname, int argc, char **argv, void const *arg)
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
		static char const *const th1 = "IPv6 Prefix";
		static char const *const th2 = "IPv4 Prefix";
		if (dargs.csv.value)
			printf("%s,%s\n", th1, th2);
		else {
			print_separator();
			printf("| %43s | %18s |\n", th1, th2);
			print_separator();
		}
	}

	result = joolnl_eamt_foreach(&sk, iname, print_entry, &dargs);

	joolnl_teardown(&sk);

	if (result.error)
		return pr_result(&result);

	if (!dargs.csv.value)
		print_separator();
	return 0;
}

void autocomplete_eamt_display(void const *args)
{
	print_wargp_opts(display_opts);
}

struct wargp_eamt_entry {
	bool prefix6_set;
	bool prefix4_set;
	struct eamt_entry value;
};

struct add_args {
	struct wargp_eamt_entry entry;
	bool force;
};

static int parse_eamt_column(void *void_field, int key, char *str)
{
	struct wargp_eamt_entry *field = void_field;
	struct jool_result result;

	if (strchr(str, ':')) {
		field->prefix6_set = true;
		result = str_to_prefix6(str, &field->value.prefix6);
		return pr_result(&result);
	}
	if (strchr(str, '.')) {
		field->prefix4_set = true;
		result = str_to_prefix4(str, &field->value.prefix4);
		return pr_result(&result);
	}

	return ARGP_ERR_UNKNOWN;
}

struct wargp_type wt_prefixes = {
	.argument = "<IPv6 prefix> <IPv4 prefix>",
	.parse = parse_eamt_column,
};

static struct wargp_option add_opts[] = {
	WARGP_FORCE(struct add_args, force),
	{
		.name = "Prefixes",
		.key = ARGP_KEY_ARG,
		.doc = "Prefixes (or addresses) that will shape the new EAMT entry",
		.offset = offsetof(struct add_args, entry),
		.type = &wt_prefixes,
	},
	{ 0 },
};

int handle_eamt_add(char *iname, int argc, char **argv, void const *arg)
{
	struct add_args aargs = { 0 };
	struct joolnl_socket sk;
	struct jool_result result;

	result.error = wargp_parse(add_opts, argc, argv, &aargs);
	if (result.error)
		return result.error;

	if (!aargs.entry.prefix6_set || !aargs.entry.prefix4_set) {
		struct requirement reqs[] = {
				{ aargs.entry.prefix6_set, "an IPv6 prefix" },
				{ aargs.entry.prefix4_set, "an IPv4 prefix" },
				{ 0 },
		};
		return requirement_print(reqs);
	}

	result = joolnl_setup(&sk, xt_get());
	if (result.error)
		return pr_result(&result);

	result = joolnl_eamt_add(&sk, iname,
			&aargs.entry.value.prefix6,
			&aargs.entry.value.prefix4,
			aargs.force);

	joolnl_teardown(&sk);
	return pr_result(&result);
}

void autocomplete_eamt_add(void const *args)
{
	print_wargp_opts(add_opts);
}

struct rm_args {
	struct wargp_eamt_entry entry;
};

static struct wargp_option remove_opts[] = {
	{
		.name = "Prefixes",
		.key = ARGP_KEY_ARG,
		.doc = "Prefixes (or addresses) that shape the EAMT entry you want to remove",
		.offset = offsetof(struct rm_args, entry),
		.type = &wt_prefixes,
	},
	{ 0 },
};

int handle_eamt_remove(char *iname, int argc, char **argv, void const *arg)
{
	struct rm_args rargs = { 0 };
	struct joolnl_socket sk;
	struct jool_result result;

	result.error = wargp_parse(remove_opts, argc, argv, &rargs);
	if (result.error)
		return result.error;

	if (!rargs.entry.prefix6_set && !rargs.entry.prefix4_set) {
		struct requirement reqs[] = {
				{ false, "a prefix" },
				{ 0 },
		};
		return requirement_print(reqs);
	}

	result = joolnl_setup(&sk, xt_get());
	if (result.error)
		return pr_result(&result);

	result = joolnl_eamt_rm(&sk, iname,
			rargs.entry.prefix6_set ? &rargs.entry.value.prefix6 : NULL,
			rargs.entry.prefix4_set ? &rargs.entry.value.prefix4 : NULL);

	joolnl_teardown(&sk);
	return pr_result(&result);
}

void autocomplete_eamt_remove(void const *args)
{
	print_wargp_opts(remove_opts);
}

int handle_eamt_flush(char *iname, int argc, char **argv, void const *arg)
{
	struct joolnl_socket sk;
	struct jool_result result;

	/*
	 * We still call wargp_parse despite not having any arguments because
	 * there's still --help and --usage that a clueless user might expect.
	 */
	result.error = wargp_parse(NULL, argc, argv, NULL);
	if (result.error)
		return result.error;

	result = joolnl_setup(&sk, xt_get());
	if (result.error)
		return pr_result(&result);

	result = joolnl_eamt_flush(&sk, iname);

	joolnl_teardown(&sk);
	return pr_result(&result);
}

void autocomplete_eamt_flush(void const *args)
{
	/* Nothing needed here. */
}
