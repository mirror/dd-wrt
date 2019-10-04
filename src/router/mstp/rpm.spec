Name:          mstpd
Summary:       STP/RSTP/PVST+/MSTP Spanning Tree Protocol Daemon
URL:           https://github.com/mstpd/mstpd
Version:       0.0.8
Release:       0%{?dist}

License:       GPLv2+
Group:         System Environment/Daemons

Source0:       %{name}-%{version}.tar.bz2

%description
This package provides a user-space daemon which replaces the STP handling that
is built into the Linux kernel Ethernet bridge and adds support for RSTP and
PVST+.

This daemon also supports participating in MSTP.  However, due to the way the
Linux kernel implements its FIBs, it is not currently possible to map MSTP
topologies onto Linux bridges.  Therefore, mstpd will not actually block ports
on Linux bridges when MSTP is used.


%prep
%setup -q

%build
./autogen.sh
%define _exec_prefix %{nil}
%define _libexecdir /lib
%configure
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}

sed -i -e 's|/etc/network/interfaces|/etc/sysconfig/network-scripts/bridge-stp|g' %{buildroot}/%{_libexecdir}/mstpctl-utils/ifquery
sed -i -e 's|/etc/network/interfaces|/etc/sysconfig/network-scripts/bridge-stp|g' %{buildroot}/%{_libexecdir}/mstpctl-utils/mstp_config_bridge

mkdir -p %{buildroot}/%{_sysconfdir}/sysconfig/network-scripts
cat <<END > %{buildroot}/%{_sysconfdir}/sysconfig/network-scripts/bridge-stp
# To automatically configure mstpd at boot in RHEL/Fedora:
# * If you are bridging VLANs / trunk ports, see
#   /usr/share/doc/mstpd/README.VLANs
# * Create a standard /etc/sysconfig/network-scripts/ifcfg-... file for the
#   bridge.  For example:
#   DEVICE=...
#   ONBOOT=yes
#   NM_CONTROLLED=no
#   BOOTPROTO=static
#   NOZEROCONF=yes
#   IPADDR=...
#   NETMASK=...
#   GATEWAY=...
#   IPV6INIT=yes
#   IPV6_AUTOCONF=no
#   IPV6ADDR=...
#   IPV6_DEFAULTGW=...
# * Note that TYPE=Bridge and the additional ifcfg-ethX files for each interface
#   attached to the bridge (as described at the following link) are unnecessary
#   when using mstpd.  If VLANs are not created on top of the bridge, these may
#   be present if desired, but they will simply cause the ifup-eth script to do
#   unnecessary work.  If VLANs are created on top of the bridge, TYPE=Bridge
#   must not be used, as it will cause the VLANs to be brought up before the
#   bridge, which will fail.
#   https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_Linux/6/html/Deployment_Guide/s2-networkscripts-interfaces_network-bridge.html
# * Add the following line (uncommented) at the end of the bridge's ifcfg file:
#   [ "\$0" = '/etc/sysconfig/network-scripts/ifup-eth' ] && /lib/mstpctl-utils/mstp_config_bridge "\$DEVICE"
# * Replace "br0" in the "iface" line below with the bridge's DEVICE name, and
#   adjust the mstpctl_* configuration options as needed.
#   See `man mstpctl-utils-interfaces` for documentation on the options.
#   To configure multiple bridges, add an additional "iface" line and associated
#   configuration options for each additional bridge.

iface br0
    mstpctl_ports eth0 eth1
    mstpctl_stp on
    mstpctl_forcevers rstp
    mstpctl_maxwait auto
    mstpctl_hello 2
    mstpctl_maxage 20
    mstpctl_fdelay 15
    mstpctl_txholdcount 6
    mstpctl_maxhops 20
    mstpctl_treeprio 8
    mstpctl_treeportprio swp3=8
    mstpctl_portpathcost swp1=0 swp2=0
    mstpctl_portadminedge swp1=no swp2=no
    mstpctl_portautoedge swp1=yes swp2=yes
    mstpctl_portp2p swp1=no swp2=no
    mstpctl_portrestrrole swp1=no swp2=no
    mstpctl_bpduguard swp1=no swp2=no
    mstpctl_portrestrtcn swp1=no swp2=no
    mstpctl_portnetwork swp1=no
END

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_sbindir}/mstpd
%{_sbindir}/mstpctl
%{_sbindir}/bridge-stp
%{_sbindir}/mstp_restart
%config(noreplace) %{_sysconfdir}/bridge-stp.conf
%config(noreplace) %{_sysconfdir}/sysconfig/network-scripts/bridge-stp
%{_sysconfdir}/bash_completion.d/mstpctl
%{_libexecdir}/mstpctl-utils/mstpctl_restart_config
%{_libexecdir}/mstpctl-utils/mstp_config_bridge
%{_libexecdir}/mstpctl-utils/ifquery
%{_libexecdir}/mstpctl-utils/ifupdown.sh
%{_libexecdir}/mstpctl-utils/mstpctl-utils-functions.sh
%doc %{_mandir}/man8/mstpctl.8.gz
%doc %{_mandir}/man5/mstpctl-utils-interfaces.5.gz
%doc %{_docdir}/mstpd/README.VLANs
