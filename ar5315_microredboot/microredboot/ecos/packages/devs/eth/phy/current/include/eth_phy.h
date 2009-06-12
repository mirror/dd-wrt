#ifndef CYGONCE_DEVS_ETH_PHY_H_
#define CYGONCE_DEVS_ETH_PHY_H_
//==========================================================================
//
//      eth_phy.h
//
//      User API for ethernet transciever (PHY) support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Gary Thomas
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2003-08-01
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#define PHY_BIT_LEVEL_ACCESS_TYPE 0
#define PHY_REG_LEVEL_ACCESS_TYPE 1

// Physical device access - defined by hardware instance
typedef struct {
    int ops_type;  // 0 => bit level, 1 => register level
    bool init_done;
    void (*init)(void);
    void (*reset)(void);
    union {
        struct {
            void (*set_data)(int);
            int  (*get_data)(void);
            void (*set_clock)(int);
            void (*set_dir)(int);
        } bit_level_ops;
        struct {
            void (*put_reg)(int reg, int unit, unsigned short data);
            bool (*get_reg)(int reg, int unit, unsigned short *data);
        } reg_level_ops;
    } ops;
    int phy_addr;
    struct _eth_phy_dev_entry *dev;  // Chip access functions
} eth_phy_access_t;

#define ETH_PHY_BIT_LEVEL_ACCESS_FUNS(_l,_init,_reset,_set_data,_get_data,_set_clock,_set_dir) \
static eth_phy_access_t _l = {PHY_BIT_LEVEL_ACCESS_TYPE, false, _init, _reset, \
                              {.bit_level_ops = {_set_data, _get_data, _set_clock, _set_dir}}}

#define ETH_PHY_REG_LEVEL_ACCESS_FUNS(_l,_init,_reset,_put_reg,_get_reg) \
static eth_phy_access_t _l = {PHY_REG_LEVEL_ACCESS_TYPE, false, _init, _reset, \
                              {.reg_level_ops = {_put_reg, _get_reg}}}

#define ETH_PHY_STAT_LINK  0x0001   // Link up/down
#define ETH_PHY_STAT_100MB 0x0002   // Connection is 100Mb/10Mb
#define ETH_PHY_STAT_FDX   0x0004   // Connection is full/half duplex

externC bool _eth_phy_init(eth_phy_access_t *f);
externC void _eth_phy_reset(eth_phy_access_t *f);
externC int  _eth_phy_state(eth_phy_access_t *f);
externC int  _eth_phy_cfg(eth_phy_access_t *f, int mode);
// Internal routines
externC void _eth_phy_write(eth_phy_access_t *f, int reg, int unit, unsigned short data);
externC bool _eth_phy_read(eth_phy_access_t *f, int reg, int unit, unsigned short *val);

#endif  // CYGONCE_DEVS_ETH_PHY_H_
// ------------------------------------------------------------------------
