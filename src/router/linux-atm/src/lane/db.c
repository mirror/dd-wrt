/*
 * Database handling functions
 *
 * $Id: db.c,v 1.2 2001/10/09 22:33:06 paulsch Exp $
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

/* System includes */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

/* Local includes */
#include "mem.h"
#include "lane.h"
#include "dump.h"
#include "load.h"
#include "connect.h"
#include "events.h"
#include "db.h"

Reg_t *reglist;

Proxy_t *proxylist;

Lecdb_t *leclist;

/* Protos */

/* Adds entry to PROXY-DB */
void 
proxydb_add(const Conn_t *conn, int fd)
{
  Proxy_t *tmp;

  assert(conn != NULL);

  Debug_unit(&conn_unit,"Proxydb_add");
  tmp = (Proxy_t *) mem_alloc(&conn_unit, sizeof(Proxy_t));
  tmp->lecid = conn->lecid;
  tmp->fd = fd;
  tmp->next = proxylist;
  proxylist = tmp;
  return;
}

/* Removes entry from PROXY-DB */
int 
proxydb_remove(const Conn_t *conn)
{
  Proxy_t *tmp, *tmp2;
  
  Debug_unit(&conn_unit,"Proxydb_remove");
  if (proxylist == NULL) {
    return 0;
  }
  
  tmp = proxylist;
  if (conn->lecid == tmp->lecid) {
    proxylist =tmp->next;
    mem_free(&conn_unit, tmp);
    return 1;
  }
  tmp2 = tmp;
  tmp = tmp->next;
  
  while(tmp) {
    if (conn->lecid == tmp->lecid) {
      tmp2->next = tmp->next;
      mem_free(&conn_unit, tmp);
      return 1;
    }
    tmp = tmp->next;
  }
  dump_error(&conn_unit,"Trying to remove unexisting entry from PROXY-DB");
  return 0;
}

Proxy_t*
proxydb_find(LecId_t to_find)
{
  Proxy_t *tmp;

  tmp = proxylist;
  while(tmp) {
    if (to_find == tmp->lecid) {
      Debug_unit(&conn_unit, "Proxy_t found");
      return tmp;
    }
    tmp = tmp->next;
  }
  Debug_unit(&conn_unit,"Proxy_t not found from database");
  return NULL;
}

/* Adds entry to LECID-DB */
void 
leciddb_add(LecId_t lecid, AtmAddr_t address, const int fd)
{
  Lecdb_t *tmp;

  Debug_unit(&conn_unit, "Leciddb_add called");
  tmp = (Lecdb_t *) mem_alloc(&conn_unit, sizeof(Lecdb_t));
  tmp->fd = fd;
  memcpy(&tmp->address, &address, sizeof(AtmAddr_t));
  tmp->lecid = lecid;
  tmp->next = leclist;
  leclist = tmp;
  return;
}

int leciddb_remove(LecId_t to_remove)
{
  Lecdb_t *tmp, *tmp2;

  Debug_unit(&conn_unit,"Leciddb_remove");
  if (leclist == NULL) {
    return 0;
  }

  tmp = leclist;
  if (to_remove == tmp->lecid) {
    leclist =tmp->next;
    mem_free(&conn_unit, tmp);
    return 1;
  }
  tmp2 = tmp;
  tmp = tmp->next;
  
  while(tmp) {
    if (to_remove == tmp->lecid) {
      tmp2->next = tmp->next;
      mem_free(&conn_unit, tmp);
      return 1;
    }
    tmp = tmp->next;
  }
  dump_error(&conn_unit,"Trying to remove unexisting entry from LECID-DB");
  return 0;
}

/* Searches LECID-DB for entry */
Lecdb_t *leciddb_find(LecId_t to_find)
{
  Lecdb_t *tmp;

  tmp = leclist;
  while(tmp) {
    if (to_find == tmp->lecid) {
      Debug_unit(&conn_unit, "Lecdb_t found");
      return tmp;
    }
    tmp = tmp->next;
  }
  Debug_unit(&conn_unit,"Lecdb_t not found from database");
  return NULL;
}

Lecdb_t *leciddb_find_atm(AtmAddr_t to_find)
{
  Lecdb_t *tmp;
  
  tmp = leclist;
  while (tmp) {
    if (memcmp(&to_find,&(tmp->address), sizeof(AtmAddr_t)) == 0) {
      Debug_unit(&conn_unit, "Lecdb_t found");
      return tmp;
    }
    tmp= tmp->next;    
  }
  Debug_unit(&conn_unit,"Lecid_t not found from database");
  return NULL;
}

/* Adds entry to REG-DB */
void regdb_add(AtmAddr_t aaddr, LaneDestination_t maddr)
{
  Reg_t *tmp;
  
  Debug_unit(&conn_unit,"Regdb_add");
  tmp = (Reg_t *)mem_alloc(&conn_unit, sizeof(Reg_t));
  memcpy(&tmp->mac_address, &maddr,sizeof(LaneDestination_t));
  memcpy(&tmp->atm_address, &aaddr, sizeof(AtmAddr_t));
  tmp->next = reglist;
  reglist = tmp;
  return;
}

/* Removes entry from REG-DB.
   Returns 0 if entry is not found. 1 otherwise.
*/
int 
regdb_remove(AtmAddr_t to_remove)
{
  Reg_t *tmp, *tmp2;

  Debug_unit(&conn_unit,"Regdb_remove");
  if (reglist == NULL) {
    return 0;
  }

  tmp = reglist;
  if (memcmp((char *)&to_remove, (char *)&tmp->atm_address, 
	      sizeof(AtmAddr_t)) == 0) {
    reglist =tmp->next;
    mem_free(&conn_unit, tmp);
    return 1;
  }
  tmp2 = tmp;
  tmp = tmp->next;
  
  while(tmp) {
    if (memcmp((char *)&to_remove, (char *)&tmp->atm_address, 
		sizeof(AtmAddr_t)) == 0) {
      tmp2->next = tmp->next;
      mem_free(&conn_unit, tmp);
      return 1;
    }
    tmp = tmp->next;
  }
  dump_error(&conn_unit,"Trying to remove unexisting entry from REG-DB");
  return 0;
}

/* Finds entry from REG_DB by comparing mac address */
Reg_t*
regdb_find_mac(LaneDestination_t maddr)
{
  Reg_t *tmp;

  tmp = reglist;
  while (tmp) {

    if((tmp->mac_address).tag == htons(LANE_DEST_MAC)) {
      if (memcmp((char *)&(tmp->mac_address).a_r.mac_address, 
		 (char *)&maddr.a_r.mac_address, 6) ==0) {
	Debug_unit(&conn_unit, "MAC found from database");
	return tmp;
      }
    }
    else if ((tmp->mac_address).tag == htons(LANE_DEST_RD)) {
      if (memcmp((char *)&(tmp->mac_address.a_r.route),
		  (char *)&maddr.a_r.route, 
		  4 + sizeof(unsigned short)) == 0) {
	Debug_unit(&conn_unit, "MAC found from database");
	return tmp;
      }
      else if (memcmp((char *)&(tmp->mac_address),
		       (char *)&maddr, 
		       sizeof(LaneDestination_t)) == 0) {
	Debug_unit(&conn_unit, "MAC found from database");
        return tmp;
      }
    }
    tmp = tmp->next;
  }
  Debug_unit(&conn_unit, "MAC address not found from database");
  return NULL;
}


