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

#ifndef _SGWDYNSPEED_CONFIGURATION_H_
#define _SGWDYNSPEED_CONFIGURATION_H_

/* Plugin includes */

/* OLSR includes */
#include "olsrd_plugin.h"

/* System includes */

/*
 * speedFile
 */

/** The name of the speedFile plugin parameter */
#define SGWDYNSPEED_SPEEDFILE_NAME				"speedFile"

char * getSpeedFile(void);
int setSpeedFile(const char *value, void *data, set_plugin_parameter_addon addon);

/*
 * speedFilePeriod
 */

/** The name of the speedFilePeriod plugin parameter */
#define SGWDYNSPEED_SPEEDFILEPERIOD_NAME		"speedFilePeriod"

/** The default value of the speedFilePeriod plugin parameter */
#define SGWDYNSPEED_SPEEDFILEPERIOD_DEFAULT		((unsigned long long)10000)

/** The minimal value of the speedFilePeriod plugin parameter */
#define SGWDYNSPEED_SPEEDFILEPERIOD_MIN			((unsigned long long)1000)

/** The maximal value of the speedFilePeriod plugin parameter */
#define SGWDYNSPEED_SPEEDFILEPERIOD_MAX			((unsigned long long)320000000)

unsigned long long getSpeedFilePeriod(void);
int setSpeedFilePeriod(const char *value, void *data, set_plugin_parameter_addon addon);

#endif /* _SGWDYNSPEED_CONFIGURATION_H_ */
