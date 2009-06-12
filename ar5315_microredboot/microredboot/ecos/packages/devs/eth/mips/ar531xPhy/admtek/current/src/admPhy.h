//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Atheros Communications, Inc.
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
// Alternative licenses for eCos may be arranged by contacting the copyright
// holders.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Atheros Communications, Inc.
// Contributors: Atheros Engineering
// Date:         2003-10-22
// Purpose:      
// Description:  AR531X ethernet hardware driver
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/*
 * rtPhy.h - definitions for the ethernet PHY.
 * This code supports a simple 1-port ethernet phy, Realtek RTL8201BL,
 * and compatible PHYs, such as the Kendin KS8721B.
 * All definitions in this file are operating system independent!
 */

#ifndef ADMPHY_H
#define ADMPHY_H

/* MII Registers */

#define	GEN_ctl		00
#define	GEN_sts		01
#define	GEN_id_hi	02
#define	GEN_id_lo	03
#define	AN_adv		04
#define	AN_lpa		05
#define	AN_exp		06

/* GEN_ctl */ 
#define	PHY_SW_RST	0x8000
#define	LOOPBACK	0x4000
#define	SPEED		0x2000	/* 100 Mbit/s */
#define	AUTONEGENA	0x1000
#define	DUPLEX		0x0100	/* Duplex mode */

		
/* GEN_sts */
#define	AUTOCMPLT	0x0020	/* Autonegotiation completed */
#define	LINK		0x0004	/* Link status */

/* GEN_ids */
#define ADM_PHY_ID1_EXPECTATION  0x22

/* AN_lpa */
#define	LPA_TXFD	0x0100	/* Link partner supports 100 TX Full Duplex */
#define	LPA_TX		0x0080	/* Link partner supports 100 TX Half Duplex */
#define	LPA_10FD	0x0040	/* Link partner supports 10 BT Full Duplex */
#define	LPA_10		0x0020	/* Link partner supports 10 BT Half Duplex */

#endif /* ADMPHY_H */
