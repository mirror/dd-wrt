/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

#include "iptraf-ng-compat.h"

#include "built-in.h"
#include "parse-options.h"
#include "ifaces.h"
#include "packet.h"
#include "capt.h"

static const char *const capture_usage[] = {
	IPTRAF_NAME " capture [options] <device>",
	NULL
};

static int cap_nr_pkt = 1, help_opt;
static char *ofilename;

static struct options capture_options[] = {
	OPT__HELP(&help_opt),
	OPT_GROUP(""),
	OPT_INTEGER('c', "capture", &cap_nr_pkt, "capture <n> packets (default: 1)"),
	OPT_STRING('o', "output", &ofilename, "file", "save captured packet into <file> (default: <stdout>)"),
	OPT_END()
};

int cmd_capture(int argc, char **argv)
{
	parse_opts(argc, argv, capture_options, capture_usage);
	argv += optind;
	if (help_opt || !*argv || argv[1])
		parse_usage_and_die(capture_usage, capture_options);

	char *dev = argv[0];

	struct capt capt;
	if (capt_init(&capt, dev) == -1)
		die_errno("Unable to initialize packet capture interface");

	FILE *fp = stdout;
	if (ofilename) {
		fp = fopen(ofilename, "wb");
		if (!fp)
			die_errno("Unable to open file");
	}

	struct pkt_hdr pkt;
	packet_init(&pkt);

	int captured = 0;
	do {
		if (capt_get_packet(&capt, &pkt, NULL, NULL) == -1)
			die_errno("Failed to get packet");

		if (!pkt.pkt_len)
			continue;

		packet_dump(&pkt, fp);
		capt_put_packet(&capt, &pkt);

		captured++;
		fprintf(stderr, "\rCaptured %d packet(s) out of %d", captured, cap_nr_pkt);
		fflush(stderr);
	} while (captured < cap_nr_pkt);
	fprintf(stderr, "\n");

	packet_destroy(&pkt);

	if (fp && fp != stderr)
		fclose(fp);

	capt_destroy(&capt);

	return 0;
}
