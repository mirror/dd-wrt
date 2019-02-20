/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2016 Red Hat, Inc. All rights reserved.
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

#ifndef _DM_LOGGING_H
#define _DM_LOGGING_H

#include "libdm/libdevmapper.h"

extern dm_log_with_errno_fn dm_log_with_errno;

#define LOG_MESG(l, f, ln, e, x...) \
	dm_log_with_errno(l, f, ln, e, ## x)

#define LOG_LINE(l, x...) LOG_MESG(l, __FILE__, __LINE__, 0, ## x)
#define LOG_LINE_WITH_ERRNO(l, e, x...) LOG_MESG(l, __FILE__, __LINE__, e, ## x)

/* Debug messages may have a type instead of an errno */
#define LOG_LINE_WITH_CLASS(l, c, x...) LOG_MESG(l, __FILE__, __LINE__, c, ## x)

#include "lib/log/log.h"

#endif
