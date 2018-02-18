# The olsr.org Optimized Link-State Routing daemon (olsrd)
#
# (c) by the OLSR project
#
# See our Git repository to find out who worked on this file
# and thus is a copyright holder on it.
#
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

include ../Makefile.inc

HTML_DIR = html
MAN_DIR = man
LATEX_DIR = latex
PDF_NAME = $(PLUGIN_NAME).pdf
DOX_FILE = doxygen.conf
TMP_DOX_FILE = $(DOX_FILE).temp

.PHONY: .gitignore all clean

.gitignore:
ifeq ($(VERBOSE),0)
	@echo "[$@] doc"
endif
	$(MAKECMDPREFIX)echo "/$(HTML_DIR)/" > $@
	$(MAKECMDPREFIX)echo "/$(MAN_DIR)/" >> $@
	$(MAKECMDPREFIX)echo "/$(LATEX_DIR)/" >> $@
	$(MAKECMDPREFIX)echo "/$(TMP_DOX_FILE)" >> $@
	$(MAKECMDPREFIX)echo "/$(PDF_NAME)" >> $@

clean:
ifeq ($(VERBOSE),0)
	@echo "[$@] doc"
endif
	$(MAKECMDPREFIX)rm -fr "$(TMP_DOX_FILE)" "$(HTML_DIR)" "$(LATEX_DIR)" "$(MAN_DIR)" "$(PDF_NAME)"

all: clean $(DOX_FILE)
ifeq ($(VERBOSE),0)
	@echo "[$@] doc"
endif
	$(MAKECMDPREFIX)sed \
	  -e "s/__PLUGIN_VER__/$(PLUGIN_VER)/" \
	  -e "s/__LIBNAME__/$(PLUGIN_NAME)/" \
	  "$(DOX_FILE)" > "$(TMP_DOX_FILE)"
	$(MAKECMDPREFIX)doxygen "$(TMP_DOX_FILE)"
	$(MAKECMDPREFIX)rm "$(TMP_DOX_FILE)"
	$(MAKECMDPREFIX)$(MAKE) -s -C $(LATEX_DIR) all > /dev/null 2>&1
	$(MAKECMDPREFIX)mv "$(LATEX_DIR)/refman.pdf" "$(PDF_NAME)"
	$(MAKECMDPREFIX)rm -fr "$(LATEX_DIR)"
