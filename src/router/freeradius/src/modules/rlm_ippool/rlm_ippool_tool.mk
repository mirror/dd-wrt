#
# $Id: 4748ee8a8d3095c78a6cdea0c486630d748ec17d $
#

SOURCES		:= rlm_ippool_tool.c
TARGET		:= rlm_ippool_tool
TGT_PREREQS	:= libfreeradius-radius.a

SRC_CFLAGS	:= $(rlm_ippool_CFLAGS)
TGT_LDLIBS	:= $(LIBS) $(rlm_ippool_LDLIBS)

MAN		:= rlm_ippool_tool.8
