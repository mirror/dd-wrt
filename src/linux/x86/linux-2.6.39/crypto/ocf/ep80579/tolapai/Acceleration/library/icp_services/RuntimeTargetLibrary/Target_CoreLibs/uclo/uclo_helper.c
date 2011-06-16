/**
**************************************************************************
* @file uclo_helper.c
*
* @description
*      This file provides Ucode Object File Loader facilities
*
* @par 
* This file is provided under a dual BSD/GPLv2 license.  When using or 
*   redistributing this file, you may do so under either license.
* 
*   GPL LICENSE SUMMARY
* 
*   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
* 
*   This program is free software; you can redistribute it and/or modify 
*   it under the terms of version 2 of the GNU General Public License as
*   published by the Free Software Foundation.
* 
*   This program is distributed in the hope that it will be useful, but 
*   WITHOUT ANY WARRANTY; without even the implied warranty of 
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
*   General Public License for more details.
* 
*   You should have received a copy of the GNU General Public License 
*   along with this program; if not, write to the Free Software 
*   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
*   The full GNU General Public License is included in this distribution 
*   in the file called LICENSE.GPL.
* 
*   Contact Information:
*   Intel Corporation
* 
*   BSD LICENSE 
* 
*   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
*   All rights reserved.
* 
*   Redistribution and use in source and binary forms, with or without 
*   modification, are permitted provided that the following conditions 
*   are met:
* 
*     * Redistributions of source code must retain the above copyright 
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright 
*       notice, this list of conditions and the following disclaimer in 
*       the documentation and/or other materials provided with the 
*       distribution.
*     * Neither the name of Intel Corporation nor the names of its 
*       contributors may be used to endorse or promote products derived 
*       from this software without specific prior written permission.
* 
*   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
*   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
*   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
*   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
* 
*  version: Security.L.1.0.3-98
*
**************************************************************************/ 

#include "uclo_helper.h"

static const unsigned int Int64Bits = 64;

/*-----------------------------------------------------------------------------
Function:    UcLo_parseNum
Description: Parses either an octal, decimal, or hex integer number from
the input string.
Returns:    0: SUCCESS, 1:FAILURE
Uses:
Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_parseNum(char *pStr, 
              int *pNum)
{
    char *token = NULL, *endPtr = NULL, *digitCheck = NULL;
    int    base = 10;

    if(!pStr || (*pStr == '\0')) 
    {
        return (1);
    }    
    /* strip leading spaces */
    token = UcLo_stripLeadBlanks(pStr);

    if((*token == '-') || (*token == '+')) 
    {
        digitCheck = token+1;
    }
    else 
    {
        digitCheck = token;
    }    

    if(!isdigit(*digitCheck))
    {
        return (1);
    }    

    if(token[0] == '0') 
    {
        if((token[1] == 'x') || (token[1] == 'X'))
        {
            base = 16;
        }      
        else 
        {
            base = 8;
        }      
    }
    *pNum = strtoul(token, &endPtr, base);

    return (0);
}

/*-----------------------------------------------------------------------------
Function: UcLo_stripLeadBlanks    
Description: Remove leading blanks or tabs from string
Returns: pointer to the string
Uses:
Modifies:
-----------------------------------------------------------------------------*/
char *
UcLo_stripLeadBlanks(char *pStr)
{
    char *pC = NULL;

    for (pC = pStr; pC && ((*pC == '\t') ||( *pC == ' ')); pC++) ;
    return (pC);
}

/*-----------------------------------------------------------------------------
Function:    UcLo_strcmp
Description: Compare two strings lexicographically. This function is similar
to ANSI strcmp, except it allows case-insensitivity comparision.

Returns:    -1 if lex(pStr1) < lex(pStr2)
0 if lex(pStr1 == lex(pStr2)
1 if lex(pStr1) > lex(pStr2)
Uses:
Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_strcmp(const char *pStr1, 
            const char *pStr2, 
            int sensitive)
{
    int c1 = 0, c2 = 0;
    int result = 0;

    while(1)
    {
        c1 = *pStr1;  c2 = *pStr2;

        if(!sensitive)
        {
            c1 =  tolower(c1);
            c2 = tolower(c2);
        }

        if(c1 < c2)
        {
            result = -1;
        }    
        else if(c1 > c2) 
        {
            result = 1;
        }    
        if(result || !c1 || !c2) 
        {
            break;
        }    
        pStr1++;  pStr2++;
    }
    return (result);
}

/*-----------------------------------------------------------------------------
Function:     UcLo_strtoken
Description: Extracts the token between the specified delimiters.
Returns:    The pointer to the token, or NULL.
Uses:
Modifies:
-----------------------------------------------------------------------------*/
char *
UcLo_strtoken(char *pStr, 
              char *pDlim, 
              char **savPtr)
{
    char *pBegStr = NULL, *pS = NULL;

    /* if pStr is NULL, then use the last reference pointer */
    if (!pStr) 
    {
        if (savPtr && *savPtr)
        {
            pStr = *savPtr;
        }    
        else
        {
            return (NULL);
        }    
    }

    if (savPtr)
    {
        *savPtr = NULL;
    } 
    /* locate the first byte of pStr that does not occur in pDlim */
    for (pBegStr = pS = pStr; *pS; pS++) 
    {
        if (!(strchr(pDlim, *pS))) 
        {
            pBegStr = pS;
            break;
        }
    }

    /* locate the first byte of pStr that does occur in pDlim */
    for (; *pS; pS++) {
        if ((strchr(pDlim, *pS))) 
        {
            *pS = '\0';
            if (savPtr)
            {
                *savPtr = pS + 1;
            }   
            break;
        }
    }

    return (pBegStr);
}

/*-----------------------------------------------------------------------------
Function:     Crc_Calc
Description: Calculate the CRC checksum
Returns:    The calculated checksum value for the string
Uses:
Modifies:
-----------------------------------------------------------------------------*/
static unsigned int 
Crc_Calc(unsigned int reg, 
         int ch)
{
    int  ii = 0;
    unsigned int topbit = CRC_BITMASK(CRC_WIDTH - 1);
    unsigned int inbyte = (unsigned int) ((reg >> 24) ^ ch);

    reg ^= inbyte << (CRC_WIDTH - 8);
    for (ii = 0; ii < 8; ii++)
    {
        if (reg & topbit) 
        {
            reg = (reg << 1) ^ CRC_POLY;
        }    
        else 
        {
            reg <<= 1;
        }    
    } 
    return (reg & CRC_WIDTHMASK(CRC_WIDTH));
}

/*-----------------------------------------------------------------------------
Function:    UcLo_strChecksum
Description: Calculate the CRC checksum of a string
Returns:     The calculated checksum value for the string
Uses:
Modifies:
-----------------------------------------------------------------------------*/
unsigned int 
UcLo_strChecksum(char *pChar, 
                 int numChar)
{
    unsigned int chksum = 0;

    if(pChar) 
    {
        while (numChar--) 
        {
            chksum = Crc_Calc(chksum, *pChar++);
        }     
    }   
    return (chksum);
}

/*-----------------------------------------------------------------------------
Function:    UcLo_setField64
Description: Sets a specified range of bits of 'word' equal to the bits of
'value'--starting from the zero bit-position of 'value'.
Returns:    The result of the operation
Uses:
Modifies:
-----------------------------------------------------------------------------*/
uint64 
UcLo_setField64(uint64 word, 
                int startBit, 
                int fieldLen, 
                uint64 value)
{
    unsigned int stopBit = 0;
    uint64 mask = 0, bit = 0;

    /* check that the field is within bound of the word */
    if((stopBit = (startBit + fieldLen)) > Int64Bits)
    {
        UCLO_PRNWARN("UcLo_setField64: field exceeds the size of the word\n");

        return (word);
    }

    /* determine the mask */
    for(bit = startBit; bit < stopBit; bit++)
    {
        mask |= ((uint64)1 << bit);
    }   

    /* Assign the first 'numBit' bits of 'value' to the field of 'fieldLen'
    starting at the 'startBit' of 'word'. */
    word &= ~mask;
    return ((word |= mask & (value << startBit)));
}
