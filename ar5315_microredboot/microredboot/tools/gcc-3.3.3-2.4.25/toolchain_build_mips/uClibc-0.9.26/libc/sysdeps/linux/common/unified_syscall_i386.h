#undef _syscall0
#undef _syscall1
#undef _syscall2
#undef _syscall3
#undef _syscall4
#undef _syscall5

#include "str_syscalls.h"

#define unified_syscall_body(name) \
__asm__ ( \
".text\n.align 4\n.global "###name"\n.type "###name",@function\n" \
#name":\nmovb $"__STR_NR_##name \
",%al;\n jmp __uClibc_syscall\n.Lfe1"###name":\n.size "###name \
",.Lfe1"###name"-"###name \
)

#define _syscall0(type,name) \
unified_syscall_body(name)

#define _syscall1(type,name,type1,arg1) \
unified_syscall_body(name)

#define _syscall2(type,name,type1,arg1,type2,arg2) \
unified_syscall_body(name)

#define _syscall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
unified_syscall_body(name)

#define _syscall4(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
unified_syscall_body(name)

#define _syscall5(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5) \
unified_syscall_body(name)

