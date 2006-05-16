%define name libnet
%define dirname Libnet
%define version 0.99e
%define release 1
%define prefix /usr

%define builddir $RPM_BUILD_DIR/%{dirname}-%{version}

Summary: Routines to help with network packet contruction and handling
Name: %{name}
Version: %{version}
Release: %{release}
Prefix: %{prefix}
Copyright: BSD
Vendor: Mike D. Schiffman <mike@infonexus.com>
Packager: Troy Engel <tengel@sonic.net>
Group: Development/Libraries
Source: %{name}-%{version}.tgz
URL: http://www.packetfactory.com/libnet
BuildRoot: /tmp/%{name}-%{version}-root

%description
Libnet is a collection of routines to help with the construction and
handling of network packets. It provides a portable framework for low-
level network packet writing and handling.

%prep
rm -rf %{builddir}

%setup -n %{dirname}-%{version}
touch `find . -type f`

%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{prefix}
make

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

cd $RPM_BUILD_ROOT
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > $RPM_BUILD_DIR/file.list.%{name}
find . -type f | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.%{name}
find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.%{name}

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf %{builddir}
rm -f $RPM_BUILD_DIR/file.list.%{name}

%files -f ../file.list.%{name}
%doc doc/CHANGELOG* doc/CONTRIB doc/COPYING doc/PORTS doc/README* doc/TODO
