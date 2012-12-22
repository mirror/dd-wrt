#ifndef MT_VM_H
#define MT_VM_H

#define hypercall(name, nr, ...)		\
	asm(					\
		".global " #name ";"		\
		".align 2;"			\
		".type " #name ",@function;"	\
		#name ":;"			\
		"li 0, " #nr ";"		\
		"crset so;"			\
		"mtspr 1023, 0;"		\
		"bnslr;"			\
		"li 3, -22;"			\
		"blr"				\
        );					\
	asmlinkage extern int name(__VA_ARGS__);

/* NOTE: do not allow vdma_descr to span multiple pages, so align it */
struct vdma_descr {
	unsigned addr;
	unsigned size;
	unsigned next;
} __attribute__((aligned(16)));

#define DONE		0x80000000

unsigned get_virq_nr(unsigned hwirq);
int vm_yield(void);
int vm_running(void);

#define hc_yield() vm_yield()


#endif
