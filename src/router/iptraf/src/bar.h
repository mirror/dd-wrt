#ifndef IPTRAF_NG_BAR_H
#define IPTRAF_NG_BAR_H

void set_barptr(void **barptr, void *entry, time_t * starttime, void *spanbr,
		size_t size, WINDOW * win, int *cleared, int x);

#endif	/* IPTRAF_NG_BAR_H */
