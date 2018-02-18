# The olsr.org Optimized Link-State Routing daemon (olsrd)
#
# (c) by the OLSR project
#
# See our Git repository to find out who worked on this file
# and thus is a copyright holder on it.
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in
#   the documentation and/or other materials provided with the
#   distribution.
# * Neither the name of olsr.org, olsrd nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Visit http://www.olsr.org for more information.
#
# If you find this software useful feel free to make a donation
# to the project. For more information see the website or contact
# the copyright holders.
#

Summary:   OLSR Daemon
Name:      olsrd
Version:   current
Release:   1
License:   BSD
Packager:  roarbr@tihlde.org
Group:     System Environment/Daemons
Source:    http://www.olsr.org/releases/0.5/olsrd-%{version}.tar.bz2
URL:       http://www.olsr.org/
BuildRoot: %{_tmppath}/%{name}-root
Provides:  olsrd

%description
olsrd is an implementation of the Ad Hoc routing protocol OLSR (RFC3626).
OLSRD provides (multihop) routing in a dynamic, changing Ad Hoc network,
wired or wireless.
This version supports both IPv4 and IPv6.
See http://www.olsr.org/ for more info.

%prep
%setup


%{__cat} << 'EOF' > %{name}.init
#!/bin/bash
#
# Startup script for the OLSR Daemon
#
# chkconfig: 235 16 84
# description: This script starts OLSRD (Ad Hoc routing protocol)
#
# processname: olsrd
# config: %{_sysconfdir}/olsrd/olsrd.conf
# pidfile: %{_localstatedir}/run/olsrd.pid

source %{_initrddir}/functions
source %{_sysconfdir}/sysconfig/network

# Check that networking is up.
[ ${NETWORKING} = "no" ] && exit 0

[ -x %{_sbindir}/olsrd ] || exit 1
[ -r %{_sysconfdir}/olsrd/olsrd.conf ] || exit 1

RETVAL=0
prog="olsrd"
desc="Ad Hoc routing protocol"

start() {
        echo -n $"Starting $desc ($prog): "
	daemon $prog -d 0 
        RETVAL=$?
        echo
        [ $RETVAL -eq 0 ] && touch %{_localstatedir}/lock/subsys/$prog
        return $RETVAL
}

stop() {
        echo -n $"Shutting down $desc ($prog): "
        killproc $prog
        RETVAL=$?
        echo
        [ $RETVAL -eq 0 ] && rm -f %{_localstatedir}/lock/subsys/$prog
        return $RETVAL
}

reload() {
        echo -n $"Reloading $desc ($prog): "
        killproc $prog -HUP
        RETVAL=$?
        echo
        return $RETVAL
}

restart() {
        stop
        start
}

case "$1" in
  start)
        start
        ;;
  stop)
        stop
        ;;
  restart)
        restart
        ;;
  reload)
        reload
        ;;
  condrestart)
        [ -e %{_localstatedir}/lock/subsys/$prog ] && restart
        RETVAL=$?
        ;;
  status)
	status olsrd
	;;
  *)
        echo $"Usage $0 {start|stop|restart|reload|condrestart|status}"
        RETVAL=1
esac

exit $RETVAL
EOF


%build
make %{?_smp_mflags}
make %{?_smp_mflags} libs

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/sbin/
mkdir -p %{buildroot}/usr/lib/
mkdir -p %{buildroot}%{_initrddir}
mkdir -p %{buildroot}/usr/share/man/man8
make DESTDIR=%{buildroot} install
make DESTDIR=%{buildroot} install_libs
%{__install} -m0755 olsrd.init %{buildroot}%{_initrddir}/olsrd


%clean
rm -rf %{buildroot}

%preun
/etc/init.d/olsrd stop
/sbin/chkconfig --del olsrd

%post
#/sbin/chkconfig --add olsrd
# Default to not start olsrd automatic
/sbin/chkconfig olsrd off
echo "Now please edit /etc/olsrd/olsrd.conf and run 'service olsrd start' or '/etc/init.d/olsrd start' to start olsrd"
echo "Run 'chkconfig olsrd on' to enable automatic starting of olsrd"

%files
%defattr(-, root, root, 0755)
%doc README CHANGELOG
%doc lib/*/*README*

%config(noreplace) %{_sysconfdir}/olsrd/olsrd.conf
%config %{_initrddir}/olsrd
/usr/sbin/olsrd
# Wildchar to cover all installed plugins
/usr/lib/olsrd_*so*
/usr/share/man/man8/olsrd.8.gz
/usr/share/man/man5/olsrd.conf.5.gz

%changelog
* Tue Jul 17 2007 Roar Bjørgum Rotvik <roarbr@tihlde.org>
- Created spec-file for 0.5.2

* Mon Jul 09 2007 Roar Bjørgum Rotvik <roarbr@tihlde.org>
- Created spec-file for 0.5.1

* Tue Apr 03 2007 Roar Bjørgum Rotvik <roarbr@tihlde.org>
- Created spec-file for 0.5.0
- Changed from INSTALL_PREFIX to DESTDIR

* Wed Jan 04 2006 Roar Bjørgum Rotvik <roarbr@tihlde.org>
- Created spec-file for 0.4.10
- Removed OS=linux option to make
- Updated plugin file list, added wildchar for plugins

* Tue Apr 05 2005 Roar Bjørgum Rotvik <roarbr@tihlde.org>
- Created spec-file for 0.4.9

* Tue Mar 29 2005 Roar Bjørgum Rotvik <roarbr@tihlde.org>
- Increased version number for nameservice and secure plugin

* Tue Dec 07 2004 Roar Bjørgum Rotvik <roarbr@tihlde.org>
- Changed spec file for olsrd-0.4.8
- Removed frontend GUI inclusion
- Removed references to Unik
- Changed licence to BSD

* Tue Jun 29 2004 Roar Bjørgum Rotvik <roarbr@tihlde.org>
- Changed spec file for unik-olsrd-0.4.5
- Remover ROOT-prefix patch as INSTALL_PREFIX is added to Makefile in 0.4.5
- Added INSTALL_PREFIX patch for front-end/Makefile
- Included plugins dot_draw and secure
- Added documentation for the plugins dyn_gw, powerinfo, dot_draw and secure

* Tue May 25 2004 Roar Bjørgum Rotvik <roarbr@tihlde.org>
- Changed spec file for unik-olsrd-0.4.4
- Added man-page for olsrd
- Removed documentation olsrd-plugin-howto.pdf as it is no longer part of source package

* Tue Mar 02 2004 Roar Bjørgum Rotvik <roarbr@tihlde.org>
- Changed spec file for unik-olsrd-0.4.3
- Added OLSRD plugins olsrd_dyn_gw and olsrd_power to package
- Added documentation olsrd-plugin-howto.pdf

* Tue Mar 02 2004 Roar Bjørgum Rotvik <roarbr@tihlde.org>
- Renamed package from uolsrd to unik-olsrd to use the same name as the .deb-package
- Start olsrd daemon with option "-d 0" to start without debugging and in daemon mode
  even if debugging is enabled in olsrd.conf.

* Mon Mar 01 2004 Roar Bjørgum Rotvik <roarbr@tihlde.org>
- Included init-script to start uolsrd daemon (installs as %{_initrddir}/uolsrd).

* Wed Feb 25 2004 Roar Bjørgum Rotvik <roarbr@tihlde.org>
- Changed Group from Applications/System to System Environment/Daemons.
- Included olsrd-gui (forgotten in first release)
- Renamed spec file from unik-olsrd-0.4.0.spec to uolsrd-0.4.0.spec

* Wed Feb 25 2004 Roar Bjørgum Rotvik <roarbr@tihlde.org>
- Created first version of this spec-file

