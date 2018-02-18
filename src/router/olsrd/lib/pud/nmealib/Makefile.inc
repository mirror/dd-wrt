  ######################
#
# Settings
#
#

LIBNAME = libnmea.so
LIBNAMESTATIC = libnmea.a


######################
#
# Highlevel configuration options for all
#
#

# activate debugging with 1 or deactivate with 0
DEBUG ?= 1

# set NOSTRIP to non-zero to inhibit stripping the binaries
NOSTRIP ?= 0

# shows full compiler/linker calls if activated
VERBOSE ?= 0

ifeq ($(VERBOSE),0)
MAKECMDPREFIX = @
else
MAKECMDPREFIX =
endif

######################
#
# Lowlevel options and rules
#

ifeq ($(DEBUG),0)
  ifeq ($(NOSTRIP),0)
  STRIP ?=  strip
  else
  STRIP ?=  :
  endif
else
STRIP ?= :
endif


# we expect the version to be like 'v0.5.3-27-g0c2727a' and then strip the 'v',
# and the '-27-g0c2727a' parts
VERSION=3.0.0

# protect against no version number
ifeq ($(strip $(VERSION)),)
VERSION=0.0.0
endif

CC ?= gcc

GCCVERSIONGTEQ6 := $(shell expr `$(CC) -dumpversion | cut -f1 -d.` \>= 6)

COMMONCFLAGS += -Wall -Wextra -Wold-style-definition -Wdeclaration-after-statement -Wmissing-prototypes \
                -Wstrict-prototypes -Wmissing-declarations -Wsign-compare -Waggregate-return -Wmissing-noreturn \
                -Wmissing-format-attribute -Wno-multichar -Wno-deprecated-declarations -Wendif-labels -Wwrite-strings \
                -Wbad-function-cast -Wpointer-arith -Wcast-qual -Wshadow -Wformat -Wsequence-point -Wcast-align \
                -Wnested-externs -Winline -Wdisabled-optimization -funit-at-a-time -fPIC -ggdb -Wformat=2 -Winit-self \
                -Wswitch-default -Wswitch-enum -Wconversion -Wdouble-promotion \
                -Werror=format-security -Wformat-security -Wformat-y2k -Wredundant-decls -Wundef -Wunreachable-code \
                -Wunused-parameter

GCCCFLAGS    += $(COMMONCFLAGS) -fearly-inlining -finline-functions-called-once -finline-limit=350 -Wtrampolines \
                -Wsync-nand -Wlogical-op -Wjump-misses-init -Werror

ifeq "$(GCCVERSIONGTEQ6)" "1"
GCCCFLAGS   += -Wnull-dereference -Wshift-negative-value -Wshift-overflow -Wtautological-compare
GCCCFLAGS   += -Wmisleading-indentation -Wduplicated-cond
endif

CLANGCFLAGS := $(COMMONCFLAGS)

ifdef COV
CFLAGS += -fprofile-arcs -ftest-coverage
LDFLAG += -lgov
endif

ifneq ($(CC),clang)
CFLAGS += $(GCCCFLAGS)
else
CFLAGS += $(CLANGCFLAGS)
endif

ifeq ($(DEBUG),0)
CFLAGS += -O2
else
CFLAGS += -O0
endif

LDFLAGS = -shared -Wl,--warn-common -fPIC

# 32/64 cross compilation
ifdef M32
CFLAGS += -m32
LDFLAGS += -m32
endif
ifdef M64
CFLAGS += -m64
LDFLAGS += -m64
endif


export COV
export DEBUG
export M32
export M64
export VERBOSE
