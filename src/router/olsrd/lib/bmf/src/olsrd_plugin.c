/*
 * OLSR Basic Multicast Forwarding (BMF) plugin.
 * Copyright (c) 2005, 2006, Thales Communications, Huizen, The Netherlands.
 * Written by Erik Tromp.
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
 * * Neither the name of Thales, BMF nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
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
 */

/* -------------------------------------------------------------------------
 * File       : olsrd_plugin.c
 * Description: Interface to the OLSRD plugin system
 * Created    : 29 Jun 2006
 *
 * $Id: olsrd_plugin.c,v 1.2 2007/02/10 17:05:56 bernd67 Exp $ 
 * ------------------------------------------------------------------------- */

/* System includes */
#include <assert.h> /* assert() */
#include <stdio.h>

/* OLSRD includes */
#include "olsrd_plugin.h"
#include "defs.h" /* olsr_u8_t, olsr_cnf */
#include "scheduler.h" /* olsr_register_scheduler_event */

/* BMF includes */
#include "Bmf.h" /* InitBmf(), CloseBmf(), RegisterBmfParameter() */
#include "PacketHistory.h" /* InitPacketHistory() */

static void __attribute__ ((constructor)) my_init(void);
static void __attribute__ ((destructor)) my_fini(void);

void olsr_plugin_exit(void);

/* -------------------------------------------------------------------------
 * Function   : olsrd_plugin_interface_version
 * Description: Plugin interface version
 * Input      : none
 * Output     : none
 * Return     : BMF plugin interface version number
 * Data Used  : none
 * Notes      : Called by main OLSRD (olsr_load_dl) to check plugin interface
 *              version
 * ------------------------------------------------------------------------- */
int olsrd_plugin_interface_version()
{
  return OLSRD_PLUGIN_INTERFACE_VERSION;
}

/* -------------------------------------------------------------------------
 * Function   : olsrd_plugin_init
 * Description: Plugin initialisation
 * Input      : none
 * Output     : none
 * Return     : fail (0) or success (1)
 * Data Used  : olsr_cnf
 * Notes      : Called by main OLSRD (init_olsr_plugin) to initialize plugin
 * ------------------------------------------------------------------------- */
int olsrd_plugin_init()
{
  /* Check validity */
  if (olsr_cnf->ip_version != AF_INET)
  {
    fprintf(stderr, PLUGIN_NAME ": This plugin only supports IPv4!\n");
    return 0;
  }

  /* Clear the packet history */
  InitPacketHistory();

  /* Register ifchange function */
  add_ifchgf(&InterfaceChange);

  /* Register the duplicate registration pruning process */
  olsr_register_scheduler_event(&PrunePacketHistory, NULL, 3.0, 2.0, NULL);

  return InitBmf(NULL);
}

/* -------------------------------------------------------------------------
 * Function   : olsr_plugin_exit
 * Description: Plugin cleanup
 * Input      : none
 * Output     : none
 * Return     : none
 * Data Used  : none
 * Notes      : Called by my_fini() at unload of shared object
 * ------------------------------------------------------------------------- */
void olsr_plugin_exit()
{
  CloseBmf();
}

/* -------------------------------------------------------------------------
 * Function   : olsrd_plugin_register_param
 * Description: Register parameters from config file
 * Input      : key - the parameter name
 *              value - the parameter value
 * Output     : none
 * Return     : fatal error (<0), minor error (0) or success (>0)
 * Data Used  : none
 * Notes      : Called by main OLSR (init_olsr_plugin) for all plugin parameters
 * ------------------------------------------------------------------------- */
int olsrd_plugin_register_param(char* key, char* value)
{
  assert(key != NULL && value != NULL);

  return RegisterBmfParameter(key, value);
}
 
/* -------------------------------------------------------------------------
 * Function   : my_init
 * Description: Plugin constructor
 * Input      : none
 * Output     : none
 * Return     : none
 * Data Used  : none
 * Notes      : Called at load of shared object
 * ------------------------------------------------------------------------- */
static void my_init()
{
  /* Print plugin info to stdout */
  printf("%s\n", MOD_DESC);

  return;
}

/* -------------------------------------------------------------------------
 * Function   : my_fini
 * Description: Plugin destructor
 * Input      : none
 * Output     : none
 * Return     : none
 * Data Used  : none
 * Notes      : Called at unload of shared object
 * ------------------------------------------------------------------------- */
static void my_fini()
{
  olsr_plugin_exit();
}
