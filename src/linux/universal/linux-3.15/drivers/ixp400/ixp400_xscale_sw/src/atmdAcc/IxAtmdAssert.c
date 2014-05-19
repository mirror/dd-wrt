/**
* @file IxAtmdAssert.c
*
 * @author Intel Corporation
* @date 17 March 2002
*
* @brief Atmd Assert handler
*
* This file defines the Assert Handling used in Atmd.
*
* Design Notes:
* This code is used only if NDEBUG is not defined at compile time.
*
 * 
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
*/

/*
* Put the system defined include files required.
*/
#include "IxOsal.h"

/*
* Put the user defined include files required.
*/
#include "IxAtmdAssert_p.h"

#ifndef NDEBUG


#if 0
PRIVATE void
ixAtmdAccAssertShow(void);
#endif

static char fileNameCopy[1024] = "";
static char conditionStringCopy[1024] = "";
static char infoStringCopy[1024] = "";
static unsigned int lineCopy = 0;

#if 0
PRIVATE void
ixAtmdAccAssertShow(void)
{
    printf("\nATMDASSERT (%s:%u) : !(%s) : %s\n",
        fileNameCopy,
        lineCopy,
        conditionStringCopy,
        infoStringCopy);
}
#endif

/*
* Function definition.
*/

/* ----------------------------------------------------------
Customized version of an assert handler
*/
void
ixAtmdAccAssert (BOOL condition,
                 char *fileName,
                 unsigned int line,
                 char *conditionString,
                 char *infoString)
{
    char *ptFname = fileName;

    /* test the condition status */
    if (!condition)
    {

        /* skip directory name from file name */
        while (*ptFname)
        {
            if (*ptFname == '/' || *ptFname == '\\')
            {
                fileName = ptFname + 1;
            }
            ptFname++;
        } /* end of while(ptFname) */

        /* remember the last assert strings */
        strcpy(fileNameCopy, fileName);
        strcpy(conditionStringCopy, conditionString);
        strcpy(infoStringCopy, infoString);
        lineCopy = line;

        /* display/log a message */
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR, 
	    IX_OSAL_LOG_DEV_STDERR, 
            "\nATMDASSERT (%s:%u) : !(%s) : %s\n",
            (int)fileName,
            (int)line,
            (int)conditionString,
            (int)infoString,
            0,
            0);

        IX_OSAL_ASSERT (0);
    } /* end of if(condition) */
}

#endif /* NDEBUG */


