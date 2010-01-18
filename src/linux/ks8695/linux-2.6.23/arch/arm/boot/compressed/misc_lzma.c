/*
 * misc.c
 * 
 * This is a collection of several routines from gzip-1.0.3 
 * adapted for Linux.
 *
 * malloc by Hannu Savolainen 1993 and Matthias Urlichs 1994
 *
 * Modified for ARM Linux by Russell King
 *
 * Nicolas Pitre <nico@visuaide.com>  1999/04/14 :
 *  For this code to run directly from Flash, all constant variables must
 *  be marked with 'const' and all other variables initialized at run-time 
 *  only.  This way all non constant variables will end up in the bss segment,
 *  which should point to addresses in RAM and cleared to 0 on start.
 *  This allows for a much quicker boot time.
 */

unsigned int __machine_arch_type;


#ifdef STANDALONE_DEBUG
#define putstr printf
#else



#include <linux/compiler.h>
#include <asm/arch/uncompress.h>


#include "printf.h"
#include "print.h"

static void myoutput(void *arg, char *s, int l)
{
    int i;

    // special termination call
    if ((l==1) && (s[0] == '\0')) return;
    
    for (i=0; i< l; i++) {
	putc(s[i]);
	if (s[i] == '\n') putc('\r');
    }
}

void printf(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    lp_Print(myoutput, 0, fmt, ap);
    va_end(ap);
}


#endif

#define __ptr_t void *
#ifdef CONFIG_SERIAL_IT8712
unsigned int it8712_uart_base;
#define UART_RX         0
#define UART_TX         0
#define UART_DLL        0
#define UART_TRG        0
#define UART_DLM        1
#define UART_IER        1
#define UART_FCTR       1
#define UART_IIR        2
#define UART_FCR        2
#define UART_EFR        2
#define UART_LCR        3
#define UART_MCR        4
#define UART_LSR        5
#define UART_MSR        6
#define UART_SCR        7
#define UART_EMSR       7
void LPCEnterMBPnP(void);
void LPCExitMBPnP(void);
int SearchIT8712(void);
int InitLPCInterface(void);
#endif



/*
 * gzip delarations
 */
#define OF(args)  args
#define STATIC static

typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;

#define WSIZE 0x8000		/* Window size must be at least 32k, */
				/* and a power of two */

static uch *inbuf;		/* input buffer */
static uch window[WSIZE];	/* Sliding window buffer */

static unsigned insize=0;		/* valid bytes in inbuf */
static unsigned inptr=0;		/* index of next byte to be processed in inbuf */
static unsigned outcnt;		/* bytes in output buffer */

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define CONTINUATION 0x02 /* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define ENCRYPTED    0x20 /* bit 5 set: file is encrypted */
#define RESERVED     0xC0 /* bit 6,7:   reserved */

#define get_byte()  (inptr < insize ? inbuf[inptr++] : fill_inbuf())

/* Diagnostic functions */
#ifdef DEBUG
#  define Assert(cond,msg) {if(!(cond)) error(msg);}
#  define Trace(x) fprintf x
#  define Tracev(x) {if (verbose) fprintf x ;}
#  define Tracevv(x) {if (verbose>1) fprintf x ;}
#  define Tracec(c,x) {if (verbose && (c)) fprintf x ;}
#  define Tracecv(c,x) {if (verbose>1 && (c)) fprintf x ;}
#else
#  define Assert(cond,msg)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

static int  fill_inbuf(void);
static void flush_window(void);
static void error(char *m);

extern char input_data[];
extern char input_data_end[];

static ulg output_ptr=0;
static uch *output_data;
static ulg bytes_out;


extern int end;
static ulg free_mem_ptr;
static ulg free_mem_ptr_end;

#define _LZMA_IN_CB

#include "LzmaDecode.h"
static __inline__ int read_byte(unsigned char **buffer, UInt32 *bufferSize);
#include "LzmaDecode.c"


/*
 * Do the lzma decompression
 */
static int lzma_unzip(void)
{

	unsigned int i;
	unsigned int uncompressedSize = 0;
        unsigned char *workspace;
        unsigned int lc,lp,pb;

	// lzma args
	i = get_byte();
	lc = i % 9, i = i / 9;
	lp = i % 5, pb = i / 5;
        
        // skip dictionary size
        for (i = 0; i < 4; i++) 
        	get_byte();
        // get uncompressed size
	uncompressedSize = (get_byte()) +
		(get_byte() << 8) +
		(get_byte() << 16) +
		(get_byte() << 24);
        workspace = output_data + uncompressedSize;
        // skip high order bytes
        for (i = 0; i < 4; i++) 
        	get_byte();
	// decompress kernel
//	if (LzmaDecode(workspace, ~0, lc, lp, pb,
//		(unsigned char*)LOADADDR, osize, &i) == LZMA_RESULT_OK)
	if (LzmaDecode(workspace, ~0, lc,lp,pb,(unsigned char*)output_data, uncompressedSize, &i) == LZMA_RESULT_OK)
	{
		if ( i != uncompressedSize )
		   error( "kernel corrupted!\n");
		//copy it back to low_buffer
		bytes_out = i;
		output_ptr = i;
		return 0;
	}
	return 1;
}


static unsigned int icnt = 0;
static __inline__ int read_byte(unsigned char **buffer, UInt32 *bufferSize)
{
	static unsigned char val;
	*bufferSize = 1;
	val = get_byte();
	*buffer = &val;
        if ( icnt++ % ( 1024 * 10 ) == 0 )
               printf(".");
	return LZMA_RESULT_OK;
}	




/* ===========================================================================
 * Fill the input buffer. This is called only when the buffer is empty
 * and at least one byte is really needed.
 */
 

int fill_inbuf(void)
{
	if (insize != 0)
		error("ran out of input data");

	inbuf = input_data;
	insize = &input_data_end[0] - &input_data[0];

	inptr = 1;
	return inbuf[0];
}

/* ===========================================================================
 * Write the output window window[0..outcnt-1] and update crc and bytes_out.
 * (Used for the decompressed data only.)
 */

#ifndef arch_error
#define arch_error(x)
#endif

static void error(char *x)
{
	arch_error(x);

	printf("\n\n");
	printf(x);
	printf("\n\n -- System halted");

	while(1);	/* Halt */
}

void __div0(void)
{
	error("division by zero");
}

#ifndef STANDALONE_DEBUG

ulg
decompress_kernel(ulg output_start, ulg free_mem_ptr_p, ulg free_mem_ptr_end_p,
		  int arch_id)
{
#ifdef CONFIG_SERIAL_IT8712
        unsigned char *addr;
#endif
	output_data		= (uch *)output_start;	/* Points to kernel start */
	free_mem_ptr		= free_mem_ptr_p;
	free_mem_ptr_end	= free_mem_ptr_end_p;
	__machine_arch_type	= arch_id;

	arch_decomp_setup();
#ifdef CONFIG_SERIAL_IT8712

        InitLPCInterface();
        LPCSetConfig(0, 0x02, 0x01);
        LPCSetConfig(LDN_SERIAL1, 0x30, 0x1);
        LPCSetConfig(LDN_SERIAL1, 0x23, 0x0);
        it8712_uart_base = IT8712_IO_BASE;
        it8712_uart_base += ((LPCGetConfig(LDN_SERIAL1, 0x60) << 8) + LPCGetConfig(LDN_SERIAL1, 0x61));

        do {
        addr = (unsigned char *)(it8712_uart_base + UART_LCR) ;
        *addr = 0x80;
        // Set Baud Rate
        addr = (unsigned char *)(it8712_uart_base+UART_DLL);
        *addr = 0x06 ;
        addr = (unsigned char *)(it8712_uart_base+UART_DLM);
        *addr = 0x00 ;

        addr = (unsigned char *)(it8712_uart_base+UART_LCR);    // LCR
        *addr = 0x07 ;
        addr = (unsigned char *)(it8712_uart_base+UART_MCR);    // MCR
        *addr = 0x08 ;
        addr = (unsigned char *)(it8712_uart_base+UART_FCR);    // FCR
        *addr = 0x01 ;
    	} while(0);
#endif

	
	printf("DD-WRT LZMA Loader v1.0\nArch ID is %d\n",arch_id);
	printf("Uncompressing Linux");
	lzma_unzip();
	printf("\ndone, booting the kernel.\n");
	return output_ptr;
}
#else

char output_buffer[1500*1024];

int main()
{
	output_data = output_buffer;
	printf("Uncompressing Linux...");
	lzma_unzip();
	printf("done.\n");
	return 0;
}
#endif
	
#ifdef CONFIG_SERIAL_IT8712

#define LPC_KEY_ADDR    (unsigned char *)(SL2312_LPC_IO_BASE + 0x2e)
#define LPC_DATA_ADDR   (unsigned char *)(SL2312_LPC_IO_BASE + 0x2f)
#define LPC_BUS_CTRL                    *( unsigned char*) (SL2312_LPC_HOST_BASE + 0)
#define LPC_BUS_STATUS                  *( unsigned char*) (SL2312_LPC_HOST_BASE + 2)
#define LPC_SERIAL_IRQ_CTRL             *( unsigned char*) (SL2312_LPC_HOST_BASE + 4)

char LPCGetConfig(char LdnNumber, char Index)
{
        char rtn;
        unsigned char *addr ;

        LPCEnterMBPnP();                                // Enter IT8712 MB PnP mode

        addr = LPC_KEY_ADDR;
        *addr = 0x07 ;

        addr = LPC_DATA_ADDR;
        *addr = LdnNumber ;

        addr = LPC_KEY_ADDR;
        *addr = Index ;

        addr = LPC_DATA_ADDR ;
        rtn = *addr ;

        LPCExitMBPnP();
        return rtn;

}

void LPCSetConfig(char LdnNumber, char Index, char data)
{
        unsigned char *addr;
        LPCEnterMBPnP();                                // Enter IT8712 MB PnP mode
        addr = LPC_KEY_ADDR;
        *addr = 0x07;
        addr = LPC_DATA_ADDR;
        *addr = LdnNumber;
        addr = LPC_KEY_ADDR;
        *addr = Index;
        addr = LPC_DATA_ADDR;
        *addr = data;

        LPCExitMBPnP();
}

//unsigned char key[4] ;
void LPCEnterMBPnP(void)
{
        unsigned char *addr;
        addr = LPC_KEY_ADDR;
        unsigned char key[4] = {0x87, 0x01, 0x55, 0x55};

 		do {
 		*addr = key[0];
 		*addr = key[1];
 		*addr = key[2];
 		*addr = key[3];
 		}while(0);
}

void LPCExitMBPnP(void)
{
        unsigned char *addr;
        addr = LPC_KEY_ADDR;
        *addr = 0x02 ;

        addr = LPC_DATA_ADDR;
        *addr = 0x02 ;
}

int InitLPCInterface(void)
{
        int i;
        LPC_BUS_CTRL = 0xc0;
        LPC_SERIAL_IRQ_CTRL = 0xc0;

        for(i=0;i<0x2000;i++) ;

        LPC_SERIAL_IRQ_CTRL = 0x80;
        if (!SearchIT8712()) ;
//			while(1);
        return 0;
}

int SearchIT8712(void)
{
        unsigned char Id1, Id2;
        unsigned short Id;
        unsigned char *addr;

        LPCEnterMBPnP();
        addr = LPC_KEY_ADDR;
        *addr = 0x20 ;
        addr = LPC_DATA_ADDR;
        Id1 = *addr ;

        addr = LPC_KEY_ADDR;
        *addr = 0x21 ;
        addr = LPC_DATA_ADDR;
        Id2 = *addr ;

        Id = (Id1 << 8) | Id2;
        LPCExitMBPnP();

        if (Id == 0x8712)
                return 1;
        else
                return 0;
}

#endif
