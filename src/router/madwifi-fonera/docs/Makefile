DOCS = users-guide.pdf
DOCSOURCES = users-guide.tex

PS2PDF = ps2pdf
DVIPS = dvips
LATEX = latex

LATEX_RERUN = grep -i "LaTeX Warning:.*Rerun"

all: $(DOCS)

%.ps: %.dvi
	$(DVIPS) -o $@ $<

%.pdf: %.ps
	$(PS2PDF) $< $@

%.dvi: %.tex
	while $(LATEX) $< </dev/null; do \
		test -f $*.log || exit 1; \
		$(LATEX_RERUN) $*.log || exit 0; \
	done

clean:
	rm -f *.dvi *.ps *.aux *.log *.toc

maintainer-clean: clean
	rm -f *.pdf
