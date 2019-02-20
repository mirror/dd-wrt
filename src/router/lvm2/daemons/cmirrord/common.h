/*
 * Copyright (C) 2004-2009 Red Hat, Inc. All rights reserved.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#ifndef _LVM_CLOG_COMMON_H
#define _LVM_CLOG_COMMON_H

/*
 * If there are problems when forking off to become a daemon,
 * the child will exist with one of these codes.  This allows
 * the parent to know the reason for the failure and print it
 * to the launching terminal.
 *
 * #define EXIT_SUCCESS 0 (from stdlib.h)
 * #define EXIT_FAILURE 1 (from stdlib.h)
 */
#define EXIT_LOCKFILE              2
#define EXIT_KERNEL_SOCKET         3 /* Failed netlink socket create */
#define EXIT_KERNEL_BIND           4
#define EXIT_KERNEL_SETSOCKOPT     5
#define EXIT_CLUSTER_CKPT_INIT     6 /* Failed to init checkpoint */
#define EXIT_QUEUE_NOMEM           7

#define DM_ULOG_REQUEST_SIZE 1024

#endif /* _LVM_CLOG_COMMON_H */
