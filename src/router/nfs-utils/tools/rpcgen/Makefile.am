CLEANFILES = *~

bin_PROGRAMS = rpcgen
man_MANS = rpcgen.1

EXTRA_DIST=${MANS}

noinst_HEADERS = proto.h rpc_parse.h rpc_scan.h rpc_util.h

rpcgen_SOURCES = rpc_clntout.c rpc_cout.c rpc_hout.c rpc_main.c \
	rpc_parse.c rpc_sample.c rpc_scan.c rpc_svcout.c rpc_tblout.c \
	rpc_util.c
rpcgen_LDADD = $(LIBINTL)
