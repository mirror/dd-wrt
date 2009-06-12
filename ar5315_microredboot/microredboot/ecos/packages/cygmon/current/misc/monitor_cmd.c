//==========================================================================
//
//      monitor_cmd.c
//
//      Monitor commands for the CygMON ROM monitor
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
// Purpose:      Monitor commands for the CygMON ROM monitor
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================

#ifdef HAVE_BSP
#include "cpu_info.h"
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include <bsp/hex-utils.h>
#ifdef __BOARD_HEADER__
#include __BOARD_HEADER__
#endif
#endif
#include <board.h>
#include <unistd.h>
#include <stdlib.h>
#include <monitor.h>
#include <ledit.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_BSP
#include "fmt_util.h"
#include "tservice.h"
#endif

#ifdef __ECOS__
#include <cyg/hal/hal_stub.h>
#endif

#if USE_CYGMON_PROTOTYPES
/* Use common prototypes */
/* Some of the composed board.h files compose these
   prototypes redundently, but if they dont,
   these are the common definitions */
#include "fmt_util.h"   /* Interface to string formatting utilities */
#include "tservice.h"   /* Interface to target specific services */
#include "generic-stub.h" /* from libstub */
#endif /* USE_CYGMON_PROTOTYPES */

static int history_cmd(cmdmode_t mode) ;

#ifndef MAXLINELEN
#define MAXLINELEN 80
#endif

#define MAXLINES 23

char inbuf[MAXLINELEN] ;
static char cmd[MAXLINELEN];

#if ! defined(PROVIDE_CRASH_CMD)
#define PROVIDE_CRASH_CMD 0
#endif

#if PROVIDE_CRASH_CMD
/*
 * The crash command is used while debugging cygmon itself
 */
static int crash_cmd(cmdmode_t mode) ; /* Command to trap into cygmon */
#endif

struct cmdentry cmdtab[] = {
  {NULL, "baud",        set_serial_speed_cmd},
  {"b",  "break",       breakpoint_cmd},
#if HAVE_CACHE
  {NULL, "cache",       cache_cmd},
#endif
  {NULL, "copy",        copy_cmd},
#if PROVIDE_CRASH_CMD
  {NULL, "crash",       crash_cmd},
#endif
  {NULL, "crc",         checksumcmd},
  {"d",  "disassemble", disassemble_cmd},
  {NULL, "dump",        dump_cmd},
#if defined(NVRAM_ETH_ADDR)
  {NULL, "ethaddr",     ethaddr_cmd},
#endif
  {NULL, "fill",        fill_cmd},
  {NULL, "go",          go_cmd},
  {NULL, "help",        help_cmd},
  {"his","history",     history_cmd},
#ifdef MONITOR_CONTROL_INTERRUPTS
  {NULL, "interrupt",   int_cmd},
#endif
#if defined(NVRAM_IP_ADDR)
  {NULL, "ipaddr",      ipaddr_cmd},
#endif
  {NULL, "load",        load_cmd},
  {"m",  "memory",      mem_cmd},
#ifdef OTHERNAMES_CMD
  {"o",  "othernames",  othernames_cmd},
#endif
  {NULL, "port",        set_serial_port_cmd},
  {"r",  "register",    reg_cmd},
  {NULL, "reset",       reset_cmd},
#ifndef HAVE_BSP
  {NULL, "setargs",     set_program_args_cmd},
#endif
  {"si", "step",        step_cmd},
  {NULL, "swapmem",     swapmem_cmd},
#if defined(NVRAM_TCP_PORT)
  {NULL, "tcpport",     tcpport_cmd},
#endif
  {NULL, "terminal",    set_term_cmd},
#ifdef HAS_TIMER
  {NULL, "timer",       timer_cmd},
#endif
#if !defined(__ECOS__) || defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
  {NULL, "transfer",    transfer_cmd},
#endif
  {"u",  "unbreak",     clear_breakpoint_cmd},
#if HAVE_USAGE
  {NULL, "usage",       memusage_cmd},
#endif
  {NULL, "version",     version_cmd},
  /* Really need to redo the way commands are done--there should be a
     dynamic list (perhaps in addition to the static one). */
#ifdef TARGET_COMMANDS
  TARGET_COMMANDS
#endif

  {NULL, NULL, NULL}
};

static int 
alias_compare (char *cmd)
{
  int m = 0;
  int match = -1;
  int num_matches = 0;

  while (cmdtab[m].cmd != NULL)
    {
      if (cmdtab[m].alias != NULL && !strcmp (cmd, cmdtab[m].alias))
	{
	  match = m;
	  num_matches++;
	  /* We're expecting that aliases will be defined 
	     uniquely, but just in case, we let the user know 
	     if there is a conflict */
	  if (num_matches > 1)
	    {
	      xprintf ("Alias conflict. Executing last matching alias.\n");
	    }
	}
      m++;
    }
  return match;
}

static int
command_compare (char *cmd)
{
  int m = 0;
  int match = -1;
  int num_matches = 0;
  int cmdlen = strlen(cmd) ;
  while (cmdtab[m].cmd != NULL)
    {
      if (!(strncmp (cmd,cmdtab[m].cmd, cmdlen)))
	{
	  /* we found a match */
	  num_matches++;
	  if (num_matches == 2)  /* we found a second match */
	    {
	      xprintf ("Ambiguous command.  Possibilities are:\n%s\n%s\n",
		       cmdtab[match].cmd,
		       cmdtab[m].cmd);
	    }
	  else if (num_matches > 2)  /* we found another match */
	    {
	      /* Show the new possibility we just found */
	      xprintf ("%s\n", cmdtab[m].cmd);
	    }
          /* Point the match at the command we just looked at.
	     We have to wait until now so that the first duplicate
	     found can output the earlier match as well */
	  match = m;
	}
      m++;
    }
  return ((num_matches == 1) ? match : -1);
}

#ifdef USE_HELP
void
usage (char *string)
{
  xprintf ("Usage: %s\n", string);
}

void
short_help (char *string)
{
  xprintf ("%s\n", string);
}

void
long_help (char *string)
{
  int linecnt = 0;
  int do_leave = 0;

  for (; *string && !do_leave; string++)
    {
      xprintf ("%c", *string);
      if (*string == '\n')
	{
	  linecnt++;
	  if (linecnt == MAXLINES)
	    {
	      int i;

	      xprintf ("-More-");
	      while ((i = input_char ()) >= 0)
		{
		  if (i == '\r' || i == '\n')
		    {
		      linecnt--;
		      break;
		    }
		  else if (i == ' ')
		    {
		      linecnt = 0;
		      break;
		    }
		  else if (i == 'q' || i == 'Q')
		    {
		      do_leave = 1;
		      break;
		    }
		  else
		    beep ();
		}
	    }
	}
    }
  xprintf ("\n");
}

void
example (char *example)
{
  xprintf ("Example: %s\n", example);
}

#else
void
no_help (void)
{
  xprintf ("No help available.\n");
}

void
no_help_usage (void)
{
  xprintf ("Incorrect usage.\n");
}
#endif

int 
help_cmd (cmdmode_t mode) 
{
  int command_number = -2;

  if (mode == USAGE)
    {
      usage ("help [command]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("The help command");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      help_cmd (USAGE);
      long_help ("\
The help command without arguments shows a list of all available commands\n\
with a short description of each one.  With a command name as an argument\n\
it shows usage for the command and a paragraph describing the command.\n\
Usage is shown as command name followed by names of extensions or arguments.\n\
Arguments in [brackets] are optional, plain text arguments are required.\n\
Note that all commands can be invoked by typing enough of the command name\n\
to uniquely specify the command.  Some commands have aliases, which are one\n\
letter abbreviations for commands which do not have unique first letters.\n\
Aliases for all commands are shown in the help screen, which displays\n\
commands in the format:\n\
command name: (alias, if any)  description of command \n");
      example("\
help foo \n\
Shows the help screen for the command foo.");
      return 0;
    }

  if (argvect[1] != NULL)
    {
      if (argvect[2] == NULL)
	{
	  command_number = command_compare (argvect[1]);
	  if (command_number < 0)
	    {
	      xprintf ("No such command as %s\n", argvect[1]);
	    }
	}
      
      if (command_number < 0)
	{
	  return help_cmd (USAGE);
	}
      else
	{
	  return cmdtab[command_number].function (LONG_HELP);
	}
    }
  else
    {
      int i;
      
      xprintf ("Available commands are:\n");
      for (i = 0; cmdtab[i].cmd != NULL; i++)
	{
	  int x = strlen (cmdtab[i].cmd) + 2;

	  xprintf ("%s: ", cmdtab[i].cmd);
	  if (cmdtab[i].alias != NULL)
	    {
	      xprintf("(%s)", cmdtab[i].alias);
	      x += 2 + strlen(cmdtab[i].alias);
	    }
	  for (; x < 20; x++)
	    {
	      xprintf (" ");
	    }
	  cmdtab[i].function (SHORT_HELP);
	  if ((i > 0) && (i % MAXLINES) == 0)
	    {
	      xprintf ("-More-");
	      input_char ();
	      xprintf ("\n");
	    }
	}
    }
  return 0;
}

#if PROVIDE_CRASH_CMD
static int crash_cmd(cmdmode_t mode)
{
  switch (mode)
    {
    case USAGE : usage("crash")  ; break ;
    case SHORT_HELP : short_help("invoke the breakpoint function");
      break ;
    case LONG_HELP :
      long_help("The crash command calls the breakpoint function() which is useful\n\
only if you are using an additional debugger to debug this software and,\n\
the general exception handler is hooked to the other debugger\n") ;
      break ;
    case INVOCATION :
      dbg_breakpoint() ;
      break ;
    }
  return 0 ;
}
#endif /* provide_crash_command */



static int history_cmd(cmdmode_t mode)
{
    switch (mode)
    {
    case USAGE : usage("history")  ; break ;
    case SHORT_HELP : short_help("Print help about line editing features.");
      break ;
    case INVOCATION :
      printHistoryList();
      break;
    case LONG_HELP :
      long_help("Cygmon line editing allows you to repeat previously entered 
commands. The line editing commands are:
        CR '\\n'  Execute the currently displayed command
        ctl-a    move to Beginning of line
        ctl-e    End of line
        ctl-f    Forward char
        ctl-b    Backward char
        ctl-k    Delete to end of line
        ctl-y    yank Character (undelete)
        ctl-d    Delete current character
        ctl-p    edit previous command
        ctl-u    Erase Line
        ctl-n    Edit next command") ;
      break ;
    }
  return 0 ;
} /* history_cmd */

#ifdef OTHERNAMES_CMD
/* Switch to using the othernames (ie alternate register names) */
int
othernames_cmd (cmdmode_t mode)
{
  if (mode == USAGE)
    {
      usage ("othernames");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Switch between alternate register names.");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      othernames_cmd (USAGE);
      long_help ("\
The othernames command allows you to switch between alternate register
names for a given target.");
      example ("\
othernames\n\
Switches to the alternate register name set..");
      return 0;
    }
  OTHERNAMES_CMD();

  return 0;
}
#endif /* OTHERNAMES_CMD */


#if defined(NVRAM_ETH_ADDR)
int
ethaddr_cmd (cmdmode_t mode)
{
  int      i, n;
  unsigned char addr[6];
  char     *p;

  if (mode == USAGE)
    {
      usage ("ethaddr [xx:xx:xx:xx:xx:xx]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("get/set NVRAM backed ethernet address");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      ethaddr_cmd (USAGE);
      long_help ("\
The ethaddr command is used to view and modify the non-volatile ethernet\n\
address. The address is specified by 6 colon-seperated 2 digit hexadecimal\n\
numbers. If no address is specified, the current address is displayed.\n");
      example ("ethaddr 00:00:8B:F1:36:01");
      return 0;
    }

  if (argvect[1] != NULL)
    {
      if (strlen(argvect[1]) != 17 || argvect[2] != NULL)
	{
	  return ethaddr_cmd (USAGE);
	}

      for (i = 0, p = argvect[1]; i < 6; i++, p++)
	{
	  n = __hex(*p++);
	  if (n < 0)
	    return ethaddr_cmd (USAGE);
	  addr[i] = (n << 4);
	  n = __hex(*p++);
	  if (n < 0)
	    return ethaddr_cmd (USAGE);
	  addr[i] |= n;

	  if (*p != ':' && !(i == 5 && *p == '\0'))
	    return ethaddr_cmd (USAGE);
	}
      for (i = 0; i < 6; i++)
	NVRAM_ETH_ADDR(i) = addr[i];
    }
  else
    {
      for (i = 0; i < 5; i++)
	xprintf("%02x:", NVRAM_ETH_ADDR(i));
      xprintf("%02x\n", NVRAM_ETH_ADDR(i));
    }

  return 0;
}
#endif /* NVRAM_ETH_ADDR */


#if defined(NVRAM_IP_ADDR)
int
ipaddr_cmd (cmdmode_t mode)
{
  int      i, j, n;
  unsigned char addr[4];
  char     *p;

  if (mode == USAGE)
    {
      usage ("ipaddr [n.n.n.n]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("get/set NVRAM backed ip address");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      ipaddr_cmd (USAGE);
      long_help ("\
The ipaddr command is used to view and modify the non-volatile internet\n\
address. The address is specified by 4 dot-seperated 1-3 digit decimal\n\
numbers. If no address is specified, the current address is displayed.\n");
      example ("ipaddr 192.161.0.1");
      return 0;
    }

  if (argvect[1] != NULL)
    {
      if (argvect[2] != NULL)
	return ipaddr_cmd (USAGE);

      p = argvect[1];

      for (i = 0; i < 3; i++, p++)
	{
	  for (j = n = 0; j < 3 && isdigit(*p); j++, p++)
	    n = n*10 + (*p - '0');
	  if (j == 0 || *p != '.' || n > 255)
	    return ipaddr_cmd (USAGE);
	  addr[i] = n;
	}
      for (j = n = 0; j < 3 && isdigit(*p); j++, p++)
	n = n*10 + (*p - '0');
      if (j == 0 || *p != '\0' || n > 255)
	return ipaddr_cmd (USAGE);
      addr[i] = n;

      for (i = 0; i < 4; i++)
	NVRAM_IP_ADDR(i) = addr[i];
    }
  else
    {
      for (i = 0; i < 3; i++)
	xprintf("%d.", NVRAM_IP_ADDR(i));
      xprintf("%d\n", NVRAM_IP_ADDR(i));
    }

  return 0;
}
#endif /* NVRAM_IP_ADDR */

#if defined(NVRAM_TCP_PORT)
int
tcpport_cmd (cmdmode_t mode)
{
  int  n;
  char *p;

  if (mode == USAGE)
    {
      usage ("ethaddr [xx:xx:xx:xx:xx:xx]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("get/set NVRAM backed tcp port number");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      tcpport_cmd (USAGE);
      long_help ("\
The tcpport command is used to view and modify the non-volatile tcp port\n\
address used for debugging. The address is specified by decimal numer in\n\
the range of 1-65535. If no port number is specified, the current port is\n\
displayed.\n");
      example ("tcpport 1000");
      return 0;
    }

  if (argvect[1] != NULL)
    {
      if (argvect[2] != NULL)
	return tcpport_cmd (USAGE);

      p = argvect[1];
      n = 0;
      while (isdigit(*p))
	{
	  n = n*10 + (*p++ - '0');
	  if (n > 65535)
	    return tcpport_cmd (USAGE);
	}
      if (*p != '\0')
	return tcpport_cmd (USAGE);

      NVRAM_TCP_PORT(0) = (n >> 8) & 0xff;
      NVRAM_TCP_PORT(1) = n & 0xff;
    }
  else
    xprintf("%d\n", (NVRAM_TCP_PORT(0) << 8) + NVRAM_TCP_PORT(1));

  return 0;
}
#endif /* NVRAM_TCP_PORT */


#ifdef __ECOS__
#  if (CYG_BYTEORDER == CYG_LSBFIRST)
#    define LITTLE_ENDIAN_TARGET
#  else
#    define BIG_ENDIAN_TARGET
#  endif
#endif

#ifdef LITTLE_ENDIAN_TARGET
static int swap_bytes = 1;
#else
static int swap_bytes = 0;
#endif

int
get_memory_display_mode (void)
{
  return swap_bytes;
}

void
set_memory_display_mode (int mode)
{
  swap_bytes = mode;
}


/* Just to make DEFAULT_SIZE something usable, this may go elsewhere later.*/
#ifndef DEFAULT_SIZE
#define DEFAULT_SIZE 1
#endif

static int
get_cmd_size (void)
{
  int size = 0;
  char *sizestr;

  sizestr = strchr (argvect[0], '.');

  if (sizestr == NULL || sizestr[0] == '\0' || sizestr[1] == '\0')
    {
      size = DEFAULT_SIZE;
    }
  else
    {
      size = str2int (sizestr + 1, 10) / 8;
    }
  if (size != 1 && size != 2 && size != 4 && size != 8)
    {
      xprintf ("Invalid size.\n");
      return -1;
    }
  return size;
}


void
display_memory (char *value, int size, int littleEndian)
{
  int x;
  int start = littleEndian ? size - 1 : 0 ;
  int end = littleEndian ? -1 : size;
  int incr = littleEndian ? -1 : 1;

  if (value)
    {
      for (x = start; x != end; x += incr)
        xprintf ("%02x", value[x] & 0xff);
    }
  else
    {
      for (x = start; x != end; x += incr)
        xprintf ("..");
    }
}


static int
peek (void)
{
  mem_addr_t addr;
  int size = get_cmd_size ();

  if (size > 0)
    {
      /* We already checked to see if the command was legal when we called the
	 function, so there's no need to worry about that here. */

      if (argvect[1] != 0)
	{
	  char value[8];

	  str2addr (argvect[1], &addr);
	  if (read_memory (&addr, size, 1, value))
	    {
	      xprintf ("Memory read failed\n");
	    }
	  else
	    {
	      display_memory (value, size, get_memory_display_mode ());
	      xprintf ("\n");
	    }
	}
      else
	{
	  xprintf ("Not enough arguments\n");
	  return mem_cmd (USAGE);
	}
    }
  return 0;
}

/* Poke a single byte in memory. */

static int 
poke (void)
{
  int size = 0;

  size = get_cmd_size ();
  if (size > 0)
    {
      /* We already checked to see if the command was legal when we called the
	 function, so there's no need to worry about that here. */

      if ((argvect[1] != 0) && (argvect[2] != 0))
	{
	  char value[8];
	  mem_addr_t addr;

	  str2addr (argvect[1], &addr);
	  hex2bytes (argvect[2], value, size);

	  if (get_memory_display_mode ())
	    {
	      /* Gotta swap this puppy. */
	      int x;

	      for (x = 0; x < (size / 2); x++)
		{
		  char tmp = value[x];
		  value [x] = value [size - 1 - x];
		  value [size - 1 - x] = tmp;
		}
	    }
	  if (write_memory (&addr, size, 1, value))
	    {
	      xprintf ("Memory write failed\n");
	    }
#ifdef HAVE_BSP
	  bsp_flush_dcache((void *)addr.addr, size);
	  bsp_flush_icache((void *)addr.addr, size);
#endif
	}
      else
	{
	  xprintf ("Not enough arguments\n");
	  return mem_cmd (USAGE);
	}
    }
  return 0;
}


/* I am cheap and easy. */
int
mem_cmd (cmdmode_t mode)
{
  if (mode == USAGE)
    {
      usage ("memory[.size] address [value]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("read/write memory");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      mem_cmd (USAGE);
      long_help ("\
The memory command is used to view and modify single locations in memory.\n\
It can take a size extension, which follows the command name or partial\n\
command name without a space, and is a period followed by the number of\n\
bits to be viewed or modified.  Options are 8, 16, 32, and 64.  Without\n\
a size extension, the memory command defaults to displaying or changing 8\n\
bits at a time.\n\
The memory command can take one or two arguments, independent of\n\
whether a size extension is specified.  With one argument, it displays the\n\
contents of the specified location.  With two arguments, it replaces the\n\
contents of the specified location with the specified value.\n");
      example ("\
memory.8 45f6b2 57\n\
Places the 8 bit value 57 at the location 45f6b2.");
      return 0;
    }

  if (argvect[1] == NULL || (argvect[2] != NULL && argvect[3] != NULL))
    {
      return mem_cmd (USAGE);
    }
  if (argvect[1] != NULL && argvect[2] != NULL)
    return poke ();
  else
    return peek ();
}

/* Memory dump function. */

int 
dump_cmd (cmdmode_t mode)
{
  mem_addr_t addr;
  char value[8];

  if (mode == USAGE)
    {
      usage ("dump[.size] location [count]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Memory dump command");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      dump_cmd (USAGE);
      long_help ("\
The dump command displays a region of memory. If no count value is\n\
supplied, the command displays 16 bytes.  It can take a size\n\
extension, which follows the command name or partial command name\n\
without a space, and is a period followed by the number of bits to be\n\
viewed or modified.  Options are 8, 16, 32, and 64.  Without a size\n\
extension, the dump command defaults to displaying 8 bits at a time.\n\
Addresses are aligned to a 16-byte boundary.  Thus, dump 65 would show\n\
all bytes from 60 through 6f.\n");
      example ("\
dump 44f5\n\
Displays 16 bytes starting with 44f0 and ending with 44ff.");
      return 0;
    }

  if (argvect[1] != 0 && (argvect[2] == NULL || argvect[3] == NULL))
    {
      mem_addr_t start_addr;
      char chardumps[32];
      int offset;
      int count;
      int line;
      int size = get_cmd_size ();
      int i;

      if (size == -1)
          /*
           * Invalid size specified.
           */
          return 0;

      str2addr (argvect[1], &addr);
      if (argvect[2] != NULL)
	{
	  count = str2int (argvect[2], 10);
	  count = (count + 15) / 16;
	}
      else
	{
	  count = 1;
	}
      ADD_ALIGN(&addr, &start_addr, 16);
      for (line = 0; line < count; line ++)
	{
	  for (offset = 0; offset < 16; offset += size)
	    {
	      ADD_OFFSET(&start_addr, &addr, offset + line * 16);

	      if (offset == 0)
		{
		  char buf[32];
	      
		  addr2str (&addr, buf);
                  xprintf("%s: ", buf);
		}
              if (read_memory (&addr, size, 1, value))
                {
                  display_memory (0, size, get_memory_display_mode ());
                  for (i = 0; i < size; i++)
                    {
                      value[i] = 0;
                    }
                } else {
                  display_memory (value, size, get_memory_display_mode ());
                }
              xprintf (" ");
              for (i = 0; i < size; i++)
                {
                  chardumps[offset + i] = value[i] & 0x7f;
                  if (chardumps[offset + i] < 32)
                    chardumps[offset + i] = '.';
                }
	    }
	  xprintf (" ");
	  for(i = 0; i < offset; i++)
	    {
	      xprintf ("%c", chardumps[i]);
	    }
          xprintf ("\n");
	}
    }
  else
    {
      dump_cmd (USAGE);
    }
  return 0;
}

static int scnt;
static int (*srec_input_char) (void);

static inline int
gethexnibble(void)
{
  int ch;

  inbuf[scnt++] = ch = srec_input_char();
  if (ch >= '0' && ch <= '9')
    return (ch - '0');
  if (ch >= 'a' && ch <= 'f')
    return (ch - 'a' + 10);
  if (ch >= 'A' && ch <= 'F')
    return (ch - 'A' + 10);

  inbuf[scnt] = '\0';
  xprintf("Bad hex char: %s\n", inbuf);
  return -1;
}


static inline int
gethexbyte(void)
{
  int nib;
  unsigned char n;

  if ((nib = gethexnibble()) < 0)
    return -1;
  n = nib << 4;
  if ((nib = gethexnibble()) < 0)
    return -1;
  n |= nib;
  return n;
}

static inline int
chk_cksum(unsigned int cksum, int rval)
{
  int n;

  if ((n = gethexbyte()) < 0)
    return -1;

  cksum = ~cksum & 0xff;

  if (cksum != n)
    {
      inbuf[scnt] = '\0';
      xprintf("Bad cksum[%02x]: %s\n", cksum, inbuf);
      return -1;
    }
  return rval;
}

int
load_srec(srec_input_func_t inp_func)
{
  int count, dcount, data, n, addr_bytes = 0, is_term, is_comment;
  unsigned int address, cksum;
  unsigned char data_buf[256];
  mem_addr_t memaddr;

  srec_input_char = inp_func;

  is_comment = is_term = 0;

  while (srec_input_char() != 'S')
    ;

  scnt = 0;
  inbuf[scnt++] = 'S';

  if ((n = gethexnibble()) < 0)
    return -1;

  switch (n)
    {
      case 0:
      case 5:
	is_comment = 1;
	break;

      case 1:
      case 2:
      case 3:
	addr_bytes = n + 1;
	break;

      case 7:
      case 8:
      case 9:
	is_term = 1;
	addr_bytes = 11 - n;
	break;

      default:
	inbuf[scnt] = '\0';
	xprintf("Bad record type: %s\n", inbuf);
	return -1;
    }

  if ((count = gethexbyte()) < 0)
    return -1;
  cksum = count;

  --count; /* don't count chksum */

  if (is_comment)
    {
      while (count > 0)
	{
	  if ((n = gethexbyte()) < 0)
	    return -1;
	  cksum += n;
	  --count;
	}
      return chk_cksum(cksum, 0);
    }

  address = 0;
  while (count > 0 && addr_bytes)
    {
      if ((n = gethexbyte()) < 0)
	return -1;
      cksum += n;
      address = (address << 8) | n;
      --addr_bytes;
      --count;
    }

  if (is_term)
    {
      if (count || addr_bytes)
	{
	  inbuf[scnt] = '\0';
	  xprintf("Malformed record cnt[%d] abytes[%d]: %s\n",
		 count, addr_bytes, inbuf);
	  return -1;
	}
      if (chk_cksum(cksum, 1) == 1)
	{
	  set_pc (address);
	  xprintf("Setting start address: 0x%08x\n", address);
	  return 1;
	}
      return -1;
    }

  dcount = 0;

  while (count > 0)
    {
      if ((data = gethexbyte()) < 0)
	return -1;
      cksum += data;
      data_buf[dcount++] = data;
      --count;
    }
    
  if (chk_cksum(cksum, 0))
    return -1;

  MAKE_STD_ADDR (address, &memaddr);
  write_memory (&memaddr, 1, dcount, data_buf);
#ifdef HAVE_BSP
  bsp_flush_dcache((void *)memaddr.addr, dcount);
  bsp_flush_icache((void *)memaddr.addr, dcount);
#endif

  return 0;
}


int 
load_cmd (cmdmode_t mode)
{
  if (mode == USAGE)
    {
      usage ("load");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Load srecords into memory");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      load_cmd (USAGE);
      long_help ("\
The load command switches the monitor into a state where it takes all input 
as s-records and stores them in memory.  The monitor exits this mode when a
termination record is hit, or certain errors (such as an invalid s-record)
cause the load to fail.");
      return 0;
    }
  
  if (argvect[1] != NULL)
    {
      return load_cmd (USAGE);
    }

  while (!load_srec(xgetchar))
      ;

  return 0;
}

#ifndef REGNAME_EXAMPLE
# define REGNAME_EXAMPLE "a0"
#endif

#if !defined(__ECOS__) || defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
int
transfer_cmd (cmdmode_t mode)
{
  if (mode == USAGE)
    {
      usage ("$");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Transfer to gdb stub");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      transfer_cmd (USAGE);
      long_help ("\
The transfer or $ command transfers control to the gdb stub.  This function\n\
does not actually need to be called by the user, as connecting to the board\n\
with gdb will call it automatically.  The transfer command takes no\n\
arguments.  The $ command does not wait for a return, but executes\n\
immediately.  A telnet setup in line mode will require a return when $ is\n\
executed by the user, as the host computer does not pass any characters to\n\
the monitor until a return is pressed.  Disconnecting from the board in gdb\n\
automatically returns control to the monitor.");
      return 0;
    }
  if (argvect[1] == NULL)
    {
      return transfer_to_stub ();
    }
  else
    {
      transfer_cmd (USAGE);
    }
  return 0;
}
#endif

static void
display_group (int which_group)
{
  int len = 0;
  int skipping_group = 0;
  char buf[80];
  int start_entry = 0;
  int i;

  if (which_group >= 0)
    {
      start_entry = which_group;
    }

  for (i = start_entry; regtab[i].registername != NULL; i++)
    {
      int buflen;

      if (regtab[i].registernumber < 0)
	{
	  if (which_group >= 0 && i != which_group)
	    {
	      break;
	    }

	  if (len > 0)
	    {
	      xprintf ("\n");
	      len = 0;
	    }
	  if (which_group < 0)
	    {
	      if (regtab[i].registernumber == -2)
		{
		  skipping_group = 1;
		  xprintf ("[skipping %s]\n", regtab[i].registername);
		}
	      else
		{
		  skipping_group = 0;
		  xprintf ("[%s]\n", regtab[i].registername);
#if 0
		  len = strlen (regtab[i].registername) + 2;
		  while (len < MAXLINES)
		    {
		      xputchar (' ');
		      len++;
		    }
#endif
		}
	    }
	}
      else 
	if (!skipping_group)
	  {
#ifdef REG_VALID_FIELD_IN_REGSTRUCT
#define REGVALID_VAL regtab[i].registervalid
#else
#define REGVALID_VAL 1
#endif
	    xsprintf(buf, "%4s: %s",
		     regtab[i].registername,
		     get_register_str (regtab[i].registernumber, 0 , REGVALID_VAL));
#undef REGVALID_VAL
	    buflen = strlen (buf);
	    if ((buflen + len + 3) >= 80)
	      {
		xprintf ("\n");
		len = 0;
	      }
	    else if(len > 0)
	      {
		xprintf ("   ");
		len += 3;
	      }

	    xprintf (buf);
	    len += buflen;
	  }
    }
  if (len > 0)
    {
      xprintf ("\n");
    }
}

static int
getregister (void)
{
  int i;
  
  if (argvect[1] == 0)
    {
      display_group (-1);
    }
  else
    {
      for (i = 0; regtab[i].registername != NULL; i++)
	{
	  if (!(strcmp (argvect[1], regtab[i].registername)))
	    {
	      break;
	    }
	}
      if (regtab[i].registername == NULL)
	{
	  xprintf ("No such register\n");
	}
      else
	{
	  if (regtab[i].registernumber < 0)
	    {
	      display_group (i);
	    }
	  else
	    {
#ifdef REG_VALID_FIELD_IN_REGSTRUCT
#define REGVALID_VAL regtab[i].registervalid
#else
#define REGVALID_VAL 1
#endif
	      xprintf("%s: %s\n", argvect[1], 
		      get_register_str (regtab[i].registernumber, 1, REGVALID_VAL));
#undef REGVALID_VAL
	    }
	}
    }
  return 0;
}

static int 
setregister (void)
{
  int number = -1;
  int i;
  if ((argvect[1] != 0) && (argvect[2] != 0))
    {
      i = 0;
      while (regtab[i].registername != NULL)
	{
	  if (!(strcmp (argvect[1], regtab[i].registername)))
	    {
	      number = regtab[i].registernumber;
	      break;
	    }
	  i++;
	}
      if (number < 0)
	xprintf ("Unknown register name %s\n", argvect[1]);
      else
	store_register (number, argvect[2]);
    }
  else
    {
      xprintf ("Not enough arguments\n");
    }
  return 0;
}


int
reg_cmd (cmdmode_t mode)
{
  if (mode == USAGE)
    {
      usage ("register [register name] [value]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("View and manipulate registers");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      reg_cmd (USAGE);
      long_help ("\
The register command allows the viewing and manipulation of register\n\
contents.  It can take zero, one, or two arguments.  When called with zero\n\
arguments, the register command displays the values of all registers.  When\n\
called with only the register name argument, it displays the contents of\n\
the specified register.  When called with both a register name and a value,\n\
it places that value into the specified register.\n");
      example ("\
register " REGNAME_EXAMPLE " 1f\n\
Places the value 1f in the register " REGNAME_EXAMPLE);
      return 0;
    }

  if (argvect[1] != NULL && argvect[2] != NULL)
    {
      if (argvect[3] != NULL)
	{
	  return reg_cmd (USAGE);
	}
      return setregister ();
    }
  else
    return getregister ();
}


/* execute the program in memory */
int 
go_cmd (cmdmode_t mode)
{
#ifdef HAS_TIMER
  extern tick_type start_tickcount;
#endif

  if (mode == USAGE)
    {
      usage ("go [location]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Start user program execution");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      go_cmd (USAGE);
      long_help ("\
The go command starts user program execution.  It can take zero or one\n\
argument.  If no argument is provided, go starts execution at the current\n\
pc.  If an argument is specified, go sets the pc to that location, and then\n\
starts execution at that location.\n");
      example ("
go 40020000\n\
Sets the pc to 40020000, and starts program execution.");
      return 0;
    }

  if (argvect[1] != NULL && argvect[2] != NULL)
    {
      return go_cmd (USAGE);
    }
  if (argvect[1] != NULL)
    {
      set_pc (str2int (argvect[1], 16));
    }

#ifdef HAS_TIMER
  start_tickcount = __read_ticks ();
#endif
  /* We want monitor_loop to exit now. */
  return 1;
}

#ifdef HAS_TIMER
int
timer_cmd (cmdmode_t mode)
{
  if (mode == USAGE)
    {
      usage ("timer [state]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Enable and disable timer");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      timer_cmd (USAGE);
      long_help ("\
The timer command allows control of the on board timer.  It takes zero\n\
or one argument.  With zero arguments, it displays the state of the timer,\n\
with one argument, which can start with  e or d, it enables or disables the\n\
timer, respectively.\n");
      example ("\
timer e\n\
Enables the timer.");
      return 0;
    }

  if (argvect[1] != NULL)
    {
      if (argvect[2] != NULL)
	{
	  return timer_cmd (USAGE);
	}
      else if (argvect[1][0] == 'e')
	{
	  __settimer (0, 0);
	}
      else if (argvect[1][0] == 'd')
	{
	  __disable_timer ();
	}
      else
	{
	  timer_cmd (USAGE);
	}
    }
  if (__timer_enabled ())
    {
      xprintf ("Timer is currently enabled.\n");
    }
  else
    {
      xprintf ("Timer is currently disabled.\n");
    }
  return 0;
}
#endif

int 
clear_breakpoint_cmd (cmdmode_t mode)
{
  if (mode == USAGE)
    {
      usage ("unbreak location");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Clear breakpoint");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      clear_breakpoint_cmd (USAGE);
      long_help ("\
The unbreak command removes breakpoints from memory.  It takes one\n\
argument, the location to remove the breakpoint from.\n");
      example ("\
unbreak 4ff5\n\
Removes a previously set breakpoint at memory location 4ff5.");
      return 0;
    }
  
  if (argvect[1] == NULL || argvect[2] != NULL)
    {
      clear_breakpoint_cmd (USAGE);
    }
  else
    {
      mem_addr_t location;
      str2addr (argvect[1], &location);
      if (clear_mon_breakpoint (location))
      {
          xprintf("Unable to remove breakpoint at 0x%08lx\n", location.addr);
      }
    }
  return 0;
}


int
breakpoint_cmd (cmdmode_t mode)
{
  if (mode == USAGE)
    {
      usage ("break [location]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Set or display breakpoints");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      breakpoint_cmd (USAGE);
      long_help ("\
The break command displays and sets breakpoints in memory.  It takes zero\n\
or one argument.  With zero arguments, it displays a list of all currently\n\
set breakpoints.  With one argument it sets a new breakpoint at the\n\
specified location.\n");
      example ("\
break 4ff5\n\
Sets a breakpoint at address 4ff5.");
      return 0;
    }
  
  if (argvect[1] == NULL)
    {
      return show_breakpoints ();
    } 
  else if (argvect[2] != NULL)
    {
      breakpoint_cmd (USAGE);
    }
  else
    {
      mem_addr_t location;
      
      str2addr (argvect[1], &location);
      if (add_mon_breakpoint (location))
      {
          xprintf("Unable to set breakpoint at 0x%08lx\n", location.addr);
      }
    }
  return 0;
}


int
version_cmd (cmdmode_t mode)
{
  if (mode == USAGE)
    {
      usage ("version");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Display version");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      version_cmd (USAGE);
      long_help ("\
The version command displays the version of the monitor.");
      return 0;
    }
  
  if (argvect[1] == NULL)
    {
      version ();
    }
  else
    {
      version_cmd (USAGE);
    }
  return 0;
}

int
set_serial_port_cmd (cmdmode_t mode)
{
  if (mode == USAGE)
    {
      usage ("port [port number]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Set the active serial port");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      set_serial_port_cmd (USAGE);
      long_help ("\
The port command allows control over the serial port being used by the\n\
monitor.  It takes zero or one argument.  Called with zero arguments it\n\
displays the port currently in use by the monitor.  Called with one\n\
argument it switches the port in use by the monitor to the one specified.\n\
It then prints out a message on the new port to confirm the switch.\n");
      example ("\
port 1\n\
Switches the port in use by the monitor to port 1.");
      return 0;
    }
  if (argvect[1] != NULL && argvect[2] != NULL)
    {
      set_serial_port_cmd (USAGE);
      return 0;
    }
  if (argvect [1] != NULL)
    {
#ifdef HAVE_BSP
      if (bsp_set_debug_comm(str2int (argvect[1], 10)) < 0)
	xprintf("Invalid port number.\n");
      else
        /* Since we are using the new port, we just need to write
	   something to tell the user that this is the active port */
        xprintf ("Cygmon I/O now on this port.\n");
#else
      __setTty (str2int (argvect[1], 10));
      /* Since we are using the new port, we just need to write
	 something to tell the user that this is the active port */
      xprintf ("Cygmon I/O now on this port.\n");
#endif
    }
  else 
    {
      xprintf ("serial port currently set to %d\n", __getTty());
      return 0;
    }
  return 0;
}

int 
step_cmd (cmdmode_t mode)
{
  if (mode == USAGE)
    {
      usage ("step [location]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Single step user program");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      step_cmd (USAGE);
      long_help ("\
The step command causes one instruction of the user program to execute, then\n\
returns control to the monitor.  It can take zero or one argument.  If no\n\
argument is provided, step executes one instruction at the current pc.  If\n\
a location is specified, step executes one instruction at the specified\n\
location.\n");
      example ("
step\n\
Executes one instruction at the current pc.");
      return 0;
    }
  
  if (argvect[1] != NULL && argvect[2] != NULL)
    {
      step_cmd (USAGE);
      return 0;
    }
  if (argvect[1] != NULL)
    set_pc (str2int (argvect[1], 16));
  
  __single_step ();
  return 1;

}

#if HAVE_CACHE
/* This cache function needs to be in processor-specific files. */

int
cache_cmd (cmdmode_t mode)
{
  int x;
  int cache_op = CACHE_NOOP;
  int which_cache = -1;

  if (mode == USAGE)
    {
      usage ("cache [type] [state]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Manipulate caches");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      cache_cmd (USAGE);
      long_help ("\
The cache command displays and changes the states of the caches.  It takes\n\
zero, one, or two arguments.  With no arguments, it displays the state of\n\
both the instruction cache and the data cache.  With one argument, it\n\
displays the state of the specified type of cache.  With two arguments, it\n\
changes the state of the specified cache to the specified state.\n");
      example ("\
cache i d\n\
Disables the instruction cache.");
      return 0;
    }

  if (argvect[1] != NULL && argvect[2] != NULL && argvect[3] != NULL)
    {
      return cache_cmd (USAGE);
    }
  if (argvect[1] != NULL)
    {
      if (argvect[1][0] == 'd')
	which_cache = 0;
      else if (argvect[1][0] == 'i')
	which_cache = 1;
      else
	{
	  xprintf ("unknown cache type %s\n", argvect[1]);
	  return 0;
	}
      if (argvect[2] != NULL)
	{
	  if (argvect[2][0] == 'e')
	    cache_op = CACHE_ENABLE;
	  else if (argvect[2][0] == 'd')
	    cache_op = CACHE_DISABLE;
	  else if (argvect[2][0] == 'f')
	    cache_op = CACHE_FLUSH;
	  else
	    {
	      xprintf ("Unknown cache op %s\n", argvect[2]);
	      return 0;
	    }
	}
    }
  if (which_cache == 0 || which_cache == -1)
    {
      __data_cache (cache_op);
      if (cache_op == CACHE_FLUSH)
      {
	xprintf ("Flushed dcache\n");
      }
      x = __data_cache (CACHE_NOOP);
      xprintf ("dcache is ");
      xprintf (x ? "enabled\n" : "disabled\n");
    }
  if (which_cache == 1 || which_cache == -1)
    {
      __instruction_cache (cache_op);
      if (cache_op == CACHE_FLUSH)
      {
	xprintf ("Flushed icache\n");
      }
      x = __instruction_cache (CACHE_NOOP);
      xprintf ("icache is ");
      xprintf (x ? "enabled\n" : "disabled\n");
    }
  return 0;
}
#endif

int
set_serial_speed_cmd (cmdmode_t mode)
{
    if (mode == USAGE)
    {
      usage ("baud speed");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Set serial port baud rate");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      set_serial_speed_cmd (USAGE);
      long_help ("\
The baud command sets the speed of the active serial port.  It takes one\n\
argument, which specifies the speed to which the port will be set.\n");
      example ("\
baud 9600\n\
Sets the speed of the active port to 9600 baud.");
      return 0;
    }

  if (argvect[1] == NULL || argvect[2] != NULL)
    {
      return set_serial_speed_cmd (USAGE);
    }
  else
    {
#ifdef HAVE_BSP
      int channel_id = bsp_set_debug_comm(-1);
      int baud = str2int (argvect[1], 10);

      xprintf("Setting serial baud rate on channel %d to %d baud\n", 
	     channel_id, baud);

      if (bsp_set_serial_baud(bsp_set_debug_comm(-1),
			      str2int (argvect[1], 10)) < 0)
	xprintf("Invalid baud rate\n");
#else
      __set_baud_rate (str2int (argvect[1], 10));
#endif
    }
  return 0;
}


int
set_term_cmd (cmdmode_t mode)
{
  if (mode == USAGE)
    {
      usage ("terminal type");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Set the terminal type");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      set_term_cmd (USAGE);
      long_help ("\
The terminal command sets the type of the current terminal to that specified\n\
in the type argument.  The only available terminal types are vt100 and dumb.\n\
This is used by the line editor to determine how to update the terminal\n\
display.\n");
      example ("\
terminal dumb\n\
Sets the type of the current terminal to a dumb terminal.\n");
      return 0;
    }

  if (argvect[1] == NULL || argvect[2] != NULL)
    set_term_cmd (USAGE);
  else
    set_term_name (argvect[1]);
  return 0;
}

int
reset_cmd (cmdmode_t mode)
{
    if (mode == USAGE)
    {
      usage ("reset");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Reset the board (not on all architectures).");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      reset_cmd (USAGE);
      long_help ("\
The reset command resets the board.  This may not be implemented\n\
on all architectures");
      return 0;
    }

  if (argvect[1] != NULL)
    {
      reset_cmd (USAGE);
    }
  else
    {
      __reset ();
    }
  return 0;
}

#if HAVE_USAGE
int
memusage_cmd (cmdmode_t mode)
{
  extern int sdata, _end;

  if (mode == USAGE)
    {
      usage ("usage");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Show monitor memory usage");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      memusage_cmd (USAGE);
      long_help ("\
The usage command shows the amount of memory being used by the monitor,\n\
broken down by category.  Despite its name, it has nothing to do with the\n\
usage of any other command.\n");
      return 0;
    }

  if (argvect[1] != NULL)
    {
      return memusage_cmd (USAGE);
    }
  else
    {
      xprintf ("%d bytes were allocated with sbrk\n", (char *)sbrk (0) - (char *)&_end);
    }
  return 0;
}
#endif /* HAVE_USAGE */

int
disassemble_cmd (cmdmode_t mode)
{
#ifdef DISASSEMBLER
  int x;

  if (mode == USAGE)
    {
      usage ("disassemble [location]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Disassemble memory");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      disassemble_cmd (USAGE);
      long_help ("\
The disassemble command disassembles the contents of memory.  Because of the\n\
way breakpoints are handled, all instructions are shown and breakpoints are\n\
not visible in the disassembled code.  The disassemble command takes zero\n\
or one argument.  When called with zero arguments, it starts disassembling\n\
from the current (user program) pc.  When called with a location, it starts\n\
disassembling from the specified location.  When called after a previous\n\
call and with no arguments, it disassembles the next area of memory after\n\
the one previously disassembled.\n");
      example ("\
disassemble 45667000\n\
Displays disassembled code starting at location 45667000.");
      return 0;
    }
  if (argvect [1] != NULL && argvect[2] != NULL)
    {
      return disassemble_cmd (USAGE);
    }

  if(argvect [1] != NULL)
    str2addr (argvect [1], &last_pc);
  for (x = 0; x < 10; x++)
    last_pc = do_dis (&last_pc);
  flush_dis ();
#else
#warning "DISASSEMBLER not implemented"
  xprintf ("disassembler not available\n");
#endif

  return 0;
}

int
copy_cmd (cmdmode_t mode)
{
  mem_addr_t src, dst;
  target_register_t size;

  if (mode == USAGE)
    {
      usage ("copy startaddr destaddr amount");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Copies one area of memory to another");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      copy_cmd (USAGE);
      long_help ("\
The copy command is used to copy 'amount' bytes of memory\n\
from 'startaddr' to 'destaddr'\n");
      example ("\
copy 10000 20000 300\n\
Copies 0x300 bytes of memory from 0x10000 to 0x20000.");
      return 0;
    }
  if (argvect[1] == NULL || argvect[2] == NULL || argvect[3] == NULL
      || argvect[4] != NULL)
    {
      return copy_cmd (USAGE);
    }
  str2addr (argvect[1], &src);
  str2addr (argvect[2], &dst);
  size = str2int (argvect[3], 16);
  while (size > 0)
    {
      char buf[128];

      int msize = (size > 128) ? 128 : size;
      if (read_memory (&src, 1, msize, buf))
        {
          xprintf ("Memory read failed\n");
          break;
        }
      if (write_memory (&dst, 1, msize, buf))
        {
          xprintf ("Memory write failed\n");
          break;
        }
#ifdef HAVE_BSP
      bsp_flush_dcache((void *)dst.addr, msize);
      bsp_flush_icache((void *)dst.addr, msize);
#endif
      ADD_OFFSET (&src, &src, msize);
      ADD_OFFSET (&dst, &dst, msize);
      size -= msize;
    }
  return 0;
}

#ifndef HAVE_BSP
int
set_program_args_cmd (cmdmode_t mode)
{
  int argc;

  if (mode == USAGE)
    {
      usage ("setargs [args]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Sets the program arguments passed to main()");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      set_program_args_cmd (SHORT_HELP);
      return 0;
    }
  for (argc = 1; argvect[argc] != NULL; argc++)
    ;
  __set_program_args (argc - 1, argvect + 1);
  return 0;
}
#endif


int
fill_cmd (cmdmode_t mode)
{
  if (mode == USAGE)
    {
      usage ("fill[.size] startaddress endaddress [value]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Fills memory with a specified value");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      fill_cmd (USAGE);
      long_help("\
The fill command is used to fill a region of memory from 'startaddress'\n\
to 'endaddress' with the value in 'value'.  If no value is specificed,\n\
it uses zero.  It can take a size extension, which follows the command\n\
name or partial command name without a space, and is a period followed\n\
by the size of the writes that are used, in bits.  Options are 8, 16,\n\
32, and 64.  Without a size extension, the fill command defaults to\n\
changing 8 bits at a time.\n");
      example ("\
fill.32 10000 20000 32\n\
Fills the region between 0x10000 and 0x20000 with the 32 bit value 0x32.");
      return 0;
    }
  if ((argvect[1] == NULL || argvect[2] == NULL)
      || (argvect[3] != NULL && argvect[4] != NULL))
    {
      fill_cmd (USAGE);
      return 0;
    } else { 
      mem_addr_t start,end;
      char value[8];
      int size = get_cmd_size();
      int amt;

      if (size == -1)
          /*
           * Invalid size specified.
           */
          return 0;

      if (argvect[3] != NULL)
        {
          hex2bytes (argvect[3], value, size);
        }
      else
        {
          hex2bytes ("0", value, size);
        }
      str2addr (argvect[1], &start);
      str2addr (argvect[2], &end);
      amt = MEM_ADDR_DIFF (end, start);
      if (amt < 0)
        {
          xprintf ("Addresses in incorrect order\n");
        }
      else
        {
          int x;
	  if (get_memory_display_mode ())
	    {
	      /* Gotta swap this puppy. */
	      int x;

	      for (x = 0; x < (size / 2); x++)
		{
		  char tmp = value[x];
		  value [x] = value [size - 1 - x];
		  value [size - 1 - x] = tmp;
		}
	    }

          xprintf ("Writing %d units\n", amt / size + 1);
          for (x = amt / size; x >= 0; x--)
            {
              if (write_memory (&start, size, 1, value))
	        {
                  xprintf ("Memory write failed\n");
                  break;
                }
              ADD_OFFSET (&start, &start, size);
            }
        }
    }
  return 0;
}


int
swapmem_cmd (cmdmode_t mode)
{
  int display_settings = 0;

  if (mode == USAGE)
    {
      usage ("swapmem [little|big]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Sets whether or not memory values are byte-swapped");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      swapmem_cmd (USAGE);

      long_help("\
The swapmem command is used to determine whether or not memory values are\n\
displayed and written in little or big-endian byte order. By default, values\n\
are read and written to match the byte order of the target CPU.\n\
This command does not alter the CPU state in any way; it only changes the\n\
way memory values are displayed and written within the monitor.\n");
      example("\
swapmem \n\
Displays the byte order that is currently in effect.");
      return 0;
      display_settings = 1;
    }
  else
    {
      if (argvect[1] != NULL && argvect[2] != NULL)
	{
	  swapmem_cmd (USAGE);
	  return 0;
	}
    }
  if (display_settings || argvect[1] == NULL)
    {
      if (get_memory_display_mode ())
	{
	  xprintf ("Memory values are read and written in little-endian byte order.\n");
	}
      else
	{
	  xprintf ("Memory values are read and written in big-endian byte order.\n");
	}
      return 0;
    }
  if (strncmp (argvect[1], "little", strlen (argvect[1])) == 0)
    {
      set_memory_display_mode (1);
      xprintf ("Memory values are now read and written in little-endian order.\n");
    }
  else if (strncmp (argvect[1], "big", strlen (argvect[1])) == 0)
    {
      set_memory_display_mode (0);
      xprintf ("Memory values are now read and written in big-endian order.\n");
    }
  else
    {
      return swapmem_cmd (USAGE);
    }
  return 0;
}

#ifdef MONITOR_CONTROL_INTERRUPTS
int
int_cmd (cmdmode_t mode)
{
  if (mode == USAGE)
    {
      usage ("interrupt [on|off]");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("Enables or disables interrupts within the monitor");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      int_cmd (USAGE);
      long_help("\
The interrupt command is used to enable or disable interrupts while the\n\
monitor is running.");
      return 0;
    }
  if (argvect[1] != NULL && argvect[2] != NULL)
    {
      int_cmd (USAGE);
      return 0;
    }
  if (argvect[1] != NULL)
    {
      if (strcmp (argvect[1], "on") == 0)
	{
	  monitor_enable_interrupts ();
	}
      else if (strcmp (argvect[1], "off") == 0)
	{
	  monitor_disable_interrupts ();
	}
      else
	{
	  int_cmd (USAGE);
	  return 0;
	}
    }
  xprintf ("Interrupts ");
  if (monitor_interrupt_state ())
    {
      xprintf ("enabled\n");
    }
  else
    {
      xprintf ("disabled\n");
    }
  return 0;
}
#endif


/* Table used by the crc32 function to calcuate the checksum. */
static uint32 crc32_table[256];
static int crc_initted = 0;

static uint32
crc32 (unsigned char *buf, int len, uint32 crc)
{
  if (! crc_initted)
    {
      /* Initialize the CRC table and the decoding table. */
      int i, j;
      uint32 c;

      crc_initted = 1;
      for (i = 0; i < 256; i++)
	{
	  for (c = i << 24, j = 8; j > 0; --j)
	    c = c & 0x80000000 ? (c << 1) ^ 0x04c11db7 : (c << 1);
	  crc32_table[i] = c;
	}
    }

  while (len--)
    {
      crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *buf) & 255];
      buf++;
    }
  return crc;
}


int
checksumcmd (cmdmode_t mode)
{
  mem_addr_t start, end;
  uint32 crc = 0xffffffff;

  if (mode == USAGE)
    {
      usage ("crc startaddr endaddr");
      return 0;
    }
  if (mode == SHORT_HELP)
    {
      short_help ("checksum an area of memory");
      return 0;
    }
  if (mode == LONG_HELP)
    {
      checksumcmd (USAGE);
      long_help ("\
The crc command is used to calculate a standard CRC32 checksum of the\n\
specified memory region. The checksum is printed out as a hexadecimal\n\
value.");
      return 0;
    }
  if (argvect[1] == NULL || argvect[2] == NULL || argvect[3] != NULL)
    {
      return checksumcmd (USAGE);
    }

  str2addr (argvect[1], &start);
  str2addr (argvect[2], &end);
  while (start.addr < end.addr)
    {
      char c[1024];
      int len = end.addr - start.addr;
      if (len > sizeof(c))
	len = sizeof(c);
      
      read_memory (&start, 1, len, c);
      crc = crc32 (c, len, crc);
      start.addr += len;
    }
  xprintf("0x%08lx is checksum\n", crc);
  return 0;
}

char **argvect;
static char argvect_cmd[MAXLINELEN];

static char **
buildargv (char *input)
{
  static char *arglist[256];
  int numargs = 0;

  while (1)
    {
      while (isspace ((unsigned char)*input) && *input != 0)
	  input++;
      if (*input == 0)
	break;
      arglist [numargs++] = input;
      while (!isspace ((unsigned char)*input) && *input != 0)
	input++;
      if (*input == 0)
	break;
      *(input++) = 0;
    }
  arglist [numargs] = NULL;
  return arglist;
}


int
monitor_loop (void)
{
  int state = 1, return_value = 0;

  while (state == 1)
    {
      /* Get a line of input, putting it in the input buffer */
      lineedit (PROMPT, inbuf, sizeof (inbuf));
      xprintf ("\n");

      if (switch_to_stub_flag)
        {
#ifndef HAVE_BSP
	  switch_to_stub_flag = 0;
#endif
          return transfer_to_stub ();
        }

      /* Separate off the command from any other stuff on the line */
      
      strcpy (argvect_cmd, inbuf);
      argvect = buildargv (argvect_cmd);

      if (argvect[0] != NULL && argvect[0][0] != '\0')
	{
	  char *ptr;
	  int command_number;

	  strcpy (cmd, argvect[0]);

	  /* Function to split off . delimiters. */
	  ptr = strchr (cmd, '.');

	  if (ptr != NULL && *ptr == '.')
	    *ptr = '\0';

	  /* See if it's an alias. */
	  command_number = alias_compare (cmd);

	  /* Compare input to command list, check for conflicts. */
	  if (command_number < 0)
	    command_number = command_compare (cmd);

	  /* If we found a command, just run the function */
	  if (command_number >= 0)
	    {
	      int status;
	      /* Execute the function, if the function returns a non-zero 
		 value, break out of the loop and return. */
	      status = (*cmdtab[command_number].function) (INVOCATION);
	      if (status)
		{
		  state = 0;
		  if (status < 0)
		    return_value = 1;
		}
	    }
	  else
	    {
	      /* If none of the commands or aliases match, complain. */
	      xprintf ("Not a legal command\n");
	    }
	  
	  if (inbuf[0] != '\0')
	    addHistoryCmd (inbuf);
	}
    }
  return return_value;
}


