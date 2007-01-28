#  Makefile for storage modules in the Squid Object Cache server
#
#  $Id: Makefile.am,v 1.5 2006/06/02 21:35:41 hno Exp $
#

AUTOMAKE_OPTIONS = subdir-objects

EXTRA_LIBRARIES = liblru.a libheap.a
noinst_LIBRARIES = @REPL_LIBS@

liblru_a_SOURCES = lru/store_repl_lru.c
libheap_a_SOURCES = heap/store_heap_replacement.h heap/store_heap_replacement.c heap/store_repl_heap.c

INCLUDES      = -I. -I$(top_builddir)/include -I$(top_srcdir)/include \
	-I$(top_srcdir)/src

