#ifndef _GRAYBOX_USR_CMD_STATS_H
#define _GRAYBOX_USR_CMD_STATS_H

#include <linux/netlink.h>
#include "common/graybox-types.h"

int stats_init_request(int argc, char **argv, enum graybox_command *cmd);
int stats_response_handle(struct nlattr **attrs, void *arg);

#endif
