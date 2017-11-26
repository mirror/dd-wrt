
bin_PROGRAMS = mbimcli

mbimcli_CPPFLAGS = \
	$(MBIMCLI_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src/libmbim-glib \
	-I$(top_builddir)/src/libmbim-glib \
	-I$(top_srcdir)/src/libmbim-glib/generated \
	-I$(top_builddir)/src/libmbim-glib/generated

mbimcli_SOURCES = \
	mbimcli.h mbimcli.c \
	mbimcli-helpers.h mbimcli-helpers.c \
	mbimcli-basic-connect.c \
	mbimcli-phonebook.c \
	mbimcli-dss.c \
	mbimcli-ms-firmware-id.c \
	mbimcli-ms-host-shutdown.c \
	mbimcli-atds.c

mbimcli_LDADD = \
	$(MBIMCLI_LIBS) \
	$(top_builddir)/src/libmbim-glib/libmbim-glib.la


completiondir = $(datadir)/bash-completion/completions

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(completiondir)
	$(INSTALL_DATA) $(srcdir)/mbimcli-completion $(DESTDIR)$(completiondir)/mbimcli

uninstall-hook:
	rm -f $(DESTDIR)$(completiondir)/mbimcli

EXTRA_DIST = mbimcli-completion
