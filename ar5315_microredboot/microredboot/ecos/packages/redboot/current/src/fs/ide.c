//==========================================================================
//
//      ide.c
//
//      RedBoot IDE support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas <gary@mind.be>
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
// Author(s):    msalter
// Contributors: msalter
// Date:         2001-07-14
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>
#include <cyg/hal/hal_io.h>
#include <fs/disk.h>
#include <fs/ide.h>

static int ide_read(struct disk *d,
		    cyg_uint32 start_sector,
		    cyg_uint32 *buf,
		    cyg_uint8  nr_sectors);

static disk_funs_t ide_funs = { ide_read };

static struct ide_priv ide_privs[HAL_IDE_NUM_CONTROLLERS * 2];

static inline void
__wait_for_ready(int ctlr)
{
    cyg_uint8 status;
    do {
	HAL_IDE_READ_UINT8(ctlr, IDE_REG_STATUS, status);
    } while (status & (IDE_STAT_BSY | IDE_STAT_DRQ));
}

static inline int
__wait_for_drq(int ctlr)
{
    cyg_uint8 status;
    cyg_ucount32 tries;

    CYGACC_CALL_IF_DELAY_US(10);
    for (tries=0; tries<1000000; tries++) {
	HAL_IDE_READ_UINT8(ctlr, IDE_REG_STATUS, status);
        if (!(status & IDE_STAT_BSY)) {
            if (status & IDE_STAT_DRQ)
                return 1;
            else
                return 0;
        }
    }
}

static int
ide_reset(int ctlr)
{
    cyg_uint8 status;
    int delay;
//
// VMware note:
// VMware virtual IDE device handler obviously expects that
// the reset and setup functions were already done
// by it's bios and complais if one uses reset here...
//
#ifndef CYGSEM_REDBOOT_DISK_IDE_VMWARE
    HAL_IDE_WRITE_CONTROL(ctlr, 6);	// polled mode, reset asserted
    CYGACC_CALL_IF_DELAY_US(5000);
    HAL_IDE_WRITE_CONTROL(ctlr, 2);	// polled mode, reset cleared
    CYGACC_CALL_IF_DELAY_US((cyg_uint32)50000);
#endif

    // wait 30 seconds max for not busy and drive ready
    for (delay = 0; delay < 300; ++delay) {
	CYGACC_CALL_IF_DELAY_US((cyg_uint32)100000);
	HAL_IDE_READ_UINT8(ctlr, IDE_REG_STATUS, status);
	  if (!(status & IDE_STAT_BSY)) {
		if (status & IDE_STAT_DRDY) {
	    return 1;
    }
	  }
    }
    return 0;
}

// Return true if any devices attached to controller
static int
ide_presence_detect(int ctlr)
{
    cyg_uint8 sel, val;
    int i;

    for (i = 0; i < 2; i++) {
	sel = (i << 4) | 0xA0;
	CYGACC_CALL_IF_DELAY_US((cyg_uint32)50000);
	HAL_IDE_WRITE_UINT8(ctlr, IDE_REG_DEVICE, sel);
	CYGACC_CALL_IF_DELAY_US((cyg_uint32)50000);
	HAL_IDE_READ_UINT8(ctlr, IDE_REG_DEVICE, val);
	if (val == sel) {
#ifndef CYGSEM_REDBOOT_DISK_IDE_VMWARE
	    if (i)
		HAL_IDE_WRITE_UINT8(ctlr, IDE_REG_DEVICE, 0);
#endif
	    return 1;
	}
    }
    return 0;
}

static int
ide_ident(int ctlr, int dev, int is_packet_dev, cyg_uint16 *buf)
{
    int i;

    HAL_IDE_WRITE_UINT8(ctlr, IDE_REG_DEVICE, dev << 4);
    HAL_IDE_WRITE_UINT8(ctlr, IDE_REG_COMMAND, is_packet_dev ? 0xA1 : 0xEC);
    CYGACC_CALL_IF_DELAY_US((cyg_uint32)50000);

    if (!__wait_for_drq(ctlr))
	return 0;

    for (i = 0; i < (SECTOR_SIZE / sizeof(cyg_uint16)); i++, buf++)
	HAL_IDE_READ_UINT16(ctlr, IDE_REG_DATA, *buf);

    return 1;
}

static int
ide_read_sectors(int ctlr, int dev, cyg_uint32 start, cyg_uint8 count, cyg_uint16 *buf)
{
    int  i, j;
    cyg_uint16 *p;

    HAL_IDE_WRITE_UINT8(ctlr, IDE_REG_COUNT, count);
    HAL_IDE_WRITE_UINT8(ctlr, IDE_REG_LBALOW, start & 0xff);
    HAL_IDE_WRITE_UINT8(ctlr, IDE_REG_LBAMID, (start >>  8) & 0xff);
    HAL_IDE_WRITE_UINT8(ctlr, IDE_REG_LBAHI,  (start >> 16) & 0xff);
    HAL_IDE_WRITE_UINT8(ctlr, IDE_REG_DEVICE,
			((start >> 24) & 0xf) | (dev << 4) | 0x40);
    HAL_IDE_WRITE_UINT8(ctlr, IDE_REG_COMMAND, 0x20);

    for(p = buf, i = 0; i < count; i++) {

        if (!__wait_for_drq(ctlr)) {
            diag_printf("%s: NO DRQ for ide%d, device %d.\n",
                        __FUNCTION__, ctlr, dev);
            return 0;
        }

        for (j = 0; j < (SECTOR_SIZE / sizeof(cyg_uint16)); j++, p++)
            HAL_IDE_READ_UINT16(ctlr, IDE_REG_DATA, *p);
    }
    return 1;
}

// max number of sectors to xfer during a single packet command
#define MAX_CD_XFER 16

static inline int
send_packet_command(int ctlr, int dev, cyg_uint16 len, cyg_uint16 *pkt, int pktlen)
{
    int i;
    cyg_uint8 status, reason;

    HAL_IDE_WRITE_UINT8(ctlr, IDE_REG_FEATURES, 0);
    HAL_IDE_WRITE_UINT8(ctlr, IDE_REG_COUNT, 0);
    HAL_IDE_WRITE_UINT8(ctlr, IDE_REG_LBALOW, 0);
    HAL_IDE_WRITE_UINT8(ctlr, IDE_REG_LBAMID, len & 0xff);
    HAL_IDE_WRITE_UINT8(ctlr, IDE_REG_LBAHI,  (len >> 8) & 0xff);
    HAL_IDE_WRITE_UINT8(ctlr, IDE_REG_DEVICE, dev << 4);
    HAL_IDE_WRITE_UINT8(ctlr, IDE_REG_COMMAND, 0xA0);

    if (!__wait_for_drq(ctlr)) {
	diag_printf("%s: NO DRQ for ide%d, device %d.\n",
		    __FUNCTION__, ctlr, dev);
	return 0;
    }

    // send packet
    for (i = 0; i < (pktlen/sizeof(cyg_uint16)); i++)
	HAL_IDE_WRITE_UINT16(ctlr, IDE_REG_DATA, pkt[i]);

    // wait for not busy transferring packet
    do {
	HAL_IDE_READ_UINT8(ctlr, IDE_REG_STATUS, status);
	HAL_IDE_READ_UINT8(ctlr, IDE_REG_REASON, reason);

	if ((status & (IDE_STAT_BSY | IDE_STAT_DRQ)) == IDE_STAT_DRQ)
	    if (reason & IDE_REASON_COD)
		continue;  // still wanting packet data (should timeout here)

    } while (status & IDE_STAT_BSY);

    return 1;
}

#define READ_COUNT(x)                                    \
        { unsigned char tmp;                             \
          HAL_IDE_READ_UINT8(ctlr, IDE_REG_LBAMID, tmp); \
          (x) = tmp;                                     \
          HAL_IDE_READ_UINT8(ctlr, IDE_REG_LBAHI, tmp);  \
          (x) = (x) | (tmp << 8);                        \
        }


// Read the sense data
static int
request_sense(int ctlr, int dev, cyg_uint16 count, cyg_uint16 *buf)
{
    int i;
    cyg_uint16 cdcount, pkt[6];
    unsigned char status, *cpkt = (unsigned char *)pkt;


    // Fill in REQUEST SENSE packet command block
    memset(cpkt, 0, sizeof(pkt));
    cpkt[0] = 0x03;
    cpkt[4] = 254;  // allocation length
	
    if (!send_packet_command(ctlr, dev, count, pkt, sizeof(pkt)))
	return 0;

    HAL_IDE_READ_UINT8(ctlr, IDE_REG_STATUS, status);
    if (!(status & IDE_STAT_DRQ)) {
	if (status & IDE_STAT_SERVICE) {
	    unsigned char reason;
	    HAL_IDE_READ_UINT8(ctlr, IDE_REG_REASON, reason);
	    diag_printf("%s: SERVICE request for ide%d, device %d, status[%02x], reason[%02x].\n",
			__FUNCTION__, ctlr, dev, status, reason);
	}
	return 0;
    }

    READ_COUNT(cdcount);
    if (cdcount != count)
	diag_printf("%s: ide%d, dev%d: his cnt[%d] our count[%d].\n",
		    __FUNCTION__, ctlr, dev, cdcount, count);

    for(i = 0; i < (cdcount / sizeof(*buf)); i++, buf++)
	HAL_IDE_READ_UINT16(ctlr, IDE_REG_DATA, *buf);

    // wait for not busy transferring data
    do {
	HAL_IDE_READ_UINT8(ctlr, IDE_REG_STATUS, status);
    } while ((status & (IDE_STAT_BSY | IDE_STAT_DRQ)) == IDE_STAT_DRQ);

    return cdcount;
}

// Interpret the sense data
static int
handle_sense(int ctlr, int dev, cyg_uint8 count, cyg_uint16 *buf)
{
#if 0
    unsigned char *p = (char *)buf;

    diag_printf("%s: %d bytes:\n", __FUNCTION__, count);
    diag_printf("sense key[%02x] additional sense[%02x]\n",
		p[2], p[12]);
#endif
    return 1;
}

static int
do_packet_read(int ctlr, int dev, cyg_uint32 start, cyg_uint8 count, cyg_uint16 *buf)
{
    int i, retry_cnt;
    cyg_uint16 cdcount, pkt[6], sense[127];
    unsigned char status, *cpkt = (unsigned char *)pkt;

    // get count number of whole cdrom sectors
    while (count) {

	retry_cnt = 3;

	i = (count > MAX_CD_XFER) ? MAX_CD_XFER : count;

    retry:
	// Fill in READ(10) packet command block
	memset(cpkt, 0, sizeof(pkt));
	cpkt[0] = 0x28;  // READ(10)
	cpkt[2] = (start >> 24) & 0xff;
	cpkt[3] = (start >> 16) & 0xff;
	cpkt[4] = (start >>  8) & 0xff;
	cpkt[5] = (start >>  0) & 0xff;
	cpkt[7] = (i >> 8) & 0xff;
	cpkt[8] = i & 0xff;
	
	if (!send_packet_command(ctlr, dev, i * CDROM_SECTOR_SIZE,
				 pkt, sizeof(pkt)))
	    return 0;

	HAL_IDE_READ_UINT8(ctlr, IDE_REG_STATUS, status);
	if (!(status & IDE_STAT_DRQ)) {
	    if (status & IDE_STAT_SERVICE) {
		unsigned char reason;
		int sense_count;
		HAL_IDE_READ_UINT8(ctlr, IDE_REG_REASON, reason);
#if 1
		diag_printf("%s: SERVICE request for ide%d, device %d, status[%02x], reason[%02x].\n",
			    __FUNCTION__, ctlr, dev, status, reason);
#endif
		sense_count = request_sense(ctlr, dev, sizeof(sense), sense);
		if (sense_count) {
		    handle_sense(ctlr, dev, sense_count, sense);
		    if (retry_cnt--)
			goto retry;
		}
	    }
	    return 0;
	}

	count -= i;
	start += i;

	READ_COUNT(cdcount);
	if (cdcount != (i * CDROM_SECTOR_SIZE))
	    diag_printf("%s: ide%d, dev%d: his cnt[%d] our count[%d].\n",
			__FUNCTION__, ctlr, dev,
			cdcount, i * CDROM_SECTOR_SIZE);

	for(i = 0; i < (cdcount / sizeof(*buf)); i++, buf++)
	    HAL_IDE_READ_UINT16(ctlr, IDE_REG_DATA, *buf);

	// wait for not busy transferring data
	do {
	    HAL_IDE_READ_UINT8(ctlr, IDE_REG_STATUS, status);
	} while ((status & (IDE_STAT_BSY | IDE_STAT_DRQ)) == IDE_STAT_DRQ);
    }
    return 1;
}


static int
ide_packet_read_sectors(int ctlr, int dev, cyg_uint32 start, cyg_uint8 count, cyg_uint16 *buf)
{
    int  i, extra;
    cyg_uint32 cdstart;
    static cyg_uint16 cdsec_buf[CDROM_SECTOR_SIZE/sizeof(cyg_uint16)];

    cdstart = (start + SECTORS_PER_CDROM_SECTOR-1) / SECTORS_PER_CDROM_SECTOR;
    
    // align to cdrom sector boundary.
    if (start % SECTORS_PER_CDROM_SECTOR) {
	if (!ide_packet_read_sectors(ctlr, dev,
				     cdstart * SECTORS_PER_CDROM_SECTOR,
				     SECTORS_PER_CDROM_SECTOR, cdsec_buf))
	    return 0;

	i = SECTORS_PER_CDROM_SECTOR - (start % SECTORS_PER_CDROM_SECTOR);
	if (i > count)
	    i = count;
	memcpy(buf, cdsec_buf + ((start % CDROM_SECTOR_SIZE) * SECTOR_SIZE),
	       i * SECTOR_SIZE);

	count -= i;
	buf += (i * SECTOR_SIZE) / sizeof(*buf);
	++cdstart;
    }

    extra = count % SECTORS_PER_CDROM_SECTOR;
    count /= SECTORS_PER_CDROM_SECTOR;

    if (count) {
	if (!do_packet_read(ctlr, dev, cdstart, count, buf))
            return 0;
	buf += count * SECTORS_PER_CDROM_SECTOR * SECTOR_SIZE;
    }

    if (extra) {
        // read cdrom sector 
        if (!ide_packet_read_sectors(ctlr, dev,
                                     cdstart * SECTORS_PER_CDROM_SECTOR,
                                     extra, cdsec_buf))
            return 0;
	memcpy(buf, cdsec_buf, extra * SECTOR_SIZE);
    }

    return 1;
}

static int
ide_read(struct disk *d,
	 cyg_uint32 start_sec, cyg_uint32 *buf, cyg_uint8 nr_secs)
{
    struct ide_priv *p = (struct ide_priv *)(d->private);

    if (p->flags & IDE_DEV_PACKET)
        return ide_packet_read_sectors(p->controller, p->drive,
                                     start_sec, nr_secs, (cyg_uint16 *)buf);

    return ide_read_sectors(p->controller, p->drive,
			    start_sec, nr_secs, (cyg_uint16 *)buf);
}


static void
ide_init(void)
{
    cyg_uint32 buf[SECTOR_SIZE/sizeof(cyg_uint32)], u32;
    cyg_uint16 u16;
    cyg_uint8 u8;
    int i, j, num_controllers;
    disk_t disk;
    struct ide_priv *priv;

#define DEV_INIT_VAL ((j << 4) | 0xA0)

    num_controllers = HAL_IDE_INIT();

    CYGACC_CALL_IF_DELAY_US(5);

    priv = ide_privs;
    for (i = 0; i < num_controllers; i++) {

	if (!ide_presence_detect(i)) {
	    diag_printf("No devices on IDE controller %d\n", i);
	    continue;
	}

	// soft reset the devices on this controller
	if (!ide_reset(i))
	    continue;

	// 2 devices per controller
	for (j = 0; j < 2; j++, priv++) {

	    priv->controller = i;
	    priv->drive = j;
	    priv->flags = 0;
	    
	    // This is reminiscent of a memory test. We write a value
	    // to a certain location (device register), then write a
	    // different value somewhere else so that the first value
	    // is not hanging on the bus, then we read back the first
	    // value to see if the write was succesful.
	    //
	    HAL_IDE_WRITE_UINT8(i, IDE_REG_DEVICE, DEV_INIT_VAL);
	    HAL_IDE_WRITE_UINT8(i, IDE_REG_FEATURES, 0);
	    CYGACC_CALL_IF_DELAY_US(50000);
	    HAL_IDE_READ_UINT8(i, IDE_REG_DEVICE, u8);
	    if (u8 != DEV_INIT_VAL) {
                diag_printf("IDE failed to identify unit %d - wrote: %x, read: %x\n", 
                            i, DEV_INIT_VAL, u8);
		continue;
            }

	    // device present
	    priv->flags |= IDE_DEV_PRESENT;

	    if (ide_ident(i, j, 0, (cyg_uint16 *)buf) <= 0) {
		if (ide_ident(i, j, 1, (cyg_uint16 *)buf) <= 0) {
		    priv->flags = 0;
		    continue;  // can't identify device
		} else {
                    u16 = *(cyg_uint16 *)((char *)buf + IDE_DEVID_GENCONFIG);
                    if (((u16 >> 8) & 0x1f) != 5) {
                        diag_printf("Non-CDROM ATAPI device #%d - skipped\n", i);
                        continue;
                    }
		    priv->flags |= IDE_DEV_PACKET;
                }
	    }
    
	    memset(&disk, 0, sizeof(disk));
	    disk.funs = &ide_funs;
	    disk.private = priv;

	    disk.kind = DISK_IDE_HD;  // until proven otherwise

	    if (priv->flags & IDE_DEV_PACKET) {
		u16 = *(cyg_uint16 *)((char *)buf + IDE_DEVID_GENCONFIG);
		if (((u16 >> 8) & 0x1f) == 5)
		    disk.kind = DISK_IDE_CDROM;
	    } else {
		u32 = *(cyg_uint32 *)((char *)buf + IDE_DEVID_LBA_CAPACITY);
		disk.nr_sectors = u32;
	    }

	    if (!disk_register(&disk))
		return;
	}
    }
}

RedBoot_init(ide_init, RedBoot_INIT_FIRST);

