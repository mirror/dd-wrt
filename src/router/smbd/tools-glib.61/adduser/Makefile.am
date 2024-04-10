AM_CFLAGS = -DSYSCONFDIR='"${sysconfdir}"' -DRUNSTATEDIR='"${runstatedir}"' \
            -I$(top_srcdir)/include $(GLIB_CFLAGS) $(LIBNL_CFLAGS) -fno-common

noinst_LIBRARIES = libadduser.a
libadduser_a_SOURCES = md4_hash.c user_admin.c adduser.c md4_hash.h user_admin.h

EXTRA_DIST = ksmbd.adduser.8.in \
	     meson.build

man_MANS = ksmbd.adduser.8
$(man_MANS): %: %.in; @$(in_script) $< >$@

CLEANFILES = $(man_MANS)

install-exec-hook: uninstall-hook
	$(MKDIR_P) $(DESTDIR)$(sbindir)
	( cd $(DESTDIR)$(sbindir) && \
	  $(LN_S) $(libexecdir)/ksmbd.tools ksmbd.adduser )

uninstall-hook:
	-rm $(DESTDIR)$(sbindir)/ksmbd.adduser
