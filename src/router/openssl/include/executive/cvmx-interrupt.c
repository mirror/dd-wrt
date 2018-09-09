/***********************license start***************
 * Copyright (c) 2003-2010  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

/**
 * @file
 *
 * Interface to the Mips interrupts.
 *
 * <hr>$Revision: 88301 $<hr>
 */
#ifndef __U_BOOT__
#if __GNUC__ >= 4
/* Backtrace is only available with the new toolchain.  */
#include <execinfo.h>
#endif
#endif /* __U_BOOT__ */
#include "cvmx.h"
#include "cvmx-interrupt.h"
#include "cvmx-sysinfo.h"
#include "cvmx-uart.h"
#include "cvmx-pow.h"
#include "cvmx-ebt3000.h"
#include "cvmx-coremask.h"
#include "cvmx-spinlock.h"
#include "cvmx-atomic.h"
#include "cvmx-app-init.h"
#include "cvmx-error.h"
#include "cvmx-profiler.h"
#ifndef __U_BOOT__
#include <octeon_mem_map.h>
#include "cvmx-coredump.h"
#else
#include <asm/arch/octeon_mem_map.h>
#endif
EXTERN_ASM void cvmx_interrupt_stage1(void);
EXTERN_ASM void cvmx_debug_handler_stage1(void);
EXTERN_ASM void cvmx_interrupt_cache_error(void);
EXTERN_ASM void cvmx_interrupt_cache_error_v3(void);

int cvmx_interrupt_in_isr = 0;

/**
 * Internal status the interrupt registration
 */
typedef struct {
	cvmx_interrupt_exception_t exception_handler;
} cvmx_interrupt_state_t;

/**
 * Internal state the interrupt registration
 */
#ifndef __U_BOOT__
static CVMX_SHARED cvmx_interrupt_state_t cvmx_interrupt_state;
static CVMX_SHARED cvmx_spinlock_t cvmx_interrupt_default_lock;
/* Incremented once first core processing is finished. */
static CVMX_SHARED int32_t cvmx_interrupt_initialize_flag;
#endif /* __U_BOOT__ */

#define HI32(data64)    ((uint32_t)(data64 >> 32))
#define LO32(data64)    ((uint32_t)(data64 & 0xFFFFFFFF))

static const char reg_names[][32] = { "r0", "at", "v0", "v1", "a0", "a1", "a2", "a3",
	"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
	"t8", "t9", "k0", "k1", "gp", "sp", "s8", "ra"
};

/**
 * version of printf that works better in exception context.
 *
 * @param format
 */
void cvmx_safe_printf(const char *format, ...)
{
	char buffer[256];
	char *ptr = buffer;
	int count;
	va_list args;

	va_start(args, format);
	count = vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	while (count-- > 0) {
		cvmx_uart_lsr_t lsrval;

		/* Spin until there is room */
		do {
			lsrval.u64 = cvmx_read_csr(CVMX_MIO_UARTX_LSR(0));
#if !defined(CONFIG_OCTEON_SIM_SPEED)
			if (lsrval.s.temt == 0)
				cvmx_wait(10000);	/* Just to reduce the load on the system */
#endif
		}
		while (lsrval.s.temt == 0);

		if (*ptr == '\n')
			cvmx_write_csr(CVMX_MIO_UARTX_THR(0), '\r');
		cvmx_write_csr(CVMX_MIO_UARTX_THR(0), *ptr++);
	}
}

/* Textual descriptions of cause codes */
static const char cause_names[][128] = {
	/*  0 */ "Interrupt",
	/*  1 */ "TLB modification",
	/*  2 */ "tlb load/fetch",
	/*  3 */ "tlb store",
	/*  4 */ "address exc, load/fetch",
	/*  5 */ "address exc, store",
	/*  6 */ "bus error, instruction fetch",
	/*  7 */ "bus error, load/store",
	/*  8 */ "syscall",
	/*  9 */ "breakpoint",
	/* 10 */ "reserved instruction",
	/* 11 */ "cop unusable",
	/* 12 */ "arithmetic overflow",
	/* 13 */ "trap",
	/* 14 */ "",
	/* 15 */ "floating point exc",
	/* 16 */ "",
	/* 17 */ "",
	/* 18 */ "cop2 exception",
	/* 19 */ "",
	/* 20 */ "",
	/* 21 */ "",
	/* 22 */ "mdmx unusable",
	/* 23 */ "watch",
	/* 24 */ "machine check",
	/* 25 */ "",
	/* 26 */ "",
	/* 27 */ "",
	/* 28 */ "",
	/* 29 */ "",
	/* 30 */ "cache error",
	/* 31 */ ""
};

/**
 * @INTERNAL
 * print_reg64
 * @param name   Name of the value to print
 * @param reg    Value to print
 */
static inline void print_reg64(const char *name, uint64_t reg)
{
	cvmx_safe_printf("%16s: 0x%08x%08x\n", name, (unsigned int)HI32(reg), (unsigned int)LO32(reg));
}

/**
 * @INTERNAL
 * Dump all useful registers to the console
 *
 * @param registers CPU register to dump
 */
void __cvmx_interrupt_dump_registers(uint64_t * registers)
{
	uint64_t r1, r2;
	int reg;
	for (reg = 0; reg < 16; reg++) {
		r1 = registers[reg];
		r2 = registers[reg + 16];
		cvmx_safe_printf("%3s ($%02d): 0x%08x%08x \t %3s ($%02d): 0x%08x%08x\n",
				 reg_names[reg], reg, (unsigned int)HI32(r1), (unsigned int)LO32(r1),
				 reg_names[reg + 16], reg + 16, (unsigned int)HI32(r2), (unsigned int)LO32(r2));
	}
	CVMX_MF_COP0(r2, COP0_STATUS);
	if ((r2 >> 29) & 1) {
		typedef union {
			uint64_t val;
			float flt;
			double dbl;
		} fpu_reg_t;
		fpu_reg_t r1;
		cvmx_safe_printf(" Floating point registers:\n");
		for (reg = 36; reg < 68; reg++) {
			r1.val = registers[reg];
			cvmx_safe_printf(" f%02d:0x%016llx\tflt:%-17.9g\tdbl:%-24.17g\n",
				 reg-36, (unsigned long long)r1.val, r1.flt, r1.dbl);
		}
		/* Print fir and fscr registers also */
		r1.val = registers[68];
		cvmx_safe_printf("%16s: 0x%08x\n", "FIR", (unsigned int)LO32(r1.val));
		r1.val = registers[69];
		cvmx_safe_printf("%16s: 0x%08x\n", "FCSR", (unsigned int)LO32(r1.val));
	}
	CVMX_MF_COP0(r1, COP0_CAUSE);
	print_reg64("COP0_CAUSE", r1);
	print_reg64("COP0_STATUS", r2);
	CVMX_MF_COP0(r1, COP0_BADVADDR);
	print_reg64("COP0_BADVADDR", r1);
	CVMX_MF_COP0(r2, COP0_EPC);
	print_reg64("COP0_EPC", r2);
}

/**
 * @INTERNAL
 * Default exception handler. Prints out the exception
 * cause decode and all relevant registers.
 *
 * @param registers Registers at time of the exception
 */
#ifndef __U_BOOT__
static
#endif				/* __U_BOOT__ */
void __cvmx_interrupt_default_exception_handler(uint64_t * registers)
{
	uint64_t trap_print_cause;
	const char *str;
#ifndef __U_BOOT__
	int modified_zero_pc = 0;

	ebt3000_str_write("Trap");
	cvmx_spinlock_lock(&cvmx_interrupt_default_lock);
        /* Start Dumping */
        cvmx_coredump_start_dump(registers);
#endif
	CVMX_MF_COP0(trap_print_cause, COP0_CAUSE);
	str = cause_names[(trap_print_cause >> 2) & 0x1f];
	cvmx_safe_printf("Core %d: Unhandled Exception. Cause register decodes to:\n%s\n", (int)cvmx_get_core_num(), str && *str ? str : "Reserved exception cause");
	cvmx_safe_printf("******************************************************************\n");
	__cvmx_interrupt_dump_registers(registers);

#ifndef __U_BOOT__

	cvmx_safe_printf("******************************************************************\n");
#if __GNUC__ >= 4 && !defined(OCTEON_DISABLE_BACKTRACE)
	cvmx_safe_printf("Backtrace:\n\n");
	if (registers[35] == 0) {
		modified_zero_pc = 1;
		/* If PC is zero we probably did jalr $zero, in which case $31 - 8 is the call site. */
		registers[35] = registers[31] - 8;
	}
	__octeon_print_backtrace_func((__octeon_backtrace_printf_t) cvmx_safe_printf);
	if (modified_zero_pc)
		registers[35] = 0;
	cvmx_safe_printf("******************************************************************\n");
#endif

	cvmx_spinlock_unlock(&cvmx_interrupt_default_lock);

	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM)
		CVMX_BREAK;

	while (1) {
		/* Interrupts are suppressed when we are in the exception
		   handler (because of SR[EXL]).  Spin and poll the uart
		   status and see if the debugger is trying to stop us. */
		cvmx_uart_lsr_t lsrval;
		lsrval.u64 = cvmx_read_csr(CVMX_MIO_UARTX_LSR(cvmx_debug_uart));
		if (lsrval.s.dr) {
			uint64_t tmp;
			/* Pulse the MCD0 signal. */
			asm volatile (".set push\n" ".set noreorder\n" ".set mips64\n" "dmfc0 %0, $22\n" "ori   %0, %0, 0x10\n" "dmtc0 %0, $22\n" ".set pop\n":"=r" (tmp));
		}
	}
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__

static CVMX_SHARED struct cvmx_interrupt *cvmx_ipx_irq[8];

/**
 * Map a ciu bit to an irq number.  0xff for invalid.
 * 0-63 for en0.
 * 64-127 for en1.
 */

union cvmx_ciux_to_irq {
	struct {
		struct cvmx_interrupt *en0_to_irq[64];
		struct cvmx_interrupt *en1_to_irq[64];
		struct cvmx_interrupt *en2_to_irq[64];
	};
	struct {
		struct cvmx_interrupt *ciu2_to_irq[8][64];
	};
	struct {
		struct cvmx_interrupt *ciu2_high_bits_to_irq[256];
	};
};

static CVMX_SHARED union cvmx_ciux_to_irq cvmx_ciux_to_irq;

#define cvmx_ciu_to_irq cvmx_ciux_to_irq.ciu2_to_irq
#define cvmx_ciu_en0_to_irq cvmx_ciux_to_irq.en0_to_irq
#define cvmx_ciu_en1_to_irq cvmx_ciux_to_irq.en1_to_irq
#define cvmx_ciu_61xx_timer_to_irq cvmx_ciux_to_irq.en2_to_irq
#define cvmx_ciu2_wrkq_to_irq cvmx_ciu_to_irq[0]
#define cvmx_ciu2_wdog_to_irq cvmx_ciu_to_irq[1]
#define cvmx_ciu2_rml_to_irq cvmx_ciu_to_irq[2]
#define cvmx_ciu2_mio_to_irq cvmx_ciu_to_irq[3]
#define cvmx_ciu2_io_to_irq cvmx_ciu_to_irq[4]
#define cvmx_ciu2_mem_to_irq cvmx_ciu_to_irq[5]
#define cvmx_ciu2_eth_to_irq cvmx_ciu_to_irq[6]
#define cvmx_ciu2_gpio_to_irq cvmx_ciu_to_irq[7]

static CVMX_SHARED struct cvmx_interrupt *cvmx_ciu2_mbox_to_irq[64];

CVMX_SHARED void (*cvmx_interrupt_register)(unsigned int hw_irq_number, struct cvmx_interrupt *info);
CVMX_SHARED unsigned int (*cvmx_interrupt_map)(cvmx_irq_t e);

struct cvmx_interrupt_cpu {
	struct cvmx_interrupt irq;
	int ip;
};

static uint64_t cvmx_interrupt_ciu_en0_mirror;
static uint64_t cvmx_interrupt_ciu_en1_mirror;
static uint64_t cvmx_interrupt_ciu_61xx_timer_mirror;

/**
 * @INTERNAL
 * Called for all Performance Counter interrupts. Handler for
 * interrupt line 6
 *
 * @param irq_number Interrupt number that we're being called for
 * @param registers  Registers at the time of the interrupt
 * @param user_arg   Unused user argument*
 */
static void __cvmx_interrupt_perf_handler(struct cvmx_interrupt *irq, uint64_t * registers)
{
	uint64_t perf_counter;
	CVMX_MF_COP0(perf_counter, COP0_PERFVALUE0);
	if (perf_counter & (1ull << 63))
		cvmx_collect_sample();
}

static CVMX_SHARED struct cvmx_interrupt __cvmx_interrupt_perf = {
	.handler = __cvmx_interrupt_perf_handler,
};

/**
 * @INTERNAL
 * Handler for interrupt lines 2 and 3. These are directly tied
 * to the CIU. The handler queries the status of the CIU and
 * calls the secondary handler for the CIU interrupt that
 * occurred.
 *
 * @param top_irq Interrupt that fired (ip2 or ip3)
 * @param registers  Registers at the time of the interrupt
 */
static void __cvmx_interrupt_ciu(struct cvmx_interrupt *top_irq, uint64_t * registers)
{
	int ciu_offset;
	uint64_t irq_mask;
	int bit;
	int core = cvmx_get_core_num();
	struct cvmx_interrupt_cpu *irqcpu = CVMX_CONTAINTER_OF(top_irq, struct cvmx_interrupt_cpu, irq);

	if (irqcpu->ip == 2) {
		/* Handle EN0 sources */
		ciu_offset = core * 2;
		irq_mask = cvmx_read_csr(CVMX_CIU_INTX_SUM0(ciu_offset)) & cvmx_interrupt_ciu_en0_mirror;
		CVMX_DCLZ(bit, irq_mask);
		bit = 63 - bit;
		if (bit == 51
		    && (octeon_has_feature(OCTEON_FEATURE_MULTICAST_TIMER))) {
			uint64_t irq_mask;
			int bit;
			irq_mask = cvmx_read_csr(CVMX_CIU_SUM2_PPX_IP2(core)) & cvmx_interrupt_ciu_61xx_timer_mirror;
			CVMX_DCLZ(bit, irq_mask);
			bit = 63 - bit;
			/* Handle TIMER(4..9) and Endor Interrupts */
			if ((bit <= 9 && bit >= 4) || (bit <= 14 && bit >= 12)) {
				struct cvmx_interrupt *irq = cvmx_ciu_61xx_timer_to_irq[bit];
				if (cvmx_unlikely(!irq)) {
					/* No mapping */
					cvmx_safe_printf("__cvmx_interrupt_ciu: Received unhandled interrupt 0:%d\n", bit);
					__cvmx_interrupt_dump_registers(registers);
					cvmx_interrupt_ciu_61xx_timer_mirror &= ~(1ull << bit);
					cvmx_write_csr(CVMX_CIU_EN2_PPX_IP2(core), cvmx_interrupt_ciu_61xx_timer_mirror);
					return;
				}
				irq->handler(irq, registers);
				return;
			}
		}

		if (bit >= 0) {
			struct cvmx_interrupt *irq = cvmx_ciu_en0_to_irq[bit];
			if (cvmx_unlikely(!irq)) {
				/* No mapping. */
				cvmx_safe_printf("__cvmx_interrupt_ciu: Received unhandled interrupt 0:%d\n", bit);
				__cvmx_interrupt_dump_registers(registers);
				cvmx_interrupt_ciu_en0_mirror &= ~(1ull << bit);
				cvmx_write_csr(CVMX_CIU_INTX_EN0(ciu_offset), cvmx_interrupt_ciu_en0_mirror);
				return;
			}
			irq->handler(irq, registers);
			return;
		}
	} else {
		/* Handle EN1 sources */
		ciu_offset = cvmx_get_core_num() * 2 + 1;
		irq_mask = cvmx_read_csr(CVMX_CIU_INT_SUM1) & cvmx_interrupt_ciu_en1_mirror;
		CVMX_DCLZ(bit, irq_mask);
		bit = 63 - bit;
		if (bit >= 0) {
			struct cvmx_interrupt *irq = cvmx_ciu_en1_to_irq[bit];
			if (cvmx_unlikely(!irq)) {
				/* No mapping. */
				cvmx_safe_printf("__cvmx_interrupt_ciu: Received unhandled interrupt 1:%d\n", bit);
				__cvmx_interrupt_dump_registers(registers);
				cvmx_interrupt_ciu_en1_mirror &= ~(1ull << bit);
				cvmx_write_csr(CVMX_CIU_INTX_EN1(ciu_offset), cvmx_interrupt_ciu_en1_mirror);
				return;
			}
			irq->handler(irq, registers);
			return;
		}
	}
}

/**
 * @INTERNAL
 * Handler for interrupt line 3, the DPI_DMA will have different value
 * per core, all other fields values are identical for different cores.
 *  These are directly tied to the CIU. The handler queries the status of
 * the CIU and calls the secondary handler for the CIU interrupt that
 * occurred.
 *
 * @param top_irq Interrupt number that fired (ip2 or ip3)
 * @param registers  Registers at the time of the interrupt
 */
static void __cvmx_interrupt_ciu_cn61xx(struct cvmx_interrupt *top_irq, uint64_t * registers)
{
	/* Handle EN1 sources */
	int core = cvmx_get_core_num();
	int ciu_offset;
	uint64_t irq_mask;
	int bit;

	ciu_offset = core * 2 + 1;
	irq_mask = cvmx_read_csr(CVMX_CIU_SUM1_PPX_IP3(core)) & cvmx_interrupt_ciu_en1_mirror;
	CVMX_DCLZ(bit, irq_mask);
	bit = 63 - bit;
	if (bit >= 0) {
		struct cvmx_interrupt *irq = cvmx_ciu_en1_to_irq[bit];
		if (cvmx_unlikely(!irq)) {
			/* No mapping. */
			cvmx_safe_printf("__cvmx_interrupt_ciu_cn61xx: Received unhandled interrupt 1:%d\n", bit);
			__cvmx_interrupt_dump_registers(registers);
			cvmx_interrupt_ciu_en1_mirror &= ~(1ull << bit);
			cvmx_write_csr(CVMX_CIU_INTX_EN1(ciu_offset), cvmx_interrupt_ciu_en1_mirror);
			return;
		}
		irq->handler(irq, registers);
		return;
	}
}

/**
 * @INTERNAL
 * Handler for interrupt line 2 on 68XX. These are directly tied
 * to the CIU2. The handler queries the status of the CIU and
 * calls the secondary handler for the CIU interrupt that
 * occurred.
 *
 * @param top_irq The CPU Interrupt that fired (ip2 or ip3)
 * @param registers  Registers at the time of the interrupt
 */
static void __cvmx_interrupt_ciu2(struct cvmx_interrupt *top_irq, uint64_t * registers)
{
	int sum_bit, src_bit;
	uint64_t src_reg, src_val;
	struct cvmx_interrupt *irq;
	int core = cvmx_get_core_num();
	uint64_t sum = cvmx_read_csr(CVMX_CIU2_SUM_PPX_IP2(core));

	CVMX_DCLZ(sum_bit, sum);
	sum_bit = 63 - sum_bit;

	if (sum_bit >= 0) {
		switch (sum_bit) {
		case 63:
		case 62:
		case 61:
		case 60:
			irq = cvmx_ciu2_mbox_to_irq[sum_bit - 60];
			if (cvmx_unlikely(!irq)) {
				/* No mapping. */
				cvmx_safe_printf("__cvmx_interrupt_ciu2: Received unhandled interrupt sum:%d\n", sum_bit);
				__cvmx_interrupt_dump_registers(registers);
				uint64_t mask_reg = CVMX_CIU2_EN_PPX_IP2_MBOX_W1C(core);
				cvmx_write_csr(mask_reg, 1ull << (sum_bit - 60));
				break;
			}
			irq->handler(irq, registers);
			break;

		case 7:
		case 6:
		case 5:
		case 4:
		case 3:
		case 2:
		case 1:
		case 0:
			src_reg = CVMX_CIU2_SRC_PPX_IP2_WRKQ(core) + (0x1000 * sum_bit);
			src_val = cvmx_read_csr(src_reg);
			if (!src_val)
				break;
			CVMX_DCLZ(src_bit, src_val);
			src_bit = 63 - src_bit;
			irq = cvmx_ciu_to_irq[sum_bit][src_bit];
			if (cvmx_unlikely(!irq)) {
				/* No mapping. */
				uint64_t mask_reg = CVMX_CIU2_EN_PPX_IP2_WRKQ_W1C(core) + (0x1000 * sum_bit);
				cvmx_write_csr(mask_reg, 1ull << src_bit);
				cvmx_safe_printf("__cvmx_interrupt_ciu2: Received unhandled interrupt %d:%d\n", sum_bit, src_bit);
				__cvmx_interrupt_dump_registers(registers);
				break;
			}
			irq->handler(irq, registers);
			break;

		default:
			cvmx_safe_printf("Unknown CIU2 bit: %d\n", sum_bit);
			break;
		}
	}
	/* Clear the source to reduce the chance for spurious interrupts.  */

	/* CN68XX has an CIU-15786 errata that accessing the ACK registers
	 * can stop interrupts from propagating
	 */

	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		cvmx_read_csr(CVMX_CIU2_INTR_CIU_READY);
	else
		cvmx_read_csr(CVMX_CIU2_ACK_PPX_IP2(core));
}

/**
 * @INTERNAL
 * Handler for interrupt line 2 on 78XX. These are directly tied
 * to the CIU3.
 *
 * @param top_irq The CPU Interrupt that fired (ip2 or ip3)
 * @param registers  Registers at the time of the interrupt
 */
static void __cvmx_interrupt_ciu3(struct cvmx_interrupt *top_irq, uint64_t * registers)
{
	cvmx_ciu3_destx_pp_int_t dest;
	int core = cvmx_get_core_num();

	dest.u64 = cvmx_read_csr(CVMX_CIU3_DESTX_PP_INT(core * 3));

	if (dest.s.intr) {
		int high = dest.s.intsn >> 12;
		struct cvmx_interrupt *irq = cvmx_ciux_to_irq.ciu2_high_bits_to_irq[high];
		while (irq && irq->data != dest.s.intsn)
			irq = irq->r;
		if (irq) {
			cvmx_ciu3_iscx_w1c_t w1c;
			w1c.u64 = 0;
			w1c.s.raw = 1;
			/* Ack the source */
			cvmx_write_csr(CVMX_CIU3_ISCX_W1C(dest.s.intsn), w1c.u64);
			cvmx_read_csr(CVMX_CIU3_ISCX_W1C(dest.s.intsn));
			irq->handler(irq, registers);
		} else {
			cvmx_ciu3_iscx_w1c_t w1c;
			w1c.u64 = 0;
			w1c.s.raw = 1;
			w1c.s.en = 1;
			/* No handler, Ack and disable the source */
			cvmx_write_csr(CVMX_CIU3_ISCX_W1C(dest.s.intsn), w1c.u64);
			cvmx_read_csr(CVMX_CIU3_ISCX_W1C(dest.s.intsn));
			cvmx_safe_printf("__cvmx_interrupt_ciu3: No handler for 0x%x\n",
					 (unsigned int)dest.s.intsn);
		}
		
	} else {
		cvmx_safe_printf("__cvmx_interrupt_ciu3: Received spurious interrupt\n");
	}
}

/**
 * @INTERNAL
 * Called for all RML interrupts. This is usually an ECC error
 *
 * @param irq The interrupt number that we're being called for
 * @param registers  Registers at the time of the interrupt
 */
static void __cvmx_interrupt_ecc(struct cvmx_interrupt *irq, uint64_t * registers)
{
	cvmx_error_poll();
}

#define REGISTER_AND_UNMASK_ERR_IRQ(_number)				\
	do {								\
		static CVMX_SHARED struct cvmx_interrupt _irq = {	\
			.handler = __cvmx_interrupt_ecc			\
		};							\
		cvmx_interrupt_register(cvmx_interrupt_map(_number), &_irq); \
		_irq.unmask(&_irq);					\
	} while (0)


/**
 * Process an interrupt request
 *
 * @param registers Registers at time of interrupt / exception
 * Registers 0-31 are standard MIPS, others specific to this routine
 * @return
 */
void cvmx_interrupt_do_irq(uint64_t * registers);
void cvmx_interrupt_do_irq(uint64_t * registers)
{
	uint64_t mask;
	uint64_t cause;
	uint64_t status;
	uint64_t cache_err;
	int i;
	uint32_t exc_vec;
	/* Determine the cause of the interrupt */
	asm volatile ("dmfc0 %0,$13,0":"=r" (cause));
	asm volatile ("dmfc0 %0,$12,0":"=r" (status));
	/* In case of exception, clear all interrupts to avoid recursive interrupts.
	   Also clear EXL bit to display the correct PC value. */
	if ((cause & 0x7c) == 0) {
		asm volatile ("dmtc0 %0, $12, 0"::"r" (status & ~(0xff02)));
	}
	/* The assembly stub at each exception vector saves its address in k1 when
	 ** it calls the stage 2 handler.  We use this to compute the exception vector
	 ** that brought us here */
	exc_vec = (uint32_t) (registers[27] & 0x780);	/* Mask off bits we need to ignore */

	/* Check for cache errors.  The cache errors go to a separate exception vector,
	 ** so we will only check these if we got here from a cache error exception, and
	 ** the ERL (error level) bit is set. */
	i = cvmx_get_core_num();
	if (exc_vec == 0x100 && (status & 0x4)) {
		CVMX_MF_CACHE_ERR(cache_err);

		/* Use copy of DCACHE_ERR register that early exception stub read */
		if (OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)) {
			if (registers[34] & 0x1)
				cvmx_safe_printf("Dcache error detected: core: %d, way: %d, va 7:3: 0x%x\n", i, (int)(registers[34] >> 8) & 0x3f, (int)(registers[34] >> 3) & 0x1f);
			else if (cache_err & 0x1)
				cvmx_safe_printf("Icache error detected: core: %d, set: %d, way : %d, va 6:3 = 0x%x\n", i, (int)(cache_err >> 5) & 0x3f,
						 (int)(cache_err >> 3) & 0x3, (int)(cache_err >> 11) & 0xf);
			else
				cvmx_safe_printf("Cache error exception: core %d\n", i);
			CVMX_MT_DCACHE_ERR(1);
		} else if (OCTEON_IS_OCTEON3()) {
			if (registers[34] & 0x1)
				cvmx_safe_printf("Dcache error detected: core: %d, way: %d, va 9:7: 0x%x\n", i, (int)(registers[34] >> 4) & 0x1f, (int)(registers[34] >> 1) & 0x3);
			else if (cache_err & 0x1)
				cvmx_safe_printf("Icache error detected: core: %d, way : %d, va 10:3 = 0x%x\n", i, (int)(cache_err >> 10) & 0x3f, (int)(cache_err >> 3) & 0xff);
			else
				cvmx_safe_printf("Cache error exception: core %d\n", i);
			/* Clear Dcache parity error */
			CVMX_MT_CACHE_ERR_CTL(1);
		} else {
			if (registers[34] & 0x1)
				cvmx_safe_printf("Dcache error detected: core: %d, way: %d, va 9:7: 0x%x\n", i, (int)(registers[34] >> 10) & 0x1f, (int)(registers[34] >> 7) & 0x3);
			else if (cache_err & 0x1)
				cvmx_safe_printf("Icache error detected: core: %d, way : %d, va 9:3 = 0x%x\n", i, (int)(cache_err >> 10) & 0x3f, (int)(cache_err >> 3) & 0x7f);
			else
				cvmx_safe_printf("Cache error exception: core %d\n", i);
			CVMX_MT_DCACHE_ERR(1);
		}
		CVMX_MT_CACHE_ERR(0);
	}

	/* The bus error exceptions can occur due to DID timeout or write buffer,
	   check by reading COP0_CACHEERRD */
	if (OCTEON_IS_OCTEON2()) {
		if (registers[34] & 0x4) {
			cvmx_safe_printf("Bus error detected due to DID timeout: core: %d\n", i);
			CVMX_MT_DCACHE_ERR(4);
		} else if (registers[34] & 0x2) {
			cvmx_safe_printf("Bus error detected due to write buffer parity: core: %d\n", i);
			CVMX_MT_DCACHE_ERR(2);
		}
	} else if (OCTEON_IS_OCTEON3()) {
		if (registers[34] & 0x400) {
			cvmx_safe_printf("Bus error detected due to DID timeout: core: %d\n", i);
			CVMX_MT_CACHE_ERR_CTL(0x400);
		} else if (registers[34] & 0x200) {
			cvmx_safe_printf("Bus error detected due to write buffer parity: core: %d\n", i);
			CVMX_MT_CACHE_ERR_CTL(0x200);
		}
	}

	if ((cause & 0x7c) != 0) {
		cvmx_interrupt_state.exception_handler(registers);
		goto return_from_interrupt;
	}

	/* Convert the cause into an active mask */
	mask = ((cause & status) >> 8) & 0xff;
	if (mask == 0) {
		goto return_from_interrupt;	/* Spurious interrupt */
	}

	for (i = 0; i < 8; i++) {
		if (mask & (1 << i)) {
			struct cvmx_interrupt *irq = cvmx_ipx_irq[i];
			irq->handler(irq, registers);
			goto return_from_interrupt;
		}
	}

	/* We should never get here */
	__cvmx_interrupt_default_exception_handler(registers);

return_from_interrupt:
	/* Restore Status register before returning from exception. */
	asm volatile ("dmtc0 %0, $12, 0"::"r" (status));
}

#define CLEAR_OR_MASK(V,M,O) ({\
            if (O)             \
                (V) &= ~(M);   \
            else               \
                (V) |= (M);    \
        })
/**
 * @INTERNAL
 */
static void __cvmx_interrupt_cpu_mask_unmask_irq(struct cvmx_interrupt *irq, int op)
{
	uint32_t flags, mask;

	flags = cvmx_interrupt_disable_save();
	asm volatile ("mfc0 %0,$12,0":"=r" (mask));
	CLEAR_OR_MASK(mask, 1 << (8 + irq->data), op);
	asm volatile ("mtc0 %0,$12,0"::"r" (mask));
	cvmx_interrupt_restore(flags);
}

/**
 * @INTERNAL
 */
static void __cvmx_interrupt_cpu_mask_irq(struct cvmx_interrupt *irq)
{
	__cvmx_interrupt_cpu_mask_unmask_irq(irq, 1);
}

/**
 * @INTERNAL
 */
static void __cvmx_interrupt_cpu_unmask_irq(struct cvmx_interrupt *irq)
{
	__cvmx_interrupt_cpu_mask_unmask_irq(irq, 0);
}

/**
 * @INTERNAL
 */
static void __cvmx_interrupt_ciu2_mask_unmask_irq(struct cvmx_interrupt *irq, int op)
{
	int idx;
	uint64_t reg;
	int core = cvmx_get_core_num();

	int bit = irq->data;

	if (bit < 0)
		return;

	idx = bit >> 6;
	bit &= 0x3f;
	if (idx > 7) {
		/* MBOX */
		if (op)
			reg = CVMX_CIU2_EN_PPX_IP2_MBOX_W1C(core);
		else
			reg = CVMX_CIU2_EN_PPX_IP2_MBOX_W1S(core);
	} else {
		if (op)
			reg = CVMX_CIU2_EN_PPX_IP2_WRKQ_W1C(core) + (0x1000 * idx);
		else
			reg = CVMX_CIU2_EN_PPX_IP2_WRKQ_W1S(core) + (0x1000 * idx);
	}
	cvmx_write_csr(reg, 1ull << bit);
}

/**
 * @INTERNAL
 */
static void __cvmx_interrupt_ciu2_mask_irq(struct cvmx_interrupt *irq)
{
	__cvmx_interrupt_ciu2_mask_unmask_irq(irq, 1);
}

/**
 * @INTERNAL
 */
static void __cvmx_interrupt_ciu2_unmask_irq(struct cvmx_interrupt *irq)
{
	__cvmx_interrupt_ciu2_mask_unmask_irq(irq, 0);
}

/**
 * @INTERNAL
 */
static void __cvmx_interrupt_ciu_mask_unmask_irq(struct cvmx_interrupt *irq, int op)
{
	uint32_t flags;
	int ciu_bit, ciu_offset;
	int bit = irq->data;
	int reg = bit >> 6;
	int core = cvmx_get_core_num();

	flags = cvmx_interrupt_disable_save();


	ciu_bit = bit & 0x3f;
	ciu_offset = core * 2;

	if (reg == 2) {
		CLEAR_OR_MASK(cvmx_interrupt_ciu_61xx_timer_mirror, 1ull << ciu_bit, op);
		CLEAR_OR_MASK(cvmx_interrupt_ciu_en0_mirror, 1ull << 51, op);	// SUM2 bit
		cvmx_write_csr(CVMX_CIU_EN2_PPX_IP2(core), cvmx_interrupt_ciu_61xx_timer_mirror);
	} else if (reg == 1) {
		/* EN1 */
		ciu_offset += 1;
		CLEAR_OR_MASK(cvmx_interrupt_ciu_en1_mirror, 1ull << ciu_bit, op);
		cvmx_write_csr(CVMX_CIU_INTX_EN1(ciu_offset), cvmx_interrupt_ciu_en1_mirror);
	} else {
		/* EN0 */
		CLEAR_OR_MASK(cvmx_interrupt_ciu_en0_mirror, 1ull << ciu_bit, op);
		cvmx_write_csr(CVMX_CIU_INTX_EN0(ciu_offset), cvmx_interrupt_ciu_en0_mirror);
	}

	cvmx_interrupt_restore(flags);
}

/**
 * @INTERNAL
 */
static void __cvmx_interrupt_ciu_mask_irq(struct cvmx_interrupt *irq)
{
	__cvmx_interrupt_ciu_mask_unmask_irq(irq, 1);
}

/**
 * @INTERNAL
 */
static void __cvmx_interrupt_ciu_unmask_irq(struct cvmx_interrupt *irq)
{
	__cvmx_interrupt_ciu_mask_unmask_irq(irq, 0);
}

/**
 * @INTERNAL
 */
static void cvmx_interrupt_register_cpu(unsigned int hw_irq_number, struct cvmx_interrupt *info)
{
	int bit = hw_irq_number & 0x3f;

	info->data = bit;
	info->mask = __cvmx_interrupt_cpu_mask_irq;
	info->unmask = __cvmx_interrupt_cpu_unmask_irq;

	cvmx_ipx_irq[bit] = info;
}

/**
 * @INTERNAL
 */
static void cvmx_interrupt_register_ciu(unsigned int hw_irq_number, struct cvmx_interrupt *info)
{
	int bit = hw_irq_number & 0x3f;
	int reg = hw_irq_number >> 6;

	switch (reg) {
	case 63: /* Magic value for CPU interrupts */
		cvmx_interrupt_register_cpu(hw_irq_number, info);
		break;
	case 0:
	case 1:
	case 2:
		info->data = hw_irq_number;
		info->mask = __cvmx_interrupt_ciu_mask_irq;
		info->unmask =__cvmx_interrupt_ciu_unmask_irq;
		cvmx_ciu_to_irq[reg][bit] = info;
		break;
	default:
		cvmx_warn("cvmx_interrupt_register: Illegal irq_number 0x%x\n", hw_irq_number);
		break;
		
	}
	CVMX_SYNC;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"

/**
 * @INTERNAL
 */
static unsigned int cvmx_interrupt_map_ciu(cvmx_irq_t e)
{
	unsigned int hw_int=0;

	switch (e) {
	case CVMX_IRQ_SW0 ... CVMX_IRQ_MIPS7:
		/* 63 is magic CPU irq value.*/
		hw_int = (e - CVMX_IRQ_SW0) + (63 << 6);
		break;
	case CVMX_IRQ_WORKQ0 ... (CVMX_IRQ_WORKQ0 + 15):
		hw_int = (e - CVMX_IRQ_WORKQ0);
		break;
	case CVMX_IRQ_GPIO0 ... (CVMX_IRQ_GPIO0 + 15):
		hw_int = (e - CVMX_IRQ_GPIO0) + 16;
		break;
	case CVMX_IRQ_MBOX0 ... (CVMX_IRQ_MBOX0 + 1):
		hw_int = (e - CVMX_IRQ_MBOX0) + 32;
		break;
	case CVMX_IRQ_UART0 ... (CVMX_IRQ_UART0 + 1):
		hw_int = (e - CVMX_IRQ_UART0) + 34;
		break;
	case CVMX_IRQ_UART0 + 2:
		hw_int = 16 + (1 << 6);
		break;
	case CVMX_IRQ_PCI_INT0 ... CVMX_IRQ_PCI_INT3:
		hw_int = (e - CVMX_IRQ_PCI_INT0) + 36;
		break;
	case CVMX_IRQ_PCI_MSI0 ... CVMX_IRQ_PCI_MSI3:
		hw_int = (e - CVMX_IRQ_PCI_MSI0) + 40;
		break;
	case CVMX_IRQ_TWSI0:
		hw_int = 45;
		break;
	case CVMX_IRQ_TWSI0 + 1:
		hw_int = 59;
		break;
	case CVMX_IRQ_RML:
		hw_int = 46;
		break;
	case CVMX_IRQ_TRACE0:
		hw_int = 47;
		break;
	case CVMX_IRQ_GMX_DRP0 ... CVMX_IRQ_GMX_DRP1:
		hw_int = (e - CVMX_IRQ_GMX_DRP0) + 48;
		break;
	case CVMX_IRQ_IPD_DRP:
		hw_int = 50;
		break;
	case CVMX_IRQ_KEY_ZERO:
		hw_int = 51;
		break;
	case CVMX_IRQ_TIMER0 ... (CVMX_IRQ_TIMER0 + 3):
		hw_int = (e - CVMX_IRQ_TIMER0) + 52;
		break;
	case CVMX_IRQ_USB0:
		hw_int = 56;
		break;
	case CVMX_IRQ_USB0 + 1:
		hw_int = 17 + (1 << 6);
		break;
	case CVMX_IRQ_PCM:
		hw_int = 57;
		break;
	case CVMX_IRQ_MPI:
		hw_int = 58;
		break;
	case CVMX_IRQ_POWIQ:
		hw_int = 60;
		break;
	case CVMX_IRQ_IPDPPTHR:
		hw_int = 61;
		break;
	case CVMX_IRQ_MII0:
		hw_int = 62;
		break;
	case CVMX_IRQ_MII0 + 1:
		hw_int = 18 + (1 << 6);
		break;
	case CVMX_IRQ_BOOTDMA:
		hw_int = 63;
		break;
	case CVMX_IRQ_WDOG0 ... (CVMX_IRQ_WDOG0 + 15):
		hw_int = (e - CVMX_IRQ_WDOG0) + (1 << 6);
		break;
	case CVMX_IRQ_NAND:
		hw_int = 19 + (1 << 6);
		break;
	case CVMX_IRQ_MIO:
		hw_int = 20 + (1 << 6);
		break;
	case CVMX_IRQ_IOB:
		hw_int = 21 + (1 << 6);
		break;
	case CVMX_IRQ_FPA:
		hw_int = 22 + (1 << 6);
		break;
	case CVMX_IRQ_POW:
		hw_int = 23 + (1 << 6);
		break;
	case CVMX_IRQ_L2C:
		hw_int = 24 + (1 << 6);
		break;
	case CVMX_IRQ_IPD:
		hw_int = 25 + (1 << 6);
		break;
	case CVMX_IRQ_PIP:
		hw_int = 26 + (1 << 6);
		break;
	case CVMX_IRQ_PKO:
		hw_int = 27 + (1 << 6);
		break;
	case CVMX_IRQ_ZIP:
		hw_int = 28 + (1 << 6);
		break;
	case CVMX_IRQ_TIM:
		hw_int = 29 + (1 << 6);
		break;
	case CVMX_IRQ_RAD:
		hw_int = 30 + (1 << 6);
		break;
	case CVMX_IRQ_KEY:
		hw_int = 31 + (1 << 6);
		break;
	case CVMX_IRQ_DFA:
		hw_int = 32 + (1 << 6);
		break;
	case CVMX_IRQ_USBCTL:
		hw_int = 33 + (1 << 6);
		break;
	case CVMX_IRQ_SLI:
		hw_int = 34 + (1 << 6);
		break;
	case CVMX_IRQ_DPI:
		hw_int = 35 + (1 << 6);
		break;
	case CVMX_IRQ_AGX0:
		hw_int = 36 + (1 << 6);
		break;
	case CVMX_IRQ_AGX0 + 1:
		hw_int = 37 + (1 << 6);
		break;
	case CVMX_IRQ_DPI_DMA:
		hw_int = 40 + (1 << 6);
		break;
	case CVMX_IRQ_AGL:
		hw_int = 46 + (1 << 6);
		break;
	case CVMX_IRQ_PTP:
		hw_int = 47 + (1 << 6);
		break;
	case CVMX_IRQ_PEM0:
		hw_int = 48 + (1 << 6);
		break;
	case CVMX_IRQ_PEM1:
		hw_int = 49 + (1 << 6);
		break;
	case CVMX_IRQ_SRIO1:
		hw_int = 51 + (1 << 6);
		break;
	case CVMX_IRQ_LMC0:
		hw_int = 52 + (1 << 6);
		break;
	case CVMX_IRQ_DFM:
		hw_int = 56 + (1 << 6);
		break;
	case CVMX_IRQ_SRIO2:
		hw_int = 60 + (1 << 6);
		break;
	case CVMX_IRQ_SRIO3:
		hw_int = 61 + (1 << 6);
		break;
	case CVMX_IRQ_RST:
		hw_int = 63 + (1 << 6);
		break;
	case CVMX_IRQ_PEM2:
	case CVMX_IRQ_SRIO0:
		hw_int = 50 + (1 << 6);
		break;
	case CVMX_IRQ_EOI:
		hw_int = 12 + (2 << 6);
		break;
	case CVMX_IRQ_ENDOR ... (CVMX_IRQ_ENDOR + 1):
		hw_int = (e - CVMX_IRQ_ENDOR) + 13 + (2 << 6);
		break;
	case CVMX_IRQ_TIMER4 ... (CVMX_IRQ_TIMER4 + 5):
		hw_int = (e - CVMX_IRQ_TIMER4) + 4 + (2 << 6);
		break;
	default:
		cvmx_warn("cvmx_interrupt_map: Illegal irq_number %d\n", e);

	}
	return hw_int;
}

#pragma GCC diagnostic pop

/**
 * @INTERNAL
 */
static void cvmx_interrupt_ciu_initialize(cvmx_sysinfo_t * sys_info_ptr)
{
	int core = cvmx_get_core_num();

	/* Disable all CIU interrupts by default */
	cvmx_interrupt_ciu_en0_mirror = 0;
	cvmx_interrupt_ciu_en1_mirror = 0;
	cvmx_interrupt_ciu_61xx_timer_mirror = 0;
	cvmx_write_csr(CVMX_CIU_INTX_EN0(core * 2), cvmx_interrupt_ciu_en0_mirror);
	cvmx_write_csr(CVMX_CIU_INTX_EN0((core * 2) + 1), cvmx_interrupt_ciu_en0_mirror);
	cvmx_write_csr(CVMX_CIU_INTX_EN1(core * 2), cvmx_interrupt_ciu_en1_mirror);
	cvmx_write_csr(CVMX_CIU_INTX_EN1((core * 2) + 1), cvmx_interrupt_ciu_en1_mirror);
	if (octeon_has_feature(OCTEON_FEATURE_MULTICAST_TIMER))
		cvmx_write_csr(CVMX_CIU_EN2_PPX_IP2(cvmx_get_core_num()),
				cvmx_interrupt_ciu_61xx_timer_mirror);

	if (cvmx_is_init_core()) {
		cvmx_interrupt_register = cvmx_interrupt_register_ciu;
		cvmx_interrupt_map = cvmx_interrupt_map_ciu;
	}
	CVMX_SYNC;
}

/**
 * @INTERNAL
 */
static void cvmx_interrupt_register_ciu2(unsigned int hw_irq_number, struct cvmx_interrupt *info)
{
	int bit = hw_irq_number & 0x3f;
	int reg = hw_irq_number >> 6;

	switch (reg) {
	case 63: /* Magic value for CPU interrupts */
		cvmx_interrupt_register_cpu(hw_irq_number, info);
		break;
	case 60: /* mailbox*/
		info->data = hw_irq_number;
		info->mask = __cvmx_interrupt_ciu2_mask_irq;
		info->unmask =__cvmx_interrupt_ciu2_unmask_irq;
		cvmx_ciu2_mbox_to_irq[bit] = info;
		break;
	case 0 ... 7:
		info->data = hw_irq_number;
		info->mask = __cvmx_interrupt_ciu2_mask_irq;
		info->unmask =__cvmx_interrupt_ciu2_unmask_irq;
		cvmx_ciu_to_irq[reg][bit] = info;
		break;
	default:
		cvmx_warn("cvmx_interrupt_register: Illegal irq_number 0x%x\n", hw_irq_number);
		break;
		
	}
	CVMX_SYNC;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"

/**
 * @INTERNAL
 */
static unsigned int cvmx_interrupt_map_ciu2(cvmx_irq_t e)
{
	unsigned int hw_int=0;

	switch (e) {
	case CVMX_IRQ_SW0 ... CVMX_IRQ_MIPS7:
		/* 63 is magic CPU irq value.*/
		hw_int = (e - CVMX_IRQ_SW0) + (63 << 6);
		break;
	case CVMX_IRQ_WORKQ0 ... (CVMX_IRQ_WORKQ0 + 63):
		hw_int = (e - CVMX_IRQ_WORKQ0);
		break;
	case CVMX_IRQ_GPIO0 ... (CVMX_IRQ_GPIO0 + 15):
		hw_int = (e - CVMX_IRQ_GPIO0) + (7 << 6);
		break;
	case CVMX_IRQ_MBOX0 ... (CVMX_IRQ_MBOX0 + 3):
		hw_int = (e - CVMX_IRQ_MBOX0) + (60 << 6);
		break;
	case CVMX_IRQ_UART0 ... (CVMX_IRQ_UART0 + 1):
		hw_int = (e - CVMX_IRQ_UART0) + 36 + (3 << 6);
		break;
	case CVMX_IRQ_PCI_INT0 ... CVMX_IRQ_PCI_INT3:
		hw_int = (e - CVMX_IRQ_PCI_INT0) + 16 + (4 << 6);
		break;
	case CVMX_IRQ_PCI_MSI0 ... CVMX_IRQ_PCI_MSI3:
		hw_int = (e - CVMX_IRQ_PCI_MSI0) + 8 + (4 << 6);
		break;
	case CVMX_IRQ_TWSI0 ... (CVMX_IRQ_TWSI0 + 1):
		hw_int = (e - CVMX_IRQ_TWSI0) + 32 + (3 << 6);
		break;
	case CVMX_IRQ_TRACE0 ... (CVMX_IRQ_TRACE0 + 3):
		hw_int = (e - CVMX_IRQ_TRACE0) + 52 + (2 << 6);;
		break;
	case CVMX_IRQ_GMX_DRP0 ... CVMX_IRQ_GMX_DRP0 + 4:
		hw_int = (e - CVMX_IRQ_GMX_DRP0) + 8 + (6 << 6);
		break;
	case CVMX_IRQ_IPD_DRP:
		hw_int = 2 + (3 << 6);
		break;
	case CVMX_IRQ_TIMER0 ... (CVMX_IRQ_TIMER0 + 3):
		hw_int = (e - CVMX_IRQ_TIMER0) + 8 + (3 << 6);
		break;
	case CVMX_IRQ_USB0:
		hw_int = 44 + (3 << 6);
		break;
	case CVMX_IRQ_MII0:
		hw_int = 40 + (6 << 6);
		break;
	case CVMX_IRQ_BOOTDMA:
		hw_int = 18 + (3 << 6);
		break;
	case CVMX_IRQ_WDOG0 ... (CVMX_IRQ_WDOG0 + 31):
		hw_int = (e - CVMX_IRQ_WDOG0) + (1 << 6);
		break;
	case CVMX_IRQ_POWIQ:
		hw_int = 1 + (3 << 6);
		break;
	case CVMX_IRQ_IPDPPTHR:
		hw_int = 0 + (3 << 6);
		break;
	case CVMX_IRQ_NAND:
		hw_int = 16 + (3 << 6);
		break;
	case CVMX_IRQ_MIO:
		hw_int = 17 + (3 << 6);
		break;
	case CVMX_IRQ_IOB:
		hw_int = 0 + (2 << 6);
		break;
	case CVMX_IRQ_FPA:
		hw_int = 4 + (2 << 6);
		break;
	case CVMX_IRQ_POW:
		hw_int = 16 + (2 << 6);
		break;
	case CVMX_IRQ_L2C:
		hw_int = 48 + (2 << 6);
		break;
	case CVMX_IRQ_IPD:
		hw_int = 5 + (2 << 6);
		break;
	case CVMX_IRQ_PIP:
		hw_int = 6 + (2 << 6);
		break;
	case CVMX_IRQ_PKO:
		hw_int = 7 + (2 << 6);
		break;
	case CVMX_IRQ_ZIP:
		hw_int = 24 + (2 << 6);
		break;
	case CVMX_IRQ_TIM:
		hw_int = 28 + (2 << 6);
		break;
	case CVMX_IRQ_RAD:
		hw_int = 29 + (2 << 6);
		break;
	case CVMX_IRQ_KEY:
		hw_int = 30 + (2 << 6);
		break;
	case CVMX_IRQ_DFA:
		hw_int = 40 + (2 << 6);
		break;
	case CVMX_IRQ_USBCTL:
		hw_int = 40 + (3 << 6);
		break;
	case CVMX_IRQ_SLI:
		hw_int = 32 + (2 << 6);
		break;
	case CVMX_IRQ_DPI:
		hw_int = 33 + (2 << 6);
		break;
	case CVMX_IRQ_DPI_DMA:
		hw_int = 36 + (2 << 6);
		break;
	case CVMX_IRQ_AGX0 ... (CVMX_IRQ_AGX0 + 4):
		hw_int = (e - CVMX_IRQ_AGX0) + 0 + (6 << 6);
		break;
	case CVMX_IRQ_AGL:
		hw_int = 32 + (6 << 6);
		break;
	case CVMX_IRQ_PTP:
		hw_int = 48 + (3 << 6);
		break;
	case CVMX_IRQ_PEM0:
		hw_int = 32 + (4 << 6);
		break;
	case CVMX_IRQ_PEM1:
		hw_int = 33 + (4<< 6);
		break;
	case CVMX_IRQ_LMC0 ... (CVMX_IRQ_LMC0 + 3):
		hw_int = (e - CVMX_IRQ_LMC0) + 0 + (5 << 6);
		break;
	case CVMX_IRQ_RST:
		hw_int = 63 + (3 << 6);
		break;
	case CVMX_IRQ_ILK:
		hw_int = 63 + (3 << 6);
		break;
	default:
		cvmx_warn("cvmx_interrupt_map: Illegal irq_number %d\n", e);

	}
	return hw_int;
}
#pragma GCC diagnostic pop

/**
 * @INTERNAL
 */
static void cvmx_interrupt_ciu2_initialize(cvmx_sysinfo_t * sys_info_ptr)
{
	/* Disable all CIU2 interrupts by default */

	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP2_WRKQ(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP3_WRKQ(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP4_WRKQ(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP2_WDOG(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP3_WDOG(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP4_WDOG(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP2_RML(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP3_RML(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP4_RML(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP2_MIO(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP3_MIO(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP4_MIO(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP2_IO(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP3_IO(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP4_IO(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP2_MEM(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP3_MEM(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP4_MEM(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP2_PKT(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP3_PKT(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP4_PKT(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP2_GPIO(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP3_GPIO(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP4_GPIO(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP2_MBOX(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP3_MBOX(cvmx_get_core_num()), 0);
	cvmx_write_csr(CVMX_CIU2_EN_PPX_IP4_MBOX(cvmx_get_core_num()), 0);

	if (cvmx_is_init_core()) {
		cvmx_interrupt_register = cvmx_interrupt_register_ciu2;
		cvmx_interrupt_map = cvmx_interrupt_map_ciu2;
	}
	CVMX_SYNC;
}

/**
 * @INTERNAL
 */
static void __cvmx_interrupt_ciu3_mask_irq(struct cvmx_interrupt *irq)
{
	cvmx_ciu3_iscx_w1c_t w1c;
	uint64_t addr = CVMX_CIU3_ISCX_W1C(irq->data);

	w1c.u64 = 0;
	w1c.s.en = 1;
	cvmx_write_csr(addr, w1c.u64);
	/* Read it back to make sure it took effect. */
	cvmx_read_csr(addr);
}

/**
 * @INTERNAL
 */
static void __cvmx_interrupt_ciu3_unmask_irq(struct cvmx_interrupt *irq)
{
	
	cvmx_ciu3_iscx_ctl_t ctl;
	uint64_t addr = CVMX_CIU3_ISCX_CTL(irq->data);

	/* It must be masked before we can modify it. */
	__cvmx_interrupt_ciu3_mask_irq(irq);

	ctl.u64 = 0;
	ctl.s.en = 1;
	/*
	 * 4 idt per core starting from 1 because zero is reserved.
	 * Base idt per core is 4 * core + 1
	 */
	ctl.s.idt = cvmx_get_core_num() * 4 + 1;

	cvmx_write_csr(addr, ctl.u64);
	/* Read it back to make sure it took effect. */
	cvmx_read_csr(addr);
}

/**
 * @INTERNAL
 */
static void cvmx_interrupt_register_ciu3(unsigned int hw_irq_number, struct cvmx_interrupt *info)
{
	struct cvmx_interrupt **slot;
	int high = hw_irq_number >> 12;

	info->r = NULL;
	info->l = NULL;

	switch (high) {
	case 0: /* Magic value for CPU interrupts */
		cvmx_interrupt_register_cpu(hw_irq_number, info);
		break;
	default:
		info->data = hw_irq_number;
		info->mask = __cvmx_interrupt_ciu3_mask_irq;
		info->unmask =__cvmx_interrupt_ciu3_unmask_irq;
		slot = &cvmx_ciux_to_irq.ciu2_high_bits_to_irq[high];
		while (*slot)
			slot = &(*slot)->r;
		*slot = info;
		break;
	}
	CVMX_SYNC;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"

/**
 * @INTERNAL
 */
static unsigned int cvmx_interrupt_map_ciu3(cvmx_irq_t e)
{
	unsigned int hw_int=0;

	switch (e) {
	case CVMX_IRQ_SW0 ... CVMX_IRQ_MIPS7:
		/* bits 12..19 == 0 is magic CPU irq value.*/
		hw_int = (e - CVMX_IRQ_SW0);
		break;
	case CVMX_IRQ_UART0:
		hw_int = 0x08000;
		break;
	case CVMX_IRQ_UART0 + 1:
		hw_int = 0x08040;
		break;
	default:
		cvmx_warn("cvmx_interrupt_map: Illegal irq_number %d\n", e);

	}
	return hw_int;
}
#pragma GCC diagnostic pop

/**
 * @INTERNAL
 */
static void cvmx_interrupt_ciu3_initialize(cvmx_sysinfo_t * sys_info_ptr)
{
	/*
	 * 4 idt per core starting from 1 because zero is reserved.
	 * Base idt per core is 4 * core + 1
	 */
	int i;
	unsigned int core = cvmx_get_core_num();
	int idt = core * 4 + 1;
	
	/* Route to ip2 */
	cvmx_write_csr(CVMX_CIU3_IDTX_CTL(idt), 0);
	/* On this core */
	cvmx_write_csr(CVMX_CIU3_IDTX_PPX(0, idt), 1ull << core);
	/* Not to I/O */
	cvmx_write_csr(CVMX_CIU3_IDTX_IO(idt), 0);

	/* The rest route to ip2 on no cores to disable */
	for (i = 1; i <=3; i++) {
		cvmx_write_csr(CVMX_CIU3_IDTX_CTL(idt + i), 0);
		cvmx_write_csr(CVMX_CIU3_IDTX_PPX(0, idt + i), 0);
		cvmx_write_csr(CVMX_CIU3_IDTX_IO(idt + i), 0);
	}
	/* Read back to ensure they are written before any interrupt is enabled. */
	cvmx_read_csr(CVMX_CIU3_IDTX_CTL(idt));

	if (cvmx_is_init_core()) {
		cvmx_interrupt_register = cvmx_interrupt_register_ciu3;
		cvmx_interrupt_map = cvmx_interrupt_map_ciu3;
	}
	CVMX_SYNC;
}

static CVMX_SHARED struct cvmx_interrupt_cpu cpu_ip2 = {
	.ip = 2
};
static CVMX_SHARED struct cvmx_interrupt_cpu cpu_ip3 = {
	.ip = 3
};

/**
 * Initialize the interrupt routine and copy the low level
 * stub into the correct interrupt vector. This is called
 * automatically during application startup.
 */
void cvmx_interrupt_initialize(void)
{
	void *low_level_loc;
	cvmx_sysinfo_t *sys_info_ptr = cvmx_sysinfo_get();

#ifndef CVMX_ENABLE_CSR_ADDRESS_CHECKING
	/* We assume this relationship between the registers. */
	CVMX_BUILD_ASSERT(CVMX_CIU2_SRC_PPX_IP2_WRKQ(0) + 0x1000 == CVMX_CIU2_SRC_PPX_IP2_WDOG(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_SRC_PPX_IP2_WRKQ(0) + 0x2000 == CVMX_CIU2_SRC_PPX_IP2_RML(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_SRC_PPX_IP2_WRKQ(0) + 0x3000 == CVMX_CIU2_SRC_PPX_IP2_MIO(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_SRC_PPX_IP2_WRKQ(0) + 0x4000 == CVMX_CIU2_SRC_PPX_IP2_IO(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_SRC_PPX_IP2_WRKQ(0) + 0x5000 == CVMX_CIU2_SRC_PPX_IP2_MEM(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_SRC_PPX_IP2_WRKQ(0) + 0x6000 == CVMX_CIU2_SRC_PPX_IP2_PKT(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_SRC_PPX_IP2_WRKQ(0) + 0x7000 == CVMX_CIU2_SRC_PPX_IP2_GPIO(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_EN_PPX_IP2_WRKQ_W1C(0) + 0x1000 == CVMX_CIU2_EN_PPX_IP2_WDOG_W1C(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_EN_PPX_IP2_WRKQ_W1C(0) + 0x2000 == CVMX_CIU2_EN_PPX_IP2_RML_W1C(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_EN_PPX_IP2_WRKQ_W1C(0) + 0x3000 == CVMX_CIU2_EN_PPX_IP2_MIO_W1C(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_EN_PPX_IP2_WRKQ_W1C(0) + 0x4000 == CVMX_CIU2_EN_PPX_IP2_IO_W1C(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_EN_PPX_IP2_WRKQ_W1C(0) + 0x5000 == CVMX_CIU2_EN_PPX_IP2_MEM_W1C(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_EN_PPX_IP2_WRKQ_W1C(0) + 0x6000 == CVMX_CIU2_EN_PPX_IP2_PKT_W1C(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_EN_PPX_IP2_WRKQ_W1C(0) + 0x7000 == CVMX_CIU2_EN_PPX_IP2_GPIO_W1C(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_EN_PPX_IP2_WRKQ_W1S(0) + 0x1000 == CVMX_CIU2_EN_PPX_IP2_WDOG_W1S(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_EN_PPX_IP2_WRKQ_W1S(0) + 0x2000 == CVMX_CIU2_EN_PPX_IP2_RML_W1S(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_EN_PPX_IP2_WRKQ_W1S(0) + 0x3000 == CVMX_CIU2_EN_PPX_IP2_MIO_W1S(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_EN_PPX_IP2_WRKQ_W1S(0) + 0x4000 == CVMX_CIU2_EN_PPX_IP2_IO_W1S(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_EN_PPX_IP2_WRKQ_W1S(0) + 0x5000 == CVMX_CIU2_EN_PPX_IP2_MEM_W1S(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_EN_PPX_IP2_WRKQ_W1S(0) + 0x6000 == CVMX_CIU2_EN_PPX_IP2_PKT_W1S(0));
	CVMX_BUILD_ASSERT(CVMX_CIU2_EN_PPX_IP2_WRKQ_W1S(0) + 0x7000 == CVMX_CIU2_EN_PPX_IP2_GPIO_W1S(0));
#endif /* !CVMX_ENABLE_CSR_ADDRESS_CHECKING */

	if (octeon_has_feature(OCTEON_FEATURE_CIU3))
		cvmx_interrupt_ciu3_initialize(sys_info_ptr);
	else if (octeon_has_feature(OCTEON_FEATURE_CIU2))
		cvmx_interrupt_ciu2_initialize(sys_info_ptr);
	else
		cvmx_interrupt_ciu_initialize(sys_info_ptr);

	/* Move performance counter interrupts to IRQ 6 */
	cvmx_update_perfcnt_irq();

	if (cvmx_is_init_core()) {
		if (octeon_has_feature(OCTEON_FEATURE_CIU3)) {
			cpu_ip2.irq.handler = __cvmx_interrupt_ciu3;
			/* Add an interrupt handlers for chained CIU interrupt */
			cvmx_interrupt_register(cvmx_interrupt_map(CVMX_IRQ_MIPS2),
						&cpu_ip2.irq);
		} else if (octeon_has_feature(OCTEON_FEATURE_CIU2)) {
			cpu_ip2.irq.handler = __cvmx_interrupt_ciu2;
			/* Add an interrupt handlers for chained CIU interrupt */
			cvmx_interrupt_register(cvmx_interrupt_map(CVMX_IRQ_MIPS2),
						&cpu_ip2.irq);
		} else if (OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN66XX_PASS1_2) || OCTEON_IS_MODEL(OCTEON_CNF71XX)) {
			cpu_ip2.irq.handler = __cvmx_interrupt_ciu;
			cpu_ip3.irq.handler = __cvmx_interrupt_ciu_cn61xx;
			/* Add an interrupt handlers for chained CIU interrupts */
			cvmx_interrupt_register(cvmx_interrupt_map(CVMX_IRQ_MIPS2),
						&cpu_ip2.irq);
			cvmx_interrupt_register(cvmx_interrupt_map(CVMX_IRQ_MIPS3),
						&cpu_ip3.irq);
		} else {
			cpu_ip2.irq.handler = __cvmx_interrupt_ciu;
			cpu_ip3.irq.handler = __cvmx_interrupt_ciu;
			/* Add an interrupt handlers for chained CIU interrupts */
			cvmx_interrupt_register(cvmx_interrupt_map(CVMX_IRQ_MIPS2),
						&cpu_ip2.irq);
			cvmx_interrupt_register(cvmx_interrupt_map(CVMX_IRQ_MIPS3),
						&cpu_ip3.irq);
		}
		/* Add an interrupt handler for Perf counter interrupts */
		cvmx_interrupt_register(cvmx_interrupt_map(CVMX_IRQ_MIPS6), &__cvmx_interrupt_perf);

		cvmx_interrupt_state.exception_handler = __cvmx_interrupt_default_exception_handler;

		low_level_loc = CASTPTR(void, CVMX_ADD_SEG32(CVMX_MIPS32_SPACE_KSEG0, sys_info_ptr->exception_base_addr));
		memcpy(low_level_loc + 0x80, (void *)cvmx_interrupt_stage1, 0x80);
		if (OCTEON_IS_OCTEON3())
			memcpy(low_level_loc + 0x100, (void *)cvmx_interrupt_cache_error_v3, 0x80);
		else
			memcpy(low_level_loc + 0x100, (void *)cvmx_interrupt_cache_error, 0x80);
		memcpy(low_level_loc + 0x180, (void *)cvmx_interrupt_stage1, 0x80);
		memcpy(low_level_loc + 0x200, (void *)cvmx_interrupt_stage1, 0x80);

		/* Make sure the locations used to count Icache and Dcache exceptions
		   starts out as zero */
		cvmx_write64_uint64(CVMX_ADD_SEG32(CVMX_MIPS32_SPACE_KSEG0, 8), 0);
		cvmx_write64_uint64(CVMX_ADD_SEG32(CVMX_MIPS32_SPACE_KSEG0, 16), 0);
		cvmx_write64_uint64(CVMX_ADD_SEG32(CVMX_MIPS32_SPACE_KSEG0, 24), 0);
		CVMX_SYNC;

		/* Add an interrupt handler for ECC failures */
		if (cvmx_error_initialize(0 /* || CVMX_ERROR_FLAGS_ECC_SINGLE_BIT */ ))
			cvmx_warn("cvmx_error_initialize() failed\n");

		/* Enable PIP/IPD, POW, PKO, FPA, NAND, KEY, RAD, L2C, LMC, GMX, AGL,
		   DFM, DFA, error handling interrupts. */
		if (octeon_has_feature(OCTEON_FEATURE_CIU3)) {
		} else if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_AGX0 + 0);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_AGX0 + 1);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_AGX0 + 2);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_AGX0 + 3);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_AGX0 + 4);

			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_POWIQ);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_NAND);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_MIO);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_FPA);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_IPD);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_PIP);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_POW);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_L2C);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_PKO);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_ZIP);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_TIM);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_RAD);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_KEY);
			/* Before enabling SLI interrupt clear any RML_TO interrupt */
			if (cvmx_read_csr(CVMX_PEXP_SLI_INT_SUM) & 0x1) {
				cvmx_safe_printf("clearing pending SLI_INT_SUM[RML_TO] interrupt (ignore)\n");
				cvmx_write_csr(CVMX_PEXP_SLI_INT_SUM, 1);
			}
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_SLI);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_DPI);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_DFA);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_AGL);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_LMC0 + 0);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_LMC0 + 1);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_LMC0 + 2);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_LMC0 + 3);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_RST);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_ILK);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_USBCTL);
		} else if (OCTEON_IS_MODEL(OCTEON_CN3XXX)
			   || OCTEON_IS_MODEL(OCTEON_CN5XXX)) {
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_RML);
		} else {
			if (octeon_has_feature(OCTEON_FEATURE_DFM))
				REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_DFM);

			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_PIP);

			if (octeon_has_feature(OCTEON_FEATURE_RAID))
				REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_RAD);

			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_POW);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_IPD);

			if (octeon_has_feature(OCTEON_FEATURE_ZIP))
				REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_ZIP);

			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_TIM);

			if (octeon_has_feature(OCTEON_FEATURE_HFA))
				REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_DFA);

			if (octeon_has_feature(OCTEON_FEATURE_DFA))
				REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_DFA);

			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_FPA);

			if (octeon_has_feature(OCTEON_FEATURE_KEY_MEMORY))
				REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_KEY);

			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_DPI);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_SLI);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_L2C);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_RST);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_MIO);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_NAND);
			REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_POWIQ);

			if (octeon_has_feature(OCTEON_FEATURE_DFA))
				REGISTER_AND_UNMASK_ERR_IRQ(CVMX_IRQ_DFA);
		}

		cvmx_atomic_set32(&cvmx_interrupt_initialize_flag, 1);
	}

	while (!cvmx_atomic_get32(&cvmx_interrupt_initialize_flag)) ;	/* Wait for first core to finish above. */

	if (octeon_has_feature(OCTEON_FEATURE_CIU3)) {
		cpu_ip2.irq.unmask(&cpu_ip2.irq);
	} else if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		cpu_ip2.irq.unmask(&cpu_ip2.irq);
	} else {
		cpu_ip2.irq.unmask(&cpu_ip2.irq);
		cpu_ip3.irq.unmask(&cpu_ip3.irq);
	}

	CVMX_ICACHE_INVALIDATE;

	/* Enable interrupts for each core (bit0 of COP0 Status) */
	cvmx_interrupt_restore(1);
}

/**
 * Set the exception handler for all non interrupt sources.
 *
 * @param handler New exception handler
 * @return Old exception handler
 */
cvmx_interrupt_exception_t cvmx_interrupt_set_exception(cvmx_interrupt_exception_t handler)
{
	cvmx_interrupt_exception_t result = cvmx_interrupt_state.exception_handler;
	cvmx_interrupt_state.exception_handler = handler;
	CVMX_SYNCWS;
	return result;
}
#endif /* !__U_BOOT__ */
