/*
 *
 * Configuration DB
 *
 * $Id: ldb.h,v 1.2 2001/10/09 22:33:07 paulsch Exp $
 *
 */
#ifndef LDB_H
#define LDB_H

typedef struct {
  char elan_name[32];
  short elan_name_size;
  int no_addresses;
  char *addresses[256];
  unsigned char les_addr[20];
  char type; /* Unspecified, 802.3, 802.5 */
  char max_frame; /* 1516, 4544, 9234, 18190 */
} Elan_t;

/* Protos */
Elan_t *new_elan(const char *name);
int add_les(Elan_t *elan, const char *addr);
int add_atm(Elan_t *elan, char *addr);
void set_default(Elan_t *elan);

void set_lecs_addr(const char *addr);
const unsigned char *get_lecs_addr(void);

Elan_t *find_elan(unsigned char *lec_addr, const char type, 
		  const char max_frame, const char *elan_name, 
		  const short elan_name_size, unsigned short *reason);

void dump_db(Elan_t *elan);

void reset_db(void);
#endif /* LDB_H */
