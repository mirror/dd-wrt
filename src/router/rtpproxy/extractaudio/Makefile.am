bin_PROGRAMS=extractaudio
MAINSRCDIR=$(top_srcdir)/src
extractaudio_SOURCES=main.c decoder.c rtpp_loader.c ${MAINSRCDIR}/rtp.c \
  ${MAINSRCDIR}/rtp_analyze.c ${MAINSRCDIR}/rtpp_util.c \
  ${MAINSRCDIR}/rtpp_log_stand.c decoder.h format_au.h g711.h rtpp_loader.h \
  session.h g729_compat.c g729_compat.h
extractaudio_LDADD=@LIBS_G729@ @LIBS_GSM@ @LIBS_G722@ @LIBS_SNDFILE@ -lm -lpthread

DEFS=   -DWITHOUT_SIPLOG -Wall @DEFS@
