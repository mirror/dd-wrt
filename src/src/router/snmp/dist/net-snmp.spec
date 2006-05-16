Summary: Tools and servers for the SNMP protocol
Name: net-snmp
Version: %version
Release: %release
URL: http://net-snmp.sourceforge.net/
Copyright: BSDish
Group: System Environment/Daemons
Source: http://prdownloads.sourceforge.net/net-snmp/net-snmp-%{version}.tar.gz
Prereq: openssl
Obsoletes: cmu-snmp ucd-snmp ucd-snmp-utils
BuildRoot: /tmp/%{name}-root
Packager: The Net-SNMP Coders <http://sourceforge.net/projects/net-snmp/>
%description

Net-SNMP provides tools and libraries relating to the Simple Network
Management Protocol including: An extensible agent, An SNMP library,
tools to request or set information from SNMP agents, tools to
generate and handle SNMP traps, etc.  Using SNMP you can check the
status of a network of computers, routers, switches, servers, ... to
evaluate the state of your network.

%package devel
Group: Development/Libraries
Summary: The includes and static libraries from the Net-SNMP package.
Requires: net-snmp = %{version}
Obsoletes: cmu-snmp-devel ucd-snmp-devel

%description devel
The net-snmp-devel package contains headers and libraries which are
useful for building SNMP applications, agents, and sub-agents.

%prep
%setup -q


%build
%configure --with-defaults --with-sys-contact="Unknown" \
	--with-mib-modules="host disman/event-mib"      \
	--with-sysconfdir="/etc/net-snmp"               \
	--enable-shared					\
	--with-cflags="$RPM_OPT_FLAGS"

make

%install
# ----------------------------------------------------------------------
# 'install' sets the current directory to _topdir/BUILD/{name}-{version}
# ----------------------------------------------------------------------
rm -rf $RPM_BUILD_ROOT

%makeinstall

# Remove 'snmpinform' from the temporary directory because it is a
# symbolic link, which cannot be handled by the rpm installation process.
%__rm -f $RPM_BUILD_ROOT%{_prefix}/bin/snmpinform

%post
# ----------------------------------------------------------------------
# The 'post' script is executed just after the package is installed.
# ----------------------------------------------------------------------
# Create the symbolic link 'snmpinform' after all other files have
# been installed.
%__rm -f $RPM_INSTALL_PREFIX/bin/snmpinform
%__ln_s $RPM_INSTALL_PREFIX/bin/snmptrap $RPM_INSTALL_PREFIX/bin/snmpinform

# run ldconfig
PATH="$PATH:/sbin" ldconfig -n $RPM_INSTALL_PREFIX/lib

%preun
# ----------------------------------------------------------------------
# The 'preun' script is executed just before the package is erased.
# ----------------------------------------------------------------------
# Remove the symbolic link 'snmpinform' before anything else, in case
# it is in a directory that rpm wants to remove (at present, it isn't).
%__rm -f $RPM_INSTALL_PREFIX/bin/snmpinform

%postun
# ----------------------------------------------------------------------
# The 'postun' script is executed just after the package is erased.
# ----------------------------------------------------------------------
PATH="$PATH:/sbin" ldconfig -n $RPM_INSTALL_PREFIX/lib

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

# Install the following documentation in _defaultdocdir/{name}-{version}/
%doc AGENT.txt ChangeLog CodingStyle COPYING
%doc EXAMPLE.conf.def FAQ INSTALL NEWS PORTING TODO
%doc README README.agentx README.hpux11 README.krb5
%doc README.snmpv3 README.solaris README.thread README.win32
	 
%dir %{_datadir}/snmp/

#%config(noreplace) /etc/net-snmp/snmpd.conf
	 
%{_datadir}/snmp/snmpconf-data

%{_bindir}
%{_sbindir}
%{_mandir}/man1/*
%{_mandir}/man5/*
%{_mandir}/man8/*
/usr/lib/*.so*

%files devel
%defattr(-,root,root)

%{_includedir}
%{_libdir}/*.a
%{_libdir}/*.la
%{_mandir}/man3/*

%verifyscript
echo "No additional verification is done for net-snmp"

%changelog
* Wed Oct 09 2002 Wes Hardaker <hardaker@users.sourceforge.net>
- Incorperated most of Mark Harig's better version of the rpm spec and Makefile

* Wed Oct 09 2002 Wes Hardaker <hardaker@users.sourceforge.net>
- Made it possibly almost usable.

* Mon Apr 22 2002 Robert Story <rstory@users.sourceforge.net>
- created
