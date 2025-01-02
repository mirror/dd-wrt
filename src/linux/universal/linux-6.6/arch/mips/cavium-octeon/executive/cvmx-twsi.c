/***********************license start***************
 * Copyright (c) 2003-2010  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

/**
 * @file
 *
 * Interface to the TWSI / I2C bus
 *
 * <hr>$Revision: 130009 $<hr>
 *
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL

#include <linux/export.h>
#include <linux/i2c.h>

#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx-twsi.h>

#else /* #ifdef CVMX_BUILD_FOR_LINUX_KERNEL */

#include "cvmx.h"
#include "cvmx-twsi.h"
#include "cvmx-csr-db.h"

#define TWSI_TIMEOUT	10000	/* 100ms */

/* #define PRINT_TWSI_CONFIG */
#ifdef PRINT_TWSI_CONFIG
#define twsi_printf printf
#else
#define twsi_printf(...)
#define cvmx_csr_db_decode(...)
#endif /*PRINT_TWSI_CONFIG */

#if 0
static int node_bus_to_i2c_bus(int node, int bus)
{
	if (octeon_has_feature(OCTEON_FEATURE_MULTINODE))
		return (node << 1) | bus;
	else
		return bus;
}
#endif

static int i2c_bus_to_node(int i2c_bus)
{
	if (octeon_has_feature(OCTEON_FEATURE_MULTINODE))
		return (i2c_bus >> 1) & 0x3;
	else
		return 0;
}

static int __i2c_twsi_bus(int i2c_bus)
{
	if (octeon_has_feature(OCTEON_FEATURE_MULTINODE))
		return i2c_bus & 0x1;
	else
		return i2c_bus;
}

#endif /* #ifdef CVMX_BUILD_FOR_LINUX_KERNEL */

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
struct i2c_adapter *__cvmx_twsix_get_adapter(int twsi_id)
{
	struct i2c_adapter *adapter;
	int i2c_id;

	i2c_id = octeon_i2c_cvmx2i2c(twsi_id);
	if (i2c_id < 0)
		return NULL;

	adapter = i2c_get_adapter(i2c_id);
	return adapter;
}
EXPORT_SYMBOL(__cvmx_twsix_get_adapter);
#endif

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
/**
 * Unblock the I2C bus.  This should be done during initialization and if the
 * I2C bus gets stuck due to a device resetting unexpectedly.
 */
int cvmx_twsix_unblock(int twsi_id)
{
	cvmx_mio_tws_sw_twsi_t sw_twsi;
	cvmx_mio_tws_int_t tws_int;
	uint64_t old_sw_twsi;
	int i;
	int twsi_bus = __i2c_twsi_bus(twsi_id);

	/* Put the bus in low-level mode */
	old_sw_twsi = cvmx_read_csr_node(i2c_bus_to_node(twsi_id), 
					 CVMX_MIO_TWSX_SW_TWSI(twsi_bus));
	sw_twsi.u64 = 0;
	sw_twsi.s.v = 1;
	sw_twsi.s.op = 6;
	sw_twsi.s.eop_ia = TWSI_CTL;
	sw_twsi.s.d = 0x40;	/* ENAB !CE !AAK */
	cvmx_write_csr_node(i2c_bus_to_node(twsi_id), 
			    CVMX_MIO_TWSX_SW_TWSI(twsi_bus), sw_twsi.u64);
	cvmx_wait_usec(10);
	tws_int.u64 = cvmx_read_csr_node(i2c_bus_to_node(twsi_id), 
					 CVMX_MIO_TWSX_INT(twsi_bus));
	cvmx_wait_usec(10);
	tws_int.s.scl_ovr = 0;
	cvmx_write_csr_node(i2c_bus_to_node(twsi_id), 
			    CVMX_MIO_TWSX_INT(twsi_bus), tws_int.u64);
	cvmx_wait_usec(10);
	for (i = 0; i < 9; i++) {
		tws_int.s.scl_ovr = 1;
		cvmx_write_csr_node(i2c_bus_to_node(twsi_id), 
				    CVMX_MIO_TWSX_INT(twsi_bus), tws_int.u64);
		cvmx_wait_usec(10);
		tws_int.s.scl_ovr = 0;
		cvmx_write_csr_node(i2c_bus_to_node(twsi_id), 
				    CVMX_MIO_TWSX_INT(twsi_bus), tws_int.u64);
		cvmx_wait_usec(10);
	}
	/* Restore back to high level mode */
	cvmx_write_csr_node(i2c_bus_to_node(twsi_id), 
			    CVMX_MIO_TWSX_SW_TWSI(twsi_bus), old_sw_twsi);
	cvmx_wait_usec(10);
	return 0;
}
#endif

/**
 * Do a twsi read from a 7 bit device address using an (optional) internal address.
 * Up to 8 bytes can be read at a time.
 *
 * @param twsi_id   which Octeon TWSI bus to use
 * @param dev_addr  Device address (7 bit)
 * @param internal_addr
 *                  Internal address.  Can be 0, 1 or 2 bytes in width
 * @param num_bytes Number of data bytes to read
 * @param ia_width_bytes
 *                  Internal address size in bytes (0, 1, or 2)
 * @param data      Pointer argument where the read data is returned.
 *
 * @return read data returned in 'data' argument
 *         Number of bytes read on success
 *         -1 on failure
 */
int cvmx_twsix_read_ia(int twsi_id, uint8_t dev_addr, uint16_t internal_addr, 
		       int num_bytes, int ia_width_bytes, uint64_t * data)
{
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	struct i2c_adapter *adapter;
	u8 data_buf[8];
	u8 addr_buf[8];
	struct i2c_msg msg[2];
	uint64_t r;
	int i, j;

	if (ia_width_bytes == 0)
		return cvmx_twsix_read(twsi_id, dev_addr, num_bytes, data);

	BUG_ON(ia_width_bytes > 2);
	BUG_ON(num_bytes > 8 || num_bytes < 1);

	adapter = __cvmx_twsix_get_adapter(twsi_id);
	if (adapter == NULL)
		return -1;

	for (j = 0, i = ia_width_bytes - 1; i >= 0; i--, j++)
		addr_buf[j] = (u8) (internal_addr >> (i * 8));

	msg[0].addr = dev_addr;
	msg[0].flags = 0;
	msg[0].len = ia_width_bytes;
	msg[0].buf = addr_buf;

	msg[1].addr = dev_addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = num_bytes;
	msg[1].buf = data_buf;

	i = i2c_transfer(adapter, msg, 2);

	i2c_put_adapter(adapter);

	if (i == 2) {
		r = 0;
		for (i = 0; i < num_bytes; i++)
			r = (r << 8) | data_buf[i];
		*data = r;
		return num_bytes;
	} else {
		return -1;
	}
#else
	cvmx_mio_twsx_sw_twsi_t sw_twsi_val;
	cvmx_mio_twsx_sw_twsi_ext_t twsi_ext;
	int retry_limit = 5;
	int count = TWSI_TIMEOUT;
	int twsi_bus = __i2c_twsi_bus(twsi_id);

	if (num_bytes < 1 || num_bytes > 8 || !data 
		|| ia_width_bytes < 0 || ia_width_bytes > 2)
		return -1;
retry:
	twsi_ext.u64 = 0;
	sw_twsi_val.u64 = 0;
	sw_twsi_val.s.v = 1;
	sw_twsi_val.s.r = 1;
	sw_twsi_val.s.sovr = 1;
	sw_twsi_val.s.size = num_bytes - 1;
	sw_twsi_val.s.a = dev_addr;

	if (ia_width_bytes > 0) {
		sw_twsi_val.s.op = 1;
		sw_twsi_val.s.ia = (internal_addr >> 3) & 0x1f;
		sw_twsi_val.s.eop_ia = internal_addr & 0x7;
	}
	if (ia_width_bytes == 2) {
		sw_twsi_val.s.eia = 1;
		twsi_ext.s.ia = internal_addr >> 8;
		cvmx_write_csr_node(i2c_bus_to_node(twsi_id), 
				    CVMX_MIO_TWSX_SW_TWSI_EXT(twsi_bus),
				    twsi_ext.u64);
	}

	cvmx_csr_db_decode(cvmx_get_proc_id(), 
			   CVMX_MIO_TWSX_SW_TWSI(twsi_bus), sw_twsi_val.u64);
	cvmx_write_csr_node(i2c_bus_to_node(twsi_id), 
			    CVMX_MIO_TWSX_SW_TWSI(twsi_bus), sw_twsi_val.u64);
	while ((((cvmx_mio_twsx_sw_twsi_t)
		(sw_twsi_val.u64 = cvmx_read_csr_node(i2c_bus_to_node(twsi_id), 
				     CVMX_MIO_TWSX_SW_TWSI(twsi_bus)))).s.v)
	       && --count > 0)
		cvmx_wait_usec(10);
	if (count <= 0) {
		cvmx_twsix_unblock(twsi_id);
		if (retry_limit-- > 0) {
			cvmx_wait_usec(100);
			goto retry;
		}
		return -1;
	}
	twsi_printf("Results:\n");
	cvmx_csr_db_decode(cvmx_get_proc_id(), 
			   CVMX_MIO_TWSX_SW_TWSI(twsi_bus), sw_twsi_val.u64);
	if (!sw_twsi_val.s.r) {
		/* Check the reason for the failure.  We may need to retry to handle multi-master
		 ** configurations.
		 ** Lost arbitration : 0x38, 0x68, 0xB0, 0x78
		 ** Core busy as slave: 0x80, 0x88, 0xA0, 0xA8, 0xB8, 0xC0, 0xC8
		 */
		if (sw_twsi_val.s.d == 0x38
		    || sw_twsi_val.s.d == 0x68
		    || sw_twsi_val.s.d == 0xB0
		    || sw_twsi_val.s.d == 0x78
		    || sw_twsi_val.s.d == 0x80
		    || sw_twsi_val.s.d == 0x88 
		    || sw_twsi_val.s.d == 0xA0 
		    || sw_twsi_val.s.d == 0xA8 
		    || sw_twsi_val.s.d == 0xB8 
		    || sw_twsi_val.s.d == 0xC8) {
			if (retry_limit-- > 0) {
				cvmx_wait_usec(100);
				goto retry;
			}
			return -1;
		}
		/* For all other errors, return an error code */
		return -1;
	}

	if (num_bytes > 4) {
		*data = (sw_twsi_val.s.d & 0xFFFFFFFF);
		twsi_ext.u64 = cvmx_read_csr_node(i2c_bus_to_node(twsi_id), 
						  CVMX_MIO_TWSX_SW_TWSI_EXT(twsi_bus));
		*data |= ((unsigned long long)(twsi_ext.s.d & (0xFFFFFFFF >> (32 - (num_bytes-4) * 8))) << 32);
	} else {
		*data = (sw_twsi_val.s.d & (0xFFFFFFFF >> (32 - num_bytes * 8)));
	}
	return num_bytes;
#endif
}

EXPORT_SYMBOL(cvmx_twsix_read_ia);

/**
 * Read from a TWSI device (7 bit device address only) without generating any
 * internal addresses.
 * Read from 1-8 bytes and returns them in the data pointer.
 *
 * @param twsi_id   TWSI interface on Octeon to use
 * @param dev_addr  TWSI device address (7 bit only)
 * @param num_bytes number of bytes to read
 * @param data      Pointer to data read from TWSI device
 *
 * @return Number of bytes read on success
 *         -1 on error
 */
int cvmx_twsix_read(int twsi_id, uint8_t dev_addr, int num_bytes, uint64_t * data)
{
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	struct i2c_adapter *adapter;
	u8 data_buf[8];
	struct i2c_msg msg[1];
	uint64_t r;
	int i;

	BUG_ON(num_bytes > 8 || num_bytes < 1);

	adapter = __cvmx_twsix_get_adapter(twsi_id);
	if (adapter == NULL)
		return -1;

	msg[0].addr = dev_addr;
	msg[0].flags = I2C_M_RD;
	msg[0].len = num_bytes;
	msg[0].buf = data_buf;

	i = i2c_transfer(adapter, msg, 1);

	i2c_put_adapter(adapter);

	if (i == 1) {
		r = 0;
		for (i = 0; i < num_bytes; i++)
			r = (r << 8) | data_buf[i];
		*data = r;
		return num_bytes;
	} else {
		return -1;
	}
#else
	cvmx_mio_twsx_sw_twsi_t sw_twsi_val;
	cvmx_mio_twsx_sw_twsi_ext_t twsi_ext;
	int retry_limit = 5;
	int count = TWSI_TIMEOUT;
	int twsi_bus = __i2c_twsi_bus(twsi_id);

	if (num_bytes > 8 || num_bytes < 1)
		return -1;
retry:
	sw_twsi_val.u64 = 0;
	sw_twsi_val.s.v = 1;
	sw_twsi_val.s.r = 1;
	sw_twsi_val.s.a = dev_addr;
	sw_twsi_val.s.sovr = 1;
	sw_twsi_val.s.size = num_bytes - 1;

	cvmx_csr_db_decode(cvmx_get_proc_id(), 
			   CVMX_MIO_TWSX_SW_TWSI(twsi_bus), sw_twsi_val.u64);
	cvmx_write_csr_node(i2c_bus_to_node(twsi_id), 
			    CVMX_MIO_TWSX_SW_TWSI(twsi_bus), sw_twsi_val.u64);
	while (((cvmx_mio_twsx_sw_twsi_t)
		(sw_twsi_val.u64 = cvmx_read_csr_node(i2c_bus_to_node(twsi_id), 
						      CVMX_MIO_TWSX_SW_TWSI(twsi_bus)))).s.v
	       && --count > 0)
		cvmx_wait_usec(10);
	if (count <= 0) {
		cvmx_twsix_unblock(twsi_id);
		if (retry_limit-- > 0) {
			cvmx_wait_usec(100);
			goto retry;
		}
		return -1;
	}
	twsi_printf("Results:\n");
	cvmx_csr_db_decode(cvmx_get_proc_id(), 
			   CVMX_MIO_TWSX_SW_TWSI(twsi_bus), sw_twsi_val.u64);
	if (!sw_twsi_val.s.r)
		if (!sw_twsi_val.s.r) {
			/* Check the reason for the failure.  We may need to retry to handle multi-master
			 ** configurations.
			 ** Lost arbitration : 0x38, 0x68, 0xB0, 0x78
			 ** Core busy as slave: 0x80, 0x88, 0xA0, 0xA8, 0xB8, 0xC0, 0xC8
			 */
			if (sw_twsi_val.s.d == 0x38
			    || sw_twsi_val.s.d == 0x68
			    || sw_twsi_val.s.d == 0xB0
			    || sw_twsi_val.s.d == 0x78
			    || sw_twsi_val.s.d == 0x80
			    || sw_twsi_val.s.d == 0x88 
			    || sw_twsi_val.s.d == 0xA0 
			    || sw_twsi_val.s.d == 0xA8 
			    || sw_twsi_val.s.d == 0xB8 
			    || sw_twsi_val.s.d == 0xC8) {
				if (retry_limit-- > 0) {
					cvmx_wait_usec(100);
					goto retry;
				}
			}
			/* For all other errors, return an error code */
			return -1;
		}

	if (num_bytes > 4) {
		*data = (sw_twsi_val.s.d & 0xFFFFFFFF);
		twsi_ext.u64 = cvmx_read_csr_node(i2c_bus_to_node(twsi_id), 
						  CVMX_MIO_TWSX_SW_TWSI_EXT(twsi_bus));
		*data |= ((unsigned long long)(twsi_ext.s.d & (0xFFFFFFFF >> (32 - (num_bytes-4) * 8))) << 32);
	} else {
		*data = (sw_twsi_val.s.d & (0xFFFFFFFF >> (32 - num_bytes * 8)));
	}
	return num_bytes;
#endif
}
EXPORT_SYMBOL(cvmx_twsix_read);

/**
 * Perform a twsi write operation to a 7 bit device address.
 *
 * Note that many eeprom devices have page restrictions regarding address boundaries
 * that can be crossed in one write operation.  This is device dependent, and this routine
 * does nothing in this regard.
 * This command does not generate any internal addressess.
 *
 * @param twsi_id   Octeon TWSI interface to use
 * @param dev_addr  TWSI device address
 * @param num_bytes Number of bytes to write (between 1 and 8 inclusive)
 * @param data      Data to write
 *
 * @return 0 on success
 *         -1 on failure
 */
int cvmx_twsix_write(int twsi_id, uint8_t dev_addr, int num_bytes, uint64_t data)
{
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	struct i2c_adapter *adapter;
	u8 data_buf[8];
	struct i2c_msg msg[1];
	int i, j;

	BUG_ON(num_bytes > 8 || num_bytes < 1);

	adapter = __cvmx_twsix_get_adapter(twsi_id);
	if (adapter == NULL)
		return -1;

	for (j = 0, i = num_bytes - 1; i >= 0; i--, j++)
		data_buf[j] = (u8) (data >> (i * 8));

	msg[0].addr = dev_addr;
	msg[0].flags = 0;
	msg[0].len = num_bytes;
	msg[0].buf = data_buf;

	i = i2c_transfer(adapter, msg, 1);

	i2c_put_adapter(adapter);

	if (i == 1)
		return num_bytes;
	else
		return -1;
#else
	cvmx_mio_twsx_sw_twsi_t sw_twsi_val;
	int count = TWSI_TIMEOUT;
	int retries = 5;
	int twsi_bus = __i2c_twsi_bus(twsi_id);

	if (num_bytes > 8 || num_bytes < 1)
		return -1;

retry:
	if (--retries == 0)
		return -1;

	sw_twsi_val.u64 = 0;
	sw_twsi_val.s.v = 1;
	sw_twsi_val.s.a = dev_addr;
	sw_twsi_val.s.d = data & 0xffffffff;
	sw_twsi_val.s.sovr = 1;
	sw_twsi_val.s.size = num_bytes - 1;
	if (num_bytes > 4) {
		/* Upper four bytes go into a separate register */
		cvmx_mio_twsx_sw_twsi_ext_t twsi_ext;
		twsi_ext.u64 = 0;
		twsi_ext.s.d = data >> 32;
		cvmx_write_csr_node(i2c_bus_to_node(twsi_id), 
				    CVMX_MIO_TWSX_SW_TWSI_EXT(twsi_bus), twsi_ext.u64);
	}
	cvmx_csr_db_decode(cvmx_get_proc_id(), 
			   CVMX_MIO_TWSX_SW_TWSI(twsi_bus), sw_twsi_val.u64);
	cvmx_write_csr_node(i2c_bus_to_node(twsi_id), 
			    CVMX_MIO_TWSX_SW_TWSI(twsi_bus), sw_twsi_val.u64);
	while (((cvmx_mio_twsx_sw_twsi_t) 
		(sw_twsi_val.u64 = cvmx_read_csr_node(i2c_bus_to_node(twsi_id), 
						      CVMX_MIO_TWSX_SW_TWSI(twsi_bus)))).s.v
	       && --count > 0)
		cvmx_wait_usec(10);
	if (count <= 0) {
		cvmx_twsix_unblock(twsi_id);
		goto retry;
	}
	twsi_printf("Results:\n");
	cvmx_csr_db_decode(cvmx_get_proc_id(), 
			   CVMX_MIO_TWSX_SW_TWSI(twsi_bus), sw_twsi_val.u64);
	if (!sw_twsi_val.s.r)
		return -1;

	return 0;
#endif
}
EXPORT_SYMBOL(cvmx_twsix_write);

/**
 * Write 1-8 bytes to a TWSI device using an internal address.
 *
 * @param twsi_id   which TWSI interface on Octeon to use
 * @param dev_addr  TWSI device address (7 bit only)
 * @param internal_addr
 *                  TWSI internal address (0, 8, or 16 bits)
 * @param num_bytes Number of bytes to write (1-8)
 * @param ia_width_bytes
 *                  internal address width, in bytes (0, 1, 2)
 * @param data      Data to write.  Data is written MSB first on the twsi bus, and only the lower
 *                  num_bytes bytes of the argument are valid.  (If a 2 byte write is done, only
 *                  the low 2 bytes of the argument is used.
 *
 * @return Number of bytes read on success,
 *         -1 on error
 */
int cvmx_twsix_write_ia(int twsi_id, uint8_t dev_addr, uint16_t internal_addr, 
			int num_bytes, int ia_width_bytes, uint64_t data)
{
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	struct i2c_adapter *adapter;
	u8 data_buf[8];
	u8 addr_buf[8];
	struct i2c_msg msg[2];
	int i, j;

	if (ia_width_bytes == 0)
		return cvmx_twsix_write(twsi_id, dev_addr, num_bytes, data);

	BUG_ON(ia_width_bytes > 2);
	BUG_ON(num_bytes > 8 || num_bytes < 1);

	adapter = __cvmx_twsix_get_adapter(twsi_id);
	if (adapter == NULL)
		return -1;

	for (j = 0, i = ia_width_bytes - 1; i >= 0; i--, j++)
		addr_buf[j] = (u8) (internal_addr >> (i * 8));

	for (j = 0, i = num_bytes - 1; i >= 0; i--, j++)
		data_buf[j] = (u8) (data >> (i * 8));

	msg[0].addr = dev_addr;
	msg[0].flags = 0;
	msg[0].len = ia_width_bytes;
	msg[0].buf = addr_buf;

	msg[1].addr = dev_addr;
	msg[1].flags = 0;
	msg[1].len = num_bytes;
	msg[1].buf = data_buf;

	i = i2c_transfer(adapter, msg, 2);

	i2c_put_adapter(adapter);

	if (i == 2) {
		/* Poll until reads succeed, or polling times out */
		int to = 100;
		while (to-- > 0) {
			uint64_t data;
			if (cvmx_twsix_read(twsi_id, dev_addr, 1, &data) >= 0)
				break;
		}
	}

	if (i == 2)
		return num_bytes;
	else
		return -1;
#else
	cvmx_mio_twsx_sw_twsi_t sw_twsi_val;
	cvmx_mio_twsx_sw_twsi_ext_t twsi_ext;
	int to;
	int count = TWSI_TIMEOUT;
	int retry_limit = 5;
	int twsi_bus = __i2c_twsi_bus(twsi_id);

	if (num_bytes < 1 || num_bytes > 8 || ia_width_bytes < 0 || ia_width_bytes > 2)
		return -1;
retry:
	twsi_ext.u64 = 0;

	sw_twsi_val.u64 = 0;
	sw_twsi_val.s.v = 1;
	sw_twsi_val.s.sovr = 1;
	sw_twsi_val.s.size = num_bytes - 1;
	sw_twsi_val.s.a = dev_addr;
	sw_twsi_val.s.d = 0xFFFFFFFF & data;

	if (ia_width_bytes > 0) {
		sw_twsi_val.s.op = 1;
		sw_twsi_val.s.ia = (internal_addr >> 3) & 0x1f;
		sw_twsi_val.s.eop_ia = internal_addr & 0x7;
	}
	if (ia_width_bytes == 2) {
		sw_twsi_val.s.eia = 1;
		twsi_ext.s.ia = internal_addr >> 8;
	}
	if (num_bytes > 4)
		twsi_ext.s.d = data >> 32;

	twsi_printf("%s: twsi_id=%x, dev_addr=%x, internal_addr=%x\n"
			"\tnum_bytes=%d, ia_width_bytes=%d, data=%lx\n",
			__func__, twsi_id, dev_addr, internal_addr, 
			num_bytes, ia_width_bytes, data);
	cvmx_csr_db_decode(cvmx_get_proc_id(), 
			   CVMX_MIO_TWSX_SW_TWSI_EXT(twsi_bus), twsi_ext.u64);
	cvmx_write_csr_node(i2c_bus_to_node(twsi_id), 
			    CVMX_MIO_TWSX_SW_TWSI_EXT(twsi_bus), twsi_ext.u64);
	cvmx_csr_db_decode(cvmx_get_proc_id(), 
			   CVMX_MIO_TWSX_SW_TWSI(twsi_bus), sw_twsi_val.u64);
	cvmx_write_csr_node(i2c_bus_to_node(twsi_id), 
			    CVMX_MIO_TWSX_SW_TWSI(twsi_bus), sw_twsi_val.u64);
	while (((cvmx_mio_twsx_sw_twsi_t)
	        (sw_twsi_val.u64 = cvmx_read_csr_node(i2c_bus_to_node(twsi_id), 
						      CVMX_MIO_TWSX_SW_TWSI(twsi_bus)))).s.v
	       && --count > 0)
		cvmx_wait_usec(10);
	if (count <= 0) {
		cvmx_twsix_unblock(twsi_id);
		if (retry_limit-- > 0) {
			cvmx_wait_usec(100);
			goto retry;
		}
		return -1;
	}

	twsi_printf("Results:\n");
	cvmx_csr_db_decode(cvmx_get_proc_id(), 
			   CVMX_MIO_TWSX_SW_TWSI(twsi_bus), sw_twsi_val.u64);
/*	cvmx_csr_db_decode(cvmx_get_proc_id(), CVMX_MIO_TWSX_SW_TWSI(twsi_id & 1), sw_twsi_val.u64); */
	if (!sw_twsi_val.s.r) {
		/* Check the reason for the failure.  We may need to retry to handle multi-master
		 ** configurations.
		 ** Lost arbitration : 0x38, 0x68, 0xB0, 0x78
		 ** Core busy as slave: 0x80, 0x88, 0xA0, 0xA8, 0xB8, 0xC0, 0xC8
		 */
		if (sw_twsi_val.s.d == 0x38
		    || sw_twsi_val.s.d == 0x68
		    || sw_twsi_val.s.d == 0xB0
		    || sw_twsi_val.s.d == 0x78
		    || sw_twsi_val.s.d == 0x80
		    || sw_twsi_val.s.d == 0x88 
		    || sw_twsi_val.s.d == 0xA0 
		    || sw_twsi_val.s.d == 0xA8 
		    || sw_twsi_val.s.d == 0xB8 
		    || sw_twsi_val.s.d == 0xC8) {
			if (retry_limit-- > 0) {
				cvmx_wait_usec(100);
				goto retry;
			}
		}
		/* For all other errors, return an error code */
		return -1;
	}

	/* Poll until reads succeed, or polling times out */
	to = 100;
	while (to-- > 0) {
		uint64_t data;
		if (cvmx_twsix_read(twsi_id, dev_addr, 1, &data) >= 0)
			break;
	}
	if (to <= 0)
		return -1;

	return num_bytes;
#endif
}
EXPORT_SYMBOL(cvmx_twsix_write_ia);

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
/* Controller command patterns */
#define SW_TWSI_EOP_TWSI_DATA   0x1
#define SW_TWSI_EOP_TWSI_CTL    0x2
#define SW_TWSI_EOP_TWSI_CLKCTL 0x3
#define SW_TWSI_EOP_TWSI_STAT   0x3
#define SW_TWSI_EOP_TWSI_RST    0x7

/* Controller command and status bits */
#define TWSI_CTL_CE   0x80
#define TWSI_CTL_ENAB 0x40
#define TWSI_CTL_STA  0x20
#define TWSI_CTL_STP  0x10
#define TWSI_CTL_IFLG 0x08
#define TWSI_CTL_AAK  0x04

/* Some status values */
#define STAT_START      0x08
#define STAT_RSTART     0x10
#define STAT_TXADDR_ACK 0x18
#define STAT_TXDATA_ACK 0x28
#define STAT_RXADDR_ACK 0x40
#define STAT_RXDATA_ACK 0x50
#define STAT_IDLE       0xF8

#define MAX_BUF_SZ 256

static void cvmx_twsix_write_llc_reg(int twsi_id, uint8_t eop_reg, uint8_t data)
{
	cvmx_mio_twsx_sw_twsi_t sw_twsi_val, tmp;
	int twsi_bus = __i2c_twsi_bus(twsi_id);

	sw_twsi_val.u64 = 0;
	sw_twsi_val.s.v = 1;
	sw_twsi_val.s.op = 6;
	sw_twsi_val.s.eop_ia = eop_reg;
	sw_twsi_val.s.d = data;
	cvmx_write_csr_node(i2c_bus_to_node(twsi_id), 
			    CVMX_MIO_TWSX_SW_TWSI(twsi_bus), sw_twsi_val.u64);

	do {
		tmp.u64 = cvmx_read_csr_node(i2c_bus_to_node(twsi_id), 
					     CVMX_MIO_TWSX_SW_TWSI(twsi_bus));
	} while (tmp.s.v != 0);
}

static uint8_t cvmx_twsix_read_llc_reg(int twsi_id, uint8_t eop_reg)
{
	cvmx_mio_twsx_sw_twsi_t sw_twsi_val, tmp;
	int twsi_bus = __i2c_twsi_bus(twsi_id);

	sw_twsi_val.u64 = 0;
	sw_twsi_val.s.v = 1;
	sw_twsi_val.s.op = 6;
	sw_twsi_val.s.eop_ia = eop_reg;
	sw_twsi_val.s.r = 1;
	cvmx_write_csr_node(i2c_bus_to_node(twsi_id), 
			    CVMX_MIO_TWSX_SW_TWSI(twsi_bus), sw_twsi_val.u64);

	do {
		tmp.u64 = cvmx_read_csr_node(i2c_bus_to_node(twsi_id), 
					     CVMX_MIO_TWSX_SW_TWSI(twsi_bus));
	} while (tmp.s.v != 0);

	return tmp.s.d & 0xff;
}

static int cvmx_twsix_wait(int twsi_id)
{
	uint8_t tmp = 0;
	int count = 1000; /* 50 ms */

	while (count--) {
		tmp = cvmx_twsix_read_llc_reg(twsi_id, SW_TWSI_EOP_TWSI_CTL);
		if (tmp & TWSI_CTL_IFLG)
			break;
		cvmx_wait_usec(50);
	}

	return (tmp & TWSI_CTL_IFLG) ? 0 : -1;
}

static int cvmx_twsix_start(int twsi_id)
{
	int ret_val;
	uint8_t tmp;

	cvmx_twsix_write_llc_reg(twsi_id, SW_TWSI_EOP_TWSI_CTL,
				 TWSI_CTL_ENAB | TWSI_CTL_STA);

	ret_val = cvmx_twsix_wait(twsi_id);
	if (ret_val) {
		tmp = cvmx_twsix_read_llc_reg(twsi_id, SW_TWSI_EOP_TWSI_STAT);
		if (tmp == STAT_IDLE) {
			/*
			 * Controller refused to send start flag May
			 * be a client is holding SDA low - let's try
			 * to free it.
			 */
			cvmx_twsix_unblock(twsi_id);
			cvmx_twsix_write_llc_reg(twsi_id, SW_TWSI_EOP_TWSI_CTL,
						 TWSI_CTL_ENAB | TWSI_CTL_STA);

			ret_val = cvmx_twsix_wait(twsi_id);
		}
		if (ret_val)
			return ret_val;
	}

	tmp = cvmx_twsix_read_llc_reg(twsi_id, SW_TWSI_EOP_TWSI_STAT);
	if ((tmp != STAT_START) && (tmp != STAT_RSTART)) {
		twsi_printf("%s %d: bad status (0x%x)\n", __func__, __LINE__,
			    tmp);
		return -1;
	}

	return 0;
}

static int octeon_i2c_lost_arb(uint8_t code)
{
	switch (code) {
	/* Arbitration lost in address or data byte */
	case 0x38:
	/*
	 * Arbitration lost in address as master, slave address +
	 * write bit received, ACK transmitted.
	 */
	case 0x68:
	/*
	 * Arbitration lost in address as master, general call address
	 * received, ACK transmitted.
	 */
	case 0x78:
	/*
	 * Arbitration lost in address as master, slave address + read
	 * bit received, ACK transmitted.
	 */
	case 0xb0:
		return 1;
	default:
		return 0;
	}
}

/*
 * write segment
 */
static int cvmx_twsix_write_seg(int twsi_id, uint8_t dev_addr, uint8_t *buf,
				int length, int phase)
{
	int i, ret_val;
	uint8_t tmp;

restart:
	ret_val = cvmx_twsix_start(twsi_id);
	if (ret_val)
		return ret_val;

	cvmx_twsix_write_llc_reg(twsi_id, SW_TWSI_EOP_TWSI_DATA, dev_addr << 1);
	cvmx_twsix_write_llc_reg(twsi_id, SW_TWSI_EOP_TWSI_CTL, TWSI_CTL_ENAB);

	ret_val = cvmx_twsix_wait(twsi_id);
	if (ret_val)
		return ret_val;

	for (i = 0; i < length; i++) {
		tmp = cvmx_twsix_read_llc_reg(twsi_id, SW_TWSI_EOP_TWSI_STAT);
		if (phase == 0 && octeon_i2c_lost_arb(tmp))
			goto restart;

		if ((tmp != STAT_TXADDR_ACK) && (tmp != STAT_TXDATA_ACK)) {
			twsi_printf("%s %d: bad status before write (0x%x)\n",
				    __func__, __LINE__, tmp);
			return -1;
		}

		cvmx_twsix_write_llc_reg(twsi_id, SW_TWSI_EOP_TWSI_DATA,
					 buf[i]);
		cvmx_twsix_write_llc_reg(twsi_id, SW_TWSI_EOP_TWSI_CTL,
					 TWSI_CTL_ENAB);

		ret_val = cvmx_twsix_wait(twsi_id);
		if (ret_val)
			return ret_val;
	}

	return 0;
}

/*
 * read segment
 */
static int cvmx_twsix_read_seg(int twsi_id, uint8_t dev_addr, uint8_t *buf,
			       int length, int phase)
{
	int i, ret_val;
	uint8_t tmp;

	if (length < 1)
		return -1;

restart:
	ret_val = cvmx_twsix_start(twsi_id);
	if (ret_val)
		return ret_val;

	cvmx_twsix_write_llc_reg(twsi_id, SW_TWSI_EOP_TWSI_DATA,
				 (dev_addr<<1) | 1);
	cvmx_twsix_write_llc_reg(twsi_id, SW_TWSI_EOP_TWSI_CTL, TWSI_CTL_ENAB);

	ret_val = cvmx_twsix_wait(twsi_id);
	if (ret_val)
		return ret_val;

	for (i = 0; i < length; i++) {
		tmp = cvmx_twsix_read_llc_reg(twsi_id, SW_TWSI_EOP_TWSI_STAT);
		if (phase == 0 && octeon_i2c_lost_arb(tmp))
			goto restart;

		if ((tmp != STAT_RXDATA_ACK) && (tmp != STAT_RXADDR_ACK)) {
			twsi_printf("%s %d: bad status before read (0x%x)\n",
				    __func__, __LINE__, tmp);
			return -1;
		}

		if (i+1 < length)
			cvmx_twsix_write_llc_reg(twsi_id, SW_TWSI_EOP_TWSI_CTL,
						 TWSI_CTL_ENAB | TWSI_CTL_AAK);
		else
			cvmx_twsix_write_llc_reg(twsi_id, SW_TWSI_EOP_TWSI_CTL,
						 TWSI_CTL_ENAB);

		ret_val = cvmx_twsix_wait(twsi_id);
		if (ret_val)
			return ret_val;

		buf[i] = cvmx_twsix_read_llc_reg(twsi_id,
						 SW_TWSI_EOP_TWSI_DATA);
	}

	return 0;
}

static int cvmx_twsix_stop(int twsi_id)
{
	uint8_t tmp;

	cvmx_twsix_write_llc_reg(twsi_id, SW_TWSI_EOP_TWSI_CTL,
				 TWSI_CTL_ENAB | TWSI_CTL_STP);

	tmp = cvmx_twsix_read_llc_reg(twsi_id, SW_TWSI_EOP_TWSI_STAT);
	if (tmp != STAT_IDLE) {
		twsi_printf("%s %d: bad status(0x%x)\n", __func__, __LINE__,
			    tmp);
			return -1;
	}

	return 0;
}

/**
 * Read up to 256 bytes to a TWSI device using an internal address.
 *
 * @param twsi_id   Which TWSI interface on Octeon to use
 * @param dev_addr  TWSI device address (7 bit only)
 * @param internal_addr
 *                  TWSI internal address (8 or 16 bits)
 * @param num_bytes Number of bytes to read (1-256)
 * @param ia_width_bytes
 *                  Internal address width, in bytes (1 or 2)
 * @param data      Data to read. Data is read MSB first on the twsi bus,
 *                  and only the higher "num_bytes" bytes of the argument are
 *                  valid. (If a 2 byte read is done, only the high 2 bytes of
 *                  the argument is used.
 * @return          0 on success; -1 on error
 */
int cvmx_twsix_read_ia_llc(int twsi_id, uint8_t dev_addr,
			   uint16_t internal_addr, int num_bytes,
			   int ia_width_bytes, uint64_t *data)
{
	uint8_t buf[MAX_BUF_SZ];
	int i, j, k, ret_val;

	/* always provide internal address */
	if (ia_width_bytes == 0) {
		twsi_printf("%s %d: no internal address\n", __func__, __LINE__);
		return -1;
	}

	/* send internal address. msb first */
	for (i = 0, j = ia_width_bytes-1; j >= 0; i++, j--)
		buf[i] = (uint8_t) (internal_addr >> (j * 8));
	ret_val = cvmx_twsix_write_seg(twsi_id, dev_addr, buf,
				       ia_width_bytes, 0);
	if (ret_val) {
		twsi_printf("%s %d: cannot send internal address\n", __func__,
			    __LINE__);
		return -1;
	}

	/* get data */
	ret_val = cvmx_twsix_read_seg(twsi_id, dev_addr, buf, num_bytes, 1);
	if (ret_val) {
		twsi_printf("%s %d: cannot send internal address\n", __func__,
			    __LINE__);
		return -1;
	}
	for (i = 0, j = 0; i < num_bytes-1; i += 8, j++)
		data[j] = (uint64_t) buf[i] << 56 |
			  (uint64_t) buf[i+1] << 48 |
			  (uint64_t) buf[i+2] << 40 |
			  (uint64_t) buf[i+3] << 32 |
			  (uint64_t) buf[i+4] << 24 |
			  (uint64_t) buf[i+5] << 16 |
			  (uint64_t) buf[i+6] << 8 |
			  buf[i+7];
	k = num_bytes % 8;
	if (k)
		data[j-1] = ((data[j-1] >> (8-k)*8)) << (8-k)*8;

	/* done */
	cvmx_twsix_stop(twsi_id);
	return 0;
}

/**
 * Write up to 256 bytes to a TWSI device using an internal address.
 *
 * @param twsi_id   Which TWSI interface on Octeon to use
 * @param dev_addr  TWSI device address (7 bit only)
 * @param internal_addr
 *                  TWSI internal address (8 or 16 bits)
 * @param num_bytes Number of bytes to write (1-256)
 * @param ia_width_bytes
 *                  Internal address width, in bytes (1 or 2)
 * @param data      Data to write. Data is written MSB first on the twsi bus,
 *                  and only the higher "num_bytes" bytes of the argument are
 *                  valid. (If a 2 byte write is done, only the high 2 bytes of
 *                  the argument is used.
 * @return          0 on success; -1 on error
 */
int cvmx_twsix_write_ia_llc(int twsi_id, uint8_t dev_addr,
			    uint16_t internal_addr, int num_bytes,
			    int ia_width_bytes, uint64_t *data)
{
	uint8_t buf[MAX_BUF_SZ];
	int i, j, k, ret_val;

	/* always provide internal address */
	if (ia_width_bytes == 0) {
		twsi_printf("%s %d: no internal address\n", __func__, __LINE__);
		return -1;
	}

	/* line up internal address. msb first */
	for (i = 0, j = ia_width_bytes-1; j >= 0; i++, j--)
		buf[i] = (uint8_t) (internal_addr >> (j * 8));

	/* plus data */
	k = (num_bytes + 7)/8;
	for (j = 0; j < k; j++, i += 8) {
		buf[i]   = (uint8_t) (data[j] >> 56);
		buf[i+1] = (uint8_t) (data[j] >> 48);
		buf[i+2] = (uint8_t) (data[j] >> 40);
		buf[i+3] = (uint8_t) (data[j] >> 32);
		buf[i+4] = (uint8_t) (data[j] >> 24);
		buf[i+5] = (uint8_t) (data[j] >> 16);
		buf[i+6] = (uint8_t) (data[j] >> 8);
		buf[i+7] = (uint8_t) data[j];
	}

	/* send data */
	ret_val = cvmx_twsix_write_seg(twsi_id, dev_addr, buf,
				       ia_width_bytes + num_bytes, 0);
	if (ret_val) {
		twsi_printf("%s %d: cannot send data\n", __func__, __LINE__);
		return -1;
	}

	/* done */
	cvmx_twsix_stop(twsi_id);
	return 0;
}
#endif
