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

#include "includes.h"

#include "common.h"

#ifdef CONFIG_INTERNAL_X509

#include "asn1.h"
#include "x509v3.h"


static void x509_free_name(struct x509_name *name)
{
	free(name->cn);
	free(name->c);
	free(name->l);
	free(name->st);
	free(name->o);
	free(name->ou);
	free(name->email);
}


void x509_certificate_free(struct x509_certificate *cert)
{
	x509_free_name(&cert->issuer);
	x509_free_name(&cert->subject);
	free(cert->public_key);
	free(cert->sign_value);
}


static int x509_parse_algorithm_identifier(
	const u8 *buf, size_t len,
	struct x509_algorithm_identifier *id, const u8 **next)
{
	struct asn1_hdr hdr;
	const u8 *pos, *end;

	/*
	 * AlgorithmIdentifier ::= SEQUENCE {
	 *     algorithm            OBJECT IDENTIFIER,
	 *     parameters           ANY DEFINED BY algorithm OPTIONAL
	 * }
	 */

	if (asn1_get_next(buf, len, &hdr) < 0 ||
	    hdr.tag != ASN1_TAG_SEQUENCE) {
		wpa_printf(MSG_DEBUG, "X509: Expected SEQUENCE "
			   "(AlgorithmIdentifier) - found tag 0x%x", hdr.tag);
		return -1;
	}
	pos = hdr.payload;
	end = pos + hdr.length;

	if (end > buf + len)
		return -1;

	*next = end;

	if (asn1_get_oid(pos, end - pos, &id->oid, &pos))
		return -1;

	/* TODO: optional parameters */

	return 0;
}


static int x509_parse_public_key(const u8 *buf, size_t len,
				 struct x509_certificate *cert,
				 const u8 **next)
{
	struct asn1_hdr hdr;
	const u8 *pos, *end;

	/*
	 * SubjectPublicKeyInfo ::= SEQUENCE {
	 *     algorithm            AlgorithmIdentifier,
	 *     subjectPublicKey     BIT STRING
	 * }
	 */

	pos = buf;
	end = buf + len;

	if (asn1_get_next(pos, end - pos, &hdr) < 0 ||
	    hdr.tag != ASN1_TAG_SEQUENCE) {
		wpa_printf(MSG_DEBUG, "X509: Expected SEQUENCE "
			   "(SubjectPublicKeyInfo) - found tag 0x%x", hdr.tag);
		return -1;
	}
	pos = hdr.payload;

	if (pos + hdr.length > end)
		return -1;
	end = pos + hdr.length;
	*next = end;

	if (x509_parse_algorithm_identifier(pos, end - pos,
					    &cert->public_key_alg, &pos))
		return -1;

	if (asn1_get_next(pos, end - pos, &hdr) < 0 ||
	    hdr.tag != ASN1_TAG_BITSTRING) {
		wpa_printf(MSG_DEBUG, "X509: Expected BITSTRING "
			   "(subjectPublicKey) - found tag 0x%x", hdr.tag);
		return -1;
	}
	if (hdr.length < 1)
		return -1;
	pos = hdr.payload;
	if (*pos) {
		wpa_printf(MSG_DEBUG, "X509: BITSTRING - %d unused bits",
			   *pos);
		/*
		 * TODO: should this be rejected? X.509 certificates are
		 * unlikely to use such a construction. Now we would end up
		 * including the extra bits in the buffer which may also be
		 * ok.
		 */
	}
	free(cert->public_key);
	cert->public_key = malloc(hdr.length - 1);
	if (cert->public_key == NULL) {
		wpa_printf(MSG_DEBUG, "X509: Failed to allocate memory for "
			   "public key");
		return -1;
	}
	memcpy(cert->public_key, pos + 1, hdr.length - 1);
	cert->public_key_len = hdr.length - 1;
	wpa_hexdump(MSG_MSGDUMP, "X509: subjectPublicKey",
		    cert->public_key, cert->public_key_len);

	return 0;
}


static int x509_parse_name(const u8 *buf, size_t len, struct x509_name *name,
			   const u8 **next)
{
	struct asn1_hdr hdr;
	const u8 *pos, *end, *set_pos, *set_end, *seq_pos, *seq_end;
	struct asn1_oid oid;
	char **fieldp;

	/*
	 * Name ::= CHOICE { RDNSequence }
	 * RDNSequence ::= SEQUENCE OF RelativeDistinguishedName
	 * RelativeDistinguishedName ::= SET OF AttributeTypeAndValue
	 * AttributeTypeAndValue ::= SEQUENCE {
	 *     type     AttributeType,
	 *     value    AttributeValue
	 * }
	 * AttributeType ::= OBJECT IDENTIFIER
	 * AttributeValue ::= ANY DEFINED BY AttributeType
	 */

	if (asn1_get_next(buf, len, &hdr) < 0 ||
	    hdr.tag != ASN1_TAG_SEQUENCE) {
		wpa_printf(MSG_DEBUG, "X509: Expected SEQUENCE "
			   "(Name / RDNSequencer) - found tag 0x%x", hdr.tag);
		return -1;
	}
	pos = hdr.payload;

	if (pos + hdr.length > buf + len)
		return -1;

	end = *next = pos + hdr.length;

	while (pos < end) {
		if (asn1_get_next(pos, end - pos, &hdr) < 0 ||
		    hdr.tag != ASN1_TAG_SET) {
			wpa_printf(MSG_DEBUG, "X509: Expected SET "
				   "(RelativeDistinguishedName) - found tag "
				   "0x%x", hdr.tag);
			x509_free_name(name);
			return -1;
		}

		set_pos = hdr.payload;
		pos = set_end = hdr.payload + hdr.length;

		if (asn1_get_next(set_pos, set_end - set_pos, &hdr) < 0 ||
		    hdr.tag != ASN1_TAG_SEQUENCE) {
			wpa_printf(MSG_DEBUG, "X509: Expected SEQUENCE "
				   "(AttributeTypeAndValue) - found tag 0x%x",
				   hdr.tag);
			x509_free_name(name);
			return -1;
		}

		seq_pos = hdr.payload;
		seq_end = hdr.payload + hdr.length;

		if (asn1_get_oid(seq_pos, seq_end - seq_pos, &oid, &seq_pos)) {
			x509_free_name(name);
			return -1;
		}

		if (asn1_get_next(seq_pos, seq_end - seq_pos, &hdr) < 0) {
			wpa_printf(MSG_DEBUG, "X509: Failed to parse "
				   "AttributeValue");
			x509_free_name(name);
			return -1;
		}

		/* RFC 3280:
		 * MUST: country, organization, organizational-unit,
		 * distinguished name qualifier, state or province name,
		 * common name, serial number.
		 * SHOULD: locality, title, surname, given name, initials,
		 * pseudonym, generation qualifier.
		 * MUST: domainComponent (RFC 2247).
		 */
		fieldp = NULL;
		if (oid.len == 4 &&
		    oid.oid[0] == 2 && oid.oid[1] == 5 && oid.oid[2] == 4) {
			/* id-at ::= 2.5.4 */
			switch (oid.oid[3]) {
			case 3:
				/* commonName */
				fieldp = &name->cn;
				break;
			case 6:
				/*  countryName */
				fieldp = &name->c;
				break;
			case 7:
				/* localityName */
				fieldp = &name->l;
				break;
			case 8:
				/* stateOrProvinceName */
				fieldp = &name->st;
				break;
			case 10:
				/* organizationName */
				fieldp = &name->o;
				break;
			case 11:
				/* organizationalUnitName */
				fieldp = &name->ou;
				break;
			}
		} else if (oid.len == 7 &&
			   oid.oid[0] == 1 && oid.oid[1] == 2 &&
			   oid.oid[2] == 840 && oid.oid[3] == 113549 &&
			   oid.oid[4] == 1 && oid.oid[5] == 9 &&
			   oid.oid[6] == 1) {
			/* 1.2.840.113549.1.9.1 - e-mailAddress */
			fieldp = &name->email;
		}

		if (fieldp == NULL) {
			wpa_hexdump(MSG_DEBUG, "X509: Unrecognized OID",
				    (u8 *) oid.oid,
				    oid.len * sizeof(oid.oid[0]));
			wpa_hexdump_ascii(MSG_MSGDUMP, "X509: Attribute Data",
					  hdr.payload, hdr.length);
			continue;
		}

		free(*fieldp);
		*fieldp = malloc(hdr.length + 1);
		if (*fieldp == NULL) {
			x509_free_name(name);
			return -1;
		}
		memcpy(*fieldp, hdr.payload, hdr.length);
		(*fieldp)[hdr.length] = '\0';
	}

	return 0;
}


void x509_name_string(struct x509_name *name, char *buf, size_t len)
{
	char *pos, *end;

	pos = buf;
	end = buf + len;

	if (name->c)
		pos += snprintf(pos, end - pos, "C=%s, ", name->c);
	if (name->st)
		pos += snprintf(pos, end - pos, "ST=%s, ", name->st);
	if (name->l)
		pos += snprintf(pos, end - pos, "L=%s, ", name->l);
	if (name->o)
		pos += snprintf(pos, end - pos, "O=%s, ", name->o);
	if (name->ou)
		pos += snprintf(pos, end - pos, "OU=%s, ", name->ou);
	if (name->cn)
		pos += snprintf(pos, end - pos, "CN=%s, ", name->cn);

	if (pos > buf + 1 && pos[-1] == ' ' && pos[-2] == ',') {
		*pos-- = '\0';
		*pos-- = '\0';
	}

	if (name->email) {
		pos += snprintf(pos, end - pos, "/emailAddress=%s",
				name->email);
	}
}


static int x509_parse_validity(const u8 *buf, size_t len,
			       struct x509_certificate *cert, const u8 **next)
{
	struct asn1_hdr hdr;
	const u8 *pos;

	/*
	 * Validity ::= SEQUENCE {
	 *     notBefore      Time,
	 *     notAfter       Time
	 * }
	 */

	if (asn1_get_next(buf, len, &hdr) < 0 ||
	    hdr.tag != ASN1_TAG_SEQUENCE) {
		wpa_printf(MSG_DEBUG, "X509: Expected SEQUENCE "
			   "(Validity) - found tag 0x%x", hdr.tag);
		return -1;
	}
	pos = hdr.payload;

	if (pos + hdr.length > buf + len)
		return -1;

	*next = pos + hdr.length;

	/* TODO */

	return 0;
}


static int x509_parse_tbs_certificate(const u8 *buf, size_t len,
				      struct x509_certificate *cert,
				      const u8 **next)
{
	struct asn1_hdr hdr;
	const u8 *pos, *end;
	size_t left;
	char sbuf[128];

	pos = buf;
	end = buf + len;

	/* tbsCertificate TBSCertificate ::= SEQUENCE */
	if (asn1_get_next(pos, end - pos, &hdr) < 0 ||
	    hdr.tag != ASN1_TAG_SEQUENCE) {
		wpa_printf(MSG_DEBUG, "X509: tbsCertificate did not start "
			   "with a valid SEQUENCE");
		return -1;
	}
	pos = hdr.payload;

	if (pos + hdr.length > end)
		return -1;

	*next = pos + hdr.length;

	/*
	 * version [0]  EXPLICIT Version DEFAULT v1
	 * Version  ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
	 */
	if (asn1_get_next(pos, end - pos, &hdr) < 0)
		return -1;
	pos = hdr.payload;
	if (hdr.class == ASN1_CLASS_CONTEXT_SPECIFIC) {
		if (asn1_get_next(pos, end - pos, &hdr) < 0)
			return -1;
	}
	if (hdr.tag != ASN1_TAG_INTEGER) {
		wpa_printf(MSG_DEBUG, "X509: No INTEGER tag found for "
			   "version field");
		return -1;
	}
	if (hdr.length != 1) {
		wpa_printf(MSG_DEBUG, "X509: Unexpected version field length "
			   "%u (expected 1)", hdr.length);
		return -1;
	}
	pos = hdr.payload;
	cert->version = *pos;
	wpa_printf(MSG_MSGDUMP, "X509: Version X.509v%d", cert->version + 1);
	pos += hdr.length;

	/* serialNumber CertificateSerialNumber ::= INTEGER */
	if (asn1_get_next(pos, end - pos, &hdr) < 0 ||
	    hdr.tag != ASN1_TAG_INTEGER) {
		wpa_printf(MSG_DEBUG, "X509: No INTEGER tag gound for "
			   "serialNumber");
		return -1;
	}
	pos = hdr.payload;
	left = hdr.length;
	while (left) {
		cert->serial_number <<= 8;
		cert->serial_number |= *pos++;
		left--;
	}
	wpa_printf(MSG_MSGDUMP, "X509: serialNumber %lu",
		   cert->serial_number);

	/* signature AlgorithmIdentifier */
	if (x509_parse_algorithm_identifier(pos, end - pos, &cert->signature,
					    &pos))
		return -1;

	/* issuer Name */
	if (x509_parse_name(pos, end - pos, &cert->issuer, &pos))
		return -1;
	x509_name_string(&cert->issuer, sbuf, sizeof(sbuf));
	wpa_printf(MSG_MSGDUMP, "X509: issuer %s", sbuf);

	/* validity Validity */
	if (x509_parse_validity(pos, end - pos, cert, &pos))
		return -1;

	/* subject Name */
	if (x509_parse_name(pos, end - pos, &cert->subject, &pos))
		return -1;
	x509_name_string(&cert->subject, sbuf, sizeof(sbuf));
	wpa_printf(MSG_MSGDUMP, "X509: subject %s", sbuf);

	/* subjectPublicKeyInfo SubjectPublicKeyInfo */
	if (x509_parse_public_key(pos, end - pos, cert, &pos))
		return -1;

	if (pos == end)
		return 0;

	if (asn1_get_next(pos, end - pos, &hdr) < 0)
		return -1;
	pos = hdr.payload;

	if (hdr.class != ASN1_CLASS_CONTEXT_SPECIFIC) {
		wpa_printf(MSG_DEBUG, "X509: Expected Context-Specific tag "
			   "to parse optional tbsCertificate field(s); "
			   "parsed tag 0x%x (identifier 0x%02x)",
			   hdr.tag, hdr.identifier);
		return -1;
	}

	/* FIX: Need to loop through all values */

	switch (hdr.tag) {
	case 1:
		/* issuerUniqueID  [1]  IMPLICIT UniqueIdentifier OPTIONAL */
		/* TODO */
		break;
	case 2:
		/* subjectUniqueID [2]  IMPLICIT UniqueIdentifier OPTIONAL */
		/* TODO */
		break;
	case 3:
		/* extensions      [3]  EXPLICIT Extensions OPTIONAL */
		/* TODO */
		break;
	default:
		wpa_printf(MSG_DEBUG, "X509: Unexpected Context-Specific tag "
			   "(%d) in optional tbsCertificate fields", hdr.tag);
		return -1;
	}

	return 0;
}


int x509_certificate_parse(const u8 *buf, size_t len,
			   struct x509_certificate *cert)
{
	struct asn1_hdr hdr;
	const u8 *pos, *end;

	memset(cert, 0, sizeof(*cert));
	pos = buf;
	end = buf + len;

	/* RFC 3280 - X.509 v3 certificate / ASN.1 DER */

	/* Certificate ::= SEQUENCE */
	if (asn1_get_next(pos, len, &hdr) < 0 ||
	    hdr.tag != ASN1_TAG_SEQUENCE) {
		wpa_printf(MSG_DEBUG, "X509: Certificate did not start with "
			   "a valid SEQUENCE");
		return -1;
	}
	pos = hdr.payload;

	if (pos + hdr.length > end)
		return -1;

	if (pos + hdr.length < end) {
		wpa_hexdump(MSG_MSGDUMP, "X509: Ignoring extra data after DER "
			    "encoded certificate",
			    pos + hdr.length, end - pos + hdr.length);
		end = pos + hdr.length;
	}

	if (x509_parse_tbs_certificate(pos, end - pos, cert, &pos))
		return -1;

	/* signatureAlgorithm AlgorithmIdnetifier */
	if (x509_parse_algorithm_identifier(pos, end - pos,
					    &cert->signature_alg, &pos))
		return -1;

	/* signatureValue BIT STRING */
	if (asn1_get_next(pos, end - pos, &hdr) < 0 ||
	    hdr.tag != ASN1_TAG_BITSTRING) {
		wpa_printf(MSG_DEBUG, "X509: Expected BITSTRING "
			   "(signatureValue) - found tag 0x%x", hdr.tag);
		return -1;
	}
	if (hdr.length < 1)
		return -1;
	pos = hdr.payload;
	if (*pos) {
		wpa_printf(MSG_DEBUG, "X509: BITSTRING - %d unused bits",
			   *pos);
		/*
		 * TODO: should this be rejected? X.509 certificates are
		 * unlikely to use such a construction. Now we would end up
		 * including the extra bits in the buffer which may also be
		 * ok.
		 */
	}
	free(cert->sign_value);
	cert->sign_value = malloc(hdr.length - 1);
	if (cert->sign_value == NULL) {
		wpa_printf(MSG_DEBUG, "X509: Failed to allocate memory for "
			   "signatureValue");
		return -1;
	}
	memcpy(cert->sign_value, pos + 1, hdr.length - 1);
	cert->sign_value_len = hdr.length - 1;
	wpa_hexdump(MSG_MSGDUMP, "X509: signature",
		    cert->sign_value, cert->sign_value_len);

	return 0;
}

#endif /* CONFIG_INTERNAL_X509 */
