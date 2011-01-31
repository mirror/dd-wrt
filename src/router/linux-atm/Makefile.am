SUBDIRS = src doc

EXTRA_DIST = COPYING.GPL COPYING.LGPL BUGS

rpms: dist src/extra/linux-atm.spec
	-mkdir -p src/extra/RPMS
	-mkdir -p src/extra/SRPMS
	rpmbuild -ba src/extra/linux-atm.spec

rpm: rpms
	@echo -n

cscope:
	cscope -b -R

