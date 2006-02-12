
/*
 * Copyright (c) 2004, Andreas Tønnesen(andreto-at-olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * * Neither the name of the UniK olsr daemon nor the names of its contributors 
 *   may be used to endorse or promote products derived from this software 
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* $Id: olsrd_plugin.c,v 1.4 2005/02/25 22:35:53 kattemat Exp $ */


/*
 * Dynamic linked library example for UniK OLSRd
 */


#include "olsrd_plugin.h"
#include <stdio.h>
#include "plugin_loader.h"

void __attribute__ ((constructor)) 
my_init(void);

void __attribute__ ((destructor)) 
my_fini(void);

int
register_olsr_data(struct olsr_plugin_data *);

int
fetch_olsrd_data(void);

/*
 * Returns the version of the plugin interface that is used
 * THIS IS NOT THE VERSION OF YOUR PLUGIN!
 */
int 
get_plugin_interface_version()
{
  return PLUGIN_INTERFACE_VERSION;
}

/**
 *Constructor
 */
void
my_init()
{
  /* Print plugin info to stdout */
  printf("%s\n", MOD_DESC);

  ifs = NULL;

  return;
}

/**
 *Destructor
 */
void
my_fini()
{

  /* Calls the destruction function
   * olsr_plugin_exit()
   * This function should be present in your
   * sourcefile and all data destruction
   * should happen there - NOT HERE!
   */
  olsr_plugin_exit();

  return;
}


/**
 *Register needed functions and pointers
 *
 *This function should not be changed!
 *
 */
int
register_olsr_data(struct olsr_plugin_data *data)
{
  /* IPversion */
  ipversion = data->ipversion;
  /* Main address */
  main_addr = data->main_addr;

  /* Multi-purpose function */
  olsr_plugin_io = data->olsr_plugin_io;

  /* Set size of IP address */
  if(ipversion == AF_INET)
    {
      ipsize = sizeof(olsr_u32_t);
    }
  else
    {
      ipsize = sizeof(struct in6_addr);
    }

  if(!fetch_olsrd_data())
    {
      fprintf(stderr, "Could not fetch the neccessary functions from olsrd!\n");
      return 0;
    }

  /* Calls the initialization function
   * olsr_plugin_init()
   * This function should be present in your
   * sourcefile and all data initialization
   * should happen there - NOT HERE!
   */
  if(!olsr_plugin_init())
    {
      fprintf(stderr, "Could not initialize plugin!\n");
      return 0;
    }

  if(!plugin_ipc_init())
    {
      fprintf(stderr, "Could not initialize plugin IPC!\n");
      return 0;
    }

  return 1;
}



int
fetch_olsrd_data()
{
  int retval = 1;


  /* Packet buffer */
  if(!olsr_plugin_io(GETF__NET_OUTBUFFER_PUSH, &net_outbuffer_push, sizeof(net_outbuffer_push)))
  {
    net_outbuffer_push = NULL;
    retval = 0;
  }

  /* Olsr debug output function */
  if(!olsr_plugin_io(GETF__OLSR_PRINTF, 
		     &olsr_printf, 
		     sizeof(olsr_printf)))
  {
    olsr_printf = NULL;
    retval = 0;
  }


  /* Olsr malloc wrapper */
  if(!olsr_plugin_io(GETF__OLSR_MALLOC, 
		     &olsr_malloc, 
		     sizeof(olsr_malloc)))
  {
    olsr_malloc = NULL;
    retval = 0;
  }

  /* Parser registration */
  if(!olsr_plugin_io(GETF__OLSR_PARSER_ADD_FUNCTION, 
		     &olsr_parser_add_function, 
		     sizeof(olsr_parser_add_function)))
  {
    olsr_parser_add_function = NULL;
    retval = 0;
  }

  /* Scheduler timeout registration */
  if(!olsr_plugin_io(GETF__OLSR_REGISTER_TIMEOUT_FUNCTION, 
		     &olsr_register_timeout_function, 
		     sizeof(olsr_register_timeout_function)))
  {
    olsr_register_timeout_function = NULL;
    retval = 0;
  }

  /* Scheduler event registration */
  if(!olsr_plugin_io(GETF__OLSR_REGISTER_SCHEDULER_EVENT, 
		     &olsr_register_scheduler_event, 
		     sizeof(olsr_register_scheduler_event)))
  {
    olsr_register_scheduler_event = NULL;
    retval = 0;
  }

  /* Double to mantissa/exponent */
  if(!olsr_plugin_io(GETF__DOUBLE_TO_ME, 
		     &double_to_me, 
		     sizeof(double_to_me)))
  {
    double_to_me = NULL;
    retval = 0;
  }



  /* Mantissa/exponent to double conversion */
  if(!olsr_plugin_io(GETF__ME_TO_DOUBLE, 
		     &me_to_double, 
		     sizeof(me_to_double)))
  {
    me_to_double = NULL;
    retval = 0;
  }


  /* Interface list */
  if(!olsr_plugin_io(GETD__IFNET, &ifs, sizeof(ifs)))
  {
    ifs = NULL;
    retval = 0;
  }

  /* Messageseqno fetch function */
  if(!olsr_plugin_io(GETF__GET_MSG_SEQNO, &get_msg_seqno, sizeof(get_msg_seqno)))
  {
    get_msg_seqno = NULL;
    retval = 0;
  }

  /* Scheduler maintained timestamp */
  if(!olsr_plugin_io(GETD__NOW, &now, sizeof(now)))
  {
    now = NULL;
    retval = 0;
  }

  /* Output function */
  if(!olsr_plugin_io(GETF__NET_OUTPUT, &net_output, sizeof(net_output)))
  {
    net_output = NULL;
    retval = 0;
  }

  /* Duplicate check (for processing) */
  if(!olsr_plugin_io(GETF__OLSR_CHECK_DUP_TABLE_PROC, &check_dup_proc, sizeof(check_dup_proc)))
  {
    check_dup_proc = NULL;
    retval = 0;
  }

  /* Default forward function */
  if(!olsr_plugin_io(GETF__OLSR_FORWARD_MESSAGE, &default_fwd, sizeof(default_fwd)))
  {
    default_fwd = NULL;
    retval = 0;
  }


  /* Add socket to OLSR select function */
  if(!olsr_plugin_io(GETF__ADD_OLSR_SOCKET, &add_olsr_socket, sizeof(add_olsr_socket)))
  {
    add_olsr_socket = NULL;
    retval = 0;
  }

  /* Neighbor link status lookup */
  if(!olsr_plugin_io(GETF__CHECK_NEIGHBOR_LINK, &check_neighbor_link, sizeof(check_neighbor_link)))
  {
    check_neighbor_link = NULL;
    retval = 0;
  }


  /* Apm info */
  if(!olsr_plugin_io(GETF__APM_READ, &apm_read, sizeof(apm_read)))
  {
    apm_read = NULL;
    retval = 0;
  }

  return retval;

}
