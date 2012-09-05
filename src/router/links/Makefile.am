codepage_sources=Unicode/gen-cp Unicode/*.cp Unicode/index.txt
entities_sources=Unicode/gen-ent Unicode/entities.lnx
uni7b_sources=Unicode/gen-7b Unicode/7bitrepl.lnx
lang_sources=intl/gen-intl intl/*.lng intl/index.txt

man_MANS=links.1

EXTRA_DIST=$(man_MANS) Unicode/* intl/* config2.h Makefile.gen BUGS TODO SITES mailcap.pl wipe-out-ssl wipe-out-ssl.awk links.nsi

bin_PROGRAMS=links
links_SOURCES=af_unix.c beos.c bfu.c bookmark.c cache.c charsets.c connect.c cookies.c default.c dns.c error.c file.c finger.c ftp.c html.c html_r.c html_tbl.c http.c https.c kbd.c kbdbind.c language.c mailto.c main.c menu.c os_dep.c sched.c select.c session.c smb.c terminal.c types.c url.c view.c links.h os_dep.h os_depx.h setup.h codepage.h language.h codepage.inc entity.inc uni_7b.inc language.inc upcase.inc rebuild rebuild.cmd

datadir = $(prefix)/@DATADIRNAME@
LIBS = @LIBS@

#codepage.inc : $(codepage_sources)
#	cd Unicode;./gen-cp;cd ..

#entity.inc : $(entities_sources)
#	cd Unicode;./gen-ent;cd ..

#uni_7b.inc : $(uni7b_sources)
#	cd Unicode;./gen-7b;cd ..

#language.inc : $(lang_sources)
#	cd intl;./gen-intl;cd ..

