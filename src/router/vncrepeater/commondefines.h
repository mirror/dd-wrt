///////////////////////////////////////////////////////////////////////
// Copyright (C) 2006 Jari Korhonen. jarit1.korhonen@dnainternet.net.
// All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.
//
//
// If the source code for the program is not available from the place
// from which you received this file, check
// http://koti.mbnet.fi/jtko
//
//////////////////////////////////////////////////////////////////////


#ifndef _COMMON_DEFINES_H_
#define _COMMON_DEFINES_H_

#define CONN_MODE1 1
#define CONN_MODE2 2

#define SERVERS_LIST_SIZE 50
#define ID_LIST_SIZE 100

#define MY_TMP_BUF_LEN 255
#define MAX_IP_LEN 50

#define MAX_SESSIONS_DEFAULT 100 
#define MAX_SESSIONS_MIN 1
#define MAX_SESSIONS_MAX 1000   
#define MAX_FIFO_EVENTS 500   

#define LEVEL_0 0
#define LEVEL_1 1
#define LEVEL_2 2
#define LEVEL_3 3
#define DEFAULT_LOGGING_LEVEL  LEVEL_2

#define TIMEOUT_10SECS  10                  //Timeout used in openConnectionToVncServer()
#define TIMEOUT_5SECS   5                   //Timeout used in readPeerHandShake(), doEventWork()


typedef struct 
{
    int a;
    int b;
    int c;
    int d;
} addrParts;


#endif
