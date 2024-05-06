#ifndef _JOOL_UNIT_TYPES_H
#define _JOOL_UNIT_TYPES_H

#include "mod/common/types.h"


int init_tuple4(struct tuple *tuple4, char *src_addr, __u16 src_port,
		char *dst_addr, __u16 dst_port, l4_protocol l4_proto);
int init_tuple6(struct tuple *tuple6, char *src_addr, __u16 src_port,
		char *dst_addr, __u16 dst_port, l4_protocol l4_proto);


#endif /* _JOOL_UNIT_TYPES_H */
