/*
 * Marko Kiiskila carnil@cs.tut.fi 
 * 
 * Copyright (c) 1996
 * Tampere University of Technology - Telecommunications Laboratory
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this
 * software and its documentation is hereby granted,
 * provided that both the copyright notice and this
 * permission notice appear in all copies of the software,
 * derivative works or modified versions, and any portions
 * thereof, that both notices appear in supporting
 * documentation, and that the use of this software is
 * acknowledged in any publications resulting from using
 * the software.
 * 
 * TUT ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION AND DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS
 * SOFTWARE.
 * 
 */

/* Copyright (C) 1999 Heikki Vatiainen hessu@cs.tut.fi */

#ifndef KERNEL_H
#define KERNEL_H
#include <linux/atmlec.h>

int kernel_init(unsigned char *mac_addr, int itf);
int msg_to_kernel(struct atmlec_msg *msg, int msg_size);
int msg_from_kernel(void);
int read_from_kernel (char *buff, int size);
int get_lecsaddr(int itf, struct sockaddr_atmsvc *addr);

#endif /* KERNEL_H */
