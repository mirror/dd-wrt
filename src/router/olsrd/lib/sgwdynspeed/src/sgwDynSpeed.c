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

#include "sgwDynSpeed.h"

/* Plugin includes */
#include "speedFile.h"
#include "configuration.h"

/* OLSRD includes */
#include "olsr.h"
#include "olsr_cookie.h"
#include "scheduler.h"
#include "log.h"
#include "gateway.h"

/* System includes */

/**
 Report a plugin error.

 @param useErrno
 when true then errno is used in the error message; the error reason is also
 reported.
 @param format
 a pointer to the format string
 @param ...
 arguments to the format string
 */
void sgwDynSpeedError(bool useErrno, const char *format, ...) {
	char strDesc[256];
	char *stringErr = NULL;

	if (useErrno) {
		stringErr = strerror(errno);
	}

	if ((format == NULL) || (*format == '\0')) {
		if (useErrno) {
			olsr_printf(0, "%s: %s\n", SGWDYNSPEED_PLUGIN_ABBR, stringErr);
		} else {
			olsr_printf(0, "%s: Unknown error\n", SGWDYNSPEED_PLUGIN_ABBR);
		}
	} else {
		va_list arglist;

		va_start(arglist, format);
		vsnprintf(strDesc, sizeof(strDesc), format, arglist);
		va_end(arglist);

		strDesc[sizeof(strDesc) - 1] = '\0'; /* Ensures null termination */

		if (useErrno) {
			olsr_printf(0, "%s: %s: %s\n", SGWDYNSPEED_PLUGIN_ABBR, strDesc, stringErr);
		} else {
			olsr_printf(0, "%s: %s\n", SGWDYNSPEED_PLUGIN_ABBR, strDesc);
		}
	}
}

/**
 * Timer callback that reads the smart gateway speed file
 */
static void smartgw_read_speed_file(void *context __attribute__ ((unused))) {
	readSpeedFile(getSpeedFile());
	return;
}

/** The timer cookie, used to trace back the originator in debug */
static struct olsr_cookie_info *smartgw_speed_file_timer_cookie = NULL;

/** The timer */
static struct timer_entry * smartgw_speed_file_timer = NULL;

/**
 Initialise the plugin: check the configuration, initialise the NMEA parser,
 create network interface sockets, hookup the plugin to OLSR and setup data
 that can be setup in advance.

 @return
 - false upon failure
 - true otherwise
 */bool initSgwDynSpeed(void) {
	char * speedFile;

	if (multi_gateway_mode()) {
	  olsr_syslog(OLSR_LOG_ERR, "sgwDynSpeed plugin can't be enabled in multi-smart-gateway mode");
	  return false;
	}

	if (!startSpeedFile()) {
		return false;
	}

	speedFile = getSpeedFile();
	if (speedFile) {
		readSpeedFile(speedFile);

		if (smartgw_speed_file_timer_cookie == NULL) {
			smartgw_speed_file_timer_cookie = olsr_alloc_cookie("smartgw speed file", OLSR_COOKIE_TYPE_TIMER);
			if (smartgw_speed_file_timer_cookie == NULL) {
				sgwDynSpeedError(false, "Could not allocate smart gateway speed file cookie");
				return false;
			}
		}
		if (smartgw_speed_file_timer == NULL) {
			smartgw_speed_file_timer = olsr_start_timer(getSpeedFilePeriod(), 0, OLSR_TIMER_PERIODIC,
					&smartgw_read_speed_file, NULL, smartgw_speed_file_timer_cookie);
			if (smartgw_speed_file_timer == NULL) {
				sgwDynSpeedError(false, "Could not start smart gateway speed file timer");
				return false;
			}
		}
	}

	return true;
}

 /**
  * stop the plugin, free resources
  */
void stopSgwDynSpeed(void) {
	if (smartgw_speed_file_timer != NULL) {
		olsr_stop_timer(smartgw_speed_file_timer);
		smartgw_speed_file_timer = NULL;
	}
	if (smartgw_speed_file_timer_cookie != NULL) {
		olsr_free_cookie(smartgw_speed_file_timer_cookie);
		smartgw_speed_file_timer_cookie = NULL;
	}
	stopSpeedFile();
}
