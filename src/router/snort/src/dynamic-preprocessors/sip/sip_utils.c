/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
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
 ****************************************************************************
 * Provides convenience functions.
 *
 * 2/17/2011 - Initial implementation ... Hui Cao <hcao@sourcefire.com>
 *
 ****************************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "sf_types.h"
#include "sip_utils.h"

/********************************************************************
 * Function: SIP_IsEmptyStr()
 *
 * Checks if string is NULL, empty or just spaces.
 * String must be 0 terminated.
 *
 * Arguments:
 *  char * - string to check
 *
 * Returns:
 *  1  if string is NULL, empty or just spaces
 *  0  otherwise
 *
 ********************************************************************/
int SIP_IsEmptyStr(char *str)
{
	char *end;

	if (str == NULL)
		return 1;

	end = str + strlen(str);

	while ((str < end) && isspace((int)*str))
		str++;

	if (str == end)
		return 1;

	return 0;
}
/*
 * Trim spaces non-destructively on both sides of string : '', \t, \n, \r
 * If string is empty return 0, otherwise 1
 * Note: end point to the location start + length,
 *       not necessary the real end of string if not end with \0
 */
int SIP_TrimSP(const char *start, const char *end, char **new_start, char** new_end)
{
	char *before;
	char *after;

	if (start >= end )
	{
		*new_start = (char *)start;
		*new_end = *new_start;
		return 0;
	}

	before = (char *) start;

	// Trim the starting spaces
	while((before < end) && isspace((int)*before))
	{
		before++;
	}
	// This is an empty string
	if (before == end)
	{
		*new_start = (char *)end;
		*new_end = *new_start;
		return 0;
	}

	// Trim the ending spaces
	after = (char *) end - 1;
	while((before < after) && isspace((int)*after))
	{
		after--;
	}
	*new_start = before;
	*new_end = after + 1;
    return 1;
}
/********************************************************************
 * Function: SIP_FindMethod()
 *
 * Find method in the method list by name
 *
 * Arguments:
 *  SIPMethodlist - methods list to be searched,
 *  char *        - method name,
 *  int           - length of the method name
 *
 * Returns:
 *  SIPMethodNode*- the founded method node, or NULL if not founded
 *
 ********************************************************************/

SIPMethodNode* SIP_FindMethod(SIPMethodlist methods, char* methodName, unsigned int length)
{
	SIPMethodNode* method = NULL;

	method = methods;
	while (NULL != method)
	{

		if ((length == strlen(method->methodName))&&
				(strncasecmp(method->methodName, methodName, length) == 0))
		{
			return method;
		}
		method = method->nextm;
	}
	return method;
}
/********************************************************************
 * Function: strToHash()
 *
 * Calculate the hash value of a string
 *
 * Arguments:
 *  char * - string to be hashed
 *  int: length of the string
 *
 * Returns:
 *  1  if string is NULL, empty or just spaces
 *  0  otherwise
 *
 ********************************************************************/
uint32_t strToHash(const char *str, int length )
{
	uint32_t a,b,c,tmp;
	int i,j,k,l;
	a = b = c = 0;
	for (i=0,j=0;i<length;i+=4)
	{
		tmp = 0;
		k = length - i;
		if (k > 4)
			k=4;

		for (l=0;l<k;l++)
		{
			tmp |= *(str + i + l) << l*8;
		}

		switch (j)
		{
		case 0:
			a += tmp;
			break;
		case 1:
			b += tmp;
			break;
		case 2:
			c += tmp;
			break;
		}
		j++;

		if (j == 3)
		{
			mix(a,b,c);
			j = 0;
		}
	}
	final(a,b,c);
	return c;
}
