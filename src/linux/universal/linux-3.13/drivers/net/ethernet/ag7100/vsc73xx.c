/* vsc73xx.c 
 * May 24, 2007 Tag before BSP resturcture
 */
 
#ifdef __BDI
#include "bdi.h"
#else
#ifdef __ECOS
#if defined(CYGNUM_USE_ENET_VERBOSE)
#   undef  VERBOSE
#   define VERBOSE CYGNUM_USE_ENET_VERBOSE
#else
#   define VERBOSE 0
#endif 
#define printk             DEBUG_PRINTF
#define udelay             A_UDELAY
#else
#include <linux/kernel.h>
#include <asm/delay.h>
#include "ar7100.h"
#endif
#endif

#ifndef VERBOSE
#define  VERBOSE           0
#endif

#include "generic_spi.h"
#include "vsc73xx.h"

#define MODULE_NAME "VSC73XX"
#ifdef CONFIG_AR9100
extern int board_version;
#endif

/* ************************************************************** */

#define VSC73XX_SYSTEM            0x7
#define VSC73XX_ICPU_CTRL         0x10
#define VSC73XX_ICPU_ADDR         0x11
#define VSC73XX_ICPU_SRAM         0x12
#define VSC73XX_ICPU_MAILBOX_VAL  0x15
#define VSC73XX_ICPU_MAILBOX_SET  0x16
#define VSC73XX_ICPU_MAILBOX_CLR  0x17
#define VSC73XX_ICPU_CHIPID       0x18
#define VSC73XX_ICPU_SIPAD        0x01
#define VSC73XX_ICPU_GPIO         0x34

#define VSC73XX_ICPU_CLOCK_DELAY  0x05

#define VSC73XX_MAC               0x1
#define VSC73XX_MAC_CFG           0x0
#define VSC73XX_ADVPORTM          0x19
#define VSC73XX_RXOCT             0x50
#define VSC73XX_TXOCT             0x51
#define VSC73XX_C_RX0             0x52
#define VSC73XX_C_RX1             0x53
#define VSC73XX_C_RX2             0x54
#define VSC73XX_C_TX0             0x55
#define VSC73XX_C_TX1             0x56
#define VSC73XX_C_TX2             0x57
#define VSC73XX_C_CFG             0x58

#define VSC73XX_MII               0x3
#define VSC73XX_MII_STAT          0x0
#define VSC73XX_MII_CMD           0x1
#define VSC73XX_MII_DATA          0x2

static void inline
vsc73xx_force_reset(void)
{
  ar7100_reg_rmw_set(AR7100_RESET, AR7100_RESET_GE1_PHY);
  udelay(10);
  ar7100_reg_rmw_clear(AR7100_RESET, AR7100_RESET_GE1_PHY);
}

#ifdef USE_TEST_CODE

void
vsc73xx_test_reset_line(void)
{
  printk(MODULE_NAME": looping 10 uSec nRESET, 100Usec RESET\n");
  generic_spi_init(GENERIC_SPI_VSC73XX_CS);
  do {
    vsc73xx_force_reset();
    udelay(100);
  } while (1);
}

#endif

static int
vsc73xx_check_block_sublock_ok(int block, int sublock) 
{
  switch (block) {
  case 1:
    switch (sublock) {
    case 0: case 1: case 2: case 3: case 4: case 6:
      return (0==0);
    }
    break;
  case 2: case 7:
    switch (sublock) {
    case 0: 
      return (0==0);
    }
    break;
  case 3: case 4: case 5:
    switch (sublock) {
    case 0: case 1: 
      return (0==0);
    }
    break;
  }
  return 0;
}

int
vsc73xx_rd(int block, int subblock, int reg, unsigned int  *value)
{
  int rc;

  rc=vsc73xx_check_block_sublock_ok(block, subblock);
  if (rc<0) {
    printk(MODULE_NAME": non-supported block/subblock %d %d\n", block, subblock);
    return -1;
  }

  rc=generic_spi_access_enable(GENERIC_SPI_VSC73XX_CS);
#ifdef CONFIG_AR9100
  if (board_version >= 50) {
    gpio_clk_setup();
  }
#endif
  if (rc<0) {
    printk(MODULE_NAME": unable to CS %08x \n", GENERIC_SPI_VSC73XX_CS);
    return -1;
  }
  
  /* Send address */
  generic_spi_raw_output_u8((block<<5) | (0/*READ*/<<4) | (subblock<<0));
  generic_spi_raw_output_u8(reg);
  
  /* Pad based on SiPAD register ( default 2) */
  generic_spi_raw_output_u8(0);
  generic_spi_raw_output_u8(0);

  /* Clock 32b data into serial shift register */
  *value = generic_spi_raw_input_u32(); 
  
  rc=generic_spi_access_done();

  if (rc<0) {
    printk(MODULE_NAME": unable to deassert CS %08x \n", GENERIC_SPI_VSC73XX_CS);
    return -1;
  }

  return 0;
}

int
vsc73xx_wr(int block, int subblock, int reg, unsigned int  value)
{
  int rc;

  rc=vsc73xx_check_block_sublock_ok(block, subblock);
  if (rc<0) {
    printk(MODULE_NAME": non-supported block/subblock %d %d\n", block, subblock);
    return -1;
  }
  
  rc=generic_spi_access_enable(GENERIC_SPI_VSC73XX_CS);
  if (rc<0) {
    printk(MODULE_NAME": unable to CS %08x \n", GENERIC_SPI_VSC73XX_CS);
    return -1;
  }	

  /* Address */
  generic_spi_raw_output_u8((block<<5) | (1/*WRITE*/<<4) | (subblock<<0) );
  generic_spi_raw_output_u8(reg);

  /* Data */
  generic_spi_raw_output_u8((value>>24) & 0xff);
  generic_spi_raw_output_u8((value>>16) & 0xff);
  generic_spi_raw_output_u8((value>>8 ) & 0xff);
  generic_spi_raw_output_u8((value>>0 ) & 0xff);
  
  rc=generic_spi_access_done();
  if (rc<0) {
    printk(MODULE_NAME": unable to deassert CS  %08x \n", GENERIC_SPI_VSC73XX_CS);
    return -1;
  }     

  return 0;
}

static int
vsc73xx_get_and_verify_chipid(void)
{
  int curVal;
  int chip;
  int ii=256;

  do {
    vsc73xx_rd(VSC73XX_SYSTEM, 0, VSC73XX_ICPU_CHIPID, &curVal);
    chip = (curVal >> 12)  & 0x0ffff;
  } while ((chip != 0x7385) && (chip != 0x7395) && (chip != 0x7396) && --ii);

#ifdef VSC73XX_DEBUG
  printk(MODULE_NAME": curval = 0x%08x\n", curVal);
#endif

  if (0==ii) {
    printk(MODULE_NAME": unknown chip: %08x\n", chip);
    return -1;
  }

#ifdef CONFIG_AR9100

  /*
   * Per Martin Olsen [martino@vitesse.com],
   * VSC7385YV chips with 0x0 in bits 31:28 of "Block 7 Subblock 0
   * Address 0x18" is the first revision and that have a problem
   * with the reset. This can be the reason why this doesn't work.
   */
  if (!((curVal >> 28) & 0xf)) {
    printk("\n==================================================\n"
           "WARNING:\n"
           "Using revision 0 of chip 0x%x. It might not work!\n"
           "==================================================\n", chip);
  }
#endif

  return chip;
}

static int
vsc73xx_reset_and_verify_chipid(void)
{
#ifdef CONFIG_AR9100
  if (board_version < 50) {
    vsc73xx_force_reset();
  }
#else
  vsc73xx_force_reset();
#endif
  return vsc73xx_get_and_verify_chipid();
}

#ifdef USE_TEST_CODE

void
vsc73xx_test_reset_and_verify_chipid(void)
{
  printk(MODULE_NAME": looping on reset & verify chipid\n");
  generic_spi_init(GENERIC_SPI_VSC73XX_CS);
  do {
    vsc73xx_reset_and_verify_chipid();
  } while (1);
}

#endif

static inline int
vsc73xx_mailbox_get(unsigned int *d)
{
  return vsc73xx_rd(VSC73XX_SYSTEM, 0, VSC73XX_ICPU_MAILBOX_VAL, d);
}

static inline int
vsc73xx_mailbox_clr(unsigned int value)
{
  return vsc73xx_wr(VSC73XX_SYSTEM, 0, VSC73XX_ICPU_MAILBOX_CLR, value);
}

static inline int
vsc73xx_mailbox_set(unsigned int value)
{
  return vsc73xx_wr(VSC73XX_SYSTEM, 0, VSC73XX_ICPU_MAILBOX_SET, value);
}

static inline int 
vsc73xx_gpio_config_output(int value)
{
  return vsc73xx_wr(VSC73XX_SYSTEM, 0, VSC73XX_ICPU_GPIO, ( value & VSC73XX_GPIO_MASK ) << 4);
}

static inline int
vsc73xx_gpio_output(int value)
{  
  unsigned int d;
  int rc;

  value &=VSC73XX_GPIO_MASK;
  
  /* 1 = high 0=low  */
  rc=vsc73xx_rd(VSC73XX_SYSTEM, 0, VSC73XX_ICPU_GPIO, &d);
  if (rc<0)
    return rc;

  d &=VSC73XX_GPIO_MASK;
  d |=value;
  return vsc73xx_wr(VSC73XX_SYSTEM, 0, VSC73XX_ICPU_GPIO, d);
}

static inline int 
vsc73xx_gpio_input(unsigned int *d)
{
  int rc;
  unsigned int e;
  rc=vsc73xx_rd(VSC73XX_SYSTEM, 0, VSC73XX_ICPU_GPIO, &e);
  if (rc<0)
    return rc;

  *d = e & VSC73XX_GPIO_MASK;
  return rc;
}

#ifdef USE_TEST_CODE

void
vsc73xx_test_gpio(void)
{
  printk(MODULE_NAME": looping on / off vsc73xx GPIO\n");
  generic_spi_init(GENERIC_SPI_VSC73XX_CS);
  vsc73xx_gpio_config_output(VSC73XX_GPIO_2);
  do {
    vsc73xx_gpio_output(VSC73XX_GPIO_2);
    udelay(50);
    vsc73xx_gpio_output(0);
    udelay(50);
  } while (1);
}

#endif

static int
vsc73xx_get_sVersion_resetNeeded(int *sVersion, int *resetNeeded)
{
  unsigned int d;
  int rc;

  rc=vsc73xx_mailbox_get(&d);
  if (rc<0)
    return rc;
  *sVersion    = d & 0xffff;
  *resetNeeded = (d & 0xffff0000) == 0xffff0000;
  return 0;
}

static inline int 
vsc73xx_restart_firmware(void)
{
  int rc;

  rc = vsc73xx_wr(
		  VSC73XX_SYSTEM, 0, VSC73XX_ICPU_CTRL,
		  (1<<7) |          /* SOFT_RST_HOLD = 1 */
		  (1<<3) |          /* BOOT_EN       = 1 */
		  (1<<2) |          /* EXT_ACC_EN    = 1 */
		  (0<<0)            /* SOFT_RST      = 0 */
		  );
  if (rc<0)
    return rc;
  
  rc = vsc73xx_wr(		
		  VSC73XX_SYSTEM, 0,VSC73XX_ICPU_ADDR,
		  0x0000
		  );
  if (rc<0)
    return rc;
  
  udelay(100);
  
  rc = vsc73xx_wr(
		  VSC73XX_SYSTEM, 0, VSC73XX_ICPU_CTRL,
		  (1<<8) |          /* CLK_DIV  = 1 */
		  (1<<3) |          /* BOOT_EN  = 1 */
		  (1<<1) |          /* CLK_EN   = 1 */
		  (1<<0)            /* SOFT_RST = 1 */
		  );
  return rc;
}

static int
vsc73xx_load_firmware_raw(unsigned char *lutonuAddr, int lutonuSize) 
{
  int            ii;
  unsigned char  *dp;
  unsigned int   curVal;
  int            diffs;
  int            rc;
  
  rc = vsc73xx_wr(
		  VSC73XX_SYSTEM, 0, VSC73XX_ICPU_CTRL,
		  (1<<7) |          /* SOFT_RST_HOLD = 1 */
		  (1<<3) |          /* BOOT_EN       = 1 */
		  (1<<2) |          /* EXT_ACC_EN    = 1 */
		  (0<<0)            /* SOFT_RST      = 0 */
		  );
  if (rc<0)
    return rc;
  
  rc = vsc73xx_wr(		
		  VSC73XX_SYSTEM, 0,VSC73XX_ICPU_ADDR,
		  0x0000
		  );
  if (rc<0)
    return rc;
  
  dp = lutonuAddr;
  for (ii=0; ii<lutonuSize; ii++) {
    rc = vsc73xx_wr(
		    VSC73XX_SYSTEM, 0, VSC73XX_ICPU_SRAM,
		    *dp++ 
		    );
    if (rc<0) {
      printk(MODULE_NAME": could not load microcode %d\n",rc);
      return rc;
    }
  }

  rc = vsc73xx_wr(		
		  VSC73XX_SYSTEM, 0, VSC73XX_ICPU_ADDR,
		  0x0000
		  );
  if (rc<0) {
    printk(MODULE_NAME": could not reset microcode %d\n",rc);
    return rc;
  }

  printk(MODULE_NAME": microcode Loaded, verifying...\n");
  
  dp = lutonuAddr;
  diffs=0;
  for (ii=0; ii<lutonuSize; ii++) {
    rc = vsc73xx_rd(
		    VSC73XX_SYSTEM, 0,VSC73XX_ICPU_SRAM, 
		    &curVal
		    );
    if (rc<0) {
      printk(MODULE_NAME": could not read microcode %d\n",rc);
      return rc;
    }
    
    if (curVal > 0xff) {
      printk(MODULE_NAME": bad val read: %04x : %02x  %02x  \n", ii, *dp, curVal);
      return -1;
    } 
    
    if ((curVal & 0xff) != *dp) {
      diffs++;
      printk(MODULE_NAME": verify error: %04x : %02x  %02x  \n", ii, *dp, curVal);
      
      if (diffs > 4)
	break;
    } 
    dp++;
  }
  
  if (diffs) {
    printk(MODULE_NAME": failed to verify\n");
    return -1;
  }
  printk(MODULE_NAME": verify OK\n");
  
  rc = vsc73xx_wr(
		  VSC73XX_SYSTEM, 0, VSC73XX_ICPU_CTRL,
		  (1<<8) |          /* CLK_DIV  = 1 */
		  (1<<3) |          /* BOOT_EN  = 1 */
		  (1<<1) |          /* CLK_EN   = 1 */
		  (1<<0)            /* SOFT_RST = 1 */
		  );
  return rc;
}

#define VSC73XX_SFTW_VERSION            0x229
#ifdef CONFIG_AR9100
#	include "g5_Plus1_2_31_unmanaged_Atheros_v3.c"
#	include "g5_Plus1_2_31_unmanaged_Atheros_v6.c"
#	include "g5_Plus1_2_31_unmanaged_Atheros_v4.c"
#else
#	include "g5_Plus1_2_29b_unmanaged_Atheros_v5.c"
#	include "g5e_Plus1_2_29a_unmanaged_Atheros_v3.c"
#	include "g5_Plus1_2_29a_unmanaged_Atheros_v5.c"
#endif
static int
vsc73xx_load_firmware(void) 
{
  int sVersion;
  int resetNeeded;
  int rc;

#ifdef CONFIG_AR9100
 ar7100_reg_rmw_set(AR9100_FLASH_CONFIG,0x3fffff);
#endif
  rc = vsc73xx_reset_and_verify_chipid();
  if (rc < 0) {
    printk(MODULE_NAME": could not identify chip, err %d\n", rc);
    return rc;
  }
  printk(MODULE_NAME": found chip id: %04x\n", rc);

#if defined(__BDI) && defined(VSC73XX_LOAD_FROM_RAM)
  rc = vsc73xx_load_firmware_raw(get_scratch_uncached(8192), 8192);
#else
  switch (rc) {
  case 0x00007385:  
#ifdef CONFIG_AR9100
    rc = vsc73xx_load_firmware_raw(g5_Plus1_2_31_unmanaged_Atheros_v6,
				   sizeof(g5_Plus1_2_31_unmanaged_Atheros_v6));
#else
    rc = vsc73xx_load_firmware_raw(g5_Plus1_2_29b_unmanaged_Atheros_v5,
                                   sizeof(g5_Plus1_2_29b_unmanaged_Atheros_v5));
#endif
    break;

  case 0x00007395:
    /* source from vitesse uses symbol lutonu, later versions use the name of the file.  */
#ifdef CONFIG_AR9100
    rc = vsc73xx_load_firmware_raw(g5_Plus1_2_31_unmanaged_Atheros_v4,
				   sizeof(g5_Plus1_2_31_unmanaged_Atheros_v4));
#else
    rc = vsc73xx_load_firmware_raw(lutonu,       /* g5e_Plus1_2_29a_unmanaged_Atheros_v3 */
				   sizeof(lutonu /* g5e_Plus1_2_29a_unmanaged_Atheros_v3 */));
#endif
    break;
    
  default:
    printk(MODULE_NAME": microcode not availale for chip id: %08x\n", rc);
    rc = -1;
  }
#endif

  if (rc<0)
    return rc;

  vsc73xx_get_sVersion_resetNeeded(&sVersion, &resetNeeded);
  if (resetNeeded) {
    printk(MODULE_NAME": failed to start\n");
    return -1;
  }
  if (sVersion < VSC73XX_SFTW_VERSION) {
    printk(MODULE_NAME": incorrect software version %04x\n", sVersion);
#ifdef CONFIG_AR9100
    if (board_version < 50) {
      return -1;
    }
#else
    return -1;
#endif
  }
  printk(MODULE_NAME": software version %08x started OK\n",sVersion);

#ifdef CONFIG_AR9100
 ar7100_reg_rmw_set(AR9100_FLASH_CONFIG,0xf2288);
#endif

  return 0;
}

#ifdef USE_TEST_CODE

void
vsc73xx_test_load_and_reset_firmware(void)
{
  int rc;
  printk(MODULE_NAME": looping on load firmware / reset firmware\n");
  generic_spi_init(GENERIC_SPI_VSC73XX_CS);
  do {
    printk(MODULE_NAME": return from load firmware: %d\n", vsc73xx_load_firmware());
    udelay(10000);
  } while (1);
}

#endif

static void
vsc73xx_print_val_bit_desc (unsigned int val, char *fieldName, char *bit_descriptions[]) 
{
  int ii;
  char *p2;
  
  printk(MODULE_NAME":   %s=%04x\n", fieldName, val);
  for (ii=0;ii<32; ii++) {
    p2 = val & 1<<(31-ii) ? bit_descriptions[ii*2] : bit_descriptions[ii*2+1];
    if (p2 && p2[0])      
      printk(MODULE_NAME":     %s\n", p2);
  }
}

/* Port 0..4, 6 */
static inline int 
vsc73xx_get_mac_cfg(int port, unsigned int *d)
{
  return vsc73xx_rd(VSC73XX_MAC, port, VSC73XX_MAC_CFG, d);
}

static char *mac_cfg_desc[32*2] = {
  "wexc_dis","", /* 31 */
  "","", /* 30 */
  "port_rst","", /* 29 */
  "tx_en","", /* 28 */
  "seed_load","", /* 27 */
  "","", /* 26 */
  "","", /* 25 */
  "","", /* 24 */
  "","", /* 23 */
  "","", /* 22 */
  "","", /* 21 */
  "","", /* 20 */
  "","", /* 19 */
  "en-fdx","", /* 18 */
  "gige","", /* 17 */
  "rx_en","", /* 16 */
  "vlan_dblawr","", /* 15 */
  "vlan_awr","", /* 14 */
  "100-base-T","", /* 13 */
  "","", /* 12 */
  "","", /* 11 */
  "","", /* 10 */
  "","", /*  9 */
  "","", /*  8 */
  "","", /*  7 */
  "","", /*  6 */
  "mac_rx_rst","", /*  5 */
  "mac_tx_rst","", /*  4 */
  "","", /*  3 */
  "","", /*  2 */
  "","", /*  1 */
  "","", /*  0 */
};

static inline void
vsc73xx_print_mac_cfg_val(unsigned int val)
{
  vsc73xx_print_val_bit_desc (val, "mac_cfg        (01P00)", mac_cfg_desc);
  printk(MODULE_NAME":   clk_sel: %02x\n", (val>>0)&0x3);
  printk(MODULE_NAME":   tx_ipg:  %02x\n", (val>>6)&0x1f);
}

static inline int 
vsc73xx_set_mac_cfg(int port, unsigned int value)
{
  return vsc73xx_wr(VSC73XX_MAC, port, VSC73XX_MAC_CFG, value);
}

static inline int 
vsc73xx_get_clock_delay_reg(unsigned int *val)
{
  return vsc73xx_rd(VSC73XX_SYSTEM, 0, VSC73XX_ICPU_CLOCK_DELAY, val);
}

static inline int
vsc73xx_set_clock_delay_reg(unsigned int val)
{
  return vsc73xx_wr(VSC73XX_SYSTEM, 0, VSC73XX_ICPU_CLOCK_DELAY, val);
}

static inline int 
vsc73xx_get_advportm(int port, unsigned int *val)
{
  return vsc73xx_rd(VSC73XX_MAC, port, VSC73XX_ADVPORTM, val);
}

static char *advportm_desc[32*2] = {
  "","", /* 31 */
  "","", /* 30 */
  "","", /* 29 */
  "","", /* 28 */
  "","", /* 27 */
  "","", /* 26 */
  "","", /* 25 */
  "","", /* 24 */
  "","", /* 23 */
  "","", /* 22 */
  "","", /* 21 */
  "","", /* 20 */
  "","", /* 19 */
  "","", /* 18 */
  "","", /* 17 */
  "","", /* 16 */
  "","", /* 15 */
  "","", /* 14 */
  "","", /* 13 */
  "","", /* 12 */
  "","", /* 11 */
  "","", /* 10 */
  "","", /*  9 */
  "","", /*  8 */
  "ifg_ppm","",       /*  7 */
  "exc_col","",       /*  6 */
  "ext_port","",      /*  5 */
  "inv_gtx","",       /*  4 */
  "ena_gtx","",       /*  3 */
  "ddr_mode","",      /*  2 */
  "io_loopback","",   /*  1 */
  "host_loopback","", /*  0 */
};

static inline void
vsc73xx_print_advportm_val(unsigned int val)
{
  vsc73xx_print_val_bit_desc (val, "advportm      (01P19)", advportm_desc);
}

static inline int
vsc73xx_set_advportm(int port, unsigned int val)
{
  return vsc73xx_wr(VSC73XX_MAC, port, VSC73XX_ADVPORTM, val);
}

/* HERE */

static inline unsigned int 
vsc73xx_get_c_rx0(int port)
{
  unsigned int d;
  vsc73xx_rd(VSC73XX_MAC, port, VSC73XX_C_RX0, &d);
  return d;
}

static inline unsigned int 
vsc73xx_get_c_rx1(int port)
{
  unsigned int d;
  vsc73xx_rd(VSC73XX_MAC, port, VSC73XX_C_RX1, &d);
  return d;
}

static inline unsigned int 
vsc73xx_get_c_rx2(int port)
{
  unsigned int d;
  vsc73xx_rd(VSC73XX_MAC, port, VSC73XX_C_RX2, &d);
  return d;
}

static inline unsigned int 
vsc73xx_get_c_tx0(int port)
{
  unsigned int d;
  vsc73xx_rd(VSC73XX_MAC, port, VSC73XX_C_TX0, &d);
  return d;
}

static inline unsigned int 
vsc73xx_get_c_tx1(int port)
{
  unsigned int d;
  vsc73xx_rd(VSC73XX_MAC, port, VSC73XX_C_TX1, &d);
  return d;
}

static inline unsigned int 
vsc73xx_get_c_tx2(int port)
{
  unsigned int d;
  vsc73xx_rd(VSC73XX_MAC, port, VSC73XX_C_TX2, &d);
  return d;
}

static inline unsigned int 
vsc73xx_get_c_cfg(int port)
{
  unsigned int d;
  vsc73xx_rd(VSC73XX_MAC, port, VSC73XX_C_CFG, &d);
  return d;
}

static inline void
vsc73xx_set_c_cfg(int port, unsigned int value)
{
  vsc73xx_wr(VSC73XX_MAC, port, VSC73XX_C_CFG, value);
}

void
vsc73xx_print_counts(int port)
{
  printk(MODULE_NAME":counters port %d\n", port);
  printk(MODULE_NAME":   cfg: %8x\n", vsc73xx_get_c_cfg(port));
  printk(MODULE_NAME":   rx0: %9d\n", vsc73xx_get_c_rx0(port));
  printk(MODULE_NAME":   rx0: %9d\n", vsc73xx_get_c_rx1(port));
  printk(MODULE_NAME":   rx0: %9d\n", vsc73xx_get_c_tx2(port));
  printk(MODULE_NAME":   tx0: %9d\n", vsc73xx_get_c_tx0(port));
  printk(MODULE_NAME":   tx0: %9d\n", vsc73xx_get_c_tx1(port));
  printk(MODULE_NAME":   tx0: %9d\n", vsc73xx_get_c_tx2(port));
}

static inline void
vsc73xx_print_link_status_from_value(int port, int up, int fdx, int speed, unsigned int cfg)
{
  printk(MODULE_NAME": STATUS Port: %d  up: %d  fdx: %d  speed: %d  mac cfg: %08x\n", port, up, fdx, speed, cfg);
  vsc73xx_print_mac_cfg_val(cfg);
}

#if 0

/* WARNING: Do not use this if the switch application is running on the 8051. This
 *          function may be used if the switch application is on the host and the 
 *          8051 is in reset.
 */
unsigned short
vsc73xx_rw_phy(int writeFlg, int unit, int phy_addr, int reg, uint16_t value)
{
  /* unit 0   = mac0 and should not happen here
   * unit 1   = mac1, PHY we connect to using MAC 1
   * unit 2-6 = ports on switch */
  
  unsigned int  request;
  unsigned int  resp;
  int sublockA;
  int sublockB;

  /* This needs to be checked since we are at a new level of 8051 code */
  sublockA = unit==1 ? 6:0; /* unit==1 is PHY tied to MAC using RGMII */
  sublockB = unit==1 ? 1:0; /* unit>=1 is PHY on switch                */
                            /* With the new 8051 code this has to be figured
			     * out again ( that is sublock values )
			     */

  request = (writeFlg ? 0 : 1)<<26 | (phy_addr<<21) | (reg<<16) | value;
  
  vsc73xx_wr(VSC73XX_MII, sublockA, VSC73XX_MII_CMD, request, 0);
  udelay(10);
  do {
    vsc73xx_rd(VSC73XX_MII, sublockB, VSC73XX_MII_STAT, &resp, 0);
    udelay(10);
  } while ( resp & 0xf );
  
  if (writeFlg)
    return 0;
  
  udelay(1);
  
  vsc73xx_rd(VSC73XX_MII, sublockB, VSC73XX_MII_DATA, &resp, 0);
  
  if (resp & 1<<16) {
    return 0xffff;
  }

  return resp & 0xffff;
}

#endif

static int
vsc73xx_setup_raw(void)
{
  int          rc;
  int          sVersion=0;
  int          resetNeeded=0;

  unsigned int t_cfg;
  unsigned int t_clock_delay;
  unsigned int t_advportm;

  rc = generic_spi_init(GENERIC_SPI_VSC73XX_CS);
  if (rc < 0) {
    printk(MODULE_NAME": could not initialize spi interface, err %d\n", rc);
    return rc;
  }

  vsc73xx_get_sVersion_resetNeeded(&sVersion, &resetNeeded);
 if ( (resetNeeded) || (sVersion < VSC73XX_SFTW_VERSION) ) {
    rc = vsc73xx_load_firmware();
    if (rc < 0)
      return rc;
  }

  /* The VSC73XX does not work very well unless we setup 2 nsec delay */
  vsc73xx_set_clock_delay_reg(VSC73XX_CLOCK_DELAY);
  vsc73xx_get_clock_delay_reg(&t_clock_delay);  
  
  if ((t_clock_delay & VSC73XX_CLOCK_DELAY_MASK) != VSC73XX_CLOCK_DELAY) {
    printk(MODULE_NAME":   unable to set clock_delay %08x %08x\n",  
		   VSC73XX_CLOCK_DELAY, 
		   VSC73XX_CLOCK_DELAY_MASK &  t_clock_delay);
    return -1;
  }
  else
    printk(MODULE_NAME":  clock_delay_reg %08x\n", t_clock_delay);
  
  /* We must tell the VSC73XX that it has an external interface */
  vsc73xx_set_advportm(VSC73XX_PORT_MAC, 
		       VSC73XX_ADVPORTM_HYDRA);

  vsc73xx_get_advportm(VSC73XX_PORT_MAC, &t_advportm);
 
  if ((VSC73XX_ADVPORTM_HYDRA_MASK & t_advportm) != VSC73XX_ADVPORTM_HYDRA) {
    printk(MODULE_NAME":   unable to set advanportm for MAC port (6) %08x %08x\n", 
		   VSC73XX_ADVPORTM_HYDRA,
		   VSC73XX_ADVPORTM_HYDRA_MASK & t_advportm);
  }
  else
    vsc73xx_print_advportm_val(t_advportm);
  
  /* We must tell the VSC73XX that it can send/recieve data */
#ifdef CONFIG_AR9100
  vsc73xx_set_mac_cfg(VSC73XX_PORT_MAC, VSC73XX_MAC_CFG_PORT_RST |
		VSC73XX_MAC_CFG_MAC_RX_RST | VSC73XX_MAC_CFG_MAC_TX_RST);
  vsc73xx_set_mac_cfg(VSC73XX_PORT_MAC, VSC73XX_MAC_CFG_AR9100);
#else
  vsc73xx_set_mac_cfg(VSC73XX_PORT_MAC, 
		      VSC73XX_MAC_CFG_HYDRA );
#endif
  vsc73xx_get_mac_cfg(VSC73XX_PORT_MAC, &t_cfg);

#ifndef CONFIG_AR9100
  if ((VSC73XX_MAC_CFG_HYDRA_MASK & t_cfg) != VSC73XX_MAC_CFG_HYDRA) {
    printk(MODULE_NAME":   unable to set mac_cfg for port 6 %08x %08x\n", 
		   VSC73XX_MAC_CFG_HYDRA, 
		   VSC73XX_MAC_CFG_HYDRA_MASK & t_cfg);
  }
  else
#endif
    vsc73xx_print_mac_cfg_val(t_cfg);
  
  return rc;
}

static unsigned int mac_cfg_port[7] = { 0, 0, 0, 0, 0, 0, 0 };

static unsigned int
vsc73xx_get_link_status_raw(int port, int *up, int *fdx, ag7100_phy_speed_t *speed, unsigned int *cfg)
{
  int rc;

  int t_up;
  int t_fdx;
  ag7100_phy_speed_t t_speed;

  unsigned int t_cfg;
  unsigned int t_chg;

  rc = vsc73xx_get_mac_cfg(port, &t_cfg);
  if (rc<0)
    return ~0;

  /* FIXME WCL
   * Occasionally the VSC73XX will return 0xffffffff for cfg status with no apparent error.
   */
  if (t_cfg == 0xffffffff) 
    return ~0;

  t_chg = mac_cfg_port[port] ^ t_cfg;
  mac_cfg_port[port] = t_cfg;

  t_up  = (t_cfg & ( VSC73XX_MAC_CFG_TX_EN | VSC73XX_MAC_CFG_RX_EN )) == ( VSC73XX_MAC_CFG_TX_EN | VSC73XX_MAC_CFG_RX_EN );
  t_fdx = (t_cfg & VSC73XX_MAC_CFG_FDX ) != 0;
  
  if ( t_cfg & VSC73XX_MAC_CFG_GIGA_MODE )
    t_speed = AG7100_PHY_SPEED_1000T;
  else
    if ( t_cfg & VSC73XX_MAC_CFG_100_BASE_T )
      t_speed = AG7100_PHY_SPEED_100TX;
    else
      t_speed = AG7100_PHY_SPEED_10T;
    
  if (up)    *up    = t_up;
  if (fdx)   *fdx   = t_fdx;
  if (speed) *speed = t_speed;
  if (cfg)   *cfg   = t_cfg;
  
  return t_chg;
}

#ifdef USE_TEST_CODE

void
vsc73xx_test_link_status(void)
{
  int          port;
  int          ii;

  unsigned int t_up;
  unsigned int t_fdx;
  ag7100_phy_speed_t t_speed;
  unsigned int t_cfg;
  unsigned int t_chg;

  int          rc;

  printk(MODULE_NAME": looping on load firmware / reset firmware\n" );
  vsc73xx_setup_raw();
  do {
    t_chg = vsc73xx_get_link_status_raw(port, &t_up, &t_fdx, &t_speed, &t_cfg);
    if (t_chg == ~0) {
      printk(MODULE_NAME": bad read from switch\n");
    }
    else {
      if (t_chg)
	vsc73xx_print_link_status_from_value(port, t_up, t_fdx, t_speed, t_cfg);
    }
  } while (1);
}

#endif

int 
vsc73xx_setup(int unit)
{
  return vsc73xx_setup_raw();
}

int
vsc73xx_get_link_status(int unit, int *up, int *fdx, ag7100_phy_speed_t *speed, unsigned int *cfg)
{
  int          port;

  unsigned int t_up;
  unsigned int t_fdx;
  ag7100_phy_speed_t t_speed;
  unsigned int t_cfg;
  unsigned int t_chg;

  /* The VSC73XX uses a fixed numbering scheme to get to ports - rather than using PHY Address.
   *  
   * unit 0   == mac 0
   * unit 1   == mac 1 
   * unit 2-6 == ports 0-4 
   */

  switch (unit) {
    case 1: port=VSC73XX_PORT_MAC; break;
    case 2: port=VSC73XX_PORT_0; break;
    case 3: port=VSC73XX_PORT_1; break;
    case 4: port=VSC73XX_PORT_2; break;
    case 5: port=VSC73XX_PORT_3; break;
    case 6: port=VSC73XX_PORT_4; break;
    default:
      printk(MODULE_NAME": bad unit number %d\n", unit);
      return -1;
  } 

  t_chg = vsc73xx_get_link_status_raw(port, &t_up, &t_fdx, &t_speed, &t_cfg);
  if (t_chg == ~0)
    return -2;
  
  if (up)    *up    = t_up;
  if (fdx)   *fdx   = t_fdx;
  if (speed) *speed = t_speed;
  if (cfg)   *cfg   = t_cfg;

#ifdef VSC73XX_DEBUG
  printk("\t==== vsc(%d) up:%d fdx:%d speed:%d cfg=0x%08x\n", unit, t_up, t_fdx, t_speed, t_cfg);
#endif
  return 0;
}

#ifdef VSC73XX_DEBUG
void
vsc73xx_flush_mac_table_all(void)
{
	/* This flushes the mac table of all the ports */
	vsc73xx_wr(2, 0, 0xB0, 0x4);
}

void
vsc73xx_get_link_status_dbg(void)
{
	printk("\n");
	vsc73xx_get_link_status(1, 0, 0, 0, 0);
	vsc73xx_get_link_status(2, 0, 0, 0, 0);
	vsc73xx_get_link_status(3, 0, 0, 0, 0);
	vsc73xx_get_link_status(4, 0, 0, 0, 0);
	vsc73xx_get_link_status(5, 0, 0, 0, 0);
	vsc73xx_get_link_status(6, 0, 0, 0, 0);
}
#endif /* VSC73XX_DEBUG */

int
vsc73xx_phy_print_link_status(int unit)
{
  int          port;

  unsigned int t_up;
  unsigned int t_fdx;
  ag7100_phy_speed_t t_speed;
  unsigned int t_cfg;
  unsigned int t_chg;

  /* The VSC73XX uses a fixed numbering scheme to get to ports - rather than using PHY Address.
   *  
   * unit 0   == mac 0
   * unit 1   == mac 1 
   * unit 2-6 == ports 0-4 
   */

  switch (unit) {
    case 1: port=VSC73XX_PORT_MAC; break;
    case 2: port=VSC73XX_PORT_0; break;
    case 3: port=VSC73XX_PORT_1; break;
    case 4: port=VSC73XX_PORT_2; break;
    case 5: port=VSC73XX_PORT_3; break;
    case 6: port=VSC73XX_PORT_4; break;
    default:
      printk(MODULE_NAME": bad unit number %d\n", unit);
      return -1;
  } 

  t_chg = vsc73xx_get_link_status_raw(port, &t_up, &t_fdx, &t_speed, &t_cfg);
  if (t_chg == ~0)
    return -2;
  
  vsc73xx_print_link_status_from_value(port, t_up, t_fdx, t_speed, t_cfg); 
  return 0;
}
