# /**********************************************************
#  SixXS - Automatic IPv6 Connectivity Configuration Utility
# ***********************************************************
#  Copyright 2003-2005 SixXS - http://www.sixxs.net
# ***********************************************************
#  Packaging Makefile
# ***********************************************************
#  $Author: jeroen $
#  $Id: Makefile,v 1.22 2007-01-11 00:29:33 jeroen Exp $
#  $Date: 2007-01-11 00:29:33 $
# **********************************************************/
#
# Note for BSD people: use GNU Make (gmake)

PROJECT=aiccu
PROJECT_DESC="Automatic IPv6 Connectivity Configuration Utility"
PROJECT_VERSION=$(shell grep "AICCU_VER" common/aiccu.h | head -n 1 | awk '{print $$3}' | tr -d \")

# Misc bins
RM=@rm -f
MAKE:=@${MAKE}
CP=@echo [Copy]; cp
RPMBUILD=@echo [RPMBUILD]; rpmbuild
RPMBUILD_SILENCE=>/dev/null 2>/dev/null

# Excludes for limited source release
EXCLUDES=--exclude "${PROJECT}/windows-*" --exclude "${PROJECT}/common/aiccu_win32.c" --exclude CVS --exclude "${PROJECT}/common/tsp*" --exclude "${PROJECT}/common/teepee*"
DEBEXCL=-Iwindows-* -Icommon/aiccu_win32.c -I*CVS* -Icommon/tsp* -Icommon/teepee*

# Change this if you want to install into another dirtree
# Required for eg the Debian Package builder
DESTDIR=
export DESTDIR

# This may be updated by RPM's for instance
CFLAGS=${RPM_OPT_FLAGS}

# Destination Paths (relative to DESTDIR)
dirsbin=/usr/sbin/
dirbin=/usr/bin/
diretc=/etc/
dirdoc=/usr/share/doc/${PROJECT}/

# Make sure the lower makefile also knows these
export PROJECT
export PROJECT_DESC
export PROJECT_VERSION
export PROJECT_COPYRIGHT
export DESTDIR
export RM
export MV
export CC
export CP
export MAKE
export dirsbin
export dirbin
export diretc
export dirdoc
export RPM_OPT_FLAGS
export CFLAGS

####################
## Makefile Targets
####################

all:	Makefile unix-console/
	@echo "Building  : $(PROJECT) - $(PROJECT_DESC)"
	@echo "Copyright : SixXS"
	@echo "Version   : $(PROJECT_VERSION)"
	$(MAKE) -C unix-console all
	@echo "Building done"

install: aiccu
	@echo "Installing into ${DESTDIR}..."
	@echo "Binaries..."
	@mkdir -p ${DESTDIR}${dirsbin}
	$(MAKE) -C unix-console install
	@mkdir -p ${DESTDIR}${dirdoc}
	@echo "Configuration..."
	@mkdir -p ${DESTDIR}${diretc}
ifeq ($(shell echo "A${RPM_BUILD_ROOT}"),A)
	$(shell [ -f ${DESTDIR}${diretc}${PROJECT}.conf ] || cp -R doc/${PROJECT}.conf ${DESTDIR}${diretc}${PROJECT}.conf)
	@echo "Documentation..."
	@cp doc/README ${DESTDIR}${dirdoc}
	@cp doc/LICENSE ${DESTDIR}${dirdoc}
	@cp doc/HOWTO  ${DESTDIR}${dirdoc}
	@echo "Installing Debian-style init.d"
	@mkdir -p ${DESTDIR}${diretc}init.d
	@cp doc/${PROJECT}.init.debian ${DESTDIR}${diretc}init.d/${PROJECT}
else
	@echo "Installing Redhat-style init.d"
	@mkdir -p ${DESTDIR}${diretc}init.d
	@cp doc/${PROJECT}.init.rpm ${DESTDIR}${diretc}init.d/${PROJECT}
	@cp doc/${PROJECT}.conf ${DESTDIR}${diretc}${PROJECT}.conf
endif
	@echo "Installation into ${DESTDIR}/ completed"

help:
	@echo "$(PROJECT) - $(PROJECT_DESC)"
	@echo
	@echo "Makefile targets:"
	@echo "all      : Build everything"
	@echo "help     : This little text"
	@echo "install  : Build & Install into ${DESTDIR}/"
	@echo "clean    : Clean the dirs to be pristine in bondage"
	@echo
	@echo "Distribution targets:"
	@echo "dist     : Make all distribution targets (except rpm's)"
	@echo "tar      : Make source tarball (tar.gz)"
	@echo "bz2      : Make source tarball (tar.bz2)"
	@echo "deb      : Make Debian binary package (.deb)"
	@echo "debsrc   : Make Debian source packages"
	@echo "rpm      : Make RPM package (.rpm)"
	@echo "rpmsrc   : Make RPM source packages"
	@echo
	@echo "SixXS targets:"
	@echo "tarfull  : Full tar including Windows directories" 
	@echo "bz2full  : Full bz2 including Windows directories" 

aiccu:	doc unix-console/
	$(MAKE) -C unix-console all

doc:	doc/aiccu.1

doc/aiccu.1: doc/aiccu.sgml
	docbook-to-man doc/aiccu.sgml >doc/aiccu.1

clean: debclean rpmclean
	$(MAKE) -C unix-console clean
	-${RM} -r windows-gui/Debug
	-${RM} -r windows-gui/Release
	-${RM} windows-gui/AICCU.APS
	-${RM} windows-gui/AICCU.ncb
	-${RM} -r windows-console/Debug
	-${RM} -r windows-console/Release
	-${RM} windows-console/AICCU.APS
	-${RM} windows-console/AICCU.ncb
	-${RM} common/aiccu_linux.o

# Generate Distribution files
dist:	tar bz2 deb

# tar.gz
tar:	clean
	-${RM} ../${PROJECT}_${PROJECT_VERSION}.tar.gz
	tar -zco -C .. ${EXCLUDES} -f ../${PROJECT}_${PROJECT_VERSION}.tar.gz ${PROJECT}

# tar.gz (full)
tarfull: clean
	-${RM} ../${PROJECT}_${PROJECT_VERSION}.tar.gz
	tar -zco -C .. -f ../${PROJECT}_${PROJECT_VERSION}-full.tar.gz ${PROJECT}

# tar.bz2
bz2:	clean
	-${RM} ../${PROJECT}_${PROJECT_VERSION}.tar.bz2
	tar -jco -C .. ${EXCLUDES} -f ../${PROJECT}_${PROJECT_VERSION}.tar.bz2 ${PROJECT}

# tar.bz2 (full)
bz2full: clean
	-${RM} ../${PROJECT}_${PROJECT_VERSION}.tar.bz2
	tar -jco -C .. -f ../${PROJECT}_${PROJECT_VERSION}-full.tar.bz2 ${PROJECT}

# .deb
deb:	clean
	# Copy the changelog
	${CP} doc/changelog debian/changelog
	${CP} doc/${PROJECT}.init.debian debian/${PROJECT}.init
	dpkg-buildpackage $(DEBEXCL) -rfakeroot
	${MAKE} clean

# Cleanup after debian
debclean:
	-${RM} debian/${PROJECT}.init debian/${PROJECT}.conffiles
	if [ -f build-stamp ]; then debian/rules clean; fi

# RPM
rpm:	clean tar
	-${RM} /usr/src/redhat/RPMS/i386/${PROJECT}-*.rpm
	${RPMBUILD} -tb --define '${PROJECT}_version ${PROJECT_VERSION}' ../${PROJECT}_${PROJECT_VERSION}.tar.gz ${RPMBUILD_SILENCE}
	@if [ -d /usr/src/redhat/RPMS/i386/ ]; then mv /usr/src/redhat/RPMS/i386/${PROJECT}-*.rpm ../; fi
	@if [ -d /usr/src/rpm/RPMS/i386/ ]; then mv /usr/src/rpm/RPMS/i386/${PROJECT}-*.rpm ../; fi
	@echo "Resulting RPM's:"
	@ls -l ../${PROJECT}-*.rpm
	${MAKE} clean
	@echo "RPMBuild done"

rpmsrc:	clean tar
	-${RM} /usr/src/redhat/RPMS/i386/${PROJECT}-*src.rpm
	${RPMBUILD} -ts --define '${PROJECT}_version ${PROJECT_VERSION}' ../${PROJECT}_${PROJECT_VERSION}.tar.gz ${RPMBUILD_SILENCE}
	@if [ -d /usr/src/redhat/RPMS/i386/ ]; then mv /usr/src/redhat/RPMS/i386/${PROJECT}-*.src.rpm ../; fi
	@if [ -d /usr/src/rpm/RPMS/i386/ ]; then mv /usr/src/rpm/RPMS/i386/${PROJECT}-*.src.rpm ../; fi
	@echo "Resulting RPM's:"
	@ls -l ../${PROJECT}-*.rpm
	${MAKE} clean
	@echo "RPMBuild-src done"

rpmclean:
	-${RM} ../${PROJECT}_${PROJECT_VERSION}.tar.gz

# Mark targets as phony
.PHONY : all install help clean dist tar bz2 deb debclean rpm rpmsrc

