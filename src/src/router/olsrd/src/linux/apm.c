/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
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
 * $Id: apm.c,v 1.14 2005/04/07 18:22:43 kattemat Exp $
 */

/*
 * Much of the ACPI code is taken from Florian Schaefers
 * Acpi-Power Enlightenment epplet
 */

#include "apm.h"
#include "defs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* APM related stuff */

#define APM_PROC "/proc/apm"

struct linux_apm_info
{
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

#define ACPI_PROC "/proc/acpi/info"

const static char * acpi_info[] = 
  {
    "/proc/acpi/battery/0/info",
    "/proc/acpi/battery/1/info",
    "/proc/acpi/battery/BATA/info",
    "/proc/acpi/battery/BAT0/info",
    "/proc/acpi/battery/BAT1/info" 
  };

const static char * acpi_state[] =
  {    
    "/proc/acpi/battery/0/status",
    "/proc/acpi/battery/1/status",
    "/proc/acpi/battery/BATA/state",
    "/proc/acpi/battery/BAT0/state",
    "/proc/acpi/battery/BAT1/state" 
  };


#define ACPI_BT_CNT  (sizeof(acpi_state) / sizeof(char *))


const static char * acpi_ac[] = 
  {    
    "/proc/acpi/ac_adapter/0/status",
    "/proc/acpi/ac_adapter/AC/state",
    "/proc/acpi/ac_adapter/ACAD/state" 
  };

#define ACPI_AC_CNT  (sizeof(acpi_ac) / sizeof(char *))


#define USE_APM    1
#define USE_ACPI   2

static int method, fd_index;

/* Prototypes */

static int
apm_read_apm(struct olsr_apm_info *);

static int
apm_read_acpi(struct olsr_apm_info *);

static int
acpi_probe(void);


int 
apm_init()
{
  struct olsr_apm_info ainfo;

  method = -1;
  OLSR_PRINTF(3, "Initializing APM\n")

  if(((fd_index = acpi_probe()) >= 0) && apm_read_acpi(&ainfo))
    method = USE_ACPI;
  else if(apm_read_apm(&ainfo))
    method = USE_APM;
  
  if(method)
    apm_printinfo(&ainfo);

  return method;
}

void
apm_printinfo(struct olsr_apm_info *ainfo)
{
  OLSR_PRINTF(5, "APM info:\n\tAC status %d\n\tBattery percentage %d%%\n\tBattery time left %d mins\n\n",
	      ainfo->ac_line_status,
	      ainfo->battery_percentage,
	      ainfo->battery_time_left)

  return;
}



int
apm_read(struct olsr_apm_info *ainfo)
{

  switch(method)
    {
    case(USE_APM):
      return apm_read_apm(ainfo);
      break;
    case(USE_ACPI):
      return apm_read_acpi(ainfo);
      break;
    default:
      return 0;
      break;
    }

  return 1;
}





static int
apm_read_apm(struct olsr_apm_info *ainfo)
{
  char buffer[100];
  char units[10];
  FILE *apm_procfile;
  struct linux_apm_info lainfo;

  /* Open procfile */
  if((apm_procfile = fopen(APM_PROC, "r")) == NULL)
    return 0;


  fgets(buffer, sizeof(buffer) - 1, apm_procfile);
  if(buffer == NULL)
    {
      /* Try re-opening the file */
      if((apm_procfile = fopen(APM_PROC, "r")) < 0)
	return 0;
      fgets(buffer, sizeof(buffer) - 1, apm_procfile);
      if(buffer == NULL)
	{
	  /* Giving up */
	  fprintf(stderr, "OLSRD: Could not read APM info - setting willingness to default");
	  fclose(apm_procfile);
	  return 0;
	}
    }

  buffer[sizeof(buffer) - 1] = '\0';

  //printf("READ: %s\n", buffer);

  /* Get the info */
  sscanf(buffer, "%s %d.%d %x %x %x %x %d%% %d %s\n",
	 lainfo.driver_version,
	 &lainfo.apm_version_major,
	 &lainfo.apm_version_minor,
	 &lainfo.apm_flags,
	 &lainfo.ac_line_status,
	 &lainfo.battery_status,
	 &lainfo.battery_flags,
	 &lainfo.battery_percentage,
	 &lainfo.battery_time,
	 units);

  lainfo.using_minutes = !strncmp(units, "min", 3) ? 1 : 0;

  /*
   * Should take care of old APM type info here
   */

  /*
   * Fix possible percentage error
   */
  if(lainfo.battery_percentage > 100)
    lainfo.battery_percentage = -1;

  /* Fill the provided struct */

  if(lainfo.ac_line_status)
    ainfo->ac_line_status = OLSR_AC_POWERED;
  else
    ainfo->ac_line_status = OLSR_BATTERY_POWERED;
  
  ainfo->battery_percentage = lainfo.battery_percentage;
  ainfo->battery_time_left = lainfo.battery_time;
  
  fclose(apm_procfile);

  return 1;
}



static int
apm_read_acpi(struct olsr_apm_info *ainfo)
{
  FILE *fd;
  char s1[32], s2[32], s3[32], s4[32], inbuff[127];
  int bat_max = 5000; /* Find some sane value */
  int bat_val = 0;
  
  printf("READING ACPI\n");


  if(fd_index < 0)
    {
      /* No battery was found or AC power was detected */
      ainfo->ac_line_status = OLSR_AC_POWERED;
      ainfo->battery_percentage = 100;
      return 1;
    }


  /* Get maxvalue */
  if((fd = fopen(acpi_info[fd_index], "r")) == NULL) 
    return 0;

  fgets(inbuff, 127, fd);
  while(!feof(fd)) 
    {
      sscanf(inbuff, "%s %s %s %s", s1, s2, s3, s4);
	
      if (!strcasecmp(s2, "full")) 
	{
	  bat_max = atoi(s4);
	}

      fgets(inbuff, 127, fd);
    }
  fclose(fd);


  if((fd = fopen(acpi_state[fd_index], "r")) == NULL) 
    return 0;

  /* Extract battery status */
  fgets(inbuff, 127, fd);
  while(!feof(fd)) 
    {
      sscanf(inbuff, "%s %s %s %s", s1, s2, s3, s4);

      /* find remaining juice */
      if(!strcasecmp(s1, "Remaining")) 
	{
	  bat_val = atoi(s3);
	}

      fgets(inbuff, 127, fd);
    }

  fclose(fd);


  ainfo->ac_line_status = OLSR_BATTERY_POWERED;
  ainfo->battery_percentage = bat_val * 100 / bat_max;

  return 1;
}



static int
acpi_probe()
{
  char s1[32], s2[32];
  FILE *fd;
  olsr_u16_t i;
  

  /* First check for AC power */

  for(i = 0; i < ACPI_AC_CNT; i++)
    {
      /* Try opening the info file */
      if((fd = fopen(acpi_ac[i], "r")) == NULL)
	continue;
      
      /* Extract info */
      if(fscanf(fd, "%s %s", s1, s2) < 2)
	{
	  fclose(fd);
	  continue;
	}
      
      /* Close info entry */
      fclose(fd);

      /* Running on AC power */
      if(!strcasecmp(s2, "on-line"))
	return -1;

    }

  /* Only checking the first found batery entry... */
  for(i = 0; i < ACPI_BT_CNT; i ++)
    {

      /* Try opening the info file */
      if((fd = fopen(acpi_info[i], "r")) == NULL)
	continue;

      /* Extract info */
      if(fscanf(fd, "%s %s", s1, s2) < 2)
	{
	  fclose(fd);
	  continue;
	}

      /* Close info entry */
      fclose(fd);

      /* Check if battery is present */
      if((!strcasecmp(s1, "present:")) && (!strcasecmp(s2, "no"))) 
	continue;

      /* Open the corresponding state file */
      if((fd = fopen(acpi_state[i], "r")) == NULL)
	continue;

      fclose(fd);

      return i;
    }

  /* No battery found */
  return -1;
}

