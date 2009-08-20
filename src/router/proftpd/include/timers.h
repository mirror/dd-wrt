/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2007 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 *
 * $Id: timers.h,v 1.15 2007/10/22 18:09:17 castaglia Exp $
 */

#ifndef PR_TIMERS_H
#define PR_TIMERS_H

/* Schedule a timer for the given amount of seconds.  The timerno argument
 * specifies a unique ID for the scheduled timer; passing -1 will have
 * a unique ID automatically assigned, and is generally recommended.
 * The module parameter identifies the module that owns the scheduled
 * timer.  The callback_t parameter specifies a function to be called
 * when the timer expires.
 *
 * Return 0 if the timer was successfully created, or -1 if there was
 * an error.  In the case of an error, errno will be set to an appropriate
 * reason.
 */
int pr_timer_add(int secs, int timerno, module *m, callback_t cb,
  const char *desc);

/* Remove the timer indicated by the timerno parameter, and owned by the
 * given module.  Note that if the caller does not know the module,
 * the value ANY_MODULE can be given.
 *
 * Return 0 on success, -1 on failure.
 */
int pr_timer_remove(int timerno, module *m);

/* Resets the scheduled timer, setting it back to its full scheduled
 * interval.  If the caller does not know the owning module, the
 * value ANY_MODULE can be given.
 *
 * Returns 0 on success, -1 on failure.
 */
int pr_timer_reset(int timerno, module *m);

/* This is a convenience function that can be used to "sleep" for the
 * given number of seconds.  This function can be used instead of
 * the sleep(3) function, for it cannot be interrupted by signals or
 * other timers.
 */
int pr_timer_sleep(int secs);

/* For internal use only. */
void handle_alarm(void);
void timers_init(void);

#endif /* PR_TIMERS_H */
