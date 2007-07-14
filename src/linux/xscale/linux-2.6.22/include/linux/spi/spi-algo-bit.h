/* ------------------------------------------------------------------------- */
/* spi-algo-bit.h spi driver algorithms for bit-shift adapters               */
/* ------------------------------------------------------------------------- */
/*   Copyright (C) 2005 Barnabas Kalman <ba...@sednet.hu>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                */
/* ------------------------------------------------------------------------- */

#ifndef _LINUX_SPI_ALGO_BIT_H
#define _LINUX_SPI_ALGO_BIT_H

/* --- Defines for bit-adapters ---------------------------------------	*/
/*
 * This struct contains the hw-dependent functions of bit-style adapters to 
 * manipulate the line states, and to init any hw-specific features. This is
 * only used if you have more than one hw-type of adapter running. 
 */
struct spi_algo_bit_data {
	void *data;		/* private data for lowlevel routines */
	void (*setspis)(void *data, int index);
	void (*setspic)(void *data, int state);
	void (*setspid)(void *data, int state);
	int  (*getspiq)(void *data);

	/* local settings */
	int udelay;		/* half-clock-cycle time in microsecs */
				/* i.e. clock is (500 / udelay) KHz */
	int mdelay;		/* in millisecs, unused */
	int timeout;		/* in jiffies */
};

#define SPI_BIT_ADAP_MAX	16

int spi_bit_add_bus(struct spi_adapter *);
int spi_bit_del_bus(struct spi_adapter *);

#endif /* _LINUX_SPI_ALGO_BIT_H */
