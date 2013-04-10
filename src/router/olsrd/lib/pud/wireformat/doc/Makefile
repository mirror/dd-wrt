include ../Makefile.inc
include ../../Makefile.inc

HTML_DIR = html
MAN_DIR = man
LATEX_DIR = latex
PDF_NAME = $(LIBNAME).pdf
DOX_FILE = doxygen.conf
TMP_DOX_FILE = $(DOX_FILE).temp

.PHONY: .gitignore all clean

.gitignore:
ifeq ($(VERBOSE),0)
	@echo "[$@]"
endif
	$(MAKECMDPREFIX)echo "/$(HTML_DIR)/" > $@
	$(MAKECMDPREFIX)echo "/$(MAN_DIR)/" >> $@
	$(MAKECMDPREFIX)echo "/$(LATEX_DIR)/" >> $@
	$(MAKECMDPREFIX)echo "/$(TMP_DOX_FILE)" >> $@
	$(MAKECMDPREFIX)echo "/$(PDF_NAME)" >> $@

all: clean $(DOX_FILE)
ifeq ($(VERBOSE),0)
	@echo "[$@]"
endif
	$(MAKECMDPREFIX)sed \
	  -e "s/__PLUGIN_VER__/$(PLUGIN_VER)/" \
	  -e "s/__LIBNAME__/$(LIBNAME)/" \
	  "$(DOX_FILE)" > "$(TMP_DOX_FILE)"
	$(MAKECMDPREFIX)doxygen "$(TMP_DOX_FILE)"
	$(MAKECMDPREFIX)rm "$(TMP_DOX_FILE)"
	$(MAKECMDPREFIX)$(MAKE) -s -C $(LATEX_DIR) all > /dev/null 2>&1
	$(MAKECMDPREFIX)mv "$(LATEX_DIR)/refman.pdf" "$(PDF_NAME)"
	$(MAKECMDPREFIX)rm -fr "$(LATEX_DIR)"

clean:
ifeq ($(VERBOSE),0)
	echo "[$@]"
endif
	$(MAKECMDPREFIX)rm -fr "$(HTML_DIR)" "$(LATEX_DIR)" "$(MAN_DIR)" "$(TMP_DOX_FILE)" "$(PDF_NAME)"
