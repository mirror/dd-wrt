/* Copyright 2015 The FreeRADIUS server project */

#ifndef _AUTH_WBCLIENT_H
#define _AUTH_WBCLIENT_H

RCSIDH(auth_wbclient_h, "$Id: 9ad5a2f771df5b9cb25c2a4dc43e61a81d7d9e8f $")

int do_auth_wbclient(rlm_mschap_t *inst, REQUEST *request,
		     uint8_t const *challenge, uint8_t const *response,
		     uint8_t nthashhash[NT_DIGEST_LENGTH]);

#endif /*_AUTH_WBCLIENT_H*/
