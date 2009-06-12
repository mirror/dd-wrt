# File:       eCos configtool makefile for GNU tools
# Author:     John Dallaway <jld@ecoscentric.com>
# Copyright:  Copyright (c) John Dallaway 2003

# Usage:
#   cd emptydir
#   make -f /path/to/this/makefile WXDIR=/path/to/wx/installation INSTALLDIR=/path/to/ecos/tools [ ECOSSRCDIR=/path/to/ecos/tools/src ] [ TCLDIR=/path/to/tcl/installation ] [ DEBUG=1 ]

INSTALLDIR=INSTALLDIR_not_defined
WXDIR=WXDIR_not_defined

CTBUILDDIR=$(shell pwd)
ECOSSRCDIR=$(INSTALLDIR)/src
CTDIR=$(ECOSSRCDIR)/tools/configtool/standalone/wxwin
TCLDIR=TCLDIR_use_system
USEEXPERIMENTALCODE=1

EXTRACPPFLAGS=\
  -I$(TCLDIR)/include \
  -I$(INSTALLDIR)/include \
  -I$(ECOSSRCDIR)/tools/configtool/common/common \
  -I$(ECOSSRCDIR)/tools/Utils/common \
  -I$(ECOSSRCDIR)/tools/ecostest/common \
  -DecUSE_EXPERIMENTAL_CODE=$(USEEXPERIMENTALCODE)
EXTRALDFLAGS=-L$(TCLDIR)/lib -L$(INSTALLDIR)/lib -lcdl -lcyginfra -ltcl

ifneq (,$(findstring CYGWIN, $(shell uname)))
  PROGRAM=configtool.exe
  CPPFLAGS=`$(WXDIR)/bin/wx-config --cppflags` -D_WIN32 -D__WIN32__ -DSTRICT
  LDFLAGS=`$(WXDIR)/bin/wx-config --libs` -lshlwapi -Wl,--subsystem,windows
  EXTRAOBJECTS=$(CTBUILDDIR)/configtoolres.o
else
  PROGRAM=configtool
  CPPFLAGS=`$(WXDIR)/bin/wx-config --cppflags`
  LDFLAGS=`$(WXDIR)/bin/wx-config --libs`
  EXTRAOBJECTS=
endif

ifeq "$(DEBUG)" ""
  CPPDEBUGOPTIONS=-O2
else
  CPPDEBUGOPTIONS=-ggdb
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

# Compilers
CC          = c++
RESCOMP     = windres

.SUFFIXES: .o .cpp .cxx

$(CTBUILDDIR)/$(PROGRAM): $(OBJECTS) $(EXTRAOBJECTS)
	$(CC) -o $@ $(OBJECTS) $(EXTRAOBJECTS) $(EXTRALDFLAGS) $(LDFLAGS)

install: $(CTBUILDDIR)/$(PROGRAM)
	install -s $< $(INSTALLDIR)/bin

.cpp.o :
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

.cxx.o :
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/configtoolres.o: $(CTDIR)/configtool.rc
	$(RESCOMP) -i $< -o $@ --preprocessor "$(CC) -c -E -xc-header -DRC_INVOKED" --include-dir $(WXDIR)/include --include-dir $(CTDIR) --define __GNUWIN32__

clean:
	rm -f $(CTBUILDDIR)/$(PROGRAM) $(CTBUILDDIR)/*.o

### GUI Configtool files

$(CTBUILDDIR)/aboutdlg.o: $(CTDIR)/aboutdlg.cpp $(CTDIR)/aboutdlg.h $(CTDIR)/symbols.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/admindlg.o: $(CTDIR)/admindlg.cpp $(CTDIR)/admindlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/appsettings.o: $(CTDIR)/appsettings.cpp $(CTDIR)/appsettings.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/buildoptionsdlg.o: $(CTDIR)/buildoptionsdlg.cpp $(CTDIR)/buildoptionsdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/choosereposdlg.o: $(CTDIR)/choosereposdlg.cpp $(CTDIR)/choosereposdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/configitem.o: $(CTDIR)/configitem.cpp $(CTDIR)/configitem.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/configpropdlg.o: $(CTDIR)/configpropdlg.cpp $(CTDIR)/configpropdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/configtool.o: $(CTDIR)/configtool.cpp $(CTDIR)/configtool.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/configtooldoc.o: $(CTDIR)/configtooldoc.cpp $(CTDIR)/configtooldoc.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/configtoolview.o: $(CTDIR)/configtoolview.cpp $(CTDIR)/configtoolview.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/configtree.o: $(CTDIR)/configtree.cpp $(CTDIR)/configtree.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/conflictsdlg.o: $(CTDIR)/conflictsdlg.cpp $(CTDIR)/conflictsdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/conflictwin.o: $(CTDIR)/conflictwin.cpp $(CTDIR)/conflictwin.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/docsystem.o: $(CTDIR)/docsystem.cpp $(CTDIR)/docsystem.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/ecutils.o: $(CTDIR)/ecutils.cpp $(CTDIR)/ecutils.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/ecscrolwin.o: $(CTDIR)/ecscrolwin.cpp $(CTDIR)/ecscrolwin.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/filename.o: $(CTDIR)/filename.cpp $(CTDIR)/filename.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/finddlg.o: $(CTDIR)/finddlg.cpp $(CTDIR)/finddlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/folderdlg.o: $(CTDIR)/folderdlg.cpp $(CTDIR)/folderdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/htmlparser.o: $(CTDIR)/htmlparser.cpp $(CTDIR)/htmlparser.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/licensedlg.o: $(CTDIR)/licensedlg.cpp $(CTDIR)/licensedlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/mainwin.o: $(CTDIR)/mainwin.cpp $(CTDIR)/mainwin.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/mltwin.o: $(CTDIR)/mltwin.cpp $(CTDIR)/mltwin.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/msgdlgex.o: $(CTDIR)/msgdlgex.cpp $(CTDIR)/msgdlgex.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/outputwin.o: $(CTDIR)/outputwin.cpp $(CTDIR)/outputwin.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/packagesdlg.o: $(CTDIR)/packagesdlg.cpp $(CTDIR)/packagesdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/platformeditordlg.o: $(CTDIR)/platformeditordlg.cpp $(CTDIR)/platformeditordlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/platformsdlg.o: $(CTDIR)/platformsdlg.cpp $(CTDIR)/platformsdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/propertywin.o: $(CTDIR)/propertywin.cpp $(CTDIR)/propertywin.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/reposdlg.o: $(CTDIR)/reposdlg.cpp $(CTDIR)/reposdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/runtestsdlg.o: $(CTDIR)/runtestsdlg.cpp $(CTDIR)/runtestsdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/sectiondlg.o: $(CTDIR)/sectiondlg.cpp $(CTDIR)/sectiondlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/settingsdlg.o: $(CTDIR)/settingsdlg.cpp $(CTDIR)/settingsdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/shortdescrwin.o: $(CTDIR)/shortdescrwin.cpp $(CTDIR)/shortdescrwin.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/solutionswin.o: $(CTDIR)/solutionswin.cpp $(CTDIR)/solutionswin.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/splittree.o: $(CTDIR)/splittree.cpp $(CTDIR)/splittree.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/templatesdlg.o: $(CTDIR)/templatesdlg.cpp $(CTDIR)/templatesdlg.h
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

### Common files

$(CTBUILDDIR)/build.o:	$(ECOSSRCDIR)/tools/configtool/common/common/build.cxx $(ECOSSRCDIR)/tools/configtool/common/common/build.hxx
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/flags.o:	$(ECOSSRCDIR)/tools/configtool/common/common/flags.cxx $(ECOSSRCDIR)/tools/configtool/common/common/flags.hxx
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/Collections.o:	$(ECOSSRCDIR)/tools/Utils/common/Collections.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/Properties.o:	$(ECOSSRCDIR)/tools/Utils/common/Properties.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/Subprocess.o:	$(ECOSSRCDIR)/tools/Utils/common/Subprocess.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/eCosSerial.o:	$(ECOSSRCDIR)/tools/Utils/common/eCosSerial.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/eCosSocket.o:	$(ECOSSRCDIR)/tools/Utils/common/eCosSocket.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/eCosStd.o:	$(ECOSSRCDIR)/tools/Utils/common/eCosStd.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/eCosThreadUtils.o:	$(ECOSSRCDIR)/tools/Utils/common/eCosThreadUtils.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/eCosTrace.o:	$(ECOSSRCDIR)/tools/Utils/common/eCosTrace.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/eCosTest.o:	$(ECOSSRCDIR)/tools/ecostest/common/eCosTest.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/eCosTestDownloadFilter.o:	$(ECOSSRCDIR)/tools/ecostest/common/eCosTestDownloadFilter.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/eCosTestPlatform.o:	$(ECOSSRCDIR)/tools/ecostest/common/eCosTestPlatform.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/eCosTestSerialFilter.o:	$(ECOSSRCDIR)/tools/ecostest/common/eCosTestSerialFilter.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/eCosTestUtils.o:	$(ECOSSRCDIR)/tools/ecostest/common/eCosTestUtils.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/ResetAttributes.o:	$(ECOSSRCDIR)/tools/ecostest/common/ResetAttributes.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

$(CTBUILDDIR)/TestResource.o:	$(ECOSSRCDIR)/tools/ecostest/common/TestResource.cpp
	$(CC) $(CPPDEBUGOPTIONS) -c $(EXTRACPPFLAGS) $(CPPFLAGS) -o $@ $<

