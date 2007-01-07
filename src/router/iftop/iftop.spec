Summary:		iftop - display bandwidth usage on an interface by host
Name:			iftop
Version:		0.17
Release:		1
Source:			http://www.ex-parrot.com/~pdw/iftop/%{name}-%{version}.tar.gz
URL:			http://www.ex-parrot.com/~pdw/iftop/
Group:			Network/Monitoring
Packager:		Richard Lucassen <spamtrap@lucassen.org>
Copyright:		GPL
BuildRoot:		/var/tmp/%{name}-%{version}-root
Prefix:			%{_prefix}

%description
iftop listens to network traffic on a named interface,  or
on  the  first  interface  it can find which looks like an
external interface if none is specified,  and  displays  a
table of current bandwidth usage by pairs of hosts.  iftop
must be run with sufficient  permissions  to  monitor  all
network  traffic  on  the  interface; see pcap(3) for more
information, but on most systems this means that  it  must
be run as root.


%prep
%setup -q

%build
CPPFLAGS=
./configure --prefix="%{_prefix}" --mandir="%{_mandir}"
make

%install
rm -rf "$RPM_BUILD_ROOT"
mkdir -p $RPM_BUILD_ROOT%{_bindir}
mkdir -p $RPM_BUILD_ROOT%{_mandir}/man8
install -s -m 755 iftop $RPM_BUILD_ROOT%{_bindir}
install iftop.8 $RPM_BUILD_ROOT%{_mandir}/man8


%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc AUTHORS COPYING ChangeLog NEWS README TODO
%{_bindir}/*
%doc %{_mandir}/man*/*

%changelog
* Fri Oct 17 2003 Richard Lucassen <spamtrap@lucassen.org>
- src.rpm can be rebuilt by a normal user now
* Fri Aug 30 2002 Iain Lea <iain@bricbrac.de>
- Updated build for Red Hat 7.3 version
* Sat Apr 13 2002 Riku Meskanen <mesrik@cc.jyu.fi>
- Initial build for Red Hat 7.2 version
