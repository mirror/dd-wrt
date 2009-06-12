/* H8MAX RTL8019AS Config */

#define NE2000_ADDR		0x800600
#define NE2000_IRQ              4
#define NE2000_IRQ_VECTOR	(12 + NE2000_IRQ)
#define	NE2000_BYTE		volatile unsigned short

#define IER                     0xfee015
#define ISR			0xfee016
#define IRQ_MASK		(1 << NE2000_IRQ)
/* sorry quick hack */
#if defined(outb)
# undef outb
#endif
#define outb(d,a)               h8max_outb((d),(a) - NE2000_ADDR)
#if defined(inb)
# undef inb
#endif
#define inb(a)                  h8max_inb((a) - NE2000_ADDR)
#if defined(outb_p)
# undef outb_p
#endif
#define outb_p(d,a)             h8max_outb((d),(a) - NE2000_ADDR)
#if defined(inb_p)
# undef inb_p
#endif
#define inb_p(a)                h8max_inb((a) - NE2000_ADDR)
#if defined(outsw)
# undef outsw
#endif
#define outsw(a,p,l)            h8max_outsw((a) - NE2000_ADDR,(unsigned short *)p,l)
#if defined(insw)
# undef insw
#endif
#define insw(a,p,l)             h8max_insw((a) - NE2000_ADDR,(unsigned short *)p,l)

#define H8300_INIT_NE()                  \
do {                                     \
	wordlength = 2;                  \
	h8max_outb(0x49, ioaddr + EN0_DCFG); \
	SA_prom[14] = SA_prom[15] = 0x57;\
} while(0)

static inline void h8max_outb(unsigned char d,unsigned char a)
{
	*(unsigned short *)(NE2000_ADDR + (a << 1)) = d;
}

static inline unsigned char h8max_inb(unsigned char a)
{
	return *(unsigned char *)(NE2000_ADDR + (a << 1) +1);
}

static inline void h8max_outsw(unsigned char a,unsigned short *p,unsigned long l)
{
	unsigned short d;
	for (; l != 0; --l, p++) {
		d = (((*p) >> 8) & 0xff) | ((*p) << 8);
		*(unsigned short *)(NE2000_ADDR + (a << 1)) = d;
	}
}

static inline void h8max_insw(unsigned char a,unsigned short *p,unsigned long l)
{
	unsigned short d;
	for (; l != 0; --l, p++) {
		d = *(unsigned short *)(NE2000_ADDR + (a << 1));
		*p = (d << 8)|((d >> 8) & 0xff);
	}
}

