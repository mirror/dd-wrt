/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 * Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#ifndef _LQ_PROM_H__
#define _LQ_PROM_H__

#define LQ_SYS_TYPE_LEN	0x100

struct lq_soc_info {
	unsigned char *name;
	unsigned int rev;
	unsigned int partnum;
	unsigned int type;
	unsigned char sys_type[LQ_SYS_TYPE_LEN];
};

void lq_soc_detect(struct lq_soc_info *i);

#endif
