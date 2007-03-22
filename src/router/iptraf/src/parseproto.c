/*
 * parseports.c - code to extract the protocol codes or ranges thereof from
 *                the user-defined string.
 *
 * Copyright (c) Gerard Paul Java 2002
 *
 * This software is open-source; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License in the included COPYING file for
 * details.
 */

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "parseproto.h"


/*
 * Extracts next token from the buffer.
 */
char *get_next_token(char *buf, char **cptr)
{
    static char rtoken[32];
    int i;

    i = 0;


    /*
     * Skip over leading whitespace
     */

    while (isspace(**cptr))
        (*cptr)++;

    if (**cptr == ',' || **cptr == '-') {
        rtoken[0] = **cptr;
        rtoken[1] = '\0';
        (*cptr)++;
    } else {
        while (!isspace(**cptr) && **cptr != '-' && **cptr != ','
               && **cptr != '\0') {
            rtoken[i] = **cptr;
            (*cptr)++;
            i++;
        }
        rtoken[i] = '\0';
    }

    return rtoken;
}

void get_next_protorange(char *src, char **cptr,
                         unsigned int *proto1, unsigned int *proto2,
                         int *parse_result, char **badtokenptr)
{
    char toktmp[5];
    char prototmp1[5];
    char prototmp2[5];
    char *cerr_ptr;
    static char bad_token[5];
    unsigned int tmp;

    memset(toktmp, 0, 5);
    memset(prototmp1, 0, 5);
    memset(prototmp2, 0, 5);
    memset(bad_token, 0, 5);

    strncpy(prototmp1, get_next_token(src, cptr), 5);
    if (prototmp1[0] == '\0') {
        *parse_result = NO_MORE_TOKENS;
        return;
    }

    strncpy(toktmp, get_next_token(src, cptr), 5);

    *parse_result = RANGE_OK;

    switch (toktmp[0]) {
    case '-':
        strncpy(prototmp2, get_next_token(src, cptr), 5);

        /*
         * Check for missing right-hand token for -
         */
        if (prototmp2[0] == '\0') {
            *parse_result = INVALID_RANGE;
            strcpy(bad_token, "-");
            *badtokenptr = bad_token;
            break;
        }
        *proto2 = (unsigned int) strtoul(prototmp2, &cerr_ptr, 10);
        /*
         * First check for an invalid character
         */
        if (*cerr_ptr != '\0') {
            *parse_result = INVALID_RANGE;
            strncpy(bad_token, prototmp2, 5);
            *badtokenptr = bad_token;
        } else {
            /*
             * Then check for the validity of the token
             */

            if (*proto2 > 255) {
                strncpy(bad_token, prototmp2, 5);
                *badtokenptr = bad_token;
                *parse_result = OUT_OF_RANGE;
            }

            /*
             * Then check if the next token is a comma
             */
            strncpy(toktmp, get_next_token(src, cptr), 5);
            if (toktmp[0] != '\0' && toktmp[0] != ',') {
                *parse_result = COMMA_EXPECTED;
                strncpy(bad_token, toktmp, 5);
                *badtokenptr = bad_token;
            }
        }

        break;
    case ',':
    case '\0':
        *proto2 = 0;
        break;
    default:
        *parse_result = COMMA_EXPECTED;
        strncpy(bad_token, toktmp, 5);
        *badtokenptr = bad_token;
        break;
    }

    if (*parse_result != RANGE_OK)
        return;

    *proto1 = (unsigned int) strtoul(prototmp1, &cerr_ptr, 10);
    if (*cerr_ptr != '\0') {
        *parse_result = INVALID_RANGE;
        strncpy(bad_token, prototmp1, 5);
        *badtokenptr = bad_token;
    } else if (*proto1 > 255) {
        *parse_result = OUT_OF_RANGE;
        strncpy(bad_token, prototmp1, 5);
        *badtokenptr = bad_token;
    } else
        *badtokenptr = NULL;

    if (*proto2 != 0 && *proto1 > *proto2) {
        tmp = *proto1;
        *proto1 = *proto2;
        *proto2 = tmp;
    }
}

int validate_ranges(char *samplestring, int *parse_result,
                    char **badtokenptr)
{
    int proto1, proto2;
    char *cptr = samplestring;

    do {
        get_next_protorange(samplestring, &cptr, &proto1, &proto2,
                            parse_result, badtokenptr);
    } while (*parse_result == RANGE_OK);

    if (*parse_result != NO_MORE_TOKENS)
        return 0;

    return 1;
}
