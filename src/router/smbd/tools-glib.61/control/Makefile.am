AM_CFLAGS = -DSYSCONFDIR='"${sysconfdir}"' -DRUNSTATEDIR='"${runstatedir}"' \
            -I$(top_srcdir)/include $(GLIB_CFLAGS) $(LIBNL_CFLAGS) -fno-common

noinst_LIBRARIES = libcontrol.a
libcontrol_a_SOURCES = control.c

EXTRA_DIST = ksmbd.control.8.in \
	     meson.build

man_MANS = ksmbd.control.8
$(man_MANS): %: %.in; @$(in_script) $< >$@

CLEANFILES = $(man_MANS)

install-exec-hook: uninstall-hook
	$(MKDIR_P) $(DESTDIR)$(sbindir)
	( cd $(DESTDIR)$(sbindir) && \
	  $(LN_S) $(libexecdir)/ksmbd.tools ksmbd.control )

uninstall-hook:
	-rm $(DESTDIR)$(sbindir)/ksmbd.control
