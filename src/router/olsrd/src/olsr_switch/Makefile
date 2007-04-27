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

clean:
	rm -f *.o *.d
	rm -f *~
	rm -f $(TOPDIR)/$(BINNAME)
