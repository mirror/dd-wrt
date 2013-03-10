/*
   Copyright (C) 2010 bg <bg_one@mail.ru>
*/
#ifndef CHAN_DONGLE_AT_PARSE_H_INCLUDED
#define CHAN_DONGLE_AT_PARSE_H_INCLUDED

#include <sys/types.h>			/* size_t */

#include "export.h"		/* EXPORT_DECL EXPORT_DECL */
#include "char_conv.h"		/* str_encoding_t */
struct pvt;

EXPORT_DECL char* at_parse_cnum (char* str);
EXPORT_DECL char* at_parse_cops (char* str);
EXPORT_DECL int at_parse_creg (char* str, unsigned len, int* gsm_reg, int* gsm_reg_status, char** lac, char** ci);
EXPORT_DECL int at_parse_cmti (const char* str);
EXPORT_DECL const char* at_parse_cmgr (char** str, size_t len, char* oa, size_t oa_len, str_encoding_t* oa_enc, char** msg, str_encoding_t* msg_enc);
EXPORT_DECL int at_parse_cusd (char* str, int * type, char ** cusd, int * dcs);
EXPORT_DECL int at_parse_cpin (char* str, size_t len);
EXPORT_DECL int at_parse_csq (const char* str, int* rssi);
EXPORT_DECL int at_parse_rssi (const char* str);
EXPORT_DECL int at_parse_mode (char* str, int * mode, int * submode);
EXPORT_DECL int at_parse_csca (char* str, char ** csca);
EXPORT_DECL int at_parse_clcc (char* str, unsigned * call_idx, unsigned * dir, unsigned * state, unsigned * mode, unsigned * mpty, char ** number, unsigned * toa);
EXPORT_DECL int at_parse_ccwa(char* str, unsigned * class);



#endif /* CHAN_DONGLE_AT_PARSE_H_INCLUDED */
