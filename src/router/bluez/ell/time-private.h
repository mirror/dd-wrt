/*
 * Embedded Linux library
 * Copyright (C) 2020  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

struct timeval;

uint64_t _time_pick_interval_secs(uint32_t min_secs, uint32_t max_secs);
uint64_t _time_fuzz_msecs(uint64_t ms);
uint64_t _time_fuzz_secs(uint32_t secs, uint32_t max_offset);
uint64_t _time_realtime_to_boottime(const struct timeval *ts);
uint64_t time_realtime_now(void);
uint64_t _time_from_timespec(const struct timespec *ts);
