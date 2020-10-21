/* libunwind - a platform-independent unwind library
   Copyright (C) 2006-2007 IBM
   Contributed by
     Corey Ashford <cjashfor@us.ibm.com>
     Jose Flavio Aguilar Paulino <jflavio@br.ibm.com> <joseflavio@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#ifndef ucontext_i_h
#define ucontext_i_h

#include "compiler.h"
#include <ucontext.h>

/* These values were derived by reading
   /usr/src/linux-2.6.18-1.8/arch/um/include/sysdep-ppc/ptrace.h and
   /usr/src/linux-2.6.18-1.8/arch/powerpc/kernel/ppc32.h
*/

//#define NIP_IDX               32
#define CTR_IDX         32
#define XER_IDX         33
#define CCR_IDX         34
#define MSR_IDX         35
//#define MQ_IDX                36
#define LINK_IDX        36

/* These are dummy structures used only for obtaining the offsets of the
   various structure members. */
static ucontext_t dmy_ctxt UNUSED;

#ifdef __GLIBC__
#define UC_MCONTEXT_OFFSET(field) ((void *)&dmy_ctxt.uc_mcontext.uc_regs->field - (void *)&dmy_ctxt)
#else
#define UC_MCONTEXT_OFFSET(field) ((void *)&dmy_ctxt.uc_mcontext.field - (void *)&dmy_ctxt)
#endif

#define UC_MCONTEXT_GREGS_R0 UC_MCONTEXT_OFFSET(gregs[0])
#define UC_MCONTEXT_GREGS_R1 UC_MCONTEXT_OFFSET(gregs[1])
#define UC_MCONTEXT_GREGS_R2 UC_MCONTEXT_OFFSET(gregs[2])
#define UC_MCONTEXT_GREGS_R3 UC_MCONTEXT_OFFSET(gregs[3])
#define UC_MCONTEXT_GREGS_R4 UC_MCONTEXT_OFFSET(gregs[4])
#define UC_MCONTEXT_GREGS_R5 UC_MCONTEXT_OFFSET(gregs[5])
#define UC_MCONTEXT_GREGS_R6 UC_MCONTEXT_OFFSET(gregs[6])
#define UC_MCONTEXT_GREGS_R7 UC_MCONTEXT_OFFSET(gregs[7])
#define UC_MCONTEXT_GREGS_R8 UC_MCONTEXT_OFFSET(gregs[8])
#define UC_MCONTEXT_GREGS_R9 UC_MCONTEXT_OFFSET(gregs[9])
#define UC_MCONTEXT_GREGS_R10 UC_MCONTEXT_OFFSET(gregs[10])
#define UC_MCONTEXT_GREGS_R11 UC_MCONTEXT_OFFSET(gregs[11])
#define UC_MCONTEXT_GREGS_R12 UC_MCONTEXT_OFFSET(gregs[12])
#define UC_MCONTEXT_GREGS_R13 UC_MCONTEXT_OFFSET(gregs[13])
#define UC_MCONTEXT_GREGS_R14 UC_MCONTEXT_OFFSET(gregs[14])
#define UC_MCONTEXT_GREGS_R15 UC_MCONTEXT_OFFSET(gregs[15])
#define UC_MCONTEXT_GREGS_R16 UC_MCONTEXT_OFFSET(gregs[16])
#define UC_MCONTEXT_GREGS_R17 UC_MCONTEXT_OFFSET(gregs[17])
#define UC_MCONTEXT_GREGS_R18 UC_MCONTEXT_OFFSET(gregs[18])
#define UC_MCONTEXT_GREGS_R19 UC_MCONTEXT_OFFSET(gregs[19])
#define UC_MCONTEXT_GREGS_R20 UC_MCONTEXT_OFFSET(gregs[20])
#define UC_MCONTEXT_GREGS_R21 UC_MCONTEXT_OFFSET(gregs[21])
#define UC_MCONTEXT_GREGS_R22 UC_MCONTEXT_OFFSET(gregs[22])
#define UC_MCONTEXT_GREGS_R23 UC_MCONTEXT_OFFSET(gregs[23])
#define UC_MCONTEXT_GREGS_R24 UC_MCONTEXT_OFFSET(gregs[24])
#define UC_MCONTEXT_GREGS_R25 UC_MCONTEXT_OFFSET(gregs[25])
#define UC_MCONTEXT_GREGS_R26 UC_MCONTEXT_OFFSET(gregs[26])
#define UC_MCONTEXT_GREGS_R27 UC_MCONTEXT_OFFSET(gregs[27])
#define UC_MCONTEXT_GREGS_R28 UC_MCONTEXT_OFFSET(gregs[28])
#define UC_MCONTEXT_GREGS_R29 UC_MCONTEXT_OFFSET(gregs[29])
#define UC_MCONTEXT_GREGS_R30 UC_MCONTEXT_OFFSET(gregs[30])
#define UC_MCONTEXT_GREGS_R31 UC_MCONTEXT_OFFSET(gregs[31])

#define UC_MCONTEXT_GREGS_MSR UC_MCONTEXT_OFFSET(gregs[MSR_IDX])
#define UC_MCONTEXT_GREGS_ORIG_GPR3 UC_MCONTEXT_OFFSET(gregs[ORIG_GPR3_IDX])
#define UC_MCONTEXT_GREGS_CTR UC_MCONTEXT_OFFSET(gregs[CTR_IDX])
#define UC_MCONTEXT_GREGS_LINK UC_MCONTEXT_OFFSET(gregs[LINK_IDX])
#define UC_MCONTEXT_GREGS_XER UC_MCONTEXT_OFFSET(gregs[XER_IDX])
#define UC_MCONTEXT_GREGS_CCR UC_MCONTEXT_OFFSET(gregs[CCR_IDX])
#define UC_MCONTEXT_GREGS_SOFTE UC_MCONTEXT_OFFSET(gregs[SOFTE_IDX])
#define UC_MCONTEXT_GREGS_TRAP UC_MCONTEXT_OFFSET(gregs[TRAP_IDX])
#define UC_MCONTEXT_GREGS_DAR UC_MCONTEXT_OFFSET(gregs[DAR_IDX])
#define UC_MCONTEXT_GREGS_DSISR UC_MCONTEXT_OFFSET(gregs[DSISR_IDX])
#define UC_MCONTEXT_GREGS_RESULT UC_MCONTEXT_OFFSET(gregs[RESULT_IDX])

#define UC_MCONTEXT_FREGS_R0 UC_MCONTEXT_OFFSET(fpregs.fpregs[0])
#define UC_MCONTEXT_FREGS_R1 UC_MCONTEXT_OFFSET(fpregs.fpregs[1])
#define UC_MCONTEXT_FREGS_R2 UC_MCONTEXT_OFFSET(fpregs.fpregs[2])
#define UC_MCONTEXT_FREGS_R3 UC_MCONTEXT_OFFSET(fpregs.fpregs[3])
#define UC_MCONTEXT_FREGS_R4 UC_MCONTEXT_OFFSET(fpregs.fpregs[4])
#define UC_MCONTEXT_FREGS_R5 UC_MCONTEXT_OFFSET(fpregs.fpregs[5])
#define UC_MCONTEXT_FREGS_R6 UC_MCONTEXT_OFFSET(fpregs.fpregs[6])
#define UC_MCONTEXT_FREGS_R7 UC_MCONTEXT_OFFSET(fpregs.fpregs[7])
#define UC_MCONTEXT_FREGS_R8 UC_MCONTEXT_OFFSET(fpregs.fpregs[8])
#define UC_MCONTEXT_FREGS_R9 UC_MCONTEXT_OFFSET(fpregs.fpregs[9])
#define UC_MCONTEXT_FREGS_R10 UC_MCONTEXT_OFFSET(fpregs.fpregs[10])
#define UC_MCONTEXT_FREGS_R11 UC_MCONTEXT_OFFSET(fpregs.fpregs[11])
#define UC_MCONTEXT_FREGS_R12 UC_MCONTEXT_OFFSET(fpregs.fpregs[12])
#define UC_MCONTEXT_FREGS_R13 UC_MCONTEXT_OFFSET(fpregs.fpregs[13])
#define UC_MCONTEXT_FREGS_R14 UC_MCONTEXT_OFFSET(fpregs.fpregs[14])
#define UC_MCONTEXT_FREGS_R15 UC_MCONTEXT_OFFSET(fpregs.fpregs[15])
#define UC_MCONTEXT_FREGS_R16 UC_MCONTEXT_OFFSET(fpregs.fpregs[16])
#define UC_MCONTEXT_FREGS_R17 UC_MCONTEXT_OFFSET(fpregs.fpregs[17])
#define UC_MCONTEXT_FREGS_R18 UC_MCONTEXT_OFFSET(fpregs.fpregs[18])
#define UC_MCONTEXT_FREGS_R19 UC_MCONTEXT_OFFSET(fpregs.fpregs[19])
#define UC_MCONTEXT_FREGS_R20 UC_MCONTEXT_OFFSET(fpregs.fpregs[20])
#define UC_MCONTEXT_FREGS_R21 UC_MCONTEXT_OFFSET(fpregs.fpregs[21])
#define UC_MCONTEXT_FREGS_R22 UC_MCONTEXT_OFFSET(fpregs.fpregs[22])
#define UC_MCONTEXT_FREGS_R23 UC_MCONTEXT_OFFSET(fpregs.fpregs[23])
#define UC_MCONTEXT_FREGS_R24 UC_MCONTEXT_OFFSET(fpregs.fpregs[24])
#define UC_MCONTEXT_FREGS_R25 UC_MCONTEXT_OFFSET(fpregs.fpregs[25])
#define UC_MCONTEXT_FREGS_R26 UC_MCONTEXT_OFFSET(fpregs.fpregs[26])
#define UC_MCONTEXT_FREGS_R27 UC_MCONTEXT_OFFSET(fpregs.fpregs[27])
#define UC_MCONTEXT_FREGS_R28 UC_MCONTEXT_OFFSET(fpregs.fpregs[28])
#define UC_MCONTEXT_FREGS_R29 UC_MCONTEXT_OFFSET(fpregs.fpregs[29])
#define UC_MCONTEXT_FREGS_R30 UC_MCONTEXT_OFFSET(fpregs.fpregs[30])
#define UC_MCONTEXT_FREGS_R31 UC_MCONTEXT_OFFSET(fpregs.fpregs[31])
#define UC_MCONTEXT_FREGS_FPSCR UC_MCONTEXT_OFFSET(fpregs.fpregs[32])

#endif
