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

/*
 * Dynamic linked library for the olsr.org olsr daemon
 */

#include "olsr.h"
#include "olsrd_plugin.h"
#include "olsr_cfg.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#ifndef _WIN32
#include <arpa/nameser.h>
#endif /* _WIN32 */

#include "olsrd_httpinfo.h"
#include "olsr.h"
#include "builddata.h"

int http_port = 0;
bool resolve_ip_addresses = false;
struct allowed_net *allowed_nets = NULL;
union olsr_ip_addr httpinfo_listen_ip;

static void my_init(void) __attribute__ ((constructor));
static void my_fini(void) __attribute__ ((destructor));

static int add_plugin_access(const char *value, void *data, set_plugin_parameter_addon);

/*
 * Defines the version of the plugin interface that is used
 * THIS IS NOT THE VERSION OF YOUR PLUGIN!
 * Do not alter unless you know what you are doing!
 */
int
olsrd_plugin_interface_version(void)
{
  return PLUGIN_INTERFACE_VERSION;
}

/**
 *Constructor
 */
static void
my_init(void)
{
  /* Print plugin info to stdout */
  olsr_printf(0, "%s (%s)\n", PLUGIN_NAME, git_descriptor);

  httpinfo_listen_ip.v4.s_addr = htonl(INADDR_ANY);
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

static const struct olsrd_plugin_parameters plugin_parameters[] = {
  {.name = "port",.set_plugin_parameter = &set_plugin_port,.data = &http_port},
  {.name = "host",.set_plugin_parameter = &add_plugin_access,.data = &allowed_nets},
  {.name = "net",.set_plugin_parameter = &add_plugin_access,.data = &allowed_nets},
  {.name = "resolve",.set_plugin_parameter = &set_plugin_boolean,.data = &resolve_ip_addresses},
};

void
olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size)
{
  *params = plugin_parameters;
  *size = sizeof(plugin_parameters) / sizeof(*plugin_parameters);
}

static int
add_plugin_access(const char *value, void *data, set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  struct olsr_ip_prefix prefix;
  struct allowed_net **my_allowed_nets = data;
  struct allowed_net *an;

  if (olsr_string_to_prefix(olsr_cnf->ip_version, &prefix, value)) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "HTTPINFO: unknown access restriction parameter: %s", value);
    olsr_exit(buf, EXIT_FAILURE);
  }

  an = olsr_malloc(sizeof(*an), __func__);
  if (an == NULL) {
    olsr_exit("HTTPINFO: register param net out of memory", EXIT_FAILURE);
  }

  an->prefix = prefix;
  an->next = *my_allowed_nets;
  *my_allowed_nets = an;
  return 0;
}
/*
 * Local Variables:
 * mode: c
 * style: linux
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
