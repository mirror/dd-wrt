#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <iptables.h>


static struct iptables_target offload_tg_reg = {
//		.family        = NFPROTO_UNSPEC,
		.name          = "FLOWOFFLOAD",
		.revision      = 0,
		.version       = IPTABLES_VERSION,
};

void _init(void)
{
	register_target(& offload_tg_reg);
}
