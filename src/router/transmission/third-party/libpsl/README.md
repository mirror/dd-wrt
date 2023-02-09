[![Travis-CI Status](https://travis-ci.org/rockdaboot/libpsl.png?branch=master)](https://travis-ci.org/rockdaboot/libpsl)
[![Coverity Scan](https://img.shields.io/coverity/scan/10227.svg)](https://scan.coverity.com/projects/rockdaboot-libpsl)
[![Coverage Status](https://coveralls.io/repos/github/rockdaboot/libpsl/badge.svg?branch=master)](https://coveralls.io/github/rockdaboot/libpsl?branch=master)

Solaris OpenCSW [![Build Status Solaris amd64](https://buildfarm.opencsw.org/buildbot/png?builder=libpsl-solaris10-amd64)](https://buildfarm.opencsw.org/buildbot/builders/libpsl-solaris10-amd64)
[![Build Status Solaris i386](https://buildfarm.opencsw.org/buildbot/png?builder=libpsl-solaris10-i386)](https://buildfarm.opencsw.org/buildbot/builders/libpsl-solaris10-i386)
[![Build Status Solaris Sparc](https://buildfarm.opencsw.org/buildbot/png?builder=libpsl-solaris10-sparc)](https://buildfarm.opencsw.org/buildbot/builders/libpsl-solaris10-sparc)
[![Build Status Solaris SparcV9](https://buildfarm.opencsw.org/buildbot/png?builder=libpsl-solaris10-sparcv9)](https://buildfarm.opencsw.org/buildbot/builders/libpsl-solaris10-sparcv9)


libpsl - C library to handle the Public Suffix List
===================================================

A *Public Suffix List* is a collection of Top Level Domains (TLDs) suffixes.
TLDs include *Global Top Level Domains* (gTLDs) like `.com` and `.net`;
*Country Top Level Domains* (ccTLDs) like `.de` and `.cn`;
and *[Brand Top Level Domains](https://icannwiki.org/Brand_TLD)* like `.apple` and `.google`.
Brand TLDs allows users to register their own top level domain that exist at the same level as ICANN's gTLDs.
Brand TLDs are sometimes referred to as Vanity Domains.

Browsers, web clients and other user agents can use a public suffix list to:

- avoid privacy-leaking "supercookies"
- avoid privacy-leaking "super domain" certificates ([see post from Jeffry Walton](https://lists.gnu.org/archive/html/bug-wget/2014-03/msg00093.html))
- domain highlighting parts of the domain in a user interface
- sorting domain lists by site

Libpsl...

- has built-in PSL data for fast access (DAWG/DAFSA reduces size from 180kB to ~32kB)
- allows to load PSL data from files
- checks if a given domain is a "public suffix"
- provides immediate cookie domain verification
- finds the longest public part of a given domain
- finds the shortest private part of a given domain
- works with international domains (UTF-8 and IDNA2008 Punycode)
- is thread-safe
- handles IDNA2008 UTS#46 (if libicu is available)

Find more information about the Public Suffix List [here](https://publicsuffix.org/).

Download the Public Suffix List [here](https://github.com/publicsuffix/list/blob/master/public_suffix_list.dat).

The original DAFSA code is from the [Chromium Project](https://code.google.com/p/chromium/).


API Documentation
-----------------

You find the current API documentation [here](https://rockdaboot.github.io/libpsl).


Quick API example
-----------------

	#include <stdio.h>
	#include <libpsl.h>

	int main(int argc, char **argv)
	{
		const char *domain = "www.example.com";
		const char *cookie_domain = ".com";
		const psl_ctx_t *psl = psl_builtin();
		int is_public, is_acceptable;

		is_public = psl_is_public_suffix(psl, domain);
		printf("%s %s a public suffix.\n", domain, is_public ? "is" : "is not");

		is_acceptable = psl_is_cookie_domain_acceptable(psl, domain, cookie_domain);
		printf("cookie domain '%s' %s acceptable for domain '%s'.\n",
			cookie_domain, is_acceptable ? "is" : "is not", domain);

		return 0;
	}

Command Line Tool
-----------------

Libpsl comes with a tool 'psl' that gives you access to most of the
library API via command line.

	$ psl --help

prints the usage.

Convert PSL into DAFSA
----------------------

The [DAFSA](https://en.wikipedia.org/wiki/Deterministic_acyclic_finite_state_automaton) format is a compressed
representation of strings. Here we use it to reduce the whole PSL to about 32k in size.

Generate `psl.dafsa` from `list/public_suffix_list.dat`

	$ src/psl-make-dafsa --output-format=binary list/public_suffix_list.dat psl.dafsa

Test the result (example)

	$ tools/psl --load-psl-file psl.dafsa aeroclub.aero

License
-------

Libpsl is made available under the terms of the MIT license.<br>
See the LICENSE file that accompanies this distribution for the full text of the license.

src/psl-make-dafsa and src/lookup_string_in_fixed_set.c are licensed under the term written in
src/LICENSE.chromium.

Building from tarball
---------------------

Choose a release from https://github.com/rockdaboot/libpsl/tags an download the tarball
named `libpsl-{version}.tar.*`. Unpack with `tar xf <filename>`, cd into the libsl* directory.

Build with
```
./configure
make
make check
```

Install with
```
sudo make install
```

Building from git
-----------------

You should have python2.7+ installed.

Download project and prepare sources with

		git clone https://github.com/rockdaboot/libpsl
		./autogen.sh
		./configure
		make
		make check

If you prefer a `meson` build

		meson builddir
		ninja -C builddir
		ninja -C builddir test

There is also an unofficial MSVC nmake build configuration in `msvc/`.   Please
see README.MSVC.md on building libpsl with Visual Studio via NMake or Meson.


Mailing List
------------

[Mailing List Archive](https://groups.google.com/forum/#!forum/libpsl-bugs)

[Mailing List](https://groups.google.com/forum/#!forum/libpsl-bugs)

To join the mailing list send an email to

libpsl-bugs+subscribe@googlegroups.com

and follow the instructions provided by the answer mail.

Or click [join](https://groups.google.com/forum/#!forum/libpsl-bugs/join).
