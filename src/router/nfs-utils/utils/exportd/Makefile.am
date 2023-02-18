## Process this file with automake to produce Makefile.in

OPTLIBS		=
if CONFIG_JUNCTION
OPTLIBS		+= ../../support/junction/libjunction.la $(LIBXML2)
endif

man8_MANS	= exportd.man
EXTRA_DIST	= $(man8_MANS)

NFSPREFIX	= nfsv4.
KPREFIX		= @kprefix@
sbin_PROGRAMS	= exportd

exportd_SOURCES = exportd.c
exportd_LDADD = ../../support/export/libexport.a \
			../../support/nfs/libnfs.la \
			../../support/misc/libmisc.a \
			$(OPTLIBS) $(LIBBLKID) $(LIBPTHREAD) -luuid

exportd_CPPFLAGS = $(AM_CPPFLAGS) $(CPPFLAGS) \
		-I$(top_srcdir)/support/export

MAINTAINERCLEANFILES = Makefile.in

#######################################################################
# The following allows the current practice of having
# daemons renamed during the install to include NFSPREFIX
# and the KPREFIX
# This could all be done much easier with program_transform_name
# ( program_transform_name = s/^/$(NFSPREFIX)$(KPREFIX)/ )
# but that also renames the man pages, which the current
# practice does not do.
install-exec-hook:
	(cd $(DESTDIR)$(sbindir) && \
	  for p in $(sbin_PROGRAMS); do \
	    mv -f $$p$(EXEEXT) $(NFSPREFIX)$(KPREFIX)$$p$(EXEEXT) ;\
	  done)
uninstall-hook:
	(cd $(DESTDIR)$(sbindir) && \
	  for p in $(sbin_PROGRAMS); do \
	    rm -f $(NFSPREFIX)$(KPREFIX)$$p$(EXEEXT) ;\
	  done)


# XXX This makes some assumptions about what automake does.
# XXX But there is no install-man-hook or install-man-local.
install-man: install-man8 install-man-links
uninstall-man: uninstall-man8 uninstall-man-links

install-man-links:
	(cd $(DESTDIR)$(man8dir) && \
	  for m in $(man8_MANS) $(dist_man8_MANS) $(nodist_man8_MANS); do \
	    inst=`echo $$m | sed -e 's/man$$/8/'`; \
	    rm -f $(NFSPREFIX)$$inst ; \
	    $(LN_S) $$inst $(NFSPREFIX)$$inst ; \
	  done)

uninstall-man-links:
	(cd $(DESTDIR)$(man8dir) && \
	  for m in $(man8_MANS) $(dist_man8_MANS) $(nodist_man8_MANS); do \
	    inst=`echo $$m | sed -e 's/man$$/8/'`; \
	    rm -f $(NFSPREFIX)$$inst ; \
	  done)

