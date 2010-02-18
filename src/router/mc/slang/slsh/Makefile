CC		= mips-linux-uclibc-gcc
CFLAGS		= -Os -pipe -mips32r2 -mtune=mips32r2 -funit-at-a-time -fPIC  
LDFLAGS		=  -Wl,-export-dynamic
#---------------------------------------------------------------------------
# Installation location of the slang library
#---------------------------------------------------------------------------
prefix 		= /usr/local
exec_prefix 	= ${prefix}
datarootdir	= ${prefix}/share
INST_LIB_DIR	= ${exec_prefix}/lib
SLANG_INST_INC	= -I${prefix}/include
SLANG_INST_LIB	= -L$(INST_LIB_DIR)
#---------------------------------------------------------------------------
SLANG_SRCINC	= -I/home/seg/DEV/pb42/src/router/mc/slang/src
SLANG_SRCLIB	= -L/home/seg/DEV/pb42/src/router/mc/slang/src/elfmipsobjs
#---------------------------------------------------------------------------
OTHER_LIBS	=  -ldl  -lm
RPATH		= -Wl,-R${exec_prefix}/lib
#----------------------------------------------------------------------------
INSTALL		= /usr/bin/install -c
INSTALL_DATA	= ${INSTALL} -m 644
MKINSDIR	= ../autoconf/mkinsdir.sh
#----------------------------------------------------------------------------
# Where system-wide slsh.rc, scripts, and library files reside
#----------------------------------------------------------------------------
SLSH_CONF_DIR	= ${prefix}/etc
SLSH_LIB_DIR	= $(datarootdir)/slsh
SLSH_HELP_DIR	= $(SLSH_LIB_DIR)/help
SLSH_DOC_DIR	= $(datarootdir)/doc/slsh
SLSH_LOCALLIB_DIR = $(SLSH_LIB_DIR)/local-packages
SLSH_SCRIPT_DIR	= $(SLSH_LIB_DIR)/scripts
SCRIPTS		= jpegsize lsrpm sldb svnsh slprof
#---------------------------------------------------------------------------
# Hard-coded paths may be configured during run-time via the SLSH_CONF_DIR
# and SLSH_PATH env variables.  The names of those variables may be
# changed here.  The SLSH_LIB_DIR variable has been deprecated.
#
SLSH_CONF_DIR_ENV = "SLSH_CONF_DIR"
SLSH_PATH_ENV	  = "SLSH_PATH"
SLSH_LIB_DIR_ENV  = "SLSH_LIB_DIR"
#---------------------------------------------------------------------------
# If you want GNU readline instead of slang's readline, then enable it here
#---------------------------------------------------------------------------
GNU_READLINE	= 0
READLINE_LIB	= # -lreadline
#---------------------------------------------------------------------------
# DESTDIR is designed to facilitate making packages.  Normally it is empty
#---------------------------------------------------------------------------
DESTDIR		=
BIN_DIR		= ${exec_prefix}/bin
MAN_DIR		= ${datarootdir}/man/man1
DEST_LIB_DIR	= -L$(DESTDIR)${exec_prefix}/lib
DEST_BIN_DIR	= $(DESTDIR)$(BIN_DIR)
DEST_MAN_DIR	= $(DESTDIR)$(MAN_DIR)
DEST_SLSH_CONF_DIR = $(DESTDIR)$(SLSH_CONF_DIR)
DEST_SLSH_LIB_DIR  = $(DESTDIR)$(SLSH_LIB_DIR)
DEST_SLSH_HELP_DIR = $(DESTDIR)$(SLSH_HELP_DIR)
DEST_SLSH_LOCALLIB_DIR = $(DESTDIR)$(SLSH_LOCALLIB_DIR)
DEST_SLSH_SCRIPT_DIR = $(DESTDIR)$(SLSH_SCRIPT_DIR)
DEST_SLSH_DOC_DIR= $(DESTDIR)$(SLSH_DOC_DIR)

#----------------------------------------------------------------------------
# End of user configuration
#----------------------------------------------------------------------------

SHELL = /bin/sh
#INST_LIBS = $(RPATH) $(SLANG_INST_LIB) $(DEST_LIB_DIR) -lslang $(READLINE_LIB) $(OTHER_LIBS)
INST_LIBS = $(DEST_LIB_DIR) $(RPATH) $(SLANG_INST_LIB) -lslang $(READLINE_LIB) $(OTHER_LIBS)
DEFS = -DSLSH_CONF_DIR='"$(SLSH_CONF_DIR)"' -DSLSH_PATH='"$(SLSH_LIB_DIR)"' \
 -DSLSH_CONF_DIR_ENV='$(SLSH_CONF_DIR_ENV)' -DSLSH_LIB_DIR_ENV='$(SLSH_LIB_DIR_ENV)' \
 -DSLSH_PATH_ENV='$(SLSH_PATH_ENV)' 
SRC_LIBS = $(SLANG_SRCLIB) -lslang $(READLINE_LIB) $(OTHER_LIBS)
#
all: slsh_exe
slsh_exe: slsh.o readline.o
	$(CC) $(CFLAGS) slsh.o readline.o -o slsh_exe $(LDFLAGS) $(SRC_LIBS)
slsh: slsh.o readline.o
	$(CC) $(CFLAGS) slsh.o readline.o -o slsh $(LDFLAGS) $(INST_LIBS)
slsh.o: slsh.c slsh.h config.h Makefile
	$(CC) -c $(CFLAGS) $(SLANG_SRCINC) $(DEFS) slsh.c
readline.o: readline.c slsh.h config.h Makefile
	$(CC) -c $(CFLAGS) $(SLANG_SRCINC) $(DEFS) -DUSE_GNU_READLINE=$(GNU_READLINE) readline.c
config.h: ../src/config.h
	cp ../src/config.h .
install_directories:
	$(MKINSDIR) $(DEST_BIN_DIR)
	$(MKINSDIR) $(DEST_MAN_DIR)
	$(MKINSDIR) $(DEST_SLSH_CONF_DIR)
	$(MKINSDIR) $(DEST_SLSH_LIB_DIR)
	$(MKINSDIR) $(DEST_SLSH_LIB_DIR)/rline
	$(MKINSDIR) $(DEST_SLSH_HELP_DIR)
	$(MKINSDIR) $(DEST_SLSH_LOCALLIB_DIR)
	$(MKINSDIR) $(DEST_SLSH_SCRIPT_DIR)
	$(MKINSDIR) $(DEST_SLSH_DOC_DIR)/html
install_lib_files:
	@for X in lib/*.sl; \
	do \
		echo $(INSTALL_DATA) $$X $(DEST_SLSH_LIB_DIR); \
		$(INSTALL_DATA) $$X $(DEST_SLSH_LIB_DIR); \
		if [ "$$?" != "0" ]; then \
		  exit 1; \
		fi; \
	done
install_rline_files:
	@for X in lib/rline/*.sl lib/rline/slrline.rc; \
	do \
		echo $(INSTALL_DATA) $$X $(DEST_SLSH_LIB_DIR)/rline/; \
		$(INSTALL_DATA) $$X $(DEST_SLSH_LIB_DIR)/rline/; \
		if [ "$$?" != "0" ]; then \
		  exit 1; \
		fi; \
	done
install_scripts:
	@for X in $(SCRIPTS); \
	do \
		echo $(INSTALL) scripts/$$X $(DEST_SLSH_SCRIPT_DIR); \
		$(INSTALL) scripts/$$X $(DEST_SLSH_SCRIPT_DIR); \
		if [ "$$?" != "0" ]; then \
		  exit 1; \
		fi; \
	done
install_help:
	@for X in lib/help/*.hlp; \
	do \
		echo $(INSTALL_DATA) $$X $(DEST_SLSH_HELP_DIR); \
		$(INSTALL_DATA) $$X $(DEST_SLSH_HELP_DIR); \
		if [ "$$?" != "0" ]; then \
		  exit 1; \
		fi; \
	done
install_docs:
	@for X in doc/html/*.html; \
	do \
		echo $(INSTALL_DATA) $$X $(DEST_SLSH_DOC_DIR)/html/; \
		$(INSTALL_DATA) $$X $(DEST_SLSH_DOC_DIR)/html/; \
		if [ "$$?" != "0" ]; then \
		  exit 1; \
		fi; \
	done

install: slsh install_directories install_lib_files install_rline_files \
 install_scripts install_help install_docs
	$(INSTALL) slsh $(DEST_BIN_DIR)/
	$(INSTALL_DATA) etc/slsh.rc $(DEST_SLSH_CONF_DIR)/
	echo 'prepend_to_slang_load_path("$(SLSH_LOCALLIB_DIR)");' >> $(DEST_SLSH_CONF_DIR)/slsh.rc
	$(INSTALL_DATA) doc/man/slsh.1 $(DEST_MAN_DIR)/
#---------------------------------------------------------------------------
# Housekeeping
#---------------------------------------------------------------------------
clean:
	-/bin/rm -f *~ slsh.o readline.o slsh slsh_exe scripts/*~ lib/*~
distclean: clean
	-/bin/rm -f Makefile
symlinks:
	-/bin/rm -f $(ARCH)objs
	-mkdir -p $(HOME)/sys/$(ARCH)/objs/jdl/src
	ln -s $(HOME)/sys/$(ARCH)/objs/jdl/src $(ARCH)objs
