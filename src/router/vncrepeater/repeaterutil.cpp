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

#include "repeaterutil.h"
#include "openbsd_stringfuncs.h"

//Parse ip address (i'm sure similar function has been recoded thousands of times ;-)
//Return ip address in structure form
addrParts getAddrPartsFromString(char *ipString)
{
    addrParts addr; 
    char tmpBuf[MY_TMP_BUF_LEN + 1];
    int ipVal[4] = {255,255,255,255};
    char *p;
    char *start;
    long value;
    int ii;
    
    strlcpy(tmpBuf, ipString, MY_TMP_BUF_LEN + 1);
    strlcat(tmpBuf, ".", MY_TMP_BUF_LEN + 1);
    start = tmpBuf;
    
    for(ii = 0; ii < 4; ii++) {
        p = strchr(start, '.');
        if (p != NULL) {
            *p = '\0';
            
            value = strtol(start, NULL, 10);
            
            if ((value < 0) || (value > 255)) 
                ipVal[ii] = 255;
            else
                ipVal[ii] = value;

            start = p;
            start++;
        }
        else {
            ipVal[ii] = 255;
        }
    }
    
    addr.a = ipVal[0];
    addr.b = ipVal[1];
    addr.c = ipVal[2];
    addr.d = ipVal[3];
    
    return addr;    
}
