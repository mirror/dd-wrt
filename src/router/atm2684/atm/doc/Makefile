.SUFFIXES:	.tex .txt .dvi .ps

all:		ps ascii

dvi:		usage.dvi
ps:		usage.ps
ascii:		usage.txt

usage.txt:	usage.tex t2a.pl
		./t2a.pl <usage.tex >usage.txt

usage.dvi:	usage.tex
		./rlatex usage

usage.ps:	usage.dvi
		dvips -o usage.ps usage

clean:
		rm -f *.aux *.log *.toc

spotless:	clean
		rm -f *.dvi *.ps *.txt
