##################################################
# Makefile
# iperf source tree
#
# by Mark Gates <mgates@nlanr.net>
# October 1999

SHELL = /bin/sh

TOP = ..

include $(TOP)/Makefile.rules

##################################################
# files to clean

CLEAN_FILES      = *.o *~ core $(PROG)

DIST_CLEAN_FILES = $(CLEAN_FILES)

##################################################
# files

OBJS = Client.o          \
       Extractor.o       \
       Listener.o        \
       PerfSocket.o      \
       PerfSocket_TCP.o  \
       PerfSocket_UDP.o  \
       Server.o          \
       Settings.o        \
       Speaker.o         \
       Notify.o          \
       Audience.o        \
       List.o            \
       main.o

INCS = -I$(TOP)/lib -I$(TOP)/cfg

##################################################
# targets
#
# all       -- compile the iperf program
# clean     -- remove object files
# depend    -- reate dependency files
# distclean -- remove object files
# install   -- compile and install the iperf program
#              call from top level 'make install'
#

all: $(PROG)

clean:
	-rm -f $(CLEAN_FILES)

depend:
	$(CC) -MM $(INCS) $(CFLAGS) *.cpp > Makefile.depends

distclean:
	-rm -f $(DIST_CLEAN_FILES)

install: $(PROG)
	$(INSTALL) -d $(INSTALL_DIR)
	$(INSTALL) $(PROG) $(INSTALL_DIR)

.PHONY: all clean depend distclean install

##################################################
# build specific files

$(PROG): $(LIBRARY) $(OBJS)
	$(LD) -o $(PROG) $(LDFLAGS) $(OBJS) -L$(TOP)/lib -ldast $(LIBS)

include Makefile.depends
