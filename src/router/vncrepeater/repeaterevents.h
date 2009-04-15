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


#ifndef REPEATER_EVENTS_H
#define REPEATER_EVENTS_H

#include <time.h>
#include "repeaterutil.h"

//Repeater events for reporting repeater status to outside world: 
//email, web-server, database etc, whoever is listening on eventlistener 
//host/port
enum repeaterEvents {
    VIEWER_CONNECT, 
    VIEWER_DISCONNECT, 
    SERVER_CONNECT, 
    SERVER_DISCONNECT, 
    VIEWER_SERVER_SESSION_START, 
    VIEWER_SERVER_SESSION_END, 
    REPEATER_STARTUP, 
    REPEATER_SHUTDOWN,
    REPEATER_HEARTBEAT
};

#define EXTRA_INFO_SIZE 100
#define REP_EVENT_VERSION 1

typedef struct
{
    //One of repeaterEvents above
    repeaterEvents eventNum;             

    //Time when this happened
    time_t timeStamp;
    
    //Pid of repeater that sent this
    pid_t repeaterProcessId;
    
    //Dump of one of structures below. 
    //I know, this is *ugly*, but I don't want to:
    //A) Use dynamic memory for this
    //B) Use pointers (more than necessary) for this 
    //C) Learn a complicated monster library (STL / Boost) for this 
    //Something like Java's Collections would be very nice. 
    //I *really* need to port to Java someday ;-)
    char extraInfo[EXTRA_INFO_SIZE];    
} repeaterEvent;


//This structure is used in following events:
//VIEWER_CONNECT, VIEWER_DISCONNECT, SERVER_CONNECT, SERVER_DISCONNECT
typedef struct
{
    int tableIndex;
    long code;      
    int connMode;   //CONN_MODE1 / CONN_MODE2
    addrParts peerIp;
} connectionEvent;


//This structure is used in following events:     
//VIEWER_SERVER_SESSION_START, VIEWER_SERVER_SESSION_END
typedef struct 
{
    int serverTableIndex;
    int viewerTableIndex;
    long code;
    int connMode;
    addrParts serverIp;
    addrParts viewerIp;
} sessionEvent;


//This structure is used in following events:     
//REPEATER_STARTUP, REPEATER_SHUTDOWN, REPEATER_HEARTBEAT
typedef struct 
{
    int maxSessions;
} startEndEvent;

extern bool sendRepeaterEvent(repeaterEvent ev);
extern void initRepeaterEventInterface(void);
extern void handleRepeaterEvents(void);

#endif

