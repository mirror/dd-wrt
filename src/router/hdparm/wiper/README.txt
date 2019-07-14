TRIM / wiper script for SATA SSDs   (June 2010)
=================================================

The wiper.sh script is for tuning up SATA SSDs (Solid-State-Drives).

It calculates a list of free (unallocated) blocks within a filesystem,
and informs the SSD firmware of those blocks, so that it can better manage
the underlying media for wear-leveling and garbage-collection purposes.

In some cases, this can restore a sluggish SSD to nearly-new speeds again.

This script may be EXTREMELY HAZARDOUS TO YOUR DATA.

It does work for me here, but it has not yet been exhaustively tested by others.

Please back-up your data to a *different* physical drive before trying it.
And if you are at all worried, then DO NOT USE THIS SCRIPT!!

DO NOT USE THIS SCRIPT if you cannot afford losing your data!!

This script works for read-write mounted ext4 and xfs filesystems,
and for read-only mounted/unmounted ext2, ext3, ext4, reiser3 and xfs filesystems.

As of Version 3.1, hfsplus and ntfs filesystem types are also supported,
but this code has not been widely tested yet.  BE CAREFUL!

Invoke the script with the pathname to the mounted filesystem
or the block device path for the filesystem.

	Eg.	./wiper.sh /boot
		./wiper.sh /
		./wiper.sh /dev/sda1

Note that the most comprehensive results are achieved when
wiping a filesystem that is not currently mounted read-write,
though the difference is small.

==================================================

btrfs -- DO NOT USE !!!

Chris Mason, the primary author/maintainer of btrfs, believes that
the FIEMAP/FIBMAP ioctl() calls are completely unsafe when used on
a btrfs filesystem.  Even when only a single device is involved.
This seems rather strange. If true, those ioctls() should be removed from btrfs.

But there are other issues, as well.

btrfs breaks the Linux filesystem model in many ways, making it rather dangerous
to your data to try and TRIM it.  It implements it's own internal multiple-device
layer, similar to DM/MD/VFS, but without any indication to external utilities like wiper.sh.
As a result, detection of the underlying device for the filesystem is haphazard at best,
and this could cause wiper.sh to destroy data on whatever device it thinks is the correct one.

Also, because of the built-in duplication of multiple-device support, the FIBMAP and FIEMAP
ioctl()s will work incorrectly on btrfs when more than a single device is involved.
This means that btrfs will mislead the wiper.sh script, causing it to TRIM the WRONG sectors,
destroying valuable data, programs, and filesystem metadata.  You will lose everything.

Finally, due to the non-standard internal volume/device remapping done by btrfs,
it is very difficult for standard Linux tools like hdparm and wiper.sh to actually
determine the device that lies underneath a given file.  Odd, but true.

So support for btrfs has been dropped as of wiper-2.5.
It used to work for single drives, but as of the Linux-2.6.31 kernel even hdparm
is now failing for simple operations like obtaining drive geometries from /sys on btrfs.

btrfs is an experimental beta with serious issues; use ext4 or xfs instead.

==================================================

The sil24_trim_protocol_fix.patch file in this directory is a kernel
patch for all recent Linux kernel versions up to and including 2.6.31.

This fixes the kernel device driver for the Silicon Image SiI-3132
SATA controller to correctly pass DSM/TRIM commands to the drives.

If you use this hardware in your system, then you will need to apply
the patch to your kernel before using the wiper scripts.

==================================================

The fiemap_compat_ioctl.patch file in this directory is a kernel patch
to speed up "hdparm --fibmap" when run as a 32-bit program on top of
a 64-bit Linux kernel.  Kernels versions up to and including 2.6.31
are missing support for this, so hdparm will fall back to the older
and slower FIBMAP call, causing wiper.sh to take much longer to run.
The older call has other limitations, such as failing on really large
files or huge disks, so use of FIEMAP really is preferred.

As of August 16, a similar patch has now been backported to the -stable
streams of most recent Linux kernel versions.  So update your kernel
and this functionality will already be included.

