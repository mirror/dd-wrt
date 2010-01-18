# Makefile for the Linux CAN-bus driver.
# Written by Arnaud Westenberg email:arnaud@wanadoo.nl
# This software is released under the GPL-License.
# Version 0.7  6 Aug 2001
#
# Changes made by Pavel Pisa pisa@cmp.felk.cvut.cz as preliminary
# study for OCERA Real Time CAN/CANOpen project ORTCAN
#
########## The following options can be changed ##########

# Compiler
ifndef CC
CC = gcc
endif

CFLAGS = -I../include -O2 -Wall

all: default

.PHONY: dep default subdirs clean cleandepend
.PHONY: check-dir include-pass library-pass binary-pass utils-pass kernel-pass

default: utils

utils-pass: utils

dep:
	$(CC) $(CFLAGS) $(CPPFLAGS) -w -E -M *.c $(MORE_C_FILES) > .depend

install:
	echo Nothing to install

utils: rxtx sendburst readburst send can-proxy

rxtx: rxtx.o

sendburst: sendburst.o

readburst: readburst.o

send: send.o

clean :
	rm -f rxtx sendburst readburst send can-proxy *.o .depend

distclean : clean

-include .depend

