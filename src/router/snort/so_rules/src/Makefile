BASEDIR=../../
ENGINEDIR=$(BASEDIR)/src/dynamic-plugins/sf_engine
PLUGINDIR=$(BASEDIR)/src/dynamic-plugins
ENGINE=$(BASEDIR)/src/dynamic-plugins/sf_engine/.libs/libsf_engine.so
SNORT=$(BASEDIR)/src/snort
SNORT_VERSION=2.9.20.0


PATH:=$(PATH):/opt/intel/compiler70/ia32/bin
CC=$(shell if PATH=$(PATH) which icc > /dev/null 2>&1; then echo icc; else echo gcc; fi)

# RANDSTRING=$(shell perl -e 'for($$i=0;$$i<128;$$i++){$$_ .= unpack("h*",chr(int(rand(256))));} print')
# For now, a static rand string is good enough, since we are not using it yet.
RANDSTRING=e57509e0e8702d1794fd29f0741f74e0d649aff186ffc1aa0dab1d6817e3ea35ee6153f553ea923c4d49b5f

MYCFLAGS = $(CFLAGS) -ggdb -I$(ENGINEDIR) -I$(PLUGINDIR) -fPIC `pcre-config --cflags` -DVRT_RAND_STRING=\"$(RANDSTRING)\" -Wall

ifeq ($(CC),gcc)
MYCFLAGS += -Wno-unused-variable -std=c99 -pedantic -Wformat -Wuninitialized -O2 -Wno-strict-aliasing -D_GNU_SOURCE
endif
ifeq ($(CC),icc)
MYCFLAGS += -wd1418,1419,810,193,177 -w2
LD_FLAGS += -lirc -static-libcxa
endif

# If you use --enable-non-ether-decoders with configure when building snort (not default behavior
# as of snort 2.9.4.1), remove the -DNO_NON_ETHER_DECODER from MYCFLAGS below
ifeq (${SNORT_VERSION},2.9.7.0)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=2 -DREQ_ENGINE_LIB_MINOR=4 -DMISSING_COMB_ADDRS=1 -DNO_NON_ETHER_DECODER -DBEFORE_2091300
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.7.2)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=2 -DREQ_ENGINE_LIB_MINOR=4 -DMISSING_COMB_ADDRS=1 -DNO_NON_ETHER_DECODER -DBEFORE_2091300
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.7.3)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=2 -DREQ_ENGINE_LIB_MINOR=4 -DMISSING_COMB_ADDRS=1 -DNO_NON_ETHER_DECODER -DBEFORE_2091300
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.7.5)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=2 -DREQ_ENGINE_LIB_MINOR=4 -DMISSING_COMB_ADDRS=1 -DNO_NON_ETHER_DECODER -DBEFORE_2091300
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.7.6)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=2 -DREQ_ENGINE_LIB_MINOR=4 -DMISSING_COMB_ADDRS=1 -DNO_NON_ETHER_DECODER -DBEFORE_2091300
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.8.0)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=2 -DREQ_ENGINE_LIB_MINOR=4 -DNO_NON_ETHER_DECODER -DBEFORE_2091300
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.8.2)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=2 -DREQ_ENGINE_LIB_MINOR=6 -DNO_NON_ETHER_DECODER -DBEFORE_2091300
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.8.3)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=2 -DREQ_ENGINE_LIB_MINOR=6 -DNO_NON_ETHER_DECODER -DBEFORE_2091300
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.9.0)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=0 -DNO_NON_ETHER_DECODER -DBEFORE_2091300
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.10.0)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=0 -DNO_NON_ETHER_DECODER -DBEFORE_2091300
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.11.0)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=0 -DNO_NON_ETHER_DECODER -DBEFORE_2091300
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.11.1)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=0 -DNO_NON_ETHER_DECODER -DBEFORE_2091300
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.12.0)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=0 -DNO_NON_ETHER_DECODER -DBEFORE_2091300
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.13.0)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=1 -DNO_NON_ETHER_DECODER
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.14.0)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=1 -DNO_NON_ETHER_DECODER
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.14.1)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=1 -DNO_NON_ETHER_DECODER
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.15.0)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=1 -DNO_NON_ETHER_DECODER
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.15.1)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=1 -DNO_NON_ETHER_DECODER
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.16.0)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=1 -DNO_NON_ETHER_DECODER
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.16.1)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=1 -DNO_NON_ETHER_DECODER
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.17.0)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=1 -DNO_NON_ETHER_DECODER
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.17.1)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=1 -DNO_NON_ETHER_DECODER
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.18.0)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=2 -DNO_NON_ETHER_DECODER
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.18.1)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=2 -DNO_NON_ETHER_DECODER
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.19.0)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=2 -DNO_NON_ETHER_DECODER
SEEN=1
endif
ifeq (${SNORT_VERSION},2.9.20.0)
MYCFLAGS += -DREQ_ENGINE_LIB_MAJOR=3 -DREQ_ENGINE_LIB_MINOR=2 -DNO_NON_ETHER_DECODER
SEEN=1
endif


# libs is the base name for each of the categories
libs := $(shell ls ?*\_*.c | cut -d_ -f1 | grep -v deleted | grep -v so-util | uniq)

# files is the list of object modules we want to create for SO rule source files
files := $(shell for i in ${libs}; do ls $$i\_*.c | cut -d. -f1; done)

# cats is the category.c files generated by category_build.pl
cats := $(shell for i in ${libs}; do echo $$i.c; done)

# utils is the list of object modules we want to create for utility source files
utils := $(shell ls so-util_*.c | cut -d. -f1)

# MYUTILS is the list of specific utility objects to compile in per category
MYUTILS = $(shell for i in `grep '//REQUIRES:' $@.c | cut -d: -f2`; do echo so-util_$$i.o; done)

OS=$(shell echo $$OSTYPE)
ifeq (darwin, $(findstring darwin, $(OS)))
LD_FLAGS = -bundle -undefined dynamic_lookup
ifneq (darwin12, $(findstring darwin12, $(OS)))
LD_FLAGS += -lSystemStubs
export MACOSX_DEPLOYMENT_TARGET=10.4
else
export MACOSX_DEPLOYMENT_TARGET=10.8
endif
else
LD_FLAGS = -shared
endif

ifneq (,$(findstring FreeBSD, $(OS)))
LD_FLAGS += -ldl
endif

# Needed for sin()
LD_FLAGS += -lm

all: $(files) $(utils) $(cats) $(libs)
	@rm -f test.conf
	@echo "# include deleted.rules" > test.conf
	@for i in ${libs}; do echo include $$i.rules >> test.conf; done

$(files):
ifneq (${SEEN}, 1)
	@echo "Unsupported snort version - ${SNORT_VERSION}"
	@false
endif
	@$(CC) -c $(MYCFLAGS) -o $@.o $@.c

$(cats):
	@echo -n "generating $@ ... "
	@perl category-build.pl $@
	@echo done

$(libs): $(cats) $(utils) $(files)
	@echo -n "building $@ ... "
	@$(CC) -c $(MYCFLAGS) -D DETECTION_LIB_NAME=\"$@\" -o $@.o $@.c
	@$(CC) -c $(MYCFLAGS) -D DETECTION_LIB_NAME=\"$@\" -o _meta.o _meta.c
	@$(CC) $@_*.o _meta.o $@.o so-util_base.o ${MYUTILS} -o $@.so $(LD_FLAGS)
	@$(SNORT) -q --dynamic-engine-lib=$(ENGINE) \
		--dynamic-detection-lib=./$@.so \
		--dump-dynamic-rules=./
	@echo done
	-@rm _meta.o

$(utils):
	@echo -n "building utility library $(shell echo -n $@ | cut -d_ -f2) ... "
	@$(CC) -c $(MYCFLAGS) -o $@.o -fvisibility=hidden $@.c
	@echo done

test:
	@perl tests/test.pl -basedir ${BASEDIR} -testdir tests -version ${SNORT_VERSION} >/dev/null

write-results:
	@perl tests/write.pl -basedir ${BASEDIR} -testdir tests -version ${SNORT_VERSION} >/dev/null

distclean: clean

clean:
	-@rm -rf *.o *.so *.rules
	-@for i in ${libs}; do rm -f $$i.c; done
