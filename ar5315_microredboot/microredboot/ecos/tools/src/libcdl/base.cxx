//{{{  Banner                           

//============================================================================
//
//     base.cxx
//
//     Implementations of the various base classes
//
//============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 2002 Bart Veer
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
// Date:        1999/02/18
// Version:     0.02
// Description: libcdl defines a hierarchy of base classes, used for
//              constructing higher-level entities such as options
//              and packages.
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

// <cdl.hxx> defines everything implemented in this module.
// It implicitly supplies <string>, <vector> and <map> because
// the class definitions rely on these headers.
#include <cdlcore.hxx>

//}}}

//{{{  Statics                          

// ----------------------------------------------------------------------------
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlNodeBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlContainerBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlLoadableBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlToplevelBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlUserVisibleBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlParentableBody);

//}}}
//{{{  CdlNodeBody                      

//{{{  Construction                             

// ----------------------------------------------------------------------------
// The real constructor takes a string argument and should get invoked first.
// Because of the way virtual inheritance is used it is also necessary to have
// a default constructor, but that need not do anything. A newly constructed
// object does not yet live in the hierarchy.

CdlNodeBody::CdlNodeBody(std::string name_arg)
{
    CYG_REPORT_FUNCNAME("CdlNode:: constructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITIONC("" != name_arg);

    name        = name_arg;
    parent      = 0;
    owner       = 0;
    toplevel    = 0;
    active      = false;
    remove_node_container_position = -1;

    // The STL containers will take care of themselves.
    
    cdlnodebody_cookie = CdlNodeBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();

    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlNodeBody::CdlNodeBody()
{
    CYG_PRECONDITION_THISC();
}

//}}}
//{{{  Destructor                               

// ----------------------------------------------------------------------------
// By the time the destructor gets invoked the node should already
// have been unbound and removed from the hierarchy.

CdlNodeBody::~CdlNodeBody()
{
    CYG_REPORT_FUNCNAME("CdlNode:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    // Make sure that the node is unbound: all references to and from
    // this node should have been destroyed already inside a
    // transaction.
    CYG_PRECONDITIONC(0 == referrers.size());

    // Make sure that the node has been removed from the hierarchy
    CYG_PRECONDITIONC(0 == toplevel);
    CYG_PRECONDITIONC(0 == owner);
    CYG_PRECONDITIONC(0 == parent);
    
    // Destroy all properties associated with this object.
    std::vector<CdlProperty>::iterator prop_i;
    for (prop_i= properties.begin(); prop_i != properties.end(); prop_i++) {
        delete *prop_i;
        *prop_i = 0;
    }
    properties.clear();

    cdlnodebody_cookie  = CdlNodeBody_Invalid;
    name   = "";
    active = false;
    unsupported_savefile_strings.clear();
    
    CYGDBG_MEMLEAK_DESTRUCTOR();

    CYG_REPORT_RETURN();
}

//}}}
//{{{  Trivial data access                      

// ----------------------------------------------------------------------------

std::string
CdlNodeBody::get_name() const
{
    CYG_REPORT_FUNCNAME("CdlNode::get_name");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return name;
}

void
CdlNodeBody::set_name(std::string name_arg)
{
    CYG_REPORT_FUNCNAME("CdlNode::set_name");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    name = name_arg;

    CYG_REPORT_RETURN();
}

CdlContainer
CdlNodeBody::get_parent() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlNode::get_parent", "parent %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlContainer result = parent;
    CYG_REPORT_RETVAL(result);
    return result;
}

CdlLoadable
CdlNodeBody::get_owner() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlNode::get_owner", "owner %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlLoadable result = owner;
    CYG_REPORT_RETVAL(result);
    return result;
}

CdlToplevel
CdlNodeBody::get_toplevel() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlNode::get_toplevel", "toplevel %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlToplevel result = toplevel;
    CYG_REPORT_RETVAL(result);
    return result;
}

std::string
CdlNodeBody::get_class_name() const
{
    CYG_REPORT_FUNCNAME("CdlNode::get_class_name");
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return "node";
}

//}}}
//{{{  The properties vector                    

// ----------------------------------------------------------------------------
// Trivial manipulation of the properties vector.

const std::vector<CdlProperty>&
CdlNodeBody::get_properties() const
{
    CYG_REPORT_FUNCNAME("CdlNode::get_properties");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return properties;
}

CdlProperty
CdlNodeBody::get_property(std::string id) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlNode::get_property", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlProperty result = 0;
    std::vector<CdlProperty>::const_iterator prop_i;
    for (prop_i = properties.begin(); prop_i != properties.end(); prop_i++) {
        if ((*prop_i)->get_property_name() == id) {
            result = *prop_i;
            break;
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlNodeBody::get_properties(std::string id, std::vector<CdlProperty>& result) const
{
    CYG_REPORT_FUNCNAME("CdlNode::get_properties");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::vector<CdlProperty>::const_iterator prop_i;
    for (prop_i = properties.begin(); prop_i != properties.end(); prop_i++) {
        if ((*prop_i)->get_property_name() == id) {
            result.push_back(*prop_i);
        }
    }

    CYG_REPORT_RETURN();
}

std::vector<CdlProperty>
CdlNodeBody::get_properties(std::string id) const
{
    CYG_REPORT_FUNCNAME("CdlNode::get_properties");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::vector<CdlProperty> result;
    std::vector<CdlProperty>::const_iterator prop_i;
    for (prop_i = properties.begin(); prop_i != properties.end(); prop_i++) {
        if ((*prop_i)->get_property_name() == id) {
            result.push_back(*prop_i);
        }
    }

    CYG_REPORT_RETURN();
    return result;
}

bool
CdlNodeBody::has_property(std::string id) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlNode::has_property", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = false;
    std::vector<CdlProperty>::const_iterator prop_i;
    for (prop_i = properties.begin(); prop_i != properties.end(); prop_i++) {
        if ((*prop_i)->get_property_name() == id) {
            result = true;
            break;
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

int
CdlNodeBody::count_properties(std::string id) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlNode::count_properties", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    int result = 0;
    std::vector<CdlProperty>::const_iterator prop_i;
    for (prop_i = properties.begin(); prop_i != properties.end(); prop_i++) {
        if ((*prop_i)->get_property_name() == id) {
            result++;
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  Conflicts                                

// ----------------------------------------------------------------------------
// Provide access to the current set of conflicts. This operates on the global
// state, more commonly these changes happen in the context of a transaction.

void
CdlNodeBody::get_conflicts(std::vector<CdlConflict>& result) const
{
    CYG_REPORT_FUNCNAME("CdlNode::get_conflicts");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    const std::list<CdlConflict>& conflicts = toplevel->get_all_conflicts();
    std::list<CdlConflict>::const_iterator conf_i;
    for (conf_i = conflicts.begin(); conf_i != conflicts.end(); conf_i++) {
        if ((*conf_i)->get_node() == this) {
            result.push_back(*conf_i);
        }
    }

    CYG_REPORT_RETURN();
}

void
CdlNodeBody::get_conflicts(bool (*fn)(CdlConflict), std::vector<CdlConflict>& result) const
{
    CYG_REPORT_FUNCNAME("CdlNode::get_conflicts");
    CYG_REPORT_FUNCARG2XV(this, fn);
    CYG_PRECONDITION_THISC();
    CYG_CHECK_FUNC_PTRC(fn);

    const std::list<CdlConflict>& conflicts = toplevel->get_all_conflicts();
    std::list<CdlConflict>::const_iterator conf_i;
    for (conf_i = conflicts.begin(); conf_i != conflicts.end(); conf_i++) {
        if (((*conf_i)->get_node() == this) && ((*fn)(*conf_i))) {
            result.push_back(*conf_i);
        }
    }
    
    CYG_REPORT_RETURN();
}

void
CdlNodeBody::get_structural_conflicts(std::vector<CdlConflict>& result) const
{
    CYG_REPORT_FUNCNAME("CdlNode::get_structural_conflicts");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    const std::list<CdlConflict>& conflicts = toplevel->get_all_structural_conflicts();
    std::list<CdlConflict>::const_iterator conf_i;
    for (conf_i = conflicts.begin(); conf_i != conflicts.end(); conf_i++) {
        if ((*conf_i)->get_node() == this) {
            result.push_back(*conf_i);
        }
    }

    CYG_REPORT_RETURN();
}

void
CdlNodeBody::get_structural_conflicts(bool (*fn)(CdlConflict), std::vector<CdlConflict>& result) const
{
    CYG_REPORT_FUNCNAME("CdlNode::get_conflicts");
    CYG_REPORT_FUNCARG2XV(this, fn);
    CYG_PRECONDITION_THISC();
    CYG_CHECK_FUNC_PTRC(fn);

    const std::list<CdlConflict>& conflicts = toplevel->get_all_structural_conflicts();
    std::list<CdlConflict>::const_iterator conf_i;
    for (conf_i = conflicts.begin(); conf_i != conflicts.end(); conf_i++) {
        if (((*conf_i)->get_node() == this) && ((*fn)(*conf_i))) {
            result.push_back(*conf_i);
        }
    }
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Referrers                                

// ----------------------------------------------------------------------------
// And access to the referrers vector.
const std::vector<CdlReferrer>&
CdlNodeBody::get_referrers() const
{
    CYG_REPORT_FUNCNAME("CdlNode::get_referrers");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return referrers;
}

//}}}
//{{{  Property parsers                         

// ----------------------------------------------------------------------------
// Property parsing. For now there are now properties guaranteed to be
// associated with every node. This may change in future, e.g.
// internal debugging-related properties.
void
CdlNodeBody::add_property_parsers(std::vector<CdlInterpreterCommandEntry>& parsers)
{
    CYG_REPORT_FUNCNAME("CdlNode::add_property_parsers");
    CYG_REPORT_RETURN();
}

void
CdlNodeBody::check_properties(CdlInterpreter interp)
{
    CYG_REPORT_FUNCNAME("CdlNode::check_properties");
    CYG_REPORT_FUNCARG2XV(this, interp);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(interp);

    CYG_REPORT_RETURN();
}

//}}}
//{{{  is_active() etc.                         

// ----------------------------------------------------------------------------
// Associated with every node is a boolean that holds the current
// "active" state. Changes to this happen only at transaction
// commit time.

bool
CdlNodeBody::is_active() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlNode::is_active", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = active;
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlNodeBody::is_active(CdlTransaction transaction)
{
    CYG_REPORT_FUNCNAMETYPE("CdlNode::is_active", "result %d");
    CYG_REPORT_FUNCARG2XV(this, transaction);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_ZERO_OR_CLASSC(transaction);

    bool result;
    if (0 != transaction) {
        result = transaction->is_active(this);
    } else {
        result = active;
    }
    CYG_REPORT_RETVAL(result);
    return result;
}

// This virtual member function allows nodes to check whether or not
// they should be active. Derived classes may impose additional
// constraints.
bool
CdlNodeBody::test_active(CdlTransaction transaction)
{
    CYG_REPORT_FUNCNAMETYPE("CdlNode::test_active", "result %d");
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    bool result = false;
    if ((0 != parent) && (transaction->is_active(parent))) {
        CdlValuable valuable = dynamic_cast<CdlValuable>(parent);
        if (0 == valuable) {
            result = true;
        } else if (valuable->is_enabled(transaction)) {
            result = true;
        }
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  Propagation support                      

// ----------------------------------------------------------------------------
// In the base class nothing needs doing for propagation.
void
CdlNodeBody::update(CdlTransaction transaction, CdlUpdate change)
{
    CYG_REPORT_FUNCNAME("CdlNode::update");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Persistence support                      

// ----------------------------------------------------------------------------
// The CdlNode::save() member should never get invoked directly, it should
// get invoked indirectly from e.g. CdlOption::save(). Normally there is
// no information associated with a node that ends up in a save file
// (the calling code will have take care of the name etc.). However there
// is support in the library for storing application-specific data in the
// save file, for example GUI information, and this information must be
// preserved even if it is not recognised. Savefiles are self-describing,
// they contain details of all the commands that are applicable. 
// CdlNode::save() is responsible for outputting the unrecognised strings
// to the save file.

void
CdlNodeBody::save(CdlInterpreter interp, Tcl_Channel chan, int indentation, bool minimal)
{
    CYG_REPORT_FUNCNAME("CdlNode::save");
    CYG_REPORT_FUNCARG5XV(this, interp, chan, indentation, minimal);

    if (unsupported_savefile_strings.size() != 0) {
        // We should already be inside the body of a suitable command,
        // e.g. cdl_option xyz { ... }
        // CdlToplevel::savefile_handle_unsupported() is responsible for
        // putting suitably formatted strings into the
        // unsupported_savefile_strings vector, so all that is needed here
        // is to dump those strings to the channel.
        std::string data = "\n";
        std::vector<std::string>::const_iterator str_i;
        for (str_i = unsupported_savefile_strings.begin(); str_i != unsupported_savefile_strings.end(); str_i++) {
            data += std::string(indentation, ' ') + *str_i + " ;\n";
        }
        interp->write_data(chan, data);
    }

    CYG_UNUSED_PARAM(bool, minimal);
    CYG_REPORT_RETURN();
}

bool
CdlNodeBody::has_additional_savefile_information() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlNode::has_additional_savefile_information", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = (0 != unsupported_savefile_strings.size());
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  check_this()                             

// ----------------------------------------------------------------------------
// Because of multiple and virtual inheritance, check_this() may
// get called rather a lot. Unfortunately all of the checks are
// useful.

bool
CdlNodeBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlNodeBody_Magic != cdlnodebody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    
    if ("" == name) {
        return false;
    }

    // It is hard to validate the toplevel, owner, and parent
    // fields.
    //
    // 1) when a node is newly created all three fields will
    //    be null.
    // 2) the toplevel may be null if the node is in the process
    //    of being removed, e.g. during an unload operation.
    //    The node should still have a valid owner, and will
    //    have a parent unless the node is also the loadable.
    // 3) some nodes are special, e.g. the orphans container,
    //    and do not have an owner.
    //
    // So the following combinations can occur:
    //  Toplevel   Owner    Parent
    //     0         0         0          Creation & toplevel
    //     0       Valid       0          Loadable being unloaded
    //     0       Valid     Valid        Node being unloaded
    //   Valid       0       Valid        Orphans container
    //   Valid     Valid     Valid        Any node
    if (0 != toplevel) {
        if (0 == parent) {
            return false;
        }
    }
 
    switch(zeal) {
      case cyg_system_test :
      case cyg_extreme     :
      {
        if ((0 != toplevel) && (toplevel != this)) {
            if (!toplevel->check_this(cyg_quick)) {
                return false;
            }
            if (toplevel->lookup_table.find(name) == toplevel->lookup_table.end()) {
                return false;
            }
        }
        if (0 != parent) {
            if (!parent->check_this(cyg_quick)) {
                return false;
            }
            if (std::find(parent->contents.begin(), parent->contents.end(), this) == parent->contents.end()) {
                return false;
            }
        }
        if (0 != owner) {
            if (!owner->check_this(cyg_quick)) {
                return false;
            }
            if (std::find(owner->owned.begin(), owner->owned.end(), this) == owner->owned.end()) {
                return false;
            }
        }
        std::vector<CdlProperty>::const_iterator prop_i;
        for (prop_i = properties.begin(); prop_i != properties.end(); prop_i++) {
            if (!(*prop_i)->check_this(cyg_quick)) {
                return false;
            }
        }
        std::vector<CdlReferrer>::const_iterator ref_i;
        for (ref_i = referrers.begin(); ref_i != referrers.end(); ref_i++) {
            if (!ref_i->check_this(cyg_quick)) {
                return false;
            }
        }
      }
      case cyg_thorough    :
      case cyg_quick       :
      case cyg_trivial     :
      case cyg_none        :
      default              :
          break;
    }

    return true;
}

//}}}

//}}}
//{{{  CdlContainerBody                 

//{{{  Constructors                     

// ----------------------------------------------------------------------------
// A container simply holds other nodes in a hierarchy. Most
// containers correspond to data in CDL scripts, but there are
// exceptions. For example, if an option is reparented below some
// package or component that is not yet known then it can instead be
// reparented into an "orphans" container immediately below the
// toplevel.
//
// Adding and removing entries to a container is done inside the
// CdlToplevel add_node() and remove_node() members. These will update
// all the fields needed to keep the hierarchy consistent. This
// means that the CdlContainer class itself provides only limited
// functionality.

CdlContainerBody::CdlContainerBody()
{
    CYG_REPORT_FUNCNAME("CdlContainer:: default constructor");
    CYG_REPORT_FUNCARG1XV(this);

    cdlcontainerbody_cookie = CdlContainerBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

// This variant is for internal use, to allow the library to create
// containers that do not correspond to CDL entities such as
// the orphans container.
CdlContainerBody::CdlContainerBody(std::string name_arg)
    : CdlNodeBody(name_arg)
{
    CYG_REPORT_FUNCNAME("CdlContainerBody:: constructor (name)");
    CYG_REPORT_FUNCARG1XV(this);

    cdlcontainerbody_cookie = CdlContainerBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();

    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Destructor                       

// ----------------------------------------------------------------------------

CdlContainerBody::~CdlContainerBody()
{
    CYG_REPORT_FUNCNAME("CdlContainer:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    // Containers should always be empty by the time they
    // get deleted. The toplevel and loadable destructors should
    // guarantee this.
    CYG_ASSERTC(0 == contents.size());
    
    cdlcontainerbody_cookie = CdlContainerBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Accessing the contents           

// ----------------------------------------------------------------------------
// Simple contents access facilities, including searching. Note that
// the toplevel class maintains a <name,ptr> map, which will usually
// be more efficient.

const std::vector<CdlNode>&
CdlContainerBody::get_contents() const
{
    CYG_REPORT_FUNCNAME("CdlContainer::get_contents");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return contents;
}

bool
CdlContainerBody::contains(CdlConstNode node, bool recurse) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlContainer::contains (node)", "result %d");
    CYG_REPORT_FUNCARG3XV(this, node, recurse);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);

    bool result = false;
    std::vector<CdlNode>::const_iterator node_i;
    for (node_i = contents.begin(); node_i != contents.end(); node_i++) {
        if (node == *node_i) {
            result = true;
            break;
        }
        if (recurse) {
            CdlConstContainer child = dynamic_cast<CdlConstContainer>(*node_i);
            if ((0 != child) && child->contains(node, true)) {
                result = true;
                break;
            }
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlContainerBody::contains(const std::string name, bool recurse) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlContainer::contains (name)", "result %d");
    CYG_REPORT_FUNCARG2XV(this, recurse);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != name);

    bool result = false;
    std::vector<CdlNode>::const_iterator node_i;
    for (node_i = contents.begin(); node_i != contents.end(); node_i++) {
        if ((*node_i)->get_name() == name) {
            result = true;
            break;
        }
        if (recurse) {
            CdlConstContainer child = dynamic_cast<CdlConstContainer>(*node_i);
            if ((0 != child) && child->contains(name, true)) {
                result = true;
                break;
            }
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

CdlNode
CdlContainerBody::find_node(const std::string name, bool recurse) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlContainer::find_node", "result %p");
    CYG_REPORT_FUNCARG2XV(this, recurse);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != name);

    CdlNode result = 0;
    std::vector<CdlNode>::const_iterator node_i;
    for (node_i = contents.begin(); node_i != contents.end(); node_i++) {
        if ((*node_i)->get_name() == name) {
            result = *node_i;
            break;
        }
        if (recurse) {
            CdlConstContainer child = dynamic_cast<CdlConstContainer>(*node_i);
            if (0 != child) {
                result = child->find_node(name, true);
                if (0 != result) {
                    break;
                }
            }
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  Misc                             

// ----------------------------------------------------------------------------

std::string
CdlContainerBody::get_class_name() const
{
    CYG_REPORT_FUNCNAME("CdlContainer::get_class_name");
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return "container";
}

//}}}
//{{{  Propagation                      

// ----------------------------------------------------------------------------
// If a container becomes active and is enabled then it is necessary
// to check all the children in case they want to become active as well.

void
CdlContainerBody::update(CdlTransaction transaction, CdlUpdate change)
{
    CYG_REPORT_FUNCNAME("CdlContainer::update");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    if ((CdlUpdate_ActiveChange != change) && (CdlUpdate_ValueChange != change)) {
        CYG_REPORT_RETURN();
        return;
    }

    if (transaction->is_active(this)) {
        // The container has become active. It is necessary to check
        // all the children. If any of them should be active as well
        // but are not then this needs to change.
        std::vector<CdlNode>::iterator node_i;
            
        for (node_i = contents.begin(); node_i != contents.end(); node_i++) {
            bool old_state = transaction->is_active(*node_i);
            bool new_state = (*node_i)->test_active(transaction);
            if (old_state != new_state) {
                transaction->set_active(*node_i, new_state);
            }
        }
    } else {
        // The container has become inactive. Any children that were
        // active should also become inactive.
        std::vector<CdlNode>::iterator node_i;
        for (node_i = contents.begin(); node_i != contents.end(); node_i++) {
            if (transaction->is_active(*node_i)) {
                transaction->set_active(*node_i, false);
            }
        }
    }
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Persistence                      

// ----------------------------------------------------------------------------
// This member function is invoked while traversing the hierarchy.
// The container itself will have been saved already, this member
// is responsible only for the contents. There are marker comments
// in the output file to indicate a new level in the hierarchy.
//
// Note that this member can also be invoked for the "orphans" container.
// That container will not appear in the save file, but its contents
// will.

void
CdlContainerBody::save(CdlInterpreter interp, Tcl_Channel chan, int indentation, bool minimal)
{
    CYG_REPORT_FUNCNAME("CdlContainer::save");
    CYG_REPORT_FUNCARG4XV(this, interp, chan, indentation);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(interp);
    CYG_PRECONDITIONC(0 == indentation);

    if (0 != contents.size()) {
        if (!minimal) {
            interp->write_data(chan, "# >\n");
        }
        std::vector<CdlNode>::const_iterator node_i;
        for (node_i = contents.begin(); node_i != contents.end(); node_i++) {
            (*node_i)->save(interp, chan, indentation, minimal);
        }
        if (!minimal) {
            interp->write_data(chan, "# <\n");
        }
    }
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  check_this()                     

// ----------------------------------------------------------------------------
bool
CdlContainerBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlContainerBody_Magic != cdlcontainerbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();

    if (cyg_extreme == zeal) {
        std::vector<CdlNode>::const_iterator node_i;
        for (node_i = contents.begin(); node_i != contents.end(); node_i++) {
            if (!((*node_i)->check_this(cyg_quick))) {
                return false;
            }
        }
    }
    return CdlNodeBody::check_this(zeal);
}

//}}}

//}}}
//{{{  CdlLoadableBody                  

//{{{  Constructor                              

// ----------------------------------------------------------------------------
// A loadable object keeps track of all the nodes read in from a
// particular script, in an "owned" vector. Simply keeping things in a
// hierarchy is not enough because of possible re-parenting. Actual
// updates of the owned vector happen inside the CdlToplevel
// add_node() and remove_node() family.

CdlLoadableBody::CdlLoadableBody(CdlToplevel toplevel, std::string dir)
    : CdlContainerBody()
{
    CYG_REPORT_FUNCNAME("CdlLoadable:: constructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_CLASSC(toplevel);

    // Initialize enough of the object to support check_this()
    directory   = dir;
    interp      = 0;
    remove_node_loadables_position = -1;
    cdlloadablebody_cookie = CdlLoadableBody_Magic;
    
    // The owned vector takes care of itself. It is necessary
    // to create a new slave interpreter, using the master
    // interpreter from the toplevel.
    CdlInterpreter master = toplevel->get_interpreter();
    CYG_ASSERTC(0 != master);
    interp      = master->create_slave(this, false);
    interp->push_context(this->get_name());
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

// Needed by derived classes, but should never actually be used.
CdlLoadableBody::CdlLoadableBody()
{
    CYG_FAIL("CdlLoadable default constructor should never get invoked");
}

//}}}
//{{{  Destructor                               

// ----------------------------------------------------------------------------
// The loadable destructor. This gets invoked from two places: after an
// unsuccessful load operation, and from inside the transaction commit
// code. Either way most of the clean-up will have happened already:
// all the nodes will have been removed from the toplevel's hierarchy,
// and all property references to and from this loadable will have been
// unbound.
//
// Since all nodes belonging to the loadable are also present in
// the owned vector, they must be destroyed before this destructor
// completes. Hence clearing out the contents cannot be left to
// the base CdlContainer destructor.

CdlLoadableBody::~CdlLoadableBody()
{
    CYG_REPORT_FUNCNAME("CdlLoadable:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    // Make sure that the loadable has already been removed from the
    // hierarchy: it should not have a toplevel or a parent.
    CYG_PRECONDITIONC(0 == toplevel);
    CYG_PRECONDITIONC(0 == parent);
    
    // Containers must have been created before any of their contents.
    // The only way to reverse this involves a parent property, but
    // all such properties will have been unbound already such that
    // the nodes can be safely deleted. The only worry is that
    // loadables own themselves.
    int i;
    for (i = owned.size() - 1; i >= 0; i--) {
        CdlNode node = owned[i];
        CYG_LOOP_INVARIANT_CLASSC(node);

        if (node != this) {
            CdlToplevelBody::remove_node(this, node->parent, node);
            delete node;
        }
    }
    
    // Now there should be exactly one entry in the owned vector,
    // the loadable itself. We already know that this is no longer
    // part of the toplevel and it does not have a parent, so
    // the only field we need to worry about is the owner.
    CYG_ASSERTC(1 == owned.size());
    CYG_ASSERTC(this == owned[0]);
    this->owner = 0;

    // Strictly speaking the owned vector should be clear by now,
    // but remove_node() does not actually bother to clear it.
    owned.clear();
    
    // The loadable should now be empty. It remains to clean up
    // a few odds and ends.
    cdlloadablebody_cookie = CdlLoadableBody_Invalid;
    CYG_ASSERTC(0 == owned.size());
    delete interp;
    interp      = 0;
    directory   = "";

    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Simple information access                

// ----------------------------------------------------------------------------

const std::vector<CdlNode>&
CdlLoadableBody::get_owned() const
{
    CYG_REPORT_FUNCNAME("CdlLoadable::get_owned");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return owned;
}

bool
CdlLoadableBody::owns(CdlConstNode node) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlLoadable::owns", "result %d");
    CYG_REPORT_FUNCARG2XV(this, node);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);

    bool result = false;
    std::vector<CdlNode>::const_iterator i = std::find(owned.begin(), owned.end(), node);
    if (i != owned.end()) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

CdlInterpreter
CdlLoadableBody::get_interpreter() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlLoadable::get_interpreter", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlInterpreter result = interp;
    CYG_REPORT_RETVAL(result);
    return result;
}

std::string
CdlLoadableBody::get_directory() const
{
    CYG_REPORT_FUNCNAME("CdlLoadable::get_directory");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return directory;
}

//}}}
//{{{  Bind/unbind support                      

// ----------------------------------------------------------------------------
// Binding a loadable. This involves checking every property of every node
// in the loadable, which the properties do themselves by a suitable
// update() virtual function. Next, there may be properties in the
// existing configuration which could not previously be bound: there
// will be structural conflicts for all of these. Once all the pointers
// go to the right places it is possible to calculate the default values
// and generally process the properties. Finally each node's active
// state is checked - the default inactive state will be inappropriate
// in many cases.
//
// FIXME: error recovery?

void
CdlLoadableBody::bind(CdlTransaction transaction)
{
    CYG_REPORT_FUNCNAME("CdlLoadable::bind");
    CYG_REPORT_FUNCARG2XV(this, transaction);
    CYG_INVARIANT_THISC(CdlLoadableBody);
    CYG_INVARIANT_CLASSC(CdlTransactionBody, transaction);

    // The loadable must already be part of the hierarchy.
    CdlToplevel toplevel = this->get_toplevel();
    CYG_ASSERT_CLASSC(toplevel);
        
    // As a first step, bind all references in this loadable.
    // This is achieved via a Loaded update.
    const std::vector<CdlNode>& nodes = this->get_owned();
    std::vector<CdlNode>::const_iterator node_i;
    for (node_i = nodes.begin(); node_i != nodes.end(); node_i++) {
        const std::vector<CdlProperty>& properties = (*node_i)->get_properties();
        std::vector<CdlProperty>::const_iterator prop_i;
        for (prop_i = properties.begin(); prop_i != properties.end(); prop_i++) {
            (*prop_i)->update(transaction, *node_i, 0, CdlUpdate_Loaded);
        }
    }

    // Next, look for all structural conflicts which are unresolved
    // references and which can now be resolved. It is necessary
    // to check per-transaction structural conflicts, plus those
    // in any parent transactions, plus the global ones.
    std::list<CdlConflict>::const_iterator conf_i;
    CdlTransaction current_transaction = transaction;
    do {
        CYG_ASSERT_CLASSC(current_transaction);
        const std::list<CdlConflict>& new_structural_conflicts = current_transaction->get_new_structural_conflicts();
        
        for (conf_i = new_structural_conflicts.begin(); conf_i != new_structural_conflicts.end(); ) {
            
            CdlConflict conflict = *conf_i++;
            CYG_LOOP_INVARIANT_CLASSC(conflict);
            
            CdlConflict_Unresolved unresolved_conflict = dynamic_cast<CdlConflict_Unresolved>(conflict);
            if ((0 != unresolved_conflict) && !transaction->has_conflict_been_cleared(conflict)) {
                CdlNode dest = toplevel->lookup(unresolved_conflict->get_target_name());
                if (0 != dest) {
                    CdlNode     node = unresolved_conflict->get_node();
                    CdlProperty prop = unresolved_conflict->get_property();
                    prop->update(transaction, node, dest, CdlUpdate_Created);
                }                    
            }
        }
        current_transaction = current_transaction->get_parent();
    } while (0 != current_transaction);

    const std::list<CdlConflict>& structural_conflicts = toplevel->get_all_structural_conflicts();
    for (conf_i = structural_conflicts.begin(); conf_i != structural_conflicts.end(); ) {
        
        CdlConflict conflict = *conf_i++;
        CYG_LOOP_INVARIANT_CLASSC(conflict);
            
        CdlConflict_Unresolved this_conflict = dynamic_cast<CdlConflict_Unresolved>(conflict);
        if ((0 != this_conflict) && !transaction->has_conflict_been_cleared(conflict)) {
            CdlNode dest = toplevel->lookup(this_conflict->get_target_name());
            if (0 != dest) {
                CdlNode     node = this_conflict->get_node();
                CdlProperty prop = this_conflict->get_property();
                prop->update(transaction, node, dest, CdlUpdate_Created);
            }
        }
    }

    // Conflict resolution has happened. Next it is time
    // to evaluate default_value expressions and the like
    // in the new loadable.
    for (node_i = nodes.begin(); node_i != nodes.end(); node_i++) {
        const std::vector<CdlProperty>& properties = (*node_i)->get_properties();
        std::vector<CdlProperty>::const_iterator prop_i;
        for (prop_i = properties.begin(); prop_i != properties.end(); prop_i++) {
            (*prop_i)->update(transaction, *node_i, 0, CdlUpdate_Init);
        }
    }

    // Nodes start off inactive. Check each one whether or not it
    // should be active.
    // NOTE: possibly this should be done via a per-node init
    // update instead.
    for (node_i = nodes.begin(); node_i != nodes.end(); node_i++) {
        bool current_state = transaction->is_active(*node_i);
        bool new_state     = (*node_i)->test_active(transaction);
        if (current_state != new_state) {
            transaction->set_active(*node_i, new_state);
        }
    }

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------

void
CdlLoadableBody::unbind(CdlTransaction transaction)
{
    CYG_REPORT_FUNCNAME("CdlLoadable::unbind");
    CYG_REPORT_FUNCARG2XV(this, transaction);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);
    
    // First take care of all references to nodes in the loadable
    // that is disappearing. This involves a Destroyed update.
    const std::vector<CdlNode>& nodes = this->get_owned();
    std::vector<CdlNode>::const_iterator node_i;
    for (node_i = nodes.begin(); node_i != nodes.end(); node_i++) {
        // The update will remove referrer objects, so it is best
        // to work from the back.
        std::vector<CdlReferrer>& referrers = (*node_i)->referrers;
        std::vector<CdlReferrer>::reverse_iterator ref_i;
        for (ref_i = referrers.rbegin(); ref_i != referrers.rend(); ref_i = referrers.rbegin()) {
            ref_i->update(transaction, *node_i, CdlUpdate_Destroyed);
            CYG_LOOP_INVARIANT(ref_i != referrers.rbegin(), "the vector should have shrunk");
        }
    }

    // Now repeat the loop, but unbind references from the unloaded objects
    // to ones which are going to stay loaded. This will not cause
    // the properties to disappear.
    for (node_i = nodes.begin(); node_i != nodes.end(); node_i++) {
        const std::vector<CdlProperty>& properties = (*node_i)->get_properties();
        std::vector<CdlProperty>::const_iterator prop_i;
        for (prop_i = properties.begin(); prop_i != properties.end(); prop_i++) {
            (*prop_i)->update(transaction, *node_i, 0, CdlUpdate_Unloading);
        }
    }

    // Eliminate any conflicts that belong to this loadable.
    // FIXME: why is his necessary? Should these conflicts not get
    // eliminated by the above property iterations?
    std::list<CdlConflict>::const_iterator conf_i;
    const std::list<CdlConflict>& global_conflicts = toplevel->get_all_conflicts();
    for (conf_i = global_conflicts.begin(); conf_i != global_conflicts.end(); ) {
        CdlConflict conflict = *conf_i++;
        CYG_LOOP_INVARIANT_CLASSC(conflict);
        CdlNode     node     = conflict->get_node();
        if ((node->get_owner() == this) && !transaction->has_conflict_been_cleared(conflict)) {
            transaction->clear_conflict(conflict);
        }
    }
    const std::list<CdlConflict>& global_structural_conflicts = toplevel->get_all_structural_conflicts();
    for (conf_i = global_structural_conflicts.begin(); conf_i != global_structural_conflicts.end(); ) {
        CdlConflict conflict = *conf_i++;
        CYG_LOOP_INVARIANT_CLASSC(conflict);
        CdlNode     node     = conflict->get_node();
        if ((node->get_owner() == this) && !transaction->has_conflict_been_cleared(conflict)) {
            transaction->clear_conflict(conflict);
        }
    }
    const std::list<CdlConflict>& transaction_conflicts = transaction->get_new_conflicts();
    for (conf_i = transaction_conflicts.begin(); conf_i != transaction_conflicts.end(); ) {
        CdlConflict conflict = *conf_i++;
        CYG_LOOP_INVARIANT_CLASSC(conflict);
        CdlNode     node     = conflict->get_node();
        if (node->get_owner() == this) {
            transaction->clear_conflict(conflict);
        }
    }
    const std::list<CdlConflict>& transaction_structural_conflicts = transaction->get_new_structural_conflicts();
    for (conf_i = transaction_structural_conflicts.begin(); conf_i != transaction_structural_conflicts.end(); ) {
        CdlConflict conflict = *conf_i++;
        CYG_LOOP_INVARIANT_CLASSC(conflict);
        CdlNode     node     = conflict->get_node();
        if (node->get_owner() == this) {
            transaction->clear_conflict(conflict);
        }
    }

    // FIXME: how about cleanup_orphans()
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// These members are invoked for load and unload operations.
//
// Committing a load does not require anything, the loadable has
// already been fully bound and all propagation has happened.

void
CdlLoadableBody::transaction_commit_load(CdlTransaction transaction, CdlLoadable loadable)
{
    CYG_REPORT_FUNCNAME("CdlLoadable::transaction_commit_load");
    CYG_REPORT_FUNCARG2XV(transaction, loadable);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(loadable);

    CYG_UNUSED_PARAM(CdlTransaction, transaction);
    CYG_UNUSED_PARAM(CdlLoadable, loadable);
    
    CYG_REPORT_RETURN();
}

// Cancelling a load is more difficult. The loadable has to be
// unbound, removed from the toplevel, and deleted. If any of
// this fails then we are in trouble, there is no easy way to
// recover.
void
CdlLoadableBody::transaction_cancel_load(CdlTransaction transaction, CdlLoadable loadable)
{
    CYG_REPORT_FUNCNAME("CdlLoadable::transaction_cancel_load");
    CYG_REPORT_FUNCARG2XV(transaction, loadable);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(loadable);

    CdlToplevel toplevel = transaction->get_toplevel();
    CYG_PRECONDITION_CLASSC(toplevel);
    CYG_ASSERTC(toplevel == loadable->get_toplevel());

    loadable->unbind(transaction);
    toplevel->remove_loadable_from_toplevel(loadable);
    delete loadable;
    
    CYG_REPORT_RETURN();
}

// Committing an unload means that the loadable can now be deleted.
// It should already be unbound and removed from the toplevel.
void
CdlLoadableBody::transaction_commit_unload(CdlTransaction transaction, CdlLoadable loadable)
{
    CYG_REPORT_FUNCNAME("CdlLoadable::transaction_commit_unload");
    CYG_REPORT_FUNCARG2XV(transaction, loadable);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(loadable);

    CYG_UNUSED_PARAM(CdlTransaction, transaction);
    delete loadable;

    CYG_REPORT_RETURN();
}

// Cancelling an unload means that the loadable has to be re-added
// to the hierarchy and then rebound. This implies that value
// propagation needs to happen. However, since all value changes
// since the very start of the transaction are held inside the
// transaction and will be eliminated, the original state will
// be restored anyway so the propagation is not actually required.
void
CdlLoadableBody::transaction_cancel_unload(CdlTransaction transaction, CdlLoadable loadable)
{
    CYG_REPORT_FUNCNAME("CdlLoadable::transaction_cancel_unload");
    CYG_REPORT_FUNCARG2XV(transaction, loadable);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(loadable);

    CdlToplevel toplevel = transaction->get_toplevel();
    CYG_PRECONDITION_CLASSC(toplevel);
    toplevel->add_loadable_to_toplevel(loadable);
    CYG_ASSERT_CLASSC(loadable);
    loadable->bind(transaction);
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  File search facilities                   

// ----------------------------------------------------------------------------
// File search facilities. Given a file name such as hello.cxx from a compile
// property, or doc.html from a doc property, find the corresponding filename,
// for example /usr/local/eCos/kernel/v1_3/doc/threads.html#create
//
// The second argument (default value "") indicates a preferred directory
// where searching should begin. This would be src for a source file,
// doc for a URL, etc.
//
// For some properties the data may refer to a URL rather than to a local
// filename. This is controlled by the third argument, allow_urls.
// If false then only local filenames will be considered. allow_urls
// also controls whether or not anchor processing is performed.
//
// RFC1807: a URL consists of <scheme>:<rest>, where <scheme> can be
// any sequence of lower-case letters, digits, plus, dot or hyphen. It
// is recommended that upper-case letters should be accepted as well.
//
// RFC1807: an anchor is everything after the first # in the URL.

static char find_absolute_file_script[] = "                                     \n\
set cdl_anchor \"\"                                                             \n\
if {$::cdl_allow_urls} {                                                        \n\
  if { [regexp -- {^[a-zA-Z+.-]*:.*$} $::cdl_target] } {                        \n\
      return $::cdl_target                                                      \n\
  }                                                                             \n\
  set tmp \"\"                                                                  \n\
  set non_anchor \"\"                                                           \n\
  if { [regexp -- {^([^#])(#.*$)} $::cdl_target tmp non_anchor cdl_anchor] } {  \n\
      set ::cdl_target $non_anchor                                              \n\
  }                                                                             \n\
}                                                                               \n\
if {$::cdl_prefdir != \"\"} {                                                   \n\
    set filename [file join $::cdl_topdir $::cdl_pkgdir $::cdl_prefdir $::cdl_target]         \n\
    if {[file exists $filename]} {                                              \n\
        return \"[set filename][set cdl_anchor]\"                               \n\
    }                                                                           \n\
}                                                                               \n\
set filename [file join $::cdl_topdir $::cdl_pkgdir $::cdl_target]              \n\
if {[file exists $filename]} {                                                  \n\
    return \"[set filename][set cdl_anchor]\"                                   \n\
}                                                                               \n\
return \"\"                                                                     \n\
";

std::string
CdlLoadableBody::find_absolute_file(std::string filename, std::string dirname, bool allow_urls) const
{
    CYG_REPORT_FUNCNAME("CdlLoadable::find_absolute_file");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != filename);

    // These variable names should be kept in step with CdlBuildable::update_all_build_info()
    interp->set_variable("::cdl_topdir",  get_toplevel()->get_directory());
    interp->set_variable("::cdl_pkgdir",  directory);
    interp->set_variable("::cdl_prefdir", dirname);
    interp->set_variable("::cdl_target",  filename);
    interp->set_variable("::cdl_allow_urls", allow_urls ? "1" : "0");

    std::string result;
    int tmp = interp->eval(find_absolute_file_script, result);
    if (tmp != TCL_OK) {
        result = "";
    }
    
    // Replace any backslashes in the repository with forward slashes.
    // The latter are used throughout the library
    // NOTE: this is not i18n-friendly.
    for (unsigned int i = 0; i < result.size(); i++) {
        if ('\\' == result[i]) {
            result[i] = '/';
        }
    }

    CYG_REPORT_RETURN();
    return result;
}

static char find_relative_file_script[] = "                                     \n\
if {$::cdl_prefdir != \"\"} {                                                   \n\
    set filename [file join $::cdl_prefdir $::cdl_target]                       \n\
    if {[file exists [file join $::cdl_topdir $::cdl_pkgdir $filename]]} {      \n\
        return $filename                                                        \n\
    }                                                                           \n\
}                                                                               \n\
set filename $::cdl_target                                                      \n\
if {[file exists [file join $::cdl_topdir $::cdl_pkgdir $filename]]} {          \n\
    return \"[set filename][set cdl_anchor]\"                                   \n\
}                                                                               \n\
return \"\"                                                                     \n\
";

std::string
CdlLoadableBody::find_relative_file(std::string filename, std::string dirname) const
{
    CYG_REPORT_FUNCNAME("CdlLoadable::find_relative_file");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != filename);

    // These variable names should be kept in step with CdlBuildable::update_all_build_info()
    interp->set_variable("::cdl_topdir",  get_toplevel()->get_directory());
    interp->set_variable("::cdl_pkgdir",  directory);
    interp->set_variable("::cdl_prefdir", dirname);
    interp->set_variable("::cdl_target",  filename);

    std::string result;
    int tmp = interp->eval(find_relative_file_script, result);
    if (tmp != TCL_OK) {
        result = "";
    }
    
    // Replace any backslashes in the repository with forward slashes.
    // The latter are used throughout the library
    // NOTE: this is not i18n-friendly.
    for (unsigned int i = 0; i < result.size(); i++) {
        if ('\\' == result[i]) {
            result[i] = '/';
        }
    }

    CYG_REPORT_RETURN();
    return result;
}

static char has_subdirectory_script[] = "                               \n\
set dirname [file join $::cdl_topdir $::cdl_pkgdir $::cdl_target]       \n\
if {[file isdirectory $dirname] == 0} {                                 \n\
    return 0                                                            \n\
}                                                                       \n\
return 1                                                                \n\
";

bool
CdlLoadableBody::has_subdirectory(std::string name) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlLoadable::has_subdirectory", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != name);

    bool        result = false;
    
    interp->set_variable("::cdl_topdir",  get_toplevel()->get_directory());
    interp->set_variable("::cdl_pkgdir",  directory);
    interp->set_variable("::cdl_target",  name);

    std::string tcl_result;
    int tmp = interp->eval(has_subdirectory_script, tcl_result);
    if ((TCL_OK == tmp) && ("1" == tcl_result)) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  Misc                                     

// ----------------------------------------------------------------------------

std::string
CdlLoadableBody::get_class_name() const
{
    CYG_REPORT_FUNCNAME("CdlLoadable::get_class_name");
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return "loadable";
}

//}}}
//{{{  check_this()                             

// ----------------------------------------------------------------------------
bool
CdlLoadableBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlLoadableBody_Magic != cdlloadablebody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();

    if ((zeal == cyg_extreme) || (zeal == cyg_thorough)) {
        std::vector<CdlNode>::const_iterator node_i;
        for (node_i = owned.begin(); node_i != owned.end(); node_i++) {
            if ((!(*node_i)->check_this(cyg_quick)) || ((*node_i)->get_owner() != this)) {
                return false;
            }
        }
    }
    return CdlContainerBody::check_this(zeal);
}

//}}}

//}}}
//{{{  CdlToplevelBody                  

//{{{  Constructor                              

// ----------------------------------------------------------------------------
// A toplevel is a container without a parent or owner. It keeps track
// of all the names in the hierarchy, thus guaranteeing uniqueness and
// providing a quick lookup facility.
//
// The member functions add_node() and remove_node() are the only
// way of modifying the hierarchy. Adding a node with a zero parent
// means adding it to a special container, Orphans.
//
// An interpreter object must be created explicitly, preventing
// toplevel objects from being statically allocated (although
// it is possible to play tricks with utility classes...)
// There are too many possible error conditions when creating
// an interpreter, so this should not happen until the world
// is ready to deal with such errors.

CdlToplevelBody::CdlToplevelBody(CdlInterpreter interp_arg, std::string directory_arg)
    : CdlContainerBody()
{
    CYG_REPORT_FUNCNAME("CdlToplevel:: constructor");
    CYG_REPORT_FUNCARG2XV(this, interp_arg);
    CYG_PRECONDITION_CLASSC(interp_arg);

    // The STL containers will take care of themselves.
    interp      = interp_arg;
    directory   = directory_arg;
    transaction = 0;

    // A toplevel is always active, override the default setting for a node
    active      = true;
    
    // Make the object valid before creating the orphans container.
    orphans     = 0;
    description = "";
    cdltoplevelbody_cookie = CdlToplevelBody_Magic;
    
    // Arguably creating the orphans container should be left until
    // it is actually needed. The advantage of creating it at the
    // start is that it will appear in a fixed location in the contents,
    // right at the start. Arguably the end would be better, but the
    // end can move as loadables get added and removed.
    //
    // The GUI code will probably want to ignore any empty
    // containers that are not valuables and not user-visible.
    orphans = new CdlContainerBody("orphans");
    add_node(0, this, orphans);

    // Let the interpreter know about its owning toplevel, as well as
    // vice versa.
    interp->set_toplevel(this);
    
    // The orphans container needs to be active as well.
    orphans->active = true;

    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Destructor                               

// ----------------------------------------------------------------------------
// The toplevel should have been mostly cleared already, by the
// appropriate derived class. Without any loadables there should not
// be any conflicts. It is necessary to clean up the orphans
// container, since that was created by the CdlToplevel constructor.
// If there are any other special nodes then these should have been
// cleared by higher level code.

CdlToplevelBody::~CdlToplevelBody()
{
    CYG_REPORT_FUNCNAME("CdlToplevel:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_PRECONDITIONC(0 == loadables.size());
    CYG_PRECONDITIONC(0 == conflicts.size());
    CYG_PRECONDITIONC(0 == structural_conflicts.size());
    CYG_PRECONDITIONC(0 == transaction);

    CYG_PRECONDITIONC(0 != orphans);
    this->remove_node_from_toplevel(orphans);
    CdlToplevelBody::remove_node(0, this, orphans);
    delete orphans;
    orphans = 0;

    CYG_PRECONDITIONC(0 == contents.size());
    
    cdltoplevelbody_cookie = CdlToplevelBody_Magic;
    description = "";
    limbo.clear();
    unsupported_savefile_toplevel_strings.clear();
    unsupported_savefile_commands.clear();
    unsupported_savefile_subcommands.clear();

    // Since the interpreter is not created by the toplevel, it is
    // not destroyed with the toplevel either. This leaves a potential
    // big memory leak in application code.
    interp = 0;

    CYGDBG_MEMLEAK_DESTRUCTOR();

    CYG_REPORT_RETURN();
}

//}}}
//{{{  Adding and removing nodes                

// ----------------------------------------------------------------------------
// Adding and removing a node, and changing a parent.
//
// These routines allow the hierarchy to be manipulated. All nodes should
// exist in a hierarchy below a toplevel, except for brief periods after
// construction and during destruction.
//
// Most nodes will belong to a loadable. An owner of 0 is allowed, for
// objects internal to the library such as the orphans container.
// Everything else must have an owner, and specifically a loadable owns
// itself.

void
CdlToplevelBody::add_node(CdlLoadable owner, CdlContainer parent, CdlNode node)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::add_node");
    CYG_REPORT_FUNCARG4XV(this, owner, parent, node);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_ZERO_OR_CLASSC(owner);
    CYG_PRECONDITION_CLASSC(parent);
    CYG_PRECONDITION_CLASSC(node);

    // The node must not be in the hierarchy already.
    CYG_ASSERTC(0 == node->toplevel);
    CYG_ASSERTC(0 == node->owner);
    CYG_ASSERTC(0 == node->parent);
    
    // The node's name should be unique. Checks for that should have happened
    // in higher-level code.
    CYG_ASSERTC(lookup_table.find(node->name) == lookup_table.end());
    node->toplevel              = this;
    lookup_table[node->name]    = node;

    node->owner                 = owner;
    if (0 != owner) {
        owner->owned.push_back(node);
    }

    // If the node is in fact a loadable, it should own itself and
    // in addition the toplevel class keeps track of its loadables
    // in a separate vector.
    if (0 != dynamic_cast<CdlLoadable>(node)) {
        CYG_ASSERTC(owner == dynamic_cast<CdlLoadable>(node));
        this->loadables.push_back(owner);
    }
    
    if (0 == parent) {
        parent = orphans;
    }
    node->parent = parent;
    parent->contents.push_back(node);

    CYG_REPORT_RETURN();
}

// Removing a node from a toplevel. This is the first step in deleting
// a node: the step may be undone by a call to add_node_to_toplevel(),
// or completed by a call to remove_node(). Removing a node from the
// toplevel involves undoing the name->node mapping. In the case
// of loadables, it also involves removing the node from the toplevel's
// contents and loadables containers.

void
CdlToplevelBody::remove_node_from_toplevel(CdlNode node)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::remove_node_from_toplevel");
    CYG_REPORT_FUNCARG2XV(this, node);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);
    CYG_ASSERTC(this == node->toplevel);
    CYG_ASSERTC(lookup_table[node->name] == node);

    node->toplevel = 0;
    lookup_table.erase(node->name);

    CdlLoadable loadable = dynamic_cast<CdlLoadable>(node);
    if (0 != loadable) {
        CYG_ASSERTC(loadable == node->owner);
        CYG_ASSERTC(this == node->parent);

        // Because remove_node_from_toplevel() is reversible, the
        // loadable should reappear in its old position. Hence we
        // had better keep track of that position. Note that
        // this code assumed that the remove_node and add_node
        // calls are exactly reversed.
        int i;
        for (i = 0; i < (int) this->contents.size(); i++) {
            if (this->contents[i] == node) {
                break;
            }
        }
        CYG_ASSERTC(i < (int) this->contents.size());
        node->remove_node_container_position = i;
        this->contents.erase(this->contents.begin() + i);
        node->parent = 0;

        // It is not clear that preserving the order of the loadables
        // in the toplevel is useful, but it is harmless.
        for (i = 0; i < (int) this->loadables.size(); i++) {
            if (this->loadables[i] == loadable) {
                break;
            }
        }
        CYG_ASSERTC(i < (int) this->loadables.size());
        loadable->remove_node_loadables_position = i;
        this->loadables.erase(this->loadables.begin() + i);
    }
    
    CYG_REPORT_RETURN();
}

void
CdlToplevelBody::remove_loadable_from_toplevel(CdlLoadable loadable)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::remove_loadable_from_toplevel");
    CYG_REPORT_FUNCARG2XV(this, loadable);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(loadable);

    const std::vector<CdlNode>& contents = loadable->get_owned();
    for (int i = contents.size() - 1; i >= 0; i--) {
        CdlToplevel toplevel = contents[i]->get_toplevel();
        CYG_LOOP_INVARIANT_ZERO_OR_CLASSC(toplevel);
        if (0 != toplevel) {
            CYG_LOOP_INVARIANTC(this == toplevel);
            this->remove_node_from_toplevel(contents[i]);
        }
    }
    
    CYG_REPORT_RETURN();
}

// Re-adding a node to a toplevel. This needs to undo all of the changes
// that may have been done by remove_node_from_toplevel() above.
void
CdlToplevelBody::add_node_to_toplevel(CdlNode node)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::add_node_to_toplevel");
    CYG_REPORT_FUNCARG2XV(this, node);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);
    CYG_ASSERTC(0 == node->toplevel);
    CYG_ASSERTC(0 != node->owner);

    CYG_ASSERTC(lookup_table.find(node->name) == lookup_table.end());
    node->toplevel = this;
    lookup_table[node->name] = node;

    CdlLoadable loadable = dynamic_cast<CdlLoadable>(node);
    if (0 != loadable) {
        CYG_ASSERTC(loadable == node->owner);
        CYG_ASSERTC(0 == node->parent);
        CYG_ASSERTC(-1 != node->remove_node_container_position);
        CYG_ASSERTC(node->remove_node_container_position <= (int) this->contents.size());

        this->contents.insert(this->contents.begin() + node->remove_node_container_position, node);
        node->remove_node_container_position = -1;
        node->parent = this;

        CYG_ASSERTC(-1 != loadable->remove_node_loadables_position);
        this->loadables.insert(this->loadables.begin() + loadable->remove_node_loadables_position, loadable);
        loadable->remove_node_loadables_position = -1;
    }

    CYG_REPORT_RETURN();
}

void
CdlToplevelBody::add_loadable_to_toplevel(CdlLoadable loadable)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::add_loadable_to_toplevel");
    CYG_REPORT_FUNCARG2XV(this, loadable);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(loadable);
    
    const std::vector<CdlNode>& contents = loadable->get_owned();
    for (int i = 0; i < (int) contents.size(); i++) {
        this->add_node_to_toplevel(contents[i]);
    }

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// The second stage remove operation. This cannot be undone, and
// happens just before the node gets deleted and after a succesful
// remove_node_from_toplevel().
void
CdlToplevelBody::remove_node(CdlLoadable owner, CdlContainer parent, CdlNode node)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::remove_node");
    CYG_REPORT_FUNCARG3XV(owner, parent, node);
    CYG_PRECONDITION_CLASSC(node);
    CYG_PRECONDITION_ZERO_OR_CLASSC(owner);
    CYG_PRECONDITION_ZERO_OR_CLASSC(parent);
    CYG_PRECONDITIONC(node->owner  == owner);
    CYG_PRECONDITIONC(node->parent == parent);
    CYG_PRECONDITIONC(0 == node->toplevel);

    if (0 != owner) {
        node->owner = 0;
        owner->owned.erase(std::find(owner->owned.begin(), owner->owned.end(), node));
    }
    if (0 != parent) {
        node->parent = 0;
        parent->contents.erase(std::find(parent->contents.begin(), parent->contents.end(), node));
    }

    CYG_REPORT_RETURN();
}

// Changing a parent does not affect the node's standing in terms of the
// overall hierarchy or its owner, only the parent field.
void
CdlToplevelBody::change_parent(CdlLoadable owner, CdlContainer old_parent, CdlContainer new_parent, CdlNode node, int pos)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::change_parent");
    CYG_REPORT_FUNCARG6XV(this, owner, parent, new_parent, node, pos);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(old_parent);
    CYG_PRECONDITION_ZERO_OR_CLASSC(new_parent);
    CYG_PRECONDITION_CLASSC(node);
    CYG_PRECONDITIONC(node->owner  == owner);
    CYG_PRECONDITIONC(node->parent == old_parent);
    CYG_PRECONDITIONC(this == node->toplevel);
    CYG_PRECONDITIONC(lookup_table[node->name] == node);

    if (0 == new_parent) {
        new_parent = orphans;
    }
    old_parent->contents.erase(std::find(old_parent->contents.begin(), old_parent->contents.end(), node));
    node->parent = 0;

    if (-1 == pos) {
        new_parent->contents.push_back(node);
    } else {
        CYG_ASSERTC(pos <= (int) new_parent->contents.size());
        new_parent->contents.insert(new_parent->contents.begin() + pos, node);
    }
    node->parent = new_parent;

    CYG_REPORT_RETURN();
}

// Cleaning up orphans.
//
// Right now this is only relevant for interfaces. Consider the case
// where a loadable is being removed and that loadable defines an
// interface. There may be other loadables which still have
// "implements" properties affecting that interface, so instead of
// deleting the cdl_interface object it is necessary to turn it into
// an auto-generated orphan. At some stage there may no longer be
// any references to an interface, in which case it can be removed
// safely.
//
// In practice it is quite hard to do a clean-up purely on the basis
// of implements properties, for example there may be an external
// "requires" property as well which would need to have its references
// cleaned up, then the expression needs to get re-evaluated, etc.
// The transaction class does not currently provide a clean way
// in which a single object can be destroyed. Instead the code below
// checks for any references whose source is not the interface itself.

void
CdlToplevelBody::cleanup_orphans()
{
    CYG_REPORT_FUNCNAME("CdlToplevel::cleanup_orphans");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    // First figure out whether or not there are any interfaces along
    // these lines.
    std::vector<CdlInterface>   interfaces;
    const std::vector<CdlNode>& contents = orphans->get_contents();
    std::vector<CdlNode>::const_iterator node_i;

    for (node_i = contents.begin(); node_i != contents.end(); node_i++) {
        CdlInterface intface = dynamic_cast<CdlInterface>(*node_i);
        if (0 == intface) {
            continue;
        }
        const std::vector<CdlReferrer>& referrers = intface->get_referrers();
        std::vector<CdlReferrer>::const_iterator ref_i;
        for (ref_i = referrers.begin(); ref_i != referrers.end(); ref_i++) {
            if (ref_i->get_source() != intface) {
                break;
            }
        }
        if (ref_i == referrers.end()) {
            // None of the existing references involve an "implements" property, so
            // this interface can be deleted.
            interfaces.push_back(intface);
        }
    }
    
    if (0 != interfaces.size()) {
        CYG_FAIL("Not yet implemented");
    }
}

//}}}
//{{{  Basic information                        

// ----------------------------------------------------------------------------

const std::vector<CdlLoadable>&
CdlToplevelBody::get_loadables() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlToplevel::get_loadables", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    const std::vector<CdlLoadable>& result = loadables;
    CYG_REPORT_RETVAL(&result);
    return result;
}


CdlNode
CdlToplevelBody::lookup(const std::string name) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlToplevel::lookup", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != name);

    CdlNode result = 0;
    std::map<std::string,CdlNode>::const_iterator i = lookup_table.find(name);
    if (i != lookup_table.end()) {
        result = i->second;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

CdlInterpreter
CdlToplevelBody::get_interpreter() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlToplevel::get_interpreter", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlInterpreter result = interp;
    CYG_REPORT_RETVAL(result);
    return result;
}

std::string
CdlToplevelBody::get_description() const
{
    CYG_REPORT_FUNCNAME("CdlToplevel::get_description");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return description;
}

void
CdlToplevelBody::set_description(std::string new_description)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::set_description");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    description = new_description;

    CYG_REPORT_RETURN();
}

std::string
CdlToplevelBody::get_directory() const
{
    CYG_REPORT_FUNCNAME("CdlToplevel::get_directory");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return directory;
}

std::string
CdlToplevelBody::get_class_name() const
{
    CYG_REPORT_FUNCNAME("CdlToplevel::get_class_name");
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return "toplevel";
}

CdlTransaction
CdlToplevelBody::get_active_transaction() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlToplevel::get_active_transaction", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlTransaction result = transaction;
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  Conflict support                         

// ----------------------------------------------------------------------------
const std::list<CdlConflict>&
CdlToplevelBody::get_all_conflicts() const
{
    CYG_REPORT_FUNCNAME("CdlToplevel::get_all_conflicts");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    const std::list<CdlConflict>& result = conflicts;

    CYG_REPORT_RETURN();
    return result;
}

const std::list<CdlConflict>&
CdlToplevelBody::get_all_structural_conflicts() const
{
    CYG_REPORT_FUNCNAME("CdlToplevel::get_all_structural_conflicts");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    const std::list<CdlConflict>& result = structural_conflicts;

    CYG_REPORT_RETURN();
    return result;
}

// ----------------------------------------------------------------------------
// Resolve one or more conflicts. This involves creating a new transaction,
// invoking the per-transaction resolve code, and then CdlTransaction::body()
// takes care of everything else like propagation, further inference,
// callbacks, committing, ...
void
CdlToplevelBody::resolve_conflicts(const std::vector<CdlConflict>& conflicts_arg)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::resolve_conflicts");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlTransaction transact = CdlTransactionBody::make(this);
    
    std::vector<CdlConflict>::const_iterator conf_i;
    for (conf_i = conflicts_arg.begin(); conf_i != conflicts_arg.end(); conf_i++) {
        CYG_LOOP_INVARIANT_CLASSC(*conf_i);
        CYG_LOOP_INVARIANTC(0 == (*conf_i)->get_transaction());

        if (((*conf_i)->resolution_implemented()) &&
            !transact->has_conflict_been_cleared(*conf_i) &&
            !(*conf_i)->has_known_solution() &&
            !(*conf_i)->has_no_solution() ) {
            transact->resolve(*conf_i);
        }
    }
    transact->body();
    delete transact;
    
    CYG_REPORT_RETURN();
}

void
CdlToplevelBody::resolve_all_conflicts()
{
    CYG_REPORT_FUNCNAME("CdlToplevel::resolve_all_conflicts");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlTransaction transact = CdlTransactionBody::make(this);
    std::list<CdlConflict>::const_iterator conf_i;

    for (conf_i = conflicts.begin(); conf_i != conflicts.end(); conf_i++) {
        CYG_LOOP_INVARIANT_CLASSC(*conf_i);
        CYG_LOOP_INVARIANTC(0 == (*conf_i)->get_transaction());
        if ((*conf_i)->resolution_implemented() &&
            !transact->has_conflict_been_cleared(*conf_i) &&
            !(*conf_i)->has_known_solution() &&
            !(*conf_i)->has_no_solution() ) {
            transact->resolve(*conf_i);
        }
    }
    for (conf_i = structural_conflicts.begin(); conf_i != structural_conflicts.end(); conf_i++) {
        CYG_LOOP_INVARIANT_CLASSC(*conf_i);
        CYG_LOOP_INVARIANTC(0 == (*conf_i)->get_transaction());
        if (((*conf_i)->resolution_implemented()) &&
            !transact->has_conflict_been_cleared(*conf_i) &&
            !(*conf_i)->has_known_solution() &&
            !(*conf_i)->has_no_solution() ) {
            transact->resolve(*conf_i);
        }
    }

    transact->body();
    delete transact;

    CYG_REPORT_RETURN();
}

//}}}
//{{{  Limbo support                            

// ----------------------------------------------------------------------------
// Limbo support. This is basically trivial, an STL map does all the
// right things.
void
CdlToplevelBody::set_limbo_value(CdlValuable valuable)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::set_limbo_value");
    CYG_REPORT_FUNCARG2XV(this, valuable);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(valuable);

    limbo[valuable->get_name()] = valuable->get_whole_value();

    CYG_REPORT_RETURN();
}

bool
CdlToplevelBody::has_limbo_value(std::string name) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlToplevel::has_limbo_value", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != name);

    bool result = false;
    if (limbo.find(name) != limbo.end()) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

CdlValue
CdlToplevelBody::get_limbo_value(std::string name) const
{
    CYG_REPORT_FUNCNAME("CdlToplevel::get_limbo_value");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != name);

    std::map<std::string,CdlValue>::const_iterator limbo_i = limbo.find(name);
    CYG_ASSERTC(limbo_i != limbo.end());

    CYG_REPORT_RETURN();
    return limbo_i->second;
}

CdlValue
CdlToplevelBody::get_and_remove_limbo_value(std::string name)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::get_and_remove_limbo_value");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != name);

    std::map<std::string,CdlValue>::iterator limbo_i = limbo.find(name);
    CYG_ASSERTC(limbo_i != limbo.end());

    CdlValue local_copy = limbo_i->second;
    limbo.erase(limbo_i);

    CYG_REPORT_RETURN();
    return local_copy;
}

void
CdlToplevelBody::clear_limbo()
{
    CYG_REPORT_FUNCNAME("CdlToplevel::clear_limbo");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    limbo.clear();

    CYG_REPORT_RETURN();
}

//}}}
//{{{  Persistence support                      

//{{{  Description                      

// ----------------------------------------------------------------------------
// Toplevels do not have any data specifically associated with them which
// should go into savefiles (not quite true, there is a description field,
// but that can be handled easily by the derived classes).
//
// However there is a need in the library for some generic savefile support:
//
// 1) it is an important goal that savefiles should be self-describing.
//    This is handled by having a header section at the start of each
//    savefile which describes what commands will appear in the savefile
//    (note that savefiles are actually just Tcl scripts). In addition
//    each savefile contains a version number so that individual commands
//    can detect and adapt to older versions of the library.
//
// 2) savefiles should also be extensible, so for example a GUI tool should
//    be able to add its own information. This can be toplevel information,
//    i.e. a new command that gets executed at the savefile's toplevel,
//    or it can be a subcommand extending an existing command such as
//    cdl_option. Right now only one level of nesting is available, but
//    this should suffice.
//
// 3) extensibility means that the application reading in a savefile may
//    not support the same set of commands as the application that generated
//    the savefile. Care is taken to avoid loss of data. However exact
//    ordering is not guaranteed to be preserved, and neither is formatting.
//
// These needs are interrelated, and supported by the CdlToplevelBody
// class. The functions of interest are:
//
// virtual void initialize_savefile_support()
//    This should be called from higher-level code such as
//    CdlConfiguration::initialize_savefile_support() at the start of
//    any savefile-related operation.
//
//    The support operates on a per-application basis rather than a
//    per-toplevel basis, in spite of being a virtual member function
//    rather than a static. A virtual member function facilitates
//    automatic initialization. This causes some problems if you need
//    to load in toplevels with different application-specific
//    extensions, but it makes life a lot simpler for the application.
//
// static bool savefile_support_initialized()
//    Has there been a call to initialize_savefile_support() yet?
//
// virtual void add_savefile_command(std::string, CdlSaveCallback, CdlInterpreterCommand)
//    Register a new savefile toplevel command. The string must be a
//    valid command name. The callback function will be 0 for savedata
//    supported directly by the library, non-zero for application-specific
//    data, and is invoked during a save operation to allow application
//    code to add extra data to the savefile. The command procedure must
//    be provided and is registered with the Tcl interpreter.
//
// virtual void add_savefile_subcommand(std::string cmd, std::string subcommand, CdlSaveCallback, CdlInterpreterCommand)
//    Typically savefile commands take the form <command> <name> <body>,
//    where <body> contains a set of subcommands. This function is used
//    to register a new subcommand.
//
// void save_command_details(CdlInterpreter, Tcl_Channel, int)
//    This should be invoked early on when generating a savefile. Its
//    purpose is to store information about the current set of savefile
//    commands in the savefile, thus making savefiles self-describing.
//    This command acts on a per-toplevel basis, since each toplevel
//    may have been created via a load operation and hence may contain
//    unrecognised commands.
//
// static void get_savefile_commands(std::vector<CdlInterpreterCommandEntry>&)
//    Work out the set of commands that should be supported by the
//    interpreter used to process a savefile. Note that this set gets
//    updated magically by savefile_handle_command().
//
// static void get_savefile_subcommands(std::string, std::vector<CdlInterpreterCommandEntry>&)
//    Ditto for subcommands.
//
// static int savefile_handle_command(CdlInterpreter, int, char**)
//    This implements cdl_savefile_command, and makes sure that
//    all of the commands that may be present in the savefile will
//    be processed.
//
// static int savefile_handle_unsupported(CdlInterpreter, int, char**)
//    This takes care of commands present in the savefile which are
//    not supported by the current application.
//
// static int savefile_handle_unknown(CdlInterpreter, int, char**)
//    This is an implementation of "unknown" suitable for savefiles.
//    All commands that may get used in a savefile should be specified
//    via cdl_savefile_command, so an unknown command is an error.
//
// cdl_int get_library_savefile_version()
//    Savefiles contain a format version number. This function can be used
//    to determine the current version used in the library.
//
// int savefile_handle_version(CdlInterpreter, int, char**)
//    This is the implementation of the cdl_savefile_version command. It
//    stores the version information in the interpreter, allowing it
//    to be retrieved by other commands.
//
// cdl_int get_savefile_version(CdlInterpreter)
//    This can be used to retrieve the version number that was present
//    in the current savefile.
//
//    The version number should not be used for application-specific
//    commands. It is possible for a savefile to be read in by a
//    different program and then updated: the updated savefile will
//    contain the unrecognised commands unchanged, but it will also
//    have the version number corresponding to that program rather
//    than to the original savefile.
//
// void save_conflicts(CdlInterpreter, Tcl_Channel, int)
//    Output details of all the conflicts in the current configuration
//
// void save_separator(CdlInterpreter, Tcl_Channel, int)
//    A utility to add a separator line to a savefile. This has to
//    go somewhere....
//
// FIXME: add limbo support

//}}}
//{{{  Statics and initialization       

// ----------------------------------------------------------------------------
bool CdlToplevelBody::savefile_commands_initialized = false;
std::vector<CdlSavefileCommand> CdlToplevelBody::savefile_commands;
std::map<std::string,std::vector<CdlSavefileCommand> > CdlToplevelBody::savefile_subcommands;

void
CdlToplevelBody::initialize_savefile_support()
{
    CYG_REPORT_FUNCNAME("CdlToplevel::initialize_savefile_support");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    
    // This assignment avoids circular dependencies. It is not
    // completely accurate but close enough - the full set of
    // commands will be initialised shortly.
    savefile_commands_initialized = true;

    // The commands cdl_savefile_version and cdl_command are a core
    // part of the CDL savefile support.
    add_savefile_command("cdl_savefile_version", 0, &savefile_handle_version);
    add_savefile_command("cdl_savefile_command", 0, &savefile_handle_command);

    CYG_REPORT_RETURN();
}

bool
CdlToplevelBody::savefile_support_initialized()
{
    CYG_REPORT_FUNCNAMETYPE("CdlToplevel::check_savefile_support_initialized", "result %d");

    bool result = savefile_commands_initialized;
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  Command details                  

// ----------------------------------------------------------------------------
// These routines are used to keep track of the savefile commands that
// are understood by the current application. There may have been
// additional per-toplevel commands when a savefile was read in, but
// these are stored separately.
//
// Currently there is only support for toplevel savefile commands
// and for one level of subcommands. Multiple levels can probably
// be accommodated by using the equivalent of a directory separator
// in the savefile_subcommands map key.

void
CdlToplevelBody::add_savefile_command(std::string name, CdlSaveCallback save_callback, CdlInterpreterCommand load_command)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::add_savefile_command");
    CYG_REPORT_FUNCARG3XV(this, save_callback, load_command);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != name);

    if (!savefile_commands_initialized) {
        this->initialize_savefile_support();
    }
    
    std::vector<CdlSavefileCommand>::const_iterator cmd_i;
    for (cmd_i = savefile_commands.begin(); cmd_i != savefile_commands.end(); cmd_i++) {
        if (cmd_i->name == name) {
            if ((cmd_i->save_callback != save_callback) || (cmd_i->load_command != load_command)) {
                CYG_FAIL("Internal error: attempt to define two toplevel savefile commands with the same name.");
            }
            break;
        }
    }
    if (cmd_i == savefile_commands.end()) {
        CdlSavefileCommand cmd;
        cmd.name                = name;
        cmd.save_callback       = save_callback;
        cmd.load_command        = load_command;
        savefile_commands.push_back(cmd);

        std::vector<CdlSavefileCommand> subcommands;
        savefile_subcommands[name] = subcommands;
    }

    CYG_REPORT_RETURN();
}

// Add a new subcommand for a given command. The command should have been
// defined already.
void
CdlToplevelBody::add_savefile_subcommand(std::string cmd, std::string subcommand, CdlSaveCallback save_callback,
                                         CdlInterpreterCommand load_command)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::add_savefile_subcommand");
    CYG_REPORT_FUNCARG3XV(this, save_callback, load_command);
    CYG_PRECONDITION_THISC();
    
    if (!savefile_commands_initialized) {
        this->initialize_savefile_support();
    }
    
    std::vector<CdlSavefileCommand>::iterator cmd_i;
    for (cmd_i = savefile_commands.begin(); cmd_i != savefile_commands.end(); cmd_i++) {
        if (cmd_i->name == cmd) {
            break;
        }
    }
    CYG_ASSERTC(cmd_i != savefile_commands.end());

    for (cmd_i = savefile_subcommands[cmd].begin(); cmd_i != savefile_subcommands[cmd].end(); cmd_i++) {
        if (cmd_i->name == subcommand) {
            if ((cmd_i->save_callback != save_callback) || (cmd_i->load_command != load_command)) {
                CYG_FAIL("Internal error: attempt to define two subcommands with the same name.");
            }
        }
    }
    if (cmd_i == savefile_subcommands[cmd].end()) {
        CdlSavefileCommand new_subcommand;
        new_subcommand.name          = subcommand;
        new_subcommand.save_callback = save_callback;
        new_subcommand.load_command  = load_command;
        savefile_subcommands[cmd].push_back(new_subcommand);
    }

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// This member function is invoked by e.g. CdlConfiguraton::save() to
// take care of the generic savefile information, specifically the
// savefile format version number and the various commands and subcommands
// Note that it has to cope with per-toplevel commands from the original
// savefile, as well as the global set.

void
CdlToplevelBody::save_command_details(CdlInterpreter interp, Tcl_Channel chan, int indentation, bool minimal)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::save_command_details");
    CYG_REPORT_FUNCARG4XV(this, interp, chan, indentation);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(interp);
    CYG_ASSERTC(0 == indentation);

    // The parent code should have provided the first couple of lines,
    // identifying whether this is an eCos configuration or some other
    // CDL-based entity.
    //
    // Immediately after these lines we want a nice big comment
    // telling people that they can edit bits of this file, but
    // that other bits are automatically generated and will
    // be overwritten.

    if (!minimal) {
        interp->write_data(chan,
"# This section contains information about the savefile format.\n\
# It should not be edited. Any modifications made to this section\n\
# may make it impossible for the configuration tools to read\n\
# the savefile.\n\
\n");
    }

    // Next output details of the savefile format version. This allows
    // all other code to adapt to the version.
    std::string savefile_data;
    Cdl::integer_to_string(savefile_version, savefile_data);
    savefile_data = "cdl_savefile_version " + savefile_data + ";\n";

    std::vector<CdlSavefileCommand>::const_iterator cmd_i, cmd_j;
    std::vector<std::string>::const_iterator cmd_k, cmd_l;
    
    for (cmd_i = savefile_commands.begin(); cmd_i != savefile_commands.end(); cmd_i++) {
        savefile_data += "cdl_savefile_command " + cmd_i->name + " ";
        
        if ((0 == savefile_subcommands[cmd_i->name].size()) &&
            (0 == this->unsupported_savefile_subcommands[cmd_i->name].size())) {
            
            savefile_data += "{};\n";
            
        } else {
            
            savefile_data += "{";
            for (cmd_j = savefile_subcommands[cmd_i->name].begin();
                 cmd_j != savefile_subcommands[cmd_i->name].end();
                 cmd_j++) {
                
                savefile_data += " " + cmd_j->name;
            }
            for (cmd_l = this->unsupported_savefile_subcommands[cmd_i->name].begin();
                 cmd_l != this->unsupported_savefile_subcommands[cmd_i->name].end();
                 cmd_l++) {

                savefile_data += " " + *cmd_l;
            }
            savefile_data += " };\n";
        }
    }
    for (cmd_k = this->unsupported_savefile_commands.begin();
         cmd_k != this->unsupported_savefile_commands.end();
         cmd_k++) {
        savefile_data += "cdl_savefile_command " + *cmd_k + " ";
        if (0 == this->unsupported_savefile_subcommands[*cmd_k].size()) {
            
            savefile_data += "{};\n";
            
        } else {

            savefile_data += "{";
            for (cmd_l = this->unsupported_savefile_subcommands[*cmd_k].begin();
                 cmd_l != this->unsupported_savefile_subcommands[*cmd_k].end();
                 cmd_l++) {

                savefile_data += " " + *cmd_l;
            }
            savefile_data += " };\n";
        }
    }
    savefile_data += "\n";

    interp->write_data(chan, savefile_data);

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Get hold of the commands that should be added to the interpreter for
// processing a savefile. Note that this will only deal with commands
// supported by the library or the application, not any additional
// unsupported commands specified in the savefile itself. The latter
// will be taken care of magically by savefile_handle_command().

void
CdlToplevelBody::get_savefile_commands(std::vector<CdlInterpreterCommandEntry>& cmds)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::get_savefile_commands");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlInterpreterCommandEntry local_cmd;
    std::vector<CdlSavefileCommand>::const_iterator cmd_i;
    for (cmd_i = savefile_commands.begin(); cmd_i != savefile_commands.end(); cmd_i++) {
        // NOTE: this use of c_str() is somewhat dubious, but the string should not
        // change so the c_str() array should remain ok as well.
        local_cmd.name = cmd_i->name;
        local_cmd.command = cmd_i->load_command;
        cmds.push_back(local_cmd);
    }

    // There is no point in iterating over this->unsupported_savefile_commands,
    // that vector should be empty since we have not actually started
    // processing the savefile yet.
    CYG_ASSERTC(0 == this->unsupported_savefile_commands.size());

    // Add an implementation of the "unknown" command.
    local_cmd.name = "unknown";
    local_cmd.command = &CdlToplevelBody::savefile_handle_unknown;
    cmds.push_back(local_cmd);
    
    CYG_REPORT_RETURN();
}

// Having repeated calls of this for e.g. every cdl_option statement in
// a savefile is expensive. Some sort of caching mechanism should be
// used to avoid unnecessary overheads.
void
CdlToplevelBody::get_savefile_subcommands(std::string main_command, std::vector<CdlInterpreterCommandEntry>& cmds)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::get_savefile_subcommands");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlInterpreterCommandEntry local_cmd;
    std::vector<CdlSavefileCommand>::const_iterator cmd_i;
    for (cmd_i = savefile_subcommands[main_command].begin();
         cmd_i != savefile_subcommands[main_command].end();
         cmd_i++) {

        local_cmd.name = cmd_i->name.c_str();
        local_cmd.command = cmd_i->load_command;
        cmds.push_back(local_cmd);
    }

    std::vector<std::string>::const_iterator cmd_j;
    for (cmd_j = this->unsupported_savefile_subcommands[main_command].begin();
         cmd_j != this->unsupported_savefile_subcommands[main_command].end();
         cmd_j++) {

        local_cmd.name = cmd_j->c_str();
        local_cmd.command = &savefile_handle_unsupported;
        cmds.push_back(local_cmd);
    }

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// This implements cdl_savefile_command which should appear near the
// start of savefiles. The command takes two arguments, a primary
// command name and a set of subcommand names.
int
CdlToplevelBody::savefile_handle_command(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAME("CdlToplevel::savefile_handle_command");
    CYG_REPORT_FUNCARG2XV(interp, argc);
    CYG_PRECONDITION_CLASSC(interp);
    
    CdlToplevel toplevel = interp->get_toplevel();
    CYG_ASSERT_CLASSC(toplevel);
    CYG_ASSERTC(toplevel->savefile_commands_initialized);

    if (1 == argc) {
        CdlParse::report_error(interp, "", "Expecting at least one argument to cdl_savefile_command");
    } else if (2 == argc) {
        CdlParse::report_warning(interp, "",
                                 std::string("Missing third argument to `cdl_savefile_command ") + argv[1] +
                                 "'\n.Expecting an additional list of subcommands.");
    } else if (3 != argc) {
        CdlParse::report_warning(interp, "", std::string("Unexpected additional arguments to `cdl_savefile_command ") +
                                 argv[1] + " { " + argv[2] + " }");
    }

    // Is the primary command one of the known ones?
    bool known_command = false;
    std::vector<CdlSavefileCommand>::const_iterator cmd_i;
    std::vector<std::string>::const_iterator cmd_j;
        
    if (1 != argc) {
        // Make sure that the primary command is known.
        for (cmd_i = savefile_commands.begin(); cmd_i != savefile_commands.end(); cmd_i++) {
            if (cmd_i->name == argv[1]) {
                known_command = true;
                break;
            }
        }
        if (!known_command) {
            // Detect duplicate definitions, just in case.
            for (cmd_j = toplevel->unsupported_savefile_commands.begin();
                 cmd_j != toplevel->unsupported_savefile_commands.end();
                 cmd_j++) {
                if (*cmd_j == argv[1]) {
                    break;
                }
            }
            if (cmd_j == toplevel->unsupported_savefile_commands.end()) {
                toplevel->unsupported_savefile_commands.push_back(argv[1]);
            }
        }
    }

    // Now take care of all the subcommands.
    if (2 != argc) {

        int          list_count = 0;
        const char** list_entries = 0;

        try {
            Tcl_Interp* tcl_interp = interp->get_tcl_interpreter();
            if (TCL_OK != Tcl_SplitList(tcl_interp, CDL_TCL_CONST_CAST(char*, argv[2]), &list_count, CDL_TCL_CONST_CAST(char***, &list_entries))) {
                CdlParse::report_error(interp, "", std::string("Invalid subcommand list for `cdl_command ") + argv[1] + "'.");
            }

            for (int i = 0; i < list_count; i++) {
                bool known_subcommand = false;
                if (known_command) {
                    for (cmd_i = savefile_subcommands[argv[1]].begin();
                         cmd_i != savefile_subcommands[argv[1]].end();
                         cmd_i++) {

                        if (cmd_i->name == list_entries[i]) {
                            known_subcommand = true;
                        }
                    }
                }
                if (!known_subcommand) {
                    for (cmd_j = toplevel->unsupported_savefile_subcommands[argv[1]].begin();
                         cmd_j != toplevel->unsupported_savefile_subcommands[argv[1]].end();
                         cmd_j++) {

                        if (*cmd_j == list_entries[i]) {
                            known_subcommand = true;
                            break;
                        }
                    }
                }
                if (!known_subcommand) {
                    toplevel->unsupported_savefile_subcommands[argv[1]].push_back(list_entries[i]);
                }
                
            }

            if (0 != list_entries) {
                Tcl_Free((char *)list_entries);
            }
            
        } catch(...) {
            if (0 != list_entries) {
                Tcl_Free((char *)list_entries);
            }
            throw;
        }
    }
    
    return TCL_OK;
}

//}}}
//{{{  handle_unsupported()             

// ----------------------------------------------------------------------------
// This function is invoked when an unsupported command is detected in
// a savefile. It turns the data back into a string which can go back
// into the next savefile, thus avoiding loss of data.
//
// It is possible that the savefile contents involved variable or
// command substitution. If so then this information will have been
// lost, there is no simple way of retrieving this from the interpreter.
// Care has to be taken when generating the new command string to
// perform appropriate quoting.
//
// Ideally the original data could be extracted from the Tcl
// interpreter somehow. Currently this data is not readily available,
// and the resulting string may not match the original data exactly.
int
CdlToplevelBody::savefile_handle_unsupported(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAME("CdlNode::savefile_handle_unsupported");
    CYG_REPORT_FUNCARG2XV(interp, argc);
    CYG_ASSERT_CLASSC(interp);

    CdlToplevel toplevel = interp->get_toplevel();
    CYG_ASSERT_CLASSC(toplevel);
    CdlNode node = interp->get_node();
    CYG_ASSERT_ZERO_OR_CLASSC(node);

    std::string tmp = CdlInterpreterBody::quote(argv[0]);
    for (int i = 1; i < argc; i++) {
        tmp = tmp + " " + CdlInterpreterBody::quote(argv[i]);
    }
    // Unknown commands may occur at the toplevel or inside
    // e.g. a cdl_option body. Toplevels are also nodes.
    if (0 == node) {
        toplevel->unsupported_savefile_toplevel_strings.push_back(tmp);
    } else {
        node->unsupported_savefile_strings.push_back(tmp);
    }

    return TCL_OK;
}

//}}}
//{{{  save_unsupported()               

// ----------------------------------------------------------------------------
// This code deals with any toplevel data present in the original save
// file that was not recognised.
void
CdlToplevelBody::save_unsupported_commands(CdlInterpreter interp, Tcl_Channel chan, int indentation, bool minimal)
{
    CYG_REPORT_FUNCNAME("CdlToplevelBody::save_unsupported_commands");
    CYG_REPORT_FUNCARG3XV(this, interp, chan);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(interp);
    CYG_PRECONDITIONC(0 == indentation);

    std::string data = "\n";
    std::vector<std::string>::const_iterator str_i;
    for (str_i = unsupported_savefile_toplevel_strings.begin();
         str_i != unsupported_savefile_toplevel_strings.end();
         str_i++) {
        data += *str_i + " ;\n";
    }
    interp->write_data(chan, data);

    CYG_UNUSED_PARAM(bool, minimal);
    CYG_REPORT_RETURN();
}

//}}}
//{{{  handle_unknown()                 

// ----------------------------------------------------------------------------

int
CdlToplevelBody::savefile_handle_unknown(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAME("CdlToplevel::savefile_handle_unknown");
    CYG_REPORT_FUNCARG2XV(interp, argc);
    CYG_PRECONDITION_CLASSC(interp);

    CdlParse::report_error(interp, "", std::string("Unknown command `") + argv[1] + "'.");
    
    CYG_UNUSED_PARAM(int, argc);
    return TCL_OK;
}

//}}}
//{{{  versioning                       

// ----------------------------------------------------------------------------
// Savefiles include a version number that can be used by library
// commands to cope with old and incompatible savefiles. This
// version number should be changed only very rarely, hopefully never.
cdl_int CdlToplevelBody::savefile_version = 1;

cdl_int
CdlToplevelBody::get_library_savefile_version()
{
    CYG_REPORT_FUNCNAMETYPE("CdlToplevel::get_library_savefile_version", "result %ld");

    cdl_int result = savefile_version;
    CYG_REPORT_RETVAL((long) result);
    return result;
}

// This implements the cdl_savefile_version command. It stores the
// version number with the interpreter, allowing it to be retrieved
// by other commands.
int
CdlToplevelBody::savefile_handle_version(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAME("CdlToplevel::savefile_handle_version");
    CYG_REPORT_FUNCARG2XV(interp, argc);
    CYG_PRECONDITION_CLASSC(interp);

    if (1 == argc) {
        CdlParse::report_warning(interp, "", "Expecting one argument to cdl_savefile_version");
    } else {
        if (2 != argc) {
            CdlParse::report_warning(interp, "",
                                     std::string("Unexpected number of arguments to cdl_savefile_version\n") +
                                     "There should be exactly one argument, the savefile format version number.");
        }
        cdl_int tmp;
        if (!Cdl::string_to_integer(argv[1], tmp)) {
            CdlParse::report_error(interp, "",
                                   std::string("Invalid version number `") + argv[1] + "' for cdl_savefile_version");
        } else {
            // Store the data in a Tcl variable. This is at least as convenient
            // as assoc data.
            interp->set_variable("cdl_savefile_version", argv[1]);
        }
    }

    return TCL_OK;
}

cdl_int
CdlToplevelBody::get_savefile_version(CdlInterpreter interp)
{
    CYG_REPORT_FUNCNAMETYPE("CdlToplevel::get_savefile_version", "result %ld");
    CYG_REPORT_FUNCARG1XV(interp);
    CYG_PRECONDITION_CLASSC(interp);
    
    cdl_int result = 0;
    std::string version = interp->get_variable("cdl_savefile_version");
    if ("" != version) {
        if (!Cdl::string_to_integer(version, result)) {
            CdlParse::report_error(interp, "", std::string("Invalid cdl_savefile_version number `") + version + "'");
        }
    }

    CYG_REPORT_RETVAL((long) result);
    return result;
}

//}}}
//{{{  conflicts                        

// ----------------------------------------------------------------------------
void
CdlToplevelBody::save_conflicts(CdlInterpreter interp, Tcl_Channel chan, int indentation, bool minimal)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::save_conflicts");
    CYG_REPORT_FUNCARG4XV(this, interp, chan, indentation);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(interp);
    CYG_PRECONDITIONC(0 == indentation);

    // For now only comments are generated here, so in a minimal save
    // there is no need for any of this data
    if (!minimal) {
        std::string data = "";
        if (0 == conflicts.size()) {
            data += "# There are no conflicts.\n";
        } else {
            std::string tmp;
            Cdl::integer_to_string((cdl_int) this->conflicts.size(), tmp);
            data += "# There are " + tmp + " conflicts.\n";

            std::list<CdlConflict>::const_iterator conf_i;
            for (conf_i = this->conflicts.begin(); conf_i != this->conflicts.end(); conf_i++) {
                data += "#\n";

                CdlNode node = (*conf_i)->get_node();
                CdlProperty prop = (*conf_i)->get_property();
                std::string description = (*conf_i)->get_explanation();
                data += "# " + node->get_class_name() + " "  + node->get_name() + "\n";
                data += "#   Property " + prop->get_property_name() + "\n";
                data += CdlInterpreterBody::multiline_comment(description, 0, 2) + "\n";
            }
            data += '\n';
        }
        data += '\n';

        interp->write_data(chan, data);
    }
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  save_separator()                 

// ----------------------------------------------------------------------------
void
CdlToplevelBody::save_separator(CdlInterpreter interp, Tcl_Channel chan, std::string msg, bool minimal)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::save_separator");
    CYG_REPORT_FUNCARG1XV(interp);
    CYG_PRECONDITION_CLASSC(interp);

    if (!minimal) {
        std::string data = "# ---- " + msg + ' ';
        if (72 > data.size()) {
            data += std::string(72 - data.size(), '-');
        }
        data += '\n';
        interp->write_data(chan, data);
    }
    
    CYG_REPORT_RETURN();
}

//}}}

//}}}
//{{{  check_this()                             

// ----------------------------------------------------------------------------
bool
CdlToplevelBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlToplevelBody_Magic != cdltoplevelbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();

    if ((zeal == cyg_extreme) || (zeal == cyg_thorough)) {
        if (!interp->check_this(cyg_quick)) {
            return false;
        }
        if ((0 == orphans) || !orphans->check_this(cyg_quick)) {
            return false;
        }
        if (orphans != *contents.begin()) {
            return false;
        }
        if ((0 != transaction) && !transaction->check_this(cyg_quick)) {
            return false;
        }
    }

    return CdlContainerBody::check_this(zeal);
}

//}}}

//}}}
//{{{  CdlUserVisiblebody               

//{{{  Basics                           

// ----------------------------------------------------------------------------
// All user-visible object can have (and usually should have) three
// properties: display (originally known as alias), description, and
// doc. There is no additional data associated with a user-visible
// object, everything is handled via the properties.

CdlUserVisibleBody::CdlUserVisibleBody()
{
    CYG_REPORT_FUNCNAME("CdlUserVisible:: default constructor");
    CYG_REPORT_FUNCARG1XV(this);

    cdluservisiblebody_cookie  = CdlUserVisibleBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlUserVisibleBody::~CdlUserVisibleBody()
{
    CYG_REPORT_FUNCNAME("CdlUserVisible:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdluservisiblebody_cookie = CdlUserVisibleBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

std::string
CdlUserVisibleBody::get_class_name() const
{
    CYG_REPORT_FUNCNAME("CdlUserVisible::get_class_name");
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return "uservisible";
}

bool
CdlUserVisibleBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlUserVisibleBody_Magic != cdluservisiblebody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return CdlNodeBody::check_this(zeal);
}

//}}}
//{{{  Extracting information           

// ----------------------------------------------------------------------------
// Extracting the information.

std::string
CdlUserVisibleBody::get_display() const
{
    CYG_REPORT_FUNCNAME("CdlUserVisible::get_display");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::string result = "";
    CdlProperty property = get_property(CdlPropertyId_Display);
    if (0 != property) {
        
        CdlProperty_String string_property = dynamic_cast<CdlProperty_String>(property);
        CYG_ASSERTC(0 != string_property);

        result = string_property->get_string();
    }

    CYG_REPORT_RETURN();
    return result;
}

std::string
CdlUserVisibleBody::get_description() const
{
    CYG_REPORT_FUNCNAME("CdlUserVisible::get_description");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::string result = "";
    CdlProperty property = get_property(CdlPropertyId_Description);
    if (0 != property) {
        
        CdlProperty_String string_property = dynamic_cast<CdlProperty_String>(property);
        CYG_ASSERTC(0 != string_property);

        result = string_property->get_string();
    }

    CYG_REPORT_RETURN();
    return result;
}

std::string
CdlUserVisibleBody::get_doc() const
{
    CYG_REPORT_FUNCNAME("CdlUserVisible::get_doc");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::string result = "";
    CdlProperty property = get_property(CdlPropertyId_Doc);
    if (0 != property) {
        
        CdlProperty_String string_property = dynamic_cast<CdlProperty_String>(property);
        CYG_ASSERTC(0 != string_property);

        result = string_property->get_string();
    }

    CYG_REPORT_RETURN();
    return result;
}

std::string
CdlUserVisibleBody::get_doc_url() const
{
    CYG_REPORT_FUNCNAME("CdlUserVisible::get_doc_url");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::string result = "";
    std::string doc_property = get_doc();
    if ("" != doc_property) {
        CdlLoadable owner = get_owner();
        CYG_ASSERTC(0 != owner);
        result = owner->find_absolute_file(doc_property, "doc", true);
    }

    CYG_REPORT_RETURN();
    return result;
}

//}}}
//{{{  Parsing                          

// ----------------------------------------------------------------------------
// Parsing support. There are three property parsers to be added to
// the current set. The checking code should make sure that at most
// one of each property has been specified. In addition it is
// necessary to recurse into the base class.

void
CdlUserVisibleBody::add_property_parsers(std::vector<CdlInterpreterCommandEntry>& parsers)
{
    CYG_REPORT_FUNCNAME("CdlUserVisible::add_property_parsers");

    static CdlInterpreterCommandEntry commands[] =
    {
        CdlInterpreterCommandEntry("display",     &parse_display),
        CdlInterpreterCommandEntry("description", &parse_description),
        CdlInterpreterCommandEntry("doc",         &parse_doc),
        CdlInterpreterCommandEntry("",            0)
    };

    for (int i = 0; commands[i].command != 0; i++) {
        std::vector<CdlInterpreterCommandEntry>::const_iterator j;
        for (j = parsers.begin(); j != parsers.end(); j++) {
            if (commands[i].name == j->name) {
                if (commands[i].command != j->command) {
                    CYG_FAIL("Property names are being re-used");
                }
                break;
            }
        }
        if (j == parsers.end()) {
            parsers.push_back(commands[i]);
        }
    }
    CdlNodeBody::add_property_parsers(parsers);
    
    CYG_REPORT_RETURN();
}

void
CdlUserVisibleBody::check_properties(CdlInterpreter interp)
{
    CYG_REPORT_FUNCNAME("CdlUserVisible::check_properties");
    CYG_REPORT_FUNCARG2XV(this, interp);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(interp);

    if (count_properties(CdlPropertyId_Display) > 1) {
        CdlParse::report_error(interp, "", "There should be at most one display property.");
    }
    if (count_properties(CdlPropertyId_Description) > 1) {
        CdlParse::report_error(interp, "", "There should be at most one description property.");
    }
    if (count_properties(CdlPropertyId_Doc) > 1) {
        CdlParse::report_error(interp, "", "There should be at most one doc property.");
    }
    
    // FIXME: more validation of the doc property, in particular check that
    // the resulting URL would be either remote or to an existing file.

    CdlNodeBody::check_properties(interp);
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Syntax: description <string>

int
CdlUserVisibleBody::parse_description(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_description", "result %d");

    int result = CdlParse::parse_string_property(interp, argc, argv, CdlPropertyId_Description, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}


// ----------------------------------------------------------------------------
// Syntax: display <short description>

int
CdlUserVisibleBody::parse_display(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_display", "result %d");

    int result = CdlParse::parse_string_property(interp, argc, argv, CdlPropertyId_Display, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Syntax: doc <url>

int
CdlUserVisibleBody::parse_doc(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_doc", "result %d");
    
    int result = CdlParse::parse_string_property(interp, argc, argv, CdlPropertyId_Doc, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  Persistence                      

// ----------------------------------------------------------------------------
// There is no data in a user visible object that users will want to edit,
// but the display string, the documentation, and the description are all
// useful and should be present in the savefile as comments.
//
// The intention is that the UserVisible information appears immediately
// above the option/component/whatever definition, e.g.:
// # <display string.
// # doc <URL>
// # <description
// # ...>
// #

void
CdlUserVisibleBody::save(CdlInterpreter interp, Tcl_Channel chan, int indentation, bool minimal)
{
    CYG_REPORT_FUNCNAME("CdlUserVisible::save");
    CYG_REPORT_FUNCARG5XV(this, interp, chan, indentation, minimal);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(interp);
    CYG_ASSERTC(0 == indentation);

    if (!minimal) {
        std::string data = "";
        std::string display = get_display();
        if ("" != display) {
            data = std::string("# ") + display + "\n";
        }
        // Note that this uses get_doc(), not get_doc_url(). The latter
        // would give an absolute pathname that is applicable to the
        // current user, but it would change if a different user loaded
        // and saved the file. This is a bad idea in terms of version
        // control.
        std::string doc = get_doc();
        if ("" != doc) {
            data += "# doc: " + doc + "\n";
        }
        std::string description = get_description();
        if ("" != description) {
            unsigned int i = 0;
            while (i < description.size()) {
                data += "# ";
                while ((i < description.size()) && isspace(description[i])) {
                    i++;
                }
                while ((i < description.size()) && ('\n' != description[i])) {
                    data += description[i++];
                }
                data += '\n';
            }
        }
        data += "#\n";
        
        interp->write_data(chan, data);
    }
    
    CYG_REPORT_RETURN();
}

//}}}

//}}}
//{{{  CdlParentableBody                

// ----------------------------------------------------------------------------
// A parentable object can have the parent property, i.e. it can be
// positioned anywhere in the hierarchy. There is no data associated
// with such an object.

CdlParentableBody::CdlParentableBody()
{
    CYG_REPORT_FUNCNAME("CdlParentable:: default constructor");
    CYG_REPORT_FUNCARG1XV(this);

    change_parent_save_position = -1;
    cdlparentablebody_cookie  = CdlParentableBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlParentableBody::~CdlParentableBody()
{
    CYG_REPORT_FUNCNAME("CdlParentable:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdlparentablebody_cookie = CdlParentableBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------

std::string
CdlParentableBody::get_class_name() const
{
    CYG_REPORT_FUNCNAME("CdlParentable::get_class_name");
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return "parentable";
}

// ----------------------------------------------------------------------------

bool
CdlParentableBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlParentableBody_Magic != cdlparentablebody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return CdlNodeBody::check_this(zeal);
}

// ----------------------------------------------------------------------------
// Parsing support. There is just one property parser to be added.

void
CdlParentableBody::add_property_parsers(std::vector<CdlInterpreterCommandEntry>& parsers)
{
    CYG_REPORT_FUNCNAME("CdlParentable::add_property_parsers");

    static CdlInterpreterCommandEntry commands[] =
    {
        CdlInterpreterCommandEntry("parent", &CdlParentableBody::parse_parent),
        CdlInterpreterCommandEntry("",       0)
    };

    for (int i = 0; commands[i].command != 0; i++) {
        std::vector<CdlInterpreterCommandEntry>::const_iterator j;
        for (j = parsers.begin(); j != parsers.end(); j++) {
            if (commands[i].name == j->name) {
                if (commands[i].command != j->command) {
                    CYG_FAIL("Property names are being re-used");
                }
                break;
            }
        }
        if (j == parsers.end()) {
            parsers.push_back(commands[i]);
        }
    }
    CdlNodeBody::add_property_parsers(parsers);
    
    CYG_REPORT_RETURN();
}

void
CdlParentableBody::check_properties(CdlInterpreter interp)
{
    CYG_REPORT_FUNCNAME("CdlParentable::check_properties");
    CYG_REPORT_FUNCARG2XV(this, interp);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(interp);

    if (has_property(CdlPropertyId_Parent)) {
        if (count_properties(CdlPropertyId_Parent) > 1) {
            CdlParse::report_error(interp, "", "There should be at most one `parent' property.");
        }
        CdlProperty_Reference refprop = dynamic_cast<CdlProperty_Reference>(get_property(CdlPropertyId_Parent));
        CYG_ASSERT_CLASSC(this);
        if (get_name() == refprop->get_destination_name()) {
            CdlParse::report_error(interp, "", std::string("Node ") + get_name() + " cannot be its own parent.");
        }
    }

    CdlNodeBody::check_properties(interp);
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Syntax:: parent <reference to container>

void
CdlParentableBody::update_handler(CdlTransaction transaction, CdlNode source, CdlProperty prop, CdlNode dest, CdlUpdate change)
{
    CYG_REPORT_FUNCNAME("CdlParentable::update_handler");
    CYG_PRECONDITION_CLASSC(source);
    CYG_PRECONDITION_ZERO_OR_CLASSC(dest);

    // Value and activity updates are of no interest.
    if ((CdlUpdate_ValueChange == change) || (CdlUpdate_ActiveChange == change)) {
        CYG_REPORT_RETURN();
        return;
    }

    // Ditto for the second stage Init.
    if (CdlUpdate_Init == change) {
        CYG_REPORT_RETURN();
        return;
    }
    
    // If this object is being unloaded then we need to clean up the hierarchy.
    // Ordinary nodes must be re-parented below the owning loadable. The
    // loadable itself must be re-parented below the toplevel. A subsequent
    // calls to remove_loadable_from_toplevel() will ensure that the loadable
    // is now completely isolated from the remaining configuration, but can
    // still be put back.
    if (CdlUpdate_Unloading == change) {
        CdlToplevel toplevel = source->get_toplevel();
        CYG_ASSERT_CLASSC(toplevel);
        CdlLoadable owner = source->get_owner();
        CYG_ASSERT_CLASSC(owner);
        CdlLoadable loadable = dynamic_cast<CdlLoadable>(source);
        CYG_ASSERT_ZERO_OR_CLASSC(loadable);
        
        if (0 != loadable) {
            toplevel->change_parent(owner, source->get_parent(), toplevel, source);
        } else {
            toplevel->change_parent(owner, source->get_parent(), owner, source);
        }
        
        CYG_REPORT_RETURN();
        return;
    }

    // We should have:
    // 1) change == Loaded, dest == (0 | valid)
    // 2) change == Created, dest == valid
    // 3) change == Destroyed, dest == valid (still)
    CYG_ASSERTC((CdlUpdate_Loaded == change)    || (CdlUpdate_Created == change) || (CdlUpdate_Destroyed == change));
    CYG_ASSERTC((CdlUpdate_Created != change)   || (0 != dest));
    CYG_ASSERTC((CdlUpdate_Destroyed != change) || (0 != dest));

    if (CdlUpdate_Destroyed == change) {
        dest = 0;
    }
    
    // Now either dest is valid or it is not. If it is then we need to
    // reparent below the destination. Otherwise if the specified
    // parent is "" then we need to reparent below the root. Otherwise
    // the node ends up in the orphans container. There are a few
    // nasty special cases to consider like reparenting below
    // something that is not a container.
    if (0 == dest) {
        CdlToplevel  toplevel = source->get_toplevel();
        
        CdlProperty_Reference refprop = dynamic_cast<CdlProperty_Reference>(prop);
        if ("" == refprop->get_destination_name()) {
            dest = toplevel;
            // Now to find the correct insertion point. Nodes which should be
            // reparented below the root should come first, ahead of any nodes
            // which are not specifically reparented.
            const std::vector<CdlNode>& contents = toplevel->get_contents();
            unsigned int index;
            for (index = 0; index < contents.size(); index++) {
                if (!contents[index]->has_property(CdlPropertyId_Parent)) {
                    break;
                }
            }
            toplevel->change_parent(source->get_owner(), source->get_parent(), toplevel, source, index);
            
        } else {
            // Orphan the node. It still has a parent, either as a
            // consequence of the loading process or because of a previous
            // binding operation.
            toplevel->change_parent(source->get_owner(), source->get_parent(), 0, source);
        }
        
        // The Unresolved conflict is handled by
        // CdlProperty_Reference::update(). The "else" code below may
        // have created some additional data conflicts.
        transaction->clear_structural_conflicts(source, prop, &CdlConflict_DataBody::test);

        // Changing the parent may affect the "active" status.
        bool old_state = transaction->is_active(source);
        bool new_state = source->test_active(transaction);
        if (old_state != new_state) {
            transaction->set_active(source, new_state);
        }

    } else {
        // The node should no longer be an orphan - probably.

        // Check that the destination is actually a container. If it is,
        // reparenting is possible.
        CdlContainer dest_container = dynamic_cast<CdlContainer>(dest);
        if (0 == dest_container) {
            
            // The reference might be resolved, but reparenting is still not possible.
            // Leave the object orphaned as at present, and create a suitable conflict
            // object.
            std::string msg = source->get_class_name() + " " + source->get_name() + " cannot be reparented below " +
                dest->get_class_name() + " " + dest->get_name() + "\n    The latter is not a container.";
            CdlConflict_DataBody::make(transaction, source, prop, msg);
            
        } else {
            
            CdlContainer tmp = dynamic_cast<CdlContainer>(source);
            if ((0 != tmp) && tmp->contains(dest_container, true)) {
                
                // Somebody trying to be clever and reparent an object
                // below one of its existing children? Note that with
                // sufficiently careful use of parent statements this
                // might actually be legal, but for now treat it as
                // too dangerous.
                std::string msg = source->get_class_name() + " " + source->get_name() + " cannot be reparented below " +
                    dest->get_class_name() + " " + dest->get_name() + "\n    This would introduce a cycle.";
                CdlConflict_DataBody::make(transaction, source, prop, msg);
                
            } else {
                
                // It is possible to reparent the object to its correct location
                CdlToplevel toplevel = source->get_toplevel();
                CYG_ASSERTC(toplevel == dest->get_toplevel());
                toplevel->change_parent(source->get_owner(), source->get_parent(), dest_container, source);
                
                bool old_state = transaction->is_active(source);
                bool new_state = source->test_active(transaction);
                if (old_state != new_state) {
                    transaction->set_active(source, new_state);
                }
            }
        }
    }

    CYG_REPORT_RETURN();
}

int
CdlParentableBody::parse_parent(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_parent", "result %d");

    int result = CdlParse::parse_reference_property(interp, argc, argv, CdlPropertyId_Parent, 0, 0, true, &update_handler);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
