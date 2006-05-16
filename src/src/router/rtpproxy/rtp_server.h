#ifndef _RTP_SERVER_H_
#define _RTP_SERVER_H_

#include <sys/types.h>

#include "rtp.h"

struct rtp_server {
    double btime;
    unsigned char buf[1024];
    rtp_hdr_t *rtp;
    unsigned char *pload;
    int fd;
    int loop;
};

#define	RTPS_LATER	(0)
#define	RTPS_EOF	(-1)
#define	RTPS_ERROR	(-2)

/*
 * Minimum length of each RTP packet in ms.
 * Actual length may differ due to codec's framing constrains.
 */
#define	RTPS_TICKS_MIN	10

#define	RTPS_SRATE	8000

struct rtp_server *rtp_server_new(const char *, rtp_type_t, int);
void rtp_server_free(struct rtp_server *);
int rtp_server_get(struct rtp_server *);

#endif
