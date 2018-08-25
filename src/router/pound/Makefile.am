AM_CFLAGS=\
 -DF_CONF=\"@sysconfdir@/pound.cfg\"\
 @PTHREAD_CFLAGS@

AM_CPPFLAGS = @SSL_CPPFLAGS@
AM_LDFLAGS  = @SSL_LDFLAGS@
LDADD=@PTHREAD_LIBS@

sbin_PROGRAMS=pound
pound_SOURCES=\
 config.c\
 http.c\
 pound.c\
 svc.c

install-exec-hook:
	-@if test "$${EUID:-65535}" -eq 0; then \
		for program in $(sbin_PROGRAMS); do \
			chown @I_OWNER@:@I_GRP@ $(DESTDIR)$(sbindir)/$$program$(EXEEXT); \
		done; \
		for program in $(bin_PROGRAMS); do \
			chown @I_OWNER@:@I_GRP@ $(DESTDIR)$(bindir)/$$program$(EXEEXT); \
		done; \
	fi

noinst_HEADERS=pound.h config.h dh.h

DHSRC = dh512.h dh@DH_LEN@.h

dh512.h:
	openssl dhparam -5 -C -noout 512 > dh512.h

dh@DH_LEN@.h:
	openssl dhparam -5 -C -noout @DH_LEN@ > dh@DH_LEN@.h

dh.h: $(DHSRC)
	cat $(DHSRC) > dh.h

bin_PROGRAMS=poundctl
poundctl_SOURCES=poundctl.c

BUILT_SOURCES=dh.h $(DHSRC)
DISTCLEANFILES=dh.h dh512.h dh1024.h dh2048.h 

dist_man_MANS=pound.8 poundctl.8
