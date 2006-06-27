/*
 *  setup.c, Setup for the IBM WorkPad z50.
 *
 *  Copyright (C) 2002-2004  Yoichi Yuasa <yuasa@hh.iij4u.or.jp>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/config.h>
#include <linux/console.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/ioport.h>

#include <asm/time.h>
#include <asm/vr41xx/workpad.h>

void __init ibm_workpad_setup(void)
{
	set_io_port_base(IO_PORT_BASE);
	ioport_resource.start = IO_PORT_RESOURCE_START;
	ioport_resource.end = IO_PORT_RESOURCE_END;
	iomem_resource.start = IO_MEM_RESOURCE_START;
	iomem_resource.end = IO_MEM_RESOURCE_END;

	board_time_init = vr41xx_time_init;
	board_timer_setup = vr41xx_timer_setup;

#ifdef CONFIG_FB
	conswitchp = &dummy_con;
#endif

#if defined(CONFIG_IDE) || defined(CONFIG_IDE_MODULE)
	ide_ops = &vr41xx_ide_ops;
#endif

	vr41xx_bcu_init();
	vr41xx_cmu_init();
	vr41xx_pmu_init();

#ifdef CONFIG_SERIAL
	vr41xx_siu_init(SIU_RS232C, 0);
#endif
}
