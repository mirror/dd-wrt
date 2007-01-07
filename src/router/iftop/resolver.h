/*
 * resolver.h:
 *
 */

#ifndef __RESOLVER_H_ /* include guard */
#define __RESOLVER_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>


void resolver_initialise(void);

void resolve(struct in_addr* addr, char* result, int buflen);

#endif /* __RESOLVER_H_ */
