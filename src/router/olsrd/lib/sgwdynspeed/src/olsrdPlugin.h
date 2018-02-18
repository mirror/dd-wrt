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

#ifndef _SGWDYNSPEED_OLSRD_PLUGIN_H_
#define _SGWDYNSPEED_OLSRD_PLUGIN_H_

/* Plugin includes */
#include "configuration.h"

/* OLSRD includes */
#include "olsrd_plugin.h"

/* System includes */
#include <stddef.h>

/** The interface version supported by the plugin */
#define SGWDYNSPEED_PLUGIN_INTERFACE_VERSION	5

/**
 The plugin parameter configuration, containing the parameter names, pointers
 to their setters, and an optional data pointer that is given to the setter
 when it is called.
 */
static const struct olsrd_plugin_parameters plugin_parameters[] = {
	{	.name = SGWDYNSPEED_SPEEDFILE_NAME, .set_plugin_parameter = &setSpeedFile, .data = NULL},
	{	.name = SGWDYNSPEED_SPEEDFILEPERIOD_NAME, .set_plugin_parameter = &setSpeedFilePeriod, .data = NULL}
};

#endif /* _SGWDYNSPEED_OLSRD_PLUGIN_H_ */
