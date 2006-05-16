root-rel=../
dir-name=doc

ifneq ($(wildcard ../Rules),)
include ../Rules
else
srcdir=$(shell cd $(root-rel) ; pwd)
srcdir_abs=$(srcdir)
endif

# Force rebuilds
.PHONY: prog.sgml bird.sgml

docs: progdocs userdocs
progdocs: prog.html prog.ps
userdocs: bird.html bird.ps

prog.sgml:
	$(srcdir)/tools/progdoc $(srcdir_abs)

%.html: %.sgml
	./sgml2html $<

%.dvi: %.tex
	latex $<
	latex $<

%.ps: %.dvi
	dvips -D600 -ta4 -o $@ $<

%.tex: %.sgml
	./sgml2latex --output=tex $<

%.txt: %.sgml
	./sgml2txt $<

progspell: prog.sgml
	sed -f prog-spell.sed <prog.sgml >prog.spell
	ispell prog.spell

clean:
	rm -f *.tex *.dvi *.log *.txt *.aux *.toc *.spell
	rm -f prog.sgml

distclean: clean
	rm -f *.html *.ps
