#
#CC=/home/luis/devel/Kai/buildroot/build_mipsel/staging_dir/bin/mipsel-linux-gcc CXX=/home/luis/devel/Kai/buildroot/build_mipsel/staging_dir/bin/mipsel-linux-g++ CFLAGS=-I/home/luis/devel/Kai/buildroot/build_mipsel/staging_dir/mipsel-linux-uclibc/include CXXFLAGS=/home/luis/devel/Kai/buildroot/build_mipsel/staging_dir/mipsel-linux-uclibc/include/c++ ac_cv_linux_vers=foo ./configure --host=mips --with-pcap=linux
# To compile stuff for the WRT ^^^^^^^^
#
#
VERSION		= 7.0.0.5
ARCH		= x86
PLATFORM    	= KaiStation
OPENWRT		= /opt/openwrt401
#OPENWRT			= /home/luis/devel/buildroot/build_mipsel/staging_dir
CC=mipsel-linux-uclibc-gcc
#CXX=i386-linux-g++-3.4
#PREFIX=
IPKG		= /home/luis/devel/Kai/ipkg
PRE_CFLAGS  = -D_REENTRANT -DPLATFORM_$(PLATFORM) -DVERSION=\"$(VERSION)\"
#STATIC		= -static
CFLAGS		= $(SYSTEM_CFLAGS) $(PRE_CFLAGS) -Os -pipe -mtune=mips32 -mips32 -funit-at-a-time -Wall -Ipthreadcc/src -Isocketcc/src -L../../libpcap -I../../libpcap $(STATIC) -DCONFIG_FILE="\"$(PREFIX)/etc/kaid.conf\""
ifneq ($(strip $(OPENWRT)),)
#PRE_CFLAGS	+= -nostdinc
PRE_CFLAGS	+= -DOPENWRT -static
#CFLAGS		+= -I$(OPENWRT)/mipsel-linux-uclibc/include/c++
CXX		= mipsel-linux-g++
STRIP		= mipsel-linux-strip
DISTHOST	= root@adm.zbit.pt:/var/www/html/www.zbit.pt/xbox/
else
STRIP		= strip
ifeq ($(PLATFORM),macosx_jaguar)
CFLAGS		+= -I/sw/include
endif
endif
CXXFLAGS	+= $(CFLAGS)
PWD			= /home/luis/devel/Kai/kaid

ifeq ($(strip $(PLATFORM)),macosx_jaguar)
BSD		= true
endif
ifeq ($(strip $(PLATFORM)),freebsd)
BSD		= true
endif
ifdef BSD
PRE_CFLAGS  += -DPLATFORM_bsd
endif

PCAP		= ../../libpcap/pcap.o \
../../libpcap/inet.o \
../../libpcap/gencode.o \
../../libpcap/savefile.o \
../../libpcap/bpf_filter.o \
../../libpcap/optimize.o \
../../libpcap/nametoaddr.o \
../../libpcap/etherent.o \
../../libpcap/grammar.o \
../../libpcap/scanner.o

ifdef BSD
PCAP	+= ../../libpcap/pcap-bpf.o
else
PCAP	+= ../../libpcap/pcap-linux.o
endif

ifneq ($(strip $(OPENWRT)),)
PCAP		+= ../../libpcap/fad-gifc.o
PRE_CFLAGS	+= -DOPENWRT
else
PCAP		+= ../../libpcap/fad-getad.o
#PCAP        += libpcap/fad-gifc.o
endif

# Uneeded obj, cutting down size for OPENWRT static bin
#libpcap/bpf_dump.o
#libpcap/bpf_image.o
#libpcap/version.o

PTCC		= pthreadcc/obj/*.o
SOCC		= socketcc/obj/*.o

ifeq ($(strip $(PLATFORM)),freebsd)
LIBS		= -pthread -lutil
else
LIBS		= -lpthread
endif

SRCS		= Api.cc \
ConfigFile.cc \
KaiDaemon.cc \
Kaid.cc \
KaiEngine.cc \
KaiItem.cc \
OrbDgramThread.cc \
OrbStreamThread.cc \
PktSnifferThread.cc \
StrUtils.cc \
TimerThread.cc \
OSXTimerThread.cc \
ifaddrlist.c

OBJS		= Api.o \
ConfigFile.o \
KaiDaemon.o \
Kaid.o \
KaiEngine.o \
KaiItem.o \
OrbDgramThread.o \
OrbStreamThread.o \
PktSnifferThread.o \
StrUtils.o \
TimerThread.o \
OSXTimerThread.o \
ifaddrlist.o


#DEPS		= .deps/Api.d \
#.deps/ConfigFile.d \
#.deps/KaiDaemon.d \
#.deps/Kaid.d \
#.deps/KaiEngine.d \
#.deps/KaiItem.d \
#.deps/OrbDgramThread.d \
#.deps/OrbStreamThread.d \
#.deps/PktSnifferThread.d \
#.deps/TimerThread.d \
#.deps/OSXTimerThread.d \
#.deps/ifaddrlist.d

all: kaid

ifeq ($(strip $(PLATFORM)),freebsd)
libpcap:
	${MAKE} -C libpcap all

pthreadcc:
	${MAKE} -C pthreadcc all

socketcc:
	${MAKE} -C socketcc all

install:
.PHONY: libpcap pthreadcc socketcc install

kaid: libpcap pthreadcc socketcc ${OBJS}
else
kaid: ${OBJS}
endif
	@echo -n "Linking... "
	${CXX} ${CFLAGS} ${LDFLAGS} -o $@ ${OBJS} ${PTCC} ${PCAP} ${SOCC} ${LIBS}
	@echo "Done!"

GENDEPS=${CXX} ${CXXFLAGS} -MM -o $@ $<

#${DEPS}: ${SRCS}
#	mkdir -p .deps
#	${GENDEPS}

.PHONY: dist all clean

#ifneq (${MAKECMDGOALS},clean)
#include	${DEPS}
#endif

dist: kaid
	@echo "Creating dist files and package(s)..."
	@rm -rf dist/kaid
	@mkdir -p dist/kaid
	@cp kaid dist/kaid/
	@strip dist/kaid/kaid
	@cd dist/kaid && md5sum kaid > MD5SUM && cd -
	@sed -e 's:\(Password =\).*:\1:g' kaid.conf > dist/kaid/kaid.conf
	@cp README WHATSNEW dist/kaid/
	@tar cjv -C dist -f dist/kaid-$(VERSION)-$(PLATFORM)_$(ARCH).tar.bz2 kaid/
#	@scp dist/kaid-$(VERSION)-$(PLATFORM).tar.bz2 $(DISTHOST)

ipkg: kaid
	@echo "Creating openwrt ipkg..."
	@rm -rf dist/mips
	@mkdir -p dist/mips/usr/bin
	@mkdir -p dist/mips/etc
	@cp kaid dist/mips/usr/bin
	@cp kaid.conf dist/mips/etc
	@$(OPENWRT)/bin/mipsel-linux-strip dist/mips/usr/bin/kaid
	@mkdir -p dist/mips/CONTROL
	@sed -e 's:#VERSION#:$(VERSION):g' control > dist/mips/CONTROL/control
	$(IPKG)/ipkg-build -c -o root -g root $(PWD)/dist/mips $(PWD)/dist
 
indent:
	indent -br -ce -ppi 4 -brs -nbc -ts4 -npsl $(SRCS)

clean:
	@echo "Cleaning binary and dist files..."
	@rm -rf dist/
	@rm -f ${OBJS} ${DEPS} kaid *~ *.o

dist-clean:
	@echo "Cleaning binary and dist files..."
	@rm -rf dist/
	@rm -f ${OBJS} ${DEPS} kaid *~ *.o
	${MAKE} -C libpcap clean
	${MAKE} -C pthreadcc clean
	${MAKE} -C socketcc clean

REL=1
BUILDDIR=/tmp/kaid-dist
DISTDIR=${BUILDDIR}/kaid-${VERSION}

rpm:
	rm -rf ${BUILDDIR}
	mkdir -p ${DISTDIR}/usr/sbin
	mkdir -p ${DISTDIR}/etc
	cp kaid ${DISTDIR}/usr/sbin
	cp kaid.conf ${DISTDIR}/etc
	sed -e 's:#VERSION#:Version\: ${VERSION}:g' -e 's:#RELEASE#:Release\: ${REL}:g' packages/kaid.spec.template > ${DISTDIR}/kaid.spec
	tar cjv -C ${BUILDDIR} -f ${BUILDDIR}/kaid-${VERSION}.tar.bz2 kaid-${VERSION}/
	rpmbuild -tb ${BUILDDIR}/kaid-${VERSION}.tar.bz2
	rpm --addsign /home/luis/rpm/RPMS/i386/kaid-${VERSION}-${REL}.i386.rpm
# scp /home/luis/redhat/RPMS/i386/bsafe-${VERSION}-${REL}.i386.rpm root@smb:/home/apt/bsafe/RPMS.updates
#	ssh root@smb /root/gen-apt
