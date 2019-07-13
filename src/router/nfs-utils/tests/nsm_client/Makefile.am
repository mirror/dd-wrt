## Process this file with automake to produce Makefile.in

GENFILES_CLNT	= nlm_sm_inter_clnt.c
GENFILES_SVC	= nlm_sm_inter_svc.c
GENFILES_XDR	= nlm_sm_inter_xdr.c
GENFILES_H	= nlm_sm_inter.h

GENFILES	= $(GENFILES_CLNT) $(GENFILES_SVC) $(GENFILES_XDR) $(GENFILES_H)

AM_CFLAGS += -Wno-missing-prototypes -Wno-missing-declarations

EXTRA_DIST = nlm_sm_inter.x

check_PROGRAMS	= nsm_client
nsm_client_SOURCES = $(GENFILES) nsm_client.c

BUILT_SOURCES = $(GENFILES)
nsm_client_LDADD = ../../support/nfs/.libs/libnfs.a \
		   ../../support/nsm/libnsm.a $(LIBCAP) $(LIBTIRPC)

RPCGEN	= rpcgen
$(RPCGEN):
	make -C ../../tools/rpcgen all

$(GENFILES_CLNT): %_clnt.c: %.x $(RPCGEN)
	test -f $@ && rm -rf $@ || true
	$(RPCGEN) -l -o $@ $<

$(GENFILES_SVC): %_svc.c: %.x $(RPCGEN)
	test -f $@ && rm -rf $@ || true
	$(RPCGEN) -m -o $@ $<

$(GENFILES_XDR): %_xdr.c: %.x $(RPCGEN)
	test -f $@ && rm -rf $@ || true
	$(RPCGEN) -c -o $@ $<

$(GENFILES_H): %.h: %.x $(RPCGEN)
	test -f $@ && rm -rf $@ || true
	$(RPCGEN) -h -o $@ $<

MAINTAINERCLEANFILES = Makefile.in

CLEANFILES = $(GENFILES)

