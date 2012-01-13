/******************************************************************************

GPL LICENSE SUMMARY

  Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.

  This program is free software; you can redistribute it and/or modify 
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program; if not, write to the Free Software 
  Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
  The full GNU General Public License is included in this distribution 
  in the file called LICENSE.GPL.

  Contact Information:
  Intel Corporation

 version: Security.L.1.0.3-98
  
  Contact Information:
  
  Intel Corporation, 5000 W Chandler Blvd, Chandler, AZ 85226 

******************************************************************************/

/* 
 * gcu_reg.h
 * Macros and constants related to the registers available on the GCU
 */

#ifndef GCU_REG_H
#define GCU_REG_H

/* Register Offsets within memory map register space */
#define MDIO_STATUS_REG    0x00000010UL
#define MDIO_COMMAND_REG   0x00000014UL

/* MDIO_STATUS_REG fields */
#define MDIO_STATUS_STATUS_MASK     0x80000000UL  /* bit 31 = 1 on error */
#define MDIO_STATUS_READ_DATA_MASK  0x0000FFFFUL

/* MDIO_COMMAND_REG fields */
#define MDIO_COMMAND_GO_MASK         0x80000000UL /* bit 31 = 1 during read or
                                                   * write, 0 on completion */
#define MDIO_COMMAND_OPER_MASK       0x04000000UL /* bit = 1 is  a write */
#define MDIO_COMMAND_PHY_ADDR_MASK   0x03E00000UL
#define MDIO_COMMAND_PHY_REG_MASK    0x001F0000UL
#define MDIO_COMMAND_WRITE_DATA_MASK 0x0000FFFFUL

#define MDIO_COMMAND_GO_OFFSET         31
#define MDIO_COMMAND_OPER_OFFSET       26
#define MDIO_COMMAND_PHY_ADDR_OFFSET   21
#define MDIO_COMMAND_PHY_REG_OFFSET    16
#define MDIO_COMMAND_WRITE_DATA_OFFSET 0

#define MDIO_COMMAND_PHY_ADDR_MAX      2  /* total phys supported by GCU */
#define MDIO_COMMAND_PHY_REG_MAX       31 /* total registers available on 
                                           * the M88 Phy used on truxton */

#endif /* ifndef GCU_REG_H */
 
