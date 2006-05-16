/*
 *	DDNS v3 Client
 *
 *		By:	Alan Yates <alany@ay.com.au>
 *		Date:	27-08-2000
 */

#ifndef _CRYPTO_H
#define _CRYPTO_H

char *ddns3_crypto_md5hash(char *s, int len);
char *ddns3_crypto_crypt(char *key, char *salt);

#endif /* _CRYPTO_H */
