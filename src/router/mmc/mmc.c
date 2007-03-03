#include <linux/delay.h>
#include <linux/timer.h>

#include <linux/module.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/blkpg.h>
#include <linux/hdreg.h>
#include <linux/major.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define DEVICE_NAME "mmc"
#define DEVICE_NR(device) (MINOR(device))
#define DEVICE_ON(device)
#define DEVICE_OFF(device)
#define MAJOR_NR 121

#include <linux/blk.h>


MODULE_AUTHOR
  ("Madsuk/Rohde (speedup Cyril CATTIAUX v1.2,GPIO detection BrainSlayer)");
MODULE_DESCRIPTION ("Driver MMC/SD-Cards");
MODULE_SUPPORTED_DEVICE ("WRT54G");
MODULE_LICENSE ("GPL");

/* GPIO pin 2 */

#define SD_DIV1 0x20
#define SD_DIV4 0x04
#define SD_DIVBUF 0x40

#define SD_DOWRT 0x10		// pin 4
#define SD_DOBUF 0x20		// pin 5

/* GPIO pin 3 */
#define SD_CLK 0x08
/* GPIO pin 7 */
#define SD_CS 0x80

static int SD_DI = SD_DIV1;
static int SD_DO = SD_DOWRT;

/* we have only one device */
static int hd_sizes[1 << 6];
static int hd_blocksizes[1 << 6];
static int hd_hardsectsizes[1 << 6];
static int hd_maxsect[1 << 6];
static struct hd_struct hd[1 << 6];

static int mmc_media_detect = 0;
static int mmc_media_changed = 1;

typedef unsigned int uint32;

static unsigned char port_state = 0x00;
static volatile uint32 *gpioaddr_input = (uint32 *) 0xb8000060;
static volatile uint32 *gpioaddr_output = (uint32 *) 0xb8000064;
static volatile uint32 *gpioaddr_enable = (uint32 *) 0xb8000068;
//static volatile uint32 *gpioaddr_control = (uint32 *)0xb800006c;

static unsigned char ps_di, ps_di_clk, ps_clk;
#define NOT_DI_NOT_CLK ((~SD_DI) & (~SD_CLK))
#define DI_CLK (SD_DI | SD_CLK)

static inline void
mmc_spi_cs_low (void)
{
  port_state &= ~(SD_CS);
  ps_di = (port_state | SD_DI);
  ps_di_clk = (port_state | DI_CLK);
  ps_clk = (port_state | SD_CLK);
  *gpioaddr_output = port_state;
}

static inline void
mmc_spi_cs_high (void)
{
  port_state |= SD_CS;
  ps_di = (port_state | SD_DI);
  ps_di_clk = (port_state | DI_CLK);
  ps_clk = (port_state | SD_CLK);
  *gpioaddr_output = port_state;
}

static inline void
mmc_spi_io_ff_v (void)
{
  const unsigned char l_ps_di = ps_di;
  const unsigned char l_ps_di_clk = ps_di_clk;
  volatile uint32 *l_gpioaddr_output = gpioaddr_output;

  *l_gpioaddr_output = l_ps_di;
  *l_gpioaddr_output = l_ps_di_clk;
  *l_gpioaddr_output = l_ps_di;
  *l_gpioaddr_output = l_ps_di_clk;
  *l_gpioaddr_output = l_ps_di;
  *l_gpioaddr_output = l_ps_di_clk;
  *l_gpioaddr_output = l_ps_di;
  *l_gpioaddr_output = l_ps_di_clk;
  *l_gpioaddr_output = l_ps_di;
  *l_gpioaddr_output = l_ps_di_clk;
  *l_gpioaddr_output = l_ps_di;
  *l_gpioaddr_output = l_ps_di_clk;
  *l_gpioaddr_output = l_ps_di;
  *l_gpioaddr_output = l_ps_di_clk;
  *l_gpioaddr_output = l_ps_di;
  *l_gpioaddr_output = l_ps_di_clk;
}

static inline unsigned char
mmc_spi_io_ff (void)
{
  const unsigned char l_ps_di = ps_di;
  const unsigned char l_ps_di_clk = ps_di_clk;
  volatile uint32 *l_gpioaddr_output = gpioaddr_output;
  volatile uint32 *l_gpioaddr_input = gpioaddr_input;

  unsigned char result = 0;
  *l_gpioaddr_output = l_ps_di;
  *l_gpioaddr_output = l_ps_di_clk;
  result |= ((*l_gpioaddr_input) & SD_DO) << 3;
  *l_gpioaddr_output = l_ps_di;
  *l_gpioaddr_output = l_ps_di_clk;
  result |= ((*l_gpioaddr_input) & SD_DO) << 2;
  *l_gpioaddr_output = l_ps_di;
  *l_gpioaddr_output = l_ps_di_clk;
  result |= ((*l_gpioaddr_input) & SD_DO) << 1;
  *l_gpioaddr_output = l_ps_di;
  *l_gpioaddr_output = l_ps_di_clk;
  result |= ((*l_gpioaddr_input) & SD_DO);
  *l_gpioaddr_output = l_ps_di;
  *l_gpioaddr_output = l_ps_di_clk;
  result |= ((*l_gpioaddr_input) & SD_DO) >> 1;
  *l_gpioaddr_output = l_ps_di;
  *l_gpioaddr_output = l_ps_di_clk;
  result |= ((*l_gpioaddr_input) & SD_DO) >> 2;
  *l_gpioaddr_output = l_ps_di;
  *l_gpioaddr_output = l_ps_di_clk;
  result |= ((*l_gpioaddr_input) & SD_DO) >> 3;
  *l_gpioaddr_output = l_ps_di;
  *l_gpioaddr_output = l_ps_di_clk;
  result |= ((*l_gpioaddr_input) & SD_DO) >> 4;

  return result;
}

static inline void
mmc_spi_io_v (unsigned char data_out)
{
  unsigned char di;
  const unsigned char l_port_state = port_state;
  const unsigned char l_ps_clk = ps_clk;
  volatile uint32 *l_gpioaddr_output = gpioaddr_output;

  di = (data_out & 0x80) >> 5;
  *l_gpioaddr_output = l_port_state | di;
  *l_gpioaddr_output = l_ps_clk | di;
  di = (data_out & 0x40) >> 4;
  *l_gpioaddr_output = l_port_state | di;
  *l_gpioaddr_output = l_ps_clk | di;
  di = (data_out & 0x20) >> 3;
  *l_gpioaddr_output = l_port_state | di;
  *l_gpioaddr_output = l_ps_clk | di;
  di = (data_out & 0x10) >> 2;
  *l_gpioaddr_output = l_port_state | di;
  *l_gpioaddr_output = l_ps_clk | di;
  di = (data_out & 0x08) >> 1;
  *l_gpioaddr_output = l_port_state | di;
  *l_gpioaddr_output = l_ps_clk | di;
  di = (data_out & 0x04);
  *l_gpioaddr_output = l_port_state | di;
  *l_gpioaddr_output = l_ps_clk | di;
  di = (data_out & 0x02) << 1;
  *l_gpioaddr_output = l_port_state | di;
  *l_gpioaddr_output = l_ps_clk | di;
  di = (data_out & 0x01) << 2;
  *l_gpioaddr_output = l_port_state | di;
  *l_gpioaddr_output = l_ps_clk | di;
}

static unsigned char
mmc_spi_io (unsigned char data_out)
{
  unsigned char result = 0;
  unsigned char di;
  const unsigned char l_port_state = port_state;
  const unsigned char l_ps_clk = ps_clk;
  volatile uint32 *l_gpioaddr_output = gpioaddr_output;
  volatile uint32 *l_gpioaddr_input = gpioaddr_input;
  di = (data_out & 0x80) >> 5;
  *l_gpioaddr_output = l_port_state | di;
  *l_gpioaddr_output = l_ps_clk | di;
  result |= ((*l_gpioaddr_input) & SD_DO) << 3;
  di = (data_out & 0x40) >> 4;
  *l_gpioaddr_output = l_port_state | di;
  *l_gpioaddr_output = l_ps_clk | di;
  result |= ((*l_gpioaddr_input) & SD_DO) << 2;
  di = (data_out & 0x20) >> 3;
  *l_gpioaddr_output = l_port_state | di;
  *l_gpioaddr_output = l_ps_clk | di;
  result |= ((*l_gpioaddr_input) & SD_DO) << 1;
  di = (data_out & 0x10) >> 2;
  *l_gpioaddr_output = l_port_state | di;
  *l_gpioaddr_output = l_ps_clk | di;
  result |= ((*l_gpioaddr_input) & SD_DO);
  di = (data_out & 0x08) >> 1;
  *l_gpioaddr_output = l_port_state | di;
  *l_gpioaddr_output = l_ps_clk | di;
  result |= ((*l_gpioaddr_input) & SD_DO) >> 1;
  di = (data_out & 0x04);
  *l_gpioaddr_output = l_port_state | di;
  *l_gpioaddr_output = l_ps_clk | di;
  result |= ((*l_gpioaddr_input) & SD_DO) >> 2;
  di = (data_out & 0x02) << 1;
  *l_gpioaddr_output = l_port_state | di;
  *l_gpioaddr_output = l_ps_clk | di;
  result |= ((*l_gpioaddr_input) & SD_DO) >> 3;
  di = (data_out & 0x01) << 2;
  *l_gpioaddr_output = l_port_state | di;
  *l_gpioaddr_output = l_ps_clk | di;
  result |= ((*l_gpioaddr_input) & SD_DO) >> 4;

  return (result);
}

#define USE_CMD25
static inline int
mmc_write_block (unsigned int dest_addr, unsigned char *data, int nbsectors)
{
  unsigned char r = 0;
  int i, k;
  unsigned char *pdata = data;
  //printk("nbsectors: %d\n", nbsectors);
  mmc_spi_cs_low ();

  mmc_spi_io_ff_v ();
  mmc_spi_io_ff_v ();
  mmc_spi_io_ff_v ();
  mmc_spi_io_ff_v ();

#ifdef USE_CMD25
  mmc_spi_io (0x59);		/* CMD25 */
#else
  mmc_spi_io (0x58);		/* CMD24 */
#endif
  mmc_spi_io_v (0xff & (dest_addr >> 24));	/* msb */
  mmc_spi_io_v (0xff & (dest_addr >> 16));
  mmc_spi_io_v (0xff & (dest_addr >> 8));
  mmc_spi_io_v (0xff & dest_addr);	/* lsb */
  mmc_spi_io_ff_v ();		/* dummy CRC */

  for (i = 0; i < 8; i++)
    {
      r = mmc_spi_io_ff ();	/*  command response */
      if (r != 0xff)
	break;
    }
  if (r != 0x00)
    {
      mmc_spi_cs_high ();
      mmc_spi_io_ff_v ();
      return (-r);
    }

#ifdef USE_CMD25
  for (k = 0; k < nbsectors; k++)
    {
      /* data token */
      mmc_spi_io_v (0xfc);
#else
  /* data token */
  mmc_spi_io_v (0xfe);
#endif

  /* data block */
  i = 0;
  while (i < 512)
    {
      mmc_spi_io_v (*pdata);
      pdata++;
      i++;
    }

  /* dummy CRC */
  mmc_spi_io_ff_v ();
  mmc_spi_io_ff_v ();

  for (i = 0; i < 8; i++)
    {
      r = mmc_spi_io_ff ();	/*  data response */
      if (r != 0xff)
	break;
    }
  if (!(r & 0x05))
    {
      // data rejected
      mmc_spi_cs_high ();
      mmc_spi_io_ff_v ();
      return (2);
    }

  /* busy... */
  for (i = 0; i < 1000000; i++)
    {
      r = mmc_spi_io_ff ();
      if (r == 0xff)
	break;
    }
  if (r != 0xff)
    {
      mmc_spi_cs_high ();
      mmc_spi_io_ff_v ();
      return (3);
    }
#ifdef USE_CMD25
}

mmc_spi_io_v (0xfd);
mmc_spi_io_ff_v ();		/* dummy args */
mmc_spi_io_ff_v ();
mmc_spi_io_ff_v ();
mmc_spi_io_ff_v ();
mmc_spi_io_ff_v ();		/* dummy CRC */

mmc_spi_io_ff_v ();		// skipping one byte
    //busy...
for (i = 0; i < 1000000; i++)
  {
    yield ();
    r = mmc_spi_io_ff ();
    if (r == 0xff)
      break;
  }

if (r != 0xff)
  {
    mmc_spi_cs_high ();
    mmc_spi_io_ff_v ();
    return (-4);
  }
#endif
mmc_spi_cs_high ();
mmc_spi_io_ff_v ();
return (0);
}

#define USE_CMD18
static inline int
mmc_read_block (unsigned char *data, unsigned int src_addr, int nbsectors)
{
  unsigned char r = 0;
  int i, k;
  unsigned char *pdata = data;

  mmc_spi_cs_low ();

  mmc_spi_io_ff_v ();
  mmc_spi_io_ff_v ();
  mmc_spi_io_ff_v ();
  mmc_spi_io_ff_v ();

#ifdef USE_CMD18
  mmc_spi_io (0x52);		/* CMD18 */
#else
  mmc_spi_io (0x51);		/* CMD17 */
#endif
  mmc_spi_io (0xff & (src_addr >> 24));	/* msb */
  mmc_spi_io (0xff & (src_addr >> 16));
  mmc_spi_io (0xff & (src_addr >> 8));
  mmc_spi_io (0xff & src_addr);	/* lsb */
  mmc_spi_io_ff_v ();		/* dummy CRC */


  for (i = 0; i < 8; i++)
    {
      r = mmc_spi_io_ff ();	/*  command response */
      if (r != 0xff)
	break;
    }
  if (r != 0x00)
    {
      mmc_spi_cs_high ();
      mmc_spi_io_ff_v ();
      return (-r);
    }

#ifdef USE_CMD18
  for (k = 0; k < nbsectors; k++)
    {
#endif
      for (i = 0; i < 1000000; i++)
	{
	  r = mmc_spi_io_ff ();
	  if (r != 0xff)
	    break;
	}
      if (r != 0xfe)
	{
	  mmc_spi_cs_high ();
	  mmc_spi_io_ff_v ();
	  return 3;
	}

      /* reading data packet */
      for (i = 0; i < 512; i++)
	{
	  r = mmc_spi_io_ff ();
	  *pdata = r;
	  pdata++;
	}
      /* skipping crc */
      mmc_spi_io_ff_v ();
      mmc_spi_io_ff_v ();
#ifdef USE_CMD18
    }
  yield ();

  mmc_spi_io_v (0x4c);		/* CMD12 */
  mmc_spi_io_ff_v ();		/* dummy args */
  mmc_spi_io_ff_v ();
  mmc_spi_io_ff_v ();
  mmc_spi_io_ff_v ();
  mmc_spi_io_ff_v ();		/* dummy CRC */

  mmc_spi_io_ff_v ();		/* skipping stuff byte */

  /* skipping 1-8 bytes and get R1 */
  for (i = 0; i < 9; i++)
    {
      r = mmc_spi_io_ff ();
      if (r != 0xff)
	break;
    }
  if (r != 0x00)
    {
      mmc_spi_cs_high ();
      mmc_spi_io_ff_v ();
      return 4;
    }

#endif

  mmc_spi_cs_high ();
  mmc_spi_io_ff_v ();

  return (0);
}


static void
mmc_request (request_queue_t * q)
{
  unsigned int mmc_address;
  unsigned char *buffer_address;
  int nr_sectors;
  int cmd;
  int rc, code;

  (void) q;
  while (1)
    {
      code = 1;			// Default is success
      INIT_REQUEST;
      mmc_address =
	(CURRENT->sector +
	 hd[MINOR (CURRENT->rq_dev)].start_sect) * hd_hardsectsizes[0];
      buffer_address = CURRENT->buffer;
      nr_sectors = CURRENT->current_nr_sectors;
      cmd = CURRENT->cmd;
      if (((CURRENT->sector + CURRENT->current_nr_sectors +
	    hd[MINOR (CURRENT->rq_dev)].start_sect) > hd[0].nr_sects)
	  || (mmc_media_detect == 0))
	{
	  code = 0;
	}
      else if (cmd == READ)
	{
	  spin_unlock_irq (&io_request_lock);
#ifndef USE_CMD18
	  for (i = 0; i < nr_sectors; i++)
	    {
#endif
	      rc = mmc_read_block (buffer_address, mmc_address, nr_sectors);
	      if (rc != 0)
		{
		  printk ("mmc: error in mmc_read_block (%d)\n", rc);
		  code = 0;
		  break;
		}
	      else
		{
#ifdef USE_CMD18
		  mmc_address += hd_hardsectsizes[0] * nr_sectors;
		  buffer_address += hd_hardsectsizes[0] * nr_sectors;
#else
		  mmc_address += hd_hardsectsizes[0];
		  buffer_address += hd_hardsectsizes[0];
#endif
		}
#ifndef USE_CMD18
	    }
#endif
	  spin_lock_irq (&io_request_lock);
	}
      else if (cmd == WRITE)
	{
	  spin_unlock_irq (&io_request_lock);
#ifndef USE_CMD25
	  for (i = 0; i < nr_sectors; i++)
	    {
#endif
	      rc = mmc_write_block (mmc_address, buffer_address, nr_sectors);
	      if (rc != 0)
		{
		  printk ("mmc: error in mmc_write_block (%d)\n", rc);
		  code = 0;
		  break;
		}
	      else
		{
#ifdef USE_CMD25
		  mmc_address += hd_hardsectsizes[0] * nr_sectors;
		  buffer_address += hd_hardsectsizes[0] * nr_sectors;
#else
		  mmc_address += hd_hardsectsizes[0];
		  buffer_address += hd_hardsectsizes[0];
#endif
		}
#ifndef USE_CMD25
	    }
#endif
	  spin_lock_irq (&io_request_lock);
	}
      else
	{
	  code = 0;
	}
      end_request (code);
    }
}


static int
mmc_open (struct inode *inode, struct file *filp)
{
  (void) filp;

  if (mmc_media_detect == 0)
    return -ENODEV;

#if defined(MODULE)
  MOD_INC_USE_COUNT;
#endif
  return 0;
}

static int
mmc_release (struct inode *inode, struct file *filp)
{
  (void) filp;
  fsync_dev (inode->i_rdev);
  invalidate_buffers (inode->i_rdev);

#if defined(MODULE)
  MOD_DEC_USE_COUNT;
#endif
  return 0;
}

extern struct gendisk hd_gendisk;
static int
mmc_revalidate (kdev_t dev)
{
  int target, max_p, start, i;
  if (mmc_media_detect == 0)
    return -ENODEV;

  target = DEVICE_NR (dev);

  max_p = hd_gendisk.max_p;
  start = target << 6;
  for (i = max_p - 1; i >= 0; i--)
    {
      int minor = start + i;
      invalidate_device (MKDEV (MAJOR_NR, minor), 1);
      hd_gendisk.part[minor].start_sect = 0;
      hd_gendisk.part[minor].nr_sects = 0;
    }

  grok_partitions (&hd_gendisk, target, 1 << 6, hd_sizes[0] * 2);

  return 0;
}

static int
mmc_ioctl (struct inode *inode, struct file *filp, unsigned int cmd,
	   unsigned long arg)
{
  if (!inode || !inode->i_rdev)
    return -EINVAL;

  switch (cmd)
    {
    case BLKGETSIZE:
      return put_user (hd[MINOR (inode->i_rdev)].nr_sects,
		       (unsigned long *) arg);
    case BLKGETSIZE64:
      return put_user ((u64) hd[MINOR (inode->i_rdev)].nr_sects, (u64 *) arg);
    case BLKRRPART:
      if (!capable (CAP_SYS_ADMIN))
	return -EACCES;

      return mmc_revalidate (inode->i_rdev);
    case HDIO_GETGEO:
      {
	struct hd_geometry *loc, g;
	loc = (struct hd_geometry *) arg;
	if (!loc)
	  return -EINVAL;
	g.heads = 4;
	g.sectors = 16;
	g.cylinders = hd[0].nr_sects / (4 * 16);
	g.start = hd[MINOR (inode->i_rdev)].start_sect;
	return copy_to_user (loc, &g, sizeof (g)) ? -EFAULT : 0;
      }
    default:
      return blk_ioctl (inode->i_rdev, cmd, arg);
    }
}

static int
mmc_card_init (void)
{
  unsigned char r = 0;
  short i, j;
  unsigned long flags;

  save_flags (flags);
  cli ();

  printk ("mmc Card init\n");
  mmc_spi_cs_high ();
  for (i = 0; i < 20; i++)
    mmc_spi_io_ff ();

  mmc_spi_cs_low ();

  mmc_spi_io (0x40);
  for (i = 0; i < 4; i++)
    mmc_spi_io (0x00);
  mmc_spi_io (0x95);
  for (i = 0; i < 8; i++)
    {
      r = mmc_spi_io_ff ();
      if (r == 0x01)
	break;
    }
  mmc_spi_cs_high ();
  mmc_spi_io_ff ();
  if (r != 0x01)
    {
      restore_flags (flags);
      return (1);
    }

  printk ("mmc Card init *1*\n");
  for (j = 0; j < 10000; j++)
    {
      mmc_spi_cs_low ();

      /* CMD1 - init */
      mmc_spi_io (0x41);
      for (i = 0; i < 4; i++)
	mmc_spi_io (0x00);	/* dummy params */
      mmc_spi_io_ff ();		/* dummy crc */

      for (i = 0; i < 8; i++)
	{
	  r = mmc_spi_io_ff ();
	  if (r == 0x00)
	    break;
	}
      mmc_spi_cs_high ();
      mmc_spi_io_ff ();
      if (r == 0x00)
	{
	  restore_flags (flags);
	  printk ("mmc Card init *2*\n");
	  return (0);
	}
    }
  restore_flags (flags);

  return (2);
}

static int
mmc_card_config (void)
{
  unsigned char r = 0;
  short i;
  unsigned char csd[32];
  unsigned int c_size;
  unsigned int c_size_mult;
  unsigned int mult;
  unsigned int read_bl_len;
  unsigned int blocknr = 0;
  unsigned int block_len = 0;
  unsigned int size = 0;

  mmc_spi_cs_low ();
  for (i = 0; i < 4; i++)
    mmc_spi_io_ff ();
  mmc_spi_io (0x49);
  for (i = 0; i < 4; i++)
    mmc_spi_io (0x00);
  mmc_spi_io_ff ();
  for (i = 0; i < 8; i++)
    {
      r = mmc_spi_io_ff ();
      if (r == 0x00)
	break;
    }
  if (r != 0x00)
    {
      mmc_spi_cs_high ();
      mmc_spi_io_ff ();
      return (1);
    }
  for (i = 0; i < 8; i++)
    {
      r = mmc_spi_io_ff ();
      if (r == 0xfe)
	break;
    }
  if (r != 0xfe)
    {
      mmc_spi_cs_high ();
      mmc_spi_io_ff ();
      return (2);
    }
  for (i = 0; i < 16; i++)
    {
      r = mmc_spi_io_ff ();
      csd[i] = r;
    }
  for (i = 0; i < 2; i++)
    {
      r = mmc_spi_io_ff ();
    }
  mmc_spi_cs_high ();
  mmc_spi_io_ff ();
  if (r == 0x00)
    return (3);

  c_size = csd[8] + csd[7] * 256 + (csd[6] & 0x03) * 256 * 256;
  c_size >>= 6;
  c_size_mult = csd[10] + (csd[9] & 0x03) * 256;
  c_size_mult >>= 7;
  read_bl_len = csd[5] & 0x0f;
  mult = 1;
  mult <<= c_size_mult + 2;
  blocknr = (c_size + 1) * mult;
  block_len = 1;
  block_len <<= read_bl_len;
  size = block_len * blocknr;
  size >>= 10;

  for (i = 0; i < (1 << 6); i++)
    {
      hd_blocksizes[i] = 1024;
      hd_hardsectsizes[i] = block_len;
      hd_maxsect[i] = 256;
    }
  hd_sizes[0] = size;
  hd[0].nr_sects = blocknr;


  printk ("Size = %d, hardsectsize = %d, sectors = %d\n",
	  size, block_len, blocknr);

  return 0;
}

static int
mmc_hardware_init (void)
{
  unsigned char gpio_outen;

  // Set inputs/outputs here
  printk ("mmc Hardware init\n");
  gpio_outen = *gpioaddr_enable;

  gpio_outen = (gpio_outen | SD_DI | SD_CLK | SD_CS) & ~SD_DO;
  *gpioaddr_enable = gpio_outen;

  port_state = *gpioaddr_input;

  // Clock low
  port_state &= ~(SD_CLK | SD_DI | SD_CS);
  *gpioaddr_output = port_state;

  return 0;
}

#if 0
static int
mmc_check_media_change (kdev_t dev)
{
  (void) dev;
  if (mmc_media_changed == 1)
    {
      mmc_media_changed = 0;
      return 1;
    }
  else
    return 0;
}
#endif
static struct block_device_operations mmc_bdops = {
open:mmc_open,
release:mmc_release,
ioctl:mmc_ioctl,
#if 0
check_media_change:mmc_check_media_change,
revalidate:mmc_revalidate,
#endif
};

static struct gendisk hd_gendisk = {
major:MAJOR_NR,
major_name:DEVICE_NAME,
minor_shift:6,
max_p:1 << 6,
part:hd,
sizes:hd_sizes,
fops:&mmc_bdops,
};

static int
mmc_init (void)
{
  int rc;

  rc = mmc_hardware_init ();

  if (rc != 0)
    {

      SD_DI = SD_DIV4;		//try second one
      rc = mmc_hardware_init ();
      if (rc != 0)
	{
	  SD_DI = SD_DIVBUF;	//try third one
	  SD_DO = SD_DOBUF;
	  rc = mmc_hardware_init ();
	  if (rc != 0)
	    {
	      printk ("mmc %s: unable to initialize hardware\n", __func__);
	    }

	}
    }


  rc = mmc_card_init ();
  if (rc != 0)
    {
      // Give it an extra shot
      rc = mmc_card_init ();
      if (rc != 0)
	{
	  printk
	    ("mmc %s: Device does not use old GPIO layout, trying to use new GPIO layout\n",
	     __func__);
	  SD_DI = SD_DIV4;	//try second one
	  SD_DO = SD_DOWRT;
	  mmc_hardware_init ();
	  rc = mmc_card_init ();
	  if (rc != 0)
	    {
	      // Give it an extra shot
	      rc = mmc_card_init ();
	      if (rc != 0)
		{
		  printk
		    ("mmc %s: Device does not use new GPIO layout, trying to use new Buffalo GPIO layout\n",
		     __func__);
		  SD_DI = SD_DIVBUF;
		  SD_DO = SD_DOBUF;
		  mmc_hardware_init ();
		  rc = mmc_card_init ();
		  if (rc != 0)
		    {
		      rc = mmc_card_init ();
		      if (rc != 0)
			{
			  printk
			    ("mmc %s: This board has no MMC mod installed!\n",__func__);
			  return -1;
			}
		    }
		}
	    }
	}
    }

  memset (hd_sizes, 0, sizeof (hd_sizes));
  rc = mmc_card_config ();
  if (rc != 0)
    {
      printk ("mmc: error in mmc_card_config (%d)\n", rc);
      return -1;
    }


  blk_size[MAJOR_NR] = hd_sizes;

  memset (hd, 0, sizeof (hd));
  hd[0].nr_sects = hd_sizes[0] * 2;

  blksize_size[MAJOR_NR] = hd_blocksizes;
  hardsect_size[MAJOR_NR] = hd_hardsectsizes;
  max_sectors[MAJOR_NR] = hd_maxsect;

  hd_gendisk.nr_real = 1;

  register_disk (&hd_gendisk, MKDEV (MAJOR_NR, 0), 1 << 6,
		 &mmc_bdops, hd_sizes[0] * 2);

  return 0;
}

static void
mmc_exit (void)
{
  blk_size[MAJOR_NR] = NULL;
  blksize_size[MAJOR_NR] = NULL;
  hardsect_size[MAJOR_NR] = NULL;
  max_sectors[MAJOR_NR] = NULL;
  hd[0].nr_sects = 0;
}

static void
mmc_check_media (void)
{
  int old_state;
  int rc;

  old_state = mmc_media_detect;

  // TODO: Add card detection here
  mmc_media_detect = 1;
  if (old_state != mmc_media_detect)
    {
      mmc_media_changed = 1;
      if (mmc_media_detect == 1)
	{
	  rc = mmc_init ();
	  if (rc != 0)
	    printk ("mmc: error in mmc_init (%d)\n", rc);
	}
      else
	{
	  mmc_exit ();
	}
    }

}

static int __init
mmc_driver_init (void)
{
  int rc;
  request_queue_t *queue;

  rc = devfs_register_blkdev (MAJOR_NR, DEVICE_NAME, &mmc_bdops);
  if (rc < 0)
    {
      printk (KERN_WARNING "mmc: can't get major %d\n", MAJOR_NR);
      return rc;
    }

  queue = BLK_DEFAULT_QUEUE (MAJOR_NR);
  //queue->max_queue_sectors = 16;
  blk_init_queue (queue, mmc_request);
  read_ahead[MAJOR_NR] = 8;
  add_gendisk (&hd_gendisk);

  mmc_check_media ();


  return 0;
}

static void __exit
mmc_driver_exit (void)
{
  int i;

  for (i = 0; i < (1 << 6); i++)
    fsync_dev (MKDEV (MAJOR_NR, i));

  blk_cleanup_queue (BLK_DEFAULT_QUEUE (MAJOR_NR));
  del_gendisk (&hd_gendisk);
  devfs_unregister_blkdev (MAJOR_NR, DEVICE_NAME);
  mmc_exit ();
}

module_init (mmc_driver_init);
module_exit (mmc_driver_exit);
