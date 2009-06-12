#define TARGET_CPU_DEFAULT (MASK_GAS)
#include "auto-host.h"
#ifdef IN_GCC
/* Provide three core typedefs used by everything, if we are compiling
   GCC.  These used to be found in rtl.h and tree.h, but this is no
   longer practical.  Providing these here rather that system.h allows
   the typedefs to be used everywhere within GCC. */
struct rtx_def;
typedef struct rtx_def *rtx;
struct rtvec_def;
typedef struct rtvec_def *rtvec;
union tree_node;
typedef union tree_node *tree;
#endif
#define GTY(x)
#ifdef IN_GCC
# include "ansidecl.h"
# include "dbxelf.h"
# include "elfos.h"
# include "svr4.h"
# include "linux.h"
# include "mips/mips.h"
# include "mips/linux.h"
# include "defaults.h"
#endif
#ifndef POSIX
# define POSIX
#endif
#ifndef GENERATOR_FILE
# include "insn-constants.h"
# include "insn-flags.h"
#endif
