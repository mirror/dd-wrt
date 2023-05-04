/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2009-2013 Sourcefire, Inc.
**
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "spp_sdf.h"
#include "sdf_us_ssn.h"
#include "sf_dynamic_preprocessor.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>

static int SDFCompareGroupNumbers(int group, int max_group);
static int SSNGroupCategory(int group);

/* This function takes a string representation of a US Social Security number
   and checks that it is valid. The string may include or omit hyphens.*/
int SDFSocialCheck(char *buf, uint32_t buflen, struct _SDFConfig *config)
{
    uint32_t i;
    int digits, area, group, serial;
    char numbuf[9];

    if (buf == NULL || buflen > 13 || buflen < 9)
        return 0;

    /* Generally, the string will have a non-digit byte on each side.
     * Sometimes, when the string is pointing to the first line of the
     * data, it might start with a digit, instead of a non-digit.
     * Strip the non-digits only.
     */
    if (isdigit((int)buf[0]))
        buflen -= 1;

    else
    {
        buf++;
        buflen -= 2;
    }

    /* Check that the string is made of digits, and strip hyphens. */
    digits = 0;
    for (i = 0; i < buflen; i++)
    {
        if (isdigit((int)buf[i]))
        {
            /* Check for too many digits */
            if (digits == 9)
                return 0;

            numbuf[digits++] = buf[i];
        }
        else if (buf[i] != '-')
            break;
    }

    if (digits != 9)
        return 0;

    /* Convert to ints */
    area = (numbuf[0] - '0') * 100 + (numbuf[1] - '0') * 10 + (numbuf[2] - '0');
    group = (numbuf[3] - '0') * 10 + (numbuf[4] - '0');
    serial = (numbuf[5] - '0') * 1000 + (numbuf[6] - '0') * 100 +
             (numbuf[7] - '0') * 10 + (numbuf[8] - '0');

    /* This range was reserved for advertising */
    if (area == 987 && group == 65)
    {
        if (serial >= 4320 && serial <= 4329)
            return 0;
    }

    /* Start validating */
    if (area > MAX_AREA ||
        area == 666 ||
        area <= 0 ||
        group <= 0 ||
        group > 99 ||
        serial <= 0 ||
        serial > 9999)
            return 0;

    return SDFCompareGroupNumbers(group, config->ssn_max_group[area]);
}

static int SDFCompareGroupNumbers(int group, int max_group)
{
    /* Group numbers are not issued in consecutive order. They go in this order:
        1. ODD numbers from 01 through 09
        2. EVEN numbers from 10 through 98
        3. EVEN numbers from 02 through 08
        4. ODD numbers from 11 through 99
       For this reason, the group check is not simple.
     */

    int group_category = SSNGroupCategory(group);
    int max_group_category = SSNGroupCategory(max_group);

    if (group_category == 0 || max_group_category == 0)
        return 0;

    if (group_category < max_group_category)
        return 1;
    if ((group_category == max_group_category) && (group <= max_group))
        return 1;

    return 0;
}

static int SSNGroupCategory(int group)
{
    if ((group % 2 == 1) && (group < 10))
        return 1;
    if ((group % 2 == 0) && (group >= 10) && (group <= 98))
        return 2;
    if ((group % 2 == 0) && (group < 10))
        return 3;
    if ((group % 2 == 1) && (group >= 11) && (group <= 99))
        return 4;

    return 0;
}

int ParseSSNGroups(char *filename, struct _SDFConfig *config)
{
    FILE *ssn_file;
    char *contents, *token, *saveptr, *endptr;
    long length;
    int i = 1;

    if (filename == NULL || config == NULL)
        return -1;

    ssn_file = fopen(filename, "r");
    if (ssn_file == NULL)
    {
        _dpd.logMsg("Sensitive Data preprocessor: Failed to open SSN groups "
                "file \"%s\": %s.\n", filename, strerror(errno));
        return -1;
    }

    /* Determine size of file */
    if (fseek(ssn_file, 0, SEEK_END) == -1)
    {
        _dpd.logMsg("Sensitive Data preprocessor: Failed to fseek() to end of "
                "SSN groups file \"%s\": %s.\n", filename, strerror(errno));

        fclose(ssn_file);
        return -1;
    }

    if ((length = ftell(ssn_file)) <= 0)
    {
        if (length == -1)
        {
            _dpd.logMsg("Sensitive Data preprocessor: Failed to get size of SSN "
                    "groups file \"%s\": %s.\n", filename, strerror(errno));
        }
        else
        {
            _dpd.logMsg("Sensitive Data preprocessor: SSN groups file \"%s\" "
                    "is empty.\n", filename);
        }

        fclose(ssn_file);
        return -1;
    }

    rewind(ssn_file);

    contents = (char *)malloc(length + 1);
    if (contents == NULL)
    {
        _dpd.logMsg("Sensitive Data preprocessor: Failed to allocate memory "
                "for SSN groups.\n");

        fclose(ssn_file);
        free(contents);
        return -1;
    }

    /* Read file into memory */
    if (fread(contents, sizeof(char), length, ssn_file) != (size_t)length)
    {
        _dpd.logMsg("Sensitive Data preprocessor: Failed read contents of "
                "SSN groups file \"%s\".\n", filename);

        free(contents);
	fclose(ssn_file);
        return -1;
    }

    fclose(ssn_file);

    contents[length] = '\0';

    /* Parse! */
    token = strtok_r(contents, " ,\n", &saveptr);
    while (token)
    {
        if (i > MAX_AREA)
        {
            /* TODO: Print error - too many ints */
            free(contents);
            return -1;
        }
        config->ssn_max_group[i++] = strtol(token, &endptr, 10);
        if (*endptr != '\0')
        {
            /* TODO: Print error - not a complete number */
            free(contents);
            return -1;
        }

        token = strtok_r(NULL, " ,\n", &saveptr);
    }

    free(contents);
    return 0;
}

/* Default array of maximum group numbers for each area.
   These values were last up-to-date as of November 2009. */
int SSNSetDefaultGroups(struct _SDFConfig *config)
{
    int i;
    int default_max_group[MAX_AREA+1] = { 0,
        8, 8, 6, 11, 11, 11, 8, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
        92, 92, 92, 92, 92, 92, 92, 92, 92, 90, 90, 90, 90, 90, 90, 74, 74, 72, 72,
        72, 15, 13, 13, 13, 13, 13, 13, 13, 13, 13, 98, 98, 98, 98, 98, 98, 98, 98,
        98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
        98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
        98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
        98, 98, 98, 98, 98, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96,
        96, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
        21, 21, 21, 21, 21, 21, 86, 86, 86, 86, 86, 86, 86, 86, 84, 84, 84, 84, 84,
        84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84,
        84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84,
        84, 84, 85, 85, 85, 85, 85, 85, 85, 85, 85, 8, 8, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 55, 55, 55, 55, 55, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
        99, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 35, 35,
        35, 35, 35, 35, 35, 35, 33, 33, 33, 33, 33, 33, 33, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 6, 6, 6,
        37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 35, 35, 35, 35, 35, 35, 35, 35,
        35, 35, 35, 35, 35, 35, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 29,
        71, 71, 71, 71, 71, 71, 69, 69, 99, 99, 99, 99, 99, 99, 99, 99, 65, 65, 65,
        65, 65, 65, 65, 63, 63, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 27, 25, 25, 25, 25, 25, 25, 25, 25, 99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 55, 53, 53, 53, 53, 53, 53, 53,
        53, 53, 41, 41, 39, 39, 39, 39, 39, 39, 27, 27, 27, 27, 27, 27, 27, 27, 27,
        27, 27, 27, 27, 27, 27, 35, 35, 43, 43, 55, 55, 55, 55, 31, 31, 31, 29, 29,
        29, 29, 47, 47, 83, 83, 59, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 67, 67,
        67, 67, 67, 67, 67, 67, 65, 79, 79, 79, 77, 77, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 57, 99, 99, 49, 49, 49, 39, 99, 99, 99, 99, 99, 65, 99, 5, 99,
        99, 99, 99, 99, 99, 99, 90, 88, 88, 88, 99, 99, 79, 79, 79, 79, 79, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 23,
        23, 23, 23, 23, 23, 23, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 13,
        11, 52, 52, 56, 56, 54, 54, 32, 32, 32, 32, 32, 20, 20, 20, 20, 18, 18, 18,
        44, 42, 42, 42, 42, 42, 42, 42, 42, 18, 18, 18, 16, 17, 20, 20, 20, 20, 18,
        18, 18, 18, 18, 18, 12, 12, 12, 12, 12, 12, 12, 12, 12, 18, 18, 18, 18, 18,
        18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
        28, 18, 18, 10, 14, 20, 18, 18, 18, 18, 14, 14, 5, 5, 5, 5, 10, 9, 9,
        9, 9, 9, 9, 9, 11, 8, 86, 86, 86, 86, 84, 84, 84
    };

    if (config == NULL)
        return -1;

    for (i = 0; i < MAX_AREA+1; i++)
    {
        config->ssn_max_group[i] = default_max_group[i];
    }

    return 1;
}
