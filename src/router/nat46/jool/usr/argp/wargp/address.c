#include "usr/argp/wargp/address.h"

#include <string.h>
#include "common/types.h"
#include "usr/util/str_utils.h"
#include "usr/nl/address.h"
#include "usr/nl/core.h"
#include "usr/argp/log.h"
#include "usr/argp/requirements.h"
#include "usr/argp/wargp.h"
#include "usr/argp/xlator_type.h"

struct query_args {
	struct wargp_bool verbose;
	struct wargp_addr addr;
};

static struct wargp_option query_opts[] = {
	{
		.name = "verbose",
		.key = 'v',
		.doc = "Show more information",
		.offset = offsetof(struct query_args, verbose),
		.type = &wt_bool,
	}, {
		.name = "Address",
		.key = ARGP_KEY_ARG,
		.doc = "IP Address you want to translate. Can be v6 or v4.",
		.offset = offsetof(struct query_args, addr),
		.type = &wt_addr,
	},
	{ 0 },
};

static void print_addr6(struct in6_addr *addr)
{
	char str[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, addr, str, sizeof(str));
	printf("%s", str);
}

static void print_prefix6(struct ipv6_prefix *prefix)
{
	print_addr6(&prefix->addr);
	printf("/%u", prefix->len);
}

static void print_addr4(struct in_addr *addr)
{
	printf("%s", inet_ntoa(*addr));
}

static void print_prefix4(struct ipv4_prefix *prefix)
{
	print_addr4(&prefix->addr);
	printf("/%u", prefix->len);
}

static void print_rfc6052_scheme(struct ipv6_prefix *prefix)
{
	printf("  Scheme: RFC 6052 prefix\n");
	printf("    Prefix: ");
	print_prefix6(prefix);
	printf("\n");
}

static void print_eamt_scheme(struct eamt_entry *eam)
{
	printf("  Scheme: EAMT\n");
	printf("    EAM: ");
	print_prefix6(&eam->prefix6);
	printf(" | ");
	print_prefix4(&eam->prefix4);
	printf("\n");
}

static void print4(struct in6_addr *src, struct result_addrxlat64 *dst,
		bool verbose)
{
	if (!verbose)
		goto end;

	printf("  Query: ");
	print_addr6(src);
	printf("\n");

	switch (dst->entry.method) {
	case AXM_RFC6052:
		print_rfc6052_scheme(&dst->entry.prefix6052);
		printf("  Operation: ");
		print_addr6(src);
		printf(" - ");
		print_prefix6(&dst->entry.prefix6052);
		printf(" = ");
		print_addr4(&dst->addr);
		printf("\n");
		break;

	case AXM_EAMT:
		print_eamt_scheme(&dst->entry.eam);
		printf("  Operation: ");
		print_addr6(src);
		printf(" - ");
		print_prefix6(&dst->entry.eam.prefix6);
		printf(" + ");
		print_prefix4(&dst->entry.eam.prefix4);
		printf(" = ");
		print_addr4(&dst->addr);
		printf("\n");
		break;

	case AXM_RFC6791:
	default:
		pr_err("Unknown translation method: %u", dst->entry.method);
		return;
	}

	printf("  Result: ");
end:	print_addr4(&dst->addr);
	printf("\n");
}

static void print6(struct in_addr *src, struct result_addrxlat46 *dst,
		bool verbose)
{
	if (!verbose)
		goto end;

	printf("  Query: ");
	print_addr4(src);
	printf("\n");

	switch (dst->entry.method) {
	case AXM_RFC6052:
		print_rfc6052_scheme(&dst->entry.prefix6052);
		printf("  Operation: ");
		print_prefix6(&dst->entry.prefix6052);
		printf(" + ");
		print_addr4(src);
		printf(" = ");
		print_addr6(&dst->addr);
		printf("\n");
		break;

	case AXM_EAMT:
		print_eamt_scheme(&dst->entry.eam);
		printf("  Operation: ");
		print_addr4(src);
		printf(" - ");
		print_prefix4(&dst->entry.eam.prefix4);
		printf(" + ");
		print_prefix6(&dst->entry.eam.prefix6);
		printf(" = ");
		print_addr6(&dst->addr);
		printf("\n");
		break;

	case AXM_RFC6791:
	default:
		pr_err("Unknown translation method: %u", dst->entry.method);
		return;
	}

	printf("  Result: ");
end:	print_addr6(&dst->addr);
	printf("\n");
}

int handle_address_query(char *iname, int argc, char **argv, void const *arg)
{
	struct query_args qargs = { 0 };
	struct joolnl_socket sk;
	union {
		struct result_addrxlat64 v4;
		struct result_addrxlat46 v6;
	} response;

	struct jool_result result;

	result.error = wargp_parse(query_opts, argc, argv, &qargs);
	if (result.error)
		return result.error;

	if (!qargs.addr.proto) {
		struct requirement reqs[] = {
			{ false, "an IP address" },
			{ 0 },
		};
		return requirement_print(reqs);
	}

	result = joolnl_setup(&sk, xt_get());
	if (result.error)
		return pr_result(&result);

	switch (qargs.addr.proto) {
	case 6:
		result = joolnl_address_query64(&sk, iname, &qargs.addr.addr.v6,
				&response.v4);
		if (result.error)
			break;
		print4(&qargs.addr.addr.v6, &response.v4, qargs.verbose.value);
		break;
	case 4:
		result = joolnl_address_query46(&sk, iname, &qargs.addr.addr.v4,
				&response.v6);
		if (result.error)
			break;
		print6(&qargs.addr.addr.v4, &response.v6, qargs.verbose.value);
		break;
	}

	joolnl_teardown(&sk);
	return pr_result(&result);
}

void autocomplete_address_query(void const *args)
{
	print_wargp_opts(query_opts);
}
