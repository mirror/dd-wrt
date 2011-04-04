/*
 * ipq_public_functions.h
 * Copyright (C) 2009-2010 by ipoque GmbH
 * 
 * This file is part of OpenDPI, an open source deep packet inspection
 * library based on the PACE technology by ipoque GmbH
 * 
 * OpenDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * OpenDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenDPI.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


#ifndef __IPOQUE_API_INCLUDE_FILE__
#error CANNOT INCLUDE THIS .H FILE, INCLUDE IPQ_API.H
#endif

#ifndef __IPQ_PUBLIC_FUNCTIONS_H__
#define __IPQ_PUBLIC_FUNCTIONS_H__

#ifdef __cplusplus
extern "C" {
#endif
	typedef enum {

		IPQ_LOG_ERROR,
		IPQ_LOG_TRACE,
		IPQ_LOG_DEBUG
	} ipq_log_level_t;
	typedef void (*ipoque_debug_function_ptr) (u32 protocol,
											   void *module_struct, ipq_log_level_t log_level, const char *format, ...);

	u32 ipoque_detection_get_sizeof_ipoque_flow_struct(void);

	u32 ipoque_detection_get_sizeof_ipoque_id_struct(void);


	struct ipoque_detection_module_struct *ipoque_init_detection_module(u32 ticks_per_second, void
																		*(*ipoque_malloc)
																		 (unsigned
																		  long size),
																		ipoque_debug_function_ptr ipoque_debug_printf);
	void
	 ipoque_exit_detection_module(struct ipoque_detection_module_struct
								  *ipoque_struct, void (*ipoque_free) (void *ptr));

	void
	 ipoque_set_protocol_detection_bitmask2(struct
											ipoque_detection_module_struct
											*ipoque_struct, const IPOQUE_PROTOCOL_BITMASK * detection_bitmask);
	unsigned int
	 ipoque_detection_process_packet(struct ipoque_detection_module_struct
									 *ipoque_struct, void *flow,
									 const unsigned char *packet,
									 const unsigned short packetlen,
									 const IPOQUE_TIMESTAMP_COUNTER_SIZE current_tick, void *src, void *dst);
#ifdef __cplusplus
}
#endif
#endif
