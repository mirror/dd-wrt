/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2003-2011 Sourcefire, Inc.
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
 
/* 
  Purpose: Enumerate all the various detection plugins entries for
           otn->ds_list[]

  No more grepping to make your own plugin!
*/

#ifndef _PLUGIN_ENUM_H
#define _PLUGIN_ENUM_H

enum {
    PLUGIN_CLIENTSERVER,
    PLUGIN_DSIZE_CHECK,
    PLUGIN_FRAG_BITS,
    PLUGIN_FRAG_OFFSET,
    PLUGIN_ICMP_CODE,
    PLUGIN_ICMP_ID_CHECK,
    PLUGIN_ICMP_SEQ_CHECK,
    PLUGIN_ICMP_TYPE,
    PLUGIN_IPOPTION_CHECK,
    PLUGIN_IP_ID_CHECK,
    PLUGIN_IP_PROTO_CHECK,
    PLUGIN_IP_SAME_CHECK,
    PLUGIN_IP_TOS_CHECK,
    PLUGIN_PATTERN_MATCH, /* AND match */
    PLUGIN_PATTERN_MATCH_OR, 
    PLUGIN_PATTERN_MATCH_URI,
    PLUGIN_RESPONSE,
    PLUGIN_RPC_CHECK,
    PLUGIN_SESSION,
    PLUGIN_TCP_ACK_CHECK,
    PLUGIN_TCP_FLAG_CHECK,
    PLUGIN_TCP_SEQ_CHECK,
    PLUGIN_TCP_WIN_CHECK,
    PLUGIN_TTL_CHECK,
    PLUGIN_BYTE_TEST,
    PLUGIN_PCRE,
    PLUGIN_URILEN_CHECK,
    PLUGIN_DYNAMIC,
    PLUGIN_FLOWBIT,
    PLUGIN_FILE_DATA,
    PLUGIN_BASE64_DECODE,
    PLUGIN_MAX  /* sentinel value */
};

#endif /* _PLUGIN_ENUM_H */

