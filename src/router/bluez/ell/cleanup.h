/*
 * Embedded Linux library
 * Copyright (C) 2021  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#define __L_AUTODESTRUCT(func)				\
	__attribute((cleanup(_l_ ## func ## _cleanup)))

#define DEFINE_CLEANUP_FUNC(func)			\
	inline __attribute__((always_inline))		\
	void _l_ ## func ## _cleanup(void *p) { func(*(void **) p); }
