/*
 * Copyright (C) 2006-2018, Marvell International Ltd.
 *
 * This software file (the "File") is distributed by Marvell International
 * Ltd. under the terms of the GNU General Public License Version 2, June 1991
 * (the "License").  You may use, redistribute and/or modify this File in
 * accordance with the terms and conditions of the License, a copy of which
 * is available by writing to the Free Software Foundation, Inc.
 *
 * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
 * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about
 * this warranty disclaimer.
 */

/* Description:  This file defines Linux thermal framework related functions. */

#ifndef _THERMAL_H_
#define _THERMAL_H_

#include <linux/kconfig.h>

#if IS_ENABLED(CONFIG_THERMAL)
int mwl_thermal_register(struct mwl_priv *priv);
void mwl_thermal_unregister(struct mwl_priv *priv);
void mwl_thermal_set_throttling(struct mwl_priv *priv);
#else
static inline int mwl_thermal_register(struct mwl_priv *priv)
{
	return 0;
}

static inline void mwl_thermal_unregister(struct mwl_priv *priv)
{
}

static inline void mwl_thermal_set_throttling(struct mwl_priv *priv)
{
}
#endif

#endif /* _THERMAL_H_ */
