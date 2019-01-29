AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS=-I m4

SUBDIRS = .

if BUILD_TESTS
SUBDIRS += tests
endif

include_HEADERS = qrencode.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libqrencode.pc
noinst_DATA = README
README_markdown_optional := $(wildcard README.md)
README: $(README_markdown_optional)
	    $(if $(README_markdown_optional), ./makeREADME.sh)

EXTRA_DIST = libqrencode.pc.in autogen.sh configure.ac acinclude.m4 \
			 Makefile.am tests/Makefile.am \
			 qrencode.1.in Doxyfile \
			 CMakeLists.txt cmake/FindIconv.cmake 

if BUILD_TOOLS
bin_PROGRAMS = qrencode
qrencode_SOURCES = qrenc.c
qrencode_CFLAGS = $(png_CFLAGS)
qrencode_LDADD = $(png_LIBS)
man1_MANS = qrencode.1
endif
