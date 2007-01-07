/*
 * addr_hash.h:
 *
 */

#ifndef __SERV_HASH_H_ /* include guard */
#define __SERV_HASH_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "hash.h"

typedef struct {
    int port;
    int protocol;
} ip_service; 

hash_type* serv_hash_create(void);
void serv_hash_initialise(hash_type* sh);

#endif /* __SERV_HASH_H_ */
