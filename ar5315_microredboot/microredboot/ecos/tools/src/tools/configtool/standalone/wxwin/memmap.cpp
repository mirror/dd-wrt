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
//        memmap.cpp
//
//        Memory Layout Tool map data structure manipulation class
//
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     John Dallaway
// Contact(s):    jld
// Date:          1998/07/29 $RcsDate$ {or whatever}
// Version:       0.00+  $RcsVersion$ {or whatever}
// Purpose:       Provides functions to create and destroy memory regions
//                and sections within the memory map.
// Description:   Each function manipulates data structures representing
//                memory regions, memory sections and the view of memory
//                sections as presented to the user. The section view
//                structure organises the sections by region and 
//                will contain two instances of each relocated section 
// Requires:      memmap.h
// Provides:      create_memory_region()
//                delete_memory_region()
//                edit_memory_region()
//                create_memory_section()
//                delete_memory_section()
//                edit_memory_section()
//                delete_all_memory_sections()
//                set_map_size()
//                section_list
//                region_list
// See also:      memmap.h
// Known bugs:    <UPDATE_ME_AT_RELEASE_TIME>
// WARNING:       Do not modify data structures other than by using the
//                provided functions
// Usage:         #include "memmap.h"
//                ...
//                status = set_map_size (0x8000);
//
//####DESCRIPTIONEND####

#pragma warning (disable:4514) /* unreferenced inline function */
#pragma warning (disable:4710) /* function not inlined */

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "memmap.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

mem_map::mem_map()
{
	map_modified_flag = true;
	map_size = (mem_address) 0;
}

mem_map::~mem_map()
{

}

mem_section::mem_section()
{

}

mem_section::~mem_section()
{

}

///////////////////////////////////////////////////////////////////////
// get_memory_region() retrieves the parameters of a memory region

bool mem_map::get_memory_region (std::string region_name, mem_address * region_address, mem_address * region_size, mem_type * region_type, std::string * note)
{
    for (list <mem_region>::iterator region = region_list.begin (); region != region_list.end (); ++region)
        if (region->name == region_name)
        {
            *region_address = region->address;
            *region_size = region->size;
            *region_type = region->type;
            *note = region->note;
            return true;
        }

    return false;
}


///////////////////////////////////////////////////////////////////////
// create_memory_region() inserts a new item into the memory region list
// in order of memory address

int mem_map::create_memory_region (std::string new_region_name, mem_address new_region_address, mem_address new_region_size, mem_type new_region_type, std::string note)
{
    const mem_address new_region_end = new_region_address + new_region_size; // the byte after the new region end

    // check that the new region name is specified

    if (new_region_name == "")
        return ERR_MEMMAP_REGION_NONAME; // the new region name must be specified

    // check that the new region lies within the memory map

    if (new_region_end > map_size)
        return ERR_MEMMAP_REGION_MAPSIZE; // the new region does not lie within the memory map

    // check that the region end address hasn't exceeded the storage size

    if (new_region_end < new_region_address)
        return ERR_MEMMAP_REGION_MAPSIZE; // the new region does not lie within the memory map

    // initialise the insertion point for the new region

    list <mem_region>::iterator insertion_point = region_list.end ();

    // check that the new region does not overlap existing regions and does not already exist

    for (list <mem_region>::iterator region = region_list.begin (); region != region_list.end (); ++region)
    {
        const mem_address region_end = region->address + region->size; // the byte after the region end

        if ((new_region_address >= region->address) && (new_region_address < region_end))
        {
            error_info = region->name;
            return ERR_MEMMAP_REGION_INTERSECT; // the start of the new region is within an existing region
        }

        if ((new_region_end > region->address) && (new_region_end <= region_end))
        {
            error_info = region->name;
            return ERR_MEMMAP_REGION_INTERSECT; // the end of the new region is within an existing region
        }

        if ((new_region_address < region->address) && (new_region_end > region_end))
        {
            error_info = region->name;
            return ERR_MEMMAP_REGION_INTERSECT; // an existing region lies within the new region
        }

        if (region->name == new_region_name)
            return ERR_MEMMAP_REGION_NAMEINUSE; // the new region name is not unique

        if ((insertion_point == region_list.end ()) && (region->address > new_region_address))
            insertion_point = region; // insert the new region here
    }

    // add the new region to the region list

    list <mem_region>::iterator new_region = region_list.insert (insertion_point);
    new_region->name = new_region_name;
    new_region->address = new_region_address;
    new_region->size = new_region_size;
    new_region->type = new_region_type;
    new_region->note = note;

    // initialise the section list for the new region

    calc_section_list (new_region);

	map_modified_flag = true;
    return 0;
}


///////////////////////////////////////////////////////////////////////
// edit_memory_region() edits an item in the memory region list

int mem_map::edit_memory_region (std::string old_region_name, std::string new_region_name, mem_address new_region_address, mem_address new_region_size, mem_type new_region_type, std::string note)
{
    list <mem_region>::iterator edit_region = find_memory_region (old_region_name);
    if (edit_region == NULL)
        return ERR_MEMMAP_REGION_NOTFOUND; // the region to be modified does not exist

    // check that the new region name is specified

    if (new_region_name == "")
        return ERR_MEMMAP_REGION_NONAME; // the new region name must be specified

    // check that the region end address hasn't exceeded the storage size

    if (new_region_address + new_region_size < new_region_address)
        return ERR_MEMMAP_REGION_MAPSIZE; // the new region does not lie within the memory map

    // check region name change

    if ((old_region_name != new_region_name) &&
        (find_memory_region (new_region_name) != NULL))
        return ERR_MEMMAP_REGION_NAMEINUSE; // new region name is not unique

    // check region address/size change wrt other regions

    const mem_address new_region_end = new_region_address + new_region_size;
    if ((new_region_address != edit_region->address) ||
        (new_region_size != edit_region->size))
    {
        for (list <mem_region>::iterator region = region_list.begin (); region != region_list.end (); ++region)
            if (region != edit_region)
        {
            const mem_address region_end = region->address + region->size; // the byte after the region end

            if ((new_region_address >= region->address) && (new_region_address < region_end))
            {
                error_info = region->name;
                return ERR_MEMMAP_REGION_INTERSECT; // the start of the modified region is within another region
            }

            if ((new_region_end > region->address) && (new_region_end <= region_end))
            {
                error_info = region->name;
                return ERR_MEMMAP_REGION_INTERSECT; // the end of the modified region is within an existing region
            }

            if ((new_region_address < region->address) && (new_region_end > region_end))
            {
                error_info = region->name;
                return ERR_MEMMAP_REGION_INTERSECT; // another region lies within the modified region
            }
        }
    }

    // check region size change wrt sections within region (if any)

    for (list <mem_section_view>::iterator section_view = edit_region->section_view_list.begin (); section_view != edit_region->section_view_list.end (); ++section_view)
        if (section_view->section != NULL)
    {
        if ((section_view->section_location == final_location) || (section_view->section_location == fixed_location))
            if (section_view->section->final_location->anchor == absolute)
                if (section_view->section->final_location->address + section_view->section->size - edit_region->address > new_region_size)
                    return ERR_MEMMAP_REGION_SIZE; // region is now too small

        if (section_view->section_location == initial_location)
            if (section_view->section->initial_location->anchor == absolute)
                if (section_view->section->initial_location->address + section_view->section->size - edit_region->address > new_region_size)
                    return ERR_MEMMAP_REGION_SIZE; // region is now too small
    }

    // check region read-only change FIXME

    // move sections within the region having absolute anchors

    for (section_view = edit_region->section_view_list.begin (); section_view != edit_region->section_view_list.end (); ++section_view)
        if (section_view->section != NULL)
    {
        if ((section_view->section_location == final_location) || (section_view->section_location == fixed_location))
            if (section_view->section->final_location->anchor == absolute)
                section_view->section->final_location->address += (new_region_address - edit_region->address);

        if ((section_view->section_location == initial_location) || (section_view->section_location == fixed_location))
            if (section_view->section->initial_location->anchor == absolute)
                section_view->section->initial_location->address += (new_region_address - edit_region->address);
    }

    // deleteZ(the region and recreate it to make sure the region list is ordered correctly)

    region_list.erase (edit_region);
    if (create_memory_region (new_region_name, new_region_address, new_region_size, new_region_type, note))
        return ERR_MEMMAP_ALLOC;

	map_modified_flag = true;
    return 0;
}


//////////////////////////////////////////////////////////////////
// delete_memory_region() removes an existing item from the memory
// region list

bool mem_map::delete_memory_region (std::string name)
{
    // make sure that there are no used sections in this region before deleting it

    for (list <mem_region>::iterator region = region_list.begin (); region != region_list.end (); ++region)
    {
        if ((region->name == name) && (region->section_view_list.size () == 1) &&  (region->section_view_list.front ().section == NULL))
        {
            region_list.erase (region);
			map_modified_flag = true;
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////
// set_map_size() sets the maximum permitted address for the end
// of any memory region

bool mem_map::set_map_size (mem_address new_map_size)
{
    // check that the new size is sufficient for all previously defined memory regions

    for (list <mem_region>::iterator region = region_list.begin (); region != region_list.end (); ++region)
    {
        if (region->address + region->size > new_map_size)
            return false; // the new map size is too small
    }

    // set the map size

    map_size = new_map_size;

    return true;
}


////////////////////////////////////////////////////////////////////
// edit_memory_section() edits an item to the memory section map

int mem_map::edit_memory_section (std::string old_section_name, std::string new_section_name, mem_address section_size, mem_address section_alignment, mem_anchor initial_section_anchor, std::string initial_anchor_section_name, mem_address initial_anchor_address, mem_anchor final_section_anchor, std::string final_anchor_section_name, mem_address final_anchor_address, bool relocates, bool anchor_to_initial_location, bool linker_defined, std::string note)
{
    // do all the parameter validation

    if (new_section_name == "") // the new section name must be specified
        return ERR_MEMMAP_SECTION_NONAME;

    if ((new_section_name != old_section_name) &&
        (find_memory_section (new_section_name) != NULL))
        return ERR_MEMMAP_SECTION_NAMEINUSE; // the new section name is not unique

    list <mem_section>::iterator section = find_memory_section (old_section_name);
    if (section == NULL)
        return ERR_MEMMAP_SECTION_NOTFOUND; // the specified old section name could not be found

    // check that the LMA (if absolute) is within a memory region

    list <mem_region>::iterator region;
    if (initial_section_anchor == absolute)
    {
        region = find_region_by_address (initial_anchor_address);
        if (region == NULL)
            return ERR_MEMMAP_SECTION_LMA_NOTINREGION; // section LMA is not within a memory region
        if ((section_size > 0) && (initial_anchor_address + section_size > region->address + region->size))
            return ERR_MEMMAP_SECTION_LMA_NOTINREGION; // end of section is not within the memory region
        if (relocates && (region->type == read_write))
            return ERR_MEMMAP_SECTION_LMA_READWRITE; // section LMA must be in a read-only memory region
    }

    // check that the VMA (if absolute) is within a memory region

    if (final_section_anchor == absolute)
    {
        region = find_region_by_address (final_anchor_address);
        if (region == NULL)
            return ERR_MEMMAP_SECTION_VMA_NOTINREGION; // section VMA is not within a memory region
        if ((section_size > 0) && (final_anchor_address + section_size > region->address + region->size))
            return ERR_MEMMAP_SECTION_VMA_NOTINREGION; // end of section is not within the memory region
        if (relocates && (region->type == read_only))
            return ERR_MEMMAP_SECTION_VMA_READONLY; // section VMA must be in a read/write memory region
    }

    // check relative location information as appropriate

    if (relocates) // only check the initial parent section if the section relocates
    {
        if (initial_section_anchor == relative)
        {
            list <mem_section>::iterator parent_section = find_memory_section (initial_anchor_section_name);
            if (parent_section == section_list.end ())
                return ERR_MEMMAP_SECTION_LMA_ANCHORNOTFOUND; // initial anchor name not found

            if ((parent_section->initial_location->following_section != section) && (parent_section->initial_location->following_section != NULL))
                return ERR_MEMMAP_SECTION_LMA_ANCHORNOTAVAIL; // initial anchor specified has changed and is unavailable

            if ((parent_section->size == 0) && (! parent_section->linker_defined))
                return ERR_MEMMAP_SECTION_LMA_ANCHORNOTAVAIL; // initial anchor specified expands to fit available space

            if (find_region_by_section (parent_section, initial_location)->type == read_write)
                return ERR_MEMMAP_SECTION_LMA_READWRITE; // initial anchor must be in a read-only memory region
        }
    }

    if (final_section_anchor == relative)
    {
        list <mem_section>::iterator parent_section = find_memory_section (final_anchor_section_name);
        if (parent_section == NULL)
            return ERR_MEMMAP_SECTION_VMA_ANCHORNOTFOUND; // final anchor name not found

        if ((parent_section->size == 0) && (! parent_section->linker_defined))
            return ERR_MEMMAP_SECTION_VMA_ANCHORNOTAVAIL; // final anchor specified expands to fit available space

        if ((!relocates) && anchor_to_initial_location) // final anchor to initial location of parent section
        {
            if ((parent_section->initial_location->following_section != section) && (parent_section->initial_location->following_section != NULL))
                return ERR_MEMMAP_SECTION_VMA_ANCHORNOTAVAIL; // final anchor specified has changed and is unavailable
        }
        else
        {
            if ((parent_section->final_location->following_section != section) && (parent_section->final_location->following_section != NULL))
                return ERR_MEMMAP_SECTION_VMA_ANCHORNOTAVAIL; // final anchor specified has changed and is unavailable
        }

        if (relocates && (find_region_by_section (parent_section, final_location)->type == read_only))
            return ERR_MEMMAP_SECTION_VMA_READONLY; // final anchor of relocating section must be in a read/write memory region
    }

	// check for a non-relocating section changing to relocating where the final
	// location moves from a read_only region to a read_write region and there
	// is a following non-relocating section

    if (relocates && (! section->relocates) &&
		(find_region_by_section (section, fixed_location)->type == read_only) &&
		(section->final_location->following_section != NULL) &&
		(! section->final_location->following_section->relocates))
	{
		return ERR_MEMMAP_SECTION_ILLEGAL_RELOCATION;
	}

    // FIXME check for overlap of absolute sections

    // modify the initial section location data

    if (section->initial_location->anchor == relative) // initial section anchor was relative
        find_preceding_section (section, true)->initial_location->following_section = NULL;

    if (initial_section_anchor == absolute) // initial location now absolute
        section->initial_location->address = initial_anchor_address;
    else // initial location now relative
    {
        list <mem_section>::iterator initial_parent = find_memory_section (initial_anchor_section_name);
        if (relocates || (! initial_parent->relocates))
            initial_parent->initial_location->following_section = section;
    }

    // modify the final section location data

    if (section->final_location->anchor == relative) // final section anchor was relative
        find_preceding_section (section, false)->final_location->following_section = NULL;

    if (final_section_anchor == absolute) // final location now absolute
        section->final_location->address = final_anchor_address;
    else // final location now relative
    {
        list <mem_section>::iterator final_parent = find_memory_section (final_anchor_section_name);
        final_parent->final_location->following_section = section;
    }

    // handle relocation changes

    if (relocates && (! section->relocates)) // section was non-relocating but now relocates
	{
		if (find_region_by_section (section, fixed_location)->type == read_only) // the section was in a read_only region
		   section->final_location->following_section = NULL; // there is now no section following the final location
		else
		   section->initial_location->following_section = NULL; // there is now no section following the initial location
	}

    else if ((! relocates) && section->relocates) // section was relocating but is now non-relocating
	{
		// determine the type of memory region in which the section now resides

		mem_type type;
		if ((final_section_anchor == relative) && anchor_to_initial_location)
			type = find_region_by_section (find_memory_section (final_anchor_section_name), initial_location)->type;
		else if (final_section_anchor == relative) // anchored to final location of preceding section
			type = find_region_by_section (find_memory_section (final_anchor_section_name), final_location)->type;
		else // final_section_anchor must be absolute
			type = find_region_by_address (final_anchor_address)->type;

		if (type == read_only) // the section is now in a read-only memory region
		{
			if ((section->initial_location->following_section != NULL) && ! section->initial_location->following_section->relocates)
				section->final_location->following_section = section->initial_location->following_section;
			else
				section->final_location->following_section = NULL;
		}
		else // the section is now in a read-write memory region
		{
			if ((section->final_location->following_section != NULL) && ! section->final_location->following_section->relocates)
				section->initial_location->following_section = section->final_location->following_section;
			else
				section->initial_location->following_section = NULL;
		}
	}

    // modify the remaining section data

    section->name = new_section_name;
    section->size = section_size;
    section->alignment = section_alignment;
    section->relocates = relocates;
    section->note = note;
    section->linker_defined = linker_defined;
    section->initial_location->anchor = initial_section_anchor;
    section->final_location->anchor = final_section_anchor;

    // recalculate section lists for all regions

    calc_section_lists ();

	map_modified_flag = true;
    return 0;
}


////////////////////////////////////////////////////////////////////
// create_memory_section() adds a new item to the memory section map
// either a section name (for relative locations) or an anchor address
// (for absolute locations) must be specified

int mem_map::create_memory_section (std::string section_name, mem_address section_size, mem_address section_alignment, mem_anchor initial_section_anchor, std::string initial_anchor_section_name, mem_address initial_anchor_address, mem_anchor final_section_anchor, std::string final_anchor_section_name, mem_address final_anchor_address, bool relocates, bool anchor_to_initial_location, bool linker_defined, std::string note)
{
    list <mem_region>::iterator region;

    // check that the new section name is specified

    if (section_name == "")
        return ERR_MEMMAP_SECTION_NONAME; // the new section name must be specified
    
    // check that the new section name is unique

    if (find_memory_section (section_name) != NULL)
        return ERR_MEMMAP_SECTION_NAMEINUSE; // the new section name is not unique

    // check that the LMA (if absolute) is within a memory region

    if (initial_section_anchor == absolute)
    {
        region = find_region_by_address (initial_anchor_address);
        if (region == NULL)
            return ERR_MEMMAP_SECTION_LMA_NOTINREGION; // section LMA is not within a memory region
        if ((section_size > 0) && (initial_anchor_address + section_size > region->address + region->size))
            return ERR_MEMMAP_SECTION_LMA_NOTINREGION; // end of section is not within the memory region
        if (relocates && (region->type == read_write))
            return ERR_MEMMAP_SECTION_LMA_READWRITE; // section LMA must be in a read-only memory region
    }

    // check that the VMA (if absolute) is within a memory region

    if (final_section_anchor == absolute)
    {
        region = find_region_by_address (final_anchor_address);
        if (region == NULL)
            return ERR_MEMMAP_SECTION_VMA_NOTINREGION; // section VMA is not within a memory region
        if ((section_size > 0) && (final_anchor_address + section_size > region->address + region->size))
            return ERR_MEMMAP_SECTION_VMA_NOTINREGION; // end of section is not within the memory region
        if (relocates && (region->type == read_only))
            return ERR_MEMMAP_SECTION_VMA_READONLY; // section VMA must be in a read/write memory region
    }

    // FIXME check for overlap of absolute sections

    // check that specified parent(s) (for relative anchors) are available

    if (relocates) // only check the initial parent section if the section relocates
    {
        if (initial_section_anchor == relative)
        {
            list <mem_section>::iterator parent_section = find_memory_section (initial_anchor_section_name);
            if (parent_section == section_list.end ())
                return ERR_MEMMAP_SECTION_LMA_ANCHORNOTFOUND; // initial anchor name not found
/*
            if (parent_section->initial_location->following_section != NULL)
                return ERR_MEMMAP_SECTION_LMA_ANCHORNOTAVAIL; // initial anchor specified is unavailable
*/
            if ((parent_section->size == 0) && (! parent_section->linker_defined))
                return ERR_MEMMAP_SECTION_LMA_ANCHORNOTAVAIL; // initial anchor specified expands to fit available space

            if (find_region_by_section (parent_section, initial_location)->type == read_write)
                return ERR_MEMMAP_SECTION_LMA_READWRITE; // initial anchor must be in a read-only memory region
        }
    }

    if (final_section_anchor == relative)
    {
        list <mem_section>::iterator parent_section = find_memory_section (final_anchor_section_name);
        if (parent_section == NULL)
            return ERR_MEMMAP_SECTION_VMA_ANCHORNOTFOUND; // final anchor name not found

        if ((parent_section->size == 0) && (! parent_section->linker_defined))
            return ERR_MEMMAP_SECTION_VMA_ANCHORNOTAVAIL; // final anchor specified expands to fit available space
/*
        if ((!relocates) && anchor_to_initial_location) // final anchor to initial location of parent section
        {
            if (parent_section->initial_location->following_section != NULL)
                return ERR_MEMMAP_SECTION_VMA_ANCHORNOTAVAIL; // final anchor specified is unavailable
        }
        else
        {
            if (parent_section->final_location->following_section != NULL)
                return ERR_MEMMAP_SECTION_VMA_ANCHORNOTAVAIL; // final anchor specified is unavailable
        }
*/
        if (relocates && (find_region_by_section (parent_section, final_location)->type == read_only))
            return ERR_MEMMAP_SECTION_VMA_READONLY; // final anchor of relocating section must be in a read/write memory region
    }

    // add the new section to the section map

    mem_section new_mem_section;
    list <mem_section>::iterator new_section = section_list.insert (section_list.begin (), new_mem_section);
    new_section->name = section_name;
    new_section->size = section_size;
    new_section->alignment = section_alignment;
    new_section->relocates = relocates;
    new_section->note = note;
    new_section->linker_defined = linker_defined;
    new_section->initial_location = new mem_location;
    new_section->final_location = new mem_location;
    new_section->initial_location->following_section = NULL; // initialize struct
    new_section->final_location->following_section = NULL; // initialize struct
    new_section->initial_location->anchor = initial_section_anchor;
    new_section->final_location->anchor = final_section_anchor;

    if ((initial_section_anchor == relative) &&
        (!relocates) && (find_memory_section (initial_anchor_section_name)->relocates))
    {
        // a non-relocating relative section anchored to a relocating section

        if (anchor_to_initial_location) // new section is anchored to the initial location of a relocating section
        {
            list <mem_section>::iterator anchor_section = find_memory_section (initial_anchor_section_name);
            new_section->initial_location->following_section = anchor_section->initial_location->following_section;
            anchor_section->initial_location->following_section = new_section;
        }
        else // new section is anchored to the final location of a relocating section
        {
            list <mem_section>::iterator anchor_section = find_memory_section (initial_anchor_section_name);
            new_section->final_location->following_section = anchor_section->final_location->following_section;
            anchor_section->final_location->following_section = new_section;
        }
    }
    else
    {
        // copy initial location data

        if (initial_section_anchor == relative) // new section follows the named anchor section
        {
            list <mem_section>::iterator anchor_section = find_memory_section (initial_anchor_section_name);
            new_section->initial_location->following_section = anchor_section->initial_location->following_section; // move anchor of the following section
            anchor_section->initial_location->following_section = new_section; // anchor the new section
        }
        else // new section has an absolute anchor
            new_section->initial_location->address = initial_anchor_address;
    
        // copy final location data

        if (final_section_anchor == relative) // new section follows the named anchor section
        {
            list <mem_section>::iterator anchor_section = find_memory_section (final_anchor_section_name);
            new_section->final_location->following_section = anchor_section->final_location->following_section; // move anchor of the following section
            anchor_section->final_location->following_section = new_section; // anchor the new section
        }
        else // new section has an absolute anchor
            new_section->final_location->address = final_anchor_address;
    }

    // recalculate section lists for all regions

    calc_section_lists ();

	map_modified_flag = true;
    return 0;
}


////////////////////////////////////////////////////////////////////////
// calc_section_lists() updates the lists of memory sections for all
// memory regions

bool mem_map::calc_section_lists ()
{
    for (list <mem_region>::iterator region = region_list.begin (); region != region_list.end(); ++region)
        calc_section_list (region);

    return true;
}


////////////////////////////////////////////////////////////////////////
// calc_section_list() updates the list of memory sections which reside 
// in the specified memory region. It is called whenever the section
// map is modified.

bool mem_map::calc_section_list (list <mem_region>::iterator region)
{
    // clear the old list (if any)

	//TRACE (_T("Calculating section list for region '%s'\n"), CString (region->name.c_str()));
    region->section_view_list.clear ();

    // add the initial and final locations of each absolute section as necessary

    for (list <mem_section>::iterator section = section_list.begin (); section != section_list.end (); ++section)
    {
        if (section->relocates) // the section is relocated and must be added to the view twice
        {
            add_absolute_section_to_list (region, section, initial_location);
            add_absolute_section_to_list (region, section, final_location);
        }
        else // the section is not relocated and must be added to the view once only
            add_absolute_section_to_list (region, section, fixed_location);
    }

    // add unused sections to section view list where appropriate

    list <mem_section_view>::iterator previous_section_view = region->section_view_list.begin ();

    if (previous_section_view == region->section_view_list.end ()) // no used sections in this region
    {
        // add a single unused section to the section view list

        mem_section_view new_section_view;
        new_section_view.section = NULL; // an unused section
        region->section_view_list.push_back (new_section_view); // add to the section list for this region
    }
    else // there are used sections in this region
    {
        list <mem_section_view>::iterator second_section_view = region->section_view_list.begin ();
        ++second_section_view;

        // add unused sections between used sections where they do not meet in either initial or final locations

        for (list <mem_section_view>::iterator section_view = second_section_view; section_view != region->section_view_list.end (); ++section_view)
        {
            if (! (absolute_sections_meet (previous_section_view->section, section_view->section)))
            {
                list <mem_section_view>::iterator new_section_view = region->section_view_list.insert (section_view); // add an unused section
                new_section_view->section = NULL;
            }

            previous_section_view = section_view;
        }

        // add an unused section to end of region if the last section does not reach the end of the region in initial or final locations

        if (! at_end_of_region (region->section_view_list.back().section, region))
        {
            mem_section_view new_section_view;
            new_section_view.section = NULL; // an unused section
            region->section_view_list.push_back (new_section_view); // add an unused section
        }

        // add an unused section to start of region if the first section does not start at the start of the region in initial or final locations

        if (! at_start_of_region (region->section_view_list.front().section, region))
        {
            mem_section_view new_section_view;
            new_section_view.section = NULL; // an unused section
            region->section_view_list.push_front (new_section_view); // add an unused section
        }
    }

    // add the initial and final locations of the each relative section as necessary

    for (list <mem_section_view>::iterator section_view = region->section_view_list.begin (); section_view != region->section_view_list.end (); ++section_view)
        if (section_view->section != NULL) // if section is used
    {
        list <mem_section>::iterator section = section_view->section;
/*
		TRACE (_T("Calculating relative sections for section view '%s' %s\n"), CString (section->name.c_str ()),
			section_view->section_location == final_location ? _T("(final)") :
			section_view->section_location == initial_location ? _T("(initial)") : _T("(fixed)"));
*/

        if (section_view->section_location == final_location)
        {
            if (section->final_location->anchor == absolute)
                add_relative_sections_to_list (region, section_view, final_location);            
        }

        else if (section_view->section_location == initial_location)
        {
            if (section->initial_location->anchor == absolute)
                add_relative_sections_to_list (region, section_view, initial_location);
        }

        else // section_view->section_location == fixed_location
        {
            if (section->initial_location->anchor == absolute)
                add_relative_sections_to_list (region, section_view, initial_location);
            if (section->final_location->anchor == absolute)
                add_relative_sections_to_list (region, section_view, final_location);
        }
    }

    // remove unused sections where user-defined section of unknown size will be placed

    section_view = region->section_view_list.begin ();
    while (section_view != region->section_view_list.end ())
    {
        bool expanding_section = false;
        if ((section_view->section != NULL) &&
            (section_view->section->size == 0) &&
            (! section_view->section->linker_defined))
            expanding_section = true;

        ++section_view;

        if (expanding_section && (section_view != region->section_view_list.end ()) && (section_view->section == NULL))
            section_view = region->section_view_list.erase (section_view);
    }

    return true;
}

/////////////////////////////////////////////////////////////////////
// add_relative_sections_to_list() inserts the sections defined relative
// to the specified section list item to the section list for the
// specified region in the appropriate order

bool mem_map::add_relative_sections_to_list (list <mem_region>::iterator region, list <mem_section_view>::iterator section_view, section_location_type location_type)
{
    // insert following relative sections of type 'location_type' in region_view.section_view_list

    list <mem_section>::iterator new_section = section_view->section;
    mem_location * new_section_location = (location_type == initial_location ? new_section->initial_location : new_section->final_location);
    list <mem_section_view>::iterator insertion_point = section_view;
    ++insertion_point;
    bool no_relocation = true;

    while (new_section_location->following_section != NULL)
    {
        // add the new section to the section view list

        mem_section_view new_section_view;
        new_section_view.section = new_section_location->following_section;
		const bool section_relocates = new_section->relocates;
        new_section = new_section_view.section;
        new_section_view.section_location = (new_section->relocates ? location_type : fixed_location);
        if ((new_section_view.section_location == fixed_location) && (location_type == final_location) && (! section_view->section->relocates) && (! section_relocates) && no_relocation)
        {
            // section already added to the view so add nothing but
            // increment insertion point for following sections
            // TRACE (_T("Skipping section %s %s location (relative) preceding %s\n"), CString (new_section_location->following_section->name.c_str()), location_type == initial_location ? _T("initial") : _T("final"), ((insertion_point != region->section_view_list.end ()) && (insertion_point->section != NULL)) ? CString (insertion_point->section->name.c_str()) : _T("(null)"));
            ++insertion_point;
        }
        else
        {
            // TRACE (_T("Inserting section %s %s location (relative) preceding %s\n"), CString (new_section_location->following_section->name.c_str()), location_type == initial_location ? _T("initial") : _T("final"), ((insertion_point != region->section_view_list.end ()) && (insertion_point->section != NULL)) ? CString (insertion_point->section->name.c_str()) : _T("(null)"));
            region->section_view_list.insert (insertion_point, new_section_view);
            no_relocation = no_relocation && ! new_section_view.section->relocates;
        }
        new_section_location = (location_type == initial_location ? new_section->initial_location : new_section->final_location);
    }    

    return true;
}

/////////////////////////////////////////////////////////////////////
// add_absolute_section_to_list() inserts the specified section to the
// specified section list at the appropriate place if it has an
// absolute location and that location is within the specified memory
// region

bool mem_map::add_absolute_section_to_list (list <mem_region>::iterator region, list <mem_section>::iterator additional_section, section_location_type location_type)
{
    // get location of new section
    mem_location * new_section_location = (location_type == initial_location ? additional_section->initial_location : additional_section->final_location);

    if ((new_section_location->anchor == absolute) && (new_section_location->address >= region->address) && (new_section_location->address < region->address + region->size))
        {
        // the section lies in the region

        // initialise the insertion point for the new section
        list <mem_section_view>::iterator insertion_point = region->section_view_list.end ();

        for (list <mem_section_view>::iterator section = region->section_view_list.begin (); section != region->section_view_list.end (); ++section)
        {
            // get location of section
            mem_location * section_location  = (section->section_location == initial_location ? section->section->initial_location : section->section->final_location);

            // compare with location of new section
            if ((new_section_location->anchor == absolute) && (section_location->address >= new_section_location->address))
            {
                // insert new section here if the current section has a higher address
                insertion_point = section;
                break;
            }
        }

        // add the new section to the section view list

		// TRACE (_T("Inserting section %s %s location (absolute) preceding %s\n"), CString (additional_section->name.c_str()), location_type == initial_location ? _T("initial") : _T("final"), insertion_point != region->section_view_list.end () ? CString (insertion_point->section->name.c_str()) : _T("(end)"));
        mem_section_view new_section_view;
        new_section_view.section = additional_section;
        new_section_view.section_location = location_type;
        region->section_view_list.insert (insertion_point, new_section_view);
    }

    return true;
}


////////////////////////////////////////////////////////////////////
// absolute_sections_meet() determines whether the specified
// absolute memory sections meet. It assumes that section2 comes
// after section1 in the memory map.

bool mem_map::absolute_sections_meet(list <mem_section>::iterator section1, list <mem_section>::iterator section2)
{
    if (section1->size == 0) // size of section1 is unknown
        return false;

    // check if initial section locations meet

    if ((section1->initial_location->anchor == absolute) && 
        ((section2->initial_location->anchor == absolute) &&
        section1->initial_location->address + section1->size == section2->initial_location->address))
        return true;

    // check if final section locations meet

    if ((section1->final_location->anchor == absolute) && 
        ((section2->final_location->anchor == absolute) &&
        section1->final_location->address + section1->size == section2->final_location->address))
        return true;

    return false;
}


//////////////////////////////////////////////////////////////
// at_start_of_region() determines whether the specified section
// is located at the very start of the specified region

bool mem_map::at_start_of_region (list <mem_section>::iterator section, list <mem_region>::iterator region)
{
    // check initial section location
    
    if ((section->initial_location->anchor == absolute) &&
        (section->initial_location->address == region->address))
        return true;

    // check final section location
    
    if ((section->final_location->anchor == absolute) &&
        (section->final_location->address == region->address))
        return true;

    return false;
}

//////////////////////////////////////////////////////////////
// at_end_of_region() determines whether the specified section
// is located at the very end of the specified region

bool mem_map::at_end_of_region (list <mem_section>::iterator section, list <mem_region>::iterator region)
{
    if (section->size == 0) // size of section is unknown
        return false;

    // check initial section location
    
    if ((section->initial_location->anchor == absolute) &&
        section->initial_location->address + section->size == region->address + region->size)
        return true;

    // check final section location
    
    if ((section->final_location->anchor == absolute) &&
        section->final_location->address + section->size == region->address + region->size)
        return true;

    return false;
}

////////////////////////////////////////////////////////////////////////
// find_preceding_section() finds the preceding section in the
// memory section list

list <mem_section>::iterator mem_map::find_preceding_section (list <mem_section>::iterator reference_section, bool initial_location)
{
    for (list <mem_section>::iterator section = section_list.begin (); section != section_list.end (); ++section)
    {
        if (reference_section == (reference_section->relocates && initial_location ? section->initial_location->following_section : section->final_location->following_section)) // if preceding section found
            return section; // return the section iterator
    }
    return NULL; // section not found
}

////////////////////////////////////////////////////////////////////////
// find_memory_section() finds an existing section in the
// memory section list

list <mem_section>::iterator mem_map::find_memory_section (std::string section_name)
{
    for (list <mem_section>::iterator section = section_list.begin (); section != section_list.end (); ++section)
        if (section->name == section_name) // if section found
            return section; // return the section iterator

    return NULL; // section not found
}


////////////////////////////////////////////////////////////////////////
// find_memory_region() finds an existing region in the
// memory region list

list <mem_region>::iterator mem_map::find_memory_region (std::string region_name)
{
    for (list <mem_region>::iterator region = region_list.begin (); region != region_list.end (); ++region)
        if (region->name == region_name) // if region found
            return region; // return the region iterator

    return NULL; // region not found
}


////////////////////////////////////////////////////////////////////////
// delete_memory_section() removes an existing item from the
// memory section map

bool mem_map::delete_memory_section (std::string name)
{
    // make sure that the section exists

    list <mem_section>::iterator section = find_memory_section (name);
    if (section == NULL)
        return false; // there is no section with this name

/*
    // make sure that there are no sections defined relative to this section before deleting it

    if (section->initial_location->following_section != NULL)
        return false;

    if (section->final_location->following_section != NULL)
        return false;
*/

    // if section is absolute, copy the initial and final location information to
    // the following sections (if any)

    if ((section->initial_location->anchor == absolute) && (section->initial_location->following_section != NULL))
    {
        section->initial_location->following_section->initial_location->anchor = absolute;
        section->initial_location->following_section->initial_location->address = section->initial_location->address;
        // FIXME adjust new address of following section for alignment here
    }

    if ((section->final_location->anchor == absolute) && (section->final_location->following_section != NULL))
    {
        section->final_location->following_section->final_location->anchor = absolute;
        section->final_location->following_section->final_location->address = section->final_location->address;
        // FIXME adjust new address of following section for alignment here
    }

    // if section is relative, find the initial and final sections to which it is attached
    // and set their pointers to the sections following the one to be deleted (if any)

    list <mem_section>::iterator related_section;

    if (section->initial_location->anchor == relative)
        for (related_section = section_list.begin (); related_section != section_list.end (); ++related_section)
            if (related_section->initial_location->following_section == section)
                related_section->initial_location->following_section = section->initial_location->following_section;

    if (section->final_location->anchor == relative)
        for (related_section = section_list.begin (); related_section != section_list.end (); ++related_section)
            if (related_section->final_location->following_section == section)
                related_section->final_location->following_section = section->final_location->following_section;

    // delete the section

    delete section->initial_location;
    section->initial_location = NULL;

    delete section->final_location;
    section->final_location = NULL;

    section_list.erase (section);

    // recalculate section lists for all regions

    calc_section_lists ();

	map_modified_flag = true;
    return true;
}


////////////////////////////////////////////////////////////////////////
// delete_memory_sections() deletes all memory sections in preparation
// for layout loading or application closure

bool mem_map::delete_all_memory_sections ()
{
    // deleteZ(each section in turn)

    while (section_list.size () > 0)
    {
        list <mem_section>::iterator section = section_list.begin ();
        delete section->initial_location;
        section->initial_location = NULL;

        delete section->final_location;
        section->final_location = NULL;

        section_list.erase (section);
    }
//    section_list.clear ();

    // recalculate section view lists for all regions

    calc_section_lists ();

	map_modified_flag = true;
    return true;
}


////////////////////////////////////////////////////////////////////////
// export_sections() exports section-related info for regions of the
// specified type to the linker script fragment and header file

bool mem_map::export_sections (FILE * script_stream, FILE * header_stream, mem_type type)
{
    for (list <mem_region>::iterator region = region_list.begin (); region != region_list.end(); ++region)
        if (region->type == type)
    {
        for (list <mem_section_view>::iterator section_view = region->section_view_list.begin (); section_view != region->section_view_list.end (); ++section_view)
        {
            if ((section_view->section != NULL) && (section_view->section_location != initial_location))
            {
                if (section_view->section->linker_defined) // section is linker-defined
                {
                    // output section name and region name
                    fprintf (script_stream, "    SECTION_%s (%s, ",
                        encode_section_name (section_view->section->name).c_str (), region->name.c_str ());

                    // output VMA
                    if (section_view->section->final_location->anchor == absolute) // an absolute VMA
                        fprintf (script_stream, "%#lx, ", section_view->section->final_location->address); // specify absolute address
                    else // a relative VMA
                        fprintf (script_stream, "ALIGN (%#lx), ", section_view->section->alignment); // specify alignment

                    // output LMA
                    if (! section_view->section->relocates) // section does not relocate so LMA == VMA
                        fprintf (script_stream, "LMA_EQ_VMA)");
                    else if (section_view->section->initial_location->anchor == absolute) // an absolute LMA
                        fprintf (script_stream, "AT (%#lx))", section_view->section->initial_location->address);
                    else // a relative LMA
                    {
                        list <mem_section>::iterator parent_section;
                        for (parent_section = section_list.begin (); parent_section != section_list.end (); ++parent_section)
                            if (parent_section->initial_location->following_section == section_view->section)
                                break;

                        if (parent_section->linker_defined) // parent section is linker-defined
                            fprintf (script_stream, "FOLLOWING (.%s))", parent_section->name.c_str ());
                        else // parent section is user-defined
                            fprintf (script_stream, "AT (__%s + %#lx))", parent_section->name.c_str (), parent_section->size);
                    }
                }
                else // section is user-defined
                {
                    // output section symbol
                    if (section_view->section->final_location->anchor == absolute) // an absolute VMA
                        fprintf (script_stream, "    CYG_LABEL_DEFN(__%s) = %#lx;", section_view->section->name.c_str (), section_view->section->final_location->address);
                    else // a relative VMA
                        fprintf (script_stream, "    CYG_LABEL_DEFN(__%s) = ALIGN (%#lx);", section_view->section->name.c_str (), section_view->section->alignment);

                    // update current location pointer
                    if (section_view->section->size != 0) // size is known
                        fprintf (script_stream, " . = CYG_LABEL_DEFN(__%s) + %#lx;", section_view->section->name.c_str (), section_view->section->size);

                    // output reference to symbol in header file
                    fprintf (header_stream, "#ifndef __ASSEMBLER__\nextern char CYG_LABEL_NAME (__%s) [];\n#endif\n", section_view->section->name.c_str ());
                    fprintf (header_stream, "#define CYGMEM_SECTION_%s (CYG_LABEL_NAME (__%s))\n", section_view->section->name.c_str (), section_view->section->name.c_str ());
                    if (section_view->section->size == 0) // a section of unknown size
                    {
                        mem_address section_end_address;

                        ++section_view; // move to next section_view
                        if (section_view == region->section_view_list.end ()) // section continues to end of region
                            section_end_address = region->address + region->size;
                        else // section continues to next section with an absolute location
                            section_end_address = section_view->section->final_location->address;
                        --section_view; // move back to previous section view

                        fprintf (header_stream, "#define CYGMEM_SECTION_%s_SIZE (%#lx - (size_t) CYG_LABEL_NAME (__%s))\n", section_view->section->name.c_str (), section_end_address, section_view->section->name.c_str ());
                    }
                    else // a section of known size
                        fprintf (header_stream, "#define CYGMEM_SECTION_%s_SIZE (%#lx)\n", section_view->section->name.c_str (), section_view->section->size);
                }

                // end of section description

                fprintf (script_stream, "\n"); // new line
            }
        }
    }
    return true;
}


////////////////////////////////////////////////////////////////////////
// export_files() creates a fragment of linker script and a header file
// describing the memory layout

bool mem_map::export_files (const wxChar* script_name, const wxChar* header_name)
{
    FILE * script_stream;
    FILE * header_stream;
    list <mem_region>::iterator region;

	// do not export files if the memory layout is empty
	// assume that there are default LDI files available

	if (region_list.size () == 0)
		return false;

    // open the script fragment file for writing

    script_stream = _tfopen (script_name, _T("wt"));
    if (script_stream == NULL)
        return false;

    // open the header file for writing

    header_stream = _tfopen (header_name, _T("wt"));
    if (header_stream == NULL)
    {
        fclose (script_stream);
        return false;
    }

    // output the linker script fragment header

    time_t export_time;
    time (&export_time);
    struct tm * local = localtime (&export_time);
    fprintf (script_stream, "// eCos memory layout - %s\n%s\n\n", asctime (local), MLT_GENERATED_WARNING);
    fprintf (script_stream, "#include <cyg/infra/cyg_type.inc>\n\n");

    // output the header file header

    fprintf (header_stream, "// eCos memory layout - %s\n%s\n\n", asctime (local), MLT_GENERATED_WARNING);
	fprintf (header_stream, "#ifndef __ASSEMBLER__\n");
	fprintf (header_stream, "#include <cyg/infra/cyg_type.h>\n"); // for the CYG_LABEL_NAME macro definition
	fprintf (header_stream, "#include <stddef.h>\n\n"); // for size_t
	fprintf (header_stream, "#endif\n");

    // output the MEMORY block

    fprintf (script_stream, "MEMORY\n{\n"); // start of MEMORY block
    for (region = region_list.begin (); region != region_list.end(); ++region)
    {
        fprintf (script_stream, "    %s : ORIGIN = %#lx, LENGTH = %#lx\n", region->name.c_str(), region->address, region->size);
        fprintf (header_stream, "#define CYGMEM_REGION_%s (%#lx)\n", region->name.c_str(), region->address);
        fprintf (header_stream, "#define CYGMEM_REGION_%s_SIZE (%#lx)\n", region->name.c_str(), region->size);
        fprintf (header_stream, "#define CYGMEM_REGION_%s_ATTR (CYGMEM_REGION_ATTR_R%s)\n", region->name.c_str(), (read_write == region->type) ? " | CYGMEM_REGION_ATTR_W" : "");
    }
    fprintf (script_stream, "}\n\n"); // end of MEMORY block

    // output the SECTIONS block

    fprintf (script_stream, "SECTIONS\n{\n"); // start of SECTIONS block
    fprintf (script_stream, "    SECTIONS_BEGIN\n"); // SECTIONS block initial script macro call
    export_sections (script_stream, header_stream, read_only); // export sections in read-only regions first
    export_sections (script_stream, header_stream, read_write); // followed by sections in read-write regions
    fprintf (script_stream, "    SECTIONS_END\n"); // SECTIONS block final script macro call
    fprintf (script_stream, "}\n"); // end of SECTIONS block

    // close the files

    fclose (script_stream);
    fclose (header_stream);

    return true;
}


////////////////////////////////////////////////////////////////////////
// import_linker_defined_sections() reads a the linker-defined section
// names from the "SECTION_*" CPP macro definitions within the linker
// script

bool mem_map::import_linker_defined_sections (const wxChar* filename)
{
    // clear the linker-defined section name list

    linker_defined_section_list.clear ();

    // open the linker script file for reading

    FILE * stream;
    stream = _tfopen (filename, _T("rt"));
    if (stream == NULL)
        return false;

    bool macro = false; // not reading a CPP macro definition initially
    char input_string [32];
    while (! feof (stream))
    {
        if (macro)
        {
            if (fscanf (stream, "%8s", input_string) == EOF) // read the next 8 chars (not including whitespace)
                break;

            if (strcmp (input_string, "SECTION_") == 0) // an MLT section macro definition
            {
                if (fscanf (stream, "%31[^(]", input_string) == EOF) // read the section name up to the '(' character
                    break;

                std::string section_name = decode_section_name (input_string);
                if (find (linker_defined_section_list.begin (), linker_defined_section_list.end (), section_name) == linker_defined_section_list.end ()) // if section name is unique
                    linker_defined_section_list.push_back (section_name);
            }

            macro = false;
        }

        else
        {
            if (fscanf (stream, "%31s", input_string) == EOF)
                break;

            if (strcmp (input_string, "#define") == 0)
                macro = true; // macro starts with "#define"
        }

    }

    // close the file

    if (fclose (stream))
        return false;

    return true;
}

////////////////////////////////////////////////////////////////////////
// encode_note() encodes newlines in note

std::string mem_map::encode_note (std::string in)
{
    std::string out = "!"; // dummy first character to ensure output string length > 0

    for (unsigned int item = 0; item < in.size (); item++)
        if (in [item] == _TCHAR('\n')) // an LF character
            out += "\x07F"; // output substitution character 0x7F instead
        else if (in [item] != _TCHAR('\r')) // ignore the CR (present under Win32 only)
            out += in [item]; // copy other characters to output string unprocessed

    return out;
}

////////////////////////////////////////////////////////////////////////
// decode_note() decodes newlines in note

std::string mem_map::decode_note (std::string in)
{
    std::string out;

    for (unsigned int item = 1; item < in.size (); item++) // ignore dummy first character
        if (in [item] == _TCHAR('\x07F')) // the newline substitution character
            out += "\r\n"; // output CRLF instead
        else
            out += in [item];

    return out;
}

////////////////////////////////////////////////////////////////////////
// encode_section_name() encodes period -> double underscore in section name

std::string mem_map::encode_section_name (std::string in)
{
    std::string out;

    for (unsigned int item = 0; item < in.size (); item++)
        if (in [item] == '.') // a period character
			out += "__"; // output a double underscore instead
        else
            out += in [item];

    return out;
}

////////////////////////////////////////////////////////////////////////
// decode_section_name() decodes double underscore -> period in section name

std::string mem_map::decode_section_name (std::string in)
{
    std::string out;

    for (unsigned int item = 0; item < in.size (); item++)
        if ((item + 1 < in.size ()) && (in [item] == '_') && (in [item + 1] == '_')) // two consecutive underscore characters
		{
			out += "."; // output a period instead
			item++; // skip the second underscore
		}
        else
            out += in [item];

    return out;
}

////////////////////////////////////////////////////////////////////////
// save_memory_layout() saves the memory layout to file for later use

bool mem_map::save_memory_layout (const wxChar* filename)
{
    FILE * stream;
    list <mem_region>::iterator region;

    // open the save file for writing

    stream = _tfopen (filename, _T("wt"));
    if (stream == NULL)
        return false;

    // write the save file format version number

	fprintf (stream, "version %u\n", (unsigned int) MLT_FILE_VERSION);

    // save the memory region data in address order

    for (region = region_list.begin (); region != region_list.end (); ++region)
        fprintf (stream, "region %s %lx %lx %d %s\n", region->name.c_str (),
            region->address, region->size, (region->type == read_only), encode_note (region->note).c_str ());

    // save the memory section data in VMA order

    for (region = region_list.begin (); region != region_list.end(); ++region)
    {
        for (list <mem_section_view>::iterator section_view = region->section_view_list.begin (); section_view != region->section_view_list.end (); ++section_view)
        {
            if ((section_view->section != NULL) && (section_view->section_location != initial_location))
            {
                list <mem_section>::iterator section = section_view->section;
                fprintf (stream, "section %s %lx %lx %d %d %d %d %d %d",
                    section->name.c_str (), section->size, section->alignment,
                    section->relocates, section->linker_defined,
                    section->final_location->anchor == absolute,
                    section->final_location->following_section != NULL,
                    section->initial_location->anchor == absolute,
                    section->initial_location->following_section != NULL);

                if (section->final_location->anchor == absolute)
                    fprintf (stream, " %lx", section->final_location->address);

                if (section->initial_location->anchor == absolute)
                    fprintf (stream, " %lx", section->initial_location->address);

                if (section->final_location->following_section != NULL)
                    fprintf (stream, " %s", section->final_location->following_section->name.c_str ());

                if (section->initial_location->following_section != NULL)
                    fprintf (stream, " %s", section->initial_location->following_section->name.c_str ());

                fprintf (stream, " %s", encode_note (section->note).c_str ());

                // end of section description

                fprintf (stream, "\n"); // new line
            }
        }
    }

    // close the file

    if (fclose (stream))
        return false;

	map_modified_flag = false;
    return true;
}


////////////////////////////////////////////////////////////////////////
// load_memory_layout() loads a previously saved memory layout from file

bool mem_map::load_memory_layout (const wxChar* filename)
{
    FILE * stream;

    // open the save file for reading

    stream = _tfopen (filename, _T("rt"));
    if (stream == NULL)
        return false;

	// read the file version

	unsigned int file_version;
	if ((fscanf (stream, "%*s %u", &file_version) != 1) ||
		(file_version != MLT_FILE_VERSION))
	{
		fclose (stream); // missing or incorrect file version
        return false;
	}

    new_memory_layout ();

    // read the new memory layout (first pass)

    while (! feof (stream))
    {
        char record_type [32];
        if (fscanf (stream, "%31s", record_type) == EOF)
            break;

        if (strcmp (record_type, "section") == 0) // a section record
        {
            if (! load_memory_section_1 (stream))
                break;
        }
        else if (strcmp (record_type, "region") == 0) // a region record
        {
            mem_address address, size;
            bool read_only_region;
            char name [32];
            char note [1024];

            fscanf (stream, "%s %lx %lx %d %1023[^\n]", name, &address, &size, &read_only_region, note);

            if (create_memory_region (name, address, size, (read_only_region ? read_only : read_write), decode_note (note)))
                break;
        }   
        else // an unknown record type
            break;
    }

	// quit if the end of the file was not reached (due to an error)

	if (! feof (stream))
	{
		new_memory_layout ();
		fclose (stream);
		return false;
	}

    // move the file pointer back to the beginning of the file

    fseek (stream, 0, SEEK_SET);

    while (! feof (stream)) // read the memory layout (second pass)
    {
        char record_type [32];
        if (fscanf (stream, "%31s", record_type) == EOF)
            break;

        if ((strcmp (record_type, "section") == 0) && (! load_memory_section_2 (stream)))
            break;
    }
    
    // close the file

    if (fclose (stream))
	{
		new_memory_layout ();
        return false;
	}

    // recalculate section view lists for all regions

    calc_section_lists ();

	map_modified_flag = false;
    return true;
}


////////////////////////////////////////////////////////////////////////
// load_memory_section_1() loads a previously saved memory section from
// file (first pass)

bool mem_map::load_memory_section_1 (FILE * stream)
{
    char section_name [32];
    int relocates, linker_defined;
    int final_absolute, initial_absolute, final_following, initial_following;
    mem_section new_section;

    new_section.initial_location = new mem_location;
    new_section.initial_location->following_section = NULL;
    new_section.final_location = new mem_location;
    new_section.final_location->following_section = NULL;
      
    fscanf (stream,"%31s %lx %lx %d %d %d %d %d %d",
        section_name, &new_section.size, &new_section.alignment,
        &relocates, &linker_defined, &final_absolute, &final_following,
        &initial_absolute, &initial_following);

    new_section.name = section_name;
    new_section.relocates = (relocates != 0);
    new_section.linker_defined = (linker_defined != 0);

    new_section.final_location->anchor = (final_absolute ? absolute : relative);
    if (final_absolute) // final location is absolute
        fscanf (stream, "%lx", &new_section.final_location->address);

    new_section.initial_location->anchor = (initial_absolute ? absolute : relative);
    if (initial_absolute) // initial location is absolute
        fscanf (stream, "%lx", &new_section.initial_location->address);

    if (final_following)
        fscanf (stream, "%*s"); // skip the final following section field on first pass

    if (initial_following)
        fscanf (stream, "%*s"); // skip the initial following section field on first pass

    char note [1024];
    fscanf (stream, " %1023[^\n]", note);
    new_section.note = decode_note (note);

    // add the new section to the section map

    section_list.push_front (new_section);

    return true;
}


////////////////////////////////////////////////////////////////////////
// load_memory_section_2() loads a previously saved memory section from
// file (second pass)

bool mem_map::load_memory_section_2 (FILE * stream)
{
    char section_name [32];
    char following_section_name [32];
    int final_absolute, initial_absolute, final_following, initial_following;

    fscanf (stream,"%31s %*lx %*lx %*d %*d %d %d %d %d",
        section_name, &final_absolute, &final_following,
        &initial_absolute, &initial_following);

    if (final_absolute) // final location is absolute
        fscanf (stream, "%*lx"); // skip the final location

    if (initial_absolute) // initial location is absolute
        fscanf (stream, "%*lx"); // skip the initial location

    if (initial_following || final_following) // the section is a parent
    {
        list <mem_section>::iterator section = find_memory_section (section_name);

        if (final_following)
        {
            fscanf (stream, "%31s", following_section_name); // read the final following section name
            section->final_location->following_section =
                find_memory_section (following_section_name);
        }

        if (initial_following)
        {
            fscanf (stream, "%31s", following_section_name); // read the initial following section name
            section->initial_location->following_section =
                find_memory_section (following_section_name);
        }
    }

    fscanf (stream, "%*1023[^\n]"); // skip the note

    return true;
}


////////////////////////////////////////////////////////////////////////
// new_memory_layout() clears the memory layout

bool mem_map::new_memory_layout ()
{
    delete_all_memory_sections ();
//    section_list.clear ();
    region_list.clear ();

	map_modified_flag = false; // no need to save an empty memory layout
    return true;
}


////////////////////////////////////////////////////////////////////////
// section_exists() determines if the specified section is defined

bool mem_map::section_exists (std::string section_name)
{
    return (find_memory_section (section_name) != NULL);
}


////////////////////////////////////////////////////////////////////////
// find_region_by_address() finds the region containing the specified
// memory address

list <mem_region>::iterator mem_map::find_region_by_address (mem_address address)
{
    for (list <mem_region>::iterator region = region_list.begin (); region !=region_list.end(); ++region)
        if ((address >= region->address) && (address < region->address + region->size))
            return region;

    return NULL; // the specified address is not in a memory region
}


////////////////////////////////////////////////////////////////////////
// find_region_by_section() finds the region containing the specified
// section

list <mem_region>::iterator mem_map::find_region_by_section (list <mem_section>::iterator section, section_location_type location_type)
{
    for (list <mem_region>::iterator region = region_list.begin (); region !=region_list.end(); ++region)
        for (list <mem_section_view>::iterator section_view = region->section_view_list.begin (); section_view != region->section_view_list.end (); ++section_view)
            if ((section_view->section != NULL) && (section_view->section == section) &&
                (section_view->section_location == (section_view->section->relocates ? location_type : fixed_location)))
                return region;

    return NULL; // the specified section location type was not found (you probably searched for the fixed_location of a relocating section)
}
