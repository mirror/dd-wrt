#
#

-include ../../path.mk
-include ../../.config

TARGET	:= seama

HGCC	:= gcc
HLD		:= gcc
HCFLAGS	:= -Wall -I$(TOPDIR)/include
HLDFLAGS:=

all: $(TARGET)

###################################################################
# Targets

seama: seama.o md5.o
	$(HLD) $(HLDFLAGS) seama.o md5.o -o $@

###################################################################
# Objects

seama.o:
	$(HGCC) $(HCFLAGS) -c $(TOPDIR)/progs.priv/rgbin/src.seama/seama.c -o $@

md5.o:
	$(HGCC) $(HCFLAGS) -c $(TOPDIR)/comlib/md5.c -o $@

###################################################################
# clean
clean:
	rm -f *.o *.gdb core $(TARGET)

.PHONY: all clean
