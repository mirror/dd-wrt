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
 *
 * There are a lot of defines in here that are unused and/or have cryptic
 * names.  Please leave them alone, as they're the closest thing we have
 * to a spec from Atheros at present. *ahem* -- CHS
 */
 
#include <linux/netdevice.h>
#include "atl1e.h"

/* This is the only thing that needs to be changed to adjust the
 * maximum number of ports that the driver can manage.
 */


/* Transmit Memory Size
 *
 * Valid Range: 64-2048 
 *
 * Default Value: 128
 */
#define AT_MIN_TX_RING_SZ       32
#define AT_MAX_TX_RING_SZ       1020            
#define AT_DEFAULT_TX_RING_SZ   128        	// 
AT_PARAM(TxRingSz, "Transmit Ring Sizen");

/* Receive Memory Block Count
 *
 * Valid Range: 16-512
 *
 * Default Value: 128
 */
#define AT_MIN_RXF_SZ        8    // 8KB
#define AT_MAX_RXF_SZ        1024 // 1MB
#define AT_DEFAULT_RXF_SZ    256  // 128KB
AT_PARAM(RxfMemSize, "memory size of rx buffer(KB)");

/* User Specified media_type Override
 *
 * Valid Range: 0-5
 *  - 0    - auto-negotiate at all supported speeds
 *  - 1    - only link at 1000Mbps FULL Duplex
 *  - 2    - only link at 100Mbps Full Duplex
 *  - 3    - only link at 100Mbps Half Duplex
 *  - 4    - only link at 10Mbps Full Duplex
 *  - 5    - only link at 10Mbps Half Duplex
 * Default Value: 0
 */
AT_PARAM(media_type, "media_type Select");
/* Interrupt Moderate Timer in units of 2 us
 *
 * Valid Range: 10-65535
 *
 * Default Value: 45000(90ms)
 */
AT_PARAM(int_mod_timer, "Interrupt Moderator Timer");


#define AUTONEG_ADV_DEFAULT  0x2F
#define AUTONEG_ADV_MASK     0x2F
#define FLOW_CONTROL_DEFAULT FLOW_CONTROL_FULL



#define FLASH_VENDOR_DEFAULT    0
#define FLASH_VENDOR_MIN        0
#define FLASH_VENDOR_MAX        2


struct atl1e_option {
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
            struct atl1e_opt_list { int i; char *str; } *p;
        } l;
    } arg;
};

static int __devinit
atl1e_validate_option(int *value, struct atl1e_option *opt)
{
    if(*value == OPTION_UNSET) {
        *value = opt->def;
        return 0;
    }

    switch (opt->type) {
    case enable_option:
        switch (*value) {
        case OPTION_ENABLED:
            printk(KERN_INFO "%s Enabled\n", opt->name);
            return 0;
        case OPTION_DISABLED:
            printk(KERN_INFO "%s Disabled\n", opt->name);
            return 0;
        }
        break;
    case range_option:
        if(*value >= opt->arg.r.min && *value <= opt->arg.r.max) {
            printk(KERN_INFO "%s set to %i\n", opt->name, *value);
            return 0;
        }
        break;
    case list_option: {
        int i;
        struct atl1e_opt_list *ent;

        for(i = 0; i < opt->arg.l.nr; i++) {
            ent = &opt->arg.l.p[i];
            if(*value == ent->i) {
                if(ent->str[0] != '\0')
                    printk(KERN_INFO "%s\n", ent->str);
                return 0;
            }
        }
    }
        break;
    default:
        BUG();
    }

    printk(KERN_INFO "Invalid %s specified (%i) %s\n",
           opt->name, *value, opt->err);
    *value = opt->def;
    return -1;
}

/**
 * atl1e_check_options - Range Checking for Command Line Parameters
 * @adapter: board private structure
 *
 * This routine checks all command line parameters for valid user
 * input.  If an invalid value is given, or if no user specified
 * value exists, a default value is used.  The final value is stored
 * in a variable in the adapter structure.
 **/

void __devinit
atl1e_check_options(struct atl1e_adapter *adapter)
{
    int bd = adapter->bd_number;
    if(bd >= AT_MAX_NIC) {
        printk(KERN_NOTICE
               "Warning: no configuration for board #%i\n", bd);
        printk(KERN_NOTICE "Using defaults for all values\n");
#ifndef module_param_array
        bd = AT_MAX_NIC;
#endif
    }

    { /* Transmit Ring Size */
        struct atl1e_option opt = {
            .type = range_option,
            .name = "Transmit Ring Size",
            .err  = "using default of "
                __MODULE_STRING(AT_DEFAULT_TX_RING_SZ),
            .def  = AT_DEFAULT_TX_RING_SZ,
            .arg  = { .r = { .min = AT_MIN_TX_RING_SZ, .max = AT_MAX_TX_RING_SZ }}
        };
        int val;
#ifdef module_param_array
        if(num_TxRingSz > bd) {
#endif
            val = TxRingSz[bd];
            atl1e_validate_option(&val, &opt);
            adapter->tpd_ring_size = (u16) val & 0xFFFC;
#ifdef module_param_array
        } else {
            adapter->tpd_ring_size = (u16)opt.def;
        }
#endif
    }

    { /* Receive Memory Block Count */
        struct atl1e_option opt = {
            .type = range_option,
            .name = "memory size of rx buffer(KB)",
            .err  = "using default of "
                __MODULE_STRING(AT_DEFAULT_RXF_SZ),
            .def  = AT_DEFAULT_RXF_SZ,
            .arg  = { .r = { .min = AT_MIN_RXF_SZ, .max = AT_MAX_RXF_SZ }}
        };
        int val;
#ifdef module_param_array
        if(num_RxfMemSize > bd) {
#endif          
            val = RxfMemSize[bd];
            atl1e_validate_option(&val, &opt);
            adapter->rxf_length = (u32)val * 1024;
#ifdef module_param_array
        } else {
            adapter->rxf_length = (u32)opt.def * 1024;
        }
#endif

    }
    
    { /* Interrupt Moderate Timer */
        struct atl1e_option opt = { 
            .type = range_option,
            .name = "Interrupt Moderate Timer",
            .err  = "using default of " __MODULE_STRING(INT_MOD_DEFAULT_CNT),
            .def  = INT_MOD_DEFAULT_CNT,
            .arg  = { .r = { .min = INT_MOD_MIN_CNT, .max = INT_MOD_MAX_CNT }}
        } ;
        int val;
#ifdef module_param_array
        if(num_int_mod_timer > bd) {
#endif          
        val = int_mod_timer[bd];
        atl1e_validate_option(&val, &opt); 
        adapter->imt = (u16) val;   
#ifdef module_param_array
        } else {
            adapter->imt = (u16)(opt.def);
        }
#endif               
    }
    
    { /* media_type */
        struct atl1e_option opt = { 
	        .type = range_option,
	        .name = "Speed/Duplex Selection",
	        .err  = "using default of " __MODULE_STRING(MEDIA_TYPE_AUTO_SENSOR),
	        .def  = MEDIA_TYPE_AUTO_SENSOR,
		.arg  = { .r = { .min = MEDIA_TYPE_AUTO_SENSOR,
					 .max = MEDIA_TYPE_10M_HALF} }
	    } ;
        int val;
#ifdef module_param_array
	if(num_media_type > bd) {
#endif	        
        val = media_type[bd];
	atl1e_validate_option(&val, &opt);	
	adapter->hw.media_type = (u16) val;
#ifdef module_param_array
	} else {
	    adapter->hw.media_type = (u16)(opt.def);
	}
#endif	             
    }
}



