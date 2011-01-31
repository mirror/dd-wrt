#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <time.h>
#include <atm.h>
#include <stdlib.h>
#include <string.h>
#include "packets.h"

struct rqst_id{
  struct rqst_id *next;
  struct rqst_id *prev;
  uint32_t request_id;
  uint32_t cache_id;
  uint8_t type;
  time_t created; 
};

static struct rqst_id *ids = NULL; /* pointer to first item in id list*/

/*
 * Searches the list according to request id.
 *
 */
static struct rqst_id *search_by_id(uint32_t id){
  struct rqst_id *pid = ids;
  while(pid != NULL){
    if(id == pid->request_id)
      return pid;
  pid = pid->next;
  }
  return NULL;
}

/*
 * Searches the list accoring to type and cache_id. 
 * Refreshes the created time of the list item if found. 
 * Returns a request_id on success.
 *
 */
uint32_t search_by_type(uint8_t type, uint32_t cache_id){
  struct rqst_id *pid = ids;
  while(pid != NULL){
    if(type == pid->type){
      if(pid->cache_id == cache_id){
	pid->created = time(NULL);
	return pid->request_id;
      }
    }
    pid = pid->next;
  }
  return 0;
}

/*
 * Creates a new id and adds it to list.
 *
 */
int new_id(uint32_t id, uint32_t cache_id, uint8_t type){
  struct rqst_id *new;
  if( search_by_id(id) != NULL )
    return 0;
  new = malloc(sizeof(struct rqst_id));
  memset(new,0,sizeof(struct rqst_id));
  new->request_id = id;
  new->cache_id = cache_id;
  new->type = type;
  new->created = time(NULL);
  new->next = ids;
  new->prev = NULL;
  if(ids != NULL)
    ids->prev = new;
  ids = new;
  return 1;
}

/*
 * Removes an id from the list and frees the allocated memory
 *
 */
static int remove_id(struct rqst_id *pid){
  printf("mpcd: id_list.c: removing id %u from the id_list.\n",pid->request_id);
  if(pid == NULL)
    return 0;
  if(pid->prev != NULL)
    pid->prev->next = pid->next;
  else
    ids = pid->next;
  if(pid->next != NULL)
    pid->next->prev = pid->prev;
  free(pid);
  return 1;
}

/* 
 * Removes ids that have stayed longer than ID_EXPIRING_TIME
 * from the list.
 *
 */
void clear_expired(){
  static int call_count=0;
  struct rqst_id *pid = ids;
  struct rqst_id *next_pid;
  time_t now = time(NULL);
  call_count++;
  if(call_count%5 != 0)
    return;
  while(pid != NULL){
    next_pid = pid->next;
    if(now - pid->created > ID_EXPIRING_TIME)
      remove_id(pid);
    pid = next_pid;
  }
  call_count = 0;
}

/* 
 * Checks whether we have sent a request with id returned in a reply 
 * and that the reply is of right type.  
 *
 */
int check_incoming(uint32_t id, uint8_t type){
  struct rqst_id *pid = search_by_id(id);
  if( pid == NULL ){
    printf("mpcd: id_list.c: no request sent with request_id %d\n",id);
    return 0;
  }
  if( pid->type != type){
    printf("mpcd: id_list.c: matching request_id found, but packet type is invalid %d\n",type);
    return 0;
  }
  remove_id(pid);
  return 1;
}

