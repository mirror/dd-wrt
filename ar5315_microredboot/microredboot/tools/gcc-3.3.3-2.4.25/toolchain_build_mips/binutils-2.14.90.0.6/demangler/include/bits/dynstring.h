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

#include <cstring>

#include "ansidecl.h"
extern "C" {
#include "dyn-string.h"
}

namespace __gnu_cxx
{
  namespace demangler
  {
    namespace
    {
      class dynstring
      {
      private:
	dyn_string_t M_dynstr;
      public:
	dynstring(void) { M_dynstr = dyn_string_new(256); }
	~dynstring() { dyn_string_delete(M_dynstr); }
	dynstring(dynstring const& in)
	{
	  M_dynstr = dyn_string_new(256);
	  dyn_string_append(M_dynstr,
			    const_cast<dyn_string_t>(in.M_dynstr));
	}
	char* data(void) { return dyn_string_buf(M_dynstr); }
	size_t size(void) { return dyn_string_length(M_dynstr); }
	void assign(char const* in, size_t len)
	{
	  if (len + 1 > (size_t) M_dynstr->allocated)
	    dyn_string_resize(M_dynstr, len + 1);
	  strncpy(M_dynstr->s, in, len);
	  M_dynstr->length = len;
	  M_dynstr->s[len] = 0;
	}
	dynstring& operator=(dynstring const& in)
	{
	  dyn_string_delete(M_dynstr);
	  M_dynstr = dyn_string_new(256);
	  dyn_string_append(M_dynstr,
			    const_cast<dyn_string_t>(in.M_dynstr));
	  return *this;
	}
	dynstring& operator=(char const* in)
	{ dyn_string_copy_cstr(M_dynstr, in); return *this; }
	dynstring& operator+=(char const* in)
	{ dyn_string_append_cstr(M_dynstr, in); return *this; }
	dynstring& operator+=(dynstring const& in)
	{
	  dyn_string_append(M_dynstr,
			    const_cast<dyn_string_t>(in.M_dynstr));
	  return *this;
	}
	dynstring& operator+=(char in)
	{ dyn_string_append_char(M_dynstr, in); return *this; }
	char const* rbegin(void) const
	{ return dyn_string_buf(M_dynstr)
		 + dyn_string_length(M_dynstr) - 1; }
	void clear(void) { dyn_string_clear(M_dynstr); }
      };
    }
  }
}
