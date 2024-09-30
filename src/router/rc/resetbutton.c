/*
 * resetbutton.c
 *
 * Copyright (C) 2006 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <rc.h>
#include <stdarg.h>
#include <dirent.h>
#include <syslog.h>

#define SES_LED_CHECK_TIMES "9999" /* How many times to check? */
#define SES_LED_CHECK_INTERVAL "1" /* Wait interval seconds */

#ifdef HAVE_ANTAIRA
#define RESET_WAIT 5 /* seconds */
#else
#define RESET_WAIT 3 /* seconds */
#endif /*HAVE_ANTAIRA */

#define RESET_WAIT_COUNT RESET_WAIT * 10 /* 10 times a second */

#ifdef HAVE_ERC
#define SES_WAIT 2 /* seconds */
#else
#define SES_WAIT 3 /* seconds */
#endif
#define SES_WAIT_COUNT SES_WAIT /* 10 times a second */
#ifdef HAVE_UNFY
#define UPGRADE_WAIT 1 /* seconds */
#define UPGRADE_WAIT_COUNT UPGRADE_WAIT * 10 - 5
#endif

#define NORMAL_INTERVAL 1 /* second */
#define URGENT_INTERVAL 100 * 1000 /* microsecond */

#ifndef HAVE_GATEWORX /* 1/10 second */
#define GPIO_FILE "/dev/gpio/in"
#endif
#if 0
#define DEBUG printf
#else
#define DEBUG(format, args...)
#endif
static int brand;

#ifdef HAVE_MAGICBOX
#include <sys/mman.h>

#define GPIO0_OR 0x0700 /* rw, output */
#define GPIO0_TCR 0x0704 /* rw, three-state control */
#define GPIO0_ODR 0x0718 /* rw, open drain */
#define GPIO0_IR 0x071c /* ro, input */
#define GPIO0_BASE 0xef600000 /* page */

#define GPIO_LED 0x20000000 /* GPIO1 */
#define GPIO_BUTTON 0x40000000 /* GPIO2 */

#define REG(buf, offset) ((unsigned int *)((void *)buf + offset))

static unsigned int *page;
static int fd;

static void init_gpio()
{
	void *start = 0;

	fd = open("/dev/mem", O_RDWR);
	if (fd < 0) {
		// dd_syslog(LOG_ERR, "Can't open /dev/mem: %s", strerror(errno));
		exit(1);
	}

	page = mmap(start, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t)GPIO0_BASE);
	if (page == MAP_FAILED) {
		// dd_syslog(LOG_ERR, "Can't mmap GPIO memory space: %s",
		// strerror(errno));
		exit(1);
	}

	/* 
	 * disable 
	 */
	*REG(page, GPIO0_TCR) &= ~(GPIO_LED | GPIO_BUTTON);
	/* 
	 * enable led 
	 */
	*REG(page, GPIO0_TCR) |= GPIO_LED | GPIO_BUTTON;
	/* 
	 * enable/disable(?) button 
	 */
	*REG(page, GPIO0_TCR) &= ~GPIO_BUTTON;

	*REG(page, GPIO0_IR) & GPIO_BUTTON;
	*REG(page, GPIO0_IR) & GPIO_BUTTON;
}

static int getbuttonstate()
{
	return (*REG(page, GPIO0_IR) & GPIO_BUTTON) == 0;
}
#endif

#if defined(HAVE_FONERA) || defined(HAVE_WHRAG108) || defined(HAVE_LS2) || defined(HAVE_CA8) || defined(HAVE_TW6600) || \
	defined(HAVE_LS5) || defined(HAVE_WP54G) || defined(HAVE_NP28G) || defined(HAVE_SOLO51) || defined(HAVE_OPENRISC)
static int getbuttonstate()
{
#if defined(HAVE_EAP3660) || defined(HAVE_EOC2610) || defined(HAVE_EOC1650) || defined(HAVE_ECB3500)
	return !get_gpio(5);
#elif defined(HAVE_WRT54G2)
	return !get_gpio(7);
#elif defined(HAVE_RTG32)
	return get_gpio(6);
#elif defined(HAVE_EOC5610)
	return !get_gpio(6);
#elif HAVE_WP54G
	return get_gpio(4);
#elif HAVE_NP28G
	return get_gpio(4);
#elif HAVE_WPE53G
	return get_gpio(6);
#elif HAVE_NP25G
	return get_gpio(4);
#elif HAVE_OPENRISC
	return get_gpio(0);
#else
	return get_gpio(6);
#endif
}
#elif defined(HAVE_NEWPORT)
static int getbuttonstate()
{
	return 0; //!get_gpio(496);
}
#elif defined(HAVE_VENTANA)
static int getbuttonstate()
{
	return !get_gpio(496);
}
#elif defined(HAVE_E200)
static int getbuttonstate()
{
	return !get_gpio(0);
}
#elif defined(HAVE_EROUTER)
static int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_UNIWIP)
static int getbuttonstate()
{
	return !get_gpio(232);
}
#elif defined(HAVE_WDR4900)
static int getbuttonstate()
{
	return !get_gpio(5);
}
#elif defined(HAVE_MVEBU)
static int getbuttonstate()
{
	int ret;
	if (brand == ROUTER_WRT_1900AC)
		ret = get_gpio(33);
	else
		ret = get_gpio(29);
	return !ret;
}
#elif defined(HAVE_R9000)
static int getbuttonstate()
{
	int ret = !get_gpio(31);
	return ret;
}
#elif defined(HAVE_IPQ6018)
static int getbuttonstate()
{
	switch (getRouterBrand()) {
	case ROUTER_LINKSYS_MR7350:
		return !get_gpio(57);
	case ROUTER_LINKSYS_MX4200V1:
	case ROUTER_LINKSYS_MX4200V2:
	case ROUTER_LINKSYS_MX4300:
		return !get_gpio(52);
	case ROUTER_DYNALINK_DLWRX36:
		return !get_gpio(34);
	case ROUTER_LINKSYS_MR5500:
	case ROUTER_LINKSYS_MX5500:
		return !get_gpio(28);
	case ROUTER_ASUS_AX89X:
		return !get_gpio(61);
	case ROUTER_BUFFALO_WXR5950AX12:
		return !get_gpio(54);
	default:
		return 0;
	}
}
#elif defined(HAVE_IPQ806X)
static int getbuttonstate()
{
	int ret = 0;
	switch (brand) {
	case ROUTER_LINKSYS_EA8500:
		ret = get_gpio(68);
		break;
	case ROUTER_ASROCK_G10:
		ret = get_gpio(16);
		break;
	case ROUTER_ASUS_AC58U:
		ret = get_gpio(4);
		break;
	case ROUTER_LINKSYS_EA8300:
		ret = get_gpio(50);
		break;
	case ROUTER_HABANERO:
#ifdef HAVE_ANTAIRA
		return 0;
#else
		ret = get_gpio(8);
#endif
		break;
	default:
		ret = get_gpio(54);
	}
	return !ret;
}
#elif defined(HAVE_DAP3410)
static int getbuttonstate()
{
	return !get_gpio(17);
}
#elif defined(HAVE_UBNTM)
static int getbuttonstate()
{
	if (brand == ROUTER_UBNT_ROCKETAC)
		return !get_gpio(19);
	if (brand == ROUTER_UBNT_UAPAC || brand == ROUTER_UBNT_UAPACPRO)
		return !get_gpio(2);
	return !get_gpio(12);
}
#elif defined(HAVE_RB2011)
static int getbuttonstate()
{
	return 0;
}
#elif defined(HAVE_WDR4300)
static int getbuttonstate()
{
	return !get_gpio(16);
}
#elif defined(HAVE_WNDR3700V4)
static int getbuttonstate()
{
	return !get_gpio(21);
}
#elif defined(HAVE_DAP2680)
static int getbuttonstate()
{
	return !get_gpio(18);
}
#elif defined(HAVE_DAP3320)
static int getbuttonstate()
{
	return !get_gpio(12);
}
#elif defined(HAVE_DAP2330)
static int getbuttonstate()
{
	return !get_gpio(17);
}
#elif defined(HAVE_DAP2230)
static int getbuttonstate()
{
	return !get_gpio(17);
}
#elif defined(HAVE_DIR862)
static int getbuttonstate()
{
	return !get_gpio(17);
}
#elif defined(HAVE_CPE880)
static int getbuttonstate()
{
	return !get_gpio(4);
}
#elif defined(HAVE_MMS344)
static int getbuttonstate()
{
	return 0;
	int ret = get_gpio(12);

	if (ret == 0)
		return 1;
	return 0;
}
#elif defined(HAVE_ARCHERA7V5)
static int getbuttonstate()
{
	return !get_gpio(5);
}
#elif defined(HAVE_ARCHERC7V4)
static int getbuttonstate()
{
	return !get_gpio(5);
}
#elif defined(HAVE_WR1043V4)
static int getbuttonstate()
{
	return !get_gpio(2);
}
#elif defined(HAVE_WR1043V2)
static int getbuttonstate()
{
	return !get_gpio(16);
}
#elif defined(HAVE_WZR450HP2)
static int getbuttonstate()
{
	return !get_gpio(17);
}
#elif defined(HAVE_XD9531)
static int getbuttonstate()
{
	return !get_gpio(17);
}
#elif defined(HAVE_WR615N)
static int getbuttonstate()
{
	return !get_gpio(17);
}
#elif defined(HAVE_E325N)
static int getbuttonstate()
{
	return !get_gpio(20);
}
#elif defined(HAVE_E355AC)
static int getbuttonstate()
{
	return !get_gpio(17);
}
#elif defined(HAVE_PERU)
static int getbuttonstate()
{
	return 0; //return !get_gpio(1);
}
#elif defined(HAVE_LIMA)
static int getbuttonstate()
{
	return !get_gpio(16);
}
#elif defined(HAVE_DW02_412H)
static int getbuttonstate()
{
	return !get_gpio(17);
}
#elif defined(HAVE_RAMBUTAN)
static int getbuttonstate()
{
	return !get_gpio(18);
}
#elif defined(HAVE_AP120C)
static int getbuttonstate()
{
	return !get_gpio(16);
}
#elif defined(HAVE_WR650AC)
static int getbuttonstate()
{
	return !get_gpio(19);
}
#elif defined(HAVE_XD3200)
static int getbuttonstate()
{
	return !get_gpio(2);
}
#elif defined(HAVE_E380AC)
static int getbuttonstate()
{
	return !get_gpio(19);
}
#elif defined(HAVE_DIR869)
static int getbuttonstate()
{
	return !get_gpio(1);
}
#elif defined(HAVE_DIR859)
static int getbuttonstate()
{
	return !get_gpio(2);
}
#elif defined(HAVE_JWAP606)
static int getbuttonstate()
{
	return !get_gpio(15);
}
#elif defined(HAVE_DIR825C1)
static int getbuttonstate()
{
	return !get_gpio(17);
}
#elif defined(HAVE_WASP)
static int getbuttonstate()
{
	return !get_gpio(12);
}
#elif defined(HAVE_CARAMBOLA)
#ifdef HAVE_ERC
static int getbuttonstate()
{
	return get_gpio(12);
}
#elif HAVE_FMS2111
static int getbuttonstate()
{
	return 0;
}
#else
static int getbuttonstate()
{
	return !get_gpio(11);
}
#endif
#elif defined(HAVE_HORNET)
static int getbuttonstate()
{
	return !get_gpio(12);
}
#elif defined(HAVE_WNR2200)
static int getbuttonstate()
{
	return !get_gpio(38);
}
#elif defined(HAVE_WNR2000)
static int getbuttonstate()
{
	return !get_gpio(40);
}
#elif defined(HAVE_WDR2543)
static int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_WHRHPGN)
static int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_WR941V6)
static int getbuttonstate()
{
	return !get_gpio(1);
}
#elif defined(HAVE_WR810N)
static int getbuttonstate()
{
	return !get_gpio(12);
}
#elif defined(HAVE_ARCHERC25)
static int getbuttonstate()
{
	return !get_gpio(1);
}
#elif defined(HAVE_WR841HPV3)
static int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_WR841V9)
static int getbuttonstate()
{
	return !get_gpio(12);
}
#elif defined(HAVE_DIR615I)
static int getbuttonstate()
{
	return !get_gpio(17);
}
#elif defined(HAVE_DIR615E)
static int getbuttonstate()
{
	return !get_gpio(8);
}
#elif defined(HAVE_WNDR3700)
static int getbuttonstate()
{
	return !get_gpio(8);
}
#elif defined(HAVE_DIR825)
static int getbuttonstate()
{
	return !get_gpio(3);
}
#elif defined(HAVE_WRT400)
static int getbuttonstate()
{
	return !get_gpio(8);
}
#elif defined(HAVE_WRT160NL)
static int getbuttonstate()
{
	return !get_gpio(21);
}
#elif defined(HAVE_TG2521)
static int getbuttonstate()
{
	return !get_gpio(21);
}
#elif defined(HAVE_TG1523)
static int getbuttonstate()
{
	return !get_gpio(0);
}
#elif defined(HAVE_WR941)
static int getbuttonstate()
{
	return !get_gpio(3);
}
#elif defined(HAVE_WR741V4)
static int getbuttonstate()
{
	return get_gpio(11);
}
#elif defined(HAVE_WR741)
static int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_WR1043)
static int getbuttonstate()
{
	return !get_gpio(3);
}
#elif defined(HAVE_WZRG300NH2)
static int getbuttonstate()
{
	return !get_gpio(1); // nxp multiplexer connected
}
#elif defined(HAVE_WZRG450)
static int getbuttonstate()
{
	return !get_gpio(6); // nxp multiplexer connected
}
#elif defined(HAVE_DIR632)
static int getbuttonstate()
{
	return !get_gpio(8); // nxp multiplexer connected
}
#elif defined(HAVE_WZRG300NH)
static int getbuttonstate()
{
	return !get_gpio(24); // nxp multiplexer connected
}
#elif defined(HAVE_WZRHPAG300NH)
static int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_TEW632BRP)
static int getbuttonstate()
{
	return !get_gpio(21);
}
#elif defined(HAVE_JA76PF)
static int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_ALFAAP94)
static int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_JWAP003)
static int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_ALFANX)
static int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_LSX)
static int getbuttonstate()
{
	return !get_gpio(8);
}
#elif defined(HAVE_WMBR_G300NH)
static int getbuttonstate()
{
	return !get_gpio(37);
}
#elif defined(HAVE_VF803)
static int getbuttonstate()
{
	return !get_gpio(28);
}
#elif defined(HAVE_SX763)
static int getbuttonstate()
{
	return get_gpio(14);
}
#endif
#if defined(HAVE_GATEWORX) || defined(HAVE_STORM)

#include <linux/mii.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#define u8 unsigned char
#define u32 unsigned int

#define GPIO_GET_BIT 0x0000001
#define GPIO_SET_BIT 0x0000005
#define GPIO_GET_CONFIG 0x0000003
#define GPIO_SET_CONFIG 0x0000004

#define IXP4XX_GPIO_OUT 0x1
#define IXP4XX_GPIO_IN 0x2

struct gpio_bit {
	unsigned char bit;
	unsigned char state;
};

char *filename = "/dev/gpio";

static int read_bit(int bit)
{
	int file;
	struct gpio_bit _bit;

	/* 
	 * open device 
	 */
	if ((file = open(filename, O_RDONLY)) == -1) {
		/* 
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		return 1;
	}

	/* 
	 * Config pin as input 
	 */
	_bit.bit = bit;
	_bit.state = IXP4XX_GPIO_IN;
	if (ioctl(file, GPIO_SET_CONFIG, (unsigned long)&_bit) < 0) {
		/* 
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		close(file);
		return 1;
	}

	/* 
	 * Read data 
	 */
	_bit.bit = bit;
	if (ioctl(file, GPIO_GET_BIT, (unsigned long)&_bit) < 0) {
		/* 
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		close(file);
		return 1;
	}

	close(file);
	return _bit.state;
}

static int isCompex(void)
{
	static int compex = -1;

	if (compex != -1)
		return compex;
	char filename2[64];

	sprintf(filename2, "/dev/mtdblock/%d", getMTD("RedBoot"));
	FILE *file = fopen(filename2, "r");

	if (file) {
		fseek(file, 0x1f800, SEEK_SET);
		unsigned int signature;

		fread(&signature, 4, 1, file);
		if (signature == 0x20021103) {
			compex = 1;
		} else {
			compex = 0;
		}
		fclose(file);
	}
	return compex;
}

static int isGW2369(void)
{
	if (brand == ROUTER_BOARD_GATEWORX_GW2369)
		return 1;
	return 0;
}

static int isGW2350(void)
{
	if (nvram_match("DD_BOARD", "Gateworks Cambria GW2350"))
		return 1;
	return 0;
}

static int getbuttonstate()
{
	FILE *in;
	int ret;

#ifdef HAVE_STORM
	ret = read_bit(60);
#elif HAVE_WG302V1
	ret = read_bit(13);
#elif HAVE_WG302
	ret = read_bit(3);
#elif HAVE_MI424WR
	ret = read_bit(10);
#elif HAVE_USR8200
	ret = read_bit(12);
#elif HAVE_CAMBRIA
	if (isGW2350())
		ret = read_bit(4);
	else
		ret = read_bit(20);
#else
	if (isCompex())
		ret = read_bit(0);
	if (isGW2369())
		ret = read_bit(3);
	else
		ret = read_bit(4);
#endif
#ifdef HAVE_TONZE
	return ret == 0 ? 0 : 1;
#else
	return ret == 0 ? 1 : 0;
#endif
}
#endif

static int mode = 0; /* mode 1 : pushed */
static int ses_mode = 0; /* mode 1 : pushed */
static int wifi_mode = 0; /* mode 1 : pushed */
static int _count = 0;

static int ses_pushed = 0;
static int wifi24_pushed = 0;
static int wifi5_pushed = 0;
static int initses = 1;

static void resetbtn_alarmtimer(unsigned long sec, unsigned long usec)
{
	struct itimerval itv;

	itv.it_value.tv_sec = sec;
	itv.it_value.tv_usec = usec;

	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

/*static int endswith(char *str, char *cmp)
{
	int cmp_len, str_len, i;

	cmp_len = strlen(cmp);
	str_len = strlen(str);
	if (cmp_len > str_len)
		return (0);
	for (i = 0; i < cmp_len; i++) {
		if (str[(str_len - 1) - i] != cmp[(cmp_len - 1) - i])
			return (0);
	}
	return (1);
}*/

static void runStartup(char *folder, char *extension)
{
	struct dirent *entry;
	DIR *directory;

	directory = opendir(folder);
	if (directory == NULL) {
		return;
	}
	// list all files in this directory
	while ((entry = readdir(directory)) != NULL) {
		if (endswith(entry->d_name, extension)) {
			sysprintf("%s/%s&\n", folder, entry->d_name);
		}
	}
	closedir(directory);
}

/* 
 * void system_reboot(void) { DEBUG("resetbutton: reboot\n"); alarmtimer(0,
 * 0); eval("reboot"); } 
 */

static void service_restart(void)
{
	DEBUG("resetbutton: restart\n");
	/* 
	 * Stop the timer alarm 
	 */
	resetbtn_alarmtimer(0, 0);
	/* 
	 * Reset the Diagnostic LED 
	 */
	diag_led(DIAG, START_LED); /* call from service.c */
	/* 
	 * Restart all of services 
	 */
	eval("rc", "restart");
}

static void handle_reset(void)
{
	if ((brand & 0x000f) != 0x000f) {
		dd_loginfo("resetbutton", "factory default.");
#if !defined(HAVE_XSCALE) && !defined(HAVE_MAGICBOX) && !defined(HAVE_FONERA) && !defined(HAVE_WHRAG108) &&                   \
	!defined(HAVE_GATEWORX) && !defined(HAVE_LS2) && !defined(HAVE_CA8) && !defined(HAVE_TW6600) && !defined(HAVE_LS5) && \
	!defined(HAVE_LSX) && !defined(HAVE_SOLO51)
		led_control(LED_DIAG, LED_ON);
#elif defined(HAVE_WHRHPGN) || defined(HAVE_WZRG300NH) || defined(HAVE_WZRHPAG300NH) || defined(HAVE_WZRG450)
		led_control(LED_DIAG, LED_ON);
#endif
		ACTION("ACT_HW_RESTORE");
		resetbtn_alarmtimer(0, 0); /* Stop the timer alarm */
#ifdef HAVE_X86
		eval("mount", "/usr/local", "-o", "remount,rw");
		eval("rm", "-f", "/tmp/nvram/*"); // delete nvram
		// database
		unlink("/tmp/nvram/.lock"); // delete
		// nvram
		// database
		eval("rm", "-f", "/usr/local/nvram/*"); // delete
		// nvram
		// database
		eval("mount", "/usr/local", "-o", "remount,ro");
#elif HAVE_NEWPORT
		eval("mount", "/usr/local", "-o", "remount,rw");
		eval("rm", "-f", "/tmp/nvram/*"); // delete nvram
		// database
		unlink("/tmp/nvram/.lock"); // delete
		// nvram
		// database
		eval("rm", "-f", "/usr/local/nvram/*"); // delete
		// nvram
		// database
		eval("mount", "/usr/local", "-o", "remount,ro");
#elif HAVE_RB500
		eval("rm", "-f", "/tmp/nvram/*"); // delete nvram
		// database
		unlink("/tmp/nvram/.lock"); // delete
		// nvram
		// database
		eval("rm", "-f", "/etc/nvram/*"); // delete nvram
		// database
#elif defined(HAVE_RB600) && !defined(HAVE_WDR4900)
		eval("rm", "-f", "/tmp/nvram/*"); // delete nvram
		// database
		unlink("/tmp/nvram/.lock"); // delete
		// nvram
		// database
		eval("rm", "-f", "/etc/nvram/*"); // delete nvram
		// database
#elif HAVE_MAGICBOX
		eval("rm", "-f", "/tmp/nvram/*"); // delete nvram
		unlink("/tmp/nvram/.lock"); // delete
		eval("erase", "nvram");
#else
#ifdef HAVE_BUFFALO_SA
		int region_sa = 0;
		if (nvram_default_match("region", "SA", ""))
			region_sa = 1;
#endif
		nvram_seti("sv_restore_defaults", 1);
		nvram_commit();
		eval("killall", "ledtool"); // stop blinking on
		// nvram_commit
#if !defined(HAVE_XSCALE) && !defined(HAVE_MAGICBOX) && !defined(HAVE_FONERA) && !defined(HAVE_WHRAG108) &&                   \
	!defined(HAVE_GATEWORX) && !defined(HAVE_LS2) && !defined(HAVE_CA8) && !defined(HAVE_TW6600) && !defined(HAVE_LS5) && \
	!defined(HAVE_LSX) && !defined(HAVE_SOLO51)
		led_control(LED_DIAG, LED_ON); // turn diag led on,
		// so we know reset
		// was pressed and
		// we're restoring
		// defaults.
#elif defined(HAVE_WHRHPGN) || defined(HAVE_WZRG300NH) || defined(HAVE_WZRHPAG300NH) || defined(HAVE_WZRG450)
		led_control(LED_DIAG, LED_ON);
#endif
#ifdef HAVE_BUFFALO_SA
		nvram_seti("sv_restore_defaults", 1);
		if (region_sa)
			nvram_set("region", "SA");
		nvram_commit();
#endif

		// nvram_set ("sv_restore_defaults", "1");
		// nvram_commit ();

		setWifiPass();

		kill(1, SIGTERM);
#endif
	}
}

static void control_wifi(int *wifi_mode, char *title, char *post, int i, int restart)
{
	switch (*wifi_mode) {
	case 1:
#ifdef HAVE_ERC
#ifdef HAVE_HORNET
		dd_syslog(LOG_DEBUG, "XXXXXXXX: TURN LED ON");
		set_gpio(1, 1);
#endif
#endif
		dd_syslog(LOG_DEBUG, "%s: turning radio(s) on", title);
		char on[32];
		sprintf(on, "radio_on%s", post);
		eval("restart", on);
		*wifi_mode = 0;

		break;
	case 0:
#ifdef HAVE_ERC
#ifdef HAVE_HORNET
		dd_syslog(LOG_DEBUG, "XXXXXXXX: TURN LED OFF");
		set_gpio(1, 0);
#endif
#endif
		// (AOSS) led
		dd_syslog(LOG_DEBUG, "%s: turning radio(s) off", title);
		char off[32];
		sprintf(off, "radio_off%s", post);
		eval("restart", off);
#ifdef HAVE_MADWIFI
		if (restart) {
			char dev[32];
			sprintf(dev, "wlan%d", i);
			eval("ifconfig", dev, "down");
			char *next;
			char var[80];
			char *vifs = nvram_nget("wlan%d_vifs", i);
			foreach(var, vifs, next)
			{
				eval("ifconfig", var, "down");
			}
		}
#endif
		*wifi_mode = 1;
		break;
	}
}

static void handle_wifi(void)
{
	led_control(LED_WLAN, LED_FLASH); // when pressed, blink white
	_count = 0;
	int dummy = wifi_mode;
	control_wifi(&dummy, "Wifi Button", "_0", 0, 0);
	control_wifi(&wifi_mode, "Wifi Button", "_1", 1, 1);
}

static void handle_wifi24(void)
{
	led_control(LED_WLAN, LED_FLASH); // when pressed, blink white
	_count = 0;
	control_wifi(&wifi_mode, "Wifi Button", "_0", 0, 1);
}

static void handle_wifi5(void)
{
	led_control(LED_WLAN, LED_FLASH); // when pressed, blink white
	_count = 0;
	control_wifi(&wifi_mode, "Wifi Button", "_1", 1, 1);
}

static void handle_ses(void)
{
	runStartup("/etc/config", ".sesbutton");
	runStartup("/usr/local/config", ".sesbutton");
	runStartup("/jffs/etc/config", ".sesbutton"); // if available
	runStartup("/mmc/etc/config", ".sesbutton"); // if available
	runStartup("/tmp/etc/config", ".sesbutton"); // if available
	_count = 0;
	if (nvram_matchi("usb_ses_umount", 1)) {
		led_control(LED_DIAG, LED_FLASH);
		runStartup("/etc/config", ".umount");
		sleep(5);
		led_control(LED_DIAG, LED_FLASH);
		sleep(1);
		led_control(LED_DIAG, LED_FLASH);
	}

	if (nvram_matchi("radiooff_button", 1)) {
		led_control(LED_SES, LED_FLASH); // when pressed, blink white
		int dummy = ses_mode;
		int dummy2 = ses_mode;
#ifndef HAVE_BUFFALO
		control_wifi(&dummy, "SES / AOSS / EZ-setup button", "_0", 0, 0);
		control_wifi(&dummy2, "SES / AOSS / EZ-setup button", "_1", 1, 1);
#else
		control_wifi(&dummy, "AOSS button", "_0", 0, 0);
		control_wifi(&dummy2, "AOSS button", "_1", 1, 1);
#endif

		switch (ses_mode) {
		case 1:
#ifdef HAVE_ERC
#ifdef HAVE_HORNET
			dd_syslog(LOG_DEBUG, "XXXXXXXX: TURN LED ON");
			set_gpio(0, 1);
			set_gpio(1, 1);
#endif
#endif
			ses_mode = 0;
			break;
		case 0:
#ifdef HAVE_ERC
#ifdef HAVE_HORNET
			dd_syslog(LOG_DEBUG, "XXXXXXXX: TURN LED OFF");
			set_gpio(0, 0);
			set_gpio(1, 0);
#endif
#endif
			ses_mode = 1;
			break;
		}

	}
#if defined(HAVE_AOSS) || defined(HAVE_WPS)
	else if (nvram_matchi("radiooff_button", 2)) {
		sysprintf("restart aoss");
	}
#endif
}

#define start_service_force_f(a) eval("startservice_f", a, "-f");

static void resetbtn_period_check(int sig)
{
	FILE *fp;
	unsigned int val = 0;

#ifdef HAVE_MINIDLNA
	static int dlna_counter = 0;
	if (nvram_match("dlna_enable", "1") && nvram_match("dlna_rescan", "1")) {
		dlna_counter++;

		if ((dlna_counter % 3600) == 0) {
			start_service_force_f("dlna_rescan");
		}
	}
#endif
	if (initses == 1 && nvram_matchi("radiooff_boot_off", 1) && nvram_matchi("radiooff_button", 1)) {
		ses_mode = 1;
		initses = 0;
	}

	// time_t t;

	// time(&t);
	// DEBUG("resetbutton: now time=%d\n", t);

#if defined(HAVE_IPQ806X) || defined(HAVE_MVEBU) || defined(HAVE_MAGICBOX) || defined(HAVE_FONERA) || defined(HAVE_WHRAG108) ||    \
	defined(HAVE_GATEWORX) || defined(HAVE_STORM) || defined(HAVE_LS2) || defined(HAVE_CA8) || defined(HAVE_TW6600) ||         \
	defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_WP54G) || defined(HAVE_NP28G) || defined(HAVE_SOLO51) ||            \
	defined(HAVE_OPENRISC) || defined(HAVE_DANUBE) || defined(HAVE_WDR4900) || defined(HAVE_VENTANA) || defined(HAVE_AC622) || \
	defined(HAVE_AC722) || defined(HAVE_EROUTER) || defined(HAVE_IPQ6018)
	val = getbuttonstate();
#ifdef HAVE_WRK54G
	val = !val;
#endif
#ifndef HAVE_ALPHA
#ifdef HAVE_USR5453
	val = !val;
#endif
#endif

#else
	switch (brand) {
	case ROUTER_BOARD_WCRGN:
	case ROUTER_BOARD_WHRG300N:
	case ROUTER_BOARD_DIR600B:
	case ROUTER_ASUS_RTN10PLUS:
	case ROUTER_BOARD_DIR615D:
	case ROUTER_BOARD_RT3352:
		val = (get_gpio(10) << 10) | (get_gpio(0) << 0);
		break;
	case ROUTER_BOARD_HAMEA15:
	case ROUTER_BOARD_EAP9550:
	case ROUTER_BOARD_ESR9752:
		val = get_gpio(0);
		break;
	case ROUTER_BOARD_ECB9750:
		val = get_gpio(11) << 11;
		break;
	case ROUTER_BOARD_NEPTUNE:
#ifdef HAVE_RUT500
		val = (get_gpio(10) << 10);
#else
		val = (get_gpio(10) << 10) | (get_gpio(0) << 0);
#endif
		break;
	case ROUTER_BOARD_F5D8235:
	case ROUTER_BOARD_BR6574N:
	case ROUTER_BOARD_ACXNR22:
	case ROUTER_BOARD_TECHNAXX3G:
	case ROUTER_BOARD_ESR6650:
	case ROUTER_BOARD_W502U:
	case ROUTER_BOARD_WR5422:
		val = get_gpio(10) << 10;
		break;
	case ROUTER_BOARD_RT15N:
		val = get_gpio(12) << 12;
		break;
	case ROUTER_BOARD_AR670W:
	case ROUTER_BOARD_AR690W:
		val = get_gpio(9) << 9;
		break;
	case ROUTER_WHR300HP2:
		val = (get_gpio(52) << 1) | (get_gpio(53) << 2);
		break;
	case ROUTER_BOARD_E1700:
	case ROUTER_DIR810L:
		val = (get_gpio(1) << 1) | (get_gpio(2) << 2);
		break;
	case ROUTER_DIR860LB1:
		val = (get_gpio(7) << 7) | (get_gpio(18) << 18);
		break;
	case ROUTER_DIR882:
		val = (get_gpio(7) << 7) | (get_gpio(15) << 15);
		break;
	case ROUTER_R6800:
		val = (get_gpio(18) << 18) | (get_gpio(12) << 12) | (get_gpio(14) << 14);
		break;
	case ROUTER_R6850:
		val = (get_gpio(7) << 7) | (get_gpio(14) << 14);
		break;
	case ROUTER_R6220:
		val = (get_gpio(7) << 7) | (get_gpio(8) << 8) | (get_gpio(14) << 14);
		break;
	case ROUTER_BOARD_GW2380:
	case ROUTER_BOARD_GW2388:
	case ROUTER_BOARD_GW6400:
#ifdef HAVE_LAGUNA
		val = !get_gpio(100);
#else
		val = get_gpio(240);
#endif
		break;
	default:
		if ((fp = fopen(GPIO_FILE, "r"))) {
#ifdef HAVE_XSCALE
			fscanf(fp, "%d", &val);
#else
			if (brand == ROUTER_NETGEAR_WGR614L) //gpio 7 power led shared with reset button
			{
				set_gpio(7, 1); //disable power led
				val = get_gpio(7) << 7; //read and shift value
				set_gpio(7, 0); //enable power led
			} else {
				fread(&val, 4, 1, fp);
			}

#endif
			fclose(fp);
		} else
			perror(GPIO_FILE);
		break;
	}
#endif
	DEBUG("resetbutton: GPIO = 0x%x\n", val);

	int gpio = 0;

	int state = 0;

#if defined(HAVE_IPQ806X) || defined(HAVE_MVEBU) || (HAVE_XSCALE) || defined(HAVE_MAGICBOX) || defined(HAVE_FONERA) ||             \
	defined(HAVE_WHRAG108) || defined(HAVE_GATEWORX) || defined(HAVE_STORM) || defined(HAVE_LS2) || defined(HAVE_CA8) ||       \
	defined(HAVE_TW6600) || defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_WP54G) || defined(HAVE_NP28G) ||            \
	defined(HAVE_SOLO51) || defined(HAVE_OPENRISC) || defined(HAVE_DANUBE) || defined(HAVE_UNIWIP) || defined(HAVE_EROUTER) || \
	defined(HAVE_VENTANA) || defined(HAVE_WDR4900) || defined(HAVE_IPQ6018)
	state = val;
	int sesgpio = 0xfff;
	int wifi24gpio = 0xfff;
	int wifi5gpio = 0xfff;
	int pushses;
	int pushwifi24;
	int pushwifi5;
#ifdef HAVE_WZRG300NH
	sesgpio = 0x117;
	val |= get_gpio(23) << 23; //aoss pushbutton
#elif defined(HAVE_WZR600DHP)
	sesgpio = 0x105;
	val |= get_gpio(5) << 5;
#elif defined(HAVE_WZRG300NH2)
	sesgpio = 0x10c;
	val |= get_gpio(12) << 12; //aoss pushbutton
#elif defined(HAVE_WMBR_G300NH)
	sesgpio = 0x100;
	val |= get_gpio(0); //aoss pushbutton
#elif defined(HAVE_WZRG450)
	sesgpio = 0x108;
	val |= get_gpio(8) << 8; //aoss pushbutton
#elif defined(HAVE_DW02_412H)
	sesgpio = 0x110;
	val |= get_gpio(16) << 16; //aoss pushbutton
#elif defined(HAVE_MVEBU)
	sesgpio = 0x101;
	if (brand == ROUTER_WRT_1900AC)
		val |= get_gpio(32) << 1; //aoss pushbutton
	else
		val |= get_gpio(24) << 1; //aoss pushbutton
#elif defined(HAVE_DIR632)
	sesgpio = 0x10c;
	val |= get_gpio(12) << 12; //aoss pushbutton
#elif defined(HAVE_WZRHPAG300NH)
	sesgpio = 0x105;
	val |= get_gpio(5) << 5; //aoss pushbutton
#elif defined(HAVE_CARAMBOLA)
#if defined(HAVE_ERC)
	//      sesgpio = 0x017;
	wifi24gpio = 0x117;
	val |= get_gpio(23) << 23;
#endif
#elif defined(HAVE_HORNET)
	sesgpio = 0x00b;
	val |= get_gpio(11) << 11; //aoss pushbutton
#elif defined(HAVE_RB2011)
//      sesgpio = 0x110;
//      val |= get_gpio(16) << 16;      //aoss pushbutton
#elif defined(HAVE_WDR4300)
//      sesgpio = 0x110;
//      val |= get_gpio(16) << 16;      //aoss pushbutton
#elif defined(HAVE_CPE880)
#elif defined(HAVE_WNDR3700V4)
	wifi24gpio = 0x10f;
	sesgpio = 0x10c;
	val |= get_gpio(15) << 15; //aoss pushbutton
	val |= get_gpio(12) << 12; //aoss pushbutton
#elif defined(HAVE_ARCHERC7V5)
	sesgpio = 0x102;
	val |= get_gpio(2) << 2; //aoss pushbutton
#elif defined(HAVE_WR1043V5)
	sesgpio = 0x105;
	val |= get_gpio(5) << 5; //aoss pushbutton
#elif defined(HAVE_WR1043V4)
	sesgpio = 0x101;
	val |= get_gpio(1) << 1; //aoss pushbutton
#elif defined(HAVE_WR1043V2)
	sesgpio = 0x111;
	val |= get_gpio(17) << 17; //aoss pushbutton
#elif defined(HAVE_WZR450HP2)
	sesgpio = 0x115;
	val |= get_gpio(21) << 21; //aoss pushbutton
#elif defined(HAVE_SR3200)
#elif defined(HAVE_CPE890)
#elif defined(HAVE_XD3200)
	sesgpio = 0x101;
	val |= get_gpio(1) << 1; //aoss pushbutton
#elif defined(HAVE_WR650AC)
#elif defined(HAVE_E355AC)
#elif defined(HAVE_WR615N)
#elif defined(HAVE_AP120C)
#elif defined(HAVE_E380AC)
#elif defined(HAVE_E325N)
#elif defined(HAVE_DIR869)
	sesgpio = 0x102;
	val |= get_gpio(2) << 2; //aoss pushbutton
#elif defined(HAVE_DIR859)
	sesgpio = 0x101;
	val |= get_gpio(1) << 1; //aoss pushbutton
#elif defined(HAVE_MMS344)

#elif defined(HAVE_DIR825C1)
	sesgpio = 0x110;
	val |= get_gpio(16) << 16; //aoss pushbutton
#elif defined(HAVE_PERU)
	//not
#elif defined(HAVE_WASP)
	sesgpio = 0x00b;
	val |= get_gpio(11) << 11; //aoss pushbutton
#elif defined(HAVE_WNR2200)
	sesgpio = 0x101; //not yet supported
	val |= get_gpio(37) << 1; //aoss pushbutton
#elif defined(HAVE_WNR2000)
	sesgpio = 0x00b;
	val |= get_gpio(11) << 11; //aoss pushbutton
#elif defined(HAVE_WDR2543)
	sesgpio = 0x10c;
	val |= get_gpio(12) << 12; //aoss pushbutton
#elif defined(HAVE_WHRHPGN)
	sesgpio = 0x10c;
	val |= get_gpio(12) << 12; //aoss pushbutton
#elif defined(HAVE_RT10N)
	sesgpio = 0x100;
	val |= get_gpio(0); //aoss pushbutton
#elif defined(HAVE_RT15N)
	sesgpio = 0x100;
	val |= get_gpio(0); //aoss pushbutton
#elif defined(HAVE_F5D8235)
	sesgpio = 0x100;
	val |= get_gpio(0); //aoss pushbutton
#elif defined(HAVE_WR5422)
	sesgpio = 0x100;
	val |= get_gpio(0); //aoss pushbutton
#elif defined(HAVE_DIR600)
	sesgpio = 0x100;
	val |= get_gpio(0); //aoss pushbutton
#elif defined(HAVE_WR941V6)
	sesgpio = 0x102;
	val |= get_gpio(2) << 2; //aoss pushbutton
#elif defined(HAVE_ARCHERC25)
	sesgpio = 0x116;
	val |= get_gpio(22) << 22; //aoss pushbutton
#elif defined(HAVE_WR841HPV3)
	sesgpio = 0x102;
	val |= get_gpio(2) << 2; //aoss pushbutton
	wifi24gpio = 0x100;
	val |= get_gpio(0);
#elif defined(HAVE_WR841V9)
	sesgpio = 0x111;
	val |= get_gpio(17) << 17; //aoss pushbutton
#elif defined(HAVE_DIR615I)
	sesgpio = 0x110;
	val |= get_gpio(16) << 16; //aoss pushbutton
#elif defined(HAVE_DIR615E)
	sesgpio = 0x10c;
	val |= get_gpio(12) << 12; //aoss pushbutton
#elif defined(HAVE_WR1043)
	sesgpio = 0x107;
	val |= get_gpio(7) << 7; //aoss pushbutton
#elif defined(HAVE_WR941)
	sesgpio = 0x107;
	val |= get_gpio(7) << 7; //aoss pushbutton
#elif defined(HAVE_MR3020)
	sesgpio = 0xfff;
#elif defined(HAVE_WR741V4)
	sesgpio = 0x01a;
	val |= get_gpio(26) << 26; //aoss pushbutton
#elif defined(HAVE_WR741)
	sesgpio = 0x10c;
	val |= get_gpio(12) << 12; //aoss pushbutton
#elif defined(HAVE_WRT400)
	sesgpio = 0x103;
	val |= get_gpio(3) << 3; //aoss pushbutton
#elif defined(HAVE_WNDR3700)
	wifi24gpio = 0x10b;
	sesgpio = 0x103;
	val |= get_gpio(3) << 3; //aoss pushbutton
	val |= get_gpio(11) << 11; //aoss pushbutton
#elif defined(HAVE_DIR825)
	sesgpio = 0x108;
	val |= get_gpio(8) << 8; //aoss pushbutton
#elif defined(HAVE_TG2521)
	sesgpio = 0x10c;
	val |= get_gpio(12) << 12; //aoss pushbutton
#elif defined(HAVE_OPENRISC)
	sesgpio = 0x005;
	val |= get_gpio(5) << 5; //aoss pushbutton
#elif defined(HAVE_IPQ6018)
	switch (brand) {
	case ROUTER_LINKSYS_MR7350:
		sesgpio = 0x105;
		val |= get_gpio(56) << 5;
		break;
	case ROUTER_LINKSYS_MX4200V1:
	case ROUTER_LINKSYS_MX4200V2:
	case ROUTER_LINKSYS_MX4300:
		sesgpio = 0x105;
		val |= get_gpio(67) << 5;
		break;
	case ROUTER_LINKSYS_MX5500:
	case ROUTER_LINKSYS_MR5500:
		wifi24gpio = 0x106;
		val |= get_gpio(27) << 6;
		sesgpio = 0x105;
		val |= get_gpio(24) << 5;
		break;
	case ROUTER_ASUS_AX89X:
		sesgpio = 0x105;
		val |= get_gpio(26) << 5;
		break;
	case ROUTER_DYNALINK_DLWRX36:
		sesgpio = 0x105;
		val |= get_gpio(63) << 5;
		break;
	case ROUTER_BUFFALO_WXR5950AX12:
		sesgpio = 0x105;
		val |= get_gpio(51) << 5;
		break;
	}
#elif defined(HAVE_IPQ806X)
	switch (brand) {
	case ROUTER_LINKSYS_EA8500:
		sesgpio = 0x105;
		wifi24gpio = 0x106;
		val |= get_gpio(65) << 5;
		val |= get_gpio(67) << 6;
		break;
	case ROUTER_ASROCK_G10:
		wifi5gpio = 0x105;
		wifi24gpio = 0x106;
		val |= get_gpio(65) << 5;
		val |= get_gpio(64) << 6;
		break;
	case ROUTER_NETGEAR_R7500:
	case ROUTER_NETGEAR_R7500V2:
	case ROUTER_NETGEAR_R7800:
		sesgpio = 0x105;
		wifi24gpio = 0x106;
		val |= get_gpio(6) << 5;
		val |= get_gpio(65) << 6;
		break;
	case ROUTER_NETGEAR_R9000:
		sesgpio = 0x105;
		wifi24gpio = 0x106;
		val |= get_gpio(32) << 5;
		val |= get_gpio(5) << 6;
		break;
	case ROUTER_ASUS_AC58U:
		sesgpio = 0x105;
		val |= get_gpio(63) << 5;
		break;
	case ROUTER_LINKSYS_EA8300:
		sesgpio = 0x105;
		val |= get_gpio(18) << 5;
		break;
	}
#endif

#ifdef HAVE_WRT160NL
	sesgpio = 0x107;
	val |= get_gpio(7) << 7; //wps/ses pushbutton
#endif
#ifdef HAVE_TEW632BRP
	sesgpio = 0x10c;
	val |= get_gpio(12) << 12; //wps/ses pushbutton
#endif
#ifdef HAVE_UNFY
	sesgpio = 0xfff;
#endif
#else
	if (brand > 0xffff) {
		if ((brand & 0x000ff) != 0x000ff)
			gpio = 1 << (brand & 0x000ff); // calculate gpio value.

		if ((brand & 0x00100) == 0) // check reset button polarity: 0
			// normal, 1 inversed
			state = (val & gpio);
		else
			state = !(val & gpio);
	} else {
		if ((brand & 0x000f) != 0x000f)
			gpio = 1 << (brand & 0x000f); // calculate gpio value.

		if ((brand & 0x0010) == 0) // check reset button polarity: 0
			// normal, 1 inversed
			state = (val & gpio);
		else
			state = !(val & gpio);
	}
	/* 
	 * 1 byte router's SES (AOSS) button gpio number and polarity; Eko
	 * 25.nov.06
	 * 
	 * R R R P N N N N = 0xXX ----- - ------- | | gpio num | | | |--- SES -
	 * AOSS button polarity (0: normal, 1 inversed) | |-------- reserved for
	 * future use
	 * 
	 * 0xff = button disabled / not available 
	 */
	int pushses;
	int pushwifi24;
	int pushwifi5;
	int sesgpio = 0xfff;
	int wifi24gpio = 0xfff;
	int wifi5gpio = 0xfff;

	switch (brand) {
	case ROUTER_BUFFALO_WHRG54S:
	case ROUTER_BUFFALO_WZRRSG54:
	case ROUTER_BUFFALO_WLI_TX4_G54HP:
		sesgpio = 0x100; // gpio 0, inversed
		break;
	case ROUTER_BUFFALO_WLA2G54C:
		sesgpio = 0x102; // gpio 2, inversed
		break;
	case ROUTER_BUFFALO_WBR2G54S:
		sesgpio = 0x004; // gpio 4, normal
		break;
	case ROUTER_BUFFALO_WZR600DHP2:
	case ROUTER_BUFFALO_WZR900DHP:
		sesgpio = 0x109; // gpio 9, inversed
		break;
	case ROUTER_BUFFALO_WZR1750:
		sesgpio = 0x10c; // gpio 12, inversed
		break;
	case ROUTER_BUFFALO_WXR1900DHP:
		sesgpio = 0x110; // gpio 16, inversed
		break;
	case ROUTER_D1800H:
		sesgpio = 0x10a; // gpio 10, inversed
		break;
#ifndef HAVE_BUFFALO
	case ROUTER_BOARD_WCRGN:
	case ROUTER_BOARD_DIR600B:
	case ROUTER_BOARD_RT3352:
#ifndef HAVE_RUT500
	case ROUTER_BOARD_NEPTUNE:
#endif
	case ROUTER_BOARD_DIR615D:
	case ROUTER_BOARD_WHRG300N:
	case ROUTER_ASUS_RTN10PLUS:
	case ROUTER_TPLINK_ARCHERC9:
	case ROUTER_TPLINK_ARCHERC8_V4:
	case ROUTER_ASUS_RTN12:
	case ROUTER_NETGEAR_WNR2000V2:
		sesgpio = 0x100;
		break;
	case ROUTER_TPLINK_ARCHERC8:
		sesgpio = 0x103;
		break;
	case ROUTER_TPLINK_ARCHERC3150:
		sesgpio = 0x103;
		break;
	case ROUTER_DIR882:
		sesgpio = 0x107;
		break;
	case ROUTER_R6800:
		sesgpio = 0x112;
		wifi24gpio = 0x10e;
		break;
	case ROUTER_R6850:
		sesgpio = 0x107;
		break;
	case ROUTER_R6220:
		sesgpio = 0x107;
		wifi24gpio = 0x108;
		break;
	case ROUTER_DIR860LB1:
		sesgpio = 0x112;
		break;
	case ROUTER_BOARD_E1700:
	case ROUTER_DIR810L:
	case ROUTER_WHR300HP2:
	case ROUTER_ASUS_RTN10:
	case ROUTER_LINKSYS_WTR54GS:
	case ROUTER_NETGEAR_WNDR4000:
	case ROUTER_NETGEAR_R6200:
		sesgpio = 0x102;
		break;
	case ROUTER_ASUS_WL500G_PRE:
	case ROUTER_ASUS_WL700GE:
		sesgpio = 0x004; // gpio 4, normal
		break;
	case ROUTER_ASUS_RTN10PLUSD1:
		sesgpio = 0x114; // gpio 20, inversed
		break;
	case ROUTER_LINKSYS_EA6900:
	case ROUTER_LINKSYS_EA6700:
	case ROUTER_LINKSYS_EA6350:
	case ROUTER_LINKSYS_EA6500V2:
	case ROUTER_TRENDNET_TEW812:
	case ROUTER_DLINK_DIR890:
	case ROUTER_DLINK_DIR880:
	case ROUTER_DLINK_DIR895:
	case ROUTER_TRENDNET_TEW828:
	case ROUTER_DLINK_DIR885:
	case ROUTER_DLINK_DIR868:
	case ROUTER_DLINK_DIR868C:
	case ROUTER_DLINK_DIR330:
		sesgpio = 0x107; // gpio 7, inversed
		break;
	case ROUTER_ASUS_AC67U:
		wifi24gpio = 0x10f;
		sesgpio = 0x107; // gpio 7, inversed
		break;
	case ROUTER_ASUS_AC87U:
		sesgpio = 0x102; // gpio 2, inversed
		wifi24gpio = 0x10f;
		break;
	case ROUTER_ASUS_AC88U:
	case ROUTER_ASUS_AC3100:
		sesgpio = 0x114; // gpio 20, inversed
		wifi24gpio = 0x112; // gpio 18, inversed
		break;
	case ROUTER_ASUS_AC1200:
		sesgpio = 0x109; // gpio 9, inversed
		break;
	case ROUTER_ASUS_AC5300:
		wifi24gpio = 0x114; // gpio 20, inversed
		sesgpio = 0x112; // gpio 18, inversed
		break;
	case ROUTER_ASUS_AC3200:
		sesgpio = 0x107; // gpio 2, inversed
		wifi24gpio = 0x104;
		break;
	case ROUTER_ASUS_AC56U:
		wifi24gpio = 0x107; // gpio 7, inversed
		sesgpio = 0x10f; // gpio 7, inversed
		break;
	case ROUTER_ASUS_RTN18U:
		sesgpio = 0x10b; // gpio 11, inversed
		break;
	case ROUTER_ASUS_WL550GE:
		sesgpio = 0x00f; // gpio 15, normal
		break;
	case ROUTER_DLINK_DIR860:
	case ROUTER_WRT310N:
	case ROUTER_WRT350N:
	case ROUTER_WRT610N:
	case ROUTER_ASUS_RTN16:
	case ROUTER_BELKIN_F7D3301:
	case ROUTER_BELKIN_F7D3302:
	case ROUTER_BELKIN_F7D4301:
	case ROUTER_BELKIN_F7D4302:
	case ROUTER_BELKIN_F5D8235V3:
	case ROUTER_LINKSYS_E3200:
		sesgpio = 0x108; // gpio 8, inversed
		break;
	case ROUTER_ASUS_WL500W:
		sesgpio = 0x007; // gpio 7, normal
		break;
	case ROUTER_ASUS_WL520GUGC:
	case ROUTER_ASUS_WL500G_PRE_V2:
		sesgpio = 0x103; // gpio 3, inversed
		break;
	case ROUTER_WAP54G_V3:
		sesgpio = 0x10e; // gpio 14, inversed
		break;
	case ROUTER_NETGEAR_WNDR3300:
		sesgpio = 0x101; // gpio 1, inversed
		break;
	case ROUTER_WRT54G_V81:
	case ROUTER_DLINK_DIR320:
	case ROUTER_WRT600N:
	case ROUTER_NETGEAR_WNDR3400:
	case ROUTER_NETGEAR_WNR3500L:
		sesgpio = 0x106; // gpio 6, inversed
		break;
	case ROUTER_NETGEAR_WNR3500LV2:
		sesgpio = 0x106; // gpio 6, inversed
		wifi24gpio = 0x108;
		break;
	case ROUTER_WRT320N:
	case ROUTER_WRT160NV3:
	case ROUTER_NETGEAR_WNDR4500:
	case ROUTER_NETGEAR_WNDR4500V2:
	case ROUTER_NETGEAR_R6300:
	case ROUTER_NETGEAR_AC1450:
	case ROUTER_NETGEAR_R6250:
	case ROUTER_NETGEAR_R6300V2:
	case ROUTER_NETGEAR_R7000:
	case ROUTER_NETGEAR_R7000P:
		wifi24gpio = 0x105;
		//fall through
	case ROUTER_WRT54G:
	case ROUTER_WRT54G_V8:
	case ROUTER_WRTSL54GS:
	case ROUTER_WRT150N:
	case ROUTER_WRT160N:
	case ROUTER_WRT300N:
	case ROUTER_WRT300NV11:
	case ROUTER_WRT610NV2:
	case ROUTER_ASKEY_RT220XD: // not soldered
	case ROUTER_DYNEX_DX_NRUTER:
	case ROUTER_LINKSYS_E4200:
	case ROUTER_ASUS_RTN66:
	case ROUTER_ASUS_AC66U:
		sesgpio = 0x104;
		break;
	case ROUTER_NETGEAR_R6400:
	case ROUTER_NETGEAR_R6400V2:
		sesgpio = 0x103;
		wifi24gpio = 0x104;
		break;
	case ROUTER_NETGEAR_R8000:
		sesgpio = 0x105;
		wifi24gpio = 0x104;
		break;
	case ROUTER_NETGEAR_R8500:
		sesgpio = 0x104;
		wifi24gpio = 0x113;
		break;
	case ROUTER_LINKSYS_EA6500:
	case ROUTER_NETGEAR_EX6200:
		wifi24gpio = 0x104;
		break;
	case ROUTER_WRT310NV2:
		sesgpio = 0x105; // gpio 5, inversed
		break;
	case ROUTER_LINKSYS_E800:
	case ROUTER_LINKSYS_E900:
	case ROUTER_LINKSYS_E1000V2:
	case ROUTER_LINKSYS_E1500:
	case ROUTER_LINKSYS_E1550:
	case ROUTER_LINKSYS_E2500:
		sesgpio = 0x109; // gpio 9, inversed
		break;

#endif
	default:
		sesgpio = 0xfff; // gpio unknown, disabled
		wifi5gpio = 0xfff; // gpio unknown, disabled
		wifi24gpio = 0xfff; // gpio unknown, disabled
	}
#endif

	pushses = 1 << (sesgpio & 0x0ff); // calculate push value from ses gpio
	pushwifi24 = 1 << (wifi24gpio & 0x0ff); // calculate push value from ses gpio
	pushwifi5 = 1 << (wifi5gpio & 0x0ff); // calculate push value from ses gpio
	//
	//
	//
	// pin no.

	/* 
	 * The value is zero during button-pushed. 
	 */
	int action = check_action();
	if (state && nvram_matchi("resetbutton_enable", 1)) {
		DEBUG("resetbutton: mode=%d, count=%d\n", mode, _count);

		if (mode == 0) {
			/* 
			 * We detect button pushed first time 
			 */
			resetbtn_alarmtimer(0, URGENT_INTERVAL);
			mode = 1;
		}
		if (++_count > RESET_WAIT_COUNT) {
			if (action != ACT_IDLE) { // Don't execute during upgrading
				dd_loginfo("resetbutton", "nothing to do...");
				resetbtn_alarmtimer(0, 0); /* Stop the timer alarm */
				return;
			}
			handle_reset();
		}
	} else if ((sesgpio != 0xfff) &&
		   (((sesgpio & 0x100) == 0 && (val & pushses)) || ((sesgpio & 0x100) == 0x100 && !(val & pushses)))) {
		if (!ses_pushed && (++_count > SES_WAIT)) {
			if (action != ACT_IDLE) { // Don't execute during upgrading
				dd_loginfo("resetbutton", "nothing to do...");
				resetbtn_alarmtimer(0, 0); /* Stop the timer alarm */
				return;
			}
			_count = 0;
			ses_pushed = 1;
			handle_ses();
		}
	} else if ((wifi24gpio != 0xfff && wifi5gpio == 0xfff) &&
		   (((wifi24gpio & 0x100) == 0 && (val & pushwifi24)) || ((wifi24gpio & 0x100) == 0x100 && !(val & pushwifi24)))) {
		if (!wifi24_pushed && (++_count > SES_WAIT)) {
			if (action != ACT_IDLE) { // Don't execute during upgrading
				dd_loginfo("resetbutton", "nothing to do...");
				resetbtn_alarmtimer(0, 0); /* Stop the timer alarm */
				return;
			}
			_count = 0;
			wifi24_pushed = 1;
			handle_wifi();
		}

	} else if ((wifi24gpio != 0xfff && wifi5gpio != 0xfff) &&
		   (((wifi24gpio & 0x100) == 0 && (val & pushwifi24)) || ((wifi24gpio & 0x100) == 0x100 && !(val & pushwifi24)))) {
		if (!wifi24_pushed && (++_count > SES_WAIT)) {
			if (action != ACT_IDLE) { // Don't execute during upgrading
				dd_loginfo("resetbutton", "nothing to do...");
				resetbtn_alarmtimer(0, 0); /* Stop the timer alarm */
				return;
			}
			_count = 0;
			wifi24_pushed = 1;
			handle_wifi24();
		}
	} else if ((wifi24gpio != 0xfff && wifi5gpio != 0xfff) &&
		   (((wifi5gpio & 0x100) == 0 && (val & pushwifi5)) || ((wifi5gpio & 0x100) == 0x100 && !(val & pushwifi5)))) {
		if (!wifi5_pushed && (++_count > SES_WAIT)) {
			if (action != ACT_IDLE) { // Don't execute during upgrading
				dd_loginfo("resetbutton", "nothing to do...");
				resetbtn_alarmtimer(0, 0); /* Stop the timer alarm */
				return;
			}
			_count = 0;
			wifi5_pushed = 1;
			handle_wifi5();
		}

	} else {
		_count = 0; // reset counter to avoid factory default
		wifi24_pushed = 0;
		wifi5_pushed = 0;
		ses_pushed = 0;
		/* 
		 * Although it's unpushed now, it had ever been pushed 
		 */
		if (mode == 1) {
			//                      fprintf(stderr, "[RESETBUTTON] released %d\n", count);
			resetbtn_alarmtimer(NORMAL_INTERVAL, 0);
			mode = 0;
#ifdef HAVE_UNFY
			if (_count > UPGRADE_WAIT_COUNT) {
				char *upgrade_script = "firmware_upgrade.sh";
				char call[32];
				dd_loginfo("resetbutton", "[RESETBUTTON] check:%d count:%d", pidof(upgrade_script), _count);
				if (pidof(upgrade_script) < 0) {
					sprintf(call, "/%s/%s", nvram_safe_get("fw_upgrade_dir"), upgrade_script);
					if (f_exists(call)) {
						dd_loginfo("resetbutton", "[RESETBUTTON] trigger update script: %s", call);
						system(call);
					} else {
						dd_loginfo("resetbutton", "[RESETBUTTON] upgrade script not found");
						led_control(LED_DIAG, LED_OFF);
					}
				}
			}
#else
			if (action != ACT_IDLE) { // Don't execute during upgrading
				dd_loginfo("resetbutton", "nothing to do...");
				resetbtn_alarmtimer(0, 0); /* Stop the timer alarm */
				return;
			}
#endif
		}
	}
}

int main(int argc, char *argv[])
{
	brand = getRouterBrand();
#ifndef HAVE_MI424WR
#if !defined(HAVE_NOP8670) && !defined(HAVE_TONZE)
	if (((brand < 0x10000) && (brand & 0x000f) == 0x000f) || ((brand > 0xffff) && (brand & 0x00ff) == 0x00ff))
#endif
	{
		puts("sorry, your unit does not support resetbutton feature\n");
		return 0;
	}
#endif
#ifndef HAVE_NOP8670
#ifdef HAVE_MAGICBOX
	init_gpio();
#endif
	/* 
	 * Run it under background 
	 */
	switch (fork()) {
	case -1:
		DEBUG("can't fork\n");
		_exit(0);
		break;
	case 0:
		/* 
		 * child process 
		 */
		DEBUG("fork ok\n");
		(void)setsid();
		break;
	default:
		/* 
		 * parent process should just die 
		 */
		_exit(0);
	}

	/* 
	 * set the signal handler 
	 */
	signal(SIGALRM, resetbtn_period_check);

	/* 
	 * set timer 
	 */
	resetbtn_alarmtimer(NORMAL_INTERVAL, 0);

	/* 
	 * Most of time it goes to sleep 
	 */
	while (1)
		pause();

	return 0;
#endif
}

#undef DEBUG
