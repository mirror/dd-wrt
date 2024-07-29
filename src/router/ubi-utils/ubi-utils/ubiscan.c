/*
 * Copyright (C) 2021 Diego Ismirlian
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * An utility to scan MTD devices.
 *
 * Author: Diego Ismirlian dismirlian (at) google's mail
 */

#define PROGRAM_NAME    "ubiscan"

#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <limits.h>

#include <mtd/ubi-media.h>
#include <libubi.h>
#include <libmtd.h>
#include <libscan.h>
#include "common.h"

#define MAX_BINS 50

/* The variables below are set by command line arguments */
struct args {
    int verbose;
    const char *node;
    int node_fd;
    int bin_thresholds[MAX_BINS - 1];
    int nbins;
};

static struct args args = {
    .verbose = 0,
    .nbins = 6,
    .bin_thresholds = {
        10,
        100,
        1000,
        10000,
        100000,
    },
};

static const char doc[] = PROGRAM_NAME " version " VERSION
        " - a tool to scan MTD devices";

static const char optionsstr[] =
"-h, -?, --help               print help message\n"
"-H, --histrogram=<list>      comma-separated list of bin thresholds\n"
"-v, --verbose                be verbose\n"
"-V, --version                print program version\n";

static const char usage[] =
"Usage: " PROGRAM_NAME " <MTD device node file name> "
"\t\t\t[--help] [--version] [--verbose] [--histogram=<list>]";

static const struct option long_options[] = {
    { .name = "help",            .has_arg = 0, .flag = NULL, .val = 'h' },
    { .name = "histogram",       .has_arg = 1, .flag = NULL, .val = 'H' },
    { .name = "verbose",         .has_arg = 0, .flag = NULL, .val = 'v' },
    { .name = "version",         .has_arg = 0, .flag = NULL, .val = 'V' },
    { NULL, 0, NULL, 0},
};

static int parse_opt(int argc, char * const argv[])
{
    int last_bin = 0;
    while (1) {
        int key;

        key = getopt_long(argc, argv, "h?VvH:", long_options, NULL);
        if (key == -1)
            break;

        switch (key) {
        case 'v':
            args.verbose = 1;
            break;
        case 'H': {
            args.nbins = 1;
            char *token = strtok(optarg, ",");
            while (token) {
                if (args.nbins == MAX_BINS)
                    return errmsg("too many bins");
                int th = atoi(token);
                if (th <= last_bin)
                    return errmsg("bad bin threshold list");
                args.bin_thresholds[args.nbins - 1] = th;
                last_bin = th;
                args.nbins++;
                token = strtok(NULL, ",");
            }
        }   break;
        case 'V':
            common_print_version();
            exit(EXIT_SUCCESS);
        case 'h':
            printf("%s\n\n", doc);
            printf("%s\n\n", usage);
            printf("%s\n", optionsstr);
            exit(EXIT_SUCCESS);
        case '?':
            printf("%s\n\n", doc);
            printf("%s\n\n", usage);
            printf("%s\n", optionsstr);
            return -1;

        case ':':
            return errmsg("parameter is missing");

        default:
            fprintf(stderr, "Use -h for help\n");
            return -1;
        }
    }

    if (optind == argc)
        return errmsg("MTD device name was not specified (use -h for help)");
    else if (optind != argc - 1)
        return errmsg("more then one MTD device specified (use -h for help)");

    args.node = argv[optind];
    return 0;
}

int main(int argc, char * const argv[])
{
    int err;
    libmtd_t libmtd;
    struct mtd_info mtd_info;
    struct mtd_dev_info mtd;
    struct ubi_scan_info *si;
    int max, min;

    struct {
        int min;
        int max;
        int cnt;
        uint64_t mean;
    } bins[MAX_BINS];

    err = parse_opt(argc, argv);
    if (err)
        return -1;

    libmtd = libmtd_open();
    if (!libmtd)
        return errmsg("MTD subsystem is not present");

    err = mtd_get_info(libmtd, &mtd_info);
    if (err) {
        sys_errmsg("cannot get MTD information");
        goto out_close_mtd;
    }

    err = mtd_get_dev_info(libmtd, args.node, &mtd);
    if (err) {
        sys_errmsg("cannot get information about \"%s\"", args.node);
        goto out_close_mtd;
    }

    args.node_fd = open(args.node, O_RDONLY);
    if (args.node_fd == -1) {
        sys_errmsg("cannot open \"%s\"", args.node);
        goto out_close_mtd;
    }

    printf("Summary\n");
    printf("=========================================================\n");
    printf("mtd    : %d\n", mtd.mtd_num);
    printf("type   : %s\n", mtd.type_str);
    printf("size   : ");
    util_print_bytes(mtd.size, 1);
    printf("\n");
    printf("PEBs   : %d\n", mtd.eb_cnt);
    printf("min I/O: %d bytes\n", mtd.min_io_size);

    printf("\n");
    printf("PEB erase counters\n");
    printf("=========================================================\n");
    err = ubi_scan(&mtd, args.node_fd, &si, 0);
    if (err) {
        errmsg("failed to scan mtd%d (%s)", mtd.mtd_num, args.node);
        goto out_close;
    }

    memset(bins, 0, sizeof(bins));

    for (int j = 0; j < args.nbins; j++)
        bins[j].min = INT_MAX;

    min = INT_MAX;
    max = 0;

    for (int eb = 0; eb < mtd.eb_cnt; eb++) {
        uint32_t ec = si->ec[eb];
        switch (ec) {
        case EB_EMPTY:
        case EB_CORRUPTED:
        case EB_ALIEN:
        case EB_BAD:
        case EC_MAX:
            break;
        default: {
            int bin = 0;

            if (ec > max)
                max = ec;
            if (ec < min)
                min = ec;

            for (int j = 0; j < args.nbins - 1 && ec >= args.bin_thresholds[j]; j++, bin++);

            bins[bin].cnt++;
            bins[bin].mean += ec;
            if (ec < bins[bin].min)
                bins[bin].min = ec;
            if (ec > bins[bin].max)
                bins[bin].max = ec;

            } break;
        }
    }

    printf("valid    : %d\n", si->ok_cnt);
    printf("empty    : %d\n", si->empty_cnt);
    printf("corrupted: %d\n", si->corrupted_cnt);
    printf("alien    : %d\n", si->alien_cnt);
    printf("bad      : %d\n", si->bad_cnt);

    if (si->ok_cnt == 0)
        min = 0;

    printf("\n");
    printf("Histogram\n");
    printf("=========================================================\n");
    printf("from              to     count      min      avg      max\n");
    printf("---------------------------------------------------------\n");
    for (int j = 0; j < args.nbins; j++) {
        if (bins[j].cnt)
            bins[j].mean /= bins[j].cnt;
        else
            bins[j].min = 0;

        int from = (j == 0) ? 0 : args.bin_thresholds[j - 1];
        if (j == args.nbins - 1)
            printf("%-8d ..      inf: %8d %8d %8" PRIu64 " %8d\n",
                from, bins[j].cnt, bins[j].min, bins[j].mean, bins[j].max);
        else
            printf("%-8d .. %8d: %8d %8d %8" PRIu64 " %8d\n",
                from, args.bin_thresholds[j] - 1,
                bins[j].cnt, bins[j].min, bins[j].mean, bins[j].max);
    }
    printf("---------------------------------------------------------\n");
    printf("Total               : %8d %8d %8llu %8d\n", si->ok_cnt, min, si->mean_ec, max);

    if (args.verbose) {
        printf("\n");
        printf("Details\n");
        printf("=========================================================\n");
        for (int eb = 0; eb < mtd.eb_cnt; eb++) {
            printf("PEB %8d: ", eb);
            uint32_t ec = si->ec[eb];
            switch (ec) {
            case EB_EMPTY:
                printf("EB_EMPTY\n");
                break;
            case EB_CORRUPTED:
                printf("EB_CORRUPTED\n");
                break;
            case EB_ALIEN:
                printf("EB_ALIEN\n");
                break;
            case EB_BAD:
                printf("EB_BAD\n");
                break;
            case EC_MAX:
                printf("EC_MAX\n");
                break;
            default:
                printf("%u\n", ec);
                break;
            }
        }
    }

    ubi_scan_free(si);
    close(args.node_fd);
    libmtd_close(libmtd);
    return 0;

out_close:
    close(args.node_fd);
out_close_mtd:
    libmtd_close(libmtd);
    return -1;
}

