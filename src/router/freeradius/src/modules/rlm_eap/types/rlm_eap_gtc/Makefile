#
# Makefile
#
# Version:	$Id: Makefile,v 1.2.4.1 2006/02/10 19:47:11 nbk Exp $
#

TARGET      = rlm_eap_gtc
SRCS        = rlm_eap_gtc.c
RLM_CFLAGS  = $(INCLTDL) -I../.. -I../../libeap
HEADERS     =  ../../rlm_eap.h ../../eap.h
RLM_INSTALL =

RLM_DIR=../../
include ${RLM_DIR}../rules.mak

$(LT_OBJS): $(HEADERS)
