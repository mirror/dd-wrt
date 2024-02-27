/*
 This file is part of GNU libmicrohttpd
  Copyright (C) 2023 Evgeny Grin (Karlson2k)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file tools/mhd_tool_get_cpu_count.h
 * @brief  Declaration of functions to detect the number of available
 *         CPU cores.
 * @author Karlson2k (Evgeny Grin)
 */


#ifndef SRC_TOOLS_MHD_TOOL_GET_CPU_COUNT_H_
#define SRC_TOOLS_MHD_TOOL_GET_CPU_COUNT_H_ 1


/**
 * Detect the number of logical CPU cores available for the process.
 * The number of cores available for this process could be different from
 * value of cores available on the system. The OS may have limit on number
 * assigned/allowed cores for single process and process may have limited
 * CPU affinity.
 * @return the number of logical CPU cores available for the process or
 *         -1 if failed to detect
 */
int
mhd_tool_get_proc_cpu_count (void);


/**
 * Try to detect the number of logical CPU cores available for the system.
 * The number of available logical CPU cores could be changed any time due to
 * CPU hotplug.
 * @return the number of logical CPU cores available,
 *         -1 if failed to detect.
 */
int
mhd_tool_get_system_cpu_count (void);

#endif /* SRC_TOOLS_MHD_TOOL_GET_CPU_COUNT_H_ */
