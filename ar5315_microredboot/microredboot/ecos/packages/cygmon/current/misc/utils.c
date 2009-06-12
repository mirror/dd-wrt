//==========================================================================
//
//      utils.c
//
//      Monitor utilities.
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    
// Contributors: gthomas, dmoseley
// Date:         1999-10-20
// Purpose:      Monitor utilities.
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_BSP
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include <bsp/hex-utils.h>
#endif
#include "monitor.h"
#include "tservice.h"

#if USE_CYGMON_PROTOTYPES
/* Use common prototypes */
/* Some of the composed board.h files compose these
   prototypes redundently, but if they dont,
   these are the common definitions */
#include "fmt_util.h"   /* Interface to string formatting utilities */
#include "generic-stub.h" /* from libstub */
#endif /* USE_CYGMON_PROTOTYPES */

volatile int switch_to_stub_flag = 0;

/* Input routine for the line editor. */
int
input_char (void)
{
  int i;

  /* We have to drop the '+' characters on the floor
     because gdb will send a '+' as the first character
     when connecting to the target. If we waste time
     echoing that, slow hw might get a uart overrun. */
  while ((i = xgetchar ()) == '+');

  if (i == '$')
    {
      xungetchar ('$');
      switch_to_stub_flag = 1;
      i = '\n';
    }
  return i;
}


static char tohex_array[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
			       '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

#define tohex(X) (tohex_array[(X) & 15])

#ifdef HAVE_BSP
#define fromhex __hex
#else
static int
fromhex (a)
{
  int number = -1;

  if (a >= '0' && a <= '9')
    number = a - '0';
  else if (a >= 'a' && a <= 'f')
    number = a - 'a' + 10;
  else if (a >= 'A' && a <= 'F')
    number = a - 'A' + 10;
  else 
    xprintf ("Invalid hex digit %c", a);

  return number;
}
#endif


static unsigned long long
str2ull (char *str, int base)
{
  unsigned long long l = 0;

  if (str[0] == '0' && str[1] == 'x')
    {
      str += 2 ;
      base = 16 ;
    }

  while (*str != '\0')
    {
      if (*str == '.')
	str++;
      else
	l = (l * base) + fromhex(*str++);
    }

  return l;
}


/* Converts a string to a long, base is the assumed base of the string */

target_register_t
str2int (char *str, int base)
{
  return str2ull(str, base);
}

/* Converts a string to a double, input is a raw integer string
 * of the given assumed base. */
#if HAVE_DOUBLE_REGS
static double
str2double (char *str, int base)
{
  double d;

  switch (sizeof(double))
    {
      case sizeof(unsigned int):
	*((unsigned int *)&d) = str2ull(str, base);
	break;
#if __LONG_MAX__ != __INT_MAX__
      case sizeof(unsigned long):
	*((unsigned long *)&d) = str2ull(str, base);
	break;
#endif
#if __LONG_LONG_MAX__ != __LONG_MAX__
      case sizeof(unsigned long long):
	*((unsigned long long *)&d) = str2ull(str, base);
	break;
#endif
      default:
	d = 0.0;
	break;
    }
  return d;
}
#endif

target_register_t
str2intlen (char *str, int base, int len)
{
  target_register_t number = 0;

  while ((len--) > 0 && *str != '\0')
    number = number * base + fromhex (*(str++));

  return number;
}

int
hex2bytes (char *str, char *dest, int maxsize)
{
  int i;
  char *ptr;

  for (i = 0; i < maxsize; i++)
    dest[i] = 0;
  maxsize--;

  // Don't try and convert 0x prefix
  if ((str[0] == '0') && (str[1] == 'x'))
      str += 2;

  ptr = str + strlen(str) - 1;
  while (maxsize >= 0 && ptr >= str)
    {
      dest [maxsize] = fromhex(*ptr);
      ptr--;
      if (ptr >= str)
	{
	  dest [maxsize--] |= fromhex(*ptr) * 16;
	  ptr--;
	}
    }
  return 0;
}


/* Converts an unsigned long long to an ASCII string, adding leading
   zeroes to pad space up to numdigs. */
static int use_dots = 1;

#define MAX_NUM_DIGS 51

static char *
ull2str (unsigned long long number, int base, int numdigs)
{
  static char string[MAX_NUM_DIGS+1];
  int dots, i;
  char *ptr = string + MAX_NUM_DIGS;

  dots = (use_dots && base == 16);

  *(ptr--) = '\0';
  *(ptr--) = tohex (number % base);
  i = 1;
  number = number / base;

  while (number != 0)
    {
      if (dots && (i % 4) == 0)
	*(ptr--) = '.';
      *(ptr--) = tohex (number % base);
      i++;
      number = number / base;
    }

  if (numdigs == 0)
    {
      numdigs = i;
    }
  else
    {
      while(i < numdigs)
	{
	  if (dots && (i % 4) == 0)
	    *(ptr--) = '.';
	  *(ptr--) = '0';
	  i++;
	}
    }
  return ptr + 1;
}


char *
int2str (target_register_t number, int base, int numdigs)
{
  return ull2str((unsigned long long)number, base, numdigs);
}

#if HAVE_DOUBLE_REGS
static char *
double2str(double d)
{
  switch(sizeof(double))
    {
      case sizeof(unsigned int):
	return ull2str(*((unsigned int *)&d), 16, sizeof(double) * 2);
	break;
#if __LONG_MAX__ != __INT_MAX__
      case sizeof(unsigned long):
	return ull2str(*((unsigned long *)&d), 16, sizeof(double) * 2);
	break;
#endif
#if __LONG_LONG_MAX__ != __LONG_MAX__
      case sizeof(unsigned long long):
	return ull2str(*((unsigned long long *)&d), 16, sizeof(double) * 2);
	break;
#endif
    }
  return "....fixme...";
}
#endif

#ifndef NO_MALLOC
char *
strdup(const char *str)
{
  char *x = malloc (strlen (str) + 1);
  if (x != NULL)
    strcpy (x, str);
  return x;
}
#endif


target_register_t
get_pc(void)
{
    return get_register(REG_PC);
}


#if defined(HAVE_BSP) && !defined(__ECOS__)
static int
get_register_type(regnames_t which)
{
    int i;

    for (i = 0; regtab[i].registername != NULL; i++)
      if (regtab[i].registernumber == which)
	return regtab[i].registertype;
    return REGTYPE_INT;
}
#endif

char *get_register_str (regnames_t which, int detail, int valid)
{
#ifdef SPECIAL_REG_OUTPUT
  char *res;

  if ((res = SPECIAL_REG_OUTPUT (which, detail)) != NULL)
    {
      return res;
    }
#endif
  if (valid == 0)
    {
      switch (sizeof (target_register_t))
        {
        case 1:  return "...";
        case 2:  return ".....";
        case 4:  return ".........";
        case 8:  return ".................";
        default: return ".........";
        }
    }
  else
    {
      return int2str (get_register (which), 16, sizeof (target_register_t) * 2);
    }
}


void
store_register (regnames_t which, char *string)
{
#ifdef SPECIAL_REG_STORE
  if (SPECIAL_REG_STORE(which, string))
    return;
#endif
  put_register (which, str2int (string, 16));
}



