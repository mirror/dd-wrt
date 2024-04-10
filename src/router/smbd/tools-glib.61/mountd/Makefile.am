AM_CFLAGS = -DSYSCONFDIR='"${sysconfdir}"' -DRUNSTATEDIR='"${runstatedir}"' \
            -I$(top_srcdir)/include $(GLIB_CFLAGS) $(LIBNL_CFLAGS) -fno-common

noinst_LIBRARIES = libmountd.a
libmountd_a_SOURCES = worker.c ipc.c rpc.c rpc_srvsvc.c rpc_wkssvc.c mountd.c \
                      smbacl.c rpc_samr.c rpc_lsarpc.c

EXTRA_DIST = ksmbd.mountd.8.in \
	     meson.build

man_MANS = ksmbd.mountd.8
$(man_MANS): %: %.in; @$(in_script) $< >$@

CLEANFILES = $(man_MANS)

install-exec-hook: uninstall-hook
	$(MKDIR_P) $(DESTDIR)$(sbindir)
	( cd $(DESTDIR)$(sbindir) && \
	  $(LN_S) $(libexecdir)/ksmbd.tools ksmbd.mountd )

uninstall-hook:
	-rm $(DESTDIR)$(sbindir)/ksmbd.mountd
