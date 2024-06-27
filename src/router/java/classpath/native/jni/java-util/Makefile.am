nativeexeclib_LTLIBRARIES = libjavautil.la

libjavautil_la_SOURCES = java_util_VMTimeZone.c

libjavautil_la_LIBADD = $(top_builddir)/native/jni/classpath/jcl.lo

AM_LDFLAGS = @CLASSPATH_MODULE@
AM_CPPFLAGS = @CLASSPATH_INCLUDES@
AM_CFLAGS = @WARNING_CFLAGS@ @STRICT_WARNING_CFLAGS@ @ERROR_CFLAGS@ \
            @EXTRA_CFLAGS@

