/*
 *	BIRD -- Unix Kernel Route Syncer -- Setting
 *
 *	(c) 2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_KRT_SOCK_H_
#define _BIRD_KRT_SOCK_H_

#include <sys/socket.h>
#include <net/route.h>
#include "lib/socket.h"

#ifndef RTAX_MAX
#define RTAX_MAX        8
#endif


struct ks_msg
{
  struct rt_msghdr rtm;
  struct sockaddr_storage buf[RTAX_MAX];
};



extern int krt_set_sock;

struct krt_set_params {
};

struct krt_set_status {
};

struct krt_if_params {
};

struct krt_if_status {
};

static inline int krt_set_params_same(struct krt_set_params *o UNUSED, struct krt_set_params *n UNUSED) { return 1; }
void krt_read_msg(struct proto *p, struct ks_msg *msg, int scan);

#endif
