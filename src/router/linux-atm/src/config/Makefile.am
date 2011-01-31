EXTRA_DIST = hosts.atm init-redhat/README init-redhat/atm init-redhat/atm-sysconfig \
		init-redhat/ifcfg-atm0 init-redhat/ifcfg-lec0 init-redhat/ifcfg-nas0

install-exec-local:
	-cp hosts.atm /etc

uninstall-local:
	-rm /etc/hosts.atm

sysconf_DATA = hosts.atm
