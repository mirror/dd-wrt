BUILT_SOURCES = $(top_srcdir)/.version

$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@

dist-hook:
	echo $(VERSION) > $(distdir)/.tarball-version

CLEANFILES = $(top_srcdir)/.version

SUBDIRS = src

EXTRA_DIST = git-version-gen
