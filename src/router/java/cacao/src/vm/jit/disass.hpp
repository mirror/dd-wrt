/* src/vm/jit/disass.hpp - disassembler header

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/


#ifndef DISASS_HPP_
#define DISASS_HPP_ 1

#include "config.h"

#if defined(WITH_BINUTILS_DISASSEMBLER)
# include <dis-asm.h>
#endif

#include "vm/types.hpp"

#include "vm/global.hpp"

#if defined(ENABLE_DISASSEMBLER)

/* some macros ****************************************************************/

#if defined(ENABLE_JIT)
# if defined(ENABLE_INTRP)

#define DISASSINSTR(code) \
    do { \
        if (opt_intrp) \
            (code) = intrp_disassinstr((u1*) (code));	\
        else \
            (code) = disassinstr((u1*) (code));	\
    } while (0)

#define DISASSEMBLE(start,end) \
    do { \
        if (opt_intrp) \
            intrp_disassemble((start), (end)); \
        else \
            disassemble((start), (end)); \
    } while (0)

# else /* defined(ENABLE_INTRP) */

#define DISASSINSTR(code) \
    (code) = disassinstr((u1*) (code))

#define DISASSEMBLE(start,end) \
    disassemble((start), (end))

# endif /* defined(ENABLE_INTRP) */
#else /* defined(ENABLE_JIT) */

#define DISASSINSTR(code) \
    (code) = intrp_disassinstr((u1*) (code))

#define DISASSEMBLE(start,end) \
    intrp_disassemble((start), (end))

#endif /* defined(ENABLE_JIT) */


/* export global variables ****************************************************/

#if defined(WITH_BINUTILS_DISASSEMBLER)
extern disassemble_info info;
extern bool disass_initialized;
#endif

#if defined(__I386__) || defined(__X86_64__) || defined(__S390__)
extern char disass_buf[512];
extern s4   disass_len;
#endif


/* function prototypes *******************************************************/

extern "C" {

#if defined(ENABLE_JIT)
void disassemble(u1 *start, u1 *end);
#endif

#if defined(WITH_BINUTILS_DISASSEMBLER)
void disass_printf(PTR p, const char *fmt, ...);

int disass_buffer_read_memory(bfd_vma memaddr, bfd_byte *myaddr, unsigned int length, struct disassemble_info *info);
#endif

/* machine dependent functions */

#if defined(ENABLE_JIT)
u1 *disassinstr(u1 *code);
#endif

#if defined(ENABLE_INTRP)
u1 *intrp_disassinstr(u1 *code);
void intrp_disassemble(u1 *start, u1 *end);
#endif

} // extern "C"

#endif /* defined(ENABLE_DISASSEMBLER) */

#endif // DISASS_HPP_


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */
