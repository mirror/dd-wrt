/** @file lldp_debug.c
 *
 * OpenLLDP Debug Core
 *
 * Licensed under a dual GPL/Proprietary license.  
 * See LICENSE file for more info.
 * 
 * File: lldp_debug.c
 * 
 * Authors: Terry Simons (terry.simons@gmail.com)
 * 
 * Some code below was borrowed from the Open1X project.
 *
 *******************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <syslog.h>
#include <stdint.h>

#include "lldp_debug.h"

/* Borrowed from Open1X */
unsigned char debug_level = 0;
int isdaemon = 0;
int syslogging = 0;
FILE *logfile = NULL;

/* Borrowed from Open1X */
static inline char to_hex_char(int val)
{
  return("0123456789abcdef"[val & 0xf]);
}



/**
 *
 * Display some information.  But only if we are at a debug level that
 * should display it.
 *
 */


/**
 *
 * Set flags based on the numeric value that was passed in.
 *
 */
void debug_set_flags(int new_flags)
{
  if (new_flags >= 1) debug_level |= DEBUG_CONFIG;
  if (new_flags >= 2) debug_level |= DEBUG_STATE;
  if (new_flags >= 3) debug_level |= DEBUG_TLV;
  if (new_flags >= 4) debug_level |= DEBUG_INT;
  if (new_flags >= 5) debug_level |= DEBUG_SNMP;
  if (new_flags >= 6) debug_level |= DEBUG_EVERYTHING;
  if (new_flags >= 7) debug_level |= DEBUG_EXCESSIVE;
}

/**
 *
 * Set flags based on an ASCII string that was passed in.
 *
 */
void debug_alpha_set_flags(char *new_flags)
{
  int i;

  debug_level = 0;

  for (i=0;i<strlen(new_flags);i++)
    {
      switch (new_flags[i])
        {
        case 'c':
          debug_level |= DEBUG_CONFIG;
          break;

        case 's':
          debug_level |= DEBUG_STATE;
          break;

        case 't':
          debug_level |= DEBUG_TLV;
          break;

        case 'i':
          debug_level |= DEBUG_INT;
          break;

        case 'n':
          debug_level |= DEBUG_SNMP;
          break;

        case 'e':
          debug_level |= DEBUG_EVERYTHING;
          break;

        case 'x':
          debug_level |= DEBUG_EXCESSIVE;
          break;

        case 'A':
          debug_level |= 0xff;   // Set all flags.
          break;
        }
    }
}
