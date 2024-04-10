ACLOCAL_AMFLAGS = -I m4

SUBDIRS = addshare adduser control mountd tools

EXTRA_DIST = include \
             README.md \
             ksmbd-tools.spec \
             ksmbd.conf.5.in \
             ksmbdpwd.db.5.in \
             ksmbd.service.in \
	     meson.build \
	     meson_options.txt

pkgsysconfdir = $(sysconfdir)/ksmbd
dist_pkgsysconf_DATA = ksmbd.conf.example

man_MANS = ksmbd.conf.5 ksmbdpwd.db.5
systemdsystemunit_DATA = ksmbd.service
$(man_MANS) $(systemdsystemunit_DATA): %: %.in; @$(in_script) $< >$@

CLEANFILES = $(man_MANS) $(systemdsystemunit_DATA)

AM_DISTCHECK_CONFIGURE_FLAGS = \
	--with-systemdsystemunitdir=$${dc_install_base}/$(systemdsystemunitdir)
