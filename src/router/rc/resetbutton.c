
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

#define	SES_LED_CHECK_TIMES	"9999"	/* How many times to check? */
#define	SES_LED_CHECK_INTERVAL	"1"	/* Wait interval seconds */
#define RESET_WAIT		3	/* seconds */
#define RESET_WAIT_COUNT	RESET_WAIT * 10	/* 10 times a second */
#ifdef HAVE_ERC
#define SES_WAIT		5	/* seconds */
#else
#define SES_WAIT		3	/* seconds */
#endif
#define SES_WAIT_COUNT	SES_WAIT	/* 10 times a second */
#ifdef HAVE_UNFY
#define UPGRADE_WAIT		1	/* seconds */
#define UPGRADE_WAIT_COUNT	UPGRADE_WAIT * 10 - 5
#endif

#define NORMAL_INTERVAL		1	/* second */
#define URGENT_INTERVAL		100 * 1000	/* microsecond */

#ifndef HAVE_GATEWORX		/* 1/10 second */
#define GPIO_FILE		"/dev/gpio/in"
#endif
#if 0
#define DEBUG printf
#else
#define DEBUG(format, args...)
#endif

#ifdef HAVE_MAGICBOX
#include <sys/mman.h>

#define GPIO0_OR   0x0700	/* rw, output */
#define GPIO0_TCR  0x0704	/* rw, three-state control */
#define GPIO0_ODR  0x0718	/* rw, open drain */
#define GPIO0_IR   0x071c	/* ro, input */
#define GPIO0_BASE 0xef600000	/* page */

#define GPIO_LED    0x20000000	/* GPIO1 */
#define GPIO_BUTTON 0x40000000	/* GPIO2 */

#define REG(buf, offset) ((unsigned int *)((void *)buf + offset))

static unsigned int *page;
static int fd;

void init_gpio()
{
	void *start = 0;

	fd = open("/dev/mem", O_RDWR);
	if (fd < 0) {
		// dd_syslog(LOG_ERR, "Can't open /dev/mem: %s", strerror(errno));
		exit(1);
	}

	page = mmap(start, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t) GPIO0_BASE);
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

int getbuttonstate()
{
	return (*REG(page, GPIO0_IR) & GPIO_BUTTON) == 0;

}
#endif

#if defined(HAVE_FONERA) || defined(HAVE_WHRAG108) || defined(HAVE_LS2) || defined(HAVE_CA8) || defined(HAVE_TW6600)  || defined(HAVE_LS5) || defined(HAVE_WP54G) || defined(HAVE_NP28G) || defined(HAVE_SOLO51) || defined(HAVE_OPENRISC)
int getbuttonstate()
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
	return  get_gpio(4);
#elif HAVE_OPENRISC
	return get_gpio(0);
#else
	return get_gpio(6);
#endif
}
#elif defined(HAVE_VENTANA)
int getbuttonstate()
{
	return !get_gpio(496);
}
#elif defined(HAVE_E200)
int getbuttonstate()
{
	return !get_gpio(0);
}
#elif defined(HAVE_EROUTER)
int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_UNIWIP)
int getbuttonstate()
{
	return !get_gpio(232);
}
#elif defined(HAVE_WDR4900)
int getbuttonstate()
{
	return get_gpio(3);
}
#elif defined(HAVE_MVEBU)
int getbuttonstate()
{
	int ret;
	if (getRouterBrand() == ROUTER_WRT_1900AC)
		ret = get_gpio(33);
	else
		ret = get_gpio(29);
	return !ret;
}
#elif defined(HAVE_IPQ806X)
int getbuttonstate()
{
	int ret = 0;
	if (getRouterBrand() == ROUTER_NETGEAR_R7500)
		ret = get_gpio(54);
	else if (getRouterBrand() == ROUTER_TRENDNET_TEW827)
		ret = get_gpio(54);
	else if (getRouterBrand() == ROUTER_LINKSYS_EA8500)
		ret = get_gpio(68);
	else
		return 0;
	return !ret;
}
#elif defined(HAVE_DAP3410)
int getbuttonstate()
{
	return !get_gpio(17);
}
#elif defined(HAVE_UBNTM)
int getbuttonstate()
{
	int brand = getRouterBrand();
	if (brand == ROUTER_UBNT_UAPAC)
		return !get_gpio(2);
	return !get_gpio(12);
}
#elif defined(HAVE_RB2011)
int getbuttonstate()
{
	return 0;
}
#elif defined(HAVE_WDR4300)
int getbuttonstate()
{
	return !get_gpio(16);
}
#elif defined(HAVE_WNDR3700V4)
int getbuttonstate()
{
	return !get_gpio(21);
}
#elif defined(HAVE_DIR862)
int getbuttonstate()
{
	return !get_gpio(17);

}
#elif defined(HAVE_MMS344)
int getbuttonstate()
{
	return 0;
	int ret = get_gpio(12);

	if (ret == 0)
		return 1;
	return 0;
}
#elif defined(HAVE_WR1043V2)
int getbuttonstate()
{
	return !get_gpio(16);
}
#elif defined(HAVE_WZR450HP2)
int getbuttonstate()
{
	return !get_gpio(17);
}
#elif defined(HAVE_DIR859)
int getbuttonstate()
{
	return !get_gpio(2);
}
#elif defined(HAVE_JWAP606)
int getbuttonstate()
{
	return !get_gpio(15);
}
#elif defined(HAVE_DIR825C1)
int getbuttonstate()
{
	return !get_gpio(17);
}
#elif defined(HAVE_WASP)
int getbuttonstate()
{
	return !get_gpio(12);
}
#elif defined(HAVE_CARAMBOLA)
#ifdef HAVE_ERC
int getbuttonstate()
{
	return get_gpio(12);

}
#else
int getbuttonstate()
{
	return !get_gpio(11);
}
#endif
#elif defined(HAVE_HORNET)
int getbuttonstate()
{
	return !get_gpio(12);
}
#elif defined(HAVE_WNR2200)
int getbuttonstate()
{
	return !get_gpio(38);
}
#elif defined(HAVE_WNR2000)
int getbuttonstate()
{
	return !get_gpio(40);
}
#elif defined(HAVE_WDR2543)
int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_WHRHPGN)
int getbuttonstate()
{
	return !get_gpio(11);

}
#elif defined(HAVE_DAP3320)
int getbuttonstate()
{
	return !get_gpio(12);
}
#elif defined(HAVE_DAP2330)
int getbuttonstate()
{
	return !get_gpio(17);
}
#elif defined(HAVE_DAP2230)
int getbuttonstate()
{
	return !get_gpio(17);
}
#elif defined(HAVE_WR841V9)
int getbuttonstate()
{
	return !get_gpio(12);
}
#elif defined(HAVE_DIR615I)
int getbuttonstate()
{
	return !get_gpio(17);
}
#elif defined(HAVE_DIR615E)
int getbuttonstate()
{
	return !get_gpio(8);
}
#elif defined(HAVE_WNDR3700)
int getbuttonstate()
{
	return !get_gpio(8);
}
#elif defined(HAVE_DIR825)
int getbuttonstate()
{
	return !get_gpio(3);

}
#elif defined(HAVE_WRT400)
int getbuttonstate()
{
	return !get_gpio(8);
}
#elif defined(HAVE_WRT160NL)
int getbuttonstate()
{
	return !get_gpio(21);
}
#elif defined(HAVE_TG2521)
int getbuttonstate()
{
	return !get_gpio(21);

}
#elif defined(HAVE_TG1523)
int getbuttonstate()
{
	return !get_gpio(0);

}
#elif defined(HAVE_WR941)
int getbuttonstate()
{
	return !get_gpio(3);

}
#elif defined(HAVE_WR741V4)
int getbuttonstate()
{
	return get_gpio(11);
}
#elif defined(HAVE_WR741)
int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_WR1043)
int getbuttonstate()
{
	return !get_gpio(3);

}
#elif defined(HAVE_WZRG300NH2)
int getbuttonstate()
{
	return !get_gpio(1);	// nxp multiplexer connected
}
#elif defined(HAVE_WZRG450)
int getbuttonstate()
{
	return !get_gpio(6);	// nxp multiplexer connected
}
#elif defined(HAVE_DIR632)
int getbuttonstate()
{
	return !get_gpio(8);	// nxp multiplexer connected

}
#elif defined(HAVE_WZRG300NH)
int getbuttonstate()
{
	return !get_gpio(24);	// nxp multiplexer connected
}
#elif defined(HAVE_WZRHPAG300NH)
int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_TEW632BRP)
int getbuttonstate()
{
	return !get_gpio(21);
}
#elif defined(HAVE_JA76PF)
int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_ALFAAP94)
int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_JWAP003)
int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_ALFANX)
int getbuttonstate()
{
	return !get_gpio(11);
}
#elif defined(HAVE_LSX)
int getbuttonstate()
{
	return !get_gpio(8);
}
#elif defined(HAVE_WMBR_G300NH)
int getbuttonstate()
{
	return !get_gpio(37);
}
#elif defined(HAVE_VF803)
int getbuttonstate()
{
	return !get_gpio(28);
}
#elif defined(HAVE_SX763)
int getbuttonstate()
{
	return get_gpio(14);
}
#endif
#if defined(HAVE_GATEWORX) || defined (HAVE_STORM)

#include <linux/mii.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#define u8 unsigned char
#define u32 unsigned int

#define GPIO_GET_BIT	0x0000001
#define GPIO_SET_BIT	0x0000005
#define GPIO_GET_CONFIG	0x0000003
#define GPIO_SET_CONFIG 0x0000004

#define IXP4XX_GPIO_OUT 		0x1
#define IXP4XX_GPIO_IN  		0x2

struct gpio_bit {
	unsigned char bit;
	unsigned char state;
};

char *filename = "/dev/gpio";

int read_bit(int bit)
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
		return 1;
	}

	close(file);
	return _bit.state;
}

int isCompex(void)
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

int isGW2369(void)
{

	int brand = getRouterBrand();
	if (brand == ROUTER_BOARD_GATEWORX_GW2369)
		return 1;
	return 0;
}

int isGW2350(void)
{
	if (nvram_match("DD_BOARD", "Gateworks Cambria GW2350")
	    || nvram_match("DD_BOARD2", "Gateworks Cambria GW2350"))
		return 1;
	return 0;
}

int getbuttonstate()
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

static int mode = 0;		/* mode 1 : pushed */
static int ses_mode = 0;	/* mode 1 : pushed */
static int wifi_mode = 0;	/* mode 1 : pushed */
static int count = 0;

static int ses_pushed = 0;
static int wifi_pushed = 0;
#ifdef HAVE_RADIOOFF
static int initses = 1;
#endif

static int brand;

static void alarmtimer(unsigned long sec, unsigned long usec)
{
	struct itimerval itv;

	itv.it_value.tv_sec = sec;
	itv.it_value.tv_usec = usec;

	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

int endswith(char *str, char *cmp)
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
}

void runStartup(char *folder, char *extension)
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

void service_restart(void)
{
	DEBUG("resetbutton: restart\n");
	/* 
	 * Stop the timer alarm 
	 */
	alarmtimer(0, 0);
	/* 
	 * Reset the Diagnostic LED 
	 */
	diag_led(DIAG, START_LED);	/* call from service.c */
	/* 
	 * Restart all of services 
	 */
	eval("rc", "restart");
}

static void handle_reset(void)
{

	if ((brand & 0x000f) != 0x000f) {
		fprintf(stderr, "resetbutton: factory default.\n");
		dd_syslog(LOG_DEBUG, "Reset button: restoring factory defaults now!\n");
#if !defined(HAVE_XSCALE) && !defined(HAVE_MAGICBOX) && !defined(HAVE_FONERA) && !defined(HAVE_WHRAG108) && !defined(HAVE_GATEWORX) && !defined(HAVE_LS2) && !defined(HAVE_CA8) && !defined(HAVE_TW6600) && !defined(HAVE_LS5) && !defined(HAVE_LSX) && !defined(HAVE_SOLO51)
		led_control(LED_DIAG, LED_ON);
#elif defined(HAVE_WHRHPGN)  || defined(HAVE_WZRG300NH) || defined(HAVE_WZRHPAG300NH) || defined(HAVE_WZRG450)
		led_control(LED_DIAG, LED_ON);
#endif
		ACTION("ACT_HW_RESTORE");
		alarmtimer(0, 0);	/* Stop the timer alarm */
#ifdef HAVE_X86
		eval("mount", "/usr/local", "-o", "remount,rw");
		eval("rm", "-f", "/tmp/nvram/*");	// delete nvram
		// database
		unlink("/tmp/nvram/.lock");	// delete
		// nvram
		// database
		eval("rm", "-f", "/usr/local/nvram/*");	// delete
		// nvram
		// database
		eval("mount", "/usr/local", "-o", "remount,ro");
#elif HAVE_RB500
		eval("rm", "-f", "/tmp/nvram/*");	// delete nvram
		// database
		unlink("/tmp/nvram/.lock");	// delete
		// nvram
		// database
		eval("rm", "-f", "/etc/nvram/*");	// delete nvram
		// database
#elif HAVE_MAGICBOX
		eval("rm", "-f", "/tmp/nvram/*");	// delete nvram
		// database
		unlink("/tmp/nvram/.lock");	// delete
		// nvram
		// database
		eval("erase", "nvram");
#else
#ifdef HAVE_BUFFALO_SA
		int region_sa = 0;
		if (nvram_default_match("region", "SA", ""))
			region_sa = 1;
#endif
		nvram_set("sv_restore_defaults", "1");
		nvram_commit();
		eval("killall", "ledtool");	// stop blinking on
		// nvram_commit
#if !defined(HAVE_XSCALE) && !defined(HAVE_MAGICBOX) && !defined(HAVE_FONERA) && !defined(HAVE_WHRAG108) && !defined(HAVE_GATEWORX) && !defined(HAVE_LS2) && !defined(HAVE_CA8) && !defined(HAVE_TW6600) && !defined(HAVE_LS5) && !defined(HAVE_LSX) && !defined(HAVE_SOLO51)
		led_control(LED_DIAG, LED_ON);	// turn diag led on,
		// so we know reset
		// was pressed and
		// we're restoring
		// defaults.
#elif defined(HAVE_WHRHPGN) || defined(HAVE_WZRG300NH) || defined(HAVE_WZRHPAG300NH) || defined(HAVE_WZRG450)
		led_control(LED_DIAG, LED_ON);
#endif
#ifdef HAVE_BUFFALO_SA
		nvram_set("sv_restore_defaults", "1");
		if (region_sa)
			nvram_set("region", "SA");
		nvram_commit();
#endif

		// nvram_set ("sv_restore_defaults", "1");
		// nvram_commit ();

		kill(1, SIGTERM);
#endif
	}

}

static void handle_wifi(void)
{

	led_control(LED_WLAN, LED_FLASH);	// when pressed, blink white
	count = 0;
	switch (wifi_mode) {
	case 1:
		dd_syslog(LOG_DEBUG, "Wifi button: turning radio(s) on\n");
		sysprintf("startstop radio_on");
		wifi_mode = 0;
		break;
	case 0:
		// (AOSS) led
		dd_syslog(LOG_DEBUG, "Wifi button: turning radio(s) off\n");
		sysprintf("startstop radio_off");
		wifi_mode = 1;
		break;
	}

}

static void handle_ses(void)
{

	runStartup("/etc/config", ".sesbutton");
	runStartup("/jffs/etc/config", ".sesbutton");	// if available
	runStartup("/mmc/etc/config", ".sesbutton");	// if available
	runStartup("/tmp/etc/config", ".sesbutton");	// if available
	count = 0;
	if (nvram_match("usb_ses_umount", "1")) {
		led_control(LED_DIAG, LED_FLASH);
		runStartup("/etc/config", ".umount");
		sleep(5);
		led_control(LED_DIAG, LED_FLASH);
		sleep(1);
		led_control(LED_DIAG, LED_FLASH);
	}

	if (nvram_match("radiooff_button", "1")) {
		led_control(LED_SES, LED_FLASH);	// when pressed, blink white
		switch (ses_mode) {

		case 1:
			// SES (AOSS) led
#ifdef HAVE_RADIOOFF
#ifndef HAVE_BUFFALO
			dd_syslog(LOG_DEBUG, "SES / AOSS / EZ-setup button: turning radio(s) on\n");
#else
			dd_syslog(LOG_DEBUG, "AOSS button: turning radio(s) on\n");
#endif
#ifndef HAVE_ERC
			sysprintf("startstop radio_on");
#endif
#endif

			ses_mode = 0;
			break;
		case 0:

			// (AOSS) led
#ifdef HAVE_RADIOOFF
#ifndef HAVE_BUFFALO
			dd_syslog(LOG_DEBUG, "SES / AOSS / EZ-setup button: turning radio(s) off\n");
#else
			dd_syslog(LOG_DEBUG, "AOSS button: turning radio(s) off\n");
#endif
#ifndef HAVE_ERC
			sysprintf("startstop radio_off");
#endif
#endif

			ses_mode = 1;
			break;
		}

	}
#if defined(HAVE_AOSS) || defined(HAVE_WPS)
	else if (nvram_match("radiooff_button", "2")) {
		sysprintf("startstop aoss");
	}
#endif

}

void period_check(int sig)
{
	FILE *fp;
	unsigned int val = 0;

#ifdef HAVE_RADIOOFF
	if (initses == 1 && nvram_match("radiooff_boot_off", "1")
	    && nvram_match("radiooff_button", "1")) {
		ses_mode = 1;
		initses = 0;
	}
#endif

	// time_t t;

	// time(&t);
	// DEBUG("resetbutton: now time=%d\n", t);

#if defined(HAVE_IPQ806X) || defined(HAVE_MVEBU) || defined(HAVE_MAGICBOX) || defined(HAVE_FONERA) || defined(HAVE_WHRAG108) || defined(HAVE_GATEWORX) || defined(HAVE_STORM) || defined(HAVE_LS2) || defined(HAVE_CA8) || defined(HAVE_TW6600)  || defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_WP54G) || defined(HAVE_NP28G) || defined(HAVE_SOLO51) || defined(HAVE_OPENRISC) || defined(HAVE_DANUBE) || defined(HAVE_WDR4900) || defined(HAVE_VENTANA) || defined(HAVE_AC622) || defined(HAVE_AC722) || defined(HAVE_EROUTER)
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
	if (brand == ROUTER_BOARD_WCRGN) {
		val = (get_gpio(10) << 10) | (get_gpio(0) << 0);
	} else if (brand == ROUTER_BOARD_WHRG300N) {
		val = (get_gpio(10) << 10) | (get_gpio(0) << 0);
	} else if (brand == ROUTER_BOARD_HAMEA15) {
		val = get_gpio(0);
	} else if (brand == ROUTER_BOARD_ECB9750) {
		val = get_gpio(11) << 11;
	} else if (brand == ROUTER_BOARD_NEPTUNE) {
		val = (get_gpio(10) << 10) | (get_gpio(0) << 0);
	} else if (brand == ROUTER_BOARD_RT3352) {
		val = (get_gpio(10) << 10) | (get_gpio(0) << 0);
	} else if (brand == ROUTER_BOARD_WR5422) {
		val = get_gpio(10) << 10;
	} else if (brand == ROUTER_BOARD_DIR600B) {
		val = (get_gpio(10) << 10) | (get_gpio(0) << 0);
	} else if (brand == ROUTER_BOARD_F5D8235) {
		val = get_gpio(10) << 10;
	} else if (brand == ROUTER_ASUS_RTN10PLUS) {
		val = get_gpio(10) << 10 | (get_gpio(0) << 0);
	} else if (brand == ROUTER_BOARD_RT15N) {
		val = get_gpio(12) << 12;
	} else if (brand == ROUTER_BOARD_DIR615D) {
		val = (get_gpio(10) << 10) | (get_gpio(0) << 0);
	} else if (brand == ROUTER_BOARD_ESR6650) {
		val = get_gpio(10) << 10;
	} else if (brand == ROUTER_BOARD_EAP9550) {
		val = get_gpio(0);
	} else if (brand == ROUTER_BOARD_ESR9752) {
		val = get_gpio(0);
	} else if (brand == ROUTER_BOARD_AR670W) {
		val = get_gpio(9) << 9;
	} else if (brand == ROUTER_BOARD_AR690W) {
		val = get_gpio(9) << 9;
	} else if (brand == ROUTER_BOARD_BR6574N) {
		val = get_gpio(10) << 10;
	} else if (brand == ROUTER_BOARD_ACXNR22) {
		val = get_gpio(10) << 10;
	} else if (brand == ROUTER_BOARD_TECHNAXX3G) {
		val = get_gpio(10) << 10;
	} else if (brand == ROUTER_WHR300HP2) {
		val = (get_gpio(52) << 1) | (get_gpio(53) << 2);
	} else if (brand == ROUTER_BOARD_E1700) {
		val = (get_gpio(1) << 1) | (get_gpio(2) << 2);
	} else if (brand == ROUTER_DIR810L) {
		val = (get_gpio(1) << 1) | (get_gpio(2) << 2);
	} else if (brand == ROUTER_DIR860LB1) {
		val = (get_gpio(7) << 7) | (get_gpio(18) << 18);
	} else if (brand == ROUTER_BOARD_W502U) {
		val = get_gpio(10) << 10;
	} else if (brand == ROUTER_BOARD_GW2380) {
		val = get_gpio(240);
	} else if (brand == ROUTER_BOARD_GW2388) {
		val = get_gpio(240);
	} else {

		if ((fp = fopen(GPIO_FILE, "r"))) {
#ifdef HAVE_XSCALE
			fscanf(fp, "%d", &val);
#else
			if (brand == ROUTER_NETGEAR_WGR614L)	//gpio 7 power led shared with reset button
			{
				set_gpio(7, 1);	//disable power led
				val = get_gpio(7) << 7;	//read and shift value
				set_gpio(7, 0);	//enable power led
			} else {
				fread(&val, 4, 1, fp);
			}

#endif
			fclose(fp);
		} else
			perror(GPIO_FILE);
	}
#endif
	DEBUG("resetbutton: GPIO = 0x%x\n", val);

	int gpio = 0;

	int state = 0;

#if defined(HAVE_IPQ806X) || defined(HAVE_MVEBU) || (HAVE_XSCALE) || defined(HAVE_MAGICBOX) || defined(HAVE_FONERA) || defined(HAVE_WHRAG108) || defined(HAVE_GATEWORX) || defined(HAVE_STORM) || defined(HAVE_LS2) || defined(HAVE_CA8) || defined(HAVE_TW6600)  || defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_WP54G) || defined(HAVE_NP28G) || defined(HAVE_SOLO51) || defined(HAVE_OPENRISC) || defined(HAVE_DANUBE) || defined(HAVE_UNIWIP) || defined(HAVE_EROUTER) || defined(HAVE_VENTANA)
	state = val;
	int sesgpio = 0xfff;
	int wifigpio = 0xfff;
	int pushses;
	int pushwifi;
#ifdef HAVE_WZRG300NH
	sesgpio = 0x117;
	val |= get_gpio(23) << 23;	//aoss pushbutton
#elif defined(HAVE_WZR600DHP)
	sesgpio = 0x105;
	val |= get_gpio(5) << 5;
#elif defined(HAVE_WZRG300NH2)
	sesgpio = 0x10c;
	val |= get_gpio(12) << 12;	//aoss pushbutton
#elif defined(HAVE_WMBR_G300NH)
	sesgpio = 0x100;
	val |= get_gpio(0);	//aoss pushbutton
#elif defined(HAVE_WZRG450)
	sesgpio = 0x108;
	val |= get_gpio(8) << 8;	//aoss pushbutton
#elif defined(HAVE_MVEBU)
	sesgpio = 0x101;
	if (getRouterBrand() == ROUTER_WRT_1900AC)
		val |= get_gpio(32) << 1;	//aoss pushbutton
	else
		val |= get_gpio(24) << 1;	//aoss pushbutton
#elif defined(HAVE_DIR632)
	sesgpio = 0x10c;
	val |= get_gpio(12) << 12;	//aoss pushbutton
#elif defined(HAVE_WZRHPAG300NH)
	sesgpio = 0x105;
	val |= get_gpio(5) << 5;	//aoss pushbutton
#elif defined(HAVE_CARAMBOLA)
#if defined(HAVE_ERC)
	wifigpio = 0x117;
	val |= get_gpio(23) << 23;
#endif
#elif defined(HAVE_HORNET)
	sesgpio = 0x00b;
	val |= get_gpio(11) << 11;	//aoss pushbutton
#elif defined(HAVE_RB2011)
//      sesgpio = 0x110;
//      val |= get_gpio(16) << 16;      //aoss pushbutton
#elif defined(HAVE_WDR4300)
//      sesgpio = 0x110;
//      val |= get_gpio(16) << 16;      //aoss pushbutton
#elif defined(HAVE_WNDR3700V4)
	wifigpio = 0x10f;
	sesgpio = 0x10c;
	val |= get_gpio(15) << 15;	//aoss pushbutton
	val |= get_gpio(12) << 12;	//aoss pushbutton
#elif defined(HAVE_WR1043V2)
	sesgpio = 0x111;
	val |= get_gpio(17) << 17;	//aoss pushbutton
#elif defined(HAVE_WZR450HP2)
	sesgpio = 0x115;
	val |= get_gpio(21) << 21;	//aoss pushbutton
#elif defined(HAVE_DIR859)
	sesgpio = 0x101;
	val |= get_gpio(1) << 1;	//aoss pushbutton
#elif defined(HAVE_MMS344)

#elif defined(HAVE_DIR825C1)
	sesgpio = 0x110;
	val |= get_gpio(16) << 16;	//aoss pushbutton
#elif defined(HAVE_WASP)
	sesgpio = 0x00b;
	val |= get_gpio(11) << 11;	//aoss pushbutton
#elif defined(HAVE_WNR2200)
	sesgpio = 0x101;	//not yet supported
	val |= get_gpio(37) << 1;	//aoss pushbutton
#elif defined(HAVE_WNR2000)
	sesgpio = 0x00b;
	val |= get_gpio(11) << 11;	//aoss pushbutton
#elif defined(HAVE_WDR2543)
	sesgpio = 0x10c;
	val |= get_gpio(12) << 12;	//aoss pushbutton
#elif defined(HAVE_WHRHPGN)
	sesgpio = 0x10c;
	val |= get_gpio(12) << 12;	//aoss pushbutton
#elif defined(HAVE_RT10N)
	sesgpio = 0x100;
	val |= get_gpio(0);	//aoss pushbutton
#elif defined(HAVE_RT15N)
	sesgpio = 0x100;
	val |= get_gpio(0);	//aoss pushbutton
#elif defined(HAVE_F5D8235)
	sesgpio = 0x100;
	val |= get_gpio(0);	//aoss pushbutton
#elif defined(HAVE_WR5422)
	sesgpio = 0x100;
	val |= get_gpio(0);	//aoss pushbutton
#elif defined(HAVE_DIR600)
	sesgpio = 0x100;
	val |= get_gpio(0);	//aoss pushbutton
#elif defined(HAVE_WR841V9)
	sesgpio = 0x111;
	val |= get_gpio(17) << 17;	//aoss pushbutton
#elif defined(HAVE_DIR615I)
	sesgpio = 0x110;
	val |= get_gpio(16) << 16;	//aoss pushbutton
#elif defined(HAVE_DIR615E)
	sesgpio = 0x10c;
	val |= get_gpio(12) << 12;	//aoss pushbutton
#elif defined(HAVE_WR1043)
	sesgpio = 0x107;
	val |= get_gpio(7) << 7;	//aoss pushbutton
#elif defined(HAVE_WR941)
	sesgpio = 0x107;
	val |= get_gpio(7) << 7;	//aoss pushbutton
#elif defined(HAVE_MR3020)
	sesgpio = 0xfff;
#elif defined(HAVE_WR741V4)
	sesgpio = 0x01a;
	val |= get_gpio(26) << 26;	//aoss pushbutton
#elif defined(HAVE_WR741)
	sesgpio = 0x10c;
	val |= get_gpio(12) << 12;	//aoss pushbutton
#elif defined(HAVE_WRT400)
	sesgpio = 0x103;
	val |= get_gpio(3) << 3;	//aoss pushbutton
#elif defined(HAVE_WNDR3700)
	wifigpio = 0x10b;
	sesgpio = 0x103;
	val |= get_gpio(3) << 3;	//aoss pushbutton
	val |= get_gpio(11) << 11;	//aoss pushbutton
#elif defined(HAVE_DIR825)
	sesgpio = 0x108;
	val |= get_gpio(8) << 8;	//aoss pushbutton
#elif defined(HAVE_TG2521)
	sesgpio = 0x10c;
	val |= get_gpio(12) << 12;	//aoss pushbutton
#elif defined(HAVE_OPENRISC)
	sesgpio = 0x005;
	val |= get_gpio(5) << 5;	//aoss pushbutton
#endif
#ifdef HAVE_WRT160NL
	sesgpio = 0x107;
	val |= get_gpio(7) << 7;	//wps/ses pushbutton
#endif
#ifdef HAVE_TEW632BRP
	sesgpio = 0x10c;
	val |= get_gpio(12) << 12;	//wps/ses pushbutton
#endif
#ifdef HAVE_UNFY
	sesgpio = 0xfff;
#endif
#else
	if (brand > 0xffff) {
		if ((brand & 0x000ff) != 0x000ff)
			gpio = 1 << (brand & 0x000ff);	// calculate gpio value.

		if ((brand & 0x00100) == 0)	// check reset button polarity: 0
			// normal, 1 inversed
			state = (val & gpio);
		else
			state = !(val & gpio);
	} else {

		if ((brand & 0x000f) != 0x000f)
			gpio = 1 << (brand & 0x000f);	// calculate gpio value.

		if ((brand & 0x0010) == 0)	// check reset button polarity: 0
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
	int pushwifi;
	int sesgpio;
	int wifigpio = 0xfff;

	switch (brand) {
	case ROUTER_BUFFALO_WHRG54S:
	case ROUTER_BUFFALO_WZRRSG54:
	case ROUTER_BUFFALO_WLI_TX4_G54HP:
		sesgpio = 0x100;	// gpio 0, inversed
		break;
	case ROUTER_BUFFALO_WLA2G54C:
		sesgpio = 0x102;	// gpio 2, inversed
		break;
	case ROUTER_BUFFALO_WBR2G54S:
		sesgpio = 0x004;	// gpio 4, normal
		break;
	case ROUTER_BUFFALO_WZR600DHP2:
	case ROUTER_BUFFALO_WZR900DHP:
		sesgpio = 0x109;	// gpio 9, inversed
		break;
	case ROUTER_BUFFALO_WZR1750:
		sesgpio = 0x10c;	// gpio 12, inversed
		break;
	case ROUTER_BUFFALO_WXR1900DHP:
		sesgpio = 0x110;	// gpio 16, inversed
		break;
	case ROUTER_D1800H:
		sesgpio = 0x10a;	// gpio 10, inversed
		break;
#ifndef HAVE_BUFFALO
	case ROUTER_BOARD_WCRGN:
	case ROUTER_BOARD_DIR600B:
	case ROUTER_BOARD_RT3352:
	case ROUTER_BOARD_NEPTUNE:
	case ROUTER_BOARD_DIR615D:
	case ROUTER_BOARD_WHRG300N:
	case ROUTER_ASUS_RTN10PLUS:
	case ROUTER_TPLINK_ARCHERC9:
		sesgpio = 0x100;
		break;
	case ROUTER_DIR860LB1:
		sesgpio = 0x112;
		break;
	case ROUTER_BOARD_E1700:
	case ROUTER_DIR810L:
		sesgpio = 0x102;
		break;
	case ROUTER_WHR300HP2:
		sesgpio = 0x102;
		break;
	case ROUTER_ASUS_WL700GE:
		sesgpio = 0x004;	// gpio 4, normal
		break;
	case ROUTER_ASUS_RTN10PLUSD1:
		sesgpio = 0x114;	// gpio 20, inversed
		break;
	case ROUTER_ASUS_RTN10:
		sesgpio = 0x102;	// gpio 2, inversed
		break;
	case ROUTER_ASUS_RTN12:
	case ROUTER_NETGEAR_WNR2000V2:
		sesgpio = 0x100;	// gpio 0, inversed
		break;
	case ROUTER_LINKSYS_WTR54GS:
	case ROUTER_NETGEAR_WNDR4000:
		sesgpio = 0x102;	// gpio 2, inversed
		break;
	case ROUTER_WRT54G:
	case ROUTER_WRT54G_V8:
	case ROUTER_WRTSL54GS:
	case ROUTER_WRT150N:
	case ROUTER_WRT160N:
	case ROUTER_WRT300N:
	case ROUTER_WRT300NV11:
	case ROUTER_WRT610NV2:
	case ROUTER_ASKEY_RT220XD:	// not soldered
	case ROUTER_DYNEX_DX_NRUTER:
	case ROUTER_LINKSYS_E4200:
	case ROUTER_ASUS_RTN66:
		sesgpio = 0x104;	// gpio 4, inversed
		break;
	case ROUTER_ASUS_AC66U:
		sesgpio = 0x104;	// gpio 4, inversed
		break;
	case ROUTER_LINKSYS_EA6900:
	case ROUTER_LINKSYS_EA6700:
	case ROUTER_LINKSYS_EA6350:
	case ROUTER_LINKSYS_EA6500V2:
	case ROUTER_TRENDNET_TEW812:
		sesgpio = 0x107;	// gpio 7, inversed
		break;
	case ROUTER_DLINK_DIR890:
	case ROUTER_DLINK_DIR880:
	case ROUTER_DLINK_DIR895:
	case ROUTER_TRENDNET_TEW828:
		sesgpio = 0x107;	// gpio 7, inversed
		break;
	case ROUTER_DLINK_DIR885:
		sesgpio = 0x107;	// gpio 7, inversed
//              wifigpio = 0x10a;       // gpio 10, inversed
		break;
	case ROUTER_DLINK_DIR868:
	case ROUTER_DLINK_DIR868C:
		sesgpio = 0x107;	// gpio 7, inversed
		break;
	case ROUTER_ASUS_AC67U:
		wifigpio = 0x10f;
		sesgpio = 0x107;	// gpio 7, inversed
		break;
	case ROUTER_DLINK_DIR860:
		sesgpio = 0x108;	// gpio 7, inversed
		break;
	case ROUTER_ASUS_AC87U:
		sesgpio = 0x102;	// gpio 2, inversed
		wifigpio = 0x10f;
		break;
	case ROUTER_ASUS_AC88U:
		sesgpio = 0x114;	// gpio 20, inversed
		wifigpio = 0x112;	// gpio 18, inversed
		break;
	case ROUTER_ASUS_AC5300:
		wifigpio = 0x114;	// gpio 20, inversed
		sesgpio = 0x112;	// gpio 18, inversed
		break;
	case ROUTER_ASUS_AC3200:
		sesgpio = 0x107;	// gpio 2, inversed
		wifigpio = 0x104;
		break;
	case ROUTER_ASUS_AC56U:
		wifigpio = 0x107;	// gpio 7, inversed
		sesgpio = 0x10f;	// gpio 7, inversed
		break;
	case ROUTER_ASUS_RTN18U:
		sesgpio = 0x10b;	// gpio 11, inversed
		break;
	case ROUTER_ASUS_WL500G_PRE:
		sesgpio = 0x004;	// gpio 4, normal
		break;
	case ROUTER_ASUS_WL550GE:
		sesgpio = 0x00f;	// gpio 15, normal
		break;
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
		sesgpio = 0x108;	// gpio 8, inversed
		break;
	case ROUTER_ASUS_WL500W:
		sesgpio = 0x007;	// gpio 7, normal
		break;
	case ROUTER_DLINK_DIR330:
		sesgpio = 0x107;	// gpio 7, inversed
		break;
	case ROUTER_ASUS_WL520GUGC:
	case ROUTER_ASUS_WL500G_PRE_V2:
		sesgpio = 0x103;	// gpio 3, inversed
		break;
	case ROUTER_WAP54G_V3:
		sesgpio = 0x10e;	// gpio 14, inversed
		break;
	case ROUTER_NETGEAR_WNDR3300:
		sesgpio = 0x101;	// gpio 1, inversed
		break;
	case ROUTER_WRT54G_V81:
	case ROUTER_DLINK_DIR320:
	case ROUTER_WRT600N:
	case ROUTER_NETGEAR_WNDR3400:
	case ROUTER_NETGEAR_WNR3500L:
		sesgpio = 0x106;	// gpio 6, inversed
		break;
	case ROUTER_NETGEAR_WNR3500LV2:
		sesgpio = 0x106;	// gpio 6, inversed
		wifigpio = 0x108;
		break;
	case ROUTER_WRT320N:
	case ROUTER_WRT160NV3:
	case ROUTER_NETGEAR_WNDR4500:
	case ROUTER_NETGEAR_WNDR4500V2:
	case ROUTER_NETGEAR_R6300:
		sesgpio = 0x104;
		wifigpio = 0x105;
		break;
	case ROUTER_NETGEAR_AC1450:
		sesgpio = 0x104;
		wifigpio = 0x105;
		break;
	case ROUTER_NETGEAR_R6250:
		sesgpio = 0x104;
		wifigpio = 0x105;
		break;
	case ROUTER_NETGEAR_R6300V2:
		sesgpio = 0x104;
		wifigpio = 0x105;
		break;
	case ROUTER_NETGEAR_R7000:
		sesgpio = 0x104;
		wifigpio = 0x105;
		break;
	case ROUTER_NETGEAR_R7500:
		wifigpio = 0x106;
		break;
	case ROUTER_NETGEAR_R8000:
		sesgpio = 0x105;
		wifigpio = 0x104;
		break;
	case ROUTER_NETGEAR_R8500:
		sesgpio = 0x104;
		wifigpio = 0x113;
		break;
	case ROUTER_NETGEAR_EX6200:
		wifigpio = 0x104;
		break;
	case ROUTER_WRT310NV2:
		sesgpio = 0x105;	// gpio 5, inversed
		break;
	case ROUTER_LINKSYS_E800:
	case ROUTER_LINKSYS_E900:
	case ROUTER_LINKSYS_E1000V2:
	case ROUTER_LINKSYS_E1500:
	case ROUTER_LINKSYS_E1550:
	case ROUTER_LINKSYS_E2500:
		sesgpio = 0x109;	// gpio 9, inversed
		break;
	case ROUTER_LINKSYS_EA6500:
		sesgpio = 0x104;	// gpio 4, inversed
		break;
	case ROUTER_LINKSYS_EA8500:
		sesgpio = 0x165;
		break;

#endif
	default:
		sesgpio = 0xfff;	// gpio unknown, disabled
		wifigpio = 0xfff;	// gpio unknown, disabled
	}
#endif

	pushses = 1 << (sesgpio & 0x0ff);	// calculate push value from ses gpio
	pushwifi = 1 << (wifigpio & 0x0ff);	// calculate push value from ses gpio 
	// 
	// 
	// 
	// pin no.

	/* 
	 * The value is zero during button-pushed. 
	 */
	if (state && nvram_match("resetbutton_enable", "1")) {
		DEBUG("resetbutton: mode=%d, count=%d\n", mode, count);

		if (mode == 0) {
			/* 
			 * We detect button pushed first time 
			 */
			alarmtimer(0, URGENT_INTERVAL);
			mode = 1;
		}
		if (++count > RESET_WAIT_COUNT) {
			if (check_action() != ACT_IDLE) {	// Don't execute during upgrading
				fprintf(stderr, "resetbutton: nothing to do...\n");
				alarmtimer(0, 0);	/* Stop the timer alarm */
				return;
			}
			handle_reset();
		}
	} else if ((sesgpio != 0xfff) && (((sesgpio & 0x100) == 0 && (val & pushses)) || ((sesgpio & 0x100) == 0x100 && !(val & pushses)))) {
		if (!ses_pushed && (++count > SES_WAIT)) {
			if (check_action() != ACT_IDLE) {	// Don't execute during upgrading
				fprintf(stderr, "resetbutton: nothing to do...\n");
				alarmtimer(0, 0);	/* Stop the timer alarm */
				return;
			}
			count = 0;
			ses_pushed = 1;
			handle_ses();
		}
	} else if ((wifigpio != 0xfff) && (((wifigpio & 0x100) == 0 && (val & pushwifi)) || ((wifigpio & 0x100) == 0x100 && !(val & pushwifi)))) {
		if (!wifi_pushed && (++count > SES_WAIT)) {
			if (check_action() != ACT_IDLE) {	// Don't execute during upgrading
				fprintf(stderr, "resetbutton: nothing to do...\n");
				alarmtimer(0, 0);	/* Stop the timer alarm */
				return;
			}
			count = 0;
			wifi_pushed = 1;
			handle_wifi();
		}

	} else {
		count = 0;	// reset counter to avoid factory default
		wifi_pushed = 0;
		ses_pushed = 0;
		/* 
		 * Although it's unpushed now, it had ever been pushed 
		 */
		if (mode == 1) {
//                      fprintf(stderr, "[RESETBUTTON] released %d\n", count);
			alarmtimer(NORMAL_INTERVAL, 0);
			mode = 0;
#ifdef HAVE_UNFY
			if (count > UPGRADE_WAIT_COUNT) {

				char *upgrade_script = "firmware_upgrade.sh";
				char call[32];
				fprintf(stderr, "[RESETBUTTON] check:%d count:%d\n", pidof(upgrade_script), count);
				if (pidof(upgrade_script) < 0) {
					sprintf(call, "/%s/%s", nvram_safe_get("fw_upgrade_dir"), upgrade_script);
					if (f_exists(call)) {
						fprintf(stderr, "[RESETBUTTON] trigger update script: %s\n", call);
						system(call);
					} else {
						fprintf(stderr, "[RESETBUTTON] upgrade script not found\n");
						led_control(LED_DIAG, LED_OFF);
					}
				}
			}
#else
			if (check_action() != ACT_IDLE) {	// Don't execute during upgrading
				fprintf(stderr, "resetbutton: nothing to do...\n");
				alarmtimer(0, 0);	/* Stop the timer alarm */
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
	if (brand <= 0xffff && (brand & 0x000f) == 0x000f)
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
		exit(0);
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
	signal(SIGALRM, period_check);

	/* 
	 * set timer 
	 */
	alarmtimer(NORMAL_INTERVAL, 0);

	/* 
	 * Most of time it goes to sleep 
	 */
	while (1)
		pause();

	return 0;
#endif
}
