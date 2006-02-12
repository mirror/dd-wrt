%define ver 1.0.9
%define prefix /usr
%define  RELEASE 1
%define  rel     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}

Summary: GNOME http client library
Name: libghttp
Version: %ver
Release: %rel
Copyright: LGPL
Group: X11/gnome
Source: ftp://ftp.gnome.org/pub/GNOME/libghttp/libghttp-%{ver}.tar.gz
BuildRoot: /var/tmp/ghttp-%{PACKAGE_VERSION}-root
Docdir: %{prefix}/doc

Packager: Christopher Blizzard <blizzard@redhat.com>
URL: http://www.gnome.org/

%description
Library for making HTTP 1.1 requests.

%package devel
Summary: GNOME http client development
Group: X11/gnome
Requires: libghttp

%description devel
Libraries and includes files you can use for libghttp development

%changelog

%prep
%setup
if [ ! -f configure ]; then
  CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh --prefix=%prefix
else
  CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%prefix
fi

if [ "$SMP" != "" ]; then
  (make "MAKE=make -k -j $SMP"; exit 0)
  make
else
  make
fi

%install
rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{prefix} install

%clean
rm -rf $RPM_BUILD_ROOT

%post 
if ! grep %{prefix}/lib /etc/ld.so.conf > /dev/null ; then
  echo "%{prefix}/lib" >> /etc/ld.so.conf
fi

/sbin/ldconfig

%postun -p /sbin/ldconfig
%files
%defattr(-, root, root)

%doc AUTHORS COPYING ChangeLog NEWS README doc/ghttp.html
%{prefix}/lib/lib*.so*

%files devel
%defattr(-, root, root)

# %{prefix}/lib/lib*.so
%{prefix}/lib/*a
%{prefix}/include/*
%{prefix}/lib/ghttpConf.sh
