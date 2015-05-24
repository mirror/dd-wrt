/*
 * ndpi_protocols.h
 *
 * Copyright (C) 2011-15 - ntop.org
 * Copyright (C) 2009-2011 by ipoque GmbH
 *
 * This file is part of nDPI, an open source deep packet inspection
 * library based on the OpenDPI and PACE technology by ipoque GmbH
 *
 * nDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nDPI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef __NDPI_PROTOCOLS_INCLUDE_FILE__
#define __NDPI_PROTOCOLS_INCLUDE_FILE__

#include "ndpi_main.h"


void ndpi_bittorrent_init(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t size,u_int32_t timeout,int logsize);
void ndpi_bittorrent_done(struct ndpi_detection_module_struct *ndpi_struct);
int ndpi_bittorrent_gc(struct hash_ip4p_table *ht,int key,time_t now);

#endif /* __NDPI_PROTOCOLS_INCLUDE_FILE__ */
