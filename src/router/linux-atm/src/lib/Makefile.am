#lib_LTLIBRARIES = libatm.la libatmd.la
lib_LTLIBRARIES = libatm.la

# ATMLIBS_VERSION comes from configure.in in the toplevel directory
AM_LDFLAGS = -version-info @LIBVER_CURRENT@:@LIBVER_REVISION@:@LIBVER_AGE@



libatm_la_SOURCES = text2atm.c atm2text.c atmequal.c sdu2cell.c text2qos.c \
			qos2text.c qosequal.c sap2text.c text2sap.c sapequal.c \
			misc.c atmres.h ans.c \
			common.c diag.c kptr.c text2ip.c timer.c unix.c
libatm_la_LIBADD = -lresolv
libatm_la_DEPENDENCIES = $(top_builddir)/src/include/atm.h \
				$(top_builddir)/src/include/atmsap.h \
				$(top_builddir)/src/include/atmd.h
