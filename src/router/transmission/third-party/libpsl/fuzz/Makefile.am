AM_CFLAGS = $(WERROR_CFLAGS) $(WARN_CFLAGS)
AM_CPPFLAGS = -I$(top_srcdir)/include -I$(srcdir) -DSRCDIR=\"$(abs_srcdir)\"
#AM_LDFLAGS = -static
LDADD = ../src/libpsl.la $(LIBICU_LIBS) $(LIBIDN_LIBS) $(LIBIDN2_LIBS)

if !FUZZING
  MAIN = main.c fuzzer.h
endif

if WITH_LIBICU
  XLIBS = $(LIBICU_LIBS)
  XTYPE = _icu
  libpsl_icu_fuzzer_SOURCES = libpsl_fuzzer.c $(MAIN)
  libpsl_icu_load_fuzzer_SOURCES = libpsl_load_fuzzer.c $(MAIN)
  libpsl_icu_load_dafsa_fuzzer_SOURCES = libpsl_load_dafsa_fuzzer.c $(MAIN)
  PSL_TESTS = \
   libpsl_icu_fuzzer$(EXEEXT) \
   libpsl_icu_load_fuzzer$(EXEEXT) \
   libpsl_icu_load_dafsa_fuzzer$(EXEEXT)
else
if WITH_LIBIDN2
  XLIBS = -lidn2 -lunistring
  XTYPE = _idn2
  libpsl_idn2_fuzzer_SOURCES = libpsl_fuzzer.c $(MAIN)
  libpsl_idn2_load_fuzzer_SOURCES = libpsl_load_fuzzer.c $(MAIN)
  libpsl_idn2_load_dafsa_fuzzer_SOURCES = libpsl_load_dafsa_fuzzer.c $(MAIN)
  PSL_TESTS = \
   libpsl_idn2_fuzzer$(EXEEXT) \
   libpsl_idn2_load_fuzzer$(EXEEXT) \
   libpsl_idn2_load_dafsa_fuzzer$(EXEEXT)
else
if WITH_LIBIDN
  XLIBS = -lidn -lunistring
  XTYPE = _idn
  libpsl_idn_fuzzer_SOURCES = libpsl_fuzzer.c $(MAIN)
  libpsl_idn_load_fuzzer_SOURCES = libpsl_load_fuzzer.c $(MAIN)
  libpsl_idn_load_dafsa_fuzzer_SOURCES = libpsl_load_dafsa_fuzzer.c $(MAIN)
  PSL_TESTS = \
   libpsl_idn_fuzzer$(EXEEXT) \
   libpsl_idn_load_fuzzer$(EXEEXT) \
   libpsl_idn_load_dafsa_fuzzer$(EXEEXT)
else
  XLIBS =
  XTYPE =
  libpsl_fuzzer_SOURCES = libpsl_fuzzer.c $(MAIN)
  libpsl_load_fuzzer_SOURCES = libpsl_load_fuzzer.c $(MAIN)
  libpsl_load_dafsa_fuzzer_SOURCES = libpsl_load_dafsa_fuzzer.c $(MAIN)
  PSL_TESTS = \
   libpsl_fuzzer$(EXEEXT) \
   libpsl_load_fuzzer$(EXEEXT) \
   libpsl_load_dafsa_fuzzer$(EXEEXT)
endif
endif
endif

if FUZZING
  bin_PROGRAMS = $(PSL_TESTS)
  LDADD += $(LIB_FUZZING_ENGINE)
  AM_LDFLAGS = -no-install
else
  AM_CPPFLAGS += -DTEST_RUN
  AM_TESTS_ENVIRONMENT = export VALGRIND_TESTS"=@VALGRIND_TESTS@";
  TESTS = $(PSL_TESTS)
  check_PROGRAMS = $(PSL_TESTS)
endif

EXTRA_DIST = meson.build

dist-hook:
	find . -name '*.options' -exec cp -v '{}' $(distdir) ';'
	find . -name '*.dict' -exec cp -v '{}' $(distdir) ';'
	find . -name '*.in' -exec cp -vr '{}' $(distdir) ';'
	find . -name '*.repro' -exec cp -vr '{}' $(distdir) ';'

TESTS_ENVIRONMENT = TESTS_VALGRIND="@VALGRIND_ENVIRONMENT@"

clean-local:
	rm -rf *.gc?? *.log lcov coverage.info *_fuzzer *.o

fuzz-coverage: $(PSL_TESTS)
	find . -name '*_fuzzer' -exec ./coverage.sh '{}' ';'

CXX ?= clang
CXXFLAGS ?= $(CFLAGS)

oss-fuzz:
	if test "$$OUT" != ""; then \
	for cfile in `ls *_fuzzer.c`; do \
	fuzzer="libpsl$(XTYPE)_"$$(basename $$cfile .c|cut -d'_' -f2-); \
	$$CXX $$CXXFLAGS $$CPPFLAGS -I$(top_srcdir)/include -I$(top_srcdir) \
	"$${cfile}" -o "$${fuzzer}" \
	../src/.libs/libpsl.a $${LIB_FUZZING_ENGINE} \
	-Wl,-Bstatic \
	$(XLIBS) \
	-Wl,-Bdynamic; \
	done; \
	fi

.PHONY: oss-fuzz
