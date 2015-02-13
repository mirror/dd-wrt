#!/usr/bin/make -f
# file: plc-utils/Makefile
# Published 2010 by Qualcomm Atheros;

# ====================================================================
# target platform symbols;
# --------------------------------------------------------------------

include make.def

# ====================================================================
# package symbols;
# --------------------------------------------------------------------

PROJECT=open-plc-utils
RELEASE=$(shell basename "${CURDIR}")
LIBRARY=${FTP}/toolkit/${RELEASE}
CDROM=${FTP}/cdrom/${RELEASE}
FOLDERS=ether key mdio mme nvm nodes pib plc ram serial slac tools VisualStudioNET 
EXCLUDE=--exclude=.git --exclude=.#* --exclude=*.[0-9][0-9][0-9]

# ====================================================================
# installation targets;
# --------------------------------------------------------------------

.PHONY: all compile compact scripts manuals install uninstall check fresh clean ignore

all compile compact scripts manuals install uninstall check fresh clean ignore:
	@for folder in ${FOLDERS}; do ${MAKE} -C $${folder} ${@} || exit 1; done

# ====================================================================
# releasetargets;
# --------------------------------------------------------------------

.PHONY: package library prepare cleanse 

debian: clean
	dpkg-buildpackage -rfakeroot -uc -us
debian-setup:
	apt-get install dpkg-dev debhelper devscripts fakeroot linda
#	apt-get install dpkg dpkg-dev debhelper dh-make devscripts quilt
package: archive prepare library mine
prepare: 
	rm -fr t.* install.* *.err *.log 
	rm -fr */t.* */install.* */*.err */*.log */*.o */*.[0-9][0-9][0-9]
library: 
	install -m 6775 -o root -g fae -d ${LIBRARY} 
	install -m 0555 -o root -g root VisualStudioNET/*.msi ${LIBRARY}
	install -m 0555 -o root -g root ../${RELEASE}.tar.* ${LIBRARY}
	cp -r VisualStudioNET/Programs/* ${LIBRARY}/programs
	crlf -w < CHANGES > ${LIBRARY}/_CHANGES.txt
	crlf -w < README > ${LIBRARY}/_README.txt
	crlf -w < LICENCES > ${LIBRARY}/_LICENSES.txt
	crlf -w < NOTICES > ${LIBRARY}/_NOTICES.txt
	ls -la ${LIBRARY}/*.tar.*
	ls -la ${LIBRARY}/*.msi

# ====================================================================
# maintain;
# --------------------------------------------------------------------

.PHONY: archive restore 

archive: clean 
	tar ${EXCLUDE} -vzcf ../${RELEASE}.tar.gz  -C .. ${RELEASE}
	tar ${EXCLUDE} -vjcf ../${RELEASE}.tar.bz2 -C .. ${RELEASE}
restore:
	tar -vzxf ../${RELEASE}.tar.gz -C ..

