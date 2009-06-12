//=============================================================================
//
//      io_utils.c - Cyclone Diagnostics
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   Scott Coulter, Jeff Frazier, Eric Breeden
// Contributors:
// Date:        2001-01-25
// Purpose:     
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

/*
 * i/o routines for tests.  Greg Ames, 9/17/90.
 *
 * Version: @(#)test_io.c	1.2 8/26/93
 */
#include <redboot.h>
#include <cyg/infra/diag.h>
#define printf diag_printf


#define TRUE	1
#define FALSE	0


#define ASCII_TO_DEC 48
void atod(char a, int* b)
{
    *b = (int)(a - ASCII_TO_DEC);
}

char xgetchar(void)
{
    hal_virtual_comm_table_t* __chan = CYGACC_CALL_IF_CONSOLE_PROCS();
    
    if (__chan == NULL)
        __chan = CYGACC_CALL_IF_DEBUG_PROCS();

    return CYGACC_COMM_IF_GETC(*__chan);
}

int xgetchar_timeout(char *ch, int msec)
{
    bool res;
    int old_to;
    hal_virtual_comm_table_t *__chan;

    __chan = CYGACC_CALL_IF_CONSOLE_PROCS();
    if (__chan == NULL)
	__chan = CYGACC_CALL_IF_DEBUG_PROCS();

    old_to = CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_SET_TIMEOUT, msec);
    res = CYGACC_COMM_IF_GETC_TIMEOUT(*__chan, ch);
    CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_SET_TIMEOUT, old_to);

    return res;
}

/*
 * naive implementation of "gets"
 * (big difference from fgets == strips newline character)
 */
char* sgets(char *s)
{
    char *retval = s;
    char ch;
 
    while ((ch = (char)xgetchar())) {
	if (ch == 0x0d) { /* user typed enter */
	    printf("\n");
	    break;
	}
	if (ch == 0x08) { /* user typed backspace */
	    printf ("\b");
	    printf (" ");
	    printf ("\b");
	    s--;
	} else { /* user typed another character */
	    printf("%c", ch);
	    *s++ = ch;   
	}
    }
  
    *s = '\0';
    return retval; 
}


/* Returns true if theChar is a valid hex digit, false if not */
char ishex(char theChar)
{
    switch(theChar) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'A':
    case 'a':
    case 'B':
    case 'b':
    case 'C':
    case 'c':
    case 'D':
    case 'd':
    case 'E':
    case 'e':
    case 'F':
    case 'f':
	return 1;
    default:
	return 0;
    }
}


/* Returns true if theChar is a valid decimal digit, false if not */
char isdec(char theChar)
{
    switch(theChar) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	return 1;
    default:
	return 0;
    }
}

/* Convert ascii code of hex digit to number (0-15) */
char hex2dec(char hex)
{
    if ((hex >= '0') && (hex <= '9'))
        return hex - '0';
    else if ((hex >= 'a') && (hex <= 'f'))
        return hex - 'a' + 10;
    else
        return hex - 'A' + 10;
}
 

/* Convert number (0-15) to ascii code of hex digit */
char dec2hex(char dec)
{
    return (dec <= 9) ? (dec + '0') : (dec - 10 + 'A');
}


/* Output an 8 bit number as 2 hex digits */
void hex8out(unsigned char num)
{
    printf("%02X",num);
}


/* Output an 32 bit number as 8 hex digits */
void hex32out(unsigned long num)
{
    printf("%08X",num);
}


/* Input a number as (at most 8) hex digits - returns value entered */
long hexIn(void)
{
    char input[40];
    long num;
    register int i;

    i = 0;
    num = 0;

    if (sgets (input)) { /* grab a line */
        num = hex2dec(input[i++]);            /* Convert MSD to dec */
        while(ishex(input[i]) && input[i]) {  /* Get next hex digit */
            num <<= 4;						/* Make room for next digit */
            num += hex2dec(input[i++]); 	/* Add it in */
        }
    }
    return num;
}


/* Input a number as decimal digits - returns value entered */
long decIn(void)
{
    char input[40];
    int num;
    int tmp;
    register int i;

    i = 0;
    num = 0;

    if (sgets (input)) {  /* grab a line */
        atod(input[i++], &num);      	/* Convert MSD to decimal */
        while(isdec(input[i]) && input[i]) { /* Get next decimal digit */
            num *= 10;                 	/* Make room for next digit */
	    atod(input[i++], &tmp);
            num += tmp; 			/* Add it in */
        }
    }

    return (num);
}


