// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Felix Fietkau <nbd@nbd.name>
 */
#ifndef __BRIDGER_UBUS_H
#define __BRIDGER_UBUS_H

#ifdef UBUS_SUPPORT
int bridger_ubus_init(void);
void bridger_ubus_stop(void);
bool bridger_ubus_dev_blacklisted(struct device *dev);
#else
static inline int bridger_ubus_init(void)
{
	return 0;
}
static inline void bridger_ubus_stop(void)
{
}
static inline bool bridger_ubus_dev_blacklisted(struct device *dev)
{
	return false;
}
#endif

#endif
