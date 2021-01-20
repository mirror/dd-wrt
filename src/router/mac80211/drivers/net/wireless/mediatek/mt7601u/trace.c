// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2014 Felix Fietkau <nbd@openwrt.org>
 * Copyright (C) 2015 Jakub Kicinski <kubakici@wp.pl>
 */

#include <linux/module.h>
#if LINUX_VERSION_IS_LESS(3,4,0)
#include <linux/interrupt.h>
#endif

#ifndef __CHECKER__
#define CREATE_TRACE_POINTS
#include "trace.h"

#endif
