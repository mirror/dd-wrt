Summary: Basic Networking Tools
Name: net-tools
Version: 1.50
Release: 1
Copyright: GPL
Group: Networking/Admin
Source: http://www.tazenda.demon.co.uk/phil/net-tools/net-tools-%{version}.tar.gz
Patch0: net-tools-1.45-config.patch
BuildRoot: /var/tmp/%{name}-root

%description
This is a collection of the basic tools necessary for setting up networking
on a Linux machine. It includes ifconfig, route, netstat, rarp, and
various other tools.

%prep
%setup  -q
%patch0 -p1 -b .config

%build
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/bin
mkdir -p $RPM_BUILD_ROOT/sbin
mkdir -p $RPM_BUILD_ROOT/usr/man/man1
mkdir -p $RPM_BUILD_ROOT/usr/man/man5
mkdir -p $RPM_BUILD_ROOT/usr/man/man8

make BASEDIR=$RPM_BUILD_ROOT install

( cd $RPM_BUILD_ROOT/sbin
  strip arp ifconfig rarp route slattach plipconfig ipmaddr iptunnel
  cd ../bin
  strip hostname netstat
) 

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/sbin/arp
/sbin/ifconfig
/sbin/rarp
/sbin/route
/sbin/slattach
/sbin/plipconfig
/sbin/ipmaddr
/sbin/iptunnel
/bin/domainname
/bin/dnsdomainname
/bin/hostname
/bin/netstat
/bin/nisdomainname
/bin/ypdomainname
/usr/man/man[158]/*
/usr/man/pt_BR/man[18]/*
/usr/man/fr_FR/man[18]/*
/usr/share/locale/*/LC_MESSAGES/net-tools.mo

%changelog
* Sun Jan 10 1999 Phil Blundell <philb@gnu.org>
- update to 1.50
- add new stuff to %files and strip it in %install

* Thu Nov 26 1998 Phil Blundell <pb@nexus.co.uk>
- update to 1.48.

* Thu Nov 12 1998 Jeff Johnson <jbj@redhat.com>
- update to 1.47.

* Wed Sep  2 1998 Jeff Johnson <jbj@redhat.com>
- update to 1.46

* Thu Jul  9 1998 Jeff Johnson <jbj@redhat.com>
- build root
- include ethers.5

* Thu Jun 11 1998 Aron Griffis <agriffis@coat.com>
- upgraded to 1.45
- patched hostname.c to initialize buffer
- patched ax25.c to use kernel headers

* Fri May 01 1998 Prospector System <bugs@redhat.com>
- translations modified for de, fr, tr

* Fri Feb 27 1998 Jason Spangler <jasons@usemail.com>
- added config patch

* Fri Feb 27 1998 Jason Spangler <jasons@usemail.com>
- changed to net-tools 1.432
- removed old glibc 2.1 patch
 
* Wed Oct 22 1997 Erik Troan <ewt@redhat.com>
- added extra patches for glibc 2.1

* Tue Oct 21 1997 Erik Troan <ewt@redhat.com>
- included complete set of network protocols (some were removed for
  initial glibc work)

* Wed Sep 03 1997 Erik Troan <ewt@redhat.com>
- updated glibc patch for glibc 2.0.5

* Thu Jun 19 1997 Erik Troan <ewt@redhat.com>
- built against glibc
- updated to 1.33
