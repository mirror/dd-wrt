//==========================================================================
//
//      monitor_cmd.h
//
//      Monitor command definitions for the CygMON ROM monitor
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
// Contributors: gthomas
// Date:         1999-10-20
// Purpose:      Monitor command definitions for the CygMON ROM monitor
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================

#ifndef MONITOR_CMD_H
#define MONITOR_CMD_H

#ifndef ASM
typedef enum {
  INVOCATION, USAGE, SHORT_HELP, LONG_HELP
} cmdmode_t;

struct cmdentry
{
  char *alias;
  char *cmd;
  int (*function) (cmdmode_t);
};

extern struct cmdentry cmdtab [];

extern int monitor_loop (void);

extern char inbuf[];

extern char **argvect;

typedef int (*srec_input_func_t)(void);

extern int load_srec(srec_input_func_t inp_func);

#ifdef USE_HELP
extern void usage (char *string);
extern void short_help (char *string);
extern void long_help (char *string);
extern void example (char *string);
#else
#define usage(x) no_help_usage ()
#define short_help(x) no_help()
#define long_help(x) no_help()
#define example(x)
extern void no_help (void);
extern void no_help_usage (void);
#endif
extern int help_cmd (cmdmode_t mode);
extern int mem_cmd (cmdmode_t mode);

extern int dump_cmd (cmdmode_t mode);
extern int ethaddr_cmd (cmdmode_t mode);
extern int ipaddr_cmd (cmdmode_t mode);
extern int tcpport_cmd (cmdmode_t mode);
extern int load_cmd (cmdmode_t mode);
extern int reg_cmd (cmdmode_t mode);
extern int go_cmd (cmdmode_t mode);
extern int othernames_cmd (cmdmode_t mode);
extern int step_cmd (cmdmode_t mode);
extern int transfer_cmd (cmdmode_t mode);
extern int timer_cmd (cmdmode_t mode);
extern int disassemble_cmd (cmdmode_t mode);
extern int breakpoint_cmd (cmdmode_t mode);
extern int clear_breakpoint_cmd (cmdmode_t mode);
extern int memusage_cmd (cmdmode_t mode);
extern int set_serial_port_cmd (cmdmode_t mode);
extern int set_serial_speed_cmd (cmdmode_t mode);
extern int version_cmd (cmdmode_t mode);
extern int cache_cmd (cmdmode_t mode);
extern int set_term_cmd (cmdmode_t mode);
extern int reset_cmd (cmdmode_t mode);
extern int copy_cmd (cmdmode_t mode);
extern int fill_cmd (cmdmode_t mode);
extern int set_program_args_cmd (cmdmode_t mode);
extern int swapmem_cmd (cmdmode_t mode);
extern int checksumcmd (cmdmode_t mode);
extern int int_cmd (cmdmode_t mode);
#endif

#endif
