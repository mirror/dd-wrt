/*
 * ipq_debug_functions.h
 * Copyright (C) 2009-2011 by ipoque GmbH
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

#ifndef __IPQ_DEBUG_FUNCTIONS_H__
#define __IPQ_DEBUG_FUNCTIONS_H__

#ifdef __cplusplus
extern "C" {
#endif
#ifdef IPOQUE_ENABLE_DEBUG_MESSAGES
	void ipoque_debug_get_last_log_function_line(struct
												 ipoque_detection_module_struct
												 *ipoque_struct, const char **file, const char **func, u32 * line);
#endif
#ifdef __cplusplus
}
#endif
#endif
