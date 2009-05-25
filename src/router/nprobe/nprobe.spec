Summary: network probe
Name: nprobe
Version: 4.0
Release: 0
License: GPL
Group: Networking/Utilities
URL: http://www.ntop.org/nProbe.html
Source: nProbe-%{version}.tgz
Packager: Fernanda Weiden <nanda@google.com>
# Temporary location where the RPM will be built
BuildRoot:  %{_tmppath}/%{name}-%{version}-root
Requires: libpcap >= 0.8.3 glibc >= 2.3.5  

%description
nprobe is a software NetFlow v5/v9 and nFlow probe that allows to turn 
a PC into a NetFlow probe. It has been designed to be compact, easy to 
embed, an memory/CPU savvy.

%prep

%setup -q -n nProbe

%build
PATH=/usr/bin:/bin:/usr/sbin:/sbin

if [ -x ./configure ]; then
  CFLAGS="$RPM_OPT_FLAGS" ./configure 
else
  CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh 
fi
make

# Installation may be a matter of running an install make target or you
# may need to manually install files with the install command.
%install
PATH=/usr/bin:/bin:/usr/sbin:/sbin
make DESTDIR=$RPM_BUILD_ROOT install 

# Clean out our build directory
%clean
rm -fr $RPM_BUILD_ROOT

%files
/usr/local/bin/nprobe
/usr/local/lib/libdumpPlugin-4.0.so
/usr/local/lib/libdumpPlugin.a
/usr/local/lib/libdumpPlugin.la
/usr/local/lib/libnprobe-4.0.so
/usr/local/lib/libnprobe.a
/usr/local/lib/libnprobe.la
/usr/local/lib/librtpPlugin-4.0.so
/usr/local/lib/librtpPlugin.a
/usr/local/lib/librtpPlugin.la
/usr/local/lib/libsipPlugin-4.0.so
/usr/local/lib/libsipPlugin.a
/usr/local/lib/libsipPlugin.la

# Set the default attributes of all of the files specified to have an
# owner and group of root and to inherit the permissions of the file
# itself.
%defattr(-, root, root)

%changelog
* Fri Jan 27 2006 Fernanda Weiden <nanda@google.com> 4.0
- Original upstream version


