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

.PHONY: .gitignore doc clean

clean:
	@echo "[$@] doc"
	@rm -fr "$(HTML_DIR)" "$(LATEX_DIR)" "$(PDF_DIR)" "$(TMP_DOX_FILE)"

doc: $(DOX_FILE)
	@echo "[$@] doc"
	@echo "  Cleaning up $(OS) outputs..."
	@rm -fr "$(HTML_OUTPUT)" "$(LATEX_OUTPUT)" "$(PDF_OUTPUT)"
	@mkdir -p "$(HTML_OUTPUT)"
	@mkdir -p "$(LATEX_OUTPUT)"
	@mkdir -p "$(PDF_DIR)"
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
	@echo "  Running latex to create a PDF..."
	@$(MAKE) -s -C $(LATEX_OUTPUT) all > /dev/null 2>&1
	@mkdir -p "$(PDF_DIR)"
	@mv "$(LATEX_OUTPUT)/refman.pdf" "$(PDF_OUTPUT)"
	@rm -fr "$(LATEX_OUTPUT)"
	@rmdir --ignore-fail-on-non-empty "$(LATEX_DIR)"
	@echo "  Done"
