#ifndef __CFMIPS_ATA_H__
#define __CFMIPS_ATA_H__

extern unsigned cf_head;
extern unsigned cf_cyl;
extern unsigned cf_spt;
extern unsigned cf_sectors;

#define CFDEV_BUF_SIZE	0x1000
#define ATA_CIS_OFFSET	0x200
#define ATA_REG_OFFSET	0x800
#define ATA_DBUF_OFFSET	0xC00

#define ATA_REG_FEAT	0x1
#define ATA_REG_SC	0x2
#define ATA_REG_SN	0x3
#define ATA_REG_CL	0x4
#define ATA_REG_CH	0x5
#define ATA_REG_DH	0x6
#define   ATA_REG_DH_BASE	0xa0
#define   ATA_REG_DH_LBA	0x40
#define   ATA_REG_DH_DRV	0x10
#define ATA_REG_CMD	0x7
#define ATA_REG_ST	0x7
#define   ATA_REG_ST_BUSY	0x80
#define   ATA_REG_ST_RDY	0x40
#define   ATA_REG_ST_DWF	0x20
#define   ATA_REG_ST_DSC	0x10
#define   ATA_REG_ST_DRQ	0x08
#define   ATA_REG_ST_CORR	0x04
#define   ATA_REG_ST_ERR	0x01
#define ATA_REG_ERR	0xd
#define ATA_REG_DC	0xe
#define   ATA_REG_DC_IEN	0x02
#define   ATA_REG_DC_SRST	0x04

#define ATA_CMD_READ_SECTORS	0x20
#define ATA_CMD_WRITE_SECTORS	0x30
#define ATA_CMD_EXEC_DRIVE_DIAG	0x90
#define ATA_CMD_READ_MULTIPLE	0xC4
#define ATA_CMD_WRITE_MULTIPLE	0xC5
#define ATA_CMD_SET_MULTIPLE	0xC6
#define ATA_CMD_IDENTIFY_DRIVE	0xEC
#define ATA_CMD_SET_FEATURES	0xEF

#define ATA_FEATURE_ENABLE_APM	0x05
#define ATA_FEATURE_DISABLE_APM	0x85
#define ATA_APM_DISABLED	0x00
#define ATA_APM_MIN_POWER	0x01
#define ATA_APM_WITH_STANDBY	0x7f
#define ATA_APM_WITHOUT_STANDBY	0x80
#define ATA_APM_MAX_PERFORMANCE	0xfe

#define CF_SECT_SIZE	0x200
#define ATA_MAX_SECT_PER_CMD	0x100

#define CF_TRANS_FAILED		0
#define CF_TRANS_OK		1
#define CF_TRANS_IN_PROGRESS	2

int cf_do_transfer(char *buf, unsigned buf_size, unsigned lba_offset,
		   unsigned sector_count, int cmd_read);

int cf_init(void);
void cf_cleanup(void);

void cf_async_trans_done(int result);
void *cf_get_next_buf(unsigned *buf_size);

#endif
