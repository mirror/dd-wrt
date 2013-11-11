#ifndef IPTRAF_NG_RATE_H
#define IPTRAF_NG_RATE_H

#include <sys/time.h>

/* SMA = Simple Moving Average */

/*
 *	SMA = (p(M) + p(M-1) + ... + p(M-n-1)) / n
 *
 *	or
 *
 *	SMA = last_SMA - (p(M-n) / n) + (p(M) / n)
 *
 *	I choose the first one because there is smaller rounding
 *	error when using integer divide.
 */

struct rate {
	unsigned int		n;	/* number of elements */
	unsigned int		index;	/* index into the values array */
	unsigned long long     *rates;	/* units are: bytes per second */
	unsigned long		sma;	/* simple moving average */
};

void rate_init(struct rate *rate);
void rate_alloc(struct rate *rate, unsigned int n);
void rate_destroy(struct rate *rate);
void rate_add_rate(struct rate *rate, unsigned long bytes,
		   unsigned long msecs);
unsigned long rate_get_average(struct rate *rate);
int rate_print(unsigned long rate, char *buf, unsigned n);
int rate_print_no_units(unsigned long rate, char *buf, unsigned n);
int rate_print_pps(unsigned long rate, char *buf, unsigned n);

#endif /* IPTRAF_NG_RATE_H */
