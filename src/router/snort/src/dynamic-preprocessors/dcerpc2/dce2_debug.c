/****************************************************************************
 * Copyright (C) 2008-2011 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 **************************************************************************** 
 * Provides functions for debugging the preprocessor.
 *
 * 8/17/2008 - Initial implementation ... Todd Wease <twease@sourcefire.com>
 *
 ****************************************************************************/

#include "dce2_debug.h"
#include "dce2_utils.h"
#include "sf_types.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>

extern DynamicPreprocessorData _dpd;

/********************************************************************
 * Function: DCE2_GetDebugLevel()
 *
 * Gets the debugging level set by the DCE2 debugging environment
 * variable on the first call.  Subsequent calls will used the
 * cached value.
 *
 * Arguments: None
 *
 * Returns:
 *  uint32_t
 *      The debugging level set by the environment variable.
 *
 ********************************************************************/
static uint32_t DCE2_GetDebugLevel(void)
{
    static int debug_init = 0;
    static uint32_t debug_level = 0;
    const char* value;

    if (debug_init)
        return debug_level;

    value = getenv(DCE2_DEBUG_VARIABLE);

    if (value != NULL)
    {
        char *endptr;

        debug_level = _dpd.SnortStrtoul(value, &endptr, 0);
        if ((errno == ERANGE) || (*endptr != '\0'))
        {
            DCE2_Log(DCE2_LOG_TYPE__WARN,
                     "\"%s\" value out of range or not a number: %s. "
                     "Debugging will not be turned on.",
                     DCE2_DEBUG_VARIABLE, value);

            debug_level = 0;
        }
    }

    debug_init = 1;

    return debug_level;
}

/********************************************************************
 * Function: DCE2_DebugThis()
 *
 * Determines based on the level if debugging is turned on.
 *
 * Arguments:
 *  int
 *      The level to check for.
 *
 * Returns:
 *  int
 *      1 if debugging is turned on.
 *      0 if debugging is not turned on.
 *
 ********************************************************************/
int DCE2_DebugThis(int level)
{
    if (level & DCE2_GetDebugLevel()) return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_DebugMsg()
 *
 * Prints message to stdout if debugging is on for specified level.
 *
 * Arguments:
 *  int
 *      The level the message refers to.
 *  const char *
 *      The format string.
 *  ...
 *      The arguments to the format string.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_DebugMsg(int level, const char *format, ...)
{
    va_list ap;

    if (!DCE2_DebugThis(level))
        return;

    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);
}

