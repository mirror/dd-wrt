#
# Makefile for IPTraf 3.0
#

#
# Architecture determination string borrowed from the kernel makefile.
#
#ARCH 		:= $(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc64/ \
			-e s/arm.*/arm/ -e s/sa110/arm/)
#TEST		:= $(PLATFORM)
PLATFORM 	= -DPLATFORM=\"DD-WRT_Linux/$(ARCH)\"
VERNUMBER	:= $(shell cat version)
VERSION 	= -DVERSION=\"$(VERNUMBER)\"

#
# Binary distribution will be placed here.
# Production use only
#
BINDIR		= ../../iptraf-$(VERNUMBER).bin.$(ARCH)

#CC		= gcc
LIBS		= -L../support -L../../ncurses/lib -ltextbox -lpanel -lncurses   # in this order!

# comment this one out to omit debug code when done.

DEBUG		= #-g #-DDEBUG

# comment this one out to prevent generation of profile code

PROF		= #-pg

# options to be passed to the compiler.  I don't believe they need to be
# modified (except for -m486 on non-Intel x86 platforms).

CFLAGS		= -Wall $(COPTS)
DIRS		= -DWORKDIR=\"$(WORKDIR)\" \
		  -DLOGDIR=\"$(LOGDIR)\" -DEXECDIR=\"$(TARGET)\"
LDOPTS		= #-static

# you may want to change this to point to your ncurses include directory
# if the ncurses include files are not in the default location.

INCLUDEDIR	= -I../../ncurses/include -I../support

# You can uncomment this one to disable the backspace key in input fields.
# This means you must use the Del key or Ctrl+H combination to erase the
# character left of the cursor.  You may need to use this directive if you
# have an earlier version of ncurses.  (Please note that earlier ncurses
# versions have quirks that may result in undesirable screen behavior as
# well.)

BSSETTING	=# -DDISABLEBS

# Define this one to allow non-root users to use the program when setuid
# root.  Undefine to restrict use to root only.  It is recommended that
# you restrict execution to root only.  This option does not install the
# executable program with the setuid bit on, or with world-execute
# permissions.  If you want it, you'll have to do it yourself with chmod.
#
# I have no plans to modify this program to be used by other users.

EXECPERM	=# -DALLOWUSERS

###################################################################
############### IPTRAF DIRECTORY DEFINITIONS.  YOU MAY CHANGE THESE
############### TO SUIT YOUR PREFERENCES.
###################################################################

# installation target directory.  The iptraf and rvnamed programs get
# stored here.  iptraf also exec's rvnamed from this directory.

TARGET		= $(INSTALLDIR)/iptraf/usr/sbin

# The IPTraf working directory; if you change this.  Starting with this
# version, this definition overrides dirs.h.

WORKDIR		= /tmp

# The IPTraf log file directory.  IPTraf log files are placed here.
# This definition overrides dirs.h

LOGDIR		= /tmp

#
# IPTraf lock file directory.  This is /var/run/iptraf/ and will not
# be passed to the compiler.  If you want to change this, you must edit
# dirs.h.
#
# ******************** !!!!!! WARNING !!!!!! ********************
# DO NOT MAKE THIS POINT TO AN EXISTING/SYSTEM DIRECTORY!!!!  THE
# LOCK OVERRIDE (iptraf -f) WILL ERASE FILES HERE!
#*****************************************************************
LOCKDIR		= /var/run/iptraf

#####################################################################
############### IPTRAF COMPILATION AND LINK RULES
#####################################################################

# Object file names

OBJS = iptraf.o itrafmon.o packet.o tcptable.o othptab.o ifstats.o deskman.o \
ipcsum.o hostmon.o fltedit.o tr.o cidr.o \
fltselect.o ipfilter.o fltmgr.o ipfrag.o serv.o servname.o instances.o \
timer.o revname.o pktsize.o landesc.o isdntab.o options.o promisc.o ifaces.o \
error.o log.o mode.o getpath.o bar.o parseproto.o

BINS = iptraf rvnamed rawtime

all: $(BINS)
	@echo
	@size $(BINS)

iptraf: $(OBJS) textlib
	$(CC) $(LDOPTS) $(PROF) -o iptraf $(OBJS) $(LIBS)

textlib:
	make -C ../support

%.o: %.c *.h version
	$(CC) $(CFLAGS) $(DIRS) $(INCLUDEDIR) $(VERSION) $(PLATFORM) $(PROF) $(DEBUG) $(EXECPERM) $(BSSETTING) -c -o $*.o $<

rvnamed: rvnamed.o getpath.o
	$(CC) $(LDOPTS) $(PROF) -o rvnamed rvnamed.o getpath.o

rvnamed.o: rvnamed.c rvnamed.h
	$(CC) $(CFLAGS) $(PROF) $(DEBUG) -c -o rvnamed.o rvnamed.c

rawtime: rawtime.c
	$(CC) $(CFLAGS) $(LDOPTS) $(PROF) $(DEBUG) -o rawtime rawtime.c


# rule to clear out all object files and the executables (pow!)

clean:
	rm -f *.o *~ core $(BINS)
	make -C ../support clean 

# I just included this rule to clear out the .o files, leaving the
# executables, stripped and ready for packing.

cleano:
	rm -f *.o *~

# installation rule

install:
	@./install.sh $(TARGET) $(WORKDIR) $(LOGDIR) $(LOCKDIR)

# Upgrade rule
#
#upgrade: updatefilters
#	@./updatefilters
#
# I use this special rule to force linking of the panels and ncurses
# libraries into the executable, since there seems to be a lot of
# libncurses.so.3 installations around, and some don't have libncurses.so
# at all.  Till then, I'll force them in.  Do not use this rule under
# normal circumstances.
#
# This rule also creates a separate directory containing the documentation
# and the compiled programs for release as a ready-to-run distribution.

dist-bin: all
	$(CC) $(LDOPTS) $(PROF) -o iptraf $(OBJS) -L../support -ltextbox /usr/lib/libpanel.a /usr/lib/libncurses.a
	rm -rf $(BINDIR)
	mkdir $(BINDIR)
	/bin/cp -p ../CHANGES ../LICENSE ../FAQ ../INSTALL ../README* \
	../RELEASE-NOTES ../Setup $(BINDIR)
	/bin/cp -pR ../Documentation $(BINDIR)
	mkdir $(BINDIR)/src
	/bin/cp -p $(BINS) Makefile install.sh version $(BINDIR)/src

#$(BINDIR)/src/updatefilters

#
# Just in case anyone needs to link a static binary
#
static: $(OBJS)
	$(CC) -static $(PROC) -o iptraf $(OBJS) ../support/libtextbox.a -lpanel -lncurses
	$(CC) -static -o rvnamed rvnamed.o getpath.o
	$(CC) -static -o rawtime rawtime.c

#
# Production rules.  These rules are used to automate production of
# the source and ready-to-run tarballs.  These won't really be needed by
# the general public.
#
tarball: clean
	(cd ../..;tar zcvf iptraf-$(VERNUMBER).tar.gz iptraf-$(VERNUMBER))

binball: dist-bin
	(cd ../..;tar zcvf iptraf-$(VERNUMBER).bin.$(ARCH).tar.gz iptraf-$(VERNUMBER).bin.$(ARCH))

alldist: tarball binball
