# Figure out release tag (e.g. rhel3, fc1) based on redhat-release file
%global _RHTAG %(sed 's/(.*)//;s/ [A-Z]* //;s/[a-z ]*//g' /etc/redhat-release | tr '[:upper:]' '[:lower:]')

Name: softflowd
Summary: Network traffic analyser capable of Cisco NetFlow data export
Version: 0.9.9
Release: 1.%{_RHTAG}
Source: softflowd-%{version}.tar.gz
Group: System/Utilities
License: BSD
BuildRoot: %{_tmppath}/%{name}-root
URL: http://www.mindrot.org/softflowd.html
Vendor: mindrot.org

%description
softflowd is a software implementation of a flow-based network traffic
monitor.  softflowd reads network traffic and gathers information about
active traffic flows.  A "traffic flow" is communication between two IP
addresses or (if the overlying protocol is TCP or UDP) address/port tuples.
The intended use of softflowd is as a software implementation of Cisco’s
NetFlow traffic account system.  softflowd supports data export using
versions 1, 5 or 9 of the NetFlow protocol.

%prep
%setup

%build
%configure
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/etc/rc.d/init.d
mkdir -p $RPM_BUILD_ROOT/etc/sysconfig
cp %_sourcedir/softflowd.init $RPM_BUILD_ROOT/etc/rc.d/init.d/softflowd
cp %_sourcedir/softflowd.sysconfig $RPM_BUILD_ROOT/etc/sysconfig/softflowd

%files
%defattr(-,root,root)
/usr/sbin/*
/usr/share/man/*
%attr(0755,root,root) /etc/rc.d/init.d/softflowd
%config(noreplace) %attr(0644,root,root) /etc/sysconfig/softflowd
%doc ChangeLog README TODO

%clean
rm -rf $RPM_BUILD_ROOT
