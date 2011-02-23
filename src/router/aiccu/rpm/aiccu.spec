############################################################
# AICCU - Automatic IPv6 Connectivity Client Utility
# by Jeroen Massar <jeroen@sixxs.net>
# (c) Copyright 2003-2005 SixXS
############################################################
# AICCU RPM Spec File
############################################################

Summary:   AICCU - SixXS Automatic IPv6 Connectivity Client Utility
Name:      aiccu
Version:   2007.01.15
Release:   1%{?dist}
License:   BSD
Group:     System Environment/Daemons
URL:       http://www.sixxs.net/tools/aiccu/
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Source:    http://www.sixxs.net/archive/sixxs/aiccu/unix/aiccu_%{version}.tar.gz
BuildRequires: gnutls-devel
Requires:  iproute
Requires(post): chkconfig
Requires(preun): chkconfig, initscripts
Requires(postun): initscripts

%description
This client automatically gives one IPv6 connectivity
without having to manually configure interfaces etc.
One does need a SixXS account and at least a tunnel. These
can be freely & gratis requested from the SixXS website.
For more information about SixXS check http://www.sixxs.net

%prep
%setup -q -n %{name}
# fix executable permissions on non-executable content
# so debuginfo can pick them up properly
find . -type f -not -name rules -and -not -name *init* -exec chmod a-x \{\} \;

%build
make %{?_smp_mflags} RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%post
if [ "$1" = "1" ]; then
	/sbin/chkconfig --add aiccu
fi

%preun
if [ "$1" = "0" ]; then
	/sbin/service aiccu stop >/dev/null 2>&1
	/sbin/chkconfig --del aiccu
fi

%postun
/sbin/service aiccu condrestart > /dev/null 2>&1 || :

%clean
make clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc doc/README doc/LICENSE
%{_sbindir}/aiccu
# aiccu.conf contains the users's SixXS password, so don't
# make it readable by non-root
%attr(600, root,root) %config(noreplace) %{_sysconfdir}/aiccu.conf
%{_sysconfdir}/init.d/aiccu


%changelog
* Wed Jun 28 2006 Matt Domsch <matt@domsch.com> 2005.01.31-4
- export CFLAGS properly, fix permissions on files for debuginfo

* Wed Jun 28 2006 Matt Domsch <matt@domsch.com> 2005.01.31-3
- cleanups per Fedora Extras review

* Sat Apr 22 2006 Matt Domsch <matt@domsch.com> 2005.01.31-2
- match Fedora Extras spec guidelines
- add postun condrestart
- add reload initscript arg to satisfy rpmlint

* Sun Aug 29 2004 Jeroen Massar <jeroen@sixxs.net> 2004.08.29
- Beta2 with TIC, 6in4, 6in4-heartbeat and AYIYA support

