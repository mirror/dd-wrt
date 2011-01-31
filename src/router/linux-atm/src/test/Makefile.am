USRPGMS = aread awrite ttcp_atm atmswitch svc_recv svc_send @pmp_send@
EXTRA_PROGRAMS = pmp_send
PGMS = align aping br bw isp window
bin_PROGRAMS = $(USRPGMS)
noinst_PROGRAMS = $(PGMS)

LDADD = $(top_builddir)/src/lib/libatm.la

atmswitch_SOURCES = atmswitch.c
aread_SOURCES = aread.c
awrite_SOURCES = awrite.c
svc_recv_SOURCES = svc_recv.c
svc_send_SOURCES = svc_send.c
ttcp_atm_SOURCES = ttcp.c
pmp_send_SOURCES = pmp_send.c

align_SOURCES = align.c
aping_SOURCES = aping.c
br_SOURCES = br.c
bw_SOURCES = bw.c
isp_SOURCES = isp.c isp.h ispl_y.y ispl_l.l
isp_XTRAS = $(LDADD)
isp_LDADD = $(isp_XTRAS)
isp_DEPENDENCIES = $(isp_XTRAS)
window_SOURCES = window.c

CLEANFILES = errnos.inc

EXTRA_DIST = ispl_y.h README.isp
