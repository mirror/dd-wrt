#
# Makefile for Documentation
#

html: manual.sgml
	docbook2html manual.sgml

pspdf: manual.sgml
	docbook2ps manual.sgml
	ps2pdf manual.ps

manual.sgml:	
	cat manual.template | sed -e s/@@version@@/`cat version`/ \
	-e s/@@major@@/`awk -F '.' -f version.awk  < version`/ \
	> manual.sgml

clean:
	rm -f *~ *.html manual.sgml


