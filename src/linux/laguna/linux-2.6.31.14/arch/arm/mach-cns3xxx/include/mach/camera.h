/*
    camera.h - CNS3XXX camera driver header file

    Copyright (C) 2003, Intel Corporation
    Copyright (C) 2008, Guennadi Liakhovetski <kernel@pengutronix.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __ASM_ARCH_CAMERA_H_
#define __ASM_ARCH_CAMERA_H_

#define CNS3XXX_CAMERA_MASTER		0x01
#define CNS3XXX_CAMERA_DATAWIDTH_4	0x02
#define CNS3XXX_CAMERA_DATAWIDTH_5	0x04
#define CNS3XXX_CAMERA_DATAWIDTH_8	0x08
#define CNS3XXX_CAMERA_DATAWIDTH_9	0x10
#define CNS3XXX_CAMERA_DATAWIDTH_10	0x20
#define CNS3XXX_CAMERA_PCLK_EN		0x40
#define CNS3XXX_CAMERA_MCLK_EN		0x80
#define CNS3XXX_CAMERA_PCP		0x100
#define CNS3XXX_CAMERA_HSP		0x200
#define CNS3XXX_CAMERA_VSP		0x400

/* Camera Interface */
#define CIM_GLOBAL_REG          0x00    /* CIM control*/
#define CIM_TIMING_V_REG        0x04    /* Vertical capture range setting */
#define CIM_TIMING_H_REG        0x08    /* Horizontal capture range setting */
#define CIM_CCIR656_0_REG       0x0C    /* CCIR656 detect and control setting*/
#define CIM_CCIR656_1_REG       0x10    /* CCIR656 self test setting */
#define CIM_SERIAL_SRC_REG      0x14    /* Serial pix capture module control settings */
#define CIM_INT_MASK_REG        0x28    /* CIM interrupt mask register. */
#define CIM_INT_STATUS_REG      0x2C    /* CIM interrupt status register. */
#define CIM_INT_CLEAR_REG       0x30    /* CIM interrupt clear register. */
#define CIM_DATAPATH_CTL_REG    0x34    /* CIM data path options and control settings */
#define CIM_VIDEO_PORT_REG      0x100   /* CIM¡¦s video port */
#define CIM_CORRECTION_R_REG    0x200   /* Internal programmable table for R component. */
#define CIM_CORRECTION_G_REG    0x600   /* Internal programmable table for G component. */
#define CIM_CORRECTION_B_REG    0xA00   /* Internal programmable table for B component. */

#define SRC_DATA_FMT_CCIR656    0x00
#define SRC_DATA_FMT_YCBCR_A    0x01
#define SRC_DATA_FMT_YCBCR_B    0x02
#define SRC_DATA_FMT_RGB565     0x03
#define SRC_DATA_FMT_RGB555     0x04
#define SRC_DATA_FMT_BAYER_82   0x05
#define SRC_DATA_FMT_BAYER_10   0x06

#define DST_DATA_FMT_RGB888     0x00
#define DST_DATA_FMT_RGB565     0x01
#define DST_DATA_FMT_RGB1555    0x02
#define DST_DATA_FMT_RGB444     0x03

#define CISR_LAST_LINE		(1 << 0)	/* Last line */
#define CISR_FIRST_LINE		(1 << 1)	/* First line */
#define CISR_LINE_END		(1 << 2)	/* Line end */
#define CISR_LINE_START		(1 << 3)	/* Line start */
#define CISR_FIELD_CHG		(1 << 4)	/* Field Change */
#define CISR_FIFO_OVERRUN	(1 << 5)	/* FIFO overrun */


#define CIMR_LAST_LINE_M	(1 << 0)	/* Last line mask*/
#define CIMR_FIRST_LINE_M	(1 << 1)	/* First line mask*/
#define CIMR_LINE_END_M		(1 << 2)	/* Line end mask*/
#define CIMR_LINE_START_M	(1 << 3)	/* Line start mask*/
#define CIMR_FIELD_CHG_M	(1 << 4)	/* Field Change mask*/
#define CIMR_FIFO_OVERRUN_M	(1 << 5)	/* FIFO overrun mask*/


struct cns3xxx_camera_platform_data {
#if 0
	int (*init)(struct device *);
	int (*power)(struct device *, int);
	int (*reset)(struct device *, int);
#endif

	unsigned long flags;
	unsigned long mclk_10khz;
	unsigned long lcd_base;
	unsigned long misc_base;
};

//extern void cns3xxx_set_camera_info(struct pxacamera_platform_data *);

#endif /* __ASM_ARCH_CAMERA_H_ */
