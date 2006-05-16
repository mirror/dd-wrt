PSFILES=ip-cref.ps ip-tunnels.ps api-ip6-flowlabels.ps
# tc-cref.ps
# api-rtnl.tex api-pmtudisc.tex api-news.tex
# iki-netdev.ps iki-neighdst.ps


LATEX=latex
DVIPS=dvips
LPR=lpr -Zsduplex
SHELL=bash

DVIFILES=$(subst .ps,.dvi,$(PSFILES))


all: pstwocol

pstwocol: $(PSFILES)

dvi: $(DVIFILES)

print: $(PSFILES)
	$(LPR) $(PSFILES)

%.dvi: %.tex
	@set -e; pass=2; echo "Running LaTeX $<"; \
	while [ `$(LATEX) $< </dev/null 2>&1 | \
		 grep -c '^\(LaTeX Warning: Label(s) may\|No file \|! Emergency stop\)'` -ge 1 ]; do \
		if [ $$pass -gt 3 ]; then \
			echo "Seems, something is wrong. Try by hands." ; exit 1 ; \
		fi; \
		echo "Re-running LaTeX $<, $${pass}d pass"; pass=$$[$$pass + 1]; \
	done

%.ps: %.dvi
	$(DVIPS) $< -o $@.tmp
	./do-psnup $@.tmp $@
	rm -f $@.tmp

clean:
	rm -f *.aux *.log *.toc $(PSFILES) $(DVIFILES)
