AM_CFLAGS = -DSYSCONFDIR='"${sysconfdir}"' -DRUNSTATEDIR='"${runstatedir}"' \
            -I$(top_srcdir)/include $(GLIB_CFLAGS) $(LIBNL_CFLAGS) -fno-common

noinst_LIBRARIES = libaddshare.a
libaddshare_a_SOURCES = share_admin.c addshare.c share_admin.h

EXTRA_DIST = ksmbd.addshare.8.in \
	     meson.build

man_MANS = ksmbd.addshare.8
$(man_MANS): %: %.in; @$(in_script) $< >$@

CLEANFILES = $(man_MANS)

install-exec-hook: uninstall-hook
	$(MKDIR_P) $(DESTDIR)$(sbindir)
	( cd $(DESTDIR)$(sbindir) && \
	  $(LN_S) $(libexecdir)/ksmbd.tools ksmbd.addshare )

uninstall-hook:
	-rm $(DESTDIR)$(sbindir)/ksmbd.addshare
