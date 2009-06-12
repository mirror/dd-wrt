#ifndef _H8300_IRQ_H_
#define _H8300_IRQ_H_

#if defined(__H8300H__)
#define NR_IRQS 64
#define EXT_IRQ0 12
#define EXT_IRQ1 13
#define EXT_IRQ2 14
#define EXT_IRQ3 15
#define EXT_IRQ4 16
#define EXT_IRQ5 17
#define EXT_IRQ6 18
#define EXT_IRQ7 19
#endif
#if defined(CONFIG_CPU_H8S)
#define NR_IRQS 128
#define EXT_IRQ0 16
#define EXT_IRQ1 17
#define EXT_IRQ2 18
#define EXT_IRQ3 19
#define EXT_IRQ4 20
#define EXT_IRQ5 21
#define EXT_IRQ6 22
#define EXT_IRQ7 23
#define EXT_IRQ8 24
#define EXT_IRQ9 25
#define EXT_IRQ10 26
#define EXT_IRQ11 27
#define EXT_IRQ12 28
#define EXT_IRQ13 29
#define EXT_IRQ14 30
#define EXT_IRQ15 31
#endif

/*
 * "Generic" interrupt sources
 */

static __inline__ int irq_cannonicalize(int irq)
{
	return irq;
}

extern void enable_irq(unsigned int);
extern void disable_irq(unsigned int);

extern int sys_request_irq(unsigned int, 
	void (*)(int, void *, struct pt_regs *), 
	unsigned long, const char *, void *);
extern void sys_free_irq(unsigned int, void *);
extern void request_irq_boot(unsigned int, 
	void (*)(int, void *, struct pt_regs *), 
	unsigned long, const char *, void *);

typedef struct irq_node {
	void		(*handler)(int, void *, struct pt_regs *);
	unsigned long	flags;
	void		*dev_id;
	const char	*devname;
	struct irq_node *next;
} irq_node_t;

/*
 * This structure has only 4 elements for speed reasons
 */
typedef struct irq_handler {
	void (*handler)(int, void *, struct pt_regs *);
	int         flags;
	int         count;
	void	    *dev_id;
	const char  *devname;
} irq_handler_t;

/* count of spurious interrupts */
extern volatile unsigned int num_spurious;

/*
 * Some drivers want these entry points
 */
#define enable_irq_nosync(x)	enable_irq(x)
#define disable_irq_nosync(x)	disable_irq(x)

#endif /* _H8300_IRQ_H_ */
