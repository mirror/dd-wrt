include ../Makefile.inc

.PHONY: all clean

clean:
	rm -fr html latex man nmealib.pdf

all: clean nmea.doxygen
	@echo "Updating version..."
	@sed -r "s/^([[:space:]]*PROJECT_NUMBER[[:space:]]*=).*/\1 $(GITVERSION)/" nmea.doxygen > nmea.doxygen.temp
	@echo "Generating HTML and man pages..."
	@doxygen nmea.doxygen.temp
	@rm nmea.doxygen.temp
	@echo "Generating PDF..."
	@make -C latex -s > /dev/null 2>&1
	@mv latex/refman.pdf nmealib.pdf
	@rm -fr latex
	@echo "Done"
