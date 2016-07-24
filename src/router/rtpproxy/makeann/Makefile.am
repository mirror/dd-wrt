bin_PROGRAMS=makeann

extractaudio_SRCDIR=$(top_srcdir)/extractaudio

makeann_SOURCES=makeann.c rtp.h g711.h ${extractaudio_SRCDIR}/g729_compat.c \
  ${extractaudio_SRCDIR}/g729_compat.h
makeann_LDADD=@LIBS_G729@ @LIBS_GSM@ @LIBS_G722@
makeann_CPPFLAGS=-I${extractaudio_SRCDIR}

DEFS=	-Wall @DEFS@
