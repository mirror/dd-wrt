bin_PROGRAMS=rtpproxy makeann
rtpproxy_SOURCES=main.c rtp.h rtp_server.c rtp_server.h \
  rtpp_defines.h rtpp_log.h rtpp_record.c rtpp_record.h rtpp_session.h \
  rtpp_util.c rtpp_util.h rtp.c rtp_resizer.c rtp_resizer.h rtpp_session.c \
  rtpp_command.c rtpp_command.h rtpp_log.c
rtpproxy_LDADD=-lm
dist_man_MANS=rtpproxy.8
makeann_SOURCES=makeann.c rtp.h g711.h
makeann_LDADD=@LIBS_G729@ @LIBS_GSM@
EXTRA_DIST=README.remote manpage.xml debian/README.Debian debian/changelog \
  debian/compat debian/conffiles.ex debian/control debian/copyright \
  debian/cron.d.ex debian/dirs debian/docs debian/manpage.1.ex \
  debian/manpage.sgml.ex debian/manpage.xml.ex debian/menu.ex \
  debian/postinst.ex debian/postrm.ex debian/preinst.ex debian/prerm.ex \
  debian/rtpproxy-default.ex debian/rtpproxy.doc-base.EX \
  debian/rtpproxy.init debian/rules debian/watch.ex  rpm/rtpproxy.init \
  rpm/rtpproxy.spec freebsd/rtpproxy.in

rtpproxy.8: manpage.xml
	xsltproc /usr/local/share/xsl/docbook/manpages/docbook.xsl manpage.xml
