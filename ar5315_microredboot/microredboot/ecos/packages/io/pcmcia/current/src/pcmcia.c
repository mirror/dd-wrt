//==========================================================================
//
//      io/pcmcia/pcmcia.c
//
//      PCMCIA support (Card Services)
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-07-06
// Purpose:      PCMCIA support
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/io_pcmcia.h>

#include <cyg/io/pcmcia.h>
#include <cyg/infra/diag.h>

#if CYGHWR_IO_PCMCIA_DEVICE == 0
#error Need hardware package for PCMCIA support
#endif

#define CF_NUM_SLOTS CYGHWR_IO_PCMCIA_DEVICE
static struct cf_slot cf_slots[CF_NUM_SLOTS];

// Implementation routines
void cf_hwr_init(struct cf_slot *slot);
void cf_hwr_change_state(struct cf_slot *slot, int desired_state);
void cf_hwr_clear_interrupt(struct cf_slot *slot);

bool
cf_get_CIS(struct cf_slot *slot, unsigned char id, 
           unsigned char *buf, int *len, int *ptr)
{
    int i, size;
    unsigned char *cis = slot->attr;
    unsigned char *cis_end = cis + slot->attr_length;
    cis += *ptr;
    while (cis < cis_end) {
        if (*cis == 0xFF) {
            break;
        }
        if (*cis == id) {
            size = *(cis+2) + 2;
            for (i = 0;  i < size;  i++) {
                *buf++ = *cis;
                cis += 2;
            }
            *len = size;
            *ptr = (unsigned long)(cis - slot->attr);
            return true;
        } else {
            // Skip to next entry
            cis += (*(cis+2) * 2) + 4;
        }
    }
    return false;
}

void
cf_set_COR(struct cf_slot *slot, unsigned long cor, unsigned char val)
{
    volatile unsigned char *cfg = slot->attr;
    cfg[cor] = val;
}

static void
cf_parse_power_structure(unsigned char **buf)
{
    unsigned char *bp = *buf;
    unsigned char tpce_pd = *bp++;
    unsigned char settings;
    int indx;
    for (indx = 6;  indx >= 0;  indx--) {
        if (tpce_pd & (1<<indx)) {
            settings = *bp++;  // main value
            if (settings & 0x80) {
                bp++;  // extension byte - FIXME
            }
        }
    }
    *buf = bp;
}

static void
cf_parse_timing_structure(unsigned char **buf)
{
    unsigned char *bp = *buf;
    unsigned char tpce_td = *bp++;
    if ((tpce_td & 0x1C) != 0x1C) {
//        diag_printf("READY = %x.%x\n",(tpce_td & 0x1C)>>2, *bp); 
        bp++;
    }
    if ((tpce_td & 0x03) != 0x03) {
//        diag_printf("WAIT = %x.%x\n",(tpce_td & 0x03)>>0, *bp); 
        bp++;
    }
    *buf = bp;
}

static void
cf_parse_IO_space_structure(unsigned char **buf, struct cf_io_space *io_space)
{
    unsigned char *bp = *buf;
    unsigned char tpce_io = *bp++;
    unsigned char rd;
    unsigned long base = 0, length = 0;
    int i;
    io_space->mode = (tpce_io & 0x60) >> 5;
    if (tpce_io & 0x80) {
        rd = *bp++;
        io_space->num_addrs = (rd & 0x0F) + 1;
        for (i = 0;  i < io_space->num_addrs;  i++) {
            // Address
            switch ((rd & 0x30) >> 4) {
            case 0:
                break;  // Not present (shouldn't happen)
            case 1:
                base = *bp++;
                break;
            case 2:
                base = (bp[1] << 8) | bp[0];
                bp += 2;
                break;
            case 3:                
                base = (bp[3] << 24) | (bp[2] << 16) | (bp[1] << 8) | bp[0];
                bp += 4;
                break;
            }
            io_space->base[i] = base;
            // Length
            switch ((rd & 0xC0) >> 6) {
            case 0:
                break;  // Not present (shouldn't happen)
            case 1:
                length = *bp++;
                break;
            case 2:
                length = (bp[1] << 8) | bp[0];
                bp += 2;
                break;
            case 3:                
                length = (bp[3] << 24) | (bp[2] << 16) | (bp[1] << 8) | bp[0];
                bp += 4;
                break;
            }
            length++;
            io_space->size[i] = length;
//            diag_printf("IO addr %d - base: %x, length: %x\n", i, base, length);
        }
    }
    *buf = bp;
}

bool
cf_parse_cftable(unsigned char *buf, int len, struct cf_cftable *cftable)
{
    unsigned char tpce_indx, tpce_fs;
    if (*buf++ != CF_CISTPL_CFTABLE_ENTRY) {
        diag_printf("%s - called with invalid CIS: %x\n", __FUNCTION__, *--buf);
        return false;
    }
    buf++;  // Skip length/link
    tpce_indx = *buf++;
    cftable->cor = tpce_indx & 0x3F;
    if (tpce_indx & 0x80) {
        cftable->interface = *buf++;
    }
    cftable->feature_select = tpce_fs = *buf++;
    if (tpce_fs & 0x01) {
        cf_parse_power_structure(&buf);
    }
    if (tpce_fs & 0x02) {
        cf_parse_power_structure(&buf);
    }
    if (tpce_fs & 0x04) {
        cf_parse_timing_structure(&buf);
    }
    if (tpce_fs & 0x08) {
        cf_parse_IO_space_structure(&buf, &cftable->io_space);
    }
    return true;
}

bool
cf_parse_config(unsigned char *buf, int len, struct cf_config *config)
{
    unsigned char tpcc_sz;
    int i;
    if (*buf++ != CF_CISTPL_CONFIG) {
        diag_printf("%s - called with invalid CIS: %x\n", __FUNCTION__, *--buf);
        return false;
    }
    buf++;  // Skip length/link
    tpcc_sz = *buf++;
    buf++;  // Skip 'last' pointer
    config->base = 0;
    for (i = (tpcc_sz & 0x03);  i >= 0;  i--) {
        config->base = (config->base << 8) | buf[i];
    }
    buf += (tpcc_sz & 0x03) + 1;
    config->mask_length = ((tpcc_sz & 0x3C) >> 2) + 1;
    for (i = 0;  i < config->mask_length;  i++) {
        config->mask[i] = *buf++;
    }
    return true;
}

//
// Return a pointer to the slot descriptor for a given slot
//
struct cf_slot *
cf_get_slot(int indx)
{
    if ((indx >= 0) && (indx < CF_NUM_SLOTS)) {
        return &cf_slots[indx];
    } else {
        diag_printf("PCMCIA: Invalid slot %d\n", indx);
        return (struct cf_slot *)0;
    }
}

//
// Initialize all PCMCIA (Compact Flash) slots
//
void
cf_init(void)
{
    int i;
    for (i = 0;  i < CF_NUM_SLOTS; i++) {
        cf_slots[i].index = i;
        cf_hwr_init(&cf_slots[i]);
    }
}

//
// Transition a card/slot
//
void
cf_change_state(struct cf_slot *slot, int desired_state)
{
    cf_hwr_change_state(slot, desired_state);
}

//
// Register an interrupt handler
//
void 
cf_register_handler(struct cf_slot *slot, 
                    void (*handler)(int, int, void *), 
                    void *param)
{
    slot->irq_handler.handler = handler;
    slot->irq_handler.param = param;
}

//
// Allow interrupt function to acknowledge interrupt
//
void
cf_clear_interrupt(struct cf_slot *slot)
{
    cf_hwr_clear_interrupt(slot);
}

