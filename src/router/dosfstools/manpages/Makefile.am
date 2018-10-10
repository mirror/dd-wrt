# dosfstools manpages/Makefile.am
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

man_MANS = fsck.fat.8 mkfs.fat.8 fatlabel.8


if COMPAT_SYMLINKS
install-data-hook:
	cd $(DESTDIR)$(mandir)/man8 && $(LN_S) -f fatlabel.8 dosfslabel.8
	cd $(DESTDIR)$(mandir)/man8 && $(LN_S) -f fsck.fat.8 dosfsck.8
	cd $(DESTDIR)$(mandir)/man8 && $(LN_S) -f fsck.fat.8 fsck.msdos.8
	cd $(DESTDIR)$(mandir)/man8 && $(LN_S) -f fsck.fat.8 fsck.vfat.8
	cd $(DESTDIR)$(mandir)/man8 && $(LN_S) -f mkfs.fat.8 mkdosfs.8
	cd $(DESTDIR)$(mandir)/man8 && $(LN_S) -f mkfs.fat.8 mkfs.msdos.8
	cd $(DESTDIR)$(mandir)/man8 && $(LN_S) -f mkfs.fat.8 mkfs.vfat.8

uninstall-hook:
	$(RM) $(DESTDIR)$(mandir)/man8/dosfslabel.8
	$(RM) $(DESTDIR)$(mandir)/man8/dosfsck.8
	$(RM) $(DESTDIR)$(mandir)/man8/fsck.msdos.8
	$(RM) $(DESTDIR)$(mandir)/man8/fsck.vfat.8
	$(RM) $(DESTDIR)$(mandir)/man8/mkdosfs.8
	$(RM) $(DESTDIR)$(mandir)/man8/mkfs.msdos.8
	$(RM) $(DESTDIR)$(mandir)/man8/mkfs.vfat.8
endif
