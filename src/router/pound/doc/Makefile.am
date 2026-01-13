# Pound - the reverse-proxy load-balancer
# Copyright (C) 2024-2025 Sergey Poznyakoff
#
# Pound is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# Pound is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with pound.  If not, see <http://www.gnu.org/licenses/>.

dist_man_MANS = pound.8 poundctl.8 poundctl.tmpl.5
info_TEXINFOS = pound.texi
pound_TEXINFOS = metrics.texi strftime.texi fdl.texi ctlapi417.texi\
  jwtlua.texi cookielua.texi

GENDOCS = perl gendocs.pl
EXTRA_DIST = gendocs.pl otherdoc.texi.in

if COND_MAKEINFO_INIT_FILE
  MAKEINFO_INIT_FILE_FLAG = --init-file=$(MAKEINFO_INIT_FILE)
endif

# Make sure you set TEXINPUTS.
# TEXINPUTS=/usr/share/texmf/pdftex/plain/misc/ is ok for most distributions
.PHONY: manual
manual:
	rm -rf manual
	TEXINPUTS=$(srcdir):$(top_srcdir)/build-aux:$(TEXINPUTS) \
	 MAKEINFO="$(MAKEINFO) $(AM_MAKEINFOFLAGS) $(MAKEINFOFLAGS)" \
	 $(GENDOCS) -C manual -o otherdoc.texi $(PACKAGE) otherdoc.texi.in
	$(MAKEINFO) $(AM_MAKEINFOFLAGS) $(MAKEINFOFLAGS) -DWEBDOC \
	   --html $(MAKEINFO_INIT_FILE_FLAG) $(info_TEXINFOS) -o manual

.PHONY: docserver
docserver: $(top_builddir)/src/pound
	@if test -f docserver.cfg; then \
	  $(top_builddir)/src/pound -Winclude-dir=$(builddir) -e -f docserver.cfg; \
	else \
	  echo >&2 "This command must be run in a cloned git tree"; \
	fi
