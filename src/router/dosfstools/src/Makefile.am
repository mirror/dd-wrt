# dosfstools src/Makefile.am
# Copyright (C) 2015  Andreas Bombe <aeb@debian.org>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

AM_CFLAGS = -Wall -Wextra -Wno-sign-compare -Wno-missing-field-initializers \
	    -Wmissing-prototypes -Wstrict-prototypes -Wwrite-strings

sbin_PROGRAMS = fsck.fat mkfs.fat fatlabel
noinst_PROGRAMS = testdevinfo

fscklabel_common_sources = boot.c boot.h check.c check.h common.c common.h \
			   fat.c fat.h file.c file.h io.c io.h lfn.c lfn.h \
			   charconv.c charconv.h msdos_fs.h \
			   fsck.fat.h endian_compat.h
fsck_fat_SOURCES = fsck.fat.c $(fscklabel_common_sources)
fatlabel_SOURCES = fatlabel.c $(fscklabel_common_sources)

mkfs_common_sources = device_info.c device_info.h \
		      blkdev/blkdev.c blkdev/blkdev.h \
		      endian_compat.h \
		      blkdev/linux_version.c blkdev/linux_version.h
mkfs_fat_SOURCES  = mkfs.fat.c msdos_fs.h $(mkfs_common_sources)
mkfs_fat_CPPFLAGS = -I$(srcdir)/blkdev
mkfs_fat_CFLAGS   = $(AM_CFLAGS) $(UDEV_CFLAGS)
mkfs_fat_LDADD    = $(UDEV_LIBS)

testdevinfo_SOURCES  = testdevinfo.c $(mkfs_common_sources)
testdevinfo_CPPFLAGS = -I$(srcdir)/blkdev
testdevinfo_CFLAGS   = $(AM_CFLAGS) $(UDEV_CFLAGS)
testdevinfo_LDADD    = $(UDEV_LIBS)


if COMPAT_SYMLINKS
install-exec-hook:
	cd $(DESTDIR)$(sbindir) && $(LN_S) -f fatlabel dosfslabel
	cd $(DESTDIR)$(sbindir) && $(LN_S) -f fsck.fat dosfsck
	cd $(DESTDIR)$(sbindir) && $(LN_S) -f fsck.fat fsck.msdos
	cd $(DESTDIR)$(sbindir) && $(LN_S) -f fsck.fat fsck.vfat
	cd $(DESTDIR)$(sbindir) && $(LN_S) -f mkfs.fat mkdosfs
	cd $(DESTDIR)$(sbindir) && $(LN_S) -f mkfs.fat mkfs.msdos
	cd $(DESTDIR)$(sbindir) && $(LN_S) -f mkfs.fat mkfs.vfat

uninstall-hook:
	$(RM) $(DESTDIR)$(sbindir)/dosfslabel
	$(RM) $(DESTDIR)$(sbindir)/dosfsck
	$(RM) $(DESTDIR)$(sbindir)/fsck.msdos
	$(RM) $(DESTDIR)$(sbindir)/fsck.vfat
	$(RM) $(DESTDIR)$(sbindir)/mkdosfs
	$(RM) $(DESTDIR)$(sbindir)/mkfs.msdos
	$(RM) $(DESTDIR)$(sbindir)/mkfs.vfat
endif
