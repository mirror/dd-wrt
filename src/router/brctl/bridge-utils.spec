# $Id: bridge-utils.spec,v 1.1.1.4 2003/10/14 08:09:37 sparq Exp $

Summary: Utilities for configuring the linux ethernet bridge.
Name: bridge-utils
Version: 0.9.1
Release: 1
Copyright: GPL
Group: Applications/System
Source0: ftp://openrock.net/bridge/bridge-utils-%{PACKAGE_VERSION}.tar.gz
BuildRoot: /var/tmp/%{name}-root

%description
This package contains utilities for configuring the linux ethernet
bridge. The linux ethernet bridge can be used for connecting multiple
ethernet devices together. The connecting is fully transparent: hosts
connected to one ethernet device see hosts connected to the other
ethernet devices directly.

Install bridge-utils if you want to use the linux ethernet bridge.

%package -n bridge-utils-devel
Summary: Utilities for configuring the linux ethernet bridge.
Group: Development/Libraries

%description -n bridge-utils-devel
The bridge-utils-devel package contains the header and object files
necessary for developing programs which use 'libbridge.a', the
interface to the linux kernel ethernet bridge. If you are developing
programs which need to configure the linux ethernet bridge, your
system needs to have these standard header and object files available
in order to create the executables.

Install bridge-utils-devel if you are going to develop programs which
will use the linux ethernet bridge interface library.

%prep
%setup -n bridge

%build
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p ${RPM_BUILD_ROOT}
mkdir -p ${RPM_BUILD_ROOT}/sbin
mkdir -p ${RPM_BUILD_ROOT}/usr/include
mkdir -p ${RPM_BUILD_ROOT}/usr/lib
mkdir -p ${RPM_BUILD_ROOT}/usr/man/man8
mkdir -p ${RPM_BUILD_ROOT}/usr/sbin
strip brctl/brctl
cp brctl/brctl ${RPM_BUILD_ROOT}/sbin
cp doc/brctl.8 ${RPM_BUILD_ROOT}/usr/man/man8
cp libbridge/libbridge.h ${RPM_BUILD_ROOT}/usr/include
cp libbridge/libbridge.a ${RPM_BUILD_ROOT}/usr/lib

%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%defattr (-,root,root)
%doc AUTHORS COPYING doc/FAQ doc/HOWTO doc/RPM-GPG-KEY
/sbin/brctl
/usr/man/man8/brctl.8

%files -n bridge-utils-devel
%defattr (-,root,root)
/usr/include/libbridge.h
/usr/lib/libbridge.a
