/* Accumulation of various pieces of knowledge about ELF.
   Copyright (C) 2000-2012, 2014, 2016 Red Hat, Inc.
   This file is part of elfutils.
   Written by Ulrich Drepper <drepper@redhat.com>, 2000.

   This file is free software; you can redistribute it and/or modify
   it under the terms of either

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at
       your option) any later version

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at
       your option) any later version

   or both in parallel, as here.

   elfutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see <http://www.gnu.org/licenses/>.  */

#ifndef _ELF_KNOWLEDGE_H
#define _ELF_KNOWLEDGE_H	1

#include <stdbool.h>


/* Test whether a section can be stripped or not.  */
#define SECTION_STRIP_P(shdr, name, remove_comment) \
  /* Sections which are allocated are not removed.  */			      \
  (((shdr)->sh_flags & SHF_ALLOC) == 0					      \
   /* We never remove .note sections.  */				      \
   && (shdr)->sh_type != SHT_NOTE					      \
   && (((shdr)->sh_type) != SHT_PROGBITS				      \
       /* Never remove .gnu.warning.* sections.  */			      \
       || (name != NULL							      \
	   && strncmp (name, ".gnu.warning.", sizeof ".gnu.warning." - 1) != 0\
	   /* We remove .comment sections only if explicitly told to do so. */\
	   && (remove_comment						      \
	       || strcmp (name, ".comment") != 0))))


/* Test whether `sh_info' field in section header contains a section
   index.  There are two kinds of sections doing this:

   - the sections containing relocation information reference in this
     field the section to which the relocations apply;

   - section with the SHF_INFO_LINK flag set to signal that `sh_info'
     references a section.  This allows correct handling of unknown
     sections.  */
#define SH_INFO_LINK_P(Shdr) \
  ((Shdr)->sh_type == SHT_REL || (Shdr)->sh_type == SHT_RELA		      \
   || ((Shdr)->sh_flags & SHF_INFO_LINK) != 0)


/* Size of an entry in the hash table.  The ELF specification says all
   entries are regardless of platform 32-bits in size.  Early 64-bit
   ports (namely Alpha for Linux) got this wrong.  The wording was not
   clear.

   Several years later the ABI for the 64-bit S390s was developed.
   Many things were copied from the IA-64 ABI (which uses the correct
   32-bit entry size) but it does get the SHT_HASH entry size wrong by
   using a 64-bit entry size.  So now we need this macro to special
   case both the alpha and s390x ABIs.  */
#define SH_ENTSIZE_HASH(Ehdr) \
  ((Ehdr)->e_machine == EM_ALPHA					      \
   || ((Ehdr)->e_machine == EM_S390					      \
       && (Ehdr)->e_ident[EI_CLASS] == ELFCLASS64) ? 8 : 4)

/* GNU Annobin notes are not fully standardized and abuses the owner name.  */

#define ELF_NOTE_GNU_BUILD_ATTRIBUTE_PREFIX "GA"

#define NT_GNU_BUILD_ATTRIBUTE_OPEN 0x100
#define NT_GNU_BUILD_ATTRIBUTE_FUNC 0x101

#define GNU_BUILD_ATTRIBUTE_TYPE_NUMERIC	'*'
#define GNU_BUILD_ATTRIBUTE_TYPE_STRING		'$'
#define GNU_BUILD_ATTRIBUTE_TYPE_BOOL_TRUE	'+'
#define GNU_BUILD_ATTRIBUTE_TYPE_BOOL_FALSE	'!'

#define GNU_BUILD_ATTRIBUTE_VERSION	1
#define GNU_BUILD_ATTRIBUTE_STACK_PROT	2
#define GNU_BUILD_ATTRIBUTE_RELRO	3
#define GNU_BUILD_ATTRIBUTE_STACK_SIZE	4
#define GNU_BUILD_ATTRIBUTE_TOOL	5
#define GNU_BUILD_ATTRIBUTE_ABI		6
#define GNU_BUILD_ATTRIBUTE_PIC		7
#define GNU_BUILD_ATTRIBUTE_SHORT_ENUM	8

/* Hexagon specific declarations.  */

/* Processor specific flags for the Ehdr e_flags field.  */
#define EF_HEXAGON_MACH_V2   0x00000001 /* Hexagon V2 */
#define EF_HEXAGON_MACH_V3   0x00000002 /* Hexagon V3 */
#define EF_HEXAGON_MACH_V4   0x00000003 /* Hexagon V4 */
#define EF_HEXAGON_MACH_V5   0x00000004 /* Hexagon V5 */
#define EF_HEXAGON_MACH_V55  0x00000005 /* Hexagon V55 */
#define EF_HEXAGON_MACH_V60  0x00000060 /* Hexagon V60 */
#define EF_HEXAGON_MACH_V61  0x00000061 /* Hexagon V61 */
#define EF_HEXAGON_MACH_V62  0x00000062 /* Hexagon V62 */
#define EF_HEXAGON_MACH_V65  0x00000065 /* Hexagon V65 */
#define EF_HEXAGON_MACH_V66  0x00000066 /* Hexagon V66 */
#define EF_HEXAGON_MACH_V67  0x00000067 /* Hexagon V67 */
#define EF_HEXAGON_MACH_V67T 0x00008067 /* Hexagon V67T */
#define EF_HEXAGON_MACH_V68  0x00000068 /* Hexagon V68 */
#define EF_HEXAGON_MACH_V69  0x00000069 /* Hexagon V68 */
#define EF_HEXAGON_MACH_V71  0x00000071 /* Hexagon V71 */
#define EF_HEXAGON_MACH_V71T 0x00008071 /* Hexagon V71T */
#define EF_HEXAGON_MACH_V73  0x00000073 /* Hexagon V73 */
#define EF_HEXAGON_MACH      0x000003ff /* Hexagon V.. */
#define EF_HEXAGON_TINY      0x00008000 /* Hexagon V..T */

/* Special section indices.  */
#define SHN_HEXAGON_SCOMMON    0xff00 /* Other access sizes */
#define SHN_HEXAGON_SCOMMON_1  0xff01 /* Byte-sized access */
#define SHN_HEXAGON_SCOMMON_2  0xff02 /* Half-word-sized access */
#define SHN_HEXAGON_SCOMMON_4  0xff03 /* Word-sized access */
#define SHN_HEXAGON_SCOMMON_8  0xff04 /* Double-word-size access */

/* Hexagon specific relocs.  */
#define R_HEX_NONE                0
#define R_HEX_B22_PCREL           1
#define R_HEX_B15_PCREL           2
#define R_HEX_B7_PCREL            3
#define R_HEX_LO16                4
#define R_HEX_HI16                5
#define R_HEX_32                  6
#define R_HEX_16                  7
#define R_HEX_8                   8
#define R_HEX_GPREL16_0           9
#define R_HEX_GPREL16_1           10
#define R_HEX_GPREL16_2           11
#define R_HEX_GPREL16_3           12
#define R_HEX_HL16                13
#define R_HEX_B13_PCREL           14
#define R_HEX_B9_PCREL            15
#define R_HEX_B32_PCREL_X         16
#define R_HEX_32_6_X              17
#define R_HEX_B22_PCREL_X         18
#define R_HEX_B15_PCREL_X         19
#define R_HEX_B13_PCREL_X         20
#define R_HEX_B9_PCREL_X          21
#define R_HEX_B7_PCREL_X          22
#define R_HEX_16_X                23
#define R_HEX_12_X                24
#define R_HEX_11_X                25
#define R_HEX_10_X                26
#define R_HEX_9_X                 27
#define R_HEX_8_X                 28
#define R_HEX_7_X                 29
#define R_HEX_6_X                 30
#define R_HEX_32_PCREL            31
#define R_HEX_COPY                32
#define R_HEX_GLOB_DAT            33
#define R_HEX_JMP_SLOT            34
#define R_HEX_RELATIVE            35
#define R_HEX_PLT_B22_PCREL       36
#define R_HEX_GOTREL_LO16         37
#define R_HEX_GOTREL_HI16         38
#define R_HEX_GOTREL_32           39
#define R_HEX_GOT_LO16            40
#define R_HEX_GOT_HI16            41
#define R_HEX_GOT_32              42
#define R_HEX_GOT_16              43
#define R_HEX_DTPMOD_32           44
#define R_HEX_DTPREL_LO16         45
#define R_HEX_DTPREL_HI16         46
#define R_HEX_DTPREL_32           47
#define R_HEX_DTPREL_16           48
#define R_HEX_GD_PLT_B22_PCREL    49
#define R_HEX_GD_GOT_LO16         50
#define R_HEX_GD_GOT_HI16         51
#define R_HEX_GD_GOT_32           52
#define R_HEX_GD_GOT_16           53
#define R_HEX_IE_LO16             54
#define R_HEX_IE_HI16             55
#define R_HEX_IE_32               56
#define R_HEX_IE_GOT_LO16         57
#define R_HEX_IE_GOT_HI16         58
#define R_HEX_IE_GOT_32           59
#define R_HEX_IE_GOT_16           60
#define R_HEX_TPREL_LO16          61
#define R_HEX_TPREL_HI16          62
#define R_HEX_TPREL_32            63
#define R_HEX_TPREL_16            64
#define R_HEX_6_PCREL_X           65
#define R_HEX_GOTREL_32_6_X       66
#define R_HEX_GOTREL_16_X         67
#define R_HEX_GOTREL_11_X         68
#define R_HEX_GOT_32_6_X          69
#define R_HEX_GOT_16_X            70
#define R_HEX_GOT_11_X            71
#define R_HEX_DTPREL_32_6_X       72
#define R_HEX_DTPREL_16_X         73
#define R_HEX_DTPREL_11_X         74
#define R_HEX_GD_GOT_32_6_X       75
#define R_HEX_GD_GOT_16_X         76
#define R_HEX_GD_GOT_11_X         77
#define R_HEX_IE_32_6_X           78
#define R_HEX_IE_16_X             79
#define R_HEX_IE_GOT_32_6_X       80
#define R_HEX_IE_GOT_16_X         81
#define R_HEX_IE_GOT_11_X         82
#define R_HEX_TPREL_32_6_X        83
#define R_HEX_TPREL_16_X          84
#define R_HEX_TPREL_11_X          85
#define R_HEX_LD_PLT_B22_PCREL    86
#define R_HEX_LD_GOT_LO16         87
#define R_HEX_LD_GOT_HI16         88
#define R_HEX_LD_GOT_32           89
#define R_HEX_LD_GOT_16           90
#define R_HEX_LD_GOT_32_6_X       91
#define R_HEX_LD_GOT_16_X         92
#define R_HEX_LD_GOT_11_X         93
#define R_HEX_23_REG              94
#define R_HEX_GD_PLT_B22_PCREL_X  95
#define R_HEX_GD_PLT_B32_PCREL_X  96
#define R_HEX_LD_PLT_B22_PCREL_X  97
#define R_HEX_LD_PLT_B32_PCREL_X  98
#define R_HEX_27_REG              99

#endif	/* elf-knowledge.h */
