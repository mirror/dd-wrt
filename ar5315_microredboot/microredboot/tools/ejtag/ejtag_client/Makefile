TARGET = ejtag_client

CC = gcc

OBJS = ejtag_client.c ../lib/ejtag_primitives.c ../lib/ejtag.c
LIBS =

CFLAGS += -I /usr/src/linux-2.4/include -I../include -D__MIPSEL__ -DPTRSIZE=4

$(TARGET): $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(INCLUDES) $(OBJS) $(LIBS)

# include last so TARGET rule is default (else pickup rule from
# Rules.mak first).
include ../Rules.mak
