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
#include "plugin_util.h"
#include "olsr.h"
#include "defs.h"
#include "scheduler.h"
#include "olsr_cookie.h"
#include "builddata.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define PLUGIN_NAME              "OLSRD watchdog plugin"
#define PLUGIN_INTERFACE_VERSION 5

static struct olsr_cookie_info *watchdog_timer_cookie;

static char watchdog_filename[FILENAME_MAX + 1] = "/tmp/olsr.watchdog";
static int watchdog_interval = 5;


static void my_init(void) __attribute__ ((constructor));
static void my_fini(void) __attribute__ ((destructor));

/**
 *Constructor
 */
static void
my_init(void)
{
  /* Print plugin info to stdout */
  olsr_printf(0, "%s (%s)\n", PLUGIN_NAME, git_descriptor);
}

/**
 *Destructor
 */
static void
my_fini(void)
{
}

/**
 * Plugin interface version
 * Used by main olsrd to check plugin interface version
 */
int
olsrd_plugin_interface_version(void)
{
  return PLUGIN_INTERFACE_VERSION;
}


static int
set_watchdog_file(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  strncpy(watchdog_filename, value, FILENAME_MAX);
  return 0;
}

static int
set_watchdog_interval(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon
                      __attribute__ ((unused)))
{
  watchdog_interval = atoi(value);
  return 0;
}

/**
 * Register parameters from config file
 * Called for all plugin parameters
 */
static const struct olsrd_plugin_parameters plugin_parameters[] = {
  {.name = "file",.set_plugin_parameter = &set_watchdog_file,.data = NULL},
  {.name = "interval",.set_plugin_parameter = &set_watchdog_interval,.data = NULL},
};

void
olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size)
{
  *params = plugin_parameters;
  *size = ARRAYSIZE(plugin_parameters);
}

static void
olsr_watchdog_write_alivefile(void *foo __attribute__ ((unused)))
{
  FILE *file = fopen(watchdog_filename, "w");
  if (file == NULL) {
    OLSR_PRINTF(3, "Error, cannot write watchdog alivefile");
  } else {
    fprintf(file, "%ld\n", (long)time(NULL));
    fflush(file);
    fclose(file);
  }
}

/**
 * Initialize plugin
 * Called after all parameters are passed
 */
int
olsrd_plugin_init(void)
{
  /* create the cookie */
  watchdog_timer_cookie = olsr_alloc_cookie("Watchdog: write alive-file", OLSR_COOKIE_TYPE_TIMER);

  /* Register the GW check */
  olsr_start_timer(watchdog_interval * MSEC_PER_SEC, 0, OLSR_TIMER_PERIODIC,
                   &olsr_watchdog_write_alivefile, NULL, watchdog_timer_cookie);

  return 1;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
