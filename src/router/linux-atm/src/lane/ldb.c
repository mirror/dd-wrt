/*
 *
 * Configuration DB
 *
 * $Id: ldb.c,v 1.2 2001/10/09 22:33:07 paulsch Exp $
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

/* System includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <atm.h>
#include <ctype.h>

/* Local includes */
#include "lecs.h"
#include "ldb.h"
#include "mem_lecs.h"

/* Local protos */
static void dump_elan(Elan_t *elan);
static int valid_char(char test);
static const char* get_type_string(char lan_type);
static const char* get_max_frame_size_string(char max_frame);
static const char* get_atm_addr_string(const unsigned char *addr);
static void dump_elan(Elan_t *elan);
static int match_addresses(Elan_t *elan, unsigned char *addr);

/* Local data */
static unsigned char *lecs_address =NULL;
static Elan_t *elan_arr[32];
static Elan_t *default_elan = NULL;
static int no_elans=0;
static char tmpbuffer[256];

#define MIN(a,b) ((a<b)?a:b)
#define COMP_NAME "LDB"

Elan_t*
new_elan(const char *name)
{
  elan_arr[no_elans] = (Elan_t *)mem_alloc(COMP_NAME, sizeof(Elan_t));
  if (!elan_arr[no_elans]) {
    return NULL;
  }
  memset(elan_arr[no_elans],0,sizeof(Elan_t));
  elan_arr[no_elans]->elan_name_size = MIN(32, strlen(name));
  strncpy(elan_arr[no_elans]->elan_name, name, 
	  elan_arr[no_elans]->elan_name_size);
  /* Default values */
  elan_arr[no_elans]->type = LE_LAN_TYPE_UNSPECIFIED;
  elan_arr[no_elans]->max_frame = LE_MAX_FRAME_UNSPECIFIED;
  return elan_arr[no_elans++];
}

int
add_les(Elan_t *elan, const char *addr)
{  
  struct sockaddr_atmsvc tmp;

  assert(elan);
  if (elan->les_addr[18] || 
      elan->les_addr[17] ||
      elan->les_addr[16] ||
      elan->les_addr[15] ||
      elan->les_addr[14] ||
      elan->les_addr[13] ||
      elan->les_addr[12]) {
    printf("LES address already set for this ELAN!\n");
    return 0;
  }
  
  if (text2atm(addr,(struct sockaddr *)&tmp,sizeof(tmp),T2A_SVC|T2A_NAME)<0) {
    return -1;
  }
  memcpy(elan->les_addr, tmp.sas_addr.prv, ATM_ESA_LEN);
  return 0;
}

static int
valid_char(char test)
{
  return ((test >= '0' && test <= '9') || (test >= 'a' || test <= 'f') ||
	   (test >= 'A' && test <= 'F') || test == 'x' || test == 'X');
}

int 
add_atm(Elan_t *elan, char *addr)
{
  char *tmp;
  char *ch;
  int pos=0;

  ch = addr;

  assert(elan && addr);
  tmp = (char*)mem_alloc(COMP_NAME, ATM_ESA_LEN*2);
  if (!tmp)
    return -1;
  memset(tmp,0,ATM_ESA_LEN*2);
  while ((*ch)!='\0' && (*(ch+1))!='\0') {
    if (!valid_char(*ch)) {
      mem_free(COMP_NAME, tmp);
      return -1;
    }
    if (*(ch+1) == '.' || *(ch+1) == ':' || *(ch+1) == '-') {
      tmp[pos++] = '0';
      tmp[pos++] = *ch;
      ch+=2;
    } else if (valid_char(*(ch+1))) {
      tmp[pos++] = *ch;
      tmp[pos++] = *(ch+1);
      ch+=2;
    } else {
      mem_free(COMP_NAME, tmp);
      return -1;
    }
    if (*ch == '.' || *ch == '-' || *ch == ':')
      ch++;
  }
  if ((*ch) != '\0' && *(ch+1) == '\0') {
    tmp[pos++] = '0';
    tmp[pos++] = *ch;
  }
  if (pos<40) {
    mem_free(COMP_NAME, tmp);
    return -1;
  }
  elan->addresses[elan->no_addresses++] = tmp;
  return 0;
}

void 
set_default(Elan_t *elan)
{
  if (default_elan) {
    printf("Warning! Default ELAN already set!\n");
    return;
  }
  default_elan = elan;
}

static const char*
get_type_string(char lan_type)
{
  switch (lan_type) {
  case LE_LAN_TYPE_UNSPECIFIED:
    return "<Unspecified>";
  case LE_LAN_TYPE_802_3:
    return "802.3";
  case LE_LAN_TYPE_802_5:
    return "802.5";
  default:
    return "UNKNOWN TYPE";
  }
}

static const char*
get_max_frame_size_string(char max_frame)
{
  switch (max_frame) {
  case LE_MAX_FRAME_UNSPECIFIED:
    return "<Unspecified>";
  case LE_MAX_FRAME_1516:
    return "1516";
  case LE_MAX_FRAME_4544:
    return "4544";
  case LE_MAX_FRAME_9234:
    return "9234";
  case LE_MAX_FRAME_18190:
    return "18190";
  default:
    return "UNKNOWN FRAME SIZE";
  }
}

static const char*
get_atm_addr_string(const unsigned char *addr)
{
  int i;

  for(i=0;i<ATM_ESA_LEN-1;i++) {
    sprintf(&tmpbuffer[i*3],"%2.2x.", addr[i]&0xff);
  }
  sprintf(&tmpbuffer[(ATM_ESA_LEN-1)*3],"%2.2x", addr[ATM_ESA_LEN-1]);
  return tmpbuffer;
}

static void
dump_elan(Elan_t *elan)
{
  int i;

  printf("ELAN:%s (namelen:%d)\n",elan->elan_name, elan->elan_name_size);
  fflush(stdout);
  if (elan == default_elan) {
    printf("\tDEFAULT ELAN\n");
    fflush(stdout);
  }
  printf("\tMax frame size : %s\n", 
	 get_max_frame_size_string(elan->max_frame));
  fflush(stdout);
  printf("\tELAN type      : %s\n", get_type_string(elan->type));
  fflush(stdout);
  printf("\tLES address    : %s\n", get_atm_addr_string(elan->les_addr));
  fflush(stdout);
  for(i=0;i<elan->no_addresses;i++)
    printf("\t\t%s\n", elan->addresses[i]);
  printf("\t------------------------\n");
}

void 
dump_db(Elan_t *elan)
{
  int i;

  if (elan) {
    dump_elan(elan);
    return;
  }
  printf("Dumping whole ELAN db\n");
  if (lecs_address)
    printf("LECS address: %s\n", get_atm_addr_string(lecs_address));
  else
    printf("LECS address not set\n");
  for(i=0;i<no_elans;i++) {
    dump_elan(elan_arr[i]);
  }
}

void 
set_lecs_addr(const char *addr)
{
  struct sockaddr_atmsvc tmp;

  if (lecs_address) {
    printf("Error: LECS address already set\n");
  }
  if (text2atm(addr,(struct sockaddr*)&tmp,sizeof(tmp),T2A_SVC|T2A_NAME)<0) {
    printf("Couldn't get LECS address!\n");
    return;
  }
  lecs_address=(unsigned char*)mem_alloc(COMP_NAME,ATM_ESA_LEN);
  if (!lecs_address)
    return;
  memcpy(lecs_address, tmp.sas_addr.prv, ATM_ESA_LEN);
}

const unsigned char*
get_lecs_addr(void)
{
  return lecs_address;
}

void
reset_db(void)
{
  Elan_t *tmp;
  int i,j;
  
  if (lecs_address) {
    mem_free(COMP_NAME, lecs_address);
    lecs_address = NULL;
  }

  for(i=0;i<no_elans;i++) {
    tmp = elan_arr[i];
    for(j=0;j<tmp->no_addresses;j++)
      mem_free(COMP_NAME, tmp->addresses[j]);
    mem_free(COMP_NAME,tmp);
  }
  no_elans=0;
  default_elan=NULL;
}

static int
match_addresses(Elan_t *elan, unsigned char *addr)
{
  int i,j,match=0;
  unsigned char tmp;

  for(i=0;i<elan->no_addresses && !match;i++) {
    match=1;
    for(j=0;j<ATM_ESA_LEN && match;j++) {
      if (elan->addresses[i][j*2] == 'x' ||
	  elan->addresses[i][j*2] == 'X') {
	tmp = addr[j]&0xf0;
      } else {
	if (elan->addresses[i][j*2] >= '0' &&
	    elan->addresses[i][j*2] <= '9') {
	  tmp = (elan->addresses[i][j*2]-'0')<<4;
	} else {
	  tmp = (tolower(elan->addresses[i][j*2])-'a'+10)<<4;
	}
      }
      if (elan->addresses[i][j*2+1] == 'x' ||
	  elan->addresses[i][j*2+1] == 'X') {
	tmp |= addr[j]&0xf;
      } else {
	if (elan->addresses[i][j*2+1] >= '0' &&
	    elan->addresses[i][j*2+1] <= '9') {
	  tmp |= 0xf&(elan->addresses[i][j*2+1]-'0');
	} else {
	  tmp |= 0xf&(tolower(elan->addresses[i][j*2+1])-'a'+10);
	}
      }

      if (addr[j] != tmp)
	match=0;
    }
  }
  return match;
}

/*
 * Rules in finding the LES address:
 * 1. If elan_name matches exactly &&
 *    there is an entry matching this ATM address for this ELAN.
 *    If elan_name matches, but ATM address is not found, reject.
 * 2. Search for first ELAN which matches in type, max_frame and
 *    ATM address.
 * 3. Return default elan
 * 4. No match, reject.
 */
Elan_t*
find_elan(unsigned char *lec_addr, const char type, 
	  const char max_frame, const char *elan_name, 
	  const short elan_name_size, unsigned short *reason)
{
  int pos;

  *reason = LE_STATUS_SUCCESS;
  for(pos=0;pos<no_elans;pos++) {
    if (elan_name_size == elan_arr[pos]->elan_name_size &&
	!memcmp(elan_name, elan_arr[pos]->elan_name, elan_name_size)) {      
      if (match_addresses(elan_arr[pos], lec_addr)) {
	return elan_arr[pos];
      } else { 
	*reason = LE_STATUS_NO_ACCESS;
	return NULL;
      }
    }
  }
  for(pos=0;pos<no_elans;pos++) {
    if ((max_frame == LE_MAX_FRAME_UNSPECIFIED ||
	 elan_arr[pos]->max_frame == LE_MAX_FRAME_UNSPECIFIED ||
	 max_frame == elan_arr[pos]->max_frame) &&
	(type == LE_LAN_TYPE_UNSPECIFIED ||
	 elan_arr[pos]->type == LE_LAN_TYPE_UNSPECIFIED ||
	 type == elan_arr[pos]->type)) {
      if (match_addresses(elan_arr[pos], lec_addr)) {
	return elan_arr[pos];
      }
    }
  }
  if (default_elan)
    return default_elan;  
  *reason = LE_STATUS_NO_CONFIG;
  return NULL;
}
