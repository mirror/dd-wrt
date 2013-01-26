
noinst_PROGRAMS = \
	test-helpers

test_helpers_SOURCES = \
	test-helpers.c \
	$(top_srcdir)/cli/qmicli-helpers.h \
	$(top_srcdir)/cli/qmicli-helpers.c

test_helpers_CPPFLAGS = \
	$(QMICLI_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/cli \
	-I$(top_srcdir)/libqmi-glib \
	-I$(top_srcdir)/libqmi-glib/generated \
	-I$(top_builddir)/libqmi-glib/generated

test_helpers_LDADD = \
	$(QMICLI_LIBS) \
	$(top_builddir)/libqmi-glib/libqmi-glib.la

if WITH_TESTS

check-local: test-helpers
	$(abs_builddir)/test-helpers

endif
