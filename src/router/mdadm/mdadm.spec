Summary:     mdadm is used for controlling Linux md devices (aka RAID arrays)
Name:        mdadm
Version:     4.1
Release:     1
Source:      http://www.kernel.org/pub/linux/utils/raid/mdadm/mdadm-%{version}.tar.gz
URL:         http://neil.brown.name/blog/mdadm
License:     GPL
Group:       Utilities/System
BuildRoot:   %{_tmppath}/%{name}-root
Obsoletes:   mdctl

%description
mdadm is a program that can be used to create, manage, and monitor
Linux MD (Software RAID) devices.

%prep
%setup -q
# we want to install in /sbin, not /usr/sbin...
%define _exec_prefix %{nil}

%build
# This is a debatable issue. The author of this RPM spec file feels that
# people who install RPMs (especially given that the default RPM options
# will strip the binary) are not going to be running gdb against the
# program.
make CXFLAGS="$RPM_OPT_FLAGS" SYSCONFDIR="%{_sysconfdir}"

%install
make DESTDIR=$RPM_BUILD_ROOT MANDIR=%{_mandir} BINDIR=%{_sbindir} install
install -D -m644 mdadm.conf-example $RPM_BUILD_ROOT/%{_sysconfdir}/mdadm.conf

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc TODO ChangeLog mdadm.conf-example COPYING
%{_sbindir}/mdadm
%{_sbindir}/mdmon
/usr/lib/udev/rules.d/01-md-raid-creating.rules
/usr/lib/udev/rules.d/63-md-raid-arrays.rules
/usr/lib/udev/rules.d/64-md-raid-assembly.rules
/usr/lib/udev/rules.d/69-md-clustered-confirm-device.rules
%config(noreplace,missingok)/%{_sysconfdir}/mdadm.conf
%{_mandir}/man*/md*

%changelog
