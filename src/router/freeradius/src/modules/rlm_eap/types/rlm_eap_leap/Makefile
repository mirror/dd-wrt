#
# Makefile
#
# Version:	$Id: 43eecaea221e245b1a3235ac39cad3b4f18d0d5e $
#

TARGET      = rlm_eap_leap
SRCS        = rlm_eap_leap.c eap_leap.c smbdes.c
RLM_CFLAGS  = -I../.. -I../../libeap
HEADERS     = eap_leap.h  ../../eap.h ../../rlm_eap.h
RLM_INSTALL =

RLM_DIR=../../
include ${RLM_DIR}../rules.mak

$(LT_OBJS): $(HEADERS)
