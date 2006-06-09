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
CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/
else
CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v23 -Wall -I$(SRCBASE)/
endif

ifeq ($(CONFIG_VFS),y)
CFLAGS += -I$(TOP)/vfs -DHAVE_VFS
LDFLAGS += -L$(TOP)/vfs
endif

LDFLAGS	+= -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram -L$(TOP)/shared  -L$(INSTALLDIR)/shared/usr/lib -lshared

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
ifeq ($(CONFIG_RB500),y)
CFLAGS += -DHAVE_RB500
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
OBJS += sputnik.o
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
OBJS  += ddm.o
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
  CFLAGS += -DHAVE_MADWIFI -I../madwifi/madwifi.dev -include ../madwifi/madwifi.dev/include/compat.h
#  OBJS += ../wireless-tools/libiw.so.29
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
OBJS  += arp.o
endif
ifeq ($(EMI_TEST),1)
OBJS  += emi_test.o
endif
ifeq ($(DEBUG_WEB),1)
CFLAGS += -DMY_DEBUG
endif
ifeq ($(WAKE_ON_LAN_SUPPORT),1)
OBJS  += wol.o
endif
ifeq ($(SAMBA_SUPPORT),1)
OBJS  += samba.o
endif
#ifeq ($(HW_QOS_SUPPORT),1)
#OBJS  += qos.o
#endif
ifeq ($(PERFORMANCE_SUPPORT),1)
OBJS  += performance.o
endif
#ifeq ($(WL_STA_SUPPORT),1)
OBJS  += site_survey.o
#endif

#OBJS += http.o

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
OBJS += matrixssl_xface.o
endif

#OBJS  += common.o server.o
#OBJS  += code_header.o


vpath %.c modules $(SRCBASE)/../tools/src

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
#	$(STRIP) $(INSTALLDIR)/usr/sbin/httpd
	install -d $(INSTALLDIR)/etc
	install *.pem $(INSTALLDIR)/etc

cert:
	./gencert.sh

httpd: cgi.o ej.o httpd.o broadcom.o \
       index.o status.o dhcp.o log.o upgrade.o filters.o forward.o dynamic_route.o static_route.o wireless.o ddns.o \
       find_pattern.o lib.o diag.o sysinfo.o wepkey.o md5c.o wl_test.o getservice.o dd-wrt.o nvramsr.o $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
	rm -f ../rc/rc
	
build_date.o: build_date.c
build_date:
	echo "#define BUILD_DATE \"`date \"+%b %d %Y\"`\"" > build_date.c
	echo "#define BUILD_TIME \"`date \"+%H:%M:%S\"`\"" >> build_date.c


*.o: $(CY_DEPS)
