# ProFTPD Platforms

See the web site for a more complete and current list:

* http://www.proftpd.org/platforms.html

If you have successfully built and run ProFTPD on a platform not listed, you
are invited to share your experience.  Please include your platform name and
version (_e.g._ `uname -a`), compiler and version, the ProFTPD version (_e.g_
`./proftpd -V`), any optional modules that you are using (_e.g._
`./proftpd -l`), and any special instructions or comments.

The Linux compilation environment largely depends on the kernel, glibc, gcc,
and libpam.  So, please either report the versions of these components, or the
distribution name, version and any patches affecting the compilation
environment.

## Known Platforms

ProFTPD has been reported to build on the following:
```text
OS                 Compiler     Comments
-----------------  -----------  ----------------------------------------
AIX 3.2                         native compiler
AIX 4.2                         tested
AIX 4.2.1          gcc 2.95.2   see AIX.md
AIX 4.2.1          cc 4.4       see AIX.md
AIX 4.3.3          cc 4.4       see AIX.md

BSDI 2.x                        possibly
BSDI 3.1                        tested; use gmake instead of make
BSDI 4.0

?BSD/OS 4.1
?BSD/OS 4.2

Compaq Tru64 5.0A
Compaq Tru64 5.0B
Compaq Tru64 5.1B

DEC OFS/1                    native compiler
Digital UNIX 4.0A

FreeBSD 2.2.7
FreeBSD 3.3
FreeBSD 3.5        gcc
FreeBSD 4.1        gcc
FreeBSD 4.2        gcc
FreeBSD 4.3        gcc
FreeBSD 4.4        gcc
FreeBSD 4.5        gcc
FreeBSD 4.6        gcc
FreeBSD 4.7        gcc
FreeBSD 4.8        gcc
FreeBSD 4.9-PRERELEASE
FreeBSD 5.0        gcc

HP/UX 10.x
HP/UX 11.x                      native compiler or gcc

IRIX 6.2
IRIX 6.3                        native compiler or gcc
IRIX 6.4                        tested
IRIX 6.5           cc 7.30     

Linux              gcc          kernel 2.0.x, 2.2.x or 2.4.x
                                    glibc2 (libc6) required
MacOS X

NetBSD 1.4         gcc
NetBSD 1.5         gcc
NetBSD 1.6.1       gcc

OpenBSD 2.2
OpenBSD 2.3
OpenBSD 2.6        gcc
OpenBSD 2.7        gcc
OpenBSD 2.8        gcc

Solaris 2.5        gcc 2.91.66
Solaris 2.5.1      gcc
Solaris 2.6
Solaris 7
Solaris 8          cc 5.0
Solaris 8          gcc 2.95.2
Solaris 9          gcc 2.95.3

UnixWare 7
```

Linux distributions:
```text
Caldera 2.3        gcc
Conectiva 5.0      gcc          bundled
Debian 2.2         gcc
Immunix 6.2        gcc
Mandrake 7.2       gcc
Red Hat 6.2        gcc
Red Hat 7.0        gcc
Slackware 7        gcc
SuSE 6.4           gcc
Trustix 1.2        gcc 2.95.2   bundled
TurboLinux 6.0     gcc
```
