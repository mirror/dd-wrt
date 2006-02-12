###############################################################################
#
# General
#
###############################################################################

Name: dnsmasq
Version: 2.22
Release: 1
Copyright: GPL
Group: Productivity/Networking/DNS/Servers
Vendor: Simon Kelley
Packager: Simon Kelley
URL: http://www.thekelleys.org.uk/dnsmasq
Provides: dns_daemon
Conflicts: bind bind8 bind9
PreReq: %fillup_prereq %insserv_prereq
Autoreqprov: on
Source0: %{name}-%{version}.tar.bz2
BuildRoot: /var/tmp/%{name}-%{version}
Summary: A lightweight caching nameserver

%description
Dnsmasq is lightweight, easy to configure DNS forwarder and DHCP server. It 
is designed to provide DNS and, optionally, DHCP, to a small network. It can
serve the names of local machines which are not in the global DNS. The DHCP 
server integrates with the DNS server and allows machines with DHCP-allocated
addresses to appear in the DNS with names configured either in each host or 
in a central configuration file. Dnsmasq supports static and dynamic DHCP 
leases and BOOTP for network booting of diskless machines.



###############################################################################
#
# Build
#
###############################################################################

%prep
%setup -q
patch -p0 <rpm/%{name}-SuSE.patch

%build
%{?suse_update_config:%{suse_update_config -f}}
make

###############################################################################
#
# Install
#
###############################################################################

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p ${RPM_BUILD_ROOT}/etc/init.d
mkdir -p ${RPM_BUILD_ROOT}/usr/sbin
mkdir -p ${RPM_BUILD_ROOT}%{_mandir}/man8
install -o root -g root -m 755 rpm/rc.dnsmasq-suse $RPM_BUILD_ROOT/etc/init.d/dnsmasq
install -o root -g root -m 644 dnsmasq.conf.example $RPM_BUILD_ROOT/etc/dnsmasq.conf
strip src/dnsmasq
install -o root -g root -m 755 src/dnsmasq $RPM_BUILD_ROOT/usr/sbin
ln -sf ../../etc/init.d/dnsmasq $RPM_BUILD_ROOT/usr/sbin/rcdnsmasq
gzip -9 dnsmasq.8
install -o root -g root -m 644 dnsmasq.8.gz $RPM_BUILD_ROOT%{_mandir}/man8

###############################################################################
#
# Clean up
#
###############################################################################

%clean
rm -rf $RPM_BUILD_ROOT

###############################################################################
#
# Post-install scriptlet
#
###############################################################################

%post
%{fillup_and_insserv dnsmasq}

###############################################################################
#
# Post-uninstall scriptlet
#
# The %postun script executes after the package has been removed. It is the
# last chance for a package to clean up after itself.
#
###############################################################################

%postun
%{insserv_cleanup}

###############################################################################
#
# File list
#
###############################################################################

%files
%defattr(-,root,root)
%doc CHANGELOG COPYING FAQ doc.html setup.html UPGRADING_to_2.0 rpm/README.susefirewall
%doc contrib
%config /etc/init.d/dnsmasq
%config /etc/dnsmasq.conf
/usr/sbin/rcdnsmasq
/usr/sbin/dnsmasq
%doc %{_mandir}/man8/dnsmasq.8.gz



