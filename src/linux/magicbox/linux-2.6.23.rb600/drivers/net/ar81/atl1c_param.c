/*
 * Copyright(c) 2007 Atheros Corporation. All rights reserved.
 *
 * Derived from Intel e1000 driver
 * Copyright(c) 1999 - 2005 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <linux/netdevice.h>

#include "atl1c.h"

/* This is the only thing that needs to be changed to adjust the
 * maximum number of ports that the driver can manage.
 */

/* Transmit Memory count
 *
 * Valid Range: 64-2048
 *
 * Default Value: 128
 */
#define ATL1C_MIN_TX_DESC_CNT		32
#define ATL1C_MAX_TX_DESC_CNT		1024
#define ATL1C_DEFAULT_TX_DESC_CNT	1024	
AT_PARAM(tx_desc_cnt, "Transmit description count");

/* Receive Memory Block Count
 *
 * Valid Range: 16-512
 *
 * Default Value: 128
 */
#define ATL1C_MIN_RX_MEM_SIZE		128 
#define ATL1C_MAX_RX_MEM_SIZE		1024
#define ATL1C_DEFAULT_RX_MEM_SIZE	512	
AT_PARAM(rx_mem_size, "memory size of rx buffer(KB)");

extern int media_type[];
extern int num_media_type;
//AT_PARAM(media_type, "MediaType Select");

/* Interrupt Moderate Timer in units of 2 us
 *
 * Valid Range: 10-65535
 *
 * Default Value: 45000(90ms)
 */
//AT_PARAM(int_mod_timer, "Interrupt Moderator Timer");
extern int int_mod_timer[];
extern int num_int_mod_timer;

#define AUTONEG_ADV_DEFAULT  0x2F
#define AUTONEG_ADV_MASK     0x2F
#define FLOW_CONTROL_DEFAULT FLOW_CONTROL_FULL

#define FLASH_VENDOR_DEFAULT    0
#define FLASH_VENDOR_MIN        0
#define FLASH_VENDOR_MAX        2

struct atl1c_option {
	enum { enable_option, range_option, list_option } type;
	char *name;
	char *err;
	int  def;
	union {
		struct { /* range_option info */
			int min;
			int max;
		} r;
		struct { /* list_option info */
			int nr;
			struct atl1c_opt_list { int i; char *str; } *p;
		} l;
	} arg;
};

static int __devinit atl1c_validate_option(int *value, struct atl1c_option *opt, struct pci_dev *pdev)
{
	if (*value == OPTION_UNSET) {
		*value = opt->def;
		return 0;
	}

	switch (opt->type) {
	case enable_option:
		switch (*value) {
		case OPTION_ENABLED:
			dev_info(&pdev->dev, "%s Enabled\n", opt->name);
			return 0;
		case OPTION_DISABLED:
			dev_info(&pdev->dev, "%s Disabled\n", opt->name);
			return 0;
		}
		break;
	case range_option:
		if (*value >= opt->arg.r.min && *value <= opt->arg.r.max) {
			dev_info(&pdev->dev, "%s set to %i\n", opt->name, *value);
			return 0;
		}
		break;
	case list_option:{
			int i;
			struct atl1c_opt_list *ent;

			for (i = 0; i < opt->arg.l.nr; i++) {
				ent = &opt->arg.l.p[i];
				if (*value == ent->i) {
					if (ent->str[0] != '\0')
						dev_info(&pdev->dev, "%s\n",
							ent->str);
					return 0;
				}
			}
			break;
		}
	default:
		BUG();
	}

	dev_info(&pdev->dev, "Invalid %s specified (%i) %s\n",
			opt->name, *value, opt->err);
	*value = opt->def;
	return -1;
}

/*
 * atl1c_check_options - Range Checking for Command Line Parameters
 * @adapter: board private structure
 *
 * This routine checks all command line parameters for valid user
 * input.  If an invalid value is given, or if no user specified
 * value exists, a default value is used.  The final value is stored
 * in a variable in the adapter structure.
 */
void __devinit atl1c_check_options(struct atl1c_adapter *adapter)
{
	struct pci_dev *pdev = adapter->pdev;
	int bd = adapter->bd_number;
	if (bd >= AT_MAX_NIC) {
		dev_warn(&pdev->dev, "no configuration for board #%i\n", bd);
		dev_warn(&pdev->dev, "Using defaults for all values\n");
	}

	{ 		/* Transmit Ring Size */
		struct atl1c_option opt = {
			.type = range_option,
			.name = "Transmit Ddescription Count",
			.err  = "using default of "
				__MODULE_STRING(ATL1C_DEFAULT_TX_DESC_CNT),
			.def  = ATL1C_DEFAULT_TX_DESC_CNT,
			.arg  = { .r = { .min = ATL1C_MIN_TX_DESC_CNT,
					 .max = ATL1C_MAX_TX_DESC_CNT} }
		};
		int val;
		if (num_tx_desc_cnt > bd) {
			val = tx_desc_cnt[bd];
			atl1c_validate_option(&val, &opt, pdev);
			adapter->tpd_ring[0].count = (u16) val & 0xFFFC;
		} else
			adapter->tpd_ring[0].count = (u16)opt.def;
	}

	{ 		/* Receive Memory Block Count */
		struct atl1c_option opt = {
			.type = range_option,
			.name = "Memory size of rx buffer(KB)",
			.err  = "using default of "
				__MODULE_STRING(ATL1C_DEFAULT_RX_MEM_SIZE),
			.def  = ATL1C_DEFAULT_RX_MEM_SIZE,
			.arg  = { .r = { .min = ATL1C_MIN_RX_MEM_SIZE,
					 .max = ATL1C_MAX_RX_MEM_SIZE} }
		};
		int val;
		if (num_rx_mem_size > bd) {
			val = rx_mem_size[bd];
			atl1c_validate_option(&val, &opt, pdev);
			adapter->rfd_ring[0].count = (u32)val;
		} else {
			adapter->rfd_ring[0].count = (u32)opt.def;
		}
	}

	{ 		/* Interrupt Moderate Timer */
		struct atl1c_option opt = {
			.type = range_option,
			.name = "Interrupt Moderate TX Timer",
			.err  = "using default of "
				__MODULE_STRING(INT_MOD_DEFAULT_CNT),
			.def  = INT_MOD_DEFAULT_CNT * 5, 
			.arg  = { .r = { .min = INT_MOD_MIN_CNT,
					 .max = INT_MOD_MAX_CNT} }
		} ;
		int val;
		if (num_int_mod_timer > bd) {
			val = int_mod_timer[bd];
			atl1c_validate_option(&val, &opt, pdev);
			adapter->hw.tx_imt = (u16) val;
		} else
			adapter->hw.tx_imt = (u16)(opt.def);
	}
	{ 		/* Interrupt Moderate Timer */
		struct atl1c_option opt = {
			.type = range_option,
			.name = "Interrupt Moderate RX Timer",
			.err  = "using default of "
				__MODULE_STRING(INT_MOD_DEFAULT_CNT),
			.def  = INT_MOD_DEFAULT_CNT,
			.arg  = { .r = { .min = INT_MOD_MIN_CNT,
					 .max = INT_MOD_MAX_CNT} }
		} ;
		int val;
		if (num_int_mod_timer > bd) {
			val = int_mod_timer[bd];
			atl1c_validate_option(&val, &opt, pdev);
			adapter->hw.rx_imt = (u16) val;
		} else
			adapter->hw.rx_imt = (u16)(opt.def);
	}


	
	{ 		/* MediaType */
		struct atl1c_option opt = {
			.type = range_option,
			.name = "Speed/Duplex Selection",
			.err  = "using default of "
				__MODULE_STRING(MEDIA_TYPE_AUTO_SENSOR),
			.def  = MEDIA_TYPE_AUTO_SENSOR,
			.arg  = { .r = { .min = MEDIA_TYPE_AUTO_SENSOR,
					 .max = MEDIA_TYPE_10M_HALF} }
		} ;
		int val;
		if (num_media_type > bd) {
			val = media_type[bd];
			atl1c_validate_option(&val, &opt, pdev);
			adapter->hw.media_type = (u16) val;
		} else
			adapter->hw.media_type = (u16)(opt.def);

	}
}
