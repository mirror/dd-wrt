/*
 * Based on certificate dump functionality from ucert.c:
 *
 *  Copyright (C) 2018 Daniel Golle <daniel@makrotopia.org>
 *  SPDX-License-Identifier: GPL-3.0
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <libgen.h>

#include "blob.h"
#include "list.h"
#include "blobmsg_json.h"

#define CERT_BUF_LEN 4096

/*
 * ucert structure
 * |               BLOB                    |
 * |    SIGNATURE    |       PAYLOAD       |
 * |                 |[ BLOBMSG CONTAINER ]|
 * |                 |[[T,i,v,e,f,pubkey ]]|
 */
enum cert_attr {
	CERT_ATTR_SIGNATURE,
	CERT_ATTR_PAYLOAD,
	CERT_ATTR_MAX
};

static const struct blob_attr_info cert_policy[CERT_ATTR_MAX] = {
	[CERT_ATTR_SIGNATURE] = { .type = BLOB_ATTR_BINARY },
	[CERT_ATTR_PAYLOAD] = { .type = BLOB_ATTR_NESTED },
};

enum cert_cont_attr {
	CERT_CT_ATTR_PAYLOAD,
	CERT_CT_ATTR_MAX
};

enum cert_payload_attr {
	CERT_PL_ATTR_CERTTYPE,
	CERT_PL_ATTR_CERTID,
	CERT_PL_ATTR_VALIDFROMTIME,
	CERT_PL_ATTR_EXPIRETIME,
	CERT_PL_ATTR_PUBKEY,
	CERT_PL_ATTR_KEY_FINGERPRINT,
	CERT_PL_ATTR_MAX
};

enum certtype_id {
	CERTTYPE_UNSPEC,
	CERTTYPE_AUTH,
	CERTTYPE_REVOKE
};

/* list to store certificate chain at runtime */
struct cert_object {
	struct list_head list;
	struct blob_attr *cert[CERT_ATTR_MAX];
};

static int cert_load(const char *certfile, struct list_head *chain)
{
	FILE *f;
	struct blob_attr *certtb[CERT_ATTR_MAX];
	struct blob_attr *bufpt;
	struct cert_object *cobj;
	char *filebuf = NULL;
	int ret = 0, pret = 0;
	size_t len, pos = 0;

	f = fopen(certfile, "r");
	if (!f)
		return 1;

	filebuf = malloc(CERT_BUF_LEN+1);
	if (!filebuf)
		return 1;

	len = fread(filebuf, 1, CERT_BUF_LEN, f);
	if (len < 64) {
		free(filebuf);
		return 1;
	}

	ret = ferror(f) || !feof(f);
	fclose(f);
	if (ret) {
		free(filebuf);
		return 1;
	}

	bufpt = (struct blob_attr *)filebuf;
	do {
		pret = blob_parse_untrusted(bufpt, len, certtb, cert_policy, CERT_ATTR_MAX);
		if (pret <= 0)
			/* no attributes found */
			break;

		if (pos + blob_pad_len(bufpt) > len)
			/* blob exceeds filebuffer */
			break;
		else
			pos += blob_pad_len(bufpt);

		if (!certtb[CERT_ATTR_SIGNATURE])
			/* no signature -> drop */
			break;

		cobj = calloc(1, sizeof(*cobj));
		cobj->cert[CERT_ATTR_SIGNATURE] = blob_memdup(certtb[CERT_ATTR_SIGNATURE]);
		if (certtb[CERT_ATTR_PAYLOAD])
			cobj->cert[CERT_ATTR_PAYLOAD] = blob_memdup(certtb[CERT_ATTR_PAYLOAD]);

		list_add_tail(&cobj->list, chain);
		ret += pret;
	/* repeat parsing while there is still enough remaining data in buffer */
	} while(len > pos + sizeof(struct blob_attr) && (bufpt = blob_next(bufpt)));

	free(filebuf);
	return (ret <= 0);
}

/* dump single chain element to console */
static void cert_dump_blob(struct blob_attr *cert[CERT_ATTR_MAX])
{
	int i;
	char *json = NULL;

	for (i = 0; i < CERT_ATTR_MAX; i++) {
		struct blob_attr *v = cert[i];

		if (!v)
			continue;

		switch(cert_policy[i].type) {
		case BLOB_ATTR_BINARY:
			fprintf(stdout, "signature:\n---\n%s---\n", (char *) blob_data(v));
			break;
		case BLOB_ATTR_NESTED:
			json = blobmsg_format_json_indent(blob_data(v), false, 0);
			if (!json)
				continue;

			fprintf(stdout, "payload:\n---\n%s\n---\n", json);
			free(json);
			break;
		}
	}
}

static int cert_dump(const char *certfile)
{
	struct cert_object *cobj;
	static LIST_HEAD(certchain);
	unsigned int count = 0;

	if (cert_load(certfile, &certchain)) {
		fprintf(stderr, "cannot parse cert %s\n", basename((char *) certfile));
		return 1;
	}

	list_for_each_entry(cobj, &certchain, list) {
		fprintf(stdout, "=== CHAIN ELEMENT %02u ===\n", ++count);
		cert_dump_blob(cobj->cert);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <cert.ucert>\n", argv[0]);
		return 3;
	}

	cert_dump(argv[1]);

	return 0;
}
