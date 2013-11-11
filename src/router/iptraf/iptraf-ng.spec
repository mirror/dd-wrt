Summary:        A console-based network monitoring utility
Name:           iptraf-ng
Version:        1.1.4
Release:        1%{?dist}
Source0:        https://fedorahosted.org/releases/i/p/iptraf-ng/%{name}-%{version}.tar.gz
Source1:        iptraf-ng-logrotate.conf
URL:            https://fedorahosted.org/iptraf-ng/
License:        GPLv2+
Group:          Applications/System
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:  ncurses-devel
Obsoletes:      iptraf < 3.1
Provides:       iptraf = 3.1

%description
IPTraf-ng is a console-based network monitoring utility.  IPTraf gathers
data like TCP connection packet and byte counts, interface statistics
and activity indicators, TCP/UDP traffic breakdowns, and LAN station
packet and byte counts.  IPTraf-ng features include an IP traffic monitor
which shows TCP flag information, packet and byte counts, ICMP
details, OSPF packet types, and oversized IP packet warnings;
interface statistics showing IP, TCP, UDP, ICMP, non-IP and other IP
packet counts, IP checksum errors, interface activity and packet size
counts; a TCP and UDP service monitor showing counts of incoming and
outgoing packets for common TCP and UDP application ports, a LAN
statistics module that discovers active hosts and displays statistics
about their activity; TCP, UDP and other protocol display filters so
you can view just the traffic you want; logging; support for Ethernet,
FDDI, ISDN, SLIP, PPP, and loopback interfaces; and utilization of the
built-in raw socket interface of the Linux kernel, so it can be used
on a wide variety of supported network cards.

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

# remove everything besides the html and pictures in Documentation
find Documentation -type f | grep -v '\.html$\|\.png$\|/stylesheet' | \
     xargs rm -f

install -D -m 0644 -p %{SOURCE1} $RPM_BUILD_ROOT%{_sysconfdir}/logrotate.d/iptraf-ng

install -d -m 0755 $RPM_BUILD_ROOT%{_localstatedir}/{lock,log,lib}/iptraf-ng

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc CHANGES FAQ LICENSE README* RELEASE-NOTES
%doc Documentation
%{_sbindir}/iptraf-ng
%{_sbindir}/rvnamed-ng
%{_mandir}/man8/iptraf-ng.8*
%{_mandir}/man8/rvnamed-ng.8*
%{_localstatedir}/lock/iptraf-ng
%{_localstatedir}/log/iptraf-ng
%{_localstatedir}/lib/iptraf-ng
%config(noreplace) %{_sysconfdir}/logrotate.d/iptraf-ng

%changelog
* Wed Jan 11 2011 Nikola Pajkovsky <npajkovs@redhat.com> - 1.1.0-1
- Initialization build
