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
// This include file serves only to include others - usually from the STL -
// that would otherwise lead to a huge number of warning messages being generated.
// (because #defines cannot use the # character this cannot be done with macros)
// So what we do is this:
//      #define INCLUDEFILE file_to_include
//      #include IncludeSTL.h
// Note that the ".." or <..> must be included in the definition of INCLUDEFILE.

#ifndef INCLUDEFILE 
    #error Need to define INCLUDEFILE
#endif

#pragma warning (push)
    #pragma warning(disable:4018) // signed/unsigned mismatch
    #pragma warning(disable:4100) // unreferenced formal parameter
    #pragma warning(disable:4146) // unary minus operator applied to unsigned type, result still unsigned
    #pragma warning(disable:4189) // local variable is initialized but not referenced
    #pragma warning(disable:4244) // conversion from 'unsigned int' to 'char', possible loss of data
    #pragma warning(disable:4250) // CdlConfigurationBody' : inherits 'CdlToplevelBody::is_active' via dominance
    #pragma warning(disable:4284) // return type for (ie; not a UDT or reference to a UDT.  Will produce errors if applied using infix notation)
    #pragma warning(disable:4290) // C++ Exception Specification ignored
    #pragma warning(disable:4503) // decorated name length exceeded, name was truncated
    #pragma warning(disable:4663) // C++ language change: to explicitly specialize class template...

    // EXTERN will be (re)defined in tcl.h via cdl.hxx
    #ifdef EXTERN
	    #undef EXTERN 
    #endif
    #include INCLUDEFILE
    #undef EXTERN
#pragma warning (pop)
#undef INCLUDEFILE
