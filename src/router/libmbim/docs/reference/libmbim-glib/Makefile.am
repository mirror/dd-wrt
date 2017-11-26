
# The name of the module.
DOC_MODULE = libmbim-glib

# The top-level SGML file.
DOC_MAIN_SGML_FILE = $(DOC_MODULE)-docs.xml

# Sections file building
ALL_SECTIONS = \
	$(srcdir)/libmbim-glib-common.sections \
	$(top_builddir)/src/libmbim-glib/generated/mbim-basic-connect.sections \
	$(top_builddir)/src/libmbim-glib/generated/mbim-sms.sections \
	$(top_builddir)/src/libmbim-glib/generated/mbim-ussd.sections \
	$(top_builddir)/src/libmbim-glib/generated/mbim-auth.sections \
	$(top_builddir)/src/libmbim-glib/generated/mbim-phonebook.sections \
	$(top_builddir)/src/libmbim-glib/generated/mbim-stk.sections \
	$(top_builddir)/src/libmbim-glib/generated/mbim-dss.sections \
	$(top_builddir)/src/libmbim-glib/generated/mbim-ms-firmware-id.sections \
	$(top_builddir)/src/libmbim-glib/generated/mbim-ms-host-shutdown.sections \
	$(top_builddir)/src/libmbim-glib/generated/mbim-qmi.sections \
	$(top_builddir)/src/libmbim-glib/generated/mbim-atds.sections

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
	$(top_srcdir)/src/libmbim-glib \
	$(top_builddir)/src/libmbim-glib \
	$(top_builddir)/src/libmbim-glib/generated

# Used for dependencies
HFILE_GLOB = \
	$(top_srcdir)/src/libmbim-glib/*.h \
	$(top_builddir)/src/libmbim-glib/*.h \
	$(top_builddir)/src/libmbim-glib/generated/*.h
CFILE_GLOB = \
	$(top_srcdir)/src/libmbim-glib/*.c \
	$(top_builddir)/src/libmbim-glib/generated/*.c

# Headers to ignore
IGNORE_HFILES = \
	mbim-message-private.h

# CFLAGS and LDFLAGS for compiling scan program. Only needed
# if $(DOC_MODULE).types is non-empty.
AM_CPPFLAGS = \
	-I$(srcdir) \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	$(LIBMBIM_GLIB_CFLAGS)

GTKDOC_LIBS = \
	$(LIBMBIM_GLIB_LIBS) \
	$(top_builddir)/src/libmbim-glib/libmbim-glib.la

# Extra options to supply to gtkdoc-mkdb
MKDB_OPTIONS = --output-format=xml --sgml-mode --name-space=mbim

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE)
content_files = version.xml

expand_content_files =

extra_files =

include $(top_srcdir)/gtk-doc.make


EXTRA_DIST += \
	libmbim-glib-common.sections \
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
