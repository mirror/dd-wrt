
sbin_PROGRAMS = \
	dnscrypt-proxy

dnscrypt_proxy_SOURCES = \
	app.c \
	app.h \
	cert.c \
	cert.h \
	cert_p.h \
	dnscrypt.c \
	dnscrypt.h \
	dnscrypt_client.c \
	dnscrypt_client.h \
	dnscrypt_proxy.h \
	edns.c \
	edns.h \
	getpwnam.h \
	logger.c \
	logger.h \
	minicsv.c \
	minicsv.h \
	options.c \
	options.h \
	pathnames.h \
	pid_file.c \
	pid_file.h \
	probes_dnscrypt_proxy.d \
	probes_no_dtrace.h \
	safe_rw.c \
	safe_rw.h \
	sandboxes.c \
	sandboxes.h \
	simpleconf.c \
	simpleconf.h \
	simpleconf_dnscrypt.h \
	shims.h \
	stack_trace.c \
	stack_trace.h \
	tcp_request.c \
	tcp_request.h \
	tcp_request_p.h \
	udp_request.c \
	udp_request.h \
	udp_request_p.h \
	utils.c \
	utils.h \
	windows_service.c \
	windows_service.h

AM_CFLAGS = @CWFLAGS@

AM_CPPFLAGS = \
	-I../ext \
	-I../libevent-modified/include \
	-I${srcdir}/../ext \
	-I${srcdir}/../libevent-modified/include \
	-DPKGDATADIR='"${pkgdatadir}"'

dnscrypt_proxy_LDADD = \
	../libevent-modified/libevent_extra.la \
	../libevent-modified/libevent_core.la

dnscrypt_proxy_DEPENDENCIES = \
	../libevent-modified/libevent_extra.la \
	../libevent-modified/libevent_core.la

BUILT_SOURCES = \
	probes.h

probes.h: @PROBES_SOURCE@
	cat $< > $@

probes_dnscrypt_proxy.h: probes_dnscrypt_proxy.d
	@DTRACE@ -o $@ -h -s probes_dnscrypt_proxy.d

CLEANFILES = \
	probes.h \
	probes_dnscrypt_proxy.h

if HAVE_SYSTEMD

dnscrypt_proxy_CFLAGS = $(SYSTEMD_CFLAGS) $(SYSTEMD_DAEMON_CFLAGS)
dnscrypt_proxy_LDADD += $(SYSTEMD_LIBS) $(SYSTEMD_DAEMON_LIBS)

endif

if PLUGINS

AM_CPPFLAGS += \
	$(LTDLINCL) \
	-I../include \
	-I${srcdir}/../include \
	-DPLUGINS_ROOT=\"$(pkglibdir)/\"

dnscrypt_proxy_LDADD += $(LIBLTDL)
dnscrypt_proxy_DEPENDENCIES += $(LTDLDEPS)

dnscrypt_proxy_SOURCES += \
	plugin_options.c \
	plugin_options.h \
	plugin_support.c \
	plugin_support.h \
	plugin_support_p.h

endif
