## Process this file with automake to produce Makefile.in.

SUBDIRS = lib qpb fpm @ZEBRA@ @BGPD@ @RIPD@ @RIPNGD@ @OSPFD@ @OSPF6D@ @NHRPD@ \
         @ISISD@ @PIMD@ @WATCHQUAGGA@ @VTYSH@ @OSPFCLIENT@ @DOC@ m4 @pkgsrcdir@ \
         redhat @SOLARIS@ tests

DIST_SUBDIRS = lib qpb fpm zebra bgpd ripd ripngd ospfd ospf6d nhrpd \
	  isisd watchquagga vtysh ospfclient doc m4 pkgsrc redhat tests \
	  solaris pimd

EXTRA_DIST = aclocal.m4 SERVICES TODO REPORTING-BUGS INSTALL.quagga.txt \
	update-autotools \
	vtysh/Makefile.in vtysh/Makefile.am \
	tools/zebra.el tools/multiple-bgpd.sh

if HAVE_PANDOC

HACKING.pdf: HACKING.md
	pandoc -o $@ $<

clean-local:
	-$(LATEXMK) -C HACKING.tex

endif

ACLOCAL_AMFLAGS = -I m4
