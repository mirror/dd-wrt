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
 
#ifndef _PREPROC_IDS_H
#define _PREPROC_IDS_H

/*
**  Preprocessor Communication Defines
**  ----------------------------------
**  These defines allow preprocessors to be turned
**  on and off for each packet.  Preprocessors can be
**  turned off and on before preprocessing occurs and
**  during preprocessing.
**
**  Currently, the order in which the preprocessors are
**  placed in the snort.conf determine the order of 
**  evaluation.  So if one module wants to turn off
**  another module, it must come first in the order.
*/

// currently 32 bits (preprocessors)
// are available.

#define PP_BO                      0
//#define PP_DCERPC                  1
#define PP_DNS                     2
#define PP_FRAG3                   3
#define PP_FTPTELNET               4
#define PP_HTTPINSPECT             5
#define PP_PERFMONITOR             6
#define PP_RPCDECODE               7
#define PP_RULES                   8
#define PP_SFPORTSCAN              9
#define PP_SMTP                   10
#define PP_SSH                    11
#define PP_SSL                    12
#define PP_STREAM5                13
#define PP_TELNET                 14
#define PP_ARPSPOOF               15
#define PP_DCE2                   16
#define PP_SDF                    17
#define PP_NORMALIZE              18

// used externally
#define PP_ISAKMP                 19
#define PP_SKYPE                  20

#define PP_ALL_ON         0xFFFFFFFF
#define PP_ALL_OFF        0x00000000

#define PRIORITY_FIRST           0x0
#define PRIORITY_NORMALIZE       0x4
#define PRIORITY_NETWORK        0x10
#define PRIORITY_TRANSPORT     0x100
#define PRIORITY_TUNNEL        0x105
#define PRIORITY_SCANNER       0x110
#define PRIORITY_APPLICATION   0x200
#define PRIORITY_LAST         0xffff

#endif /* _PREPROC_IDS_H */

