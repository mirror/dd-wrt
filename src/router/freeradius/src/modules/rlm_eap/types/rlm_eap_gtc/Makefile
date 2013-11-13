#
# Makefile
#
# Version:	$Id: 0faf33b63ceadad1dfdc3df1e414a2ee03f329c5 $
#

TARGET      = rlm_eap_gtc
SRCS        = rlm_eap_gtc.c
RLM_CFLAGS  = -I../.. -I../../libeap
HEADERS     =  ../../rlm_eap.h ../../eap.h
RLM_INSTALL =

RLM_DIR=../../
include ${RLM_DIR}../rules.mak

$(LT_OBJS): $(HEADERS)
