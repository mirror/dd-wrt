#define here() do { printk("%s(%d)\n", __func__, __LINE__); } while(0)

/* generic_spi.c */

#ifdef __BDI
#include "bdi.h"

#define protect()
#define unprotect()

#else
#ifdef __ECOS
#include <cyg/hal/ar7100_soc.h>
#include "ag7100_ecos.h" 
#define printk          DEBUG_PRINTF

#define protect()
#define unprotect()

#else

#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <asm/delay.h>
#include <asm/io.h>

#include "ar7100.h"

void ar7100_flash_spi_down(void);
void ar7100_flash_spi_up(void);

#define protect()   ar7100_flash_spi_down()
#define unprotect() ar7100_flash_spi_up()

#endif
#endif

#include "generic_spi.h"
#include "generic_i2c.h"

/* ************************************************************** */
#ifdef CONFIG_AR9100

extern int board_version;

#define AR9100_SPI_GPIO_OE       0x18040000 /* GPIO Output Enable */
#define AR9100_SPI_GPIO_OUT      0x18040008 /* GPIO Output Value Register */
#define AR9100_SPI_GPIO_FUNC     0x18040028
#define AR9100_SPI_CLK_HIGH      (1<<23)
#define AR9100_SPI_CLK_LOW       ~(1<<23)
#define AR9100_SPI_DO_HI         (1<<0)
#define AR9100_SPI_CS_DISABLE_0  (1<<12)
#define AR9100_SPI_CS_DISABLE_1  (1<<13)
#define AR9100_SPI_CLK
#define AR9100_SPI_GPIO_IN       0x18040004

#else

#define GENERIC_SPI_FS           0x1f000000 /* Function select */
#define GENERIC_SPI_CLOCK        0x1f000004 /* Alias-disable + clock */

#define GENERIC_SPI_WRITE        0x1f000008 /* SPI_IO_CTRL
					    * spi_do    = 1<<0  (reflects pin)
					    * spi_clk   = 1<<8  (reflects pin)
					    * cs0/gpio  = 1<<16 (active low)
					    * cs1/gpio  = 1<<17 (active low)
					    * cs2gpio   = 1<<18 (active low)
					    */

#define GENERIC_SPI_RD_SHIFT_REG 0x1f00000c /* spi_di is clocked into register pos 0 every clock */

#define GENERIC_SPI_CS_DISABLE_0 (1<<16)
#define GENERIC_SPI_CS_DISABLE_1 (1<<17)
#define GENERIC_SPI_CS_DISABLE_2 (1<<18)

#define GENERIC_SPI_CS_DIS       (GENERIC_SPI_CS_DISABLE_0|GENERIC_SPI_CS_DISABLE_1|GENERIC_SPI_CS_DISABLE_2)

#define GENERIC_SPI_D0_HIGH      (1<<0)
#define GENERIC_SPI_CLK_HIGH     (1<<8)
#endif
#ifdef CONFIG_AR9100
int
generic_spi_access_enable(int cs)
{
  uint32_t val;

  protect();
  val = (ar7100_reg_rd(AR9100_SPI_GPIO_OE)|0x02);
  ar7100_reg_wr_nf(AR9100_SPI_GPIO_OE,val);
  val = (ar7100_reg_rd(AR9100_SPI_GPIO_OUT) & ~(0x0A));
  ar7100_reg_wr_nf(AR9100_SPI_GPIO_OUT,val);

  return 0;
}
int
generic_spi_access_done(void)
{
  uint32_t val;

  val = (ar7100_reg_rd(AR9100_SPI_GPIO_OE)|0x02);
  ar7100_reg_wr_nf(AR9100_SPI_GPIO_OE,val);
  val = (ar7100_reg_rd(AR9100_SPI_GPIO_OUT)|0x02);
  ar7100_reg_wr_nf(AR9100_SPI_GPIO_OUT,val);

  unprotect();

  return 0;
}
int
generic_spi_init(int cs)
{
        return 0;
}
/* Data needs to be stable on rising edge of clock */

#define IN_DATA_GPIO		3
#define OUT_DATA_GPIO		16
#define CLK_GPIO		17

void
generic_spi_raw_output_u8_050(unsigned char val)
{
	int i, rd_val, wr_val;

	for (i = 7; i >= 0; i--) {
		rd_val = ar7100_reg_rd(AR9100_SPI_GPIO_OUT);

		rd_val &= ~(1 << CLK_GPIO); /* lower clock */

		wr_val = (((val >> i) & 1) << OUT_DATA_GPIO);
		wr_val |= (rd_val & ~(1 << OUT_DATA_GPIO));

		ar7100_reg_wr_nf(AR9100_SPI_GPIO_OUT, wr_val);

		wr_val |= (1 << CLK_GPIO); /* high clock */
		ar7100_reg_wr_nf(AR9100_SPI_GPIO_OUT, wr_val);
	}
}
unsigned int
generic_spi_raw_input_u32_050(void)
{
	int i, rd_val;
	unsigned int ret_val = 0;

	/* Configure GPIO as input for data */
	rd_val = (ar7100_reg_rd(AR9100_SPI_GPIO_OE) & ~(1 << IN_DATA_GPIO));
	ar7100_reg_wr_nf(AR9100_SPI_GPIO_OE, rd_val);

	/* Configure GPIO as output for clock */
	rd_val = (ar7100_reg_rd(AR9100_SPI_GPIO_OE) | (1 << CLK_GPIO));
	ar7100_reg_wr_nf(AR9100_SPI_GPIO_OE, rd_val);

	for (i = 31; i >= 0; i--) {
		rd_val = ar7100_reg_rd(AR9100_SPI_GPIO_OUT);
		rd_val &= ~(1 << CLK_GPIO); /* lower clock */
		ar7100_reg_wr_nf(AR9100_SPI_GPIO_OUT, rd_val);

		rd_val = ar7100_reg_rd(AR9100_SPI_GPIO_OUT);
		rd_val |= (1 << CLK_GPIO); /* high clock */
		ar7100_reg_wr_nf(AR9100_SPI_GPIO_OUT, rd_val);

		rd_val = ar7100_reg_rd(AR9100_SPI_GPIO_IN);

		ret_val |= ((rd_val & (1 << IN_DATA_GPIO)) >> IN_DATA_GPIO) << i;
	}

	rd_val = ar7100_reg_rd(AR9100_SPI_GPIO_OUT);
	rd_val &= ~(1 << CLK_GPIO); /* lower clock */
	ar7100_reg_wr_nf(AR9100_SPI_GPIO_OUT, rd_val);

	return ret_val;
}

void
gpio_clk_setup(void)
{
        unsigned rd_val;

        rd_val = (ar7100_reg_rd(AR9100_SPI_GPIO_OE) | (1 << OUT_DATA_GPIO));
        ar7100_reg_wr_nf(AR9100_SPI_GPIO_OE, rd_val);

        /* Configure GPIO as output for clock */
        rd_val = (ar7100_reg_rd(AR9100_SPI_GPIO_OE) | (1 << CLK_GPIO));
        ar7100_reg_wr_nf(AR9100_SPI_GPIO_OE, rd_val);
}

void
generic_spi_raw_output_u8_040(unsigned char val)
{
  int ii,rd_val, wr_val;

  /* Only change GPIO 0, 1 or 2 in mask */
  unsigned int cs0to2mask = (0xbf000000);

  rd_val = (ar7100_reg_rd(AR9100_SPI_GPIO_OE) & ~(0x08));
  ar7100_reg_wr_nf(AR9100_SPI_GPIO_OE,rd_val);

  /* Shift out SPI values keeping GPIO bits the same */
  for (ii = 7; ii>=0 ; ii--) {
    uint32_t  jj = (val >> ii) & 1;
    /* First assert data, make sure clock is low */

    if(jj)
    wr_val = (cs0to2mask | (1 << 22));
    else
    wr_val = cs0to2mask;
    /* Falling edge of the spi clk*/
    rd_val=ar7100_reg_rd(wr_val);
    /* Leave data there, rising edge of clock causes data to be output */
    rd_val=ar7100_reg_rd((wr_val | AR9100_SPI_CLK_HIGH));
  }
}
unsigned int
generic_spi_raw_input_u32_040(void)
{
  int ii;
  uint32_t val;
  uint32_t rd_val = 0x0;
  unsigned int cs0to2mask = 0xbf000000;

  val = (ar7100_reg_rd(AR9100_SPI_GPIO_OE) & ~(0x08));
  ar7100_reg_wr_nf(AR9100_SPI_GPIO_OE,val);
  val=ar7100_reg_rd(cs0to2mask);

  /* Only change GPIO 0, 1 or 2 in mask */
  /* For each rising edge the data in is sampled and clocked into serial shift register */
  for (ii = 31; ii>=0 ; ii--) {

    val=ar7100_reg_rd((cs0to2mask | AR9100_SPI_CLK_HIGH));
    val = ar7100_reg_rd(AR9100_SPI_GPIO_IN);
    rd_val = rd_val | (((val >> 3) & 0x1)<< ii);
    val=ar7100_reg_rd(cs0to2mask);
  }

    val=ar7100_reg_rd(cs0to2mask);

  /* Shift register contains value read */
  return rd_val;
}

void
generic_spi_raw_output_u8(unsigned char val)
{
	if (board_version >= 50) {
		generic_spi_raw_output_u8_050(val);
	} else {
		generic_spi_raw_output_u8_040(val);
	}
}

unsigned int
generic_spi_raw_input_u32(void)
{
	if (board_version >= 50) {
		return generic_spi_raw_input_u32_050();
	} else {
		return generic_spi_raw_input_u32_040();
	}
}

#else
int
generic_spi_access_enable(int cs)
{

  protect();
  
  ar7100_reg_wr(GENERIC_SPI_CLOCK, 0x43);
  
  /* Enable access to SPI bits through memory
   * mapped registers.
   */
  ar7100_reg_wr_nf(GENERIC_SPI_FS, 1);
  ar7100_reg_wr_nf(GENERIC_SPI_WRITE, GENERIC_SPI_CS_DIS);     
  
  {	  
    unsigned int cs0to2mask;
    cs0to2mask = ar7100_reg_rd(GENERIC_SPI_WRITE) & GENERIC_SPI_CS_DIS;
    ar7100_reg_wr  (GENERIC_SPI_WRITE, cs0to2mask | GENERIC_SPI_CLK_HIGH );
  }

  generic_i2c_assert_cs(cs);

  return 0;
}

int
generic_spi_access_done(void)
{
  generic_i2c_deassert_cs();
  ar7100_reg_wr_nf(GENERIC_SPI_FS, 0);

  unprotect();

  return 0;
}

int
generic_spi_init(int cs)
{
  /* For now just initialize the i2c, this will be different for bdi/ecos/linux */

  return generic_i2c_init_gpio();
}

/* Data needs to be stable on rising edge of clock */

void
generic_spi_raw_output_u8(unsigned char val) 
{
  int ii;

  unsigned int cs0to2mask = ar7100_reg_rd(GENERIC_SPI_WRITE) & ~(GENERIC_SPI_D0_HIGH|GENERIC_SPI_CLK_HIGH);

  /* Shift out SPI values keeping GPIO bits the same */
  for (ii = 7; ii>=0 ; ii--) {
    unsigned char  jj = (val >> ii) & 1;
    
    ar7100_reg_wr(GENERIC_SPI_WRITE, cs0to2mask | jj );
    ar7100_reg_wr(GENERIC_SPI_WRITE, cs0to2mask | jj | GENERIC_SPI_CLK_HIGH );
  }
}

unsigned int 
generic_spi_raw_input_u32(void) 
{
  int ii;

  unsigned int cs0to2mask = ar7100_reg_rd(GENERIC_SPI_WRITE) & ~(GENERIC_SPI_D0_HIGH|GENERIC_SPI_CLK_HIGH);

  /* For each rising edge the data in is sampled and clocked into serial shift register */
  for (ii = 31; ii>=0 ; ii--) {
    ar7100_reg_wr(GENERIC_SPI_WRITE, cs0to2mask                            );
    ar7100_reg_wr(GENERIC_SPI_WRITE, cs0to2mask | GENERIC_SPI_CLK_HIGH     );
  }

  /* Shift register contains value read */
  return ar7100_reg_rd(GENERIC_SPI_RD_SHIFT_REG);
}

unsigned int 
generic_spi_raw_input_u8(void) 
{
  int ii;

  unsigned int cs0to2mask = ar7100_reg_rd(GENERIC_SPI_WRITE) & ~(GENERIC_SPI_D0_HIGH|GENERIC_SPI_CLK_HIGH);

  /* For each rising edge the data in is sampled and clocked into serial shift register */
  for (ii = 7; ii>=0 ; ii--) {
    ar7100_reg_wr(GENERIC_SPI_WRITE, cs0to2mask                            );
    ar7100_reg_wr(GENERIC_SPI_WRITE, cs0to2mask | GENERIC_SPI_CLK_HIGH     );
  }

  /* Shift register contains value read */
  return ar7100_reg_rd(GENERIC_SPI_RD_SHIFT_REG);
}
#endif
