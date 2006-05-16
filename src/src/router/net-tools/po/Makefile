INSTALL=install -c
INSTALL_PROGRAM= ${INSTALL}
INSTALL_DATA= ${INSTALL} -m 644
INSTALLNLSDIR=${BASEDIR}/usr/share/locale

TUPDATE = tupdate

NLSPACKAGE = net-tools

-include ../config.make
ifeq ($(I18N),1)
CATALOGS = de.mo fr.mo pt_BR.mo et_EE.mo cs.mo
else
CATALOGS =
endif

POTFILES=../arp.c ../hostname.c ../ifconfig.c ../netstat.c ../rarp.c \
	../route.c ../plipconfig.c ../iptunnel.c ../statistics.c \
	../lib/activate.c ../lib/af.c ../lib/arcnet.c ../lib/ash.c \
	../lib/ax25.c ../lib/ax25_gr.c ../lib/ddp.c ../lib/ddp_gr.c \
	../lib/econet.c ../lib/ether.c ../lib/fddi.c ../lib/frame.c \
	../lib/getargs.c ../lib/getroute.c ../lib/hdlclapb.c ../lib/hippi.c \
	../lib/hw.c ../lib/inet.c ../lib/inet6.c ../lib/inet6_gr.c \
	../lib/inet6_sr.c ../lib/inet_gr.c ../lib/inet_sr.c ../lib/ipx.c \
	../lib/ipx_gr.c ../lib/ipx_sr.c ../lib/loopback.c ../lib/masq_info.c \
	../lib/nstrcmp.c ../lib/netrom.c ../lib/netrom_gr.c ../lib/netrom_sr.c \
	../lib/ppp.c ../lib/ppp_ac.c ../lib/proc.c ../lib/rose.c \
	../lib/rose_gr.c ../lib/setroute.c ../lib/sit.c ../lib/slip.c \
	../lib/slip_ac.c ../lib/tr.c ../lib/tunnel.c ../lib/unix.c \
	../lib/util.c ../lib/interface.c ../lib/sockets.c ../lib/util-ank.c \
	../ipmaddr.c ../slattach.c ../iptunnel.c

all: $(NLSPACKAGE).pot $(CATALOGS)

$(NLSPACKAGE).pot: $(POTFILES)
	xgettext --default-domain=$(NLSPACKAGE) \
	  --add-comments --keyword=_ --keyword=N_ $(POTFILES)
	if cmp -s $(NLSPACKAGE).po $(NLSPACKAGE).pot; then \
	    rm -f $(NLSPACKAGE).po; \
	else \
	    mv $(NLSPACKAGE).po $(NLSPACKAGE).pot; \
	fi


update-po: Makefile
	$(MAKE) $(NLSPACKAGE).pot
	catalogs='$(CATALOGS)'; \
	for cat in $$catalogs; do \
		lang=`echo $$cat | sed 's/.mo//'`; \
		mv $$lang.po $$lang.old.po; \
		echo "$$lang:"; \
		if $(TUPDATE) $(NLSPACKAGE).pot $$lang.old.po > $$lang.po; then \
		rm -f $$lang.old.po; \
	else \
	 	echo "tupdate for $$cat failed!"; \
		rm -f $$lang.po; \
		mv $$lang.old.po $$lang.po; \
	fi; \
	done

clean:
	rm -f *mo

distclean: clean
	rm -f .depend Makefile

depend:

install: $(CATALOGS)
	catalogs='$(CATALOGS)'; \
	for n in $$catalogs; do \
	    l=`basename $$n .mo`; \
	    $(INSTALL) -m 755 -d $(INSTALLNLSDIR)/$$l; \
	    $(INSTALL) -m 755 -d $(INSTALLNLSDIR)/$$l/LC_MESSAGES; \
	    $(INSTALL) -m 644 $$n $(INSTALLNLSDIR)/$$l/LC_MESSAGES/net-tools.mo; \
	done

%.mo: %.po
	msgfmt -o $@ $<
