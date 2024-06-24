/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
 * 2013, 2014 Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <inttypes.h>
#include <stdarg.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>

/* Configure options */
#include "config.h"

/* Architecture dependent definitions */
#include "arch.h"

/* Classlib definitions */
#include "classlib-defs.h"

#ifndef TRUE
#define         TRUE    1
#define         FALSE   0
#endif

/* These should go in the interpreter file */

#define OPC_NOP                           0
#define OPC_ACONST_NULL                   1
#define OPC_ICONST_M1                     2
#define OPC_ICONST_0                      3
#define OPC_ICONST_1                      4
#define OPC_ICONST_2                      5
#define OPC_ICONST_3                      6
#define OPC_ICONST_4                      7
#define OPC_ICONST_5                      8
#define OPC_LCONST_0                      9
#define OPC_LCONST_1                     10
#define OPC_FCONST_0                     11
#define OPC_FCONST_1                     12
#define OPC_FCONST_2                     13
#define OPC_DCONST_0                     14
#define OPC_DCONST_1                     15
#define OPC_BIPUSH                       16
#define OPC_SIPUSH                       17
#define OPC_LDC                          18
#define OPC_LDC_W                        19
#define OPC_LDC2_W                       20
#define OPC_ILOAD                        21
#define OPC_LLOAD                        22
#define OPC_FLOAD                        23
#define OPC_DLOAD                        24
#define OPC_ALOAD                        25
#define OPC_ILOAD_0                      26
#define OPC_ILOAD_1                      27
#define OPC_ILOAD_2                      28
#define OPC_ILOAD_3                      29
#define OPC_LLOAD_0                      30
#define OPC_LLOAD_1                      31
#define OPC_LLOAD_2                      32
#define OPC_LLOAD_3                      33
#define OPC_FLOAD_0                      34
#define OPC_FLOAD_1                      35
#define OPC_FLOAD_2                      36
#define OPC_FLOAD_3                      37
#define OPC_DLOAD_0                      38
#define OPC_DLOAD_1                      39
#define OPC_DLOAD_2                      40
#define OPC_DLOAD_3                      41
#define OPC_ALOAD_0                      42
#define OPC_ALOAD_1                      43
#define OPC_ALOAD_2                      44
#define OPC_ALOAD_3                      45
#define OPC_IALOAD                       46
#define OPC_LALOAD                       47
#define OPC_FALOAD                       48
#define OPC_DALOAD                       49
#define OPC_AALOAD                       50
#define OPC_BALOAD                       51
#define OPC_CALOAD                       52
#define OPC_SALOAD                       53
#define OPC_ISTORE                       54
#define OPC_LSTORE                       55
#define OPC_FSTORE                       56
#define OPC_DSTORE                       57
#define OPC_ASTORE                       58
#define OPC_ISTORE_0                     59
#define OPC_ISTORE_1                     60
#define OPC_ISTORE_2                     61
#define OPC_ISTORE_3                     62
#define OPC_LSTORE_0                     63
#define OPC_LSTORE_1                     64
#define OPC_LSTORE_2                     65
#define OPC_LSTORE_3                     66
#define OPC_FSTORE_0                     67
#define OPC_FSTORE_1                     68
#define OPC_FSTORE_2                     69
#define OPC_FSTORE_3                     70
#define OPC_DSTORE_0                     71
#define OPC_DSTORE_1                     72
#define OPC_DSTORE_2                     73
#define OPC_DSTORE_3                     74
#define OPC_ASTORE_0                     75
#define OPC_ASTORE_1                     76
#define OPC_ASTORE_2                     77
#define OPC_ASTORE_3                     78
#define OPC_IASTORE                      79
#define OPC_LASTORE                      80
#define OPC_FASTORE                      81
#define OPC_DASTORE                      82
#define OPC_AASTORE                      83
#define OPC_BASTORE                      84
#define OPC_CASTORE                      85
#define OPC_SASTORE                      86
#define OPC_POP                          87
#define OPC_POP2                         88
#define OPC_DUP                          89
#define OPC_DUP_X1                       90
#define OPC_DUP_X2                       91
#define OPC_DUP2                         92
#define OPC_DUP2_X1                      93
#define OPC_DUP2_X2                      94
#define OPC_SWAP                         95
#define OPC_IADD                         96
#define OPC_LADD                         97
#define OPC_FADD                         98
#define OPC_DADD                         99
#define OPC_ISUB                        100
#define OPC_LSUB                        101
#define OPC_FSUB                        102
#define OPC_DSUB                        103
#define OPC_IMUL                        104
#define OPC_LMUL                        105
#define OPC_FMUL                        106
#define OPC_DMUL                        107
#define OPC_IDIV                        108
#define OPC_LDIV                        109
#define OPC_FDIV                        110
#define OPC_DDIV                        111
#define OPC_IREM                        112
#define OPC_LREM                        113
#define OPC_FREM                        114
#define OPC_DREM                        115
#define OPC_INEG                        116
#define OPC_LNEG                        117
#define OPC_FNEG                        118
#define OPC_DNEG                        119
#define OPC_ISHL                        120
#define OPC_LSHL                        121
#define OPC_ISHR                        122
#define OPC_LSHR                        123
#define OPC_IUSHR                       124
#define OPC_LUSHR                       125
#define OPC_IAND                        126
#define OPC_LAND                        127
#define OPC_IOR                         128
#define OPC_LOR                         129
#define OPC_IXOR                        130
#define OPC_LXOR                        131
#define OPC_IINC                        132
#define OPC_I2L                         133
#define OPC_I2F                         134
#define OPC_I2D                         135
#define OPC_L2I                         136
#define OPC_L2F                         137
#define OPC_L2D                         138
#define OPC_F2I                         139
#define OPC_F2L                         140
#define OPC_F2D                         141
#define OPC_D2I                         142
#define OPC_D2L                         143
#define OPC_D2F                         144
#define OPC_I2B                         145
#define OPC_I2C                         146
#define OPC_I2S                         147
#define OPC_LCMP                        148
#define OPC_FCMPL                       149
#define OPC_FCMPG                       150
#define OPC_DCMPL                       151
#define OPC_DCMPG                       152
#define OPC_IFEQ                        153
#define OPC_IFNE                        154
#define OPC_IFLT                        155
#define OPC_IFGE                        156
#define OPC_IFGT                        157
#define OPC_IFLE                        158
#define OPC_IF_ICMPEQ                   159
#define OPC_IF_ICMPNE                   160
#define OPC_IF_ICMPLT                   161
#define OPC_IF_ICMPGE                   162
#define OPC_IF_ICMPGT                   163
#define OPC_IF_ICMPLE                   164
#define OPC_IF_ACMPEQ                   165
#define OPC_IF_ACMPNE                   166
#define OPC_GOTO                        167
#define OPC_JSR                         168
#define OPC_RET                         169
#define OPC_TABLESWITCH                 170
#define OPC_LOOKUPSWITCH                171
#define OPC_IRETURN                     172
#define OPC_LRETURN                     173
#define OPC_FRETURN                     174
#define OPC_DRETURN                     175
#define OPC_ARETURN                     176
#define OPC_RETURN                      177
#define OPC_GETSTATIC                   178
#define OPC_PUTSTATIC                   179
#define OPC_GETFIELD                    180
#define OPC_PUTFIELD                    181
#define OPC_INVOKEVIRTUAL               182
#define OPC_INVOKESPECIAL               183
#define OPC_INVOKESTATIC                184
#define OPC_INVOKEINTERFACE             185
#define OPC_INVOKEDYNAMIC               186
#define OPC_NEW                         187
#define OPC_NEWARRAY                    188
#define OPC_ANEWARRAY                   189
#define OPC_ARRAYLENGTH                 190
#define OPC_ATHROW                      191
#define OPC_CHECKCAST                   192
#define OPC_INSTANCEOF                  193
#define OPC_MONITORENTER                194
#define OPC_MONITOREXIT                 195
#define OPC_WIDE                        196
#define OPC_MULTIANEWARRAY              197
#define OPC_IFNULL                      198
#define OPC_IFNONNULL                   199
#define OPC_GOTO_W                      200
#define OPC_JSR_W                       201
#define OPC_LDC_QUICK                   203
#define OPC_LDC_W_QUICK                 204
#define OPC_GETFIELD_QUICK              206
#define OPC_PUTFIELD_QUICK              207
#define OPC_GETFIELD2_QUICK             208
#define OPC_PUTFIELD2_QUICK             209
#define OPC_GETSTATIC_QUICK             210
#define OPC_PUTSTATIC_QUICK             211
#define OPC_GETSTATIC2_QUICK            212
#define OPC_PUTSTATIC2_QUICK            213
#define OPC_INVOKEVIRTUAL_QUICK         214
#define OPC_INVOKENONVIRTUAL_QUICK      215
#define OPC_INVOKESUPER_QUICK           216
#define OPC_GETFIELD_QUICK_REF          217
#define OPC_PUTFIELD_QUICK_REF          218
#define OPC_GETSTATIC_QUICK_REF         219
#define OPC_PUTSTATIC_QUICK_REF         220
#define OPC_GETFIELD_THIS_REF           221
#define OPC_MIRANDA_BRIDGE              222
#define OPC_ABSTRACT_METHOD_ERROR       223
#define OPC_INLINE_REWRITER             224
#define OPC_PROFILE_REWRITER            225
#define OPC_INVOKEVIRTUAL_QUICK_W       226
#define OPC_GETFIELD_QUICK_W            227
#define OPC_PUTFIELD_QUICK_W            228
#define OPC_GETFIELD_THIS               229
#define OPC_LOCK                        230
#define OPC_ALOAD_THIS                  231
#define OPC_INVOKESTATIC_QUICK          232
#define OPC_NEW_QUICK                   233
#define OPC_ANEWARRAY_QUICK             235
#define OPC_CHECKCAST_QUICK             238
#define OPC_INSTANCEOF_QUICK            239
#define OPC_MULTIANEWARRAY_QUICK        243
#define OPC_INVOKEHANDLE                244
#define OPC_INVOKEBASIC                 245
#define OPC_LINKTOSPECIAL               246
#define OPC_LINKTOVIRTUAL               247
#define OPC_LINKTOINTERFACE             248
#define OPC_INVOKEINTERFACE_QUICK       249
#define OPC_INVOKEDYNAMIC_QUICK         250

/* Constant pool tags */

#define CONSTANT_Utf8                    1
#define CONSTANT_Integer                 3
#define CONSTANT_Float                   4
#define CONSTANT_Long                    5
#define CONSTANT_Double                  6
#define CONSTANT_Class                   7
#define CONSTANT_String                  8
#define CONSTANT_Fieldref                9
#define CONSTANT_Methodref              10
#define CONSTANT_InterfaceMethodref     11
#define CONSTANT_NameAndType            12

/* JSR 292 */
#define CONSTANT_MethodHandle           15
#define CONSTANT_MethodType             16
#define CONSTANT_InvokeDynamic          18

/* Internal */
#define CONSTANT_Locked                100
#define CONSTANT_Resolved              101
#define CONSTANT_ResolvedMethod        102
#define CONSTANT_ResolvedInvokeDynamic 103
#define CONSTANT_ResolvedClass         104
#define CONSTANT_ResolvedString        105
#define CONSTANT_ResolvedMethodType    106
#define CONSTANT_ResolvedMethodHandle  107
#define CONSTANT_ResolvedPolyMethod    108

#define ACC_PUBLIC              0x0001
#define ACC_PRIVATE             0x0002
#define ACC_PROTECTED           0x0004
#define ACC_STATIC              0x0008
#define ACC_FINAL               0x0010
#define ACC_SYNCHRONIZED        0x0020
#define ACC_SUPER               0x0020
#define ACC_VOLATILE            0x0040
#define ACC_BRIDGE              0x0040
#define ACC_VARARGS             0x0080
#define ACC_TRANSIENT           0x0080
#define ACC_NATIVE              0x0100
#define ACC_INTERFACE           0x0200
#define ACC_ABSTRACT            0x0400
#define ACC_STRICT              0x0800
#define ACC_SYNTHETIC           0x1000
#define ACC_ANNOTATION          0x2000
#define ACC_ENUM                0x4000
#define ACC_MIRANDA             0x8000
#define ACC_REFLECT_MASK        0xffff

#define T_BOOLEAN               4
#define T_CHAR                  5
#define T_FLOAT                 6
#define T_DOUBLE                7
#define T_BYTE                  8
#define T_SHORT                 9
#define T_INT                  10
#define T_LONG                 11

/* Class states */

#define CLASS_LOADED            1
#define CLASS_LINKED            2
#define CLASS_BAD               3
#define CLASS_INITING           4
#define CLASS_INITED            5

#define CLASS_ARRAY             6
#define CLASS_PRIM              7

/* Internal primitive type numbers */

#define PRIM_IDX_VOID           0
#define PRIM_IDX_BOOLEAN        1
#define PRIM_IDX_BYTE           2
#define PRIM_IDX_CHAR           3
#define PRIM_IDX_SHORT          4
#define PRIM_IDX_INT            5
#define PRIM_IDX_FLOAT          6
#define PRIM_IDX_LONG           7
#define PRIM_IDX_DOUBLE         8

/* Class flags */

#define CLASS_CLASS             1
#define REFERENCE               2
#define SOFT_REFERENCE          4
#define WEAK_REFERENCE          8
#define PHANTOM_REFERENCE      16
#define FINALIZED              32
#define CLASS_LOADER           64
#define CLASS_CLASH           128
#define ANONYMOUS             256

/* Method flags */

#define MB_LAMBDA_HIDDEN        1
#define MB_LAMBDA_COMPILED      2
#define MB_CALLER_SENSITIVE     4
#define MB_DEFAULT_CONFLICT     8

/* Method states (direct or inlining
   interpreter variants) */

#define MB_UNPREPARED           0
#define MB_PREPARING            1
#define MB_PREPARED             2

typedef unsigned char          u1;
typedef unsigned short         u2;
typedef unsigned int           u4;
typedef unsigned long long     u8;

typedef uintptr_t ConstantPoolEntry;

typedef struct constant_pool {
    volatile u1 *type;
    ConstantPoolEntry *info;
} ConstantPool;

typedef struct exception_table_entry {
    u2 start_pc;
    u2 end_pc;
    u2 handler_pc;
    u2 catch_type;
} ExceptionTableEntry;

typedef struct line_no_table_entry {
    u2 start_pc;
    u2 line_no;
} LineNoTableEntry;

typedef struct object Class;

typedef struct object {
   uintptr_t lock;
   Class *class;
} Object;

typedef struct attribute_data {
   u1 *data;
   int len;
} AttributeData;

typedef union extra_attributes {
    struct {
        AttributeData *class_annos;
        AttributeData **field_annos;
        AttributeData **method_annos;
        AttributeData **method_parameter_annos;
        AttributeData **method_anno_default_val;
#ifdef JSR308
        AttributeData *class_type_annos;
        AttributeData **field_type_annos;
        AttributeData **method_type_annos;
#endif
#ifdef JSR901
        AttributeData **method_parameters;
#endif
    };
    void *data[0];
} ExtraAttributes;

#ifdef DIRECT
typedef union ins_operand {
    uintptr_t u;
    int i;
    struct {
        signed short i1;
        signed short i2;
    } ii;
    struct {
        unsigned short u1;
        unsigned short u2;
    } uu;
    struct {
        unsigned short u1;
        unsigned char u2;
        char i;
    } uui;
    void *pntr;
} Operand;

typedef struct instruction {
#ifdef DIRECT_DEBUG
    unsigned char opcode;
    char cache_depth;
    short bytecode_pc;
#endif
    const void *handler;
    Operand operand;
} Instruction;

typedef struct switch_table {
    int low;
    int high;
    Instruction *deflt;
    Instruction **entries;
} SwitchTable;

typedef struct lookup_entry {
    int key;
    Instruction *handler;
} LookupEntry;

typedef struct lookup_table {
    int num_entries;
    Instruction *deflt;
    LookupEntry *entries;
} LookupTable;

#ifdef INLINING
typedef struct opcode_info {
    unsigned char opcode;
    unsigned char cache_depth;
} OpcodeInfo;

typedef struct profile_info ProfileInfo;

typedef struct basic_block {
    union {
        struct {
            int quickened;
            ProfileInfo *profiled;
        } profile;
        struct {
            char *addr;
            struct basic_block *next;
        } patch;
    } u;
    int length;
    Instruction *start;
    OpcodeInfo *opcodes;
    struct basic_block *prev;
    struct basic_block *next;
} BasicBlock;

typedef struct quick_prepare_info {
    BasicBlock *block;
    Instruction *quickened;
    struct quick_prepare_info *next;
} QuickPrepareInfo;

typedef struct prepare_info {
    BasicBlock *block;
    Operand operand;
} PrepareInfo;

struct profile_info {
    BasicBlock *block;
    int profile_count;
    const void *handler;
    struct profile_info *next;
    struct profile_info *prev;
};
#endif

typedef Instruction *CodePntr;
#else
typedef unsigned char *CodePntr;
#endif

typedef struct methodblock MethodBlock;
typedef uintptr_t *(*NativeMethod)(Class*, MethodBlock*, uintptr_t*);

struct methodblock {
   Class *class;
   char *name;
   char *type;
   char *signature;
   u1 state;
   u1 flags;
   u2 access_flags;
   u2 max_stack;
   u2 max_locals;
   u2 args_count;
   u2 throw_table_size;
   u2 *throw_table;
   void *code;
   int code_size;
   union {
       struct {
           u2 exception_table_size;
           u2 line_no_table_size;
           ExceptionTableEntry *exception_table;
           union {
               LineNoTableEntry *line_no_table;
               MethodBlock *miranda_mb;
           };
       };
       struct {
           union {
               struct {
                   char *simple_sig;
                   int native_extra_arg;
               };
               struct {
                   int ref_count;
                   int ret_slot_size;
               };
           };
           NativeMethod native_invoker;
       };
   };
   int method_table_index;
#ifdef INLINING
   QuickPrepareInfo *quick_prepare_info;
   ProfileInfo *profile_info;
#endif
};

typedef struct poly_methodblock  {
    char *name;
    char *type;
    Object *appendix;
    MethodBlock *invoker;
} PolyMethodBlock;

typedef struct invdyn_methodblock  {
#ifndef DIRECT
    int id;
#endif
    Object *appendix;
    MethodBlock *invoker;
    struct invdyn_methodblock *next;
} InvDynMethodBlock;

typedef struct resolved_inv_dyn_cp_entry {
    char *name;
    char *type;
    int boot_method_cp_idx;
    InvDynMethodBlock *idmb_list;
#ifndef DIRECT
    InvDynMethodBlock *cache;
#endif
} ResolvedInvDynCPEntry;

typedef struct fieldblock {
   Class *class;
   char *name;
   char *type;
   char *signature;
   u2 access_flags;
   u2 constant;
   union {
       union {
           char data[8];
           uintptr_t u;
           long long l;
           void *p;
           int i;
       } static_value; 
       u4 offset;
   } u;
} FieldBlock;

typedef struct itable_entry {
   Class *interface;
   int *offsets;
} ITableEntry;

typedef struct refs_offsets_entry {
    int start;
    int end;
} RefsOffsetsEntry;

typedef struct classblock {
   CLASSLIB_CLASS_PAD
   u1 state;
   u2 flags;
   u2 access_flags;
   u2 declaring_class;
   u2 enclosing_class;
   u2 enclosing_method;
   u2 inner_access_flags;
   u2 fields_count;
   u2 methods_count;
   u2 interfaces_count;
   u2 inner_class_count;
   u2 constant_pool_count;
   int object_size;
   int method_table_size;
   int imethod_table_size;
   int initing_tid;
   union {
       struct {
           int dim;
           Class *element_class;
           CLASSLIB_ARRAY_CLASS_EXTRA_FIELDS
       };
       struct {
           int refs_offsets_size;
           u2 *inner_classes;
           FieldBlock *fields;
           MethodBlock *methods;
           RefsOffsetsEntry *refs_offsets_table;
       };
   };
   char *name;
   char *signature;
   char *source_file_name;
   Class *super;
   Class **interfaces;
   MethodBlock **method_table;
   ITableEntry *imethod_table;
   char *bootstrap_methods;
   ExtraAttributes *extra_attributes;
   ConstantPool constant_pool;
   CLASSLIB_CLASS_EXTRA_FIELDS
} ClassBlock;

typedef struct frame {
   CodePntr last_pc;
   uintptr_t *lvars;
   uintptr_t *ostack;
   MethodBlock *mb;
   struct frame *prev;
} Frame;

typedef struct jni_frame {
   Object **next_ref;
   Object **lrefs;
   uintptr_t *ostack;
   MethodBlock *mb;
   struct frame *prev;
} JNIFrame;

typedef struct exec_env {
    Object *exception;
    char *stack;
    char *stack_end;
    int stack_size;
    Frame *last_frame;
    Object *thread;
    char overflow;
} ExecEnv;

typedef struct prop {
    char *key;
    char *value;
} Property;

typedef struct InitArgs {
    int asyncgc;
    int verbosegc;
    int verbosedll;
    int verboseclass;

    int compact_specified; /* Whether compaction has been given on the
                              command line, and the value if it has */
    int do_compact;

    int trace_jni_sigs;

    char *classpath;

    char *bootpath;
    char *bootpath_a;
    char *bootpath_p;
    char *bootpath_c;
    char *bootpath_v;

    int java_stack;
    unsigned long min_heap;
    unsigned long max_heap;

    Property *commandline_props;
    int props_count;

    void *main_stack_base;

    /* JNI invocation API hooks */
    
    int (*vfprintf)(FILE *stream, const char *fmt, va_list ap);
    void (*exit)(int status);
    void (*abort)(void);

#ifdef INLINING
    unsigned int codemem;
    int replication_threshold;
    int profile_threshold;
    int branch_patching_dup;
    int branch_patching;
    int print_codestats;
    int join_blocks;
    int profiling;
#endif

#ifdef HAVE_PROFILE_STUBS
    int dump_stubs_profiles;
#endif
} InitArgs;

#define CLASS_CB(classRef)           ((ClassBlock*)(classRef+1))

#define INST_DATA(obj, type, offset) *(type*)&((char*)obj)[offset]
#define INST_BASE(obj, type)         ((type*)(obj+1))

#define ARRAY_DATA(arrayRef, type)   ((type*)(((uintptr_t*)(arrayRef+1))+1)) 
#define ARRAY_LEN(arrayRef)          *(uintptr_t*)(arrayRef+1)

#define IS_CLASS(object)             (object->class && IS_CLASS_CLASS( \
                                                  CLASS_CB(object->class)))

#define IS_INTERFACE(cb)             (cb->access_flags & ACC_INTERFACE)
#define IS_SYNTHETIC(cb)             (cb->access_flags & ACC_SYNTHETIC)
#define IS_ANNOTATION(cb)            (cb->access_flags & ACC_ANNOTATION)
#define IS_ENUM(cb)                  (cb->access_flags & ACC_ENUM)
#define IS_ARRAY(cb)                 (cb->state == CLASS_ARRAY)
#define IS_PRIMITIVE(cb)             (cb->state >= CLASS_PRIM)
 
#define IS_FINALIZED(cb)             (cb->flags & FINALIZED)
#define IS_REFERENCE(cb)             (cb->flags & REFERENCE)
#define IS_SOFT_REFERENCE(cb)        (cb->flags & SOFT_REFERENCE)
#define IS_WEAK_REFERENCE(cb)        (cb->flags & WEAK_REFERENCE)
#define IS_PHANTOM_REFERENCE(cb)     (cb->flags & PHANTOM_REFERENCE)
#define IS_CLASS_LOADER(cb)          (cb->flags & CLASS_LOADER)
#define IS_CLASS_DUP(cb)             (cb->flags & CLASS_CLASH)
#define IS_CLASS_CLASS(cb)           (cb->flags & CLASS_CLASS)
#define IS_ANONYMOUS(cb)             (cb->flags & ANONYMOUS)
#define IS_SPECIAL(cb)               (cb->flags & (CLASSLIB_CLASS_SPECIAL | \
                                                   CLASS_CLASS | REFERENCE | \
                                                   CLASS_LOADER))
#define IS_CLASSLIB_SPECIAL(cb)      (cb->flags & CLASSLIB_CLASS_SPECIAL)
#define IS_MEMBER(cb)                cb->declaring_class
#define IS_LOCAL(cb)                 (cb->enclosing_method && !IS_ANONYMOUS(cb))

/* Macros for accessing constant pool entries */

#define CP_TYPE(cp,i)                   cp->type[i]
#define CP_INFO(cp,i)                   cp->info[i]
#define CP_CLASS(cp,i)                  (u2)cp->info[i]
#define CP_STRING(cp,i)                 (u2)cp->info[i]
#define CP_METHOD_CLASS(cp,i)           (u2)cp->info[i]
#define CP_METHOD_NAME_TYPE(cp,i)       (u2)(cp->info[i]>>16)
#define CP_INTERFACE_CLASS(cp,i)        (u2)cp->info[i]
#define CP_INTERFACE_NAME_TYPE(cp,i)    (u2)(cp->info[i]>>16)
#define CP_FIELD_CLASS(cp,i)            (u2)cp->info[i]
#define CP_FIELD_NAME_TYPE(cp,i)        (u2)(cp->info[i]>>16)
#define CP_NAME_TYPE_NAME(cp,i)         (u2)cp->info[i]
#define CP_NAME_TYPE_TYPE(cp,i)         (u2)(cp->info[i]>>16)
#define CP_UTF8(cp,i)                   (char *)(cp->info[i])
#define CP_METHOD_TYPE(cp,i)            (u2)cp->info[i]
#define CP_METHOD_HANDLE_KIND(cp,i)     (u1)cp->info[i]
#define CP_METHOD_HANDLE_REF(cp,i)      (u2)(cp->info[i]>>16)
#define CP_INVDYN_BOOT_MTHD(cp,i)       (u2)cp->info[i]
#define CP_INVDYN_NAME_TYPE(cp,i)       (u2)(cp->info[i]>>16)

#define CP_INTEGER(cp,i)                (int)(cp->info[i])      
#define CP_FLOAT(cp,i)                  *((float *)&(cp->info[i]) + IS_BE64)
#define CP_LONG(cp,i)                   *(long long *)&(cp->info[i])
#define CP_DOUBLE(cp,i)                 *(double *)&(cp->info[i])

#define KB 1024
#define MB (KB*KB)

/* minimum allowable size of object heap specified on command line */
#define MIN_HEAP 4*KB

/* minimum allowable size of the Java stack specified on command line */
#define MIN_STACK 2*KB

/* minimum size of object heap used when size of physical memory
   is not available */
#ifndef DEFAULT_MIN_HEAP
#define DEFAULT_MIN_HEAP 16*MB
#endif

/* maximum size of object heap used when size of physical memory
   is not available */
#ifndef DEFAULT_MAX_HEAP
#define DEFAULT_MAX_HEAP 256*MB
#endif

/* the mimimum heap size is a ratio of the size of physical memory
   (when available) but it must be at least min min heap size */
#ifndef MIN_MIN_HEAP
#define MIN_MIN_HEAP 16*MB
#endif

/* the maximum heap size is a ratio of the size of physical memory
   (when available) but it can't be more than max max heap size */
#ifndef MAX_MAX_HEAP
#define MAX_MAX_HEAP 1024*MB
#endif

/* default size of the Java stack */
#define DEFAULT_STACK 256*KB

/* size of emergency area - big enough to create
   a StackOverflow exception */
#define STACK_RED_ZONE_SIZE 1*KB

#define JAVA_COMPAT_VERSION "1.5.0"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#define OPT_OK    0
#define OPT_ERROR 1
#define OPT_UNREC 2

#ifndef RMBARRIER
#define RMBARRIER() MBARRIER()
#endif

#ifndef WMBARRIER
#define WMBARRIER() MBARRIER()
#endif

/* --------------------- Function prototypes  --------------------------- */

/* Alloc */

extern int initialiseAlloc(InitArgs *args);
extern int initialiseGC(InitArgs *args);
extern Class *allocClass();
extern Object *allocObject(Class *class);
extern Object *allocTypeArray(int type, int size);
extern Object *allocObjectArray(Class *element_class, int size);
extern Object *allocArray(Class *class, int size, int el_size);
extern Object *allocMultiArray(Class *array_class, int dim, intptr_t *count);
extern Object *cloneObject(Object *ob);
extern uintptr_t getObjectHashcode(Object *ob);

extern void gc1();
extern void runFinalizers();

extern unsigned long freeHeapMem();
extern unsigned long totalHeapMem();
extern unsigned long maxHeapMem();

extern void *sysMalloc(int n);
extern void sysFree(void *ptr);
extern void *sysRealloc(void *ptr, int n);

extern void *gcMemMalloc(int n);
extern void gcMemFree(void *ptr);
extern void *gcMemRealloc(void *ptr, int n);

extern void registerStaticObjectRef(Object **ref);
extern void registerStaticObjectRefLocked(Object **ref, Object *obj);

#define registerStaticClassRef(ref) \
    registerStaticObjectRef(ref);

#define registerStaticClassRefLocked(ref, class) \
    registerStaticObjectRefLocked(ref, class);

extern void gcPendingFree(void *addr);

/* GC support */
extern void markRoot(Object *ob);
extern void markConservativeRoot(Object *ob);
extern void markObject(Object *ob, int mark);
extern void markJNIClearedWeakRef(Object *ob);
extern void markJNIGlobalRef(Object *ob);
extern int isMarkedJNIWeakGlobalRef(Object *ob);
extern int isObject(void *pntr);
extern int isMarked(Object *ob);
extern void threadReference(Object **ref);

/* Class */

extern Class *java_lang_Class;

extern Class *parseClass(char *classname, char *data, int offset, int len,
                          Object *class_loader);
extern Class *defineClass(char *classname, char *data, int offset, int len,
                          Object *class_loader);
extern void linkClass(Class *class);
extern Class *initClass(Class *class);
extern Class *findSystemClass(char *name);
extern Class *findSystemClass0(char *name);
extern Class *loadSystemClass(char *name);

extern Class *findPrimitiveClass(char name);
extern Class *findPrimitiveClassByName(char *name);
extern Class *findHashedClass(char *name, Object *loader);
extern Class *findClassFromClassLoader(char *name, Object *loader);
extern Class *findArrayClassFromClassLoader(char *name, Object *loader);

extern Object *getSystemClassLoader();

extern int bootClassPathSize();
extern char *getBootClassPathEntry(int index);
extern Object *bootClassPathResource(char *filename, int index);

#define findArrayClassFromClass(name, class) \
             findArrayClassFromClassLoader(name, CLASS_CB(class)->class_loader)

#define findArrayClass(name) findArrayClassFromClassLoader(name, NULL)

#define findClassFromClass(classname, class)                               \
    (CLASS_CB(class)->name == classname ? class :                          \
        findClassFromClassLoader(classname, CLASS_CB(class)->class_loader))

extern void freeClassLoaderData(Object *class_loader);
extern void freeClassData(Class *class);

extern char *getClassPath();
extern char *getBootClassPath();
extern char *getEndorsedDirs();

extern void markBootClasses();
extern void markLoaderClasses(Object *loader, int mark);
extern void threadBootClasses();
extern void threadLoaderClasses(Object *class_loader);
extern void newLibraryUnloader(Object *class_loader, void *entry);
extern int initialiseClassStage1(InitArgs *args);
extern int initialiseClassStage2();

extern Object *bootPackage(char *package_name);
extern Object *bootPackages();

extern int hideFieldFromGC(FieldBlock *hidden);

/* resolve */

extern FieldBlock *findField(Class *, char *, char *);
extern MethodBlock *findMethod(Class *class, char *methodname, char *type);
extern FieldBlock *lookupField(Class *, char *, char *);
extern MethodBlock *lookupMethod(Class *class, char *methodname, char *type);
extern MethodBlock *lookupInterfaceMethod(Class *class, char *methodname,
                                          char *type);
extern MethodBlock *lookupVirtualMethod(Object *ob, MethodBlock *mb);
extern Class *resolveClass(Class *class, int index, int check_access, int init);
extern MethodBlock *resolveMethod(Class *class, int index);
extern MethodBlock *resolveInterfaceMethod(Class *class, int index);
extern FieldBlock *resolveField(Class *class, int index);
extern uintptr_t resolveSingleConstant(Class *class, int index);
extern int peekIsFieldLong(Class *class, int index);

/* cast */

extern char implements(Class *class, Class *test);
extern char isSubClassOf(Class *class, Class *test);
extern char isInstanceOf(Class *class, Class *test);
extern char arrayStoreCheck(Class *class, Class *test);

/* execute */

extern void *executeMethodArgs(Object *ob, Class *class, MethodBlock *mb, ...);
extern void *executeMethodVaList(Object *ob, Class *class, MethodBlock *mb,
                                  va_list args);
extern void *executeMethodList(Object *ob, Class *class, MethodBlock *mb,
                               u8 *args);

#define executeMethod(ob, mb, args...) \
    executeMethodArgs(ob, ob->class, mb, ##args)

#define executeStaticMethod(clazz, mb, args...) \
    executeMethodArgs(NULL, clazz, mb, ##args)

/* excep */

extern Object *exceptionOccurred();
extern void signalChainedExceptionEnum(int excep_enum, char *excep_mess,
                                       Object *cause);
extern void signalChainedExceptionName(char *excep_name, char *excep_mess,
                                       Object *cause);
extern void signalChainedExceptionClass(Class *excep_class, char *excep_mess,
                                        Object *cause);
extern void setException(Object *excep);
extern void clearException();
extern void printException();
extern CodePntr findCatchBlock(Class *exception);
extern int mapPC2LineNo(MethodBlock *mb, CodePntr pc_pntr);
extern Object *stackTraceElements(Object *trace);
extern Object *stackTrace(ExecEnv *ee, int max_depth);
extern Object *stackTraceElement(MethodBlock *mb, CodePntr pc);
extern int initialiseException();

extern int countStackFrames(Frame *last, int max_depth);
extern Object *convertTrace2Elements(void **trace, int len);
extern void stackTrace2Buffer(Frame *last, void **data, int max_depth);

extern Object *convertStackTrace(Object *vmthrwble);
extern Object *setStackTrace0(ExecEnv *ee, int max_depth);
extern Object *exceptionEnumToException(int excep_enum);	

#define exceptionOccurred0(ee) \
    ee->exception

#define EXCEPTION(excep_name) \
    exceptionEnumToException(EXCEPTION_ENUM(excep_name))

#define signalException(excep_name, excep_mess) \
    signalChainedExceptionEnum(EXCEPTION_ENUM(excep_name), excep_mess, NULL)

#define signalChainedException(excep_name, excep_mess, cause) \
    signalChainedExceptionEnum(EXCEPTION_ENUM(excep_name), excep_mess, cause)

#define signalExceptionClass(excep_class, excep_mess) \
    signalChainedExceptionClass(excep_class, excep_mess, NULL)

#define setStackTrace() \
    setStackTrace0(getExecEnv(), INT_MAX)

/* interp */

extern uintptr_t *executeJava();
extern void shutdownInterpreter();
extern int initialiseInterpreter(InitArgs *args);

/* String */

extern Object *findInternedString(Object *string);
extern Object *createString(char *utf8);
extern Object *createStringFromUnicode(unsigned short *unicode, int len);
extern char *String2Cstr(Object *string);
extern char *String2Buff(Object *string, char *buff, int buff_len);
extern int getStringLen(Object *string);
extern unsigned short *getStringChars(Object *string);
extern Object *getStringCharsArray(Object *string);
extern int getStringUtf8Len(Object *string);
extern char *String2Utf8(Object *string);
extern char *StringRegion2Utf8(Object *string, int start, int len, char *utf8);
extern void freeInternedStrings();
extern void threadInternedStrings();
extern int initialiseString();

#define Cstr2String(cstr) createString(cstr)

/* Utf8 */

extern int utf8Len(char *utf8);
extern int utf8Hash(char *utf8);
extern int utf8Comp(char *utf81, char *utf82);
extern void convertUtf8(char *utf8, unsigned short *buff);
extern char *findHashedUtf8(char *string, int add_if_absent);
extern char *copyUtf8(char *string);
extern int utf8CharLen(unsigned short *unicode, int len);
extern char *unicode2Utf8(unsigned short *unicode, int len, char *utf8);
extern char *dots2Slash(char *utf8);
extern char *slash2Dots(char *utf8);
extern char *slash2DotsDup(char *utf8);
extern char *slash2DotsBuff(char *utf8, char *buff, int buff_len);
extern int initialiseUtf8();

#define findUtf8(string) \
    findHashedUtf8(string, FALSE)

#define newUtf8(string) \
    findHashedUtf8(string, TRUE)

/* Dll */

extern int resolveDll(char *name, Object *loader);
extern char *getDllPath();
extern char *getBootDllPath();
extern char *getDllName(char *name);
extern int initialiseDll(InitArgs *args);
extern uintptr_t *resolveNativeWrapper(Class *class, MethodBlock *mb,
                                       uintptr_t *ostack);
extern void unloaderUnloadDll(uintptr_t entry);
extern void unloadClassLoaderDlls(Object *loader);
extern void threadLiveClassLoaderDlls();
extern NativeMethod setJNIMethod(MethodBlock *mb, void *func);
extern void shutdownDll();

extern void *lookupLoadedDlls0(char *name, Object *loader);

/* OS */

extern char *nativeLibPath();
extern char *nativeLibError();
extern void *nativeLibOpen(char *path);
extern void nativeLibClose(void *handle);
extern char *nativeLibMapName(char *name);
extern void *nativeLibSym(void *handle, char *symbol);
extern void *nativeStackBase();
extern char *nativeJVMPath();
extern int nativeAvailableProcessors();
extern long long nativePhysicalMemory();

extern char *convertSig2Simple(char *sig);

/* Threading */

extern int initialiseThreadStage1(InitArgs *args);
extern int initialiseThreadStage2(InitArgs *args);
extern ExecEnv *getExecEnv();

extern void createJavaThread(Object *jThread, long long stack_size);
extern void mainThreadSetContextClassLoader(Object *loader);
extern void mainThreadWaitToExitVM();
extern void uncaughtException();
extern void exitVM(int status);
extern void scanThreads();

/* Monitors */

extern int initialiseMonitor();

/* JNI */

extern int initJNILrefs();
extern int initialiseJNI();
extern void *getJNIInterface();
extern void markJNIGlobalRefs();
extern void scanJNIWeakGlobalRefs();
extern void markJNIClearedWeakRefs();
extern Object *allocObjectClassCheck(Class *class);
extern int isSupportedJNIVersion(int version);
extern int isSupportedJNIVersion_1_1(int version);

/* properties */

extern void setProperty(Object *properties, char *key, char *value);
extern int initialiseProperties(InitArgs *args);
extern void addCommandLineProperties(Object *properties);
extern void addDefaultProperties(Object *properties);
extern char *getCommandLineProperty(char *key);
extern char *getExecutionEngineName();
extern char *getJavaHome();
extern char *getCwd();

/* access */

extern int initialiseAccess();
extern int checkClassAccess(Class *class1, Class *class2);
extern int checkMethodAccess(MethodBlock *mb, Class *class);
extern int checkFieldAccess(FieldBlock *fb, Class *class);

/* frame */

extern int initialiseFrame();
extern Object *getClassContext();
extern Class *getCallerClass(int depth);
extern Object *firstNonNullClassLoader();

/* native */

extern int initialiseNatives();
extern void copyarray(Object *src, int start1, Object *dest,
                      int start2, int length);

extern uintptr_t *unpark(Class *class, MethodBlock *mb, uintptr_t *ostack);
extern uintptr_t *park(Class *class, MethodBlock *mb, uintptr_t *ostack);
extern uintptr_t *putLong(Class *class, MethodBlock *mb, uintptr_t *ostack);
extern uintptr_t *getLong(Class *class, MethodBlock *mb, uintptr_t *ostack);
extern uintptr_t *putObject(Class *class, MethodBlock *mb, uintptr_t *ostack);

extern uintptr_t *objectFieldOffset(Class *class, MethodBlock *mb,
                                    uintptr_t *ostack);
extern uintptr_t *compareAndSwapInt(Class *class, MethodBlock *mb,
                                    uintptr_t *ostack);
extern uintptr_t *compareAndSwapLong(Class *class, MethodBlock *mb,
                                     uintptr_t *ostack);
extern uintptr_t *putOrderedInt(Class *class, MethodBlock *mb,
                                uintptr_t *ostack);
extern uintptr_t *putOrderedLong(Class *class, MethodBlock *mb,
                                 uintptr_t *ostack);
extern uintptr_t *putIntVolatile(Class *class, MethodBlock *mb,
                                 uintptr_t *ostack);
extern uintptr_t *getIntVolatile(Class *class, MethodBlock *mb,
                                 uintptr_t *ostack);
extern uintptr_t *getLongVolatile(Class *class, MethodBlock *mb,
                                  uintptr_t *ostack);
extern uintptr_t *compareAndSwapObject(Class *class, MethodBlock *mb,
                                       uintptr_t *ostack);
extern uintptr_t *putOrderedObject(Class *class, MethodBlock *mb,
                                   uintptr_t *ostack);
extern uintptr_t *putObjectVolatile(Class *class, MethodBlock *mb,
                                    uintptr_t *ostack);
extern uintptr_t *getObjectVolatile(Class *class, MethodBlock *mb,
                                    uintptr_t *ostack);
extern uintptr_t *arrayBaseOffset(Class *class, MethodBlock *mb,
                                  uintptr_t *ostack);
extern uintptr_t *arrayIndexScale(Class *class, MethodBlock *mb,
                                  uintptr_t *ostack);
extern uintptr_t *vmSupportsCS8(Class *class, MethodBlock *mb,
                                uintptr_t *ostack);

/* init */

extern int parseCommonOpts(char *string, InitArgs *args, int is_jni);
extern void optError(InitArgs *args, const char *fmt, ...);
extern void setDefaultInitArgs(InitArgs *args);
extern unsigned long parseMemValue(char *str);
extern int initVM(InitArgs *args);
extern int VMInitialising();

/* shutdown */

extern void shutdownVM();

/* hooks */

extern int initialiseHooks(InitArgs *args);
extern void jam_fprintf(FILE *stream, const char *fmt, ...);
extern void jamvm_exit(int status);

#define jam_printf(fmt, ...) jam_fprintf(stdout, fmt, ## __VA_ARGS__)

/* inlining */

extern void freeMethodInlinedInfo(MethodBlock *mb);
extern int  initialiseInlining(InitArgs *args);
extern void showRelocatability();
extern void shutdownInlining();

/* symbol */

extern int initialiseSymbol();

/* time */

extern void getTimeoutAbsolute(struct timespec *ts, long long millis,
                               long long nanos);
extern void getTimeoutRelative(struct timespec *ts, long long millis,
                               long long nanos);

/* sig */

extern int sigElement2Size(char element);
extern int sigArgsCount(char *sig);
