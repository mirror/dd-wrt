AUTOMAKE_OPTIONS=foreign no-dependencies

EXTRA_DIST = \
sfdynamic_preproc_libs.vcxproj \
sfdynamic_preproc_libs.dsp \
sfparser.c \
sfcommon.h \
sf_preproc_info.h \
snort_preproc.pc.in 

if SO_WITH_STATIC_LIB

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = snort_preproc.pc

endif
