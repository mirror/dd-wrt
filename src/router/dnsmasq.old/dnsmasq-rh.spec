###############################################################################
#
# General mumbojumbo
#
###############################################################################

Name: dnsmasq
Version: 2.22
Release: 1
Copyright: GPL
Group: System Environment/Daemons
Vendor: Simon Kelley
Packager: Simon Kelley
Distribution: Red Hat Linux
URL: http://www.thekelleys.org.uk/dnsmasq
Source0: %{name}-%{version}.tar.gz
Requires: chkconfig
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
%build
make


###############################################################################
#
# Install
#
###############################################################################

%install
rm -rf $RPM_BUILD_ROOT

mkdir -p -m 755 $RPM_BUILD_ROOT/usr/sbin
mkdir -p -m 755 $RPM_BUILD_ROOT/etc/rc.d/init.d
mkdir -p -m 755 $RPM_BUILD_ROOT/usr/share/man/man8

cp rpm/dnsmasq.rh $RPM_BUILD_ROOT/etc/rc.d/init.d/dnsmasq
strip src/dnsmasq
cp src/dnsmasq $RPM_BUILD_ROOT/usr/sbin
cp dnsmasq.8 $RPM_BUILD_ROOT/usr/share/man/man8
cp dnsmasq.conf.example $RPM_BUILD_ROOT/etc/dnsmasq.conf

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
/sbin/chkconfig --add dnsmasq


###############################################################################
#
# Pre-uninstall scriptlet
#
# If there's a time when your package needs to have one last look around before
# the user erases it, the place to do it is in the %preun script. Anything that
# a package needs to do immediately prior to RPM taking any action to erase the
# package, can be done here.
#
###############################################################################

%preun
if [ $1 = 0 ]; then     # execute this only if we are NOT doing an upgrade
    service dnsmasq stop >/dev/null 2>&1
    /sbin/chkconfig --del dnsmasq
fi


###############################################################################
#
# Post-uninstall scriptlet
#
# The %postun script executes after the package has been removed. It is the
# last chance for a package to clean up after itself.
#
###############################################################################

%postun
if [ "$1" -ge "1" ]; then
    service dnsmasq restart >/dev/null 2>&1
fi


###############################################################################
#
# File list
#
###############################################################################

%files
%defattr(-,root,root)
%doc CHANGELOG COPYING FAQ doc.html setup.html UPGRADING_to_2.0
%config /etc/rc.d/init.d/dnsmasq 
%config /etc/dnsmasq.conf
%attr(0755,root,root) /etc/rc.d/init.d/dnsmasq
%attr(0664,root,root) /etc/dnsmasq.conf
%attr(0755,root,root) /usr/sbin/dnsmasq
%attr(0644,root,root) /usr/share/man/man8/dnsmasq*


