/*
 * X.509v3 certificate parsing and processing
 * Copyright (c) 2006, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef X509V3_H
#define X509V3_H

#include "asn1.h"

struct x509_algorithm_identifier {
	struct asn1_oid oid;
};

struct x509_name {
	char *cn; /* commonName */
	char *c; /* countryName */
	char *l; /* localityName */
	char *st; /* stateOrProvinceName */
	char *o; /* organizationName */
	char *ou; /* organizationalUnitName */
	char *email; /* emailAddress */
};

struct x509_certificate {
	enum { X509_CERT_V1 = 0, X509_CERT_V2 = 1, X509_CERT_V3 = 2 } version;
	unsigned long serial_number;
	struct x509_algorithm_identifier signature;
	struct x509_name issuer;
	struct x509_name subject;
	struct x509_algorithm_identifier public_key_alg;
	u8 *public_key;
	size_t public_key_len;
	struct x509_algorithm_identifier signature_alg;
	u8 *sign_value;
	size_t sign_value_len;
};

#ifdef CONFIG_INTERNAL_X509

void x509_certificate_free(struct x509_certificate *cert);
int x509_certificate_parse(const u8 *buf, size_t len,
			   struct x509_certificate *cert);
void x509_name_string(struct x509_name *name, char *buf, size_t len);

#else /* CONFIG_INTERNAL_X509 */

static inline void x509_certificate_free(struct x509_certificate *cert)
{
}

static inline int x509_certificate_parse(const u8 *buf, size_t len,
					 struct x509_certificate *cert)
{
	return -1;
}

static inline void x509_name_string(struct x509_name *name, char *buf,
				    size_t len)
{
	if (len)
		buf[0] = '\0';
}

#endif /* CONFIG_INTERNAL_X509 */


#endif /* X509V3_H */
