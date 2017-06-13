#
# $Id: 566adfab64255dc8f1b08a397f65d69987474a2f $
#

SOURCES		:= rlm_ippool.c
TARGET		:= rlm_ippool.a

SRC_CFLAGS	:= $(rlm_ippool_CFLAGS) 
TGT_LDLIBS	:= $(rlm_ippool_LDLIBS)
