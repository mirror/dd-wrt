/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2002
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  The header file for the linux driver
  */

#ifndef GOT_SYS_LINUX_H
#define GOT_SYS_LINUX_H

extern void SYS_Linux_Initialise(void);

extern void SYS_Linux_Finalise(void);

extern void SYS_Linux_DropRoot(uid_t uid, gid_t gid, int clock_control);

extern void SYS_Linux_EnableSystemCallFilter(int level);

extern int SYS_Linux_CheckKernelVersion(int req_major, int req_minor);

extern int SYS_Linux_OpenPHC(const char *path, int phc_index);

extern int SYS_Linux_GetPHCSample(int fd, int nocrossts, double precision, int *reading_mode,
                                  struct timespec *phc_ts, struct timespec *sys_ts, double *err);

extern int SYS_Linux_SetPHCExtTimestamping(int fd, int pin, int channel,
                                           int rising, int falling, int enable);

extern int SYS_Linux_ReadPHCExtTimestamp(int fd, struct timespec *phc_ts, int *channel);

#endif  /* GOT_SYS_LINUX_H */
