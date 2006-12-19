# $Id: proftpd.spec,v 1.1 2006/04/24 11:39:27 honor Exp $

# You can specify additional modules on the RPM build line by specifying
# flags like:
#
#   --with mod_tls
#
# The following modules/support can be added in this manner:
#
#   mod_tls
#   mod_radius
#   mod_ldap
#   mod_wrap
#   mod_sql
#   mod_sql_mysql
#   mod_sql_postgres
#   mod_rewrite
#   mod_ifsession
#   ipv6

%define proftpd_version 1.3.0rc2
%define usecvsversion             0
%define proftpd_cvs_version_main  1.2
%define proftpd_cvs_version_date  20031009

Summary:		ProFTPD -- Professional FTP Server.
Name:			proftpd
Release:		1
License:		GPL
Group:			System Environment/Daemons
Packager:		The ProFTPD Project <core@proftpd.org>
Vendor:			The ProFTPD Project
URL:			http://www.proftpd.org/
%if %{usecvsversion}
Source:			ftp://ftp.proftpd.org/devel/source/proftpd-%{proftpd_cvs_version_main}-cvs-%{proftpd_cvs_version_date}.tgz
Version:		%{proftpd_cvs_version_main}cvs%{proftpd_cvs_version_date}
%else
Source:			ftp://ftp.proftpd.org/distrib/%{name}-%{version}.tar.bz2
Version:		%{proftpd_version}
%endif
Prefix:			/usr
BuildRoot:		%{_builddir}/%{name}-%{version}-root
Requires:		pam >= 0.72, /sbin/chkconfig, %{?_with_mod_tls:openssl krb5-libs}
BuildPreReq:	pam-devel %{?_with_mod_tls:openssl-devel krb5-devel}
Provides:		ftpserver
Prereq:			fileutils
Obsoletes:		proftpd-core, proftpd-standalone

%description
ProFTPD is an enhanced FTP server with a focus toward simplicity, security,
and ease of configuration.  It features a very Apache-like configuration
syntax, and a highly customizable server infrastructure, including support for
multiple 'virtual' FTP servers, anonymous FTP, and permission-based directory
visibility.

The base proftpd package installs standalone support. You can install the
proftpd-inetd package to enable inetd/xinetd support.

%{?_with_ipv6: This package is IPv6 enabled.}
Addtional modules enabled: mod_ratio mod_readme %{?_with_mod_tls: mod_tls}%{?_with_mod_radius: mod_radius}%{?_with_mod_ldap: mod_ldap}%{?_with_mod_wrap: mod_wrap}%{?_with_mod_sql: mod_sql}%{?_with_mod_sql_mysql: mod_sql_mysql}%{?_with_mod_sql_postgres: mod_sql_postgres}%{?_with_mod_rewrite: mod_rewrite}%{?_with_mod_ifsession: mod_ifsession}

%package inetd
Summary:	ProFTPD -- Setup for inetd/xinetd operation.
Group:		System Environment/Daemons
Requires:	proftpd, inetd
Obsoletes:	proftpd-standalone

%description inetd
This package is neccesary to setup ProFTPD to run from inetd/xinetd.

%prep
%if %{usecvsversion}
%setup -q -n %{name}-%{proftpd_cvs_version_main}
%else
%setup -q
%endif
  MODULES="mod_ratio:mod_readme"
  MODULES="${MODULES}%{?_with_mod_tls::mod_tls}"
  MODULES="${MODULES}%{?_with_mod_radius::mod_radius}"
  MODULES="${MODULES}%{?_with_mod_ldap::mod_ldap}"
  MODULES="${MODULES}%{?_with_mod_wrap::mod_wrap}"
  MODULES="${MODULES}%{?_with_mod_sql::mod_sql}"
  MODULES="${MODULES}%{?_with_mod_sql_mysql::mod_sql_mysql}"
  MODULES="${MODULES}%{?_with_mod_sql_postgres::mod_sql_postgres}"
  MODULES="${MODULES}%{?_with_mod_rewrite::mod_rewrite}"
  MODULES="${MODULES}%{?_with_mod_ifsession::mod_ifsession}"
  CFLAGS="$RPM_OPT_FLAGS" ./configure \
	--prefix=%{prefix} \
	--sysconfdir=/etc \
	--localstatedir=/var/run \
	--mandir=%_mandir \
	%{?_with_mod_tls:--with-includes=/usr/kerberos/include} \
	%{?_with_mod_sql_mysql:--with-includes=/usr/include/mysql} \
	%{?_with_ipv6:--enable-ipv6} \
	--with-modules=${MODULES}

%build
  make

%install
  rm -rf $RPM_BUILD_ROOT
  make prefix=$RPM_BUILD_ROOT%{prefix} \
	exec_prefix=$RPM_BUILD_ROOT%{prefix} \
	sysconfdir=$RPM_BUILD_ROOT/etc \
    mandir=$RPM_BUILD_ROOT/%_mandir \
	localstatedir=$RPM_BUILD_ROOT/var/run \
	rundir=$RPM_BUILD_ROOT/var/run/proftpd \
	INSTALL_USER=`id -un` INSTALL_GROUP=`id -gn` \
    install
  mkdir -p $RPM_BUILD_ROOT/home/ftp
  mkdir -p $RPM_BUILD_ROOT/etc/pam.d
  install -m 644 contrib/dist/rpm/ftp.pamd $RPM_BUILD_ROOT/etc/pam.d/ftp
  install -m 644 sample-configurations/basic.conf $RPM_BUILD_ROOT/etc/proftpd.conf
  mkdir -p $RPM_BUILD_ROOT/etc/rc.d/init.d
  sed -e '/FTPSHUT=/c\' \
	  -e 'FTPSHUT=%{prefix}/sbin/ftpshut' \
	contrib/dist/rpm/proftpd.init.d >contrib/dist/rpm/proftpd.init.d.tmp
  mv --force contrib/dist/rpm/proftpd.init.d.tmp contrib/dist/rpm/proftpd.init.d
  install -m 755 contrib/dist/rpm/proftpd.init.d $RPM_BUILD_ROOT/etc/rc.d/init.d/proftpd
  mkdir -p $RPM_BUILD_ROOT/etc/logrotate.d/
  install -m 644 contrib/dist/rpm/proftpd.logrotate $RPM_BUILD_ROOT/etc/logrotate.d/proftpd
  mkdir -p $RPM_BUILD_ROOT/etc/xinetd.d/
  install -m 644 contrib/dist/rpm/xinetd $RPM_BUILD_ROOT/etc/xinetd.d/proftpd
  # We don't want this dangling symlinks to make it into the RPM
  rm -f contrib/README.mod_sql

%pre
  if [ ! -f /etc/ftpusers ]; then
  	touch /etc/ftpusers
  	IFS=":"
	while { read username nu nu gid nu; }; do
		if [ $gid -le 100 -a "$username" != "ftp" ]; then
			echo $username
		fi
  	done </etc/passwd >/etc/ftpusers
  fi

%preun
  if [ "$1" = 0 ]; then
    if [ -d /var/run/proftpd ]; then
		rm -rf /var/run/proftpd/*
    fi
    /sbin/chkconfig --del proftpd
  fi

%post
  /sbin/chkconfig --add proftpd
  if [ $1 = 1 ]; then
    # Force the "ServerType" directive for this operation type.
    tmpfile=/tmp/proftpd-conf.$$
    sed -e '/ServerType/c\' \
      -e 'ServerType	standalone' \
      /etc/proftpd.conf >$tmpfile
    mv $tmpfile /etc/proftpd.conf
  fi

%post inetd
  # Force the "ServerType" directive for this operation type.
  tmpfile=/tmp/proftpd-conf.$$
  sed	-e '/ServerType/c\' \
	-e 'ServerType	inetd' \
	/etc/proftpd.conf >$tmpfile
  mv $tmpfile /etc/proftpd.conf

  if [ -f /etc/inetd.conf ]; then
    # Look if there is already an entry for 'ftp' service even when commented.
    grep '^[#[:space:]]*ftp' /etc/inetd.conf >/dev/null
    errcode=$?
    if [ $errcode -eq 0 ]; then
      # Found, replace the 'in.ftpd' with 'in.proftpd'
      tmpfile=/tmp/proftpd-inetd.$$
      sed -e '/^[#[:space:]]*ftp/{' \
          -e 's^in.ftpd.*$^in.proftpd^' \
          -e '}' \
          /etc/inetd.conf >$tmpfile
      mv $tmpfile /etc/inetd.conf
    else
    # Not found, append a new entry.
      echo 'ftp      stream  tcp     nowait  root    /usr/sbin/tcpd  in.proftpd' >>/etc/inetd.conf
    fi
    # Reread 'inetd.conf' file.
    killall -HUP inetd || :
  else
    killall -HUP xinetd || :
  fi

%postun inetd
  if [ "$1" = 0 ]; then
	# Return the ServerType to standalone when the inetd subpackage is
    # uninstalled.
    tmpfile=/tmp/proftpd-conf.$$
    sed -e '/ServerType/c\' \
      -e 'ServerType	standalone' \
      /etc/proftpd.conf >$tmpfile
    mv $tmpfile /etc/proftpd.conf

    if [ -f /etc/inetd.conf ]; then
      # Remove ProFTPD entry from /etc/inetd.conf
      tmpfile=/tmp/proftpd-inetd.$$
      sed -e '/^.*proftpd.*$/d' /etc/inetd.conf >$tmpfile
      mv $tmpfile /etc/inetd.conf
      killall -HUP inetd || :
    fi
  fi

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf %{_builddir}/%{name}-%{version}

%files
%defattr(-,root,root)
%{prefix}/sbin/*
%{prefix}/bin/*
%dir /var/run/proftpd
%dir /home/ftp
/etc/rc.d/init.d/proftpd
%config(noreplace) /etc/proftpd.conf
%config(noreplace) /etc/pam.d/ftp
%config(noreplace) /etc/logrotate.d/proftpd

%doc COPYING CREDITS ChangeLog NEWS
%doc README* doc/*
%doc contrib/README* contrib/xferstats.holger-preiss
%doc sample-configurations
%_mandir/*/*

%files inetd
%defattr(-,root,root)
%config(noreplace) /etc/proftpd.conf
%config(noreplace) /etc/xinetd.d/proftpd

%changelog
* Sun Nov  2 2003 John Morrissey <jwm@horde.net>
- This changelog is not frequently updated - please check the CVS revision
  history at http://cvs.proftpd.org/ instead.

* Tue Sep 23 2003 Daniel Roesen <dr@proftpd.org>
- Added "--with ipv6" to enable IPv6 support while building the RPM

* Sun May 25 2003 John Morrissey <jwm@horde.net>
- Permit selection of additional contrib modules when building the RPM
  Submitted by: Ivan F. Martinez <ivanfm@users.sourceforge.net>

* Sat Nov  2 2002 John Morrissey <jwm@horde.net>
- Don't let dangling contrib/README.* symlinks get into the built RPM
- logrotate for xferlog

* Wed Aug 14 2002 John Morrissey <jwm@horde.net>
- Added removal of build leftover directory in %clean.
  Submitted by: Christian Pelealu <kurisu@mweb.co.id>

* Wed Jul  3 2002 John Morrissey <jwm@horde.net> 1.2.6rc1-1
- 1.2.6rc1 release.

* Sun Jun  9 2002 John Morrissey <jwm@horde.net> 1.2.5-1
- 1.2.5 release.

* Fri May 10 2002 TJ Saunders <tj@castaglia.org>
- Added use of %defattr to allow build of RPMs by non-root users
  For details see http://bugs.proftpd.org/show_bug.cgi?id=1580

* Mon Mar 05 2001 Daniel Roesen <droesen@entire-systems.com>
- PAM >= 0.72 is now a requirement. Versions before are broken and
  Red Hat provides a PAM update for all RH 6.x releases. See:
  http://www.redhat.com/support/errata/RHSA-2000-120.html
  Thanks to O.Elliyasa <osman@Cable.EU.org> for the suggestion.
  For details see http://bugs.proftpd.org/show_bug.cgi?id=1048
- release: 1.2.1-2

* Wed Mar 01 2001 Daniel Roesen <droesen@entire-systems.com>
- Update to 1.2.1
- release: 1.2.1-1

* Wed Feb 27 2001 Daniel Roesen <droesen@entire-systems.com>
- added "Obsoletes: proftpd-core" to make migration to new RPMs easier.
  Thanks to Sébastien Prud'homme <prudhomme@easy-flying.com> for the hint.
- release: 1.2.0-3

* Wed Feb 26 2001 Daniel Roesen <droesen@entire-systems.com>
- cleaned up .spec formatting (cosmetics)
- fixed CFLAGS (fixes /etc/shadow support)
- included COPYING, CREDITS, ChangeLog and NEWS
- Renamed main package from "proftpd-core" to just "proftpd"
- release: 1.2.0-2

* Wed Feb 14 2001 Daniel Roesen <droesen@entire-systems.com>
- moved Changelog to bottom
- fixed %pre script /etc/ftpusers generator
- removed /ftp/ftpusers from package management. Deinstalling ProFTPD
  should _not_ result in removal of this file.

* Thu Oct 03 1999 O.Elliyasa <osman@Cable.EU.org>
- Multi package creation.
  Created core, standalone, inetd (&doc) package creations.
  Added startup script for init.d
  Need to make the "standalone & inetd" packages being created as "noarch"
- Added URL.
- Added prefix to make the package relocatable.

* Wed Sep 08 1999 O.Elliyasa <osman@Cable.EU.org>
- Corrected inetd.conf line addition/change logic.

* Sat Jul 24 1999 MacGyver <macgyver@tos.net>
- Initial import of spec.
