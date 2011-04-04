/* 
 * xt_opendpi.h
 * Copyright (C) 2010 Gerardo E. Gidoni <gerel@gnu.org>
 * 
 * This file is part of OpenDPI, an open source deep packet inspection
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

#ifndef _LINUX_NETFILTER_XT_OPENDPI_H
#define _LINUX_NETFILTER_XT_OPENDPI_H 1

#include <linux/netfilter.h>
#include "ipq_api.h"

struct xt_opendpi_mtinfo {
        IPOQUE_PROTOCOL_BITMASK flags;
};

#endif /* _LINUX_NETFILTER_XT_OPENDPI_H */
