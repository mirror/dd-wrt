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
#include "sdf_credit_card.h"
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>

/* Check the Issuer Identification Number of a CC#. */
static inline int CheckIssuers(char *cardnum, uint32_t buflen)
{
    if (cardnum == NULL || buflen < ISSUER_SIZE)
        return 0;

    /* Visa */
    if (cardnum[0] == '4')
        return 1;

    /* Mastercard */
    if ((cardnum[0] == '5') &&
        (cardnum[1] > '0') &&
        (cardnum[1] < '6'))
        return 1;

    /* Amex */
    if ((cardnum[0] == '3') &&
        (cardnum[1] == '4' || cardnum[1] == '7'))
        return 1;

    /* Discover */
    if (cardnum[0] == '6' && cardnum[1] == '0' && cardnum[2] == '1' && cardnum[3] == '1')
        return 1;

    return 0;
}

/* This function takes a string representation of a credit card number and
 * checks that it's a valid number. The number may contain spaces or dashes.
 *
 * Returns: 1 on match, 0 otherwise.
 */
int SDFLuhnAlgorithm(char *buf, uint32_t buflen, struct _SDFConfig *config)
{
    int i, digits, alternate, sum, val;
    char cc_digits[CC_COPY_BUF_LEN]; /* Normalized CC# string */
    uint32_t j;

    if (buf == NULL || buflen < MIN_CC_BUF_LEN)
        return 0;

    /* Generally, the buffer has two non-digits, one on either side. Sometimes,
     * when the buffer is pointing to the first line of the data, it might
     * start with a digit, instead of a non-digit.  Sometimes, the buffer has only 
     * one non-digit either starting or ending. Strip the non-digits only.
     */

    /* Check the buffer is ending with non-digit or digit. If buffer is ending with digit do not decrement the buflen,
     * If buffer is ending with non-digit then decrement buflen as non-digit is not part of credit card.
     */
    if(!isdigit((int)buf[buflen-1]))
    {
        buflen -= 1;
    }

    /* Check start of the buffer, if it is non-digit, decrement buflen by 1 and move pointer to next character/digit in buf.
     */
    if(!isdigit((int)buf[0]))
    {
        buf++;
        buflen -= 1;
    }

    /* If the first digit is greater than 6, this isn't one of the major
       credit cards. */
    if (!isdigit((int)buf[0]) || buf[0] > '6')
        return 0;

    /* Check the issuer number for Visa, Mastercard, Amex, or Discover. */
    if (CheckIssuers(buf, buflen) == 0)
        return 0;

    /* Limit to 16 digits + spaces in between */
    if (buflen >= CC_COPY_BUF_LEN)
        buflen = CC_COPY_BUF_LEN - 1;

    /* Copy the string into cc_digits, stripping out spaces & dashes. */
    digits = 0;
    for (j = 0; j < buflen; j++)
    {
        if (isdigit((int)buf[j]) == 0)
        {
            if (buf[j] == ' ' || buf[j] == '-')
                continue;
            else
                break;
        }

        cc_digits[digits++] = buf[j];
    }
    cc_digits[digits] = '\0';

    /* Check if the string was too short, or we broke at an invalid character */
    if (digits < 13 || digits > 16 || j < buflen)
        return 0;

    /* The Luhn algorithm:
        1) Starting at the right-most digit, double every second digit.
        2) Sum all the *individual* digits (i.e. 16 => 1+6)
        3) If the Sum mod 10 == 0, the CC# is valid.
     */
    alternate = 0;
    sum = 0;
    for (i = digits - 1; i >= 0; i--)
    {
        val = cc_digits[i] - '0';
        if (alternate)
        {
            val *= 2;
            if (val > 9)
                val -= 9;
        }
        alternate = !alternate;
        sum += val;
    }

    if (sum % 10)
        return 0;

    return 1;
}
