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
