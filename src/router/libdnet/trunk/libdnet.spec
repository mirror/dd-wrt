#
# Libdnet -- Simplified interface to low-level networking (RPM package spec)
#
#
#	Options:
#		--with gnuld    Assume the C compiler uses GNU ld
#						(see --with-gnu-ld in ./configure)
#
%define name libdnet
%define version 1.12
%define release 1
%define appname %{name}-%{version}
%define rpmname %{name}-%{version}-%{release}
%define buildname %{appname}
Name: %{name}
Version: %{version}
Release: %{release}
Summary: Simplified, portable interface to low-level networking routines
Group: System Environment/Libraries
Vendor: Dug Song <dugsong@monkey.org>
URL: http://libdnet.sourceforge.net
License: BSD
Source: http://prdownloads.sourceforge.net/libdnet/%{appname}.tar.gz
Prefix: %{_prefix}
BuildRoot: %{_tmppath}/%{name}-root


%description
libdnet provides a simplified, portable interface to several low-level
networking routines, including network address manipulation, kernel
arp(4) cache and route(4) table lookup and manipulation, network
firewalling, network interface lookup and manipulation, IP tunnelling, 
and raw IP packet and Ethernet frame transmission.


%package devel
Summary: Development files for %{name}
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}


%description devel
Development files (Headers, libraries for static linking, etc) for %{name}.


%prep
rm -rf $RPM_BUILD_DIR/%{buildname}
%setup -n %{buildname}


%build
%configure %{?_with_gnuld:--with-gnu-ld}
make


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
rm $RPM_BUILD_ROOT/.%{_libdir}/*.la


%clean
rm -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_DIR/%{buildname}


%post -p /sbin/ldconfig


%preun -p /sbin/ldconfign


%files
%defattr(-,root,root)
%{_sbindir}
%{_libdir}/*.so.*
%{_mandir}/man8


%files devel
%defattr(-,root,root)
%{_bindir}
%{_includedir}
%{_libdir}/*.a
%{_libdir}/*.so
%{_mandir}/man3
%doc	README
%doc	TODO


%changelog
* Wed Jun  1 2004 nnposter at users,sourceforge,net
- Created 1.8-1 RPM spec


# vim:ts=4:
