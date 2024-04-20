Name: libyang2
Version: {{ version }}
Release: {{ release }}%{?dist}
Summary: YANG data modeling language library
Url: https://github.com/CESNET/libyang
Source: libyang-%{version}.tar.gz
License: BSD-3-Clause

BuildRequires:  cmake
BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  pcre2-devel

%if "x%{?suse_version}" == "x"
Requires:  pcre2
%else
Requires:  libpcre2-posix2
%endif

%package -n libyang2-devel
Summary:    Headers of libyang library
Conflicts:  libyang-devel
Requires:   %{name} = %{version}-%{release}
Requires:   pcre2-devel

%package -n libyang2-tools
Summary:    Helper Tools and examples for libyang library
Conflicts:  libyang
Provides:   libyang-tools
Requires:   %{name} = %{version}-%{release}

%description -n libyang2-devel
Headers of libyang library.

%description -n libyang2-tools
Helper Tools and examples for libyang library.

%description
libyang is a YANG data modelling language parser and toolkit
written (and providing API) in C.

%prep
%setup -n libyang-%{version}
mkdir build

%build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr \
    -DCMAKE_BUILD_TYPE:String="Release" \
    -DCMAKE_C_FLAGS="${RPM_OPT_FLAGS}" \
    -DCMAKE_CXX_FLAGS="${RPM_OPT_FLAGS}" \
    ..
make

%check
cd build
ctest --output-on-failure

%install
cd build
make DESTDIR=%{buildroot} install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%{_libdir}/libyang.so.2*

%files -n libyang2-devel
%defattr(-,root,root)
%{_libdir}/libyang.so
%{_libdir}/pkgconfig/libyang.pc
%{_includedir}/libyang/*.h
%dir %{_includedir}/libyang/

%files -n libyang2-tools
%{_bindir}/yanglint
%{_bindir}/yangre
%{_mandir}/man1/yanglint.1.gz

%changelog
* Fri Apr 30 2021 Jakub Ružička <jakub.ruzicka@nic.cz> - {{ version }}-{{ release }}
- upstream package
