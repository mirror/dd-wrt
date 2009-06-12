#ifndef _HYPERSTONE_PROCESSOR_H_
#define _HYPERSTONE_PROCESSOR_H_

#include <linux/types.h>
#include <asm/ptrace.h>
#include <asm/bitops.h>

typedef struct
{
    unsigned char DownLoadMode;         // offset 0
    unsigned char BoardType;            //        1
    unsigned char BoardRevisionNumber;  //        2
    unsigned char SysDriverPriority;    //        3
    unsigned int  TPRValue;				//        4
    unsigned int  BCRValue;             //        8
    unsigned int  MCRValue;             //       12
    unsigned int  FCRValue;             //       16
    unsigned int  SDCRValue;            //       20
    unsigned int  SDMRValue;            //       24
    unsigned char CPUType;              //       28
/*    unsigned char MEMAllocation;        //       29 */
} shadow_regs_t;

#define TPR_reg		1
#define BCR_reg		2
#define MCR_reg		3
#define FCR_reg		4
#define SDCR_reg	5
#define SDMR_reg	6

/*
 * Default implementation that returns current
 * value of program counter
 */
#define current_text_addr() ({ __label__ _l; _l: &&_l;})

#define EXPAND_STACK 		1
#define DONT_EXPAND_STACK 	0

/* struct indicates if an exception happened
 * in kernel mode, but we were using either the
 * user register stack or the kernel register stack.
 * In that case we have to switch stack before calling
 * the appropriate exception handler.
 *
 * We should maintain this information in order to
 * gracefully return from exception to the exact point
 * the exception happened in kernel mode.
 */
struct stack_use {
	unsigned long excp_in_km_while_using_user_reg_stack;
	unsigned long excp_in_km_while_using_user_aggr_stack;
};

struct return_from_vfork {
	unsigned long ret_from_vfork;
	unsigned long ReturnPC;
	unsigned long ReturnSR;
};

struct thread_struct {
	unsigned long SR;
	unsigned long G2;
	unsigned long G3;
	unsigned long G4;
	unsigned long G5;
	unsigned long G6;
	unsigned long G7;
	unsigned long G8;
	unsigned long G9;
	//unsigned long G10; G10 is used by GDB
	unsigned long G11;
	unsigned long G12;
	unsigned long G13;
	unsigned long G14;
	unsigned long G15;
	unsigned long SP;
	unsigned long UB;
	struct stack_use stack_use;
	struct return_from_vfork vfork_ret_info;
 };

#define INIT_THREAD { \
0,0,0,0, \
0,0,0,0, \
0,0,0,0, \
0,0,0,0, \
0,\
/*stack_use*/0,0,\
/*vfork_ret_info*/0,0,0\
}

/* Allocation and freeing of basic task resources. */
#define alloc_task_struct() \
        ((struct task_struct *) __get_free_pages(GFP_KERNEL,1))
#define free_task_struct(p)     free_pages((unsigned long)(p),1)
#define get_task_struct(tsk)    atomic_inc(&mem_map[MAP_NR(tsk)].count)

#define init_task       (init_task_union.task)
#define init_stack      (init_task_union.stack)

#define THREAD_SIZE (2*PAGE_SIZE)

/*
 * User space process size: 3.75GB. This is hardcoded into a few places,
 * so don't change it unless you know what you are doing.
 */
/* FIXME */
#define TASK_SIZE       (0xFFFFFFFFUL)

extern long arch_kernel_thread(int (*fn)(void *), void * arg, unsigned long flags);

#define copy_segments(tsk, mm)          do { } while (0)
#define release_segments(mm)            do { } while (0)
#define forget_segments()               do { } while (0)

/* Forward declaration, a strange C thing - Don't remove it!*/
struct task_struct;

/* Free all resources held by a thread. */
static inline void release_thread(struct task_struct *dead_task) { }

extern shadow_regs_t *system_sh_regs; 

#ifdef CONFIG_RAMKERNEL 
static inline void update_sh_reg_bit(unsigned char reg, unsigned short position, unsigned char value)
{
volatile long *base_addr;

	asm volatile("mov %0, G10"
				:"=l" (base_addr)
				:);

		switch (reg){
				case FCR_reg:
					if(value) set_bit(position, &(system_sh_regs->FCRValue)); 
					else clear_bit (position, &(system_sh_regs->FCRValue));
					*(base_addr+FCR_reg) = system_sh_regs->FCRValue;
					asm volatile ("
									fetch 2
									ori SR, 0x20
									mov FCR, %0"
									:
									:"l" (system_sh_regs->FCRValue));
					break;
				case TPR_reg:
					if(value) set_bit(position, &(system_sh_regs->TPRValue)); 
					else clear_bit (position, &(system_sh_regs->TPRValue));
					*(base_addr+TPR_reg) = system_sh_regs->TPRValue;
					asm volatile ("
									fetch 2
									ori SR, 0x20
									mov TPR, %0"
									:
									:"l" (system_sh_regs->TPRValue));
					break;
				case BCR_reg:
					if(value) set_bit(position, &(system_sh_regs->BCRValue)); 
					else clear_bit (position, &(system_sh_regs->BCRValue));
					*(base_addr+BCR_reg) = system_sh_regs->BCRValue;
					asm volatile ("
									fetch 2
									ori SR, 0x20
									mov BCR, %0"
									:
									:"l" (system_sh_regs->BCRValue));
					break;
				case MCR_reg:
					if(value) set_bit(position, &(system_sh_regs->MCRValue)); 
					else clear_bit (position, &(system_sh_regs->MCRValue));
					*(base_addr+MCR_reg) = system_sh_regs->MCRValue;
					asm volatile ("
									fetch 2
									ori SR, 0x20
									mov MCR, %0"
									:
									:"l" (system_sh_regs->MCRValue));
					break;
				default:
					asm volatile ("trap 16"::); /* FIXME */
				}
}

static inline void update_sh_reg(unsigned char reg, unsigned long value)
{
volatile long *base_addr;

	asm volatile("mov %0, G10"
				:"=l" (base_addr)
				:);

		switch (reg){
				case FCR_reg:
					system_sh_regs->FCRValue = value; 
					*(base_addr+FCR_reg) = system_sh_regs->FCRValue;
					asm volatile ("
									fetch 2
									ori SR, 0x20
									mov FCR, %0"
									:
									:"l" (system_sh_regs->FCRValue));
					break;
				case TPR_reg:
					system_sh_regs->TPRValue = value; 
					*(base_addr+TPR_reg) = system_sh_regs->TPRValue;
					asm volatile ("
									fetch 2
									ori SR, 0x20
									mov TPR, %0"
									:
									:"l" (system_sh_regs->TPRValue));
					break;
				case BCR_reg:
					system_sh_regs->BCRValue = value; 
					*(base_addr+BCR_reg) = system_sh_regs->BCRValue;
					asm volatile ("
									fetch 2
									ori SR, 0x20
									mov BCR, %0"
									:
									:"l" (system_sh_regs->BCRValue));
					break;
				case MCR_reg:
					system_sh_regs->MCRValue = value; 
					*(base_addr+MCR_reg) = system_sh_regs->MCRValue;
					asm volatile ("
									fetch 2
									ori SR, 0x20
									mov MCR, %0"
									:
									:"l" (system_sh_regs->MCRValue));
					break;
				default:
					asm volatile ("trap 16"::); /* FIXME */	

		}
}

#ifdef __KERNEL__
extern void __udelay(unsigned long usecs);
static inline void toggle_red_led(unsigned int usec)
{
		unsigned long tmp; 

		if (test_bit(2, &(system_sh_regs->FCRValue))){ 
				update_sh_reg_bit(FCR_reg, 2, 0); 
		}
				tmp=system_sh_regs->FCRValue;
		if(!usec){
				test_and_change_bit(1, &tmp);
				update_sh_reg(FCR_reg, tmp); 
		}
		else{
			while(1){
				__udelay(usec);
				test_and_change_bit(1, &tmp);
				asm volatile ("
								fetch 2
								ori SR, 0x20
								mov FCR, %0"
								:
								:"l" (tmp));
			};
		}

}

static inline void toggle_green_led(unsigned int usec)
{
		unsigned long tmp;;

		if (test_bit(6, &(system_sh_regs->FCRValue))){ 
				update_sh_reg_bit(FCR_reg, 6, 0); 
		}
				tmp=system_sh_regs->FCRValue;
		if(!usec){
				test_and_change_bit(5, &tmp);
				update_sh_reg(FCR_reg, tmp); 
		}
		else{
			while(1){
				__udelay(usec);
				test_and_change_bit(5, &tmp);
				asm volatile ("
								fetch 2
								ori SR, 0x20
								mov FCR, %0"
								:
								:"l" (tmp));
			};
		}

}

static inline void toggle_red_and_green_led(unsigned int usec)
{
		unsigned long tmp;;

		if (test_bit(2, &(system_sh_regs->FCRValue))){ 
				update_sh_reg_bit(FCR_reg, 2, 0); 
		}
		if (test_bit(6, &(system_sh_regs->FCRValue))){ 
				update_sh_reg_bit(FCR_reg, 6, 0); 
		}

		tmp=system_sh_regs->FCRValue;
		if(!usec){
				test_and_change_bit(1, &tmp);
				test_and_change_bit(5, &tmp);
				update_sh_reg(FCR_reg, tmp); 
		}
		else{
			while(1){
				__udelay(usec);
				test_and_change_bit(1, &tmp);
				test_and_change_bit(5, &tmp);
				asm volatile ("
								fetch 2
								ori SR, 0x20
								mov FCR, %0"
								:
								:"l" (tmp));
			};
		}

}
#endif /* __KERNEL__ */
#else
#error "The write-only registers are not initialized"


#endif /* RAMKERNEL */

extern unsigned long get_wchan(struct task_struct *p);

#define KSTK_EIP(tsk)  (1)
#define KSTK_ESP(tsk)  (1)


#define init_task       (init_task_union.task)
#define init_stack      (init_task_union.stack)
 
#define cpu_relax()     do { } while (0)

unsigned long get_wchan(struct task_struct *p);
#define KSTK_EIP(tsk) (1)
#define KSTK_ESP(tsk) (1)


#endif /* _HYPERSTONE_PROCESSOR_H_ */
