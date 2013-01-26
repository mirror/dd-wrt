
# The name of the module.
DOC_MODULE = libqmi-glib

# The top-level SGML file.
DOC_MAIN_SGML_FILE = $(DOC_MODULE)-docs.xml

# Sections file building
ALL_SECTIONS = \
	$(srcdir)/libqmi-glib-common.sections \
	$(top_builddir)/libqmi-glib/generated/qmi-dms.sections \
	$(top_builddir)/libqmi-glib/generated/qmi-nas.sections \
	$(top_builddir)/libqmi-glib/generated/qmi-wds.sections \
	$(top_builddir)/libqmi-glib/generated/qmi-wms.sections \
	$(top_builddir)/libqmi-glib/generated/qmi-pds.sections

$(DOC_MODULE)-sections.mstamp: $(ALL_SECTIONS)
	$(AM_V_GEN) \
		rm -f $(DOC_MODULE)-sections.txt && \
		cat $(ALL_SECTIONS) > $(DOC_MODULE)-sections.txt && \
		touch $(DOC_MODULE)-sections.mstamp

BUILT_SOURCES = $(DOC_MODULE)-sections.mstamp

# Extra options to supply to gtkdoc-scan
SCAN_OPTIONS = --rebuild-types

# The directory containing the source code.
DOC_SOURCE_DIR = \
	$(top_srcdir)/libqmi-glib \
	$(top_builddir)/libqmi-glib/generated

# Used for dependencies
HFILE_GLOB = \
	$(top_srcdir)/libqmi-glib/*.h \
	$(top_builddir)/libqmi-glib/generated/*.h
CFILE_GLOB = \
	$(top_srcdir)/libqmi-glib/*.c \
	$(top_builddir)/libqmi-glib/generated/*.c

# Headers to ignore
IGNORE_HFILES = \
	qmi-enums-private.h \
	qmi-enum-types-private.h \
	qmi-ctl.h

# CFLAGS and LDFLAGS for compiling scan program. Only needed
# if $(DOC_MODULE).types is non-empty.
INCLUDES = \
	-I$(srcdir) \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	$(LIBQMI_GLIB_CFLAGS)

GTKDOC_LIBS = \
	$(LIBQMI_GLIB_LIBS) \
	$(top_builddir)/libqmi-glib/libqmi-glib.la

# Extra options to supply to gtkdoc-mkdb
MKDB_OPTIONS = --output-format=xml --sgml-mode --name-space=qmi

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE)
content_files = version.xml

expand_content_files =

extra_files =

if ENABLE_GTK_DOC
include $(top_srcdir)/gtk-doc.make
else
EXTRA_DIST =
CLEANFILES =
endif

EXTRA_DIST += \
	libqmi-glib-common.sections \
	$(DOC_MODULE)-sections.txt \
	$(DOC_MODULE)-sections.mstamp \
	version.xml.in \
	$(DIAGRAMS) \
	$(NULL)

CLEANFILES += \
	$(DOC_MODULE)-decl-list.txt \
	$(DOC_MODULE)-decl.txt \
	$(DOC_MODULE)-overrides.txt \
	$(DOC_MODULE)-undeclared.txt \
	$(DOC_MODULE)-undocumented.txt \
	$(DOC_MODULE)-overrides.txt \
	$(DOC_MODULE)-unused.txt \
	$(DOC_MODULE).args \
	$(DOC_MODULE).hierarchy \
	$(DOC_MODULE).interfaces \
	$(DOC_MODULE).prerequisites \
	$(DOC_MODULE).signals \
	$(DOC_MODULE).types \
	$(DOC_MODULE)-sections.txt \
	$(DOC_MODULE)-sections.mstamp \
	*.stamp \
	-rf xml html tmpl \
	$(NULL)
