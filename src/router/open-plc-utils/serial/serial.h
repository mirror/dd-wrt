/*====================================================================*
 *
 *   Copyright (c) 2013 Qualcomm Atheros, Inc.
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or 
 *   without modification, are permitted (subject to the limitations 
 *   in the disclaimer below) provided that the following conditions 
 *   are met:
 *
 *   * Redistributions of source code must retain the above copyright 
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above 
 *     copyright notice, this list of conditions and the following 
 *     disclaimer in the documentation and/or other materials 
 *     provided with the distribution.
 *
 *   * Neither the name of Qualcomm Atheros nor the names of 
 *     its contributors may be used to endorse or promote products 
 *     derived from this software without specific prior written 
 *     permission.
 *
 *   NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE 
 *   GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
 *   COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 *   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 *   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER 
 *   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 *   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *
 *   serial.h - Atheros Serial Line Definitions and declarations;
 *
 *.  Qualcomm Atheros HomePlug AV Powerline Toolkit
 *:  Copyright (c) 2009-2013 by Qualcomm Atheros Inc. ALL RIGHTS RESERVED;
 *;  For demonstration and evaluation only; Not for production use.
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef SERIAL_HEADER
#define SERIAL_HEADER

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <ctype.h>

#ifndef WIN32
#include <termios.h>
#endif

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/types.h"

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#if defined (WIN32)
#       define DEVICE "com1:"
#       define PAUSE(x) Sleep (x)
#else
#       define DEVICE "/dev/ttyUSB0"
#       define PAUSE(x) usleep ((x)*1000)
#endif

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#define UART_MODE 0
#define UART_BAUDRATE 115200
#define UART_DATABITS 8
#define UART_STOPBITS 1
#define UART_PARITY 0
#define UART_FLOWCTRL 0

#define UART_BUFFERSIZE 4096
#define UART_BLOCKSIZE 512
#define UART_PORT "PLCUART"

/*====================================================================*
 *   Atheros Serial Line Command flags;
 *--------------------------------------------------------------------*/

#define UART_SILENCE    (1 << 0)
#define UART_VERBOSE    (1 << 1)
#define UART_DEFAULT    (1 << 2)
#define UART_COMMAND    (1 << 3)
#define UART_WAKE       (1 << 4)
#define UART_RESPOND    (1 << 5)
#define UART_ATRV       (1 << 6)
#define UART_ATRPM      (1 << 7)
#define UART_ATSK1      (1 << 8)
#define UART_ATSK2      (1 << 9)
#define UART_ATZ        (1 << 10)
#define UART_ATDST1     (1 << 11)
#define UART_ATDST2     (1 << 12)
#define UART_ATNI       (1 << 13)
#define UART_ATFD       (1 << 14)
#define UART_ATPS       (1 << 15)
#define UART_ATO        (1 << 16)
#define UART_ATHSC      (1 << 17)
#define UART_ATRP       (1 << 18)
#define UART_ATWPF1     (1 << 19)
#define UART_ATWPF2     (1 << 20)
#define UART_ATWNV      (1 << 21)
#define UART_ATBSZ1     (1 << 22)
#define UART_ATBSZ2     (1 << 23)
#define UART_ATTO       (1 << 24)
#define UART_ATBR       (1 << 25)
#define UART_ATM        (1 << 26)

/*====================================================================*
 *
 *   struct command;
 *
 *   structure containing serial command buffer and two pointers; the
 *   buffer is used for both transmit and receive and shared by most
 *   function below;
 *
 *--------------------------------------------------------------------*/

typedef struct command

{
	char buffer [UART_BUFFERSIZE];
	unsigned length;
	unsigned offset;
}

COMMAND;

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#ifndef WIN32

signed baudrate (unsigned rate, speed_t * speed);

#endif

void openport (struct _file_ * port, flag_t mode);
void closeport (struct _file_ * port);

/*====================================================================*
 *   serial command line buffer functions;
 *--------------------------------------------------------------------*/

void clearcommand ();
void sendcommand (struct _file_ * port, flag_t flags);
void readcommand (struct _file_ * port, flag_t flags);
void insert (char c);
size_t readframe (signed fd, void * memory, size_t extent);
void decode (void const * memory, size_t extent);
void encode (void * memory, size_t extent);
void string (char * string);
uint64_t hextoint (unsigned bytes);
void mustbe (char c);;

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif

