include ../Makefile.inc

#
# Settings
#

SAMPLES = generate generator math parse parse_file
SMPLS = $(SAMPLES:%=../build/samples/%)
SMPLOBJ = $(SAMPLES:%=%/main.o)

LIBRARIES = -lm -L../lib -lnmea
INCLUDES = -I ../include


#
# Targets
#

all: all-before samples

remake: clean all

samples: $(SMPLS)

../build/samples/%: %/main.o
ifeq ($(VERBOSE),0)
	@echo "[LD] $@"
endif
	$(MAKECMDPREFIX)$(CC) $(CFLAGS) $< $(LIBRARIES) -o $@

%/main.o: %/main.c Makefile ../Makefile.inc
ifeq ($(VERBOSE),0)
	@echo "[CC] $<"
endif
	$(MAKECMDPREFIX)$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@


#
# Phony Targets
#

.PHONY: all all-before clean samples

all-before:
	$(MAKECMDPREFIX)mkdir -p ../build/samples

clean:
	$(MAKECMDPREFIX)rm -f $(SMPLOBJ) $(SMPLS)
