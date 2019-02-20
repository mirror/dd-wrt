/*
 * Copyright (C) 2012 Red Hat, Inc.
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

#ifndef _LVM_DAEMON_LOG_H
#define _LVM_DAEMON_LOG_H

enum { DAEMON_LOG_FATAL = 0 /* usually preceding daemon death */
     , DAEMON_LOG_ERROR = 1 /* something serious has happened */
     , DAEMON_LOG_WARN = 2 /* something unusual has happened */
     , DAEMON_LOG_INFO = 3 /* thought you might be interested */
     , DAEMON_LOG_WIRE = 4 /* dump traffic on client sockets */
     , DAEMON_LOG_DEBUG = 5 /* unsorted debug stuff */
};

#define DEBUGLOG(s, x...) daemon_logf((s)->log, DAEMON_LOG_DEBUG, x)
#define DEBUGLOG_cft(s, i, n) daemon_log_cft((s)->log, DAEMON_LOG_DEBUG, i, n)
#define WARN(s, x...) daemon_logf((s)->log, DAEMON_LOG_WARN, x)
#define INFO(s, x...) daemon_logf((s)->log, DAEMON_LOG_INFO, x)
#define ERROR(s, x...) daemon_logf((s)->log, DAEMON_LOG_ERROR, x)
#define FATAL(s, x...) daemon_logf((s)->log, DAEMON_LOG_FATAL, x)

#endif
