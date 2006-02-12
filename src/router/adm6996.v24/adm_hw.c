/*
 *       adm6996.c  -- ADM6996L linux interface driver.
 *	Copyright (c) 2004 Nikki Chumakov (nikki@gattaca.ru)
 */
#define ADMDRIVER
#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/segment.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>

#include "adm6996.h"


#include <typedefs.h>
#include <osl.h>
#include <sbutils.h>
#include <bcmutils.h>
#include <bcmendian.h>

#if 0
#define GPIO0 0
#define GPIO1 1
#define GPIO2 2
#define GPIO3 3
#define GPIO4 4
#define GPIO5 5
#define GPIO6 6
#define GPIO7 7
#define GPIO8 8

#define B_RESET   1<<GPIO0
#define B_ECS   1<<GPIO2
#define B_ECK   1<<GPIO3
#define B_EDO   1<<GPIO4
#define B_EDI   1<<GPIO5

#define EEDO_PIN  4
#define EECS_PIN  2
#define EECK_PIN  3
#define EEDI_PIN  5
#define RESET_PIN 0
#endif

// #include <et_dbg.h>

/* Minimum timing constants */
#define EECK_EDGE_TIME	3	/* 3us - max(adm 2.5us, 93c 1us) */
#define EEDI_SETUP_TIME	1	/* 1us - max(adm 10ns, 93c 400ns) */
#define EECS_SETUP_TIME	1	/* 1us - max(adm no, 93c 200ns) */

/* Forward declarations */
adm_info_t *adm_attach(void *sbh); // , char *vars);
void adm_detach(adm_info_t *adm);
void adm_enable_device(adm_info_t *adm, char *vars);
int adm_config_vlan(adm_info_t *adm, char *vars);

#define OLDDRIVER 1

#ifdef OLDDRIVER
#define sb_gpioouten2(a,b,c,d) sb_gpioouten(a,b,c,d)
#define sb_gpioout2(a,b,c,d) sb_gpioout(a,b,c,d)
#define sb_gpiointmask2(a,b,c,d) sb_gpioout(a,b,c,d)
#endif

//extern uint32 sb_gpioouten(void *sbh, uint32 mask, uint32 val,uint32 flags);
//extern uint32 sb_gpioout(void *sbh, uint32 mask, uint32 val,uint32 flags);
//extern uint32 sb_gpioin(void *sbh);
//extern uint32 sb_gpiointmask(void *sbh, uint32 mask, uint32 val,uint32 flags);

#define OUTENMASK B_RESET|B_ECS|B_ECK|B_EDI
#define CFGMASK   B_ECS|B_ECK|B_EDI
#define BIT(x)  (1 << (x))
#define ASSERT(exp)   do {} while (0)

#if 0
void
conf_gpio(int x)
{
  ASSERT(sbh);

  /* Enable all of output pins */
  sb_gpioouten2(sbh, OUTENMASK, OUTENMASK,0);

  /* We don't want the B_RESET pin changed, unless
   * it tries to set the B_RESET pin.
   */
  if (x & B_RESET)
    sb_gpioout2(sbh, OUTENMASK, x,0);
  else
    sb_gpioout2(sbh, CFGMASK, x,0);

}

void
gpio_line_set(int x, unsigned int value)
{
  ASSERT(sbh);

  if (value == 1)
    sb_gpioout2(sbh, BIT(x), BIT(x));
  else if (value == 0)
    sb_gpioout2(sbh, BIT(x), 0);
}

void
gpio_line_get(int x, int *value)
{
  ASSERT(sbh);

  *value = (sb_gpioin(sbh) >> x) & 0x1;
}
void
gpio_line_config_in(int x)
{
  ASSERT(sbh);

  sb_gpioouten2(sbh, BIT(x), 0,0);
  sb_gpiointmask2(sbh, BIT(x), BIT(x),0);
}

void
gpio_line_config_out(int x)
{
  ASSERT(sbh);

  sb_gpiointmask(sbh, BIT(x), 0,0);
  sb_gpioouten2(sbh, BIT(x), BIT(x),0);
}

void
gpio_line_config_out_all(void)
{
  ASSERT(sbh);

  sb_gpioouten2(sbh, OUTENMASK, OUTENMASK,0);
}

inline static void SerialDelay(int count)
{
	udelay(count);
}

static void InitSerialInterface(void)
{
	gpio_line_set(EECK_PIN, 0);
	gpio_line_set(EEDI_PIN, 0);
}
#endif
/* Return gpio number assigned to the named pin */
/*
* Variable should be in format:
*
*	gpio<N>=pin_name
*
* 'def_gpio' is returned if there is no such variable is found.
*/
#define NUMGPIO	16
static uint8
adm_get_gpio(char *vars, char *pin_name, uint8 def_gpio)
{
	char name[] = "gpioXXXXXXXX";
	char *val;
	uint8 i;

	/* Go thru all possibilities till a match in pin name */
	for (i = 0; i < NUMGPIO; i ++) {
		sprintf(name, "gpio%d", i);
		val = getvar(vars, name);
		if (val && !strcmp(val, pin_name))
			return (1 << i);
	}
	return def_gpio;
}

/* Allocate private resource */
adm_info_t *
adm_attach(void *sbh) // , char *vars)
{
	adm_info_t *adm;

	/* Allocate private data */
	if (!(adm = kmalloc(sizeof(adm_info_t),GFP_ATOMIC))) {
		printk("adm_attach: out of memory");
		return NULL;
	}
	bzero((char *) adm, sizeof(adm_info_t));
	adm->sbh = sbh;

	/* Init GPIO mapping. Default GPIO: 2, 3, 4 */
	adm->eecs = (1 << 2);
	adm->eesk = (1 << 3);
	adm->eedi = (1 << 5);

#if 0
	/* nvram overrides */
	adm->eecs = adm_get_gpio(vars, "adm_eecs", adm->eecs);
	adm->eesk = adm_get_gpio(vars, "adm_eesk", adm->eesk);
	adm->eedi = adm_get_gpio(vars, "adm_eedi", adm->eedi);
#endif

	return adm;
}

/* Release private resource */
void
adm_detach(adm_info_t *adm)
{
	/* Free private data */
	kfree(adm);
	
	//MFREE(adm, sizeof(adm_info_t));
}

/*
* The following local functions provide chip access control. The 
* general rules in writing these supporting routines are:
*
*   1. EECS should be kept low after each routine.
*   2. EESK should be kept low after each routine.
*/


/* Enable outputs with specified value to the chip */
static void
adm_enout(adm_info_t *adm, uint8 pins, uint8 val)
{
	/* Prepare GPIO output value */
	sb_gpioout2(adm->sbh, pins, val,0);
	/* Enable GPIO outputs */
	sb_gpioouten2(adm->sbh, pins, pins,0);
	OSL_DELAY(EECK_EDGE_TIME);
}

/* Disable outputs to the chip */
static void
adm_disout(adm_info_t *adm, uint8 pins)
{
	/* Disable GPIO outputs */
	sb_gpioouten2(adm->sbh, pins, 0,0);
	OSL_DELAY(EECK_EDGE_TIME);
}

/* Advance clock(s) */
static void
adm_adclk(adm_info_t *adm, int clocks)
{
	int i;
	for (i = 0; i < clocks; i ++) {
		/* Clock high */
		sb_gpioout2(adm->sbh, adm->eesk, adm->eesk,0);
		OSL_DELAY(EECK_EDGE_TIME);
		/* Clock low */
		sb_gpioout2(adm->sbh, adm->eesk, 0,0);
		OSL_DELAY(EECK_EDGE_TIME);
	}
}	

/* Read a bit stream to the chip */
static void
adm_read(adm_info_t *adm, uint8 cs, uint8 *rbuf, uint rbits)
{
	uint i, len = (rbits + 7) / 8;
	uint8 mask;

	if (cs)
		sb_gpioout2(adm->sbh, adm->eecs, adm->eecs,0);
	
	/* read sequence */
	/* Byte assemble from MSB to LSB */
	for (i = 0; i < len; i++) {
		uint8 byte;
		/* Bit bang from MSB to LSB */
		for (mask = 0x80, byte = 0; mask && rbits > 0; mask >>= 1, rbits --) {
			uint8 gp;
			
			/* Clock low */
			sb_gpioout2(adm->sbh, adm->eesk, 0,0);
			OSL_DELAY(EECK_EDGE_TIME);

			gp = sb_gpioin(adm->sbh);
			
			if (gp & adm->eedi)
				byte |= mask;
				
			/* Clock high */
			sb_gpioout2(adm->sbh, adm->eesk, adm->eesk,0);
			OSL_DELAY(EECK_EDGE_TIME);
		}

		*rbuf++ = byte;
	}

	/* Clock low */
	sb_gpioout2(adm->sbh, adm->eesk, 0,0);
	OSL_DELAY(EECK_EDGE_TIME);

	/* CS low */
	if (cs)
		sb_gpioout2(adm->sbh, adm->eecs, 0,0);
}

/* Write a bit stream to the chip */
static void
adm_write(adm_info_t *adm, uint8 cs, uint8 *buf, uint bits)
{
	uint i, len = (bits + 7) / 8;
	uint8 mask;

	/* CS high/low */
	if (cs)
		sb_gpioout2(adm->sbh, adm->eecs, adm->eecs,0);
	else
		sb_gpioout2(adm->sbh, adm->eecs, 0,0);
	OSL_DELAY(EECK_EDGE_TIME);

	/* Byte assemble from MSB to LSB */
	for (i = 0; i < len; i++) {
		/* Bit bang from MSB to LSB */
		for (mask = 0x80; mask && bits > 0; mask >>= 1, bits --) {
			/* Clock low */
			sb_gpioout2(adm->sbh, adm->eesk, 0,0);
			OSL_DELAY(EECK_EDGE_TIME);

			/* Output on rising edge */
			if (mask & buf[i])
				sb_gpioout2(adm->sbh, adm->eedi, adm->eedi,0);
			else
				sb_gpioout2(adm->sbh, adm->eedi, 0,0);
			OSL_DELAY(EEDI_SETUP_TIME);
		
			/* Clock high */
			sb_gpioout2(adm->sbh, adm->eesk, adm->eesk,0);
			OSL_DELAY(EECK_EDGE_TIME);
		}
	}

	/* Clock low */
	sb_gpioout2(adm->sbh, adm->eesk, 0,0);
	OSL_DELAY(EECK_EDGE_TIME);

	/* CS low */
	if (cs)
		sb_gpioout2(adm->sbh, adm->eecs, 0,0);
}


void
adm_rreg(adm_info_t *adm, uint8 domain, uint8 addr, uint32* val)
{
	/* cmd(27bits): sb(1) + opc(01) + addr(bbbbbbbb) + data(bbbbbbbbbbbbbbbb) */
	/* cmd: 01 10 T DD R RRRRRR */
	uint8 bits[6] = {
		0xFF, 0xFF, 0xFF, 0xFF, 
		(0x06 << 4) | ((domain&1)<<3 | ((0&3)<<1) | (addr&64)>>6),
		((addr&63)<<2)
	};

	uint8 rbits[4];

	// printk ("adm_rreg: enabling gpio output.. (sbh=%p)\n", adm->sbh);
	// printk("adm_rreg: addr %02x domain %01x (%02X%02X)\n", 
	//	addr, domain, bits[4], bits[5]);

	// printk ("enabling gpio output.. (sbh=%p)\n", adm->sbh);
	/* Enable GPIO outputs with all pins to 0 */
	adm_enout(adm, (uint8)(adm->eecs | adm->eesk | adm->eedi), 0);

	// printk ("adm write 46...\n");

	adm_write(adm, 0, bits, 46);
	adm_disout (adm, (uint8)(adm->eedi));
	adm_adclk(adm, 2);
	adm_read (adm, 0, rbits, 32);

	/* Extra clock(s) required per datasheet */
	adm_adclk(adm, 2);

	/* Disable GPIO outputs */
	adm_disout(adm, (uint8)(adm->eecs | adm->eesk));

	*val = (rbits[0]<<24) | (rbits[1]<<16) | (rbits[2]<<8) | rbits[3];
}

/* Handy macros for writing fixed length values */
#define adm_write8(adm, cs, b) { uint8 val = (uint8) (b); adm_write(adm, cs, &val, sizeof(val)*8); }
#define adm_write16(adm, cs, w) { uint16 val = hton16(w); adm_write(adm, cs, (uint8 *)&val, sizeof(val)*8); }
#define adm_write32(adm, cs, i) { uint32 val = hton32(i); adm_write(adm, cs, (uint8 *)&val, sizeof(val)*8); }

/* Write chip configuration register */
/* Follow 93c66 timing and chip's min EEPROM timing requirement */
void
adm_wreg(adm_info_t *adm, uint8 addr, uint16 val)
{
	/* cmd(27bits): sb(1) + opc(01) + addr(bbbbbbbb) + data(bbbbbbbbbbbbbbbb) */
	uint8 bits[4] = {
		(0x05 << 5) | (addr >> 3),
		(addr << 5) | (uint8)(val >> 11),
		(uint8)(val >> 3),
		(uint8)(val << 5)
	};

	printk("ADM6996: adm_wreg: addr %02x val %04x (%02X%02X%02X%02X)\n", 
		addr, val, bits[0], bits[1], bits[2], bits[3]);

	/* Enable GPIO outputs with all pins to 0 */
	adm_enout(adm, (uint8)(adm->eecs | adm->eesk | adm->eedi), 0);

	/* Write cmd. Total 27 bits */
	adm_write(adm, 1, bits, 27);

	/* Extra clock(s) required per datasheet */
	adm_adclk(adm, 2);

	/* Disable GPIO outputs */
	adm_disout(adm, (uint8)(adm->eecs | adm->eesk | adm->eedi));
}


/* Copy each token in wordlist delimited by space into word */
static int _strspn(const char * p, const char * s)
{
	int i,j;

	for (i=0;p[i];i++) {
		for (j=0;s[j];j++) {
			if (s[j] == p[i]) break;
		}
		if (!s[j]) break;
	}
	return(i);
}
static int _strcspn(char *p, char *s)
{
	int i,j;

	for (i=0;p[i];i++) {
		for (j=0;s[j];j++) {
			if (s[j] == p[i]) break;
		}
		if (s[j]) break;
	}
	return(i);
}
#define foreach(word, wordlist, next) \
	for (next = &wordlist[_strspn(wordlist, " ")], \
	     strncpy(word, next, sizeof(word)), \
	     word[_strcspn(word, " ")] = '\0', \
	     word[sizeof(word) - 1] = '\0', \
	     next = strchr(next, ' '); \
	     strlen(word); \
	     next = next ? &next[_strspn(next, " ")] : "", \
	     strncpy(word, next, sizeof(word)), \
	     word[_strcspn(word, " ")] = '\0', \
	     word[sizeof(word) - 1] = '\0', \
	     next = strchr(next, ' '))

/* Maximum vlan groups */
#define MAX_VLAN_GROUPS	16

/* Port configuration */
typedef struct {
	uint8 addr;	/* port configuration register */
	uint16 vlan;	/* vlan port mapping */
	uint8 tagged;	/* output tagging */
	uint8 cpu;	/* cpu port? 1 - yes, 0 - no */
	uint16 pvid;	/* cpu port pvid */
} port_cfg;

/* Vlan port map */
typedef struct {
	uint8 addr;	/* vlan port map register */
} vlan_cfg;

/* Configure the chip based on nvram settings */
int
adm_config_vlan(adm_info_t *adm, char *vars)
{
	port_cfg port_cfg_tab[] = {
		{1, 1<<0, 0, 0, -1},
		{3, 1<<2, 0, 0, -1},
		{5, 1<<4, 0, 0, -1},
		{7, 1<<6, 0, 0, -1},
		{8, 1<<7, 0, 0, -1},
#if defined(PMON) || defined(_CFE_)
		{9, 1<<8, 0, 1, -1}	/* no output tagging for pmon/cfe */
#else	/* #if defined(PMON) || defined(CFE) */
		{9, 1<<8, 1, 1, -1}	/* output tagging for linux... */
#endif	/* #if defined(PMON) || defined(CFE) */
	};
	vlan_cfg vlan_cfg_tab[] = {
		{0x13},
		{0x14},
		{0x15},
		{0x16},
		{0x17},
		{0x18},
		{0x19},
		{0x1a},
		{0x1b},
		{0x1c},
		{0x1d},
		{0x1e},
		{0x1f},
		{0x20},
		{0x21},
		{0x22}
	};
	uint16 i;

	/* vlan mode select register (0x11): vlan on, mac clone */
	adm_wreg(adm, 0x11, 0xff30);

	/* vlan port group: port configuration, vlan port map */
	for (i = 0; i < MAX_VLAN_GROUPS; i ++) {
		char port[16], *next;
		char vlan_group[16];
		uint16 vlan_map = 0;
		char *ports;

		/* get nvram port settings */
		sprintf(vlan_group, "vlan%dports", i);
		ports = getvar(vars, vlan_group);
		if (!ports)
			continue;

		/*
		* port configuration register (0x01, 0x03, 0x05, 0x07, 0x08, 0x09): 
		*   input/output tagging, pvid, auto mdix, auto negotiation, ...
		* cpu port needs special handing to support pmon/cfe/linux...
		*/
		foreach (port, ports, next) {
			int port_num = bcm_atoi(port);
			uint16 port_cfg;
			
			/* make sure port # is within the range */
			if (port_num >= sizeof(port_cfg_tab) / sizeof(port_cfg_tab[0])) {
				printk("port number %d is out of range\n", port_num);
				continue;
			}
			
			/* build vlan port map */
			vlan_map |= port_cfg_tab[port_num].vlan;
			
			/* cpu port needs special care */
			if (port_cfg_tab[port_num].cpu) {
				/* check if the port is marked */
				if (strchr(port, '*'))
					port_cfg_tab[port_num].pvid = i;
				/* will be done later */
				continue;
			}
			
			/* configure port */
			port_cfg = 0x8000 |	/* auto mdix */
				(i << 10) | 	/* pvid */
				0x000f;		/* full duplex, 100Mbps, auto neg, flow ctrl */
			adm_wreg(adm, port_cfg_tab[port_num].addr, port_cfg);
		}

		/* vlan port map register (0x13 - 0x22) */
		adm_wreg(adm, vlan_cfg_tab[i].addr, vlan_map);
	}
	
	/* cpu port config: auto mdix, pvid, output tagging, ... */
	for (i = 0; i < sizeof(port_cfg_tab)/sizeof(port_cfg_tab[0]); i ++) {
		uint16 tagged, pvid;
		uint16 port_cfg;
		
		/* cpu port only */
		if (port_cfg_tab[i].cpu == 0 || port_cfg_tab[i].pvid == 0xffff)
			continue;
		
		/* configure port */
		tagged = port_cfg_tab[i].tagged ? 1 : 0;
		pvid = port_cfg_tab[i].pvid;
		port_cfg = 0x8000 |	/* auto mdix */
			(pvid << 10) | 	/* pvid */
			(tagged << 4) |	/* output tagging */
			0x000f;		/* full duplex, 100Mbps, auto neg, flow ctrl */
		adm_wreg(adm, port_cfg_tab[i].addr, port_cfg);
	}
	
	return 0;
}

/*
* Enable the chip with preset default configuration:
*
*  TP Auto MDIX (EESK/GPIO = 1)
*  Single Color LED (EEDI/GPIO = 0)
*  EEPROM Disable (H/W pull down)
*/
void
adm_enable_device(adm_info_t *adm, char *vars)
{
	uint8 rc;
	int i;
	
	/* Check nvram override existance */
	if ((rc = adm_get_gpio(vars, "adm_rc", 0xff)) == 0xff)
		return;
	
	/*
	* Reset sequence: RC high->low(100ms)->high(30ms)
	*
	* WAR: Certain boards don't have the correct power on 
	* reset logic therefore we must explicitly perform the
	* sequece in software.
	*/
	/* Keep RC high for at least 20ms */
	adm_enout(adm, rc, rc);
	for (i = 0; i < 20; i ++)
		OSL_DELAY(1000);
	/* Keep RC low for at least 100ms */
	adm_enout(adm, rc, 0);
	for (i = 0; i < 100; i++)
		OSL_DELAY(1000);
	/* Set default configuration */
	adm_enout(adm, (uint8)(adm->eesk | adm->eedi), adm->eesk);
	/* Keep RC high for at least 30ms */
	adm_enout(adm, rc, rc);
	for (i = 0; i < 30; i++)
		OSL_DELAY(1000);
	/* Leave RC high and disable GPIO outputs */
	adm_disout(adm, (uint8)(adm->eecs | adm->eesk | adm->eedi));
}

