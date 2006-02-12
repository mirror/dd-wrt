%define all_x86 i686 i386 i586 athlon
ExclusiveArch: %{all_x86}

%define kversion 2.4.18
%define krelease 5

Name: cryptoapi
Version: 0.1.0
Release: %{kversion}_%{krelease}
License: GPL
Group: Unsorted
Summary: Linux Kernel CryptoAPI
BuildPreReq: kernel-source = %{kversion}-%{krelease}
BuildRoot: /var/tmp/%{name}-buildroot
Requires: kernel = %{kversion}-%{krelease}

%define srcbase cryptoapi-0.1.0-rc1
%define modpath /lib/modules/%{kversion}-%{krelease}

Source0: %{srcbase}.tar.gz

%define KSRC /usr/src/linux-%{kversion}-%{krelease}

%description 
Linux Kernel CryptoAPI

%prep
echo %{_target_cpu}
rm -rf %{srcbase}
%setup -n %{srcbase}
rm -fv kernel/crypto/cryptoloop/cryptoloop.c
pushd %{KSRC}
cp -v configs/kernel-%{kversion}-%{_target_cpu}.config .config
make oldconfig_nonint
popd

%build
echo "%{kversion}-%{krelease}" > ./version
make -k KDIR=%{KSRC} modules

%install
rm -rf $RPM_BUILD_ROOT
mkdir $RPM_BUILD_ROOT
make -k INSTALL_MOD_PATH=$RPM_BUILD_ROOT modules_install

%files 
%{modpath}/cryptoapi/cryptoapi.o
%{modpath}/cryptoapi/cipher-null.o
%{modpath}/cryptoapi/cipher-aes.o
%{modpath}/cryptoapi/cipher-twofish.o
%{modpath}/cryptoapi/cipher-serpent.o
%{modpath}/cryptoapi/cipher-blowfish.o
%{modpath}/cryptoapi/cipher-des.o
%{modpath}/cryptoapi/cipher-3des.o
%{modpath}/cryptoapi/digest-md5.o

%changelog
* Fri Jul 12 2002 Herbert Valerio Riedel <hvr@gnu.org>
- first spec file
