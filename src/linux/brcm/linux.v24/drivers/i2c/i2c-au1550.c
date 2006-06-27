/*
 * i2c-au1550.c: SMBus (i2c) adapter for Alchemy PSC interface
 * Copyright (C) 2004 Embedded Edge, LLC <dan@embeddededge.com>
 *
 * This is just a skeleton adapter to use with the Au1550 PSC
 * algorithm.  It was developed for the Pb1550, but will work with
 * any Au1550 board that has a similar PSC configuration.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>

#include <asm/au1000.h>
#include <asm/au1xxx_psc.h>
#if defined( CONFIG_MIPS_PB1550 )
	#include <asm/pb1550.h>
#endif
#if defined( CONFIG_MIPS_PB1200 )
	#include <asm/pb1200.h>
#endif
#if defined( CONFIG_MIPS_DB1200 )
	#include <asm/db1200.h>
#endif
#if defined( CONFIG_MIPS_FICMMP )
	#include <asm/ficmmp.h>
#endif

#include <linux/i2c.h>
#include <linux/i2c-algo-au1550.h>



static int
pb1550_reg(struct i2c_client *client)
{
	return 0;
}

static int
pb1550_unreg(struct i2c_client *client)
{
	return 0;
}

static void
pb1550_inc_use(struct i2c_adapter *adap)
{
#ifdef MODULE
	MOD_INC_USE_COUNT;
#endif
}

static void
pb1550_dec_use(struct i2c_adapter *adap)
{
#ifdef MODULE
	MOD_DEC_USE_COUNT;
#endif
}

static struct i2c_algo_au1550_data pb1550_i2c_info = {
	SMBUS_PSC_BASE, 200, 200
};

static struct i2c_adapter pb1550_board_adapter = {
	name:              "pb1550 adapter",
	id:                I2C_HW_AU1550_PSC,
	algo:              NULL,
	algo_data:         &pb1550_i2c_info,
	inc_use:           pb1550_inc_use,
	dec_use:           pb1550_dec_use,
	client_register:   pb1550_reg,
	client_unregister: pb1550_unreg,
	client_count:      0,
};

int __init
i2c_pb1550_init(void)
{
	/* This is where we would set up a 50MHz clock source
	 * and routing.  On the Pb1550, the SMBus is PSC2, which
	 * uses a shared clock with USB.  This has been already
	 * configured by Yamon as a 48MHz clock, close enough
	 * for our work.
	 */
        if (i2c_au1550_add_bus(&pb1550_board_adapter) < 0)
                return -ENODEV;

	return 0;
}

/* BIG hack to support the control interface on the Wolfson WM8731
 * audio codec on the Pb1550 board.  We get an address and two data
 * bytes to write, create an i2c message, and send it across the
 * i2c transfer function.  We do this here because we have access to
 * the i2c adapter structure.
 */
static struct i2c_msg wm_i2c_msg;  /* We don't want this stuff on the stack */
static	u8 i2cbuf[2];

int
pb1550_wm_codec_write(u8 addr, u8 reg, u8 val)
{
	wm_i2c_msg.addr = addr;
	wm_i2c_msg.flags = 0;
	wm_i2c_msg.buf = i2cbuf;
	wm_i2c_msg.len = 2;
	i2cbuf[0] = reg;
	i2cbuf[1] = val;

	return pb1550_board_adapter.algo->master_xfer(&pb1550_board_adapter, &wm_i2c_msg, 1);
}

/* the next function is needed by DVB driver. */
int pb1550_i2c_xfer(struct i2c_msg msgs[], int num)
{
    return pb1550_board_adapter.algo->master_xfer(&pb1550_board_adapter, msgs, num);
}

EXPORT_SYMBOL(pb1550_wm_codec_write);
EXPORT_SYMBOL(pb1550_i2c_xfer);

MODULE_AUTHOR("Dan Malek, Embedded Edge, LLC.");
MODULE_DESCRIPTION("SMBus adapter Alchemy pb1550");
MODULE_LICENSE("GPL");

int
init_module(void)
{
	return i2c_pb1550_init();
}

void
cleanup_module(void)
{
	i2c_au1550_del_bus(&pb1550_board_adapter);
}
