#
# Makefile
#
# Version:	$Id: b7f60306e1c36c0b3ce817d493c74c1c59a3bc10 $
#

TARGET      = rlm_eap_md5
SRCS        = rlm_eap_md5.c eap_md5.c
HEADERS     = eap_md5.h
RLM_CFLAGS  = -I../.. -I../../libeap
RLM_INSTALL =

RLM_DIR=../../
include ${RLM_DIR}../rules.mak

$(LT_OBJS): $(HEADERS)
