#ifndef CYGONCE_LOADER_ELF_H
#define CYGONCE_LOADER_ELF_H

//==========================================================================
//
//      elf.h
//
//      ELF file format definitions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           nickg
// Contributors:        nickg
// Date:                2000-11-15
// Purpose:             Define ELF file format
// Description:         The types defined here describe the ELF file format.
//              
// Usage:       #include <cyg/loader/elf.h>
//
//####DESCRIPTIONEND####
//
//==========================================================================
//
// Quite a lot of this file was taken from the BSD exec_elf.h header file.
// Hence we should show you this...
//
/*      $OpenBSD: exec_elf.h,v 1.20 1999/09/19 16:16:49 kstailey Exp $  */
/*
 * Copyright (c) 1995, 1996 Erik Theisen.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
//==========================================================================

#include <cyg/infra/cyg_type.h>

// -------------------------------------------------------------------------
// Basic types:

typedef cyg_uint32 Elf32_Addr;
typedef cyg_uint32 Elf32_Off;
typedef cyg_uint16 Elf32_Half;
typedef cyg_uint32 Elf32_Word;
typedef cyg_int32  Elf32_Sword;

typedef cyg_uint64 Elf64_Addr;
typedef cyg_uint64 Elf64_Off;
typedef cyg_uint16 Elf64_Half;
typedef cyg_uint32 Elf64_Word;
typedef cyg_int32  Elf64_Sword;
typedef cyg_uint64 Elf64_Xword;
typedef cyg_int64  Elf64_Sxword;

// -------------------------------------------------------------------------
// ELF header

#define EI_NIDENT 16

typedef struct {
    unsigned char   e_ident[EI_NIDENT];
    Elf32_Half      e_type;
    Elf32_Half      e_machine;
    Elf32_Word      e_version;
    Elf32_Addr      e_entry;
    Elf32_Off       e_phoff;
    Elf32_Off       e_shoff;
    Elf32_Word      e_flags;
    Elf32_Half      e_ehsize;
    Elf32_Half      e_phentsize;
    Elf32_Half      e_phnum;
    Elf32_Half      e_shentsize;
    Elf32_Half      e_shnum;
    Elf32_Half      e_shtrndx;
} Elf32_Ehdr;

typedef struct {
    unsigned char   e_ident[EI_NIDENT];
    Elf64_Half      e_type;
    Elf64_Half      e_machine;
    Elf64_Word      e_version;
    Elf64_Addr      e_entry;
    Elf64_Off       e_phoff;
    Elf64_Off       e_shoff;
    Elf64_Word      e_flags;
    Elf64_Half      e_ehsize;
    Elf64_Half      e_phentsize;
    Elf64_Half      e_phnum;
    Elf64_Half      e_shentsize;
    Elf64_Half      e_shnum;
    Elf64_Half      e_shtrndx;
} Elf64_Ehdr;

// -------------------------------------------------------------------------
/* e_ident[] identification indexes */

#define EI_MAG0         0               /* file ID */
#define EI_MAG1         1               /* file ID */
#define EI_MAG2         2               /* file ID */
#define EI_MAG3         3               /* file ID */
#define EI_CLASS        4               /* file class */
#define EI_DATA         5               /* data encoding */
#define EI_VERSION      6               /* ELF header version */
#define EI_OSABI        7               /* Operating system/ABI identification */
#define EI_ABIVERSION   8               /* ABI version */
#define EI_PAD          9               /* start of pad bytes */

// -------------------------------------------------------------------------
/* e_ident[] magic number */

#define ELFMAG0         0x7f            /* e_ident[EI_MAG0] */
#define ELFMAG1         'E'             /* e_ident[EI_MAG1] */
#define ELFMAG2         'L'             /* e_ident[EI_MAG2] */
#define ELFMAG3         'F'             /* e_ident[EI_MAG3] */
#define ELFMAG          "\177ELF"       /* magic */
#define SELFMAG         4               /* size of magic */

// -------------------------------------------------------------------------
/* e_ident[] file class */

#define ELFCLASSNONE    0               /* invalid */
#define ELFCLASS32      1               /* 32-bit objs */
#define ELFCLASS64      2               /* 64-bit objs */
#define ELFCLASSNUM     3               /* number of classes */

// -------------------------------------------------------------------------
/* e_ident[] data encoding */

#define ELFDATANONE     0               /* invalid */
#define ELFDATA2LSB     1               /* Little-Endian */
#define ELFDATA2MSB     2               /* Big-Endian */
#define ELFDATANUM      3               /* number of data encode defines */

// -------------------------------------------------------------------------
/* e_ident */

#define IS_ELF(ehdr) ((ehdr).e_ident[EI_MAG0] == ELFMAG0 && \
                      (ehdr).e_ident[EI_MAG1] == ELFMAG1 && \
                      (ehdr).e_ident[EI_MAG2] == ELFMAG2 && \
                      (ehdr).e_ident[EI_MAG3] == ELFMAG3)

// -------------------------------------------------------------------------
/* e_type */

#define ET_NONE         0               /* No file type */
#define ET_REL          1               /* relocatable file */
#define ET_EXEC         2               /* executable file */
#define ET_DYN          3               /* shared object file */
#define ET_CORE         4               /* core file */
#define ET_NUM          5               /* number of types */
#define ET_LOOS         0xfe00          /* Operating system-specific */
#define ET_HIOS         0xfeff          /* Operating system-specific */
#define ET_LOPROC       0xff00          /* reserved range for processor */
#define ET_HIPROC       0xffff          /*  specific e_type */

// -------------------------------------------------------------------------
/* e_machine */
// The following values taken from 22 June 2000 SysV ABI spec, updated with
// extra values from binutils elf/common.h.

#define EM_NONE                 0       // No machine
#define EM_M32                  1       // AT&T WE 32100
#define EM_SPARC                2       // SPARC
#define EM_386                  3       // Intel 80386
#define EM_68K                  4       // Motorola 68000
#define EM_88K                  5       // Motorola 88000
#define EM_860                  7       // Intel 80860
#define EM_MIPS                 8       // MIPS I Architecture
#define EM_S370                 9       // IBM System/370 Processor
#define EM_MIPS_RS3_LE          10      // MIPS RS3000 Little-endian
#define EM_PARISC               15      // Hewlett-Packard PA-RISC
#define EM_VPP500               17      // Fujitsu VPP500
#define EM_SPARC32PLUS          18      // Enhanced instruction set SPARC
#define EM_960                  19      // Intel 80960
#define EM_PPC                  20      // PowerPC
#define EM_PPC64                21      // 64-bit PowerPC
#define EM_V800                 36      // NEC V800
#define EM_FR20                 37      // Fujitsu FR20
#define EM_RH32                 38      // TRW RH-32
#define EM_RCE                  39      // Motorola RCE
#define EM_ARM                  40      // Advanced RISC Machines ARM
#define EM_ALPHA                41      // Digital Alpha
#define EM_SH                   42      // Hitachi SH
#define EM_SPARCV9              43      // SPARC Version 9
#define EM_TRICORE              44      // Siemens Tricore embedded processor
#define EM_ARC                  45      // Argonaut RISC Core, Argonaut Technologies Inc.
#define EM_H8_300               46      // Hitachi H8/300
#define EM_H8_300H              47      // Hitachi H8/300H
#define EM_H8S                  48      // Hitachi H8S
#define EM_H8_500               49      // Hitachi H8/500
#define EM_IA_64                50      // Intel IA-64 processor architecture
#define EM_MIPS_X               51      // Stanford MIPS-X
#define EM_COLDFIRE             52      // Motorola ColdFire
#define EM_68HC12               53      // Motorola M68HC12
#define EM_MMA                  54      // Fujitsu MMA Multimedia Accelerator
#define EM_PCP                  55      // Siemens PCP
#define EM_NCPU                 56      // Sony nCPU embedded RISC processor
#define EM_NDR1                 57      // Denso NDR1 microprocessor
#define EM_STARCORE             58      // Motorola Star*Core processor
#define EM_ME16                 59      // Toyota ME16 processor
#define EM_ST100                60      // STMicroelectronics ST100 processor
#define EM_TINYJ                61      // Advanced Logic Corp. TinyJ embedded processor family
#define EM_FX66                 66      // Siemens FX66 microcontroller
#define EM_ST9PLUS              67      // STMicroelectronics ST9+ 8/16 bit microcontroller
#define EM_ST7                  68      // STMicroelectronics ST7 8-bit microcontroller
#define EM_68HC16               69      // Motorola MC68HC16 Microcontroller
#define EM_68HC11               70      // Motorola MC68HC11 Microcontroller
#define EM_68HC08               71      // Motorola MC68HC08 Microcontroller
#define EM_68HC05               72      // Motorola MC68HC05 Microcontroller
#define EM_SVX                  73      // Silicon Graphics SVx
#define EM_ST19                 74      // STMicroelectronics ST19 8-bit microcontroller
#define EM_VAX                  75      // Digital VAX
#define EM_CRIS                 76      // Axis Communications 32-bit embedded processor
#define EM_JAVELIN              77      // Infineon Technologies 32-bit embedded processor
#define EM_FIREPATH             78      // Element 14 64-bit DSP Processor
#define EM_ZSP                  79      // LSI Logic 16-bit DSP Processor
#define EM_MMIX                 80      // Donald Knuth's educational 64-bit processor
#define EM_HUANY                81      // Harvard University machine-independent object files
#define EM_PRISM                82      // SiTera Prism

/* Cygnus PowerPC ELF backend.  Written in the absence of an ABI.  */
#define EM_CYGNUS_POWERPC 0x9025

/* Old version of Sparc v9, from before the ABI; this should be
   removed shortly.  */
#define EM_OLD_SPARCV9  11

/* Old version of PowerPC, this should be removed shortly. */
#define EM_PPC_OLD      17

/* Cygnus ARC ELF backend.  Written in the absence of an ABI.  */
#define EM_CYGNUS_ARC 0x9040

/* Cygnus M32R ELF backend.  Written in the absence of an ABI.  */
#define EM_CYGNUS_M32R 0x9041

/* Alpha backend magic number.  Written in the absence of an ABI.  */
//#define EM_ALPHA        0x9026

/* D10V backend magic number.  Written in the absence of an ABI.  */
#define EM_CYGNUS_D10V  0x7650

/* D30V backend magic number.  Written in the absence of an ABI.  */
#define EM_CYGNUS_D30V  0x7676

/* V850 backend magic number.  Written in the absense of an ABI.  */
#define EM_CYGNUS_V850  0x9080

/* mn10200 and mn10300 backend magic numbers.
   Written in the absense of an ABI.  */
#define EM_CYGNUS_MN10200       0xdead
#define EM_CYGNUS_MN10300       0xbeef

/* FR30 magic number - no EABI available.  */
#define EM_CYGNUS_FR30          0x3330

/* AVR magic number
   Written in the absense of an ABI.  */
#define EM_AVR                  0x1057

// -------------------------------------------------------------------------
/* Version */

#define EV_NONE         0               /* Invalid */
#define EV_CURRENT      1               /* Current */
#define EV_NUM          2               /* number of versions */

// -------------------------------------------------------------------------
/* Section Header */

typedef struct {
    Elf32_Word  sh_name;        /* name - index into section header
                                   string table section */
    Elf32_Word  sh_type;        /* type */
    Elf32_Word  sh_flags;       /* flags */
    Elf32_Addr  sh_addr;        /* address */
    Elf32_Off   sh_offset;      /* file offset */
    Elf32_Word  sh_size;        /* section size */
    Elf32_Word  sh_link;        /* section header table index link */
    Elf32_Word  sh_info;        /* extra information */
    Elf32_Word  sh_addralign;   /* address alignment */
    Elf32_Word  sh_entsize;     /* section entry size */
} Elf32_Shdr;

typedef struct {
    Elf64_Word  sh_name;        /* section name */
    Elf64_Word  sh_type;        /* section type */
    Elf64_Xword sh_flags;       /* section flags */
    Elf64_Addr  sh_addr;        /* virtual address */
    Elf64_Off   sh_offset;      /* file offset */
    Elf64_Xword sh_size;        /* section size */
    Elf64_Word  sh_link;        /* link to another */
    Elf64_Word  sh_info;        /* misc info */
    Elf64_Xword sh_addralign;   /* memory alignment */
    Elf64_Xword sh_entsize;     /* table entry size */
} Elf64_Shdr;

// -------------------------------------------------------------------------
/* Special Section Indexes */

#define SHN_UNDEF       0               /* undefined */
#define SHN_LORESERVE   0xff00          /* lower bounds of reserved indexes */
#define SHN_LOPROC      0xff00          /* reserved range for processor */
#define SHN_HIPROC      0xff1f          /*   specific section indexes */
#define SHN_LOOS        0xff20          /* reserved range for operating */
#define SHN_HIOS        0xff3f          /*   system specific section indexes */
#define SHN_ABS         0xfff1          /* absolute value */
#define SHN_COMMON      0xfff2          /* common symbol */
#define SHN_XINDEX      0xffff          /* escape value for oversize index */
#define SHN_HIRESERVE   0xffff          /* upper bounds of reserved indexes */

// -------------------------------------------------------------------------
/* sh_type */

#define SHT_NULL        0               /* inactive */
#define SHT_PROGBITS    1               /* program defined information */
#define SHT_SYMTAB      2               /* symbol table section */
#define SHT_STRTAB      3               /* string table section */
#define SHT_RELA        4               /* relocation section with addends*/
#define SHT_HASH        5               /* symbol hash table section */
#define SHT_DYNAMIC     6               /* dynamic section */
#define SHT_NOTE        7               /* note section */
#define SHT_NOBITS      8               /* no space section */
#define SHT_REL         9               /* relation section without addends */
#define SHT_SHLIB       10              /* reserved - purpose unknown */
#define SHT_DYNSYM      11              /* dynamic symbol table section */
#define SHT_INIT_ARRAY  14              /* init procedure array */
#define SHT_FINI_ARRAY  15              /* fini procedure array */
#define SHT_PREINIT_ARRAY 16            /* preinit procedure array */
#define SHT_GROUP       17              /* section group */
#define SHT_SYMTAB_SHNDX 18             /* oversize index table */
#define SHT_NUM         19              /* number of section types */
#define SHT_LOOS        0x60000000      /* reserved range for O/S */
#define SHT_HIOS        0x6fffffff      /*  specific section header types */
#define SHT_LOPROC      0x70000000      /* reserved range for processor */
#define SHT_HIPROC      0x7fffffff      /*  specific section header types */
#define SHT_LOUSER      0x80000000      /* reserved range for application */
#define SHT_HIUSER      0xffffffff      /*  specific indexes */

// -------------------------------------------------------------------------
/* Section names */

#define ELF_BSS         ".bss"          /* uninitialized data */
#define ELF_DATA        ".data"         /* initialized data */
#define ELF_DEBUG       ".debug"        /* debug */
#define ELF_DYNAMIC     ".dynamic"      /* dynamic linking information */
#define ELF_DYNSTR      ".dynstr"       /* dynamic string table */
#define ELF_DYNSYM      ".dynsym"       /* dynamic symbol table */
#define ELF_FINI        ".fini"         /* termination code */
#define ELF_GOT         ".got"          /* global offset table */
#define ELF_HASH        ".hash"         /* symbol hash table */
#define ELF_INIT        ".init"         /* initialization code */
#define ELF_REL_DATA    ".rel.data"     /* relocation data */
#define ELF_REL_FINI    ".rel.fini"     /* relocation termination code */
#define ELF_REL_INIT    ".rel.init"     /* relocation initialization code */
#define ELF_REL_DYN     ".rel.dyn"      /* relocaltion dynamic link info */
#define ELF_REL_RODATA  ".rel.rodata"   /* relocation read-only data */
#define ELF_REL_TEXT    ".rel.text"     /* relocation code */
#define ELF_RODATA      ".rodata"       /* read-only data */
#define ELF_SHSTRTAB    ".shstrtab"     /* section header string table */
#define ELF_STRTAB      ".strtab"       /* string table */
#define ELF_SYMTAB      ".symtab"       /* symbol table */
#define ELF_TEXT        ".text"         /* code */

// -------------------------------------------------------------------------
/* Section Attribute Flags - sh_flags */

#define SHF_WRITE               0x001           /* Writable */
#define SHF_ALLOC               0x002           /* occupies memory */
#define SHF_EXECINSTR           0x004           /* executable */
#define SHF_MERGE               0x010           /* merge data */
#define SHF_STRINGS             0x020           /* contains strings */
#define SHF_INFO_LINK           0x040           /* link in sh_info field */
#define SHF_LINK_ORDER          0x080           /* preserve link order */
#define SHF_OS_NONCONFORMING    0x100           /* special OS-specific */
                                                /*  processing needed */
#define SHF_GROUP               0x200           /* member of group */
#define SHF_MASKOS              0x0ff00000      /* reserved bits for OS */
                                                /*  specific section attributes */
#define SHF_MASKPROC            0xf0000000      /* reserved bits for processor */
                                                /*  specific section attributes */

// -------------------------------------------------------------------------
/* Symbol Table Entry */

typedef struct {
    Elf32_Word          st_name;        /* name - index into string table */
    Elf32_Addr          st_value;       /* symbol value */
    Elf32_Word          st_size;        /* symbol size */
    unsigned char       st_info;        /* type and binding */
    unsigned char       st_other;       /* visibility */
    Elf32_Half          st_shndx;       /* section header index */
} Elf32_Sym;

typedef struct {
    Elf64_Word          st_name;        /* Symbol name index in str table */
    unsigned char       st_info;        /* type / binding attrs */
    unsigned char       st_other;       /* visibility */
    Elf64_Half          st_shndx;       /* section index of symbol */
    Elf64_Addr          st_value;       /* value of symbol */
    Elf64_Xword          st_size;        /* size of symbol */
} Elf64_Sym;

// -------------------------------------------------------------------------
/* Symbol table index */

#define STN_UNDEF       0               /* undefined */

/* Extract symbol info - st_info */
#define ELF32_ST_BIND(x)        ((x) >> 4)
#define ELF32_ST_TYPE(x)        (((unsigned int) x) & 0xf)
#define ELF32_ST_INFO(b,t)      (((b) << 4) + ((t) & 0xf))

#define ELF64_ST_BIND(x)        ((x) >> 4)
#define ELF64_ST_TYPE(x)        (((unsigned int) x) & 0xf)
#define ELF64_ST_INFO(b,t)      (((b) << 4) + ((t) & 0xf))

#define ELF32_ST_VISIBILITY(o)  ((o)&0x3)
#define ELF64_ST_VISIBILITY(o)  ((o)&0x3)

// -------------------------------------------------------------------------
/* Symbol Binding - ELF32_ST_BIND - st_info */

#define STB_LOCAL       0               /* Local symbol */
#define STB_GLOBAL      1               /* Global symbol */
#define STB_WEAK        2               /* like global - lower precedence */
#define STB_NUM         3               /* number of symbol bindings */
#define STB_LOOS        10              /* reserved range for OS */
#define STB_HIOS        12              /*  specific symbol bindings */
#define STB_LOPROC      13              /* reserved range for processor */
#define STB_HIPROC      15              /*  specific symbol bindings */

// -------------------------------------------------------------------------
/* Symbol type - ELF32_ST_TYPE - st_info */

#define STT_NOTYPE      0               /* not specified */
#define STT_OBJECT      1               /* data object */
#define STT_FUNC        2               /* function */
#define STT_SECTION     3               /* section */
#define STT_FILE        4               /* file */
#define STT_COMMON      5               /* common block */
#define STT_NUM         6               /* number of symbol types */
#define STT_LOOS        10              /* reserved range for OS */
#define STT_HIOS        12              /*  specific symbol types */
#define STT_LOPROC      13              /* reserved range for processor */
#define STT_HIPROC      15              /*  specific symbol types */

// -------------------------------------------------------------------------
// symbol visibility in st_other

#define STV_DEFAULT     0               /* default to binding type */
#define STV_INTERNAL    1               /* processor specific */
#define STV_HIDDEN      2               /* invisible */
#define STV_PROTECTED   3               /* non-premptable */

// -------------------------------------------------------------------------
// 32 bit relocation records

/* Relocation entry with implicit addend */
typedef struct 
{
        Elf32_Addr      r_offset;       /* offset of relocation */
        Elf32_Word      r_info;         /* symbol table index and type */
} Elf32_Rel;

/* Relocation entry with explicit addend */
typedef struct 
{
        Elf32_Addr      r_offset;       /* offset of relocation */
        Elf32_Word      r_info;         /* symbol table index and type */
        Elf32_Sword     r_addend;
} Elf32_Rela;

/* Extract relocation info - r_info */
#define ELF32_R_SYM(i)          ((i) >> 8)
#define ELF32_R_TYPE(i)         ((unsigned char) (i))
#define ELF32_R_INFO(s,t)       (((s) << 8) + (unsigned char)(t))

// -------------------------------------------------------------------------
// 64 bit equivalents of above structures and macros.

typedef struct {
        Elf64_Addr      r_offset;       /* where to do it */
        Elf64_Xword     r_info;         /* index & type of relocation */
} Elf64_Rel;

typedef struct {
        Elf64_Addr      r_offset;       /* where to do it */
        Elf64_Xword     r_info;         /* index & type of relocation */
        Elf64_Sxword    r_addend;       /* adjustment value */
} Elf64_RelA;

#define ELF64_R_SYM(info)       ((info) >> 32)
#define ELF64_R_TYPE(info)      ((info) & 0xFFFFFFFF)
#define ELF64_R_INFO(s,t)       (((s) << 32) + (u_int32_t)(t))

// -------------------------------------------------------------------------
/* Program Header */

typedef struct {
    Elf32_Word  p_type;         /* segment type */
    Elf32_Off   p_offset;       /* segment offset */
    Elf32_Addr  p_vaddr;        /* virtual address of segment */
    Elf32_Addr  p_paddr;        /* physical address - ignored? */
    Elf32_Word  p_filesz;       /* number of bytes in file for seg. */
    Elf32_Word  p_memsz;        /* number of bytes in mem. for seg. */
    Elf32_Word  p_flags;        /* flags */
    Elf32_Word  p_align;        /* memory alignment */
} Elf32_Phdr;

typedef struct {
    Elf64_Word  p_type;         /* entry type */
    Elf64_Word  p_flags;        /* flags */
    Elf64_Off   p_offset;       /* offset */
    Elf64_Addr  p_vaddr;        /* virtual address */
    Elf64_Addr  p_paddr;        /* physical address */
    Elf64_Xword p_filesz;       /* file size */
    Elf64_Xword p_memsz;        /* memory size */
    Elf64_Xword p_align;        /* memory & file alignment */
} Elf64_Phdr;

// -------------------------------------------------------------------------
/* Segment types - p_type */

#define PT_NULL         0               /* unused */
#define PT_LOAD         1               /* loadable segment */
#define PT_DYNAMIC      2               /* dynamic linking section */
#define PT_INTERP       3               /* the RTLD */
#define PT_NOTE         4               /* auxiliary information */
#define PT_SHLIB        5               /* reserved - purpose undefined */
#define PT_PHDR         6               /* program header */
#define PT_NUM          7               /* Number of segment types */
#define PT_LOOS         0x60000000      /* reserved range for OS */
#define PT_HIOS         0x6fffffff      /*  specific segment types */
#define PT_LOPROC       0x70000000      /* reserved range for processor */
#define PT_HIPROC       0x7fffffff      /*  specific segment types */

// -------------------------------------------------------------------------
/* Segment flags - p_flags */

#define PF_X            0x1             /* Executable */
#define PF_W            0x2             /* Writable */
#define PF_R            0x4             /* Readable */
#define PF_MASKOS       0x0ff00000      /* reserved bits for OS */
                                        /*  specific segment flags */
#define PF_MASKPROC     0xf0000000      /* reserved bits for processor */
                                        /*  specific segment flags */

// -------------------------------------------------------------------------
/* Dynamic structure */

typedef struct {
    Elf32_Sword         d_tag;  /* controls meaning of d_val */
    union {
        Elf32_Word      d_val;  /* Multiple meanings - see d_tag */
        Elf32_Addr      d_ptr;  /* program virtual address */
    } d_un;
} Elf32_Dyn;

extern Elf32_Dyn        _DYNAMIC[];     /* XXX not 64-bit clean */

typedef struct {
    Elf64_Sxword        d_tag;  /* controls meaning of d_val */
    union {
        Elf64_Xword     d_val;
        Elf64_Addr      d_ptr;
    } d_un;
} Elf64_Dyn;

// -------------------------------------------------------------------------
/* Dynamic Array Tags - d_tag */

#define DT_NULL         0               /* marks end of _DYNAMIC array */
#define DT_NEEDED       1               /* string table offset of needed lib */
#define DT_PLTRELSZ     2               /* size of relocation entries in PLT */
#define DT_PLTGOT       3               /* address PLT/GOT */
#define DT_HASH         4               /* address of symbol hash table */
#define DT_STRTAB       5               /* address of string table */
#define DT_SYMTAB       6               /* address of symbol table */
#define DT_RELA         7               /* address of relocation table */
#define DT_RELASZ       8               /* size of relocation table */
#define DT_RELAENT      9               /* size of relocation entry */
#define DT_STRSZ        10              /* size of string table */
#define DT_SYMENT       11              /* size of symbol table entry */
#define DT_INIT         12              /* address of initialization func. */
#define DT_FINI         13              /* address of termination function */
#define DT_SONAME       14              /* string table offset of shared obj */
#define DT_RPATH        15              /* string table offset of library
                                           search path */
#define DT_SYMBOLIC     16              /* start sym search in shared obj. */
#define DT_REL          17              /* address of rel. tbl. w addends */
#define DT_RELSZ        18              /* size of DT_REL relocation table */
#define DT_RELENT       19              /* size of DT_REL relocation entry */
#define DT_PLTREL       20              /* PLT referenced relocation entry */
#define DT_DEBUG        21              /* bugger */
#define DT_TEXTREL      22              /* Allow rel. mod. to unwritable seg */
#define DT_JMPREL       23              /* add. of PLT's relocation entries */
#define DT_BIND_NOW     24              /* Bind now regardless of env setting */
#define DT_INIT_ARRAY   25              /* init array address */
#define DT_FINI_ARRAY   26              /* fini array address */
#define DT_INIT_ARRAYSZ 27              /* init array size */
#define DT_FINI_ARRAYSZ 28              /* fini array size */
#define DT_RUNPATH      29              /* library search path */
#define DT_FLAGS        30              /* flags */
#define DT_ENCODING     32              /* encoding rules start here */
#define DT_PREINIT_ARRAY 32             /* preinit array address */
#define DT_PREINIT_ARRAYSZ 33           /* preinit array size */
#define DT_NUM          26              /* Number used. */
#define DT_LOOS         0x60000000      /* reserved range for OS */
#define DT_HIOS         0x6fffffff      /*  specific dynamic array tags */
#define DT_LOPROC       0x70000000      /* reserved range for processor */
#define DT_HIPROC       0x7fffffff      /*  specific dynamic array tags */

// -------------------------------------------------------------------------
// Values for DT_FLAGS entry

#define DF_ORIGIN       0x1             /* Uses $ORIGIN substitution string */
#define DF_SYMBOLIC     0x2             /* search for symbols here first */
#define DF_TEXTREL      0x4             /* text may be relocatable */
#define DF_BIND_NOW     0x8             /* bind references now, dammit */

// -------------------------------------------------------------------------
/* Note Definitions */

typedef struct {
    Elf32_Word namesz;
    Elf32_Word descsz;
    Elf32_Word type;
} Elf32_Note;

typedef struct {
    Elf64_Word namesz;
    Elf64_Word descsz;
    Elf64_Word type;
} Elf64_Note;

// -------------------------------------------------------------------------
// Hash table structure
// The same structure is used by both 32 and 64 bit formats

typedef struct {
    Elf32_Word          nbucket;        /* number of buckets */
    Elf32_Word          nchain;         /* number of chains */

    /* The buckets follow this structure in memory and the chains
       follow those. */
} Elf_Hash;

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_LOADER_ELF_H
// EOF elf.h
