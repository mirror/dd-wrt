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

#include "olsrd_plugin.h"
#include "info/olsrd_info.h"
#include "olsrd_netjson.h"
#include "olsr.h"
#include "builddata.h"

#define PLUGIN_NAME              "NETJSON"
#define PLUGIN_TITLE             "OLSRD netjson plugin"
#define PLUGIN_INTERFACE_VERSION 5

info_plugin_functions_t functions;
info_plugin_config_t config;
bool pretty = false;

static void my_init(void) __attribute__ ((constructor));
static void my_fini(void) __attribute__ ((destructor));

/**
 *Constructor
 */
static void my_init(void) {
  /* Print plugin info to stdout */
  olsr_printf(0, "%s (%s)\n", PLUGIN_TITLE, git_descriptor);

  info_plugin_config_init(&config, 2005);
}

/**
 *Destructor
 */
static void my_fini(void) {
  /* Calls the destruction function
   * olsr_plugin_exit()
   * This function should be present in your
   * sourcefile and all data destruction
   * should happen there - NOT HERE!
   */
  olsr_plugin_exit();
}

/**
 *Do initialization here
 *
 *This function is called by the my_init
 *function in uolsrd_plugin.c
 */
int olsrd_plugin_init(void) {
  memset(&functions, 0, sizeof(functions));

  functions.supportsCompositeCommands = false;
  functions.supported_commands_mask = get_supported_commands_mask;
  functions.is_command = isCommand;
  functions.cache_timeout = cache_timeout_generic;
  functions.determine_mime_type = determine_mime_type;
  functions.output_start = output_start;
  functions.output_end = output_end;
  functions.output_error = output_error;

  functions.networkRoutes = ipc_print_network_routes;
  functions.networkGraph = ipc_print_network_graph;
  functions.networkCollection = ipc_print_network_collection;

  return info_plugin_init(PLUGIN_NAME, &functions, &config);
}

/**
 * destructor - called at unload
 */
void olsr_plugin_exit(void) {
  info_plugin_exit();
}

int olsrd_plugin_interface_version(void) {
  return PLUGIN_INTERFACE_VERSION;
}

static const struct olsrd_plugin_parameters plugin_parameters[] = { //
    //
        INFO_PLUGIN_CONFIG_PLUGIN_PARAMETERS(config), //
        { .name = "pretty", .set_plugin_parameter = &set_plugin_boolean, .data = &pretty, .addon = { .pc = NULL } } //
    };

void olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size) {
  *params = plugin_parameters;
  *size = sizeof(plugin_parameters) / sizeof(*plugin_parameters);
}
