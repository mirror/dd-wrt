#
# milli_httpd Makefile
#
# Copyright 2001-2003, Broadcom Corporation
# All Rights Reserved.
#
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.6 2005/11/30 11:55:05 seg Exp $
#

include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

CFLAGS	= $(COPTS)
ifeq ($(CONFIG_MSSID),y)
CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/ -Imodules
else
CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v23 -Wall -I$(SRCBASE)/ -Imodules
endif

ifeq ($(CONFIG_VFS),y)
CFLAGS += -I$(TOP)/vfs -DHAVE_VFS
LDFLAGS += -L$(TOP)/vfs
endif

#CFLAGS += -Ilzma
LDFLAGS	+= -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/shared  -L$(INSTALLDIR)/shared/usr/lib -lshared -lcrypt -ldl

REWRITE_REVISION_H1 := $(shell echo -n '\#define SVN_REVISION "' > revision.h)
REWRITE_REVISION_H2 := $(shell svnversion -n . >> revision.h)
REWRITE_REVISION_H3 := $(shell echo '"' >> revision.h)

ifeq ($(CONFIG_SPECIALEDITION),y)
CFLAGS += -DHAVE_SPECIALEDITION
ADDOBJ = special.o
endif

ifeq ($(CONFIG_ANTIFLASH),y)
CFLAGS += -DANTI_FLASH
endif
CFLAGS += -DDIST=\"$(CONFIG_DIST)\"

ifeq ($(CONFIG_NOAD),y)
CFLAGS += -DHAVE_NOAD
endif
ifeq ($(CONFIG_RADLOCAL),y)
CFLAGS += -DHAVE_RADLOCAL
endif
ifeq ($(CONFIG_MULTICAST),y)
CFLAGS += -DHAVE_MULTICAST
endif
ifeq ($(CONFIG_RB500),y)
CFLAGS += -DHAVE_RB500
OBJS += upgrade_rb500.c
else
ifneq ($(CONFIG_X86),y)
OBJS += upgrade.c
endif
endif

ifeq ($(CONFIG_XSCALE),y)
CFLAGS += -DHAVE_XSCALE
endif
ifeq ($(CONFIG_RSTATS),y)
CFLAGS += -DHAVE_RSTATS
endif
ifeq ($(CONFIG_PORTSETUP),y)
CFLAGS += -DHAVE_PORTSETUP
endif
ifeq ($(CONFIG_NOWIFI),y)
CFLAGS += -DHAVE_NOWIFI
endif

ifeq ($(CONFIG_X86),y)
CFLAGS += -DHAVE_X86
OBJS += upgrade_x86.c
endif

ifeq ($(CONFIG_MAGICBOX),y)
CFLAGS += -DHAVE_MAGICBOX
CFLAGS += -DHAVE_CPUTEMP
endif
ifeq ($(CONFIG_GATEWORX),y)
CFLAGS += -DHAVE_GATEWORX
CFLAGS += -DHAVE_CPUTEMP
CFLAGS += -DHAVE_VOLT
endif

ifeq ($(REGDOMAIN_OVERRIDE),y)
CFLAGS += -DREGDOMAIN_OVERRIDE
endif

ifeq ($(CONFIG_34TELECOM),y)
CFLAGS += -DHAVE_34TELECOM
endif
ifeq ($(CONFIG_DDLAN),y)
CFLAGS += -DHAVE_DDLAN
endif

ifeq ($(CONFIG_GGEW),y)
CFLAGS += -DHAVE_GGEW
CFLAGS += -DHAVE_NEWMEDIA
endif

ifeq ($(CONFIG_NEWMEDIA),y)
CFLAGS += -DHAVE_NEWMEDIA
endif

ifeq ($(CONFIG_PPPOE),y)
CFLAGS += -DHAVE_PPPOE
endif

ifeq ($(CONFIG_UPNP),y)
CFLAGS += -DHAVE_UPNP
endif

ifeq ($(CONFIG_BIRD),y)
CFLAGS += -DHAVE_BIRD
endif
ifeq ($(CONFIG_PPTP),y)
CFLAGS += -DHAVE_PPTP
endif
ifeq ($(CONFIG_L2TP),y)
CFLAGS += -DHAVE_L2TP
endif
ifeq ($(CONFIG_HEARTBEAT),y)
CFLAGS += -DHAVE_HEARTBEAT
endif

ifeq ($(CONFIG_CHILLILOCAL),y)
CFLAGS += -DHAVE_CHILLILOCAL
endif

ifeq ($(CONFIG_CHILLISPOT),y)
CFLAGS += -DHAVE_CHILLI
endif

ifeq ($(CONFIG_SPUTNIK_APD),y)
CFLAGS += -DHAVE_SPUTNIK_APD
OBJS += sputnik.c
endif
ifeq ($(CONFIG_REGISTER),y)
CFLAGS += -DHAVE_REGISTER
OBJS += crc.c register.c
endif
#ifeq ($(HSIAB_SUPPORT),1)
#CFLAGS += -I$(TOP)/hsiab
#LDFLAGS += -L$(TOP)/openssl -L$(TOP)/hsiab -lcrypto -lssl -lhsiab
#OBJS += hsiab.o
#endif
#ifeq ($(HTTPS_SUPPORT),1)
#CFLAGS += -I$(TOP)/openssl/include  -I$(TOP)/openssl/include/openssl
#LDFLAGS += -L$(TOP)/openssl -L../openssl -lcrypto -lssl
#endif
#ifeq ($(BACKUP_RESTORE_SUPPORT),1)
#OBJS  += $(SRCBASE)/../tools/src/code_header.o backup_restore.o
#endif

ifeq ($(CONFIG_MASKMAC),y)
CFLAGS += -DMASKMAC
CFLAGS += -DFBNFW
endif


ifeq ($(DDM_SUPPORT),1)
OBJS  += ddm.c
endif

ifeq ($(CONFIG_ZEROIP),y)
CFLAGS += -DHAVE_ZEROIP
endif
ifeq ($(CONFIG_FON),y)
CFLAGS += -DHAVE_FON
endif

ifeq ($(CONFIG_OPENVPN),y)
CFLAGS += -DHAVE_OPENVPN
endif

ifeq ($(CONFIG_MSSID),y)
  CFLAGS += -DHAVE_MSSID
endif

ifeq ($(CONFIG_MADWIFI),y)
  CFLAGS += -DHAVE_MADWIFI -I../madwifi -include ../madwifi/include/compat.h
  OBJS += ../wireless-tools/libiw.so.29
endif

ifeq ($(CONFIG_CDEBUG),y)
CFLAGS += -DCDEBUG
endif

ifeq ($(CONFIG_AQOS),y)
CFLAGS += -DHAVE_AQOS
endif

ifeq ($(CONFIG_WTS),y)
CFLAGS += -DHAVE_WTS
endif

ifeq ($(CONFIG_OMNI),y)
CFLAGS += -DHAVE_OMNI
endif


ifeq ($(CONFIG_SKYTEL),y)
CFLAGS += -DHAVE_SKYTEL
endif

ifeq ($(CONFIG_SKYTRON),y)
CFLAGS += -DHAVE_SKYTRON
endif


ifeq ($(CONFIG_DLS),y)
CFLAGS += -DHAVE_DLS
endif

ifeq ($(CONFIG_NOTRANS),y)
CFLAGS += -DHAVE_NOTRANS
endif

ifeq ($(CONFIG_MACBIND),y)
CFLAGS += -DHAVE_MACBIND
endif

ifeq ($(CONFIG_POWERNOC),y)
CFLAGS += -DHAVE_POWERNOC
endif

ifeq ($(CONFIG_FREEBIRD),y)
CFLAGS += -DCONFIG_FREEBIRD
endif

ifeq ($(CONFIG_BRANDING),y)
CFLAGS += -DCONFIG_BRANDING
endif

ifeq ($(ARP_TABLE_SUPPORT),1)
OBJS  += arp.c
endif
ifeq ($(EMI_TEST),1)
OBJS  += emi_test.c
endif
ifeq ($(DEBUG_WEB),1)
CFLAGS += -DMY_DEBUG
endif
ifeq ($(WAKE_ON_LAN_SUPPORT),1)
OBJS  += wol.c
endif
ifeq ($(SAMBA_SUPPORT),1)
OBJS  += samba.c
endif
#ifeq ($(HW_QOS_SUPPORT),1)
#OBJS  += qos.o
#endif
ifeq ($(PERFORMANCE_SUPPORT),1)
OBJS  += performance.c
endif
#ifeq ($(WL_STA_SUPPORT),1)
OBJS  += site_survey.c
#endif

#OBJS += http.o

ifneq ($(CONFIG_NOSSL),y)

ifneq ($(CONFIG_MATRIXSSL),y)
ifeq ($(CONFIG_OPENSSL),y)
CFLAGS += -I$(TOP)/openssl/include  -I$(TOP)/openssl/include/openssl
LDFLAGS += -L$(TOP)/openssl -L../openssl -lcrypto -lssl
CFLAGS += -DHAVE_OPENSSL
CFLAGS += -DHAVE_HTTPS
endif
endif

ifeq ($(CONFIG_MATRIXSSL),y)
CFLAGS += -I$(TOP)/matrixssl
LDFLAGS += -L$(TOP)/matrixssl/src -lmatrixsslstatic -lpthread
CFLAGS += -DHAVE_MATRIXSSL
CFLAGS += -DHAVE_HTTPS
OBJS += matrixssl_xface.c
endif

endif

#OBJS  += common.o server.o
#OBJS  += code_header.o


vpath %.c modules modules/special.$(ARCH) $(SRCBASE)/../tools/src

all: httpd cert

clean:
	rm -f *.o *~ httpd
	rm -f modules/*.o
	rm -f modules/*.*~
	rm -f *.a 
	#Added by Daniel(2004-07-29)
	#cp ../shared/ezc.o ./
	#rm -rf $(SRCBASE)/../tools/src/code_header.o	

install:
	install -D httpd $(INSTALLDIR)/usr/sbin/httpd
ifneq ($(CONFIG_NOSSL),y)
	install -d $(INSTALLDIR)/etc
	install *.pem $(INSTALLDIR)/etc
endif

cert:
	./gencert.sh

httpd: cgi.c ej.c httpd.c broadcom.c \
       index.c status.c dhcp.c log.c filters.c forward.c dynamic_route.c static_route.c wireless.c ddns.c \
       find_pattern.c lib.c $(ADDOBJ) diag.c sysinfo.c wepkey.c md5c.c getservice.c dd-wrt.c nvramsr.c $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)
#	ar arc -o $@ $^
	rm -f ../rc/rc
	
build_date.o: build_date.c
build_date:
	echo "#define BUILD_DATE \"`date \"+%b %d %Y\"`\"" > build_date.c
	echo "#define BUILD_TIME \"`date \"+%H:%M:%S\"`\"" >> build_date.c


*.o: $(CY_DEPS)
