/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2002
 * Copyright (C) Miroslav Lichvar  2014
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

  This is the header file for the NTP socket I/O bits.

  */

#ifndef GOT_NTP_IO_H
#define GOT_NTP_IO_H

#include "ntp.h"
#include "addressing.h"

/* Function to initialise the module. */
extern void NIO_Initialise(int family);

/* Function to finalise the module */
extern void NIO_Finalise(void);

/* Function to obtain a socket for sending client packets */
extern int NIO_OpenClientSocket(NTP_Remote_Address *remote_addr);

/* Function to obtain a socket for sending server/peer packets */
extern int NIO_OpenServerSocket(NTP_Remote_Address *remote_addr);

/* Function to close a socket returned by NIO_OpenClientSocket() */
extern void NIO_CloseClientSocket(int sock_fd);

/* Function to close a socket returned by NIO_OpenServerSocket() */
extern void NIO_CloseServerSocket(int sock_fd);

/* Function to check if socket is a server socket */
extern int NIO_IsServerSocket(int sock_fd);

/* Function to check if client packets can be sent to a server */
extern int NIO_IsServerConnectable(NTP_Remote_Address *remote_addr);

/* Function to transmit a packet */
extern int NIO_SendPacket(NTP_Packet *packet, NTP_Remote_Address *remote_addr,
                          NTP_Local_Address *local_addr, int length, int process_tx);

#endif /* GOT_NTP_IO_H */
