INCLUDES=-I$(top_srcdir)/include
SUBDIRS= include src
EXTRA_DIST= config.rpath mkinstalldirs ChangeLog INSTALL TODO README configure bootstrap depcomp
AUTOMAKE_OPTIONS=foreign

rpm: dist
	$(MAKE) -C $@

dist-hook:
	-chmod -R a+r $(distdir)
	@if ! test -z "$(AMTAR)"; then \
		$(AMTAR) --create --verbose --file=- $(distdir) | bzip2 -c -9 > $(distdir).tar.bz2 ; \
	else \
		$(TAR) --create --verbose --file=- $(distdir) | bzip2 -c -9 > $(distdir).tar.bz2 ; \
	fi
