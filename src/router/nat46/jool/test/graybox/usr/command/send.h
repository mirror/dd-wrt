#ifndef _GRAYBOX_USR_CMD_SEND_H
#define _GRAYBOX_USR_CMD_SEND_H

#include <stddef.h>
#include <netlink/msg.h>
#include "common/graybox-types.h"

struct send_request {
	char *file_name;
	unsigned char *pkt;
	size_t pkt_len;
};

int send_init_request(int argc, char **argv, enum graybox_command *cmd,
		struct send_request *req);
void send_clean(struct send_request *req);
int send_build_pkt(struct send_request *req, struct nl_msg *pkt);

#endif
