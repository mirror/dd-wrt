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

TOPDIR = ..
include $(TOPDIR)/Makefile.inc

HTML_DIR = html
HTML_OUTPUT = $(HTML_DIR)/$(OS)

LATEX_DIR = latex
LATEX_OUTPUT = $(LATEX_DIR)/$(OS)

PDF_DIR = pdf
PDF_OUTPUT = $(PDF_DIR)/olsrd_$(OS).pdf

DOX_FILE = doxygen.conf
TMP_DOX_FILE = $(DOX_FILE).temp

OLSRD_VER = $(shell git describe --dirty --always)


DOC_DEFINES = $(shell ./getdefines.bash $(CPPFLAGS) $(CFLAGS))


# All excludes
# DOC_INPUT   = ../src ../android
# DOC_EXCLUDE = ../src/linux ../src/unix ../android ../src/win32 ../src/bsd

DOC_INPUT   = ../src ../android

ifeq ($(OS), linux)
DOC_DEFINES += __linux__
DOC_EXCLUDE = ../android ../src/win32 ../src/bsd
else
ifeq ($(OS), android)
DOC_DEFINES += __linux__ _ANDROID
DOC_EXCLUDE = ../src/win32 ../src/bsd
else
ifeq ($(OS), win32)
DOC_DEFINES += _WIN32
DOC_EXCLUDE = ../src/linux ../src/unix ../android ../src/bsd
else
$(error $(OS) not known to the doc makefile)
endif
endif
endif


all: default_target

default_target: doc

.PHONY: .gitignore doc doc-pdf clean

clean:
	@echo "[$@] doc"
	@rm -fr "$(HTML_DIR)" "$(LATEX_DIR)" "$(PDF_DIR)" "$(TMP_DOX_FILE)"

doc: $(DOX_FILE)
	@echo "[$@] doc"
	@echo "  Cleaning up $(OS) outputs..."
	@rm -fr "$(HTML_OUTPUT)" "$(LATEX_OUTPUT)"
	@mkdir -pv "$(HTML_OUTPUT)"
	@mkdir -pv "$(LATEX_OUTPUT)"
	@echo "  Generating doxygen configuration..."
	@sed \
	  -e "s/__OLSRD_VER__/$(OLSRD_VER)/" \
	  -e "s#__OLSRD_NAME__#olsrd ($(OS))#" \
	  -e "s#__DEFINES__#$(DOC_DEFINES)#" \
	  -e "s#__INPUT__#$(DOC_INPUT)#" \
	  -e "s#__EXCLUDE__#$(DOC_EXCLUDE)#" \
	  -e "s#__HTML_OUTPUT__#$(HTML_OUTPUT)#" \
	  -e "s#__LATEX_OUTPUT__#$(LATEX_OUTPUT)#" \
	  "$(DOX_FILE)" > "$(TMP_DOX_FILE)"
	@echo "  Running doxygen to create HTML..."
	@doxygen "$(TMP_DOX_FILE)"
	@rm -f "$(TMP_DOX_FILE)"
	@echo "  Done"

doc-pdf: doc
	@echo "[$@] doc"
	@echo "  Cleaning up $(OS) outputs..."
	@rm -fr "$(PDF_OUTPUT)"
	@mkdir -pv "$(PDF_DIR)"
	@echo "  Running latex to create a PDF..."
	@$(MAKE) -s -C $(LATEX_OUTPUT) all
	@mkdir -pv "$(PDF_DIR)"
	@mv -v "$(LATEX_OUTPUT)/refman.pdf" "$(PDF_OUTPUT)"
	@rm -fr "$(LATEX_OUTPUT)"
	@rmdir -v --ignore-fail-on-non-empty "$(LATEX_DIR)"
	@echo "  Done"
