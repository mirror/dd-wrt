/* AE-3068 (aka. aki3068net) RTL8019AS Config */

#define NE2000_ADDR		CONFIG_NE_BASE
#define NE2000_IRQ              5
#define NE2000_IRQ_VECTOR	(12 + NE2000_IRQ)
#define	NE2000_BYTE		volatile unsigned short

#define IER                     0xfee015
#define ISR			0xfee016
#define IRQ_MASK		(1 << NE2000_IRQ)

#define WCRL                    0xfee023
#define MAR0A                   0xffff20
#define ETCR0A                  0xffff24
#define DTCR0A                  0xffff27
#define MAR0B                   0xffff28
#define DTCR0B                  0xffff2f

#define H8300_INIT_NE()                  \
do {                                     \
	wordlength = 1;                  \
        outb_p(0x48, ioaddr + EN0_DCFG); \
} while(0)
