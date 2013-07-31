all: slt.dvi

slt.dvi: slt.tex
	csplain slt.tex

slt.ps: slt.dvi
	dvips -o slt.ps -D600 -ta4 slt.dvi

clean:
	rm -f *~ *.log *.tfm *.*pk *.*gf *.ps *.dvi
