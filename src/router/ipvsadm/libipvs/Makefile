# Makefile for libipvs
#CC            = gcc
CFLAGS = $(COPTS) $(MIPS16_OPT) -I$(TOP)/libnl-tiny/include -D_GNU_SOURCE  -ffunction-sections -fdata-sections -Wl,--gc-sections
ifneq (0,$(HAVE_NL))
CFLAGS		+= -DLIBIPVS_USE_NL
CFLAGS		+= $(shell \
		if which pkg-config > /dev/null 2>&1; then \
		  if   pkg-config --cflags libnl-3.0  2> /dev/null; then :; \
		  elif pkg-config --cflags libnl-2.0  2> /dev/null; then :; \
		  elif pkg-config --cflags libnl-1    2> /dev/null; then :; \
		  fi; \
		fi)
endif

INCLUDE		+= $(shell if [ -f ../../ip_vs.h ]; then	\
		     echo "-I../../."; fi;)
DEFINES		= $(shell if [ ! -f ../../ip_vs.h ]; then	\
		    echo "-DHAVE_NET_IP_VS_H"; fi;)
DEFINES		+= $(shell if which pkg-config > /dev/null 2>&1; then \
			 if   pkg-config --exists libnl-3.0; then :; \
			 elif pkg-config --exists libnl-2.0; then :; \
			 elif pkg-config --exists libnl-1; \
			 then echo "-DFALLBACK_LIBNL1"; fi; fi)

.PHONY		= all clean install dist distclean rpm rpms
STATIC_LIB	= libipvs.a
SHARED_LIB	= libipvs.so

all:		$(STATIC_LIB) $(SHARED_LIB)

$(STATIC_LIB):	libipvs.o ip_vs_nl_policy.o
		ar rv $@ $^

$(SHARED_LIB):	libipvs.o ip_vs_nl_policy.o
		$(CC) -shared -Wl,-soname,$@ -o $@ $^

%.o:		%.c
		$(CC) $(CFLAGS) $(INCLUDE) $(DEFINES) -c -o $@ $<

clean:
		rm -f *.[ao] *~ *.orig *.rej core *.so

distclean:	clean
