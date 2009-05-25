#
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
# auto makefile for the nprobe 
# (this file is processed with 'automake' to produce Makefile.in)
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#
# Fusco Francesco     <fusco@ntop.org>
# Luca Deri           <deri@ntop.org>
#
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#



AUTOMAKE_OPTIONS = 1.6   # Require Automake 1.6 or better.

#
## The name of the game with its own major, minor and release version.
VERSION  = @PACKAGE_VERSION@
PROGRAM  = @PACKAGE_NAME@

APPL= nprobe
APPL_DEMO= nprobe_demo

PACKAGE=$(APPL)-3.9.1-0.i386.rpm
PKGNAME  = @PACKAGE_NAME@
PACKAGEDEMO=$(APPL)-demo-3.9.1-0.i386.rpm

DEFS      = @DEFS@
INCS      = @INCS@

INCLUDES  = -I. $(INCS)
LIBS      = @LIBS@ @CORELIBS@ @MORELIBS@
CC        = @CC@
CCLD      = @CCLD@
AM_CFLAGS = @CFLAGS@
AM_LDFLAGS= @LDFLAGS@

DISTCLEANFILES =  \#* *~ *.log *.lo *.o $(APPL) @PLUGINS@/*.o @PLUGINS@/*.lo  @PLUGINS@/*~
CLEANFILES     = $(DISTCLEANFILES)

SUBDIRS = . @PLUGINS@

bin_PROGRAMS  		 = nprobe
EXTRA_PROGRAMS		 = nprobe_demo


# nprobe 
nprobe_SOURCES = nprobe.c 
nprobe_DEPENDENCIES = libnprobe.la
nprobe_LDADD = libnprobe.la
nprobe_LDFLAGS = $(AM_LDFLAGS)
nprobe_CFLAGS =

lib_LTLIBRARIES = libnprobe.la

# nprobe core library

#
# On old systems getopt might be undefined
#
GETOPT_FILES=#getopt1.c getopt.c 
libnprobe_la_SOURCES = engine.c database.c $(GETOPT_FILES) plugin.c util.c version.c ixp.c
libnprobe_la_LDFLAGS = $(AM_LDFLAGS) -release $(VERSION) -export-dynamic @DYN_FLAGS@

# demo nprobe 
nprobe_demo_SOURCES = nprobe.c 
nprobe_demo_DEPENDENCIES = libnprobe.la
nprobe_demo_LDADD = libnprobe.la
nprobe_demo_LDFLAGS = $(AM_LDFLAGS) 
nprobe_demo_CFLAGS = $(AM_LDFLAGS) -DDEMO_MODE

build-suse: $(APPL)
	@rm -f /usr/src/redhat/RPMS/i386/$(PACKAGE)
	@cp $(APPL)  /usr/src/packages/BUILD
	@rpm -bb $(APPL).spec 
	@mv /usr/src/packages/RPMS/i386/$(PACKAGE) .

build-rh: $(APPL)
	@rm -f /usr/src/redhat/RPMS/i386/$(PACKAGE)
	@cp $(APPL) /usr/src/redhat/BUILD
	@rpmbuild -bb $(APPL).spec 
	@mv /usr/src/redhat/RPMS/i386/$(PACKAGE) .

build-rh-demo: $(APPL_DEMO)
	@rm -f /usr/src/redhat/RPMS/i386/$(PACKAGE)
	@cp $(APPL_DEMO)  /usr/src/redhat/BUILD
	@mv /usr/src/redhat/BUILD/$(APPL_DEMO) /usr/src/redhat/BUILD/$(APPL) 
	@rpmbuild -bb $(APPL).spec 
	@mv /usr/src/redhat/RPMS/i386/$(PACKAGE) $(PACKAGEDEMO)

CLEANLIST = `ls -l $(DESTDIR)$(libdir)/*nprobe*.so $(DESTDIR)$(libdir)/*Plugin*.so | \
          grep -v $(VERSION) | \
          grep -v '\->' | \
          $(AWK) '{ print $$NF }'`

listlibclean:
	@echo ""
	@echo "This is a list of old shared library files that will be deleted"
	@echo "if you run 'make libclean'"
	@echo ""
	@for file in $(CLEANLIST); do \
          if test -f $$file; then \
             echo "   $$file"; \
          fi; \
        done
	@echo ""

libclean:
	@echo ""
	@echo "Cleanup of old shared library files..."
	@echo ""
	@echo "  This is a list of files that will be deleted..."
	@echo ""
	@for file in $(CLEANLIST); do \
          if test -f $$file; then \
             echo "   $$file"; \
          fi; \
        done
	@echo ""
	@echo ""
	@echo "Press cntl-c to stop or enter to continue and delete these files..."
	@read select
	@echo ""
	@for file in $(CLEANLIST); do \
          if test -f $$file; then \
             echo "   ...Deleting $$file"; \
             rm -f $$file; \
          fi; \
        done


clean:
	/bin/rm -f $(CLEANFILES)

nprobes: $(nprobe_OBJECTS) $(nprobe_DEPENDENCIES) 
	cd plugins; make
	gcc -static *.o plugins/*.o -o nprobes -lz -lpcap -lpthread -ldl
