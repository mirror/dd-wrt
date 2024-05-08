
prefix= /tmp
exec_prefix= ${prefix}
bindir= ${exec_prefix}/bin
sysconfdir= ${prefix}/etc
datadir= ${prefix}/share

#CC=mipsel-linux-uclibc-gcc -pipe
#CC= gcc -pipe
LDFLAGS+=-s $(LDLTO) -ffunction-sections -fdata-sections -Wl,--gc-sections
LIBS+= -lpthread -L$(TOP)/libpcap -lpcap
CFLAGS+= $(COPTS) $(MIPS16_OPT) $(LTO) -I$(LINUXDIR)  -DHAVE_CONFIG_H -W -Wall -ffunction-sections -fdata-sections -Wl,--gc-sections
CPPFLAGS+=-DIPCAD_VERSION=\"3.6.2\"
CPPFLAGS+=-DCONFIG_FILE=\"${sysconfdir}/ipcad.conf\"
CPPFLAGS+=-DHAVE_CONFIG_H -D_REENTRANT -D_THREAD_SAFE -I$(TOP)/libpcap
CPPFLAGS+=-DIFST_linux -DPSRC_pcap 
#-DIPCAD_IFLIST_USE_GETIFADDRS

YACC=bison -y
LEX=flex
INSTALL=/usr/bin/install -c

MANROOT=${prefix}/man/man
MAN8=${MANROOT}8
MAN5=${MANROOT}5

NROFF= groff -Tascii -mandoc

PACKAGE=rflow
TARGETS=rflow
VERSION=3.6.2

IPCAD_OBJS= main.o nflow.o servers.o opt.o pidfile.o storage.o pps.o
IPCAD_OBJS+= cfgvar.o disp.o usage.o ifst_linux.o process.o sf_lite.o
#IPCAD_OBJS+= ifst_linux.o		# Interface statistics
IPCAD_OBJS+= ifs_list.o			# Interface statistics
#IPCAD_OBJS+= loop-ring.o		# Method of capturing
IPCAD_OBJS+= loop-pcap.o		# Method of capturing
IPCAD_OBJS+= psrc.o psrc-pcap.o	# Initialize capturers
#IPCAD_OBJS+= ring.o	# Initialize capturers
#IPCAD_OBJS+= psrc-dynamic.o		# Initialize capturers
#IPCAD_OBJS+= wrap_oclose.o		# Special file descriptors cache
IPCAD_OBJS+= genhash.o			# General hashing
#IPCAD_OBJS+= sigintr.o

all: ${TARGETS}
	install -D config/rflow.webservices $(TOP)/httpd/ej_temp/rflow.webservices

rflow: ${IPCAD_OBJS}
	${CC} ${CFLAGS} -o $@ ${IPCAD_OBJS} ${LDFLAGS} ${LIBS}

.SUFFIXES:
.SUFFIXES: .o .c .0 .8 .5

.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} -o $@ -c $<

.8.0:
	${NROFF} $< > $@

.5.0:
	${NROFF} $< > $@

cfgy.h cfgy.c: cfg.y
	${YACC} -p ipcacfg -d cfg.y
	@mv y.tab.c cfgy.c
	@mv y.tab.h cfgy.h

cshelly.h cshelly.c: cshell.y
	${YACC} -p CS -d cshell.y
	@mv y.tab.c cshelly.c
	@mv y.tab.h cshelly.h

cslex.c: cslex.l
	${LEX} -sp -Cem -ocslex.c -PCS cslex.l

cfglex.c: cfglex.l
	${LEX} -sp -Cem -ocfglex.c -Pipcacfg cfglex.l


man: ipcad.8 ipcad.conf.5

distdir = $(PACKAGE)-$(VERSION)
distdir:
	rm -rf $(distdir)
	mkdir $(distdir)
	cp *.c *.h $(distdir)
	cp ipcad.8 ipcad.conf.5 $(distdir)
	cp *.y *.l $(distdir)
	cp *.in configure install-sh $(distdir)
	cp ipcad.conf.default $(distdir)
	cp ipcad.conf.simple $(distdir)
	cp Makefile.in Makefile $(distdir)
	cp ChangeLog INSTALL README TODO BUGS AUTHORS COPYING FAQ $(distdir)
	cd $(distdir) && make distclean

dist: distdir
	tar chof - $(distdir) | GZIP="--best" gzip -c > $(distdir).tar.gz
	rm -rf $(distdir)

clean:
	rm -f *.o ${TARGETS} *.core *.a 1
	rm -f *.0

distclean: clean
	rm -f config.h
	rm -f config.cache config.log config.status
	rm -f Makefile
	rm -f $(distdir).tar.gz
	(echo "all: bootstrap"; echo; echo "bootstrap:"; \
		echo "	./configure && make") > Makefile

maintainer-clean: distclean
	@echo "This command is intended for maintainers to use; it"
	@echo "deletes files that may need special tools to rebuild."
	rm -f Makefile
	rm -f *.tab.* cfgy.? *lex.c
	rm -f config.h.in configure

install: all
	install -D rflow $(INSTALLDIR)/usr/sbin/rflow
	install -D config/rflow.webservices $(INSTALLDIR)/etc/config/rflow.webservices
	install -D config/rflow.startup $(INSTALLDIR)/etc/config/rflow.startup
	install -D config/rflow.nvramconfig $(INSTALLDIR)/etc/config/rflow.nvramconfig
#	$(STRIP) $(INSTALLDIR)/usr/sbin/rflow

install_old: all install-bin install-conf

install-bin:
	@${INSTALL} -d ${bindir}
	${INSTALL} ipcad ${bindir}

install-man: man
	${INSTALL} -m 444 ipcad.8 ${MAN8}/ipcad.8
	${INSTALL} -m 444 ipcad.conf.5 ${MAN5}/ipcad.conf.5

install-conf:
	@${INSTALL} -d ${sysconfdir}
	${INSTALL} -m 644 ipcad.conf.simple ${sysconfdir}/ipcad.conf.simple
	-@diff ${sysconfdir}/ipcad.conf					\
	      ${sysconfdir}/ipcad.conf.default >/dev/null 2>&1;		\
	if [ "$$?" -eq 0 ] || [ ! -f ${sysconfdir}/ipcad.conf ]; then	\
	echo ${INSTALL} -m 644 ipcad.conf.default ${sysconfdir}/ipcad.conf.default; \
	${INSTALL} -m 644 ipcad.conf.default ${sysconfdir}/ipcad.conf.default; \
	echo ${INSTALL} -m 644 ipcad.conf.default ${sysconfdir}/ipcad.conf;	\
	${INSTALL} -m 644 ipcad.conf.default ${sysconfdir}/ipcad.conf;	\
	echo "**********************************************";		\
	echo "* Please customize ${sysconfdir}/ipcad.conf *";		\
	echo "**********************************************";		\
	else								\
	echo ${INSTALL} -m 644 ipcad.conf.default ${sysconfdir}/ipcad.conf.default; \
	${INSTALL} -m 644 ipcad.conf.default ${sysconfdir}/ipcad.conf.default; \
	echo "*********************************************";		\
	echo "* Updated ${sysconfdir}/ipcad.conf.default *";		\
	echo "*********************************************";		\
	fi
	@echo ""
	@echo "Now you might want to 'make install-man'"
	@echo ""


uninstall:
	rm -f ${bindir}/ipcad
	rm -f ${sysconfdir}/ipcad.conf.default
	rm -f ${sysconfdir}/ipcad.conf.simple
	rm -f ${MAN8}/ipcad.8
	rm -f ${MAN5}/ipcad.conf.5

