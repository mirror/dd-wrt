EXTRA_DIST = README.CLP README.tc atm-linux-howto.sgml atm-linux-howto.txt

HOWTO = atm-linux-howto.sgml


atm-linux-howto/atm-linux-howto.html: ${HOWTO}
	sgmltools --backend=html ${HOWTO}

html: atm-linux-howto/atm-linux-howto.html


atm-linux-howto.txt: ${HOWTO}
	sgmltools --backend=txt ${HOWTO}

txt: atm-linux-howto.txt


atm-linux-howto.ps: ${HOWTO}
	sgmltools --backend=ps ${HOWTO}

ps: atm-linux-howto.ps


atm-linux-howto.pdf: ${HOWTO}
	db2pdf ${HOWTO}

pdf: atm-linux-howto.pdf


atm-linux-howto.rtf: ${HOWTO}
	db2rtf ${HOWTO}

rtf: atm-linux-howto.rtf


clean:
	-rm -rf atm-linux-howto/ *.ps *.pdf *.rtf *.out

