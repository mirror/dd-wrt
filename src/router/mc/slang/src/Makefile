# -*- sh -*-
#---------------------------------------------------------------------------
# ANSI C compiler
#---------------------------------------------------------------------------
CC              = mips-linux-uclibc-gcc
CFLAGS 		= -Os -pipe -mips32r2 -mtune=mips32r2 -funit-at-a-time -fPIC 
LDFLAGS		= 
#-----------------------------------------------------------------------------
# System library that contains functions for dynamic linking (-ldl)
#-----------------------------------------------------------------------------
DL_LIB		= -ldl

#---------------------------------------------------------------------------
# Compiler for producing a shared library (ELF)
#---------------------------------------------------------------------------
ELF_CC 		= $(CC)
ELF_CFLAGS	= $(CFLAGS) -fPIC 
ELF_LINK        = $(CC) $(LDFLAGS) -shared -Wl,-O1 -Wl,--version-script,$(VERSION_SCRIPT) -Wl,-soname,$(ELFLIB_MAJOR)
ELF_DEP_LIBS	= $(DL_LIB) -lm -lc

#---------------------------------------------------------------------------
# Set these values to ABSOLUTE path names
#---------------------------------------------------------------------------
SRCDIR = /home/seg/DEV/pb42/src/router/mc/slang/src# Location of sources
OBJDIR = /home/seg/DEV/pb42/src/router/mc/slang/src/mipsobjs# Location of objects
ELFDIR = /home/seg/DEV/pb42/src/router/mc/slang/src/elfmipsobjs# Location of elf objects
VERSION_SCRIPT	= $(SRCDIR)/slang.ver

#---------------------------------------------------------------------------
# When searching for modules, slang will search the paths specified by the
# value of the SLANG_MODULE_PATH environment variable.  The name of this
# variable may be changed here.
#---------------------------------------------------------------------------
MODULE_PATH_ENV_NAME = "SLANG_MODULE_PATH"

#---------------------------------------------------------------------------
# Misc Libraries
MISC_TERMINFO_DIRS = /usr/share/terminfo
OTHERLIBS =   -lm
#---------------------------------------------------------------------------
# Directory where library is going to go when installed
#---------------------------------------------------------------------------
prefix = /usr/local
exec_prefix = ${prefix}
install_lib_dir = ${exec_prefix}/lib
install_bin_dir = ${exec_prefix}/bin
install_include_dir = ${prefix}/include
install_doc_dir = $(prefix)/share/doc/slang/v2
DOC_FILES = ../changes.txt ../COPY* ../doc/slangdoc.html ../doc/text/*.txt
MODULE_INSTALL_DIR = ${exec_prefix}/lib/slang/v2/modules
#---------------------------------------------------------------------------
# DESTDIR is designed to facilitate making packages.  Normally it is empty
#---------------------------------------------------------------------------
DESTDIR =
DEST_LIBDIR = $(DESTDIR)$(install_lib_dir)
DEST_DOCDIR = $(DESTDIR)$(install_doc_dir)
DEST_INCDIR = $(DESTDIR)$(install_include_dir)
DEST_BINDIR = $(DESTDIR)$(install_bin_dir)

#---------------------------------------------------------------------------
# Misc commands (safe to leave these untouched)
#---------------------------------------------------------------------------
RANLIB 		= mips-linux-uclibc-ranlib
INSTALL		= /usr/bin/install -c
INSTALL_DATA	= ${INSTALL} -m 644
MKINSDIR	= ../autoconf/mkinsdir.sh
RM 		= rm -f
RM_R		= rm -rf
AR_CR 		= ar cr
RMDIR 		= rmdir
LN 		= /bin/ln -sf
CP 		= cp
MKDIR		= mkdir

#---------------------------------------------------------------------------
# There should be no need to change anything below here.
#---------------------------------------------------------------------------
THIS_LIB		= slang#
DLL_IMPLIB_NAME		= 
OTHERSTUFF 		= 
THIS_LIB_DEFINES	= -DSLANG
ELF_MAJOR_VERSION	= 2#
ELF_MINOR_VERSION	= 2#
ELF_MICRO_VERSION	= 2#
INSTALL_ELFLIB_TARGET	= install-elf-and-links

CONFIG_H		= config.h
ALL_CFLAGS		= $(CFLAGS) -Dunix $(THIS_LIB_DEFINES)
ALL_ELF_CFLAGS		= $(ELF_CFLAGS) -Dunix $(THIS_LIB_DEFINES)
COMPILE_CMD		= $(CC) -c $(ALL_CFLAGS)
ELFCOMPILE_CMD		= $(ELF_CC) -c $(ALL_ELF_CFLAGS)
ELF_LINK_CMD		= $(ELF_LINK)

NORMAL_LIB = lib$(THIS_LIB).a
OBJDIR_NORMAL_LIB = $(OBJDIR)/$(NORMAL_LIB)

# On most systems, these variables have values of the form:
#   libslang.so, libslang.so.X, libslang.so.X.Y, libslang.so.X.Y.Z
ELFLIB = lib$(THIS_LIB).so
ELFLIB_MAJOR = $(ELFLIB).$(ELF_MAJOR_VERSION)
ELFLIB_MAJOR_MINOR = $(ELFLIB_MAJOR).$(ELF_MINOR_VERSION)
ELFLIB_MAJOR_MINOR_MICRO = $(ELFLIB_MAJOR_MINOR).$(ELF_MICRO_VERSION)

# Set this to one of the above ELFLIB_MAJOR_* values
ELFLIB_BUILD_NAME       = $(ELFLIB_MAJOR_MINOR_MICRO)

ELFDIR_ELF_LIB = $(ELFDIR)/$(ELFLIB_BUILD_NAME)#
ELFDIR_DLL_LIB = $(ELFDIR)/$(DLL_IMPLIB_NAME)#

EXECLIBS = -L$(OBJDIR) -lslang $(OTHERLIBS) $(DL_LIB)
EXECDEPS = $(OBJDIR_NORMAL_LIB)

OFILES =  sltermin.o sldisply.o slutty.o slang.o slarray.o slclass.o slcmd.o slerr.o slgetkey.o slkeymap.o slmalloc.o slmath.o slmemchr.o slmemcmp.o slmemcpy.o slmemset.o slmisc.o slparse.o slprepr.o slregexp.o slrline.o slsearch.o slsmg.o slstd.o sltoken.o sltypes.o slxstrng.o slcurses.o slscroll.o slsignal.o slkeypad.o slerrno.o slstring.o slstruct.o slcmplex.o slarrfun.o slimport.o slpath.o slarith.o slassoc.o slcompat.o slposdir.o slstdio.o slproc.o sltime.o slstrops.o slbstr.o slpack.o slintall.o slistruc.o slposio.o slnspace.o slarrmis.o slospath.o slscanf.o sllower.o slupper.o slischar.o slutf8.o slwcwidth.o slwclut.o slcommon.o sllist.o slexcept.o slfpu.o slsig.o slboseos.o
OBJS =  $(OBJDIR)/sltermin.o $(OBJDIR)/sldisply.o $(OBJDIR)/slutty.o $(OBJDIR)/slang.o $(OBJDIR)/slarray.o $(OBJDIR)/slclass.o $(OBJDIR)/slcmd.o $(OBJDIR)/slerr.o $(OBJDIR)/slgetkey.o $(OBJDIR)/slkeymap.o $(OBJDIR)/slmalloc.o $(OBJDIR)/slmath.o $(OBJDIR)/slmemchr.o $(OBJDIR)/slmemcmp.o $(OBJDIR)/slmemcpy.o $(OBJDIR)/slmemset.o $(OBJDIR)/slmisc.o $(OBJDIR)/slparse.o $(OBJDIR)/slprepr.o $(OBJDIR)/slregexp.o $(OBJDIR)/slrline.o $(OBJDIR)/slsearch.o $(OBJDIR)/slsmg.o $(OBJDIR)/slstd.o $(OBJDIR)/sltoken.o $(OBJDIR)/sltypes.o $(OBJDIR)/slxstrng.o $(OBJDIR)/slcurses.o $(OBJDIR)/slscroll.o $(OBJDIR)/slsignal.o $(OBJDIR)/slkeypad.o $(OBJDIR)/slerrno.o $(OBJDIR)/slstring.o $(OBJDIR)/slstruct.o $(OBJDIR)/slcmplex.o $(OBJDIR)/slarrfun.o $(OBJDIR)/slimport.o $(OBJDIR)/slpath.o $(OBJDIR)/slarith.o $(OBJDIR)/slassoc.o $(OBJDIR)/slcompat.o $(OBJDIR)/slposdir.o $(OBJDIR)/slstdio.o $(OBJDIR)/slproc.o $(OBJDIR)/sltime.o $(OBJDIR)/slstrops.o $(OBJDIR)/slbstr.o $(OBJDIR)/slpack.o $(OBJDIR)/slintall.o $(OBJDIR)/slistruc.o $(OBJDIR)/slposio.o $(OBJDIR)/slnspace.o $(OBJDIR)/slarrmis.o $(OBJDIR)/slospath.o $(OBJDIR)/slscanf.o $(OBJDIR)/sllower.o $(OBJDIR)/slupper.o $(OBJDIR)/slischar.o $(OBJDIR)/slutf8.o $(OBJDIR)/slwcwidth.o $(OBJDIR)/slwclut.o $(OBJDIR)/slcommon.o $(OBJDIR)/sllist.o $(OBJDIR)/slexcept.o $(OBJDIR)/slfpu.o $(OBJDIR)/slsig.o $(OBJDIR)/slboseos.o
ELFOBJS =  $(ELFDIR)/sltermin.o $(ELFDIR)/sldisply.o $(ELFDIR)/slutty.o $(ELFDIR)/slang.o $(ELFDIR)/slarray.o $(ELFDIR)/slclass.o $(ELFDIR)/slcmd.o $(ELFDIR)/slerr.o $(ELFDIR)/slgetkey.o $(ELFDIR)/slkeymap.o $(ELFDIR)/slmalloc.o $(ELFDIR)/slmath.o $(ELFDIR)/slmemchr.o $(ELFDIR)/slmemcmp.o $(ELFDIR)/slmemcpy.o $(ELFDIR)/slmemset.o $(ELFDIR)/slmisc.o $(ELFDIR)/slparse.o $(ELFDIR)/slprepr.o $(ELFDIR)/slregexp.o $(ELFDIR)/slrline.o $(ELFDIR)/slsearch.o $(ELFDIR)/slsmg.o $(ELFDIR)/slstd.o $(ELFDIR)/sltoken.o $(ELFDIR)/sltypes.o $(ELFDIR)/slxstrng.o $(ELFDIR)/slcurses.o $(ELFDIR)/slscroll.o $(ELFDIR)/slsignal.o $(ELFDIR)/slkeypad.o $(ELFDIR)/slerrno.o $(ELFDIR)/slstring.o $(ELFDIR)/slstruct.o $(ELFDIR)/slcmplex.o $(ELFDIR)/slarrfun.o $(ELFDIR)/slimport.o $(ELFDIR)/slpath.o $(ELFDIR)/slarith.o $(ELFDIR)/slassoc.o $(ELFDIR)/slcompat.o $(ELFDIR)/slposdir.o $(ELFDIR)/slstdio.o $(ELFDIR)/slproc.o $(ELFDIR)/sltime.o $(ELFDIR)/slstrops.o $(ELFDIR)/slbstr.o $(ELFDIR)/slpack.o $(ELFDIR)/slintall.o $(ELFDIR)/slistruc.o $(ELFDIR)/slposio.o $(ELFDIR)/slnspace.o $(ELFDIR)/slarrmis.o $(ELFDIR)/slospath.o $(ELFDIR)/slscanf.o $(ELFDIR)/sllower.o $(ELFDIR)/slupper.o $(ELFDIR)/slischar.o $(ELFDIR)/slutf8.o $(ELFDIR)/slwcwidth.o $(ELFDIR)/slwclut.o $(ELFDIR)/slcommon.o $(ELFDIR)/sllist.o $(ELFDIR)/slexcept.o $(ELFDIR)/slfpu.o $(ELFDIR)/slsig.o $(ELFDIR)/slboseos.o

SHELL = /bin/sh

DOT_O_DEPS = sllimits.h _slang.h $(CONFIG_H)
sltoken_O_DEP = keywhash.c
slarith_O_DEP = slarith.inc slarith2.inc
slarrfun_O_DEP = slarrfun.inc
slarray_O_DEP = slagetput.inc
slischar_O_DEP = slischar.h
slwclut_O_DEP = slischar.h
sllower_O_DEP = sllower.h
slupper_O_DEP = slupper.h
slwcwidth_O_DEP = slwcwidth.h
slcommon_O_DEP = slang.h
slstd_O_DEP = Makefile
slimport_O_DEP = Makefile
sltermin_O_DEP = Makefile
slstd_C_FLAGS = -DSLANG_DOC_DIR='"$(install_doc_dir)"' -DSLANG_INSTALL_PREFIX='"$(prefix)"'
slimport_C_FLAGS = -DMODULE_INSTALL_DIR='"$(MODULE_INSTALL_DIR)"' -DMODULE_PATH_ENV_NAME='$(MODULE_PATH_ENV_NAME)'
sltermin_C_FLAGS = -DMISC_TERMINFO_DIRS='"$(MISC_TERMINFO_DIRS)"'
UPDATE_VERSION_SCRIPT = $(HOME)/bin/update_changes_version

#---------------------------------------------------------------------------
# Rules
#---------------------------------------------------------------------------
all: static elf
static: $(OBJDIR) $(CONFIG_H) $(OBJDIR_NORMAL_LIB) $(OTHERSTUFF)
elf: $(ELFDIR) $(CONFIG_H) $(ELFDIR_ELF_LIB)

$(OBJDIR_NORMAL_LIB): $(OBJDIR) $(CONFIG_H) $(OBJS)
	-$(RM) $(OBJDIR_NORMAL_LIB)
	cd $(OBJDIR); $(AR_CR) $(NORMAL_LIB) $(OFILES)
	$(RANLIB) $(OBJDIR_NORMAL_LIB)
	@echo ""
	@echo $(NORMAL_LIB) created in $(OBJDIR)

$(ELFDIR_ELF_LIB): $(ELFDIR) $(CONFIG_H) $(ELFOBJS) $(VERSION_SCRIPT)
	-$(RM) $(ELFDIR_ELF_LIB)
	cd $(ELFDIR); $(ELF_LINK_CMD) -o $(ELFLIB_BUILD_NAME) $(OFILES) $(LDFLAGS) $(ELF_DEP_LIBS)
	if [ $(ELFLIB_MAJOR) != $(ELFLIB_BUILD_NAME) ]; then \
	  cd $(ELFDIR); $(RM) $(ELFLIB_MAJOR); $(LN) $(ELFLIB_BUILD_NAME) $(ELFLIB_MAJOR); \
	fi
	cd $(ELFDIR); $(RM) $(ELFLIB); $(LN) $(ELFLIB_BUILD_NAME) $(ELFLIB)
	@echo ""
	@echo $(ELFLIB_BUILD_NAME) created in $(ELFDIR).
	@echo The link $(ELFLIB) to $(ELFLIB_BUILD_NAME) was also created.
	@echo ""
$(OBJDIR) :
	-$(MKDIR) $(OBJDIR)
$(ELFDIR) :
	-$(MKDIR) $(ELFDIR)
$(CONFIG_H) : sysconf.h
	-$(CP) sysconf.h $(CONFIG_H)

#---------------------------------------------------------------------------
# Intallation rules
#---------------------------------------------------------------------------
$(DEST_DOCDIR):
	$(MKINSDIR) $(DEST_DOCDIR)
$(DEST_INCDIR):
	$(MKINSDIR) $(DEST_INCDIR)
$(DEST_LIBDIR):
	$(MKINSDIR) $(DEST_LIBDIR)
install-headers: $(DEST_INCDIR)
	@echo installing slang.h and slcurses.h in $(DEST_INCDIR)/
	$(INSTALL_DATA) $(SRCDIR)/slang.h $(DEST_INCDIR)/
	$(INSTALL_DATA) $(SRCDIR)/slcurses.h $(DEST_INCDIR)/
install-docs: $(DEST_DOCDIR)
	@for i in $(DOC_FILES); \
	do \
		echo $(INSTALL_DATA) $$i $(DEST_DOCDIR)/; \
		$(INSTALL_DATA) $$i $(DEST_DOCDIR)/; \
	done
install-static: static install-headers install-docs $(DEST_LIBDIR)
	@echo installing $(OBJDIR_NORMAL_LIB) in $(DEST_LIBDIR)/
	$(INSTALL_DATA) $(OBJDIR_NORMAL_LIB) $(DEST_LIBDIR)/
	$(RANLIB) $(DEST_LIBDIR)/$(NORMAL_LIB)
install-elf-and-links:
	-$(RM) $(DEST_LIBDIR)/$(ELFLIB)
	-$(RM) $(DEST_LIBDIR)/$(ELFLIB_MAJOR)
	@echo installing $(ELFLIB_BUILD_NAME) in $(DEST_LIBDIR)
	$(INSTALL_DATA) $(ELFDIR_ELF_LIB) $(DEST_LIBDIR)/
	-cd $(DEST_LIBDIR); $(LN) $(ELFLIB_BUILD_NAME) $(ELFLIB_MAJOR)
	-cd $(DEST_LIBDIR); $(LN) $(ELFLIB_BUILD_NAME) $(ELFLIB)
	@echo ""
	@echo $(ELFLIB_BUILD_NAME) created in $(DEST_LIBDIR).
	@echo The links $(ELFLIB) and $(ELFLIB_MAJOR) to $(ELFLIB_BUILD_NAME) were also created.
	@echo ""
# On cygwin, the DLL must be installed in the bin directory and the import-lib
# goes in the lib directory
install-elf-cygwin:
	$(INSTALL_DATA) $(ELFDIR_DLL_LIB) $(DEST_LIBDIR)/
	$(MKINSDIR) $(DEST_BINDIR)
	$(INSTALL) $(ELFDIR_ELF_LIB) $(DEST_BINDIR)/
install-elf: elf install-headers install-docs $(DEST_LIBDIR) $(INSTALL_ELFLIB_TARGET)
install-all: install-elf install-static
install: install-all
#---------------------------------------------------------------------------
# Tests
#---------------------------------------------------------------------------
runtests: $(OBJDIR_NORMAL_LIB)
	cd test; $(MAKE) CC="$(CC)" CFLAGS="$(CFLAGS)" OTHERLIBS="$(OTHERLIBS)" SLANGLIB="$(OBJDIR)"
#---------------------------------------------------------------------------
# Housekeeping
#---------------------------------------------------------------------------
elf-clean:
	-$(RM) $(ELFDIR)/*
clean: elf-clean
	-$(RM) *~ "#"*
	-$(RM) $(OBJDIR)/*
	-$(RM) $(ELFDIR)/*
distclean: clean
	-$(RM_R) $(OBJDIR) $(ELFDIR) Makefile sysconf.h $(CONFIG_H)

# The symlinks target is for my own private use.  It simply creates the object
# directory as a symbolic link to a local disk instead of an NFS mounted one.
symlinks:
	-/bin/rm -f $(ARCH)objs
	mkdir -p $(HOME)/sys/$(ARCH)/objs/slang/src
	ln -s $(HOME)/sys/$(ARCH)/objs/slang/src $(ARCH)objs
	@echo "Also try: make elfsymlinks"
elfsymlinks: symlinks
	-/bin/rm -f elf$(ARCH)objs
	mkdir -p $(HOME)/sys/$(ARCH)/objs/slang/src/elf
	ln -s $(HOME)/sys/$(ARCH)/objs/slang/src/elf elf$(ARCH)objs
slang.h: $(SRCDIR)/../changes.txt
	if [ -x $(UPDATE_VERSION_SCRIPT) ]; then \
	  $(UPDATE_VERSION_SCRIPT) $(SRCDIR)/../changes.txt $(SRCDIR)/slang.h; \
	fi
#
#---------------------------------------------------------------------------
# Object Rules : These are created from the configure script (hopefully)
#---------------------------------------------------------------------------

$(OBJDIR)/sltermin.o : $(SRCDIR)/sltermin.c $(DOT_O_DEPS) $(sltermin_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(sltermin_C_FLAGS) $(SRCDIR)/sltermin.c

$(OBJDIR)/sldisply.o : $(SRCDIR)/sldisply.c $(DOT_O_DEPS) $(sldisply_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(sldisply_C_FLAGS) $(SRCDIR)/sldisply.c

$(OBJDIR)/slutty.o : $(SRCDIR)/slutty.c $(DOT_O_DEPS) $(slutty_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slutty_C_FLAGS) $(SRCDIR)/slutty.c

$(OBJDIR)/slang.o : $(SRCDIR)/slang.c $(DOT_O_DEPS) $(slang_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slang_C_FLAGS) $(SRCDIR)/slang.c

$(OBJDIR)/slarray.o : $(SRCDIR)/slarray.c $(DOT_O_DEPS) $(slarray_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slarray_C_FLAGS) $(SRCDIR)/slarray.c

$(OBJDIR)/slclass.o : $(SRCDIR)/slclass.c $(DOT_O_DEPS) $(slclass_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slclass_C_FLAGS) $(SRCDIR)/slclass.c

$(OBJDIR)/slcmd.o : $(SRCDIR)/slcmd.c $(DOT_O_DEPS) $(slcmd_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slcmd_C_FLAGS) $(SRCDIR)/slcmd.c

$(OBJDIR)/slerr.o : $(SRCDIR)/slerr.c $(DOT_O_DEPS) $(slerr_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slerr_C_FLAGS) $(SRCDIR)/slerr.c

$(OBJDIR)/slgetkey.o : $(SRCDIR)/slgetkey.c $(DOT_O_DEPS) $(slgetkey_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slgetkey_C_FLAGS) $(SRCDIR)/slgetkey.c

$(OBJDIR)/slkeymap.o : $(SRCDIR)/slkeymap.c $(DOT_O_DEPS) $(slkeymap_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slkeymap_C_FLAGS) $(SRCDIR)/slkeymap.c

$(OBJDIR)/slmalloc.o : $(SRCDIR)/slmalloc.c $(DOT_O_DEPS) $(slmalloc_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slmalloc_C_FLAGS) $(SRCDIR)/slmalloc.c

$(OBJDIR)/slmath.o : $(SRCDIR)/slmath.c $(DOT_O_DEPS) $(slmath_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slmath_C_FLAGS) $(SRCDIR)/slmath.c

$(OBJDIR)/slmemchr.o : $(SRCDIR)/slmemchr.c $(DOT_O_DEPS) $(slmemchr_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slmemchr_C_FLAGS) $(SRCDIR)/slmemchr.c

$(OBJDIR)/slmemcmp.o : $(SRCDIR)/slmemcmp.c $(DOT_O_DEPS) $(slmemcmp_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slmemcmp_C_FLAGS) $(SRCDIR)/slmemcmp.c

$(OBJDIR)/slmemcpy.o : $(SRCDIR)/slmemcpy.c $(DOT_O_DEPS) $(slmemcpy_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slmemcpy_C_FLAGS) $(SRCDIR)/slmemcpy.c

$(OBJDIR)/slmemset.o : $(SRCDIR)/slmemset.c $(DOT_O_DEPS) $(slmemset_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slmemset_C_FLAGS) $(SRCDIR)/slmemset.c

$(OBJDIR)/slmisc.o : $(SRCDIR)/slmisc.c $(DOT_O_DEPS) $(slmisc_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slmisc_C_FLAGS) $(SRCDIR)/slmisc.c

$(OBJDIR)/slparse.o : $(SRCDIR)/slparse.c $(DOT_O_DEPS) $(slparse_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slparse_C_FLAGS) $(SRCDIR)/slparse.c

$(OBJDIR)/slprepr.o : $(SRCDIR)/slprepr.c $(DOT_O_DEPS) $(slprepr_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slprepr_C_FLAGS) $(SRCDIR)/slprepr.c

$(OBJDIR)/slregexp.o : $(SRCDIR)/slregexp.c $(DOT_O_DEPS) $(slregexp_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slregexp_C_FLAGS) $(SRCDIR)/slregexp.c

$(OBJDIR)/slrline.o : $(SRCDIR)/slrline.c $(DOT_O_DEPS) $(slrline_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slrline_C_FLAGS) $(SRCDIR)/slrline.c

$(OBJDIR)/slsearch.o : $(SRCDIR)/slsearch.c $(DOT_O_DEPS) $(slsearch_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slsearch_C_FLAGS) $(SRCDIR)/slsearch.c

$(OBJDIR)/slsmg.o : $(SRCDIR)/slsmg.c $(DOT_O_DEPS) $(slsmg_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slsmg_C_FLAGS) $(SRCDIR)/slsmg.c

$(OBJDIR)/slstd.o : $(SRCDIR)/slstd.c $(DOT_O_DEPS) $(slstd_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slstd_C_FLAGS) $(SRCDIR)/slstd.c

$(OBJDIR)/sltoken.o : $(SRCDIR)/sltoken.c $(DOT_O_DEPS) $(sltoken_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(sltoken_C_FLAGS) $(SRCDIR)/sltoken.c

$(OBJDIR)/sltypes.o : $(SRCDIR)/sltypes.c $(DOT_O_DEPS) $(sltypes_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(sltypes_C_FLAGS) $(SRCDIR)/sltypes.c

$(OBJDIR)/slxstrng.o : $(SRCDIR)/slxstrng.c $(DOT_O_DEPS) $(slxstrng_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slxstrng_C_FLAGS) $(SRCDIR)/slxstrng.c

$(OBJDIR)/slcurses.o : $(SRCDIR)/slcurses.c $(DOT_O_DEPS) $(slcurses_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slcurses_C_FLAGS) $(SRCDIR)/slcurses.c

$(OBJDIR)/slscroll.o : $(SRCDIR)/slscroll.c $(DOT_O_DEPS) $(slscroll_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slscroll_C_FLAGS) $(SRCDIR)/slscroll.c

$(OBJDIR)/slsignal.o : $(SRCDIR)/slsignal.c $(DOT_O_DEPS) $(slsignal_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slsignal_C_FLAGS) $(SRCDIR)/slsignal.c

$(OBJDIR)/slkeypad.o : $(SRCDIR)/slkeypad.c $(DOT_O_DEPS) $(slkeypad_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slkeypad_C_FLAGS) $(SRCDIR)/slkeypad.c

$(OBJDIR)/slerrno.o : $(SRCDIR)/slerrno.c $(DOT_O_DEPS) $(slerrno_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slerrno_C_FLAGS) $(SRCDIR)/slerrno.c

$(OBJDIR)/slstring.o : $(SRCDIR)/slstring.c $(DOT_O_DEPS) $(slstring_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slstring_C_FLAGS) $(SRCDIR)/slstring.c

$(OBJDIR)/slstruct.o : $(SRCDIR)/slstruct.c $(DOT_O_DEPS) $(slstruct_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slstruct_C_FLAGS) $(SRCDIR)/slstruct.c

$(OBJDIR)/slcmplex.o : $(SRCDIR)/slcmplex.c $(DOT_O_DEPS) $(slcmplex_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slcmplex_C_FLAGS) $(SRCDIR)/slcmplex.c

$(OBJDIR)/slarrfun.o : $(SRCDIR)/slarrfun.c $(DOT_O_DEPS) $(slarrfun_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slarrfun_C_FLAGS) $(SRCDIR)/slarrfun.c

$(OBJDIR)/slimport.o : $(SRCDIR)/slimport.c $(DOT_O_DEPS) $(slimport_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slimport_C_FLAGS) $(SRCDIR)/slimport.c

$(OBJDIR)/slpath.o : $(SRCDIR)/slpath.c $(DOT_O_DEPS) $(slpath_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slpath_C_FLAGS) $(SRCDIR)/slpath.c

$(OBJDIR)/slarith.o : $(SRCDIR)/slarith.c $(DOT_O_DEPS) $(slarith_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slarith_C_FLAGS) $(SRCDIR)/slarith.c

$(OBJDIR)/slassoc.o : $(SRCDIR)/slassoc.c $(DOT_O_DEPS) $(slassoc_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slassoc_C_FLAGS) $(SRCDIR)/slassoc.c

$(OBJDIR)/slcompat.o : $(SRCDIR)/slcompat.c $(DOT_O_DEPS) $(slcompat_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slcompat_C_FLAGS) $(SRCDIR)/slcompat.c

$(OBJDIR)/slposdir.o : $(SRCDIR)/slposdir.c $(DOT_O_DEPS) $(slposdir_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slposdir_C_FLAGS) $(SRCDIR)/slposdir.c

$(OBJDIR)/slstdio.o : $(SRCDIR)/slstdio.c $(DOT_O_DEPS) $(slstdio_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slstdio_C_FLAGS) $(SRCDIR)/slstdio.c

$(OBJDIR)/slproc.o : $(SRCDIR)/slproc.c $(DOT_O_DEPS) $(slproc_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slproc_C_FLAGS) $(SRCDIR)/slproc.c

$(OBJDIR)/sltime.o : $(SRCDIR)/sltime.c $(DOT_O_DEPS) $(sltime_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(sltime_C_FLAGS) $(SRCDIR)/sltime.c

$(OBJDIR)/slstrops.o : $(SRCDIR)/slstrops.c $(DOT_O_DEPS) $(slstrops_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slstrops_C_FLAGS) $(SRCDIR)/slstrops.c

$(OBJDIR)/slbstr.o : $(SRCDIR)/slbstr.c $(DOT_O_DEPS) $(slbstr_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slbstr_C_FLAGS) $(SRCDIR)/slbstr.c

$(OBJDIR)/slpack.o : $(SRCDIR)/slpack.c $(DOT_O_DEPS) $(slpack_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slpack_C_FLAGS) $(SRCDIR)/slpack.c

$(OBJDIR)/slintall.o : $(SRCDIR)/slintall.c $(DOT_O_DEPS) $(slintall_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slintall_C_FLAGS) $(SRCDIR)/slintall.c

$(OBJDIR)/slistruc.o : $(SRCDIR)/slistruc.c $(DOT_O_DEPS) $(slistruc_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slistruc_C_FLAGS) $(SRCDIR)/slistruc.c

$(OBJDIR)/slposio.o : $(SRCDIR)/slposio.c $(DOT_O_DEPS) $(slposio_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slposio_C_FLAGS) $(SRCDIR)/slposio.c

$(OBJDIR)/slnspace.o : $(SRCDIR)/slnspace.c $(DOT_O_DEPS) $(slnspace_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slnspace_C_FLAGS) $(SRCDIR)/slnspace.c

$(OBJDIR)/slarrmis.o : $(SRCDIR)/slarrmis.c $(DOT_O_DEPS) $(slarrmis_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slarrmis_C_FLAGS) $(SRCDIR)/slarrmis.c

$(OBJDIR)/slospath.o : $(SRCDIR)/slospath.c $(DOT_O_DEPS) $(slospath_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slospath_C_FLAGS) $(SRCDIR)/slospath.c

$(OBJDIR)/slscanf.o : $(SRCDIR)/slscanf.c $(DOT_O_DEPS) $(slscanf_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slscanf_C_FLAGS) $(SRCDIR)/slscanf.c

$(OBJDIR)/sllower.o : $(SRCDIR)/sllower.c $(DOT_O_DEPS) $(sllower_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(sllower_C_FLAGS) $(SRCDIR)/sllower.c

$(OBJDIR)/slupper.o : $(SRCDIR)/slupper.c $(DOT_O_DEPS) $(slupper_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slupper_C_FLAGS) $(SRCDIR)/slupper.c

$(OBJDIR)/slischar.o : $(SRCDIR)/slischar.c $(DOT_O_DEPS) $(slischar_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slischar_C_FLAGS) $(SRCDIR)/slischar.c

$(OBJDIR)/slutf8.o : $(SRCDIR)/slutf8.c $(DOT_O_DEPS) $(slutf8_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slutf8_C_FLAGS) $(SRCDIR)/slutf8.c

$(OBJDIR)/slwcwidth.o : $(SRCDIR)/slwcwidth.c $(DOT_O_DEPS) $(slwcwidth_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slwcwidth_C_FLAGS) $(SRCDIR)/slwcwidth.c

$(OBJDIR)/slwclut.o : $(SRCDIR)/slwclut.c $(DOT_O_DEPS) $(slwclut_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slwclut_C_FLAGS) $(SRCDIR)/slwclut.c

$(OBJDIR)/slcommon.o : $(SRCDIR)/slcommon.c $(DOT_O_DEPS) $(slcommon_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slcommon_C_FLAGS) $(SRCDIR)/slcommon.c

$(OBJDIR)/sllist.o : $(SRCDIR)/sllist.c $(DOT_O_DEPS) $(sllist_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(sllist_C_FLAGS) $(SRCDIR)/sllist.c

$(OBJDIR)/slexcept.o : $(SRCDIR)/slexcept.c $(DOT_O_DEPS) $(slexcept_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slexcept_C_FLAGS) $(SRCDIR)/slexcept.c

$(OBJDIR)/slfpu.o : $(SRCDIR)/slfpu.c $(DOT_O_DEPS) $(slfpu_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slfpu_C_FLAGS) $(SRCDIR)/slfpu.c

$(OBJDIR)/slsig.o : $(SRCDIR)/slsig.c $(DOT_O_DEPS) $(slsig_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slsig_C_FLAGS) $(SRCDIR)/slsig.c

$(OBJDIR)/slboseos.o : $(SRCDIR)/slboseos.c $(DOT_O_DEPS) $(slboseos_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(slboseos_C_FLAGS) $(SRCDIR)/slboseos.c

calc : $(OBJDIR)/calc
	@echo calc created in $(OBJDIR)
$(OBJDIR)/calc : $(OBJDIR)/calc.o $(calc_DEPS) $(EXECDEPS)
	$(CC) -o $(OBJDIR)/calc $(LDFLAGS) $(OBJDIR)/calc.o $(calc_LIBS) $(EXECLIBS)
$(OBJDIR)/calc.o : $(SRCDIR)/calc.c $(DOT_O_DEPS) $(calc_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(calc_INC) $(EXECINC) $(SRCDIR)/calc.c

untic : $(OBJDIR)/untic
	@echo untic created in $(OBJDIR)
$(OBJDIR)/untic : $(OBJDIR)/untic.o $(untic_DEPS) $(EXECDEPS)
	$(CC) -o $(OBJDIR)/untic $(LDFLAGS) $(OBJDIR)/untic.o $(untic_LIBS) $(EXECLIBS)
$(OBJDIR)/untic.o : $(SRCDIR)/untic.c $(DOT_O_DEPS) $(untic_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(untic_INC) $(EXECINC) $(SRCDIR)/untic.c


$(ELFDIR)/sltermin.o : $(SRCDIR)/sltermin.c $(DOT_O_DEPS) $(sltermin_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(sltermin_C_FLAGS) $(SRCDIR)/sltermin.c

$(ELFDIR)/sldisply.o : $(SRCDIR)/sldisply.c $(DOT_O_DEPS) $(sldisply_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(sldisply_C_FLAGS) $(SRCDIR)/sldisply.c

$(ELFDIR)/slutty.o : $(SRCDIR)/slutty.c $(DOT_O_DEPS) $(slutty_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slutty_C_FLAGS) $(SRCDIR)/slutty.c

$(ELFDIR)/slang.o : $(SRCDIR)/slang.c $(DOT_O_DEPS) $(slang_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slang_C_FLAGS) $(SRCDIR)/slang.c

$(ELFDIR)/slarray.o : $(SRCDIR)/slarray.c $(DOT_O_DEPS) $(slarray_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slarray_C_FLAGS) $(SRCDIR)/slarray.c

$(ELFDIR)/slclass.o : $(SRCDIR)/slclass.c $(DOT_O_DEPS) $(slclass_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slclass_C_FLAGS) $(SRCDIR)/slclass.c

$(ELFDIR)/slcmd.o : $(SRCDIR)/slcmd.c $(DOT_O_DEPS) $(slcmd_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slcmd_C_FLAGS) $(SRCDIR)/slcmd.c

$(ELFDIR)/slerr.o : $(SRCDIR)/slerr.c $(DOT_O_DEPS) $(slerr_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slerr_C_FLAGS) $(SRCDIR)/slerr.c

$(ELFDIR)/slgetkey.o : $(SRCDIR)/slgetkey.c $(DOT_O_DEPS) $(slgetkey_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slgetkey_C_FLAGS) $(SRCDIR)/slgetkey.c

$(ELFDIR)/slkeymap.o : $(SRCDIR)/slkeymap.c $(DOT_O_DEPS) $(slkeymap_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slkeymap_C_FLAGS) $(SRCDIR)/slkeymap.c

$(ELFDIR)/slmalloc.o : $(SRCDIR)/slmalloc.c $(DOT_O_DEPS) $(slmalloc_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slmalloc_C_FLAGS) $(SRCDIR)/slmalloc.c

$(ELFDIR)/slmath.o : $(SRCDIR)/slmath.c $(DOT_O_DEPS) $(slmath_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slmath_C_FLAGS) $(SRCDIR)/slmath.c

$(ELFDIR)/slmemchr.o : $(SRCDIR)/slmemchr.c $(DOT_O_DEPS) $(slmemchr_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slmemchr_C_FLAGS) $(SRCDIR)/slmemchr.c

$(ELFDIR)/slmemcmp.o : $(SRCDIR)/slmemcmp.c $(DOT_O_DEPS) $(slmemcmp_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slmemcmp_C_FLAGS) $(SRCDIR)/slmemcmp.c

$(ELFDIR)/slmemcpy.o : $(SRCDIR)/slmemcpy.c $(DOT_O_DEPS) $(slmemcpy_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slmemcpy_C_FLAGS) $(SRCDIR)/slmemcpy.c

$(ELFDIR)/slmemset.o : $(SRCDIR)/slmemset.c $(DOT_O_DEPS) $(slmemset_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slmemset_C_FLAGS) $(SRCDIR)/slmemset.c

$(ELFDIR)/slmisc.o : $(SRCDIR)/slmisc.c $(DOT_O_DEPS) $(slmisc_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slmisc_C_FLAGS) $(SRCDIR)/slmisc.c

$(ELFDIR)/slparse.o : $(SRCDIR)/slparse.c $(DOT_O_DEPS) $(slparse_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slparse_C_FLAGS) $(SRCDIR)/slparse.c

$(ELFDIR)/slprepr.o : $(SRCDIR)/slprepr.c $(DOT_O_DEPS) $(slprepr_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slprepr_C_FLAGS) $(SRCDIR)/slprepr.c

$(ELFDIR)/slregexp.o : $(SRCDIR)/slregexp.c $(DOT_O_DEPS) $(slregexp_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slregexp_C_FLAGS) $(SRCDIR)/slregexp.c

$(ELFDIR)/slrline.o : $(SRCDIR)/slrline.c $(DOT_O_DEPS) $(slrline_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slrline_C_FLAGS) $(SRCDIR)/slrline.c

$(ELFDIR)/slsearch.o : $(SRCDIR)/slsearch.c $(DOT_O_DEPS) $(slsearch_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slsearch_C_FLAGS) $(SRCDIR)/slsearch.c

$(ELFDIR)/slsmg.o : $(SRCDIR)/slsmg.c $(DOT_O_DEPS) $(slsmg_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slsmg_C_FLAGS) $(SRCDIR)/slsmg.c

$(ELFDIR)/slstd.o : $(SRCDIR)/slstd.c $(DOT_O_DEPS) $(slstd_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slstd_C_FLAGS) $(SRCDIR)/slstd.c

$(ELFDIR)/sltoken.o : $(SRCDIR)/sltoken.c $(DOT_O_DEPS) $(sltoken_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(sltoken_C_FLAGS) $(SRCDIR)/sltoken.c

$(ELFDIR)/sltypes.o : $(SRCDIR)/sltypes.c $(DOT_O_DEPS) $(sltypes_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(sltypes_C_FLAGS) $(SRCDIR)/sltypes.c

$(ELFDIR)/slxstrng.o : $(SRCDIR)/slxstrng.c $(DOT_O_DEPS) $(slxstrng_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slxstrng_C_FLAGS) $(SRCDIR)/slxstrng.c

$(ELFDIR)/slcurses.o : $(SRCDIR)/slcurses.c $(DOT_O_DEPS) $(slcurses_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slcurses_C_FLAGS) $(SRCDIR)/slcurses.c

$(ELFDIR)/slscroll.o : $(SRCDIR)/slscroll.c $(DOT_O_DEPS) $(slscroll_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slscroll_C_FLAGS) $(SRCDIR)/slscroll.c

$(ELFDIR)/slsignal.o : $(SRCDIR)/slsignal.c $(DOT_O_DEPS) $(slsignal_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slsignal_C_FLAGS) $(SRCDIR)/slsignal.c

$(ELFDIR)/slkeypad.o : $(SRCDIR)/slkeypad.c $(DOT_O_DEPS) $(slkeypad_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slkeypad_C_FLAGS) $(SRCDIR)/slkeypad.c

$(ELFDIR)/slerrno.o : $(SRCDIR)/slerrno.c $(DOT_O_DEPS) $(slerrno_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slerrno_C_FLAGS) $(SRCDIR)/slerrno.c

$(ELFDIR)/slstring.o : $(SRCDIR)/slstring.c $(DOT_O_DEPS) $(slstring_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slstring_C_FLAGS) $(SRCDIR)/slstring.c

$(ELFDIR)/slstruct.o : $(SRCDIR)/slstruct.c $(DOT_O_DEPS) $(slstruct_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slstruct_C_FLAGS) $(SRCDIR)/slstruct.c

$(ELFDIR)/slcmplex.o : $(SRCDIR)/slcmplex.c $(DOT_O_DEPS) $(slcmplex_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slcmplex_C_FLAGS) $(SRCDIR)/slcmplex.c

$(ELFDIR)/slarrfun.o : $(SRCDIR)/slarrfun.c $(DOT_O_DEPS) $(slarrfun_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slarrfun_C_FLAGS) $(SRCDIR)/slarrfun.c

$(ELFDIR)/slimport.o : $(SRCDIR)/slimport.c $(DOT_O_DEPS) $(slimport_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slimport_C_FLAGS) $(SRCDIR)/slimport.c

$(ELFDIR)/slpath.o : $(SRCDIR)/slpath.c $(DOT_O_DEPS) $(slpath_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slpath_C_FLAGS) $(SRCDIR)/slpath.c

$(ELFDIR)/slarith.o : $(SRCDIR)/slarith.c $(DOT_O_DEPS) $(slarith_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slarith_C_FLAGS) $(SRCDIR)/slarith.c

$(ELFDIR)/slassoc.o : $(SRCDIR)/slassoc.c $(DOT_O_DEPS) $(slassoc_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slassoc_C_FLAGS) $(SRCDIR)/slassoc.c

$(ELFDIR)/slcompat.o : $(SRCDIR)/slcompat.c $(DOT_O_DEPS) $(slcompat_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slcompat_C_FLAGS) $(SRCDIR)/slcompat.c

$(ELFDIR)/slposdir.o : $(SRCDIR)/slposdir.c $(DOT_O_DEPS) $(slposdir_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slposdir_C_FLAGS) $(SRCDIR)/slposdir.c

$(ELFDIR)/slstdio.o : $(SRCDIR)/slstdio.c $(DOT_O_DEPS) $(slstdio_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slstdio_C_FLAGS) $(SRCDIR)/slstdio.c

$(ELFDIR)/slproc.o : $(SRCDIR)/slproc.c $(DOT_O_DEPS) $(slproc_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slproc_C_FLAGS) $(SRCDIR)/slproc.c

$(ELFDIR)/sltime.o : $(SRCDIR)/sltime.c $(DOT_O_DEPS) $(sltime_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(sltime_C_FLAGS) $(SRCDIR)/sltime.c

$(ELFDIR)/slstrops.o : $(SRCDIR)/slstrops.c $(DOT_O_DEPS) $(slstrops_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slstrops_C_FLAGS) $(SRCDIR)/slstrops.c

$(ELFDIR)/slbstr.o : $(SRCDIR)/slbstr.c $(DOT_O_DEPS) $(slbstr_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slbstr_C_FLAGS) $(SRCDIR)/slbstr.c

$(ELFDIR)/slpack.o : $(SRCDIR)/slpack.c $(DOT_O_DEPS) $(slpack_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slpack_C_FLAGS) $(SRCDIR)/slpack.c

$(ELFDIR)/slintall.o : $(SRCDIR)/slintall.c $(DOT_O_DEPS) $(slintall_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slintall_C_FLAGS) $(SRCDIR)/slintall.c

$(ELFDIR)/slistruc.o : $(SRCDIR)/slistruc.c $(DOT_O_DEPS) $(slistruc_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slistruc_C_FLAGS) $(SRCDIR)/slistruc.c

$(ELFDIR)/slposio.o : $(SRCDIR)/slposio.c $(DOT_O_DEPS) $(slposio_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slposio_C_FLAGS) $(SRCDIR)/slposio.c

$(ELFDIR)/slnspace.o : $(SRCDIR)/slnspace.c $(DOT_O_DEPS) $(slnspace_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slnspace_C_FLAGS) $(SRCDIR)/slnspace.c

$(ELFDIR)/slarrmis.o : $(SRCDIR)/slarrmis.c $(DOT_O_DEPS) $(slarrmis_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slarrmis_C_FLAGS) $(SRCDIR)/slarrmis.c

$(ELFDIR)/slospath.o : $(SRCDIR)/slospath.c $(DOT_O_DEPS) $(slospath_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slospath_C_FLAGS) $(SRCDIR)/slospath.c

$(ELFDIR)/slscanf.o : $(SRCDIR)/slscanf.c $(DOT_O_DEPS) $(slscanf_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slscanf_C_FLAGS) $(SRCDIR)/slscanf.c

$(ELFDIR)/sllower.o : $(SRCDIR)/sllower.c $(DOT_O_DEPS) $(sllower_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(sllower_C_FLAGS) $(SRCDIR)/sllower.c

$(ELFDIR)/slupper.o : $(SRCDIR)/slupper.c $(DOT_O_DEPS) $(slupper_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slupper_C_FLAGS) $(SRCDIR)/slupper.c

$(ELFDIR)/slischar.o : $(SRCDIR)/slischar.c $(DOT_O_DEPS) $(slischar_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slischar_C_FLAGS) $(SRCDIR)/slischar.c

$(ELFDIR)/slutf8.o : $(SRCDIR)/slutf8.c $(DOT_O_DEPS) $(slutf8_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slutf8_C_FLAGS) $(SRCDIR)/slutf8.c

$(ELFDIR)/slwcwidth.o : $(SRCDIR)/slwcwidth.c $(DOT_O_DEPS) $(slwcwidth_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slwcwidth_C_FLAGS) $(SRCDIR)/slwcwidth.c

$(ELFDIR)/slwclut.o : $(SRCDIR)/slwclut.c $(DOT_O_DEPS) $(slwclut_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slwclut_C_FLAGS) $(SRCDIR)/slwclut.c

$(ELFDIR)/slcommon.o : $(SRCDIR)/slcommon.c $(DOT_O_DEPS) $(slcommon_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slcommon_C_FLAGS) $(SRCDIR)/slcommon.c

$(ELFDIR)/sllist.o : $(SRCDIR)/sllist.c $(DOT_O_DEPS) $(sllist_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(sllist_C_FLAGS) $(SRCDIR)/sllist.c

$(ELFDIR)/slexcept.o : $(SRCDIR)/slexcept.c $(DOT_O_DEPS) $(slexcept_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slexcept_C_FLAGS) $(SRCDIR)/slexcept.c

$(ELFDIR)/slfpu.o : $(SRCDIR)/slfpu.c $(DOT_O_DEPS) $(slfpu_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slfpu_C_FLAGS) $(SRCDIR)/slfpu.c

$(ELFDIR)/slsig.o : $(SRCDIR)/slsig.c $(DOT_O_DEPS) $(slsig_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slsig_C_FLAGS) $(SRCDIR)/slsig.c

$(ELFDIR)/slboseos.o : $(SRCDIR)/slboseos.c $(DOT_O_DEPS) $(slboseos_O_DEP)
	cd $(ELFDIR); $(ELFCOMPILE_CMD) $(slboseos_C_FLAGS) $(SRCDIR)/slboseos.c

