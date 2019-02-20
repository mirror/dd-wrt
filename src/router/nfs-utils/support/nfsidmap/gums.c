/*
 *  gums.c
 *
 *  Copyright (c) 2004 The Regents of the University of Michigan.
 *  All rights reserved.
 *
 *  Olga Kornievskaia <aglo@umich.edu>
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the University nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <err.h>
#include <syslog.h>
#include "nfsidmap.h"
#include "nfsidmap_plugin.h"

#include <voms_apic.h>

#include <prima_logger.h>
#include <prima_soap_client.h>
#include <prima_saml_support.h>

#define DEFAULT_PRIMA_CONF_LOCATION "/etc/grid-security/prima-authz.conf"
#define DEFAULT_VOMSDIR "/etc/grid-security/vomsdir"
#define DEFAULT_CADIR "/etc/grid-security/certificates"
#define X509_DN_SIZE 1024

//#define DEBUG_PRINT_VOMS

#define USING_TEST_PROGRAM
#ifdef USING_TEST_PROGRAM
nfs4_idmap_log_function_t idmap_log_func = printf;
int idmap_verbosity = 10;
#endif

/*
 * GUMS Translation Methods
 *
 */

/* global variables. voms/gums configuration attributes*/
static char prima_conf[] = DEFAULT_PRIMA_CONF_LOCATION;
typedef struct _plugin_config_params {
	char *saml_schema_dir;
	int   saml_log_level;
	char *server_cert;
	char *server_key;
	char *ca_dir;
	char *gums_server_location;
	char *voms_dir;
} plugin_config_params;
plugin_config_params conf;

#ifdef VOMS_BUG
static void my_VOMS_Delete(struct voms *v)
{
	int i;

	if (!v) return;
	if (v->user)
		free(v->user);
	if (v->server)
		free(v->server);
	if (v->fqan) {
		for (i = 0; v->fqan[i] != NULL; i++)
			free(v->fqan[i]);
		free(v->fqan);
	}
	free(v);
}

static struct voms *my_VOMS_Copy(struct voms *v, int *err)
{
	struct voms *cv;
	int i;

	cv = calloc(1, sizeof(struct voms));
	if (cv == NULL)
		goto out;
	cv->user = strdup(v->user);
	if (cv->user == NULL)
		goto out;
	cv->server = strdup(v->server);
	if (cv->server == NULL)
		goto out;
	for (i = 0; v->fqan[i] != NULL; i++) {
		if (v->fqan[i] == NULL)
			break;
	}
	cv->fqan = calloc(i+1, sizeof(char *));
	if (cv->fqan == NULL)
		goto out;
	cv->fqan[i] = NULL;
	for (i = 0; v->fqan[i] != NULL; i++) {
		cv->fqan[i] = strdup(v->fqan[i]);
		if (cv->fqan[i] == NULL)
			goto out;
	}
	return cv;
out:
	if (cv)
		my_VOMS_Delete(cv);

	return NULL;
}
#endif


#ifdef DEBUG_PRINT_VOMS
void printvoms(struct voms *v)
{
  int j;

  printf("SIGLEN: %d\nUSER: %s\n", v->siglen, v->user);
  printf("UCA: %s\nSERVER: %s\n", v->userca, v->server);
  printf("SCA: %s\nVO: %s\n", v->serverca, v->voname);
  printf("URI: %s\nDATE1: %s\n", v->uri, v->date1);
  printf("DATE2: %s\n", v->date2);

  switch (v->type) {
  case TYPE_NODATA:
    printf("NO DATA\n");
    break;
  case TYPE_CUSTOM:
    printf("%*s\n", v->datalen - 10, v->custom);
    break;
  case TYPE_STD:
    j = 0;
    while (v->std[j]) {
      printf("GROUP: %s\nROLE: %s\nCAP: %s\n",v->std[j]->group,
             v->std[j]->role,v->std[j]->cap);
      j++;
    }
  }
}

void print(struct vomsdata *d)
{
  struct voms **vo = d->data;
  struct voms *v;
  int k = 0;

  while(vo[k]) {
    v = vo[k++];
    printf("%d *******************************************\n",k);
    printvoms(v);
  }

  if (d->workvo)
    printf("WORKVO: %s\n", d->workvo);

  if (d->extra_data)
    printf("EXTRA: %s\n", d->extra_data);
}
#endif

static void free_plugin_config_params()
{
	if (conf.saml_schema_dir)
		free(conf.saml_schema_dir);
	conf.saml_schema_dir = NULL;
	if (conf.server_cert)
		free(conf.server_cert);
	conf.server_cert = NULL;
	if (conf.server_key)
		free(conf.server_key);
	conf.server_key = NULL;
	if (conf.ca_dir)
		free(conf.ca_dir);
	conf.ca_dir = NULL;
	if (conf.voms_dir)
		free(conf.voms_dir);
	conf.voms_dir = NULL;
}

static int validate_plugin_config_params()
{
	if (conf.saml_schema_dir == NULL ||
		conf.server_cert == NULL ||
		conf.server_key == NULL ||
		conf.gums_server_location == NULL)
		return -1;

	if (conf.ca_dir == NULL) {
		conf.ca_dir = strdup(DEFAULT_CADIR);
		if (conf.ca_dir == NULL)
			return -1;
	}
	if (conf.voms_dir == NULL) {
		conf.voms_dir = strdup(DEFAULT_VOMSDIR);
		if (conf.voms_dir == NULL)
			return -1;
	}
	return 0;
}

static int gums_init(void)
{
	FILE *f = NULL;
	int ret = -1, i = 0;
	char buf[512], type[128], value[256];
	char *alt_conf = NULL;

	alt_conf = nfsidmap_config_get("GUMS", "Conf_File");
	if (alt_conf == NULL)
		f = fopen(prima_conf, "r");
	else
		f = fopen(alt_conf, "r");
	if (f == NULL)
		goto out;

	while (fgets(buf, 512, f)) {
		i = 0;
		while(buf[i] == ' ' || buf[i] == '\t')
			i++;
		if (buf[i] == '#' || buf[i] == '\0' || buf[i] == '\n')
			continue;
		if (sscanf(&buf[i], "%127s%255s",type,value) < 2) {
			IDMAP_LOG(0, ("ERROR: malformed line: %s\n", &buf[i]));
			goto out;
		}
		IDMAP_LOG(1, ("PRIMA conf: type=%s value=%s\n", type, value));
		if (strncmp(type, "imsContact", 10) == 0) {
			conf.gums_server_location = strdup(value);
		} else if (strncmp(type, "serviceCert", 11) == 0) {
			conf.server_cert = strdup(value);
		} else if (strncmp(type, "serviceKey", 10) == 0) {
			conf.server_key = strdup(value);
		} else if (strncmp(type, "caCertDir", 9) == 0) {
			conf.ca_dir = strdup(value);
		} else if (strncmp(type, "samlSchemaDir", 13) == 0) {
			conf.saml_schema_dir = strdup(value);
		} else if (strncmp(type, "logLevel", 8) == 0) {
			if (strncmp(value, "debug", 5) == 0)
				conf.saml_log_level = PRIMA_LOG_DEBUG;
			else if (strncmp(value, "error", 5) == 0)
				conf.saml_log_level = PRIMA_LOG_ERROR;
			else if (strncmp(value, "none", 4) == 0)
				conf.saml_log_level = PRIMA_LOG_NONE;
			else
				conf.saml_log_level = PRIMA_LOG_INFO;
		}
	}

	if (validate_plugin_config_params() != 0)
		goto out;

	ret = 0;
out:
	if (f)
		fclose(f);
	if (ret)
		free_plugin_config_params();

	return ret;
}

static int retrieve_attributes(X509 *cert, STACK_OF(X509) *cas,
			       struct voms **attrs)
{
	int ret = -1, err = 0;
	struct vomsdata *vd = NULL;

	vd = VOMS_Init(conf.voms_dir, conf.ca_dir);
	if (vd == NULL) {
		IDMAP_LOG(0, ("VOMS_Init failed\n"));
		return -1;
	}
	ret = VOMS_Retrieve(cert, cas, RECURSE_CHAIN, vd, &err);
	if (err) {
		char *err_msg;
		err_msg = VOMS_ErrorMessage(vd, err, NULL, 0);
		if (err == VERR_NOEXT)
			ret = 0;
		else
			IDMAP_LOG(0, ("VOMS error %s\n", err_msg));
		goto out;
	} else if (ret) {
		struct voms *v, *v2;
#ifdef DEBUG_PRINT_VOMS
		print(vd);
#endif
		v = VOMS_DefaultData(vd, &err);
		if (err == VERR_NONE) {
#ifdef DEBUG_PRINT_VOMS
			printvoms(v);
			while (v->fqan[i] != NULL)
				IDMAP_LOG(1, ("user's fqan: %s\n", v->fqan[i++]));
#endif
#ifdef VOMS_BUG
			v2 = my_VOMS_Copy(v, &err);
#else
			v2 = VOMS_Copy(v, &err);
#endif
			if (v2 == NULL) {
				IDMAP_LOG(0, ("VOMS_Copy failed err=%d\n", err));
				goto out;
			}
			*attrs = v2;
		}
	}
	ret = 0;
out:
	if (vd)
		VOMS_Destroy(vd);
	return ret;
}

static int get_server_dn(unsigned char **server_dn)
{
	BIO *tmp = NULL;
	X509 *cert = NULL;
	int ret = -1;
	char dn[X509_DN_SIZE];

	tmp = BIO_new(BIO_s_file());
	if (tmp == NULL)
		goto out;

	ret = BIO_read_filename(tmp, conf.server_cert);
	if (ret == 0) {
		ret = errno;
		goto out;
	}

	cert = (X509 *) PEM_read_bio_X509(tmp, NULL, NULL, NULL);
	if (cert == NULL)
		goto out;

	X509_NAME_oneline(X509_get_subject_name(cert), dn, sizeof(dn));

	*server_dn = strdup(dn);
	if (*server_dn == NULL)
		goto out;

	ret = 0;
out:
	if (tmp)
		BIO_free(tmp);
	if (cert)
		X509_free(cert);

	return ret;
}

static int create_saml_request(char *dn, struct voms *attrs, char **saml_req)
{
	int ret = -1, i;
	char *req = NULL;
	unsigned char *server_dn = NULL;
	prima_saml_fqans fqans;

	IDMAP_LOG(2, ("create_saml_request start\n"));
	ret = initPrimaSAMLFQANs(&fqans);
	if (ret) {
		IDMAP_LOG(0, ("initPrimaSAMLFQANs failed with %d\n", ret));
		goto out;
	}

	if (attrs) {
		for (i = 0; attrs->fqan[i] != NULL; i++) {
			ret = addPrimaSAMLFQAN(&fqans, attrs->server, attrs->fqan[i]);
			IDMAP_LOG(1, ("addPrimaSAMLFQAN returned %d\n", ret));
		}
		dn = attrs->user;
	} else
		IDMAP_LOG(1, ("No VOMS attributes present in the cert\n"));

	if (get_server_dn(&server_dn) != 0)
		goto out;
	req = createSAMLQueryAndRequest(server_dn, dn, &fqans);
	if (req == NULL) {
		IDMAP_LOG(0, ("createSAMLQueryAndRequest failed to create "
			"SAML request\n"));
		goto out;
	}
 	IDMAP_LOG(1, ("SAML Request %s\n", req));
 
	ret = 0;
	*saml_req = req;
out:
	cleanupPrimaSAMLFQANs(&fqans);

	if (server_dn)
		free(server_dn);

	IDMAP_LOG(2, ("create_saml_request returning %d\n", ret));
	return ret;
}

static int process_parameters(extra_mapping_params **ex, X509 **user_cert,
				STACK_OF(X509) **user_chain)
{

	int ret = -1, i;
	X509 *cert = NULL, *x;
	STACK_OF(X509) *chain = NULL;
	unsigned char *p;

	if (ex[0]->content_type != X509_CERT)
		return -1;

	/* get user's x509 certificate */
	p = ex[0]->content;
	cert = d2i_X509(NULL, &p, ex[0]->content_len);
	if (cert == NULL)
		goto out;

	/* get user's other certificates */
	chain = sk_X509_new_null();
	if (chain == NULL)
		goto out;
	for (i = 1; ex[i] != NULL; i++) {
		if (ex[i]->content_type != X509_CERT)
			continue;
		p = ex[i]->content;
		x = d2i_X509(NULL, &p, ex[i]->content_len);
		if (x == NULL)
			goto out;
		sk_X509_push(chain, x);
	}
	ret = 0;

	*user_cert = cert;
	*user_chain = chain;
out:
	if (ret) {
		int num;
		if (cert)
			X509_free(cert);
		if (chain)
			sk_X509_pop_free(chain, X509_free);
	}

	return ret;
}

struct pwbuf {
        struct passwd pwbuf;
        char buf[1];
};

static int translate_to_uid(char *local_uid, uid_t *uid, uid_t *gid)
{
	int ret = -1;
	struct passwd *pw = NULL;
	struct pwbuf *buf = NULL;
	size_t buflen = sysconf(_SC_GETPW_R_SIZE_MAX);

	buf = malloc(sizeof(*buf) + buflen);
	if (buf == NULL)
		goto out;

	ret = getpwnam_r(local_uid, &buf->pwbuf, buf->buf, buflen, &pw);
	if (pw == NULL) {
		IDMAP_LOG(0, ("getpwnam: name %s not found\n", local_uid));
		goto out;
	}
	*uid = pw->pw_uid;
	*gid = pw->pw_gid;

	ret = 0;
out:
	if (buf)
		free(buf);
	return ret;
}

static int translate_to_gid(char *local_gid, uid_t *gid)
{
	struct group *gr = NULL;
	struct group grbuf;
	char *buf = NULL;
	size_t buflen = sysconf(_SC_GETGR_R_SIZE_MAX);
	int ret = -1;

	do {
		buf = malloc(buflen);
		if (buf == NULL)
			goto out;

		ret = -getgrnam_r(local_gid, &grbuf, buf, buflen, &gr);
		if (gr == NULL && !ret)
			ret = -ENOENT;
		if (ret == -ERANGE) {
			buflen *= 2;
			free(buf);
		}
	} while (ret == -ERANGE);

	if (ret)
		goto out;

	*gid = gr->gr_gid;

	ret = 0;
out:
	if (buf)
		free(buf);
	return ret;
}

static int gums_gss_princ_to_ids(char *secname, char *princ,
				uid_t *uid, uid_t *gid,
				extra_mapping_params **ex)
{
	int ret = -1, size, i;
	X509 *cert = NULL;
	STACK_OF(X509) *cas = NULL;
	char dn[X509_DN_SIZE];
	struct voms *attrs = NULL;
	char *saml_req = NULL, *saml_resp = NULL;
	int saml_result;
	char *local_uid = NULL, *local_gid = NULL, *p;

	/* accept only spkm3 translations */
	if (strcmp(secname, "spkm3"))
		return -EINVAL;

	/* must supply either a DN and/or at least 1 binary blob */
	if (princ == NULL && (ex == NULL || (ex && ex[0] == NULL)))
		return -EINVAL;

	/* process extra parameters */
	if (process_parameters(ex, &cert, &cas) != 0)
		goto out;

	IDMAP_LOG(1, ("Processing name translation of client\n"));
	X509_NAME_oneline(X509_get_subject_name(cert), dn, sizeof(dn));
	IDMAP_LOG(1, ("DN=%s\n", dn));
	size = sk_X509_num(cas);
	IDMAP_LOG(1, ("Including following CAs (%d)\n", size));
	for (i=0; i < size; i++) {
		X509_NAME_oneline(X509_get_subject_name(sk_X509_value(cas, i)),
 					dn, sizeof(dn));
		IDMAP_LOG(1, ("DN=%s\n", dn));
	}

	/* retrieve VOMS attributes */
	if (retrieve_attributes(cert, cas, &attrs) != 0)
		goto out;
	if (attrs == NULL)
		X509_NAME_oneline(X509_get_subject_name(cert), dn, sizeof(dn));

	/* initialize SAML library */
	if (initPrimaSAMLSupport(conf.saml_schema_dir,
			conf.saml_log_level) != 0) {
		IDMAP_LOG(0, ("initPrimaSAMLSupport failed\n"));
		goto out;
	}

	/* create SAML request */
	if (create_saml_request(dn, attrs, &saml_req) != 0)
		goto out;

	/* contact GUMS server */
	saml_resp = queryIdentityMappingService(conf.gums_server_location,
			saml_req, conf.server_cert, conf.server_key,
			conf.ca_dir);
	if (saml_resp != NULL) {
		saml_result = processResponse(saml_resp, saml_req, &local_uid,
						&local_gid);
		IDMAP_LOG(1, ("processResponse returned %d\n", saml_result));
		if (saml_result || local_uid == NULL) {
			IDMAP_LOG(0, ("processResponse failed to return "
					"local id\n"));
			ret = -ENOENT;
			goto out;
		}
		IDMAP_LOG(1, ("GUMS returned uid=%s gid=%s\n", local_uid,
			local_gid));
	}

	/* translate account name to uid */
	if (translate_to_uid(local_uid, uid, gid))
		goto out;
	if (local_gid)
		if (translate_to_gid(local_gid, gid))
			goto out;

	ret = 0;
out:
	if (cert)
		X509_free(cert);

	if (cas)
		sk_X509_pop_free(cas, X509_free);

	if (attrs)
#ifdef VOMS_BUG
		my_VOMS_Delete(attrs);
#else
		VOMS_Delete(attrs);
#endif

	if (saml_req)
		free(saml_req);

	if (saml_resp)
		free(saml_resp);

	cleanupPrimaSAMLSupport();

	return ret;
}

struct trans_func gums_trans = {
	.name		= "gums",
	.init		= gums_init,
	.princ_to_ids	= gums_gss_princ_to_ids,
	.name_to_uid	= NULL,
	.name_to_gid	= NULL,
	.uid_to_name	= NULL,
	.gid_to_name	= NULL,
	.gss_princ_to_grouplist = NULL,
};

struct trans_func *libnfsidmap_plugin_init()
{
	return (&gums_trans);
}

#ifdef USING_TEST_PROGRAM
static STACK_OF(X509) *load_chain(char *certfile)
{
  STACK_OF(X509_INFO) *sk=NULL;
  STACK_OF(X509) *stack=NULL, *ret=NULL;
  BIO *in=NULL;
  X509_INFO *xi;
  int first = 1;

  if (!(stack = sk_X509_new_null())) {
    printf("memory allocation failure\n");
    goto end;
  }

  if (!(in=BIO_new_file(certfile, "r"))) {
    printf("error opening the file, %s\n",certfile);
    goto end;
  }

  /* This loads from a file, a stack of x509/crl/pkey sets */
  if (!(sk=(STACK_OF(X509_INFO) *)PEM_X509_INFO_read_bio(in,NULL,NULL,NULL))) {
    /*     if (!(sk=PEM_X509_read_bio(in,NULL,NULL,NULL))) { */
    printf("error reading the file, %s\n",certfile);
    goto end;
  }

  /* scan over it and pull out the certs */
  while (sk_X509_INFO_num(sk)) {
    /* skip first cert */
    if (first) {
      xi=sk_X509_INFO_shift(sk);
      X509_INFO_free(xi);
      first = 0;
      continue;
    }
    xi=sk_X509_INFO_shift(sk);
    if (xi->x509 != NULL) {
      sk_X509_push(stack,xi->x509);
      xi->x509=NULL;
    }
    X509_INFO_free(xi);
  }
  if (!sk_X509_num(stack)) {
    printf("no certificates in file, %s\n",certfile);
    sk_X509_free(stack);
    goto end;
  }
  ret=stack;
end:
  BIO_free(in);
  sk_X509_INFO_free(sk);
  return(ret);
}

void create_params(X509 *cert, STACK_OF(X509) *cas,
		   extra_mapping_params ***ret_params)
{
	int len = 0, i, size = 0;
	unsigned char *p, *buf = NULL;
	extra_mapping_params **params = NULL;
	X509 *x;

	if (cas)
		size = sk_X509_num(cas);
	params = malloc((size+2)*sizeof(extra_mapping_params *));
	params[size+1] = NULL;

	/* 1st element is user's certificate */
	len = i2d_X509(cert, NULL);
	p = buf = malloc(len);
	i2d_X509(cert, &p);
	params[0] = malloc(sizeof(extra_mapping_params));
	params[0]->content_type = X509_CERT;
	params[0]->content = buf;
	params[0]->content_len = len;

	/* add other certificates to the array */
	for (i = 0; i < size; i++) {
		x = sk_X509_value(cas, i);
		params[i+1] = malloc(sizeof(extra_mapping_params));
		len = i2d_X509(x, NULL);
		p = buf = malloc(len);
		i2d_X509(x, &p);
		params[i+1]->content_type = X509_CERT;
		params[i+1]->content = buf;
		params[i+1]->content_len = len;
	}
	*ret_params = params;
}

int main(void)
{
	int uid, gid, ret, i;
	extra_mapping_params **params = NULL;
	BIO *tmp = NULL;
	X509 *cert = NULL, *x;
	STACK_OF(X509) *cas = NULL;
	unsigned char *proxy_file;

	if (gums_init())
		return -1;
	proxy_file = getenv("X509_USER_PROXY");
	if (proxy_file == NULL) {
		fprintf(stderr, "X509_USER_PROXY is not set\n");
		return -1;
	}
	tmp = BIO_new(BIO_s_file());
	BIO_read_filename(tmp, proxy_file);
	cert = (X509 *) PEM_read_bio_X509(tmp, NULL, NULL, NULL);
	cas = load_chain(proxy_file);
	create_params(cert, cas, &params);
	ret = gums_gss_princ_to_ids("spkm3", NULL, &uid, &gid, params);
	fprintf(stderr, "gums_gss_princ_to_ids returns %d uid=%d gid=%d\n",
		ret, uid, gid);

	if (tmp)
		BIO_free(tmp);
	if (cert)
		X509_free(cert);
	if (cas)
		sk_X509_pop_free(cas, X509_free);

	free_plugin_config_params();

	if (params) {
		for (i=0; params[i] != NULL; i++) {
			free(params[i]->content);
			free(params[i]);
		}
		free(params);
	}

	return 0;
}
#endif
