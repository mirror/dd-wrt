
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <sys/sysinfo.h>

#include <string.h>
#include <linux/version.h>

#if KERNEL_VERSION(2,6,0) <= LINUX_VERSION_CODE
#define IS_KERNEL26 1
#else
#define IS_KERNEL26 0
#endif

#if IS_KERNEL26
#include <linux/mtd/mtd-abi.h>
#else
#include <linux/mtd/mtd.h>
#endif

#include <trxhdr.h>
#include <crc.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <shutils.h>

#include <cy_conf.h>
#include <utils.h>

#include <endian.h>
#include <byteswap.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define STORE32_LE(X)		bswap_32(X)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define STORE32_LE(X)		(X)
#else
#error unkown endianness!
#endif

/* Netgear definitions */
#define NETGEAR_CRC_FAKE 1	//we fake checksum only over 4 bytes (HDR0)

#ifdef NETGEAR_CRC_FAKE
#define NETGEAR_CRC_FAKE_LEN		0x00000004
#define NETGEAR_CRC_FAKE_CHK		0x02C0010E
#else
static unsigned long calculate_checksum(int action, char *s, int size);
#endif
#define WGR614_LZMA_LOADER_SIZE		0x000919	//loader+400.lzma = 2329 bytes, please change if size changes!
#define FLASH_SIZE_4M			0x400000
#define FLASH_SIZE_8M			0x800000
#define CFE_SIZE_128K			0x020000
#define CFE_SIZE_256K			0x040000
#define NETGEAR_LEN_CHK_ADDR_4M		0x3AFFF8
#define NETGEAR_LEN_CHK_ADDR_8M		0x7AFFF8
#define NETGEAR_LEN_CHK_ADDR_8M_2	0x73FFF8
/* end */

/* Belkin series */
#define TRX_MAGIC_F7D3301		0x20100322	/* Belkin Share Max; router's birthday ? */
#define TRX_MAGIC_F7D3302		0x20090928	/* Belkin Share; router's birthday ? */
#define TRX_MAGIC_F7D4302		0x20091006	/* Belkin Play; router's birthday ? */
#define TRX_MAGIC_F5D8235V3		0x00017116	/* Belkin F7D8235V3 */
#define TRX_MAGIC_QA			0x12345678	/* Belkin: cfe: It's QA firmware */
/* end */

/* Netgear chk header */
#define NETGEAR_CHK_MAGIC			0x5E24232A

struct __attribute__((__packed__)) chk_header {
	uint32_t magic;
	uint32_t header_len;
	uint8_t reserved[8];
	uint32_t kernel_chksum;
	uint32_t rootfs_chksum;
	uint32_t kernel_len;
	uint32_t rootfs_len;
	uint32_t image_chksum;
	uint32_t header_chksum;

};

/* 
 * Open an MTD device
 * @param       mtd     path to or partition name of MTD device
 * @param       flags   open() flags
 * @return      return value of open()
 */
int mtd_open(const char *mtd, int flags)
{
	FILE *fp;
	char dev[PATH_MAX];
	int i;

	if ((fp = fopen("/proc/mtd", "r"))) {
		while (fgets(dev, sizeof(dev), fp)) {
			if (sscanf(dev, "mtd%d:", &i) && strstr(dev, mtd)) {
				snprintf(dev, sizeof(dev), "/dev/mtd/%d", i);
				fclose(fp);
				return open(dev, flags);
			}
		}
		fclose(fp);
	}

	return open(mtd, flags);
}

/* 
 * Erase an MTD device
 * @param       mtd     path to or partition name of MTD device
 * @return      0 on success and errno on failure
 */
int mtd_erase(const char *mtd)
{
	int mtd_fd;
	struct mtd_info_user mtd_info;
	struct erase_info_user erase_info;

	/* 
	 * char *et0; char *et1;
	 * 
	 * et0=nvram_safe_get("et0macaddr"); et1=nvram_safe_get("et1macaddr");
	 * et0=strdup(et0); et1=strdup(et1); 
	 */
	/* 
	 * Open MTD device 
	 */
	if ((mtd_fd = mtd_open(mtd, O_RDWR)) < 0) {
		perror(mtd);
		return errno;
	}

	/* 
	 * Get sector size 
	 */
	if (ioctl(mtd_fd, MEMGETINFO, &mtd_info) != 0) {
		perror(mtd);
		close(mtd_fd);
		return errno;
	}

	erase_info.length = mtd_info.erasesize;
#if defined(HAVE_80211AC) && !defined(HAVE_NORTHSTAR)
	for (erase_info.start = 0; erase_info.start < mtd_info.size; erase_info.start += mtd_info.erasesize) {
		fprintf(stderr, "erase[%d]\r", erase_info.start);
		(void)ioctl(mtd_fd, MEMUNLOCK, &erase_info);
		if (ioctl(mtd_fd, MEMERASE, &erase_info) != 0) {
			fprintf(stderr, "\nerror on sector %d, skip\n", erase_info.start);
			continue;
		}
	}
#else

	for (erase_info.start = 0; erase_info.start < mtd_info.size; erase_info.start += mtd_info.erasesize) {
		fprintf(stderr, "erase[%d]\r", erase_info.start);
		(void)ioctl(mtd_fd, MEMUNLOCK, &erase_info);
		if (ioctl(mtd_fd, MEMERASE, &erase_info) != 0) {
			perror(mtd);
			close(mtd_fd);
			return errno;
		}
	}
#endif
	close(mtd_fd);
	fprintf(stderr, "erase[%d]\n", erase_info.start);
	/* 
	 * nvram_set("et0macaddr",et0); nvram_set("et1macaddr",et1);
	 * nvram_commit(); free(et0); free(et1); 
	 */

	return 0;
}

extern int http_get(const char *server, char *buf, size_t count, off_t offset);

/* 
 * Write a file to an MTD device
 * @param       path    file to write or a URL
 * @param       mtd     path to or partition name of MTD device 
 * @return      0 on success and errno on failure
 */

struct img_info {
	uint32_t lenght;
	uint32_t CRC;
};

struct code_header {
	char magic[4];
	char res1[4];		// for extra magic
	char fwdate[3];
	char fwvern[3];
	char id[4];		// U2ND
	char hw_ver;		// 0) for 4702, 1) for 4712, 2) for 4712L, 3) for 4704
	char res2;
	unsigned short flags;
	unsigned char res3[10];
} __attribute__((packed));

struct etrx_header {
	struct code_header code;
	struct trx_header trx;
} __attribute__((packed));

#define SQUASHFS_MAGIC			0x74717368

int mtd_write(const char *path, const char *mtd)
{
	int mtd_fd = -1;
	struct mtd_info_user mtd_info;
	struct erase_info_user erase_info;

	struct sysinfo info;
#ifdef HAVE_WRT160NL
	struct etrx_header etrx;
#endif
	struct trx_header trx;
	struct chk_header chk;
	unsigned long crc;
	int squashfound = 0;
	unsigned int crc_data = 0;
	unsigned int data_len = 0;
	unsigned int cal_chksum = 0;
	FILE *fp;
	char *buf = NULL;
	long count, len, off;
	long sum = 0;		// for debug
	int ret = -1;
	int i;
	int skipoffset = 0;
	unsigned char lzmaloader[4096];
	int brand = getRouterBrand();
	/* 
	 * Netgear WGR614v8_L: Read, store and write back old lzma loader from 1st block 
	 */
	unsigned long trxhd = STORE32_LE(TRX_MAGIC);
	switch (brand) {
	case ROUTER_BUFFALO_WZR900DHP:
	case ROUTER_BUFFALO_WZR600DHP2:
	case ROUTER_LINKSYS_EA6900:
		if (nvram_match("bootpartition", "1")) {
			mtd = "linux2";
			nvram_set("bootpartition", "0");
			nvram_commit();
		}else{
			mtd = "linux2";
			nvram_set("bootpartition", "1");
			nvram_commit();		
		}
		break;
	case ROUTER_NETGEAR_WGR614L:
		if ((fp = fopen("/dev/mtdblock/1", "rb")))
			count = safe_fread(&trx, 1, sizeof(struct trx_header), fp);
		else
			return -1;

		memset(lzmaloader, 0, 4096);
		fseek(fp, trx.offsets[0], SEEK_SET);
		fread(lzmaloader, WGR614_LZMA_LOADER_SIZE, 1, fp);
		fclose(fp);
		break;
#ifdef HAVE_BCMMODERN
	case ROUTER_BELKIN_F7D3301:
	case ROUTER_BELKIN_F7D3302:
	case ROUTER_BELKIN_F7D4302:
	case ROUTER_BELKIN_F5D8235V3:

		if ((fp = fopen("/dev/mtdblock/1", "rb"))) {
			fread(&trxhd, 4, 1, fp);
			fclose(fp);
		}
		break;
#endif
	}
	nvram_set("flash_active", "1");
	sleep(1);

	/* 
	 * Examine TRX/CHK header 
	 */
#ifdef HAVE_WRT160NL
	fprintf(stderr, "size of ETRX header = %d\n", sizeof(struct etrx_header));
	if ((fp = fopen(path, "r")))
		count = safe_fread(&etrx, 1, sizeof(struct etrx_header), fp);
	else
		return -1;
	memcpy(&trx, &etrx.trx, sizeof(struct trx_header));
#else

	if ((fp = fopen(path, "r"))) {

		count = safe_fread(&trx, 1, sizeof(struct trx_header), fp);

		if (trx.magic == NETGEAR_CHK_MAGIC) {
			fprintf(stderr, "Netgear chk format detected\n");
			char board_id[18];
			safe_fread(&trx, 1, sizeof(struct chk_header) - sizeof(struct trx_header), fp);
			safe_fread(board_id, 1, sizeof(board_id), fp);

			switch (brand) {

			case ROUTER_NETGEAR_WNDR4000:
				if (strncmp(board_id, "U12H181T00_NETGEAR", sizeof(board_id))) {
					fprintf(stderr, "Error: board id %s expected %s\n", board_id, "U12H181T00_NETGEAR");
					fclose(fp);
					return -1;
				}
				break;
			case ROUTER_NETGEAR_WNDR4500:
				if (strncmp(board_id, "U12H189T00_NETGEAR", sizeof(board_id))) {
					fprintf(stderr, "Error: board id %s expected %s\n", board_id, "U12H189T00_NETGEAR");
					fclose(fp);
					return -1;
				}
				break;
			case ROUTER_NETGEAR_WNDR4500V2:
				if (strncmp(board_id, "U12H224T00_NETGEAR", sizeof(board_id))) {
					fprintf(stderr, "Error: board id %s expected %s\n", board_id, "U12H224T00_NETGEAR");
					fclose(fp);
					return -1;
				}
				break;
			case ROUTER_NETGEAR_R6250:
				if (strncmp(board_id, "U12H245T00_NETGEAR", sizeof(board_id))) {
					fprintf(stderr, "Error: board id %s expected %s\n", board_id, "U12H245T00_NETGEAR");
					fclose(fp);
					return -1;
				}
				break;
			case ROUTER_NETGEAR_R6300:
				if (strncmp(board_id, "U12H218T00_NETGEAR", sizeof(board_id))) {
					fprintf(stderr, "Error: board id %s expected %s\n", board_id, "U12H218T00_NETGEAR");
					fclose(fp);
					return -1;
				}
				break;
			case ROUTER_NETGEAR_R6300V2:
				if (strncmp(board_id, "U12H240T00_NETGEAR", sizeof(board_id))) {
					fprintf(stderr, "Error: board id %s expected %s\n", board_id, "U12H240T00_NETGEAR");
					fclose(fp);
					return -1;
				}
				break;
			case ROUTER_NETGEAR_R7000:
				if (strncmp(board_id, "U12H270T00_NETGEAR", sizeof(board_id))) {
					fprintf(stderr, "Error: board id %s expected %s\n", board_id, "U12H270T00_NETGEAR");
					fclose(fp);
					return -1;
				}
				break;
			default:
				fprintf(stderr, "Error: Flash to OEM for board %s not supported yet\n", board_id);
				fclose(fp);
				return -1;
			}

			count = safe_fread(&trx, 1, sizeof(struct trx_header), fp);
		}

	} else {
		return -1;
	}

#endif
	// count = http_get (path, (char *) &trx, sizeof (struct trx_header), 0);
	if (count < sizeof(struct trx_header)) {
		fprintf(stderr, "%s: File is too small (%ld bytes)\n", path, count);
		goto fail;
	}
	sysinfo(&info);
#ifndef HAVE_CAMBRIA
	stop_service("sshd");
	stop_service("snmp");
	stop_service("zebra");
	stop_service("pppoeserver");
	stop_service("olsrd");
	stop_service("openvpn");
	stop_service("openvpnserver");
	stop_service("pptpd");
	stop_service("upnp");
	stop_service("freeradius");
	system("killall process_monitor");
#endif
#ifdef HAVE_MAGICBOX
	trx.magic = STORE32_LE(trx.magic);
	trx.len = STORE32_LE(trx.len);
	trx.crc32 = STORE32_LE(trx.crc32);
#elif HAVE_FONERA
	trx.magic = STORE32_LE(trx.magic);
	trx.len = STORE32_LE(trx.len);
	trx.crc32 = STORE32_LE(trx.crc32);
	// info.freeram = 64; //fix, must be flashed in erase blocks
#elif HAVE_SOLO51
	trx.magic = STORE32_LE(trx.magic);
	trx.len = STORE32_LE(trx.len);
	trx.crc32 = STORE32_LE(trx.crc32);
	// info.freeram = 64; //fix, must be flashed in erase blocks
#elif HAVE_MERAKI
	trx.magic = STORE32_LE(trx.magic);
	trx.len = STORE32_LE(trx.len);
	trx.crc32 = STORE32_LE(trx.crc32);
	// info.freeram = 64; //fix, must be flashed in erase blocks
#elif HAVE_LS2
	trx.magic = STORE32_LE(trx.magic);
	trx.len = STORE32_LE(trx.len);
	trx.crc32 = STORE32_LE(trx.crc32);
	// info.freeram = 64; //fix, must be flashed in erase blocks
#elif HAVE_LS5
	trx.magic = STORE32_LE(trx.magic);
	trx.len = STORE32_LE(trx.len);
	trx.crc32 = STORE32_LE(trx.crc32);
#elif HAVE_WDR4900
	trx.magic = STORE32_LE(trx.magic);
	trx.len = STORE32_LE(trx.len);
	trx.crc32 = STORE32_LE(trx.crc32);
#elif HAVE_WHRAG108
	trx.magic = STORE32_LE(trx.magic);
	trx.len = STORE32_LE(trx.len);
	trx.crc32 = STORE32_LE(trx.crc32);
#elif HAVE_PB42
	trx.magic = STORE32_LE(trx.magic);
	trx.len = STORE32_LE(trx.len);
	trx.crc32 = STORE32_LE(trx.crc32);
	// info.freeram = 64; //fix, must be flashed in erase blocks
#elif HAVE_LSX
	trx.magic = STORE32_LE(trx.magic);
	trx.len = STORE32_LE(trx.len);
	trx.crc32 = STORE32_LE(trx.crc32);
	// info.freeram = 64; //fix, must be flashed in erase blocks
#elif HAVE_TW6600
	trx.magic = STORE32_LE(trx.magic);
	trx.len = STORE32_LE(trx.len);
	trx.crc32 = STORE32_LE(trx.crc32);
#elif HAVE_CA8
	trx.magic = STORE32_LE(trx.magic);
	trx.len = STORE32_LE(trx.len);
	trx.crc32 = STORE32_LE(trx.crc32);
#elif HAVE_DANUBE
	trx.magic = STORE32_LE(trx.magic);
	trx.len = STORE32_LE(trx.len);
	trx.crc32 = STORE32_LE(trx.crc32);
#elif HAVE_XSCALE
	trx.magic = STORE32_LE(trx.magic);
	trx.len = STORE32_LE(trx.len);
	trx.crc32 = STORE32_LE(trx.crc32);
#endif

	if (trx.magic != TRX_MAGIC || trx.len < sizeof(struct trx_header)) {
		fprintf(stderr, "%s: Bad trx header\n", path);
		goto fail;
	}
	/* 
	 * Open MTD device and get sector size 
	 */
	if ((mtd_fd = mtd_open(mtd, O_RDWR)) < 0 || ioctl(mtd_fd, MEMGETINFO, &mtd_info) != 0 || mtd_info.erasesize < sizeof(struct trx_header)) {
		perror(mtd);
		goto fail;
	}
	if (STORE32_LE(trx.flag_version) & TRX_NO_HEADER)
		trx.len -= sizeof(struct trx_header);

	// #ifndef HAVE_WRK54G
	if (mtd_info.size < trx.len) {
		fprintf(stderr, "Image too big for partition: %s\n", mtd);
		perror(mtd);
		goto fail;
	}
	// #endif

	/* 
	 * See if we have enough memory to store the whole file 
	 */
	fprintf(stderr, "freeram=[%ld] bufferram=[%ld]\n", info.freeram, info.bufferram);
	int mul = 1;		// temporarily use 1 instead of 4 until we

	// found a a solution
	if ((info.freeram + info.bufferram) >= (trx.len + 4 * 1024 * 1024)) {
		fprintf(stderr, "The free memory is enough, writing image once.\n");
		/* 
		 * Begin to write image after all image be downloaded by web upgrade.
		 * In order to avoid upgrade fail if user unplug the ethernet cable
		 * during upgrading 
		 */
		// if(check_action() == ACT_WEBS_UPGRADE || check_action() ==
		// ACT_WEB_UPGRADE)
		erase_info.length = ROUNDUP(trx.len, mtd_info.erasesize);
	} else {
		erase_info.length = mtd_info.erasesize * mul;
		fprintf(stderr, "The free memory is not enough, writing image per %d bytes.\n", erase_info.length);
	}

	/* 
	 * Allocate temporary buffer 
	 */
	if (!(buf = malloc(erase_info.length))) {
		mul = 1;
		erase_info.length = mtd_info.erasesize * mul;
		fprintf(stderr, "The free memory is not enough, writing image per %d bytes.\n", erase_info.length);
		if (!(buf = malloc(erase_info.length))) {
			perror("malloc");
			goto fail;
		}
	}

	/* 
	 * Calculate CRC over header 
	 */
	crc = crc32((uint8 *) & trx.flag_version, sizeof(struct trx_header) - OFFSETOF(struct trx_header, flag_version), CRC32_INIT_VALUE);
	crc_data = 0;
#ifndef NETGEAR_CRC_FAKE
	calculate_checksum(0, NULL, 0);	// init
#endif
	/* 
	 * Write file or URL to MTD device 
	 */
	for (erase_info.start = 0; erase_info.start < trx.len; erase_info.start += count) {
		len = MIN(erase_info.length, trx.len - erase_info.start);
		if ((STORE32_LE(trx.flag_version) & TRX_NO_HEADER)
		    || erase_info.start)
			count = off = 0;
		else {
#ifdef HAVE_WRT160NL
			count = off = sizeof(struct etrx_header);
			memcpy(buf, &etrx, sizeof(struct etrx_header));
#else
			count = off = sizeof(struct trx_header);
			memcpy(buf, &trx, sizeof(struct trx_header));
#endif
		}
		// if (fp)
		count += safe_fread(&buf[off], 1, len - off, fp);
		// else
		// count +=
		// http_get (path, &buf[off], len - off, erase_info.start + off);

		/* 
		 * for debug 
		 */
		sum = sum + count;
		fprintf(stderr, "write=[%ld]         \n", sum);

		if (((count < len)
		     && (len - off) > (mtd_info.erasesize * mul))
		    || (count == 0)) {
			fprintf(stderr, "%s: Truncated file (actual %ld expect %ld)\n", path, count - off, len - off);
			goto fail;
		}
		/* 
		 * Update CRC 
		 */
		crc = crc32(&buf[off], count - off, crc);
#ifndef NETGEAR_CRC_FAKE
		calculate_checksum(1, buf, count);
#endif

		if (!squashfound) {
			for (i = 0; i < (count - off); i++) {
				unsigned int *sq = (unsigned int *)&buf[off + i];

				if (*sq == SQUASHFS_MAGIC) {
					squashfound = 1;
					break;
				}
				crc_data += (unsigned char)buf[off + i];
			}
			data_len += i;
		}
		/* 
		 * Check CRC before writing if possible 
		 */
#ifdef HAVE_WRT160NL
		if (count == trx.len + sizeof(struct code_header)) {
#else
		if (count == trx.len) {
#endif
			if (crc != trx.crc32) {
				fprintf(stderr, "%s: Bad CRC\n", path);
				goto fail;
			} else {
				fprintf(stderr, "%s: CRC OK\n", mtd);
				fprintf(stderr, "Writing image to flash, waiting a moment...\n");
			}
		}
		erase_info.length = mtd_info.erasesize;

		int length = ROUNDUP(count, mtd_info.erasesize);
		int base = erase_info.start;
#if defined(HAVE_80211AC) && !defined(HAVE_NORTHSTAR)
		for (i = 0; i < (length / mtd_info.erasesize); i++) {
			int redo = 0;
		      again:;
			fprintf(stderr, "write block [%ld] at [0x%08X]        \n", i * mtd_info.erasesize, base + ((i + skipoffset) * mtd_info.erasesize));
			erase_info.start = base + ((i + skipoffset) * mtd_info.erasesize);
			(void)ioctl(mtd_fd, MEMUNLOCK, &erase_info);
			if (ioctl(mtd_fd, MEMERASE, &erase_info) != 0) {
				fprintf(stderr, "erase/write failed, skip block\n");
				skipoffset++;
				goto again;
			}
			lseek(mtd_fd, (i + skipoffset) * mtd_info.erasesize, SEEK_SET);

			if (write(mtd_fd, buf + (i * mtd_info.erasesize), mtd_info.erasesize) != mtd_info.erasesize) {
				fprintf(stderr, "try again %d\n", redo++);
				if (redo < 10)
					goto again;
				goto fail;
			}

		}
#else
		for (i = 0; i < (length / mtd_info.erasesize); i++) {
			fprintf(stderr, "write block [%ld] at [0x%08X]        \n", i * mtd_info.erasesize, base + (i * mtd_info.erasesize));
			erase_info.start = base + (i * mtd_info.erasesize);
			(void)ioctl(mtd_fd, MEMUNLOCK, &erase_info);
			if (ioctl(mtd_fd, MEMERASE, &erase_info) != 0 || write(mtd_fd, buf + (i * mtd_info.erasesize), mtd_info.erasesize) != mtd_info.erasesize) {
				perror(mtd);
				goto fail;
			}
		}

#endif
	}
	/* 
	 * Netgear: Write len and checksum at the end of mtd1 
	 */
	int sector_start;
	char *tmp;
	if (brand == ROUTER_NETGEAR_WGR614L || brand == ROUTER_NETGEAR_WNR834B || brand == ROUTER_NETGEAR_WNR834BV2 || brand == ROUTER_NETGEAR_WNDR3300
//          || brand == ROUTER_NETGEAR_WNDR4000
	    || brand == ROUTER_NETGEAR_WNR3500L) {
#ifndef NETGEAR_CRC_FAKE
		cal_chksum = calculate_checksum(2, NULL, 0);
#endif
		char imageInfo[8];
		unsigned long cfe_size = CFE_SIZE_128K;
		unsigned long flash_len_chk_addr = NETGEAR_LEN_CHK_ADDR_4M;
		if (brand == ROUTER_NETGEAR_WNR3500L) {
			cfe_size = CFE_SIZE_256K;
			if (mtd_info.size > FLASH_SIZE_4M) {
				flash_len_chk_addr = NETGEAR_LEN_CHK_ADDR_8M;
			}
		}
/*
		if (brand == ROUTER_NETGEAR_WNDR4000) {
			cfe_size = CFE_SIZE_256K;
			flash_len_chk_addr = NETGEAR_LEN_CHK_ADDR_8M_2;
		}
*/
#ifndef NETGEAR_CRC_FAKE
		trx.len = STORE32_LE(trx.len);
		cal_chksum = STORE32_LE(cal_chksum);
#else
		trx.len = STORE32_LE(NETGEAR_CRC_FAKE_LEN);
		cal_chksum = STORE32_LE(NETGEAR_CRC_FAKE_CHK);
#endif
		memcpy(&imageInfo[0], (char *)&trx.len, 4);
		memcpy(&imageInfo[4], (char *)&cal_chksum, 4);
		sector_start = ((flash_len_chk_addr - cfe_size) / mtd_info.erasesize) * mtd_info.erasesize;
		if (lseek(mtd_fd, sector_start, SEEK_SET) < 0) {
			//fprintf( stderr, "Error seeking the file descriptor\n" );
			goto fail;
		}

		free(buf);
		if (!(buf = malloc(mtd_info.erasesize))) {
			//fprintf( stderr, "Error allocating image block\n");
			goto fail;
		}

		memset(buf, 0, mtd_info.erasesize);
		if (read(mtd_fd, buf, mtd_info.erasesize) != mtd_info.erasesize) {
			//fprintf( stderr, "Error reading last block from MTD device\n" );
			goto fail;
		}

		if (lseek(mtd_fd, sector_start, SEEK_SET) < 0) {
			//fprintf( stderr, "Error seeking the file descriptor\n" );
			goto fail;
		}

		erase_info.start = sector_start;
		erase_info.length = mtd_info.erasesize;
		ioctl(mtd_fd, MEMUNLOCK, &erase_info);
		if (ioctl(mtd_fd, MEMERASE, &erase_info) != 0) {
			//fprintf( stderr, "Error erasing MTD block\n" );
			goto fail;
		}

		tmp = buf + ((flash_len_chk_addr - cfe_size) % mtd_info.erasesize);
		memcpy(tmp, imageInfo, sizeof(imageInfo));
		if (write(mtd_fd, buf, mtd_info.erasesize) != mtd_info.erasesize) {
			//fprintf( stderr, "Error writing chksum to MTD device\n" );
			goto fail;
		}
		//fprintf( stderr, "TRX LEN = %x , CHECKSUM = %x\n", trx.len, cal_chksum );
#ifndef NETGEAR_CRC_FAKE
		fprintf(stderr, "Write len/chksum @ 0x%X ...done.\n", flash_len_chk_addr);
#else
		fprintf(stderr, "Write fake len/chksum @ 0x%X ...done.\n", flash_len_chk_addr);
#endif
	}

	/* Write old lzma loader */
	if (brand == ROUTER_NETGEAR_WGR614L) {
		int offset = trx.offsets[0];
		sector_start = (offset / mtd_info.erasesize) * mtd_info.erasesize;
		if (lseek(mtd_fd, sector_start, SEEK_SET) < 0) {
			//fprintf( stderr, "Error seeking the file descriptor\n" );
			goto fail;
		}

		free(buf);
		if (!(buf = malloc(mtd_info.erasesize))) {
			//fprintf( stderr, "Error allocating image block\n");
			goto fail;
		}

		memset(buf, 0, mtd_info.erasesize);
		if (read(mtd_fd, buf, mtd_info.erasesize) != mtd_info.erasesize) {
			//fprintf( stderr, "Error reading first block from MTD device\n" );
			goto fail;
		}

		if (lseek(mtd_fd, sector_start, SEEK_SET) < 0) {
			//fprintf( stderr, "Error seeking the file descriptor\n" );
			goto fail;
		}

		erase_info.start = sector_start;
		erase_info.length = mtd_info.erasesize;
		ioctl(mtd_fd, MEMUNLOCK, &erase_info);
		if (ioctl(mtd_fd, MEMERASE, &erase_info) != 0) {
			//fprintf( stderr, "Error erasing MTD block\n" );
			goto fail;
		}

		tmp = buf + (offset % mtd_info.erasesize);
		if (trx.offsets[1] - trx.offsets[0] >= WGR614_LZMA_LOADER_SIZE) {
			memcpy(tmp, lzmaloader, trx.offsets[1] - trx.offsets[0]);	//we asume lzma loader is shorter then gz loader
			//fprintf( stderr, "LZMA loader size OK, space=%d needed=%d\n", trx.offsets[1] - trx.offsets[0], WGR614_LZMA_LOADER_SIZE );
		} else {
			memset(buf, 0, mtd_info.erasesize);	//destroy 1st block, which puts router into tftp mode to allow recover
			//fprintf( stderr, "LZMA loader size too large, space=%d needed=%d\n", trx.offsets[1] - trx.offsets[0], WGR614_LZMA_LOADER_SIZE );
		}
		if (write(mtd_fd, buf, mtd_info.erasesize) != mtd_info.erasesize) {
			//fprintf( stderr, "Error writing LZMA loader to MTD device\n" );
			goto fail;
		}

		fprintf(stderr, "Write lzma loader...done.\n");
	}			// end

#ifdef HAVE_BCMMODERN
	/* Write Belkin magic */
	if (brand == ROUTER_BELKIN_F7D3301 || brand == ROUTER_BELKIN_F7D3302 || brand == ROUTER_BELKIN_F7D4302 || brand == ROUTER_BELKIN_F5D8235V3) {

		sector_start = 0;
		unsigned long be_magic = STORE32_LE(trxhd);
		char be_trx[4];
		memcpy(&be_trx[0], (char *)&be_magic, 4);
		if (lseek(mtd_fd, sector_start, SEEK_SET) < 0) {
			//fprintf( stderr, "Error seeking the file descriptor\n" );
			goto fail;
		}

		free(buf);
		if (!(buf = malloc(mtd_info.erasesize))) {
			//fprintf( stderr, "Error allocating image block\n");
			goto fail;
		}

		memset(buf, 0, mtd_info.erasesize);
		if (read(mtd_fd, buf, mtd_info.erasesize) != mtd_info.erasesize) {
			//fprintf( stderr, "Error reading first block from MTD device\n" );
			goto fail;
		}

		if (lseek(mtd_fd, sector_start, SEEK_SET) < 0) {
			//fprintf( stderr, "Error seeking the file descriptor\n" );
			goto fail;
		}

		erase_info.start = sector_start;
		erase_info.length = mtd_info.erasesize;
		ioctl(mtd_fd, MEMUNLOCK, &erase_info);
		if (ioctl(mtd_fd, MEMERASE, &erase_info) != 0) {
			//fprintf( stderr, "Error erasing MTD block\n" );
			goto fail;
		}

		memcpy(buf, be_trx, 4);
		if (write(mtd_fd, buf, mtd_info.erasesize) != mtd_info.erasesize) {
			//fprintf( stderr, "Error writing Belkin magic to MTD device\n" );
			goto fail;
		}

		fprintf(stderr, "Write Belkin magic...done.\n");
	}			// end
#endif

	ret = 0;
fail:
	nvram_set("flash_active", "0");
	if (buf) {
		/* 
		 * Dummy read to ensure chip(s) are out of lock/suspend state 
		 */
		(void)read(mtd_fd, buf, 2);
		free(buf);
	}

	if (mtd_fd >= 0)
		close(mtd_fd);
	if (fp)
		fclose(fp);
#ifdef HAVE_CA8
#ifndef HAVE_ALPHA
#ifndef HAVE_USR5453
	buf = malloc(65536);
	FILE *in = fopen("/dev/mtdblock/2", "rb");
	fread(buf, 65536, 1, in);
	fclose(in);
	struct img_info *image_info;
	image_info = buf + 0x56;
	image_info->lenght = data_len;
	image_info->CRC = crc_data;
	in = fopen("/tmp/bdata", "wb");
	fwrite(buf, 65536, 1, in);
	fclose(in);
	fprintf(stderr, "fixup CRC %X and LEN %X\n", crc_data, data_len);
	eval("mtd", "-f", "write", "/tmp/bdata", "bdata");
#endif
#endif
#endif
#ifdef HAVE_WZRHPAG300NH
	// must delete checksum, otherwise device will not boot anymore. the checksum gets recreated by the bootloader
	if (!ret)
		sysprintf("ubootenv del buf_crc");
#endif
	// eval("fischecksum");
	return ret;
}

/* 
 * Irving -  We need an unlock function in order to mount a r/w jffs2 partition
 * Unlock an MTD device
 * @param       mtd     path to or partition name of MTD device
 * @return      0 on success and errno on failure
 */
int mtd_unlock(const char *mtd)
{
	int mtd_fd;
	struct mtd_info_user mtd_info;
	struct erase_info_user lock_info;
	/* 
	 * Open MTD device 
	 */
	if ((mtd_fd = mtd_open(mtd, O_RDWR)) < 0) {
		perror(mtd);
		return errno;
	}

	/* 
	 * Get sector size 
	 */
	if (ioctl(mtd_fd, MEMGETINFO, &mtd_info) != 0) {
		perror(mtd);
		close(mtd_fd);
		return errno;
	}

	lock_info.start = 0;
	lock_info.length = mtd_info.size;
	if (ioctl(mtd_fd, MEMUNLOCK, &lock_info)) {
		fprintf(stderr, "Could not unlock MTD device: %s\n", mtd);
		perror(mtd);
		close(mtd_fd);
		return errno;
	}

	close(mtd_fd);
	return 0;
}

#ifndef NETGEAR_CRC_FAKE
// Netgear image checksum
static unsigned long calculate_checksum(int action, char *s, int size)
{
	static unsigned long c0, c1;
	unsigned long checksum, b;
	int i;
	switch (action) {
	case 0:
		c0 = c1 = 0;
		break;
	case 1:
		for (i = 0; i < size; i++) {
			c0 += s[i] & 0xff;
			c1 += c0;
		}
		break;
	case 2:
		b = (c0 & 65535) + ((c0 >> 16) & 65535);
		c0 = ((b >> 16) + b) & 65535;
		b = (c1 & 65535) + ((c1 >> 16) & 65535);
		c1 = ((b >> 16) + b) & 65535;
		checksum = ((c1 << 16) | c0);
		return checksum;
	}
	return 0;
}
#endif
