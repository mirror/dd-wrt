# -*- sh -*-
MODULES = slsmg-module.so termios-module.so select-module.so fcntl-module.so \
  varray-module.so socket-module.so rand-module.so fork-module.so   \
  iconv-module.so   sysconf-module.so
SLFILES = slsmg termios select fcntl varray socket pcre png iconv \
  onig rand zlib fork sysconf
#---------------------------------------------------------------------------
CC		= mips-linux-uclibc-gcc
CFLAGS		= -Os -pipe -mips32r2 -mtune=mips32r2 -funit-at-a-time -fPIC  
LDFLAGS		= 
CC_SHARED 	= $(CC) $(CFLAGS) -shared -fPIC
#---------------------------------------------------------------------------
# Installation location of the slang library
#---------------------------------------------------------------------------
prefix 		= /usr/local
exec_prefix 	= ${prefix}
SLANG_INC	= -I../src#-I${prefix}/include
#SLANG_INC	= #-I${prefix}/include
#SLANG_LIB	= #-L${exec_prefix}/lib -lslang
ELFDIR		= /home/seg/DEV/pb42/src/router/mc/slang/src/elfmipsobjs
DLL_IMPLIB_NAME = 
# On some systems, the module must be linked to the slang library,
# but not all do.  If you have such system, point SLANG_LIB to the 
# slang library, otherwise leave it blank.
SLANG_LIB	= 

#---------------------------------------------------------------------------
# If building the PCRE module, you may need to set these
#---------------------------------------------------------------------------
PCRE_INC	= 
PCRE_LIB	=  -lpcre
#---------------------------------------------------------------------------
# If building the PNG module, you may need to set these
#---------------------------------------------------------------------------
PNG_INC	= 
PNG_LIB	=  -lpng
#---------------------------------------------------------------------------
# If building the ICONV module, you may need to set these
#---------------------------------------------------------------------------
ICONV_INC	= 
ICONV_LIB	= -liconv
#---------------------------------------------------------------------------
# If building the ONIG module, you may need to set these
#---------------------------------------------------------------------------
ONIG_INC	= 
ONIG_LIB	=  -lonig
#---------------------------------------------------------------------------
# If building the ZLIB module, you may need to set these
#---------------------------------------------------------------------------
ZLIB_INC	= 
ZLIB_LIB	=  -lz
#---------------------------------------------------------------------------
# If building the SOCKET module, you may need to set these
#---------------------------------------------------------------------------
SOCKET_LIBS = 
#---------------------------------------------------------------------------
# Installation location of the modules
#---------------------------------------------------------------------------
MODULE_INSTALL_DIR = ${exec_prefix}/lib/slang/v2/modules
INSTALL		= /usr/bin/install -c
INSTALL_DATA	= ${INSTALL} -m 644
INSTALL_MODULE	= $(INSTALL_DATA)
MKINSDIR	= ../autoconf/mkinsdir.sh
SLSH_DIR	= $(prefix)/share/slsh
CMAPS_DIR	= $(SLSH_DIR)/cmaps
HELP_DIR	= $(SLSH_DIR)/help
#---------------------------------------------------------------------------
# DESTDIR is designed to facilitate making packages.  Normally it is empty
#---------------------------------------------------------------------------
DESTDIR =
DEST_MODULEDIR	= $(DESTDIR)$(MODULE_INSTALL_DIR)
DEST_SLSH_DIR	= $(DESTDIR)$(SLSH_DIR)
DEST_CMAPSDIR	= $(DESTDIR)$(CMAPS_DIR)
DEST_HELPDIR	= $(DESTDIR)$(HELP_DIR)
#---------------------------------------------------------------------------
RPATH = -Wl,-R${exec_prefix}/lib

LIBS = $(LDFLAGS) $(RPATH) $(SLANG_LIB) $(DL_LIB) -lm
#LIBS = $(RPATH) $(DL_LIB) -lm
INCS = $(SLANG_INC)

all: $(MODULES)

config.h: ../src/config.h
	cp ../src/config.h .

slsmg-module.so: slsmg-module.c config.h
	$(CC_SHARED) $(INCS) slsmg-module.c -o slsmg-module.so $(LIBS)
rand-module.so: rand-module.c config.h
	$(CC_SHARED) $(INCS) rand-module.c -o rand-module.so $(LIBS)
newt-module.so: newt-module.c config.h
	$(CC_SHARED) $(INCS) newt-module.c -o newt-module.so -lnewt $(LIBS)
termios-module.so: termios-module.c config.h
	$(CC_SHARED) $(INCS) termios-module.c -o termios-module.so $(LIBS)
select-module.so: select-module.c config.h
	$(CC_SHARED) $(INCS) select-module.c -o select-module.so $(LIBS)
fcntl-module.so: fcntl-module.c config.h
	$(CC_SHARED) $(INCS) fcntl-module.c -o fcntl-module.so $(LIBS)
varray-module.so: varray-module.c config.h
	$(CC_SHARED) $(INCS) varray-module.c -o varray-module.so $(LIBS)
pcre-module.so: pcre-module.c config.h
	$(CC_SHARED) $(INCS) $(PCRE_INC) pcre-module.c -o pcre-module.so $(PCRE_LIB) $(LIBS)
onig-module.so: onig-module.c config.h
	$(CC_SHARED) $(INCS) $(ONIG_INC) onig-module.c -o onig-module.so $(ONIG_LIB) $(LIBS)
png-module.so: png-module.c config.h
	$(CC_SHARED) $(INCS) $(PNG_INC) png-module.c -o png-module.so $(PNG_LIB) $(LIBS)
socket-module.so: socket-module.c config.h
	$(CC_SHARED) $(INCS) socket-module.c -o socket-module.so $(SOCKET_LIBS) $(LIBS)
iconv-module.so: iconv-module.c config.h
	$(CC_SHARED) $(INCS) $(ICONV_INC) iconv-module.c -o iconv-module.so $(ICONV_LIB) $(LIBS)
zlib-module.so: zlib-module.c config.h
	$(CC_SHARED) $(INCS) $(ZLIB_INC) zlib-module.c -o zlib-module.so $(ZLIB_LIB) $(LIBS)
fork-module.so: fork-module.c config.h
	$(CC_SHARED) $(INCS) fork-module.c -o fork-module.so $(LIBS)
sysconf-module.so: sysconf-module.c config.h
	$(CC_SHARED) $(INCS) sysconf-module.c -o sysconf-module.so $(LIBS)

# Note that Windows/Cygwin modules need execute permissions
install: all
	-$(MKINSDIR) $(DEST_MODULEDIR)
	@for i in $(MODULES); \
	do \
		echo $(INSTALL_MODULE) $$i $(DEST_MODULEDIR); \
		$(INSTALL_MODULE) $$i $(DEST_MODULEDIR); \
	done
	-$(MKINSDIR) $(DEST_SLSH_DIR)
	@for i in $(SLFILES); \
	do \
		echo $(INSTALL_DATA) $$i.sl $(DEST_SLSH_DIR); \
		$(INSTALL_DATA) $$i.sl $(DEST_SLSH_DIR); \
	done
	-$(MKINSDIR) $(DEST_CMAPSDIR)
	@for i in cmaps/*.map; \
	do \
		echo $(INSTALL_DATA) $$i $(DEST_CMAPSDIR); \
		$(INSTALL_DATA) $$i $(DEST_CMAPSDIR); \
	done
	-$(MKINSDIR) $(DEST_HELPDIR)
	@for i in help/*.hlp; \
	do \
		echo $(INSTALL_DATA) $$i $(DEST_HELPDIR); \
		$(INSTALL_DATA) $$i $(DEST_HELPDIR); \
	done
clean:
	-/bin/rm -f $(MODULES) *~
distclean: clean
	-/bin/rm -f Makefile

