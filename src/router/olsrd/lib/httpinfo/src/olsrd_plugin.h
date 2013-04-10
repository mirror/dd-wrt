
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

#ifndef _OLSRD_PLUGIN_DEFS
#define _OLSRD_PLUGIN_DEFS

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

#include "olsr_types.h"
#include "../../../src/olsrd_plugin.h"
#include "neighbor_table.h"
#include "two_hop_neighbor_table.h"
#include "tc_set.h"
#include "hna_set.h"
#include "mid_set.h"
#include "mpr_selector_set.h"
#include "routing_table.h"

#define PLUGIN_NAME    "Httpinfo olsrd plugin"
#define PLUGIN_VERSION "0.1"
#define PLUGIN_AUTHOR   "Andreas Tonnesen"
#define MOD_DESC PLUGIN_NAME " " PLUGIN_VERSION " by " PLUGIN_AUTHOR
#define PLUGIN_INTERFACE_VERSION 5

extern int http_port;
extern int resolve_ip_addresses;

/* Allowed hosts stuff */

struct allowed_net {
  struct olsr_ip_prefix prefix;
  struct allowed_net *next;
};

extern struct allowed_net *allowed_nets;

/****************************************************************************
 *                Functions that the plugin MUST provide                    *
 ****************************************************************************/

/* Initialization function */
int olsrd_plugin_init(void);

void olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size);

int olsrd_plugin_interface_version(void);

#endif /* _OLSRD_PLUGIN_DEFS */

/*
 * Local Variables:
 * mode: c
 * style: linux
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
