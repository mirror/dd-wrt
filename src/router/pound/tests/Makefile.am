## Makefile for the pound testsuite
## Process this file with automake to produce Makefile.in.
## Copyright (C) 2022-2025 Sergey Poznyakoff
##
## Pound is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
##
## Pound is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with pound.  If not, see <http://www.gnu.org/licenses/>.
EXTRA_DIST = $(TESTSUITE_AT) testsuite package.m4 poundharness.pl\
 perllib/PoundNS.pm perllib/PoundSub.pm
DISTCLEANFILES       = atconfig $(check_SCRIPTS)
MAINTAINERCLEANFILES = Makefile.in $(TESTSUITE) package.m4

## ------------ ##
## package.m4.  ##
## ------------ ##

$(srcdir)/package.m4: $(top_srcdir)/configure.ac
	{                                      \
	  echo '# Signature of the current package.'; \
	  echo 'm4_define([AT_PACKAGE_NAME],      [@PACKAGE_NAME@])'; \
	  echo 'm4_define([AT_PACKAGE_TARNAME],   [@PACKAGE_TARNAME@])'; \
	  echo 'm4_define([AT_PACKAGE_VERSION],   [@PACKAGE_VERSION@])'; \
	  echo 'm4_define([AT_PACKAGE_STRING],    [@PACKAGE_STRING@])'; \
	  echo 'm4_define([AT_PACKAGE_BUGREPORT], [@PACKAGE_BUGREPORT@])'; \
	} >$(srcdir)/package.m4

#

## ------------ ##
## Test suite.  ##
## ------------ ##

TESTSUITE = $(srcdir)/testsuite
M4=m4

AUTOTEST = $(AUTOM4TE) --language=autotest
$(TESTSUITE): package.m4 $(TESTSUITE_AT)
	$(AM_V_GEN)$(AUTOTEST) -I $(srcdir) testsuite.at -o $@.tmp
	$(AM_V_at)mv $@.tmp $@

atconfig: $(top_builddir)/config.status
	cd $(top_builddir) && ./config.status tests/$@

clean-local:
	@test ! -f $(TESTSUITE) || $(SHELL) $(TESTSUITE) --clean

check-local: atconfig atlocal $(TESTSUITE)
	@$(SHELL) $(TESTSUITE)

# Run the test suite on the *installed* tree.
#installcheck-local:
#	$(SHELL) $(TESTSUITE) AUTOTEST_PATH=$(exec_prefix)/bin

TESTSUITE_AT = \
 testsuite.at \
 acl.at\
 acme.at\
 addheader.at\
 backref.at\
 balancing.at\
 basicauth.at\
 bemix.at\
 checkurl.at\
 chgvis.at\
 chunked.at\
 chunked2.at\
 config.at\
 ctl-core.at\
 ctl-listener.at\
 ctl-service.at\
 dti.at\
 detcond.at\
 disable.at\
 dyn_a.at\
 dyn_a_ttl.at\
 dyn_srv.at\
 dyn_srv_0.at\
 dyn_srv_ttl.at\
 dynacl.at\
 dynbauth.at\
 dynpat.at\
 echo.at\
 err503.at\
 errfile.at\
 error.at\
 experr.at\
 fromfile.at\
 headdeny.at\
 header.at\
 headrem.at\
 headrequire.at\
 host.at\
 https.at\
 include.at\
 incldir.at\
 inclglob.at\
 inclnest.at\
 inclscope.at\
 invenc.at\
 list.at\
 logfmt.at\
 loglevcomp.at\
 loglevrun.at\
 logsup.at\
 lstset.at\
 lua-be.at\
 lua-load.at\
 lua-match.at\
 lua-modresp.at\
 lua-modreq.at\
 lua-http.at\
 lua-path.at\
 maxrequest.at\
 maxuri.at\
 multival.at\
 nacl.at\
 ndynacl.at\
 nb.at\
 not.at\
 optfwd.at\
 optorder.at\
 optssl.at\
 or.at\
 patfile.at\
 path.at\
 pcre.at\
 prio.at\
 query.at\
 queryparam.at\
 regextype.at\
 reqacc.at\
 redirect.at\
 resprw.at\
 rewriteloc.at\
 rewriteloc_https.at\
 rwchain.at\
 rwerr.at\
 sendfile.at\
 sessauth.at\
 sessctl.at\
 sesscookie.at\
 sesshdr.at\
 sessip.at\
 sessparm.at\
 sessurl.at\
 set.at\
 stringmatch.at\
 tbf.at\
 template.at\
 url.at\
 virthost.at\
 warndep.at\
 xhttp.at

noinst_PROGRAMS = tmplrun
tmplrun_SOURCES = tmplrun.c
AM_CPPFLAGS = -I$(top_srcdir)/src
tmplrun_LDADD = ../src/libpound.a ../src/tmpl.o
tmplrun_LDFLAGS = -lm

if COND_DYNAMIC_BACKENDS
if COND_BUILD_FAKEDNS
noinst_LTLIBRARIES = libfakedns.la
libfakedns_la_SOURCES = fakedns.c
libfakedns_la_LDFLAGS = -module -export-dynamic -avoid-version -rpath '/lib'
noinst_PROGRAMS += getsoa
endif
endif
