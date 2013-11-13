#
# Makefile
#
# Version:	$Id: 4d4b72f4838059c2c0a58e40db3a704e95be8b99 $
#

TARGET      = rlm_eap_mschapv2
SRCS        = rlm_eap_mschapv2.c
HEADERS     = eap_mschapv2.h
RLM_CFLAGS  = -I../.. -I../../libeap
RLM_INSTALL =

RLM_DIR=../../
include ${RLM_DIR}../rules.mak

$(LT_OBJS): $(HEADERS)
