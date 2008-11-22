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

   $Id: supp.h 312 2008-06-15 18:09:42Z Joerg Mayer $
*/

#ifndef __SUPP_H__
#define __SUPP_H__

enum supp_algo_key {
	SUPP_ALGO_NAME,
	SUPP_ALGO_MY_ID,
	SUPP_ALGO_IKE_SA,
	SUPP_ALGO_IPSEC_SA
};

enum algo_group {
	SUPP_ALGO_DH_GROUP,
	SUPP_ALGO_HASH,
	SUPP_ALGO_CRYPT,
	SUPP_ALGO_AUTH
};

typedef struct {
	const char *name;
	int my_id, ike_sa_id, ipsec_sa_id;
	int keylen;
} supported_algo_t;

extern const supported_algo_t supp_dh_group[];
extern const supported_algo_t supp_hash[];
extern const supported_algo_t supp_crypt[];
extern const supported_algo_t supp_auth[];

extern const supported_algo_t *get_algo(enum algo_group what, enum supp_algo_key key, int id, const char *name, int keylen);
extern const supported_algo_t *get_dh_group_ike(void);
extern const supported_algo_t *get_dh_group_ipsec(int server_setting);

#endif
