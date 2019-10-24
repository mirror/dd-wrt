libsispmctl_la_LDFLAGS = \
	-version-info 2:0:2

bin_PROGRAMS = sispmctl 

lib_LTLIBRARIES = libsispmctl.la

sispmctl_LDADD = libsispmctl.la

EXTRA_DIST =  \
	web1/index.html web1/off1.html web1/off3.html web1/on1.html \
	web1/on3.html web1/status0.png web1/style.css web1/logo.png \
	web1/off2.html web1/off4.html web1/on2.html web1/on4.html \
	web1/status1.png web2/index.html web2/off1.html web2/off3.html \
	web2/on1.html web2/on3.html web2/style.css web2/logo.png \
	web2/off2.html web2/off4.html web2/on2.html web2/on4.html \
	web2/off.gif web2/on.gif web2/onoff.gif 

if WEBLESSCOND
AM_CFLAGS=-Wall -DWEBLESS=@WEBLESS@
else
AM_CFLAGS=-Wall -DBINDADDR="\"@BINDADDR@\"" -DDATADIR="\"@WEBDIR@\""

pkgdata1dir = "$(docdir)/httpd/skin1"
pkgdata2dir = "$(docdir)/httpd/skin2"

pkgdata1_DATA = \
	web1/index.html web1/logo.png web1/off1.html web1/off2.html \
	web1/off3.html web1/off4.html web1/on1.html web1/on2.html \
	web1/on3.html web1/on4.html web1/status0.png web1/status1.png \
	web1/style.css
pkgdata2_DATA = \
	web2/index.html web2/logo.png web2/off1.html web2/off2.html \
	web2/off3.html web2/off4.html web2/off.gif web2/on1.html \
	web2/on2.html web2/on3.html web2/on4.html web2/on.gif web2/onoff.gif \
	web2/style.css 
endif

libsispmctl_la_SOURCES = \
	process.c sispm_ctl.c nethelp.c socket.c sispm_ctl.h nethelp.h socket.h

sispmctl_SOURCES = main.c

AM_CPPFLAGS = $(all_includes)

AM_LDFLAGS = $(all_libraries)

install-data-local:
if WEBLESSCOND
else
	mkdir -p "$(DESTDIR)$(docdir)/httpd/skin1"
	cd "$(DESTDIR)$(docdir)" && ln -sf httpd/skin1 skin
endif

uninstall-local:
	rm -rf $(DESTDIR)$(datadir)/$(PACKAGE)/httpd

