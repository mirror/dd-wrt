/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef ASN1_DECODER_H_
#define ASN1_DECODER_H_

/* Class */
#define ASN1_UNI	0	/* Universal */
#define ASN1_APL	1	/* Application */
#define ASN1_CTX	2	/* Context */
#define ASN1_PRV	3	/* Private */

/* Tag */
#define ASN1_EOC	0	/* End Of Contents or N/A */
#define ASN1_BOL	1	/* Boolean */
#define ASN1_INT	2	/* Integer */
#define ASN1_BTS	3	/* Bit String */
#define ASN1_OTS	4	/* Octet String */
#define ASN1_NUL	5	/* Null */
#define ASN1_OJI	6	/* Object Identifier  */
#define ASN1_OJD	7	/* Object Description */
#define ASN1_EXT	8	/* External */
#define ASN1_ENUM	10	/* Enumerated */
#define ASN1_SEQ	16	/* Sequence */
#define ASN1_SET	17	/* Set */
#define ASN1_NUMSTR	18	/* Numerical String */
#define ASN1_PRNSTR	19	/* Printable String */
#define ASN1_TEXSTR	20	/* Teletext String */
#define ASN1_VIDSTR	21	/* Video String */
#define ASN1_IA5STR	22	/* IA5 String */
#define ASN1_UNITIM	23	/* Universal Time */
#define ASN1_GENTIM	24	/* General Time */
#define ASN1_GRASTR	25	/* Graphical String */
#define ASN1_VISSTR	26	/* Visible String */
#define ASN1_GENSTR	27	/* General String */

/* Primitive / Constructed methods*/
#define ASN1_PRI	0	/* Primitive */
#define ASN1_CON	1	/* Constructed */

/*
 * Error codes.
 */
#define ASN1_ERR_NOERROR		0
#define ASN1_ERR_DEC_EMPTY		2
#define ASN1_ERR_DEC_EOC_MISMATCH	3
#define ASN1_ERR_DEC_LENGTH_MISMATCH	4
#define ASN1_ERR_DEC_BADVALUE		5

#define SPNEGO_OID_LEN 7
#define NTLMSSP_OID_LEN  10
#define KRB5_OID_LEN  7
#define KRB5U2U_OID_LEN  8
#define MSKRB5_OID_LEN  7
static unsigned long SPNEGO_OID[7] = { 1, 3, 6, 1, 5, 5, 2 };
static unsigned long NTLMSSP_OID[10] = { 1, 3, 6, 1, 4, 1, 311, 2, 2, 10 };
static unsigned long KRB5_OID[7] = { 1, 2, 840, 113554, 1, 2, 2 };
static unsigned long KRB5U2U_OID[8] = { 1, 2, 840, 113554, 1, 2, 2, 3 };
static unsigned long MSKRB5_OID[7] = { 1, 2, 840, 48018, 1, 2, 2 };

/*
 * ASN.1 context.
 */
struct asn1_ctx {
	int error;		/* Error condition */
	unsigned char *pointer;	/* Octet just to be decoded */
	unsigned char *begin;	/* First octet */
	unsigned char *end;	/* Octet after last octet */
};

/*
 * Octet string (not null terminated)
 */
struct asn1_octstr {
	unsigned char *data;
	unsigned int len;
};

void asn1_open(struct asn1_ctx *ctx, unsigned char *buf, unsigned int len);
unsigned char asn1_header_decode(struct asn1_ctx *ctx, unsigned char **eoc,
		   unsigned int *cls, unsigned int *con, unsigned int *tag);
unsigned char asn1_octets_decode(struct asn1_ctx *ctx, unsigned char *eoc,
		   unsigned char **octets, unsigned int *len);
unsigned char asn1_read(struct asn1_ctx *ctx,
		unsigned char **buf, unsigned int len);
int asn1_oid_decode(struct asn1_ctx *ctx, unsigned char *eoc,
		unsigned long **oid, unsigned int *len);

int asn1_header_len(unsigned int payload_len, int depth);
int asn1_header_encode(unsigned char **buf,
			unsigned int cls, unsigned int con, unsigned int tag,
			unsigned int *len);
int asn1_oid_encode(const unsigned long *in_oid, int in_len,
			unsigned char **out_oid, int *out_len);

#endif
