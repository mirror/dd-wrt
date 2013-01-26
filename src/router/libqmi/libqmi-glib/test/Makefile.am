
noinst_PROGRAMS = \
	test-utils \
	test-message

test_utils_SOURCES = \
	test-utils.c
test_utils_CPPFLAGS = \
	$(LIBQMI_GLIB_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libqmi-glib \
	-I$(top_srcdir)/libqmi-glib/generated \
	-I$(top_builddir)/libqmi-glib/generated \
	-DLIBQMI_GLIB_COMPILATION
test_utils_LDADD = \
	$(top_builddir)/libqmi-glib/libqmi-glib.la \
	$(LIBQMI_GLIB_LIBS)

test_message_SOURCES = \
	test-message.c
test_message_CPPFLAGS = \
	$(LIBQMI_GLIB_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libqmi-glib \
	-I$(top_srcdir)/libqmi-glib/generated \
	-I$(top_builddir)/libqmi-glib/generated \
	-DLIBQMI_GLIB_COMPILATION
test_message_LDADD = \
	$(top_builddir)/libqmi-glib/libqmi-glib.la \
	$(LIBQMI_GLIB_LIBS)

if WITH_TESTS

check-local: test-utils test-message
	$(abs_builddir)/test-utils
	$(abs_builddir)/test-message

endif
