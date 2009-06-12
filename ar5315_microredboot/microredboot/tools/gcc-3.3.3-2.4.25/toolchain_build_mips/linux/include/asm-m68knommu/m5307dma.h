#ifndef _M5307_DMA_H
#define _M5307_DMA_H 1


#include <asm/coldfire.h>
#include <asm/mcfsim.h>
#include <asm/irq.h>


#define MAX_DMA_CHANNELS 4

/* 
*DMA Address Definitions
*/


#define MCF5307_DMA_SAR 	(MCF_MBAR+0x300)  /*Source Address Register Channel */
#define MCF5307_DMA_DAR 	(MCF_MBAR+0x304)  /*Destination Address Register Channel*/
#define MCF5307_DMA_DCR 	(MCF_MBAR+0x308)  /*DMA Controll Register Channel*/
#define MCF5307_DMA_BCR 	(MCF_MBAR+0x30C)  /*Byte Count Register Channel*/
#define MCF5307_DMA_SR 		(MCF_MBAR+0x310)  /*Status Register Channel*/
#define MCF5307_DMA_IVR 	(MCF_MBAR+0x314)  /*InterruptVectorRegister Channel*/

/*
*DMA Controll Register Definition
*/


#define MCF5307_DMA_DCR_INT             (0x8000) /* Interrupt on Completion     */
#define MCF5307_DMA_DCR_EEXT            (0x4000) /* Enable External Request     */
#define MCF5307_DMA_DCR_CS              (0x2000) /* Cycle Steal                 */
#define MCF5307_DMA_DCR_AA              (0x1000) /* Auto Align                  */
#define MCF5307_DMA_DCR_BWC_DMA         (0x0000) /* Bandwidth: DMA Priority     */
#define MCF5307_DMA_DCR_BWC_512         (0x0200) /* Bandwidth:   512 Bytes      */
#define MCF5307_DMA_DCR_BWC_1024        (0x0400) /* Bandwidth:  1024 Bytes      */
#define MCF5307_DMA_DCR_BWC_2048        (0x0600) /* Bandwidth:  2048 Bytes      */
#define MCF5307_DMA_DCR_BWC_4096        (0x0800) /* Bandwidth:  4096 Bytes      */
#define MCF5307_DMA_DCR_BWC_8192        (0x0a00) /* Bandwidth:  8192 Bytes      */
#define MCF5307_DMA_DCR_BWC_16384       (0x0c00) /* Bandwidth: 16384 Bytes      */
#define MCF5307_DMA_DCR_BWC_32768       (0x0e00) /* Bandwidth: 32768 Bytes      */
#define MCF5307_DMA_DCR_SAA             (0x0100) /* Single Address Access       */
#define MCF5307_DMA_DCR_SRW             (0x0080) /* Forces MRW Signal High      */
#define MCF5307_DMA_DCR_SINC            (0x0040) /* Source Increment            */
#define MCF5307_DMA_DCR_SSIZE_LONG      (0x0000) /* Source Size:  Longword      */
#define MCF5307_DMA_DCR_SSIZE_BYTE      (0x0010) /* Source Size:  Byte          */
#define MCF5307_DMA_DCR_SSIZE_WORD      (0x0020) /* Source Size:  Word          */
#define MCF5307_DMA_DCR_SSIZE_LINE      (0x0030) /* Source Size:  Line          */
#define MCF5307_DMA_DCR_DINC            (0x0008) /* Destination Increment       */
#define MCF5307_DMA_DCR_DSIZE_LONG      (0x0000) /* Destination Size:  Longword */
#define MCF5307_DMA_DCR_DSIZE_BYTE      (0x0002) /* Destination Size:  Byte     */
#define MCF5307_DMA_DCR_DSIZE_WORD      (0x0004) /* Destination Size:  Word     */
#define MCF5307_DMA_DCR_DSIZE_LINE      (0x0006) /* Destination Size:  Line          */
#define MCF5307_DMA_DCR_START           (0x0001) /* Start Transfer                      */

/*
*DMA Status Register Definitions
*/


#define MCF5307_DMA_DSR_CE              (0x40)  /* Configuration Error          */
#define MCF5307_DMA_DSR_BES             (0x20)  /* Bus Error on Source          */
#define MCF5307_DMA_DSR_BED             (0x10)  /* Bus Error on Destination     */
#define MCF5307_DMA_DSR_REQ             (0x04)  /* Request                                      */
#define MCF5307_DMA_DSR_BSY             (0x02)  /* Busy                                         */
#define MCF5307_DMA_DSR_DONE            (0x01)  /* Transaction Done                     */          


#define DMA_STATE_BUSY			1
#define DMA_STATE_IDLE			0
#define DMA_STATE_CONFIGURATION_ERROR	-1
#define DMA_STATE_SOURCE_ERROR		-2
#define DMA_STATE_DESTINATION_ERROR	-3

struct dma_mcf
{
	const char *devname;
	unsigned char* source;
	unsigned char* dest;
	unsigned short count;
	unsigned short creg;  /*16 bit controllregister use the defines above*/
	void (*handler) (int, void *, struct pt_regs *);
	unsigned int irq_vector;
	unsigned int irq_level;
	unsigned long irq_flags;
};

static __inline__ int dma_init(int channel)
{
	int err;
	if ((err = request_dma(channel, 0)) != 0) return err;
	
	*(unsigned char*) (MCF_MBAR + MCFSIM_MPARK) |= 0x80; /* DMA BUS
	MASTER with highest priority*/
	return err;
}



static __inline__ void dma_set(int channel, struct dma_mcf dma)
{
		
	
	*(unsigned int*) (MCF5307_DMA_SAR + (0x40 * channel)) = dma.source;
        *(unsigned int*) (MCF5307_DMA_DAR + (0x40 * channel)) = dma.dest;
	*(unsigned short*) (MCF5307_DMA_BCR + (0x40 * channel)) = dma.count;
	if ((dma.creg & MCF5307_DMA_DCR_INT) !=0)		/* DMA generates a Interrupt on complrtion*/
		{					/* Now configure the Interrupt handler*/
		*(unsigned short*) (MCF5307_DMA_IVR + (0x40 * channel)) = dma.irq_vector;
		if (request_irq(dma.irq_vector,dma.handler,dma.irq_flags,
		dma.devname, 0) !=0)
				printk("request irq fail\n");
		
		*(volatile unsigned char*)(MCF_MBAR + 0x52 + channel) = dma.irq_level | MCFSIM_ICR_AUTOVEC;    /* set MBUS IRQ-Level and autovector */
	        mcf_setimr(mcf_getimr() & ~(0x4000 << channel));
		printk("maskreg%X\n",mcf_getimr());
		}
	*(unsigned short*) (MCF5307_DMA_DCR + (0x40 * channel)) = dma.creg;
}


static __inline__ unsigned char dma_busy(unsigned char channel)
{
	if ((*(unsigned char*) (MCF5307_DMA_SR + (0x40 * channel)) & MCF5307_DMA_DSR_BSY)!=0) return(0x1);
	
	return 0;
}

static __inline__ void dma_done(unsigned char channel)
{


	*(unsigned char*) (MCF5307_DMA_SR + (0x40 * channel)) = MCF5307_DMA_DSR_DONE;
}


static __inline__ void dma_clear(unsigned char channel)
{				
	*(unsigned short*) (MCF5307_DMA_DCR + (0x40 * channel)) = 0x0000;
}


static __inline__ unsigned char dma_get_status(unsigned char dsr)
{
	if ((dsr & MCF5307_DMA_DSR_CE) != 0) return(DMA_STATE_CONFIGURATION_ERROR);
	if ((dsr & MCF5307_DMA_DSR_BES) !=0) return(DMA_STATE_SOURCE_ERROR);
	if ((dsr & MCF5307_DMA_DSR_BED) !=0) return(DMA_STATE_DESTINATION_ERROR);
	return 0;
}

extern int request_dma(unsigned int dmanr, const char * device_id);	/* reserve a DMA channel */
extern void free_dma(unsigned int dmanr);	/* release it again */
#endif /* _M5307_DMA_H */
