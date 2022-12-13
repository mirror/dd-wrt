# Define USE_WRAP if you want to compile with
# libwrap (hosts.{allow,deny} access control)
ifneq ($(USE_WRAP),)
LIBS += -lwrap
DEFINES += -DUSE_LIBWRAP
endif

# If you don't have it in /var/log/subsys, uncomment and define
#CFLAGS += -DLOCKFILE_DIR=\"/var/log\"

# GNU target string
#CROSS = 

#CC = $(CROSS)gcc
#STRIP = $(CROSS)strip

CFLAGS += $(COPTS)

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
	$(CC) -o $@ $^ $(CFLAGS) $(COPTS) $(MIPS16_OPT) $(LTO) $(DEFINES) $(LIBS)

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
