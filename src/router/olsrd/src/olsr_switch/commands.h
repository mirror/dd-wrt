/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#ifndef _COMMAND
#define _COMMAND

#include "ohs_cmd.h"

static const struct ohs_command ohs_commands[] = {
  {"help", "help",
   "Help on shell commands",
   "Help on shell commands",
   ohs_cmd_help},
  {"exit", "exit",
   "Exits olsr host switch",
   "Exits olsr host switch",
   ohs_cmd_exit},
  {"log", "log <set [type]>",
   "Displays or sets log bits",
   "This command sets or clears various logbits. Available logbits are:\n FORWARD - display info about all traffic forwarding\n CONNECT - display info about all connections happening\n LINK - display all link lookup information\n",
   ohs_cmd_log},
  {"list", "list <clients|links>",
   "List all connected clients or links",
   "This command will list all the clients or all the links registered by olsr_switch. By default clients are listed.",
   ohs_cmd_list},
  {"link", "link <bi> [srcIP|*] [dstIP|*] [0-100]",
   "Manipulate links",
   "This command is used for manipulating olsr links. The link quality is a number between 0-100 representing the chance in percentage for a packet to be forwarded on the link.\nTo make the link between 10.0.0.1 and 10.0.0.2 have 50% packet loss do:\nlink 10.0.0.1 10.0.0.2 50\nNote that this will only effect the unidirectional link 10.0.0.1 -> 10.0.0.2.\nTo make the changes affect traffic in both directions do:\nlink bi 10.0.0.1 10.0.0.2 50\nTo completely block a link do:\nlink 10.0.0.1 10.0.0.2 0\nTo make all traffic pass(delete the entry) do:\nlink 10.0.0.1 10.0.0.2 100\nNote that \"bi\" can be used in all these examples.\nWildcard source and/or destinations are also supported.\nTo block all traffic from a node do:\nlink 10.0.0.1 * 0\nTo set 50% packet loss on all links to 10.0.0.2 do:\nlink * 10.0.0.2 50\nTo delete all links do:\nlink * * 100\nWildcards can also be used in combination with 'bi'.\nTo list all manipulated links use 'list links'.\n",
   ohs_cmd_link},
  {"olsrd", "olsrd [start|stop|show|setb|seta] [IP|path|args]",
   "Start or stop local olsrd processes. Also used to set the olsrd binary path and arguments",
   "This command is used for managing local olsrd instances from within olsr_switch.\nThe command can be configured in runtime using the setb and seta sub-commands.\nTo show the current olsrd command-configuration do:\nolsrd show\nTo set the olsrd binary path do:\nolsrd setb /full/path/to/olsrd\nTo start a olsrd instance with a IP address of 10.0.0.1, do:\nolsrd start 10.0.0.1\nTo stop that same instance do:\nolsrd stop 10.0.0.1\nseta would set arguments but is currently not implemented\n",
   ohs_cmd_olsrd},
  {NULL, NULL,
   NULL,
   NULL,
   NULL}
};

#endif /* _COMMAND */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
