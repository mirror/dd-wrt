
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
