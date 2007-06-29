/* ------------------------------------------------------------------------- */
/* 									     */
/* spi-id.h - definitions for the spi-bus interface			     */
/* 									     */
/* ------------------------------------------------------------------------- */
/*   Copyright (C) 1995-1999 Barnabas Kalman

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.		     */
/* ------------------------------------------------------------------------- */

#ifndef LINUX_SPI_ID_H
#define LINUX_SPI_ID_H

/*
 * This file is part of the spi-bus package and contains the identifier
 * values for drivers, adapters and other folk populating these serial
 * worlds. 
 *
 * These will change often (i.e. additions) , therefore this has been 
 * separated from the functional interface definitions of the spi api.
 *
 */

/*
 * ---- Driver types -----------------------------------------------------
 *       device id name + number        function description, spi address(es)
 *
 */

#define SPI_DRIVERID_KS8995M	 1

#define SPI_DRIVERID_EXP0	0xF0	/* experimental use id's	*/
#define SPI_DRIVERID_EXP1	0xF1
#define SPI_DRIVERID_EXP2	0xF2
#define SPI_DRIVERID_EXP3	0xF3

#define SPI_DRIVERID_SPIDEV	900
#define SPI_DRIVERID_SPIPROC	901

/*
 * ---- Adapter types ----------------------------------------------------
 *
 * First, we distinguish between several algorithms to access the hardware
 * interface types, as a PCF 8584 needs other care than a bit adapter.
 */

#define SPI_ALGO_NONE	0x000000
#define SPI_ALGO_BIT	0x010000	/* bit style adapters		*/

#define SPI_HW_ADAPS	0x10000		/* # adapter types		*/
#define SPI_HW_MASK	0x0ffff		


/* hw specific modules that are defined per algorithm layer
 */

/* --- Bit algorithm adapters 						*/
#define SPI_HW_B_IXP4XX 0x01	/* GPIO on IXP4XX systems		*/

#endif /* LINUX_SPI_ID_H */
