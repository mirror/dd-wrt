install:	atm atm.init atmsigd.conf ifcfg-atm0 ifup-atm
		install -c -m 0644 atm /etc/sysconfig
		install -c -m 0644 atm.init /etc/rc.d/init.d/atm
		install -c -m 0644 atmsigd.conf /etc
		install -c -m 0755 ifcfg-atm0 ifup-atm \
		  /etc/sysconfig/network-scripts
