/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2007-2013 Sourcefire, Inc.
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

/**
**  @file        sp_cvs.h
**
**  @author      Taimur Aslam
**  @author      Todd Wease
** 
**  @brief       Decode and detect CVS vulnerabilities
**
**  This CVS detection plugin provides support for detecting published CVS vulnerabilities. The
**  vulnerabilities that can be detected are:
**  Bugtraq-10384, CVE-2004-0396: "Malformed Entry Modified and Unchanged flag insertion"
**
**  Detection Functions:
**
**  cvs: invalid-entry;
**
*/


#ifndef __SP_CVS_H__
#define __SP_CVS_H__


/* macros */
#define CVS_CONFIG_DELIMITERS  " \t\n"

#define CVS_COMMAND_DELIMITER  '\n'
#define CVS_COMMAND_SEPARATOR  ' '

#define CVS_CONF_INVALID_ENTRY_STR  "invalid-entry"

#define CVS_NO_ALERT  0
#define CVS_ALERT     1

#define CVS_ENTRY_STR  "Entry"
#define CVS_ENTRY_VALID   0
#define CVS_ENTRY_INVALID 1


/* the types of vulnerabilities it will detect */
typedef enum _CvsTypes
{
    CVS_INVALID_ENTRY = 1,
    CVS_END_OF_ENUM
    
} CvsTypes;


/* encapsulate the rule option */
typedef struct _CvsRuleOption
{
    CvsTypes type;

} CvsRuleOption;


/* represents a CVS command with argument */
typedef struct _CvsCommand
{
    const uint8_t  *cmd_str;        /* command string */
    int              cmd_str_len;
    const uint8_t  *cmd_arg;        /* command argument */
    int              cmd_arg_len;
    
} CvsCommand;


/* global function prototypes */
void SetupCvs(void);
uint32_t CvsHash(void *d);
int CvsCompare(void *l, void *r);

#endif

