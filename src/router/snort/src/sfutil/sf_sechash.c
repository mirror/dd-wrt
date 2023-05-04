
/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2013 Sourcefire, Inc.
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

/***************************************************************************
 *
 * File: sf_sechash.C
 *
 * Purpose: Provide a set of secure hash utilities
 *
 * History:
 *
 * Date:      Author:  Notes:
 * ---------- ------- ----------------------------------------------
 *  12/05/13    ECB    Initial coding begun
 *
 **************************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include "sf_types.h"
#include "sf_sechash.h"
#include "snort_debug.h"


static struct 
{
    Secure_Hash_Type type;
    char *name;
    unsigned int length;
} Secure_Hash_Map[] =
{
    { SECHASH_SHA512, "SHA512", 64 },
    { SECHASH_SHA256, "SHA256", 32 },
    { SECHASH_MD5,    "MD5",    16 },
    { SECHASH_NONE,   "",        0 }
};

unsigned int SecHash_Type2Length( const Secure_Hash_Type type )
{
    unsigned int index;

    index = 0;

    while(Secure_Hash_Map[index].type != SECHASH_NONE)
    {
        if( Secure_Hash_Map[index].type == type )
        {
            return( Secure_Hash_Map[index].length );
        }
        index += 1;
    }

    return( 0 );
}

Secure_Hash_Type SecHash_Name2Type( const char *name )
{
    unsigned int index;

    index = 0;

    while(Secure_Hash_Map[index].type != SECHASH_NONE)
    {
        if( strcasecmp(name,Secure_Hash_Map[index].name) == 0 )
        {
            return( Secure_Hash_Map[index].type );
        }
        index += 1;
    }

    return( SECHASH_NONE );
}
