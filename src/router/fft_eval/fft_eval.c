/* 
 * Copyright (C) 2012 Simon Wunderlich <siwu@hrz.tu-chemnitz.de>
 * Copyright (C) 2012 Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
 * Copyright (C) 2013 Gui Iribarren <gui@altermundi.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 * 
 */

/*
 * This program has been created to aid open source spectrum
 * analyzer development for Qualcomm/Atheros AR92xx and AR93xx
 * based chipsets.
 */

#define _BSD_SOURCE
#include <endian.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>
#include <inttypes.h>

typedef int8_t s8;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;

/* taken from ath9k.h */
#define SPECTRAL_HT20_NUM_BINS          56

enum ath_fft_sample_type {
        ATH_FFT_SAMPLE_HT20 = 1
};

struct fft_sample_tlv {
        u8 type;        /* see ath_fft_sample */
        u16 length;
        /* type dependent data follows */
} __attribute__((packed));

struct fft_sample_ht20 {
        struct fft_sample_tlv tlv;

        u8 max_exp;

        u16 freq;
        s8 rssi;
        s8 noise;

        u16 max_magnitude;
        u8 max_index;
        u8 bitmap_weight;

        u64 tsf;

        u8 data[SPECTRAL_HT20_NUM_BINS];
} __attribute__((packed));


struct scanresult {
	struct fft_sample_ht20 sample;
	struct scanresult *next;
};

struct scanresult *result_list;
int scanresults_n = 0;

/*
 * print_values - spit out the analyzed values in text form, JSON-like.
 */
void print_values()
{
	int i, rnum;
	struct scanresult *result;

	printf("[");
	rnum = 0;
	for (result = result_list; result ; result = result->next) {
		int datamax = 0, datamin = 65536;
		int datasquaresum = 0;

		for (i = 0; i < SPECTRAL_HT20_NUM_BINS; i++) {
			int data;

			data = (result->sample.data[i] << result->sample.max_exp);
			data *= data;
			datasquaresum += data;
			if (data > datamax) datamax = data;
			if (data < datamin) datamin = data;
		}

		/* prints some statistical data about the
		 * data sample and auxiliary data. */
		printf("\n{ \"tsf\": %"PRIu64", \"central_freq\": %d, \"rssi\": %d, \"noise\": %d, \"data\": [ ",
			result->sample.tsf, result->sample.freq, result->sample.rssi, result->sample.noise);

		for (i = 0; i < SPECTRAL_HT20_NUM_BINS; i++) {
			float freq;
			float signal;
			int data;
			freq = result->sample.freq - 10.0 + ((20.0 * i) / SPECTRAL_HT20_NUM_BINS);
			
			/* This is where the "magic" happens: interpret the signal
			 * to output some kind of data which looks useful.  */

			data = result->sample.data[i] << result->sample.max_exp;
			if (data == 0)
				data = 1;
			signal = result->sample.noise + result->sample.rssi + 20 * log10f(data) - log10f(datasquaresum) * 10;

			printf("[ %f, %f ]", freq, signal);
			if ( i < SPECTRAL_HT20_NUM_BINS - 1 )
				printf(", ");
		}
		printf(" ] }");
		if ( result->next )
			printf(",");
		rnum++;
	}
	printf("\n]\n");

	return 0;
}

/* read_file - reads an file into a big buffer and returns it
 *
 * @fname: file name
 *
 * returns the buffer with the files content
 */
char *read_file(char *fname, size_t *size)
{
	FILE *fp;
	char *buf = NULL;
	size_t ret;

	fp = fopen(fname, "r");

	if (!fp)
		return NULL;

	*size = 0;
	while (!feof(fp)) {

		buf = realloc(buf, *size + 4097);
		if (!buf)
			return NULL;

		ret = fread(buf + *size, 1, 4096, fp);
		*size += ret;
	}
	fclose(fp);

	buf[*size] = 0;

	return buf;
}

/*
 * read_scandata - reads the fft scandata and compiles a linked list of datasets
 *
 * @fname: file name
 *
 * returns 0 on success, -1 on error.
 */
int read_scandata(char *fname)
{
	char *pos, *scandata;
	size_t len, sample_len;
	struct scanresult *result;
	struct fft_sample_tlv *tlv;
	struct scanresult *tail = result_list;

	scandata = read_file(fname, &len);
	if (!scandata)
		return -1;

	pos = scandata;

	while (pos - scandata < len) {
		tlv = (struct fft_sample_tlv *) pos;
		sample_len = sizeof(*tlv) + be16toh(tlv->length);
		pos += sample_len;
		if (tlv->type != ATH_FFT_SAMPLE_HT20) {
			fprintf(stderr, "unknown sample type (%d)\n", tlv->type);
			continue;
		}

		if (sample_len != sizeof(result->sample)) {
			fprintf(stderr, "wrong sample length (have %zd, expected %zd)\n", sample_len, sizeof(result->sample));
			continue;
		}

		result = malloc(sizeof(*result));
		if (!result)
			continue;

		memset(result, 0, sizeof(*result));
		memcpy(&result->sample, tlv, sizeof(result->sample));
		fprintf(stderr, "copy %zd bytes\n", sizeof(result->sample));

		result->sample.freq = be16toh(result->sample.freq);
		result->sample.max_magnitude = be16toh(result->sample.max_magnitude);
		result->sample.tsf = be64toh(result->sample.tsf);
		
		if (tail)
			tail->next = result;
		else
			result_list = result;

		tail = result;

		scanresults_n++;
	}

	fprintf(stderr, "read %d scan results\n", scanresults_n);
	return 0;
}

void usage(int argc, char *argv[])
{
	fprintf(stderr, "Usage: %s [scanfile]\n", argv[0]);
	fprintf(stderr, "\n");
	fprintf(stderr, "scanfile is generated by the spectral analyzer feature\n");
	fprintf(stderr, "of your wifi card. If you have a AR92xx or AR93xx based\n");
	fprintf(stderr, "card, try:\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "ifconfig wlan0 up\n");
	fprintf(stderr, "echo chanscan > /sys/kernel/debug/ieee80211/phy0/ath9k/spectral_scan_ctl\n");
	fprintf(stderr, "iw dev wlan0 scan\n");
	fprintf(stderr, "cat /sys/kernel/debug/ieee80211/phy0/ath9k/spectral_scan0 > /tmp/fft_results\n");
	fprintf(stderr, "echo disable > /sys/kernel/debug/ieee80211/phy0/ath9k/spectral_scan_ctl\n");
	fprintf(stderr, "%s /tmp/fft_results\n", argv[0]);
	fprintf(stderr, "\n");
	fprintf(stderr, "(NOTE: maybe debugfs must be mounted first: mount -t debugfs none /sys/kernel/debug/ )\n");
	fprintf(stderr, "\n");

}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		usage(argc, argv);
		return -1;
	}

	fprintf(stderr, "WARNING: Experimental Software! Don't trust anything you see. :)\n");
	fprintf(stderr, "\n");
	if (read_scandata(argv[1]) < 0) {
		fprintf(stderr, "Couldn't read scanfile ...\n");
		usage(argc, argv);
		return -1;
	}
	print_values();

	return 0;
}
