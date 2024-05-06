#include "usr/argp/wargp/instance.h"

#include <inttypes.h>
#include "common/config.h"
#include "common/constants.h"
#include "usr/argp/log.h"
#include "usr/argp/requirements.h"
#include "usr/argp/wargp.h"
#include "usr/argp/xlator_type.h"
#include "usr/util/str_utils.h"
#include "usr/nl/core.h"
#include "usr/nl/instance.h"

#define OPTNAME_NETFILTER		"netfilter"
#define OPTNAME_IPTABLES		"iptables"

#define ARGP_IPTABLES 1000
#define ARGP_NETFILTER 1001
#define ARGP_POOL6 '6'

struct wargp_iname {
	bool set;
	char value[INAME_MAX_SIZE];
};

#define WARGP_INAME(container, field, description) \
	{ \
		.name = "instance name", \
		.key = ARGP_KEY_ARG, \
		.doc = "Name of the instance you want to " description, \
		.offset = offsetof(container, field), \
		.type = &wt_iname, \
	}

static int parse_iname(void *void_field, int key, char *str)
{
	struct wargp_iname *field = void_field;
	int error;

	error = iname_validate(str, false);
	if (error) {
		pr_err(INAME_VALIDATE_ERRMSG);
		return error;
	}

	field->set = true;
	strcpy(field->value, str);
	return 0;
}

struct wargp_type wt_iname = {
	.argument = "<instance name>",
	.parse = parse_iname,
	.candidates = "default",
};

struct display_args {
	struct wargp_bool no_headers;
	struct wargp_bool csv;
};

static struct wargp_option display_opts[] = {
	WARGP_NO_HEADERS(struct display_args, no_headers),
	WARGP_CSV(struct display_args, csv),
	{ 0 },
};

static void print_table_divisor(void)
{
	printf("+--------------------+-----------------+-----------+\n");
}

static void print_entry_csv(struct instance_entry_usr const *entry)
{
	printf("%" PRIx64 ",%s,", (uint64_t)entry->ns, entry->iname);
	if (entry->xf & XF_NETFILTER)
		printf("netfilter");
	else if (entry->xf & XF_IPTABLES)
		printf("iptables");
	else
		printf("unknown");
	printf("\n");
}

static void print_entry_normal(struct instance_entry_usr const *entry)
{
	/*
	 * 18 is "0x" plus 16 hexadecimal digits.
	 * Why is it necessary? Because the table headers and stuff assume 18
	 * characters and I'm assuming that 32-bit machines would print smaller
	 * pointers.
	 */
	printf("| %18" PRIx64 " | %15s | ", (uint64_t)entry->ns, entry->iname);
	if (entry->xf & XF_NETFILTER)
		printf("netfilter");
	else if (entry->xf & XF_IPTABLES)
		printf(" iptables");
	else
		printf("  unknown");
	printf(" |\n");
}

static struct jool_result print_entry(struct instance_entry_usr const *instance,
		void *arg)
{
	struct display_args *args = arg;
	if (args->csv.value)
		print_entry_csv(instance);
	else
		print_entry_normal(instance);
	return result_success();
}

int handle_instance_display(char *iname, int argc, char **argv, void const *arg)
{
	struct display_args dargs = { 0 };
	struct joolnl_socket sk;
	struct jool_result result;

	if (iname)
		pr_warn("instance display ignores the instance name argument.");

	result.error = wargp_parse(display_opts, argc, argv, &dargs);
	if (result.error)
		return result.error;

	result = joolnl_setup(&sk, xt_get());
	if (result.error)
		return pr_result(&result);

	if (!dargs.no_headers.value) {
		if (dargs.csv.value) {
			printf("Namespace,Name,Framework\n");
		} else {
			print_table_divisor();
			printf("|          Namespace |            Name | Framework |\n");
		}
	}

	if (!dargs.csv.value)
		print_table_divisor();

	result = joolnl_instance_foreach(&sk, print_entry, &dargs);

	joolnl_teardown(&sk);

	if (result.error)
		return pr_result(&result);

	if (!dargs.csv.value)
		print_table_divisor();

	return 0;
}

void autocomplete_instance_display(void const *args)
{
	print_wargp_opts(display_opts);
}

struct add_args {
	struct wargp_iname iname;
	struct wargp_bool iptables;
	struct wargp_bool netfilter;
	struct wargp_prefix6 pool6;
};

static struct wargp_option add_opts[] = {
	WARGP_INAME(struct add_args, iname, "add"),
	{
#ifndef XTABLES_DISABLED
		.name = OPTNAME_IPTABLES,
		.key = ARGP_IPTABLES,
		.doc = "Sit the translator on top of iptables",
		.offset = offsetof(struct add_args, iptables),
		.type = &wt_bool,
	}, {
#endif
		.name = OPTNAME_NETFILTER,
		.key = ARGP_NETFILTER,
		.doc = "Sit the translator on top of Netfilter (default)",
		.offset = offsetof(struct add_args, netfilter),
		.type = &wt_bool,
	}, {
		.name = "pool6",
		.key = ARGP_POOL6,
		.doc = "Prefix that will populate the IPv6 Address Pool",
		.offset = offsetof(struct add_args, pool6),
		.type = &wt_prefix6,
	},
	{ 0 },
};

int handle_instance_add(char *iname, int argc, char **argv, void const *arg)
{
	struct add_args aargs = { 0 };
	struct joolnl_socket sk;
	xlator_framework xf;
	struct jool_result result;

	result.error = wargp_parse(add_opts, argc, argv, &aargs);
	if (result.error)
		return result.error;

	/* Validate instance name */
	if (iname && aargs.iname.set && !STR_EQUAL(iname, aargs.iname.value)) {
		pr_err("You entered two different instance names. Please delete one of them.");
		return -EINVAL;
	}
	if (!iname && aargs.iname.set)
		iname = aargs.iname.value;

	/* Validate framework */
	if (!aargs.netfilter.value && !aargs.iptables.value)
		aargs.netfilter.value = true;
	if (aargs.netfilter.value && aargs.iptables.value) {
		pr_err("The translator can only be hooked to one framework.");
		return -EINVAL;
	}
#ifdef XTABLES_DISABLED
	if (aargs.iptables.value) {
		pr_err("iptables cannot be used; it was disabled during compilation.");
		return -EINVAL;
	}
#endif

	result = joolnl_setup(&sk, xt_get());
	if (result.error)
		return pr_result(&result);

	xf = aargs.netfilter.value ? XF_NETFILTER : XF_IPTABLES;
	result = joolnl_instance_add(&sk, xf, iname,
			aargs.pool6.set ? &aargs.pool6.prefix : NULL);

	joolnl_teardown(&sk);
	return pr_result(&result);
}

void autocomplete_instance_add(void const *args)
{
	print_wargp_opts(add_opts);
}

struct rm_args {
	struct wargp_iname iname;
};

static struct wargp_option remove_opts[] = {
	WARGP_INAME(struct rm_args, iname, "remove"),
	{ 0 },
};

int handle_instance_remove(char *iname, int argc, char **argv, void const *arg)
{
	struct rm_args rargs = { 0 };
	struct joolnl_socket sk;
	struct jool_result result;

	result.error = wargp_parse(remove_opts, argc, argv, &rargs);
	if (result.error)
		return result.error;

	if (iname && rargs.iname.set && !STR_EQUAL(iname, rargs.iname.value)) {
		pr_err("You entered two different instance names. Please delete one of them.");
		return -EINVAL;
	}
	if (!iname && rargs.iname.set)
		iname = rargs.iname.value;

	result = joolnl_setup(&sk, xt_get());
	if (result.error)
		return pr_result(&result);

	result = joolnl_instance_rm(&sk, iname);

	joolnl_teardown(&sk);
	return pr_result(&result);
}

void autocomplete_instance_remove(void const *args)
{
	print_wargp_opts(remove_opts);
}

static struct wargp_option flush_opts[] = {
	{ 0 },
};

int handle_instance_flush(char *iname, int argc, char **argv, void const *arg)
{
	struct joolnl_socket sk;
	struct jool_result result;

	if (iname)
		pr_warn("instance flush ignores the instance name argument.");

	result.error = wargp_parse(flush_opts, argc, argv, NULL);
	if (result.error)
		return result.error;

	result = joolnl_setup(&sk, xt_get());
	if (result.error)
		return pr_result(&result);

	result = joolnl_instance_flush(&sk);

	joolnl_teardown(&sk);
	return pr_result(&result);
}

void autocomplete_instance_flush(void const *args)
{
	print_wargp_opts(flush_opts);
}

static struct wargp_option status_opts[] = {
	{ 0 },
};

int handle_instance_status(char *iname, int argc, char **argv, void const *arg)
{
	/*
	 * Note: If you want to change the labels "Dead" and "Running", do
	 * remember that this change will need to cascade to the init script.
	 */
	static char const *RUNNING_MSG = "Running\n";
	static char const *DEAD_MSG = "Dead\n";
	static char const *UNKNOWN_MSG = "Status unknown\n";

	struct joolnl_socket sk;
	enum instance_hello_status status;
	struct jool_result result;

	result.error = wargp_parse(flush_opts, argc, argv, NULL);
	if (result.error)
		return result.error;

	result = joolnl_setup(&sk, xt_get());
	if (result.error == -ESRCH)
		printf("%s", DEAD_MSG);
	if (result.error)
		return pr_result(&result);

	result = joolnl_instance_hello(&sk, iname, &status);
	if (result.error) {
		printf("%s", UNKNOWN_MSG);
		goto end;
	}

	switch (status) {
	case IHS_ALIVE:
		printf("%s", RUNNING_MSG);
		break;
	case IHS_DEAD:
		printf("%s", DEAD_MSG);
		printf("(Instance '%s' does not exist.)\n",
				iname ? iname : INAME_DEFAULT);
		break;
	}

end:
	joolnl_teardown(&sk);
	return pr_result(&result);
}

void autocomplete_instance_status(void const *args)
{
	print_wargp_opts(status_opts);
}
