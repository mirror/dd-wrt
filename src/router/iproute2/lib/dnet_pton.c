#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "utils.h"

static __inline__ u_int16_t dn_htons(uint16_t addr)
{
        union {
                uint8_t byte[2];
                uint16_t word;
        } u;

        u.word = addr;
        return ((uint16_t)u.byte[0]) | (((uint16_t)u.byte[1]) << 8);
}


static int dnet_num(const char *src, uint16_t * dst)
{
	int rv = 0;
	int tmp;
	*dst = 0;

	while ((tmp = *src++) != 0) {
		tmp -= '0';
		if ((tmp < 0) || (tmp > 9))
			return rv;

		rv++;
		(*dst) *= 10;
		(*dst) += tmp;
	}

	return rv;
}

static int dnet_pton1(const char *src, struct dn_naddr *dna)
{
	uint16_t area = 0;
	uint16_t node = 0;
	int pos;

	pos = dnet_num(src, &area);
	if ((pos == 0) || (area > 63) || (*(src + pos) != '.'))
		return 0;
	pos = dnet_num(src + pos + 1, &node);
	if ((pos == 0) || (node > 1023))
		return 0;
	dna->a_len = 2;
	*(uint16_t *)dna->a_addr = dn_htons((area << 10) | node);

	return 1;
}

int dnet_pton(int af, const char *src, void *addr)
{
	int err;

	switch (af) {
	case AF_DECnet:
		errno = 0;
		err = dnet_pton1(src, (struct dn_naddr *)addr);
		break;
	default:
		errno = EAFNOSUPPORT;
		err = -1;
	}

	return err;
}
