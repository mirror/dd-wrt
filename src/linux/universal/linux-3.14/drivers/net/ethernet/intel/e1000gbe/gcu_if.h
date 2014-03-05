/*****************************************************************************

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

*****************************************************************************/

/* 
 * gcu_if.h
 * Shared functions for accessing and configuring the GCU
 */

#ifndef GCU_IF_H
#define GCU_IF_H
#define GCU_DEVID 0x503E
#define GCU_MAX_ATTEMPTS 64

int32_t gcu_write_eth_phy(uint32_t phy_num, uint32_t reg_addr, 
                          uint16_t phy_data);

int32_t gcu_read_eth_phy(uint32_t phy_num, uint32_t reg_addr, 
                         uint16_t *phy_data);

int gcu_iegbe_suspend(struct pci_dev *pdev, uint32_t state);
void gcu_iegbe_resume(struct pci_dev *pdev);
#endif /* ifndef GCU_IF_H */
 
