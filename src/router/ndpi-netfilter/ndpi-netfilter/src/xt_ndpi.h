/* 
 * xt_ndpi.h
 * Copyright (C) 2010-2012 G. Elian Gidoni <geg@gnu.org>
 *               2012 Ed Wildgoose <lists@wildgooses.com>
 * 
 * This file is part of nDPI, an open source deep packet inspection
 * library based on the PACE technology by ipoque GmbH
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _LINUX_NETFILTER_XT_NDPI_H
#define _LINUX_NETFILTER_XT_NDPI_H 1

#include <linux/netfilter.h>
#include "ndpi_main.h"

#ifndef NDPI_BITMASK_IS_ZERO
#define NDPI_BITMASK_IS_ZERO(a) NDPI_BITMASK_IS_EMPTY(a)
#endif

struct xt_ndpi_mtinfo {
        NDPI_PROTOCOL_BITMASK flags;
	uint64_t    risk;
	uint32_t    invert:1,error:1,m_proto:1,p_proto:1,have_master:1,
		    host:1,re:1,empty:1,proto:1,inprogress:1,
		    ja3s:1,ja3c:1,tlsfp:1,tlsv:1,ja4c:1,
		    untracked:1,clevel:4,clevel_op:2;
	char	hostname[256 - sizeof(NDPI_PROTOCOL_BITMASK) - sizeof(uint32_t)-sizeof(void *)];
	void	*reg_data; // kernel only
};

struct xt_ndpi_tginfo {
       __u32 mark, mask;
       __u16 p_proto_id:1,m_proto_id:1,any_proto_id:1,
	     t_accept:1,t_mark:1,t_clsf:1,flow_yes:1,t_mark2:1;
};

#endif /* _LINUX_NETFILTER_XT_NDPI_H */
