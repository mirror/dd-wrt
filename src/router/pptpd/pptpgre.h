/*
 * pptpgre.h
 *
 * Functions to handle the GRE en/decapsulation
 *
 * $Id: pptpgre.h,v 1.1.1.1 2002/06/21 08:52:01 fenix_nl Exp $
 */

#ifndef _PPTPD_PPTPGRE_H
#define _PPTPD_PPTPGRE_H

extern int decaps_hdlc(int fd, int (*cb) (int cl, void *pack, unsigned len), int cl);
extern int encaps_hdlc(int fd, void *pack, unsigned len);
extern int decaps_gre(int fd, int (*cb) (int cl, void *pack, unsigned len), int cl);
extern int encaps_gre(int fd, void *pack, unsigned len);

extern int pptp_gre_init(u_int32_t call_id_pair, int pty_fd, struct in_addr *inetaddrs);

struct gre_state {
	u_int32_t ack_sent, ack_recv;
	u_int32_t seq_sent, seq_recv;
	u_int32_t call_id_pair;
};

#endif	/* !_PPTPD_PPTPGRE_H */
