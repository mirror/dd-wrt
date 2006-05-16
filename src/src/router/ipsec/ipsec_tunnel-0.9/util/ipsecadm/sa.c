/*
 *  Copyright 2002 Tobias Ringstrom <tobias@ringstrom.mine.nu>
 *  Authentication Copyright 2002 Arcturus Networks Inc.
 *      by Norman Shulman <norm@arcturusnetworks.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#include "ipsecadm.h"

#define MAX(a,b)  ((b) > (a) ? (b) : (a))

struct ipsec_sa_parm
{
	uint32_t    version;

	uint32_t    dst;
	uint32_t    src;
	uint32_t    spi;
	uint32_t    flags;

	char        cipher[IPSEC_SA_CRYPTOLEN];
	int         cipher_keylen;
	const void *cipher_key;
	char        digest[IPSEC_SA_CRYPTOLEN];
	int         digest_keylen;
	const void *digest_key;
	int         digest_hmaclen;
};

static void
sa_usage(void)
{
	fputs("Usage:\n", stderr);
	fputs("    ipsecadm sa help\n", stderr);
	fputs("    ipsecadm sa add --dst=<dst> --src=<src> --spi=<spi>\n", stderr);
	fputs("                    [--cipher=<cipher>]\n", stderr);
	fputs("                    [--cipher-key=<key>]\n", stderr);
	fputs("                    [--cipher-keyfile=<file>]\n", stderr);
	fputs("                    [--digest=<digest>]\n", stderr);
	fputs("                    [--digest-hmac=<size>]\n", stderr);
	fputs("                    [--digest-key=<key>]\n", stderr);
	fputs("                    [--digest-keyfile=<file>]\n", stderr);
	fputs("                    [--duplex]\n", stderr);
	fputs("    ipsecadm sa del [--dst=<dst>] [--src=<src>] [--spi=<spi>]\n", stderr);
	fputs("                    [--duplex] [--all]\n", stderr);
	fputs("    ipsecadm sa show [--dst=<dst>] [--src=<src>] [--spi=<spi>]\n", stderr);

	exit(1);
}

static const char*
ipsec_sa_add_error(int err)
{
	switch (err)
	{
	case EINVAL:
		return "Key too long";
	case EEXIST:
		return "SA already exists";
	case ENOENT:
		return "Unknown cipher or digest";
	case EPROTO:
		return "Bad key";
	}

	return strerror(err);
}

static int
ipsec_sa_add(uint32_t dst,
			 uint32_t src,
			 uint32_t spi,
			 const char *cipher,
			 const void *cipher_key,
			 int cipher_keylen,
			 const char *digest,
			 const void *digest_key,
			 int digest_keylen,
			 int digest_hmaclen)
{
	struct ipsec_sa_parm sa;
	struct ifreq ifr;
	int fd, st;

	fd = ipsec_tunnel_open(IPSECDEVNAME, &ifr, 0);
	if (fd == -1)
		return -1;

	memset(&sa, 0, sizeof(struct ipsec_sa_parm));
	ifr.ifr_data = (char*)&sa;

	sa.version = IPSEC_SA_VERSION;
	sa.dst = dst;
	sa.src = src;
	sa.spi = spi;

	if (cipher) {
	    strncpy(sa.cipher, cipher, sizeof(sa.cipher));
	    sa.cipher_key = cipher_key;
	    sa.cipher_keylen = cipher_keylen;
	}

	if (digest) {
		strncpy(sa.digest, digest, sizeof(sa.digest));
	    sa.digest_key = digest_key;
	    sa.digest_keylen = digest_keylen;
		sa.digest_hmaclen = digest_hmaclen;
	}

	st = ioctl(fd, SIOCIPSEC_ADD_SA, &ifr);
	if (st != 0)
		error("Cannot add SA! [%s]", ipsec_sa_add_error(errno));

	close(fd);

	return 0;
}

static int
ipsec_sa_del(uint32_t dst, uint32_t src, uint32_t spi)
{
	struct ipsec_sa_parm sa;
	struct ifreq ifr;
	int fd, st;

	fd = ipsec_tunnel_open(IPSECDEVNAME, &ifr, 0);
	if (fd == -1)
		return -1;

	memset(&sa, 0, sizeof(struct ipsec_sa_parm));
	ifr.ifr_data = (char*)&sa;

	sa.version = IPSEC_SA_VERSION;
	sa.dst = dst;
	sa.src = src;
	sa.spi = spi;

	st = ioctl(fd, SIOCIPSEC_DEL_SA, &ifr);
	if (st < 0 && !(errno == ENOENT && dst == INADDR_ANY &&
					src == INADDR_ANY && spi == IPSEC_SPI_ANY))
	{
		error("Cannot delete SA [%s]",
			  errno == ENOENT ? "No matching SA" : strerror(errno));
	}

	close(fd);

	return 0;
}

static void
ipsec_sa_show_hdr(void)
{
	puts("Destination     Source          SPI        Cipher/bits        Digest-HMAC/bits");
	puts("===============================================================================");
}

void
ipsec_sa_show_one(const struct ipsec_sa_parm *sa)
{
	char str[100];

	printf("%-15s ", ipv4_ntoa(sa->dst));
	printf("%-15s ", ipv4_ntoa(sa->src));
	printf("0x%08x ", sa->spi);
	if (sa->cipher[0] == '\0')
		strcpy(str, "-");
	else
		sprintf(str, "%s/%d", sa->cipher, 8 * sa->cipher_keylen);
	printf("%-18s ", str);

	if (sa->digest[0] == '\0')
		printf("-");
	else
		printf("%s-%d/%d", sa->digest, 8 * sa->digest_hmaclen, 8 * sa->digest_keylen);

	putchar('\n');
}

static int
ipsec_sa_show(uint32_t dst, uint32_t src, uint32_t spi)
{
	struct ipsec_sa_parm sa;
	struct ifreq ifr;
	int fd, st, i, n = 0;

	fd = ipsec_tunnel_open(IPSECDEVNAME, &ifr, 0);
	if (fd == -1)
		return -1;

	ipsec_sa_show_hdr();

	for (i = 0; ; ++i)
	{
		memset(&sa, 0, sizeof(struct ipsec_sa_parm));
		ifr.ifr_data = (char*)&sa;

		sa.version = IPSEC_SA_VERSION;
		sa.dst = INADDR_ANY;
		sa.src = INADDR_ANY;
		sa.spi = i;

		st = ioctl(fd, SIOCIPSEC_GET_SA, &ifr);
		if (st < 0)
		{
			if (errno == ENOENT)
				break;
			error("Cannot get IPsec SA!");
		}

		if ((dst == INADDR_ANY || dst == sa.dst) &&
			(src == INADDR_ANY || src == sa.src) &&
			(spi == IPSEC_SPI_ANY || spi == sa.spi))
		{
			ipsec_sa_show_one(&sa);
			++n;
		}
	}
	close(fd);
	if (n == 0 && (dst != INADDR_ANY || src != INADDR_ANY || spi != IPSEC_SPI_ANY))
		error("No such SA!");

	return 0;
}

static int
sa_add(int argc, char *argv[])
{
	uint32_t dst = 0, src = 0, spi = 0;
	char *cipher = NULL, cipher_key[IPSEC_SA_CRYPTOLEN];
	int cipher_keylen = -1;
	char *digest = NULL, digest_key[IPSEC_SA_CRYPTOLEN];
	int digest_keylen = -1, digest_hmacsize = 96;
	int duplex = 0, c;
	int got_dst = 0, got_src = 0, got_spi = 0;
	int got_cipher = 0, got_cipher_key = 0;
	int got_digest = 0, got_digest_key = 0;

	static struct option opts[] = {
		{ "dst",              1, 0, 'd' },
		{ "src",              1, 0, 's' },
		{ "spi",              1, 0, 'S' },
		{ "cipher",           1, 0, 'c' },
		{ "cipher-key",       1, 0, 'k' },
		{ "cipher-keyfile",   1, 0, 'f' },
		{ "digest",           1, 0, 'h' },
		{ "digest-key",       1, 0, 'K' },
		{ "digest-keyfile",   1, 0, 'F' },
		{ "digest-hmac",      1, 0, 'H' },
		{ "duplex",           0, 0, 'D' },
		{0, 0, 0, 0}
	};

	for (;;)
	{
		c = getopt_long(argc, argv, "", opts, NULL);
		if (c == -1)
			break;

		switch (c)
		{
		case 'd':
			dst = ipv4_aton(optarg);
			if (dst == INADDR_ANY)
				error("Invalid destination address!");
			got_dst = 1;
			break;
		case 's':
			src = ipv4_aton(optarg);
			if (src == INADDR_ANY)
				error("Invalid source address!");
			got_src = 1;
			break;
		case 'S':
			spi = strtospi(optarg);
			if (spi == IPSEC_SPI_ANY || (spi >= 1 && spi <= 255))
				error("Invalid SPI!");
			got_spi = 1;
			break;
		case 'c':
			if (got_cipher)
				error("More than one cipher!");
			cipher = optarg;
			got_cipher = 1;
			break;
		case 'k':
			if (got_cipher_key)
				error("More than one cipher key!");
			cipher_keylen = parse_key(optarg, cipher_key, sizeof(cipher_key));
			if (cipher_keylen <= 0)
				error("Bad cipher key!");
			got_cipher_key = 1;
			break;
		case 'f':
			if (got_cipher_key)
				error("More than one cipher key!");
			cipher_keylen = read_key_file(optarg, cipher_key, sizeof(cipher_key));
			if (cipher_keylen <= 0)
				error("Bad cipher key!");
			got_cipher_key = 1;
			break;
		case 'h':
			if (got_digest)
				error("More than one digest!");
			digest = optarg;
			got_digest = 1;
			break;
		case 'K':
			if (got_digest_key)
				error("More than one digest key!");
			digest_keylen =
			    parse_key(optarg, digest_key, sizeof(digest_key));
			if (digest_keylen <= 0)
				error("Bad digest key!");
			got_digest_key = 1;
			break;
		case 'F':
			if (got_digest_key)
				error("More than one digest key!");
			digest_keylen =
			    read_key_file(optarg, digest_key, sizeof(digest_key));
			if (digest_keylen <= 0)
				error("Bad digest key!");
			got_digest_key = 1;
			break;
		case 'H':
			digest_hmacsize = strtospi(optarg);
			if (digest_hmacsize < 4 || digest_hmacsize % 32 != 0)
				error("Invalid HMAC size!");
			break;
		case 'D':
			duplex = 1;
			break;
		case '?':
			sa_usage();
			break;
		default:
			printf("default: %d %c\n", c, c);
			break;
		}
	}

	if (optind != argc)
		sa_usage();
	if (!got_dst || !got_src)
		error("Destination and source addresses must be specified!");
	if (!got_spi)
		error("SPI must be specified!");
	if (!got_cipher && !got_digest)
		error("Cipher and/or digest must be specified!");
	if (got_cipher && !got_cipher_key)
		error("Cipher key must be specified!");
	if (got_digest && !got_digest_key)
		error("Digest key must be specified!");
	if (got_digest && digest_hmacsize > 8 * digest_keylen)
		error("Invalid HMAC size!");

	if (got_digest && 8 * digest_keylen < 80)
		printf("Warning: Weak digest key; it should be at least 80 bits!\n");
	if (got_digest && digest_hmacsize < MAX(8 * digest_keylen / 2, 80))
	{
		printf("Warning: Weak HMAC size; it should be at least %d bits!\n",
			   MAX(8 * digest_keylen / 2, 80));
	}

	if (duplex)
	{
		int ret1, ret2;
		ret1 = ipsec_sa_add(dst, src, spi,
							cipher, cipher_key, cipher_keylen,
							digest, digest_key, digest_keylen,
							digest_hmacsize / 8);
		ret2 = ipsec_sa_add(src, dst, spi,
							cipher, cipher_key, cipher_keylen,
							digest, digest_key, digest_keylen,
							digest_hmacsize / 8);
		return ret1 ? ret1 : ret2;
	}
	else
	{
		return ipsec_sa_add(dst, src, spi,
							cipher, cipher_key, cipher_keylen,
							digest, digest_key, digest_keylen,
							digest_hmacsize / 8);
	}
}

static int
sa_del(int argc, char *argv[])
{
	uint32_t dst = 0, src = 0, spi = 0, duplex = 0, all = 0;
	int c, need_all = 1;

	static struct option opts[] = {
		{ "dst",              1, 0, 'd' },
		{ "src",              1, 0, 's' },
		{ "spi",              1, 0, 'S' },
		{ "duplex",           0, 0, 'D' },
		{ "all",              0, 0, 'a' },
		{0, 0, 0, 0}
	};

	for (;;)
	{
		c = getopt_long(argc, argv, "", opts, NULL);
		if (c == -1)
			break;

		switch (c)
		{
		case 'd':
			dst = ipv4_aton(optarg);
			need_all = 0;
			break;
		case 's':
			src = ipv4_aton(optarg);
			need_all = 0;
			break;
		case 'S':
			spi = strtospi(optarg);
			need_all = 0;
			break;
		case 'D':
			duplex = 1;
			break;
		case 'a':
			all = 1;
			break;
		case '?':
			sa_usage();
			break;
		default:
			printf("default: %d %c\n", c, c);
			break;
		}
	}

	if (optind != argc)
		sa_usage();

	if (all && (dst != INADDR_ANY || src != INADDR_ANY || spi != IPSEC_SPI_ANY))
		error("Option --all conficts with dst, src, and/or spi!");
	if (need_all && !all)
		error("Specify --all to delete all SAs!");
	if (duplex && (dst == INADDR_ANY || src == INADDR_ANY))
		error("Duplex without unicast source and destination addresses!");

	if (duplex)
	{
		int ret1, ret2;
		ret1 = ipsec_sa_del(dst, src, spi);
		ret2 = ipsec_sa_del(src, dst, spi);
		return ret1 ? ret1 : ret2;
	}
	return ipsec_sa_del(dst, src, spi);
}

static int
sa_show(int argc, char *argv[])
{
	uint32_t dst = 0, src = 0, spi = 0;
	int c;

	static struct option opts[] = {
		{ "dst",              1, 0, 'd' },
		{ "src",              1, 0, 's' },
		{ "spi",              1, 0, 'S' },
		{0, 0, 0, 0}
	};

	for (;;)
	{
		c = getopt_long(argc, argv, "", opts, NULL);
		if (c == -1)
			break;

		switch (c)
		{
		case 'd':
			dst = ipv4_aton(optarg);
			break;
		case 's':
			src = ipv4_aton(optarg);
			break;
		case 'S':
			spi = strtospi(optarg);
			break;
		case '?':
			sa_usage();
			break;
		default:
			printf("default: %d %c\n", c, c);
			break;
		}
	}

	if (optind != argc)
		sa_usage();

	return ipsec_sa_show(dst, src, spi);
}

int
sa_main(int argc, char *argv[])
{
	const char *modes[] = { "help", "usage", "add", "del", "show", NULL };
	int mode;

	if (argc < 2)
		sa_usage();

	mode = find_unambiguous_string(modes, argv[1]);
	switch (mode)
	{
	case 0:
	case 1:
		sa_usage();
		return 0;
	case 2:
		return sa_add(argc - 1, argv + 1);
	case 3:
		return sa_del(argc - 1, argv + 1);
	case 4:
		return sa_show(argc - 1, argv + 1);
	}

	sa_usage();

	return 1;
}
