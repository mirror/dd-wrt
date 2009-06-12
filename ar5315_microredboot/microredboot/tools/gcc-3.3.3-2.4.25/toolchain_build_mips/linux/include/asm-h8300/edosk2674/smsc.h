/* EDOSK-2674R SMSC Network Controler Target Depend impliments */

#define SMSC_BASE 0xf80000
#define SMSC_IRQ 16

/* sorry quick hack */
#if defined(outw)
# undef outw
#endif
#define outw(d,a) edosk2674_smsc_outw(d,(volatile unsigned short *)(a))
#if defined(inw)
# undef inw
#endif
#define inw(a) edosk2674_smsc_inw((volatile unsigned short *)(a))
#if defined(outsw)
# undef outsw
#endif
#define outsw(a,p,l) edosk2674_smsc_outsw((volatile unsigned short *)(a),p,l)
#if defined(insw)
# undef insw
#endif
#define insw(a,p,l) edosk2674_smsc_insw((volatile unsigned short *)(a),p,l)

static inline void edosk2674_smsc_outw(
	unsigned short d,
	volatile unsigned short *a
	)
{
	*a = (d >> 8) | (d << 8);
}

static inline unsigned short edosk2674_smsc_inw(
	volatile unsigned short *a
	)
{
	unsigned short d;
	d = *a;
	return (d >> 8) | (d << 8);
}

static inline void edosk2674_smsc_outsw(
	volatile unsigned short *a,
	unsigned short *p,
	unsigned long l
	)
{
	for (; l != 0; --l, p++)
		*a = *p;
}

static inline void edosk2674_smsc_insw(
	volatile unsigned short *a,
	unsigned short *p,
	unsigned long l
	)
{
	for (; l != 0; --l, p++)
		*p = *a;
}

