## $Id$
AUTOMAKE_OPTIONS=foreign no-dependencies

EXTRA_DIST = \
snort_manual.pdf \
snort_manual.tex

dist_doc_DATA = \
AUTHORS \
BUGS \
CREDITS \
generators \
INSTALL \
NEWS \
PROBLEMS \
README \
README.active \
README.alert_order \
README.appid OpenDetectorDeveloperGuide.pdf \
README.asn1 \
README.counts \
README.csv \
README.daq \
README.dcerpc2 \
README.decode \
README.decoder_preproc_rules \
README.dnp3 \
README.dns \
README.event_queue \
README.file \
README.file_ips \
README.filters \
README.flowbits \
README.frag3 \
README.ftptelnet \
README.gre \
README.GTP \
README.ha \
README.http_inspect \
README.imap \
README.ipip \
README.ipv6 \
README.modbus \
README.multipleconfigs \
README.normalize \
README.pcap_readmode \
README.PerfProfiling \
README.PLUGINS \
README.pop \
README.ppm \
README.reload \
README.reputation \
README.sensitive_data \
README.sfportscan \
README.SMTP \
README.ssh \
README.ssl \
README.sip \
README.stream5 \
README.tag \
README.thresholding \
README.UNSOCK \
README.unified2 \
README.variables \
README.WIN32 \
TODO \
USAGE \
WISHLIST

DISTCLEANFILES= snort_manual.log snort_manual.toc snort_manual.aux snort_manual.pdf snort_manual.out snort_manual.idx 

docdir = ${datadir}/doc/${PACKAGE}

SUFFIXES = .tex .dvi .ps

.tex.dvi:
	if [ -f /etc/centos-release ]; then \
		latex $< && latex $< && latex $<; \
	else \
		pslatex $< && pslatex $< && pslatex $<; \
	fi

.dvi.ps:
	dvips $< -o

.ps.pdf:
	ps2pdf $<

.tex.html:
	latex2html -local_icons $<


# perhaps one day, we will have a Makefile in the signatures directory...
# SUBDIRS = signatures
