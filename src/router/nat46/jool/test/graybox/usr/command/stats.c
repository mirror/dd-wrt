#include "usr/command/stats.h"

#include <errno.h>
#include <netlink/attr.h>
#include "usr/log.h"

int stats_init_request(int argc, char **argv, enum graybox_command *cmd)
{
	if (argc < 1) {
		pr_err("stats needs an operation as first argument.");
		return -EINVAL;
	}

	if (strcasecmp(argv[0], "display") == 0) {
		*cmd = COMMAND_STATS_DISPLAY;
		return 0;
	} else if (strcasecmp(argv[0], "flush") == 0) {
		*cmd = COMMAND_STATS_FLUSH;
		return 0;
	}

	pr_err("Unknown operation for stats: %s", argv[0]);
	return -EINVAL;
}

int stats_response_handle(struct nlattr **attrs, void *arg)
{
	struct graybox_stats *stats;

	if (!attrs[ATTR_STATS]) {
		pr_err("The module's response lacks a stats structure.");
		return -EINVAL;
	}

	stats = nla_data(attrs[ATTR_STATS]);
	printf("IPv6:\n");
	printf("	Successes: %u\n", stats->ipv6.successes);
	printf("	Failures:  %u\n", stats->ipv6.failures);
	printf("	Queued:    %u\n", stats->ipv6.queued);
	printf("IPv4:\n");
	printf("	Successes: %u\n", stats->ipv4.successes);
	printf("	Failures:  %u\n", stats->ipv4.failures);
	printf("	Queued:    %u\n", stats->ipv4.queued);

	if (stats->ipv6.failures > 0 || stats->ipv6.queued > 0)
		return -EIO;
	if (stats->ipv4.failures > 0 || stats->ipv4.queued > 0)
		return -EIO;
	return 0;
}
