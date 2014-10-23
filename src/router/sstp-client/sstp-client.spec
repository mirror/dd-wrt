%define ppp_version 2.4.5
%define event_version 2.0.10

Name:		sstp-client
Version:	1.0.9
Release:	1%{?dist}
Summary:	Secure Socket Tunneling Protocol (SSTP) Client
Group:		Applications/Internet
License:	GPLv2+
Packager:	Eivind Naess <eivnaes@yahoo.com>
Provides:	sstp-client
URL:		http://sstp-client.sourceforge.net/
Source0:	http://downloads.sf.net/sstp-client/sstp-client-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Requires:	ppp >= %ppp_version
Requires:	libevent >= %event_version
Requires(pre):      /usr/sbin/useradd, /usr/bin/getent
Requires(postun):   /usr/sbin/userdel

%package devel
Summary:	Provide development headers for sstp-client
Group: 		Applications/Internet
Provides:	sstp-client-devel

%description devel
This package contains the necessary header files for sstp-client development

This package is required to compile plugin's for sstp-client.

%description
Client for the proprietary Microsoft Secure Socket Tunneling Protocol, SSTP.
Allows connection to a SSTP based VPN as used by employers and some cable 
and ADSL service providers.

%prep
%setup -q

%build
CFLAGS="-Wall %{optflags}" 		\
	./configure 			\
		--disable-static	\
		--prefix=/usr 		\
		--libdir=%_libdir	\
		--with-libevent=2 	\
		--with-pppd-plugin-dir=%_libdir/pppd/%ppp_version \
		--with-runtime-dir="/var/run/sstpc"
%{__make} %{?_smp_mflags}

%pre
/usr/bin/getent group sstpc || /usr/sbin/groupadd -r \
    sstpc

/usr/bin/getent passwd sstpc || /usr/sbin/useradd -r \
    -c "Secure Socket Tunneling Protocol (SSTP) Client" \
    -g sstpc \
    -d /var/run/sstpc \
    -s /bin/false \
    sstpc

%postun
rm -rf /var/run/sstpc
/usr/sbin/userdel sstpc

%install
%{__rm} -rf %{buildroot}
%{__make} DESTDIR=%{buildroot} install
%{__install} -c -d -m 755 %{buildroot}/%{_mandir}/man8
%{__install} -c -m 755 sstpc.8 %{buildroot}/%{_mandir}/man8

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING DEVELOPERS NEWS README TODO USING
%doc ChangeLog 
%{_sbindir}/sstpc
%{_mandir}/man8/sstpc.8*
%{_libdir}/libsstp_api*.so*
%{_libdir}/pppd/%ppp_version/sstp-pppd-plugin*.so*
%{_libdir}/pkgconfig/sstp-client*.pc
%{_docdir}/sstp-client/sstp-test*

%exclude %{_libdir}/*.la
%exclude %{_libdir}/pppd/%ppp_version/*.la


%files devel
/usr/include/sstp-client/sstp-api.h

%changelog
* Sat Jan 13 2013 Eivind Naess <eivnaes@yahoo.com> - 1.0.9-1
  * Fixed a critical bug that caused received packets to become corrupt, inability to access
    internal web-sites and caused random segfaults. Thanks to Damijan A. for reporting and 
    providing an environment to test in.

* Sat Oct 12 2012 Eivind Naess <eivnaes@yahoo.com> - 1.0.8-1
  * Unable to authenticate using --password, causing server to timeout
    the connection after 60 seconds.
  * Fixed a bug resulting in connection aborted on Ubuntu 12.04 with the 
    message: "Connection was aborted, value of attribute is incorrect"

* Sat Apr 28 2012 Eivind Naess <eivnaes@yahoo.com> - 1.0.7-1
  * Fixed a bug where connection failed to authenticate with server when specifying
    username / password on command line (first seen by microtik routers).
  * Various fixes to support Mac OS-X / Darwin

* Sat Mar 15 2012 Eivind Naess <eivnaes@yahoo.com> - 1.0.6-1
  * Fixed a bug where the temporary password file was deleted before authentication could
    proceed.

* Sat Mar 3 2012 Eivind Naess <evinaes@yahoo.com> - 1.0.5-1 
  * Fixed a critical bug in the sstp pppd plugin that caused the connection to fail.
  * Updated the documentation to better describe how to setup sstp-client

* Wed Feb 8 2012 Eivind Naess <evinaes@yahoo.com> - 1.0.4-1 
  * Added ability to add host route thanks to Dmitriy Belokurov for providing the inital patch.
  * Fixed various bugs, to mention:
    - Disconnect of SSL connection during multiple SSL_write() with inconsitent buffers.
    - Perform the authentication after IP is up

* Sun Nov 13 2011 Eivind Naess <eivnaes@yahoo.com> - 1.0.3-1
  * Added command line option to specify the uuid of the connection
  * Fixed various bugs, to mention:
    - Cleanup of unix socket on termination
    - Correct parsing of the URL 
    - Fix connected time error when using --nolaunchpppd option
    - Unit tests was added
    - Added hardening of ubuntu build scripts

* Sun Oct 20 2011 Eivind Naess <eivnaes@yahoo.com> - 1.2-1
  - Added http proxy support, using basic authentication
  - Adding privilege separation by chroot, and sstpc user.
  - Covering up traces of passwords after specifying --password per command line.
  - Command line option to ignore cerfificate errors (e.g. does not match host).
  - Fixing various bugs
  
* Sun Oct 02 2011 Eivind Naess <eivnaes@yahoo.com> - 1.0.1-1
- Initial packaging release
