/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
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

#include <xfs/xfs.h>
#include <xfs/command.h>
#include "init.h"
#include "io.h"

static cmdinfo_t bmap_cmd;

static void
bmap_help(void)
{
	printf(_(
"\n"
" prints the block mapping for an XFS file's data or attribute forks"
"\n"
" Example:\n"
" 'bmap -vp' - tabular format verbose map, including unwritten extents\n"
"\n"
" bmap prints the map of disk blocks used by the current file.\n"
" The map lists each extent used by the file, as well as regions in the\n"
" file that do not have any corresponding blocks (holes).\n"
" By default, each line of the listing takes the following form:\n"
"     extent: [startoffset..endoffset]: startblock..endblock\n"
" Holes are marked by replacing the startblock..endblock with 'hole'.\n"
" All the file offsets and disk blocks are in units of 512-byte blocks.\n"
" -a -- prints the attribute fork map instead of the data fork.\n"
" -d -- suppresses a DMAPI read event, offline portions shown as holes.\n"
" -l -- also displays the length of each extent in 512-byte blocks.\n"
" -n -- query n extents.\n"
" -p -- obtain all unwritten extents as well (w/ -v show which are unwritten.)\n"
" -v -- Verbose information, specify ag info.  Show flags legend on 2nd -v\n"
" Note: the bmap for non-regular files can be obtained provided the file\n"
" was opened appropriately (in particular, must be opened read-only).\n"
"\n"));
}

static int
numlen(
	off64_t	val)
{
	off64_t	tmp;
	int	len;

	for (len = 0, tmp = val; tmp > 0; tmp = tmp/10)
		len++;
	return (len == 0 ? 1 : len);
}

int
bmap_f(
	int			argc,
	char			**argv)
{
	struct fsxattr		fsx;
	struct getbmapx		*map;
	struct xfs_fsop_geom	fsgeo;
	int			map_size;
	int			loop = 0;
	int			flg = 0;
	int			aflag = 0;
	int			lflag = 0;
	int			nflag = 0;
	int			pflag = 0;
	int			vflag = 0;
	int			is_rt = 0;
	int			bmv_iflags = 0;	/* flags for XFS_IOC_GETBMAPX */
	int			i = 0;
	int			c;
	int			egcnt;

	while ((c = getopt(argc, argv, "adln:pv")) != EOF) {
		switch (c) {
		case 'a':	/* Attribute fork. */
			bmv_iflags |= BMV_IF_ATTRFORK;
			aflag = 1;
			break;
		case 'l':	/* list number of blocks with each extent */
			lflag = 1;
			break;
		case 'n':	/* number of extents specified */
			nflag = atoi(optarg);
			break;
		case 'd':
		/* do not recall possibly offline DMAPI files */
			bmv_iflags |= BMV_IF_NO_DMAPI_READ;
			break;
		case 'p':
		/* report unwritten preallocated blocks */
			pflag = 1;
			bmv_iflags |= BMV_IF_PREALLOC;
			break;
		case 'v':	/* Verbose output */
			vflag++;
			break;
		default:
			return command_usage(&bmap_cmd);
		}
	}
	if (aflag)
		bmv_iflags &= ~(BMV_IF_PREALLOC|BMV_IF_NO_DMAPI_READ);

	if (vflag) {
		c = xfsctl(file->name, file->fd, XFS_IOC_FSGEOMETRY_V1, &fsgeo);
		if (c < 0) {
			fprintf(stderr,
				_("%s: can't get geometry [\"%s\"]: %s\n"),
				progname, file->name, strerror(errno));
			exitcode = 1;
			return 0;
		}
		c = xfsctl(file->name, file->fd, XFS_IOC_FSGETXATTR, &fsx);
		if (c < 0) {
			fprintf(stderr,
				_("%s: cannot read attrs on \"%s\": %s\n"),
				progname, file->name, strerror(errno));
			exitcode = 1;
			return 0;
		}

		if (fsx.fsx_xflags == XFS_XFLAG_REALTIME) {
			/*
			 * ag info not applicable to rt, continue
			 * without ag output.
			 */
			is_rt = 1;
		}
	}

	map_size = nflag ? nflag+2 : 32;	/* initial guess - 32 */
	map = malloc(map_size*sizeof(*map));
	if (map == NULL) {
		fprintf(stderr, _("%s: malloc of %d bytes failed.\n"),
			progname, (int)(map_size * sizeof(*map)));
		exitcode = 1;
		return 0;
	}


/*	Try the xfsctl(XFS_IOC_GETBMAPX) for the number of extents specified
 *	by nflag, or the initial guess number of extents (32).
 *
 *	If there are more extents than we guessed, use xfsctl
 *	(XFS_IOC_FSGETXATTR[A]) to get the extent count, realloc some more
 *	space based on this count, and try again.
 *
 *	If the initial FGETBMAPX attempt returns EINVAL, this may mean
 *	that we tried the FGETBMAPX on a zero length file.  If we get
 *	EINVAL, check the length with fstat() and return "no extents"
 *	if the length == 0.
 *
 *	Why not do the xfsctl(XFS_IOC_FSGETXATTR[A]) first?  Two reasons:
 *	(1)	The extent count may be wrong for a file with delayed
 *		allocation blocks.  The XFS_IOC_GETBMAPX forces the real
 *		allocation and fixes up the extent count.
 *	(2)	For XFS_IOC_GETBMAP[X] on a DMAPI file that has been moved
 *		offline by a DMAPI application (e.g., DMF) the
 *		XFS_IOC_FSGETXATTR only reflects the extents actually online.
 *		Doing XFS_IOC_GETBMAPX call first forces that data blocks online
 *		and then everything proceeds normally (see PV #545725).
 *
 *		If you don't want this behavior on a DMAPI offline file,
 *		try the "-d" option which sets the BMV_IF_NO_DMAPI_READ
 *		iflag for XFS_IOC_GETBMAPX.
 */

	do {	/* loop a miximum of two times */

		memset(map, 0, sizeof(*map));	/* zero header */

		map->bmv_length = -1;
		map->bmv_count = map_size;
		map->bmv_iflags = bmv_iflags;

		i = xfsctl(file->name, file->fd, XFS_IOC_GETBMAPX, map);
		if (i < 0) {
			if (   errno == EINVAL
			    && !aflag && filesize() == 0) {
				break;
			} else	{
				fprintf(stderr, _("%s: xfsctl(XFS_IOC_GETBMAPX)"
					" iflags=0x%x [\"%s\"]: %s\n"),
					progname, map->bmv_iflags, file->name,
					strerror(errno));
				free(map);
				exitcode = 1;
				return 0;
			}
		}
		if (nflag)
			break;
		if (map->bmv_entries < map->bmv_count-1)
			break;
		/* Get number of extents from xfsctl XFS_IOC_FSGETXATTR[A]
		 * syscall.
		 */
		i = xfsctl(file->name, file->fd, aflag ?
				XFS_IOC_FSGETXATTRA : XFS_IOC_FSGETXATTR, &fsx);
		if (i < 0) {
			fprintf(stderr, "%s: xfsctl(XFS_IOC_FSGETXATTR%s) "
				"[\"%s\"]: %s\n", progname, aflag ? "A" : "",
				file->name, strerror(errno));
			free(map);
			exitcode = 1;
			return 0;
		}
		if (2 * fsx.fsx_nextents > map_size) {
			map_size = 2 * fsx.fsx_nextents + 1;
			map = realloc(map, map_size*sizeof(*map));
			if (map == NULL) {
				fprintf(stderr,
					_("%s: cannot realloc %d bytes\n"),
					progname, (int)(map_size*sizeof(*map)));
				exitcode = 1;
				return 0;
			}
		}
	} while (++loop < 2);
	if (!nflag) {
		if (map->bmv_entries <= 0) {
			printf(_("%s: no extents\n"), file->name);
			free(map);
			return 0;
		}
	}
	egcnt = nflag ? min(nflag, map->bmv_entries) : map->bmv_entries;
	printf("%s:\n", file->name);
	if (!vflag) {
		for (i = 0; i < egcnt; i++) {
			printf("\t%d: [%lld..%lld]: ", i,
				(long long) map[i + 1].bmv_offset,
				(long long)(map[i + 1].bmv_offset +
				map[i + 1].bmv_length - 1LL));
			if (map[i + 1].bmv_block == -1)
				printf(_("hole"));
			else {
				printf("%lld..%lld",
					(long long) map[i + 1].bmv_block,
					(long long)(map[i + 1].bmv_block +
						map[i + 1].bmv_length - 1LL));

			}
			if (lflag)
				printf(_(" %lld blocks\n"),
					(long long)map[i+1].bmv_length);
			else
				printf("\n");
		}
	} else {
		/*
		 * Verbose mode displays:
		 *   extent: [startoffset..endoffset]: startblock..endblock \
		 *	ag# (agoffset..agendoffset) totalbbs
		 */
#define MINRANGE_WIDTH	16
#define MINAG_WIDTH	2
#define MINTOT_WIDTH	5
#define NFLG		5	/* count of flags */
#define	FLG_NULL	000000	/* Null flag */
#define	FLG_PRE		010000	/* Unwritten extent */
#define	FLG_BSU		001000	/* Not on begin of stripe unit  */
#define	FLG_ESU		000100	/* Not on end   of stripe unit  */
#define	FLG_BSW		000010	/* Not on begin of stripe width */
#define	FLG_ESW		000001	/* Not on end   of stripe width */
		int	agno;
		off64_t agoff, bbperag;
		int	foff_w, boff_w, aoff_w, tot_w, agno_w;
		char	rbuf[32], bbuf[32], abuf[32];
		int	sunit, swidth;

		foff_w = boff_w = aoff_w = MINRANGE_WIDTH;
		tot_w = MINTOT_WIDTH;
		if (is_rt)
			sunit = swidth = bbperag = 0;
		else {
			bbperag = (off64_t)fsgeo.agblocks *
				  (off64_t)fsgeo.blocksize / BBSIZE;
			sunit = (fsgeo.sunit * fsgeo.blocksize) / BBSIZE;
			swidth = (fsgeo.swidth * fsgeo.blocksize) / BBSIZE;
		}
		flg = sunit | pflag;

		/*
		 * Go through the extents and figure out the width
		 * needed for all columns.
		 */
		for (i = 0; i < egcnt; i++) {
			snprintf(rbuf, sizeof(rbuf), "[%lld..%lld]:",
				(long long) map[i + 1].bmv_offset,
				(long long)(map[i + 1].bmv_offset +
				map[i + 1].bmv_length - 1LL));
			if (map[i + 1].bmv_oflags & BMV_OF_PREALLOC)
				flg = 1;
			if (map[i + 1].bmv_block == -1) {
				foff_w = max(foff_w, strlen(rbuf));
				tot_w = max(tot_w,
					numlen(map[i+1].bmv_length));
			} else {
				snprintf(bbuf, sizeof(bbuf), "%lld..%lld",
					(long long) map[i + 1].bmv_block,
					(long long)(map[i + 1].bmv_block +
						map[i + 1].bmv_length - 1LL));
				boff_w = max(boff_w, strlen(bbuf));
				if (!is_rt) {
					agno = map[i + 1].bmv_block / bbperag;
					agoff = map[i + 1].bmv_block -
							(agno * bbperag);
					snprintf(abuf, sizeof(abuf),
						"(%lld..%lld)",
						(long long)agoff,
						(long long)(agoff +
						 map[i + 1].bmv_length - 1LL));
					aoff_w = max(aoff_w, strlen(abuf));
				} else
					aoff_w = 0;
				foff_w = max(foff_w, strlen(rbuf));
				tot_w = max(tot_w,
					numlen(map[i+1].bmv_length));
			}
		}
		agno_w = is_rt ? 0 : max(MINAG_WIDTH, numlen(fsgeo.agcount));
		printf("%4s: %-*s %-*s %*s %-*s %*s%s\n",
			_("EXT"),
			foff_w, _("FILE-OFFSET"),
			boff_w, is_rt ? _("RT-BLOCK-RANGE") : _("BLOCK-RANGE"),
			agno_w, is_rt ? "" : _("AG"),
			aoff_w, is_rt ? "" : _("AG-OFFSET"),
			tot_w, _("TOTAL"),
			flg ? _(" FLAGS") : "");
		for (i = 0; i < egcnt; i++) {
			flg = FLG_NULL;
			if (map[i + 1].bmv_oflags & BMV_OF_PREALLOC) {
				flg |= FLG_PRE;
			}
			/*
			 * If striping enabled, determine if extent starts/ends
			 * on a stripe unit boundary.
			 */
			if (sunit) {
				if (map[i + 1].bmv_block  % sunit != 0) {
					flg |= FLG_BSU;
				}
				if (((map[i + 1].bmv_block +
				      map[i + 1].bmv_length ) % sunit ) != 0) {
					flg |= FLG_ESU;
				}
				if (map[i + 1].bmv_block % swidth != 0) {
					flg |= FLG_BSW;
				}
				if (((map[i + 1].bmv_block +
				      map[i + 1].bmv_length ) % swidth ) != 0) {
					flg |= FLG_ESW;
				}
			}
			snprintf(rbuf, sizeof(rbuf), "[%lld..%lld]:",
				(long long) map[i + 1].bmv_offset,
				(long long)(map[i + 1].bmv_offset +
				map[i + 1].bmv_length - 1LL));
			if (map[i + 1].bmv_block == -1) {
				printf("%4d: %-*s %-*s %*s %-*s %*lld\n",
					i,
					foff_w, rbuf,
					boff_w, _("hole"),
					agno_w, "",
					aoff_w, "",
					tot_w, (long long)map[i+1].bmv_length);
			} else {
				snprintf(bbuf, sizeof(bbuf), "%lld..%lld",
					(long long) map[i + 1].bmv_block,
					(long long)(map[i + 1].bmv_block +
						map[i + 1].bmv_length - 1LL));
				printf("%4d: %-*s %-*s", i, foff_w, rbuf,
					boff_w, bbuf);
				if (!is_rt) {
					agno = map[i + 1].bmv_block / bbperag;
					agoff = map[i + 1].bmv_block -
							(agno * bbperag);
					snprintf(abuf, sizeof(abuf),
						"(%lld..%lld)",
						(long long)agoff,
						(long long)(agoff +
						 map[i + 1].bmv_length - 1LL));
					printf(" %*d %-*s", agno_w, agno,
						aoff_w, abuf);
				} else
					printf("  ");
				printf(" %*lld", tot_w,
					(long long)map[i+1].bmv_length);
				if (flg == FLG_NULL && !pflag) {
					printf("\n");
				} else {
					printf(" %-*.*o\n", NFLG, NFLG, flg);
				}
			}
		}
		if ((flg || pflag) && vflag > 1) {
			printf(_(" FLAG Values:\n"));
			printf(_("    %*.*o Unwritten preallocated extent\n"),
				NFLG+1, NFLG+1, FLG_PRE);
			printf(_("    %*.*o Doesn't begin on stripe unit\n"),
				NFLG+1, NFLG+1, FLG_BSU);
			printf(_("    %*.*o Doesn't end   on stripe unit\n"),
				NFLG+1, NFLG+1, FLG_ESU);
			printf(_("    %*.*o Doesn't begin on stripe width\n"),
				NFLG+1, NFLG+1, FLG_BSW);
			printf(_("    %*.*o Doesn't end   on stripe width\n"),
				NFLG+1, NFLG+1, FLG_ESW);
		}
	}
	free(map);
	return 0;
}

void
bmap_init(void)
{
	bmap_cmd.name = "bmap";
	bmap_cmd.cfunc = bmap_f;
	bmap_cmd.argmin = 0;
	bmap_cmd.argmax = -1;
	bmap_cmd.flags = CMD_NOMAP_OK;
	bmap_cmd.args = _("[-adlpv] [-n nx]");
	bmap_cmd.oneline = _("print block mapping for an XFS file");
	bmap_cmd.help = bmap_help;

	add_command(&bmap_cmd);
}
