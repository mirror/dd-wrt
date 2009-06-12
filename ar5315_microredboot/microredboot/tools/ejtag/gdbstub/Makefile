TARGET = gdbstub

CC = gcc

OBJS = gdbstub.c ../lib/ejtag_primitives.c ../lib/ejtag.c
LIBS =

CFLAGS += -I /usr/src/linux-2.4/include -I../include -D__MIPSEL__ -DPTRSIZE=4

# default to 64 for now (objects built using toolchain redboot
#   uses require 32 bit).
#REG_SIZE_64 = 1
ifdef REG_SIZE_64
CFLAGS += -DREG_SIZE_64=1
endif

ifdef VERBOSE
CFLAGS += -DVERBOSE=1
endif

$(TARGET): $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(INCLUDES) $(OBJS) $(LIBS)

# include last so TARGET rule is default (else pickup rule from
# Rules.mak first).
include ../Rules.mak
