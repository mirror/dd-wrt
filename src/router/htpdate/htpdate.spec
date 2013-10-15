Summary: HTTP based time synchronization tool
Name: htpdate
Version: 1.0.5
Release: 1
License: GPL
Group: System Environment/Daemons
URL: http://www.vervest.org/htp
Packager: Eddy Vervest <eddy@vervest.org>
Source: http://www.vervest.org/htp/archive/c/%{name}-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Prereq: /sbin/chkconfig


%description
The HTTP Time Protocol (HTP) is used to synchronize a computer's time
with web servers as reference time source. Htpdate will synchronize your
computer's time by extracting timestamps from HTTP headers found
in web servers responses. Htpdate can be used as a daemon, to keep your
computer synchronized.
Accuracy of htpdate is usually better than 0.5 seconds (even better with
multiple servers). If this is not good enough for you, try the ntpd package.

Install the htp package if you need tools for keeping your system's
time synchronized via the HTP protocol. Htpdate works also through
proxy servers.

%prep
%setup -q

%build
make
strip -s htpdate

%install
mkdir -p %{buildroot}/%{_bindir}
mkdir -p %{buildroot}/%{_mandir}
mkdir -p %{buildroot}/%{_initrddir}

install -m0755 htpdate %{buildroot}%{_bindir}/htpdate
install -m0644 htpdate.8.gz %{buildroot}%{_mandir}/htpdate.8.gz
install -m0755 scripts/htpdate.init %{buildroot}%{_initrddir}/htpdate

%post
%{_initrddir}/htpdate stop
/sbin/chkconfig --add htpdate
%{_initrddir}/htpdate start

%preun
%{_initrddir}/htpdate stop
/sbin/chkconfig --del htpdate

%clean
[ "%{buildroot}" != "/" ] && rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc README Changelog
%config(noreplace) %{_initrddir}/htpdate
%{_bindir}/htpdate
%{_mandir}/htpdate.8.gz
