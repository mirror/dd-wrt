/* simple.c -- the annotated simple example program for the LZO library

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
 */


#pragma pack(2)
#include <Common.h>
#include <System/SysAll.h>
#include <UI/UIAll.h>

#if 1
#include <lzo1x.h>
#undef LZO1X_1_MEM_COMPRESS
#define LZO1X_1_MEM_COMPRESS    LZO1X_1_11_MEM_COMPRESS
#define lzo1x_1_compress        lzo1x_1_11_compress
#define IN_LEN  8192
#endif

#define main    test
#include "examples/simple.c"
#undef main


/*************************************************************************
//
**************************************************************************/

DWord PilotMain(Word cmd, Ptr cmdPBP, Word launchFlags)
{
    int r;

    if (cmd != 0)
        return 0;

    r = test(0,NULL);

    for (;;)
    {
        EventType e;

        EvtGetEvent(&e,100);
        if (e.eType == appStopEvent)
            break;
        if (!SysHandleEvent(&e))
            FrmHandleEvent(FrmGetActiveForm(),&e);
    }
    return 0;
}


/*
vi:ts=4
*/

