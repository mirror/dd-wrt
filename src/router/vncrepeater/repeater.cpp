// ///////////////////////////////////////////////////////////////////////////
// Copyright (C) 2002 Ultr@VNC Team Members. All Rights Reserved.
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
// Program is based on the
// http://www.imasy.or.jp/~gotoh/ssh/connect.c
// Written By Shun-ichi GOTO <gotoh@taiyo.co.jp>
//
// If the source code for the program is not available from the place
// from which you received this file, check
// http://ultravnc.sourceforge.net/
//
// Linux port (C) 2005- Jari Korhonen, jarit1.korhonen@dnainternet.net
//////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <memory.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pwd.h>        //for getpwnam() in dropPrivileges()

#include "commondefines.h"
#include "repeaterproc.h"
#include "readini.h"
#include "repeaterutil.h"
#include "repeaterevents.h"
#include "repeater.h"

#define REPEATER_VERSION "0.14"

#define RFB_PROTOCOL_VERSION_FORMAT "RFB %03d.%03d\n"
#define RFB_PROTOCOL_MAJOR_VERSION 0
#define RFB_PROTOCOL_MINOR_VERSION 0
#define SIZE_RFBPROTOCOLVERSIONMSG 12

#define RFB_PORT_OFFSET 5900                //servers 1st display is in this port number
#define MAX_IDLE_CONNECTION_TIME 600        //Seconds
#define MAX_HOST_NAME_LEN 250
#define MAX_PATH 250
#define MAX_HANDSHAKE_LEN 100
#define UNKNOWN_REPINFO_IND 999999          //Notice: This should always be bigger than maxSessions

#define LISTEN_BACKLOG  5                   //Listen() queues 5 connections

//connectionFrom defines for acceptConnection(). Used also in connectionRemover()
#define CONNECTION_FROM_SERVER 0
#define CONNECTION_FROM_VIEWER 1

//Use safer openbsd stringfuncs: strlcpy, strlcat
#include "openbsd_stringfuncs.h"

typedef char rfbProtocolVersionMsg[SIZE_RFBPROTOCOLVERSIONMSG+1]; /* allow extra byte for null */

typedef struct _repeaterInfo {
    int socket;

    //Code is used for cross-connection between servers and viewers
    //In Mode 2, Server/Viewer sends IdCode string "ID:xxxxx", where xxxxx is some positive (1 or bigger) long integer number
    //In Mode 1, Repeater "invents" a non-used code (negative number) and assigns that to both Server/Viewer
    //code == 0 means that entry in servers[] / viewers[] table is free 
    long code;

    unsigned long timeStamp;

    //Ip address of peer
    addrParts peerIp;  
    
    //There are 3 connection levels (using variables "code" and "active"): 
    //A. code==0,active==false: fully idle, no connection attempt detected
    //B. code==non-zero,active==false: server/viewer has connected, waiting for other end to connect
    //C. code==non-zero,active=true: doRepeater() running on viewer/server connection, fully active
    //-after viewer/server disconnects or some error in doRepeater, returns both to level A 
    //(and closes respective sockets)
    //This logic means, that when one end disconnects, BOTH ends need to reconnect. 
    //This is not a bug, it is a feature ;-)
    bool active;    
} repeaterInfo;
static repeaterInfo *servers[MAX_SESSIONS_MAX];
static repeaterInfo *viewers[MAX_SESSIONS_MAX];


//Server handshake strings for use when respective viewer connects later
typedef struct _handShakeInfo
{
    char handShake[MAX_HANDSHAKE_LEN];
    int handShakeLength;
} handShakeInfo;
static handShakeInfo *handShakes[MAX_SESSIONS_MAX];

//mode1ConnCode is used in Mode1 to "invent" code field in repeaterInfo, when new Mode1 connection from 
//viewer is accepted. This is just decremented for each new Mode 1 connection to ensure unique number 
//for each Mode 1 session 
//Values for this are: 0=program has just started, -1....MIN_INVENTED_CONN_CODE: Codes for each session
#define MIN_INVENTED_CONN_CODE -1000000
static long mode1ConnCode;


//This structure (and repeaterProcs[] table) is used for 
//keeping track of child processes running doRepeater 
//and cleaning up after they exit
typedef struct _repeaterProcInfo
{
    long code;
    pid_t pid;
} repeaterProcInfo;
static repeaterProcInfo *repeaterProcs[MAX_SESSIONS_MAX];


//This structure keeps information of ports/socket used when 
//routeConnections() listens for new incoming connections
typedef struct _listenPortInfo {
    int socket;
    int port;
} listenPortInfo;

//Repeater "events" interface uses this variable. Various "events" 
//are sent to interface using function sendRepeaterEvent() 
//and later handled with function handleRepeaterEvents(), which forks a child 
//process to handle the grunt work of event posting 
//Child process is later cleaned up calling function cleanUpAfterEventProc()
static repeaterEvent event;

//stopped==true means that user wants program to stop (has pressed ctrl+c)
//From version 0.08 onwards, function fatal() also sets stopped == TRUE to achieve clean shutdown
static bool stopped;

static int nonBlockingRead(int sock, char *buf, int len, int timeOut);
static int findViewerList(long code);
static void cleanUpAfterRepeaterProcs(void);
static void logLineStart(const char *prefix);
static int connectWithTimeout(int socket, const struct sockaddr *addr, socklen_t addrlen, int timeOutSecs);

//Global functions
//Global functions
//Global functions

void debug(int msgLevel, const char *fmt, ...)
{
    va_list args;

    if (msgLevel <= loggingLevel) {
        logLineStart("UltraVnc");

        va_start(args, fmt);
    
        vfprintf(stderr, fmt, args);
        va_end(args);
    }
}

void fatal(const char *fmt, ...)
{
    va_list args;

    logLineStart("UltraVnc FATAL");
    
    va_start(args, fmt);
   
    vfprintf(stderr, fmt, args);
    va_end(args);
 
    //Close program down cleanly (as if user just pressed ctrl+c, of course 
    //log file will show FATAL message in case program shuts down)
    stopped = true;
}


//Try to connect to event listener, return connected socket if success, -1 if error
//parameter listenerIp holds eventlistener's ip address on return (or "" in case of error)
int openConnectionToEventListener(const char *host, unsigned short port, char *listenerIp, int listenerIpSize)
{
    int s;
    struct sockaddr_in saddr;
    struct hostent *h;
        
    h = gethostbyname(host);
    if (NULL == h) {
        debug(LEVEL_2, "openConnectionToEventListener(): can't resolve hostname: %s\n", host);
        return -1;
    }
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    
    //Interesting;-) typecast / indirection thing copied from "Beej's Guide to network programming". 
    //See http://beej.us/guide/bgnet/ for more info
    saddr.sin_addr = *((struct in_addr *)h->h_addr);

    memset(&(saddr.sin_zero), '\0', 8); // zero the rest of the struct
    
    strlcpy(listenerIp, inet_ntoa(saddr.sin_addr), listenerIpSize);
    
    debug(LEVEL_3, "openConnectionToEventListener(): connecting to %s:%u\n", listenerIp, port);

    s = socket(AF_INET, SOCK_STREAM, 0);
    
    //Trying to connect with timeout
    if (connectWithTimeout(s, (struct sockaddr *) &saddr, sizeof(saddr), TIMEOUT_10SECS) != 0) {
        debug(LEVEL_2, "openConnectionToEventListener(): connectWithTimeout() failed.\n");
        close(s);
        strlcpy(listenerIp, "", listenerIpSize);
        return -1;
    }
    else
        return s;
}


//Try to write exact number of bytes to socket
//return 1 if things went OK,
//return -2 in case of timeout
//return -1 in case of error 
int writeExact(int sock, char *buf, int len, int timeOutSecs)
{
    int n;
    int timeOutCtr;

    debug(LEVEL_3, "writeExact(): start\n");
    timeOutCtr=0;
    while ((len > 0) && (timeOutCtr < timeOutSecs)) {
        n = send(sock, buf, len, MSG_DONTWAIT);

        if (n > 0) {
            buf += n;
            len -= n;
        }
        else if (n == 0) {
            debug(LEVEL_3, "writeExact(): send returned 0\n");
            return -1;
        }
        else {
            //send() returned -1 to indicate some error
            //Because we use non-blocking in send(), we have to 
            //handle EAGAIN by incrementing timeout counter
            if (errno == EAGAIN) {
                debug(LEVEL_3, "writeExact(): EAGAIN detected\n");
                sleep(1);
                timeOutCtr++;
            }
            else {
                debug(LEVEL_3, "writeExact(): send() returned error, errno = %d (%s)\n", errno, strerror(errno));
                return -1;
            }
        }
    }

    if (timeOutCtr <  timeOutSecs) {
        debug(LEVEL_3, "writeExact(): returning normally\n");
        return 1;
    }
    else {
        debug(LEVEL_3, "writeExact(): timeout error\n");
        return -2;
    }
}


//Local functions
//Local functions
//Local functions

//Standard log line start common for all types of messages: debug / fatal
static void logLineStart(const char *prefix)
{
    time_t errTime;
    char buf[MY_TMP_BUF_LEN];
    char *lf;

    errTime = time(NULL);

    //ctime() adds '\n' to line end, change that to space
    strlcpy(buf, ((errTime != -1) ? ctime(&errTime) : ""), MY_TMP_BUF_LEN);
    lf = strchr(buf, '\n');
    if (NULL != lf)
        *lf = ' ';

    fprintf(stderr, "%s %s> ", prefix, buf);
}

//Allocate memory for various lists of repeater when program starts
//This routine is needed from version 0.12,
//because lists are not statically allocated anymore
//but can be dynamically changed via ini file setting
//Return true if Ok, false if error
static bool allocateMemoryForRepeaterLists(int numSessions)
{
    int ii;
    
    for(ii = 0; ii < MAX_SESSIONS_MAX; ii++) {
        handShakes[ii] = NULL;
        repeaterProcs[ii] = NULL;
        servers[ii] = NULL;
        viewers[ii] = NULL;
    }
    
    
    for(ii = 0; ii < numSessions; ii++) {
        handShakes[ii] = (handShakeInfo *) calloc(1, sizeof(handShakeInfo));
        if (handShakes[ii] == NULL)
            return false;

        repeaterProcs[ii] = (repeaterProcInfo *) calloc(1, sizeof(repeaterProcInfo));
        if (repeaterProcs[ii] == NULL)
            return false;

        servers[ii] = (repeaterInfo *) calloc(1, sizeof(repeaterInfo));
        if (servers[ii] == NULL)
            return false;

        viewers[ii] = (repeaterInfo *) calloc(1, sizeof(repeaterInfo));
        if (viewers[ii] == NULL)
            return false;
    }
    
    return true;
}

//Free memory of various repeater lists (if allocated)
static void freeMemoryOfRepeaterLists(void)
{
    int ii;
    
    for(ii = 0; ii < MAX_SESSIONS_MAX; ii++) {
        if (handShakes[ii] != NULL) {
            free(handShakes[ii]);
            handShakes[ii] = NULL;
        }
        
        if (repeaterProcs[ii] != NULL) {
            free(repeaterProcs[ii]);
            repeaterProcs[ii] = NULL;
        }
        
        if (servers[ii] != NULL) {
            free(servers[ii]);
            servers[ii] = NULL;
        }
            
        if (viewers[ii] != NULL) {
            free(viewers[ii]);
            viewers[ii] = NULL;
        }
    }
}


//Clean various lists of repeater when program starts
static void cleanLists(void)
{
    int ii;
    for (ii = 0; ii < maxSessions; ii++) {
        memset(handShakes[ii], 0, sizeof(handShakeInfo));
        memset(repeaterProcs[ii], 0, sizeof(repeaterProcInfo));
        memset(servers[ii], 0, sizeof(repeaterInfo));
        memset(viewers[ii], 0, sizeof(repeaterInfo));
    }
}


static void addRepeaterProcList(long code, pid_t pid)
{
    int i;

    for (i = 0; i < maxSessions; i++) {
        if (repeaterProcs[i] -> code == 0) {
            debug(LEVEL_3, "addRepeaterProcList(): Added proc to index=%d, pid=%d, code=%ld\n", i, pid, code);
            repeaterProcs[i] -> code = code;
            repeaterProcs[i] -> pid = pid;
            return;
        }
    }
    debug(LEVEL_2, "addRepeaterProcList(): Warning, no free process slots found\n");
}


static void removeRepeaterProcList(pid_t pid)
{
    int i;

    for (i = 0; i < maxSessions; i++) {
        if (repeaterProcs[i] -> pid == pid) {
            debug(LEVEL_3, "removeRepeaterProcList(): Removing proc from index=%d, pid=%d\n", i, pid);
            repeaterProcs[i] -> code = 0;
            repeaterProcs[i] -> pid = 0;
            return;
        }
    }
    debug(LEVEL_2, "removeRepeaterProcList(): Warning, did not find any process to remove\n");

}




static int findRepeaterProcList(pid_t pid)
{
    int i;

    for (i = 0; i < maxSessions; i++) {
        if (repeaterProcs[i] -> pid == pid) {
            debug(LEVEL_3, "findRepeaterProcList(): proc found at index=%d, pid=%d, code = %ld\n", 
                i, pid, repeaterProcs[i] -> code);
            return i;
        }
    }

    debug(LEVEL_2, "findRepeaterProcList(): Warning, did not find any proc (pid=%d)\n", pid); 
    return UNKNOWN_REPINFO_IND;
}


static int addServerList(int socket, long code, char *peerIp)
{
    int i;

    for (i = 0; i < maxSessions; i++) {
        if (servers[i] -> code == 0) {
            debug(LEVEL_3, "addServerList(): Server added to list: code = %ld, index = %d\n", code, i);
            servers[i] -> code = code;
            servers[i] -> socket = socket;
            servers[i] -> peerIp = getAddrPartsFromString(peerIp);
            servers[i] -> timeStamp = time(NULL);  /* 1 second accuracy is enough ? */
            servers[i] -> active = false;
            return i;
        }
    }
    
    debug(LEVEL_2, "addServerList(): Warning, no table slots available\n"); 
    return -1;  //Not added
}


static void removeServerList(long code)
{
    int i;

    for (i = 0; i < maxSessions; i++) {
        if (servers[i] -> code == code) {
            debug(LEVEL_3, "removeServerList(): Server Removed from list: code = %ld, index = %d\n", code, i);
            servers[i] -> code = 0;
            servers[i] -> active = false;
            return;
        }
    }
    debug(LEVEL_2, "removeServerList(): Warning, server not found (code = %ld)\n", code);
}


static void setServerActive(long code)
{
    int i;

    for (i = 0; i < maxSessions; i++) {
        if (servers[i] -> code == code) {
            servers[i] -> active = true;
            debug(LEVEL_3, "setServerActive(): activated server at index = %d, code = %ld\n", i, servers[i] -> code);
            return;
        }
    }
    debug(LEVEL_2, "setServerActive(): server not found (code = %ld)\n", code);
}


static int findServerList(long code)
{
    int i;

    for (i = 0; i < maxSessions; i++) {
        if (servers[i] -> code == code) {
            debug(LEVEL_3, "findServerList(): server found at index %d, code = %ld\n", i, servers[i] -> code);
            return i;
        }
    }

    debug(LEVEL_2, "findServerList(): server not found (code = %ld)\n", code);
    return UNKNOWN_REPINFO_IND;
}


static int addViewerList(int socket, long code, char *peerIp)
{
    int i;

    for (i = 0; i < maxSessions; i++) {
        if (viewers[i] -> code == 0) {
            debug(LEVEL_3, "addViewerList(): Viewer added to list: code = %ld, index = %d\n", code, i);
            viewers[i] -> code = code;
            viewers[i] -> socket = socket;
            viewers[i] -> peerIp = getAddrPartsFromString(peerIp);
            viewers[i] -> timeStamp = time(NULL);
            viewers[i] -> active = false;
            return i;
        }
    }

    debug(LEVEL_2, "addViewerList(): Warning, no table slots available\n");
    return -1;  //Not added
}


static void removeViewerList(long code)
{
    int i;

    for (i = 0; i < maxSessions; i++) {
        if (viewers[i] -> code == code) {
            debug(LEVEL_3, "removeViewerList(): Viewer removed from list: code = %ld, index = %d\n", code, i);
            viewers[i] -> code = 0;
            viewers[i] -> active = false;
            return;
        }
    }

    debug(LEVEL_2, "removeViewerList(): Warning, viewer not found (code = %ld)\n", code);
}


static void setViewerActive(long code)
{
    int i;

    for (i = 0; i < maxSessions; i++) {
        if (viewers[i] -> code == code) {
            viewers[i] -> active = true;
            debug(LEVEL_3, "setViewerActive(): activated viewer at index %d, code = %ld\n", i, viewers[i] -> code);
            return;
        }
    }

    debug(LEVEL_2, "setViewerActive(): Warning, viewer not found (code = %ld)\n", code);
}


static int findViewerList(long code)
{
    int i;

    for (i = 0; i < maxSessions; i++) {
        if (viewers[i] -> code == code) {
            debug(LEVEL_3, "findViewerList(): viewer found at index %d, code = %ld\n", i, viewers[i] -> code);
            return i;
        }
    }

    debug(LEVEL_2, "findViewerList(): Warning, viewer not found (code = %ld)\n", code);
    return UNKNOWN_REPINFO_IND;
}


//Check IdCode string, require that 1st 3 characters of IdCode are 'I','D',':'
static bool checkIdCode(char *IdCode)
{
    if ((IdCode[0] != 'I') || (IdCode[1] != 'D') || (IdCode[2] != ':')) {
        debug(LEVEL_3, "checkIdCode(): %s is not IdCode string\n", IdCode);
        return false;
    }
    return true;
}


//Parse IdCode string of format "ID:xxxxx", where xxxxx is some positive (non-zero) long integer number 
//Return -1 on error, xxxxx on success
static long parseId(char *IdCode)
{
    unsigned int ii;
    int retVal;

    debug(LEVEL_3, "parseId(): IdCode = %s\n", IdCode);

    //Require that 1st 3 characters of IdCode are 'I','D',':'
    if (false == checkIdCode(IdCode)) {
        debug(LEVEL_3, "parseId(): IdCode format error, does not start ""ID:"" \n");
        return -1;
    }
    else {
        //Require that all other characters of IdCode are digits
        for (ii = 3; ii < strlen(IdCode); ii++) {
            if (!isdigit(IdCode[ii])) {
                debug(LEVEL_3, "parseId(): IdCode format error, code should consist of decimal digits\n");
                return -1;
            }
        }

        retVal = strtol(&(IdCode[3]), NULL, 10);
        if (retVal <= 0) {
            debug(LEVEL_3, "parseId(): IdCode format error, code should be positive long integer number\n");
            return -1;
        }
        else if (retVal == LONG_MAX) {
            debug(LEVEL_3, "parseId(): IdCode format error, code is too big\n");
            return -1;
        }

        return retVal;
    }
}




//Return value: n > 0: number of bytes read
//-1: recv() error
//-2: timeout error
static int nonBlockingRead(int sock, char *buf, int len, int timeOut)
{
    int n;
    int timeOutCtr;
    int numRead = 0;
    
    debug(LEVEL_3, "nonBlockingRead(): start\n");
    timeOutCtr=0;
    while ((len > 0) && (timeOutCtr < timeOut)) {
        n = recv(sock, buf, len, MSG_DONTWAIT);

        if (n > 0) {
            buf += n;
            len -= n;
            numRead += n;
        }
        else {
            if (n == -1) {
                //recv() returned -1 to indicate some error
                //Because we use non-blocking in recv(), we have to 
                //handle EAGAIN by incrementing timeout counter
                if (errno == EAGAIN) {
                    sleep(1);
                    timeOutCtr++;
                }
            }
            else {
                debug(LEVEL_2, "nonBlockingRead(): recv() returned error, errno= %d (%s)\n", errno, strerror(errno));
                return -1;  //return value of recv() was unknown
            }
        }
    }

    if (timeOutCtr < timeOut) {
        debug(LEVEL_3, "nonBlockingRead(): returning normally\n");
        return numRead;
    }
    else {
        //In case of timeout, return number of bytes received if > 0, 
        //otherwise return -2 to indicate timeout
        if (numRead > 0) {
            debug(LEVEL_3, "nonBlockingRead(): returning %d bytes\n", numRead);
            return numRead;
        }
        else {
            debug(LEVEL_3, "nonBlockingRead(): timeout error\n");
            return -2;
        }
    }
}


//Function determines if connection is "too old" (older thar MAX_IDLE_CONNECTION_TIME)
bool isConnectionTooOld(unsigned long timeStamp)
{
    unsigned long tick = time(NULL);
    if ((tick - timeStamp) > MAX_IDLE_CONNECTION_TIME)
        return true;
    else
        return false;
}


//Function determines if connection is inactive
bool isExistingConnectionInactive(bool active, bool existing)
{
    if ((existing) && (!active))
        return true;
    else
        return false;
}


//Function determines if connection is broken
bool isPeerDisconnected(int socket, int connectionFrom)
{
    ssize_t n;

    char buf[SIZE_RFBPROTOCOLVERSIONMSG+1];
    buf[SIZE_RFBPROTOCOLVERSIONMSG] = '\0';
    
    n = recv(socket, buf, SIZE_RFBPROTOCOLVERSIONMSG, MSG_DONTWAIT);

    if (n == 0) {
        debug(LEVEL_3, "isPeerDisconnected: recv() returned 0 (peer has disconnected orderly)\n");
        return true;    //peer has disconnected orderly
    }
    else if (n == -1) {
        //recv() returned -1 to indicate some error
        if (errno == EAGAIN) {
            //Because we use non-blocking in recv(), EAGAIN is ok
            return false;
        }
        else {
            debug(LEVEL_2, "isPeerDisconnected: recv() returned error, errno = %d (%s)\n", errno, strerror(errno));
            return true;
        }
    }
    else if (n >= 1) {
        //peer has sent data OK
        debug(LEVEL_3, "isPeerDisconnected: recv() returned: %s\n", buf);
        return false;  
    }
    else {
        //unknown error
        debug(LEVEL_2, "isPeerDisconnected: recv() returned error, errno = %d (%s)\n", errno, strerror(errno));
        return true;
    }
}

//Remove [old idle | broken] [viewer | server] connection 
static void connectionRemover(int connectionFrom, repeaterInfo *rI, int index)
{
    bool fRemove;
    char removalReason[MY_TMP_BUF_LEN];
    
    fRemove = false;
    strlcpy(removalReason, "", MY_TMP_BUF_LEN);

     
    if (isExistingConnectionInactive(rI -> active, (rI -> code != 0))) {
        if (isConnectionTooOld(rI -> timeStamp)) {
            //Existing connection has been idle for too long, remove
            fRemove = true;
            snprintf(removalReason, MY_TMP_BUF_LEN, "%s", "idle connection too old");
        }
        else if (isPeerDisconnected(rI -> socket, connectionFrom)) {
            //Peer has closed the connection before another peer appeared, remove
            fRemove = true;
            snprintf(removalReason, MY_TMP_BUF_LEN, "%s", "peer has disconnected");
        }

        if (fRemove) {
            //Send VIEWER_DISCONNECT / SERVER_DISCONNECT to event interface
            if (useEventInterface) {
                repeaterEvent event;
                connectionEvent connEv;
                            
                event.eventNum = (connectionFrom == CONNECTION_FROM_VIEWER) ? VIEWER_DISCONNECT : SERVER_DISCONNECT;
                event.timeStamp = time(NULL);
                event.repeaterProcessId = getpid();

                connEv.tableIndex = index;
                connEv.code = rI -> code;
                connEv.connMode = (rI -> code < 0) ? CONN_MODE1 : CONN_MODE2;
                connEv.peerIp = rI -> peerIp;
                memcpy(event.extraInfo, &connEv, sizeof(connectionEvent));
                
                if (false == sendRepeaterEvent(event)) {
                    debug(LEVEL_1, "connectionRemover(): Warning, event fifo is full\n");
                }
            }

            //Remove & close connection 
            close(rI -> socket);
            debug(LEVEL_1, "connectionRemover(): Removing %s %ld at index %d (%s)\n", 
                (connectionFrom == CONNECTION_FROM_VIEWER) ? "viewer" : "server",
                rI -> code, 
                index,
                removalReason);
            
            if (connectionFrom == CONNECTION_FROM_VIEWER)
                removeViewerList(rI -> code);
            else
                removeServerList(rI -> code);
        }
    }
}

//This function is periodically called from routeConnections() to remove
//servers / viewers that did not receive any matching other end connection
static void removeOldOrBrokenConnections(void)
{
    int ii;

    for (ii = 0; ii < maxSessions; ii++) {
        //Remove old inactive viewers
        connectionRemover(CONNECTION_FROM_VIEWER, viewers[ii], ii);


        //Remove old inactive servers
        connectionRemover(CONNECTION_FROM_SERVER, servers[ii], ii);
    }
}


//Parse [hostname / ip address] / [port number / display number] combination
//Return true if success, false if error
static bool parseHostAndPort(char *id, char *host, int hostLen, int *port)
{
    int tmpPort;
    char *colonPos;
    
    debug(LEVEL_3, "parseHostAndPort() start: id = %s\n", id);
    
    colonPos = strchr(id, ':');
    if (hostLen < (int) strlen(id)) {
        debug(LEVEL_3, "parseHostAndPort(): Id string too long\n");
        return false;
    }

    if (colonPos == NULL) {
        // No colon -- use default port number
        tmpPort = RFB_PORT_OFFSET;
        strlcpy(host, id, hostLen);
    }
    else {
        strlcpy(host, id, (colonPos-id)+1);

        if (colonPos[1] == ':') {
            // Two colons -- interpret as a port number
            if (sscanf(colonPos + 2, "%d", &tmpPort) != 1) {
                debug(LEVEL_3, "parseHostAndPort(): sscanf error 1\n");
                return false;
            }
        }
        else {
            // One colon -- interpret as a display number or port
            // number
            if (sscanf(colonPos + 1, "%d", &tmpPort) != 1) {
                return false;
            }
            // RealVNC method - If port < 100 interpret as display
            // number else as Port number
            if (tmpPort < 100)
                tmpPort += RFB_PORT_OFFSET;
        }
    }

    *port = tmpPort;
    debug(LEVEL_3, "parseHostAndPort() end: host = %s, port = %d\n", host, tmpPort);
    return true;
}

//Connect-with-timeout function borrowed from unix sockets faq
//Maybe Java guys have some point when talking about exception handling ;-)
static int connectWithTimeout(int socket, const struct sockaddr *addr, socklen_t addrlen, int timeOutSecs)
{
    int res;
    long arg; 
    fd_set myset; 
    struct timeval tv; 
    int valopt;     
    socklen_t lon;
    
    //First, set socket non-blocking 
    arg = fcntl(socket, F_GETFL, NULL);
    if (arg < 0) { 
        debug(LEVEL_2, "connectWithTimeout(): error in fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
        return -1; 
    } 
    arg |= O_NONBLOCK; 
    if (fcntl(socket, F_SETFL, arg) < 0) { 
        debug(LEVEL_2, "connectWithTimeout(): error in fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
        return -1; 
    }     

    //Try to connect with timeout 
    res = connect(socket, addr, addrlen); 
    if (res < 0) { 
        if (errno == EINPROGRESS) { 
            debug(LEVEL_3, "connectWithTimeout(): EINPROGRESS in connect() - selecting\n"); 
        
            do { 
                tv.tv_sec = timeOutSecs; 
                tv.tv_usec = 0; 
                
                FD_ZERO(&myset); 
                FD_SET(socket, &myset); 
                
                res = select(socket + 1, NULL, &myset, NULL, &tv); 
           
                if ((res < 0) && (errno != EINTR)) { 
                    debug(LEVEL_3, "connectWithTimeout(): Error connecting %d (%s)\n", errno, strerror(errno)); 
                    return -1; 
                } 
                else if (res > 0) { 
                    // Socket selected for write, check if connection was succesful 
                    lon = sizeof(int); 
                    
                    if (getsockopt(socket, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
                        debug(LEVEL_2, "connectWithTimeout(): Error in getsockopt() %d (%s)\n", errno, strerror(errno)); 
                        return -1; 
                    } 
              
                    // Check the value returned... 
                    if (valopt) { 
                        debug(LEVEL_2, "connectWithTimeout(): Error in delayed connection() %d (%s)\n", 
                            valopt, strerror(valopt)); 
                        return -1; 
                    } 
                    else {
                        debug(LEVEL_3, "connectWithTimeout(): connected OK\n"); 
                        break;
                    } 
                } 
                else { 
                    debug(LEVEL_3, "connectWithTimeout(): Timeout in select() - Cancelling!\n"); 
                    return -1; 
                } 
            } while (1); 
        } 
        else { 
            debug(LEVEL_3, "connectWithTimeout(): Error connecting %d (%s)\n", errno, strerror(errno)); 
            return -1; 
        } 
    } 
  
    //Set to blocking mode again... 
    if ((arg = fcntl(socket, F_GETFL, NULL)) < 0) { 
        debug(LEVEL_2, "connectWithTimeout(): Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
        return -1; 
    } 
    arg &= (~O_NONBLOCK); 
    if (fcntl(socket, F_SETFL, arg) < 0) { 
        debug(LEVEL_2, "connectWithTimeout(): Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
        return -1; 
    } 
    
    return 0;
}


//check intended Mode 1 server address against list of denied addresses/ranges in repeater.ini
//return true if denied address, false otherwise
static bool isServerAddressDenied(addrParts srvAddr)
{
    int ii;
    
    for(ii = 0; ii < SERVERS_LIST_SIZE; ii++) {
        if (((srvAddr.a == srvListDeny[ii].a) || (srvListDeny[ii].a == 0)) &&
            ((srvAddr.b == srvListDeny[ii].b) || (srvListDeny[ii].b == 0)) &&
            ((srvAddr.c == srvListDeny[ii].c) || (srvListDeny[ii].c == 0)) &&
            ((srvAddr.d == srvListDeny[ii].d) || (srvListDeny[ii].d == 0)) ) {
                debug(LEVEL_3, "isServerAddressDenied(): address is in deny list, denying (%d.%d.%d.%d)\n", 
                    srvAddr.a,srvAddr.b,srvAddr.c,srvAddr.d);
                
                return true;
        }
    }
    
    return false;
}

//check intended Mode 1 server address against list of allowed addresses/ranges in repeater.ini
//return true if allowed address, false otherwise
static bool isServerAddressAllowed(char *serverIp)
{
    int ii;
    addrParts srvAddr;
    bool allow;
    
    srvAddr = getAddrPartsFromString(serverIp);
    
    for(ii = 0; ii < SERVERS_LIST_SIZE; ii++) {
        allow = true;

        //List 255 == denied
        if ((srvListAllow[ii].a == 255) || (srvListAllow[ii].b == 255) || 
            (srvListAllow[ii].c == 255) || (srvListAllow[ii].d == 255))
            allow = false;
            
        //server 255 == denied
        if ((srvAddr.a == 255) || (srvAddr.b == 255) || (srvAddr.c == 255) || (srvAddr.d == 255))
            allow = false;
            
        //server 0 == denied
        if ((srvAddr.a == 0) || (srvAddr.b == 0) || (srvAddr.c == 0) || (srvAddr.d == 0))
            allow = false;
            
        
        //allowed so far ? 
        if (allow)
        {
            //allow if exact match or if place is 0 in allow list
            if (((srvAddr.a == srvListAllow[ii].a) || (srvListAllow[ii].a == 0)) &&
                ((srvAddr.b == srvListAllow[ii].b) || (srvListAllow[ii].b == 0)) &&
                ((srvAddr.c == srvListAllow[ii].c) || (srvListAllow[ii].c == 0)) &&
                ((srvAddr.d == srvListAllow[ii].d) || (srvListAllow[ii].d == 0)) ) {
                    //Allowed so far, check denial
                    if (!isServerAddressDenied(srvAddr)) {
                        debug(LEVEL_3, "isServerAddressAllowed(): address is OK, allowing (%s)\n", serverIp);
                        return true;
                    }
            }
        }
    }
    
    return false;
}


//Try to connect to vnc server, return connected socket if success, -1 if error
//parameter serverIp holds server ip address on return (or "" in case of error)
static int openConnectionToVncServer(const char *host, unsigned short port, char *serverIp)
{
    int s;
    struct sockaddr_in saddr;
    struct hostent *h;
        
    h = gethostbyname(host);
    if (NULL == h) {
        debug(LEVEL_2, "openConnectionToVncServer(): can't resolve hostname: %s\n", host);
        return -1;
    }
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    
    //Interesting;-) typecast / indirection thing copied from "Beej's Guide to network programming". 
    //See http://beej.us/guide/bgnet/ for more info
    saddr.sin_addr = *((struct in_addr *)h->h_addr);

    memset(&(saddr.sin_zero), '\0', 8); // zero the rest of the struct
    
    strlcpy(serverIp, inet_ntoa(saddr.sin_addr), MAX_IP_LEN);
    
    //Check server addresses against list of allowed addresses / ranges
    if (requireListedServer == 1) {
        if (!isServerAddressAllowed(serverIp)) {
            debug(LEVEL_2, "openConnectionToVncServer(): server is not allowed (%s)\n", serverIp);
            return -1;
        }
    }
    
    debug(LEVEL_3, "openConnectionToVncServer(): connecting to %s:%u\n", serverIp, port);

    s = socket(AF_INET, SOCK_STREAM, 0);
    
    //Trying to connect with timeout
    if (connectWithTimeout(s, (struct sockaddr *) &saddr, sizeof(saddr), TIMEOUT_10SECS) != 0) {
        debug(LEVEL_2, "openConnectionToVncServer(): connectWithTimeout() failed.\n");
        close(s);
        strlcpy(serverIp, "", MAX_IP_LEN);
        return -1;
    }
    else
        return s;
}


//Check ID code against connection tables.
//Return index of similar ID if similar connection found
//Return -1 if similar connection not found
static int findDuplicateIdIndex(int connectionFrom, long code)
{
    repeaterInfo *repInfo;
    int ii;
    
    for(ii = 0; ii < maxSessions; ii++) {
        if (connectionFrom == CONNECTION_FROM_VIEWER)
            repInfo = viewers[ii]; 
        else
            repInfo = servers[ii]; 

        if (repInfo -> code == code) {
            debug(LEVEL_2, "findDuplicateIdIndex(): similar %s ID already there\n",
                (connectionFrom == CONNECTION_FROM_VIEWER) ? "viewer" : "server");
            return ii;
        }
    }
    
    debug(LEVEL_3, "findDuplicateIdIndex(): similar ID not found\n");
    return -1;
}

//Common part of forking repeater for various connection modes
static void forkRepeater(int serverSocket, int viewerSocket, long idCode)
{
    pid_t pid;

    //fork repeater
    pid = fork();
    if (-1 == pid) {
        //fork failed. This is so unfair. Exit and blame Linus ;-)
        fatal(LEVEL_0, "forkRepeater(): fork() failed. Linus, this is *so* unfair.\n");
    }
    else if (0 == pid) {
        //child code
        debug(LEVEL_3, "forkRepeater(): in child process, starting doRepeater(%d, %d)\n", serverSocket, viewerSocket);
        exit(doRepeater(serverSocket, viewerSocket));
    }
    else {
        //parent code
        //Add necessary information of child to repeaterProcs list so we can
        //properly clean up after child has exited
        addRepeaterProcList(idCode, pid);
        
        //Close (parents copies of) repeater sockets right away here,
        //so we don't need to close them in cleanUpAfterRepeaterProcExit()
        close(serverSocket);
        close(viewerSocket);
    }
}

//Compare ID code against codes in list
//return true if match found, false otherwise
static bool isCodeInIdList(long code)
{
    int ii;
    
    for(ii = 0; ii < ID_LIST_SIZE; ii++) {
        if (code == idList[ii]) {
            debug(LEVEL_3, "isCodeInIdList(): ID code match found (%ld)\n", code);
            return true;
        }
    }

    return false;
}

//Read peer handshake
void readPeerHandShake(int socket, int index)
{
    int len;
    
    //Make sure handshake is null-terminated
    handShakes[index] -> handShake[MAX_HANDSHAKE_LEN-1] ='\0';
    
    len = nonBlockingRead(socket, handShakes[index] -> handShake, MAX_HANDSHAKE_LEN - 1, TIMEOUT_5SECS);
    if (len < 0) {
       strlcpy(handShakes[index] -> handShake, "", MAX_HANDSHAKE_LEN);
       handShakes[index] -> handShakeLength = 0;
    }
    else {
        debug(LEVEL_3, "readPeerHandShake(): len = %d\n", len);
        handShakes[index] -> handShakeLength = len;
    }
}


//Accept connection in non-blocking way (this should stop hanging if something bad
//has happened to socket between select() and accept())
//Return -1 if error, accept():ed socket in normal case
int nonBlockingAccept(int socket, struct sockaddr *sa, socklen_t *sockLen)
{
    long arg; 
    int socketToReturn;
    
    //First, set socket non-blocking 
    arg = fcntl(socket, F_GETFL, NULL);
    if (arg < 0) { 
        debug(LEVEL_2, "nonBlockingAccept(): error in fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
        return -1; 
    } 
    arg |= O_NONBLOCK; 
    if (fcntl(socket, F_SETFL, arg) < 0) { 
        debug(LEVEL_2, "nonBlockingAccept(): error in fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
        return -1; 
    }     
    
    //Accept connection 
    socketToReturn = accept(socket, sa, sockLen);

    //Set to blocking mode again... 
    if ((arg = fcntl(socket, F_GETFL, NULL)) < 0) { 
        debug(LEVEL_2, "nonBlockingAccept(): Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
        if (socketToReturn != -1)
            close(socketToReturn);
        return -1; 
    } 
    arg &= (~O_NONBLOCK); 
    if (fcntl(socket, F_SETFL, arg) < 0) { 
        debug(LEVEL_2, "nonBlockingAccept(): Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
        if (socketToReturn != -1)
            close(socketToReturn);
        return -1; 
    } 

    //At last, we are ready to return accept():ed socket ;-)
    return socketToReturn;
}


//Accept connections from both servers and viewers 
// connectionFrom == CONNECTIONFROMSERVER means server is connecting, 
// connectionFrom==CONNECTIONFROMVIEWER means viewer is connecting
// Mode 1 connections are only accepted from viewers (repeater then connects to server)
static void acceptConnection(int socket, int connectionFrom)
{
    rfbProtocolVersionMsg pv;
    int connection;
    char id[MAX_HOST_NAME_LEN + 1];
    long code;
    struct sockaddr_in client;
    socklen_t sockLen;
    char peerIp[MAX_IP_LEN];
    int connMode;   //Connection mode: CONN_MODE1 or CONN_MODE2

    //These variables are used in Mode 1
    char host[MAX_HOST_NAME_LEN+1];
    char connMode1ServerIp[MAX_IP_LEN];
    int port;    
    
    sockLen = sizeof(struct sockaddr_in);
    
    connection = nonBlockingAccept(socket, (struct sockaddr *) &client, &sockLen);

    if (connection < 0)
        debug(LEVEL_2, "acceptConnection(): accept() failed, errno=%d (%s)\n", errno, strerror(errno));
    else {
        strlcpy(peerIp, inet_ntoa(client.sin_addr), MAX_IP_LEN);
    
        debug(LEVEL_1, "acceptConnection(): connection accepted ok from ip: %s\n", peerIp);
        
        if (connectionFrom == CONNECTION_FROM_VIEWER) {
            //We handshake viewers by transmitting rfbProtocolVersion first
            snprintf(pv, SIZE_RFBPROTOCOLVERSIONMSG+1, RFB_PROTOCOL_VERSION_FORMAT, 
                RFB_PROTOCOL_MAJOR_VERSION, RFB_PROTOCOL_MINOR_VERSION);
            
            debug(LEVEL_3, "acceptConnection(): pv = %s", pv);
            
            if (writeExact(connection, pv, SIZE_RFBPROTOCOLVERSIONMSG, TIMEOUT_10SECS) < 0) {
                debug(LEVEL_2, "acceptConnection(): Writing protocol version error\n");
                close(connection);
                return;
            }
        }

        //Make sure that id is null-terminated
        id[MAX_HOST_NAME_LEN] = '\0';
        if (nonBlockingRead(connection, id, MAX_HOST_NAME_LEN, TIMEOUT_5SECS) < 0) {
            debug(LEVEL_2, "acceptConnection(): Reading id error\n");
            close(connection);
            return;
        }

        //id can be of format:
        //Normally in Mode 2:
        //"ID:xxxxx", where xxxxx is some positive (non-zero) long integer number.
        //
        //Normally in Mode 1:
        //"xx.yy.zz.nn::pppp" (Ip address, 2 colons, port number)
        //"xx.yy.zz.nn:pppp" (Ip address, 1 colons, some number): This is a problematic case.
        //It is interpreted in the following way (copied directly from original repeater):
        //If pppp is < 100, it is a display number. If >= 100, it is a port number. 
        //"xx.yy.zz.nn" (Only Ip Address): Default port number RFB_PORT_OFFSET is used
        //In mode 1, instead of ip address, also DNS hostname can be used in any combination with 
        //port / display number
        if (checkIdCode(id)) {
            if ((allowedModes & CONN_MODE2) > 0) {
                connMode = CONN_MODE2;

                //id is an IdCode string, parse it
                code = parseId(id);
                if (-1 == code) {
                    debug(LEVEL_3, "acceptConnection(): parseId returned error, closing connection\n");
                    close(connection);
                    return;
                }
                debug(LEVEL_3, "acceptConnection():  %s sent code %ld \n", 
                    (connectionFrom == CONNECTION_FROM_VIEWER) ? "Viewer" : "Server", code);

                //Check that there isn't similar ID:xxxx string in use
                //If similar ID:xxxx is found, refuse new connection
                int index;
                index = findDuplicateIdIndex(connectionFrom, code);
                if (index != -1) {
                    debug(LEVEL_2, "acceptConnection(): duplicate ID string found, closing connection\n");
                    close(connection);
                    return;
                }
                
                //If listed ID is required, check that ID matches one in list
                if (requireListedId) {
                    if (!isCodeInIdList(code)) {
                        debug(LEVEL_2, 
                            "acceptConnection(): Id code does not match codes in list, closing connection\n", code);
                        close(connection);
                        return;
                    }
                }
            }
            else {
                debug(LEVEL_2, "acceptConnection(): mode 2 connections are not allowed, closing connection\n");
                close(connection);
                return;
            } 
        }
        else {
            if ((allowedModes & CONN_MODE1) > 0) {
                connMode = CONN_MODE1;

                //id is an [hostname / ip address] / [port number / display number] combination of some sort, parse it
                if (false == parseHostAndPort(id, host, MAX_HOST_NAME_LEN + 1, &port)) {
                    debug(LEVEL_2, "acceptConnection(): parseHostAndPort returned error\n");
                    close(connection);
                    return;
                }
                
                //check server port if not all allowed
                if (allowedMode1ServerPort != 0) {
                    if (port != allowedMode1ServerPort) {
                        debug(LEVEL_2, "acceptConnection(): connection to server port %d is not allowed,"
                            " closing connection\n", port);
                        close(connection);
                        return;
                    }
                }
            }
            else {
                debug(LEVEL_2, "acceptConnection(): mode 1 connections are not allowed, closing connection\n");
                close(connection);
                return;
            } 
        }
        
        if (connMode == CONN_MODE1) {
            if (connectionFrom == CONNECTION_FROM_VIEWER) {
                int server;
            
                server = openConnectionToVncServer(host, (unsigned short) port, connMode1ServerIp);
                if (server == -1) {
                    debug(LEVEL_2, "acceptConnection(): openConnectionToVncServer() failed\n"); 
                    close(connection);
                    return;
                }
                else {
                    bool fServerOk ;
                    bool fViewerOk;
                    int viewerInd;
                    int serverInd;
                    
                    fServerOk = true;
                    fViewerOk = true;
                    
                    //Invent new unique connection code
                    //Minus-side numbers are used for Mode1 sessions
                    mode1ConnCode--;
                    if (mode1ConnCode < MIN_INVENTED_CONN_CODE)
                        mode1ConnCode = -1; 
            
                    //Add new viewer
                    viewerInd = addViewerList(connection, mode1ConnCode, peerIp);
                    if (-1 != viewerInd) {
                        setViewerActive(mode1ConnCode);
                    }
                    else
                        fViewerOk = false;  //Out of slots in viewer table
                
                    //Add new server
                    serverInd = addServerList(server, mode1ConnCode, connMode1ServerIp); 
                    if (-1 != serverInd) {
                        setServerActive(mode1ConnCode);
                    }
                    else
                        fServerOk = false;  //Out of slots in server table
                                                
                    if ((fServerOk) && (fViewerOk)) {
                        //fork repeater
                        forkRepeater(server, connection, mode1ConnCode);

                        //Send appropriate events to event interface
                        //Here we post 3 events: 
                        //VIEWER_CONNECT, SERVER_CONNECT, VIEWER_SERVER_SESSION_START
                        if (useEventInterface) {
                            repeaterEvent event;
                            connectionEvent connEv;
                            sessionEvent sessEv;
                            addrParts serverIp;
                            addrParts viewerIp;
                            
                            //Addresses in compact binary form
                            viewerIp = getAddrPartsFromString(peerIp);
                            serverIp = getAddrPartsFromString(connMode1ServerIp);
                            
                            //VIEWER_CONNECT
                            event.eventNum = VIEWER_CONNECT;
                            event.timeStamp = time(NULL);
                            event.repeaterProcessId = getpid();
                            
                            connEv.tableIndex = viewerInd;
                            connEv.code = mode1ConnCode;
                            connEv.connMode = CONN_MODE1;
                            connEv.peerIp = viewerIp;
                            memcpy(event.extraInfo, &connEv, sizeof(connectionEvent));
                            if (false == sendRepeaterEvent(event)) {
                                debug(LEVEL_1, "acceptConnection(): Warning, event fifo is full\n"); 
                            }
                            
                            //SERVER_CONNECT
                            event.eventNum = SERVER_CONNECT;
                            event.timeStamp = time(NULL);
                            event.repeaterProcessId = getpid();

                            connEv.tableIndex = serverInd;
                            connEv.code = mode1ConnCode;
                            connEv.connMode = CONN_MODE1;
                            connEv.peerIp = serverIp;
                            memcpy(event.extraInfo, &connEv, sizeof(connectionEvent));
                            if (false == sendRepeaterEvent(event)) {
                                debug(LEVEL_1, "acceptConnection(): Warning, event fifo is full\n"); 
                            }
                            
                            //VIEWER_SERVER_SESSION_START
                            event.eventNum = VIEWER_SERVER_SESSION_START;
                            event.timeStamp = time(NULL);
                            event.repeaterProcessId = getpid();
                            
                            sessEv.serverTableIndex = serverInd;
                            sessEv.viewerTableIndex = viewerInd;
                            sessEv.code = mode1ConnCode;
                            sessEv.connMode = CONN_MODE1;
                            sessEv.serverIp = serverIp;
                            sessEv.viewerIp = viewerIp;
                            memcpy(event.extraInfo, &sessEv, sizeof(sessionEvent));
                            if (false == sendRepeaterEvent(event)) {
                                debug(LEVEL_1, "acceptConnection(): Warning, event fifo is full\n"); 
                            }
                        }
                    }
                    else {
                        //we have run out of slots in server or viewer table, refuse new connection
                        if (!fServerOk) {
                            debug(LEVEL_3, "acceptConnection(): Mode1: out of slots in server table, closing connection\n"); 
                            close(server);
                        }
                        
                        if (!fViewerOk) {
                            debug(LEVEL_3, "acceptConnection(): Mode1: out of slots in viewer table, closing connection\n"); 
                            close(connection);
                        }

                        return;
                    }
                }
            }
            else {
                debug(LEVEL_3, "acceptConnection():  Mode 1 connections only allowed from viewers, closing connection\n"); 
                close(connection);
                return;
            }
        }
        else if (connMode == CONN_MODE2) {
            if (connectionFrom == CONNECTION_FROM_VIEWER) {
                int serverInd;
                int viewerInd;
                
                viewerInd = addViewerList(connection, code, peerIp);
                if (-1 != viewerInd) {
                    //Send VIEWER_CONNECT to event interface
                    if (useEventInterface) {
                        repeaterEvent event;
                        connectionEvent connEv;
                        addrParts viewerIp;
                                                    
                        //Address in compact binary form
                        viewerIp = getAddrPartsFromString(peerIp);
                            
                        //VIEWER_CONNECT
                        event.eventNum = VIEWER_CONNECT;
                        event.timeStamp = time(NULL);
                        event.repeaterProcessId = getpid();
                        
                        connEv.tableIndex = viewerInd;
                        connEv.code = code;
                        connEv.connMode = CONN_MODE2;
                        connEv.peerIp = viewerIp;
                        memcpy(event.extraInfo, &connEv, sizeof(connectionEvent));
                        if (false == sendRepeaterEvent(event)) {
                            debug(LEVEL_1, "acceptConnection(): Warning, event fifo is full\n"); 
                        }
                    }
                    
                    //New viewer, find respective server
                    serverInd = findServerList(code);
                    if (serverInd != UNKNOWN_REPINFO_IND) {
                        int server;

                        //found respective server, activate viewer and server
                        setViewerActive(code);
                        setServerActive(code);

                        server = servers[serverInd] -> socket;
               
                        //kickstart viewer using handshake received previously (if any) from server
                        if (handShakes[serverInd] -> handShakeLength > 0)
                            writeExact(connection, handShakes[serverInd] -> handShake, 
                                handShakes[serverInd] -> handShakeLength, TIMEOUT_5SECS);
                    
                        //fork repeater
                        forkRepeater(server, connection, code);

                        //Send VIEWER_SERVER_SESSION_START to event interface
                        if (useEventInterface) {
                            repeaterEvent event;
                            sessionEvent sessEv;

                            //VIEWER_SERVER_SESSION_START
                            event.eventNum = VIEWER_SERVER_SESSION_START;
                            event.timeStamp = time(NULL);
                            event.repeaterProcessId = getpid();
                            
                            sessEv.serverTableIndex = serverInd;
                            sessEv.viewerTableIndex = viewerInd;
                            sessEv.code = code;
                            sessEv.connMode = CONN_MODE2;
                            sessEv.serverIp = servers[serverInd] -> peerIp;
                            sessEv.viewerIp = viewers[viewerInd] -> peerIp;
                            memcpy(event.extraInfo, &sessEv, sizeof(sessionEvent));
                            if (false == sendRepeaterEvent(event)) {
                                debug(LEVEL_1, "acceptConnection(): Warning, event fifo is full\n"); 
                            }
                        }
                    }
                    else {
                        debug(LEVEL_3, "acceptConnection(): respective server has not connected yet\n");
                    }
                }
                else {
                    //we have run out of slots in viewer table, refuse new connection
                    debug(LEVEL_3, "acceptConnection(): Mode 2: out of slots in viewer table, closing connection\n"); 
                    close(connection);
                    return;
                }
            }
            else {
                int viewerInd;
                int serverInd;

                //Add server to tables, initialize handshake to nil
                serverInd = addServerList(connection, code, peerIp);
                if (serverInd != -1) {
                    handShakes[serverInd] -> handShakeLength = 0;
    
                    //Send SERVER_CONNECT to event interface
                    if (useEventInterface) {
                        repeaterEvent event;
                        connectionEvent connEv;
                            
                        //SERVER_CONNECT
                        event.eventNum = SERVER_CONNECT;
                        event.timeStamp = time(NULL);
                        event.repeaterProcessId = getpid();
                        
                        connEv.tableIndex = serverInd;
                        connEv.code = code;
                        connEv.connMode = CONN_MODE2;
                        connEv.peerIp = servers[serverInd] -> peerIp;
                        memcpy(event.extraInfo, &connEv, sizeof(connectionEvent));
                        if (false == sendRepeaterEvent(event)) {
                            debug(LEVEL_1, "acceptConnection(): Warning, event fifo is full\n"); 
                        }
                    }

                    //New server, find respective viewer
                    viewerInd = findViewerList(code);
                    if (viewerInd != UNKNOWN_REPINFO_IND) {
                        int viewer;

                        //found respective viewer, activate server and viewer
                        setServerActive(code);
                        setViewerActive(code);

                        viewer = viewers[viewerInd] -> socket;

                        //fork repeater
                        forkRepeater(connection, viewer, code);

                        //Send VIEWER_SERVER_SESSION_START to event interface
                        if (useEventInterface) {
                            repeaterEvent event;
                            sessionEvent sessEv;
                            
                            //VIEWER_SERVER_SESSION_START
                            event.eventNum = VIEWER_SERVER_SESSION_START;
                            event.timeStamp = time(NULL);
                            event.repeaterProcessId = getpid();
                            
                            sessEv.serverTableIndex = serverInd;
                            sessEv.viewerTableIndex = viewerInd;
                            sessEv.code = code;
                            sessEv.connMode = CONN_MODE2;
                            sessEv.serverIp = servers[serverInd] -> peerIp;
                            sessEv.viewerIp = viewers[viewerInd] -> peerIp;
                            memcpy(event.extraInfo, &sessEv, sizeof(sessionEvent));
                            if (false == sendRepeaterEvent(event)) {
                                debug(LEVEL_1, "acceptConnection(): Warning, event fifo is full\n"); 
                            }
                        }
                    }
                    else {
                        debug(LEVEL_3, "acceptConnection(): respective viewer has not connected yet\n");
                    
                        //Read servers' handshake string to buffer, for use when respective 
                        //viewer later connects and needs a kickstart
                        if (serverInd != -1) {
                            readPeerHandShake(connection, serverInd);
                        }
                    }
                }
                else {
                    //we have run out of slots in server table, refuse new connection
                    debug(LEVEL_3, "acceptConnection(): Mode 2: out of slots in server table, closing connection\n"); 
                    close(connection);
                    return;
                }
            }
        }
    }
}

//Initialize listening on port. 
//Listening itself happens on function routeConnections
static void startListeningOnPort(listenPortInfo * pInfo)
{
    int yes = 1;
    struct sockaddr_in name;
    
    pInfo->socket = socket(PF_INET, SOCK_STREAM, 0);

    if (pInfo->socket < 0)
        fatal("startListeningOnPort(): socket() failed, errno=%d (%s)\n", errno, strerror(errno));
    else
        debug(LEVEL_3, "startListeningOnPort(): socket() initialized\n");

    if (setsockopt(pInfo->socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        fatal("startListeningOnPort(): setsockopt() failed, errno=%d (%s)\n", errno, strerror(errno));
    else
        debug(LEVEL_3, "startListeningOnPort(): setsockopt() success\n");

    name.sin_family = AF_INET;

    name.sin_port = htons(pInfo->port);

    name.sin_addr.s_addr = inet_addr(ownIpAddress);

    if (bind(pInfo->socket, (struct sockaddr *) &name, sizeof(name)) < 0)
        fatal("startListeningOnPort(): bind() to (ip: %s, port: %d) failed, errno=%d (%s)\n", 
            ownIpAddress, pInfo -> port, errno, strerror(errno));
    else
        debug(LEVEL_3, "startListeningOnPort(): bind() to (ip: %s, port: %d) succeeded\n", ownIpAddress, pInfo->port);

    if (listen(pInfo->socket, LISTEN_BACKLOG) < 0)
        fatal("startListeningOnPort(): listen() failed, errno=%d (%s)\n", errno, strerror(errno));
    else
        debug(LEVEL_3, "startListeningOnPort(): listen() succeeded\n");

}

//1976578th implementation of max(). Where was it in my system when I needed it ?
static int myMax(int valA, int valB)
{
    return (valA > valB) ? valA : valB;
}

//Listen for new connections on both server and viewer ports, 
//call acceptConnection() to accept them. 
//Periodically also remove old inactive (or broken) connections by calling removeOldOrBrokenConnections()
//Periodically call updateServerViewerInfo() to check changes in servers[]/viewers[] tables
static void routeConnections(int viewerSocket, int serverSocket)
{
    int seconds;
    int heartBeatSeconds;
    fd_set readfds;
    int numfds;
    bool select_ok;
    struct timeval tv;
    const int SELECT_WAIT_SECONDS=1;
    const int CLEANUP_SECONDS=5;
    const int HEARTBEAT_SECONDS=90;
    startEndEvent seEv;
        
    seconds = 0;
    heartBeatSeconds = 0;
    numfds = myMax(viewerSocket, serverSocket) + 1;

    debug(LEVEL_0, "routeConnections(): starting select() loop, terminate with ctrl+c\n");
    while (stopped == false) {
        FD_ZERO(&readfds);
        
        if (viewerSocket != -1)
            FD_SET(viewerSocket, &readfds);
            
        if (serverSocket != -1)
            FD_SET(serverSocket, &readfds);

        tv.tv_sec = SELECT_WAIT_SECONDS;
        tv.tv_usec = 0;
        
        select_ok = true;
        if (-1 == select(numfds, &readfds, NULL, NULL, &tv)) {
            select_ok = false;
            if (stopped == false) {
                debug(LEVEL_2, "routeConnections(): select() failed, errno=%d (%s)\n", errno, strerror(errno));
            }
        }

        if ((select_ok == true) && (stopped == false)) {
            //New viewer trying to connect ?
            if (viewerSocket != -1) {
                if (FD_ISSET(viewerSocket, &readfds)) {
                    debug(LEVEL_3, "routeConnections(): new viewer connecting, accepting...\n");
                    acceptConnection(viewerSocket, CONNECTION_FROM_VIEWER);
                }
            }

            //New server trying to connect ?
            if (serverSocket != -1) {
                if (FD_ISSET(serverSocket, &readfds)) {
                    debug(LEVEL_3, "routeConnections(): new server connecting, accepting...\n");
                    acceptConnection(serverSocket, CONNECTION_FROM_SERVER);
                }
            }

            //Remove old inactive connections, check after every 5 seconds
            seconds += SELECT_WAIT_SECONDS;
            if (seconds >= CLEANUP_SECONDS) {
                seconds = 0;
                removeOldOrBrokenConnections();
            }
            
            //Send REPEATER_HEATBEAT to event interface every 90 seconds
            heartBeatSeconds += SELECT_WAIT_SECONDS;
            if (heartBeatSeconds >= HEARTBEAT_SECONDS) {
                heartBeatSeconds = 0;
                if (useEventInterface) {
                    event.eventNum = REPEATER_HEARTBEAT;
                    event.timeStamp = time(NULL);
                    event.repeaterProcessId = getpid();
     
                    seEv.maxSessions = maxSessions;
                    memcpy(event.extraInfo, &seEv, sizeof(startEndEvent));
                    
                    if (false == sendRepeaterEvent(event)) {
                        debug(LEVEL_1, "routeConnections(): Warning, event fifo is full\n");
                    }
                }
            }
        
            //Clean up after children (Repeaterprocs that have exited)
            cleanUpAfterRepeaterProcs();
            
            //Handle event interface posting & cleanup
            handleRepeaterEvents();
        }
    }
}


//After doRepeater process has exited, this function reads exit code/pid and clears 
//servers[], viewers[] and repeaterProcs[] tables accordingly
static void cleanUpAfterRepeaterProcExit(int exitCode, pid_t pid) {
    long code;
    int index;
    int serverInd;
    int viewerInd;
    
    debug(LEVEL_3, "cleanUpAfterRepeaterProcExit(): exitCode=%d, pid=%d\n", exitCode, pid);
    index = findRepeaterProcList(pid);
    if (index != UNKNOWN_REPINFO_IND) {
        code = repeaterProcs[index] -> code;
        serverInd = findServerList(code);
        viewerInd = findViewerList(code);
        
        if ((serverInd != UNKNOWN_REPINFO_IND) && (viewerInd != UNKNOWN_REPINFO_IND)) {
            //Remove repeaterproc from list
            removeRepeaterProcList(pid);
            
            debug(LEVEL_3, "cleanUpAfterRepeaterProcExit(): code=%ld, serverInd=%d, viewerInd=%d\n", 
                code, serverInd, viewerInd);
        
            //Send VIEWER_SERVER_SESSION_END to event interface
            if (useEventInterface) {
                repeaterEvent event;
                sessionEvent sessEv;
                            
                //VIEWER_SERVER_SESSION_END
                event.eventNum = VIEWER_SERVER_SESSION_END;
                event.timeStamp = time(NULL);
                event.repeaterProcessId = getpid();
                
                sessEv.serverTableIndex = serverInd;
                sessEv.viewerTableIndex = viewerInd;
                sessEv.code = code;
                sessEv.connMode = (code < 0) ? CONN_MODE1 : CONN_MODE2;
                sessEv.serverIp = servers[serverInd] -> peerIp;
                sessEv.viewerIp = viewers[viewerInd] -> peerIp;
                memcpy(event.extraInfo, &sessEv, sizeof(sessionEvent));
                if (false == sendRepeaterEvent(event)) {
                    debug(LEVEL_1, "cleanUpAfterRepeaterProcExit(): Warning, event fifo is full\n");
                }
            }
        
            switch(exitCode) {
                case 1:
                    //Error in select(), fall through
                case 2:
                    //Server has disconnected, fall through
                case 3:                
                    //Viewer has disconnected, fall through
                case 4:                
                    //Error when reading from viewer, fall through
                case 5:
                    //Error when reading from server
                    debug(LEVEL_1, "cleanUpAfterRepeaterProcExit(): closing connection (server=%d, viewer=%d)\n", 
                        servers[serverInd] -> socket, viewers[viewerInd] -> socket);
                    removeServerList(code);
                    removeViewerList(code);
                    break;

                default:
                    break;
            }
        }
        else {
            debug(LEVEL_2, "cleanUpAfterRepeaterProcExit(): illegal viewerInd = %d or serverInd =%d\n", 
                viewerInd, serverInd);
        }        
    }
    else {
        debug(LEVEL_2, "cleanUpAfterRepeaterProcExit(): proc not found\n");
    }
}
        
//Check each possible children and clean up after they have exited
static void cleanUpAfterRepeaterProcs(void)
{
    int status;
    pid_t pid;
    int i;
    
    for(i = 0; i < maxSessions; i++) {
        if (repeaterProcs[i] -> code != 0) {
            pid = waitpid(repeaterProcs[i] -> pid, &status, WNOHANG);
            if (pid > 0) {
                cleanUpAfterRepeaterProcExit(WEXITSTATUS(status), pid);
            }
        } 
    }
}

//Terminate program with ctrl+c cleanly
static void handleSigInt(int s)
{
    stopped = true;
}


//Announce what initializations we got
static void listInitializationValues(void)
{
    int ii;
    
    debug(LEVEL_2, "listInitializationValues(): viewerPort : %d\n", viewerPort);
    debug(LEVEL_2, "listInitializationValues(): serverPort : %d\n", serverPort);
    debug(LEVEL_2, "listInitializationValues(): maxSessions: %d\n", maxSessions);
    debug(LEVEL_2, "listInitializationValues(): loggingLevel: %d\n", loggingLevel);
    debug(LEVEL_2, "listInitializationValues(): ownIpAddress (0.0.0.0 = listen all interfaces) : %s\n", ownIpAddress);
    debug(LEVEL_2, "listInitializationValues(): runAsUser (if started as root) : %s\n", runAsUser);
    
    debug(LEVEL_2, "listInitializationValues(): Mode 1 connections allowed : %s\n", 
        ((allowedModes & CONN_MODE1) > 0) ? "Yes" : "No");
    debug(LEVEL_2, "listInitializationValues(): Mode 2 connections allowed : %s\n", 
        ((allowedModes & CONN_MODE2) > 0) ? "Yes" : "No");

    debug(LEVEL_2, "listInitializationValues(): Mode 1 allowed server port (0=All) : %d\n", allowedMode1ServerPort);
    debug(LEVEL_2, "listInitializationValues(): Mode 1 requires listed addresses : %s\n", 
        (requireListedServer == 1) ? "Yes" : "No");
    if (requireListedServer == 1) {
        //Allow list
        if (LEVEL_2 <= loggingLevel) {
            debug(LEVEL_2, "listInitializationValues(): Mode 1 allowed servers/networks (255=Not allowed):");
            for(ii = 0; ii < SERVERS_LIST_SIZE; ii++) {
                fprintf(stderr, " %d.%d.%d.%d", srvListAllow[ii].a, srvListAllow[ii].b, 
                    srvListAllow[ii].c, srvListAllow[ii].d);
            }
            fprintf(stderr, "\n");
        }

        //Deny list
        if (LEVEL_2 <= loggingLevel) {
            debug(LEVEL_2, "listInitializationValues(): Mode 1 denied servers/networks (255=Not denied):");
            for(ii = 0; ii < SERVERS_LIST_SIZE; ii++) {
                fprintf(stderr, " %d.%d.%d.%d", srvListDeny[ii].a, srvListDeny[ii].b, srvListDeny[ii].c, srvListDeny[ii].d);
            }
            fprintf(stderr, "\n");
        }
    }

    debug(LEVEL_2, "listInitializationValues(): Mode 2 requires listed ID numbers : %s\n",
        (requireListedId == 1) ? "Yes" : "No");
    
    if (requireListedId == 1) {
        if (LEVEL_2 <= loggingLevel) {
            debug(LEVEL_2, "listInitializationValues(): Mode 2 allowed ID list (0=Not allowed):");
        
            for(ii = 0; ii < ID_LIST_SIZE; ii++) {
                fprintf(stderr, " %d", idList[ii]);
            }
            fprintf(stderr, "\n");
        }
    }

    debug(LEVEL_2, "listInitializationValues(): useEventInterface: %s\n", 
    	(useEventInterface) ? "true" : "false");
    
    debug(LEVEL_2, "listInitializationValues(): eventListenerHost : %s\n", eventListenerHost);
    
    debug(LEVEL_2, "listInitializationValues(): eventListenerPort : %d\n", eventListenerPort);
    
    debug(LEVEL_2, "listInitializationValues(): useHttpForEventListener : %s\n", 
    	(useHttpForEventListener) ? "true" : "false");
}

//After bind() we drop to mere mortal privileges (in case we started as root)
//to limit damages in case of security flaws in this program  
//In case of error, calls fatal() which sets up a clean exit from program
static void dropRootPrivileges()
{
    struct passwd *pw;
    
    pw = getpwnam(runAsUser);

    if (pw != NULL) {
        if (0 != setgid(pw -> pw_gid)) {
            fatal("dropRootPrivileges(): setgid() failed\n");    
        }

        if (0 != setuid(pw -> pw_uid)) {
            fatal("dropRootPrivileges(): setuid() failed\n");    
        }
                                
        //We should now be mere mortal, check effective uid to be sure
        if (geteuid() == 0) {
            //Still root, was this intended ? 
            if (strcmp("root", runAsUser) == 0) {
                //Intentionally root, complain about this security violation
                debug(LEVEL_1, "dropRootPrivileges(): you seem to WANT TO run as user root, this IS VERY DANGEROUS !\n");
            }
            else
                fatal("dropRootPrivileges(): dropping privileges failed\n");
        }
        else
            debug(LEVEL_1, "dropRootPrivileges(): privileges successfully dropped, now running as user %s\n", runAsUser);
    }
    else
        fatal("dropRootPrivileges(): getpwnam() failed\n");    
}


int main(int argc, char **argv)
{
    //Viewer port listener variable
    listenPortInfo viewerListener = {-1, -1};

    //Server port listener variable
    listenPortInfo serverListener = {-1, -1};

    //ctrl+c signal handler
    struct sigaction saInt;

    //ini file default
    char defaultIniFilePathAndName[] = "/etc/uvnc/uvncrepeater.ini";
    char tmpBuf[MAX_PATH];
    bool memoryOk;

    //Startup event
    startEndEvent seEv;
    
    stopped = false;
    mode1ConnCode = 0;
    
    fprintf(stderr, "UltraVnc Linux Repeater version %s\n", REPEATER_VERSION);
    
    //Read parameters from ini file
    strlcpy(tmpBuf, (argc >= 2) ? argv[1] : defaultIniFilePathAndName, MAX_PATH);
    if (false == readIniFile(tmpBuf)) {
        debug(LEVEL_1, "main(): ini file (%s) read error, using defaults\n", tmpBuf);
    }
    listInitializationValues();

    //Send startup event to event interface 
    initRepeaterEventInterface();
    if (useEventInterface) {
        event.eventNum = REPEATER_STARTUP;
        event.timeStamp = time(NULL);
        event.repeaterProcessId = getpid();
        
        seEv.maxSessions = maxSessions;
        memcpy(event.extraInfo, &seEv, sizeof(startEndEvent));
        
        //Event fifo can not be full here ;-)
        sendRepeaterEvent(event);
    }

    //Allocate and clean various repeater lists
    if (false == allocateMemoryForRepeaterLists(maxSessions)) {
        debug(LEVEL_0, "main(): memory allocation for repeater lists failed, exiting program\n");
        memoryOk = false;
    }
    else {
        memoryOk = true;
        cleanLists();
    }

    //If we got lists allocated, we can run
    if (memoryOk) {
        //Initialize ctrl+c signal handler
        memset(&saInt, 0, sizeof(saInt));
    
        //Restart interrupted system calls after handler returns    
        saInt.sa_flags = SA_RESTART;    
        saInt.sa_handler = &handleSigInt;
        sigaction(SIGINT, &saInt, NULL);

        //Initialize and start listening on viewer port
        if (((allowedModes & CONN_MODE1) > 0) || ((allowedModes & CONN_MODE2) > 0)) {
            viewerListener.port = viewerPort;
            startListeningOnPort(&viewerListener);
        }

        //Initialize and start listening on server port
        if ((allowedModes & CONN_MODE2) > 0) {
            serverListener.port = serverPort;
            startListeningOnPort(&serverListener);
        }

        //Drop root privileges (if we are running as root) after all listen ports have been bound
        if (geteuid() == 0) {
            dropRootPrivileges(); 
        }

        //Accept & Route new connections
        if (stopped == false) {
            routeConnections(viewerListener.socket, serverListener.socket);
        }

        close(viewerListener.socket);
        close(serverListener.socket);
    }

    //Free allocated memory of repeater lists
    freeMemoryOfRepeaterLists();

    //Send shutdown event to event interface
    if (useEventInterface) {
        event.eventNum = REPEATER_SHUTDOWN;
        event.timeStamp = time(NULL);
        event.repeaterProcessId = getpid();
        
        seEv.maxSessions = maxSessions;
        memcpy(event.extraInfo, &seEv, sizeof(startEndEvent));
        if (false == sendRepeaterEvent(event)) {
            debug(LEVEL_1, "main(): Warning, event fifo is full\n");
        }
        handleRepeaterEvents();
    }

    debug(LEVEL_0, "main(): relaying done.\n");

    return 0;
}
