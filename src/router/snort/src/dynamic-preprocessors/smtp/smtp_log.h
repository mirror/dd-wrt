/****************************************************************************
 * 
 * Copyright (C) 2005-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/

/**************************************************************************
 *
 * smtp_log.h
 *
 * Author: Andy Mullican
 *
 **************************************************************************/

#ifndef __SMTP_LOG_H__
#define __SMTP_LOG_H__


#define GENERATOR_SMTP  124

/* Events for SMTP */
#define SMTP_COMMAND_OVERFLOW       1
#define SMTP_DATA_HDR_OVERFLOW      2
#define SMTP_RESPONSE_OVERFLOW      3
#define SMTP_SPECIFIC_CMD_OVERFLOW  4
#define SMTP_UNKNOWN_CMD            5
#define SMTP_ILLEGAL_CMD            6
#define SMTP_HEADER_NAME_OVERFLOW   7
#define SMTP_XLINK2STATE_OVERFLOW   8

#define SMTP_EVENT_MAX  9

/* Messages for each event */
#define SMTP_COMMAND_OVERFLOW_STR        "(smtp) Attempted command buffer overflow"
#define SMTP_DATA_HDR_OVERFLOW_STR       "(smtp) Attempted data header buffer overflow"
#define SMTP_RESPONSE_OVERFLOW_STR       "(smtp) Attempted response buffer overflow"
#define SMTP_SPECIFIC_CMD_OVERFLOW_STR   "(smtp) Attempted specific command buffer overflow"
#define SMTP_UNKNOWN_CMD_STR             "(smtp) Unknown command"
#define SMTP_ILLEGAL_CMD_STR             "(smtp) Illegal command"
#define SMTP_HEADER_NAME_OVERFLOW_STR    "(smtp) Attempted header name buffer overflow"
#define SMTP_XLINK2STATE_OVERFLOW_STR    "(smtp) Attempted X-Link2State command buffer overflow"

#define EVENT_STR_LEN  256


/* Function prototypes  */
void SMTP_GenerateAlert(int, char *, ...);


#endif

