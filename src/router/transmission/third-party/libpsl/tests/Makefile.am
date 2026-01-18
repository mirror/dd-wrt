DEFS = @DEFS@ \
       -DSRCDIR=\"$(srcdir)\" \
       -DPSL_FILE=\"$(PSL_FILE)\" \
       -DPSL_TESTFILE=\"$(PSL_TESTFILE)\" \
       -DPSL_DAFSA=\"psl.dafsa\" \
       -DPSL_ASCII_DAFSA=\"psl_ascii.dafsa\"
AM_CPPFLAGS = -I$(top_srcdir)/include
LDADD = ../src/libpsl.la
AM_LDFLAGS = -no-install

# ./configure'd with '--disable-builtin'
# Do not call test-is-public-builtin here: it does not make sense.
# Do not call test-registrable-domain here: it would fail due to missing punycode entries in PSL file.
PSL_TESTS = test-is-public test-is-public-all test-is-cookie-domain-acceptable

if ENABLE_BUILTIN
  PSL_TESTS += test-is-public-builtin test-registrable-domain
  test_is_public_builtin_SOURCES = test-is-public-builtin.c $(common_SOURCES)
  test_registrable_domain_SOURCES = test-registrable-domain.c $(common_SOURCES)
endif

check_PROGRAMS = $(PSL_TESTS)

TESTS_ENVIRONMENT = TESTS_VALGRIND="@VALGRIND_ENVIRONMENT@"
TESTS = $(PSL_TESTS)

common_SOURCES = common.c common.h
test_is_public_SOURCES = test-is-public.c $(common_SOURCES)
test_is_public_all_SOURCES = test-is-public-all.c $(common_SOURCES)
test_is_cookie_domain_acceptable_SOURCES = test-is-cookie-domain-acceptable.c $(common_SOURCES)

# dafsa.psl and dafsa_ascii.psl must be created before any test is executed
# check-local target works in parallel to the tests, so the test suite will likely fail
BUILT_SOURCES = psl.dafsa psl_ascii.dafsa
psl.dafsa: $(PSL_FILE)
	$(PYTHON) $(top_srcdir)/src/psl-make-dafsa --output-format=binary "$(PSL_FILE)" psl.dafsa
psl_ascii.dafsa: $(PSL_FILE)
	$(PYTHON) $(top_srcdir)/src/psl-make-dafsa --output-format=binary --encoding=ascii "$(PSL_FILE)" psl_ascii.dafsa

clean-local:
	rm -f psl.dafsa psl_ascii.dafsa

EXTRA_DIST = meson.build
