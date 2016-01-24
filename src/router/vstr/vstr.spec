%define ver      1.0.15
%define libver  1.0
%define real_release_num 1
%define RELEASE %{real_release_num}
%define rel     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
%define prefix    /usr
%define varprefix %nil
%define vardir %{varprefix}/var
%define wwwdir %{vardir}/www
%define name    vstr

%define jechod_uid 442
%define jhttpd_uid 444

# Do we want to run "make check" after the build...
%define chk_opt %{?chk}%{!?chk:0}

%define makeopts %{?jnum:-j %jnum}%{!?chk:%nil}

# Auto work out what kind of float support we want.
%ifarch i386
%define fmtfloatopt --with-fmt-float=%{?float}%{!?float:glibc}
%else
%define fmtfloatopt --with-fmt-float=%{?float}%{!?float:host}
%endif

%define devdoco %{_datadir}/doc/%{name}-devel-%{ver}

Summary: String library, safe, easy to use and fast.
Name: %{name}
Version: %ver
Release: %rel
License: LGPL
Group: Development/Libraries
Source: ftp://ftp.and.org/pub/james/%{name}/%{ver}/%{name}-%{ver}.tar.gz
BuildRoot: /var/tmp/%{name}-%{PACKAGE_VERSION}-root
Packager: James Antill <james@and.org>
BuildRequires: pkgconfig >= 0.8, gcc, make, glibc-devel
# Conflicts: vstr-debuginfo <> %{ver}
URL: http://www.and.org/%{name}/

%description
 Vstr is a string library designed for network communication, but applicable
in a number of other areas. It works on the idea of separate nodes of
information, and the length/ptr model and not the termination model of
"C strings". It does dynamic resizing of strings as you add/delete data.
 It can also do automatic referencing for mmap() areas, and includes a
portable version of a printf-like function (which is ISO 9899:1999 compliant,
and includes support for i18n parameter position modifiers).

 Development libs and headers are in %{name}-devel.

%package examples
Summary: String library, examples.
Group: Development/Tools
BuildRequires: gmp-devel, openssl-devel, glib2-devel
Requires: %{name} >= %{ver}

%description examples
 Compiled example programs in %{_libdir}/%{name}-%{ver}/examples

%package initd
Summary: String library, init.d files for daemon examples.
Group: Development/Tools
Requires: %{name}-examples = %{ver}
BuildRequires: timer_q-devel, socket_poll-devel

%description initd
 init.d start files for example programs in %{_libdir}/%{name}-%{ver}/examples

%package devel
Summary: String library, safe, easy to use and fast. Specilizes in IO.
Group: Development/Libraries
Requires: pkgconfig >= 0.8
Requires: %{name} = %{ver}

%description devel
 Static libraries and header files for the Vstr string library
 Also includes a %{name}.pc file for pkg-config.

%package debug
Summary: String library, safe, quick and easy to use. Specilizes in IO.
Group: Development/Libraries
Requires: pkgconfig >= 0.8
Requires: %{name}-devel = %{ver}

%description debug
 Static libraries, header files and dynamic libraries for a debug build of
the Vstr string library
 Also includes a %{name}-debug.pc file for pkg-config development.

%changelog
* Sun Aug 29 2004 James Antill <james@and.org>
- Added initd subpackage

* Tue Aug 24 2004 James Antill <james@and.org>
- Add -examples and -debug sub packages

* Fri Jun  4 2004 James Antill <james@and.org>
- Add build requires on gcc

* Wed Jan 14 2004 James Antill <james@and.org>
- Change to glob for examples.

* Fri Aug  8 2003 James Antill <james@and.org>
- Extra example programs to be installed

* Sat Mar 15 2003 James Antill <james@and.org>
- Add dia diagram of internal structure.
- Added design and printf_comparison html doco.

* Fri Jan 31 2003 James Antill <james@and.org>
- Added chk option for doing a "make check"

* Mon Jan 13 2003 James Antill <james@and.org>
- Moved the includedir to /usr/include/%{name}-*

* Sat Nov 16 2002 James Antill <james@and.org>
- Added wrap memcpy/memset to the default configure options.

* Sat Nov  9 2002 James Antill <james@and.org>
- Add comparison to installed documentation.

* Mon Sep 30 2002 James Antill <james@and.org>
- Remove files for check-files 8.0 rpm check.

* Mon Sep 30 2002 James Antill <james@and.org>
- Changed COPYING to COPYING.LIB

* Tue Sep 17 2002 James Antill <james@and.org>
- Add BuildRequires
- Add Requires to -devel package.
- Use glibc FLOAT by default on i386 arch.

* Mon Sep  2 2002 James Antill <james@and.org>
- Add `float' cmd line define.

* Tue May 21 2002 James Antill <james@and.org>
- Add linker script to configure.

* Wed May  6 2002 James Antill <james@and.org>
- Add man page.

* Wed Mar 20 2002 James Antill <james@and.org>
- Hack a spec file.

%prep
%setup

%build

function build()
{
%if %{chk_opt}
  make %{makeopts} check
%else
  make %{makeopts} 
%endif
}

mkdir dbg opt
cd dbg

../configure --prefix=%{prefix} --mandir=%{_mandir} \
  --datadir=%{_datadir} --sysconfdir=%{_sysconfdir} --libdir=%{_libdir} \
  --localstatedir=%{vardir} \
  --includedir=%{_includedir}/%{name}-%{libver} --enable-linker-script \
  --enable-debug --enable-examples \
  %{fmtfloatopt}
build

CFLAGS="$RPM_OPT_FLAGS"
export CFLAGS
cd ../opt
../configure  --prefix=%{prefix} --mandir=%{_mandir} \
  --datadir=%{_datadir} --sysconfdir=%{_sysconfdir} --libdir=%{_libdir} \
  --localstatedir=%{vardir} \
  --includedir=%{_includedir}/%{name}-%{libver} --enable-linker-script \
  --enable-wrap-memcpy --enable-wrap-memset --enable-examples \
  %{fmtfloatopt}
build

%install
rm -rf $RPM_BUILD_ROOT

cd opt
make DESTDIR=$RPM_BUILD_ROOT install
cd ..

cd dbg/src
make DESTDIR=$RPM_BUILD_ROOT libdir=%{_libdir}/%{name}-%{ver}/debug install
cd ..
cp -p %{name}-debug.pc $RPM_BUILD_ROOT/%{_libdir}/pkgconfig
cd ..

cp -p TODO BUGS $RPM_BUILD_ROOT/%{devdoco}/

rm -f  $RPM_BUILD_ROOT/%{_libdir}/lib%{name}.la
rm -f  $RPM_BUILD_ROOT/%{_libdir}/%{name}-%{ver}/debug/lib%{name}.la
rm -rf $RPM_BUILD_ROOT/%{_libdir}/%{name}-%{ver}/examples/html
rm -f  $RPM_BUILD_ROOT/usr/share/doc/%{name}-devel-%{ver}/functions.3
rm -f  $RPM_BUILD_ROOT/usr/share/doc/%{name}-devel-%{ver}/constants.3

mkdir -p $RPM_BUILD_ROOT/var/empty/jechod/dev
touch    $RPM_BUILD_ROOT/var/empty/jechod/dev/log

mkdir -p $RPM_BUILD_ROOT/var/www/html/localhost
ln -s localhost $RPM_BUILD_ROOT/var/www/html/127.0.0.1
mkdir -p $RPM_BUILD_ROOT/var/www/dev
touch    $RPM_BUILD_ROOT/var/www/dev/log

mkdir -p $RPM_BUILD_ROOT/%{_sbindir}
ln -s %{_libdir}/%{name}-%{ver}/examples/ex_echod $RPM_BUILD_ROOT/%{_sbindir}/jechod
ln -s %{_libdir}/%{name}-%{ver}/examples/ex_httpd $RPM_BUILD_ROOT/%{_sbindir}/jhttpd
ln -s %{_libdir}/%{name}-%{ver}/examples/ex_cntl  $RPM_BUILD_ROOT/%{_sbindir}/jcntl

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%pre initd

# Add the user id's
/usr/sbin/useradd -c "jECHOd" -u %{jechod_uid} \
        -s /sbin/nologin -r -d %{vardir}/empty/jechod jechod 2> /dev/null || :

%post initd

if [ $1 = 1 ]; then
    /sbin/chkconfig --add jechod
fi
if [ $1 = 2 ]; then
    /sbin/service jechod condrestart > /dev/null 2>&1
fi

%preun initd

if [ $1 = 0 ]; then
    /sbin/service jechod stop > /dev/null 2>&1
    /sbin/chkconfig --del jechod
fi

%postun -p /sbin/ldconfig

%files
%defattr(-, root, root)

%doc AUTHORS COPYING ChangeLog NEWS README TODO

%{_libdir}/lib%{name}-%{libver}.so.*

%files debug
%defattr(-, root, root)

%{_libdir}/%{name}-%{ver}/debug
%{_libdir}/pkgconfig/%{name}-debug.pc

%files devel
%defattr(-, root, root)

%{_libdir}/lib%{name}.so
%{_libdir}/lib%{name}.a
%{_libdir}/pkgconfig/%{name}.pc
%{_includedir}/%{name}-%{libver}/*.h

%doc
%{devdoco}
%{_mandir}/man3

%files examples
%defattr(-, root, root)

%{_libdir}/%{name}-%{ver}/examples

%files initd
%defattr(111, root, root)

/var/empty/

%defattr(-, root, root)
%{_sysconfdir}/init.d/*
%{_sbindir}/
/var/www/html
/var/www/dev
/var/www/dev/log
