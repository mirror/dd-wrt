/*
 * ctrlpacket.h
 *
 * Functions to parse and send pptp control packets.
 */

#ifndef _PPTPD_CTRLPACKET_H
#define _PPTPD_CTRLPACKET_H

int read_pptp_packet(int clientFd, void *packet, struct pptp_out_call_rply *rply_packet, ssize_t * rply_size);
ssize_t send_pptp_packet(int clientFd, void *packet, size_t packet_size);
void make_echo_req_packet(struct pptp_out_call_rply *rply_packet, ssize_t * rply_size, u_int32_t echo_id);
void make_call_admin_shutdown(struct pptp_out_call_rply *rply_packet, ssize_t * rply_size);
void make_stop_ctrl_req(struct pptp_out_call_rply *rply_packet, ssize_t * rply_size);

#endif  /* !_PPTPD_CTRLPACKET_H */
