# $Id: proftpd.spec,v 1.73 2010/02/24 17:54:53 castaglia Exp $

# You can specify additional modules on the RPM build line by specifying
# flags like:
#
#   --with mod_tls
#
# The following modules/support can be added in this manner:
#
#   mod_tls
#   mod_sftp
#   mod_radius
#   mod_ldap
#   mod_wrap
#   mod_wrap2
#   mod_wrap2_file
#   mod_wrap2_sql
#   mod_sql
#   mod_sql_mysql
#   mod_sql_postgres
#   mod_rewrite
#   mod_ifsession
#   mod_facl
#   mod_quotatab
#   mod_quotatab_file
#   mod_quotatab_sql
#   mod_ban
#   mod_ctrls_admin
#   mod_site_misc
#   ipv6
#   ctrls
#   dso
#   lastlog
#   nls
#
# note that the following are equivalent:
#
#	--enable-nls		--with-mod_lang
#	--enable-dso		--with-mod_dso
#	--enable-ctrls		--with-mod_ctrls
#
# to turn everything on, build as:
#
# rpmbuild -ba --with ctrls --with mod_facl --with mod_tls --with nls \
#	--with ipv6 --with dso proftpd.spec
#
# If the /home directory is a network mount (e.g. NFS) which squashes
# root privileges, then rpm will fail at the install step.  To avoid this,
# use:
#
#  rpmbuild --define 'nohome 1' ...
%{!?nohome:%define nohome 0}

%define proftpd_version 1.3.3
%define usecvsversion             0%{?_with_cvs:1}
%define proftpd_cvs_version_main  1.2
%define proftpd_cvs_version_date  20070929

# define static_modules only if mod_facl has been requested
%if 0%{?_with_mod_facl:1}
%define static_modules	mod_facl
%else
%undefine static_modules
%endif

# put mod_ifsession at the end of the list (always)
%define shared_modules	%{?_with_mod_tls:mod_tls:}%{?_with_mod_sftp:mod_sftp:}mod_sql:mod_radius:mod_ban:mod_ctrls_admin:mod_load:mod_quotatab:mod_quotatab_file:mod_quotatab_ldap:mod_quotatab_radius:mod_quotatab_sql:mod_ratio:mod_readme:mod_rewrite:mod_site_misc:mod_wrap2:mod_wrap2_file:mod_wrap2_sql%{?_with_nls::mod_lang}:mod_ifsession

%define unbundled_modules mod_ldap:mod_sql_mysql:mod_sql_postgres:mod_wrap

%if ! %([ -n '%{shared_modules}' -o -n '%{unbundled_modules}' ] ; echo $?)
%define _with_dso 	1
%endif

# avoid confusion and redundancy
%if 0%{?_with_mod_dso:1}
%define _with_dso	1
%undefine _with_mod_dso
%endif

# we don't actually pass this list into configure... we just use it to
# generate the manifest of modules for the %description section of the
# resulting package.
%define builtin_modules	mod_core:mod_xfer:mod_auth_unix:mod_auth_file:mod_auth:mod_ls:mod_log:mod_site:mod_delay%{?_with_dso::mod_dso}:mod_cap:mod_auth_pam%{?_with_ctrls::mod_ctrls}

Summary:		ProFTPD -- Professional FTP Server.
Name:			proftpd
Release:		1%{?dist}
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
Prefix:			%{_prefix}
BuildRoot:		%{_tmppath}/%{name}-%{version}-root
Requires:		pam >= 0.99, /sbin/chkconfig
BuildRequires:		pkgconfig, pam-devel, ncurses-devel, zlib-devel
BuildRequires:		rpm-build >= 4.2
# For mod_sftp:
%if 0%{?_with_mod_sftp:1}
%define _with_openssl   1
Requires:               zlib
BuildRequires:          zlib-devel
%endif
# For mod_tls:
%if 0%{?_with_mod_tls:1}
%define _with_openssl	1
Requires:		krb5-libs, zlib
BuildRequires:		krb5-devel, zlib-devel
%endif
%if 0%{?_with_openssl:1}
Requires:		openssl
BuildRequires:		openssl-devel
%endif
%if 0%{?_with_mod_facl:1}
# For mod_facl
Requires:		libacl
BuildRequires:		libacl-devel
%endif
# For mod_cap
Requires:		libcap
BuildRequires:		libcap-devel
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

Likewise, mod_sql_mysql, mod_ldap, etc. can be installed separately to avoid
unnecessary dependencies.

%{?_with_dso:This package supports DSO (shared) modules.}

Intrinsic static modules: %(echo "%{builtin_modules}" | sed -e 's/\:/, /g')

Optional static modules: %(echo "%{static_modules}" | sed -e 's/\:/, /g')

Bundled shared modules: %(echo "%{shared_modules}" | sed -e 's/\:/, /g')

Unbundled (optional) shared modules: %(echo "%{unbundled_modules}" | sed -e 's/\:/, /g')

%package inetd
Summary:	ProFTPD -- Setup for inetd/xinetd operation.
Group:		System Environment/Daemons
Requires:	proftpd, inetd
Obsoletes:	proftpd-standalone

%description inetd
This package is neccesary to setup ProFTPD to run from inetd/xinetd.

%package ldap
Summary:	ProFTPD -- Modules relying on LDAP.
Group:		Development/Libraries
Requires:	proftpd, openldap, krb5-libs, openssl
BuildRequires:	openldap-devel, krb5-devel, openssl-devel

%description ldap 
This optional package contains the modules using LDAP.

%package mysql
Summary:	ProFTPD -- Modules relying on SQL.
Group:		Development/Libraries
Requires:	proftpd, mysql, krb5-libs
BuildRequires:	mysql-devel, krb5-devel

%description mysql
This optional package contains the modules using MySQL.

%package postgres
Summary:	ProFTPD -- Modules relying on Postgres.
Group:		Development/Libraries
Requires:	proftpd, postgresql, krb5-libs, openssl
BuildRequires:	postgresql-devel, krb5-devel, openssl-devel

%description postgres
This optional package contains the modules using Postgres.

%package wrap
Summary:	ProFTPD -- Modules relying on TCP Wrappers.
Group:		Development/Libraries
Requires:	proftpd, tcp_wrappers
BuildRequires:	tcp_wrappers

%description wrap
This optional package contains the modules using tcpwrappers/libwrap.

%package devel
Summary:	ProFTPD -- Header files for developers.
Group:		Development/Libraries
Requires:	proftpd

%description devel
This package is required to develop additional modules for ProFTPD.

%prep
%if %{usecvsversion}
%setup -q -n %{name}-%{proftpd_cvs_version_main}
%else
%setup -q
%endif
CFLAGS="$RPM_OPT_FLAGS" ./configure \
	--prefix=%{prefix} \
	--sysconfdir=%{_sysconfdir} \
	--localstatedir=%{_localstatedir}/run \
	--mandir=%{_mandir} \
	%{?_with_ipv6:--enable-ipv6} \
	%{?_with_ctrls:--enable-ctrls} \
	%{?_with_dso:--enable-dso} \
	%{?_with_lastlog:--with-lastlog} \
	%{?_with_openssl:--enable-openssl} \
	--with-libraries=%{_libdir}/mysql \
	--with-includes=%{_includedir}/mysql \
	%{?static_modules:--with-modules=%{static_modules}} \
	%{?shared_modules:--with-shared=%{?unbundled_modules:%{unbundled_modules}:}%{shared_modules}}

%build
  make

%install
  rm -rf $RPM_BUILD_ROOT
  make DESTDIR=$RPM_BUILD_ROOT \
	libdir=%{_libdir} \
	prefix=%{prefix} \
	exec_prefix=%{_exec_prefix} \
	sysconfdir=%{_sysconfdir} \
	mandir=%{_mandir} \
	localstatedir=%{_localstatedir}/run \
	rundir=%{_localstatedir}/run \
	INSTALL_USER=`id -un` INSTALL_GROUP=`id -gn` \
    install
%if !%{nohome}
  mkdir -p $RPM_BUILD_ROOT/home/ftp
%endif
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
%{_sbindir}/*
%{_bindir}/*
%if 0%{?_with_mod_sftp:1}
%{_sysconfdir}/blacklist.dat
%{_sysconfdir}/dhparams.pem
%endif
# need to figure out how to exclude from this list...
%exclude %{_libexecdir}/*.a
%exclude %{_libexecdir}/*.la
%exclude %{_libexecdir}/mod_ldap.so
%exclude %{_libexecdir}/mod_sql_mysql.so
%exclude %{_libexecdir}/mod_sql_postgres.so
%exclude %{_libexecdir}/mod_wrap.so
%{_libdir}/pkgconfig/*.pc
%{_libexecdir}/*.a
%{_libexecdir}/*.so
%dir %{_localstatedir}/run
%if !%{nohome}
%dir /home/ftp
%endif
%{_initrddir}/proftpd
%config(noreplace) %{_sysconfdir}/proftpd.conf
%config(noreplace) %{_sysconfdir}/pam.d/ftp
%config(noreplace) %{_sysconfdir}/logrotate.d/proftpd

%doc COPYING CREDITS ChangeLog NEWS
%doc README* doc/*
%doc contrib/README* contrib/xferstats.holger-preiss
%doc sample-configurations
%_mandir/*/*

%files inetd
%defattr(-,root,root)
%config(noreplace) %{_sysconfdir}/proftpd.conf
%config(noreplace) %{_sysconfdir}/xinetd.d/proftpd

%files ldap
%defattr(-,root,root)
%{_libexecdir}/mod_ldap.so

%files mysql
%defattr(-,root,root)
%{_libexecdir}/mod_sql_mysql.so

%files postgres
%defattr(-,root,root)
%{_libexecdir}/mod_sql_postgres.so

%files wrap
%defattr(-,root,root)
%{_libexecdir}/mod_wrap.so

%files devel
%defattr(-,root,root)
%{_includedir}/proftpd/*.h

%changelog
* Mon Sep 11 2007 Philip Prindeville <philipp_subx@redfish-solutions.com>
- Cleaned up the .spec file to work with more recent releases of RPM.  Moved
  header files into separate component.

* Sun Mar 5 2006 Itamar Reis Peixoto <itamar@ispbrasil.com.br>
- Added "--with mod_quotatab" and "--with mod_quotatab_sql" to enable Quota Support while building the RPM

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
