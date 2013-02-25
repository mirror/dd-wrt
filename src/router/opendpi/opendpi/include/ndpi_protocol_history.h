/*
 * ndpi_protocol_history.h
 *
 * Copyright (C) 2009-2011 by ipoque GmbH
 * Copyright (C) 2011-13 - ntop.org
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



#ifndef NDPI_PROTOCOL_HISTORY_H
#define NDPI_PROTOCOL_HISTORY_H

typedef enum {
  NDPI_REAL_PROTOCOL = 0,
  NDPI_CORRELATED_PROTOCOL = 1
} ndpi_protocol_type_t;

void ndpi_int_add_connection(struct ndpi_detection_module_struct *ndpi_struct,                             
                             struct ndpi_flow_struct *flow,
                             u_int16_t detected_protocol, ndpi_protocol_type_t protocol_type);

#endif
