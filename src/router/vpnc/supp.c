/* Algorithm support checks
   Copyright (C) 2005 Maurice Massar
   Reorganised 2006 by Dan Villiom Podlaski Christiansen

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   $Id: supp.c 312 2008-06-15 18:09:42Z Joerg Mayer $
*/

#include "supp.h"
#include "math_group.h"
#include "config.h"
#include "isakmp.h"

#include <gcrypt.h>
#include <stdlib.h>

const supported_algo_t supp_dh_group[] = {
	{"nopfs", 0, 0, 0, 0},
	{"dh1", OAKLEY_GRP_1, IKE_GROUP_MODP_768,  IKE_GROUP_MODP_768,  0},
	{"dh2", OAKLEY_GRP_2, IKE_GROUP_MODP_1024, IKE_GROUP_MODP_1024, 0},
	{"dh5", OAKLEY_GRP_5, IKE_GROUP_MODP_1536, IKE_GROUP_MODP_1536, 0},
	/*{ "dh7", OAKLEY_GRP_7, IKE_GROUP_EC2N_163K, IKE_GROUP_EC2N_163K, 0 } note: code missing */
	{NULL, 0, 0, 0, 0}
};

const supported_algo_t supp_hash[] = {
	{"md5", GCRY_MD_MD5, IKE_HASH_MD5, IPSEC_AUTH_HMAC_MD5, 0},
	{"sha1", GCRY_MD_SHA1, IKE_HASH_SHA, IPSEC_AUTH_HMAC_SHA, 0},
	{NULL, 0, 0, 0, 0}
};

const supported_algo_t supp_crypt[] = {
	{"null", GCRY_CIPHER_NONE, IKE_ENC_NO_CBC, ISAKMP_IPSEC_ESP_NULL, 0},
	{"des", GCRY_CIPHER_DES, IKE_ENC_DES_CBC, ISAKMP_IPSEC_ESP_DES, 0},
	{"3des", GCRY_CIPHER_3DES, IKE_ENC_3DES_CBC, ISAKMP_IPSEC_ESP_3DES, 0},
	{"aes128", GCRY_CIPHER_AES128, IKE_ENC_AES_CBC, ISAKMP_IPSEC_ESP_AES, 128},
	{"aes192", GCRY_CIPHER_AES192, IKE_ENC_AES_CBC, ISAKMP_IPSEC_ESP_AES, 192},
	{"aes256", GCRY_CIPHER_AES256, IKE_ENC_AES_CBC, ISAKMP_IPSEC_ESP_AES, 256},
	{NULL, 0, 0, 0, 0}
};

const supported_algo_t supp_auth[] = {
	{"psk", 0, IKE_AUTH_PRESHARED, 0, 0},
	{"psk+xauth", 0, IKE_AUTH_XAUTHInitPreShared, 0, 0},
#ifdef OPENSSL_GPL_VIOLATION
#if 0
	{"cert(dsa)", 0, IKE_AUTH_RSA_SIG, 0, 0},
	{"cert(rsasig)", 0, IKE_AUTH_DSS, 0, 0},
	{"hybrid(dsa)", 0, IKE_AUTH_DSS, 0, 0},
#endif /* 0 */
	{"hybrid(rsa)", 0, IKE_AUTH_HybridInitRSA, 0, 0},
#endif /* OPENSSL_GPL_VIOLATION */
	{NULL, 0, 0, 0, 0}
};

const supported_algo_t *get_algo(enum algo_group what, enum supp_algo_key key, int id,
	const char *name, int keylen)
{
	const supported_algo_t *sa = NULL;
	int i = 0, val = 0;
	const char *valname = NULL;

	switch (what) {
	case SUPP_ALGO_DH_GROUP:
		sa = supp_dh_group;
		break;
	case SUPP_ALGO_HASH:
		sa = supp_hash;
		break;
	case SUPP_ALGO_CRYPT:
		sa = supp_crypt;
		break;
	case SUPP_ALGO_AUTH:
		sa = supp_auth;
		break;
	default:
		abort();
	}

	for (i = 0; sa[i].name != NULL; i++) {
		switch (key) {
		case SUPP_ALGO_NAME:
			valname = sa[i].name;
			break;
		case SUPP_ALGO_MY_ID:
			val = sa[i].my_id;
			break;
		case SUPP_ALGO_IKE_SA:
			val = sa[i].ike_sa_id;
			break;
		case SUPP_ALGO_IPSEC_SA:
			val = sa[i].ipsec_sa_id;
			break;
		default:
			abort();
		}
		if ((key == SUPP_ALGO_NAME) ? !strcasecmp(name, valname) : (val == id))
			if (keylen == sa[i].keylen)
				return sa + i;
	}

	return NULL;
}

const supported_algo_t *get_dh_group_ike(void)
{
	return get_algo(SUPP_ALGO_DH_GROUP, SUPP_ALGO_NAME, 0, config[CONFIG_IKE_DH], 0);
}
const supported_algo_t *get_dh_group_ipsec(int server_setting)
{
	const char *pfs_setting = config[CONFIG_IPSEC_PFS];

	if (!strcmp(config[CONFIG_IPSEC_PFS], "server")) {
		/* treat server_setting == -1 (unknown) as 0 */
		pfs_setting = (server_setting == 1) ? "dh2" : "nopfs";
	}

	return get_algo(SUPP_ALGO_DH_GROUP, SUPP_ALGO_NAME, 0, pfs_setting, 0);
}
