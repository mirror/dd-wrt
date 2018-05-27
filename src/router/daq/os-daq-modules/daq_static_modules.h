/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2010-2013 Sourcefire, Inc.
** Author: Michael R. Altizer <maltizer@sourcefire.com>
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

#include <daq.h>

#ifdef BUILD_AFPACKET_MODULE
extern const DAQ_Module_t afpacket_daq_module_data;
#endif
#ifdef BUILD_DUMP_MODULE
extern const DAQ_Module_t dump_daq_module_data;
#endif
#ifdef BUILD_IPFW_MODULE
extern const DAQ_Module_t ipfw_daq_module_data;
#endif
#ifdef BUILD_IPQ_MODULE
extern const DAQ_Module_t ipq_daq_module_data;
#endif
#ifdef BUILD_NFQ_MODULE
extern const DAQ_Module_t nfq_daq_module_data;
#endif
#ifdef BUILD_PCAP_MODULE
extern const DAQ_Module_t pcap_daq_module_data;
#endif
#ifdef BUILD_NETMAP_MODULE
extern const DAQ_Module_t netmap_daq_module_data;
#endif
