#ifndef IPTRAF_NG_CIDR_H
#define IPTRAF_NG_CIDR_H

unsigned long cidr_get_mask(unsigned int maskbits);
char *cidr_get_quad_mask(unsigned int maskbits);
unsigned int cidr_get_maskbits(unsigned long mask);
void cidr_split_address(char *cidr_addr, char *addresspart,
			unsigned int *maskbits);

#endif	/* IPTRAF_NG_CIDR_H */
