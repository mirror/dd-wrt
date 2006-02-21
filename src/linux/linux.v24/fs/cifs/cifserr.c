/*
 *   fs/cifserr.c
 *
 *   Copyright (c) International Business Machines  Corp., 2002
 *   Author(s): Steve French (sfrench@us.ibm.com)
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation; either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/smbno.h>
#include "cifsfs.h"

int map_cifs_error(int error_class, int error_code,
		   int status_codes_negotiated)
{


	if (status_codes_negotiated) {
		switch (error_code) {
		default:
			return EIO;
		}
	} else
		switch (error_class) {
		case SUCCESS:
			return 0;

		case ERRDOS:
			switch (error_code) {
			case ERRbadfunc:
				return EINVAL;
			default:
				return EIO;
			}

		case ERRSRV:
			switch (error_code) {
			default:
				return EIO;
			}

		case ERRHRD:
			switch (error_code) {
			default:
				return EIO;
			}
		default:
			return EIO;
		}
	return 0;
}

int map_smb_error(int error_class, int error_code)
{
	return map_cifs_error(error_class, error_code, FALSE);
}
