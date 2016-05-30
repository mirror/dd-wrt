/* src/vm/jit/ir/bytecode.c - Java byte code handling

   Copyright (C) 2007
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


#include "config.h"

#include "vm/jit/ir/bytecode.hpp"


/* bytecodes ******************************************************************/

bytecode_t bytecode[256] = {
	{ 1, 0,   "nop"             },
	{ 1, 1,   "aconst_null"     },
	{ 1, 1,   "iconst_m1"       },
	{ 1, 1,   "iconst"          },
	{ 1, 1,   "iconst_1"        },
	{ 1, 1,   "iconst_2"        },
	{ 1, 1,   "iconst_3"        },
	{ 1, 1,   "iconst_4"        },
	{ 1, 1,   "iconst_5"        },
	{ 1, 1,   "lconst_0"        },
	{ 1, 1,   "lconst_1"        },
	{ 1, 1,   "fconst_0"        },
	{ 1, 1,   "fconst_1"        },
	{ 1, 1,   "fconst_2"        },
	{ 1, 1,   "dconst_0"        },
	{ 1, 1,   "dconst_1"        },
	{ 2, 1,   "bipush"          },
	{ 3, 1,   "sipush"          },
	{ 2, 1,   "ldc"             },
	{ 3, 1,   "ldc_w"           },
	{ 3, 1,   "ldc2_w"          },
	{ 2, 1,   "iload"           },
	{ 2, 1,   "lload"           },
	{ 2, 1,   "fload"           },
	{ 2, 1,   "dload"           },
	{ 2, 1,   "aload"           },
	{ 1, 1,   "iload_0"         },
	{ 1, 1,   "iload_1"         },
	{ 1, 1,   "iload_2"         },
	{ 1, 1,   "iload_3"         },
	{ 1, 1,   "lload_0"         },
	{ 1, 1,   "lload_1"         },
	{ 1, 1,   "lload_2"         },
	{ 1, 1,   "lload_3"         },
	{ 1, 1,   "fload_0"         },
	{ 1, 1,   "fload_1"         },
	{ 1, 1,   "fload_2"         },
	{ 1, 1,   "fload_3"         },
	{ 1, 1,   "dload_0"         },
	{ 1, 1,   "dload_1"         },
	{ 1, 1,   "dload_2"         },
	{ 1, 1,   "dload_3"         },
	{ 1, 1,   "aload_0"         },
	{ 1, 1,   "aload_1"         },
	{ 1, 1,   "aload_2"         },
	{ 1, 1,   "aload_3"         },
	{ 1, 1,   "iaload"          },
	{ 1, 1,   "laload"          },
	{ 1, 1,   "faload"          },
	{ 1, 1,   "daload"          },
	{ 1, 1,   "aaload"          },
	{ 1, 1,   "baload"          },
	{ 1, 1,   "caload"          },
	{ 1, 1,   "saload"          },
	{ 2, 0,   "istore"          },
	{ 2, 0,   "lstore"          },
	{ 2, 0,   "fstore"          },
	{ 2, 0,   "dstore"          },
	{ 2, 0,   "astore"          },
	{ 1, 0,   "istore_0"        },
	{ 1, 0,   "istore_1"        },
	{ 1, 0,   "istore_2"        },
	{ 1, 0,   "istore_3"        },
	{ 1, 0,   "lstore_0"        },
	{ 1, 0,   "lstore_1"        },
	{ 1, 0,   "lstore_2"        },
	{ 1, 0,   "lstore_3"        },
	{ 1, 0,   "fstore_0"        },
	{ 1, 0,   "fstore_1"        },
	{ 1, 0,   "fstore_2"        },
	{ 1, 0,   "fstore_3"        },
	{ 1, 0,   "dstore_0"        },
	{ 1, 0,   "dstore_1"        },
	{ 1, 0,   "dstore_2"        },
	{ 1, 0,   "dstore_3"        },
	{ 1, 0,   "astore_0"        },
	{ 1, 0,   "astore_1"        },
	{ 1, 0,   "astore_2"        },
	{ 1, 0,   "astore_3"        },
	{ 1, 0,   "iastore"         },
	{ 1, 0,   "lastore"         },
	{ 1, 0,   "fastore"         },
	{ 1, 0,   "dastore"         },
	{ 1, 0,   "aastore"         },
	{ 1, 0,   "bastore"         },
	{ 1, 0,   "castore"         },
	{ 1, 0,   "sastore"         },
	{ 1, 0,   "pop"             },
	{ 1, 0,   "pop2"            },
	{ 1, 1,   "dup"             },
	{ 1, 1+3, "dup_x1"          },
	{ 1, 2+4, "dup_x2"          },
	{ 1, 2,   "dup2"            },
	{ 1, 2+5, "dup2_x1"         },
	{ 1, 3+6, "dup2_x2"         },
	{ 1, 1+2, "swap"            },
	{ 1, 1,   "iadd"            },
	{ 1, 1,   "ladd"            },
	{ 1, 1,   "fadd"            },
	{ 1, 1,   "dadd"            },
	{ 1, 1,   "isub"            },
	{ 1, 1,   "lsub"            },
	{ 1, 1,   "fsub"            },
	{ 1, 1,   "dsub"            },
	{ 1, 1,   "imul"            },
	{ 1, 1,   "lmul"            },
	{ 1, 1,   "fmul"            },
	{ 1, 1,   "dmul"            },
	{ 1, 1,   "idiv"            },
	{ 1, 1,   "ldiv"            },
	{ 1, 1,   "fdiv"            },
	{ 1, 1,   "ddiv"            },
	{ 1, 1,   "irem"            },
	{ 1, 1,   "lrem"            },
	{ 1, 1,   "frem"            },
	{ 1, 1,   "drem"            },
	{ 1, 1,   "ineg"            },
	{ 1, 1,   "lneg"            },
	{ 1, 1,   "fneg"            },
	{ 1, 1,   "dneg"            },
	{ 1, 1,   "ishl"            },
	{ 1, 1,   "lshl"            },
	{ 1, 1,   "ishr"            },
	{ 1, 1,   "lshr"            },
	{ 1, 1,   "iushr"           },
	{ 1, 1,   "lushr"           },
	{ 1, 1,   "iand"            },
	{ 1, 1,   "land"            },
	{ 1, 1,   "ior"             },
	{ 1, 1,   "lor"             },
	{ 1, 1,   "ixor"            },
	{ 1, 1,   "lxor"            },
	{ 3, 0,   "iinc"            },
	{ 1, 1,   "i2l"             },
	{ 1, 1,   "i2f"             },
	{ 1, 1,   "i2d"             },
	{ 1, 1,   "l2i"             },
	{ 1, 1,   "l2f"             },
	{ 1, 1,   "l2d"             },
	{ 1, 1,   "f2i"             },
	{ 1, 1,   "f2l"             },
	{ 1, 1,   "f2d"             },
	{ 1, 1,   "d2i"             },
	{ 1, 1,   "d2l"             },
	{ 1, 1,   "d2f"             },
	{ 1, 1,   "int2byte"        },
	{ 1, 1,   "int2char"        },
	{ 1, 1,   "int2short"       },
	{ 1, 1,   "lcmp"            },
	{ 1, 1,   "fcmpl"           },
	{ 1, 1,   "fcmpg"           },
	{ 1, 1,   "dcmpl"           },
	{ 1, 1,   "dcmpg"           },
	{ 3, 0,   "ifeq"            },
	{ 3, 0,   "ifne"            },
	{ 3, 0,   "iflt"            },
	{ 3, 0,   "ifge"            },
	{ 3, 0,   "ifgt"            },
	{ 3, 0,   "ifle"            },
	{ 3, 0,   "if_icmpe"        },
	{ 3, 0,   "if_icmpne"       },
	{ 3, 0,   "if_icmplt"       },
	{ 3, 0,   "if_icmpge"       },
	{ 3, 0,   "if_icmpgt"       },
	{ 3, 0,   "if_icmple"       },
	{ 3, 0,   "if_acmpeq"       },
	{ 3, 0,   "if_acmpne"       },
	{ 3, 0,   "goto"            },
	{ 3, 1,   "jsr"             },
	{ 2, 0,   "ret"             },
	{ 0, 0,   "tableswitch"     },
	{ 0, 0,   "lookupswitch"    },
	{ 1, 0,   "ireturn"         },
	{ 1, 0,   "lreturn"         },
	{ 1, 0,   "freturn"         },
	{ 1, 0,   "dreturn"         },
	{ 1, 0,   "areturn"         },
	{ 1, 0,   "return"          },
	{ 3, 1,   "getstatic"       },
	{ 3, 0,   "putstatic"       },
	{ 3, 1,   "getfield"        },
	{ 3, 0,   "putfield"        },
	{ 3, 1,   "invokevirtual"   },
	{ 3, 1,   "invokespecial"   },
	{ 3, 1,   "invokestatic"    },
	{ 5, 1,   "invokeinterface" },
	{ 1, 1,   "undef186"        }, /* XXX */
	{ 3, 1,   "new"             },
	{ 2, 1,   "newarray"        },
	{ 3, 1,   "anewarray"       },
	{ 1, 1,   "arraylength"     },
	{ 1, 1,   "athrow"          },
	{ 3, 1,   "checkcast"       },
	{ 3, 1,   "instanceof"      },
	{ 1, 0,   "monitorenter"    },
	{ 1, 0,   "monitorexit"     },
	{ 0, 0,   "wide"            },
	{ 4, 1,   "multianewarray"  },
	{ 3, 0,   "ifnull"          },
	{ 3, 0,   "ifnonnull"       },
	{ 5, 0,   "goto_w"          },
	{ 5, 1,   "jsr_w"           },
	{ 1, 0,   "breakpoint"      },
};


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
 * vim:noexpandtab:sw=4:ts=4:
 */
