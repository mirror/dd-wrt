/* 
 * Copyright (C) 2012 Simon Wunderlich <siwu@hrz.tu-chemnitz.de>
 * Copyright (C) 2012 Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
 * Copyright (C) 2013 Gui Iribarren <gui@altermundi.net>
 * Copyright (C) 2016 - 2025 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include <stdbool.h>

#define _BSD_SOURCE
#ifdef __FreeBSD__
#include <sys/endian.h>
#else
#include <endian.h>
#endif /* __FreeBSD__ */
#define CONVERT_BE16(val) val = be16toh(val)
#define CONVERT_BE32(val) val = be32toh(val)
#define CONVERT_BE64(val) val = be64toh(val)

typedef int8_t s8;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;

/* taken from ath9k.h */
#define SPECTRAL_HT20_NUM_BINS 56
#define SPECTRAL_HT20_40_NUM_BINS 128

enum ath_fft_sample_type {
	ATH_FFT_SAMPLE_HT20 = 1,
	ATH_FFT_SAMPLE_HT20_40 = 2,
	ATH_FFT_SAMPLE_ATH10K = 3,
	ATH_FFT_SAMPLE_ATH11K = 4,
};

enum nl80211_channel_type { NL80211_CHAN_NO_HT, NL80211_CHAN_HT20, NL80211_CHAN_HT40MINUS, NL80211_CHAN_HT40PLUS };

struct fft_sample_tlv {
	u8 type; /* see ath_fft_sample */
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

struct fft_sample_ht20_40 {
	struct fft_sample_tlv tlv;

	u8 channel_type;
	u16 freq;

	s8 lower_rssi;
	s8 upper_rssi;

	u64 tsf;

	s8 lower_noise;
	s8 upper_noise;

	u16 lower_max_magnitude;
	u16 upper_max_magnitude;

	u8 lower_max_index;
	u8 upper_max_index;

	u8 lower_bitmap_weight;
	u8 upper_bitmap_weight;

	u8 max_exp;

	u8 data[SPECTRAL_HT20_40_NUM_BINS];
} __attribute__((packed));

/*
 * ath10k spectral sample definition
 */

#define SPECTRAL_ATH10K_MAX_NUM_BINS 256
#define SPECTRAL_ATH11K_MAX_NUM_BINS 1024

struct fft_sample_ath10k {
	struct fft_sample_tlv tlv;
	u8 chan_width_mhz;
	uint16_t freq1;
	uint16_t freq2;
	int16_t noise;
	uint16_t max_magnitude;
	uint16_t total_gain_db;
	uint16_t base_pwr_db;
	uint64_t tsf;
	s8 max_index;
	u8 rssi;
	u8 relpwr_db;
	u8 avgpwr_db;
	u8 max_exp;

	u8 data[0];
} __attribute__((packed));

struct fft_sample_ath11k {
	struct fft_sample_tlv tlv;
	u8 chan_width_mhz;
	s8 max_index;
	u8 max_exp;
	bool is_primary;
	uint16_t freq1;
	uint16_t freq2;
	uint16_t max_magnitude;
	uint16_t rssi;
	uint32_t tsf;
	int32_t noise;

	u8 data[0];
} __attribute__((packed));

struct scanresult {
	union {
		struct fft_sample_tlv tlv;
		struct fft_sample_ht20 ht20;
		struct fft_sample_ht20_40 ht40;
		struct {
			struct fft_sample_ath10k header;
			u8 data[SPECTRAL_ATH10K_MAX_NUM_BINS];
		} ath10k;
		struct {
			struct fft_sample_ath11k header;
			u8 data[SPECTRAL_ATH11K_MAX_NUM_BINS];
		} ath11k;
	} sample;
	struct scanresult *next;
};

struct scanresult *result_list;
int scanresults_n = 0;

struct resultsort {
	int freq;
	int signal;
};

static struct resultsort *initbins(int bins)
{
	struct resultsort *b;
	int i;
	b = malloc(sizeof(struct resultsort) * bins);
	for (i = 0; i < bins; i++) {
		b[i].signal = -1;
	}
	return b;
}

static int notvalid(int val)
{
	return val == -1;
}

static inline unsigned msb(const unsigned int x)
{
	unsigned int lz;
	lz = __builtin_clz(x);
	return ((sizeof(x) * CHAR_BIT) - 1) - lz;
}

static unsigned int fast_log10(const unsigned int x)
{
	static const uint16_t lut[8] = { 0, 2, 13, 69, 347, 1736, 8680, 43402 };
	const unsigned int shr = (msb(x | 1) & ((sizeof(x) * CHAR_BIT) - 2)) << 1, dig = (0x8877665433221100ull >> shr) & 0xF,
			   val = lut[(0x07D63440u >> (dig * 3)) & 0x7],
			   p10 = ((val << dig) + (~((unsigned)-1 << dig) & 0xC7)) * 90 + 10;
	return dig + (x >= p10 ? 2 : 1);
}
static void insert(struct resultsort *b, int bins, int freq, int signal)
{
	int i;
	if (notvalid(signal))
		return; // ignore
	/* seek for already existing frequency, in case order is incorrect */
	for (i = 0; i < bins; i++) {
		if (b[i].freq == freq) {
			b[i].signal += signal;
			b[i].signal /= 2;
			return;
		}
	}
	/* insert new slot */
	for (i = 0; i < bins; i++) {
		if (notvalid(b[i].signal)) {
			b[i].freq = freq;
			b[i].signal = signal;
			return;
		}
	}
}

/*
 * print_values - spit out the analyzed values in text form, JSON-like.
 */
static int print_values()
{
	int i, rnum;
	struct scanresult *result;
	struct resultsort *b = NULL;
	int bins;
	fprintf(stdout, "[");
	rnum = 0;
	for (result = result_list; result; result = result->next) {
		switch (result->sample.tlv.type) {
		case ATH_FFT_SAMPLE_HT20: {
			int datamax = 0, datamin = 65536;
			int datasquaresum = 0;

			/* prints some statistical data about the
				 * data sample and auxiliary data. */
			for (i = 0; i < SPECTRAL_HT20_NUM_BINS; i++) {
				int data;
				data = (result->sample.ht20.data[i] << result->sample.ht20.max_exp);
				data *= data;
				datasquaresum += data;
				if (data > datamax)
					datamax = data;
				if (data < datamin)
					datamin = data;
			}
			if (!rnum)
				fprintf(stdout,
					"\n{ \"tsf\": %" PRIu64
					", \"central_freq\": %d, \"rssi\": %d, \"noise\": %d, \"data\": [ \n",
					result->sample.ht20.tsf, result->sample.ht20.freq, result->sample.ht20.rssi,
					result->sample.ht20.noise);
			bins = SPECTRAL_HT20_NUM_BINS;
			if (!b)
				b = initbins(bins);

			for (i = 0; i < bins; i++) {
				int freq;
				int signal;
				int data;
				/*
					 * According to Dave Aragon from University of Washington,
					 * formerly Trapeze/Juniper Networks, in 2.4 GHz it should
					 * divide 22 MHz channel width into 64 subcarriers but
					 * only report the middle 56 subcarriers.
					 *
					 * For 5 GHz we do not know (Atheros claims it does not support
					 * this frequency band, but it works).
					 *
					 * Since all these calculations map pretty much to -10/+10 MHz,
					 * and we don't know better, use this assumption as well in 5 GHz.
					 */
				freq = result->sample.ht20.freq - (22 * bins / 64) / 2 + (22 * (((i * 10) + 5) / 64) / 10);

				/* This is where the "magic" happens: interpret the signal
					 * to output some kind of data which looks useful.  */

				data = result->sample.ht20.data[i] << result->sample.ht20.max_exp;
				if (data == 0)
					data = 1;
				signal = result->sample.ht20.noise + result->sample.ht20.rssi + 20 * fast_log10(data) -
					 fast_log10(datasquaresum) * 10;
				insert(b, bins, freq, signal);
			}
		} break;
		case ATH_FFT_SAMPLE_HT20_40: {
			int datamax = 0, datamin = 65536;
			int datasquaresum_lower = 0;
			int datasquaresum_upper = 0;
			int datasquaresum;
			int i;
			int centerfreq;
			s8 noise;
			s8 rssi;
			//todo build average
			bins = SPECTRAL_HT20_40_NUM_BINS;

			for (i = 0; i < bins / 2; i++) {
				int data;

				data = result->sample.ht40.data[i];
				data <<= result->sample.ht40.max_exp;
				data *= data;
				datasquaresum_lower += data;

				if (data > datamax)
					datamax = data;
				if (data < datamin)
					datamin = data;
			}

			for (i = bins / 2; i < bins; i++) {
				int data;

				data = result->sample.ht40.data[i];
				data <<= result->sample.ht40.max_exp;
				data *= data;
				datasquaresum_upper += data;

				if (data > datamax)
					datamax = data;
				if (data < datamin)
					datamin = data;
			}

			switch (result->sample.ht40.channel_type) {
			case NL80211_CHAN_HT40PLUS:
				centerfreq = result->sample.ht40.freq + 10;
				break;
			case NL80211_CHAN_HT40MINUS:
				centerfreq = result->sample.ht40.freq - 10;
				break;
			default:
				return -1;
			}

			if (!rnum)
				fprintf(stdout,
					"\n{ \"tsf\": %" PRIu64
					", \"central_freq\": %d, \"rssi\": %d, \"noise\": %d, \"data\": [ \n",
					result->sample.ht40.tsf, centerfreq, result->sample.ht40.lower_rssi,
					result->sample.ht40.lower_noise);

			if (!b)
				b = initbins(bins);

			for (i = 0; i < bins; i++) {
				int freq;
				int data;

				freq = centerfreq - (40 * bins / 128) / 2 + (40 * (((i * 10) + 5) / 128) / 10);

				if (i < bins / 2) {
					noise = result->sample.ht40.lower_noise;
					datasquaresum = datasquaresum_lower;
					rssi = result->sample.ht40.lower_rssi;
				} else {
					noise = result->sample.ht40.upper_noise;
					datasquaresum = datasquaresum_upper;
					rssi = result->sample.ht40.upper_rssi;
				}

				data = result->sample.ht40.data[i];
				data <<= result->sample.ht40.max_exp;

				if (data == 0)
					data = 1;
				int signal = noise + rssi + 20 * fast_log10(data) - fast_log10(datasquaresum) * 10;
				insert(b, bins, freq, signal);
			}
		} break;
		case ATH_FFT_SAMPLE_ATH10K: {
			int datamax = 0, datamin = 65536;
			int datasquaresum = 0;
			int i;
			if (!rnum)
				fprintf(stdout,
					"\n{ \"tsf\": %" PRIu64
					", \"central_freq\": %d, \"rssi\": %d, \"noise\": %d, \"data\": [ \n",
					result->sample.ath10k.header.tsf, result->sample.ath10k.header.freq1,
					result->sample.ath10k.header.rssi, result->sample.ath10k.header.noise);

			bins = result->sample.tlv.length -
			       (sizeof(result->sample.ath10k.header) - sizeof(result->sample.ath10k.header.tlv));

			for (i = 0; i < bins; i++) {
				int data;

				data = (result->sample.ath10k.data[i] << result->sample.ath10k.header.max_exp);
				data *= data;
				datasquaresum += data;
				if (data > datamax)
					datamax = data;
				if (data < datamin)
					datamin = data;
			}

			if (!b)
				b = initbins(bins);

			for (i = 0; i < bins; i++) {
				int freq;
				int data;
				int signal;
				freq = result->sample.ath10k.header.freq1 - (result->sample.ath10k.header.chan_width_mhz) / 2 +
				       ((result->sample.ath10k.header.chan_width_mhz * ((i * 10) + 5) / bins) / 10);

				data = result->sample.ath10k.data[i] << result->sample.ath10k.header.max_exp;
				if (data == 0)
					data = 1;
				signal = result->sample.ath10k.header.noise + result->sample.ath10k.header.rssi +
					 20 * fast_log10(data) - fast_log10(datasquaresum) * 10;

				insert(b, bins, freq, signal);
			}

		} break;
		case ATH_FFT_SAMPLE_ATH11K: {
			int datamax = 0, datamin = 65536;
			int datasquaresum = 0;
			unsigned short frequency;
			unsigned char width;
			int i;
			if (!rnum)
				fprintf(stdout,
					"\n{ \"tsf\": %" PRIu64
					", \"central_freq\": %d, \"rssi\": %d, \"noise\": %d, \"data\": [ \n",
					result->sample.ath11k.header.tsf, result->sample.ath11k.header.freq1,
					result->sample.ath11k.header.rssi, result->sample.ath11k.header.noise);

			bins = result->sample.tlv.length -
			       (sizeof(result->sample.ath11k.header) - sizeof(result->sample.ath11k.header.tlv));
			/* If freq2 is non zero and not equal to freq1 then the scan results are fragmented */
			if (result->sample.ath11k.header.freq2 &&
			    result->sample.ath11k.header.freq1 != result->sample.ath11k.header.freq2) {
				width = result->sample.ath11k.header.chan_width_mhz / 2;
				if (result->sample.ath11k.header.is_primary)
					frequency = result->sample.ath11k.header.freq1;
				else
					frequency = result->sample.ath11k.header.freq2;
			} else {
				frequency = result->sample.ath11k.header.freq1;
				width = result->sample.ath11k.header.chan_width_mhz;
			}
			for (i = 0; i < bins; i++) {
				int data;

				data = result->sample.ath11k.data[i];
				data *= data;
				datasquaresum += data;
				if (data > datamax)
					datamax = data;
				if (data < datamin)
					datamin = data;
			}
			if (!b)
				b = initbins(bins);

			for (i = 0; i < bins; i++) {
				int freq;
				int data;
				int signal;

				freq = frequency - width / 2 + ((width * ((i * 10) + 5) / bins) / 10);
				data = result->sample.ath11k.data[i];
				if (data == 0)
					data = 1;
				signal = result->sample.ath11k.header.noise + result->sample.ath11k.header.rssi +
					 20 * fast_log10(data) - fast_log10(datasquaresum) * 10;
				insert(b, bins, freq, signal);
			}
		} break;
		}

		rnum++;
	}
	if (b) {
		for (i = 0; i < bins; i++) {
			if (!notvalid(b[i].signal)) {
				fprintf(stdout, "[ %d, %d ]", b[i].freq, b[i].signal);
				if (i < (bins - 1))
					fprintf(stdout, ", ");
			}
		}
		fprintf(stdout, "\n");
		free(b);
	}
	fprintf(stdout, " ] }");
	fprintf(stdout, "\n]\n");

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
static int read_scandata(char *fname)
{
	char *pos, *scandata;
	size_t len, sample_len;
	struct scanresult *result;
	struct fft_sample_tlv *tlv;
	struct scanresult *tail = result_list;
	int handled, bins;

	scandata = read_file(fname, &len);
	if (!scandata)
		return -1;

	pos = scandata;

	while ((uintptr_t)(pos - scandata) < len) {
		tlv = (struct fft_sample_tlv *)pos;
		CONVERT_BE16(tlv->length);
		sample_len = sizeof(*tlv) + tlv->length;
		pos += sample_len;

		if (sample_len > sizeof(*result)) {
			//      fprintf(stderr, "sample length %zu too long\n", sample_len);
			continue;
		}

		result = malloc(sizeof(*result));
		if (!result)
			continue;

		memset(result, 0, sizeof(*result));
		memcpy(&result->sample, tlv, sample_len);

		handled = 0;
		switch (tlv->type) {
		case ATH_FFT_SAMPLE_HT20:
			if (sample_len != sizeof(result->sample.ht20)) {
				//      fprintf(stderr, "wrong sample length (have %zd, expected %zd)\n", sample_len, sizeof(result->sample));
				break;
			}

			CONVERT_BE16(result->sample.ht20.freq);
			CONVERT_BE16(result->sample.ht20.max_magnitude);
			CONVERT_BE64(result->sample.ht20.tsf);

			handled = 1;
			break;
		case ATH_FFT_SAMPLE_HT20_40:
			if (sample_len != sizeof(result->sample.ht40)) {
				//      fprintf(stderr, "wrong sample length (have %zd, expected %zd)\n", sample_len, sizeof(result->sample));
				break;
			}

			CONVERT_BE16(result->sample.ht40.freq);
			CONVERT_BE64(result->sample.ht40.tsf);
			CONVERT_BE16(result->sample.ht40.lower_max_magnitude);
			CONVERT_BE16(result->sample.ht40.upper_max_magnitude);

			handled = 1;
			break;
		case ATH_FFT_SAMPLE_ATH10K:
			bins = sample_len - sizeof(result->sample.ath10k.header);

			if (bins != 64 && bins != 128 && bins != 256 && bins != 512) {
				fprintf(stderr, "invalid bin length %d\n", bins);
				break;
			}

			/*
			 * Zero noise level should not happen in a real environment
			 * but some datasets contain it which creates bogus results.
			 */
			if (result->sample.ath10k.header.noise == 0)
				break;

			CONVERT_BE16(result->sample.ath10k.header.freq1);
			CONVERT_BE16(result->sample.ath10k.header.freq2);
			CONVERT_BE16(result->sample.ath10k.header.noise);
			CONVERT_BE16(result->sample.ath10k.header.max_magnitude);
			CONVERT_BE16(result->sample.ath10k.header.total_gain_db);
			CONVERT_BE16(result->sample.ath10k.header.base_pwr_db);
			CONVERT_BE64(result->sample.ath10k.header.tsf);

			handled = 1;
			break;
		case ATH_FFT_SAMPLE_ATH11K:
			if (sample_len < sizeof(result->sample.ath11k.header)) {
				fprintf(stderr, "wrong sample length (have %zd, expected at least %zd)\n", sample_len,
					sizeof(result->sample.ath11k.header));
				break;
			}

			bins = sample_len - sizeof(result->sample.ath11k.header);

			if (bins != 16 && bins != 32 && bins != 64 && bins != 128 && bins != 256 && bins != 512 && bins != 1024) {
				fprintf(stderr, "invalid bin length %d\n", bins);
				break;
			}

			/*
			 * Zero noise level should not happen in a real environment
			 * but some datasets contain it which creates bogus results.
			 */
			if (result->sample.ath11k.header.noise == 0)
				break;

			CONVERT_BE16(result->sample.ath11k.header.freq1);
			CONVERT_BE16(result->sample.ath11k.header.freq2);
			CONVERT_BE16(result->sample.ath11k.header.max_magnitude);
			CONVERT_BE16(result->sample.ath11k.header.rssi);
			CONVERT_BE32(result->sample.ath11k.header.tsf);
			CONVERT_BE32(result->sample.ath11k.header.noise);

			handled = 1;
			break;
		default:
			//      fprintf(stderr, "unknown sample type (%d)\n", tlv->type);
			break;
		}

		if (!handled) {
			free(result);
			continue;
		}

		if (tail)
			tail->next = result;
		else
			result_list = result;

		tail = result;

		scanresults_n++;
	}

	//      fprintf(stderr, "read %d scan results\n", scanresults_n);
	free(scandata);

	return 0;
}

void usage(int argc, char *argv[])
{
	printf("\n");
	printf("scanfile is generated by the spectral analyzer feature\n");
	printf("of your wifi card. If you have a AR92xx or AR93xx based\n");
	printf("card, try:\n");
	printf("\n");
	printf("ip link set dev wlan0 up\n");
	printf("echo chanscan > /sys/kernel/debug/ieee80211/phy0/ath9k/spectral_scan_ctl\n");
	printf("iw dev wlan0 scan\n");
	printf("cat /sys/kernel/debug/ieee80211/phy0/ath9k/spectral_scan0 > /tmp/fft_results\n");
	printf("echo disable > /sys/kernel/debug/ieee80211/phy0/ath9k/spectral_scan_ctl\n");
	printf("%s /tmp/fft_results\n", argv[0]);
	printf("\n");
	printf("for AR98xx based cards, you may use:\n");
	printf("ip link set dev wlan0 up\n");
	printf("echo background > /sys/kernel/debug/ieee80211/phy0/ath10k/spectral_scan_ctl\n");
	printf("echo trigger > /sys/kernel/debug/ieee80211/phy0/ath10k/spectral_scan_ctl\n");
	printf("iw dev wlan0 scan\n");
	printf("echo disable > /sys/kernel/debug/ieee80211/phy0/ath10k/spectral_scan_ctl\n");
	printf("cat /sys/kernel/debug/ieee80211/phy0/ath10k/spectral_scan0 > samples\n");
	printf("\n");
	printf("(NOTE: maybe debugfs must be mounted first: mount -t debugfs none /sys/kernel/debug/ )\n");
	printf("\n");
	printf("For ath11k based cards, use:\n");
	printf("ip link set dev wlan0 up\n");
	printf("echo background > /sys/kernel/debug/ieee80211/phy0/ath11k/spectral_scan_ctl\n");
	printf("echo trigger > /sys/kernel/debug/ieee80211/phy0/ath11k/spectral_scan_ctl\n");
	printf("iw dev wlan0 scan\n");
	printf("echo disable > /sys/kernel/debug/ieee80211/phy0/ath11k/spectral_scan_ctl\n");
	printf("cat /sys/kernel/debug/ieee80211/phy0/ath11k/spectral_scan0 > samples\n");
	printf("\n");
	printf("(NOTE: maybe debugfs must be mounted first: mount -t debugfs none /sys/kernel/debug/ )\n");
	printf("\n");
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		usage(argc, argv);
		return -1;
	}
	//      printf( "WARNING: Experimental Software! Don't trust anything you see. :)\n");
	//      printf( "\n");
	if (read_scandata(argv[1]) < 0) {
		printf("Couldn't read scanfile ...\n");
		usage(argc, argv);
		return -1;
	}
	print_values();

	return 0;
}
