/*
 * blkid.c - User command-line interface for libblkid
 *
 * Copyright (C) 2001 Andreas Dilger
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 * %End-Header%
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
extern char *optarg;
extern int optind;
#endif

#define OUTPUT_VALUE_ONLY	0x0001
#define OUTPUT_DEVICE_ONLY	0x0002

#include "blkid/blkid.h"

const char *progname = "blkid";

static void print_version(FILE *out)
{
	fprintf(out, "%s %s (%s)\n", progname, BLKID_VERSION, BLKID_DATE);
}

static void usage(int error)
{
	FILE *out = error ? stderr : stdout;

	print_version(out);
	fprintf(out,
		"usage:\t%s [-c <file>] [-hl] [-o format] "
		"[-s <tag>] [-t <token>]\n    [-v] [-w <file>] [dev ...]\n"
		"\t-c\tcache file (default: /etc/blkid.tab, /dev/null = none)\n"
		"\t-h\tprint this usage message and exit\n"
		"\t-s\tshow specified tag(s) (default show all tags)\n"
		"\t-t\tfind device with a specific token (NAME=value pair)\n"
		"\t-l\tlookup the the first device with arguments specified by -t\n"
		"\t-v\tprint version and exit\n"
		"\t-w\twrite cache to different file (/dev/null = no write)\n"
		"\tdev\tspecify device(s) to probe (default: all devices)\n",
		progname);
	exit(error);
}

static void print_tags(blkid_dev dev, char *show[], int numtag, int output)
{
	blkid_tag_iterate	iter;
	const char		*type, *value;
	int 			i, first = 1;

	if (!dev)
		return;

	if (output & OUTPUT_DEVICE_ONLY) {
		printf("%s\n", blkid_dev_devname(dev));
		return;
	}

	iter = blkid_tag_iterate_begin(dev);
	while (blkid_tag_next(iter, &type, &value) == 0) {
		if (numtag && show) {
			for (i=0; i < numtag; i++)
				if (!strcmp(type, show[i]))
					break;
			if (i >= numtag)
				continue;
		}
		if (first && !(output & OUTPUT_VALUE_ONLY)) {
			printf("%s: ", blkid_dev_devname(dev));
			first = 0;
		}
		if ((output & OUTPUT_VALUE_ONLY))
			printf("%s\n", value);
		else
			printf("%s=\"%s\" ", type, value);
	}
	blkid_tag_iterate_end(iter);

	if (!first && !(output & OUTPUT_VALUE_ONLY))
		printf("\n");
}

int main(int argc, char **argv)
{
	blkid_cache cache = NULL;
	char *devices[128] = { NULL, };
	char *show[128] = { NULL, };
	char *search_type = NULL, *search_value = NULL;
	char *read = NULL;
	char *write = NULL;
	unsigned int numdev = 0, numtag = 0;
	int version = 0;
	int err = 4;
	unsigned int i;
	int output_format = 0;
	int lookup = 0;
	int c;

	while ((c = getopt (argc, argv, "c:f:hlo:s:t:w:v")) != EOF)
		switch (c) {
		case 'c':
			if (optarg && !*optarg)
				read = NULL;
			else
				read = optarg;
			if (!write)
				write = read;
			break;
		case 'l':
			lookup++;
			break;
		case 'o':
			if (!strcmp(optarg, "value"))
				output_format = OUTPUT_VALUE_ONLY;
			else if (!strcmp(optarg, "device"))
				output_format = OUTPUT_DEVICE_ONLY;
			else if (!strcmp(optarg, "full"))
				output_format = 0;
			else {
				fprintf(stderr, "Invalid output format %s.  Chose from value, device, or full\n", optarg);
				exit(1);
			}
			break;
		case 's':
			if (numtag >= sizeof(show) / sizeof(*show)) {
				fprintf(stderr, "Too many tags specified\n");
				usage(err);
			}
			show[numtag++] = optarg;
			break;
		case 't':
			if (search_type) {
				fprintf(stderr, "Can only search for "
						"one NAME=value pair\n");
				usage(err);
			}
			if (blkid_parse_tag_string(optarg,
						   &search_type,
						   &search_value)) {
				fprintf(stderr, "-t needs NAME=value pair\n");
				usage(err);
			}
			break;
		case 'v':
			version = 1;
			break;
		case 'w':
			if (optarg && !*optarg)
				write = NULL;
			else
				write = optarg;
			break;
		case 'h':
			err = 0;
		default:
			usage(err);
		}

	while (optind < argc)
		devices[numdev++] = argv[optind++];

	if (version) {
		print_version(stdout);
		goto exit;
	}

	if (blkid_get_cache(&cache, read) < 0)
		goto exit;

	err = 2;
	if (lookup) {
		blkid_dev dev;

		if (!search_type) {
			fprintf(stderr, "The lookup option requires a "
				"search type specified using -t\n");
			exit(1);
		}
		/* Load any additional devices not in the cache */
		for (i = 0; i < numdev; i++)
			blkid_get_dev(cache, devices[i], BLKID_DEV_NORMAL);

		if ((dev = blkid_find_dev_with_tag(cache, search_type,
						   search_value))) {
			print_tags(dev, show, numtag, output_format);
			err = 0;
		}
	/* If we didn't specify a single device, show all available devices */
	} else if (!numdev) {
		blkid_dev_iterate	iter;
		blkid_dev		dev;

		blkid_probe_all(cache);

		iter = blkid_dev_iterate_begin(cache);
		blkid_dev_set_search(iter, search_type, search_value);
		while (blkid_dev_next(iter, &dev) == 0) {
			dev = blkid_verify(cache, dev);
			if (!dev)
				continue;
			print_tags(dev, show, numtag, output_format);
			err = 0;
		}
		blkid_dev_iterate_end(iter);
	/* Add all specified devices to cache (optionally display tags) */
	} else for (i = 0; i < numdev; i++) {
		blkid_dev dev = blkid_get_dev(cache, devices[i],
						  BLKID_DEV_NORMAL);

		if (dev) {
			if (search_type && 
			    !blkid_dev_has_tag(dev, search_type, 
					       search_value))
				continue;
			print_tags(dev, show, numtag, output_format);
			err = 0;
		}
	}

exit:
	if (search_type)
		free(search_type);
	if (search_value)
		free(search_value);
	blkid_put_cache(cache);
	return err;
}
