/*
 * Broadcom Home Gateway Reference Design
 * BCM53xx RoboSwitch utility functions
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id$
 */

#ifndef _robo_h_
#define _robo_h_

/* PMII definitions */
#define PMII_PHYADDR 0x1e
#define PMII_PAGE_REG     16
#define PMII_ADDR_REG     17
#define PMII_ACCESS_STAT  18
#define PMII_REG_WORD1    24
#define PMII_REG_WORD2    25
#define PMII_REG_WORD3    26
#define PMII_REG_WORD4    27
#define PMII_MDC_ACCESS_ENB 0x1
#define PMII_WRITE        0x1
#define PMII_READ         0x2
#define PMII_OPCODE_MASK  0x3
#define PMII_FORMAT_PAGE(page) ((page<<8)  | PMII_MDC_ACCESS_ENB)
#define PMII_FORMAT_ADDR_WR(addr) ((addr<<8)  | PMII_WRITE)
#define PMII_FORMAT_ADDR_RD(addr) ((addr<<8)  | PMII_READ)

/* OCP definitions */
#define BROADCOM_VENDOR_ID      0x14e4
#define BROADCOM_DEVICE_ID_ROBO BCM47XX_ROBO_ID
#define REG_BE_8b  0x010000
#define REG_BE_16b 0x030000
#define REG_BE_32b 0x0f0000
#define REG_BE_48b 0x3f0000
#define REG_BE_64b 0xff0000
#define REG_CMD_WRITE  0x01000000
#define REG_CMD_READ   0x02000000
#define REG_DONE_WRITE 0x40000000
#define REG_DONE_READ  0x80000000
#define REG_OFFSET_SW_CTRL      0x00
#define REG_OFFSET_SW_ADDR      0x04
#define REG_OFFSET_SW_STATUS    0x0c
#define REG_OFFSET_SW_LRD       0x10
#define REG_OFFSET_SW_URD       0x14
#define REG_OFFSET_SW_LWD       0x18
#define REG_OFFSET_SW_UWD       0x1c

/* Private state per RoboSwitch */
typedef struct {
	void *sbh;			/* SiliconBackplane handle */
	uint32 coreidx;			/* Current core index */
	uint32 ssl, clk, mosi, miso;	/* GPIO mapping */
	int cid, page;			/* Current chip ID and page */
} robo_info_t;

typedef struct {
	void *ch;
	uint16 (*phyrd)(void *ch, uint32 phyaddr, uint32 reg);	          /* read phy register */
	void (*phywr)(void *ch, uint32 phyaddr, uint32 reg, uint16 val);	/* write phy register */
} robo_info_pmii_t;

typedef struct {
	void *ch;
	uint16 (*phyrd)(void *ch, uint32 phyaddr, uint32 reg);	          /* read phy register */
	void (*phywr)(void *ch, uint32 phyaddr, uint32 reg, uint16 val);	/* write phy register */
	void *sbh;			/* SiliconBackplane handle */
    void *regsva;
    uint32 *reg_ctrl;
    uint32 *reg_addr;
    uint32 *reg_status;
    uint32 *reg_lrd;
    uint32 *reg_urd;
    uint32 *reg_lwd;
    uint32 *reg_uwd;
} robo_info_ocp_t;

typedef void (*ROBO_READ)(robo_info_t *robo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint32 len);
typedef void (*ROBO_WRITE)(robo_info_t *robo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint32 len);

robo_info_t *robosw_attach(void *sbh, uint32 ssl, uint32 clk, uint32 mosi, uint32 miso);
void robosw_detach(robo_info_t *robo);
robo_info_pmii_t *robosw_attach_pmii(void);
void robosw_detach_pmii(robo_info_pmii_t *robo);
robo_info_ocp_t *robosw_attach_ocp(void);
void robosw_detach_ocp(robo_info_ocp_t *robo);
void robosw_rreg(robo_info_t *robo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint32 len);
void robosw_wreg(robo_info_t *robo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint32 len);
void robosw_rreg_pmii(robo_info_pmii_t *robo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint32 len);
void robosw_wreg_pmii(robo_info_pmii_t *robo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint32 len);
int robosw_pmii_poll(robo_info_pmii_t *robo);
int robosw_ocp_poll(robo_info_ocp_t *robo, uint32 done_mask);
void robosw_wreg_ocp(robo_info_ocp_t *robo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint32 len);
void robosw_rreg_ocp(robo_info_ocp_t *robo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint32 len);

typedef struct robo_driver_s {
  char  *drv_name;
  ROBO_READ robo_read;
  ROBO_WRITE robo_write;
} robo_driver_t;

#define PMII_WREG(robo,reg,val)  (*(robo->phywr))(robo->ch,PMII_PHYADDR,reg,val)
#define PMII_RREG(robo,reg)  (*(robo->phyrd))(robo->ch,PMII_PHYADDR,reg)

#endif /* _robo_h_ */
