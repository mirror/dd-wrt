%define name pptpd
%define ver 1.2.3
%define rel 0
%define prefix /usr
%define buildlibwrap 1
%define buildbsdppp 0
%define buildslirp 0
%define buildipalloc 0
%define buildbcrelay 1
%define buildpnsmode 0

Summary: PoPToP Point to Point Tunneling Server
Name: %{name}
Requires: ppp >= 2.4.3
Version: %{ver}
Release: %{rel}
Copyright: GPL
Group: Networking/Daemons
Vendor: Hewlett-Packard
Packager: James Cameron <james.cameron@hp.com>
Source0: %{name}-%{ver}.tar.gz
URL: http://poptop.sourceforge.net/
Buildroot: %{_tmppath}/%{name}-root

%description
This implements a Virtual Private Networking Server (VPN) that is
compatible with Microsoft VPN clients. It allows windows users to
connect to an internal firewalled network using their dialup.

# allow --with[out] <feature> at rpm command line build
# e.g. --with ipalloc --without libwrap
# --with overrides --without
%{?_without_libwrap: %{expand: %%define buildlibwrap 0}}
%{?_without_bsdppp: %{expand: %%define buildbsdppp 0}}
%{?_without_slirp: %{expand: %%define buildslirp 0}}
%{?_without_ipalloc: %{expand: %%define buildipalloc 0}}
%{?_without_bcrelay: %{expand: %%define buildbcrelay 0}}
%{?_without_pnsmode: %{expand: %%define buildpnsmode 0}}
%{?_with_libwrap: %{expand: %%define buildlibwrap 1}}
%{?_with_bsdppp: %{expand: %%define buildbsdppp 1}}
%{?_with_slirp: %{expand: %%define buildslirp 1}}
%{?_with_ipalloc: %{expand: %%define buildipalloc 1}}
%{?_with_bcrelay: %{expand: %%define buildbcrelay 1}}
%{?_with_pnsmode: %{expand: %%define buildpnsmode 1}}

%prep

%setup -q -n pptpd-%{ver}

%build
buildopts=""
%if %{buildlibwrap}
buildopts="$buildopts --with-libwrap"
%endif
%if %{buildbsdppp}
buildopts="$buildopts --with-bsdppp"
%endif
%if %{buildslirp}
buildopts="$buildopts --with-slirp"
%endif
%if %{buildipalloc}
buildopts="$buildopts --with-pppd-ip-alloc"
%endif
%if %{buildbcrelay}
buildopts="$buildopts --with-bcrelay"
%endif
%if %{buildpnsmode}
buildopts="$buildopts --with-pns-mode"
%endif
./configure --prefix=%{prefix} $buildopts
make 

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/etc/rc.d/init.d
mkdir -p $RPM_BUILD_ROOT/etc/ppp
mkdir -p $RPM_BUILD_ROOT/usr/bin/
make prefix=$RPM_BUILD_ROOT%{prefix} INSTALL=install install
install -m 0755 pptpd.init $RPM_BUILD_ROOT/etc/rc.d/init.d/pptpd
install -m 0644 samples/pptpd.conf $RPM_BUILD_ROOT/etc/pptpd.conf
install -m 0644 samples/options.pptpd $RPM_BUILD_ROOT/etc/ppp/options.pptpd
install -m 0755 tools/vpnuser $RPM_BUILD_ROOT/usr/bin/vpnuser
#install -m 0755 tools/vpnstats $RPM_BUILD_ROOT/usr/bin/vpnstats
install -m 0755 tools/vpnstats.pl $RPM_BUILD_ROOT/usr/bin/vpnstats.pl
install -m 0755 tools/pptp-portslave $RPM_BUILD_ROOT/usr/sbin/pptp-portslave
mkdir -p $RPM_BUILD_ROOT/usr/man/man5
mkdir -p $RPM_BUILD_ROOT/usr/man/man8
install -m 0644 pptpd.conf.5 $RPM_BUILD_ROOT/usr/man/man5/pptpd.conf.5
install -m 0644 pptpd.8 $RPM_BUILD_ROOT/usr/man/man8/pptpd.8
install -m 0644 pptpctrl.8 $RPM_BUILD_ROOT/usr/man/man8/pptpctrl.8
# disabled to ease upstream support load, final packagers may choose otherwise
#strip $RPM_BUILD_ROOT/%{prefix}/sbin/* || :

%post
/sbin/chkconfig --add pptpd
OUTD="" ; for i in d manager ctrl ; do
    test -x /sbin/pptp$i && OUTD="$OUTD /sbin/pptp$i" ;
done
test -z "$OUTD" || \
{ echo "possible outdated executable detected; we now use /usr/sbin/pptp*, perhaps you should do run the following command:"; echo "rm -i $OUTD" ;}

%preun
/sbin/chkconfig --del pptpd
if [ "$1" -lt 1 ]; then
    /etc/init.d/pptpd stop > /dev/null 2>&1
    /sbin/chkconfig --del pptpd
fi

%clean
#rm -rf $RPM_BUILD_DIR/%{name}-%{ver}
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc AUTHORS COPYING INSTALL README* TODO ChangeLog* samples
/usr/sbin/pptpd
/usr/sbin/pptpctrl
/usr/sbin/pptp-portslave
%if %{buildbcrelay}
/usr/sbin/bcrelay
%endif
/usr/lib/pptpd/pptpd-logwtmp.so
/usr/bin/vpnuser
#/usr/bin/vpnstats
/usr/bin/vpnstats.pl
/usr/man/man5/pptpd.conf.5*
/usr/man/man8/pptpd.8*
/usr/man/man8/pptpctrl.8*
/etc/rc.d/init.d/pptpd
%config(noreplace) /etc/pptpd.conf
%config(noreplace) /etc/ppp/options.pptpd

%changelog
* Thu Nov 11 2004 James Cameron <james.cameron@hp.com>
- adjust for building on Red Hat Enterprise Linux, per Charlie Brady
- remove vpnstats, superceded by vpnstats.pl
* Fri May 21 2004 James Cameron <james.cameron@hp.com>
- adjust for packaging naming and test
* Fri Apr 23 2004 James Cameron <james.cameron@hp.com>
- include vpnwho.pl
* Thu Apr 22 2004 James Cameron <james.cameron@hp.com>
- change description wording
- change URL for upstream
- release first candidate for 1.2.0
* Fri Jul 18 2003 R. de Vroede <richard@oip.tudelft.nl>
- Check the ChangeLog files.

