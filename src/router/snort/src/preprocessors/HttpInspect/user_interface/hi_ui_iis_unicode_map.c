/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2003-2013 Sourcefire, Inc.
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
 ****************************************************************************/
 
/**
**  @file       hi_ui_iis_unicode_map.c
**  
**  @author     Daniel Roelker <droelker@atlas.cs.cuc.edu>
**  
**  @brief      Functions for parsing the unicode map file
**  
**  This file contains the routines for parsing generated IIS unicode
**  maps.  We read in the map, find where the codepage is located in
**  the map, and convert the codepoint maps, and store in the supplied
**  array.
**  
**  NOTES
**    -  Initial development.  DJR
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "hi_ui_config.h"
#include "hi_ui_iis_unicode_map.h"
#include "hi_util_xmalloc.h"
#include "hi_return_codes.h"

#define MAX_BUFFER 50000
#define CODEPAGE_SEPARATORS  " \t\n\r"
#define CODEPOINT_SEPARATORS ": \n\r"

/*
**  NAME
**    FindCodePage::
*/
/**
**  Locate the codepage mapping the IIS Unicode Map file.
**  
**  We iterate through the file lines until we get to the codepage
**  reference.  We then return that it was found successfully, and 
**  the FILE pointer is located on the codepoint mapping line.
**  
**  @param fFile     the codemap file pointer
**  @param iCodePage the codepage number
**  
**  @return int
**  
**  @retval HI_FATAL_ERR  Did not find the codepage listing.
**  @retval HI_SUCCESS    function successful
*/
static int FindCodePage(FILE *fFile, int iCodePage)
{
    static char buffer[MAX_BUFFER];
    char *pcToken;
    int  iCodePageTest;
    char *pcEnd;
    char *pcPtr;

    while(fgets(buffer, MAX_BUFFER, fFile))
    {
        pcToken = strtok_r(buffer, CODEPAGE_SEPARATORS, &pcPtr);
        if(!pcToken)
            continue;

        if(pcToken[0] == '#')
            continue;

        /*
        **  Is this a codepage or the beginning of a codemap
        */
        if(strchr(pcToken, ':'))
            continue;

        /*
        **  So we now have the beginning of a codepage number
        */
        iCodePageTest = strtol(pcToken, &pcEnd, 10);
        if(*pcEnd)
            continue;

        if(iCodePageTest == iCodePage)
            return HI_SUCCESS;
    }

    return HI_FATAL_ERR;
}

/*
**  NAME
**    MapCodePoints::
*/
/**
**  Read the codepoint mapping and covert to codepoint and ASCII.
**  
**  This is where the bulk of the work is done.  We read in 9 bytes at a time
**  because the mappings are in chunks of 8 (+1 for the NULL at the end).  The
**  chunks are as follows:
**  
**  xxxx:xx (the first set of 4 is the codepoint, the second set is the ASCII
**  representation)
**  
**  We then convert and check these values before storing them in the
**  supplied array.
**  
**  @param fFile           the unicode map file pointer
**  @param iis_unicode_map the array to store the mappings in
**  
**  @return integer
**  
**  @retval HI_FATAL_ERR there was an error while parsing the file
**  @retval HI_SUCCESS   function was successful
*/
static int MapCodePoints(FILE *fFile, uint8_t *iis_unicode_map)
{
    char buffer[9];
    char *pcPtr;
    char *pcEnd;
    char *pcToken;
    char *pcCodePoint;
    char *pcAsciiMap;
    int  iCodePoint;
    int  iAsciiMap;
    

    /*
    **  We should now be pointing to the beginning of the codemap area for
    **  the selected codepage.
    */
    while(fgets(buffer, 9, fFile))
    {
        pcToken = strtok_r(buffer, CODEPAGE_SEPARATORS, &pcPtr);
        if(!pcToken)
        {
            return HI_SUCCESS;
        }

        pcCodePoint = strtok_r(pcToken, CODEPOINT_SEPARATORS, &pcPtr);
        if(!pcCodePoint)
            return HI_FATAL_ERR;

        pcAsciiMap = strtok_r(NULL, CODEPOINT_SEPARATORS, &pcPtr);
        if(!pcAsciiMap)
            return HI_FATAL_ERR;

        iCodePoint = strtol(pcCodePoint, &pcEnd, 16);
        if(*pcEnd)
        {
            return HI_FATAL_ERR;
        }

        if(iCodePoint < 0 || iCodePoint > 65535)
        {
            return HI_FATAL_ERR;
        }

        iAsciiMap = strtol(pcAsciiMap, &pcEnd, 16);
        if(*pcEnd)
        {
            return HI_FATAL_ERR;
        }

        if(iAsciiMap < 0 || iAsciiMap > 0x7f)
        {
            return HI_FATAL_ERR;
        }

        iis_unicode_map[iCodePoint] = iAsciiMap;

        //printf("** iis_unicode_map[%s] = %s\n", pcCodePoint, pcAsciiMap);
        //printf("** iis_unicode_map[%.2x] = %.2x\n", iCodePoint, 
        //       (u_char)iAsciiMap);
    }

    return HI_FATAL_ERR;
}

/*
**  NAME
**    hi_ui_parse_iis_unicode_map::
*/
/**
**  Parses an IIS Unicode Map file and store in the supplied array.
**  
**  This routine allocates the necessary memory to store the array values
**  in, and parses the supplied filename.
**  
**  @param iis_unicode_map  double pointer so we can allocate the memory
**  @param filename         the name of the file to open and parse
**  @param iCodePage        the codpage number to read the mappings from
**  
**  @return integer
**  
**  @retval HI_INVALID ARG     invalid argument
**  @retval HI_MEM_ALLOC_FAIL  memory allocation failed
**  @retval HI_INVALID_FILE    Could not open the supplied filename
**  @retval HI_SUCCESS         function was successful
*/
int hi_ui_parse_iis_unicode_map(uint8_t **iis_unicode_map, char *filename,
                                int iCodePage)
{
    int  iRet;
    FILE *fFile;

    if(!filename || iCodePage < 0)
    {
        return HI_INVALID_ARG;
    }

    fFile = fopen(filename, "r");
    if(fFile == NULL)
    {
        /*
        **  Couldn't open the file
        */
        return HI_INVALID_FILE;
    }

    *iis_unicode_map = (uint8_t *)xmalloc(sizeof(uint8_t) * 65536);
    if(*iis_unicode_map == NULL)
    {   
        fclose(fFile);
        return HI_MEM_ALLOC_FAIL;
    }

    memset(*iis_unicode_map, HI_UI_NON_ASCII_CODEPOINT, (sizeof(uint8_t)*65536));

    /*
    **  Find the correct codepage
    */
    iRet = FindCodePage(fFile, iCodePage);
    if (iRet)
    {
        //printf("** Did not find codepage\n");
        fclose(fFile);
        return iRet;
    }

    iRet = MapCodePoints(fFile, *iis_unicode_map);
    if (iRet)
    {
        //printf("** Error while parsing codepage.\n");
        fclose(fFile);
        return iRet;
    }

    fclose(fFile);
    return HI_SUCCESS;
}
