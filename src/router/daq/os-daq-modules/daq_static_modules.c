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

#include "daq_static_modules.h"

const DAQ_Module_t *static_modules[] = 
{
#ifdef BUILD_AFPACKET_MODULE
    &afpacket_daq_module_data,
#endif
#ifdef BUILD_DUMP_MODULE
    &dump_daq_module_data,
#endif
#ifdef BUILD_IPFW_MODULE
    &ipfw_daq_module_data,
#endif
#ifdef BUILD_IPQ_MODULE
    &ipq_daq_module_data,
#endif
#ifdef BUILD_NFQ_MODULE
    &nfq_daq_module_data,
#endif
#ifdef BUILD_PCAP_MODULE
    &pcap_daq_module_data,
#endif
#ifdef BUILD_NETMAP_MODULE
    &netmap_daq_module_data,
#endif
};
const int num_static_modules = sizeof(static_modules) / sizeof(static_modules[0]);
