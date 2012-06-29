include ../Makefile.inc

HTML_DIR = html
MAN_DIR = man
LATEX_DIR = latex
PDF_NAME = $(PLUGIN_NAME).pdf
DOX_FILE = doxygen.conf
TMP_DOX_FILE = $(DOX_FILE).temp

.PHONY: .gitignore all clean

.gitignore:
	@echo "[$@] doc"
	@echo "/$(HTML_DIR)/" > $@
	@echo "/$(MAN_DIR)/" >> $@
	@echo "/$(LATEX_DIR)/" >> $@
	@echo "/$(TMP_DOX_FILE)" >> $@
	@echo "/$(PDF_NAME)" >> $@

clean:
	@echo "[$@] doc"
	@rm -fr "$(TMP_DOX_FILE)" "$(HTML_DIR)" "$(LATEX_DIR)" "$(MAN_DIR)" "$(PDF_NAME)"

all: clean $(DOX_FILE)
	@echo "[$@] doc"
	@sed \
	  -e "s/__PLUGIN_VER__/$(PLUGIN_VER)/" \
	  -e "s/__LIBNAME__/$(PLUGIN_NAME)/" \
	  "$(DOX_FILE)" > "$(TMP_DOX_FILE)"
	@doxygen "$(TMP_DOX_FILE)"
	@rm "$(TMP_DOX_FILE)"
	@$(MAKE) -s -C $(LATEX_DIR) all > /dev/null 2>&1
	@mv "$(LATEX_DIR)/refman.pdf" "$(PDF_NAME)"
	@rm -fr "$(LATEX_DIR)"
