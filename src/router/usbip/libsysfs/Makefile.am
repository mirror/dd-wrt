man_MANS = systool.1
EXTRA_DIST = docs include $(man_MANS) CREDITS lib/LGPL cmd/GPL test/GPL
SUBDIRS = lib cmd test
includedir=@includedir@/sysfs
include_HEADERS = include/libsysfs.h include/dlist.h

dist-hook:
	rm -rf `find $(distdir)/docs -name CVS`
	rm -rf `find $(distdir)/include -name CVS`

