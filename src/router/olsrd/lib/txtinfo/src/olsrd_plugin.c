
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org)
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

/*
 * Dynamic linked library for the olsr.org olsr daemon
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "olsrd_plugin.h"
#include "olsrd_txtinfo.h"
#include "defs.h"

#define PLUGIN_NAME    "OLSRD txtinfo plugin"
#define PLUGIN_VERSION "0.1"
#define PLUGIN_AUTHOR   "Lorenz Schori"
#define MOD_DESC PLUGIN_NAME " " PLUGIN_VERSION " by " PLUGIN_AUTHOR
#define PLUGIN_INTERFACE_VERSION 5

union olsr_ip_addr ipc_accept_ip;
int ipc_port;
int nompr;

static void my_init(void) __attribute__ ((constructor));
static void my_fini(void) __attribute__ ((destructor));

/**
 *Constructor
 */
static void
my_init(void)
{
  /* Print plugin info to stdout */
  printf("%s\n", MOD_DESC);

  /* defaults for parameters */
  ipc_port = 2006;
  if (olsr_cnf->ip_version == AF_INET) {
    ipc_accept_ip.v4.s_addr = htonl(INADDR_LOOPBACK);
  } else {
    ipc_accept_ip.v6 = in6addr_loopback;
  }

  /* highlite neighbours by default */
  nompr = 0;
}

/**
 *Destructor
 */
static void
my_fini(void)
{
  /* Calls the destruction function
   * olsr_plugin_exit()
   * This function should be present in your
   * sourcefile and all data destruction
   * should happen there - NOT HERE!
   */
  olsr_plugin_exit();
}

int
olsrd_plugin_interface_version(void)
{
  return PLUGIN_INTERFACE_VERSION;
}

static const struct olsrd_plugin_parameters plugin_parameters[] = {
  {.name = "port",.set_plugin_parameter = &set_plugin_port,.data = &ipc_port},
  {.name = "accept",.set_plugin_parameter = &set_plugin_ipaddress,.data = &ipc_accept_ip},
};

void
olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size)
{
  *params = plugin_parameters;
  *size = sizeof(plugin_parameters) / sizeof(*plugin_parameters);
}

/*
 * Local Variables:
 * mode: c
 * style: linux
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
