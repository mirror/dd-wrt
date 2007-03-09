//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 2003 Bart Veer
// Copyright (C) 1998, 1999, 2000, 2002 Red Hat, Inc.
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
//      build.cxx
//
//      The implementation of build tree and makefile generation using
//      CDL data
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

#if defined(_WIN32) || defined(__CYGWIN__)
	#include <windows.h> /* for GetShortPathNameA() */
#endif
#ifdef __CYGWIN__
	#include <sys/cygwin.h> /* for cygwin_conv_to_posix_path() */
#endif
#ifdef __WXMSW__
// We take advantage of wxWindows' recursive wxFileName::Mkdir function
// to workaround a bug in Tcl on Windows 9x
#include "wx/filename.h"
#  ifdef new
#    undef new
#  endif
#endif
#include "flags.hxx"
#include "build.hxx"

// Two methods of generating Cygwin filenames
// ECOS_USE_CYGDRIVE = 0: use e.g. //c/, but this is deprecated in new versions of Cygwin
// ECOS_USE_CYGDRIVE = 1: use e.g. /cygdrive/c/
// ECOS_USE_CYGDRIVE = 2: use e.g. c:/ notation
// ECOS_USE_CYGDRIVE = 3: use e.g. /ecos-x notation where x is a drive name.
#define ECOS_USE_CYGDRIVE 3

// Use registry functions to find out location of /cygdrive
#define ECOS_USE_REGISTRY 1

// Don't use Reg... functions in command-line version until we know how
// to add advapi32.lib
#if defined(_WIN32) && !defined(__WXMSW__) && !defined(ECOS_CT)
#undef ECOS_USE_REGISTRY
#define ECOS_USE_REGISTRY 0
#endif

std::string makefile_header = "# eCos makefile\n\n# This is a generated file - do not edit\n\n";

// This code seems to crash Tcl under Windows ME and Linux, so
// disable for now. What should the criterion be? TCL version?
#define SET_STDOUT_TO_NULL 0

bool eval_tcl_command (const std::string command) {
	Tcl_Interp * interp = Tcl_CreateInterp ();
#if SET_STDOUT_TO_NULL
	Tcl_Channel outchan = Tcl_OpenFileChannel (interp, "nul", "a+", 777);
	Tcl_SetStdChannel (outchan, TCL_STDOUT); // direct standard output to the null device
#endif
	int nStatus = Tcl_Eval (interp, CDL_TCL_CONST_CAST(char*, command.c_str()));
#if SET_STDOUT_TO_NULL
	Tcl_SetStdChannel (NULL, TCL_STDOUT);
	Tcl_UnregisterChannel (interp, outchan);
#endif
	Tcl_DeleteInterp (interp);
	return (TCL_OK == nStatus);
}

// generate a copy of a string where each occurance of a specified char is replaced with another char
std::string replace_char (const std::string input, const char old_char, const char new_char) {
	std::string output;
	for (unsigned int n = 0; n < input.size (); n++) { // for each char
		output += (old_char == input [n]) ? new_char : input [n]; // convert during copy
	}
	return output;
}

// convert a filepath into a vector of path components
static void path_to_vector (std::string input, std::vector <std::string> & output) {
	std::string component;
	output.clear ();

	for (unsigned int n = 0; n < input.size (); n++) { // for each char in the path
		if (('/' == input [n]) || ('\\' == input [n])) { // if char is a directory separator
			output.push_back (component); // add path component to output vector
			component.erase (); // clear path component string
		} else { // char is not a separator
			component += input [n]; // add char to path component string
		}
	}
	output.push_back (component); // add final path component to output vector
}

// eliminate spaces from a DOS filepath by substituting the
// short form of path components containing spaces
#if defined(_WIN32) || defined(__CYGWIN__)
std::string nospace_path (const std::string input) {
	// split the path into a vector of path components
	std::vector <std::string> long_path_vector;
	path_to_vector (input, long_path_vector);

	// convert the path to its short form and split
	// the result into a vector of path components
	char buffer [MAX_PATH + 1];
	GetShortPathNameA (input.c_str (), buffer, sizeof (buffer));
	std::vector <std::string> short_path_vector;
	path_to_vector (buffer, short_path_vector);

	// append the short or long form of each path component to the output string as appropriate
	std::string output;
	for (unsigned int n = 0; n < long_path_vector.size (); n++) { // for each component of the path
		if (long_path_vector [n].end () != std::find (long_path_vector [n].begin (), long_path_vector [n].end (), ' ')) { // if there is a space in the path component
			output += short_path_vector [n]; // add the short form of the path component
		} else { // there is no space in the path component
			output += long_path_vector [n]; // add the long form of the path component
		}
		output += '\\'; // add a directory separator
	}
	output.resize (output.size () - 1); // remove the trailing separator

	return output;
}
#endif

// convert a DOS filepath to a Cygwin filepath
std::string cygpath (const std::string input) {
#if defined(_WIN32) || defined(__CYGWIN__)
	// remove spaces from the DOS filepath
	const std::string path = nospace_path (input);
	std::string output;

	// convert the DOS filepath to Cygwin notation - using Cygwin if available
    // 2001-10-15: should now do the same thing under Cygwin as under VC++, namely
    // use the /ecos-x form.
#if 0 // def __CYGWIN__
	char buffer [MAX_PATH + 1];
	cygwin_conv_to_posix_path (path.c_str (), buffer);
	output = buffer;
#else

#if ECOS_USE_CYGDRIVE == 1
    std::string strCygdrive("/cygdrive");

#if ECOS_USE_REGISTRY
    HKEY hKey = 0;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Cygnus Solutions\\Cygwin\\mounts v2",
        0, KEY_READ, &hKey))
    {
        DWORD type;
        BYTE value[256];
        DWORD sz;
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, "cygdrive prefix", NULL, & type, value, & sz))
        {
            strCygdrive = (const char*) value;
        }

        RegCloseKey(hKey);
    }
#endif
    strCygdrive = strCygdrive + "/";

	for (unsigned int n = 0; n < path.size (); n++) { // for each char
		if ((1 == n) && (':' == path [n])) { // if a DOS logical drive letter is present
			output = strCygdrive + output; // convert to Cygwin notation
		} else {
			output += ('\\' == path [n]) ? '/' : path [n]; // convert backslash to slash
		}
	}
#elif ECOS_USE_CYGDRIVE == 2
    // Convert to c:/foo/bar notation
	for (unsigned int n = 0; n < path.size (); n++) { // for each char
			output += ('\\' == path [n]) ? '/' : path [n]; // convert backslash to slash
	}
#elif ECOS_USE_CYGDRIVE == 3
    // Convert to /ecos-x notation, assuming that this mount point will be created
    // by the application.

    std::string output1;

    if (path.size() > 1 && path[1] == ':')
    {
        output1 = "/ecos-";
        output1 += tolower(path[0]);
        output1 += "/";

        // Append the rest of the path
        if (path.size() > 2)
        {
            unsigned int n = 2;
            unsigned int i;

            if (path[n] == '\\' || path[n] == '/')
                n ++;
            
            for (i = n; i < path.size(); i++)
                output1 += path[i];
        }
    }
    else
        output1 = path;

    for (unsigned int n = 0; n < output1.size (); n++) { // for each char
    	output += ('\\' == output1 [n]) ? '/' : output1 [n]; // convert backslash to slash
	}
#else
	for (unsigned int n = 0; n < path.size (); n++) { // for each char
		if ((1 == n) && (':' == path [n])) { // if a DOS logical drive letter is present
			output = "//" + output; // convert to Cygwin notation
		} else {
			output += ('\\' == path [n]) ? '/' : path [n]; // convert backslash to slash
		}
	}
#endif
    // ECOS_USE_CYGDRIVE
#endif
	return output;
#else
	return input;
#endif
}

// create a directory
bool create_directory (const std::string directory) {
// We take advantage of wxWindows' recursive wxFileName::Mkdir function
// to workaround a bug in Tcl on Windows 9x
#if defined(__WXMSW__)
    if (wxDirExists(directory.c_str()))
        return TRUE;
    return wxFileName::Mkdir(directory.c_str(), 0777, TRUE);
#else
    return eval_tcl_command ("file mkdir \"" + directory + "\"");
#endif
}

// copy a file
bool copy_file (const std::string file, const std::string destination) {
	return eval_tcl_command ("file copy \"" + file + "\" \"" + destination + "\"");
	return true;
}

// returns the directory of the specified file
std::string file_to_directory (const std::string file) {
	for (unsigned int n = file.size (); n >= 0; n--) {
		if ('/' == file [n]) {
			std::string directory = file;
			directory.resize (n);
			return directory;
		}
	}
	return "";
}

std::string tab_indent (const std::string input) {
	std::string output;
	bool indent = true;
	for (unsigned int n = 0; n < input.size (); n++) {
		if (indent) {
			output += '\t';
			indent = false;
		} else {
			indent = ('\n' == input [n]);
		}
		output += input [n];
	}
	return output;
}

// return the tests of the specified loadable
std::string get_tests (const CdlConfiguration config, const CdlBuildInfo_Loadable & build_info) {
	CdlValuable tests = dynamic_cast <CdlValuable> (config->lookup (build_info.name + "_TESTS"));
    // check if there are tests active and enabled
	if (tests && tests->is_active() && tests->is_enabled()) {
		return tests->get_value ();
	} else { // there are no tests
		return "";
	}
}

// replaces all occurances of a substring with a new substring
std::string replace_substr (const std::string input, const std::string old_substring, const std::string new_substring) {
	std::string output = input;
	std::string::size_type index = 0;
	while (index = output.find (old_substring, index), std::string::npos != index) {
		output.replace (index, old_substring.size (), new_substring);
	}
	return output;
}

// resolve tokens in custom make rule targets and dependencies
std::string resolve_tokens (const std::string input) {
	std::string output = input;
	output = replace_substr (output, "<PREFIX>", "$(PREFIX)");
	output = replace_substr (output, "<PACKAGE>", "$(REPOSITORY)/$(PACKAGE)");
	return output;
}

// create the makefile for a loadable
bool generate_makefile (const CdlConfiguration config, const CdlBuildInfo_Loadable & info, const std::string install_tree, const std::string filename) {
	unsigned int count;
	unsigned int library;

	// obtain the command prefix
	std::string command_prefix = get_flags (config, NULL, "COMMAND_PREFIX");
	if (! command_prefix.empty ()) { // if there is a command prefix
		command_prefix += '-'; // add a trailing hyphen
	}

	// generate the prefix for archived objects
	unsigned int final_separator = 0; // the index of the last directory separator
	std::string object_prefix = info.directory; // start with the loadable directory
	for (count = 0; count < object_prefix.size (); count++) { // for each char
		if ('/' == object_prefix [count]) { // if the char is a directory separator
			object_prefix [count] = '_'; // replace the char with an underscore
			final_separator = count;
		}
	}
	object_prefix.resize (final_separator); // remove the version directory
	
	// open the makefile
	FILE * stream = fopen (filename.c_str (), "wt");
	if (stream == NULL) // if the file could not be opened
		return false;

	// generate the header
	fprintf (stream, makefile_header.c_str ());

	// generate the global variables
	fprintf (stream, "export PREFIX := %s\n", cygpath (install_tree).c_str ());
	fprintf (stream, "export COMMAND_PREFIX := %s\n", command_prefix.c_str ());
	fprintf (stream, "export CC := $(COMMAND_PREFIX)gcc\n");
	fprintf (stream, "export OBJCOPY := $(COMMAND_PREFIX)objcopy\n");
#if defined(_WIN32) || defined(__CYGWIN__)
    fprintf (stream, "export HOST := CYGWIN\n");
#else
    fprintf (stream, "export HOST := UNIX\n");
#endif
	fprintf (stream, "export AR := $(COMMAND_PREFIX)ar\n\n");

	// generate the package variables
	fprintf (stream, "export REPOSITORY := %s\n", cygpath (info.repository).c_str());
	fprintf (stream, "PACKAGE := %s\n", info.directory.c_str ());
	fprintf (stream, "OBJECT_PREFIX := %s\n", object_prefix.c_str ());
	fprintf (stream, "CFLAGS := %s\n", get_flags (config, &info, "CFLAGS").c_str ());
	fprintf (stream, "LDFLAGS := %s\n", get_flags (config, &info, "LDFLAGS").c_str ());
	fprintf (stream, "VPATH := $(REPOSITORY)/$(PACKAGE)\n");
	fprintf (stream, "INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.\n");
	fprintf (stream, "MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)\n");
	fprintf (stream, "TESTS := %s\n\n", get_tests (config, info).c_str ());

	// create a vector of libraries
	std::vector <std::string> library_vector;
	for (count = 0; count < info.compiles.size (); count++) { // for each compilable file
		const std::string & library = info.compiles [count].library;
		if (library_vector.end () == std::find (library_vector.begin (), library_vector.end (), library)) { // if a new library
			library_vector.push_back (library); // add the library to the vector
		}
	}
	for (count = 0; count < info.objects.size (); count++) { // for each object file
		const std::string & library = info.objects [count].library;
		if (library_vector.end () == std::find (library_vector.begin (), library_vector.end (), library)) { // if a new library
			library_vector.push_back (library); // add the library to the vector
		}
	}
	for (count = 0; count < info.make_objects.size (); count++) { // for each make object
		const std::string & library = info.make_objects [count].library;
		if (library_vector.end () == std::find (library_vector.begin (), library_vector.end (), library)) { // if a new library
			library_vector.push_back (library); // add the library to the vector
		}
	}

	// generate the default rule
	fprintf (stream, "build: headers");
	for (library = 0; library < library_vector.size (); library++) { // for each library
		fprintf (stream, " %s.stamp", library_vector [library].c_str ());
	}
	fprintf (stream, "\n\n");

	// generate library rules
	for (library = 0; library < library_vector.size (); library++) { // for each library
		fprintf (stream, "LIBRARY := %s\n", library_vector [library].c_str ());
		fprintf (stream, "COMPILE :=");
		for (count = 0; count < info.compiles.size (); count++) { // for each compilable file
			if (library_vector [library] == info.compiles [count].library) { // if the file and library are related
				fprintf (stream, " %s", info.compiles [count].source.c_str ());
			}
		}
		for (count = 0; count < info.objects.size (); count++) { // for each object file
			if (library_vector [library] == info.objects [count].library) { // if the file and library are related
				fprintf (stream, " %s", info.objects [count].object.c_str ());
			}
		}
		for (count = 0; count < info.make_objects.size (); count++) { // for each make object
			if (library_vector [library] == info.make_objects [count].library) { // if the object and library are related
				fprintf (stream, " %s", info.make_objects [count].object.c_str ());
			}
		}

		fprintf (stream, "\nOBJECTS := $(COMPILE:.cxx=.o.d)\n");
		fprintf (stream, "OBJECTS := $(OBJECTS:.c=.o.d)\n");
		fprintf (stream, "OBJECTS := $(OBJECTS:.S=.o.d)\n\n");
		fprintf (stream, "$(LIBRARY).stamp: $(OBJECTS)\n");
		fprintf (stream, "\t$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)))\n");
		fprintf (stream, "\t@cat $^ > $(@:.stamp=.deps)\n");
		fprintf (stream, "\t@touch $@\n\n");
	}	

	// generate make objects rules
	for (count = 0; count < info.make_objects.size (); count++) { // for each make object
		fprintf (stream, "%s: %s\n%s\n", info.make_objects [count].object.c_str (), info.make_objects [count].deps.c_str (), tab_indent (info.make_objects [count].rules).c_str ());
	}

	// generate makes rules
	for (count = 0; count < info.makes.size (); count++) { // for each make
		fprintf (stream, "%s: $(wildcard %s)\n%s\n", resolve_tokens (info.makes [count].target).c_str (), resolve_tokens (info.makes [count].deps).c_str (), tab_indent (info.makes [count].rules).c_str ());
	}

	// generate header rules
	fprintf (stream, "headers: mlt_headers");
	for (count = 0; count < info.headers.size (); count++) { // for each header
		fprintf (stream, " $(PREFIX)/include/%s", info.headers [count].destination.c_str ());
	}
	fprintf (stream, "\n\n");
	for (count = 0; count < info.headers.size (); count++) { // for each header
		fprintf (stream, "$(PREFIX)/include/%s: $(REPOSITORY)/$(PACKAGE)/%s\n", info.headers [count].destination.c_str (), info.headers [count].source.c_str ());
#if (defined(_WIN32) || defined(__CYGWIN__)) && (ECOS_USE_CYGDRIVE > 0)
        fprintf (stream, "ifeq ($(HOST),CYGWIN)\n");
	    fprintf (stream, "\t@mkdir -p `cygpath -w \"$(dir $@)\" | sed \"s@\\\\\\\\\\\\\\\\@/@g\"`\n");
        fprintf (stream, "else\n");
	    fprintf (stream, "\t@mkdir -p $(dir $@)\n");
        fprintf (stream, "endif\n");
#else
        // This prevents older versions of mkdir failing
        fprintf (stream, "\t@mkdir -p $(dir $@)\n");
#endif

		fprintf (stream, "\t@cp $< $@\n");
		fprintf (stream, "\t@chmod u+w $@\n\n");
	}

	// include default rules
	fprintf (stream, "include $(REPOSITORY)/pkgconf/rules.mak\n\n");

	// close the makefile
	return (0 == fclose (stream));
}

// a structure and operator for sorting custom make rules by priority
struct info_make {
	std::vector <CdlBuildInfo_Loadable>::const_iterator loadable;
	unsigned int make;
};
bool operator < (info_make op1, info_make op2) {
	return op1.loadable->makes [op1.make].priority < op2.loadable->makes [op2.make].priority;
}

// create the top-level makefile
bool generate_toplevel_makefile (const CdlConfiguration config, const std::string install_tree, const std::string filename) {
	unsigned int loadable;
	unsigned int make;

	// obtain the command prefix
	std::string command_prefix = get_flags (config, NULL, "COMMAND_PREFIX");
	if (! command_prefix.empty ()) { // if there is a command prefix
		command_prefix += '-'; // add a trailing hyphen
	}

	// obtain build information from the specified configuration
	CdlBuildInfo build_info;
	config->get_build_info (build_info);
	std::vector <CdlBuildInfo_Loadable> info_vector = build_info.entries;

	// create a vector of makes and sort them by priority
	std::vector <info_make> info_make_vector;
	for (std::vector <CdlBuildInfo_Loadable>::iterator info = info_vector.begin (); info != info_vector.end (); info++) { // for each buildable loaded package
		for (unsigned int count = 0; count < info->makes.size (); count++) { // for each make
			info_make make_info = {info, count};
			info_make_vector.push_back (make_info);
		}
	}
	std::sort (info_make_vector.begin (), info_make_vector.end ());

	// open the makefile
	FILE * stream = fopen (filename.c_str (), "wt");
	if (stream == NULL) // if the file could not be opened
		return false;

	// generate the header
	fprintf (stream, makefile_header.c_str ());

	// generate the variables
#if defined(_WIN32) || defined(__CYGWIN__)
    fprintf (stream, "export HOST := CYGWIN\n");
#else
    fprintf (stream, "export HOST := UNIX\n");
#endif
	fprintf (stream, "export PREFIX := %s\n", cygpath (install_tree).c_str ());
	fprintf (stream, "export COMMAND_PREFIX := %s\n", command_prefix.c_str ());
	fprintf (stream, "export CC := $(COMMAND_PREFIX)gcc\n");
	fprintf (stream, "export OBJCOPY := $(COMMAND_PREFIX)objcopy\n");
	fprintf (stream, "export AR := $(COMMAND_PREFIX)ar\n\n");

    // generate the makefile contents
	fprintf (stream, ".PHONY: default build clean tests headers\n\n");

	fprintf (stream, "build: headers $(PREFIX)/include/pkgconf/ecos.mak\n");
	for (make = 0; make < info_make_vector.size (); make++) { // for each make
		if (info_make_vector [make].loadable->makes [info_make_vector [make].make].priority < 100) { // if priority higher than default complilation
			fprintf (stream, "\t$(MAKE) -r -C %s %s\n", info_make_vector [make].loadable->directory.c_str (), resolve_tokens (info_make_vector [make].loadable->makes [info_make_vector [make].make].target).c_str ());
		}
	}
	for (loadable = 0; loadable < info_vector.size (); loadable++) { // for each buildable loaded package
		const std::string source_path = info_vector [loadable].directory;
		fprintf (stream, "\t$(MAKE) -r -C %s $@\n", source_path.c_str ());
	}
	for (make = 0; make < info_make_vector.size (); make++) { // for each make
		if (info_make_vector [make].loadable->makes [info_make_vector [make].make].priority >= 100) { // if priority lower than or equal to default complilation
			fprintf (stream, "\t$(MAKE) -r -C %s %s\n", info_make_vector [make].loadable->directory.c_str (), resolve_tokens (info_make_vector [make].loadable->makes [info_make_vector [make].make].target).c_str ());
		}
	}
	fprintf (stream, "\t@echo $@ finished\n\n");

	fprintf (stream, "clean:\n");
	for (loadable = 0; loadable < info_vector.size (); loadable++) { // for each buildable loaded package
		const std::string source_path = info_vector [loadable].directory;
		fprintf (stream, "\t$(MAKE) -r -C %s $@\n", source_path.c_str ());
	}
	fprintf (stream, "\t@echo $@ finished\n\n");

	fprintf (stream, "tests: build\n");
	for (loadable = 0; loadable < info_vector.size (); loadable++) { // for each buildable loaded package
		const std::string source_path = info_vector [loadable].directory;
		fprintf (stream, "\t$(MAKE) -r -C %s $@\n", source_path.c_str ());
	}
	fprintf (stream, "\t@echo $@ finished\n\n");

	fprintf (stream, "headers:\n");
	for (loadable = 0; loadable < info_vector.size (); loadable++) { // for each buildable loaded package
		const std::string source_path = info_vector [loadable].directory;
		fprintf (stream, "\t$(MAKE) -r -C %s $@\n", source_path.c_str ());
	}
	fprintf (stream, "\t@echo $@ finished\n\n");

	fprintf (stream, "$(PREFIX)/include/pkgconf/ecos.mak: makefile\n");
	fprintf (stream, "\t@echo 'ECOS_GLOBAL_CFLAGS = %s' > $@\n", get_flags (config, NULL, "CFLAGS").c_str ());
	fprintf (stream, "\t@echo 'ECOS_GLOBAL_LDFLAGS = %s' >> $@\n", get_flags (config, NULL, "LDFLAGS").c_str ());
	fprintf (stream, "\t@echo 'ECOS_COMMAND_PREFIX = $(COMMAND_PREFIX)' >> $@\n\n");

	// close the makefile
	return (0 == fclose (stream));
}

// generates the directory structure for the build and install trees
bool generate_build_tree (const CdlConfiguration config, const std::string build_tree, const std::string install_tree /* = "" */ ) {
#if defined(_WIN32) || defined(__CYGWIN__)
	// convert backslash directory separators to forward slashes under Win32
	const std::string build_dir = replace_char (build_tree, '\\', '/');
	const std::string install_dir = install_tree.empty () ? build_dir + "/install" : replace_char (install_tree, '\\', '/');
#else
	const std::string build_dir = build_tree;
	const std::string install_dir = install_tree.empty () ? build_dir + "/install" : install_tree;
#endif

	// create build and install directories to ensure they are in writable locations
	if (! create_directory (build_dir))
		return false;
	if (! create_directory (install_dir + "/lib"))
		return false;
	if (! create_directory (install_dir + "/include/pkgconf"))
		return false;

	// obtain build information from the specified configuration
	CdlBuildInfo build_info;
	config->get_build_info (build_info);
	std::vector <CdlBuildInfo_Loadable> info_vector = build_info.entries;

	for (unsigned int loadable = 0; loadable < info_vector.size (); loadable++) { // for each buildable loaded package
		const std::string build_dir_loadable = build_dir + "/" + info_vector [loadable].directory;

		// create loadable directory in build tree
		if (! create_directory (build_dir_loadable))
			return false;

		// generate makefile
		generate_makefile (config, info_vector [loadable], install_dir, build_dir_loadable + "/makefile");
	}

	generate_toplevel_makefile (config, install_dir, build_dir + "/makefile");

	return true;
}
