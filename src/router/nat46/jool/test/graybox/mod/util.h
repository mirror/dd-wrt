#ifndef FRAGS_MOD_UTIL_H
#define FRAGS_MOD_UTIL_H

static inline int get_l3_proto(void *l3_hdr)
{
	return (*((char *) l3_hdr)) >> 4;
}

#endif
