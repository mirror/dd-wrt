/* $Id$ */
/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 2002 Martin Roesch <roesch@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "snort.h"
#include "util.h"


/****************************************************************************
 *
 * Function: gettimeofday(struct timeval *, struct timezone *)
 *
 * Purpose:  Get current time of day.
 *
 * Arguments: tv => Place to store the curent time of day.
 *            tz => Ignored.
 *
 * Returns: 0 => Success.
 *
 ****************************************************************************/

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    struct _timeb tb;

	if(tv==NULL)
    {
        return -1;
    }
	_ftime(&tb);
	tv->tv_sec = (unsigned long)tb.time;
	tv->tv_usec = ((int) tb.millitm) * 1000;
	return 0;
}

/****************************************************************************
 *
 * Function: GetAdapterFromList(void *,int)
 *
 * Purpose:  Get a specific adapter from the list of adapters on the system.
 *
 * Arguments: device => Device to look for.
 *            index => Adapter number.
 *
 * Returns: Adapter if device was valid.
 *
 * Comments: Shamelessly ripped from WinDump.
 *
 ****************************************************************************/

void *GetAdapterFromList(void *device,int index)
{
    DWORD dwVersion;
    DWORD dwWindowsMajorVersion;
    char *Adapter95;
    WCHAR *Adapter;
    int i;

    dwVersion = GetVersion();
    dwWindowsMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));

    /* Windows 95. */
    if (dwVersion >= 0x80000000 && dwWindowsMajorVersion >= 4)
    {
        Adapter95 = device;
        for( i=0; i<index-1; i++ )
        {
            while(*Adapter95++ != 0)
            {
            }
            if( *Adapter95 == 0 )
            {
				return NULL;
            }
        }
        return  Adapter95;
    }
    else
    {
		/* NT. */
        Adapter = (WCHAR *) device;
        for( i=0; i<index-1; i++ )
        {
            while( *Adapter++ != 0 )
            {
            }
            if( *Adapter == 0 )
            {
                return NULL;
            }
        }
        return Adapter;
    }
}

/****************************************************************************
 *
 * Function: print_interface(char *)
 *
 * Purpose:  Print the interface number. Platform Independent.
 *
 * Arguments: interface => Name of Interface to print.
 *
 * Returns: Correct character format of Interface for the current platform.
 *
 * Comments: Shamelessly ripped from WinDump.
 *
 ****************************************************************************/

char *print_interface(const char *szInterface)
{
    static char device[128];

    if (szInterface == NULL)
        return("NULL");

    /* Device always ends with a double \0, so this way to
       determine its length should be always valid */
    if(IsTextUnicode(szInterface, wcslen((wchar_t *)szInterface), NULL))
        SnortSnprintf(device, 128, "%S", (wchar_t *)szInterface);
    else
        SnortSnprintf(device, 128, "%s", szInterface);

    return(device);
}

/****************************************************************************
 *
 * Function: PrintDeviceList(const char *)
 *
 * Purpose:  Print all interfaces forund on the system that we can listen on.
 *
 * Arguments: device => List of all devices to listen on.
 *
 * Returns: void function.
 *
 * Comments: Shamelessly ripped from WinDump.
 *
 ****************************************************************************/

void PrintDeviceList(const char *device)
{
    DWORD dwVersion;
    DWORD dwWindowsMajorVersion;
    const WCHAR* t;
    const char* t95;
    int i=0;
    int DescPos=0;
    char *Desc;
    int n=1;

    dwVersion=GetVersion();
    dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));

    /* Windows 95. */
    if (dwVersion >= 0x80000000 && dwWindowsMajorVersion >= 4)
    {
        t95 = (char*)device;

        while(*(t95+DescPos)!=0 || *(t95+DescPos-1)!=0)
        {
            DescPos++;
        }

        Desc=(char*)t95+DescPos+1;
        printf("\nInterface\tDevice\t\tDescription\n-------------------------------------------\n");
        printf("%d  ",n++);

        while ( ! (t95[i]==0 && t95[i-1]==0) )
        {
            if ( t95[i] == 0 )
            {
                putchar(' ');
                putchar('(');
                while( *Desc !=0 )
                {
                    putchar(*Desc);
                    Desc++;
                }
                Desc++;
                putchar(')');
                putchar('\n');
            }
            else
            {
                putchar(t95[i]);
            }

            if( (t95[i]==0) && (t95[i+1]!=0) )
            {
                printf("%d ",n++);
            }

            i++;
        }
        putchar('\n');
    }
    else
    {
        /* WinNT. */
        t = (WCHAR*) device;
        while( *(t+DescPos)!=0 || *(t+DescPos-1)!=0 )
        {
                DescPos++;
        }
        DescPos <<= 1;
        Desc = (char*)t+DescPos+2;
        printf("\nInterface\tDevice\t\tDescription\n-------------------------------------------\n");
        printf("%d  ",n++);

        while ( ! ( t[i]==0 && t[i-1]==0 ) )
        {
            if ( t[i] == 0 )
            {
                putchar(' ');
                putchar('(');
                while( *Desc != 0 )
                {
                    putchar(*Desc);
                    Desc++;
                }
                Desc++;
                putchar(')');
                putchar('\n');
            }
            else
            {
                putchar(t[i]);
            }

            if( t[i]==0 && t[i+1]!=0 )
            {
                printf("%d ",n++);
            }

            i++;
        }
        putchar('\n');
    }
}

/****************************************************************************
 *
 * Function: init_winsock(void)
 *
 * Purpose:  Initialize winsock.
 *
 * Arguments: None.
 *
 * Returns: 0 => Initilization failed.
 *          1 => Initilization succeeded.
 *
 ****************************************************************************/

int init_winsock(void)
{
    WORD wVersionRequested = MAKEWORD(1, 1);
    WSADATA wsaData;

    if (WSAStartup(wVersionRequested, &wsaData))
    {
        FatalError("[!]: Unable to find a usable Winsock.\n");
    }

    if (LOBYTE(wsaData.wVersion) < 1 || HIBYTE(wsaData.wVersion) < 1)
    {
        FatalError("[!]: Unable to find Winsock version 1.1 or greater. You have version %d.%d.\n",
	               LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
        //WSACleanup();
        //return 0;
    }

    return 1;
}

int geteuid(void)
{
	return 0;
}

/****************************************************************************
 *
 * Function: ffs(int x)
 *
 * Purpose:  find first bit set in x
 *
 * Arguments: int x => integer in which to find bit
 *
 * Returns: bit => position of first LSB that is set in x
 *
 ****************************************************************************/
int ffs(int x)
{
	int bit = 1;
	int mask = 1;

	if (x == 0)
		return 0;

	while ((x & mask) == 0)
	{
		mask = mask << 1;
		bit += 1;

	}

	if (bit > (sizeof(int)*8))
		bit = 0;

	return bit;
}