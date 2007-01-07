/*
 * ns_hash.h:
 *
 * Copyright (c) 2002 DecisionSoft Ltd.
 *
 */

#ifndef __NS_HASH_H_ /* include guard */
#define __NS_HASH_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "hash.h"

hash_type* ns_hash_create(void);

#endif /* __NS_HASH_H_ */
