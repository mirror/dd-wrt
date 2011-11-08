/*
 * Copyright (c) 2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "quota.h"
#include <sys/quota.h>

static int
xcommand_to_qcommand(
	uint		command,
	uint		type)
{
	switch (command) {
	case XFS_QUOTAON:
		return Q_XQUOTAON;
	case XFS_QUOTAOFF:
		return Q_XQUOTAOFF;
	case XFS_GETQUOTA:
		if (type == XFS_GROUP_QUOTA)
			return Q_XGETGQUOTA;
		if (type == XFS_PROJ_QUOTA)
			return Q_XGETPQUOTA;
		return Q_XGETQUOTA;
	case XFS_SETQLIM:
		if (type == XFS_GROUP_QUOTA)
			return Q_XSETGQLIM;
		if (type == XFS_PROJ_QUOTA)
			return Q_XSETPQLIM;
		return Q_XSETQLIM;
	case XFS_GETQSTAT:
		return Q_XGETQSTAT;
	case XFS_QUOTARM:
		return Q_XQUOTARM;
	case XFS_QSYNC:
		return Q_SYNC;
	}
	return 0;
}

int
xfsquotactl(
	int		command,
	const char	*device,
	uint		type,
	uint		id,
	void		*addr)
{
	int		qcommand;

	qcommand = xcommand_to_qcommand(command, type);
	return quotactl(qcommand, (char *)device, id, addr);
}
