#include "usr/argp/wargp/session.h"

#include "common/config.h"
#include "common/constants.h"
#include "common/session.h"
#include "usr/util/str_utils.h"
#include "usr/nl/core.h"
#include "usr/nl/session.h"
#include "usr/argp/dns.h"
#include "usr/argp/log.h"
#include "usr/argp/userspace-types.h"
#include "usr/argp/wargp.h"
#include "usr/argp/xlator_type.h"

struct display_args {
	struct wargp_bool no_headers;
	struct wargp_bool csv;
	struct wargp_bool numeric;
	struct wargp_l4proto proto;
};

static struct wargp_option display_opts[] = {
	WARGP_TCP(struct display_args, proto, "Print the TCP table (default)"),
	WARGP_UDP(struct display_args, proto, "Print the UDP table"),
	WARGP_ICMP(struct display_args, proto, "Print the ICMP table"),
	WARGP_NO_HEADERS(struct display_args, no_headers),
	WARGP_CSV(struct display_args, csv),
	WARGP_NUMERIC(struct display_args, numeric),
	{ 0 },
};

static char *tcp_state_to_string(tcp_state state)
{
	switch (state) {
	case ESTABLISHED:
		return "ESTABLISHED";
	case V4_INIT:
		return "V4_INIT";
	case V6_INIT:
		return "V6_INIT";
	case V4_FIN_RCV:
		return "V4_FIN_RCV";
	case V6_FIN_RCV:
		return "V6_FIN_RCV";
	case V4_FIN_V6_FIN_RCV:
		return "V4_FIN_V6_FIN_RCV";
	case TRANS:
		return "TRANS";
	}

	return "UNKNOWN";
}

static struct jool_result handle_display_response(
		struct session_entry_usr const *entry, void *args)
{
	struct display_args *dargs = args;
	l4_protocol proto = dargs->proto.proto;
	char timeout[TIMEOUT_BUFLEN];

	timeout2str(entry->dying_time, timeout);

	if (dargs->csv.value) {
		printf("%s,", l4proto_to_string(proto));
		print_addr6(&entry->src6, dargs->numeric.value, ",", proto);
		printf(",");
		print_addr6(&entry->dst6, true, ",", proto);
		printf(",");
		print_addr4(&entry->src4, true, ",", proto);
		printf(",");
		print_addr4(&entry->dst4, dargs->numeric.value, ",", proto);
		printf(",");
		printf("%s", timeout);
		if (proto == L4PROTO_TCP)
			printf(",%s", tcp_state_to_string(entry->state));
		printf("\n");
	} else {
		if (proto == L4PROTO_TCP)
			printf("(%s) ", tcp_state_to_string(entry->state));

		printf("Expires in %s\n", timeout);

		printf("Remote: ");
		print_addr4(&entry->dst4, dargs->numeric.value, "#", proto);
		printf("\t");
		print_addr6(&entry->src6, dargs->numeric.value, "#", proto);
		printf("\n");

		printf("Local: ");
		print_addr4(&entry->src4, true, "#", proto);
		printf("\t");
		print_addr6(&entry->dst6, true, "#", proto);
		printf("\n");

		printf("---------------------------------\n");
	}

	return result_success();
}

int handle_session_display(char *iname, int argc, char **argv, void const *arg)
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

	if (!dargs.csv.value) {
		printf("---------------------------------\n");
	} else if (show_csv_header(dargs.no_headers.value, dargs.csv.value)) {
		printf("Protocol,");
		printf("IPv6 Remote Address,IPv6 Remote L4-ID,");
		printf("IPv6 Local Address,IPv6 Local L4-ID,");
		printf("IPv4 Local Address,IPv4 Local L4-ID,");
		printf("IPv4 Remote Address,IPv4 Remote L4-ID,");
		printf("Expires in,State\n");
	}

	result = joolnl_session_foreach(&sk, iname, dargs.proto.proto,
			handle_display_response, &dargs);

	joolnl_teardown(&sk);

	return pr_result(&result);
}

void autocomplete_session_display(void const *args)
{
	print_wargp_opts(display_opts);
}
