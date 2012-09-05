%define name nrpe
%define version 2.12
%define release 1
%define nsusr nagios
%define nsgrp nagios
%define nsport 5666

# Reserve option to override port setting with:
# rpm -ba|--rebuild --define 'nsport 5666'
%{?port:%define nsport %{port}}

# Macro that print mesages to syslog at package (un)install time
%define nnmmsg logger -t %{name}/rpm

Summary: Host/service/network monitoring agent for Nagios
URL: http://www.nagios.org
Name: %{name}
Version: %{version}
Release: %{release}
License: GPL
Group: Application/System
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-buildroot
Prefix: %{_prefix}
Prefix: /etc/init.d
Prefix: /etc/nagios
Requires: bash, grep, nagios-plugins
PreReq: /usr/bin/logger, chkconfig, sh-utils, shadow-utils, sed, initscripts, fileutils, mktemp

%description
Nrpe is a system daemon that will execute various Nagios plugins
locally on behalf of a remote (monitoring) host that uses the
check_nrpe plugin.  Various plugins that can be executed by the 
daemon are available at:
http://sourceforge.net/projects/nagiosplug

This package provides the core agent.

%package plugin
Group: Application/System
Summary: Provides nrpe plugin for Nagios.
Requires: nagios-plugins

%description plugin
Nrpe is a system daemon that will execute various Nagios plugins
locally on behalf of a remote (monitoring) host that uses the
check_nrpe plugin.  Various plugins that can be executed by the 
daemon are available at:
http://sourceforge.net/projects/nagiosplug

This package provides the nrpe plugin for Nagios-related applications.

%prep
%setup -q


%pre
# Create `nagios' group on the system if necessary
if grep ^nagios: /etc/group; then
	: # group already exists
else
	/usr/sbin/groupadd %{nsgrp} || %nnmmsg Unexpected error adding group "%{nsgrp}". Aborting install process.
fi

# Create `nagios' user on the system if necessary
if id %{nsusr} ; then
	: # user already exists
else
	/usr/sbin/useradd -r -d /var/log/nagios -s /bin/sh -c "%{nsusr}" -g %{nsgrp} %{nsusr} || \
		%nnmmsg Unexpected error adding user "%{nsusr}". Aborting install process.
fi

# if LSB standard /etc/init.d does not exist,
# create it as a symlink to the first match we find
if [ -d /etc/init.d -o -L /etc/init.d ]; then
  : # we're done
elif [ -d /etc/rc.d/init.d ]; then
  ln -s /etc/rc.d/init.d /etc/init.d
elif [ -d /usr/local/etc/rc.d ]; then
  ln -s  /usr/local/etc/rc.d /etc/init.d
elif [ -d /sbin/init.d ]; then
  ln -s /sbin/init.d /etc/init.d
fi


%preun
if [ "$1" = 0 ]; then
	/sbin/service nrpe stop > /dev/null 2>&1
	/sbin/chkconfig --del nrpe
fi

%postun
if [ "$1" -ge "1" ]; then
	/sbin/service nrpe condrestart >/dev/null 2>&1 || :
fi

%build
export PATH=$PATH:/usr/sbin
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" \
./configure \
	--with-init-dir=/etc/init.d \
	--with-nrpe-port=%{nsport} \
	--with-nrpe-user=%{nsusr} \
	--with-nrpe-group=%{nsgrp} \
	--prefix=%{_prefix} \
	--exec-prefix=%{_prefix}/sbin \
	--bindir=%{_prefix}/sbin \
	--sbindir=%{_prefix}/lib/nagios/cgi \
	--libexecdir=%{_prefix}/lib/nagios/plugins \
	--datadir=%{_prefix}/share/nagios \
	--sysconfdir=/etc/nagios \
	--localstatedir=/var/log/nagios \
	--enable-command-args

make all

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
install -d -m 0755 ${RPM_BUILD_ROOT}/etc/init.d
install -d -m 0755 ${RPM_BUILD_ROOT}/etc/nagios
install -d -m 0755 ${RPM_BUILD_ROOT}/usr/sbin
install -d -m 0755 ${RPM_BUILD_ROOT}/usr/lib/nagios/plugins

# install templated configuration files
cp sample-config/nrpe.cfg ${RPM_BUILD_ROOT}/etc/nagios/nrpe.cfg
cp init-script ${RPM_BUILD_ROOT}/etc/init.d/nrpe
cp src/nrpe ${RPM_BUILD_ROOT}/usr/sbin
cp src/check_nrpe ${RPM_BUILD_ROOT}/usr/lib/nagios/plugins

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(755,root,root)
/etc/init.d/nrpe
%{_prefix}/sbin/nrpe
%dir /etc/nagios
%defattr(644,root,root)
%config(noreplace) /etc/nagios/*.cfg
%defattr(755,%{nsusr},%{nsgrp})
%doc Changelog LEGAL README 

%files plugin
%defattr(755,root,root)
%{_prefix}/lib/nagios/plugins
%defattr(755,%{nsusr},%{nsgrp})
%doc Changelog LEGAL README

%changelog
* Mon Jan 23 2006 Andreas Kasenides ank<@>cs.ucy.ac.cy
- fixed nrpe.cfg relocation to sample-config
- replaced Copyright label with License
- added --enable-command-args to enable remote arg passing (if desired can be disabled by commenting out)

* Wed Nov 12 2003 Ingimar Robertsson <iar@skyrr.is>
- Added adding of nagios group if it does not exist.

* Tue Jan 07 2003 James 'Showkilr' Peterson <showkilr@showkilr.com>
- Removed the lines which removed the nagios user and group from the system
- changed the patch release version from 3 to 1

* Mon Jan 06 2003 James 'Showkilr' Peterson <showkilr@showkilr.com>
- Removed patch files required for nrpe 1.5
- Update spec file for version 1.6 (1.6-1)

* Sat Dec 28 2002 James 'Showkilr' Peterson <showkilr@showkilr.com>
- First RPM build (1.5-1)
