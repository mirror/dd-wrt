## --*- rpm-spec -*--

# Copyright (C) 2004, 2008, 2014
#               Enrico Scholz <enrico.scholz@ensc.de>
#  
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 3 of the License.
#  
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#  
# You should have received a copy of the GNU General Public License
# along with this program. If not, see http://www.gnu.org/licenses/.


## This package understands the following switches:
## --without dietlibc   ...   disable usage of dietlibc
## --without minit      ...   disable creation of 'minit' subpackage
## --with    fedora     ...   enable fedora.us specific parts


%define uid		11
%define useradd		/usr/sbin/%{?_with_fedora:fedora-}useradd  %{?_with_fedora:%uid}
%define groupadd	/usr/sbin/%{?_with_fedora:fedora-}groupadd %{?_with_fedora:%uid}
%define userdel		/usr/sbin/%{?_with_fedora:fedora-}userdel
%define groupdel	/usr/sbin/%{?_with_fedora:fedora-}groupdel


%define	username	dhcp-fwd
%define homedir		%_var/lib/%username
%define minitdir	%_sysconfdir/minit
%define minitsvcdir	%minitdir/services/dhcp-fwd
%define service		dhcp-fwd

%{!?release_func:%define release_func() %1}

Summary:	A DHCP relay agent
Name:		dhcp-forwarder
Version:	0.11
Release:	%release_func 0
Epoch:		0
License:	GPLv3
Group:		System Environment/Daemons
URL:		http://www.tu-chemnitz.de/~ensc/dhcp-fwd
Source0:	http://www.tu-chemnitz.de/~ensc/dhcp-fwd/files/%name-%version.tar.bz2
BuildRoot:	%_tmppath/%name-%version-%release-root
Requires:		init(dhcp-forwarder)

%{!?_without_dietlibc:BuildRequires:	dietlibc}

## The Requires(...) which depend on the '--with fedora' switch
%{!?_with_fedora:Requires(pre):		/usr/sbin/useradd /usr/sbin/groupadd}
%{!?_with_fedora:Requires(postun):	/usr/sbin/userdel /usr/sbin/groupdel}
%{?_with_fedora:Requires(pre):		fedora-usermgmt}
%{?_with_fedora:Requires(postun):	fedora-usermgmt}


%package sysv
Summary:		SysV initscripts for dhcp-forwarder
Group:			System Environment/Base
Provides:		init(dhcp-forwarder) = sysv
Requires:		%name = %epoch:%version-%release
Requires(preun):	%name initscripts
Requires(postun):	%name initscripts
Requires(post):		/sbin/chkconfig
Requires(preun):	/sbin/chkconfig


%package minit
Summary:		minit initscripts for dhcp-forwarder
Group:			System Environment/Base
Provides:		init(dhcp-forwarder) = minit
Requires:		%name = %epoch:%version-%release
Requires(pre):		minit-setup
Requires(postun):	minit-setup



%description
dhcp-fwd forwards DHCP messages between subnets with different sublayer
broadcast domains. It is similar to the DHCP relay agent dhcrelay of
ISC's DHCP, but has the following important features:

* Runs as non-root in a chroot-environment
* Uses AF_INET sockets which makes it possible to filter incoming
  messages with packetfilters
* The DHCP agent IDs can be defined freely
* Has a small memory footprint when using dietlibc


%description sysv
dhcp-fwd forwards DHCP messages between subnets with different sublayer
broadcast domains.

This package provides the scripts which can be used to start dhcp-forwarder
with the SysV initconcept.


%description minit
dhcp-fwd forwards DHCP messages between subnets with different sublayer
broadcast domains.

This package provides the scripts which can be used to start dhcp-forwarder
with the minit initconcept.


##---------------------------------------------


%prep
%setup -q


##---------------------------------------------


%build
%configure \
	--enable-release \
        --with-initrddir=%_initrddir \
	%{?_without_dietlibc:--disable-dietlibc}
%__make %{?_smp_mflags}


##---------------------------------------------


%install
rm -rf $RPM_BUILD_ROOT

%__make DESTDIR=$RPM_BUILD_ROOT install install-contrib
%__install -d -m700 $RPM_BUILD_ROOT%homedir

%{?_without_minit:rm -rf $RPM_BUILD_ROOT%minitsvcdir}


##---------------------------------------------


%check
%__make check


##---------------------------------------------


%clean
rm -rf $RPM_BUILD_ROOT


%pre
%groupadd -r %username &>/dev/null || :
%useradd  -r -s /sbin/nologin -M -c 'DHCP Forwarder user'	\
	  -d %homedir -g %username %username &>/dev/null || :


%postun
if test "$1" = "0"; then
	%userdel  %username &>/dev/null || :
	%groupdel %username &>/dev/null || :
fi


%post sysv
/sbin/chkconfig --add %service

%preun sysv
if test "$1" = "0"; then
	%_initrddir/%service stop >/dev/null
	/sbin/chkconfig --del %service
fi

%postun sysv
test "$1" = 0 || %_initrddir/%service condrestart &>/dev/null


%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING ChangeLog NEWS README THANKS
%_mandir/*/*
%config(noreplace) %_sysconfdir/dhcp-fwd.conf
%_sbindir/*

## *DO NOT* add %defattr(-,dhcp-fwd,dhcp-fwd); the homedir is used for
## the chroot() only and there is no reason why the setuid()'ed daemon
## needs any kind of access there
%homedir


%files sysv
%defattr(-,root,root,-)
%config %_initrddir/*
%config(noreplace) %_sysconfdir/sysconfig/*


%if 0%{!?_without_minit:1}

%files minit
%defattr(-,root,root,-)
%dir %minitsvcdir
%minitsvcdir/run
%minitsvcdir/respawn
%config(noreplace) %minitsvcdir/params

%endif


%changelog
* Thu Aug 19 2004 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de> - 0:0.6.1-0
- added support for 'fedora-usermgmt' (enabled with '--with fedora' switch)

* Thu Jun 17 2004 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de> - 0:0.6.1-0
- conditionalized build of -minit subpackage

* Thu Aug  7 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de> 0:0.5.1-0
- added minit support
- removed superfluous %%doc attribute of %%_mandir entries
- removed superfluous curlies
- use 'install-contrib' and cleaned up %%install section
- moved /etc/sysconfig/* file into -sysv subpackage; it is not used by
  -minit anymore
- minor cleanups

* Wed Jul 30 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de> 0:0.5-0.fdr.1
- updated to version 0.5

* Tue May 27 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de> 0:0.4-0.fdr.2
- create and remove group explicitely
- s/adduser/useradd/
- removed dependency on initscripts by calling the service-script in
  the %%post/%%preun scriptlets directly
- do not call '--install-contrib' anymore; it creates too much
  clutter to make sure that the initscripts will be installed into
  %%_initrddir but not in /etc/init.d. Instead of, install the
  scripts manually.

* Fri May  2 2003 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de> 0:0.4-0.fdr.1
- cleanups
- applied fedora.us naming scheme

* Wed Aug 28 2002 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de> 0.3.1-1
- Added /etc/sysconfig/dhcp-fwd file

* Fri Jul 12 2002 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de> 0.2.5-2
- Renamed username from dhcpfwd to dhcp-fwd
- Adjusted URL

* Fri Jul 12 2002 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de> 0.2.5-1
- version 0.2.5
- Fixed some typos
- Added some PreReq's
- Enhanced %postun script

* Mon Jun 17 2002 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de> 0.2-2
- Described purpose of the %%homedir and its handling

* Fri Jun 14 2002 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de> 0.2-1
- Added manpage

* Thu Jun 13 2002 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de> 0.1-0.3
- Added --without dietlibc option

* Sat Jun  1 2002 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de> 
- Initial build.
