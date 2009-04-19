
/*
 * Copyright (c) 2005, Bruno Randolf <bruno.randolf@4g-systems.biz>
 * Copyright (c) 2004, Andreas Tonnesen(andreto-at-olsr.org)
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

/*
 * Example plugin for olsrd.org OLSR daemon
 * Only the bare minimum
 */

#ifndef _OLSRD_PLUGIN
#define _OLSRD_PLUGIN

/* Define the most recent version */
#define MOST_RECENT_PLUGIN_INTERFACE_VERSION		5
#define LAST_SUPPORTED_PLUGIN_INTERFACE_VERSION		4

/****************************************************************************
 *                Functions that the plugin MUST provide                    *
 ****************************************************************************/
#if 1

/* We hide them from the compiler here to allow the plugins itself to declare them
 * as they also implement them if we activate -Wredundant-decls.
 * Normally we leave it seen so that we enforce a check by the compiler if they are
 * identical.
 */

/**
 * Plugin interface version
 * Used by main olsrd to check plugin interface version
 */
int olsrd_plugin_interface_version(void);

/**
 * Initialize plugin
 * Called after all parameters are passed
 */
int olsrd_plugin_init(void);

/* Interface version 4 */

/**
 * Register parameters from config file
 * Called for all plugin parameters
 */
int olsrd_plugin_register_param(char *key, char *value);

/* Interface version 5 */

typedef union {
  unsigned int ui;
  char *pc;
} set_plugin_parameter_addon;

typedef int set_plugin_parameter(const char *value, void *data, set_plugin_parameter_addon addon);

struct olsrd_plugin_parameters {
  const char *name;
  set_plugin_parameter *set_plugin_parameter;
  void *data;
  set_plugin_parameter_addon addon;
};

/**
 * Delivers the (address of the) table and the size of the parameter description
 */
void olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size);

#endif

#endif

/*
 * Local Variables:
 * mode: c
 * style: linux
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
