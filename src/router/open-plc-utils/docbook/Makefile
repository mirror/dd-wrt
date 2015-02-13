#!/bin/make -f
# file: docbook/Makefile

# ====================================================================
# environment definitions;
# --------------------------------------------------------------------

include ../make.def

PROJECT=open-plc-utils
DOCBOOK=/usr/share/xml/docbook/stylesheet/docbook-xsl/fo  
DOCBOOK=/usr/share/xml/docbook/stylesheet/docbook-xsl/html
LIBRARY=${DOC}/${PROJECT}
XFLAGS=--skip-validation

# ====================================================================
# symbols;
# --------------------------------------------------------------------

TRASH=*~ *.[0-9][0-9][0-9] t t.* *.1 *.1a *.1.html *.c.html *.cpp.html *.h.html *.hpp.html *.1.html *.1a.html *.sh.html *.xml.html *.xsd.html ch[0-9][0-9]*.html 
STYLES=param.xsl lists.xsl synop.xsl
BOOKS=index.html 

# ====================================================================
# pseudo targets;
# --------------------------------------------------------------------

all: compile install
compile: manuals ${TOOLS} ${BOOKS}
	rm -f *.[0-9][0-9][0-9]
manuals: 
	${SHELL} ../docbook/manuals
scripts:
library:
	install -m ${DIR_PERM} -o ${OWNER} -g ${GROUP} -d ${LIBRARY}
install: library
	if [ -d ${LIBRARY} ]; then install -m ${WWW_PERM} -o ${OWNER} -g ${GROUP} *.html *.css *.png ${LIBRARY}; fi
uninstall:
	rm -f ${LIBRARY}/*.c.html ${LIBRARY}/*.h.html ${LIBRARY}/*.1.html
setup:
	install ${STYLES} ${DOCBOOK}
clean:
	rm -f ${PAGES} ${TRASH} 
fresh: clean compile

# ====================================================================
# pseudo targets;
# --------------------------------------------------------------------

index.html: open-plc-utils.xml *.xml *.png
	xmlto ${XFLAGS} -o . html ${<} 
open-plc-utils.pdf: *.xml *.png
	docbook2pdf ${<}
open-plc-utils.txt: *.xml 
	xmlto ${XFLAGS} txt ${<}

