######################
#
# Highlevel configuration options for all
#
#

# activate debugging with 1 or deactivate with 0
DEBUG ?= 1


######################
#
# Lowlevel options and rules
#

ifeq ($(DEBUG),0)
STRIP ?=	strip
else
STRIP ?=	:
endif


# we expect the version to be like 'v0.5.3-27-g0c2727a' and then strip the 'v',
# and the '-27-g0c2727a' parts
VERSION=0.6.9

# protect against no version number
ifeq ($(strip $(VERSION)),)
VERSION=0.0.0
endif

CC ?= gcc
CFLAGS ?= -Wall -Wextra -Wold-style-definition -Wdeclaration-after-statement -Wmissing-prototypes -Wstrict-prototypes \
          -Wmissing-declarations -Wsign-compare -Waggregate-return -Wmissing-noreturn -Wmissing-format-attribute \
          -Wno-multichar -Wno-deprecated-declarations -Wendif-labels -Wwrite-strings -Wbad-function-cast \
          -Wpointer-arith -Wcast-qual -Wshadow -Wformat -Wsequence-point -Wcast-align -Werror -Wnested-externs \
          -Winline -Wdisabled-optimization -finline-functions-called-once -funit-at-a-time -fearly-inlining \
          -finline-limit=350 -O2  -fPIC -ggdb -fPIC -O2 -Wall -Wextra -Wformat=2 -Winit-self -Wmissing-include-dirs \
          -Wswitch-default -Wswitch-enum -Werror
