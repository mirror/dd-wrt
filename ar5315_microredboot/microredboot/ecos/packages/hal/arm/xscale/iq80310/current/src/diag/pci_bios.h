//=============================================================================
//
//      pci_bios.h - Cyclone Diagnostics
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   Scott Coulter, Jeff Frazier, Eric Breeden
// Contributors:
// Date:        2001-01-25
// Purpose:     
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

/****************************************************************************/
/* File:        pci_bios.h                                                            */
/*                                                                          */
/* Use:         mon960                                                      */
/*                                                                          */
/* Purpose:  PCI BIOS Routines                                              */
/*                                                                          */
/* Remarks:  Conforming to the Revision 2.1 PCI BIOS Specfication           */
/*                                                                          */
/* Functions Supported:                                                     */
/*                                                                          */
/*       pci_bios_present()                                                 */
/*       find_pci_device()                                                  */
/*       find_pci_class_code()                                              */
/*       generate_special_cycle()                                           */
/*       read_config_byte()                                                 */
/*       read_config_word()                                                 */
/*       read_config_dword()                                                */
/*       write_config_byte()                                                */
/*       write_config_word()                                                */
/*       write_config_dword()                                               */
/*       get_irq_routing_options()                                          */
/*       set_pci_irq()                                                      */
/*                                                                          */
/* History:                                                                 */
/*   06Sep00  Scott Coulter		Changed NUM_PCI_BUSES from 31 to 2          */
/*   09Sep97  Jim Otto			Defined NUM_PCI_BUSES                       */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

#include "iq80310.h"

#define XINT0 0
#define XINT1 1
#define XINT2 2
#define XINT3 3

/* primary PCI bus definitions */ 
#define PRIMARY_BUS_NUM		0
#define PRIMARY_MEM_BASE	0x80000000
#define PRIMARY_DAC_BASE	0x84000000
#define PRIMARY_IO_BASE		0x90000000
#define PRIMARY_MEM_LIMIT	0x83ffffff
#define PRIMARY_DAC_LIMIT	0x87ffffff
#define PRIMARY_IO_LIMIT	0x9000ffff


/* secondary PCI bus definitions */
#define	SECONDARY_BUS_NUM	1
#define SECONDARY_MEM_BASE	0x88000000
#define SECONDARY_DAC_BASE	0x8c000000
#define SECONDARY_IO_BASE	0x90010000
#define SECONDARY_MEM_LIMIT	0x8bffffff
#define SECONDARY_DAC_LIMIT	0x8fffffff
#define SECONDARY_IO_LIMIT	0x9001ffff


#define LAST_SYSPROC 260

#define NUM_PCI_BUSES 2

#ifndef ASM_LANGUAGE

/******************************************************************************
*
*	Required PCI BIOS Data Structures
*
*/
typedef struct 
{
	int num_devices; 
	int num_functions;	
} PCI_DATA;

typedef struct
{
	int	present_status;	 /* set to 0x00 for BIOS present */
	int	hardware_mech_config;	 /* for accessing config. space */
	int	hardware_mech_special;	/* for performing special cycles */
	int	if_level_major_ver;	 /* in BCD, 0x02 for version 2.1 */
	int	if_level_minor_ver;	 /* in BCD, 0x01 for version 2.1 */
	int	last_pci_bus;	 /* numbers start at 0 */
} PCI_BIOS_INFO;

/*******************************************************************************
*
* Type 0 PCI Configuration Space Header
*
*/

typedef struct
{
  unsigned short  vendor_id;
  unsigned short  device_id;
  unsigned short  command;
  unsigned short  status;
  unsigned char   revision_id;
  unsigned char   prog_if;
  unsigned char   sub_class;
  unsigned char   base_class;
  unsigned char   cache_line_size;
  unsigned char   latency_timer;
  unsigned char   header_type;
  unsigned char   bist;
  unsigned long   pcibase_addr0;
  unsigned long   pcibase_addr1;
  unsigned long   pcibase_addr2;
  unsigned long   pcibase_addr3;
  unsigned long   pcibase_addr4;
  unsigned long   pcibase_addr5;
  unsigned long   cardbus_cis_ptr;
  unsigned short  sub_vendor_id;
  unsigned short  sub_device_id;
  unsigned long   pcibase_exp_rom;
  unsigned long   reserved2[2];
  unsigned char   int_line;
  unsigned char   int_pin;
  unsigned char   min_gnt;
  unsigned char   max_lat;
} PCI_CONFIG_SPACE_0;

/*******************************************************************************
*
* PCI Bridge Configuration Space Header
*
*/

typedef struct
{
  unsigned short  vendor_id;
  unsigned short  device_id;
  unsigned short  command;
  unsigned short  status;
  unsigned char   revision_id;
  unsigned char   prog_if;
  unsigned char   sub_class;
  unsigned char   base_class;
  unsigned char   cache_line_size;
  unsigned char   latency_timer;
  unsigned char   header_type;
  unsigned char   bist;
  unsigned long   pcibase_addr0;
  unsigned long   pcibase_addr1;
  unsigned char   primary_busno;
  unsigned char   secondary_busno;
  unsigned char   subordinate_busno;
  unsigned char   secondary_latency_timer;
  unsigned char   io_base;
  unsigned char   io_limit;
  unsigned short  secondary_status;
  unsigned short  mem_base;
  unsigned short  mem_limit;
  unsigned short  pfmem_base;
  unsigned short  pfmem_limit;
  unsigned long   pfbase_upper32;
  unsigned long   pflimit_upper32;
  unsigned short  iobase_upper16;
  unsigned short  iolimit_upper16;
  unsigned short  sub_vendor_id;
  unsigned short  sub_device_id;
  unsigned long   pcibase_exp_rom;
  unsigned char   int_line;
  unsigned char   int_pin;
  unsigned short  bridge_control;
} PCI_CONFIG_SPACE_1;

typedef union
{
  PCI_CONFIG_SPACE_0  pci0_config;
  PCI_CONFIG_SPACE_1  pci1_config;
} PCI_CONFIG_SPACE;

#define CONFIG_MECHANISM_1	1
#define CONFIG_MECHANISM_2	2

typedef struct
{
	int	bus_number;  /* 0...255 */
  int	device_number;  /* Device number on bus */
  int	function_number;  /* Function number on device */
} PCI_DEVICE_LOCATION;


typedef struct
{
	int	bus_number;  /* 0...255 */
  int	device_number;  /* Device number on bus */
  int	inta_link;  /* Which ints. are or'd together */
  int	inta_bitmap;  /* Which XINT connected to */
  int	intb_link;  /* Which ints. are or'd together */
  int	intb_bitmap;  /* Which XINT connected to */
  int	intc_link;  /* Which ints. are or'd together */
  int	intc_bitmap;  /* Which XINT connected to */
  int	intd_link;  /* Which ints. are or'd together */
  int	intd_bitmap;  /* Which XINT connected to */
  int	slot_number;  /* Physical slot (1 - NUM_PCI_SLOTS) */
} SLOT_IRQ_ROUTING;


/* Link values used to indicate which PCI interrupts are wire OR'ed together, the
   value 0 indicates no connection to an interrupt controller and should not be used */

#define LINK_XINT0	1
#define LINK_XINT1	2
#define LINK_XINT2	3
#define LINK_XINT3	4
#define LINK_XINT4	5
#define LINK_XINT5	6
#define LINK_XINT6	7
#define LINK_XINT7	8

#define INTA 		1
#define INTB 		2
#define INTC 		3
#define INTD 		4

#define INTA_PTR	0
#define INTB_PTR	1
#define INTC_PTR	2
#define INTD_PTR	3

#define SLOT0		0
#define SLOT1		1
#define SLOT2		2
#define SLOT3		3

/* PCI Errors - Status Registers */
#define PARITY_ERROR		0x8000
#define SERR_ERROR			0x4000
#define MASTER_ABORT		0x2000
#define TARGET_ABORT_M	0x1000
#define TARGET_ABORT_T	0x0800
#define MASTER_PAR_ERR  0x0100

/* PCI Errors - PCI Interrupt Status Registers */
#define SERR_ASSERTED	0x00000400
#define ATU_PERR		0x00000200
#define ATU_BIST_ERR	0x00000100
#define IB_MA_ABORT		0x00000080
#define BRIDGE_PERR		0x00000020
#define PSERR_FAULT		0x00000010
#define MA_FAULT		0x00000008
#define TA_M_FAULT		0x00000004
#define TA_T_FAULT		0x00000002
#define PAR_FAULT		0x00000001

/* Generic PCI Constants */
#define MAX_PCI_BUSES				31
#define MAX_DEVICE_NUMBER			31
#define MAX_FUNCTION_NUMBER			8
#define DEVS_PER_BRIDGE				6
#define STANDARD_HEADER				0
#define PCITOPCI_HEADER				1
#define NUM_PCI_SLOTS				4
#define MULTIFUNCTION_DEVICE		(1 << 7)
#define MAX_SUB_BUSNO				0xff
#define LATENCY_VALUE				0x0f
#define FIRST_DEVICE_NUM			5
#define LAST_DEVICE_NUM				8
#define SLOTS_PER_BUS				4

/* PCI command register bits */
#define PCI_CMD_IOSPACE			(1 << 0)
#define PCI_CMD_MEMSPACE		(1 << 1)
#define PCI_CMD_BUS_MASTER		(1 << 2)
#define PCI_CMD_SPECIAL			(1 << 3)
#define PCI_CMD_MWI_ENAB		(1 << 4)
#define PCI_CMD_VGA_SNOOP		(1 << 5)
#define PCI_CMD_PARITY			(1 << 6)
#define PCI_CMD_WAIT_CYC		(1 << 7)
#define PCI_CMD_SERR_ENAB		(1 << 8)
#define PCI_CMD_FBB_ENAB		(1 << 9)

/* Bridge Command Register Bit Definitions*/
#define BRIDGE_IOSPACE_ENAB		(1 << 0)
#define BRIDGE_MEMSPACE_ENAB	(1 << 1)
#define BRIDGE_MASTER_ENAB		(1 << 2)
#define BRIDGE_WAIT_CYCLE		(1 << 7)
#define BRIDGE_SERR_ENAB		(1 << 8)

/* Bridge Control Register Bit Definitions */
#define BRIDGE_PARITY_ERR		(1 << 0)
#define BRIDGE_SEER_ENAB		(1 << 1)
#define BRIDGE_MASTER_ABORT		(1 << 5)

/* configuration offsets */
#define VENDOR_ID_OFFSET        0x00
#define DEVICE_ID_OFFSET        0x02
#define COMMAND_OFFSET          0x04
#define STATUS_OFFSET           0x06
#define REVISION_OFFSET         0x08
#define PROG_IF_OFFSET          0x09
#define SUB_CLASS_OFFSET        0x0a
#define BASE_CLASS_OFFSET       0x0b
#define CACHE_LINE_OFFSET       0x0c
#define LATENCY_TIMER_OFFSET    0x0d
#define HEADER_TYPE_OFFSET      0x0e
#define BIST_OFFSET             0x0f
#define REGION0_BASE_OFFSET     0x10
#define REGION1_BASE_OFFSET     0x14
#define REGION2_BASE_OFFSET     0x18
#define PRIMARY_BUSNO_OFFSET	0x18
#define SECONDARY_BUSNO_OFFSET	0x19
#define SUBORD_BUSNO_OFFSET		0x1a
#define SECONDARY_LAT_OFFSET	0x1b
#define REGION3_BASE_OFFSET     0x1c
#define IO_BASE_OFFSET			0x1c
#define IO_LIMIT_OFFSET			0x1d
#define SECONDARY_STAT_OFFSET	0x1e
#define REGION4_BASE_OFFSET     0x20
#define MEMORY_BASE_OFFSET		0x20
#define MEMORY_LIMIT_OFFSET		0x22
#define REGION5_BASE_OFFSET     0x24
#define PREF_MEM_BASE_OFFSET	0x24
#define PREF_MEM_LIMIT_OFFSET	0x26
#define CARDBUS_CISPTR_OFFSET	0x28
#define PREF_BASE_UPPER_OFFSET	0x28
#define SUB_VENDOR_ID_OFFSET	0x2c
#define PREF_LIMIT_UPPER_OFFSET	0x2c
#define SUB_DEVICE_ID_OFFSET	0x2e
#define EXP_ROM_OFFSET          0x30
#define IO_BASE_UPPER_OFFSET	0x30
#define IO_LIMIT_UPPER_OFFSET	0x32
#define CAP_PTR_OFFSET			0x34
#define TYPE1_EXP_ROM_OFFSET    0x38
#define INT_LINE_OFFSET         0x3c
#define INT_PIN_OFFSET          0x3d
#define MIN_GNT_OFFSET          0x3e
#define BRIDGE_CTRL_OFFSET		0x3e
#define MAX_LAT_OFFSET          0x3f

typedef struct
{
	SLOT_IRQ_ROUTING  info[NUM_PCI_SLOTS];
} PCI_IRQ_ROUTING_TABLE;


/******************************************************************************
*
*	Return values from BIOS Calls
*
*/

#define SUCCESSFUL					 0
#define DEVICE_NOT_FOUND			-1
#define BAD_VENDOR_ID				-2
#define FUNC_NOT_SUPPORTED			-3
#define BUFFER_TOO_SMALL			-4
#define SET_FAILED					-5
#define BAD_REGISTER_NUMBER			-6


/******************************************************************************
*
*	BIOS Function Prototypes
*
*/

STATUS pci_bios_present (PCI_BIOS_INFO *info);

STATUS find_pci_device (int	device_id, int vendor_id, int index);

STATUS find_pci_class_code (int	class_code, int	index);

STATUS generate_special_cycle (int bus_number, int special_cycle_data);

STATUS read_config_byte (int bus_number, int device_number, int	function_number, int register_number,	/* 0,1,2,...,255 */
													UINT8	*data);

STATUS read_config_word (int bus_number, int device_number, int	function_number, int register_number,	/* 0,2,4,...,254 */
													UINT16 *data);

STATUS read_config_dword (int bus_number, int device_number, int function_number, int register_number,	/* 0,4,8,...,252 */
													UINT32 *data);

STATUS write_config_byte (int bus_number, int device_number, int function_number, int register_number,	/* 0,1,2,...,255 */
													UINT8	data);

STATUS write_config_word (int bus_number, int device_number, int function_number, int register_number,	/* 0,2,4,...,254 */
													UINT16 data);

STATUS write_config_dword (int bus_number, int device_number, int function_number, int register_number,	/* 0,4,8,...,252 */
														UINT32 data);

STATUS get_irq_routing_options (PCI_IRQ_ROUTING_TABLE *table);

STATUS set_pci_irq (int	int_pin, int irq_num, int	bus_dev);


/******************************************************************************
*
* sysPciIsrConnect - connect a routine to an PCI interrupt
*
* This function uses the Breeze System Services.  Parameters are left
* unchanged in the global registers just as the service call expects.
* Likewise, the return value of the service call is left unmodified.
*
* intline is the PCI interrupt line PCI_INTA - PCI_INTD
*
* bus is the PCI bus the targeted device is on
*
* device is the targeted device for the PCI interrupt
*
* handler is an interrupt handler which accepts an integer as an argument and
* 	returns 0 if no interrupt was serviced and 1 if an interrupt was
*	serviced (necessary for interrupt sharing).
*
* arg is the argument to be passed to the handler when called.
* 
*/
STATUS sysPciIsrConnect (int intline, 
						 int bus, 
						 int device, 
						 int (*handler)(int), 
						 int arg);


/******************************************************************************
*
* sysPciIsrDisconnect - disconnect a routine from an PCI interrupt
*
* This function uses the Breeze System Services.  Parameters are left
* unchanged in the global registers just as the service call expects.
* Likewise, the return value of the service call is left unmodified.
*
* intline is the PCI interrupt line INTA - INTD
*
* bus is the PCI bus the targeted device is on
*
* device is the PCI device sourcing the interrupt
*
                                                                                                                                                                                                                                                                                                                                                                                                                                                                           */
STATUS sysPciIsrDisconnect (int intline,
							int bus,
							int device);

#endif	/* ASM_LANGUAGE */

