/* Copyright (C) 1999 Heikki Vatiainen hessu@cs.tut.fi */

#ifndef FRAMES_H
#define FRAMES_H

#include "frame_defs.h"

#define MAX_CTRL_FRAME 512

void prefill_frame(void *ctrl_frame, uint16_t type);
int validate_frame(unsigned char *buff, int size);

void send_ready_ind(Conn_t *conn);
void send_register_req(void);

int handle_frame(Conn_t *conn, char *buff, int size);
uint32_t send_flush_req(Conn_t *conn);

void parse_tlvs(uint16_t opcode, unsigned char *tlvp, int numtlvs, int sizeoftlvs);

#endif /* FRAMES_H */
