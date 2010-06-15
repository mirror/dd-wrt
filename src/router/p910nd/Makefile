# Comment out line bellow if you don't want to use 
# libwrap (hosts.{allow,deny} access control)
#LIBWRAP = -lwrap

# If you don't have it in /var/log/subsys, uncomment and define
#CFLAGS += -DLOCKFILE_DIR=\"/var/log\"

# GNU target string
#CROSS = 

#CC = $(CROSS)gcc
#STRIP = $(CROSS)strip

CFLAGS += -O2  -Wall

ifneq ("$(LIBWRAP)", "")
LIBWRAP += -DUSE_LIBWRAP
endif

PROG = p910nd
CONFIG = p910nd.conf
INITSCRIPT = p910nd.init
MANPAGE = p910nd.8
INSTALL = install
BINDIR = /usr/sbin
CONFIGDIR = /etc/sysconfig
SCRIPTDIR = /etc/init.d
MANDIR = /usr/share/man/man8

$(PROG):	p910nd.c
	$(CC) -o $@ $^ $(CFLAGS) $(LIBWRAP)

strip: $(PROG)
	$(STRIP) -s $(PROG)

install: $(PROG) $(CONFIG) $(INITSCRIPT) $(MANPAGE)
	mkdir -p $(DESTDIR)$(BINDIR) $(DESTDIR)$(CONFIGDIR) \
		$(DESTDIR)$(SCRIPTDIR) $(DESTDIR)$(MANDIR)
	$(INSTALL) $(PROG) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 644 $(CONFIG) $(DESTDIR)$(CONFIGDIR)/$(PROG)
	$(INSTALL) $(INITSCRIPT) $(DESTDIR)$(SCRIPTDIR)/$(PROG)
	$(INSTALL) -m 644 $(MANPAGE) $(DESTDIR)$(MANDIR)

.PHONY: clean
clean:
	rm -f *.o $(PROG)
