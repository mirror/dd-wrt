/*
 **
 **  reg_test.c
 **
 **  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 **
 **  This program is free software; you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License Version 2 as
 **  published by the Free Software Foundation.  You may not use, modify or
 **  distribute this program under any other version of the GNU General
 **  Public License.
 **
 **  This program is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with this program; if not, write to the Free Software
 **  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>

#include "reg_test.h"
#include "sf_types.h"

#ifdef REG_TEST

uint32_t rt_ip_increment = 0;

uint64_t getRegTestFlags(void)
{
    static uint64_t regTestFlags = 0;
    static bool retTestInitialized = false;

    if (!retTestInitialized)
    {
        const char* key = getenv(REG_TEST_VARIABLE);

        if (key)
            regTestFlags = strtoul(key, NULL, 0);

        retTestInitialized = true;
    }

    return regTestFlags;
}
uint64_t getRegTestFlagsForEmail(void)
{
    static uint64_t regTestFlags = 0;
    static bool retTestInitialized = false;

    if (!retTestInitialized)
    {
        const char* key = getenv(REG_TEST_EMAIL_VARIABLE);

        if (key)
            regTestFlags = strtoul(key, NULL, 0);

        retTestInitialized = true;
    }

    return regTestFlags;
}

void regTestCheckIPIncrement(void)
{
    if (REG_TEST_FLAG_INCREMENT_IP_ADDRESS & getRegTestFlags())
        rt_ip_increment++;
}

#endif

