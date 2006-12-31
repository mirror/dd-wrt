TCPDUMPSRC=tcpdump-3.4.tar.Z
TCPDUMPDIR=tcpdump-3.4
LIBPCAPSRC=libpcap-0.4.tar.Z
LIBPCAPDIR=libpcap-0.4
TCPDUMPPATCHES=tcpdump-3.4.patch tcpdump-3.4.patch.sunrpc
ANSSRC=bind-4.9.5-REL.tar.gz
ANSDIR=bind-4.9.5
ANSPATCH=bind-4.9.5-REL.patch
DIR=ftp://lrcftp.epfl.ch/pub/linux/atm/extra

CLEAN = clean_extra
SPOTLESS = spotless_extra
include ../Rules.make

all:		tcpdump ans

install:
		[ ! -d $(TCPDUMPDIR) ] || $(MAKE) install-tcpdump
		[ ! -d $(ANSDIR) ] || $(MAKE) install-ans

tcpdump:
		[ -f $(TCPDUMPSRC) ] || \
		  lynx -dump $(DIR)/$(TCPDUMPSRC) >$(TCPDUMPSRC)
		[ -f $(LIBPCAPSRC) ] || \
		  lynx -dump $(DIR)/$(LIBPCAPSRC) >$(LIBPCAPSRC)
		[ -d $(TCPDUMPDIR) ] || tar xfz $(TCPDUMPSRC)
		[ -d $(LIBPCAPDIR) ] || tar xfz $(LIBPCAPSRC)
		[ -f $(TCPDUMPDIR)/.patched ] || { \
		  for n in $(TCPDUMPPATCHES); do \
		    patch -p1 -s <$$n || exit 1; done && \
		  touch $(TCPDUMPDIR)/.patched \
		}
		[ -f $(LIBPCAPDIR)/.compiled ] || { \
		  cd $(LIBPCAPDIR) && ./configure && make && touch .compiled \
		}
		[ -f $(TCPDUMPDIR)/.compiled ] || { \
		  cd $(TCPDUMPDIR) && ./configure && make && touch .compiled \
		}
		
install-tcpdump:
		[ -f $(LIBPCAPDIR)/.compiled ] || $(MAKE) libpcap
		[ -f $(TCPDUMPDIR)/.compiled ] || $(MAKE) tcpdump
		install -c -m 755 $(TCPDUMPDIR)/$(TCPDUMPDIR)/tcpdump \
		  $(INSTSYSBIN)/tcpdump_atm

ans:
		[ -f $(ANSSRC) ] || \
		  lynx -dump $(DIR)/$(ANSSRC) >$(ANSSRC)
		[ -d $(ANSDIR) ] || { mkdir $(ANSDIR) && cd $(ANSDIR) && \
		  tar xfz ../$(ANSSRC) }
		[ -f $(ANSDIR)/.patched ] || { \
		  cd $(ANSDIR) && \
		  patch -p1 -s --remove-empty-files <../$(ANSPATCH) && \
		  touch .patched \
		}
		[ -f $(ANSDIR)/.compiled ] || { \
		  cd $(ANSDIR) && make && touch .compiled \
		}

install-ans:
		[ -f $(ANSDIR)/.compiled ] || $(MAKE) ans
		cd $(ANSDIR) && make install	# brute-force

clean_extra:
		[ ! -f $(TCPDUMPDIR)/Makefile ] || \
		  { cd $(TCPDUMPDIR) && make clean; }
		[ ! -f $(LIBPCAPDIR)/Makefile ] || \
		  { cd $(LIBPCAPDIR) && make clean; }

spotless:
		rm -f $(TCPDUMPSRC) $(LIBPCAPSRC)
		rm -rf $(TCPDUMPDIR) $(LIBPCAPDIR)
