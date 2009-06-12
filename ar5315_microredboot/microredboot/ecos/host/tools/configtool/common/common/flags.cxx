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
//==========================================================================
//
//      flags.cxx
//
//      The implementation of build flags extraction from CDL data
//
//==========================================================================
//==========================================================================
//#####DESCRIPTIONBEGIN####                                             
//
// Author(s):           jld
// Date:                1999-11-08
//
//####DESCRIPTIONEND####
//==========================================================================

#include "flags.hxx"
#include <cctype>

static const std::string GLOBAL_FLAGS_PREFIX = "CYGBLD_GLOBAL_";
static const std::string ADD_FLAGS_SUFFIX    = "_ADD";
static const std::string REMOVE_FLAGS_SUFFIX = "_REMOVE";

// convert a string of space-separated values into a list of strings
static void string_to_list (std::string input, std::list <std::string> & output) {
	std::string item;
	bool space = true;
	output.clear ();

	for (unsigned int n = 0; n < input.size (); n++) { // for each char in the string
		if (isspace (input [n])) { // if char is a space
			if (! space) { // if previous char not a space
				output.push_back (item); // add item to output list
				item.erase (); // clear item string
				space = true;
			}
		} else { // char is not a space
			item += input [n]; // add char to item string
			space = false;
		}
	}

	if (! space) { // if final char not a space
		output.push_back (item); // add final item to output list
	}
}

// convert a list of strings into a string of space-separated values
static std::string list_to_string (std::list <std::string> & input) {
	std::string output;

	for (std::list <std::string>::iterator item = input.begin (); item != input.end (); item++) { // for each item in the list
		output += * item + " "; // add item to output string followed by a space
	}

	if (! output.empty ()) { // if the output string is not empty
		output.resize (output.size () - 1); // remove the trailing space
	}

	return output;
}

// return the build flags of the specified category
std::string get_flags (const CdlConfiguration config, const CdlBuildInfo_Loadable * build_info, const std::string flag_category) {
	std::list <std::string> flags_list;

	CdlValuable global_flags = dynamic_cast <CdlValuable> (config->lookup (GLOBAL_FLAGS_PREFIX + flag_category));
	if (global_flags) { // if there are global flags
		string_to_list (global_flags->get_value (), flags_list);
	}

	if (build_info) { // if the caller requires flags specific to a particular loadable
		// process flags to be removed
		CdlValuable remove_flags = dynamic_cast <CdlValuable> (config->lookup (build_info->name + "_" + flag_category + REMOVE_FLAGS_SUFFIX));
		if (remove_flags) { // if there are flags to be removed
			std::list <std::string> remove_flags_list;
			string_to_list (remove_flags->get_value (), remove_flags_list);
			for (std::list <std::string>::iterator item = remove_flags_list.begin (); item != remove_flags_list.end (); item++) {
				flags_list.remove (* item); // remove the flag from the list
			}
		}

		// process flags to be added
		CdlValuable add_flags = dynamic_cast <CdlValuable> (config->lookup (build_info->name + "_" + flag_category + ADD_FLAGS_SUFFIX));
		if (add_flags) { // if there are flags to be added
			std::list <std::string> add_flags_list;
			string_to_list (add_flags->get_value (), add_flags_list);
			flags_list.splice (flags_list.end (), add_flags_list); // splice the additional flags onto the end of the list
		}
	}

	return list_to_string (flags_list);
}
