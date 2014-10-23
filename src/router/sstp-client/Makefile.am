ACLOCAL_AMFLAGS = -I m4
SUBDIRS = \
          src/libsstp-log   \
          src/libsstp-api   \
          src/libsstp-compat\
          src/pppd-plugin   \
          include           \
          src

docdir         = $(datadir)/doc/@PACKAGE@
pkgconfigdir   = $(libdir)/pkgconfig
pkgconfig_DATA = sstp-client-1.0.pc

doc_DATA       = \
	sstp-test.example \
	sstp-test-nopty.example

man8_MANS = sstpc.8
EXTRA_DIST = 		\
	$(man8_MANS) 	\
	$(doc_DATA)	\
	support 	\
	debian		\
	sstp-client.spec\
	DEVELOPERS 	\
	USING

dist-hook:
	for i in $(EXTRA_DIST) ; do \
        if [ -d $i ] ; then \
            rm -rf `find $(distdir)/$i -type d -name .svn` ; \
        fi ; \
	done
