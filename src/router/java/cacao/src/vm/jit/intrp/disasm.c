/* vm disassembler wrapper

  Copyright (C) 2001,2002,2003 Free Software Foundation, Inc.

  This file is part of Gforth.

  Gforth is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
*/

#include "intrp.h"

#define IP (ip+1)
#define IPTOS IP[0]

#define STATIC_PROFILING 0

#if STATIC_PROFILING
#define add_inst(_b,_s) fputs(_s,vm_out)

/* for purely static "profiles" for selecting static superinstructions */
Inst *vm_disassemble_inst(Inst *ip, Inst vm_prim[])
{
#define return do {fputs(" \n             0\t",vm_out); goto _endif2_; } while (0)
#include <java-profile.i>
#undef return
  /* else */
  {
    add_inst(b,"unknown");
    ip++;
  }
 _endif_:
  fputc(' ',vm_out);
 _endif2_:
  return ip;
}
#else
Inst *vm_disassemble_inst(Inst *ip, Inst vm_prim[])
{
  fprintf(vm_out,"%p: ",(void *)ip);
#include <java-disasm.i>
  {
    fprintf(vm_out,"unknown instruction %p",ip[0]);
    ip++;
  }
 _endif_:
  fputc('\n',vm_out);
  return ip;
}
#endif


void vm_disassemble(Inst *ip, Inst *endp, Inst vm_prim[])
{
#if STATIC_PROFILING
  fputs("             0\t",vm_out);
#endif
  while (ip<endp) {
    ip = vm_disassemble_inst(ip, vm_prim);
  }
#if STATIC_PROFILING
  fputc('\n',vm_out);
#endif
}
