# Makefile for libipvs

#CC		= gcc
CFLAGS = $(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE  -ffunction-sections -fdata-sections -Wl,--gc-sections
#ifneq (0,$(HAVE_NL))
#CFLAGS		+= -DLIBIPVS_USE_NL
#endif

INCLUDE		+= $(shell if [ -f ../../ip_vs.h ]; then	\
		     echo "-I../../."; fi;)
DEFINES		= $(shell if [ ! -f ../../ip_vs.h ]; then	\
		    echo "-DHAVE_NET_IP_VS_H"; fi;)

.PHONY		= all clean install dist distclean rpm rpms
STATIC_LIB	= libipvs.a
SHARED_LIB	= libipvs.so

all:		$(STATIC_LIB)

$(STATIC_LIB):	libipvs.o ip_vs_nl_policy.o
		ar rv $@ $^

$(SHARED_LIB):	libipvs.o ip_vs_nl_policy.o
		$(CC) -shared -Wl,-soname,$@ -o $@ $^

%.o:		%.c
		$(CC) $(CFLAGS) $(INCLUDE) $(DEFINES) -c -o $@ $<

clean:
		rm -f *.[ao] *~ *.orig *.rej core *.so

distclean:	clean
