##################################################
# Makefile
# iperf library tree
#
# by Mark Gates <mgates@nlanr.net>
# October 1999
# $Id: Makefile,v 1.4 2002/10/03 22:11:20 kgibbs Exp $

SHELL = /bin/sh

TOP = ..

include $(TOP)/Makefile.rules

##################################################
# files to clean

CLEAN_FILES = *.o *~ core $(LIBRARY)

DIST_CLEAN_FILES = $(CLEAN_FILES)

##################################################
# files

OBJS = Socket.o          \
       SocketAddr.o      \
       Thread.o          \
       delay.o           \
       error.o           \
       gnu_getopt.o      \
       gnu_getopt_long.o \
       signal.o          \
       sockets.o         \
       stdio.o           \
       string.o          \
       tcp_window_size.o \
       $(LIBOBJS)

# MORE is a couple library files not used in Iperf
# but used by other programs

MORE = Queue.o \
       endian.o

INCS = -I$(TOP)/cfg

##################################################
# targets
#
# all       -- compile library
# clean     -- remove object files
# depend    -- create dependency files
# distclean -- remove object files

all: $(LIBRARY)

clean:
	-rm -f $(CLEAN_FILES)

depend:
	$(CC) -MM $(INCS) $(CFLAGS) *.c *.cpp > Makefile.depends

distclean:
	-rm -f $(DIST_CLEAN_FILES)

.PHONY: all clean depend distclean

##################################################
# build specific files

$(LIBRARY): $(OBJS)
	$(AR) $(ARFLAGS) $(LIBRARY) $(OBJS)
	$(RANLIB) $(LIBRARY)

include Makefile.depends
