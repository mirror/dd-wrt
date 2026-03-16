noinst_HEADERS = common.h config.h errorsymbols.h fmt.h interposition.h	\
   osdep.h qos.h redefac.h sockd.h socks.h socks_gssapi.h socks_krb5.h \
   symbols.h tostring.h upnp.h yacconfig.h monitor.h

EXTRA_DIST = redefgen.sh symbols_common.txt symbols_darwin.txt  \
   symbols_freebsd.txt symbols_glibc.txt symbols_osol.txt       \
   symbols_osol_stdio.txt symbols_preload.txt

redefac.h: autoconf.h.in
	sh $(top_srcdir)/include/redefgen.sh $(top_srcdir)/include
