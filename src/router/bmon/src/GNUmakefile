# Makefile                bmon Makefile
#
# Copyright (c) 2001-2005 Thomas Graf <tgraf@suug.ch>
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
ifeq ($(shell [ ! -r ../Makefile.opts ] && echo 1),)
    include ../Makefile.opts
endif

# Core
CIN  := bmon.c utils.c input.c output.c conf.c node.c item.c graph.c
CIN  += signal.c bindings.c itemtab.c

# Primary input modules
CIN  += in_null.c in_dummy.c in_proc.c in_kstat.c in_netlink.c in_sysfs.c
CIN  += in_sysctl.c in_distribution.c in_netstat.c

# Primary output modules
CIN  += out_null.c out_ascii.c out_format.c

# Secondary output module
CIN  += out_html.c

ifeq ($(CURSES),Yes)
CIN  += out_curses.c
endif

ifeq ($(RRD),Yes)
CIN  += out_rrd.c
endif

ifeq ($(ASOUND),Yes)
CIN  += out_audio.c
endif

ifeq ($(DBI),Yes)
CIN  += out_db.c
endif

DEPS := $(CIN:%.c=.deps/%.d)
OBJ  := $(CIN:%.c=%.o)
OUT  := bmon

export

.PHONY: all clean install $(OUT)

all: bmon

$(OUT): ../Makefile.opts $(OBJ)
	@echo "  LD $(OUT)"; \
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $(OUT) $(OBJ) $(LDFLAGS) $(BMON_LIB)

clean:
	@echo "  CLEAN src"; \
	$(RM) -f $(OBJ) $(OUT)

distclean:
	@echo "  DISTCLEAN src"; \
	$(RM) -f .deps/*

$(DEPS): ../Makefile.opts

include ../Makefile.rules

