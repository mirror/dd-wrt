/*
 * aton.h
 *
 * Copyright (c) 2002 Dug Song <dugsong@monkey.org>
 *
 * $Id: aton.h 274 2002-02-08 07:26:58Z dugsong $
 */

#ifndef ATON_H
#define ATON_H

int	type_aton(char *string, uint16_t *type);
int	op_aton(char *string, uint16_t *op);
int	proto_aton(char *string, uint8_t *proto);
int	off_aton(char *string, uint16_t *off);
int	port_aton(char *string, uint16_t *port);
int	seq_aton(char *string, uint32_t *seq);
int	flags_aton(char *string, uint8_t *flags);
int	fmt_aton(char *string, u_char *buf);

#endif /* ATON_H */
