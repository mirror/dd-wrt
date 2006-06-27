#ifndef _IPT_XOR_H
#define _IPT_XOR_H

struct ipt_XOR_info {
	char		key[30];
	u_int8_t	block_size;
};

#endif /* _IPT_XOR_H */
