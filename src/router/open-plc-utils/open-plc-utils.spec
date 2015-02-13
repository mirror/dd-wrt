#
# spec file for package open-plc-utils
#

Summary: 	Open version of the Qualcomm Atheros Powerline Toolkit
Name: 		open-plc-utils
Version: 	0.0.3
Release: 	1
Source:	 	https://codeload.github.com/qca/open-plc-utils/zip/master/open-plc-utils-master.zip
URL:		https://github.com/qca/open-plc-utils.git
License:	BSD
Vendor:		Qualcomm-Atheros <cmaier@qca.qualcomm.com>
Group: 		Productivity/Networking/Diagnostic
BuildRoot: 	{_tmppath}/%{name}-%{version}-build
AutoReqProv:	on 

%description
This package installs the tools required to configure Qualcomm-Atheros
Powerline devices on the network.

%package slac
License:        BSD
Summary:        SLAC tools of the Qualcomm Atheros Powerline Toolkit
Group:          Development/Tools/Other
AutoReqProv:    on

%description slac
This package installs the tools for Signal Level Attenuation Characterization.

%prep -q
%setup -n open-plc-utils-master 

%build
make

%install
rm -rf "%buildroot"
mkdir "%buildroot"
make ROOTFS="%buildroot" install
make ROOTFS="%buildroot" manuals 

install -d -m 755 $RPM_BUILD_ROOT%{_docdir}/open-plc-utils
install -d -m 755 $RPM_BUILD_ROOT%{_docdir}/open-plc-utils/html
cp docbook/*.html docbook/*.png docbook/*.css $RPM_BUILD_ROOT%{_docdir}/open-plc-utils/html

%clean
rm -rf "%buildroot"

%files
%defattr (-,root,root)
%doc README 
/usr/local/bin/CMEncrypt
/usr/local/bin/ampID
/usr/local/bin/ampboot
/usr/local/bin/amphost
/usr/local/bin/amplist
/usr/local/bin/amprate
/usr/local/bin/amprule
/usr/local/bin/ampstat
/usr/local/bin/amptest
/usr/local/bin/amptone
/usr/local/bin/amptool
/usr/local/bin/ampwait
/usr/local/bin/chknvm
/usr/local/bin/chknvm2
/usr/local/bin/chkpib
/usr/local/bin/chkpib2
/usr/local/bin/config2cfg
/usr/local/bin/coqos_add
/usr/local/bin/coqos_info
/usr/local/bin/coqos_man
/usr/local/bin/coqos_mod
/usr/local/bin/coqos_rel
/usr/local/bin/edru
/usr/local/bin/edsu
/usr/local/bin/efbu
/usr/local/bin/efeu
/usr/local/bin/efru
/usr/local/bin/efsu
/usr/local/bin/getpib
/usr/local/bin/hpav
/usr/local/bin/hpavkey
/usr/local/bin/hpavkeys
/usr/local/bin/int64host
/usr/local/bin/int6k
/usr/local/bin/int6kbaud
/usr/local/bin/int6kboot
/usr/local/bin/int6kdetect
/usr/local/bin/int6keth
/usr/local/bin/int6kf
/usr/local/bin/int6khost
/usr/local/bin/int6kid
/usr/local/bin/int6klist
/usr/local/bin/int6klog
/usr/local/bin/int6kmdio
/usr/local/bin/int6kmdio2
/usr/local/bin/int6kmod
/usr/local/bin/int6krate
/usr/local/bin/int6krule
/usr/local/bin/int6kstat
/usr/local/bin/int6ktest
/usr/local/bin/int6ktone
/usr/local/bin/int6kuart
/usr/local/bin/int6kwait
/usr/local/bin/mac2pw
/usr/local/bin/mac2pwd
/usr/local/bin/mdioblock
/usr/local/bin/mdioblock2
/usr/local/bin/mdiodump
/usr/local/bin/mdiogen
/usr/local/bin/mdustats
/usr/local/bin/mme
/usr/local/bin/modpib
/usr/local/bin/nics
/usr/local/bin/nvmmerge
/usr/local/bin/nvmsplit
/usr/local/bin/pib2xml
/usr/local/bin/pibcomp
/usr/local/bin/pibdump
/usr/local/bin/pibruin
/usr/local/bin/pibrump
/usr/local/bin/plcID
/usr/local/bin/plcboot
/usr/local/bin/plcdevs
/usr/local/bin/plcfwd
/usr/local/bin/plcget
/usr/local/bin/plchost
/usr/local/bin/plchostd
/usr/local/bin/plclist
/usr/local/bin/plclog
/usr/local/bin/plcmdio16
/usr/local/bin/plcmdio32
/usr/local/bin/plcotst
/usr/local/bin/plcrate
/usr/local/bin/plcrule
/usr/local/bin/plcset
/usr/local/bin/plcstat
/usr/local/bin/plctest
/usr/local/bin/plctone
/usr/local/bin/plctool
/usr/local/bin/plcwait
/usr/local/bin/psgraph
/usr/local/bin/psin
/usr/local/bin/pskey
/usr/local/bin/psnotch
/usr/local/bin/psout
/usr/local/bin/ptsctl
/usr/local/bin/rkey
/usr/local/bin/sada
/usr/local/bin/sdram
/usr/local/bin/setpib
/usr/local/bin/ttycat
/usr/local/bin/ttyrecv
/usr/local/bin/ttysend
/usr/local/bin/ttysig
/usr/local/bin/weeder
/usr/local/bin/xml2pib
%{_mandir}/man1/CMEncrypt.1.gz
%{_mandir}/man1/ampID.1.gz
%{_mandir}/man1/ampboot.1.gz
%{_mandir}/man1/amphost.1.gz
%{_mandir}/man1/amplist.1.gz
%{_mandir}/man1/amprate.1.gz
%{_mandir}/man1/amprule.1.gz
%{_mandir}/man1/ampstat.1.gz
%{_mandir}/man1/amptest.1.gz
%{_mandir}/man1/amptone.1.gz
%{_mandir}/man1/amptool.1.gz
%{_mandir}/man1/ampwait.1.gz
%{_mandir}/man1/chknvm.1.gz
%{_mandir}/man1/chknvm2.1.gz
%{_mandir}/man1/chkpib.1.gz
%{_mandir}/man1/chkpib2.1.gz
%{_mandir}/man1/config2cfg.1.gz
%{_mandir}/man1/coqos_add.1.gz
%{_mandir}/man1/coqos_info.1.gz
%{_mandir}/man1/coqos_man.1.gz
%{_mandir}/man1/coqos_mod.1.gz
%{_mandir}/man1/coqos_rel.1.gz
%{_mandir}/man1/edru.1.gz
%{_mandir}/man1/edsu.1.gz
%{_mandir}/man1/efbu.1.gz
%{_mandir}/man1/efeu.1.gz
%{_mandir}/man1/efru.1.gz
%{_mandir}/man1/efsu.1.gz
%{_mandir}/man1/getpib.1.gz
%{_mandir}/man1/hpav.1.gz
%{_mandir}/man1/hpavkey.1.gz
%{_mandir}/man1/hpavkeys.1.gz
%{_mandir}/man1/int64host.1.gz
%{_mandir}/man1/int6k.1.gz
%{_mandir}/man1/int6kbaud.1.gz
%{_mandir}/man1/int6kboot.1.gz
%{_mandir}/man1/int6kdetect.1.gz
%{_mandir}/man1/int6keth.1.gz
%{_mandir}/man1/int6kf.1.gz
%{_mandir}/man1/int6khost.1.gz
%{_mandir}/man1/int6kid.1.gz
%{_mandir}/man1/int6klist.1.gz
%{_mandir}/man1/int6klog.1.gz
%{_mandir}/man1/int6kmdio.1.gz
%{_mandir}/man1/int6kmdio2.1.gz
%{_mandir}/man1/int6kmod.1.gz
%{_mandir}/man1/int6krate.1.gz
%{_mandir}/man1/int6krule.1.gz
%{_mandir}/man1/int6kstat.1.gz
%{_mandir}/man1/int6ktest.1.gz
%{_mandir}/man1/int6ktone.1.gz
%{_mandir}/man1/int6kuart.1.gz
%{_mandir}/man1/int6kwait.1.gz
%{_mandir}/man1/mac2pw.1.gz
%{_mandir}/man1/mac2pwd.1.gz
%{_mandir}/man1/mdioblock.1.gz
%{_mandir}/man1/mdioblock2.1.gz
%{_mandir}/man1/mdiodump.1.gz
%{_mandir}/man1/mdustats.1.gz
%{_mandir}/man1/mme.1.gz
%{_mandir}/man1/modpib.1.gz
%{_mandir}/man1/nics.1.gz
%{_mandir}/man1/nvmmerge.1.gz
%{_mandir}/man1/nvmsplit.1.gz
%{_mandir}/man1/pcapdevs.1.gz
%{_mandir}/man1/pib2xml.1.gz
%{_mandir}/man1/pibcomp.1.gz
%{_mandir}/man1/pibdump.1.gz
%{_mandir}/man1/plc.1.gz
%{_mandir}/man1/plcID.1.gz
%{_mandir}/man1/plcboot.1.gz
%{_mandir}/man1/plcdevs.1.gz
%{_mandir}/man1/plcfwd.1.gz
%{_mandir}/man1/plcget.1.gz
%{_mandir}/man1/plchost.1.gz
%{_mandir}/man1/plchostd.1.gz
%{_mandir}/man1/plclist.1.gz
%{_mandir}/man1/plclog.1.gz
%{_mandir}/man1/plcmdio16.1.gz
%{_mandir}/man1/plcmdio32.1.gz
%{_mandir}/man1/plcotst.1.gz
%{_mandir}/man1/plcrate.1.gz
%{_mandir}/man1/plcrule.1.gz
%{_mandir}/man1/plcset.1.gz
%{_mandir}/man1/plcstat.1.gz
%{_mandir}/man1/plctest.1.gz
%{_mandir}/man1/plctone.1.gz
%{_mandir}/man1/plctool.1.gz
%{_mandir}/man1/plcwait.1.gz
%{_mandir}/man1/psin.1.gz
%{_mandir}/man1/pskey.1.gz
%{_mandir}/man1/psout.1.gz
%{_mandir}/man1/ptsctl.1.gz
%{_mandir}/man1/rkey.1.gz
%{_mandir}/man1/sada.1.gz
%{_mandir}/man1/sdram.1.gz
%{_mandir}/man1/setpib.1.gz
%{_mandir}/man1/ttycat.1.gz
%{_mandir}/man1/ttyrecv.1.gz
%{_mandir}/man1/ttysend.1.gz
%{_mandir}/man1/ttysig.1.gz
%{_mandir}/man1/weeder.1.gz
%{_mandir}/man1/xml2pib.1.gz

%files slac
%defattr (-,root,root)
/usr/local/bin/evse
/usr/local/bin/pev
%{_mandir}/man1/evse.1.gz
%{_mandir}/man1/pev.1.gz

%changelog
* Thu Nov 14 2013 Stefan Wahren <stefan.wahren@i2se.com> - 0.0.3-1
- Update License
- Add new package slac 

* Sat Aug 31 2013 Stefan Wahren <stefan.wahren@i2se.com> - 0.0.2-1
- Initial package based on the debian branch
