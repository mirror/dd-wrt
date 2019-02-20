/*
 * Copyright (C) 2011-2012 Red Hat, Inc.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _LVM_DAEMON_IO_H
#define _LVM_DAEMON_IO_H

#include "libdaemon/client/config-util.h"

/* TODO function names */

int buffer_read(int fd, struct buffer *buffer);
int buffer_write(int fd, const struct buffer *buffer);

#endif /* _LVM_DAEMON_IO_H */
