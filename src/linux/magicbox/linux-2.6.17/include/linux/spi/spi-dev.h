/*
    spi-dev.h - spi-bus driver, char device interface

    Copyright (C) 2005 Barnabas Kalman <barnik@sednet.hu>

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

#ifndef _LINUX_SPI_DEV_H
#define _LINUX_SPI_DEV_H

#include <linux/types.h>
#include <linux/compiler.h>

/* Some IOCTL commands are defined in <linux/spi/spi.h> */

/* This is the structure as used in the I2C_RDWR ioctl call */
struct spi_rdwr_ioctl_data {
	struct spi_msg __user *msgs;	/* pointers to spi_msgs */
	__u32 nmsgs;			/* number of spi_msgs */
};

#define  SPI_RDRW_IOCTL_MAX_MSGS	42

#endif /* _LINUX_SPI_DEV_H */
