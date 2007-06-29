/* ------------------------------------------------------------------------- */
/* spi-algo-bit.c spi driver algorithms for bit-shift adapters		     */
/* ------------------------------------------------------------------------- */
/*   Copyright (C) 2005 Barnabas Kalman <ba...@sednet.hu>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.		     */
/* ------------------------------------------------------------------------- */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi-algo-bit.h>


/* ----- global defines ----------------------------------------------- */
#define DEB(x) if (spi_debug>=1) x;
#define DEB2(x) if (spi_debug>=2) x;
#define DEBSTAT(x) if (spi_debug>=3) x; /* print several statistical values*/
#define DEBPROTO(x) if (spi_debug>=9) { x; }
 	/* debug the protocol by showing transferred bits */

#ifdef DEBUG
#define TRACE pr_debug("%s: %d\n",__FUNCTION__,__LINE__)
#else
#define TRACE
#endif
/* ----- global variables ---------------------------------------------	*/

/* module parameters:
 */
static int spi_debug;
static int bit_test;	/* see if the line-setting functions work	*/

/* --- setting states on the bus with the right timing: ---------------	*/


static void bit_select( struct spi_algo_bit_data *adap, int index )
{
    (*adap->setspis)( adap->data, index );
    (*adap->setspic)( adap->data, 1 );
    udelay( adap->udelay );
}

static int bit_outb(struct spi_adapter *spi_adap, u8 c)
{
	struct spi_algo_bit_data *adap = spi_adap->algo_data;
	u8 mask;
	
	for (mask = 0x80; mask; mask >>= 1) {
		/* Clock low */
		(*adap->setspic)( adap->data, 0 );

		(*adap->setspid)( adap->data, 0 != (mask & c) );

   		udelay( adap->udelay );

		(*adap->setspic)( adap->data, 1 );

   		udelay( adap->udelay );
	}
	
	return 1;
}

static int bit_inb(struct spi_adapter *spi_adap)
{
	struct spi_algo_bit_data *adap = spi_adap->algo_data;
	u8 indata = 0;
	u8 mask;

	for (mask = 0x80; mask; mask >>= 1) {
		/* Clock low */
		(*adap->setspic)( adap->data, 0 );
			
   		udelay( adap->udelay );

		/* Sample on falling edge */
		if( (*adap->getspiq)( adap->data ) )
				indata |= mask;

		(*adap->setspic)( adap->data, 1 );

   		udelay( adap->udelay );
	}
	
	return (int)indata;
}


static int bit_sendbytes(struct spi_adapter *spi_adap, struct spi_msg *msg)
{
	char c;
	const char *temp = msg->buf;
	int count = msg->len;
	int retval;
	int wrcount=0;

	while (count > 0) {
		c = *temp;
		DEB2(dev_dbg(&spi_adap->dev, "sendbytes: writing %2.2X\n", c&0xff));
		retval = bit_outb(spi_adap,c);
		if ((retval>0))  { /* ok or ignored NAK */
			count--; 
			temp++;
			wrcount++;
		} else { /* arbitration or no acknowledge */
			dev_err(&spi_adap->dev, "sendbytes: error - bailout.\n");
//			spi_stop(adap);
			return (retval<0)? retval : -EFAULT;
			        /* got a better one ?? */
		}
	}
	return wrcount;
}

static inline int bit_readbytes(struct spi_adapter *spi_adap, struct spi_msg *msg)
{
	int inval;
	int rdcount=0;   	/* counts bytes read */
	char *temp = msg->buf;
	int count = msg->len;

	while (count > 0) {
		inval = bit_inb(spi_adap);
/*printk("%#02x ",inval); if ( ! (count % 16) ) printk("\n"); */
		if (inval>=0) {
			*temp = inval;
			rdcount++;
		} else {   /* read timed out */
			printk(KERN_ERR "spi-algo-bit.o: readbytes: spi_inb timed out.\n");
			break;
		}

		temp++;
		count--;
	}
	return rdcount;
}

static int bit_xfer(struct spi_adapter *spi_adap,
                    struct spi_msg msgs[], int num)
{
    struct spi_msg *pmsg;
    struct spi_algo_bit_data *adap = spi_adap->algo_data;
    u16 addr = !0u;
    
    int i,ret;

    for( i=0; i<num; i++ ) {
		
		pmsg = &msgs[i];
		
		if( addr != pmsg->addr ) {
			bit_select( adap, (int)(__u32)pmsg->addr );
			addr = pmsg->addr;
		}
		
		if( pmsg->flags & SPI_M_RD ) {
            /* read bytes into buffer*/
		    ret = bit_readbytes( spi_adap, pmsg );
		    DEB2(printk(KERN_DEBUG "spi-algo-bit.o: read %d bytes.\n",ret));
	    	if( ret < pmsg->len ) {
			return (ret<0)? ret : -EREMOTEIO;
		    }
		} else {
		    /* write bytes from buffer */
	    	ret = bit_sendbytes( spi_adap, pmsg );
		    DEB2(printk(KERN_DEBUG "spi-algo-bit.o: wrote %d bytes.\n",ret));
		    if (ret < pmsg->len ) {
				return (ret<0) ? ret : -EREMOTEIO;
		    }
		}
    }
    
    bit_select( adap, -1 );
    
    return num;
}
		     

static u32 bit_func(struct spi_adapter *adap)
{
    return SPI_FUNC_SPI;
}
		       
		       
/* -----exported algorithm data: -------------------------------------	*/

static struct spi_algorithm spi_bit_algo = {
	.name		= "Bit-shift algorithm",
	.id		= SPI_ALGO_BIT,
	.master_xfer	= bit_xfer,
	.functionality	= bit_func,
};

/* 
 * registering functions to load algorithms at runtime 
 */
int spi_bit_add_bus(struct spi_adapter *adap)
{
/*	struct spi_algo_bit_data *bit_adap = adap->algo_data;

	if (bit_test) {
		int ret = test_bus(bit_adap, adap->name);
		if (ret<0)
			return -ENODEV;
	}
*/

	/* register new adapter to spi module... */

	adap->id |= spi_bit_algo.id;
	adap->algo = &spi_bit_algo;

	adap->timeout = 100;	/* default values, should	*/
	adap->retries = 3;	/* be replaced by defines	*/

	spi_add_adapter(adap);

	DEB2(dev_dbg(&adap->dev, "hw routines registered.\n"));

	return 0;
}


int spi_bit_del_bus(struct spi_adapter *adap)
{
	return spi_del_adapter(adap);
}

EXPORT_SYMBOL(spi_bit_add_bus);
EXPORT_SYMBOL(spi_bit_del_bus);

MODULE_AUTHOR("Barnabas Kalman <ba...@sednet.hu>");
MODULE_DESCRIPTION("SPI-Bus bit-banging algorithm");
MODULE_LICENSE("GPL");

module_param(bit_test, bool, 0);
module_param(spi_debug, int, S_IRUGO | S_IWUSR);

MODULE_PARM_DESC(bit_test, "Test the lines of the bus to see if it is stuck");
MODULE_PARM_DESC(spi_debug, "debug level - 0 off; 1 normal; 2,3 more verbose; 9 bit-protocol");
