AUTOMAKE_OPTIONS=foreign no-dependencies

noinst_LIBRARIES = libhi_utils.a

libhi_utils_a_SOURCES = \
hi_util_kmap.c \
hi_util_xmalloc.c \
hi_util_hbm.c \
hi_cmd_lookup.c \
hi_buffer_dump.c \
hi_paf.c \
h2_paf.c \
h2_common.c

#if BUILD_BUFFER_DUMP
#libhi_utils_a_SOURCES += \
#hi_buffer_dump.c
#endif

INCLUDES = @INCLUDES@
