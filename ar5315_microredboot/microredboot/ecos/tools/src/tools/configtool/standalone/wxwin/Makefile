#
# File:		Makefile for Configtool
# Author:	Julian Smart
# Created:	2000.08.25
# Updated:	
# Copyright:	(c) Red Hat, Inc.
#
# To make absolutely everything:
#
# % mkdir /tmp/ecos-build
# % cd /tmp/ecos-build
# % make -f $(ECOSDIR)/host/tools/configtool/standalone/wxwin/Makefile WXDIR=$(WXDIR) ECOSDIR=$(ECOSDIR) LEVEL=debug full
#
# To make just Configtool:
#
# % make ... LEVEL=debug ct
#
# To make wxWindows & eCos libraries:
#
# % make ... LEVEL=debug wx ecc
#
# The result will be subdirectories under your /tmp/ecos-build (or other specified
# directory) containing:
#
# ct-build-debug/
# ecc-build-debug/
# wxwin-build-debug/
#
# To make a release version, specify LEVEL=release instead of LEVEL=debug.
#

# If this is set to gcc, we get some link errors relating to
# streams and such
CC = c++
#CC = gcc

PROGRAM = configtool
LEVEL = debug
WXDEBUGFLAGS=--with-debug_flag --with-debug_info --enable-debug --enable-debug_cntxt
ECCDEBUGFLAGS=--enable-debug 
# IMPORTANT NOTE for recent versions of wxWindows, you must pass
# --static to wx-config. For older versions, comment this out.
#WXCONFIGFLAGS=--static
CPPDEBUGOPTIONS=-ggdb
LINKDEBUGOPTIONS=
STRIPCOMMAND=echo 
USEEXPERIMENTALCODE=1

# You should specify this on the command line
WXDIR=You_should_override_the_WXDIR_variable

# Current (build) directory
CURRDIR = $(shell pwd)
BUILDDIR=$(CURRDIR)

# Where the CDL, infra etc. libraries have been built
ECOSBUILDDIR=$(BUILDDIR)/ecc-build-$(LEVEL)
WXBUILDDIR=$(BUILDDIR)/wxwin-build-$(LEVEL)
CTBUILDDIR=$(BUILDDIR)/ct-build-$(LEVEL)

# You have to specify this when invoking make.
ECOSDIR=You_should_override_the_ECOSDIR_variable
CTDIR=$(ECOSDIR)/host/tools/configtool/standalone/wxwin
WXCONFIG=$(WXBUILDDIR)/wx-config $(WXCONFIGFLAGS)
# Where the binary will be installed locally
LOCALBINDIR=/home/ecos/misctools/H-i686-pc-linux-gnu/bin
# A suitable home for archives of the tool
LOCALARCHIVEDIR=/netopt/ecc-share/releng/eCosTools/eCosConfigurationTool

ifneq "$(LEVEL)" "debug"
  WXDEBUGFLAGS=--without-debug_flag --without-debug_info --disable-debug 
  ECCDEBUGFLAGS=--disable-debug 
  CPPDEBUGOPTIONS=-O2
  LINKDEBUGOPTIONS=--strip-debug
  STRIPCOMMAND=strip
endif

OBJECTS = \
 $(CTBUILDDIR)/aboutdlg.o \
 $(CTBUILDDIR)/admindlg.o \
 $(CTBUILDDIR)/appsettings.o \
 $(CTBUILDDIR)/build.o \
 $(CTBUILDDIR)/buildoptionsdlg.o \
 $(CTBUILDDIR)/choosereposdlg.o \
 $(CTBUILDDIR)/Collections.o \
 $(CTBUILDDIR)/configitem.o \
 $(CTBUILDDIR)/configpropdlg.o \
 $(CTBUILDDIR)/configtool.o \
 $(CTBUILDDIR)/configtooldoc.o \
 $(CTBUILDDIR)/configtoolview.o \
 $(CTBUILDDIR)/configtree.o \
 $(CTBUILDDIR)/conflictsdlg.o \
 $(CTBUILDDIR)/conflictwin.o \
 $(CTBUILDDIR)/docsystem.o \
 $(CTBUILDDIR)/ecscrolwin.o \
 $(CTBUILDDIR)/eCosSerial.o \
 $(CTBUILDDIR)/eCosSocket.o \
 $(CTBUILDDIR)/eCosStd.o \
 $(CTBUILDDIR)/eCosTest.o \
 $(CTBUILDDIR)/eCosTestDownloadFilter.o \
 $(CTBUILDDIR)/eCosTestPlatform.o \
 $(CTBUILDDIR)/eCosTestSerialFilter.o \
 $(CTBUILDDIR)/eCosTestUtils.o \
 $(CTBUILDDIR)/eCosThreadUtils.o \
 $(CTBUILDDIR)/eCosTrace.o \
 $(CTBUILDDIR)/ecutils.o \
 $(CTBUILDDIR)/filename.o \
 $(CTBUILDDIR)/finddlg.o \
 $(CTBUILDDIR)/flags.o \
 $(CTBUILDDIR)/folderdlg.o \
 $(CTBUILDDIR)/htmlparser.o \
 $(CTBUILDDIR)/licensedlg.o \
 $(CTBUILDDIR)/mainwin.o \
 $(CTBUILDDIR)/mltwin.o \
 $(CTBUILDDIR)/msgdlgex.o \
 $(CTBUILDDIR)/outputwin.o \
 $(CTBUILDDIR)/packagesdlg.o \
 $(CTBUILDDIR)/platformeditordlg.o \
 $(CTBUILDDIR)/platformsdlg.o \
 $(CTBUILDDIR)/propertywin.o \
 $(CTBUILDDIR)/Properties.o \
 $(CTBUILDDIR)/reposdlg.o \
 $(CTBUILDDIR)/ResetAttributes.o \
 $(CTBUILDDIR)/runtestsdlg.o \
 $(CTBUILDDIR)/sectiondlg.o \
 $(CTBUILDDIR)/settingsdlg.o \
 $(CTBUILDDIR)/shortdescrwin.o \
 $(CTBUILDDIR)/solutionswin.o \
 $(CTBUILDDIR)/splittree.o \
 $(CTBUILDDIR)/Subprocess.o \
 $(CTBUILDDIR)/templatesdlg.o \
 $(CTBUILDDIR)/TestResource.o


# C++ compiler
CC          = c++

INCLUDEPREFIX=--prefix=$(WXBUILDDIR)
LIBPREFIX=--prefix=$(WXBUILDDIR)
EXTRACPPFLAGS=\
  -I$(WXDIR)/include\
  -I$(ECOSDIR)/host/libcdl\
  -I$(ECOSDIR)/host/infra\
  -I$(ECOSDIR)/host/tools/configtool/common/common\
  -I$(ECOSDIR)/host/tools/Utils/common\
  -I$(ECOSDIR)/host/tools/ecostest/common\
  -I$(ECOSBUILDDIR)/libcdl\
  -I$(ECOSBUILDDIR)/infra\
  -DecUSE_EXPERIMENTAL_CODE=$(USEEXPERIMENTALCODE)

EXTRALDFLAGS=-L$(ECOSBUILDDIR)/infra -L$(ECOSBUILDDIR)/libcdl -lcdl -lcyginfra -ltcl

.SUFFIXES:	.o .cpp .cxx

.cpp.o :
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

.cxx.o :
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

all:    mkbuilddir $(CTBUILDDIR)/$(PROGRAM)

ct:	mkbuilddir $(CTBUILDDIR)/$(PROGRAM)

mkbuilddir:
	mkdir -p $(CTBUILDDIR)

$(CTBUILDDIR)/$(PROGRAM):	$(OBJECTS)
	$(CC) $(LINKDEBUGOPTIONS) -o $(CTBUILDDIR)/$(PROGRAM) $(OBJECTS) $(EXTRALDFLAGS) `$(WXCONFIG) $(LIBPREFIX) --libs`
	$(STRIPCOMMAND) $(CTBUILDDIR)/$(PROGRAM)

full: wx ecc all

wx:
	@mkdir -p $(WXBUILDDIR) ; \
	cd $(WXBUILDDIR) ; \
        if test ! -f Makefile ; then rm -f *.o *.d ; \
        rm -f *.cache ; \
        $(WXDIR)/configure --disable-shared --enable-gui --with-gtk --enable-log $(WXDEBUGFLAGS) --with-threads --disable-no_rtti --without-sockets --without-odbc --without-libjpeg ; \
        fi; \
        make

ecc:
	@mkdir -p $(ECOSBUILDDIR) ; \
        cd $(ECOSBUILDDIR) ; \
        if test ! -f Makefile ; then $(ECOSDIR)/host/configure --prefix=$(ECOSBUILDDIR) $(ECCDEBUGFLAGS) --with-tcl=/usr ; \
        fi ; \
        make

# Some GTK+ installations are a bit broken and we need to define __WXGTK127__ even if
# we don't really have GTK++ 1.2.7. This manifests itself in a compile error in dcclient.cpp.

wxgtkfix:
	@rm -f $(WXBUILDDIR)/setup.h; rm -f $(WXBUILDIR)/lib/wx/include/wx/gtk*/setup/h; \
	sed -e "s;/\* #undef __WXGTK127__ \*/;#define __WXGTK127__ 1;g;" < $(WXBUILDDIR)/include/wx/gtk/setup.h > /tmp/setup.h ; \
	mv /tmp/setup.h $(WXBUILDDIR)/include/wx/gtk/setup.h

clean: 
	rm -f $(CTBUILDDIR)/$(PROGRAM) $(CTBUILDDIR)/*.o

cleanct: clean

cleanecc:
	rm -f -r $(ECOSBUILDDIR)

cleanwx:
	rm -f -r $(WXBUILDDIR)

cleanall:	clean cleanecc cleanwx

# Making a binary, RH Linux tarball release using the setup script.
# It will put the result in /tmp/ctdeliver.
releasect:
	echo "#include <stdio.h>" > /tmp/ctver.c
	echo "#include \"symbols.h\"" >> /tmp/ctver.c
	echo "int main() { printf(\"%.2f\", ecCONFIGURATION_TOOL_VERSION); }" >> /tmp/ctver.c
	cc /tmp/ctver.c -I$(CTDIR) -o /tmp/ctver
	chmod a+x $(CTDIR)/setup/maketarball.sh
	$(CTDIR)/setup/maketarball.sh $(ECOSDIR)/.. $(CTBUILDDIR) /tmp/ctdeliver `/tmp/ctver`
#	rm -f /tmp/ctver /tmp/ctver.c

# Installs the binary and associated files in the correct place for local running
installbinary: releasect
	cp /tmp/ctdeliver/configtool-`/tmp/ctver`/configtool $(LOCALBINDIR)/configtool-`date -I`
	mv /tmp/ctdeliver/configtool-`/tmp/ctver`/README.txt /tmp/ctdeliver/configtool-`/tmp/ctver`/README-configtool.txt
	rm -f $(LOCALBINDIR)/configtool
	ln -s $(LOCALBINDIR)/configtool-`date -I` $(LOCALBINDIR)/configtool
	cp -r /tmp/ctdeliver/configtool-`/tmp/ctver`/* $(LOCALBINDIR)

# Copies an archive to a suitable place for people to grab
installtarball: releasect
	mkdir -p $(LOCALARCHIVEDIR)/`date -I`
	cp /tmp/ctdeliver/configtool*.tar.?z* $(LOCALARCHIVEDIR)/`date -I`

### GUI Configtool files

$(CTBUILDDIR)/aboutdlg.o: $(CTDIR)/aboutdlg.cpp $(CTDIR)/aboutdlg.h $(CTDIR)/symbols.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/admindlg.o: $(CTDIR)/admindlg.cpp $(CTDIR)/admindlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/appsettings.o: $(CTDIR)/appsettings.cpp $(CTDIR)/appsettings.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/buildoptionsdlg.o: $(CTDIR)/buildoptionsdlg.cpp $(CTDIR)/buildoptionsdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/choosereposdlg.o: $(CTDIR)/choosereposdlg.cpp $(CTDIR)/choosereposdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/configitem.o: $(CTDIR)/configitem.cpp $(CTDIR)/configitem.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/configpropdlg.o: $(CTDIR)/configpropdlg.cpp $(CTDIR)/configpropdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/configtool.o: $(CTDIR)/configtool.cpp $(CTDIR)/configtool.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/configtooldoc.o: $(CTDIR)/configtooldoc.cpp $(CTDIR)/configtooldoc.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/configtoolview.o: $(CTDIR)/configtoolview.cpp $(CTDIR)/configtoolview.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/configtree.o: $(CTDIR)/configtree.cpp $(CTDIR)/configtree.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/conflictsdlg.o: $(CTDIR)/conflictsdlg.cpp $(CTDIR)/conflictsdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/conflictwin.o: $(CTDIR)/conflictwin.cpp $(CTDIR)/conflictwin.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/docsystem.o: $(CTDIR)/docsystem.cpp $(CTDIR)/docsystem.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/ecutils.o: $(CTDIR)/ecutils.cpp $(CTDIR)/ecutils.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/ecscrolwin.o: $(CTDIR)/ecscrolwin.cpp $(CTDIR)/ecscrolwin.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/filename.o: $(CTDIR)/filename.cpp $(CTDIR)/filename.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/finddlg.o: $(CTDIR)/finddlg.cpp $(CTDIR)/finddlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/folderdlg.o: $(CTDIR)/folderdlg.cpp $(CTDIR)/folderdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/htmlparser.o: $(CTDIR)/htmlparser.cpp $(CTDIR)/htmlparser.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/licensedlg.o: $(CTDIR)/licensedlg.cpp $(CTDIR)/licensedlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/mainwin.o: $(CTDIR)/mainwin.cpp $(CTDIR)/mainwin.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/mltwin.o: $(CTDIR)/mltwin.cpp $(CTDIR)/mltwin.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/msgdlgex.o: $(CTDIR)/msgdlgex.cpp $(CTDIR)/msgdlgex.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/outputwin.o: $(CTDIR)/outputwin.cpp $(CTDIR)/outputwin.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/packagesdlg.o: $(CTDIR)/packagesdlg.cpp $(CTDIR)/packagesdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/platformeditordlg.o: $(CTDIR)/platformeditordlg.cpp $(CTDIR)/platformeditordlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/platformsdlg.o: $(CTDIR)/platformsdlg.cpp $(CTDIR)/platformsdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/propertywin.o: $(CTDIR)/propertywin.cpp $(CTDIR)/propertywin.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/reposdlg.o: $(CTDIR)/reposdlg.cpp $(CTDIR)/reposdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<
 
$(CTBUILDDIR)/runtestsdlg.o: $(CTDIR)/runtestsdlg.cpp $(CTDIR)/runtestsdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/sectiondlg.o: $(CTDIR)/sectiondlg.cpp $(CTDIR)/sectiondlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/settingsdlg.o: $(CTDIR)/settingsdlg.cpp $(CTDIR)/settingsdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/shortdescrwin.o: $(CTDIR)/shortdescrwin.cpp $(CTDIR)/shortdescrwin.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/solutionswin.o: $(CTDIR)/solutionswin.cpp $(CTDIR)/solutionswin.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/splittree.o: $(CTDIR)/splittree.cpp $(CTDIR)/splittree.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/templatesdlg.o: $(CTDIR)/templatesdlg.cpp $(CTDIR)/templatesdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<


### Common files

$(CTBUILDDIR)/build.o:	$(ECOSDIR)/host/tools/configtool/common/common/build.cxx $(ECOSDIR)/host/tools/configtool/common/common/build.hxx
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/flags.o:	$(ECOSDIR)/host/tools/configtool/common/common/flags.cxx $(ECOSDIR)/host/tools/configtool/common/common/flags.hxx
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/Collections.o:	$(ECOSDIR)/host/tools/Utils/common/Collections.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/Properties.o:	$(ECOSDIR)/host/tools/Utils/common/Properties.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/Subprocess.o:	$(ECOSDIR)/host/tools/Utils/common/Subprocess.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/eCosSerial.o:	$(ECOSDIR)/host/tools/Utils/common/eCosSerial.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/eCosSocket.o:	$(ECOSDIR)/host/tools/Utils/common/eCosSocket.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/eCosStd.o:	$(ECOSDIR)/host/tools/Utils/common/eCosStd.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/eCosThreadUtils.o:	$(ECOSDIR)/host/tools/Utils/common/eCosThreadUtils.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/eCosTrace.o:	$(ECOSDIR)/host/tools/Utils/common/eCosTrace.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/eCosTest.o:	$(ECOSDIR)/host/tools/ecostest/common/eCosTest.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/eCosTestDownloadFilter.o:	$(ECOSDIR)/host/tools/ecostest/common/eCosTestDownloadFilter.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/eCosTestPlatform.o:	$(ECOSDIR)/host/tools/ecostest/common/eCosTestPlatform.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/eCosTestSerialFilter.o:	$(ECOSDIR)/host/tools/ecostest/common/eCosTestSerialFilter.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/eCosTestUtils.o:	$(ECOSDIR)/host/tools/ecostest/common/eCosTestUtils.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/ResetAttributes.o:	$(ECOSDIR)/host/tools/ecostest/common/ResetAttributes.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<

$(CTBUILDDIR)/TestResource.o:	$(ECOSDIR)/host/tools/ecostest/common/TestResource.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) `$(WXCONFIG) $(INCLUDEPREFIX) --cflags` -o $@ $<


help:
	@echo "Targets: full, ecc, wx, ct, cleanall, cleanct, cleanwx, installbinary, installtarball"