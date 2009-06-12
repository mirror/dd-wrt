#ifndef CYGONCE_LOADER_LOADER_HXX
#define CYGONCE_LOADER_LOADER_HXX

//==========================================================================
//
//      loader.hxx
//
//      ELF dynamic loader definitions
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
// Purpose:             Define ELF dynamic loader
// Description: The classes defined here collectively implement the
//              internal API of the ELF dynamic loader.
// Usage:       #include <cyg/loader/loader.hxx>
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>

#ifndef CYG_LOADER_DYNAMIC_LD

#include <cyg/loader/elf.h>           // ELF data structures

#ifdef __cplusplus

#include <cyg/kernel/ktypes.h>
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <cyg/infra/clist.hxx>

// -------------------------------------------------------------------------
// Forward definitions

class Cyg_LoaderStream;
class Cyg_LoaderMemBlock;
class Cyg_LoaderMemAlloc;
class Cyg_LoadObject_Base;
class Cyg_LoadObject_Proc;
class Cyg_LoadObject;
class Cyg_Loader;

// -------------------------------------------------------------------------
// Error codes

#define CYG_LOADERR_NOERROR             0       // No error
#define CYG_LOADERR_NOT_ELF             1       // Not ELF format file
#define CYG_LOADERR_INVALID_CLASS       2       // Not expected file class
#define CYG_LOADERR_INVALID_BYTEORDER   3       // Not expected byte order
#define CYG_LOADERR_INVALID_VERSION     4       // Not expected ELF version
#define CYG_LOADERR_INVALID_MACHINE     5       // Not expected machine type
#define CYG_LOADERR_NO_MEMORY           6       // No memory
#define CYG_LOADERR_EOF                 7       // End of input stream
#define CYG_LOADERR_SEEK                8       // Cannot seek to stream position
#define CYG_LOADERR_INVALID_RELOC       9       // Invalid or unexpected relocation
#define CYG_LOADERR_NO_HASHTABLE        10      // No hash table in ELF file
#define CYG_LOADERR_NO_SYMTAB           11      // No symbol table in ELF file
#define CYG_LOADERR_NO_STRTAB           12      // No srting table in ELF file
#define CYG_LOADERR_NO_SYMBOL           13      // Symbol not found

// -------------------------------------------------------------------------
// Value for undefined or otherwise invalid symbol addresses.

#define CYG_LOADER_NULLSYMADDR          0

// -------------------------------------------------------------------------
// Loader Stream base class.
// This defines the interface to data streams that are ELF dynamic executable
// files.

class Cyg_LoaderStream
{
public:

    Cyg_LoaderStream();                         // Constructor

    virtual ~Cyg_LoaderStream();                // Destructor


    // The following functions are virtual and are supplied by a
    // derived class to access to data stream.
    
    virtual cyg_code get_byte(CYG_BYTE *val); // Get a byte from the stream

    virtual cyg_code get_data(CYG_BYTE *addr, CYG_ADDRWORD size);

    virtual cyg_code seek(CYG_ADDRWORD pos);  // seek to given position

    // The following functions all make use of the virtual functions
    // to access the data stream.
    
    cyg_code get_word16(CYG_WORD16 *val);       // get a 16 bit value

    cyg_code get_word32(CYG_WORD32 *val);       // get a 32 bit value

    cyg_code get_word64(CYG_WORD64 *val);       // get a 64 bit value
};

// -------------------------------------------------------------------------
// Memory allocation object. All memory allocated by the Loader is described
// by one of these. 

struct Cyg_LoaderMemBlock
    : public Cyg_DNode_T<Cyg_LoaderMemBlock>
{
    void                *address;       // data address
    cyg_int32           size;           // block size
    cyg_int32           alignment;      // block alignment
    Cyg_LoaderMemAlloc  *mem;           // allocator used
    CYG_ADDRESS         actual_address; // allocator specific actual address for block
    CYG_WORD32          actual_size;    // allocator specific actual size for block

    // Convenience free() function
    void free();
};

// -------------------------------------------------------------------------
// Memory allocator base class
// This defines the interface to a memory allocator used by the loader.

class Cyg_LoaderMemAlloc
{

public:

    Cyg_LoaderMemAlloc();

    virtual ~Cyg_LoaderMemAlloc();

    // The following functions are virtual so that alternative memory
    // allocators may be implemented. The default behaviour of this
    // class is to use malloc/realloc/free to support these functions.

    // Allocate memory of the supplied size and alignment.
    // size      - size in bytes
    // alignment - alignment expressed as a power of 2
    //             defaults to 8 byte alignment
    virtual Cyg_LoaderMemBlock *alloc( cyg_int32 size,
                                       cyg_int32 alignment = 8);

    // Reallocate block
    // block    - block to reallocate
    // size     - new size in bytes
    // alignment - new alignment, -1 if old alignment is to be used.
    virtual Cyg_LoaderMemBlock *realloc( Cyg_LoaderMemBlock *block,
                                         cyg_int32 size,
                                         cyg_int32 alignment = -1);
    
    // Free a previously allocated memory segment.
    virtual void free( Cyg_LoaderMemBlock *block );

};

// -------------------------------------------------------------------------
// Cyg_LoaderMemBlock convenience free() function implementation.

inline void Cyg_LoaderMemBlock::free() { mem->free(this); };

// -------------------------------------------------------------------------
// A loaded object

class Cyg_LoadObject_Base
    : public Cyg_DNode_T<Cyg_LoadObject>
{
    friend class Cyg_Loader;
    
protected:    
    // The following fields come from the constructor arguments

    cyg_uint32          mode;           // mode flags
    
    Cyg_LoaderMemAlloc  *memalloc;      // Memory allocator

    
    // The following fields are derived from the ELF header

    Elf32_Word          e_type;         // File type

    Elf32_Addr          e_entry;        // Executable entry point


    
    // The following fields are derived from the PT_DYNAMIC segment in
    // the program header. These fields are named after the DT_ tags
    // that their values are derived from.
    
    Elf32_Word          flags;          // flags from DT_FLAGS

    Elf32_Word          soname;         // name of module, if defined
    
    Elf_Hash            *hash;          // address of hash table
    Elf32_Word          *bucket;        // derived bucket array address
    Elf32_Word          *chain;         // derived chain array address
    
    // String table
    unsigned char       *strtab;        // address of table
    Elf32_Word          strsize;        // size of table in bytes

    // Symbol table
    Elf32_Sym           *symtab;        // address of table
    Elf32_Word          syment;         // size of entry
    
    // PTL and GOT
    Elf32_Addr          pltgot;         // address of PLT and/or GOT

    // PLT relocation entries
    Elf32_Addr          jmprel;         // address of relocation table
    Elf32_Word          pltrelsz;       // size of jmprel table in bytes
    Elf32_Word          pltrel;         // type of table entries: DT_REL
                                        //   or DT_RELA

    // Relocation table with implicit addends
    Elf32_Rel           *rel;           // table address
    Elf32_Word          relsize;        // table size in bytes
    Elf32_Word          relent;         // size of entry

    // Relocation table with explicit addends
    Elf32_Rela          *rela;          // table address
    Elf32_Word          relasize;       // table size in bytes
    Elf32_Word          relaent;        // size of entry

    // Init and Fini support
    Elf32_Addr          init;           // address of init function
    Elf32_Addr          fini;           // address of fini function
    Elf32_Addr          *init_array;    // address of init array
    Elf32_Word          init_array_sz;  // size of init array
    Elf32_Addr          *fini_array;    // address of fini array
    Elf32_Word          fini_array_sz;  // size of fini array
    Elf32_Addr          *pre_init_array;   // address of pre_init array
    Elf32_Word          pre_init_array_sz; // size of pre_init array

    Elf32_Dyn           *dynamic;       // address of _DYNAMIC section
    
    CYG_ADDRESS         base;           // base address used in address and
                                        // relocation calcualtions
    
    Cyg_CList_T<Cyg_LoaderMemBlock> segs;      // chain of loaded segments
    
    cyg_code            error;          // most recent error code


    // private functions

    void parse_dynamic( Elf32_Dyn *dynamic );

    // Translate a symbol from its symbol table index into its
    // actual address
    
    CYG_ADDRESS get_sym_addr_from_ix( Elf32_Word sym );

    // Get a symbol from this object's symbol table.
    Elf32_Sym *get_sym( Elf32_Word ix );

    // Get a name from this object's string table.
    char *get_name( Elf32_Word offset );

    // Look up the name in the hash table and return its symbol
    // table entry, or NULL if not found.
    Elf32_Sym *hash_lookup( const char *name );

    
    // Look up the name in the hash table and return its absolute
    // address or CYG_LOADER_NULLSYMADDR if it is not found.
    CYG_ADDRESS hash_lookup_addr( const char *name );

    // Standard ELF-defined hash function for symbol table.
    static unsigned long elf_hash( const unsigned char *name );
    
public:

    // Constructor - reads and allocates the executable.
    
    Cyg_LoadObject_Base();                        

    Cyg_LoadObject_Base( Cyg_LoaderStream& stream,
                         cyg_uint32 mode,
                         Cyg_LoaderMemAlloc *mem );
    
    ~Cyg_LoadObject_Base();                          // Destructor

    inline cyg_code get_error() { return error; };
    
    // Translate a symbol into its address.
    void *symbol( const char *name );

    // Start this executable running
    cyg_code exec(int argc, char **argv, char **envv);
};

#endif // __cplusplus

#endif // CYG_LOADER_DYNAMIC_LD

// -------------------------------------------------------------------------
// All the following files should have suitable ifdefs so that only
// one actually provides content.

#include <cyg/loader/mips_elf.h>        // MIPS ELF support
#include <cyg/loader/arm_elf.h>         // ARM ELF support
#include <cyg/loader/i386_elf.h>        // i386 ELF support
#include <cyg/loader/ppc_elf.h>         // PowerPC ELF support
//#include <cyg/loader/sparc_elf.h>       // Sparc ELF support
//#include <cyg/loader/sh_elf.h>          // SH ELF support

// -------------------------------------------------------------------------

#ifndef CYG_LOADER_DYNAMIC_LD

#ifdef __cplusplus

class Cyg_LoadObject :
    public Cyg_LoadObject_Proc
{
    Cyg_LoaderMemBlock  *block;
    
public:

    inline Cyg_LoadObject()
        : Cyg_LoadObject_Proc()
        {
        };

    inline Cyg_LoadObject( Cyg_LoaderStream& stream,
                    cyg_uint32 mode,
                    Cyg_LoaderMemAlloc *mem,
                    Cyg_LoaderMemBlock *ablock = NULL)
        : Cyg_LoadObject_Proc( stream, mode, mem )
        {
            block = ablock;
        };
    
    inline ~Cyg_LoadObject() {};

    Cyg_LoaderMemBlock *get_block() { return block; };
    
    // Apply dynamic relocations to the executable
    void relocate();

    // Apply PLT relocations
    void relocate_plt();
};

// -------------------------------------------------------------------------
// Main loader class

class Cyg_Loader
{
    // current error code.
    cyg_code            error;

    // Default memory allocator.
    Cyg_LoaderMemAlloc  *mem_default;

    // Load object for main program
    Cyg_LoadObject      *main;

    // List of loaded executables, including the main
    // program and all libraries.
    Cyg_CList_T<Cyg_LoadObject> loadlist;

public:

    Cyg_Loader( Cyg_LoaderMemAlloc *mem);       // Constructor

    ~Cyg_Loader();                              // Destructor


    // Load an object and all its dependencies.
    cyg_code load( Cyg_LoaderStream& stream,
                   cyg_uint32 mode,
                   Cyg_LoadObject **object);

    // Close an object and remove it from memory
    cyg_code close( Cyg_LoadObject *object );
    
    // Translate current error code into a string.
    const char *error_string( );

    // Look up a symbol
    Elf32_Sym *hash_lookup( const char *name );
    
    // look up a named symbol in loadlist and return its
    // address relocated to its load address.
    CYG_ADDRESS hash_lookup_addr( const char *name );
    
    // Static pointer to loader object
    static Cyg_Loader *loader;
};

//==========================================================================
// Memory based loader stream

class Cyg_LoaderStream_Mem
    : public Cyg_LoaderStream
{
    CYG_ADDRESS         base;
    CYG_ADDRESS         pos;
    CYG_ADDRESS         end;
    
public:
    
    Cyg_LoaderStream_Mem( const void *addr, cyg_int32 size );

    ~Cyg_LoaderStream_Mem();                // Destructor
    
    cyg_code get_byte(CYG_BYTE *val); // Get a byte from the stream

    cyg_code get_data(CYG_BYTE *addr, CYG_ADDRWORD size);

    cyg_code seek(CYG_ADDRWORD pos);  // seek to given position

};

//==========================================================================
// Fileio based loader stream

#ifdef CYGPKG_IO_FILEIO

class Cyg_LoaderStream_File
    : public Cyg_LoaderStream
{
    int         fd;
    
public:
    
    Cyg_LoaderStream_File( int afd );

    ~Cyg_LoaderStream_File();                // Destructor
    
    cyg_code get_byte(CYG_BYTE *val); // Get a byte from the stream

    cyg_code get_data(CYG_BYTE *addr, CYG_ADDRWORD size);

    cyg_code seek(CYG_ADDRWORD pos);  // seek to given position

};

#endif

// -------------------------------------------------------------------------

#endif // __cplusplus

#endif // CYG_LOADER_DYNAMIC_LD

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_LOADER_LOADER_HXX
// EOF loader.hxx
