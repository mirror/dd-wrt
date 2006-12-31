/* Copyright (C) 1999 Heikki Vatiainen hessu@cs.tut.fi */

#ifndef DISPLAY_H
#define DISPLAY_H

void display_frame(void *frame);

void mac2text(char *buff, unsigned char *mac_addr);
const char *opcode2text(uint16_t opcode);
const char *tlv2text(uint32_t type);
const char *status2text(uint16_t status);

#endif /* DISPLAY_H */
