%{!?python_sitearch: %global python_sitearch %(%{__python} -c "from distutils.sysconfig import get_python_lib; print get_python_lib(1)")}

Name: gpsd
Version: 3.9~dev
Release: 1%{?dist}
Summary: Service daemon for mediating access to a GPS

Group: System Environment/Daemons
License: BSD
URL: https://savannah.nongnu.org/projects/gpsd/
Source0: http://download-mirror.savannah.gnu.org/releases/gpsd//%{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: bluez-libs-devel
BuildRequires: chrpath
BuildRequires: dbus-devel
BuildRequires: dbus-glib-devel
BuildRequires: desktop-file-utils
BuildRequires: gcc-c++
BuildRequires: libXaw-devel
BuildRequires: ncurses-devel
BuildRequires: python-devel
BuildRequires: qt-devel
BuildRequires: scons
BuildRequires: xmlto
%if 0%{?fedora} >= 16
BuildRequires: pps-tools-devel
%endif

Requires: udev
Requires(post): /sbin/ldconfig
Requires(post): /sbin/chkconfig
Requires(postun): /sbin/ldconfig
Requires(preun): initscripts
Requires(preun): /sbin/chkconfig

%description 
gpsd is a service daemon that mediates access to a GPS sensor
connected to the host computer by serial or USB interface, making its
data on the location/course/velocity of the sensor available to be
queried on TCP port 2947 of the host computer.  With gpsd, multiple
GPS client applications (such as navigational and wardriving software)
can share access to a GPS without contention or loss of data.  Also,
gpsd responds to queries with a format that is substantially easier to
parse than NMEA 0183.  

%package devel
Summary: Client libraries in C and Python for talking to a running gpsd or GPS
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
Requires: pkgconfig

%description devel
This package provides C header files and python modules for the gpsd shared 
libraries that manage access to a GPS for applications

%package -n libQgpsmm
Summary: Qt Client libraries for talking to a running gpsd or GPS
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
Requires: qt
Requires: pkgconfig

%description -n libQgpsmm
This package provides Qt shared libraries that manage access to a GPS
for Qt applications

%package clients
Summary: Clients for gpsd
Group: Applications/System
Requires: clients-x11 = %{version}-%{release}
Requires: clients-cli = %{version}-%{release}

%package clients-x11
Summary: Clients for gpsd
Group: Applications/System
Requires: xorg-x11-xinit

%package clients-cli
Summary: Clients for gpsd
Group: Applications/System

%package clients-httpd
Summary: Clients for gpsd
Group: Applications/System
Requires: httpd
Requires: perl
Requires: perl(Net::GPSD3)
Requires: perl(GD::Graph::Polar)
Requires: perl(CGI)
Requires: perl(CGI::Carp)

%description clients
Installs X11 and Command Line Interface clients.

%description clients-x11
xgps is a simple test client for gpsd with an X interface. It displays
current GPS position/time/velocity information and (for GPSes that
support the feature) the locations of accessible satellites.

xgpsspeed is a speedometer that uses position information from the GPS.
It accepts an -h option and optional argument as for gps, or a -v option
to dump the package version and exit. Additionally, it accepts -rv
(reverse video) and -nc (needle color) options.

%description clients-cli
cgps resembles xgps, but without the pictorial satellite display.  It
can run on a serial terminal or terminal emulator.

%description clients-httpd
pgps is a Perl CGI client for gpsd. It displays current GPS
position/time/velocity information and (for GPSes that support the
feature) the locations of accessible satellites. This package installs
the CGI scripts at http://localhost/gpsd/.

%prep
%setup -q

%build
#Note: prefix must be set to / since the current scons build only supports relative paths
scons %{_smp_mflags}         \
   prefix=/                  \
   bindir=%{_bindir}         \
   includedir=%{_includedir} \
   libdir=%{_libdir}         \
   sbindir=%{_sbindir}       \
   mandir=%{_mandir}         \
   docdir=%{_docdir}         \
   pkgconfigdir=%{_libdir}/pkgconfig

scons build

%install
rm -rf $RPM_BUILD_ROOT
export DESTDIR=$RPM_BUILD_ROOT
scons install

#Apps need man pages!
for MAN in gpsprof xgps xgpsspeed gpscat gpxlogger gegps
do 
  cp $RPM_BUILD_ROOT%{_mandir}/man1/gps.1 $RPM_BUILD_ROOT%{_mandir}/man1/$MAN.1
done 

#httpd client
%{__install} -d -m 0755 $RPM_BUILD_ROOT%{_var}/www/html/gpsd
%{__install} -p -m 0755 packaging/rpm/httpd/skyview.cgi $RPM_BUILD_ROOT%{_var}/www/html/gpsd
%{__install} -p -m 0755 packaging/rpm/httpd/pgps.cgi $RPM_BUILD_ROOT%{_var}/www/html/gpsd
%{__install} -d -m 0755 $RPM_BUILD_ROOT%{_sysconfdir}/httpd/conf.d
%{__install} -p -m 0644 packaging/rpm/httpd/gpsd.conf $RPM_BUILD_ROOT%{_sysconfdir}/httpd/conf.d/gpsd.conf

# init scripts
%{__install} -d -m 0755 $RPM_BUILD_ROOT%{_sysconfdir}/init.d
%{__install} -p -m 0755 packaging/rpm/gpsd.init $RPM_BUILD_ROOT%{_sysconfdir}/init.d/gpsd

%{__install} -d -m 0755 $RPM_BUILD_ROOT%{_sysconfdir}/sysconfig
%{__install} -p -m 0644 packaging/rpm/gpsd.sysconfig $RPM_BUILD_ROOT%{_sysconfdir}/sysconfig/gpsd

# udev rules
%{__install} -d -m 0755 $RPM_BUILD_ROOT%{_sysconfdir}/udev/rules.d
%{__install} -p -m 0644 gpsd.rules $RPM_BUILD_ROOT%{_sysconfdir}/udev/rules.d/99-gpsd.rules

# hotplug script
#%{__install} -d -m 0755 $RPM_BUILD_ROOT/lib/udev
#%{__install} -p -m 0755 gpsd.hotplug gpsd.hotplug.wrapper $RPM_BUILD_ROOT/lib/udev

# remove .la files
#rm -f $RPM_BUILD_ROOT%{_libdir}/libgps*.la

# fix non-executable python script
#%{__chmod} +x $RPM_BUILD_ROOT%{python_sitearch}/gps/gps.py

# Install the .desktop files
desktop-file-install --vendor fedora \
  --dir $RPM_BUILD_ROOT%{_datadir}/applications \
  --add-category X-Fedora \
  packaging/X11/xgps.desktop
desktop-file-install --vendor fedora \
  --dir $RPM_BUILD_ROOT%{_datadir}/applications \
  --add-category X-Fedora \
  packaging/X11/xgpsspeed.desktop

# Install logo icon for .desktop files
%{__install} -d -m 0755 $RPM_BUILD_ROOT%{_datadir}/gpsd
%{__install} -p -m 0644 packaging/X11/gpsd-logo.png $RPM_BUILD_ROOT%{_datadir}/gpsd/gpsd-logo.png

%clean
rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig
/sbin/chkconfig --add %{name}

%preun
if [ $1 = 0 ]; then
  /sbin/service %{name} stop > /dev/null 2>&1 || true
  /sbin/chkconfig --del %{name}
fi

%postun
/sbin/ldconfig

%files
%defattr(-,root,root,-)
%doc README INSTALL COPYING
%config(noreplace) %{_sysconfdir}/init.d/%{name}
%config(noreplace) %{_sysconfdir}/sysconfig/%{name}
%config(noreplace) %{_sysconfdir}/udev/rules.d/*
%{_sbindir}/gpsd
%{_sbindir}/gpsdctl
%{_bindir}/gpsprof
%{_bindir}/gpsmon
%{_bindir}/gpsctl
%{_bindir}/gegps
%{_libdir}/libgps*.so.*
#/lib/udev/gpsd*
%{python_sitearch}/gps*
%{_mandir}/man8/gpsd.8*
%{_mandir}/man8/gpsdctl.8*
%{_mandir}/man8/gpsinit.8.*
%{_mandir}/man1/gpsprof.1*
%{_mandir}/man1/gpsmon.1*
%{_mandir}/man1/gegps.1*
%{_mandir}/man1/gpsctl.1*
%{_mandir}/man5/gpsd_json.5*

%files devel
%defattr(-,root,root,-)
%doc TODO
%{_bindir}/gpsfake
%{_libdir}/libgps*.so
%{_libdir}/pkgconfig/*.pc
%{python_sitearch}/gps/fake*
%{_includedir}/gps.h
%{_includedir}/libgpsmm.h
#%{_includedir}/gpsd.h
%{_mandir}/man1/gpsfake.1*
%{_mandir}/man3/libgps.3*
%{_mandir}/man3/libgpsmm.3*
%{_mandir}/man3/libgpsd.3*
%{_mandir}/man5/srec.5*

%files -n libQgpsmm
%defattr(-,root,root,-)
%{_qt4_libdir}/libQgpsmm.so*
%{_qt4_libdir}/libQgpsmm.prl
%{_mandir}/man3/libQgpsmm.3*

%files clients
%defattr(-,root,root,-)
%{_mandir}/man1/gps.1*

%files clients-x11
%defattr(-,root,root,-)
%{_bindir}/xgps
%{_bindir}/xgpsspeed
%{_mandir}/man1/xgps.1*
%{_mandir}/man1/xgpsspeed.1*
%{_datadir}/applications/*.desktop
%dir %{_datadir}/gpsd
%{_datadir}/gpsd/gpsd-logo.png

%files clients-cli
%defattr(-,root,root,-)
%{_bindir}/cgps
%{_bindir}/gpscat
%{_bindir}/gpsdecode
%{_bindir}/gpspipe
%{_bindir}/gpxlogger
%{_bindir}/lcdgps
%{_mandir}/man1/cgps.1*
%{_mandir}/man1/gpscat.1*
%{_mandir}/man1/gpsdecode.1*
%{_mandir}/man1/gpspipe.1*
%{_mandir}/man1/lcdgps.1*
%{_mandir}/man1/gpxlogger.1*

%files clients-httpd
%defattr(-,root,root,-)
%config(noreplace) %{_sysconfdir}/httpd/conf.d/gpsd.conf
%dir %{_var}/www/html/gpsd/
%attr(0755,root,root) %{_var}/www/html/gpsd/skyview.cgi
%attr(0755,root,root) %{_var}/www/html/gpsd/pgps.cgi

%changelog
* Sun Jul 31 2011 Michael R. Davis <mrdvt@cpan.org> 3.0-2
- Added gegps, gpsdctl
- Dropped hotplug wrapper
- Removed tabs to pass rpmlint tests
- Updated scons install paths
- Updated scons build paths

* Sat Jun 25 2011 Eric S. Raymond <esr@thyrsus.com> 3.0-2
- Minor changes from the refactoring of the hotplug system. 

* Fri May 13 2011 Michael R. Davis <mrdvt@cpan.org> 3.0-1
- Added httpd Perl client package
- Separated x11 and cli packages

* Mon Apr 18 2011 Michael R. Davis <mrdvt@cpan.org> 3.0-1
- Updates to support SCons software construction tool
- Added httpd Perl client
- macro _buildrootdir does not exist in EPEL 4 & 5

* Mon Jul 05 2010 Michael R. Davis <mrdvt@cpan.org> - 2.95-3
- Updated to move rpm files to packaging/rpm folder
- Renamed gpsd-qt to libQgpsmm

* Sun Jul 04 2010 Michael R. Davis <mrdvt@cpan.org> - 2.95-2
- missing X11/app-defaults/xgpsspeed

* Sat Jul 03 2010 Michael R. Davis <mrdvt@cpan.org> - 2.95-1
- back ported spec to gpsd from Fedora 14
- updated to 2.95
- added gpsd-qt package

* Thu May 06 2010 Miroslav Lichvar <mlichvar@redhat.com> - 2.94-1
- update to 2.94 (#556642)

* Tue Mar 02 2010 Miroslav Lichvar <mlichvar@redhat.com> - 2.39-7
- don't use deprecated SYSFS{} in udev rules (#569089)
- fix init script LSB compliance

* Mon Feb 15 2010 Miroslav Lichvar <mlichvar@redhat.com> - 2.39-6
- fix linking with --no-add-needed (#564662)
- use %%global macro instead of %%define

* Wed Aug 12 2009 Marek Mahut <mmahut@fedoraproject.org> - 2.39-5
- RHBZ#505588: gpsd has a broken initscript that fails to launch daemon

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.39-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Tue Mar 31 2009 Tom "spot" Callaway <tcallawa@redhat.com> - 2.39-3
- some of the gpsd client bits went into gpsdclient.h, but that file wasn't 
  getting installed
- specifically, viking needs that header to build. 

* Wed Mar 25 2009 Douglas E. Warner <silfreed@silfreed.net> - 2.39-2
- adding patch to try to fix parallel make errors

* Thu Mar 19 2009 Douglas E. Warner <silfreed@silfreed.net> - 2.39-1
- updating to 2.39
- fixed potential core dump in C client handling of "K" responses
- Made device hotplugging work again; had been broken by changes in udev
- Introduced major and minor API version symbols into the public interfaces
- The sirfmon utility is gone, replaced by gpsmon which does the same job
  for multiple GPS types
- Fixed a two-year old error in NMEA parsing that nobody noticed because its
  only effect was to trash VDOP values from GSA sentences, and gpsd computes
  those with an internal error model when they look wonky
- cgpxlogger has been merged into gpxlogger
- Speed-setting commands now allow parity and stop-bit setting if the GPS
  chipset and adaptor can support it
- Specfile and other packaging paraphenalia now live in a packaging
  subdirectory
- rtcmdecode becomes gpsdecode and can now de-armor and dump AIDVM packets
- The client library now work correctly in locales where the decimal separator
  is not a period

* Mon Mar 16 2009 Douglas E. Warner <silfreed@silfreed.net> - 2.38-1
- updating to 2.38
- creating init script and sysconfig files
- migrating hotplug rules to udev + hotplug wrapper script from svn r5147
- updating pyexecdir patch
- fixing udev rule subsystem match
- Regression test load for RoyalTek RGM3800 and Blumax GPS-009 added
- Scaling on E error-estimate fields fixed to match O
- Listen on localhost only by default to avoid security problems; this can be
  overridden with the -G command-line option
- The packet-state machine can now recognize RTCM3 packets, though support is
  not yet complete
- Added support for ublox5 and mkt-3301 devices
- Add a wrapper around gpsd_hexdump to save CPU
- Lots of little fixes to various packet parsers
- Always keep the device open: "-n" is not optional any more
- xgpsspeed no longer depends on Motif
- gpsctl can now ship arbitrary payloads to a device; 
  It's possible to send binary through the control channel with the
  new "&" command
- Experimental new driver for Novatel SuperStarII
- The 'g' mode switch command now requires, and returns, 'rtcm104v2' rather
  than 'rtcm104'; this is design forward for when RTCM104v2 is fully working

* Tue Feb 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.37-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Sat Nov 29 2008 Ignacio Vazquez-Abrams <ivazqueznet+rpm@gmail.com> - 2.37-3
- Rebuild for Python 2.6

* Wed Mar 19 2008 Douglas E. Warner <silfreed@silfreed.net> - 2.37-2
- moving gpspacket.so python lib to main package

* Wed Feb 27 2008 Douglas E. Warner <silfreed@silfreed.net> - 2.37-1
- update to 2.37
- removed install-gpsd_config.h.patch
- installed pkgconfig files in devel package
- added patch to install python modules in sitearch
- removing rpath from inclucded libtool
- moving X11 app-defaults to datadir
- using macros for commands in install; using install instead of cp and mkdir
- cleaning up spaces/tabs for rpmlint

* Tue Feb 19 2008 Fedora Release Engineering <rel-eng@fedoraproject.org> - 2.34-9
- Autorebuild for GCC 4.3

* Sun Aug 19 2007 Matthew Truch <matt at truch.net> - 2.34-8
- Patch Makefile to also install gpsd_config.h as needed by
  libgpsmm.h.  Redhat BZ 253433.

* Sat Jun 30 2007 Matthew Truch <matt at truch.net> - 2.34-7
- Make sure the logo is actually included (via the spec file).
  I need to wake up before I try even trivial updates.  

* Sat Jun 30 2007 Matthew Truch <matt at truch.net> - 2.34-6
- Learn how to use search and replace (aka fix all instances of
  gpsd-logo.png spelled incorrectly as gspd-logo.png).

* Sat Jun 30 2007 Matthew Truch <matt at truch.net> - 2.34-5
- Fix desktop file and logo file name.

* Sat Jun 30 2007 Matthew Truch <matt at truch.net> - 2.34-4
- Include icon for .desktop files per BZ 241428

* Tue Mar 20 2007 Michael Schwendt <mschwendt[AT]users.sf.net> - 2.34-3
- Bump release for FE5 -> Fedora 7 upgrade path.

* Tue Feb 27 2007 Matthew Truch <matt at truch.net> - 2.34-2
- BR python-devel instead of python to make it build.  

* Tue Feb 27 2007 Matthew Truch <matt at truch.net> - 2.34-1
- Upgrade to 2.34.
- Get rid of %%makeinstall (which was never needed).
- Possibly fix hotplug issuses (BZ 219750).
- Use %%python_sitelib for python site-files stuff.

* Sat Dec 9 2006 Matthew Truch <matt at truch.net> - 2.33-6
- Rebuild to pull in new version of python.

* Tue Sep 26 2006 Matthew Truch <matt at truch.net> - 2.33-5
- Remove openmotif requirment, and switch to lesstif.

* Mon Aug 28 2006 Matthew Truch <matt at truch.net> - 2.33-4
- Bump release for rebuild in prep. for FC6.

* Thu Jul 20 2006 Matthew Truch <matt at truch.net> - 2.33-3
- Actually, was a missing BR glib-dbus-devel. Ooops.

* Thu Jul 20 2006 Matthew Truch <matt at truch.net> - 2.33-2
- Missing BR glib-devel

* Thu Jul 20 2006 Matthew Truch <matt at truch.net> - 2.33-1
- Update to version 2.33

* Wed Apr 19 2006 Matthew Truch <matt at truch.net> - 2.32-5
- Don't --enable-tnt in build as it causes some gpses to not work
  properly with sattelite view mode.  See bugzilla bug 189220.

* Thu Apr 13 2006 Matthew Truch <matt at truch.net> - 2.32-4
- Add dbus-glib to BuildRequires as needed for build.

* Sun Apr 9 2006 Matthew Truch <matt at truch.net> - 2.32-3
- Include xmlto and python in buildrequires so things build right.
- Don't package static library file.  

* Wed Apr 5 2006 Matthew Truch <matt at truch.net> - 2.32-2
- Use ye olde %%{?dist} tag.

* Wed Apr 5 2006 Matthew Truch <matt at truch.net> - 2.32-1
- Initial Fedora Extras specfile
