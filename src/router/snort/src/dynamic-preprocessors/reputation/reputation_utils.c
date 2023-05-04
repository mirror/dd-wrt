/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************
 * Provides convenience functions.
 *
 * 6/11/2011 - Initial implementation ... Hui Cao <hcao@sourcefire.com>
 *
 ****************************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "reputation_utils.h"
#include <stdio.h>
#include <limits.h>

#define MAX_ADDR_LINE_LENGTH    8192

/********************************************************************
 * Function: Reputation_IsEmptyStr()
 *
 * Checks if string is NULL, empty or just spaces.
 * String must be 0 terminated.
 *
 * Arguments:
 *  char * - string to check
 *
 * Returns:
 *  1  if string is NULL, empty or just spaces
 *  0  otherwise
 *
 ********************************************************************/
int Reputation_IsEmptyStr(char *str)
{
    char *end;

    if (str == NULL)
        return 1;

    end = str + strlen(str);

    while ((str < end) && isspace((int)*str))
        str++;

    if (str == end)
        return 1;

    return 0;
}

/********************************************************************
 * Function: numLinesInFile()
 *
 * Number of lines in the file
 *
 * Arguments:
 *  fname: file name
 *
 * Returns:
 *  uint32_t  number of lines
 *
 ********************************************************************/
int numLinesInFile(char *fname)
{
    FILE *fp;
    uint32_t numlines = 0;
    char buf[MAX_ADDR_LINE_LENGTH];

    fp = fopen(fname, "rb");

    if (NULL == fp)
        return 0;

    while((fgets(buf, MAX_ADDR_LINE_LENGTH, fp)) != NULL)
    {
        if (buf[0] != '#')
        {
            numlines++;
            if (numlines == INT_MAX)
            {
                fclose(fp);
                return INT_MAX;
            }
        }
    }

    fclose(fp);
    return numlines;
}
