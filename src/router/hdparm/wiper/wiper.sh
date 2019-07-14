#!/bin/bash
#
# SATA SSD free-space TRIM utility, by Mark Lord <mlord@pobox.com>

VERSION=3.6 

# Copyright (C) 2009-2010 Mark Lord.  All rights reserved.
#
# Contains hfsplus and ntfs code contributed by Heiko Wegeler <heiko.wegeler@googlemail.com>.
# Package sleuthkit version >=3.1.1 is required for HFS+. Package ntfs-3g and ntfsprogs is required for NTFS.
#
# Requires gawk, a really-recent hdparm, and various other programs.
# This needs to be redone entirely in C, for 64-bit math, someday.
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License Version 2,
# as published by the Free Software Foundation.
# 
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Note for OCZ Vertex-LE users:  the drive firmware will error when
# attempting to trim the final sector of the drive.  To avoid this,
# partition the drive such that the final sector is not used.

export LANG=C

## The usual terse usage information:
##
function usage_error(){
	echo >&2
	echo "Linux tune-up (TRIM) utility for SATA SSDs"
	echo "Usage:  $0 [--verbose] [--commit] <mount_point|block_device>" >&2
	echo "   Eg:  $0 /dev/sda1" >&2
	echo >&2
	exit 1
}

## Parameter parsing for the main script.
## Yeah, we could use getopt here instead, but what fun would that be?
##

echo
echo "${0##*/}: Linux SATA SSD TRIM utility, version $VERSION, by Mark Lord."

export verbose=0
commit=""
destroy_me=""
argc=$#
arg=""
while [ $argc -gt 0 ]; do
	if [ "$1" = "--commit" ]; then
		commit=yes
	elif [ "$1" = "--please-prematurely-wear-out-my-ssd" ]; then
		destroy_me=yes
	elif [ "$1" = "--verbose" ]; then
		verbose=$((verbose + 1))
	elif [ "$1" = "" ]; then
		usage_error
	else
		if [ "$arg" != "" ]; then
			echo "$1: too many arguments, aborting." >&2
			exit 1
		fi
		arg="$1"
	fi
	argc=$((argc - 1))
	shift
done
[ "$arg" = "" ] && usage_error

## Find a required program, or else give a nicer error message than we'd otherwise see:
##
function find_prog(){
	prog="$1"
	if [ ! -x "$prog" ]; then
		prog="${prog##*/}"
		p=`type -f -P "$prog" 2>/dev/null`
		if [ "$p" = "" ]; then
			[ "$2" != "quiet" ] && echo "$1: needed but not found, aborting." >&2
			exit 1
		fi
		prog="$p"
		[ $verbose -gt 0 ] && echo "  --> using $prog instead of $1" >&2
	fi
	echo "$prog"
}

## Ensure we have most of the necessary utilities available before trying to proceed:
##
hash -r  ## Refresh bash's cached PATH entries
HDPARM=`find_prog /sbin/hdparm`	|| exit 1
FIND=`find_prog /usr/bin/find`	|| exit 1
STAT=`find_prog /usr/bin/stat`	|| exit 1
GAWK=`find_prog /usr/bin/gawk`	|| exit 1
BLKID=`find_prog /sbin/blkid`	|| exit 1
GREP=`find_prog /bin/grep`	|| exit 1
ID=`find_prog /usr/bin/id`	|| exit 1
LS=`find_prog /bin/ls`		|| exit 1
DF=`find_prog /bin/df`		|| exit 1
RM=`find_prog /bin/rm`		|| exit 1
STAT=`find_prog /usr/bin/stat`	|| exit 1

[ $verbose -gt 1 ] && HDPARM="$HDPARM --verbose"

## I suppose this will confuse the three SELinux users out there:
##
if [ `$ID -u` -ne 0 ]; then
	echo "Only the super-user can use this (try \"sudo $0\" instead), aborting." >&2
	exit 1
fi

## We need a very modern hdparm, for its --fallocate and --trim-sector-ranges-stdin flags:
## Version 9.25 added automatic determination of safe max-size of TRIM commands.
##
HDPVER=`$HDPARM -V | $GAWK '{gsub("[^0-9.]","",$2); if ($2 > 0) print ($2 * 100); else print 0; exit(0)}'`
if [ $HDPVER -lt 925 ]; then
	echo "$HDPARM: version >= 9.25 is required, aborting." >&2
	exit 1
fi

## Convert relative path "$1" into an absolute pathname, resolving all symlinks:
##
function get_realpath(){
	iter=0
	p="$1"
	while [ -e "$p" -a $iter -lt 100 ]; do
		## Strip trailing slashes:
		while [ "$p" != "/" -a "$p" != "${p%%/}" ]; do
			p="${p%%/}"
		done
		## Split into directory:leaf portions:
		d="${p%/*}"
		t="${p##*/}"
		## If the split worked, then cd into the directory portion:
		if [ "$d" != "" -a "$d" != "$p" ]; then
			cd -P "$d" || exit
			p="$t"
		fi
		## If what we have left is a directory, then cd to it and print realpath:
		if [ -d "$p" ]; then
			cd -P "$p" || exit
			pwd -P
			exit
		## Otherwise if it is a symlink, read the link and loop again:
		elif [ -h "$p" ]; then
			p="`$LS -ld "$p" | awk '{sub("^[^>]*-[>] *",""); print}'`"
		## Otherwise, prefix $p with the cwd path and print it:
		elif [ -e "$p" ]; then
			[ "${p:0:1}" = "/" ] || p="`pwd -P`/$p"
			echo "$p"
			exit
		fi
		iter=$((iter + 1))
	done
}

function get_devpath(){
	dir="$1"
	kdev=`$STAT --format="%04D" "$dir" 2>/dev/null`
	[ "$kdev" = "" ] && exit 1
	major=$((0x${kdev:0:2}))
	minor=$((0x${kdev:2:2}))
	$FIND /dev -xdev -type b -exec $LS -ln {} \; | $GAWK -v major="$major," -v minor="$minor" \
		'($5 == major && $6 == minor){r=$NF}END{print r}'
}

## Convert "$arg" into an absolute pathname target, with no symlinks or embedded blanks:
target="`get_realpath "$arg"`"
if [ "$target" = "" ]; then
	[ "$arg" = "/dev/root" ] && target="`get_devpath /`"
	if [ "$target" = "" ]; then
		echo "$arg: unable to determine full pathname, aborting." >&2
		exit 1
	fi
fi
if [ "$target" != "${target##* }" ]; then
	echo "\"$target\": pathname has embedded blanks, aborting." >&2
	exit 1
fi

## Take a first cut at online/offline determination, based on the target:
##
if [ -d "$target" ]; then
	method=online
elif [ -b "$target" ]; then
	method=offline
else
	echo "$target: not a block device or mount point, aborting." >&2
	exit 1
fi

## Find the active mount-point (fsdir) associated with a device ($1: fsdev).
## This is complicated, and probably still buggy, because a single
## device can show up under *multiple* mount points in /proc/mounts.
##
function get_fsdir(){
	rw=""
	r=""
	while read -a m ; do
		pdev="${m[0]}"
		[ "$pdev" = "$1" ] || pdev="`get_realpath "$pdev"`"
		if [ "$pdev" = "$1" ]; then
			if [ "$rw" != "rw" ]; then
				rw="${m[3]:0:2}"
				r="${m[1]}"
			fi
		fi
		#echo "$pdev ${m[1]} ${m[2]} ${m[3]}"
	done
	echo -n "$r"
}

## Find the device (fsdev) associated with a mount point ($1: fsdir).
## Since mounts can be stacked on top of each other, we return the
## one from the last occurance in the list from /proc/mounts.
##
function get_fsdev(){   ## from fsdir
	get_realpath "`$GAWK -v p="$1" '{if ($2 == p) r=$1} END{print r}' < /proc/mounts`"
}

## Find the r/w or r/o status (fsmode) of a filesystem mount point  ($1: fsdir)
## We get it from the last occurance of the mount point in the list from /proc/mounts,
## and convert it to a longer human-readable string.
##
function get_fsmode(){  ## from fsdir
	mode="`$GAWK -v p="$1" '{if ($2 == p) r=substr($4,1,2)} END{print r}' < /proc/mounts`"
	if [ "$mode" = "ro" ]; then
		echo "read-only"
	elif [ "$mode" = "rw" ]; then
		echo "read-write"
	else
		echo "$fsdir: unable to determine mount status, aborting." >&2
		exit 1
	fi
}

## Try and determine the device name associated with the root filesystem.
## This is nearly impossible to do in any perfect fashion.
##
## Redhat/Fedora no longer have an rdev command.  Silly them.
## So we now implement it internally, below.
##
## match_rootdev *should* work, but on some distros it may find only "/dev/root",
## and "/dev/root" is not usually a real device.  We leave it like that for now,
## because that's the pattern such systems also use in /proc/mounts.
## Later, at time of use, we'll try harder to find the real rootdev.
##
## FIXME: apparently this doesn't work on SuSE Linux, though.
## So for there, we'll likely need to read /etc/mtab,
## or be a lot more clever and get it somehow from statfs or something.
## FIXME: or use target from /dev/root symlink for Gentoo as well.
##
function match_rootdev() {
	rdev=""
	rdevno="$1"
	while read bdev ; do
		if [ "$rdev" = "" -o "$bdev" != "/dev/root" ]; then
			devno=$($STAT -c "0x%t%02T" "$bdev" 2>/dev/null)
			[ "$devno" = "$rdevno" ] && rdev="$bdev"
		fi
	done
	echo -n "$rdev"
}

rootdev=$($FIND /dev/ -type b 2>/dev/null | match_rootdev $($STAT -c "0x%D" '/'))
[ $verbose -gt 0 ] && echo "rootdev=$rootdev"

## The user gave us a directory (mount point) to TRIM,
## which implies that we will be doing an online TRIM
## using --fallocate and --fibmap to find the free extents.
## Do some preliminary correctness/feasibility checks on fsdir:
##
if [ "$method" = "online" ]; then
	## Ensure fsdir exists and is accessible to us:
	fsdir="$target"
	cd "$fsdir" || exit 1

	if [ "$fsdir" = "/" ]; then
		fsdev="$rootdev"
	else
		## Figure out what device holds the filesystem.
		fsdev="`get_fsdev $fsdir`"
		if [ "$fsdev" = "" ]; then
			echo "$fsdir: not found in /proc/mounts, aborting." >&2
			exit 1
		fi
	fi

	## The root filesystem may show up as the phoney "/dev/root" device
	## in /proc/mounts (ugh).  So if we see that, then substitute the rootdev
	## that $DF gave us earlier.  But $DF may have the same problem (double ugh).
	##
	[ ! -e "$fsdev" -a "$fsdev" = "/dev/root" ] && fsdev="$rootdev"

	## Ensure that fsdev exists and is a block device:
	if [ ! -e "$fsdev" ]; then
		if [ "$fsdev" != "/dev/root" ]; then
			echo "$fsdev: not found" >&2
			exit 1
		fi
		if [ "$rootdev" = "" ]; then
			echo "$fsdev: not found" >&2
			exit 1
		fi
		fsdev="$rootdev"
	fi
	if [ ! -b "$fsdev" ]; then
		echo "$fsdev: not a block device" >&2
		exit 1
	fi

	## If it is mounted read-only, we must switch to doing an "offline" trim of fsdev:
	fsmode="`get_fsmode $fsdir`" || exit 1
	[ $verbose -gt 0 ] && echo "fsmode1: fsmode=$fsmode"
	[ "$fsmode" = "read-only" ] && method=offline
fi

## This is not an "else" clause from the above, because "method" may have changed.
## For offline TRIM, we need the block device, and it cannot be mounted read-write:
##
if [ "$method" = "offline" ]; then
	## We might already have fsdev/fsdir from above; if not, we need to find them.
	if [ "$fsdev" = "" -o "$fsdir" = "" ]; then
		fsdev="$target"
		fsdir="`get_fsdir "$fsdev" < /proc/mounts`"
		## More weirdness for /dev/root in /proc/mounts:
		if [ "$fsdir" = "" -a "$fsdev" = "$rootdev" ]; then
			fsdir="`get_fsdir /dev/root < /proc/mounts`"
			if [ "$fsdir" = "" ]; then
				rdev="`get_devpath /`"
				[ "$rdev" != "" ] && fsdir="`get_fsdir "$rdev" < /proc/mounts`"
			fi
		fi
	fi

	## If the filesystem is truly not-mounted, then fsdir will still be empty here.
	## It could be mounted, though.  Read-only is fine, but read-write means we need
	## to switch gears and do an "online" TRIM instead of an "offline" TRIM.
	##
	if [ "$fsdir" != "" ]; then
		fsmode="`get_fsmode $fsdir`" || exit 1
		[ $verbose -gt 0 ] && echo "fsmode2: fsmode=$fsmode"
		if [ "$fsmode" = "read-write" ]; then
			method=online
			cd "$fsdir" || exit 1
		fi
	fi
fi

## Use $LS to find the major number of a block device:
##
function get_major(){
	$LS -ln "$1" | $GAWK '{print gensub(",","",1,$5)}'
}

## At this point, we have finalized our selection of online vs. offline,
## and we definitely know the fsdev, as well as the fsdir (fsdir="" if not-mounted).
##
## Now guess at the underlying rawdev name, which could be exactly the same as fsdev.
## Then determine whether or not rawdev claims support for TRIM commands.
## Note that some devices lie about support, and later reject the TRIM commands.
##
rawdev=`echo $fsdev | $GAWK '{print gensub("[0-9]*$","","g")}'`
rawdev="`get_realpath "$rawdev"`"
if [ ! -e "$rawdev" ]; then
	rawdev=""
elif [ ! -b "$rawdev" ]; then
	rawdev=""
elif [ "`get_major $fsdev`" -ne "`get_major $rawdev`" ]; then  ## sanity check
	rawdev=""
else
	## "SCSI" drives only; no LVM confusion for now:
	maj="$(get_major $fsdev)"
	maj_ok=0
	for scsi_major in 8 65 66 67 68 69 70 71 ; do
		[ "$maj" = "$scsi_major" ] && maj_ok=1
	done
	if [ $maj_ok -eq 0 ]; then
		echo "$rawdev: does not appear to be a SCSI/SATA SSD, aborting." >&2
		exit 1
	fi
	if ! $HDPARM -I $rawdev | $GREP -i '[ 	][*][ 	]*Data Set Management TRIM supported' &>/dev/null ; then
		if [ "$commit" = "yes" ]; then
			echo "$rawdev: DSM/TRIM command not supported, aborting." >&2
			exit 1
		fi
		echo "$rawdev: DSM/TRIM command not supported (continuing with dry-run)." >&2
	fi
fi
if [ "$rawdev" = "" ]; then
	echo "$fsdev: unable to reliably determine the underlying physical device name, aborting" >&2
	exit 1
fi

## We also need to know the offset of fsdev from the beginning of rawdev,
## because TRIM requires absolute sector numbers within rawdev:
##
fsoffset=`$HDPARM -g "$fsdev" | $GAWK 'END {print $NF}'`

## Next step is to determine what type of filesystem we are dealing with (fstype):
##
if [ "$fsdir" = "" ]; then
	## Not mounted: use $BLKID to determine the fstype of fsdev:
	fstype=`$BLKID -w /dev/null -c /dev/null $fsdev 2>/dev/null | \
		 $GAWK '/ TYPE=".*"/{sub("^.* TYPE=\"",""); sub("[\" ][\" ]*.*$",""); print}'`
	[ $verbose -gt 0 ] && echo "$fsdev: fstype=$fstype"
else
	## Mounted: we could just use $BLKID here, too, but it's safer to use /proc/mounts directly:
	fstype="`$GAWK -v p="$fsdir" '{if ($2 == p) r=$3} END{print r}' < /proc/mounts`"
	[ $verbose -gt 0 ] && echo "$fsdir: fstype=$fstype"
fi
if [ "$fstype" = "" ]; then
	echo "$fsdev: unable to determine filesystem type, aborting." >&2
	exit 1
fi

## Some helper funcs and vars for use with the xfs filesystem tools:
##
function xfs_abort(){
	echo "$fsdev: unable to determine xfs filesystem ${1-parameters}, aborting." >&2
	exit 1
}
function xfs_trimlist(){
	$XFS_DB -r -c "freesp -d" "$fsdev"  ## couldn't get this to work inline
}
xfs_agoffsets=""
xfs_blksects=0

## We used to allow single-drive btrfs here, but it stopped working in linux-2.6.31,
## and Chris Mason says "unsafe at any speed" really.  So it's been dropped now.
##
if [ "$fstype" = "btrfs" ]; then  ## hdparm --fibmap fails, due to fake 0:xx device nodes
	echo "$target: btrfs filesystem type not supported (cannot determine physical devices), aborting." >&2
	exit 1
fi

## Now figure out whether we can actually do TRIM on this type of filesystem:
##
if [ "$method" = "online" ]; then
	## Print sensible error messages for some common situations,
	## rather than failing with more confusing messages later on..
	##
	if [ "$fstype" = "ext2" -o "$fstype" = "ext3" ]; then  ## No --fallocate support
		echo "$target: cannot TRIM $fstype filesystem when mounted read-write, aborting." >&2
		exit 1
	fi

	## Figure out if we have enough free space to even attempt TRIM:
	##
	freesize=`$DF -P -B 1024 . | $GAWK '{r=$4}END{print r}'`
	if [ "$freesize" = "" ]; then
		echo "$fsdev: unknown to '$DF'"
		exit 1
	fi
	if [ $freesize -lt 15000 ]; then
		echo "$target: filesystem too full for TRIM, aborting." >&2
		exit 1
	fi

	## Figure out how much space to --fallocate (later), keeping in mind
	## that this is a live filesystem, and we need to leave some space for
	## other concurrent activities, as well as for filesystem overhead (metadata).
	## So, reserve at least 1% or 7500 KB, whichever is larger:
	##
	reserved=$((freesize / 100))
	[ $reserved -lt 7500 ] && reserved=7500
	[ $verbose -gt 0 ] && echo "freesize = ${freesize} KB, reserved = ${reserved} KB"
	tmpsize=$((freesize - reserved))
	tmpfile="WIPER_TMPFILE.$$"
	get_trimlist="$HDPARM --fibmap $tmpfile"
else
	## We can only do offline TRIM on filesystems that we "know" about here.
	## Currently, this includes the ext2/3/4 family, xfs, and reiserfs.
	## The first step for any of these is to ensure that the filesystem is "clean",
	## and immediately abort if it is not.
	##
	get_trimlist=""
	if [ "$fstype" = "ext2" -o "$fstype" = "ext3" -o "$fstype" = "ext4" ]; then
		DUMPE2FS=`find_prog /sbin/dumpe2fs` || exit 1
		fstate="`$DUMPE2FS $fsdev 2>/dev/null | $GAWK '/^[Ff]ilesystem state:/{print $NF}' 2>/dev/null`"
		if [ "$fstate" != "clean" ]; then
			echo "$target: filesystem not clean, please run \"e2fsck $fsdev\" first, aborting." >&2
			exit 1
		fi
		get_trimlist="$DUMPE2FS $fsdev"
	elif [ "$fstype" = "xfs" ]; then
		XFS_DB=`find_prog /sbin/xfs_db` || exit 1
		XFS_REPAIR=`find_prog /sbin/xfs_repair` || exit 1
		if ! $XFS_REPAIR -n "$fsdev" &>/dev/null ; then
			echo "$fsdev: filesystem not clean, please run \"xfs_repair $fsdev\" first, aborting." >&2
			exit 1
		fi

		## For xfs, life is more complex than with ext2/3/4 above.
		## The $XFS_DB tool does not return absolute block numbers for freespace,
		## but rather gives them as relative to it's allocation groups (ag's).
		## So, we'll need to interogate it for the offset of each ag within the filesystem.
		## The agoffsets are extracted from $XFS_DB as sector offsets within the fsdev.
		##
		agcount=`$XFS_DB -r -c "sb" -c "print agcount" "$fsdev" | $GAWK '{print 0 + $NF}'`
		[ "$agcount" = "" -o "$agcount" = "0" ] && xfs_abort "agcount"
		xfs_agoffsets=
		i=0
		while [ $i -lt $agcount ]; do
			agoffset=`$XFS_DB -r -c "sb" -c "convert agno $i daddr" "$fsdev" \
				| $GAWK '{print 0 + gensub("[( )]","","g",$2)}'`
			[ "$agoffset" = "" ] && xfs_abort "agoffset-$i"
			[ $i -gt 0 ] && [ $agoffset -le ${xfs_agoffsets##* } ] && xfs_abort "agoffset[$i]"
			xfs_agoffsets="$xfs_agoffsets $agoffset"
			i=$((i + 1))
		done
		xfs_agoffsets="${xfs_agoffsets:1}"	## strip leading space

		## We also need xfs_blksects for later, because freespace gets listed as block numbers.
		##
		blksize=`$XFS_DB -r -c "sb" -c "print blocksize" "$fsdev" | $GAWK '{print 0 + $NF}'`
		[ "$blksize" = "" -o "$blksize" = "0" ] && xfs_abort "block size"
		xfs_blksects=$((blksize/512))
		get_trimlist="xfs_trimlist"
	elif [ "$fstype" = "reiserfs" ]; then
		DEBUGREISERFS=`find_prog /sbin/debugreiserfs` || exit 1
		( $DEBUGREISERFS $fsdev | $GREP '^Filesystem state:.consistent' ) &> /dev/null
		if [ $? -ne 0 ]; then
			echo "Please run fsck.reiserfs first, aborting." >&2
			exit 1
		fi
		get_trimlist="$DEBUGREISERFS -m $fsdev"
	elif [ "$fstype" = "hfsplus" ]; then
		OD=`find_prog /usr/bin/od` || exit 1
		TR=`find_prog /usr/bin/tr` || exit 1
		#check sleuthkit
		FSSTAT=`find_prog /usr/local/bin/fsstat` 
		if [ "$?" = "1" ]; then
			echo "fsstat and icat from package sleuthkit >= 3.1.1 is required for hfsplus."
			exit 1
		fi
		ICAT=`find_prog /usr/local/bin/icat` 
		if [ "`$ICAT -f list 2>/dev/stdout|$GREP HFS+`" = "" ]; then
                        echo "Wrong icat, version from package sleuthkit >= 3.1.1 is required for hfsplus."
                        exit 1
                fi
		#check for unmounted properly
		if [ "`$FSSTAT -f hfs $fsdev | $GREP "Volume Unmounted Properly"`" = ""  ]; then
			echo "Hfsplus volume unmounted improperly!"
			exit 1
		fi
		#check $AllocationFile inode
		FFIND=`find_prog /usr/local/bin/ffind`
		if [ "`$FFIND -f hfs $fsdev 6`" != "/\$AllocationFile" ]; then
			echo "Hfsplus bitmap \$AllocationFile is not inode 6!"
			exit 1
		fi
		#get offset for hfsplus with a wrapper
		hfsoffset=`$FSSTAT -f hfs $fsdev | $GREP "File system is embedded in an HFS wrapper at offset "|$TR -d "\t"`
		if [ -n "$hfsoffset" ]; then
			hfsoffset=${hfsoffset:52}
			((fsoffset=fsoffset+hfsoffset))
			echo "File system is embedded in an HFS wrapper at offset $hfsoffset"
		fi
		blksize=`$FSSTAT -f hfs $fsdev | $GREP "Allocation Block Size: "|$TR -d "\t"`
		blksize=${blksize:23}
		blksects=$((blksize / 512))
		#get count of used bytes in $AllocationFile
		blkcount=`$FSSTAT -f hfs $fsdev | $GREP "Block Range: 0 - "`
		blkcount=${blkcount:17}
		bytecount=$((blkcount/blksects))
		
		method="bitmap_offline"
		get_trimlist="echo $blksects hfsplus `$ICAT -f hfs $fsdev 6 | $OD -N $bytecount -An -vtu1 -j0 -w1`"
	elif [ "$fstype" = "ntfs" ]; then
		NTFSINFO=`find_prog /usr/bin/ntfsinfo` || exit 1
		NTFSCAT=`find_prog /usr/bin/ntfscat` || exit 1
		NTFSPROBE=`find_prog /usr/bin/ntfs-3g.probe` || exit 1
		OD=`find_prog /usr/bin/od` || exit 1
		TR=`find_prog /usr/bin/tr` || exit 1
		#check for unmounted properly
		$NTFSPROBE -w $fsdev 2>/dev/null
		if [ $? -ne 0 ]; then
			echo "$fsdev contains an unclean file system!"
			exit 1
		fi
		#check for volume version
		if [ "`$NTFSINFO -m -f $fsdev | $GREP "Volume Version: 3.1"`" = "" ]; then
			echo "NTFS volume version must be 3.1!"
			exit 1
		fi
		blksize=`$NTFSINFO -m -f $fsdev | $GREP "Cluster Size: " | $TR -d "\t"`
		blksize=${blksize:14}
		blksects=$((blksize / 512))
		#get count of used bytes in $Bitmap
		blkcount=`$NTFSINFO -m -f $fsdev | $GREP "Volume Size in Clusters: " | $TR -d "\t"`
		blkcount=${blkcount:25}
		bytecount=$((blkcount/blksects))

		method="bitmap_offline"
		get_trimlist="echo $blksects ntfs `$NTFSCAT $fsdev \\\$Bitmap | $OD -N $bytecount -An -vtu1 -j0 -w1`"
	fi
	if [ "$get_trimlist" = "" ]; then
		echo "$target: offline TRIM not supported for $fstype filesystems, aborting." >&2
		exit 1
	fi
fi

## All ready.  Now let the user know exactly what we intend to do:
##
mountstatus="$fstype non-mounted"
[ "$fsdir" = "" ] || mountstatus="$fstype mounted $fsmode at $fsdir"
echo "Preparing for $method TRIM of free space on $fsdev ($mountstatus)."

## If they specified "--commit" on the command line, then prompt for confirmation first:
##
if [ "$commit" = "yes" ]; then
	if [ "$destroy_me" = "" ]; then
		echo >/dev/tty
		echo -n "This operation could silently destroy your data.  Are you sure (y/N)? " >/dev/tty
		read yn < /dev/tty
		if [ "$yn" != "y" -a "$yn" != "Y" ]; then
			echo "Aborting." >&2
			exit 1
		fi
	fi
	TRIM="$HDPARM --please-destroy-my-drive --trim-sector-ranges-stdin $rawdev"
else
	echo "This will be a DRY-RUN only.  Use --commit to do it for real."
	TRIM="$GAWK {}"
fi

## Useful in a few places later on:
##
function sync_disks(){
	echo -n "Syncing disks.. "
	sync
	echo
}

## Clean up tmpfile (if any) and exit:
##
function do_cleanup(){
	if [ "$method" = "online" ]; then
		if [ -e $tmpfile ]; then
			echo "Removing temporary file.."
			$RM -f $tmpfile
		fi
		sync_disks
	fi
	[ $1 -eq 0 ] && echo "Done."
	[ $1 -eq 0 ] || echo "Aborted." >&2
	exit $1
}

## Prepare signal handling, in case we get interrupted while $tmpfile exists:
##
function do_abort(){
	echo
	do_cleanup 1
}
trap do_abort SIGTERM
trap do_abort SIGQUIT
trap do_abort SIGINT
trap do_abort SIGHUP
trap do_abort SIGPIPE

## For online TRIM, go ahead and create the huge temporary file.
## This is where we finally discover whether the filesystem actually
## supports --fallocate or not.  Some folks will be disappointed here.
##
## Note that --fallocate does not actually write any file data to fsdev,
## but rather simply allocates formerly-free space to the tmpfile.
##
if [ "$method" = "online" ]; then
	if [ -e "$tmpfile" ]; then
		if ! $RM -f "$tmpfile" ; then
			echo "$tmpfile: already exists and could not be removed, aborting." >&2
			exit 1
		fi
	fi
	echo -n "Allocating temporary file (${tmpsize} KB).. "
	if ! $HDPARM --fallocate "${tmpsize}" $tmpfile ; then
		echo "$target: this kernel may not support 'fallocate' on a $fstype filesystem, aborting." >&2
		exit 1
	fi
	echo
fi

## Finally, we are now ready to TRIM something!
##
## Feed the "get_trimlist" output into a gawk program which will
## extract the trimable lba-ranges (extents) and batch them together
## into huge --trim-sector-ranges calls.
##
## We are limited by at least one thing when doing this:
##   1. Some device drivers may not support more than 255 sectors
##      full of lba:count range data per TRIM command.
## The latest hdparm versions now take care of that automatically.
##
sync_disks
if [ "$commit" = "yes" ]; then
	echo "Beginning TRIM operations.."
else
	echo "Simulating TRIM operations.."
fi
[ $verbose -gt 0 ] && echo "get_trimlist=$get_trimlist"

## Begin gawk program
GAWKPROG='
	BEGIN {
		if (xfs_agoffsets != "") {
			method = "xfs_offline"
			agcount = split(xfs_agoffsets,agoffset," ");
		}
	}
	function append_range (lba,count  ,this_count){
		nsectors += count;
		while (count > 0) {
			this_count  = (count > 65535) ? 65535 : count
			printf "%u:%u ", lba, this_count
			if (verbose > 1)
				printf "%u:%u ", lba, this_count > "/dev/stderr"
			lba        += this_count
			count      -= this_count
			nranges++;
		}
	}
	(method == "online") {	## Output from "hdparm --fibmap", in absolute sectors:
		if (NF == 4 && $2 ~ "^[1-9][0-9]*$")
			append_range($2,$4)
		next
	}
	(method == "xfs_offline") { ## Output from xfs_db:
		if (NF == 3 && gensub("[0-9 ]","","g",$0) == "" && $1 < agcount) {
			lba   = agoffset[1 + $1] + ($2 * xfs_blksects) + fsoffset
			count = $3 * xfs_blksects
			append_range(lba,count)
		}
		next
	}
	(method == "bitmap_offline") {
		n = split($0,f)
		blksects = f[1]
		fstype = f[2]
		bitmap_start = 3
		range_first = -1 #clusters
		range_last = -1
		for (i = bitmap_start; i <= n-1; i++) {
			if (f[i] == 0) {
				if (range_first == -1)
					range_first = (i-bitmap_start) * 8 
				range_last = (i-bitmap_start) * 8 + 7
			} else if (f[i] == 255 && range_first > -1){
				#printf range_first "-" range_last "\n" > "/dev/stderr"
				lba = (range_first * blksects) + fsoffset
				count = (range_last - range_first + 1) * blksects
				append_range(lba,count)
				range_first = -1
				range_last = -1
			} else {
				for (b = 0; b < 8; b++) {
					if (fstype == "ntfs")
						bit = and(f[i], lshift(1, b)) ? 1 : 0
					else #hfsplus
						bit = and(f[i], lshift(1, 7-b)) ? 1 : 0
					if (bit == 0) {
						if (range_first == -1) {
							range_first = (i-bitmap_start) * 8 + b
							range_last = (i-bitmap_start) * 8 + b
						} else
							range_last += 1
					} else if (range_first > -1) {
						#printf range_first "-" range_last " " > "/dev/stderr"
						lba = (range_first * blksects) + fsoffset
						count = (range_last - range_first + 1) * blksects
						if (fstype == "ntfs")
							append_range(lba,count)
						else if (count > (2 * blksects)) #faster for hfsplus
							append_range(lba,count)
						range_first = -1
						range_last = -1
					}
				}
			}
		}
		if (range_first > -1){
			#printf range_first "-" range_last " " > "/dev/stderr"
			lba = (range_first * blksects) + fsoffset
			count = (range_last - range_first + 1) * blksects
			append_range(lba,count)
		}
		next
	}
	/^Block size: *[1-9]/ {	## First stage output from dumpe2fs:
		blksects = $NF / 512
		next
	}
	/^Group [0-9][0-9]*:/ {	## Second stage output from dumpe2fs:
		in_groups = 1
		next
	}
	/^ *Free blocks: [0-9]/	{ ## Bulk of output from dumpe2fs:
		if (blksects && in_groups) {
			n = split(substr($0,16),f,",*  *")
			for (i = 1; i <= n; ++i) {
				if (f[i] ~ "^[1-9][0-9]*-[1-9][0-9]*$") {
					split(f[i],b,"-")
					lba   = (b[1] * blksects) + fsoffset
					count = (b[2] - b[1] + 1) * blksects
					append_range(lba,count)
				} else if (f[i] ~ "^[1-9][0-9]*$") {
					lba   = (f[i] * blksects) + fsoffset
					count = blksects
					append_range(lba,count)
				}
			}
			next
		}
	}
	/^Reiserfs super block/ {
		method = "reiserfs"
		next
	}
	/^Blocksize: / {
		if (method == "reiserfs") {
			blksects = $2 / 512
			next
		}
	}
	/^#[0-9][0-9]*:.*Free[(]/ { ## debugreiserfs
		if (method == "reiserfs" && blksects > 0) {
			n = split($0,f)
			for (i = 4; i <= n; ++i) {
				if (f[i] ~ "^ *Free[(]") {
					if (2 == split(gensub("[^-0-9]","","g",f[i]),b,"-")) {
						lba = (b[1] * blksects) + fsoffset
						count = (b[2] - b[1] + 1) * blksects
						append_range(lba, count)
					}
				}
			}
			next
		}
	}
	END {
		if (err == 0 && commit != "yes")
			printf "(dry-run) trimming %u sectors from %u ranges\n", nsectors, nranges > "/dev/stderr"
		exit err
	}'
## End gawk program

$get_trimlist 2>/dev/null | $GAWK		\
	-v commit="$commit"			\
	-v method="$method"			\
	-v rawdev="$rawdev"			\
	-v fsoffset="$fsoffset"			\
	-v verbose="$verbose"			\
	-v xfs_blksects="$xfs_blksects"		\
	-v xfs_agoffsets="$xfs_agoffsets"	\
	"$GAWKPROG" | $TRIM

do_cleanup $?
