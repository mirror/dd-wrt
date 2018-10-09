/*
 * Copyright (C) 2007-2008 Intel Corporation
 *
 *	Retrieve drive serial numbers for scsi disks
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <string.h>
#include <scsi/scsi.h>
#include <scsi/sg.h>
#include <sys/ioctl.h>

int scsi_get_serial(int fd, void *buf, size_t buf_len)
{
	unsigned char rsp_buf[255];
	unsigned char inq_cmd[] = {INQUIRY, 1, 0x80, 0, sizeof(rsp_buf), 0};
	unsigned char sense[32];
	struct sg_io_hdr io_hdr;
	int rv;
	unsigned int rsp_len;

	memset(&io_hdr, 0, sizeof(io_hdr));
	io_hdr.interface_id = 'S';
	io_hdr.cmdp = inq_cmd;
	io_hdr.cmd_len = sizeof(inq_cmd);
	io_hdr.dxferp = rsp_buf;
	io_hdr.dxfer_len = sizeof(rsp_buf);
	io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
	io_hdr.sbp = sense;
	io_hdr.mx_sb_len = sizeof(sense);
	io_hdr.timeout = 5000;

	rv = ioctl(fd, SG_IO, &io_hdr);

	if (rv)
		return rv;

	if ((io_hdr.info & SG_INFO_OK_MASK) != SG_INFO_OK)
		return -1;

	rsp_len = rsp_buf[3];

	if (!rsp_len || buf_len < rsp_len)
		return -1;

	memcpy(buf, &rsp_buf[4], rsp_len);

	return 0;
}
