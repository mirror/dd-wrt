%define	name	sdparm
%define	version	1.12
%define	release	1

Summary:	List or change SCSI disk parameters
Name:		%{name}
Version:	%{version}
Release:	%{release}
License:	FreeBSD
Group:		Utilities/System
URL:		https://sg.danny.cz/sg/sdparm.html
Source0:	https://sg.danny.cz/sg/p/%{name}-%{version}.tgz
BuildRoot:	%{_tmppath}/%{name}-%{version}-root
Packager:	Douglas Gilbert <dgilbert at interlog dot com>

%description
SCSI disk parameters are held in mode pages. This utility lists or
changes those parameters. Other SCSI devices (or devices that use
the SCSI command set) such as CD/DVD and tape drives may also find
parts of sdparm useful. Requires the linux kernel 2.4 series or later.
In the 2.6 series (or later) any device node the understands a SCSI
command set may be used (e.g. /dev/sda).

Fetches Vital Product Data (VPD) pages. Can send commands to start
or stop the media and load or unload removable media.

Warning: It is possible (but unlikely) to change SCSI disk settings
such that the disk stops operating or is slowed down. Use with care.

%prep

%setup -q

%build
%configure

%install
if [ "$RPM_BUILD_ROOT" != "/" ]; then
        rm -rf $RPM_BUILD_ROOT
fi

make install \
        DESTDIR=$RPM_BUILD_ROOT

%clean
if [ "$RPM_BUILD_ROOT" != "/" ]; then
        rm -rf $RPM_BUILD_ROOT
fi


%files
%defattr(-,root,root)
%doc ChangeLog INSTALL README CREDITS AUTHORS COPYING notes.txt
%attr(0755,root,root) %{_bindir}/*
# >> should that be %attr(0755,root,root) %{_sbindir}/*   ??
%{_mandir}/man8/*

%changelog
* Wed Apr 21 2021 - dgilbert at interlog dot com
- track recent spc6 and sbc4 drafts
  * sdparm-1.12
* Sun Mar 01 2020 - dgilbert at interlog dot com
- track recent spc6 and sbc4 drafts
  * sdparm-1.11
* Mon Feb 22 2016 - dgilbert at interlog dot com
- track recent spc5 and sbc4 drafts
  * sdparm-1.10
* Fri Dec 26 2014 - dgilbert at interlog dot com
- track recent spc5 and sbc4 drafts
  * sdparm-1.09
* Thu Jun 06 2013 - dgilbert at interlog dot com
- track recent spc4 and sbc3 drafts
  * sdparm-1.08
* Mon Mar 19 2012 - dgilbert at interlog dot com
- track recent spc4 and sbc3 drafts
  * sdparm-1.07
* Sun Oct 31 2010 - dgilbert at interlog dot com
- track recent spc4 and sbc3 drafts
  * sdparm-1.06
* Tue Apr 13 2010 - dgilbert at interlog dot com
- update SAS Enhanced phy control mpage (spl-r04); add '--readonly'
  * sdparm-1.05
* Sun Sep 20 2009 - dgilbert at interlog dot com
- linux bsg support, rework win32 device scan, thin provisioning
  * sdparm-1.04
* Mon Jun 23 2008 - dgilbert at interlog dot com
- allow multiple devices to be given, profile and speed commands
  * sdparm-1.03
* Mon Oct 08 2007 - dgilbert at interlog dot com
- add block device characteristics VPD page, descriptor based mpages
  * sdparm-1.02
* Thu Apr 05 2007 - dgilbert at interlog dot com
- add element address assignment mode page (smc)
  * sdparm-1.01
* Mon Oct 16 2006 - dgilbert at interlog dot com
- update Background control mode subpage, vendor specific mode pages
  * sdparm-1.00
* Sat Jul 08 2006 - dgilbert at interlog dot com
- add old power condition page for disks
  * sdparm-0.99
* Thu May 18 2006 - dgilbert at interlog dot com
- add medium configuration mode page
  * sdparm-0.98
* Wed Jan 25 2006 - dgilbert at interlog dot com
- add SAT pATA control and medium partition mode (sub)pages
  * sdparm-0.97
* Fri Nov 18 2005 - dgilbert at interlog dot com
- add capacity, ready and sync commands
  * sdparm-0.96
* Tue Sep 20 2005 - dgilbert at interlog dot com
- add debian build directory, decode more VPD pages
  * sdparm-0.95
* Thu Jul 28 2005 - dgilbert at interlog dot com
- add '--command=<cmd>' option
  * sdparm-0.94
* Thu Jun 02 2005 - dgilbert at interlog dot com
- add '--transport=' and '--dbd' options
  * sdparm-0.93
* Fri May 20 2005 - dgilbert at interlog dot com
- add some tape, cd/dvd, disk, ses and rbc mode pages
  * sdparm-0.92
* Fri May 06 2005 - dgilbert at interlog dot com
- if lk 2.4 detected, map non-sg SCSI device node to sg equivalent
  * sdparm-0.91
* Mon Apr 18 2005 - dgilbert at interlog dot com
- initial version
  * sdparm-0.90
