/*
**
**  reg_test.h
**
**  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License Version 2 as
**  published by the Free Software Foundation.  You may not use, modify or
**  distribute this program under any other version of the GNU General
**  Public License.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
*/

#ifndef __REG_TEST_H__
#define __REG_TEST_H__

#ifdef REG_TEST
#define REG_TEST_VARIABLE   "SNORT_REG_TEST"
#define REG_TEST_EMAIL_VARIABLE "SNORT_EMAIL_REG_TEST"

#include <stdint.h>

#define REG_TEST_FLAG_SESSION_RELOAD            (1 << 0)
#define REG_TEST_FLAG_INCREMENT_IP_ADDRESS      (1 << 1)
#define REG_TEST_FLAG_RELOAD                    (1 << 2)
#define REG_TEST_FLAG_PERFMON_RELOAD            (1 << 3)
#define REG_TEST_FLAG_APPDATA_ADJUSTER_RELOAD   (1 << 4)
#define REG_TEST_FLAG_FILE_CACHE                (1 << 5)
#define REG_TEST_FLAG_PORTSCAN_RELOAD           (1 << 6)
#define REG_TEST_FLAG_SESSION_FORCE_RELOAD      (1 << 7)
#define REG_TEST_FLAG_REPUTATION                (1 << 8)
#define REG_TEST_FLAG_STREAM_DECODE             (1 << 9)

#define REG_TEST_EMAIL_FLAG_MIME_MEMPOOL_ADJUST  (1 << 0)
#define REG_TEST_EMAIL_FLAG_LOG_MEMPOOL_ADJUST   (1 << 1)
#define REG_TEST_EMAIL_FLAG_DECODE_DEPTH_ADJUST  (1 << 2)
#define REG_TEST_EMAIL_FLAG_FD_MEMPOOL_ADJUST    (1 << 3)
#define REG_TEST_EMAIL_FLAG_GZIP_MEMPOOL_ADJUST  (1 << 4)
#define REG_TEST_EMAIL_FLAG_HTTP_MEMPOOL_ADJUST  (1 << 5)

extern uint32_t rt_ip_increment;

uint64_t getRegTestFlags(void);
uint64_t getRegTestFlagsForEmail(void);
void regTestCheckIPIncrement(void);

#endif

#endif

