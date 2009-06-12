//==========================================================================
//
//      loader.cxx
//
//      Loader class implementation
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
// Date:                2000-11-03
// Purpose:             Loader class implementation
// Description:         This file contains the implementation of the ELF loader
//                      classes.
//              
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/isoinfra.h>

#include <cyg/kernel/ktypes.h>          // base kernel types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <string.h>

#include <cyg/loader/loader.hxx>        // our header

#if CYGINT_ISO_MALLOC
#include <stdlib.h>                     // for malloc() etc
#endif

// ----------------------------------------------------------------------------

#ifdef CYGPKG_LIBC_STRING

#define streq( a, b ) (strcmp(a,b) == 0)

#else

static int streq( const char *s1, const char *s2 )
{
    while( *s1 == *s2 && *s1 && *s2 ) s1++,s2++;

    return !(*s2-*s1);
}

#endif

// ----------------------------------------------------------------------------
// new operator to allow us to invoke constructors on previously allocated
// memory.

inline void *operator new(size_t size,  void *ptr) { return ptr; };


// =========================================================================
// Static objects

// Default memory allocator
static Cyg_LoaderMemAlloc memalloc;

// Loader object
static Cyg_Loader loader(&memalloc);

Cyg_Loader *Cyg_Loader::loader = &::loader;


// =========================================================================
// Main loader class members

// -------------------------------------------------------------------------
// Constructor

Cyg_Loader::Cyg_Loader( Cyg_LoaderMemAlloc *memalloc )
{
    CYG_REPORT_FUNCTION();

    error = 0;
    mem_default = memalloc;

    // build an object for the main program
    Cyg_LoaderMemBlock *obj = mem_default->alloc( sizeof(Cyg_LoadObject));
    
    main = new(obj->address) Cyg_LoadObject( );

    // Add to load list
    loadlist.add_head(main);
    
    error = main->get_error();

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Destructor

Cyg_Loader::~Cyg_Loader()
{
    CYG_REPORT_FUNCTION();

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Load an object and all its dependencies.

cyg_code Cyg_Loader::load( Cyg_LoaderStream& stream,
                           cyg_uint32 mode,
                           Cyg_LoadObject **object)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3XV( &stream, mode, object );

    cyg_code error = 0;

    Cyg_LoaderMemBlock *obj = mem_default->alloc( sizeof(Cyg_LoadObject));
    Cyg_LoadObject *pobj = NULL;
    
    pobj = new(obj->address) Cyg_LoadObject( stream, mode, mem_default, obj );

    error = pobj->get_error();

    if( error != 0 )
        goto finish;

    // Add this object to list before we do any relocations to make
    // the symbol lookups work.
    
    loadlist.add_tail(pobj);

    
    // The object is now loaded. We must now do any relocations.

    pobj->relocate();

    error = pobj->get_error();

    if( error != 0 )
        goto finish;

    // Handle PLT relocations if we are told to do so

// We always do this for now..    
//    if( mode & RTLD_NOW )
        pobj->relocate_plt();

    error = pobj->get_error();
    
 finish:
    
    if( error != 0 )
    {
        // remove object from list.
        loadlist.remove( pobj );
        
        pobj->~Cyg_LoadObject();
        mem_default->free( obj );
    }
    else
    {
        // return it from this function.
        *object = pobj;
    }
    
    CYG_REPORT_RETVAL(error);

    return error;
}

// -------------------------------------------------------------------------
// Close object and remove it from memory

cyg_code Cyg_Loader::close( Cyg_LoadObject *object )
{
    CYG_REPORT_FUNCTION();    

    cyg_code error = 0;

    Cyg_LoaderMemBlock *block = object->get_block();
    
    object->~Cyg_LoadObject();

    if( block )
        block->free();
    
    CYG_REPORT_RETVAL(error);

    return error;
}

// -------------------------------------------------------------------------
// Translate current error code into a string.

const char *Cyg_Loader::error_string( )
{
    CYG_REPORT_FUNCTION();

    char *ret = "";

    CYG_REPORT_RETVAL(ret);    
    return ret;
}

// -------------------------------------------------------------------------
// Look up a named symbol in loadlist

CYG_ADDRESS Cyg_Loader::hash_lookup_addr( const char *name )
{
    CYG_ADDRESS addr = 0;
    Cyg_LoadObject *object = loadlist.get_head();
   
    do
    {
        addr = object->hash_lookup_addr( name );

        if( addr != CYG_LOADER_NULLSYMADDR )
            break;
        
        object = object->get_next();
        
    } while( object != loadlist.get_head() );

    if( addr == CYG_LOADER_NULLSYMADDR )
        error = CYG_LOADERR_NO_SYMBOL;
    
    return addr;
}

// =========================================================================
// Loader Object class members

Cyg_LoadObject_Base::Cyg_LoadObject_Base()
{
    e_type = ET_EXEC;
    e_entry = 0;
    base = 0;
    
    dynamic = _DYNAMIC;

    parse_dynamic( dynamic );
}

// -------------------------------------------------------------------------
// Constructor - reads and allocates the executable.

Cyg_LoadObject_Base::Cyg_LoadObject_Base( Cyg_LoaderStream& stream,
                                cyg_uint32 amode,
                                Cyg_LoaderMemAlloc *mem )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3XV( &stream, mode, mem );

    Cyg_LoaderMemBlock *phblock = NULL;
    Cyg_LoaderMemBlock *block = NULL;
    Elf32_Phdr *phdr;
    Elf32_Phdr *dynhdr = NULL;
    cyg_uint32 memsize = 0;
    cyg_uint32 maxalign = 0;
    CYG_BYTE *memaddr;
    Elf32_Addr vaddr_low = 0x7FFFFFFF;
    Elf32_Addr vaddr_hi = 0;
    
    mode = amode;
    memalloc = mem;
    error = CYG_LOADERR_NOERROR;
    
    // OK, let's start by getting the ELF header...

    Elf32_Ehdr elf_hdr;
    
    error = stream.get_data( (CYG_BYTE *)&elf_hdr, sizeof( elf_hdr ) );

    if( error != 0 )
        goto finish;

    // Check that this is a valid ELF file and that the various header
    // fields match what we expect for our current architecture and
    // platform.

    if( !IS_ELF( elf_hdr ) )
        error = CYG_LOADERR_NOT_ELF;
    else if( elf_hdr.e_ident[EI_CLASS] != ELFCLASS32 )
        error = CYG_LOADERR_INVALID_CLASS;
#if CYG_BYTEORDER == CYG_LSBFIRST 
    else if( elf_hdr.e_ident[EI_DATA] != ELFDATA2LSB )
#else
    else if( elf_hdr.e_ident[EI_DATA] != ELFDATA2MSB )
#endif        
        error = CYG_LOADERR_INVALID_BYTEORDER;
    else if( elf_hdr.e_ident[EI_VERSION] != EV_CURRENT )
        error = CYG_LOADERR_INVALID_VERSION;
    else if( elf_hdr.e_machine != CYG_ELF_MACHINE )
        error = CYG_LOADERR_INVALID_MACHINE;
    else if( elf_hdr.e_version != EV_CURRENT )
        error = CYG_LOADERR_INVALID_VERSION;
    else if( elf_hdr.e_phentsize != sizeof(Elf32_Phdr) )
        error = CYG_LOADERR_INVALID_VERSION;

    if( error != 0 )
        goto finish;
    
    // OK that all seems in order, save some fields away for later.

    e_type = elf_hdr.e_type;
    e_entry = elf_hdr.e_entry;

    // Now we must read the program header and prepare to read the
    // object file into memory.

    error = stream.seek( elf_hdr.e_phoff );

    if( error != 0 )
        goto finish;


    // Allocate space for the header
    
    phblock = memalloc->alloc( elf_hdr.e_phentsize * elf_hdr.e_phnum );

    if( phblock == NULL )
    {
        error = CYG_LOADERR_NO_MEMORY;
        goto finish;
    }

    error = stream.get_data( (CYG_BYTE *)phblock->address, sizeof(Elf32_Phdr)*elf_hdr.e_phnum );

    if( error != 0 )
        goto finish;
    
    phdr = (Elf32_Phdr *)phblock->address;

    // Loop over the program headers, totalling the sizes of the the
    // PT_LOAD entries and saving a pointer to the PT_DYNAMIC entry
    // when we find it.
    // Since the segments must retain the same relationship to
    // eachother in memory that their virtual addresses do in the
    // headers, we determine the amount of memory needed by finding
    // the extent of the virtual addresses covered by the executable.
    
    for( int i = 0; i < elf_hdr.e_phnum; i++,phdr++ )
    {
        if( phdr->p_type == PT_DYNAMIC )
        {
            dynhdr = phdr;
            continue;
        }
        if( phdr->p_type != PT_LOAD )
            continue;

        if( phdr->p_vaddr < vaddr_low )
            vaddr_low = phdr->p_vaddr;

        if( (phdr->p_vaddr+phdr->p_memsz) > vaddr_hi )
            vaddr_hi = phdr->p_vaddr+phdr->p_memsz;

        if( phdr->p_align > maxalign )
            maxalign = phdr->p_align;        
    }

    // Calculate how much memory we need and allocate it
    memsize = vaddr_hi - vaddr_low;
    
    block = memalloc->alloc( memsize, maxalign );

    if( block == NULL )
    {
        error = CYG_LOADERR_NO_MEMORY;
        goto finish;
    }
    
    // Attach to segments list
    segs.add_tail( block );
    
    // Calculate the base address for this executable. This is the
    // difference between the actual address the executable is loaded
    // at and its lowest virtual address. This value must be added to
    // all addresses derived from the executable to relocate them into
    // the real memory space.

    base = (CYG_ADDRESS)block->address - vaddr_low;
        
    // Loop over the program headers again, this time loading them
    // into the memory segment we have allocated and clearing any
    // unused areas to zero.

    phdr = (Elf32_Phdr *)phblock->address;

    memaddr = (CYG_BYTE *)block->address;
    
    for( int i = 0; i < elf_hdr.e_phnum; i++,phdr++ )
    {
        if( phdr->p_type != PT_LOAD )
            continue;

        error = stream.seek( phdr->p_offset );

        if( error != 0 ) break;

        // Calculate the actual load address for this segment.
        CYG_BYTE *loadaddr = (CYG_BYTE *)(phdr->p_vaddr + base);
        
        error = stream.get_data( loadaddr, phdr->p_filesz );

        if( error != 0 ) break;

        // If the memory size is more than we got from the file, zero the remainder.
        
        if( phdr->p_filesz < phdr->p_memsz )
            memset( loadaddr+phdr->p_filesz,
                    0,
                    phdr->p_memsz-phdr->p_filesz );
    }


    dynamic = (Elf32_Dyn *)(dynhdr->p_vaddr + base);

    parse_dynamic( dynamic );
    
 finish:
    
    if( phblock != NULL )
        memalloc->free( phblock );

    CYG_REPORT_RETURN();    
}

// -------------------------------------------------------------------------
// Parse the dynamic segment 

void Cyg_LoadObject_Base::parse_dynamic( Elf32_Dyn *dynamic )
{
    CYG_REPORT_FUNCTION();

    flags = 0;
    
    for(;; dynamic++)
    {
        switch( dynamic->d_tag )
        {
        case DT_NULL:                   /* marks end of _DYNAMIC array */
            return;
        case DT_NEEDED:                 /* string table offset of needed lib */
            break;                      // ignore for now
        case DT_PLTRELSZ:               /* size of relocation entries in PLT */
            pltrelsz = dynamic->d_un.d_val;
            break;
        case DT_PLTGOT:                 /* address PLT/GOT */
            pltgot = dynamic->d_un.d_ptr + base;
            break;
        case DT_HASH:                   /* address of symbol hash table */
            hash = (Elf_Hash *)(dynamic->d_un.d_ptr + base);
            bucket = (Elf32_Word *)(hash+1);
            chain = bucket+hash->nbucket;
            break;
        case DT_STRTAB:                 /* address of string table */
            strtab = (unsigned char *)(dynamic->d_un.d_ptr + base);
            break;
        case DT_SYMTAB:                 /* address of symbol table */
            symtab = (Elf32_Sym *)(dynamic->d_un.d_ptr + base);
            break;
        case DT_RELA:                   /* address of relocation table */
            rela = (Elf32_Rela *)(dynamic->d_un.d_ptr + base);
            break;
        case DT_RELASZ:                 /* size of relocation table */
            relasize = dynamic->d_un.d_val;
            break;
        case DT_RELAENT:                /* size of relocation entry */
            relaent = dynamic->d_un.d_val;
            break;
        case DT_STRSZ:                  /* size of string table */
            strsize = dynamic->d_un.d_val;
            break;
        case DT_SYMENT:                 /* size of symbol table entry */
            syment = dynamic->d_un.d_val;
            break;
        case DT_INIT:                   /* address of initialization func. */
            init = dynamic->d_un.d_ptr + base;
            break;
        case DT_FINI:                   /* address of termination function */
            fini = dynamic->d_un.d_ptr + base;
            break;
        case DT_SONAME:                 /* string table offset of shared obj */
            soname = dynamic->d_un.d_val;
            break;
        case DT_SYMBOLIC:               /* start sym search in shared obj. */
            flags |= DF_SYMBOLIC;
            break;
        case DT_REL:                    /* address of rel. tbl. w addends */
            rel = (Elf32_Rel *)(dynamic->d_un.d_ptr + base);
            break;
        case DT_RELSZ:                  /* size of DT_REL relocation table */
            relsize = dynamic->d_un.d_val;
            break;
        case DT_RELENT:                 /* size of DT_REL relocation entry */
            relent = dynamic->d_un.d_val;
            break;
        case DT_PLTREL:                 /* PLT referenced relocation entry */
            pltrel = dynamic->d_un.d_val;
            break;
        case DT_DEBUG:                  /* Debug data */
            break;                      /* ignore for now */
        case DT_TEXTREL:                /* Allow rel. mod. to unwritable seg */
            flags |= DF_TEXTREL;
            break;
        case DT_JMPREL:                 /* add. of PLT's relocation entries */
            jmprel = dynamic->d_un.d_ptr + base;
            break;
        case DT_BIND_NOW:               /* Bind now regardless of env setting */
            flags |= DF_BIND_NOW;
            break;
        case DT_INIT_ARRAY:             /* init array address */
            init_array = (Elf32_Addr *)(dynamic->d_un.d_ptr + base);
            break;
        case DT_FINI_ARRAY:             /* fini array address */
            fini_array = (Elf32_Addr *)(dynamic->d_un.d_ptr + base);
            break;
        case DT_INIT_ARRAYSZ:           /* init array size */
            init_array_sz = dynamic->d_un.d_val;
            break;
        case DT_FINI_ARRAYSZ:           /* fini array size */
            fini_array_sz = dynamic->d_un.d_val;
            break;
        case DT_FLAGS:                  /* flags */
            flags |= dynamic->d_un.d_val;
            break;
        case DT_PREINIT_ARRAY:          /* preinit array address */
            pre_init_array = (Elf32_Addr *)(dynamic->d_un.d_ptr + base);
            break;
        case DT_PREINIT_ARRAYSZ:        /* preinit array size */
            pre_init_array_sz = dynamic->d_un.d_val;
            break;
            
        default:
            // handle format-specific entries
            break;
        }
    }
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Get the symbol name from the current object's symbol table, look it
// up in the hash tables of the loaded objects, and return its address.

CYG_ADDRESS Cyg_LoadObject_Base::get_sym_addr_from_ix( Elf32_Word ix )
{
    Elf32_Sym *sym = get_sym( ix );

    if( sym == NULL ) return 0;

    const char *name = get_name( sym->st_name );

    // If the symbol has local binding, we must look for
    // it in this object only.
    if( ELF32_ST_BIND(sym->st_info) == STB_LOCAL )
         return hash_lookup_addr( name );

    // Otherwise search the loaded objects in load order
    return Cyg_Loader::loader->hash_lookup_addr( name );

}

// -------------------------------------------------------------------------
// Lookup the name in our hash table and return the symbol table entry

Elf32_Sym *Cyg_LoadObject_Base::hash_lookup( const char *name )
{
    Elf32_Sym *ret = NULL;

    if( hash == NULL )
    {
        error = CYG_LOADERR_NO_HASHTABLE;
        return NULL;
    }

    error = CYG_LOADERR_NO_SYMBOL;
    
    Elf32_Word ix = elf_hash( (const unsigned char *)name );

    ix %= hash->nbucket;

    // get head of chain
    Elf32_Word iy = bucket[ ix ]; 
    
    while( iy != STN_UNDEF )
    {
        Elf32_Sym *sym = get_sym( iy );
        const char *sname = get_name( sym->st_name );

        if( streq( name, sname ) )
        {
            ret = sym;
            error = CYG_LOADERR_NOERROR;
            break;
        }

        iy = chain[ iy ];
    }

    return ret;
}

// -------------------------------------------------------------------------
// Lookup the given name in our symbol table and return it's value
// relocated to our load address.

CYG_ADDRESS Cyg_LoadObject_Base::hash_lookup_addr( const char *name )
{
    Elf32_Sym *sym = hash_lookup( name );

    if( sym == NULL )
        return CYG_LOADER_NULLSYMADDR;

    // Check that this symbol is for a defined object, if its type is
    // NOTYPE then it is undefined, here, and we cannot take its address.
    // Hopefully it is defined in some other object.
    
    if( ELF32_ST_TYPE(sym->st_info) == STT_NOTYPE )
    {
        error = CYG_LOADERR_NO_SYMBOL;
        return CYG_LOADER_NULLSYMADDR;
    }
    
    return sym->st_value + base;
}

// -------------------------------------------------------------------------
// ELF hash function
// This is the standard hash function used for indexing the bucket
// array in the hash table.

unsigned long Cyg_LoadObject_Base::elf_hash( const unsigned char *name )
{
    unsigned long h = 0, g;

    while( *name )
    {
        h = ( h << 4 ) + *name++;
        if( (g = h & 0xf0000000) != 0 )
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

// -------------------------------------------------------------------------
//

Elf32_Sym *Cyg_LoadObject_Base::get_sym( Elf32_Word ix )
{
    if( symtab == NULL )
        return NULL;
    
    return &symtab[ix];
}

char *Cyg_LoadObject_Base::get_name( Elf32_Word offset )
{
    if( strtab == NULL || offset > strsize )
        return NULL;
    
    return (char *)(&strtab[offset]);
}

// -------------------------------------------------------------------------
// Destructor

// -------------------------------------------------------------------------
// Cyg_LoadObject_Base destructor

Cyg_LoadObject_Base::~Cyg_LoadObject_Base()
{
    CYG_REPORT_FUNCTION();
    // empty out segments list
    while( !segs.empty() )
    {
        Cyg_LoaderMemBlock *block = segs.rem_head();

        block->free();
    }
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Translate a symbol into its address.

void *Cyg_LoadObject_Base::symbol( const char *name )
{
    CYG_REPORT_FUNCTION();

    Elf32_Addr addr = hash_lookup_addr( name );

    if( addr == CYG_LOADER_NULLSYMADDR )
        addr = 0;
    
    CYG_REPORT_RETVAL( addr );

    return (void *)addr;
}

// -------------------------------------------------------------------------
// Start the given executable object running

cyg_code Cyg_LoadObject_Base::exec(int argc, char **argv, char **envv)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3XV( argc, argv, envv );

    cyg_code error = 0;
    
    CYG_REPORT_RETVAL(error);

    return error;
}

// =========================================================================
// Cyg_LoadObject members

// -------------------------------------------------------------------------
// Apply relocations

void Cyg_LoadObject::relocate()
{
    CYG_REPORT_FUNCTION();

    if( rel != NULL )
    {
        Elf32_Rel *r = rel;
        for( int i = relsize; i > 0 && error == 0; i -= relent, r++ )
            error = apply_rel( ELF32_R_TYPE(r->r_info),
                               ELF32_R_SYM(r->r_info),
                               r->r_offset);
    }
        

    if( error == 0 && rela != NULL )
    {
        Elf32_Rela *r = rela;
        for( int i = relasize; i > 0 && error == 0; i -= relaent, r++ )
            error = apply_rela( ELF32_R_TYPE(r->r_info),
                                ELF32_R_SYM(r->r_info),
                                r->r_offset,
                                r->r_addend);
    }
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Apply JMPREL relocations for the PLT

void Cyg_LoadObject::relocate_plt()
{
    CYG_REPORT_FUNCTION();

    if( pltrel == DT_REL )
    {
        Elf32_Rel *r = (Elf32_Rel *)jmprel;
        for( int i = pltrelsz; i > 0 && error == 0; i -= sizeof(Elf32_Rel), r++ )
            error = apply_rel( ELF32_R_TYPE(r->r_info),
                               ELF32_R_SYM(r->r_info),
                               r->r_offset);
    }
        

    if( error == 0 && pltrel == DT_RELA )
    {
        Elf32_Rela *r = (Elf32_Rela *)jmprel;
        for( int i = pltrelsz; i > 0 && error == 0; i -= sizeof(Elf32_Rela), r++ )
            error = apply_rela( ELF32_R_TYPE(r->r_info),
                                ELF32_R_SYM(r->r_info),
                                r->r_offset,
                                r->r_addend);
    }
    
    CYG_REPORT_RETURN();
}

// =========================================================================
// Loader memory allocator default class methods.
// The default behaviour of this class is to use malloc/realloc/free
// to handle memory.

// -------------------------------------------------------------------------

Cyg_LoaderMemAlloc::Cyg_LoaderMemAlloc()
{
    // no initialization needed
}

// -------------------------------------------------------------------------

Cyg_LoaderMemAlloc::~Cyg_LoaderMemAlloc()
{
    // No destruction needed
}

// -------------------------------------------------------------------------
// Allocate memory of the supplied size and alignment.
// size      - size in bytes
// alignment - alignment expressed as a power of 2

Cyg_LoaderMemBlock *Cyg_LoaderMemAlloc::alloc( cyg_int32 size,
                                               cyg_int32 alignment)
{
#if CYGINT_ISO_MALLOC    
    Cyg_LoaderMemBlock *block;
    cyg_uint8 *mem;
    cyg_uint32 acsize = sizeof(Cyg_LoaderMemBlock) + size + alignment;

    mem = (cyg_uint8 *)::malloc( acsize );

    if( mem == NULL )
        return NULL;

    block = (Cyg_LoaderMemBlock *)mem;

    // set up aligned block address
    block->address      = (void *)((((CYG_ADDRWORD)mem+sizeof(Cyg_LoaderMemBlock))+alignment) & ~(alignment-1));
    block->size         = size;
    block->alignment    = alignment;
    block->mem          = this;
    block->actual_address = (CYG_ADDRESS) mem;
    block->actual_size    = acsize;

    return block;

#else

    return 0;

#endif
    
}

// -------------------------------------------------------------------------
// Reallocate block

Cyg_LoaderMemBlock *Cyg_LoaderMemAlloc::realloc( Cyg_LoaderMemBlock *oblock,
                                                 cyg_int32 size,
                                                 cyg_int32 alignment)
{
#if CYGINT_ISO_MALLOC
    
    Cyg_LoaderMemBlock *block;
    cyg_uint8 *mem;

    if( alignment == -1 )
        alignment = oblock->alignment;
    
    cyg_uint32 acsize = sizeof(Cyg_LoaderMemBlock) + size + alignment;

    mem = (cyg_uint8 *)::realloc( (void *)(oblock->actual_address), acsize );

    if( mem == NULL )
        return NULL;

    block = (Cyg_LoaderMemBlock *)mem;

    // set up aligned block address
    block->address      = (void *)((((CYG_ADDRWORD)mem+sizeof(Cyg_LoaderMemBlock))+alignment) & (alignment-1));
    block->size         = size;
    block->alignment    = alignment;
    block->mem          = this;
    block->actual_address = (CYG_ADDRESS) mem;
    block->actual_size    = acsize;

    return block;

#else

    return NULL;

#endif
}
    
// -------------------------------------------------------------------------
// Free a previously allocated memory segment.

void Cyg_LoaderMemAlloc::free( Cyg_LoaderMemBlock *block )
{
#if CYGINT_ISO_MALLOC        
    ::free( (void *)block->actual_address );
#endif    
}

// =========================================================================
// Loader stream functions

Cyg_LoaderStream::Cyg_LoaderStream()
{
}

Cyg_LoaderStream::~Cyg_LoaderStream()
{
}

cyg_code Cyg_LoaderStream::get_byte(CYG_BYTE *val)
{
    return CYG_LOADERR_EOF;
}

cyg_code Cyg_LoaderStream::get_data(CYG_BYTE *addr, CYG_ADDRWORD size)
{
    return CYG_LOADERR_EOF;
}

cyg_code Cyg_LoaderStream::seek(CYG_ADDRWORD pos)
{
    return CYG_LOADERR_SEEK;    
}

// =========================================================================
// Memory based loader stream

Cyg_LoaderStream_Mem::Cyg_LoaderStream_Mem( const void *addr, cyg_int32 size )
{
    base = pos = (CYG_ADDRESS)addr;
    end = base + size;
}

Cyg_LoaderStream_Mem::~Cyg_LoaderStream_Mem()
{
    // nothing to do
}
    
cyg_code Cyg_LoaderStream_Mem::get_byte(CYG_BYTE *val)
{
    if( pos == end )
        return CYG_LOADERR_EOF;

    *val = *(CYG_BYTE *)pos;
    pos++;
    
    return CYG_LOADERR_NOERROR;
}

cyg_code Cyg_LoaderStream_Mem::get_data(CYG_BYTE *addr, CYG_ADDRWORD size)
{
    if( pos == end || (pos+size) > end )
        return CYG_LOADERR_EOF;

    memcpy( (void *)addr, (void *)pos, size );

    pos += size;
    
    return CYG_LOADERR_NOERROR;
}

cyg_code Cyg_LoaderStream_Mem::seek(CYG_ADDRWORD apos)
{
    CYG_ADDRWORD npos = base+apos;
    
    if( npos > end || npos < base )
        return CYG_LOADERR_SEEK;

    pos = npos;

    return CYG_LOADERR_NOERROR;
}

// =========================================================================
// file based loader stream

#ifdef CYGPKG_IO_FILEIO

#include <unistd.h>

Cyg_LoaderStream_File::Cyg_LoaderStream_File( int afd )
{
    fd = afd;
}

Cyg_LoaderStream_File::~Cyg_LoaderStream_File()
{
    // nothing to do
    fd = 0;
}
    
cyg_code Cyg_LoaderStream_File::get_byte(CYG_BYTE *val)
{
    ssize_t got = read( fd, (void *)val, 1 );
    
    if( got == 0 )
        return CYG_LOADERR_EOF;

    return CYG_LOADERR_NOERROR;
}

cyg_code Cyg_LoaderStream_File::get_data(CYG_BYTE *addr, CYG_ADDRWORD size)
{
    ssize_t got = read( fd, (void *)addr, size );
    
    if( got != (ssize_t)size )
        return CYG_LOADERR_EOF;

    return CYG_LOADERR_NOERROR;
}

cyg_code Cyg_LoaderStream_File::seek(CYG_ADDRWORD apos)
{
    off_t npos = lseek( fd, apos, SEEK_SET );
    
    if( npos != apos )
        return CYG_LOADERR_SEEK;

    return CYG_LOADERR_NOERROR;
}

#endif

// =========================================================================
// EOF loader.cxx
