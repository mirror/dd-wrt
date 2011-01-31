/* policy.h - Access control policies */
 
/* Written 1997 by Werner Almesberger, EPFL-LRC */
 

#ifndef POLICY_H
#define POLICY_H

#define ACL_ALLOW	1
#define ACL_REJECT	2
#define ACL_IN		4
#define ACL_OUT		8


typedef struct _rule {
    int type;
    struct sockaddr_atmsvc addr;
    int mask; /* -1 for none */
    int hits;
    struct _rule *next;
} RULE;


void add_rule(RULE *rule);
int allow(const struct sockaddr_atmsvc *addr,int direction);

#endif
