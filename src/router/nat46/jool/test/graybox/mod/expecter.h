#ifndef _GRAYBOX_MOD_EXPECTER_H
#define _GRAYBOX_MOD_EXPECTER_H

#include <linux/skbuff.h>
#include "common/graybox-types.h"
#include "common/types.h"

struct expected_packet {
	char *filename;
	unsigned char *bytes;
	size_t bytes_len;
	struct mtu_plateaus exceptions;
};

int expecter_setup(void);
void expecter_teardown(void);

int expecter_add(struct expected_packet *pkt);
void expecter_flush(void);

void expecter_stat(struct graybox_stats *result);
void expecter_stat_flush(void);

#endif
