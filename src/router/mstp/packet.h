/* SPDX-License-Identifier: GPL-2.0-or-later */
/*****************************************************************************
  Copyright (c) 2006 EMC Corporation.

  Authors: Srinivas Aji <Aji_Srinivas@emc.com>

******************************************************************************/

#ifndef PACKET_SOCK_H
#define PACKET_SOCK_H

#include <sys/uio.h>

void packet_send(int ifindex, const struct iovec *iov, int iov_count, int len);
int packet_sock_init(void);

#endif /* PACKET_SOCK_H */
