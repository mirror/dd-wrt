
/*
 * Secure OLSR plugin
 * http://www.olsr.org
 *
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or 
 * without modification, are permitted provided that the following 
 * conditions are met:
 *
 * * Redistributions of source code must retain the above copyright 
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in 
 *   the documentation and/or other materials provided with the 
 *   distribution.
 * * Neither the name of olsrd, olsr.org nor the names of its 
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
 * $Id: olsrd_plugin.c,v 1.10 2005/05/29 12:47:43 br1 Exp $
 */



#include "olsrd_plugin.h"
#include "olsrd_secure.h"
#include <stdio.h>
#include <string.h>


#define PLUGIN_NAME    "OLSRD signature plugin"
#define PLUGIN_VERSION "0.5"
#define PLUGIN_AUTHOR   "Andreas Tønnesen"
#define MOD_DESC PLUGIN_NAME " " PLUGIN_VERSION " by " PLUGIN_AUTHOR


static void __attribute__ ((constructor)) 
my_init(void);

static void __attribute__ ((destructor)) 
my_fini(void);


/*
 * Defines the version of the plugin interface that is used
 * THIS IS NOT THE VERSION OF YOUR PLUGIN!
 * Do not alter unless you know what you are doing!
 */
int 
olsrd_plugin_interface_version()
{
  return OLSRD_PLUGIN_INTERFACE_VERSION;
}



/**
 *Constructor
 */
static void
my_init()
{
  /* Print plugin info to stdout */
  /* We cannot use olsr_printf yet! */
  printf("%s\n", MOD_DESC);
  printf("[ENC]Accepted parameter pairs: (\"Keyfile\" <FILENAME>)\n"); 
}

/**
 *Destructor
 */
static void
my_fini()
{

  /* Calls the destruction function
   * olsr_plugin_exit()
   * This function should be present in your
   * sourcefile and all data destruction
   * should happen there - NOT HERE!
   */
  secure_plugin_exit();
}


int
olsrd_plugin_register_param(char *key, char *value)
{
  if(!strcmp(key, "Keyfile"))
    {
      strncpy(keyfile, value, FILENAME_MAX);
    }

  return 1;
}


int
olsrd_plugin_init() {
  /* Calls the initialization function
   * olsr_plugin_init()
   * This function should be present in your
   * sourcefile and all data initialization
   * should happen there - NOT HERE!
   */
  if(!secure_plugin_init())
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
