#
#   Makefile - build and install the comgt package
#   Copyright (C) 2005  Martin Gregorie
#   Copyright (C) 2006  Paul Hardwick
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#    martin@gregorie.org, paul@peck.org.uk
#
#    $Id: Makefile,v 1.4 2006/10/20 14:30:19 pharscape Exp $
#
#

#LIB     = -L/usr/local/lib
#INC     = -I/usr/local/include
#EXE	= /usr/local/bin
#MAN     = /usr/share/man/man1
CPROG	= comgt
SCRIPTPATH = /tmp/comgt/
SCRIPTSRC = ./scripts/
BIN     = $(CPROG) 
MANP	= comgt.1 sigmon.1

CFLAGS  = -c
LDFLAGS =

all: $(BIN)

install:
	chmod a-w $(BIN)
	chmod u+rw $(BIN)
	chmod a+x $(BIN)
	cp $(BIN) $(EXE)
	chmod a-wx $(MANP)
	chmod u+rw $(MANP)
	chmod a+r $(MANP)
	cp $(MANP) $(MAN)
	-mkdir $(SCRIPTPATH)
	chmod a-w $(SCRIPTPATH)
	chmod u+rw $(SCRIPTPATH)
	chmod a+x $(SCRIPTPATH)
	cp -f $(SCRIPTSRC)* $(SCRIPTPATH)
	chmod a-w $(SCRIPTPATH)*
	chmod u+rw $(SCRIPTPATH)*
	chmod a+x $(SCRIPTPATH)*




uninstall:
	cd $(EXE); rm $(BIN)
	cd $(MAN); rm $(MANP)
	-rm -r $(SCRIPTPATH)

clean:
	-rm *.o 
	-rm $(CPROG) 
	-rm *~
	-rm $(SCRIPTSRC)*~


comgt: comgt.o
	$(CC) comgt.o $(LDFLAGS) -o comgt

comgt.o: comgt.c comgt.h
	$(CC) $(COPTS) comgt.c $(CFLAGS) 

