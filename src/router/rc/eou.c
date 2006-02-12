
/*
 *********************************************************
 *   Copyright 2003, CyberTAN  Inc.  All Rights Reserved *
 *********************************************************

 This is UNPUBLISHED PROPRIETARY SOURCE CODE of CyberTAN Inc.
 the contents of this file may not be disclosed to third parties,
 copied or duplicated in any form without the prior written
 permission of CyberTAN Inc.

 This software should be used as a reference only, and it not
 intended for production use!


 THIS SOFTWARE IS OFFERED "AS IS", AND CYBERTAN GRANTS NO WARRANTIES OF ANY
 KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.  CYBERTAN
 SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <syslog.h>
#include <sys/wait.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <rc.h>
#include <wlioctl.h>

static const unsigned char comm_oui[] = { 0x00, 0x03, 0x47 };
static const unsigned char comm_data[] =
  { 0x02, 0x02, 0x01, 0x01, 0x03, 0x14 };
#define COMM_SP		9
#define COMM_INDEX	10
#define COMM_FLAG	11
#define COMM_INUSE	12
#define COMM_LEN	13
#define RES_OUI		14
#define RES_DATA	17
#define RES_DATA_CFS	20
#define RES_DATA_HASH	23
#define RES_DATA_LEN	29
#define COMM_BUF_LEN    43	//= 10 + 4 + RES_DATA_LEN

#define DISABLE		0x00
#define ENABLE		0x01
#define UNCONFIGURED    0x00
#define CONFIGURED      0x01

#define MY_DEBUG	1

unsigned char *hash = NULL;

static void
ascii_string_to_binary_string (char *src, char *destin, int length)
{
  int iloop;
  char hex[] = "XX";
  for (iloop = 0; iloop < 129; iloop++)
    {
      strncpy (hex, src, 2);
      *(destin + iloop) = (unsigned char) strtol (hex, NULL, 16);
      src += 2;
    }
}

static int
set_probe_response_ie (int index, int flags, int enable, int configured_state)
{
  int ret;
  unsigned char command_buf[129];

  eval ("wl", "down");
  memset (command_buf, 0, 128);
  memcpy (command_buf, "custom_ie", 9);
  command_buf[COMM_SP] = 0x00;
  command_buf[COMM_INDEX] = index;	//To use the first IE
  command_buf[COMM_FLAG] = flags;	//To issue in PROBE Response
  command_buf[COMM_INUSE] = enable;
  command_buf[COMM_LEN] = RES_DATA_LEN;
  memcpy (&command_buf[RES_OUI], comm_oui, 3);
  memcpy (&command_buf[RES_DATA], comm_data, 6);
  memcpy (&command_buf[RES_DATA_HASH], hash, 20);
  if (configured_state == CONFIGURED)
    command_buf[RES_DATA_CFS] |= 0x02;	//To set the configured state bit
  ret =
    wl_ioctl (nvram_safe_get ("wl0_ifname"), WLC_SET_VAR, command_buf,
	      COMM_BUF_LEN);
  if (ret == -1)
    cprintf ("SET ERROR");
  eval ("wl", "up");

  return 0;

}

static int
check_status (void)
{
  int ret = 0;

  if (nvram_match ("get_eou_index", "0"))
    {
      cprintf ("The Router has not yet burned a EoU key!\n");
      ret = -1;
      goto Exit;
    }
  else if (nvram_match ("get_eou_index", "8"))
    {
      cprintf ("The EoU table is full!\n");
      ret = 0;
      goto Exit;
    }

  if (nvram_match ("eou_device_id", "")
      || strlen (nvram_safe_get ("eou_device_id")) != 8
      || nvram_match ("eou_private_key", "")
      || strlen (nvram_safe_get ("eou_private_key")) != 256
      || nvram_match ("eou_public_key", "")
      || strlen (nvram_safe_get ("eou_public_key")) != 258)
    {
      cprintf ("Some value for EoU is invalid!\n");
      ret = -1;
      goto Exit;
    }
Exit:

  return ret;
}

int
start_eou (void)
{
  int i, ret;

  static const char rnd_seed[] =
    "string to make the random number generator think it has entropy";
  unsigned char public_key[129];
  char *key = nvram_safe_get ("eou_public_key");

  if (nvram_match ("wl_gmode", "-1"))
    return 0;

  if (check_status () < 0)
    return 0;

  ascii_string_to_binary_string (key, public_key, 129);

  //RAND_seed(rnd_seed, sizeof rnd_seed);
  //hash = SHA1(public_key, 129, hash);
  cprintf ("Generate hash of public key:\n");
  //printHEX(hash, 20);

  //if(nvram_match("eou_configured", "1")) {
  if (nvram_match ("eou_configured", "1")
      || nvram_match ("wl_auth_mode", "psk")
      || nvram_match ("wl0_akm", "psk"))
    {				//Added by Daniel(2004-08-19)
      //ret = set_probe_response_ie(0x00, 0x02, ENABLE, CONFIGURED);
      cprintf ("The EoU is already configured\n");
      return 0;
    }
  else
    {
      cprintf ("The EoU is not already configured\n");
      ret = set_probe_response_ie (0x00, 0x02, ENABLE, UNCONFIGURED);

      eval ("eou", nvram_safe_get ("wl0_ifname"));
    }

  dprintf ("done\n");
  return 0;
}

int
stop_eou (void)
{
  int ret;

  ret = eval ("killall", "-9", "eou");

  dprintf ("done\n");

  return ret;
}


int
eou_status_main (int argc, char *argv[])
{
  cprintf ("STATUS : %s",
	   atoi (nvram_safe_get ("eou_configured")) ? "Configured" :
	   "Not Configured");
  if (nvram_match ("eou_key_index", "0"))
    cprintf ("\t(The Router has not yet burned a EoU key!)\n");
  else if (nvram_match ("eou_key_index", "-1"))
    cprintf ("\t(The EoU table is full!)\n");
  else if (nvram_match ("eou_key_index", "-2"))
    cprintf ("\t(The boot cann't support to burn eou key)\n");
  else
    cprintf ("\t(Next index is %s)\n", nvram_safe_get ("eou_key_index"));

  cprintf ("DEVICE_ID : %s\n", nvram_safe_get ("eou_device_id"));

  cprintf ("OWNERSHIP_KEY : \n");

  cprintf ("PRIVATE_KEY : \n");
  printASC (nvram_safe_get ("eou_private_key"),
	    strlen (nvram_safe_get ("eou_private_key")));

  cprintf ("PUBLIC_KEY : \n");
  printASC (nvram_safe_get ("eou_public_key"),
	    strlen (nvram_safe_get ("eou_public_key")));


  return 0;
}
