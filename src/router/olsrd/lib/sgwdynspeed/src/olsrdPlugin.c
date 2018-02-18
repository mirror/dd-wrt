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

#include "olsrdPlugin.h"

/* Plugin includes */
#include "sgwDynSpeed.h"

/* OLSRD includes */
#include "olsr.h"
#include "builddata.h"

/* System includes */
#include <stdbool.h>

/*
 * OLSR Entrypoints
 */

/**
 OLSR entrypoint to initialise the plugin.

 @return
 - 0 on fail
 - 1 on success
 */
int olsrd_plugin_init(void) {
	bool retval = initSgwDynSpeed();
	return (retval ? 1 : 0);
}

/**
 OLSR entrypoint to retrieve the interface version supported by the plugin.

 @return
 the supported interface version
 */
int olsrd_plugin_interface_version(void) {
	return SGWDYNSPEED_PLUGIN_INTERFACE_VERSION;
}

/**
 OLSR entrypoint to retrieve the plugin parameter configuration.

 @param params
 a pointer to a variable in which the function stores a pointer to the
 plugin parameter configuration
 @param size
 a pointer to a variable in which the function stores the number of rows of the
 plugin parameter configuration
 */
void olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size) {
	*params = &plugin_parameters[0];
	*size = ARRAYSIZE(plugin_parameters);
}

/*
 * Shared Library Entrypoints
 */

/**
 Shared library entrypoint declaration for initialisation
 */
static void __attribute__ ((constructor)) sgwDynSpeed_init(void);

/**
 Shared library entrypoint declaration for destruction
 */
static void __attribute__ ((destructor)) sgwDynSpeed_fini(void);

/**
 Shared library entrypoint for initialisation
 */
static void sgwDynSpeed_init(void) {
  /* Print plugin info to stdout */
  olsr_printf(0, "%s (%s)\n", SGWDYNSPEED_PLUGIN_NAME_LONG, git_descriptor);
}

/**
 Shared library entrypoint for destruction
 */
static void sgwDynSpeed_fini(void) {
	stopSgwDynSpeed();
}
