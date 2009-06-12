// C++ IA64 / g++ v3 demangler  -*- C++ -*-

// Copyright (C) 2003 Free Software Foundation, Inc.
// Written by Carlo Wood <carlo@alinoe.com>
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

#include "bits/demangle.h"

#ifdef _GLIBCXX_DEMANGLER_NOSTDCXX
#include "bits/myallocator.h"
#include "demangle.h"
namespace __gnu_cxx
{
  namespace demangler
  {
    namespace 
    {
      typedef dynstring string;
    }
  }
}
#else
namespace __gnu_cxx
{
  namespace demangler
  {
    namespace 
    {
      typedef std::string string;
    }
  }
}
#endif

// __cxa_demangle
//
// Demangle a C++ symbol or type name.
//
// `mangled-name' is a pointer to a null-terminated array of characters.
// It may be either an external name, i.e. with a "_Z" prefix, or an
// internal NTBS mangling, e.g. of a type for type_info.
//
// `buf' may be null.  If it is non-null, then n must also be non-null,
// and buf is a pointer to an array, of at least *n characters, that
// was allocated using malloc.
//
// `status' points to an int that is used as an error indicator. It is
// permitted to be null, in which case the user just doesn't get any
// detailed error information. 
//
// Returns: a pointer to a null-terminated array of characters, the
//          demangled name.  Or NULL in case of failure.
//
// If there is an error in demangling, the return value is a null pointer.
// The user can examine *status to find out what kind of error occurred.
// Meaning of error indications:
//
//     *  0: success
//     * -1: memory allocation failure
//     * -2: invalid mangled name
//     * -3: invalid arguments (e.g. buf nonnull and n null) 
//

namespace __gnu_cxx
{
  namespace demangler
  {
    namespace 
    {
      char* const error = 0;

      enum status_codes 
	{
	  success = 0,
	  memory_allocation_failure = -1,
	  invalid_mangled_name = -2,
	  invalid_argument = -3
	};

      inline char*
      failure(status_codes error_code, int* status)
      {
	if (status)
	  *status = error_code;
	return error;
      }

      char*
      finish(char const* demangled_name, size_t demangled_name_size,
	     char* buf, size_t* n, int* status)
      {
	if (!buf || *n < demangled_name_size + 1)
	  {
	    if (n)
	      *n = demangled_name_size + 1;
	    buf = (char*)realloc(buf, demangled_name_size + 1);
	    if (!buf)
	      return failure(memory_allocation_failure, status);
	  }
	if (status)
	  *status = success;
	std::strncpy(buf, demangled_name, demangled_name_size);
	buf[demangled_name_size] = 0;
	return buf;
      }

      // Replace occurances of JArray<TYPE> with TYPE[].
      string
      decode_java (string &result)
      {
	int len;
	int nesting = 0;
	char *next, *end;
	string java;

	len = result.size ();
	next = (char *) result.data ();
	end = next + len;

	while (next < end)
	  {
	    char *open_str = strstr (next, "JArray<");
	    char *close_str = NULL;
	    if (nesting > 0)
	      close_str = strchr (next, '>');

	    if (open_str != NULL
		&& (close_str == NULL || close_str > open_str))
	      {
		++nesting;

		// Copy prepending symbols, if any.
		if (open_str > next)
		  {
		    open_str[0] = 0;
		    java += next;
		  }
		next = open_str + 7;
	      }
	    else if (close_str != NULL)
	      {
		--nesting;
		// Copy prepending type symbol, if any. Squash any
		// spurious whitespace.
		if (close_str > next && next[0] != ' ')
		  {
		    close_str[0] = 0;
		    java += next;
		  }
		java += "[]";
		next = close_str + 1;
	      }
	    else
	      {
		// There are no more arrays. Copy the rest of the
		// symbol, or simply return the original symbol if no
		// changes were made.
		if (next == result.data ())
		  java = result;
		else
		  java += next;
		break;
	      }
	  }

	return java;
      }

      char*
      demangle(char const* mangled_name, char* buf, std::size_t* n,
	       int* status, bool type, bool java)
      {
	using namespace std;
	typedef session<allocator<char> > session_type;

	if (!mangled_name || (buf && !n))
	  return failure(invalid_argument, status);

	// If we don't do type, just return if it is a type.
	if (!type && (mangled_name[0] != '_'
		      || (mangled_name[1] != 'Z'
			  && mangled_name[1] != 'G')))
	  return NULL;

	string result;
	if (mangled_name[0] == '_')		
	  {
	    // External name?
	    if (mangled_name[1] == 'Z')		
	      {
		// C++ name?
		int cnt = session_type::decode_encoding
			    (result, mangled_name + 2, INT_MAX, java);
		if (cnt < 0 || mangled_name[cnt + 2] != 0)
		  return failure(invalid_mangled_name, status);
		if (java)
		  result = decode_java (result);
		return finish(result.data(), result.size(), buf, n, status);
	      }
	    else if (mangled_name[1] == 'G')	
	      {
		// Possible _GLOBAL__ extension?
		if (!std::strncmp(mangled_name, "_GLOBAL__", 9) 
		    && (mangled_name[9] == 'D' || mangled_name[9] == 'I')
		    && mangled_name[10] == '_' && mangled_name[11] == '_' 
		    && mangled_name[12] == 'Z')
		  {
		    if (mangled_name[9] == 'D')
		      result.assign("global destructors keyed to ", 28);
		    else
		      result.assign("global constructors keyed to ", 29);
		    int cnt = session_type::decode_encoding
				(result, mangled_name + 13, INT_MAX, java);
		    if (cnt < 0 || mangled_name[cnt + 13] != 0)
		      return failure(invalid_mangled_name, status);
		    if (java)
		      result = decode_java (result);
		    return finish(result.data(), result.size(), buf, n,
				  status);
		  }
	      }
	  }

	// Ambiguities are possible between extern "C" object names and
	// internal built-in type names, e.g. "i" may be either an object
	// named "i" or the built-in "int" type.  Such ambiguities should
	// be resolved to user names over built-in names.  Builtin types
	// are any single lower case character.  Any other single
	// character is not a mangled type so we can treat those the same
	// here.
	if (mangled_name[1] == 0)
	  return finish(mangled_name, 1, buf, n, status);

	// Not a built-in type or external name, try to demangle input as
	// NTBS mangled type name.
	session_type demangler_session(mangled_name, INT_MAX);
	if (!demangler_session.decode_type(result) 
	    || demangler_session.remaining_input_characters())
	  {
	    // Failure to demangle, assume extern "C" name.
	    result = mangled_name;		
	  }
	if (java)
	  result = decode_java (result);
	return finish(result.data(), result.size(), buf, n, status);
      }

#ifdef _GLIBCXX_DEMANGLER_NOSTDCXX
      int
      demangle_cdtor(char const* mangled_name, bool ctor)
      {
	typedef session<allocator<char> > session_type;

	if (!mangled_name)
	  return -1;

	if (mangled_name[0] == '_')		
	  {
	    // External name?
	    if (mangled_name[1] == 'Z')		
	      {
		// C++ name?
		return session_type::decode_cdtor
			 (mangled_name + 2, INT_MAX, ctor);
	      }
	    else if (mangled_name[1] == 'G')	
	      {
		// Possible _GLOBAL__ extension?
		if (!std::strncmp(mangled_name, "_GLOBAL__", 9) 
		    && (mangled_name[9] == 'D' || mangled_name[9] == 'I')
		    && mangled_name[10] == '_' && mangled_name[11] == '_' 
		    && mangled_name[12] == 'Z')
		  {
		    return session_type::decode_cdtor
			     (mangled_name + 13, INT_MAX, ctor);
		  }
	      }
	  }

	return -1;
      }

#endif
    } // namespace

#ifndef _GLIBCXX_DEMANGLER_NOSTDCXX
    extern "C" char*
    __cxa_demangle(char const* mangled_name, char* buf, std::size_t* n, 
		   int* status)
    {
      return demangle (mangled_name, buf, n, status, true, false);
    }
#else
    // Variant entry point for integration with the existing cplus-dem
    // demangler.  Attempts to demangle MANGLED.  If the demangling
    // succeeds, returns a buffer, allocated with malloc, containing
    // the demangled name.  The caller must deallocate the buffer using
    // free. If the demangling failes, returns NULL.

    extern "C" char *
    cplus_demangle_v3 (const char* mangled, int options)
    {
      int status;
      return demangle (mangled, NULL, NULL, &status,
		       (options & DMGL_TYPES) != 0, false);
    }

    // Demangle a Java symbol.  Java uses a subset of the V3 ABI C++
    // mangling conventions, but the output formatting is a little
    // different. This instructs the C++ demangler not to emit pointer
    // characters ("*"), and to use Java's namespace separator symbol
    // ("." instead of "::").  It then does an additional pass over
    // the demangled output to replace instances of JArray<TYPE> with
    // TYPE[].
    extern "C" char *
    java_demangle_v3 (const char* mangled)
    {
      int status;
      return demangle (mangled, NULL, NULL, &status, 0, true);
    }

    // Return non-zero iff NAME is the mangled form of a constructor
    // name in the G++ V3 ABI demangling style.  Specifically, return:
    // - '1' if NAME is a complete object constructor,
    // - '2' if NAME is a base object constructor, or
    // - '3' if NAME is a complete object allocating constructor.
    extern "C" enum gnu_v3_ctor_kinds
    is_gnu_v3_mangled_ctor (const char *name)
    {
      enum gnu_v3_ctor_kinds kind;
      int ctor = demangle_cdtor (name, true);
      switch (ctor)
	{
	default:
	  kind = (enum gnu_v3_ctor_kinds) 0;
	  break;

	case 1:
	case 2:
	case 3:
	  kind = (enum gnu_v3_ctor_kinds) ctor;
	  break;
	}

      return kind;
    }

    // Return non-zero iff NAME is the mangled form of a destructor
    // name in the G++ V3 ABI demangling style.  Specifically, return:
    // - '1' if NAME is a deleting destructor,
    // - '2' if NAME is a complete object destructor, or
    // - '3' if NAME is a base object destructor.
    extern "C" enum gnu_v3_dtor_kinds
    is_gnu_v3_mangled_dtor (const char *name)
    {
      enum gnu_v3_dtor_kinds kind;
      int dtor = demangle_cdtor (name, false);
      switch (dtor)
	{
	default:
	  kind = (enum gnu_v3_dtor_kinds) 0;
	  break;

	case 0:
	case 1:
	case 2:
	  kind = (enum gnu_v3_dtor_kinds) ((int) dtor + 1);
	  break;
	}

      return kind;
    }
#endif
  }
}
