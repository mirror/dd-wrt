/*
 * Logging function
 * Copyright (C) 1999 Yasuhiro Ohara
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
 * Boston, MA 02111-1307, USA.  
 */

#include <zebra.h>

/* Include other stuffs */
#include "log.h"
#include "command.h"

#include "linklist.h"
#include "prefix.h"

#include "ospf6_prefix.h"
#include "ospf6_lsa.h"
#include "ospf6_message.h"
#include "ospf6_interface.h"

#define HEADER_DEPENDENCY
#include "ospf6d.h"
#undef HEADER_DEPENDENCY
#include "ospf6_dump.h"

void
ospf6_log_init ()
{
  int flag = 0;

  if (!daemon_mode)
    flag |= ZLOG_STDOUT;

  zlog_default = openzlog (progname, flag, ZLOG_OSPF6,
                 LOG_CONS|LOG_NDELAY|LOG_PERROR|LOG_PID,
                 LOG_DAEMON);
}

struct _ospf6_dump ospf6_dump[] =
{
  {0, "hello"},
  {0, "dbdesc"},
  {0, "lsreq"},
  {0, "lsupdate"},
  {0, "lsack"},
  {0, "neighbor"},
  {0, "interface"},
  {0, "area"},
  {0, "lsa"},
  {0, "zebra"},
  {0, "config"},
  {0, "dbex"},
  {0, "spf"},
  {0, "route"},
  {0, "lsdb"},
  {0, "redistribute"}
};

/* this is included in "../lib/command.h"
#define OSPF6_DUMP_TYPE_LIST \
  "(hello|dbdesc|lsreq|lsupdate|lsack|neighbor|interface|area" \
  "|lsa|zebra|config|dbex|spf|route|lsdb|redistribute)"
 */

DEFUN (debug_ospf6,
       debug_ospf6_cmd,
       "debug ospf6 " OSPF6_DUMP_TYPE_LIST,
       "Debugging information\n"
       OSPF6_STR
       )
{
  int i;

  if (! strcmp (argv[0], "all"))
    {
      for (i = 0; i < OSPF6_DUMP_MAX; i++)
        ospf6_dump[i].dump = 1;
      return CMD_SUCCESS;
    }

  for (i = 0; i < OSPF6_DUMP_MAX; i++)
    {
      if (strcmp (argv[0], ospf6_dump[i].string))
        continue;
      ospf6_dump[i].dump = 1;
      return CMD_SUCCESS;
    }
  return CMD_ERR_NO_MATCH;
}

DEFUN (no_debug_ospf6,
       no_debug_ospf6_cmd,
       "no debug ospf6 " OSPF6_DUMP_TYPE_LIST,
       NO_STR
       "Debugging information\n"
       OSPF6_STR
       )
{
  int i;

  if (! strcmp (argv[0], "all"))
    {
      for (i = 0; i < OSPF6_DUMP_MAX; i++)
        ospf6_dump[i].dump = 0;
      return CMD_SUCCESS;
    }

  for (i = 0; i < OSPF6_DUMP_MAX; i++)
    {
      if (strcmp (argv[0], ospf6_dump[i].string))
        continue;
      ospf6_dump[i].dump = 0;
      return CMD_SUCCESS;
    }
  return CMD_ERR_NO_MATCH;
}

DEFUN (show_debugging_ospf6,
       show_debugging_ospf6_cmd,
       "show debugging ospf6",
       SHOW_STR
       "Debugging infomation\n"
       OSPF6_STR)
{
  int i;
  vty_out (vty, "OSPF6 debugging status:%s", VTY_NEWLINE);
  for (i = 0; i < OSPF6_DUMP_MAX; i++)
    {
      vty_out (vty, "  OSPF6 Dump %s: %s%s", ospf6_dump[i].string,
               (ospf6_dump[i].dump ? "on " : "off"), VTY_NEWLINE);
    }
  return CMD_SUCCESS;
}

struct cmd_node debug_node =
{
  DEBUG_NODE,
  ""
};

int
ospf6_config_write_debug (struct vty *vty)
{
  int i;
  for (i = 0; i < OSPF6_DUMP_MAX; i++)
    {
      if (! ospf6_dump[i].dump)
        continue;
      vty_out (vty, "debug ospf6 %s%s", ospf6_dump[i].string, VTY_NEWLINE);
    }
  vty_out (vty, "!%s", VTY_NEWLINE);
  return 0;
}

/* Backward campatibility 2000/12/29 */
DEFUN (debug_ospf6_message,
       debug_ospf6_message_cmd,
       "debug ospf6 message (hello|dbdesc|lsreq|lsupdate|lsack|all)",
       "Debugging infomation\n"
       OSPF6_STR
       "OSPF6 messages\n"
       "OSPF6 Hello\n"
       "OSPF6 Database Description\n"
       "OSPF6 Link State Request\n"
       "OSPF6 Link State Update\n"
       "OSPF6 Link State Acknowledgement\n"
       "OSPF6 all messages\n"
       )
{
  assert (argc);
  if (!strcmp (argv[0], "hello"))
    ospf6_dump[OSPF6_DUMP_HELLO].dump = 1;
  else if (!strcmp (argv[0], "dbdesc"))
    ospf6_dump[OSPF6_DUMP_DBDESC].dump = 1;
  else if (!strcmp (argv[0], "lsreq"))
    ospf6_dump[OSPF6_DUMP_LSREQ].dump = 1;
  else if (!strcmp (argv[0], "lsupdate"))
    ospf6_dump[OSPF6_DUMP_LSUPDATE].dump = 1;
  else if (!strcmp (argv[0], "lsack"))
    ospf6_dump[OSPF6_DUMP_LSACK].dump = 1;
  else if (!strcmp (argv[0], "all"))
    ospf6_dump[OSPF6_DUMP_HELLO].dump = ospf6_dump[OSPF6_DUMP_DBDESC].dump =
    ospf6_dump[OSPF6_DUMP_LSREQ].dump = ospf6_dump[OSPF6_DUMP_LSUPDATE].dump =
    ospf6_dump[OSPF6_DUMP_LSACK].dump = 1;
  else
    return CMD_ERR_NO_MATCH;

  return CMD_SUCCESS;
}

DEFUN (no_debug_ospf6_message,
       no_debug_ospf6_message_cmd,
       "no debug ospf6 message (hello|dbdesc|lsreq|lsupdate|lsack|all)",
       NO_STR
       "Debugging infomation\n"
       OSPF6_STR
       "OSPF6 messages\n"
       "OSPF6 Hello\n"
       "OSPF6 Database Description\n"
       "OSPF6 Link State Request\n"
       "OSPF6 Link State Update\n"
       "OSPF6 Link State Acknowledgement\n"
       "OSPF6 all messages\n"
       )
{
  assert (argc);
  if (!strcmp (argv[0], "hello"))
    ospf6_dump[OSPF6_DUMP_HELLO].dump = 0;
  else if (!strcmp (argv[0], "dbdesc"))
    ospf6_dump[OSPF6_DUMP_DBDESC].dump = 0;
  else if (!strcmp (argv[0], "lsreq"))
    ospf6_dump[OSPF6_DUMP_LSREQ].dump = 0;
  else if (!strcmp (argv[0], "lsupdate"))
    ospf6_dump[OSPF6_DUMP_LSUPDATE].dump = 0;
  else if (!strcmp (argv[0], "lsack"))
    ospf6_dump[OSPF6_DUMP_LSACK].dump = 0;
  else if (!strcmp (argv[0], "all"))
    ospf6_dump[OSPF6_DUMP_HELLO].dump = ospf6_dump[OSPF6_DUMP_DBDESC].dump =
    ospf6_dump[OSPF6_DUMP_LSREQ].dump = ospf6_dump[OSPF6_DUMP_LSUPDATE].dump =
    ospf6_dump[OSPF6_DUMP_LSACK].dump = 0;
  else
    return CMD_ERR_NO_MATCH;

  return CMD_SUCCESS;
}

void
ospf6_debug_init ()
{
  install_node (&debug_node, ospf6_config_write_debug);

  install_element (VIEW_NODE, &show_debugging_ospf6_cmd);
  install_element (ENABLE_NODE, &show_debugging_ospf6_cmd);

  install_element (CONFIG_NODE, &debug_ospf6_message_cmd);
  install_element (CONFIG_NODE, &no_debug_ospf6_message_cmd);
  install_element (CONFIG_NODE, &debug_ospf6_cmd);
  install_element (CONFIG_NODE, &no_debug_ospf6_cmd);
}

