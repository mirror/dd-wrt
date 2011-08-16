/*
** Copyright (C) 2005-2011 Sourcefire, Inc.
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*
 * sp_urilen_check.h: Structure definitions/function prototype(s)
 * 		      for the URI length detection plugin.
 */

/* $Id */

#ifndef SP_URILEN_CHECK_H
#define SP_URILEN_CHECK_H

/* Structure stored in the rule OTN struct for use by URILEN 
 * detection plugin code.
 */
typedef struct _UriLenCheckData 
{
    int urilen;
    int urilen2;
    char oper;
} UriLenCheckData;

#define URILEN_CHECK_EQ 1
#define URILEN_CHECK_GT 2
#define URILEN_CHECK_LT 3
#define URILEN_CHECK_RG 4

/* 
 * Structure stored in the rule OTN struct for use by URINORMLEN
 * detection plugin code.
 */
typedef struct _UriNormLenCheckData
{
    int urinormlen;
    int urinormlen2;
} UriNormLenCheckData;


extern void SetupUriLenCheck(void);
uint32_t UriLenCheckHash(void *d);
int UriLenCheckCompare(void *l, void *r);

#endif /* SP_URILEN_CHECK_H */
