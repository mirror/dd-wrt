#
# Rocco Carbone <rocco@ntop.org>
#
# Makefile to build Solaris package for
#
#           ____             __
#    ____  / __ \_________  / /_  ___
#   / __ \/ /_/ / ___/ __ \/ __ \/ _ \
#  / / / / ____/ /  / /_/ / /_/ /  __/
# /_/ /_/_/   /_/   \____/_.___/\___/
#
#

# Do not write anything to the standard output when executing pkg* commands
QUIET      = 1> /dev/null 2>&1

# who is making the package
WHO        = `id | sed -e "s/uid=[0-9]*(\([a-z]*\)) gid=[0-9]*(\([a-z]*\)).*/\1 \2/"`

PACKAGER   = `/usr/ucb/whoami`@`hostname`
YEAR       = `date | sed -e 's%^.*\ \([0-9]*\)%\1%'`
MONTH      = `date | cut -d ' ' -f 2`
DAY        = `date | cut -d ' ' -f 4`
PKGSERIAL  = ${YEAR}${MONTH}${DAY}-`cat SERIAL`

#
# root installation directory
PREFIX     = /usr/local

DISTRIBDIR = ${PREFIX}/packages

# owner of the files installed with the package
USER       = root
GROUP      = bin

# PKGINST is an abbreviation of SW due to a limit up to 9 characters of the Solaris packaging
PKGINST    = nprobe
SW         = nProbe
# VERSION    = `ls -d ../../${SW}* | sed -e 's%^.*/%%' | sed -e 's%^.*-%%'`
VERSION    = 3.9.2
DESC       = nProbe is a NetFlow v5 and v9 software probe.
VENDOR     = Luca Deri
WWW        = http://www.ntop.org/nProbe.html
EMAIL      = info@ntop.org
DATE       = `date`
OS         = `grep OS= /var/sadm/system/admin/INST_RELEASE | cut -c 4-`
OSVERS     = `grep VERSION= /var/sadm/system/admin/INST_RELEASE | cut -c 9-`
ARCH       = `uname -p`
PLATFORM   = ${ARCH}-${OS}-${OSVERS}
PACKAGE    = ${SW}-${VERSION}-${PLATFORM}.pkg
PCKGDEMO   = ${SW}-${VERSION}-demo-${PLATFORM}.pkg
TEMPLATE   = pkginfo.in

all: copy serial custom build-package

copy:
	@mkdir -p /usr/local/sbin
	@cp ../nprobe /usr/local/sbin
	@strip /usr/local/sbin/nprobe

build-package: custom
#
# build the pkginfo file from the template file
#
	@echo "making pkginfo .........................................."
	@sed -e "s/PKG=.*/PKG=${PKGINST}/" \
             -e "s/NAME=.*/NAME=${SW} ${VERSION} ${PLATFORM}/" \
             -e "s/VERSION=.*/VERSION=${VERSION}/" \
             -e "s/ARCH=.*/ARCH=${PLATFORM}/" \
             -e "s/DESC=.*/DESC=${DESC}/" \
             -e "s/VENDOR=.*/VENDOR=${VENDOR}/" \
             -e "s/EMAIL=.*/EMAIL=${EMAIL}/" \
             -e "s/PSTAMP=.*/PSTAMP=${PKGSERIAL} packaged by ${PACKAGER}/" \
             -e "s/PKGINST=.*/PKGINST=${SW}/" \
             -e "s|PKGSAV=.*|PKGSAV=/var/sadm/pkg/${SW}/save|" \
         < ${TEMPLATE} > pkginfo
	@echo "making list of files ...................................."
	@sed -e "s%^@PREFIX@%${PREFIX}%g" < list-of-files.in > list-of-files
#
# build the prototype file
#
	@echo "making table of contents (prototype) ...................."
	@echo "i pkginfo" > prototype
	@echo "i copyright=Copyright" >> prototype
	@echo "i checkinstall" >> prototype
	@cat list-of-files | pkgproto >> prototype
#
# build the package file
#
	@echo "making relocable package ................................"
	@-/bin/rm -rf tmp && /bin/mkdir tmp
#	@pkgmk -a `uname -p` -o -r ${PREFIX} -d tmp -f prototype ${QUIET}
	@pkgmk -a `uname -p` -o -r / -d tmp -f prototype ${QUIET}
#
# a directory into tmp called ${PKGINST} was created with the two files pkgmap and pkginfo
#
	@/bin/rm -f ${PACKAGE}
	@touch ${PACKAGE}
	@echo 1 | pkgtrans -o -s tmp ${PACKAGE} ${QUIET}
	@/bin/rm -rf tmp
	@echo "The package file is in ${PACKAGE}"

serial:
	@if [ ! -f SERIAL ] ; then \
           echo 1 > SERIAL; \
         else \
           expr `cat SERIAL` + 1 > SERIAL; \
         fi

custom:
	@echo "making Copyright ........................................"
	@sed -e "s/@SW@/${SW}/" \
             -e "s/@VERSION@/${VERSION}/" \
             -e "s/@OS@/${OS}/" \
             -e "s/@OSVERS@/${OSVERS}/" \
             -e "s/@ARCH@/${ARCH}/" \
             -e "s/@DATE@/${DATE}/" \
         < Copyright.in > Copyright
	@echo "making checkinstall ....................................."
	@sed -e "s/@OSVERS@/${OSVERS}/" \
         < checkinstall.in > checkinstall
	@for file in request preinstall postinstall preremove postremove; do \
           if [ -f $$file.in ]; then \
             echo "making $$file ......................................"; \
             cat $$file.in | \
             sed -e "s%@PREFIX@%${PREFIX}%g" \
                 -e "s/@SW@/${SW}/" \
                 -e "s/@USER@/${USER}/" \
                 -e "s/@GROUP@/${GROUP}/" \
             > $$file; \
           fi \
         done

clean:
	@/bin/rm -f Copyright checkinstall pkginfo
	@/bin/rm -f request preinstall postinstall preremove postremove list-of-files
	@/bin/rm -f prototype ${PACKAGE} *~

install:
	@/bin/cp ${PACKAGE} ${DISTRIBDIR}/${PACKAGE}

demo:
	@make PACKAGE=${PCKGDEMO} all
	@/bin/mv ${PACKAGE} ${PCKGDEMO}
