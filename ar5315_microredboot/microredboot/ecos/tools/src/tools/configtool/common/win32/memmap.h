//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
//
// This program is part of the eCos host tools.
//
// This program is free software; you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation; either version 2 of the License, or (at your option) 
// any later version.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
// more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ----------------------------------------------------------------------------
//                                                                          
//####COPYRIGHTEND####
//=================================================================
//
//        memmap.h
//
//        Memory Layout Tool map data structure manipulation interface
//
//=================================================================
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     John Dallaway
// Contact(s):    jld
// Date:          1998/07/29 $RcsDate$ {or whatever}
// Version:       0.00+  $RcsVersion$ {or whatever}
// Purpose:       Provides an interface to create and destroy memory
//                regions and sections within the memory map. Exposes
//                data structures for the presentation of this data
//                by external code.
// See also:      memmap.cpp
// Known bugs:    <UPDATE_ME_AT_RELEASE_TIME>
// WARNING:       Do not modify data structures other than by using the
//                provided functions
// Usage:         #include "memmap.h"
//                ...
//                status = set_map_size (0x8000);
//
//####DESCRIPTIONEND####

#if !defined(AFX_MEMMAP_H__75497C90_17F4_11D2_BFBB_00A0C9554250__INCLUDED_)
#define AFX_MEMMAP_H__75497C90_17F4_11D2_BFBB_00A0C9554250__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifdef _AFXDLL
#include "stdafx.h"
    #define INCLUDEFILE <list>
    #include "IncludeSTL.h"
    #define INCLUDEFILE <map>
    #include "IncludeSTL.h"
    #define INCLUDEFILE <string>
    #include "IncludeSTL.h"
    #define INCLUDEFILE <algorithm>
    #include "IncludeSTL.h"
#else
    #include <list>
    #include <map>
    #include <string>
    #include <algorithm>
#endif

#include <time.h>

#define ERR_MEMMAP_REGION_NONAME 1
#define ERR_MEMMAP_REGION_MAPSIZE 2
#define ERR_MEMMAP_REGION_INTERSECT 3 /* region name returned in error_info */
#define ERR_MEMMAP_REGION_NAMEINUSE 4
#define ERR_MEMMAP_REGION_NOTFOUND 5
#define ERR_MEMMAP_REGION_SIZE 6
#define ERR_MEMMAP_ALLOC 7
#define ERR_MEMMAP_SECTION_NONAME 8
#define ERR_MEMMAP_SECTION_NAMEINUSE 9
#define ERR_MEMMAP_SECTION_LMA_NOTINREGION 10
#define ERR_MEMMAP_SECTION_VMA_NOTINREGION 11
#define ERR_MEMMAP_SECTION_LMA_ANCHORNOTFOUND 12
#define ERR_MEMMAP_SECTION_LMA_ANCHORNOTAVAIL 13
#define ERR_MEMMAP_SECTION_VMA_ANCHORNOTFOUND 14
#define ERR_MEMMAP_SECTION_VMA_ANCHORNOTAVAIL 15
#define ERR_MEMMAP_SECTION_NOTFOUND 17
#define ERR_MEMMAP_SECTION_VMA_READONLY 18
#define ERR_MEMMAP_SECTION_LMA_READWRITE 19
#define ERR_MEMMAP_SECTION_ILLEGAL_RELOCATION 20

#define LD_ILLEGAL_CHARS _T(" ,.")
#define MLT_FILE_VERSION 0
#define MLT_GENERATED_WARNING "// This is a generated file - do not edit"

typedef unsigned long mem_address; // FIXME: is a 32-bit memory address sufficient?
typedef struct tag_mem_location mem_location; // forward declaration for struct tag_mem_location

// the location of each section is specified either relative to another
// section or using an absolute memory address, either the start or end
// address may be specified

typedef enum mem_anchor {relative, absolute};

// each section view item may represent either the initial location of
// the section, the final location, or both locations if the section does
// not relocate

typedef enum section_location_type {initial_location, final_location, fixed_location};

// a memory region may be either ROM (read-only) or RAM (read-write).

typedef enum mem_type {read_only, read_write};

// the memory section structure describes the initial and final locations
// of a section, its size and relocation information

class mem_section
{
public:
    std::string name; // the name of the section
    bool relocates; // if the memory section relocates
    bool linker_defined; // if the memory section is linker-defined
    mem_address alignment; // the section alignment
    mem_address size; // memory section size (zero if unknown)
    mem_location * final_location; // the final memory section location (always defined)
    mem_location * initial_location; // the initial memory section location (always defined)
    std::string note; // comment lines
    mem_section ();
    ~mem_section ();
};

// the memory location structure describes the way in which the section is
// anchored, the absolute address of the anchor (if any) and the names of
// the preceding and following relative sections (if any)

typedef struct tag_mem_location
{
    mem_anchor anchor; // type of anchor
    mem_address address; // the absolute anchor address (if any)
    std::list <mem_section>::iterator following_section; // the section declared as following this one
} mem_location;

// the section view structure consists of the section name (which is used
// as a key to look up section information in the section map) and an enum
// describing the state of the section which it represents

typedef struct tag_mem_section_view
{
    std::list <mem_section>::iterator section; // unused section if NULL
    section_location_type section_location;
} mem_section_view;

// the memory region structure describes the region name, size, address,
// RAM/ROM status and a list of section views which reside in the region

typedef struct tag_mem_region
{
    std::string name; // the name of the region
    mem_address size; // the size of the memory region in bytes
    mem_address address; // the absolute location of the memory region
    mem_type type; // ROM or RAM
    std::list <mem_section_view> section_view_list;
    std::string note; // comment lines
} mem_region;


class mem_map
{
public:
	bool delete_memory_section (std::string name);
    int edit_memory_section (std::string old_name,
        std::string new_name,
        mem_address section_size,
        mem_address section_alignment,
        mem_anchor initial_section_anchor,
        std::string initial_anchor_section_name,
        mem_address initial_anchor_address,
        mem_anchor final_section_anchor,
        std::string final_anchor_section_name,
        mem_address final_anchor_address,
        bool relocates,
        bool anchor_to_initial_location,
        bool linker_defined,
        std::string note);
    int create_memory_section (std::string section_name,
        mem_address section_size,
        mem_address section_alignment,
        mem_anchor initial_section_anchor,
        std::string initial_anchor_section_name,
        mem_address initial_anchor_address,
        mem_anchor final_section_anchor,
        std::string final_anchor_section_name,
        mem_address final_anchor_address,
        bool relocates,
        bool anchor_to_initial_location,
        bool linker_defined,
        std::string note);
	bool set_map_size (mem_address size);
	bool delete_memory_region (std::string name);
    bool get_memory_region (std::string region_name,
        mem_address * region_address,
        mem_address * region_size,
        mem_type * region_type,
        std::string * note);
	int create_memory_region (std::string name, mem_address location,
        mem_address size, mem_type type, std::string note);
    int edit_memory_region (std::string old_name, std::string new_name, mem_address new_location,
        mem_address new_size, mem_type new_type, std::string note);
    bool delete_all_memory_sections ();
    bool export_files (LPCTSTR  script_name, LPCTSTR  header_name);
    bool import_linker_defined_sections (LPCTSTR  filename);
    bool save_memory_layout (LPCTSTR  filename);
    bool load_memory_layout (LPCTSTR  filename);
    bool new_memory_layout ();
	bool map_modified () { return map_modified_flag; };
    bool section_exists (std::string section_name);
    std::list <mem_section>::iterator find_memory_section (std::string section_name);
    std::list <mem_section>::iterator find_preceding_section (std::list <mem_section>::iterator section, bool initial_location);
    std::string error_info;
    std::list <mem_region> region_list; // ordered list of memory regions
    std::list <mem_section> section_list; // list of memory sections
    std::list <std::string> linker_defined_section_list; // list of linker-defined sections

	mem_map();
	virtual ~mem_map();

private:
    std::list <mem_region>::iterator find_memory_region (std::string);
    std::list <mem_region>::iterator find_region_by_address (mem_address);
    std::list <mem_region>::iterator find_region_by_section (std::list <mem_section>::iterator section, section_location_type location_type);
    mem_address map_size; // total size of memory map
    bool add_absolute_section_to_list (std::list <mem_region>::iterator region,
        std::list <mem_section>::iterator additional_section,
        section_location_type location_type);
    bool add_relative_sections_to_list (std::list <mem_region>::iterator region,
        std::list <mem_section_view>::iterator section_view,
        section_location_type location_type);
    bool calc_section_list (std::list <mem_region>::iterator region);
    bool calc_section_lists ();
    bool at_start_of_region (std::list <mem_section>::iterator section,
        std::list <mem_region>::iterator region);
    bool at_end_of_region (std::list <mem_section>::iterator section,
        std::list <mem_region>::iterator region);
	bool absolute_sections_meet (std::list <mem_section>::iterator section1,
        std::list <mem_section>::iterator section2);
    bool load_memory_section_1 (FILE * stream);
    bool load_memory_section_2 (FILE * stream);
    bool export_sections (FILE *, FILE *, mem_type);
	bool map_modified_flag;
    std::string encode_note (std::string);
    std::string decode_note (std::string);
    std::string encode_section_name (std::string);
    std::string decode_section_name (std::string);
};

#endif // !defined(AFX_MEMMAP_H__75497C90_17F4_11D2_BFBB_00A0C9554250__INCLUDED_)
