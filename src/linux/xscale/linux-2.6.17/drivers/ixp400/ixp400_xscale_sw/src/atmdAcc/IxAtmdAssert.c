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
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
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


