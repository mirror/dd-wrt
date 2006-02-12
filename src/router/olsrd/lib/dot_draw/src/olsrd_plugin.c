/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
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
 * $Id: olsrd_plugin.c,v 1.10 2005/02/25 22:43:21 kattemat Exp $
 */

/*
 * Dynamic linked library for the olsr.org olsr daemon
 */


#include "olsrd_plugin.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
 * Defines the version of the plugin interface that is used
 * THIS IS NOT THE VERSION OF YOUR PLUGIN!
 * Do not alter unless you know what you are doing!
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

  /* defaults for parameters */
  ipc_port = 2004;
  ipc_accept_ip.s_addr = htonl(INADDR_LOOPBACK);
  
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

int
register_olsr_param(char *key, char *value)
{
  if(!strcmp(key, "port"))
    {
     ipc_port = atoi(value);
     printf("(DOT DRAW) listening on port: %d\n", ipc_port);
    }

  if(!strcmp(key, "accept"))
    {
	inet_aton(value, &ipc_accept_ip);
	printf("(DOT DRAW) accept only: %s\n", inet_ntoa(ipc_accept_ip));
    }
  return 1;
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

  
  /* Neighbor table */
  if(!olsr_plugin_io(GETD__NEIGHBORTABLE, 
		     &neighbortable, 
		     sizeof(neighbortable)))
  {
    neighbortable = NULL;
    retval = 0;
  }
  
  /* Two hop neighbor table */
  if(!olsr_plugin_io(GETD__TWO_HOP_NEIGHBORTABLE, 
		     &two_hop_neighbortable, 
		     sizeof(two_hop_neighbortable)))
  {
    two_hop_neighbortable = NULL;
    retval = 0;
  }

  /* link set */
  if(!olsr_plugin_io(GETD__LINK_SET, &link_set, sizeof(link_set))) {
    link_set = NULL;
    printf("********* err\n");
    retval = 0;
  }
  
  /* Topoloy table */
  if(!olsr_plugin_io(GETD__TC_TABLE, 
		     &tc_table, 
		     sizeof(tc_table)))
  {
    tc_table = NULL;
    retval = 0;
  }

  /* HNA table */
  if(!olsr_plugin_io(GETD__HNA_SET, 
		     &hna_set, 
		     sizeof(hna_set)))
  {
    hna_set = NULL;
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

  /* "ProcessChanges" event registration */
  if(!olsr_plugin_io(GETF__REGISTER_PCF, 
		     &register_pcf, 
		     sizeof(register_pcf)))
  {
    register_pcf = NULL;
    retval = 0;
  }



  /* Add socket to OLSR select function */
  if(!olsr_plugin_io(GETF__ADD_OLSR_SOCKET, &add_olsr_socket, sizeof(add_olsr_socket)))
  {
    add_olsr_socket = NULL;
    retval = 0;
  }

  /* Remove socket from OLSR select function */
  if(!olsr_plugin_io(GETF__REMOVE_OLSR_SOCKET, &remove_olsr_socket, sizeof(remove_olsr_socket)))
  {
    remove_olsr_socket = NULL;
    retval = 0;
  }

  return retval;

}
