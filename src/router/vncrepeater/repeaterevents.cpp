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


#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "readini.h"
#include "repeaterevents.h"
#include "openbsd_stringfuncs.h"
#include "repeater.h"

//Local data
//Local data
//Local data
#define MAX_EVENT_MSG_LEN 200

//This variable is used for keeping track of child process running doEventWork 
//(routine posting various repeater "events" to outside world)
//and cleaning up after its exit
static pid_t eventProc;
static repeaterEvent eventFifo[MAX_FIFO_EVENTS];
static int fifoHeadInd;
static int fifoTailInd;
static int itemsInFifo;

//Local functions
//Local functions
//Local functions

static int advanceFifoIndex(int index)
{
    if (index < (MAX_FIFO_EVENTS-1)) 
        index++;
    else
        index = 0;
    
    return index;
}

static bool isFifoFull(void)
{
    return (MAX_FIFO_EVENTS == itemsInFifo);
}

static bool isFifoEmpty(void)
{
    return (0 == itemsInFifo);
}

//Clear Fifo indexes. Parent process (repeater itself) needs to call this after 
//forking event-sending process, otherwise it will fork handler for same old events forever. 
//I did this --> 100 % cpu load until I killed repeater process ;-)
void clearFifo(void)
{
    fifoHeadInd = 0;
    fifoTailInd = 0;
    itemsInFifo = 0;
}

//We need 2 types of format strings: Normal / Http request
static const char *formatStrings[3][2] =
{
	{
		"EvMsgVer:%d,EvNum:%d,Time:%ld,Pid:%d,TblInd:%d,Code:%ld,Mode:%d,Ip:%d.%d.%d.%d\n",
		"GET /? EvMsgVer=%d&EvNum=%d&Time=%ld&Pid=%d&TblInd=%d&Code=%ld&Mode=%d&Ip=%d.%d.%d.%d HTTP/1.0\n"
	},
	{
		"EvMsgVer:%d,EvNum:%d,Time:%ld,Pid:%d,SvrTblInd:%d,VwrTblInd:%d,Code:%ld,Mode:%d,SvrIp:%d.%d.%d.%d,VwrIp:%d.%d.%d.%d\n",
		"GET /? EvMsgVer=%d&EvNum=%d&Time=%ld&Pid=%d&SvrTblInd=%d&VwrTblInd=%d&Code=%ld&Mode=%d&SvrIp=%d.%d.%d.%d&VwrIp=%d.%d.%d.%d HTTP/1.0\n"
	},
	{
		"EvMsgVer:%d,EvNum:%d,Time:%ld,Pid:%d,MaxSessions:%d\n",
		"GET /? EvMsgVer=%d&EvNum=%d&Time=%ld&Pid=%d&MaxSessions=%d HTTP/1.0\n",
	}
};


//For each event in FIFO, build a message line for repeatereventlistener and send it
//This procedure accesses eventFifo, but there is no need for synchronization 
//(with main repeater process)because we got our own copy of memory when kernel started us  
static int doEventWork(void)
{
    char eventMessageToListener[MAX_EVENT_MSG_LEN];
    connectionEvent *connEv;
    sessionEvent *sessEv;
    startEndEvent *seEv;
    int eventNum;
    repeaterEvent ev;
    int connection;
    char eventListenerIp[MAX_IP_LEN];
    int msgLen;
    int formatStrInd;
    
    if (useHttpForEventListener) {
    	formatStrInd = 1;
    }
    else {
    	formatStrInd = 0;
    }
    
    connection = openConnectionToEventListener(eventListenerHost, eventListenerPort, eventListenerIp, MAX_IP_LEN);  
    if (-1 != connection) {
        while(!isFifoEmpty()) {
            ev = eventFifo[fifoTailInd];
            itemsInFifo--;
            fifoTailInd = advanceFifoIndex(fifoTailInd);
        
            eventNum = ev.eventNum;
    
            switch (eventNum) {
                case VIEWER_CONNECT:
                    //Fall through
                case VIEWER_DISCONNECT:
                    //Fall through
                case SERVER_CONNECT:
                    //Fall through
                case SERVER_DISCONNECT:
                    connEv = (connectionEvent *) ev.extraInfo;
                    msgLen = snprintf(eventMessageToListener, MAX_EVENT_MSG_LEN, 
                        formatStrings[0][formatStrInd],
                        REP_EVENT_VERSION, eventNum, ev.timeStamp, ev.repeaterProcessId, 
                        connEv -> tableIndex, connEv -> code, connEv -> connMode, connEv->peerIp.a,
                        connEv->peerIp.b, connEv->peerIp.c, connEv->peerIp.d);  
                    break;
            
                case VIEWER_SERVER_SESSION_START:
                    //Fall through
                case VIEWER_SERVER_SESSION_END:
                    sessEv = (sessionEvent *) ev.extraInfo;
                    msgLen = snprintf(eventMessageToListener, MAX_EVENT_MSG_LEN, 
                        formatStrings[1][formatStrInd],
                        REP_EVENT_VERSION, eventNum, ev.timeStamp, ev.repeaterProcessId, 
                        sessEv -> serverTableIndex, sessEv -> viewerTableIndex, sessEv -> code, sessEv -> connMode, 
                        sessEv->serverIp.a, sessEv->serverIp.b, sessEv->serverIp.c, sessEv->serverIp.d,
                        sessEv->viewerIp.a, sessEv->viewerIp.b, sessEv->viewerIp.c, sessEv->viewerIp.d);  
                    break;
            
                case REPEATER_STARTUP:
                    //Fall through
                case REPEATER_SHUTDOWN:
                    //Fall through
                case REPEATER_HEARTBEAT:
                    seEv = (startEndEvent *) ev.extraInfo;
                    msgLen = snprintf(eventMessageToListener, MAX_EVENT_MSG_LEN, 
                        formatStrings[2][formatStrInd],
                        REP_EVENT_VERSION, eventNum, ev.timeStamp, ev.repeaterProcessId, 
                        seEv -> maxSessions);  
                    break;
            
                default:
                    msgLen = 0;
                    strlcpy(eventMessageToListener, "\n", MAX_EVENT_MSG_LEN);
                    break;
            }

            if (msgLen > 0) { 
                writeExact(connection, eventMessageToListener, 
                    strlen(eventMessageToListener), TIMEOUT_5SECS);
                    
                debug(LEVEL_3, "%s", eventMessageToListener);
            }
        }
    
        close(connection);
    }
    
    return 0;
}

//Clean up after event-sending processes is finished
static void cleanUpAfterEventProc(void)
{
    pid_t pid;
    
    if (eventProc != 0) {
        pid = waitpid(eventProc, NULL, WNOHANG);
        if (pid > 0) {
            debug(LEVEL_3, "cleanUpAfterEventProc(): Removing event posting process (pid=%d)\n", pid);
            eventProc = 0;
        }
    } 
}



//Global functions
//Global functions
//Global functions
void initRepeaterEventInterface(void)
{
    clearFifo();
    memset(eventFifo, 0, MAX_FIFO_EVENTS * sizeof(repeaterEvent));
    eventProc = 0;
}


//Send event from repeater to event FIFO
//Return true if success, false if failure
bool sendRepeaterEvent(repeaterEvent ev)
{
    if (!isFifoFull()) {
        //Add timestamp
        ev.timeStamp = time(NULL);
        
        eventFifo[fifoHeadInd] = ev;
        
        fifoHeadInd = advanceFifoIndex(fifoHeadInd);
        
        itemsInFifo++;
        
        return true;
    }
    return false;
}


//Send event from repeater to outside world by forking an event-sending process
//If event-sending process is running, check if it finished and clean up
void handleRepeaterEvents(void)
{
    pid_t pid;
    
    if (0 == eventProc) {
        //Event posting process is not running, create one if events in fifo
        if (!isFifoEmpty()) {
            //fork doEvent
            pid = fork();
            if (-1 == pid) {
                //fork failed. This is so unfair. Exit and blame Linus ;-)
                fatal(LEVEL_0, "handleRepeaterEvents(): fork() failed. Linus, this is *so* unfair\n");
            }
            else if (0 == pid) {
                //child code
                debug(LEVEL_3, "handleRepeaterEvents(): in child process, starting doEventWork()\n");
                exit(doEventWork());
            }
            else {
                //parent code
                //Store child pid to variable eventProc so we can
                //properly clean up after child has exited
                eventProc = pid;
                
                //Clean parent process' fifo
                clearFifo();
            }
        }
    }
    else {
        //Event-sending process is running, check if run has ended
        cleanUpAfterEventProc();
    }
}

