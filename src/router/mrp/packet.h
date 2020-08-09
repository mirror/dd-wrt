// Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: (GPL-2.0)

#ifndef PACKET_H
#define PACKET_H

#include <sys/uio.h>

void packet_send(int ifindex, const struct iovec *iov, int iov_count, int len);
int packet_socket_init(void);
void packet_socket_cleanup(void);

#endif /* PACKET_H */
