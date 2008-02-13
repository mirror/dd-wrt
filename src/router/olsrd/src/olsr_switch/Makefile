TOPDIR=../..
include $(TOPDIR)/Makefile.inc

ifeq ($(OS), win32)
BINNAME = olsr_switch.exe
OBJS += ../win32/compat.o
else
BINNAME = olsr_switch
endif

default_target:	$(OBJS)
	$(CC) $(LDFLAGS) -o $(TOPDIR)/$(BINNAME) $(OBJS) $(LIBS)

ohs_cmd.o: CFLAGS += -Wno-cast-qual

clean:
	rm -f *.[od]
	rm -f *~
	rm -f $(TOPDIR)/$(BINNAME)
