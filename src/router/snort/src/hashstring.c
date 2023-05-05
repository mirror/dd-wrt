
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
 * File: HashString.C
 *
 * Purpose: Provide a hashed string compare function.
 *
 * History:
 *
 * Date:      Author:  Notes:
 * ---------- ------- ----------------------------------------------
 *  10/02/13    ECB    Initial coding begun
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
#include "hashstring.h"
#include "snort_debug.h"
#include "util.h"
#include "sf_sechash.h"
#include "detection_util.h"

#ifdef TEST_HASHSTRING

int main()
{
    char test[] = "\0\0\0\0\0\0\0\0\0CKAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\0\0";
    char find[] = "CKAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\0\0";

/*   char test[] = "\x90\x90\x90\x90\x90\x90\xe8\xc0\xff\xff\xff/bin/sh\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90";
     char find[] = "\xe8\xc0\xff\xff\xff/bin/sh";  */
    int i;
    int toks;
    int *shift;
    int *skip;

/*   shift=make_shift(find,sizeof(find)-1);
     skip=make_skip(find,sizeof(find)-1); */

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,"%d\n",
			    mSearch(test, sizeof(test) - 1, find,
				    sizeof(find) - 1, shift, skip)););

    return 0;
}

#endif

/****************************************************************
 *
 *  Function: hashSearchFixed(char *, int, Secure_Hash_Type type, char *)
 *
 *  Purpose: Determines if a string hashes to a given hash digest.
 *
 *  Parameters:
 *      buf => data buffer we want to find the data in
 *      blen => data buffer length
 *      type => Pattern_Type
 *      ptrn => pattern to find
 *
 *  Returns:
 *      Integer value, 1 on success (hash of string matches pattern), 0 on
 *      failure
 *
 ****************************************************************/
int hashSearchFixed(const char *buf, int blen, const Secure_Hash_Type type, const char *ptrn)
{
    unsigned char *digest;
    size_t pattern_length;

#ifdef DEBUG_MSGS
    char *hexbuf;
    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,"buf: %p  blen: %d  ptrn: %p  "
                "type: %d\n", buf, blen, ptrn, (int)type););
    hexbuf = fasthex((const u_char *)buf, blen);
    DebugMessage(DEBUG_PATTERN_MATCH,"buf: %s\n", hexbuf);
    free(hexbuf);
    hexbuf = fasthex((const u_char *)ptrn, strlen(ptrn));
    DebugMessage(DEBUG_PATTERN_MATCH,"ptrn: %s\n", hexbuf);
    free(hexbuf);
#endif /* DEBUG_MSGS */

    if((blen == 0) || (ptrn == NULL) || (buf == NULL))
        return( 0 );

    switch( type )
    {
        case SECHASH_MD5:
            {
                digest = MD5DIGEST((const unsigned char *)buf, (unsigned int)blen, NULL);
                pattern_length = MD5_HASH_SIZE;
                break;
            }
        case SECHASH_SHA256:
            {
                digest = SHA256DIGEST((const unsigned char *)buf, (unsigned int)blen, NULL);
                pattern_length = SHA256_HASH_SIZE;
                break;
            }
        case SECHASH_SHA512:
            {
                digest = SHA512DIGEST((const unsigned char *)buf, (unsigned int)blen, NULL);
                pattern_length = SHA512_HASH_SIZE;
                break;
            }
        default:
            {
                return( 0 );
            }
    }

    if(memcmp((const void *)digest, (const void *)ptrn, pattern_length) == 0)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "match\n"););
        UpdateDoePtr(((const uint8_t *)&(buf[0]) + blen), 0);
        return 1;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "no match\n"););

    return 0;
}
