/*
**  igmpproxy - IGMP proxy based multicast router 
**  Copyright (C) 2005 Johnny Egeland <johnny@rlo.org>
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
**----------------------------------------------------------------------------
**
**  This software is derived work from the following software. The original
**  source code has been modified from it's original state by the author
**  of igmpproxy.
**
**  smcroute 0.92 - Copyright (C) 2001 Carsten Schill <carsten@cschill.de>
**  - Licensed under the GNU General Public License, version 2
**  
**  mrouted 3.9-beta3 - COPYRIGHT 1989 by The Board of Trustees of 
**  Leland Stanford Junior University.
**  - Original license can be found in the "doc/mrouted-LINCESE" file.
**
*/
/**
*   udpsock.c contains function for creating a UDP socket.
*
*/

#include "defs.h"

/**
*  Creates and connects a simple UDP socket to the target 
*  'PeerInAdr':'PeerPort'
*
*   @param PeerInAdr - The address to connect to
*   @param PeerPort  - The port to connect to
*           
*/
int openUdpSocket( uint32 PeerInAdr, uint16 PeerPort ) {
    int Sock;
    struct sockaddr_in SockAdr;
    
    if( (Sock = socket( AF_INET, SOCK_DGRAM, 0 )) < 0 )
        log( LOG_ERR, errno, "UDP socket open" );
    
    SockAdr.sin_family      = AF_INET;
    SockAdr.sin_port        = PeerPort;
    SockAdr.sin_addr.s_addr = PeerInAdr;
    memset( &SockAdr.sin_zero, 0, sizeof( SockAdr.sin_zero ) );
    
    if( connect( Sock, (struct sockaddr *)&SockAdr, sizeof( SockAdr ) ) )
        log( LOG_ERR, errno, "UDP socket connect" );
    
    return Sock;
}

