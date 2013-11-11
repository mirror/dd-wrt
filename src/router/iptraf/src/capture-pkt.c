/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

#include "iptraf-ng-compat.h"

#include "built-in.h"
#include "parse-options.h"
#include "ifaces.h"
#include "packet.h"

static const char *const capture_usage[] = {
	IPTRAF_NAME " capture [-c] <device>",
	NULL
};

static int cap_nr_pkt = 1, help_opt;
static char *ofilename;

static struct options capture_options[] = {
	OPT__HELP(&help_opt),
	OPT_GROUP(""),
	OPT_INTEGER('c', "capture", &cap_nr_pkt, "capture <n> packets"),
	OPT_STRING('o', "output", &ofilename, "file", "save captured packet into <file>"),
	OPT_END()
};

int cmd_capture(int argc, char **argv)
{
	parse_opts(argc, argv, capture_options, capture_usage);
	argv += optind;
	if (help_opt || !*argv || argv[1])
		parse_usage_and_die(capture_usage, capture_options);

	char *dev = argv[0];

	int fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (fd < 0)
		die_errno("Unable to obtain monitoring socket");

	if (dev_bind_ifname(fd, dev) < 0)
		perror("Unable to bind device on the socket");

	FILE *fp = NULL;
	if (ofilename) {
		fp = fopen(ofilename, "wb");
		if (!fp)
			die_errno("fopen");
	}

	PACKET_INIT(p);
	int captured = 0;
	for (;;) {
		if (packet_get(fd, &p, NULL, NULL) == -1)
			die_errno("fail to get packet");

		if (!p.pkt_len)
			continue;

		printf(".");
		fflush(stdout);

		if (fp)
			fwrite(&p, sizeof(p), 1, fp);

		if (++captured == cap_nr_pkt)
			break;
	}
	printf("\n");

	close(fd);

	if (fp)
		fclose(fp);

	return 0;
}
