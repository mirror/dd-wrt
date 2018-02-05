/*
 * Copyright (c) 2013 Red Hat, Inc.
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

#include "libxfs.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "bit.h"
#include "init.h"


/*
 * XXX: no idea how to handle multiple contiguous block symlinks here.
 */
static int
symlink_count(
	void		*obj,
	int		startoff)
{
	struct xfs_dsymlink_hdr	*hdr = obj;

	ASSERT(startoff == 0);

	if (hdr->sl_magic != cpu_to_be32(XFS_SYMLINK_MAGIC))
		return 0;
	if (be32_to_cpu(hdr->sl_bytes) + sizeof(*hdr) > mp->m_sb.sb_blocksize)
		return mp->m_sb.sb_blocksize - sizeof(*hdr);
	return be32_to_cpu(hdr->sl_bytes);
}

int
symlink_size(
	void	*obj,
	int	startoff,
	int	idx)
{
	struct xfs_dsymlink_hdr	*hdr = obj;

	ASSERT(startoff == 0);
	if (hdr->sl_magic != cpu_to_be32(XFS_SYMLINK_MAGIC))
		return 0;
	return be32_to_cpu(hdr->sl_bytes) + sizeof(*hdr);
}

const struct field	symlink_crc_hfld[] = {
	{ "", FLDT_SYMLINK_CRC, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};

#define	OFF(f)	bitize(offsetof(struct xfs_dsymlink_hdr, sl_ ## f))
#define	SZOF(f)	bitize(sizeof(struct xfs_dsymlink_hdr))
const struct field	symlink_crc_flds[] = {
	{ "magic", FLDT_UINT32X, OI(OFF(magic)), C1, 0, TYP_NONE },
	{ "offset", FLDT_UINT32D, OI(OFF(offset)), C1, 0, TYP_NONE },
	{ "bytes", FLDT_UINT32D, OI(OFF(bytes)), C1, 0, TYP_NONE },
	{ "crc", FLDT_CRC, OI(OFF(crc)), C1, 0, TYP_NONE },
	{ "uuid", FLDT_UUID, OI(OFF(uuid)), C1, 0, TYP_NONE },
	{ "owner", FLDT_INO, OI(OFF(owner)), C1, 0, TYP_NONE },
	{ "bno", FLDT_DFSBNO, OI(OFF(blkno)), C1, 0, TYP_BMAPBTD },
	{ "lsn", FLDT_UINT64X, OI(OFF(lsn)), C1, 0, TYP_NONE },
	{ "data", FLDT_CHARNS, OI(bitize(sizeof(struct xfs_dsymlink_hdr))),
		symlink_count, FLD_COUNT, TYP_NONE },
	{ NULL }
};
