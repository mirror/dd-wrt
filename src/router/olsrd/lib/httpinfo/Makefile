# The olsr.org Optimized Link-State Routing daemon(olsrd)
# Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org)
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions 
# are met:
#
# * Redistributions of source code must retain the above copyright 
#   notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright 
#   notice, this list of conditions and the following disclaimer in 
#   the documentation and/or other materials provided with the 
#   distribution.
# * Neither the name of olsr.org, olsrd nor the names of its 
#   contributors may be used to endorse or promote products derived 
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
# POSSIBILITY OF SUCH DAMAGE.
#
# Visit http://www.olsr.org for more information.
#
# If you find this software useful feel free to make a donation
# to the project. For more information see the website or contact
# the copyright holders.
#

OLSRD_PLUGIN =	true
PLUGIN_NAME =	olsrd_httpinfo
PLUGIN_VER =	0.1

TOPDIR = ../..
include $(TOPDIR)/Makefile.inc

ifdef ADMIN_INTERFACE
CPPFLAGS +=	-DADMIN_INTERFACE
endif

OBJS += $(TOPDIR)/src/cfgparser/cfgfile_gen.o

default_target: $(PLUGIN_FULLNAME)
ifdef ADMIN_INTERFACE
		@echo 'WARNING - BUILT WITH EXPERIMENTAL ADMIN INTERFACE!'
endif

$(PLUGIN_FULLNAME): $(OBJS) version-script.txt
		@echo "[LD] $@"
		@$(CC) $(LDFLAGS) -o $(PLUGIN_FULLNAME) $(OBJS) $(LIBS)

install:	$(PLUGIN_FULLNAME)
		$(STRIP) $(PLUGIN_FULLNAME)
		$(INSTALL_LIB)

uninstall:
		$(UNINSTALL_LIB)

clean:
		rm -f $(OBJS) $(SRCS:%.c=%.d) $(PLUGIN_FULLNAME)
