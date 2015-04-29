#
# Makefile
#
# Version:	$Id: b30947fc0e45ff1f2aaa2d19462f29db81bddc12 $
#

TARGET      = rlm_eap_sim
SRCS        = rlm_eap_sim.c
HEADERS     =
RLM_CFLAGS  = -I../.. -I../../libeap
RLM_LIBS    =  ../../libeap/$(LIBPREFIX)freeradius-eap.la
RLM_INSTALL =

RLM_DIR=../../
include ${RLM_DIR}../rules.mak

$(LT_OBJS): $(HEADERS)
