/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

#include "iptraf-ng-compat.h"
#include "options.h"
#include "rate.h"

void rate_init(struct rate *rate)
{
	if (!rate)
		return;

	rate->index = 0;
	rate->sma = 0;
	memset(rate->rates, 0, rate->n * sizeof(rate->rates[0]));
}

void rate_alloc(struct rate *rate, unsigned int n)
{
	if (!rate)
		return;

	rate->n = n;
	rate->rates = xmalloc(n * sizeof(rate->rates[0]));

	rate_init(rate);
}

void rate_destroy(struct rate *rate)
{
	if (!rate)
		return;

	rate->n = 0;
	if (!rate->rates)
		return;

	free(rate->rates);
	rate->rates = NULL;
}

void rate_add_rate(struct rate *rate, unsigned long bytes,
		   unsigned long msecs)
{
	if (!rate)
		return;

	rate->rates[rate->index] = bytes * 1000ULL / msecs;

	if ((rate->index + 1) >= rate->n)
		rate->index = 0;
	else
		rate->index++;

	/* compute the moving average */
	unsigned long long sum = 0;
	for(unsigned int i = 0; i < rate->n; i++)
		sum += rate->rates[i];
	rate->sma = sum / rate->n;
}

unsigned long rate_get_average(struct rate *rate)
{
	if (rate)
		return rate->sma;
	else
		return 0UL;
}

int rate_print(unsigned long rate, char *buf, unsigned n)
{
	char *suffix[] = { "k", "M", "G", "T", "P", "E", "Z", "Y" };
	unsigned n_suffix = ARRAY_SIZE(suffix);

	int chars;

	if (options.actmode == KBITS) {
		unsigned long tmp = rate;
		unsigned int i = 0;
		unsigned long divider = 1000;

		rate *= 8;
		while(tmp >= 100000000) {
			tmp /= 1000;
			i++;
			divider *= 1000;
		}
		if (i >= n_suffix)
			chars = snprintf(buf, n, "error");
		else
			chars = snprintf(buf, n, "%9.2f %sbps", (double)rate / divider, suffix[i]);
	} else {
		unsigned int i = 0;

		while(rate > 99 * (1UL << 20)) {
			rate >>= 10;
			i++;
		}
		if (i >= n_suffix)
			chars = snprintf(buf, n, "error");
		else
			chars = snprintf(buf, n, "%9.2f %sBps", (double)rate / 1024, suffix[i]);
	}
	buf[n - 1] = '\0';

	return chars;
}

int rate_print_no_units(unsigned long rate, char *buf, unsigned n)
{
	int chars;

	if (options.actmode == KBITS) {
		chars = snprintf(buf, n, "%8.1f", (double)rate * 8 / 1000);
	} else {
		chars = snprintf(buf, n, "%8.1f", (double)rate / 1024);
	}
	buf[n - 1] = '\0';

	return chars;
}

int rate_print_pps(unsigned long rate, char *buf, unsigned n)
{
	int chars;

	chars = snprintf(buf, n, "%9lu pps", rate);
	buf[n - 1] = '\0';

	return chars;
}
