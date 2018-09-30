/*
 * Copyright (C) 2008 Intel Corporation
 *
 *	mdmon socket / message handling
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

struct mdinfo;
struct metadata_update;

extern int receive_message(int fd, struct metadata_update *msg, int tmo);
extern int send_message(int fd, struct metadata_update *msg, int tmo);
extern int ack(int fd, int tmo);
extern int wait_reply(int fd, int tmo);
extern int connect_monitor(char *devname);
extern int ping_monitor(char *devname);
extern int block_subarray(struct mdinfo *sra);
extern int unblock_subarray(struct mdinfo *sra, const int unfreeze);
extern int block_monitor(char *container, const int freeze);
extern void unblock_monitor(char *container, const int unfreeze);
extern int fping_monitor(int sock);
extern int ping_manager(char *devname);
extern void flush_mdmon(char *container);

#define MSG_MAX_LEN (4*1024*1024)
