# See if we can cast the automake spell. -*-Makefile-*-
#

#
AUTOMAKE_OPTIONS = no-dependencies 1.4

lib_LTLIBRARIES = libvstr.la

INCLUDES = -I$(top_srcdir)/include

if HAVE_LINKER_MAIN_RELOCATE
VSTR__MAIN_RELOCATE_AM=-Wl,-e,vstr_version_func
endif

if HAVE_LINKER_VERSION_SCRIPT
VSTR__VERSION_SCRIPT_AM=-Wl,--version-script=$(top_srcdir)/src/linker-script
endif

if HAVE_LINKER_MAIN_RELOCATE
VSTR__RELOC_MAIN =
else
VSTR__RELOC_MAIN =  -DHAVE_NO_RELOC_MAIN
endif
CFLAGS = @CFLAGS@ @WARNS@ $(VSTR__RELOC_MAIN)

if HAVE_POSIX_HOST
POSIX_HOST_SRC = vstr_sc_posix.c
else
POSIX_HOST_SRC = vstr_sc_noposix.c
endif

if FMT_DBL_glibc
ASM_SOURCE = vstr_add_fmt_dbl_asm.S
endif

libvstr_la_SOURCES = assert_loop.c fix.c \
                        vstr.c \
                        vstr_add.c \
                        vstr_add_fmt.c \
			$(ASM_SOURCE) \
                        vstr_add_netstr.c \
                        vstr_cache.c \
                        vstr_cmp.c \
                        vstr_cntl.c \
                        vstr_conv.c \
                        vstr_cstr.c \
                        vstr_data.c \
                        vstr_del.c \
                        vstr_dup.c \
                        vstr_export.c \
                        vstr_fmt.c \
                        vstr_inline.c \
                        vstr_mov.c \
                        vstr_parse.c \
                        vstr_parse_netstr.c \
                        vstr_ref.c \
                        vstr_sc.c \
                        $(POSIX_HOST_SRC) \
                        vstr_sect.c \
                        vstr_spn.c \
                        vstr_split.c \
                        vstr_srch.c \
                        vstr_srch_case.c \
                        vstr_sub.c \
                        vstr_version.c

# Rebuild the extern inline functions, when the inline's are changed.
vstr_inline.lo: ../include/vstr-inline.h ../include/vstr-nx-inline.h

libvstr_la_LDFLAGS = \
        -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
        -release $(LT_RELEASE) \
        -export-dynamic $(VSTR__MAIN_RELOCATE_AM) $(VSTR__VERSION_SCRIPT_AM)
