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
#include <string.h>
#include <stdlib.h>

#include "openbsd_stringfuncs.h"
#include "iniparser.h"
#include "commondefines.h"
#include "repeaterutil.h"

//Ports where we listen 
int viewerPort = 5900;
int serverPort = 5500;

//Repeater allowed modes 
//CONN_MODE1 = Only Mode 1, 
//CONN_MODE2 = Only Mode 2, 
//(CONN_MODE1 | CONN_MODE2) = Both modes
int allowedModes = (CONN_MODE1 | CONN_MODE2);

//In mode 1, what server port we can connect() to. 
//0=All ports
int allowedMode1ServerPort = 0; 

//In mode 2, require listed IDs ? (0=Allow all IDs, 1=Require that ID number is in the list)
int requireListedId = 0;

//In Mode 2 (in case allowOnlyListedIds == 1) what IDs are allowed ?
//0 here does not match anything
int idList[ID_LIST_SIZE] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//if running as root, by default change to this user after all listen ports have been bound
char runAsUser[MY_TMP_BUF_LEN] = "uvncrep";

//if we have several ip addresses, we can choose one to listen on
//by default, listen on all addresses
char ownIpAddress[MY_TMP_BUF_LEN] = "0.0.0.0";

//In mode 1, do we require server addresses/ranges listed in ini file ?
//0=Allow all server addresses, 1=Only allow addresses/ranges defined in ini file
int requireListedServer = 0;

//Mode 1 allowed servers (255.255.255.255 = Not allowed)
addrParts srvListAllow[SERVERS_LIST_SIZE] = {
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255}};

//Mode 1 denied servers (255.255.255.255 = Not denied)
addrParts srvListDeny[SERVERS_LIST_SIZE] = {
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},\
    {255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255},{255,255,255,255}};


//How many active sessions we can have ?
int maxSessions = MAX_SESSIONS_DEFAULT;


//Logging level
int loggingLevel = DEFAULT_LOGGING_LEVEL;

//Use event interface ?
bool useEventInterface = false;

//EventListener hostname/ip address and port/http usage
char eventListenerHost[MY_TMP_BUF_LEN] = "localhost";
int eventListenerPort = 2002;
bool useHttpForEventListener = false;
 
bool readIniFile(char *iniFilePathAndName)
{
    dictionary *dict;
    int ii;
    char tmpBuf[MY_TMP_BUF_LEN];
    char tmpBuf2[MY_TMP_BUF_LEN];
    
    dict = iniparser_new(iniFilePathAndName);
    
    if (dict != NULL) {        
        //General settings
        //General settings
        viewerPort = iniparser_getint(dict, "general:viewerport", 5900);    
        serverPort = iniparser_getint(dict, "general:serverport", 5500);    

        maxSessions = iniparser_getint(dict, "general:maxsessions", MAX_SESSIONS_DEFAULT);
        if (maxSessions > MAX_SESSIONS_MAX)
            maxSessions = MAX_SESSIONS_DEFAULT;
        if (maxSessions < MAX_SESSIONS_MIN)
            maxSessions = MAX_SESSIONS_DEFAULT;

        allowedModes = iniparser_getint(dict, "general:allowedmodes", (CONN_MODE1 | CONN_MODE2));     
        
        strlcpy(ownIpAddress, iniparser_getstring(dict, "general:ownipaddress", "0.0.0.0"), MY_TMP_BUF_LEN);
        
        strlcpy(runAsUser, iniparser_getstring(dict, "general:runasuser", "uvncrep"), MY_TMP_BUF_LEN);
                
        loggingLevel = iniparser_getint(dict, "general:logginglevel", DEFAULT_LOGGING_LEVEL);     
        if (loggingLevel > LEVEL_3)
            loggingLevel = DEFAULT_LOGGING_LEVEL;
        if (loggingLevel < LEVEL_0)
            loggingLevel = DEFAULT_LOGGING_LEVEL;

        
        //Mode 1 settings
        //Mode 1 settings
        allowedMode1ServerPort = iniparser_getint(dict, "mode1:allowedmode1serverport", 0);        
        requireListedServer = iniparser_getint(dict, "mode1:requirelistedserver", 0);        
        if (requireListedServer == 1) {
            //Allowed servers
            for(ii = 0; ii < SERVERS_LIST_SIZE; ii++) {
                snprintf(tmpBuf, MY_TMP_BUF_LEN, "%s%d", "mode1:srvListAllow", ii);
                
                strlcpy(tmpBuf2, iniparser_getstring(dict, tmpBuf, "255.255.255.255"), MY_TMP_BUF_LEN);
                
                srvListAllow[ii] = getAddrPartsFromString(tmpBuf2);
            }


            //Denied servers
            for(ii = 0; ii < SERVERS_LIST_SIZE; ii++) {
                snprintf(tmpBuf, MY_TMP_BUF_LEN, "%s%d", "mode1:srvListDeny", ii);
                
                strlcpy(tmpBuf2, iniparser_getstring(dict, tmpBuf, "255.255.255.255"), MY_TMP_BUF_LEN);
                
                srvListDeny[ii] = getAddrPartsFromString(tmpBuf2);
            }
        }
        
        
        //Mode 2 settings
        //Mode 2 settings
        requireListedId = iniparser_getint(dict, "mode2:requirelistedid", 0);        
        if (requireListedId == 1) {
            for(ii = 0; ii < ID_LIST_SIZE; ii++) {
                snprintf(tmpBuf, MY_TMP_BUF_LEN, "%s%d", "mode2:idlist", ii);
                idList[ii] = iniparser_getint(dict, tmpBuf, 0);
            }
        }
       
        //Event interface settings
        //Event interface settings
        useEventInterface = (bool) iniparser_getboolean(dict, "eventinterface:useeventinterface", 0);     

        strlcpy(eventListenerHost, iniparser_getstring(dict, 
            "eventinterface:eventlistenerhost", "localhost"), MY_TMP_BUF_LEN);

        eventListenerPort = iniparser_getint(dict, "eventinterface:eventlistenerport", 2002);     
        
        useHttpForEventListener = (bool) iniparser_getboolean(dict,"eventinterface:usehttp", 0);     
        
        iniparser_free(dict);
        return true;           
    }
    else 
        return false;
        
    return true;
}
