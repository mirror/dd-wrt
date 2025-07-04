/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2016
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

  This is the header file for the Linux-specific NTP socket I/O bits.
  */

#ifndef GOT_NTP_IO_LINUX_H
#define GOT_NTP_IO_LINUX_H

#include "socket.h"

extern void NIO_Linux_Initialise(void);

extern void NIO_Linux_Finalise(void);

extern int NIO_Linux_IsHwTsEnabled(void);

extern int NIO_Linux_SetTimestampSocketOptions(int sock_fd, int client_only, int *events);

extern int NIO_Linux_ProcessMessage(SCK_Message *message, NTP_Local_Address *local_addr,
                                    NTP_Local_Timestamp *local_ts, int event);

extern void NIO_Linux_RequestTxTimestamp(SCK_Message *message, int sock_fd);

#endif
