/*
 * upgrade.c
 *
 * Copyright (C) 2005 - 2021 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <broadcom.h>
#include <dd_defs.h>
#include <shutils.h>
#include <byteswap.h>
#include <endian.h> /* for __BYTE_ORDER */
#include <utils.h>

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define HOST_TO_BE32(x) bswap_32(x)
#define HOST_TO_BE16(x) bswap_16(x)
#else
#define HOST_TO_BE32(x) (x)
#define HOST_TO_BE16(x) (x)
#endif

#ifndef HAVE_MICRO
#define MIN_BUF_SIZE 65536
#else
#define MIN_BUF_SIZE 4096
#endif
#define CODE_PATTERN_ERROR 9999
static void set_upgrade_ret(webs_t stream, int result)
{
	if (result != 0) {
		stream->upgrade_ret = result;
	} else {
		stream->upgrade_ret = 0;
	}
}

static int do_upgrade_cgi(unsigned char method, struct mime_handler *handler,
			  char *url, webs_t stream) // jimmy, https,
	// 8/6/2003
{
	int ret;
#ifndef ANTI_FLASH

	if (stream->upgrade_ret) {
		ret = do_ej(METHOD_GET, handler, "Fail_u_s.asp", stream);
		killall("ledtool", SIGTERM);
		led_control(LED_DIAG, LED_OFF);
	} else {
		ret = do_ej(METHOD_GET, handler, "Success_u_s.asp", stream);
	}
	if (ret)
		return ret;
	websDone(stream, 200);

	/*
	 * Reboot if successful 
	 */
	if (stream->upgrade_ret == 0) {
		// sleep (10);
		nvram_set("shutdown", "fast");
		sleep(1);
		sys_reboot();
		sleep(10);
		writeproc("/proc/sysrq-trigger", "b");
	}
#else

	ret = do_ej(METHOD_GET, handler, "Fail_u_s.asp", stream);
	if (ret)
		return ret;
	websDone(stream, 200);

#endif
	return 0;
}

typedef struct {
	char magic[8]; // "FIRMWARE"
	char ver_inter[4];
	char ver_outer[4];
	char model[24];
	unsigned int len;
	unsigned int crc32;
} ralink_firmware_header;

static char *allmagics[] = { CODE_PATTERN_WRT54G,     CODE_PATTERN_WRT54GS,
			     CODE_PATTERN_WRH54G,     CODE_PATTERN_WRT150N,
			     CODE_PATTERN_WRT160N,    CODE_PATTERN_WRT300N,
			     CODE_PATTERN_WRT300NV11, CODE_PATTERN_WRT310N,
			     CODE_PATTERN_WRT350N,    CODE_PATTERN_WRTSL54GS,
			     CODE_PATTERN_WRT54G3G,   CODE_PATTERN_WRT54G3GV,
			     CODE_PATTERN_WRT610N,    CODE_PATTERN_WRT54GSV4,
			     CODE_PATTERN_WRT320N,    CODE_PATTERN_VALET_M10,
			     CODE_PATTERN_VALET_M20,  CODE_PATTERN_E900,
			     CODE_PATTERN_E800,	      CODE_PATTERN_E1000,
			     CODE_PATTERN_E1200V1,    CODE_PATTERN_E1200V2,
			     CODE_PATTERN_E1500,      CODE_PATTERN_E1550,
			     CODE_PATTERN_E2000,      CODE_PATTERN_E2500,
			     CODE_PATTERN_E3000,      CODE_PATTERN_E3200,
			     CODE_PATTERN_E4200,      CODE_PATTERN_NV60K,
			     CODE_PATTERN_NV64K,      NULL };

static char *nv60k[] = { CODE_PATTERN_E1550, CODE_PATTERN_E2000,
			 CODE_PATTERN_E2500, CODE_PATTERN_E3000,
			 CODE_PATTERN_E3200, CODE_PATTERN_E4200,
			 CODE_PATTERN_NV60K, NULL };

static char *nv64k[] = { CODE_PATTERN_E900,
			 CODE_PATTERN_E800,
			 CODE_PATTERN_E1200V1,
			 CODE_PATTERN_E1200V2,
			 CODE_PATTERN_E1500,
			 CODE_PATTERN_NV64K,
			 NULL };

static int checkmagic(char *magic, char *check[])
{
	int cnt = 0;
	while (check[cnt]) {
		if (!memcmp(magic, check[cnt], 4))
			return 0;
		cnt++;
	}
	return -1;
}

static int
// sys_upgrade(char *url, FILE *stream, int *total)
sys_upgrade(char *url, webs_t stream, size_t *total, int type) // jimmy,
	// https,
	// 8/6/2003
{
	int brand = getRouterBrand();

#ifndef ANTI_FLASH
#ifdef HAVE_VENTANA
#define HAVE_VENTANA_NEW_UPGRADE
#endif
#ifdef HAVE_VENTANA_NEW_UPGRADE
	char upload_fifo[] = "/tmp/new_root/tmp/uploadXXXXXX";
#else
	char upload_fifo[] = "/tmp/uploadXXXXXX";
#endif
	FILE *fifo = NULL;
	FILE *fifo2 = NULL;
	char *write_argv[4];
	pid_t pid = 0;
	char *buf = NULL;
	int count, ret = 0;
	long flags = -1;
	int size = BUFSIZ;
	int i = 0;
#if (defined(HAVE_FONERA) || defined(HAVE_WHRAG108) || defined(HAVE_MERAKI) || \
     defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) ||        \
     defined(HAVE_USR5453)) &&                                                 \
	(!defined(HAVE_DIR400) && !defined(HAVE_WRT54G2) &&                    \
	 !defined(HAVE_GWMF54G2) && !defined(HAVE_FONERA2200) &&               \
	 !defined(HAVE_MR3202A) && !defined(HAVE_CA8PRO) &&                    \
	 !defined(HAVE_CA8) && !defined(HAVE_RT2880) && !defined(HAVE_LS2) &&  \
	 !defined(HAVE_WRK54G) && !defined(HAVE_ADM5120) &&                    \
	 !defined(HAVE_DIR300) && !defined(HAVE_DLM101) &&                     \
	 !defined(HAVE_MERAKI) && !defined(HAVE_SOLO51) &&                     \
	 !defined(HAVE_RTG32) && !defined(HAVE_EOC5610) &&                     \
	 !defined(HAVE_NP25G))
#define WRITEPART "rootfs"
#else
#define WRITEPART "linux"
#endif

	write_argv[0] = "write";
	write_argv[1] = upload_fifo;
	write_argv[2] = WRITEPART;
	write_argv[3] = NULL;
	FILE *check = fopen("/usr/sbin/fischecksum", "rb");
	if (check) {
		fclose(check);
		eval("fischecksum");
	}
	if (url)
		return eval("write", url, WRITEPART);
	// diag_led(DIAG, START_LED); // blink the diag led
	if (DO_SSL(stream))
		ACTION("ACT_WEBS_UPGRADE");
	else
		ACTION("ACT_WEB_UPGRADE");
	int uploadcount = 0;

	/*
	 * Set nonblock on the socket so we can timeout 
	 */
	if (!DO_SSL(stream)) {
		if ((flags = fcntl(fileno(stream->fp_in), F_GETFL)) < 0 ||
		    fcntl(fileno(stream->fp_in), F_SETFL, flags | O_NONBLOCK) <
			    0) {
			ret = errno;
			goto err;
		}
	}

	/*
	 ** The buffer must be at least as big as what the stream file is
	 ** using so that it can read all the data that has been buffered
	 ** in the stream file. Otherwise it would be out of sync with fn
	 ** select specially at the end of the data stream in which case
	 ** the select tells there is no more data available but there in
	 ** fact is data buffered in the stream file's buffer. Since no
	 ** one has changed the default stream file's buffer size, let's
	 ** use the constant BUFSIZ until someone changes it.
	 **/

	if (size < MIN_BUF_SIZE)
		size = MIN_BUF_SIZE;
	fprintf(stderr, "use buffer size %d\n", size);
	if ((buf = safe_malloc(size)) == NULL) {
		ret = ENOMEM;
		goto err;
	}
	eval("ledtool", "500");
	/*
	 * Pipe the rest to the FIFO 
	 */
	cprintf("Upgrading\n");
	int lastblock = 0;
	while (total && *total) {
		if (DO_SSL(stream)) {
			if (size > *total)
				size = *total;
			count = wfread(buf, 1, size, stream);
		} else {
			if (waitfor(fileno(stream->fp_in), 5) <= 0) {
				lastblock = 1;
			}
			count = safe_fread(buf, 1, size, stream->fp_in);
			if (!count &&
			    (ferror(stream->fp_in) || feof(stream->fp_in))) {
				break;
			}
		}

		if (i == 0) { // check code pattern, the first data must
#ifdef HAVE_VENTANA
#ifdef HAVE_VENTANA_NEW_UPGRADE
			if (!strncmp(buf, "HDR0", 4)) { // check for "trx"
				char *write_argv_buf[8];
				eval("mkdir", "-p", "/tmp/new_root");
				eval("mount", "-n", "-t", "tmpfs", "none",
				     "/tmp/new_root");
				eval("mkdir", "-p", "/tmp/new_root/tmp");
				write_argv_buf[0] = "update-prepare.sh";
				write_argv_buf[1] = upload_fifo;
				write_argv_buf[2] = "rootfs";
				write_argv_buf[3] = "nomount";
				write_argv_buf[4] = "noreboot";
				write_argv_buf[5] = NULL;
				if (!mktemp(upload_fifo) ||
				    mkfifo(upload_fifo, S_IRWXU) < 0 ||
				    (ret = _evalpid(write_argv_buf, NULL, 0,
						    &pid)) ||
				    !(fifo = fopen(upload_fifo, "w"))) {
					if (!ret)
						ret = errno;
					goto err;
				}
				goto write_data;
			}
#else
			if (!strncmp(buf, "UBI#", 4)) { // check for "UBI#"
				eval("mount", "-o", "remount,ro", "/");
				char *write_argv_buf[8];
				fprintf(stderr, "erase nandflash\n");
				eval("mtd", "erase", "rootfs");
				struct timespec tim, tim2;
				tim.tv_sec = 10;
				tim.tv_nsec = 0;
				nanosleep(&tim, &tim2);
				write_argv_buf[0] = "mtd";
				write_argv_buf[1] = "-f";
				write_argv_buf[2] = "write";
				write_argv_buf[3] = upload_fifo;
				write_argv_buf[4] = "rootfs";
				write_argv_buf[5] = NULL;
				if (!mktemp(upload_fifo) ||
				    mkfifo(upload_fifo, S_IRWXU) < 0 ||
				    (ret = _evalpid(write_argv_buf, NULL, 0,
						    &pid)) ||
				    !(fifo = fopen(upload_fifo, "w"))) {
					if (!ret)
						ret = errno;
					goto err;
				}
				goto write_data;
			}
#endif
#endif
//0x1200000 0x70000 RN67
#ifdef HAVE_IPQ806X

#define _WEB_HEADER_ "RN67"
#define FW_HEADER ((char *)"CSYS")
#define DWORD_SWAP(v)                             \
	((((v & 0xff) << 24) & 0xff000000) |      \
	 ((((v >> 8) & 0xff) << 16) & 0xff0000) | \
	 ((((v >> 16) & 0xff) << 8) & 0xff00) | (((v >> 24) & 0xff) & 0xff))

			typedef struct img_header {
				unsigned char signature[4];
				unsigned int startAddr __attribute__((packed));
				unsigned int burnAddr __attribute__((packed));
				unsigned char modTag[4];
				unsigned int len __attribute__((packed));
			} __attribute__((packed)) img_header_t;
			if (brand == ROUTER_ASROCK_G10) {
				img_header_t *header = (img_header_t *)buf;
				if (!memcmp(header->signature, FW_HEADER, 4) &&
				    !memcmp(header->modTag, _WEB_HEADER_, 4)) {
					fprintf(stderr,
						"found valid ASROCK-G10 Image\n");
					eval("startservice", "bootprimary");
					eval("startservice", "finishupgrade");
					count -= sizeof(struct img_header);
					memcpy(buf,
					       buf + sizeof(struct img_header),
					       count);
					char *write_argv_buf[8];
					write_argv_buf[0] = "mtd";
					write_argv_buf[1] = "-e";
					write_argv_buf[2] = "linux";
					write_argv_buf[3] = "-f";
					write_argv_buf[4] = "write";
					write_argv_buf[5] = upload_fifo;
					write_argv_buf[6] = "linux";
					write_argv_buf[7] = NULL;
					if (!mktemp(upload_fifo) ||
					    mkfifo(upload_fifo, S_IRWXU) < 0 ||
					    (ret = _evalpid(write_argv_buf,
							    NULL, 0, &pid)) ||
					    !(fifo = fopen(upload_fifo, "w"))) {
						if (!ret)
							ret = errno;
						goto err;
					}
					fifo2 = fopen("/tmp/parttemp", "wb");
					goto write_data;
				}
			}
#endif
#if defined(HAVE_DIR860) || defined(HAVE_DIR859)
			if (brand == ROUTER_DIR882) {
				unsigned int *uboot_magic = (unsigned int *)buf;
				if (*uboot_magic == HOST_TO_BE32(0x27051956)) {
					char *write_argv_buf[8];
					write_argv_buf[0] = "mtd";
					write_argv_buf[1] = "-f";
					write_argv_buf[2] = "write";
					write_argv_buf[3] = upload_fifo;
					write_argv_buf[4] = "linux";
					write_argv_buf[5] = NULL;
					if (!mktemp(upload_fifo) ||
					    mkfifo(upload_fifo, S_IRWXU) < 0 ||
					    (ret = _evalpid(write_argv_buf,
							    NULL, 0, &pid)) ||
					    !(fifo = fopen(upload_fifo, "w"))) {
						if (!ret)
							ret = errno;
						goto err;
					}
					goto write_data;
				}

			} else {
#define SEAMA_MAGIC 0x5EA3A417

				typedef struct seama_hdr seamahdr_t;
				struct seama_hdr {
					unsigned int
						magic; /* should always be SEAMA_MAGIC. */
					unsigned short
						reserved; /* reserved for  */
					unsigned short
						metasize; /* size of the META data */
					unsigned int size; /* size of the image */
				} __attribute__((packed));
				seamahdr_t *seama = (seamahdr_t *)buf;

				if (seama->magic == HOST_TO_BE32(SEAMA_MAGIC)) {
					unsigned int skip =
						HOST_TO_BE16(seama->metasize) +
						sizeof(seamahdr_t);
					fprintf(stderr,
						"found seama header, skip seal header of %d bytes\n",
						skip);
					if (skip > count)
						goto err;
#ifdef HAVE_DIR869
#define signature "signature=wrgac54_dlink.2015_dir869"
#elif HAVE_DIR859
#define signature "signature=wrgac37_dlink.2013gui_dir859"
#else
#define signature "signature=wrgac13_dlink.2013gui_dir860lb"
#endif
					if (memcmp(buf + sizeof(seamahdr_t),
						   signature,
						   sizeof(signature))) {
						fprintf(stderr,
							"firmware signature must be %s\n",
							signature);
						goto err;
					}

					count -= skip;
					memcpy(buf, buf + skip, count);
					char *write_argv_buf[8];
					write_argv_buf[0] = "mtd";
					write_argv_buf[1] = "-f";
					write_argv_buf[2] = "write";
					write_argv_buf[3] = upload_fifo;
					write_argv_buf[4] = "linux";
					write_argv_buf[5] = NULL;
					if (!mktemp(upload_fifo) ||
					    mkfifo(upload_fifo, S_IRWXU) < 0 ||
					    (ret = _evalpid(write_argv_buf,
							    NULL, 0, &pid)) ||
					    !(fifo = fopen(upload_fifo, "w"))) {
						if (!ret)
							ret = errno;
						goto err;
					}
					goto write_data;
				}
			}
#endif
#if defined(HAVE_DIR862)
			unsigned int *uboot_magic = (unsigned int *)buf;
			if (*uboot_magic == HOST_TO_BE32(0x27051956)) {
				char *write_argv_buf[8];
				write_argv_buf[0] = "mtd";
				write_argv_buf[1] = "-f";
				write_argv_buf[2] = "write";
				write_argv_buf[3] = upload_fifo;
				write_argv_buf[4] = "linux";
				write_argv_buf[5] = NULL;
				if (!mktemp(upload_fifo) ||
				    mkfifo(upload_fifo, S_IRWXU) < 0 ||
				    (ret = _evalpid(write_argv_buf, NULL, 0,
						    &pid)) ||
				    !(fifo = fopen(upload_fifo, "w"))) {
					if (!ret)
						ret = errno;
					goto err;
				}
				goto write_data;
			}
#elif defined(HAVE_MVEBU)
			unsigned int *uboot_magic = (unsigned int *)buf;
			if (*uboot_magic == HOST_TO_BE32(0x27051956)) {
				char *write_argv_buf[8];
				write_argv_buf[0] = "mtd";
				write_argv_buf[1] = "-e";
				write_argv_buf[2] = "linux";
				write_argv_buf[3] = "-f";
				write_argv_buf[4] = "write";
				write_argv_buf[5] = upload_fifo;
				write_argv_buf[6] = "linux";
				write_argv_buf[7] = NULL;
				char *bootpart_argv_buf[5];
				bootpart_argv_buf[0] = "ubootenv";
				bootpart_argv_buf[1] = "set";
				bootpart_argv_buf[2] = "boot_part";
				bootpart_argv_buf[3] = "1";
				bootpart_argv_buf[4] = NULL;

				char *part = getUEnv("boot_part");
				if (part) {
					fprintf(stderr, "boot partiton is %s\n",
						part);
					if (!strcmp(part, "2")) {
					} else {
						write_argv_buf[2] = "linux2";
						write_argv_buf[6] = "linux2";
						bootpart_argv_buf[3] = "2";
					}
					fprintf(stderr,
						"flash to partition %s\n",
						write_argv_buf[4]);
				} else {
					fprintf(stderr,
						"no boot partition info found\n");
					ret = EINVAL;
					goto err;
				}

				_evalpid(bootpart_argv_buf, NULL, 0, &pid);

				if (!mktemp(upload_fifo) ||
				    mkfifo(upload_fifo, S_IRWXU) < 0 ||
				    (ret = _evalpid(write_argv_buf, NULL, 0,
						    &pid)) ||
				    !(fifo = fopen(upload_fifo, "w"))) {
					if (!ret)
						ret = errno;
					goto err;
				}
				goto write_data;
			}
#endif
#if defined(HAVE_DAP2230) || defined(HAVE_DAP2330) || defined(HAVE_DAP2660) || \
	defined(HAVE_DAP3662) || defined(HAVE_DAP3320)
#ifdef HAVE_DAP2660
#define MAGIC "wapac09_dkbs_dap2660"
#elif HAVE_DAP2330
#define MAGIC "wapn24_dkbs_dap2330"
#elif HAVE_DAP3662
#define MAGIC "wapac11_dkbs_dap3662"
#elif HAVE_DAP3320
#define MAGIC "wapn29_dkbs_dap3320"
#elif HAVE_DAP2230
#define MAGIC "wapn31_dkbs_dap2230"
#endif
			if (!strncmp(buf, MAGIC, strlen(MAGIC))) {
				char *write_argv_buf[8];
				write_argv_buf[0] = "mtd";
				write_argv_buf[1] = "-f";
				write_argv_buf[2] = "write";
				write_argv_buf[3] = upload_fifo;
				write_argv_buf[4] = "linux";
				write_argv_buf[5] = NULL;
				if (!mktemp(upload_fifo) ||
				    mkfifo(upload_fifo, S_IRWXU) < 0 ||
				    (ret = _evalpid(write_argv_buf, NULL, 0,
						    &pid)) ||
				    !(fifo = fopen(upload_fifo, "w"))) {
					if (!ret)
						ret = errno;
					goto err;
				}
				goto write_data;
			}
#endif
#if defined(HAVE_BUFFALO) || defined(HAVE_IDEXX)
			ralink_firmware_header fh;
			memcpy(&fh, buf, sizeof(fh));
			char *str;
			str = (char *)&fh;
			unsigned char ch, temp;
			int idx, index;
			ch = 0xff;
			index = sizeof(fh) - 1;
			for (idx = 0; idx < sizeof(fh) / 2; idx++) {
				temp = str[idx];
				str[idx] = str[index - idx];
				str[index - idx] = temp;
			}
			for (idx = 0; idx < sizeof(fh); idx++)
				str[idx] ^= ch;
			if (!strncmp(buf, "bgn", 3) ||
			    !strncmp(buf, "WZR", 3) ||
			    !strncmp(buf, "WHR", 3) ||
			    !strncmp(buf, "WLA", 3)) {
				char *write_argv_buf[4];
				write_argv_buf[0] = "buffalo_flash";
				write_argv_buf[1] = upload_fifo;
#if defined(HAVE_IDEXX) || defined(HAVE_IDEXX_SIGNATUR)
				write_argv_buf[2] = "verify";
				write_argv_buf[3] = NULL;
#else
				write_argv_buf[2] = NULL;
#endif
				if (!mktemp(upload_fifo) ||
				    mkfifo(upload_fifo, S_IRWXU) < 0 ||
				    (ret = _evalpid(write_argv_buf, NULL, 0,
						    &pid)) ||
				    !(fifo = fopen(upload_fifo, "w"))) {
					if (!ret)
						ret = errno;
					goto err;
				}
				goto write_data;
			} else if (!strncmp(fh.magic, "FIRMWARE",
					    8)) { // check for "FIRMWARE"
				char *write_argv_buf[5];
				write_argv_buf[0] = "buffalo_flash";
				write_argv_buf[1] = upload_fifo;
				write_argv_buf[2] = "ralink";
#if defined(HAVE_IDEXX) || defined(HAVE_IDEXX_SIGNATUR)
				write_argv_buf[3] = "verify";
				write_argv_buf[4] = NULL;
#else
				write_argv_buf[3] = NULL;
#endif
				if (!mktemp(upload_fifo) ||
				    mkfifo(upload_fifo, S_IRWXU) < 0 ||
				    (ret = _evalpid(write_argv_buf, NULL, 0,
						    &pid)) ||
				    !(fifo = fopen(upload_fifo, "w"))) {
					if (!ret)
						ret = errno;
					goto err;
				}
				goto write_data;
			} else {
#ifdef HAVE_IDEXX_SIGNATUR
				goto err;
#endif
				if (!mktemp(upload_fifo) ||
				    mkfifo(upload_fifo, S_IRWXU) < 0 ||
				    (ret = _evalpid(write_argv, NULL, 0,
						    &pid)) ||
				    !(fifo = fopen(upload_fifo, "w"))) {
					if (!ret)
						ret = errno;
					goto err;
				}
			}

#else
			/*
			 * Feed write from a temporary FIFO 
			 */
			if (!mktemp(upload_fifo) ||
			    mkfifo(upload_fifo, S_IRWXU) < 0 ||
			    (ret = _evalpid(write_argv, NULL, 0, &pid)) ||
			    !(fifo = fopen(upload_fifo, "w"))) {
				if (!ret)
					ret = errno;
				goto err;
			}
#endif
			// have code pattern
			char ver[40];
			long ver1, ver2, ver3;
			snprintf(ver, sizeof(ver), "v%d.%d.%d", buf[11],
				 buf[12], buf[13]);
			ver1 = convert_ver(ver);
			ver2 = convert_ver(INTEL_FLASH_SUPPORT_VERSION_FROM);
			ver3 = convert_ver(BCM4712_CHIP_SUPPORT_VERSION_FROM);
			fprintf(stderr,
				"upgrade_ver[%s] upgrade_ver[%ld] intel_ver[%ld] 4712_ver[%ld]\n",
				ver, ver1, ver2, ver3);
#if defined(HAVE_WIKINGS) || defined(HAVE_ESPOD)
#ifdef HAVE_WIKINGS
#ifdef HAVE_SUB3
#define V "XMED"
#elif HAVE_SUB6
#define V "XMIN"
#else
#define V "XMAX"
#endif
#endif
#ifdef HAVE_ESPOD
#ifdef HAVE_SUB3
#define V "EPMN"
#elif HAVE_SUB6
#define V "EPMD"
#else
#define V "EPMX"
#endif
#endif
			if (memcmp(&buf[0], V, 4)) {
				fprintf(stderr, "code pattern error!\n");
				goto write_data; // must be there, otherwise fail here
				//goto err;     // must be there, otherwise fail here
			}
#undef V
#endif

#if defined(HAVE_WIKINGS) || defined(HAVE_ESPOD)
#else
#ifdef HAVE_WRT160NL
			if (memcmp(&buf[0], &CODE_PATTERN_WRT160NL, 4) &&
			    memcmp(&buf[0], &CODE_PATTERN_E2100L, 4)) {
				cprintf("code pattern error!\n");
				goto err; // must be there, otherwise fail here
			}
#else

#ifndef HAVE_80211AC
			if (brand == ROUTER_LINKSYS_E1550 ||
			    (brand == ROUTER_WRT320N &&
			     nvram_match("boardrev", "0x1307")) //E2000
			    || brand == ROUTER_LINKSYS_E2500 ||
			    (brand == ROUTER_WRT610NV2 &&
			     nvram_match("boot_hw_model", "E300")) //E3000
			    || brand == ROUTER_LINKSYS_E3200 ||
			    brand == ROUTER_LINKSYS_E4200) {
				if (checkmagic(&buf[0], nv60k)) {
					cprintf("image not compatible with nv60k router!\n");
					goto err; // must be there, otherwise fail here
				}
			} else if (brand == ROUTER_NETGEAR_WNDR4000 ||
				   brand == ROUTER_NETGEAR_R6200 ||
				   brand == ROUTER_NETGEAR_WNDR3400 ||
				   brand == ROUTER_LINKSYS_E900 ||
				   brand == ROUTER_LINKSYS_E800 ||
				   brand == ROUTER_LINKSYS_E1500) {
				if (checkmagic(&buf[0], nv64k)) {
					cprintf("image not compatible with nv64k router!\n");
					goto err; // must be there, otherwise fail here
				}
			} else {
				if (memcmp(&buf[0], &CODE_PATTERN_NV60K, 4) ==
					    0 ||
				    memcmp(&buf[0], &CODE_PATTERN_NV64K, 4) ==
					    0) {
					cprintf("image not compatible with your router!\n");
					goto err; // fail here
				}
			}
#endif
			if (checkmagic(&buf[0], allmagics)) {
				cprintf("code pattern error!\n");
				goto write_data;
			}
#endif
#endif

			if (check_hw_type() == BCM4712_CHIP && ver1 < ver3) {
				fprintf(stderr,
					"The old firmware version can't support bcm4712 chipset\n");
				fprintf(stderr,
					"Can't downgrade to this old firmware version (%s), must be above %s(included)\n",
					ver, BCM4712_CHIP_SUPPORT_VERSION_FROM);
				goto write_data;
			}

			fprintf(stderr, "code pattern correct!\n");
			*total -= count;
#ifdef HAVE_WRT160NL
			safe_fwrite(
				buf, 1, count,
				fifo); // we have to write the whole header to flash too
#else
			safe_fwrite(&buf[sizeof(struct code_header)], 1,
				    count - sizeof(struct code_header), fifo);
			if (fifo2)
				safe_fwrite(&buf[sizeof(struct code_header)], 1,
					    count - sizeof(struct code_header),
					    fifo2);
#endif
			i++;
			continue;
		}

write_data:
		*total -= count;
		safe_fwrite(buf, 1, count, fifo);
		if (fifo2)
			safe_fwrite(buf, 1, count, fifo2);
		uploadcount += count;
		fprintf(stderr, "uploading [%d]\r", uploadcount);
		if (lastblock)
			break;
		i++;
	}

	fprintf(stderr, "uploading [%d]\n", uploadcount);
	if (fifo)
		fclose(fifo);
	if (fifo2)
		fclose(fifo2);
	fifo = NULL;
	fifo2 = NULL;
	/*
	 * Wait for write to terminate 
	 */
	waitpid(pid, &ret, 0);
	cprintf("done\n");
	if (!DO_SSL(stream)) {
		/*
		 * Reset nonblock on the socket 
		 */
		if (fcntl(fileno(stream->fp_in), F_SETFL, flags) < 0) {
			ret = errno;
			goto err;
		}
	}

err:
	if (buf)
		debug_free(buf);
	if (fifo)
		fclose(fifo);
	if (fifo2)
		fclose(fifo2);
	unlink(upload_fifo);
	if (brand == ROUTER_ASROCK_G10) {
		FILE *check = fopen("/tmp/parttemp", "rb");
		if (check) {
			fclose(check);
			fprintf(stderr,
				"write secondary partition for asrock-g10\n");
			eval("mtd", "-e", "linux2", "-f", "write",
			     "/tmp/parttemp", "linux2");
		}
	}
	// diag_led(DIAG, STOP_LED);
	ACTION("ACT_IDLE");
	return ret;
#else
	return 0;
#endif
}

static int
// do_upgrade_post(char *url, FILE *stream, int len, char *boundary)
do_upgrade_post(char *url, webs_t stream, size_t len, char *boundary) // jimmy,
	// https,
	// 8/6/2003
{
	killall("udhcpc", SIGKILL);

#ifndef ANTI_FLASH
	int type = 0;
	char *buf = malloc(1024);
	stream->upgrade_ret = EINVAL;

	/*
	 * Look for our part 
	 */
	while (len > 0) {
		if (!wfgets(buf, MIN(len + 1, 1024), stream, NULL)) {
			debug_free(buf);
			return -1;
		}

		len -= strlen(buf);
		if (!strncasecmp(buf, "Content-Disposition:", 20)) {
			if (strstr(buf, "name=\"erase\"")) {
				while (len > 0 && strcmp(buf, "\n") &&
				       strcmp(buf, "\r\n")) {
					if (!wfgets(buf, MIN(len + 1, 1024),
						    stream, NULL)) {
						debug_free(buf);
						return -1;
					}

					len -= strlen(buf);
				}
				if (!wfgets(buf, MIN(len + 1, 1024), stream,
					    NULL)) {
					debug_free(buf);
					return -1;
				}
				len -= strlen(buf);
				buf[1] = '\0'; // we only want the 1st digit
				nvram_set("sv_restore_defaults", buf);
				nvram_commit();
			} else if (strstr(buf,
					  "name=\"file\"")) { // upgrade image
				type = 0;
				break;
			}
		}
	}

	/*
	 * Skip boundary and headers 
	 */
	while (len > 0) {
		if (!wfgets(buf, MIN(len + 1, 1024), stream, NULL)) {
			debug_free(buf);
			return -1;
		}

		len -= strlen(buf);
		if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n"))
			break;
	}
	stream->upgrade_ret = sys_upgrade(NULL, stream, &len, type);

	/*
	 * Restore factory original settings if told to. This will also cause a
	 * restore defaults on reboot of a Sveasoft firmware. 
	 */
#ifdef HAVE_BUFFALO_SA
	int region_sa = 0;
	if (nvram_default_match("region", "SA", ""))
		region_sa = 1;
#endif
	if (nvram_matchi("sv_restore_defaults", 1)) {
		eval("erase", "nvram");
#ifdef HAVE_BUFFALO_SA
		nvram_seti("sv_restore_defaults", 1);
		if (region_sa)
			nvram_set("region", "SA");
#endif
	}
	_sys_commit(1);

	/*
	 * Slurp anything remaining in the request 
	 */

	wfgets(buf, len, stream, NULL);
	debug_free(buf);
#endif
	return 0;
}
