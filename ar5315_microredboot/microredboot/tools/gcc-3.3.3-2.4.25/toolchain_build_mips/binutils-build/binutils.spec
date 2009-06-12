%define cross_compile 0
%{?_with_crosscompile: %{expand: %%define cross_compile 1}}
%{!?_with_crosscompile: %{expand: %%define _host %{_target_platform}}}
%{!?_with_crosscompile: %{expand: %%define _build %{_target_platform}}}

# Define `coff_target' as coff if you want to add i386-coff instead of
# i386-pe.
%define coff_target pe
%{?_with_coff: %{expand: %%define coff_target coff}}
%define all_targets 0
%{?_with_all: %{expand: %%define all_targets 1}}

Summary: A GNU collection of binary utilities.
Name: binutils
Version: 2.14.90.0.6
Release: 1
Copyright: GPL
Group: Development/Tools
URL: http://sources.redhat.com/binutils
Source: http://www.kernel.org/pub/linux/devel/binutils/binutils-%{version}.tar.gz
Buildroot: %{_tmppath}/%{name}-%{version}-%{release}-root
Prereq: /sbin/install-info

%description
Binutils is a collection of binary utilities, including ar (for creating,
modifying and extracting from archives), nm (for listing symbols from
object files), objcopy (for copying and translating object files),
objdump (for displaying information from object files), ranlib (for
generating an index for the contents of an archive), size (for listing
the section sizes of an object or archive file), strings (for listing
printable strings from files), strip (for discarding symbols), c++filt
(a filter for demangling encoded C++ symbols), and addr2line (for
converting addresses to file and line).

%prep
%setup -q

# Apply additional Linux patches.
/bin/sh patches/README

%build
if [ -x /usr/bin/getconf ] ; then
  NRPROC=$(/usr/bin/getconf _NPROCESSORS_ONLN)
   if [ $NRPROC -eq 0 ] ; then
    NRPROC=1
  fi
else
  NRPROC=1
fi
echo "MAKE=make -j $NRPROC" > makefile
echo "include Makefile" >> makefile
ADDITIONAL_TARGETS=""
%ifos linux
%if %{all_targets}
ADDITIONAL_TARGETS="--enable-targets=alpha-linux,arm-linux,cris-linux,hppa-linux,i386-linux,x86_64-linux,ia64-linux,m68k-linux,mips-linux,mips64-linux,mips64el-linux,mipsel-linux,ppc-linux,ppc64-linux,s390-linux,s390x-linux,sh-linux,sparc-linux,sparc64-linux,i386-linuxaout"
%else
%ifarch %{ix86}
ADDITIONAL_TARGETS="--enable-targets=i386-linuxaout,i386-%{coff_target}"
%endif
%ifarch ia64
ADDITIONAL_TARGETS="--enable-targets=i386-linux"
%endif
%ifarch sparc ppc s390
ADDITIONAL_TARGETS="--enable-64-bit-bfd"
%endif
%endif
%endif

# Binutils come with its own custom libtool
%define __libtoolize echo
AR=%{__ar}; export AR
CC=%{__cc}; export CC
LD=%{__ld}; export LD
NM=%{__nm}; export NM
RANLIB=%{__ranlib}; export RANLIB
%configure --enable-shared $ADDITIONAL_TARGETS
make configure-bfd
%if %{cross_compile}
make headers -C bfd CFLAGS=-O
make tooldir=%{_prefix} all info CFLAGS_FOR_BUILD="-O -pipe"
%else
make headers -C bfd
make tooldir=%{_prefix} all info
%endif

%install
rm -rf ${RPM_BUILD_ROOT}
mkdir -p ${RPM_BUILD_ROOT}%{_prefix}
# Works for both rpm 3.0 and 4.0.
make prefix=${RPM_BUILD_ROOT}%{_prefix} \
     exec_prefix=$RPM_BUILD_ROOT%{_prefix} \
     tooldir=$RPM_BUILD_ROOT%{_prefix} \
     infodir=${RPM_BUILD_ROOT}%{_infodir} \
     mandir=${RPM_BUILD_ROOT}%{_mandir} \
     includedir=$RPM_BUILD_ROOT%{_prefix}/include \
     libdir=$RPM_BUILD_ROOT%{_prefix}/lib \
     bindir=$RPM_BUILD_ROOT%{_prefix}/bin \
     install install-info
#%makeinstall tooldir=${RPM_BUILD_ROOT}%{_prefix}
#make prefix=${RPM_BUILD_ROOT}%{_prefix} infodir=${RPM_BUILD_ROOT}%{_infodir} install-info
strip ${RPM_BUILD_ROOT}%{_prefix}/bin/*
gzip -q9f ${RPM_BUILD_ROOT}%{_infodir}/*.info*

#install -m 644 libiberty/libiberty.a ${RPM_BUILD_ROOT}%{_prefix}/lib
install -m 644 include/libiberty.h ${RPM_BUILD_ROOT}%{_prefix}/include

chmod +x ${RPM_BUILD_ROOT}%{_prefix}/lib/lib*.so*

rm -f ${RPM_BUILD_ROOT}%{_prefix}/lib/lib{bfd,opcodes}.{la,so}
rm -f ${RPM_BUILD_ROOT}%{_infodir}/dir
rm -f %{buildroot}%{_mandir}/man1/{dlltool,nlmconv,windres}*

%clean
rm -rf ${RPM_BUILD_ROOT}

%post
/sbin/ldconfig
/sbin/install-info --info-dir=%{_infodir} %{_infodir}/as.info.gz
/sbin/install-info --info-dir=%{_infodir} %{_infodir}/bfd.info.gz
/sbin/install-info --info-dir=%{_infodir} %{_infodir}/binutils.info.gz
/sbin/install-info --info-dir=%{_infodir} %{_infodir}/gprof.info.gz
/sbin/install-info --info-dir=%{_infodir} %{_infodir}/ld.info.gz
/sbin/install-info --info-dir=%{_infodir} %{_infodir}/standards.info.gz

%preun
if [ $1 = 0 ] ;then
  /sbin/install-info --delete --info-dir=%{_infodir} %{_infodir}/as.info.gz
  /sbin/install-info --delete --info-dir=%{_infodir} %{_infodir}/bfd.info.gz
  /sbin/install-info --delete --info-dir=%{_infodir} %{_infodir}/binutils.info.gz
  /sbin/install-info --delete --info-dir=%{_infodir} %{_infodir}/gprof.info.gz
  /sbin/install-info --delete --info-dir=%{_infodir} %{_infodir}/ld.info.gz
  /sbin/install-info --delete --info-dir=%{_infodir} %{_infodir}/standards.info.gz
fi

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%doc README
%{_prefix}/bin/*
%{_mandir}/man1/*
%{_prefix}/include/*
%{_prefix}/lib/ldscripts/*
%{_prefix}/lib/lib*
%{_infodir}/*info*
%{_datadir}/locale/*/LC_MESSAGES/*.mo
