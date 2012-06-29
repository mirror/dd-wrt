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
	@echo "[LD] $@"
	@$(CC) $(CFLAGS) $< $(LIBRARIES) -o $@

%/main.o: %/main.c Makefile ../Makefile.inc
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@


#
# Phony Targets
#

.PHONY: all all-before clean samples

all-before:
	@mkdir -p ../build/samples

clean:
	rm -f $(SMPLOBJ) $(SMPLS)
