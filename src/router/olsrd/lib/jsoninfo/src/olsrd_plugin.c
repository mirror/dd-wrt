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

#include "olsrd_plugin.h"
#include "info/olsrd_info.h"
#include "olsrd_jsoninfo.h"
#include "olsr.h"
#include "builddata.h"

#define PLUGIN_NAME              "JSONINFO"
#define PLUGIN_TITLE             "OLSRD jsoninfo plugin"
#define PLUGIN_INTERFACE_VERSION 5

info_plugin_functions_t functions;
info_plugin_config_t config;
char uuidfile[FILENAME_MAX];
bool pretty = false;

static void my_init(void) __attribute__ ((constructor));
static void my_fini(void) __attribute__ ((destructor));

/**
 *Constructor
 */
static void my_init(void) {
  /* Print plugin info to stdout */
  olsr_printf(0, "%s (%s)\n", PLUGIN_TITLE, git_descriptor);

  info_plugin_config_init(&config, 9090);
  memset(uuidfile, 0, sizeof(uuidfile));
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

  functions.supportsCompositeCommands = true;
  functions.init = plugin_init;
  functions.supported_commands_mask = get_supported_commands_mask;
  functions.is_command = isCommand;
  functions.cache_timeout = cache_timeout_generic;
  functions.determine_mime_type = determine_mime_type;
  functions.output_start = output_start;
  functions.output_end = output_end;
  functions.output_error = output_error;

  functions.neighbors = ipc_print_neighbors;
  functions.links = ipc_print_links;
  functions.routes = ipc_print_routes;
  functions.topology = ipc_print_topology;
  functions.hna = ipc_print_hna;
  functions.mid = ipc_print_mid;
  functions.gateways = ipc_print_gateways;
  functions.sgw = ipc_print_sgw;
  functions.pudPosition = ipc_print_pud_position;
  functions.version = ipc_print_version;
  functions.olsrd_conf = ipc_print_olsrd_conf;
  functions.interfaces = ipc_print_interfaces;
  functions.twohop = ipc_print_twohop;
  functions.config = ipc_print_config;
  functions.plugins = ipc_print_plugins;

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
        { .name = "uuidfile", .set_plugin_parameter = &set_plugin_string, .data = uuidfile, .addon = { .ui = FILENAME_MAX - 1 } }, //
        { .name = "pretty", .set_plugin_parameter = set_plugin_boolean, .data = &pretty, .addon = { .pc = NULL } } //
    };

void olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size) {
  *params = plugin_parameters;
  *size = sizeof(plugin_parameters) / sizeof(*plugin_parameters);
}

/*
 * Local Variables:
 * mode: c
 * style: linux
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
