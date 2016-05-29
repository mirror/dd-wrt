/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2010, 2012, 2013
 * Robert Lougher <rob@jamvm.org.uk>.
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

/* Macros for reading data values from class files - values
   are in big endian format, and non-aligned.  See arch.h
   for READ_DBL - this is platform dependent */

#define READ_U1(v,p,l)  v = *(p)++
#define READ_U2(v,p,l)  v = ((p)[0]<<8)|(p)[1]; (p)+=2
#define READ_U4(v,p,l)  v = ((p)[0]<<24)|((p)[1]<<16)|((p)[2]<<8)|(p)[3]; (p)+=4
#define READ_U8(v,p,l)  v = ((u8)(p)[0]<<56)|((u8)(p)[1]<<48)|((u8)(p)[2]<<40) \
                            |((u8)(p)[3]<<32)|((u8)(p)[4]<<24)|((u8)(p)[5]<<16) \
                            |((u8)(p)[6]<<8)|(u8)(p)[7]; (p)+=8
#define SKIP_U2(v,p,l)  (p)+=2

#define READ_INDEX(v,p,l)               READ_U2(v,p,l)
#define READ_TYPE_INDEX(v,cp,t,p,l)     READ_U2(v,p,l)

/* Hashtable entry for each package defined by the boot loader */
typedef struct package_entry {
    int index;
    char name[0];
} PackageEntry;

#define BOOTSTRAP_OFFSET(bootstrap_methods, idx)                         \
    (((int*)bootstrap_methods)[idx])

#define BOOTSTRAP_IDX_PNTR(bootstrap_methods, idx)                       \
    ((u2*)(bootstrap_methods + BOOTSTRAP_OFFSET(bootstrap_methods, idx)))

#define BOOTSTRAP_METHOD_REF(bootstrap_methods, idx)                     \
    (BOOTSTRAP_IDX_PNTR(bootstrap_methods, idx)[0])

#define BOOTSTRAP_METHOD_ARG(bootstrap_methods, idx, arg)                \
    (BOOTSTRAP_IDX_PNTR(bootstrap_methods, idx)[arg+1])

#define BOOTSTRAP_METHOD_ARG_COUNT(bootstrap_methods, idx)               \
    ((BOOTSTRAP_OFFSET(bootstrap_methods, idx+1) -                       \
      BOOTSTRAP_OFFSET(bootstrap_methods, idx))/sizeof(u2)-1)

#define BOOTSTRAP_METHODS_COUNT(bootstrap_methods)                       \
    (BOOTSTRAP_OFFSET(bootstrap_methods, 0)/sizeof(int)-1)

#define BOOTSTRAP_DATA_LEN(bootstrap_methods)                            \
    BOOTSTRAP_OFFSET(bootstrap_methods,                                  \
                     BOOTSTRAP_METHODS_COUNT(bootstrap_methods))


#define CLASS_EXTRA_ATTRIBUTES(class, name) ({           \
    ClassBlock *cb = CLASS_CB(class);                    \
    ExtraAttributes *attributes = cb->extra_attributes;  \
    attributes == NULL ? NULL : attributes->name;        \
})

#define INDEXED_ATTRIBUTE_DATA(attributes, name, index) ( \
    attributes == NULL || attributes->name == NULL ?      \
        NULL : attributes->name[index]                    \
)

#define METHOD_EXTRA_ATTRIBUTES(mb, name) ({                   \
    ClassBlock *cb = CLASS_CB(mb->class);                      \
    int index = mb - cb->methods;                              \
    INDEXED_ATTRIBUTE_DATA(cb->extra_attributes, name, index); \
})

#define FIELD_EXTRA_ATTRIBUTES(fb, name) ({                    \
    ClassBlock *cb = CLASS_CB(fb->class);                      \
    int index = fb - cb->fields;                               \
    INDEXED_ATTRIBUTE_DATA(cb->extra_attributes, name, index); \
})
