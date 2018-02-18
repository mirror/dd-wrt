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
 * Much of the ACPI code is taken from Florian Schaefers
 * Acpi-Power Enlightenment epplet
 */

#ifdef __linux__

#include "apm.h"
#include "defs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* APM related stuff */

#define APM_PROC "/proc/apm"

struct linux_apm_info {
  char driver_version[10];
  int apm_version_major;
  int apm_version_minor;
  int apm_flags;
  int ac_line_status;
  int battery_status;
  int battery_flags;
  int battery_percentage;
  int battery_time;
  int using_minutes;
};

/* ACPI related stuff */
static const char *const acpi_info[] = {
  "/proc/acpi/battery/0/info",
  "/proc/acpi/battery/1/info",
  "/proc/acpi/battery/BATA/info",
  "/proc/acpi/battery/BAT0/info",
  "/proc/acpi/battery/BAT1/info"
};

static const char *const acpi_state[] = {
  "/proc/acpi/battery/0/status",
  "/proc/acpi/battery/1/status",
  "/proc/acpi/battery/BATA/state",
  "/proc/acpi/battery/BAT0/state",
  "/proc/acpi/battery/BAT1/state"
};

#define ACPI_BT_CNT  ARRAYSIZE(acpi_state)

static const char *const acpi_ac[] = {
  "/proc/acpi/ac_adapter/0/status",
  "/proc/acpi/ac_adapter/AC/state",
  "/proc/acpi/ac_adapter/ACAD/state"
};

#define ACPI_AC_CNT  ARRAYSIZE(acpi_ac)

#define USE_APM    1
#define USE_ACPI   2

static int method;

static int fd_index;

static int ac_power_on;

/* Prototypes */

static int apm_read_apm(struct olsr_apm_info *);

static int apm_read_acpi(struct olsr_apm_info *);

static int acpi_probe(void);

int
apm_init(void)
{
  struct olsr_apm_info ainfo;

  method = -1;
  OLSR_PRINTF(3, "Initializing APM\n");

  if ((((fd_index = acpi_probe()) >= 0) || ac_power_on) && apm_read_acpi(&ainfo))
    method = USE_ACPI;
  else if (apm_read_apm(&ainfo))
    method = USE_APM;

  if (method != -1)
    apm_printinfo(&ainfo);

  return method;
}

void
apm_printinfo(struct olsr_apm_info *ainfo)
{
  OLSR_PRINTF(5, "APM info:\n\tAC status %d\n\tBattery percentage %d%%\n\tBattery time left %d mins\n\n", ainfo->ac_line_status,
              ainfo->battery_percentage, ainfo->battery_time_left);

  ainfo = NULL;                 /* squelch compiler warnings */
}

int
apm_read(struct olsr_apm_info *ainfo)
{
  switch (method) {
  case USE_APM:
    return apm_read_apm(ainfo);
  case USE_ACPI:
    return apm_read_acpi(ainfo);
  default:
    break;
  }
  return 0;
}

static int
apm_read_apm(struct olsr_apm_info *ainfo)
{
  char buffer[100];
  char units[10];
  FILE *apm_procfile;
  struct linux_apm_info lainfo;

  /* Open procfile */
  if ((apm_procfile = fopen(APM_PROC, "r")) == NULL)
    return 0;

  if (fgets(buffer, sizeof(buffer), apm_procfile) == NULL) {
    fclose(apm_procfile);
    /* Try re-opening the file */
    if ((apm_procfile = fopen(APM_PROC, "r")) == NULL)
      return 0;

    if (fgets(buffer, sizeof(buffer), apm_procfile) == NULL) {
      /* Giving up */
      fprintf(stderr, "OLSRD: Could not read APM info - setting willingness to default");
      fclose(apm_procfile);
      return 0;
    }
  }
  fclose(apm_procfile);

  //printf("READ: %s\n", buffer);

  /* Get the info */
  sscanf(buffer, "%10s %d.%d %x %x %x %x %d%% %d %10s\n", lainfo.driver_version, &lainfo.apm_version_major, &lainfo.apm_version_minor,
         &lainfo.apm_flags, &lainfo.ac_line_status, &lainfo.battery_status, &lainfo.battery_flags, &lainfo.battery_percentage,
         &lainfo.battery_time, units);

  lainfo.using_minutes = strncmp(units, "min", 3) ? 0 : 1;

  /*
   * Should take care of old APM type info here
   */

  /*
   * Fix possible percentage error
   */
  if (lainfo.battery_percentage > 100)
    lainfo.battery_percentage = -1;

  /* Fill the provided struct */

  if (lainfo.ac_line_status)
    ainfo->ac_line_status = OLSR_AC_POWERED;
  else
    ainfo->ac_line_status = OLSR_BATTERY_POWERED;

  ainfo->battery_percentage = lainfo.battery_percentage;
  ainfo->battery_time_left = lainfo.battery_time;

  return 1;
}

static int
apm_read_acpi(struct olsr_apm_info *ainfo)
{
  FILE *fd;
  int bat_max = 5000;                  /* Find some sane value */
  int bat_val = 0;

  /* reporbe in case ac status changed */
  fd_index = acpi_probe();

  /* No battery was found */
  if (fd_index < 0) {
    /* but we have ac */
    if (ac_power_on) {
      ainfo->ac_line_status = OLSR_AC_POWERED;

      ainfo->battery_percentage = -1;

      return 1;
    }

    /* not enough info */
    return 0;
  }

  /* Get maxvalue */
  if ((fd = fopen(acpi_info[fd_index], "r")) == NULL)
    return 0;

  for (;;) {
    char s1[32], s2[32], s3[32], s4[32], inbuff[127];
    if (fgets(inbuff, sizeof(inbuff), fd) == NULL)
      break;

    sscanf(inbuff, "%32s %32s %32s %32s", s1, s2, s3, s4);
    if (!strcasecmp(s2, "full"))
      bat_max = atoi(s4);
  }
  fclose(fd);

  if ((fd = fopen(acpi_state[fd_index], "r")) == NULL)
    return 0;

  /* Extract battery status */
  for (;;) {
    char s1[32], s2[32], s3[32], s4[32], inbuff[127];
    if (fgets(inbuff, sizeof(inbuff), fd) == NULL)
      break;
    sscanf(inbuff, "%32s %32s %32s %32s", s1, s2, s3, s4);

    /* find remaining juice */
    if (!strcasecmp(s1, "Remaining"))
      bat_val = atoi(s3);
  }
  fclose(fd);

  ainfo->ac_line_status = ac_power_on ? OLSR_AC_POWERED : OLSR_BATTERY_POWERED;

  /* sanitise ACPI battery data */
  bat_max = abs(bat_max);
  bat_val = abs(bat_val);
  if (bat_val > bat_max) {
    bat_val = bat_max;
  }

  if (bat_max == 0) {
    /* protection against stupid acpi data */
    ainfo->battery_percentage = 0;
  }
  else {
    ainfo->battery_percentage = (bat_val >= bat_max) ? 100 : (bat_val * 100 / bat_max);
  }

  return 1;
}

static int
acpi_probe(void)
{
  unsigned int i;

  /* First check for AC power */
  ac_power_on = 0;

  for (i = 0; i < ACPI_AC_CNT; i++) {
    char s1[32], s2[32];
    int rc;
    FILE *fd = fopen(acpi_ac[i], "r");

    /* Try opening the info file */
    if (fd == NULL)
      continue;

    /* Extract info */
    rc = fscanf(fd, "%32s %32s", s1, s2);

    /* Close info entry */
    fclose(fd);

    if (rc < 2)
      continue;

    /* Running on AC power */
    if (!strcasecmp(s2, "on-line")) {

      /* ac power enabled */
      ac_power_on = 1;

      break;
    }
  }

  /* Only checking the first found battery entry... */
  for (i = 0; i < ACPI_BT_CNT; i++) {
    char s1[32], s2[32];
    int rc;
    FILE *fd = fopen(acpi_info[i], "r");

    /* Try opening the info file */
    if (fd == NULL)
      continue;

    /* Extract info */
    rc = fscanf(fd, "%32s %32s", s1, s2);

    /* Close info entry */
    fclose(fd);

    if (rc < 2)
      continue;

    /* Check if battery is present */
    if ((!strcasecmp(s1, "present:")) && (!strcasecmp(s2, "no")))
      continue;

    /* Open the corresponding state file */
    if ((fd = fopen(acpi_state[i], "r")) == NULL)
      continue;

    fclose(fd);
    return i;
  }

  /* No battery found */
  return -1;
}
#endif /* __linux__ */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
