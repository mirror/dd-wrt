/*
   Copyright (C) 2010 bg <bg_one@mail.ru>
*/
#ifndef CHAN_DONGLE_PDU_H_INCLUDED
#define CHAN_DONGLE_PDU_H_INCLUDED

#include <sys/types.h>			/* size_t */
#include "export.h"			/* EXPORT_DECL EXPORT_DEF */
#include "char_conv.h"			/* str_encoding_t */

EXPORT_DECL char pdu_digit2code(char digit);
EXPORT_DECL int pdu_build(char * buffer, size_t length, const char * csca, const char * dst, const char * msg, unsigned valid_minutes, int srr);
EXPORT_DECL const char * pdu_parse(char ** pdu, size_t tpdu_length, char * oa, size_t oa_len, str_encoding_t * oa_enc, char ** msg, str_encoding_t * msg_enc);
EXPORT_DECL int pdu_parse_sca(char ** pdu, size_t * length);

#endif /* CHAN_DONGLE_PDU_H_INCLUDED */
