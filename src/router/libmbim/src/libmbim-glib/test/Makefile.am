include $(top_srcdir)/gtester.make

noinst_PROGRAMS = \
	test-uuid \
	test-cid \
	test-message \
	test-fragment \
	test-message-parser \
	test-message-builder \
	test-proxy-helpers

TEST_PROGS += $(noinst_PROGRAMS)


test_uuid_SOURCES = \
	test-uuid.c
test_uuid_CPPFLAGS = \
	$(LIBMBIM_GLIB_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src/libmbim-glib \
	-I$(top_builddir)/src/libmbim-glib \
	-DLIBMBIM_GLIB_COMPILATION
test_uuid_LDADD = \
	$(top_builddir)/src/libmbim-glib/libmbim-glib-core.la \
	$(top_builddir)/src/libmbim-glib/generated/libmbim-glib-generated.la \
	$(LIBMBIM_GLIB_LIBS)

test_cid_SOURCES = \
	test-cid.c
test_cid_CPPFLAGS = \
	$(LIBMBIM_GLIB_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src/libmbim-glib \
	-I$(top_builddir)/src/libmbim-glib \
	-DLIBMBIM_GLIB_COMPILATION
test_cid_LDADD = \
	$(top_builddir)/src/libmbim-glib/libmbim-glib-core.la \
	$(top_builddir)/src/libmbim-glib/generated/libmbim-glib-generated.la \
	$(LIBMBIM_GLIB_LIBS)

test_message_SOURCES = \
	test-message.c
test_message_CPPFLAGS = \
	$(LIBMBIM_GLIB_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src/libmbim-glib \
	-I$(top_builddir)/src/libmbim-glib \
	-DLIBMBIM_GLIB_COMPILATION
test_message_LDADD = \
	$(top_builddir)/src/libmbim-glib/libmbim-glib-core.la \
	$(top_builddir)/src/libmbim-glib/generated/libmbim-glib-generated.la \
	$(LIBMBIM_GLIB_LIBS)

test_fragment_SOURCES = \
	test-fragment.c
test_fragment_CPPFLAGS = \
	$(LIBMBIM_GLIB_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src/libmbim-glib \
	-I$(top_builddir)/src/libmbim-glib \
	-DLIBMBIM_GLIB_COMPILATION
test_fragment_LDADD = \
	$(top_builddir)/src/libmbim-glib/libmbim-glib-core.la \
	$(top_builddir)/src/libmbim-glib/generated/libmbim-glib-generated.la \
	$(LIBMBIM_GLIB_LIBS)

test_message_parser_SOURCES = \
	test-message-parser.c
test_message_parser_CPPFLAGS = \
	$(LIBMBIM_GLIB_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src/libmbim-glib \
	-I$(top_builddir)/src/libmbim-glib \
	-I$(top_builddir)/src/libmbim-glib/generated \
	-DLIBMBIM_GLIB_COMPILATION
test_message_parser_LDADD = \
	$(top_builddir)/src/libmbim-glib/libmbim-glib-core.la \
	$(top_builddir)/src/libmbim-glib/generated/libmbim-glib-generated.la \
	$(LIBMBIM_GLIB_LIBS)

test_message_builder_SOURCES = \
	test-message-builder.c
test_message_builder_CPPFLAGS = \
	$(LIBMBIM_GLIB_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src/libmbim-glib \
	-I$(top_builddir)/src/libmbim-glib \
	-I$(top_builddir)/src/libmbim-glib/generated \
	-DLIBMBIM_GLIB_COMPILATION
test_message_builder_LDADD = \
	$(top_builddir)/src/libmbim-glib/libmbim-glib-core.la \
	$(top_builddir)/src/libmbim-glib/generated/libmbim-glib-generated.la \
	$(LIBMBIM_GLIB_LIBS)

test_proxy_helpers_SOURCES = \
	test-proxy-helpers.c \
	$(top_srcdir)/src/libmbim-glib/mbim-proxy-helpers.h \
	$(top_srcdir)/src/libmbim-glib/mbim-proxy-helpers.c
test_proxy_helpers_CPPFLAGS = \
	$(LIBMBIM_GLIB_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src/libmbim-glib \
	-I$(top_builddir)/src/libmbim-glib \
	-I$(top_builddir)/src/libmbim-glib/generated \
	-DLIBMBIM_GLIB_COMPILATION
test_proxy_helpers_LDADD = \
	$(top_builddir)/src/libmbim-glib/libmbim-glib-core.la \
	$(top_builddir)/src/libmbim-glib/generated/libmbim-glib-generated.la \
	$(LIBMBIM_GLIB_LIBS)
