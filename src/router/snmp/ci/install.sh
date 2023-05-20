#!/bin/bash

# Instead of relying on the hosts file provided by the CI host, replace it.
# See also
# https://blog.justincarmony.com/2011/07/27/mac-os-x-lion-etc-hosts-bugs-and-dns-resolution/.
sudo sh -c 'printf "127.0.0.1 ipv4-loopback\n::1 localhost ipv6-localhost ipv6-loopback\n" >/etc/hosts'

case "$(uname)" in
    Linux)
	packages=(
	    libatm1-dev
	    libkrb5-dev
	    libmariadb-client-lgpl-dev
	    libmariadbclient-dev
	    libmysqlclient-dev
	    libncurses-dev
	    libncurses5-dev
	    libnl-route-3-dev
	    libpcre3-dev
	    libperl-dev
	    libsensors-dev
	    libsensors4-dev
	    libssh2-1-dev
	    libssl-dev
	    make
	    pkg-config
	    python3-dev
	)
	for p in "${packages[@]}"; do
	    sh -c "apt-get -qq install -y $p"
	done
	;;
    Darwin)
	# Upgrade openssl such that Net-SNMP can be built with Blumenthal
	# AES support. Disabled because this upgrade takes long and even
	# sometimes fails.
	if false; then
	    brew upgrade openssl
	fi
	;;
    FreeBSD)
	pkg install -y bash
	pkg install -y gawk
	pkg install -y krb5 krb5-appl krb5-devel
	pkg install -y libssh2
	#pkg install -y openssl111
	pkg install -y perl5 perl5-devel p5-ExtUtils-MakeMaker
	pkg install -y pkgconf
	pkg install -y py27-setuptools
	if [ ! -e /usr/bin/perl ]; then
	    ln -s /usr/local/bin/perl /usr/bin/perl
	fi
	;;
esac

head -n 999 /etc/hosts
