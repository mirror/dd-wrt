#ifndef CYGONCE_PCMCIA_H
#define CYGONCE_PCMCIA_H
// ====================================================================
//
//      pcmcia.h
//
//      Device I/O 
//
// ====================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004 Red Hat, Inc.
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
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     gthomas
// Contributors:  gthomas
// Date:          2000-07-06
// Purpose:       Interfaces for PCMCIA I/O drivers
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// PCMCIA I/O interfaces

#include <pkgconf/system.h>
#include <pkgconf/io_pcmcia.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/drv_api.h>

struct cf_irq_handler {
    void         (*handler)(int, int, void *);
    void          *param;
};

// Basic information about a slot/device
struct cf_slot {
    int            index;       // In case hardware layer needs it
    int            state;
    unsigned char *attr;
    int            attr_length;
    unsigned char *io;
    int            io_length;
    unsigned char *mem;
    int            mem_length;
    int            int_num;    // Hardware interrupt number
    struct cf_irq_handler irq_handler;
};

#define CF_SLOT_STATE_Empty     0
#define CF_SLOT_STATE_Inserted  1
#define CF_SLOT_STATE_Powered   2
#define CF_SLOT_STATE_Reset     3
#define CF_SLOT_STATE_Ready     4
#define CF_SLOT_STATE_Removed   5

#define CF_CISTPL_VERS_1        0x15
#define CF_CISTPL_CONFIG        0x1A
#define CF_CISTPL_CFTABLE_ENTRY 0x1B
#define CF_CISTPL_MANFID        0x20
#define CF_CISTPL_FUNCID        0x21

// Configuration register offsets
#define CF_CONFIG_COR           0x00
#define CF_CONFIG_CCSR          0x02
#define CF_CONFIG_PRR           0x04
#define CF_CONFIG_SCR           0x06
#define CF_CONFIG_ESR           0x08
#define CF_CONFIG_IOBASE_0      0x0a
#define CF_CONFIG_IOBASE_1      0x0c
#define CF_CONFIG_IOBASE_2      0x0e
#define CF_CONFIG_IOBASE_3      0x10
#define CF_CONFIG_IOSIZE        0x12

// Configuration Option Register bits
#define CF_COR_FUNC_ENA         0x01
#define CF_COR_ADDR_DECODE      0x02
#define CF_COR_IREQ_ENA         0x04
#define CF_COR_LEVEL_REQ        0x40
#define CF_COR_SOFT_RESET       0x80


#define CF_MAX_IO_ADDRS      8
struct cf_io_space {    
    unsigned long base[CF_MAX_IO_ADDRS];     // Base address of I/O registers
    unsigned long size[CF_MAX_IO_ADDRS];     // Length(-1) of I/O registers
    int           num_addrs;
    unsigned char mode;
};

// Corresponds to CISTPL_CFTABLE_ENTRY
struct cf_cftable {
    unsigned char      cor;              // Value to write to COR register
    unsigned char      interface;
    unsigned char      feature_select;
    struct cf_io_space io_space;
};

// Corresponds to CISTPL_CONFIG
struct cf_config {
    unsigned long base;
    int           mask_length;
    unsigned char mask[16];
};

// Function prototypes

bool cf_get_CIS(struct cf_slot *slot, unsigned char id, 
                unsigned char *buf, int *len, int *ptr);
void cf_set_COR(struct cf_slot *slot, unsigned long cor, unsigned char val);
bool cf_parse_cftable(unsigned char *buf, int len, struct cf_cftable *cftable);
bool cf_parse_config(unsigned char *buf, int len, struct cf_config *config);
struct cf_slot *cf_get_slot(int indx);
void cf_change_state(struct cf_slot *slot, int desired_state);
void cf_register_handler(struct cf_slot *, void (*handler)(int, int, void *), void *);
void cf_clear_interrupt(struct cf_slot *);
void cf_init(void);

void cf_hwr_poll(struct cf_slot *);

#endif // CYGONCE_PCMCIA_H
