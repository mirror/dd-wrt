/*
 *      BIRD -- OSPF
 *
 *      (c) 1999 - 2000 Ondrej Filip <feela@network.cz>
 *
 *      Can be freely distributed and used under the terms of the GNU GPL.
 *
 */

#ifndef _BIRD_OSPF_LSALIB_H_
#define _BIRD_OSPF_LSALIB_H_

#ifdef CPU_BIG_ENDIAN
static inline void htonlsah(struct ospf_lsa_header *h, struct ospf_lsa_header *n) { *n = *h; };
static inline void ntohlsah(struct ospf_lsa_header *n, struct ospf_lsa_header *h) { *h = *n; };
static inline void htonlsab(void *h, void *n, u16 len) { ASSERT(h != n); memcpy(n, h, len); };
static inline void ntohlsab(void *n, void *h, u16 len) { ASSERT(n != h); memcpy(h, n, len); };
static inline void htonlsab1(void *h, u16 len) { };
static inline void ntohlsab1(void *n, u16 len) { };
#else
void htonlsah(struct ospf_lsa_header *h, struct ospf_lsa_header *n);
void ntohlsah(struct ospf_lsa_header *n, struct ospf_lsa_header *h);
void htonlsab(void *h, void *n, u16 len);
void ntohlsab(void *n, void *h, u16 len);
static inline void htonlsab1(void *h, u16 len) { htonlsab(h, h, len); };
static inline void ntohlsab1(void *n, u16 len) { ntohlsab(n, n, len); };
#endif

void lsasum_calculate(struct ospf_lsa_header *header, void *body);
u16 lsasum_check(struct ospf_lsa_header *h, void *body);
#define CMP_NEWER 1
#define CMP_SAME 0
#define CMP_OLDER -1
int lsa_comp(struct ospf_lsa_header *l1, struct ospf_lsa_header *l2);
int lsa_validate(struct ospf_lsa_header *lsa, void *body);
struct top_hash_entry * lsa_install_new(struct proto_ospf *po, struct ospf_lsa_header *lsa, u32 domain, void *body);
void ospf_age(struct proto_ospf *po);
void flush_lsa(struct top_hash_entry *en, struct proto_ospf *po);
void ospf_flush_area(struct proto_ospf *po, u32 areaid);


#endif /* _BIRD_OSPF_LSALIB_H_ */
