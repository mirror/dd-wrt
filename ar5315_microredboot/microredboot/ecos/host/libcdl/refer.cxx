//{{{  Banner                           

//============================================================================
//
//      refer.cxx
//
//      Implementation of the CdlReference and CdlReferrer classes.
//
//============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 1999, 2000 Red Hat, Inc.
//
// This file is part of the eCos host tools.
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
//============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   bartv
// Contact(s):  bartv
// Date:        1999/02/01
// Version:     0.02
//
//####DESCRIPTIONEND####
//============================================================================

//}}}
//{{{  #include's                       

// ----------------------------------------------------------------------------
#include "cdlconfig.h"

// Get the infrastructure types, assertions, tracing and similar
// facilities.
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>

// <cdlcore.hxx> defines everything implemented in this module.
// It implicitly supplies <string>, <vector> and <map> because
// the class definitions rely on these headers.
#include <cdlcore.hxx>

//}}}

//{{{  Statics                          

// ----------------------------------------------------------------------------
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlReference);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlReferrer);

//}}}
//{{{  CdlReference class               

// ----------------------------------------------------------------------------
// The default constructor. This should not normally get invoked, instead
// a string argument should be supplied. However it is occasionally useful
// to construct a reference and then set the name later.

CdlReference::CdlReference()
{
    CYG_REPORT_FUNCNAME("CdlReference:: default constructor");
    CYG_REPORT_FUNCARG1XV(this);

    dest_name           = "";
    dest                = 0;
    cdlreference_cookie = CdlReference_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();

    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

// This constructor typically gets used when parsing a property.
// The object may be created either on the stack or in the heap,
// depending on the requirements of the property. During a later
// phase in the parsing process the object may get bound.
CdlReference::CdlReference(const std::string dest_arg)
{
    CYG_REPORT_FUNCNAME("CdlReference:: constructor");
    CYG_REPORT_FUNCARG1("this %p", this);
    
    dest_name           = dest_arg;
    dest                = 0;
    cdlreference_cookie = CdlReference_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

// For some properties, notably LocalReference ones, it is convenient
// to create a temporary reference object on the stack and then
// derive the property from the temporary. This requires the
// copy constructor. When this operation is used the object
// should still be unbound.
CdlReference::CdlReference(const CdlReference& orig)
{
    CYG_REPORT_FUNCNAME("CdlReference:: copy constructor");
    CYG_REPORT_FUNCARG2("this %p, orig %p", this, &orig);
    CYG_INVARIANT_CLASSC(CdlReference, orig);
    CYG_PRECONDITIONC(0 == orig.dest);

    dest_name           = orig.dest_name;
    dest                = 0;
    cdlreference_cookie = CdlReference_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

// The assignment operator may be needed for similar reasons.
CdlReference&
CdlReference::operator=(const CdlReference& orig)
{
    CYG_REPORT_FUNCNAME("CdlReference:: assignment operator");
    CYG_REPORT_FUNCARG2("this %p, orig %p", this, &orig);
    CYG_INVARIANT_CLASSC(CdlReference, orig);
    CYG_PRECONDITIONC(0 == orig.dest);

    if (this != &orig) {
        dest_name               = orig.dest_name;
        dest                    = 0;
        cdlreference_cookie     = CdlReference_Magic;
    }

    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
    return *this;
}
    
// The destructor should only get invoked when a package is unloaded.
// All appropriate clean-ups should have happened already, in particular
// the reference should not currently be bound.
CdlReference::~CdlReference()
{
    CYG_REPORT_FUNCNAME("CdlReference:: destructor");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC(0 == dest);

    cdlreference_cookie = CdlReference_Invalid;
    dest_name           = "";
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Accessing the fields.

void
CdlReference::set_destination_name(const std::string name)
{
    CYG_REPORT_FUNCNAME("CdlReference::set_destination_name");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC((0 == dest) && ("" == dest_name));

    dest_name = name;

    CYG_REPORT_RETURN();
}

const std::string&
CdlReference::get_destination_name(void) const
{
    CYG_REPORT_FUNCNAME("CdlReference::get_destination_name");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return dest_name;
}

CdlNode
CdlReference::get_destination() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlReference::get_destination", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlNode result = dest;
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlReference::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlReference_Magic != cdlreference_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    switch(zeal) {
      case cyg_system_test:
      case cyg_extreme:
          // There is not enough information in the reference
          // object itself to allow for a check for a corresponding
          // referrer object. If more debugability is needed
          // then extra data will have to be stored.
          //
          // However, it is possible to do a basic check of dest itself.
          if (0 != dest) {
              if (!dest->check_this(cyg_quick)) {
                  return false;
              }
          }
      case cyg_thorough:
          if (0 != dest) {
              if (dest_name != dest->get_name()) {
                  return false;
              }
          }
      case cyg_quick:
      case cyg_trivial:
      case cyg_none   :
      default         :
          break;
    }
    return true;
}

// ----------------------------------------------------------------------------
// Binding. This is simply a case of filling in some fields in the reference
// object and pushing a new referrer object. Binding generally happens as
// a result of calling reference update handlers when a package gets loaded.

void
CdlReference::bind(CdlNode src_arg, CdlProperty src_prop_arg, CdlNode dest_arg)
{
    CYG_REPORT_FUNCNAME("CdlReference::bind");
    CYG_REPORT_FUNCARG4XV(this, src_arg, src_prop_arg, dest_arg);
    CYG_INVARIANT_THISC(CdlReference);
    CYG_PRECONDITION_CLASSC(src_arg);
    CYG_PRECONDITION_CLASSC(src_prop_arg);
    CYG_PRECONDITION_CLASSC(dest_arg);

    CYG_ASSERTC(dest_name == dest_arg->get_name());

    CdlReferrer local_copy;
    local_copy.source           = src_arg;
    local_copy.source_property  = src_prop_arg;

    dest_arg->referrers.push_back(local_copy);
    dest = dest_arg;

    CYG_REPORT_RETURN();
}

// Unbinding involves finding and removing the appropriate referrer object (there
// may be several, but each will result in a separate unbind() call.)
void
CdlReference::unbind(CdlNode src_arg, CdlProperty src_prop_arg)
{
    CYG_REPORT_FUNCNAME("CdlReference::unbind");
    CYG_REPORT_FUNCARG3XV(this, src_arg, src_prop_arg);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(src_arg);
    CYG_PRECONDITION_CLASSC(src_prop_arg);

    if (0 != dest) {
        std::vector<CdlReferrer>::iterator ref_i;
        for (ref_i = dest->referrers.begin(); ref_i != dest->referrers.end(); ref_i++) {
            if ((ref_i->source == src_arg) && (ref_i->source_property == src_prop_arg)) {
                dest->referrers.erase(ref_i);
                break;
            }
        }
    }

    dest = 0;

    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlReferrer class                

// ----------------------------------------------------------------------------
// The constructors etc. should only get invoked from
// CdlReference::bind() and unbind(). However because Referrer objects
// get held in  STL vectors/lists/whatever it is hard to know exactly
// what will happen when, so assertions are a bit thin on the ground.

CdlReferrer::CdlReferrer()
{
    CYG_REPORT_FUNCNAME("CdlReferrer:: default constructor");
    CYG_REPORT_FUNCARG1XV(this);

    source              = 0;
    source_property     = 0;
    cdlreferrer_cookie  = CdlReferrer_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlReferrer::CdlReferrer(const CdlReferrer& original)
{
    CYG_REPORT_FUNCNAME("CdlReferrer:: copy constructor");
    CYG_REPORT_FUNCARG2XV(this, &original);
    CYG_PRECONDITION_CLASSOC(original);

    source              = original.source;
    source_property     = original.source_property;
    cdlreferrer_cookie  = CdlReferrer_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();

    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlReferrer&
CdlReferrer::operator=(const CdlReferrer& original)
{
    CYG_REPORT_FUNCNAME("CdlReferrer:: assignment operator");
    CYG_REPORT_FUNCARG2XV(this, &original);
    CYG_PRECONDITION_CLASSOC(original);

    if (this != &original) {
        source                  = original.source;
        source_property         = original.source_property;
        cdlreferrer_cookie      = CdlReferrer_Magic;
    }

    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
    return *this;
}

CdlReferrer::~CdlReferrer()
{
    CYG_REPORT_FUNCNAME("CdlReferrer:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdlreferrer_cookie  = CdlReferrer_Magic;
    source              = 0;
    source_property     = 0;
    CYGDBG_MEMLEAK_DESTRUCTOR();

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// This routine is the main reason for having reference and referrer
// objects around in the first place. It allows changes to an entity
// to be propagated back to anything else interested in the entity.

void
CdlReferrer::update(CdlTransaction transact, CdlNode dest_arg, CdlUpdate change)
{
    CYG_REPORT_FUNCNAME("CdlReferrer::update");
    CYG_REPORT_FUNCARG4XV(this, transact, dest_arg, change);
    CYG_PRECONDITION_THISC();

    source_property->update(transact, source, dest_arg, change);

    CYG_REPORT_RETURN();
}

CdlNode
CdlReferrer::get_source() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlReferrer::get_source", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlNode result = source;
    CYG_REPORT_RETVAL(result);
    return result;
}

CdlProperty
CdlReferrer::get_source_property() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlReferrer::get_source_property", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlProperty result = source_property;
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlReferrer::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlReferrer_Magic != cdlreferrer_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    switch(zeal) {
      case cyg_system_test :
      case cyg_extreme:
      case cyg_thorough:
          if (0 != source) {
              if (!source->check_this(cyg_quick)) {
                  return false;
              }
          }
          if (0 != source_property) {
              if (!source_property->check_this(cyg_quick)) {
                  return false;
              }
          }
      case cyg_quick:
      case cyg_trivial:
      case cyg_none:
        break;
    }
    return true;
}

//}}}
