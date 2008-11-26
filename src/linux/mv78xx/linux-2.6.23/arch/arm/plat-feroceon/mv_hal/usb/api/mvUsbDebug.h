/*******************************************************************************

This software file (the "File") is distributed by Marvell International Ltd. 
or its affiliate(s) under the terms of the GNU General Public License Version 2, 
June 1991 (the "License").  You may use, redistribute and/or modify this File 
in accordance with the terms and conditions of the License, a copy of which 
is available along with the File in the license.txt file or by writing to the 
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 
or on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.

(C) Copyright 2004 - 2007 Marvell Semiconductor Israel Ltd. All Rights Reserved.
(C) Copyright 1999 - 2004 Chipidea Microelectronica, S.A. All Rights Reserved.

*******************************************************************************/

#ifndef __mvUsbDebug_h__
#define __mvUsbDebug_h__

#include "mvUsbTypes.h"

#define MV_USB_RT_DEBUG

/************************************************************
The following array is used to make a run time trace route
inside the USB stack.
*************************************************************/

#define ARC_DEBUG_FLAG_ANY      0x00000000

#define ARC_DEBUG_FLAG_TRACE    0x00000001
#define ARC_DEBUG_FLAG_CTRL     0x00000002
#define ARC_DEBUG_FLAG_RX       0x00000004
#define ARC_DEBUG_FLAG_TX       0x00000008
#define ARC_DEBUG_FLAG_STALL    0x00000010
#define ARC_DEBUG_FLAG_STATUS   0x00000020
#define ARC_DEBUG_FLAG_TRANSFER 0x00000040
#define ARC_DEBUG_FLAG_INIT     0x00000080
#define ARC_DEBUG_FLAG_ISR      0x00000100
#define ARC_DEBUG_FLAG_ERROR    0x00000200
#define ARC_DEBUG_FLAG_ADDR     0x00000400
#define ARC_DEBUG_FLAG_DUMP     0x00000800
#define ARC_DEBUG_FLAG_SETUP    0x00001000
#define ARC_DEBUG_FLAG_CLASS    0x00002000
#define ARC_DEBUG_FLAG_SPEED    0x00004000
#define ARC_DEBUG_FLAG_RESET    0x00008000
#define ARC_DEBUG_FLAG_SUSPEND  0x00010000
#define ARC_DEBUG_FLAG_RESUME   0x00020000
#define ARC_DEBUG_FLAG_EP0      0x00040000
#define ARC_DEBUG_FLAG_EP1      0x00080000
#define ARC_DEBUG_FLAG_STATS    0x00100000


#define ARC_DEBUG_FLAG_ALL      0xffffffff

extern uint_32  usbDebugFlags;

#ifdef MV_USB_RT_DEBUG
#   define ARC_DEBUG_CODE(flags, code)    	        \
        if( (usbDebugFlags & (flags)) == (flags) )  \
            code
#else
#   define ARC_DEBUG_CODE(flags, code)
#endif

#if defined(MV_USB_TRACE_LOG)

#define TRACE_ARRAY_SIZE 400
#define MAX_STRING_SIZE  132

extern uint_16 DEBUG_TRACE_ARRAY_COUNTER;
extern char    DEBUG_TRACE_ARRAY[TRACE_ARRAY_SIZE][MAX_STRING_SIZE];

#define ARC_DEBUG_TRACE(flags, format, x...)                                        \
{                                                                                   \
    if( (usbDebugFlags & (flags)) == (flags))                                   \
    {                                                                               \
        USB_sprintf(DEBUG_TRACE_ARRAY[DEBUG_TRACE_ARRAY_COUNTER], format, ##x);     \
        DEBUG_TRACE_ARRAY_COUNTER++;                                                \
        if(DEBUG_TRACE_ARRAY_COUNTER >= TRACE_ARRAY_SIZE)                           \
            {DEBUG_TRACE_ARRAY_COUNTER = 0;}                                        \
    }                                                                               \
}
                                                    
#elif defined(MV_USB_TRACE_PRINT)

#   define ARC_DEBUG_TRACE(flags, format, x...)           \
        if((usbDebugFlags & (flags)) == (flags))      \
            USB_printf(format, ##x)

/*if trace switch is not enabled define debug log trace to empty*/
#else
#   define ARC_DEBUG_TRACE(flags, fromat, x...)
#endif


/************************************************************
The following are global data structures that can be used
to copy data from stack on run time. This structure can
be analyzed at run time to see the state of various other
data structures in the memory.
*************************************************************/

#endif /* __mvUsbDebug_h__ */
/* EOF */
