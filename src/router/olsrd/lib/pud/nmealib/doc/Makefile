include ../Makefile.inc

.PHONY: all clean

clean:
	$(MAKECMDPREFIX)rm -fr html latex man nmealib.pdf

all: clean nmea.doxygen
ifeq ($(VERBOSE),0)
	@echo "Updating version..."
endif
	$(MAKECMDPREFIX)sed -r "s/^([[:space:]]*PROJECT_NUMBER[[:space:]]*=).*/\1 $(GITVERSION)/" nmea.doxygen > nmea.doxygen.temp
ifeq ($(VERBOSE),0)
	@echo "Generating HTML and man pages..."
endif
	$(MAKECMDPREFIX)doxygen nmea.doxygen.temp
	$(MAKECMDPREFIX)rm nmea.doxygen.temp
#ifeq ($(VERBOSE),0)
#	@echo "Generating PDF..."
#	@$(MAKE) -C latex -s > /dev/null 2>&1
#else
#	$(MAKE) -C latex
#endif
#	$(MAKECMDPREFIX)mv latex/refman.pdf nmealib.pdf
#	$(MAKECMDPREFIX)rm -fr latex
ifeq ($(VERBOSE),0)
	@echo "Done"
endif

