//{{{  Banner                                   

//============================================================================
//
//      transaction.cxx
//
//      Implementation of the CdlTransaction class
//
//============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 1999, 2000, 2001, 2002 Red Hat, Inc.
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
// Date:        1999/07/16
// Version:     0.01
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

//{{{  CdlTransactionCallback class             

// ----------------------------------------------------------------------------
// The callback class is very straightforward. The hard work is done in
// the transaction class.

CdlTransactionCallback::CdlTransactionCallback(CdlTransaction transact_arg)
{
    CYG_REPORT_FUNCNAME("CdlTransactionCallback:: constructor");
    CYG_REPORT_FUNCARG2XV(this, transact_arg);
    CYG_PRECONDITION_CLASSC(transact_arg);
    
    // The vectors etc. will take care of themselves.
    transact = transact_arg;
    cdltransactioncallback_cookie = CdlTransactionCallback_Magic;
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlTransactionCallback::~CdlTransactionCallback()
{
    CYG_REPORT_FUNCNAME("CdlTransactionCallback:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdltransactioncallback_cookie = CdlTransactionCallback_Invalid;
    transact = 0;
    value_changes.clear();
    active_changes.clear();
    legal_values_changes.clear();
    value_source_changes.clear();
    new_conflicts.clear();
    new_structural_conflicts.clear();
    nodes_with_resolved_conflicts.clear();
    nodes_with_resolved_structural_conflicts.clear();
    
    CYG_REPORT_RETURN();
}

void
CdlTransactionCallback::set_callback_fn(void (*fn)(const CdlTransactionCallback&))
{
    CYG_REPORT_FUNCNAME("CdlTransactionCallback::set_callback_fn");
    CYG_REPORT_FUNCARG1XV(fn);

    CdlTransactionBody::set_callback_fn(fn);

    CYG_REPORT_RETURN();
}

void (*CdlTransactionCallback::get_callback_fn())(const CdlTransactionCallback&)
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransactionCallback::get_callback_fn", "result %p");

    void (*result)(const CdlTransactionCallback&) = CdlTransactionBody::get_callback_fn();
    
    CYG_REPORT_RETVAL(result);
    return result;
}

CdlTransaction
CdlTransactionCallback::get_transaction() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransactionCallback::get_transaction", "result %p");
    CYG_PRECONDITION_THISC();

    CdlTransaction result = transact;
    CYG_POSTCONDITION_CLASSC(result);

    CYG_REPORT_RETVAL(result);
    return result;
}

CdlToplevel
CdlTransactionCallback::get_toplevel() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransactionCallback::get_toplevel", "result %p");
    CYG_PRECONDITION_THISC();

    CdlToplevel result = transact->get_toplevel();
    CYG_POSTCONDITION_CLASSC(result);

    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlTransactionCallback::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlTransactionCallback_Magic != cdltransactioncallback_cookie) {
        return false;
    }
    return true;
}

//}}}
//{{{  CdlTransaction statics                   

// ----------------------------------------------------------------------------
void (*CdlTransactionBody::callback_fn)(const CdlTransactionCallback&)  = 0;
CdlInferenceCallback    CdlTransactionBody::inference_callback          = 0;
bool                    CdlTransactionBody::inference_enabled           = true;
int                     CdlTransactionBody::inference_recursion_limit   = 3;
CdlValueSource          CdlTransactionBody::inference_override          = CdlValueSource_Inferred;
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlTransactionBody);

//}}}
//{{{  Transaction creation and destruction     

// ----------------------------------------------------------------------------
CdlTransaction
CdlTransactionBody::make(CdlToplevel toplevel)
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::make", "result %p");
    CYG_REPORT_FUNCARG1XV(toplevel);
    CYG_PRECONDITION_CLASSC(toplevel);
    CYG_PRECONDITIONC(0 == toplevel->transaction);
    
    CdlTransaction result = new CdlTransactionBody(toplevel, 0, 0);
    toplevel->transaction = result;

    CYG_REPORT_RETVAL(result);
    return result;
}

CdlTransaction
CdlTransactionBody::make(CdlConflict conflict)
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::make (sub-transaction)", "result %p");
    CYG_REPORT_FUNCARG2XV(this, conflict);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_ZERO_OR_CLASSC(conflict);

    CdlTransaction result = new CdlTransactionBody(0, this, conflict);

    CYG_REPORT_RETVAL(result);
    return result;
}
    
CdlTransactionBody::CdlTransactionBody(CdlToplevel toplevel_arg, CdlTransaction parent_arg, CdlConflict conflict_arg)
{
    CYG_REPORT_FUNCNAME("CdlTransaction:: constructor");
    CYG_REPORT_FUNCARG4XV(this, toplevel_arg, parent_arg, conflict_arg);
    CYG_PRECONDITION_ZERO_OR_CLASSC(toplevel_arg);
    CYG_PRECONDITION_ZERO_OR_CLASSC(parent_arg);
    CYG_PRECONDITION_ZERO_OR_CLASSC(conflict_arg);
    CYG_PRECONDITIONC( ((0 == toplevel_arg) && (0 != parent_arg)) || ((0 == parent_arg) && (0 != toplevel_arg)));

    // The containers will take care of themselves, as will all_changes
    toplevel    = toplevel_arg;
    parent      = parent_arg;
    conflict    = conflict_arg;
    dirty       = false;
    cdltransactionbody_cookie   = CdlTransactionBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
CdlTransactionBody::~CdlTransactionBody()
{
    CYG_REPORT_FUNCNAME("CdlTransaction:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    // The transaction must have been either committed or cancelled.
    // This means that various of the STL containers should be empty
    CYG_ASSERTC(0 == commit_cancel_ops.size());
    CYG_ASSERTC(0 == changes.size());
    CYG_ASSERTC(0 == deleted_conflicts.size());
    CYG_ASSERTC(0 == deleted_structural_conflicts.size());
    CYG_ASSERTC(0 == new_conflicts.size());
    CYG_ASSERTC(0 == new_structural_conflicts.size());
    CYG_ASSERTC(0 == resolved_conflicts.size());
    CYG_ASSERTC(0 == global_conflicts_with_solutions.size());
    CYG_ASSERTC(0 == activated.size());
    CYG_ASSERTC(0 == deactivated.size());
    CYG_ASSERTC(0 == legal_values_changes.size());
    CYG_ASSERTC(0 == value_changes.size());
    CYG_ASSERTC(0 == active_changes.size());
    
    // If this was a toplevel transaction, the toplevel knows
    // about the transaction.
    if (0 != toplevel) {
        CYG_ASSERTC(toplevel->transaction == this);
        toplevel->transaction = 0;
    }
    cdltransactionbody_cookie   = CdlTransactionBody_Invalid;
    toplevel    = 0;
    parent      = 0;
    conflict    = 0;
    dirty       = false;

    CYGDBG_MEMLEAK_DESTRUCTOR();

    CYG_REPORT_RETURN();
}

//}}}
//{{{  check_this()                             

// ----------------------------------------------------------------------------

bool
CdlTransactionBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlTransactionBody_Magic != cdltransactionbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();

    //zeal = cyg_extreme;
    switch(zeal) {
      case cyg_system_test:
      case cyg_extreme :
      {
          std::map<CdlValuable,CdlValue>::const_iterator map_i;
          for (map_i = changes.begin(); map_i != changes.end(); map_i++) {
              if (!map_i->first->check_this(cyg_quick) || !map_i->second.check_this(cyg_quick)) {
                  return false;
              }
          }

          std::list<CdlConflict>::const_iterator conf_i;
          for (conf_i = new_conflicts.begin(); conf_i != new_conflicts.end(); conf_i++) {
              if (!(*conf_i)->check_this(cyg_quick)) {
                  return false;
              }
          }
          for (conf_i = new_structural_conflicts.begin(); conf_i != new_structural_conflicts.end(); conf_i++) {
              if (!(*conf_i)->check_this(cyg_quick)) {
                  return false;
              }
          }

          std::vector<CdlConflict>::const_iterator conf_i2;
          for (conf_i2 = deleted_conflicts.begin(); conf_i2 != deleted_conflicts.end(); conf_i2++) {
              if (!(*conf_i2)->check_this(cyg_quick)) {
                  return false;
              }
          }
          for (conf_i2 = resolved_conflicts.begin(); conf_i2 != resolved_conflicts.end(); conf_i2++) {
              if (!(*conf_i2)->check_this(cyg_quick)) {
                  return false;
              }
          }
          for (conf_i2 = deleted_structural_conflicts.begin(); conf_i2 != deleted_structural_conflicts.end(); conf_i2++) {
              if (!(*conf_i2)->check_this(cyg_quick)) {
                  return false;
              }
          }
          for (conf_i = global_conflicts_with_solutions.begin(); conf_i != global_conflicts_with_solutions.end(); conf_i++) {
              if (!(*conf_i)->check_this(cyg_quick)) {
                  return false;
              }
              if (0 != (*conf_i)->transaction) {
                  return false;
              }
          }
          
          // Nodes cannot have been both activated and deactivated in one transaction
          std::set<CdlNode>::const_iterator node_i;
          for (node_i = activated.begin(); node_i != activated.end(); node_i++) {
              if (!(*node_i)->check_this(cyg_quick)) {
                  return false;
              }
              if (deactivated.end() != deactivated.find(*node_i)) {
                  return false;
              }
          }
          for (node_i = deactivated.begin(); node_i != deactivated.end(); node_i++) {
              if (!(*node_i)->check_this(cyg_quick)) {
                  return false;
              }
              if (activated.end() != activated.find(*node_i)) {
                  return false;
              }
          }
          std::set<CdlValuable>::const_iterator val_i;
          for (val_i = legal_values_changes.begin(); val_i != legal_values_changes.end(); val_i++) {
              if (!(*val_i)->check_this(cyg_quick)) {
                  return false;
              }
          }
          
          std::deque<CdlValuable>::const_iterator val_i2;
          for (val_i2 = value_changes.begin(); val_i2 != value_changes.end(); val_i2++) {
              if (!(*val_i2)->check_this(cyg_quick)) {
                  return false;
              }
          }
          
          std::deque<CdlNode>::const_iterator active_i;
          for (active_i = active_changes.begin(); active_i != active_changes.end(); active_i++) {
              if (!(*active_i)->check_this(cyg_quick)) {
                  return false;
              }
          }
      }
      case cyg_thorough:
          if ((0 != toplevel) && !toplevel->check_this(cyg_quick)) {
              return false;
          }
          if ((0 != parent) && !parent->check_this(cyg_quick)) {
              return false;
          }
          if ((0 != conflict) && !conflict->check_this(cyg_quick)) {
              return false;
          }
              
      case cyg_quick:
      case cyg_trivial :
          if ((0 == toplevel) && (0 == parent)) {
              return false;
          }
          if (this == parent) {
              return false;
          }
          
      case cyg_none :
        break;
    }

    return true;
}

//}}}
//{{{  Misc                                     

// ----------------------------------------------------------------------------
CdlToplevel
CdlTransactionBody::get_toplevel() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::get_toplevel", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlToplevel result = toplevel;
    
    CYG_REPORT_RETVAL(result);
    return result;
}

CdlTransaction
CdlTransactionBody::get_parent() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::get_parent", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlTransaction result = parent;

    CYG_REPORT_RETVAL(result);
    return result;
}

CdlConflict
CdlTransactionBody::get_conflict() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::get_conflict", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlConflict result = conflict;

    CYG_REPORT_RETVAL(result);
    return result;
}


// ----------------------------------------------------------------------------
void (*CdlTransactionBody::get_callback_fn())(const CdlTransactionCallback&)
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::get_callback_fn", "result %p");

    void (*result)(const CdlTransactionCallback&) = callback_fn;

    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlTransactionBody::set_callback_fn(void (*fn)(const CdlTransactionCallback&))
{
    CYG_REPORT_FUNCNAME("CdlTransaction::set_callback_fn");
    CYG_REPORT_FUNCARG1XV(fn);

    callback_fn = fn;

    CYG_REPORT_RETURN();
}

CdlInferenceCallback
CdlTransactionBody::get_inference_callback_fn()
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::get_inference_callback_fn", "result %p");

    CdlInferenceCallback result = inference_callback;

    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlTransactionBody::set_inference_callback_fn(CdlInferenceCallback fn)
{
    CYG_REPORT_FUNCNAME("CdlTransaction::set_inference_callback");
    CYG_REPORT_FUNCARG1XV(fn);

    inference_callback = fn;

    CYG_REPORT_RETURN();
}

void
CdlTransactionBody::enable_automatic_inference()
{
    CYG_REPORT_FUNCNAME("CdlTransaction::enable_automatic_inference");

    inference_enabled = true;
    
    CYG_REPORT_RETURN();
}

void
CdlTransactionBody::disable_automatic_inference()
{
    CYG_REPORT_FUNCNAME("CdlTransaction::disable_automatic_inference");

    inference_enabled = false;

    CYG_REPORT_RETURN();
}

bool
CdlTransactionBody::is_automatic_inference_enabled()
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::is_automatic_inference_enabled", "result %d");

    bool result = inference_enabled;

    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlTransactionBody::set_inference_recursion_limit(int limit)
{
    CYG_REPORT_FUNCNAME("CdlTransaction::set_inference_recursion_limit");
    CYG_REPORT_FUNCARG1XV(limit);
    CYG_PRECONDITIONC(0 < limit);
    CYG_PRECONDITIONC(limit < 16);    // arbitrary number
    
    inference_recursion_limit = limit;

    CYG_REPORT_RETURN();
}

int
CdlTransactionBody::get_inference_recursion_limit()
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::get_inference_recursion_limit", "result %d");

    int result = inference_recursion_limit;

    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlTransactionBody::set_inference_override(CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlTransaction::set_inference_override");
    CYG_REPORT_FUNCARG1XV(source);
    CYG_PRECONDITIONC((CdlValueSource_Invalid == source) || Cdl::is_valid_value_source(source));
    
    inference_override = source;

    CYG_REPORT_RETURN();
}

CdlValueSource
CdlTransactionBody::get_inference_override()
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::get_inference_override", "result %d");

    CdlValueSource result = inference_override;

    CYG_REPORT_RETVAL((int) result);
    return result;
}

//}}}
//{{{  Value access and updates                 

// ----------------------------------------------------------------------------
const CdlValue&
CdlTransactionBody::get_whole_value(CdlConstValuable valuable_arg) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::get_whole_value", "result %p");
    CYG_REPORT_FUNCARG2XV(this, valuable_arg);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(valuable_arg);

    // Unfortunately I need a valuable rather than a const-valuable when
    // accessing the STL containers.
    CdlValuable valuable = const_cast<CdlValuable>(valuable_arg);

    // If we are trying to find a solution, keep track of all valuables
    // that were accessed.
    const CdlValue* result = 0;
    if (0 != conflict) {
        conflict->solution_references.insert(valuable);
    }
    
    std::map<CdlValuable,CdlValue>::const_iterator val_i;
    val_i = changes.find(valuable);
    if (val_i != changes.end()) {
        result = &(val_i->second);
    } else if (0 != parent) {
        result = &(parent->get_whole_value(valuable));
    } else {
        result = &(valuable->get_whole_value());
    }
        
    CYG_REPORT_RETVAL(result);
    return *result;
}

void
CdlTransactionBody::set_whole_value(CdlValuable valuable, const CdlValue& old_value, const CdlValue& new_value)
{
    CYG_REPORT_FUNCNAME("CdlTransaction::set_whole_value");
    CYG_REPORT_FUNCARG3XV(this, valuable, &new_value);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(valuable);
    CYG_PRECONDITION_CLASSOC(old_value);
    CYG_PRECONDITION_CLASSOC(new_value);
    CYG_PRECONDITIONC(&old_value != &new_value);

    CdlValueFlavor flavor = old_value.get_flavor();
    CYG_ASSERTC(flavor == new_value.get_flavor());
    CYG_ASSERTC(CdlValueFlavor_None != flavor);
    
    bool value_changed = false;
    bool bool_changed  = false;
    
    if ((CdlValueFlavor_Bool == flavor) || (CdlValueFlavor_BoolData == flavor)) {
        if (old_value.is_enabled() != new_value.is_enabled()) {
            value_changed = true;
            bool_changed  = true;
        }
    }
    if (!value_changed && ((CdlValueFlavor_BoolData == flavor) || (CdlValueFlavor_Data == flavor))) {
        if (old_value.get_simple_value() != new_value.get_simple_value()) {
            value_changed = true;
        }
    }
    if (value_changed) {
        std::deque<CdlValuable>::const_iterator change_i;
        change_i = std::find(value_changes.begin(), value_changes.end(), valuable);
        if (change_i == value_changes.end()) {
            value_changes.push_back(valuable);
        }
    }
                                                                              
    // Actually do the update. This may modify old_value, so has to be
    // left to the end.
    changes[valuable] = new_value;

    // If there was a change to the boolean part of the value and the valuable
    // implements an interface, the interface value needs to be recalculated.
    if (bool_changed && valuable->has_property(CdlPropertyId_Implements)) {
        std::vector<CdlInterface> interfaces;
        std::vector<CdlInterface>::const_iterator interface_i;
        valuable->get_implemented_interfaces(interfaces);
        for (interface_i = interfaces.begin(); interface_i != interfaces.end(); interface_i++) {
            (*interface_i)->recalculate(this);
        }
    }
    
    CYG_REPORT_RETURN();
}

const std::map<CdlValuable, CdlValue>&
CdlTransactionBody::get_changes() const
{
    CYG_REPORT_FUNCNAME("CdlTransaction::get_changes");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return changes;
}

//}}}
//{{{  Active access and updates                

// ----------------------------------------------------------------------------
// Nodes can become active or inactive during transactions, and this affects
// propagation and expression evaluation

bool
CdlTransactionBody::is_active(CdlNode node) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::is_active", "result %d");
    CYG_REPORT_FUNCARG2XV(this, node);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);

    bool result = false;
    if (activated.end() != activated.find(node)) {
        result = true;
    } else if (deactivated.end() != deactivated.find(node)) {
        result = false;
    } else if (0 != parent) {
        result = parent->is_active(node);
    } else {
        result = node->is_active();
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlTransactionBody::set_active(CdlNode node, bool state)
{
    CYG_REPORT_FUNCNAME("CdlTransaction::set_active");
    CYG_REPORT_FUNCARG3XV(this, node, state);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);

    if (state) {
        activated.insert(node);
        std::set<CdlNode>::iterator node_i = deactivated.find(node);
        if (deactivated.end() != node_i) {
            deactivated.erase(node_i);
        }
    } else {
        deactivated.insert(node);
        std::set<CdlNode>::iterator node_i = activated.find(node);
        if (activated.end() != node_i) {
            activated.erase(node_i);
        }
    }
    active_changes.push_back(node);

    CdlValuable valuable = dynamic_cast<CdlValuable>(node);
    if ((0 != valuable) && valuable->has_property(CdlPropertyId_Implements)) {
        std::vector<CdlInterface> interfaces;
        std::vector<CdlInterface>::const_iterator interface_i;
        valuable->get_implemented_interfaces(interfaces);
        for (interface_i = interfaces.begin(); interface_i != interfaces.end(); interface_i++) {
            (*interface_i)->recalculate(this);
        }
    }
        
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Conflict access and updates              

//{{{  get_conflict() etc.                      

// ----------------------------------------------------------------------------
bool
CdlTransactionBody::has_conflict(CdlNode node, bool (*pFn)(CdlConflict))
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::has_conflict", "result %d");
    CYG_REPORT_FUNCARG3XV(this, node, pFn);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);

    // Because it is necessary to check whether or not any given
    // conflict has been cleared in the current transaction or any
    // parent transaction, recursion into the parent is not
    // appropriate.
    bool result = false;
    std::list<CdlConflict>::const_iterator conf_i;
    CdlTransaction current_transaction = this;
    CdlToplevel    toplevel            = this->toplevel;
    do {
        CYG_LOOP_INVARIANT_CLASSC(current_transaction);
        
        for (conf_i = current_transaction->new_conflicts.begin();
             conf_i != current_transaction->new_conflicts.end();
             conf_i++) {

            if ((node == (*conf_i)->get_node()) &&
                !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                result = true;
                break;
            }
        }
            
        toplevel            = current_transaction->toplevel;
        current_transaction = current_transaction->parent;
    } while (!result && (0 != current_transaction));

    if (!result) {
        CYG_ASSERT_CLASSC(toplevel);
        for (conf_i = toplevel->conflicts.begin(); conf_i != toplevel->conflicts.end(); conf_i++) {
            if ((node == (*conf_i)->get_node()) && !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                result = true;
                break;
            }
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

CdlConflict
CdlTransactionBody::get_conflict(CdlNode node, bool (*pFn)(CdlConflict))
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::get_conflict", "result %p");
    CYG_REPORT_FUNCARG3XV(this, node, pFn);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);

    CdlConflict result = 0;
    std::list<CdlConflict>::const_iterator conf_i;
    CdlTransaction current_transaction = this;
    CdlToplevel    toplevel            = this->toplevel;
    do {
        CYG_LOOP_INVARIANT_CLASSC(current_transaction);
        
        for (conf_i = current_transaction->new_conflicts.begin();
             conf_i != current_transaction->new_conflicts.end();
             conf_i++) {

            if ((node == (*conf_i)->get_node()) &&
                !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                result = *conf_i;
                break;
            }
        }
            
        toplevel            = current_transaction->toplevel;
        current_transaction = current_transaction->parent;
    } while ((0 == result) && (0 != current_transaction));

    if (0 == result) {
        CYG_ASSERT_CLASSC(toplevel);
        for (conf_i = toplevel->conflicts.begin(); conf_i != toplevel->conflicts.end(); conf_i++) {
            if ((node == (*conf_i)->get_node()) && !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                result = *conf_i;
                break;
            }
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlTransactionBody::get_conflicts(CdlNode node, bool (*pFn)(CdlConflict), std::vector<CdlConflict>& result)
{
    CYG_REPORT_FUNCNAME("CdlTransaction::get_conflicts");
    CYG_REPORT_FUNCARG3XV(this, node, pFn);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);

    std::list<CdlConflict>::const_iterator conf_i;
    CdlTransaction current_transaction = this;
    CdlToplevel    toplevel            = this->toplevel;
    do {
        CYG_LOOP_INVARIANT_CLASSC(current_transaction);
        
        for (conf_i = current_transaction->new_conflicts.begin();
             conf_i != current_transaction->new_conflicts.end();
             conf_i++) {

            if ((node == (*conf_i)->get_node()) &&
                !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                result.push_back(*conf_i);
            }
        }
            
        toplevel            = current_transaction->toplevel;
        current_transaction = current_transaction->parent;
    } while (0 != current_transaction);

    CYG_ASSERT_CLASSC(toplevel);
    for (conf_i = toplevel->conflicts.begin(); conf_i != toplevel->conflicts.end(); conf_i++) {
        if ((node == (*conf_i)->get_node()) && !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
            result.push_back(*conf_i);
        }
    }

    CYG_REPORT_RETURN();
}

bool
CdlTransactionBody::has_conflict(CdlNode node, CdlProperty prop, bool (*pFn)(CdlConflict))
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::has_conflict", "result %d");
    CYG_REPORT_FUNCARG4XV(this, node, prop, pFn);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);
    CYG_PRECONDITION_CLASSC(prop);
    
    bool result = false;
    std::list<CdlConflict>::const_iterator conf_i;
    CdlTransaction current_transaction = this;
    CdlToplevel    toplevel            = this->toplevel;
    do {
        CYG_LOOP_INVARIANT_CLASSC(current_transaction);
        
        for (conf_i = current_transaction->new_conflicts.begin();
             conf_i != current_transaction->new_conflicts.end();
             conf_i++) {

            if ((node == (*conf_i)->get_node()) && (prop == (*conf_i)->get_property()) &&
                !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                
                result = true;
                break;
            }
        }
            
        toplevel            = current_transaction->toplevel;
        current_transaction = current_transaction->parent;
    } while (!result && (0 != current_transaction));

    if (!result) {
        CYG_ASSERT_CLASSC(toplevel);
        for (conf_i = toplevel->conflicts.begin(); conf_i != toplevel->conflicts.end(); conf_i++) {
            if ((node == (*conf_i)->get_node()) && (prop == (*conf_i)->get_property()) &&
                !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                
                result = true;
                break;
            }
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

CdlConflict
CdlTransactionBody::get_conflict(CdlNode node, CdlProperty prop, bool (*pFn)(CdlConflict))
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::get_conflict", "result %p");
    CYG_REPORT_FUNCARG4XV(this, node, prop, pFn);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);
    CYG_PRECONDITION_CLASSC(prop);

    CdlConflict result = 0;
    std::list<CdlConflict>::const_iterator conf_i;
    CdlTransaction current_transaction = this;
    CdlToplevel    toplevel            = this->toplevel;
    do {
        CYG_LOOP_INVARIANT_CLASSC(current_transaction);
        
        for (conf_i = current_transaction->new_conflicts.begin();
             conf_i != current_transaction->new_conflicts.end();
             conf_i++) {

            if ((node == (*conf_i)->get_node()) && (prop == (*conf_i)->get_property()) &&
                !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                result = *conf_i;
                break;
            }
        }
            
        toplevel            = current_transaction->toplevel;
        current_transaction = current_transaction->parent;
    } while ((0 == result) && (0 != current_transaction));

    if (0 == result) {
        CYG_ASSERT_CLASSC(toplevel);
        for (conf_i = toplevel->conflicts.begin(); conf_i != toplevel->conflicts.end(); conf_i++) {
            if ((node == (*conf_i)->get_node()) && (prop == (*conf_i)->get_property()) &&
                !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                
                result = *conf_i;
                break;
            }
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlTransactionBody::get_conflicts(CdlNode node, CdlProperty prop, bool (*pFn)(CdlConflict), std::vector<CdlConflict>& result)
{
    CYG_REPORT_FUNCNAME("CdlTransaction::get_conflict");
    CYG_REPORT_FUNCARG4XV(this, node, prop, pFn);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);
    CYG_PRECONDITION_CLASSC(prop);

    std::list<CdlConflict>::const_iterator conf_i;
    CdlTransaction current_transaction = this;
    CdlToplevel    toplevel            = this->toplevel;
    do {
        CYG_LOOP_INVARIANT_CLASSC(current_transaction);
        
        for (conf_i = current_transaction->new_conflicts.begin();
             conf_i != current_transaction->new_conflicts.end();
             conf_i++) {

            if ((node == (*conf_i)->get_node()) && (prop == (*conf_i)->get_property()) &&
                !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                result.push_back(*conf_i);
            }
        }
            
        toplevel            = current_transaction->toplevel;
        current_transaction = current_transaction->parent;
    } while (0 != current_transaction);

    CYG_ASSERT_CLASSC(toplevel);
    for (conf_i = toplevel->conflicts.begin(); conf_i != toplevel->conflicts.end(); conf_i++) {
        if ((node == (*conf_i)->get_node()) && (prop == (*conf_i)->get_property()) &&
            !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
            
            result.push_back(*conf_i);
        }
    }

    CYG_REPORT_RETURN();
}

//}}}
//{{{  get_structural_conflict() etc            

// ----------------------------------------------------------------------------
bool
CdlTransactionBody::has_structural_conflict(CdlNode node, bool (*pFn)(CdlConflict))
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::has_structural_conflict", "result %d");
    CYG_REPORT_FUNCARG3XV(this, node, pFn);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);

    // Because it is necessary to check whether or not any given
    // conflict has been cleared in the current transaction or any
    // parent transaction, recursion into the parent is not
    // appropriate.
    bool result = false;
    std::list<CdlConflict>::const_iterator conf_i;
    CdlTransaction current_transaction = this;
    CdlToplevel    toplevel            = this->toplevel;
    do {
        CYG_LOOP_INVARIANT_CLASSC(current_transaction);
        
        for (conf_i = current_transaction->new_structural_conflicts.begin();
             conf_i != current_transaction->new_structural_conflicts.end();
             conf_i++) {

            if ((node == (*conf_i)->get_node()) &&
                !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                result = true;
                break;
            }
        }
            
        toplevel            = current_transaction->toplevel;
        current_transaction = current_transaction->parent;
    } while (!result && (0 != current_transaction));

    if (!result) {
        CYG_ASSERT_CLASSC(toplevel);
        for (conf_i = toplevel->structural_conflicts.begin(); conf_i != toplevel->structural_conflicts.end(); conf_i++) {
            if ((node == (*conf_i)->get_node()) && !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                result = true;
                break;
            }
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

CdlConflict
CdlTransactionBody::get_structural_conflict(CdlNode node, bool (*pFn)(CdlConflict))
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::get_structural_conflict", "result %p");
    CYG_REPORT_FUNCARG3XV(this, node, pFn);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);

    CdlConflict result = 0;
    std::list<CdlConflict>::const_iterator conf_i;
    CdlTransaction current_transaction = this;
    CdlToplevel    toplevel            = this->toplevel;
    do {
        CYG_LOOP_INVARIANT_CLASSC(current_transaction);
        
        for (conf_i = current_transaction->new_structural_conflicts.begin();
             conf_i != current_transaction->new_structural_conflicts.end();
             conf_i++) {

            if ((node == (*conf_i)->get_node()) &&
                !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                result = *conf_i;
                break;
            }
        }
            
        toplevel            = current_transaction->toplevel;
        current_transaction = current_transaction->parent;
    } while ((0 == result) && (0 != current_transaction));

    if (0 == result) {
        CYG_ASSERT_CLASSC(toplevel);
        for (conf_i = toplevel->structural_conflicts.begin(); conf_i != toplevel->structural_conflicts.end(); conf_i++) {
            if ((node == (*conf_i)->get_node()) && !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                result = *conf_i;
                break;
            }
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlTransactionBody::get_structural_conflicts(CdlNode node, bool (*pFn)(CdlConflict), std::vector<CdlConflict>& result)
{
    CYG_REPORT_FUNCNAME("CdlTransaction::get_structural_conflicts");
    CYG_REPORT_FUNCARG3XV(this, node, pFn);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);

    std::list<CdlConflict>::const_iterator conf_i;
    CdlTransaction current_transaction = this;
    CdlToplevel    toplevel            = this->toplevel;
    do {
        CYG_LOOP_INVARIANT_CLASSC(current_transaction);
        
        for (conf_i = current_transaction->new_structural_conflicts.begin();
             conf_i != current_transaction->new_structural_conflicts.end();
             conf_i++) {

            if ((node == (*conf_i)->get_node()) &&
                !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                result.push_back(*conf_i);
            }
        }
            
        toplevel            = current_transaction->toplevel;
        current_transaction = current_transaction->parent;
    } while (0 != current_transaction);

    CYG_ASSERT_CLASSC(toplevel);
    for (conf_i = toplevel->structural_conflicts.begin(); conf_i != toplevel->structural_conflicts.end(); conf_i++) {
        if ((node == (*conf_i)->get_node()) && !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
            result.push_back(*conf_i);
        }
    }

    CYG_REPORT_RETURN();
}

bool
CdlTransactionBody::has_structural_conflict(CdlNode node, CdlProperty prop, bool (*pFn)(CdlConflict))
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::has_structural_conflict", "result %d");
    CYG_REPORT_FUNCARG4XV(this, node, prop, pFn);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);
    CYG_PRECONDITION_CLASSC(prop);

    bool result = false;
    std::list<CdlConflict>::const_iterator conf_i;
    CdlTransaction current_transaction = this;
    CdlToplevel    toplevel            = this->toplevel;
    do {
        CYG_LOOP_INVARIANT_CLASSC(current_transaction);
        
        for (conf_i = current_transaction->new_structural_conflicts.begin();
             conf_i != current_transaction->new_structural_conflicts.end();
             conf_i++) {

            if ((node == (*conf_i)->get_node()) && (prop == (*conf_i)->get_property()) &&
                !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                
                result = true;
                break;
            }
        }
            
        toplevel            = current_transaction->toplevel;
        current_transaction = current_transaction->parent;
    } while (!result && (0 != current_transaction));

    if (!result) {
        CYG_ASSERT_CLASSC(toplevel);
        for (conf_i = toplevel->structural_conflicts.begin(); conf_i != toplevel->structural_conflicts.end(); conf_i++) {
            if ((node == (*conf_i)->get_node()) && (prop == (*conf_i)->get_property()) &&
                !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                
                result = true;
                break;
            }
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

CdlConflict
CdlTransactionBody::get_structural_conflict(CdlNode node, CdlProperty prop, bool (*pFn)(CdlConflict))
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::get_structural_conflict", "result %p");
    CYG_REPORT_FUNCARG4XV(this, node, prop, pFn);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);
    CYG_PRECONDITION_CLASSC(prop);

    CdlConflict result = 0;
    std::list<CdlConflict>::const_iterator conf_i;
    CdlTransaction current_transaction = this;
    CdlToplevel    toplevel            = this->toplevel;
    do {
        CYG_LOOP_INVARIANT_CLASSC(current_transaction);
        
        for (conf_i = current_transaction->new_structural_conflicts.begin();
             conf_i != current_transaction->new_structural_conflicts.end();
             conf_i++) {

            if ((node == (*conf_i)->get_node()) && (prop == (*conf_i)->get_property()) &&
                !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                result = *conf_i;
                break;
            }
        }
            
        toplevel            = current_transaction->toplevel;
        current_transaction = current_transaction->parent;
    } while ((0 == result) && (0 != current_transaction));

    if (0 == result) {
        CYG_ASSERT_CLASSC(toplevel);
        for (conf_i = toplevel->structural_conflicts.begin(); conf_i != toplevel->structural_conflicts.end(); conf_i++) {
            if ((node == (*conf_i)->get_node()) && (prop == (*conf_i)->get_property()) &&
                !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                
                result = *conf_i;
                break;
            }
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlTransactionBody::get_structural_conflicts(CdlNode node, CdlProperty prop, bool (*pFn)(CdlConflict),
                                             std::vector<CdlConflict>& result)
{
    CYG_REPORT_FUNCNAME("CdlTransaction::get_structural_conflict");
    CYG_REPORT_FUNCARG4XV(this, node, prop, pFn);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);
    CYG_PRECONDITION_CLASSC(prop);

    std::list<CdlConflict>::const_iterator conf_i;
    CdlTransaction current_transaction = this;
    CdlToplevel    toplevel            = this->toplevel;
    do {
        CYG_LOOP_INVARIANT_CLASSC(current_transaction);
        
        for (conf_i = current_transaction->new_structural_conflicts.begin();
             conf_i != current_transaction->new_structural_conflicts.end();
             conf_i++) {

            if ((node == (*conf_i)->get_node()) && (prop == (*conf_i)->get_property()) &&
                !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
                result.push_back(*conf_i);
            }
        }
            
        toplevel            = current_transaction->toplevel;
        current_transaction = current_transaction->parent;
    } while (0 != current_transaction);

    CYG_ASSERT_CLASSC(toplevel);
    for (conf_i = toplevel->structural_conflicts.begin(); conf_i != toplevel->structural_conflicts.end(); conf_i++) {
        if ((node == (*conf_i)->get_node()) && (prop == (*conf_i)->get_property()) &&
            !(this->has_conflict_been_cleared(*conf_i)) && (*pFn)(*conf_i)) {
            
            result.push_back(*conf_i);
        }
    }

    CYG_REPORT_RETURN();
}

//}}}
//{{{  clear_conflicts()                        

// ----------------------------------------------------------------------------
// Clearing a conflict. This can only happen in the context of a
// transaction.
void
CdlTransactionBody::clear_conflicts(CdlNode node, bool (*pFn)(CdlConflict))
{
    CYG_REPORT_FUNCNAME("CdlTransaction::clear_conflicts");
    CYG_REPORT_FUNCARG3XV(this, node, pFn);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);

    // Recursing into the parent is the wrong thing to do here, it
    // would result in the conflict being cleared in the parent rather
    // than in the current transaction.
    std::list<CdlConflict>::iterator conf_i;
    CdlTransaction current_transaction = this;
    CdlToplevel    toplevel            = this->toplevel;
    do {
        CYG_LOOP_INVARIANT_CLASSC(current_transaction);
        
        for (conf_i = current_transaction->new_conflicts.begin(); conf_i != current_transaction->new_conflicts.end(); ) {
            CdlConflict conflict = *conf_i++;
            if ((node == conflict->get_node()) && !(this->has_conflict_been_cleared(conflict)) && (*pFn)(conflict)) {
                this->clear_conflict(conflict);
            }
        }
        toplevel = current_transaction->toplevel;
        current_transaction = current_transaction->parent;
    } while (0 != current_transaction);
    
    CYG_ASSERT_CLASSC(toplevel);
    for (conf_i = toplevel->conflicts.begin(); conf_i != toplevel->conflicts.end(); ) {
        CdlConflict conflict = *conf_i++;
        if ((node == conflict->get_node()) && !(this->has_conflict_been_cleared(conflict)) && (*pFn)(conflict)) {
            this->clear_conflict(conflict);
        }
    }
    
    CYG_REPORT_RETURN();
}

void
CdlTransactionBody::clear_conflicts(CdlNode node, CdlProperty prop, bool (*pFn)(CdlConflict))
{
    CYG_REPORT_FUNCNAME("CdlTransaction::clear_conflicts");
    CYG_REPORT_FUNCARG4XV(this, node, prop, pFn);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);

    std::list<CdlConflict>::iterator conf_i;
    CdlTransaction current_transaction = this;
    CdlToplevel    toplevel            = this->toplevel;
    do {
        CYG_LOOP_INVARIANT_CLASSC(current_transaction);
        
        for (conf_i = current_transaction->new_conflicts.begin(); conf_i != current_transaction->new_conflicts.end(); ) {
            CdlConflict conflict = *conf_i++;
            if ((node == conflict->get_node()) && (prop == conflict->get_property()) && 
                !(this->has_conflict_been_cleared(conflict)) && (*pFn)(conflict)) {
                this->clear_conflict(conflict);
            }
        }
        toplevel = current_transaction->toplevel;
        current_transaction = current_transaction->parent;
    } while (0 != current_transaction);
    
    CYG_ASSERT_CLASSC(toplevel);
    for (conf_i = toplevel->conflicts.begin(); conf_i != toplevel->conflicts.end(); ) {
        CdlConflict conflict = *conf_i++;
        if ((node == conflict->get_node()) && (prop == conflict->get_property()) &&
            !(this->has_conflict_been_cleared(conflict)) && (*pFn)(conflict)) {
            this->clear_conflict(conflict);
        }
    }
    
    CYG_REPORT_RETURN();
}

void
CdlTransactionBody::clear_structural_conflicts(CdlNode node, bool (*pFn)(CdlConflict))
{
    CYG_REPORT_FUNCNAME("CdlTransaction::clear_structural_conflicts");
    CYG_REPORT_FUNCARG3XV(this, node, pFn);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);

    std::list<CdlConflict>::iterator conf_i;
    CdlTransaction current_transaction = this;
    CdlToplevel    toplevel            = this->toplevel;
    do {
        CYG_LOOP_INVARIANT_CLASSC(current_transaction);
        
        for (conf_i = current_transaction->new_structural_conflicts.begin();
             conf_i != current_transaction->new_structural_conflicts.end();
            ) {
            
            CdlConflict conflict = *conf_i++;
            if ((node == conflict->get_node()) && !(this->has_conflict_been_cleared(conflict)) && (*pFn)(conflict)) {
                this->clear_conflict(conflict);
            }
        }
        toplevel = current_transaction->toplevel;
        current_transaction = current_transaction->parent;
    } while (0 != current_transaction);
    
    CYG_ASSERT_CLASSC(toplevel);
    for (conf_i = toplevel->structural_conflicts.begin(); conf_i != toplevel->structural_conflicts.end(); ) {
        CdlConflict conflict = *conf_i++;
        if ((node == conflict->get_node()) && !(this->has_conflict_been_cleared(conflict)) && (*pFn)(conflict)) {
            this->clear_conflict(conflict);
        }
    }
    
    CYG_REPORT_RETURN();
}

void
CdlTransactionBody::clear_structural_conflicts(CdlNode node, CdlProperty prop, bool (*pFn)(CdlConflict))
{
    CYG_REPORT_FUNCNAME("CdlTransaction::clear_structural_conflicts");
    CYG_REPORT_FUNCARG4XV(this, node, prop, pFn);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(node);

    std::list<CdlConflict>::iterator conf_i;
    CdlTransaction current_transaction = this;
    CdlToplevel    toplevel            = this->toplevel;
    do {
        CYG_LOOP_INVARIANT_CLASSC(current_transaction);
        
        for (conf_i = current_transaction->new_structural_conflicts.begin();
             conf_i != current_transaction->new_structural_conflicts.end();
            ) {
            
            CdlConflict conflict = *conf_i++;
            if ((node == conflict->get_node()) && (prop == conflict->get_property()) && 
                !(this->has_conflict_been_cleared(conflict)) && (*pFn)(conflict)) {
                
                this->clear_conflict(conflict);
            }
        }
        toplevel = current_transaction->toplevel;
        current_transaction = current_transaction->parent;
    } while (0 != current_transaction);
    
    CYG_ASSERT_CLASSC(toplevel);
    for (conf_i = toplevel->structural_conflicts.begin(); conf_i != toplevel->structural_conflicts.end(); ) {
        CdlConflict conflict = *conf_i++;
        if ((node == conflict->get_node()) && (prop == conflict->get_property()) &&
            !(this->has_conflict_been_cleared(conflict)) && (*pFn)(conflict)) {
            this->clear_conflict(conflict);
        }
    }
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  clear_conflict()                         

// ----------------------------------------------------------------------------
void
CdlTransactionBody::clear_conflict(CdlConflict conflict)
{
    CYG_REPORT_FUNCNAME("CdlTransaction::clear_conflict");
    CYG_REPORT_FUNCARG2XV(this, conflict);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(conflict);

    // If this conflict was created during the transaction, it should
    // be on the new_conflicts or new_structural_conflicts container
    if (this == conflict->transaction) {
        // The conflict should be on one of the two new_conflicts deques.
        if (conflict->structural) {
            std::list<CdlConflict>::iterator conf_i = std::find(new_structural_conflicts.begin(),
                                                                  new_structural_conflicts.end(), conflict);
            CYG_ASSERTC(conf_i != new_structural_conflicts.end());
            new_structural_conflicts.erase(conf_i);
        } else {
            std::list<CdlConflict>::iterator conf_i = std::find(new_conflicts.begin(), new_conflicts.end(), conflict);
            CYG_ASSERTC(conf_i != new_conflicts.end());
            new_conflicts.erase(conf_i);
        }
        delete conflict;
        
    } else {
        if (conflict->structural) {
            deleted_structural_conflicts.push_back(conflict);
        } else {
            deleted_conflicts.push_back(conflict);
        }
    }

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
bool
CdlTransactionBody::has_conflict_been_cleared(CdlConflict conf)
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::has_conflict_been_cleared", "result %d");
    CYG_REPORT_FUNCARG2XV(this, conf);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(conf);
    
    bool result = false;
    CdlTransaction current_transaction = this;
    do {
        if (conf->structural) {
            if (std::find(current_transaction->deleted_structural_conflicts.begin(),
                          current_transaction->deleted_structural_conflicts.end(), conf) !=
                current_transaction->deleted_structural_conflicts.end()) {
                
                result = true;
            }
        } else {
            if (std::find(current_transaction->deleted_conflicts.begin(), current_transaction->deleted_conflicts.end(),
                          conf) != current_transaction->deleted_conflicts.end()) {
                result = true;
            }
        }

        current_transaction = current_transaction->parent;
    } while (!result && (0 != current_transaction));
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  per-transaction data                     

// ----------------------------------------------------------------------------
// Accessing the per-transaction conflict data.

const std::list<CdlConflict>&
CdlTransactionBody::get_new_conflicts() const
{
    CYG_REPORT_FUNCNAME("CdlTransaction::get_new_conflicts");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return new_conflicts;
}

const std::list<CdlConflict>&
CdlTransactionBody::get_new_structural_conflicts() const
{
    CYG_REPORT_FUNCNAME("CdlTransaction::get_new_structural_conflicts");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return new_structural_conflicts;
}

const std::vector<CdlConflict>&
CdlTransactionBody::get_deleted_conflicts() const
{
    CYG_REPORT_FUNCNAME("CdlTransaction::get_deleted_conflicts");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return deleted_conflicts;
}

const std::vector<CdlConflict>&
CdlTransactionBody::get_deleted_structural_conflicts() const
{
    CYG_REPORT_FUNCNAME("CdlTransaction::get_deleted_structural_conflicts");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return deleted_structural_conflicts;
}

const std::vector<CdlConflict>&
CdlTransactionBody::get_resolved_conflicts() const
{
    CYG_REPORT_FUNCNAME("CdlTransaction::get_resolved_conflicts");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return resolved_conflicts;
}

const std::list<CdlConflict>&
CdlTransactionBody::get_global_conflicts_with_solutions() const
{
    CYG_REPORT_FUNCNAME("CdlTransaction::get_global_conflicts_with_solutions");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return global_conflicts_with_solutions;
}

//}}}

//}}}
//{{{  Commit/cancel operations                 

// ----------------------------------------------------------------------------
void
CdlTransactionBody::add_commit_cancel_op(CdlTransactionCommitCancelOp* op)
{
    CYG_REPORT_FUNCNAME("CdlTransaction::add_commit_cancel_op");
    CYG_REPORT_FUNCARG2XV(this, op);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC(0 != op);

    commit_cancel_ops.push_back(op);
    
    CYG_REPORT_RETURN();
}

void
CdlTransactionBody::cancel_last_commit_cancel_op()
{
    CYG_REPORT_FUNCNAME("CdlTransaction::cancel_last_commit_cancel_op");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlTransactionCommitCancelOp* op = *(commit_cancel_ops.rbegin());
    commit_cancel_ops.pop_back();
    op->cancel(this);
    delete op;
        
    CYG_REPORT_RETURN();
}

CdlTransactionCommitCancelOp*
CdlTransactionBody::get_last_commit_cancel_op() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::get_last_commit_cancel_op", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlTransactionCommitCancelOp* op = *(commit_cancel_ops.rbegin());
    
    CYG_REPORT_RETVAL(op);
    return op;
}

const std::vector<CdlTransactionCommitCancelOp*>&
CdlTransactionBody::get_commit_cancel_ops() const
{
    CYG_REPORT_FUNCNAME("CdlTransaction::get_commit_cancel_ops");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return commit_cancel_ops;
}

//}}}
//{{{  Propagation                              

// ----------------------------------------------------------------------------
// Propagation should happen whenever one or more changes have been applied,
// so that the impact of these changes on other parts of the configuration
// can be fully assessed. The transaction keeps track of all the changes
// to date and invokes appropriate node and property update handlers.

void
CdlTransactionBody::propagate()
{
    CYG_REPORT_FUNCNAME("CdlTransaction::propagate");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_INVARIANT_THISC(CdlTransactionBody);
    
    // Now it is time to worry about value and active changes.
    // Propagation may result in new entries, so only the
    // front item of one of the vectors is modified.
    while ((0 < value_changes.size()) || (0 < active_changes.size())) {
        
        if (0 != value_changes.size()) {
            
            CdlValuable valuable = value_changes.front();
            value_changes.pop_front();
            
            // A value change may invalidate one or more solutions.
            // This happens during propagation rather than at the time
            // that the value is actually changed, so that multiple
            // solutions can be applied in one go.
            std::list<CdlConflict>::iterator conf_i, conf_j;
            for (conf_i = new_conflicts.begin(); conf_i != new_conflicts.end(); conf_i++) {
                CYG_LOOP_INVARIANT_CLASSC(*conf_i);
                (*conf_i)->update_solution_validity(valuable);
            }
            for (conf_i = global_conflicts_with_solutions.begin();
                 conf_i != global_conflicts_with_solutions.end();
                 ) {
                
                CYG_LOOP_INVARIANT_CLASSC(*conf_i);
                conf_j = conf_i++;

                (*conf_j)->update_solution_validity(valuable);
                if (!(*conf_j)->has_known_solution()) {
                    global_conflicts_with_solutions.erase(conf_j);
                }
            }

            // If the valuable is no longer loaded then there is
            // no need to worry about propagation
            if (0 != valuable->get_toplevel()) {
                
                // Inform the valuable itself about the update, so that
                // e.g. the value can be checked against legal_values
                valuable->update(this, CdlUpdate_ValueChange);
            
                std::vector<CdlReferrer>& referrers = valuable->referrers;
                std::vector<CdlReferrer>::iterator ref_i;
                for (ref_i = referrers.begin(); ref_i != referrers.end(); ref_i++) {
                    ref_i->update(this, valuable, CdlUpdate_ValueChange);
                }
            }
                
        } else {
            
            CdlNode node = active_changes.front();
            active_changes.pop_front();

            if (0 != node->get_toplevel()) {
                node->update(this, CdlUpdate_ActiveChange);
            
                std::vector<CdlReferrer>& referrers = node->referrers;
                std::vector<CdlReferrer>::iterator ref_i;
                for (ref_i = referrers.begin(); ref_i != referrers.end(); ref_i++) {
                    ref_i->update(this, node, CdlUpdate_ActiveChange);
                }
            }
        }
    }

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
bool
CdlTransactionBody::is_propagation_required() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::is_propagation_required", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = false;

    if ((0 != value_changes.size()) || (0 != active_changes.size())) {
        result = true;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}


// ----------------------------------------------------------------------------
void
CdlTransactionBody::add_legal_values_change(CdlValuable valuable)
{
    CYG_REPORT_FUNCNAME("CdlTransaction::add_legal_values_change");
    CYG_REPORT_FUNCARG2XV(this, valuable);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(valuable);

    legal_values_changes.insert(valuable);
    
    CYG_REPORT_RETURN();
}

const std::set<CdlValuable>&
CdlTransactionBody::get_legal_values_changes() const
{
    CYG_REPORT_FUNCNAME("CdlTransaction::get_legal_values_changes");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return legal_values_changes;
}

//}}}
//{{{  Cancel                                   

// ----------------------------------------------------------------------------
// Cancellation is straightforward, essentially it just involves clearing
// out all of the STL containers. The transaction object can then be-used,
// so fields like parent and toplevel must not change.
void
CdlTransactionBody::cancel()
{
    CYG_REPORT_FUNCNAME("CdlTransaction::cancel");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_INVARIANT_THISC(CdlTransactionBody);

    // First take care of the cancel ops, if any, in case a cancel op
    // depends on some of the other transaction state.
    std::vector<CdlTransactionCommitCancelOp*>::reverse_iterator cancel_i;
    for (cancel_i = commit_cancel_ops.rbegin(); cancel_i != commit_cancel_ops.rend(); cancel_i++) {
        (*cancel_i)->cancel(this);
        delete *cancel_i;
        *cancel_i = 0;
    }
    commit_cancel_ops.clear();
    
    this->changes.clear();
    std::list<CdlConflict>::iterator conf_i;
    for (conf_i = this->new_conflicts.begin(); conf_i != this->new_conflicts.end(); conf_i++) {
        CYG_LOOP_INVARIANT_CLASSC(*conf_i);
        delete *conf_i;
    }
    this->new_conflicts.clear();
    for (conf_i = this->new_structural_conflicts.begin(); conf_i != this->new_structural_conflicts.end(); conf_i++) {
        CYG_LOOP_INVARIANT_CLASSC(*conf_i);
        delete *conf_i;
    }
    this->new_structural_conflicts.clear();
    
    this->deleted_structural_conflicts.clear();
    this->deleted_conflicts.clear();
    
    // Any conflicts created and resolved during this transaction will
    // still be present in resolved_conflicts. Some global conflicts
    // may be there as well.
    std::vector<CdlConflict>::iterator conf_i2;
    for (conf_i2 = this->resolved_conflicts.begin(); conf_i2 != this->resolved_conflicts.end(); conf_i2++) {
        if (this == (*conf_i2)->transaction) {
            delete (*conf_i2);
        }
    }
    this->resolved_conflicts.clear();

    // Any global conflicts which have been updated with a solution need
    // to have that solution cleared. Currently no attempt is made to
    // keep solutions valid for global conflicts.
    for (conf_i = this->global_conflicts_with_solutions.begin();
         conf_i != this->global_conflicts_with_solutions.end();
         conf_i++) {

        CYG_LOOP_INVARIANT_CLASSC(*conf_i);
        (*conf_i)->clear_solution();
    }
    this->global_conflicts_with_solutions.clear();
    
    this->activated.clear();
    this->deactivated.clear();
    this->legal_values_changes.clear();
    this->value_changes.clear();
    this->active_changes.clear();

    CYG_REPORT_RETURN();
}

//}}}
//{{{  Commit                                   

// ----------------------------------------------------------------------------
// The commit operation. There are two main branches for this code. The
// first branch deals with sub-transactions, and basically involves
// transferring changes from the sub-transaction to the parent. It is
// assumed that the sub-transaction has been fully propagated, so
// data can just be transferred from the child to the parent.
//
// The second branch involves committing changes from a transaction to
// the toplevel, invoking the transaction callback if necessary.

void
CdlTransactionBody::commit()
{
    CYG_REPORT_FUNCNAME("CdlTransaction::commit");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_INVARIANT_THISC(CdlTransactionBody);

    std::map<CdlValuable, CdlValue>::iterator map_i;
    std::list<CdlConflict>::iterator conf_i, conf_j;
    std::vector<CdlConflict>::const_iterator conf_i2, conf_j2;
    std::set<CdlNode>::iterator set_i, set_j, set_k;
    std::set<CdlValuable>::iterator set_i2, set_j2;

    if (0 != parent) {
        // Any conflicts that were solved by the inference engine further
        // down are still resolved.
        // Great care has to be taken with conflict ownership. The following
        // cases have to be considered.
        // 1) the resolved conflict is global, its transaction is zero, this
        //    conflict must only be destroyed if the toplevel transaction
        //    is committed - at which time time the conflict should appear
        //    on the deleted_conflicts lists.
        // 2) the conflict belongs to a higher level transaction, we have
        //    recursed a certain amount trying to resolve it e.g. to explore
        //    OR branches of the tree. Again the resolved conflict can only
        //    be destroyed when the appropriate higher-level commit happens,
        //    and should appear on the deleted conflicts list.
        // 3) the conflict was created and resolved further down the tree.
        //    We are keeping it around for informational purposes only.
        //    Associating it with this transaction allows the code to
        //    distinguish this case from (1) and (2).
        for (conf_i2 = resolved_conflicts.begin(); conf_i2 != resolved_conflicts.end(); conf_i2++) {
            CdlConflict conf = *conf_i2;
            CYG_LOOP_INVARIANT_CLASSC(conf);
            CYG_LOOP_INVARIANTC(parent->resolved_conflicts.end() == \
                                std::find(parent->resolved_conflicts.begin(), \
                                          parent->resolved_conflicts.end(), conf));
            parent->resolved_conflicts.push_back(conf);
            parent->dirty = true;
            if (this == conf->transaction) {
                conf->transaction = parent;
            }
        }
        resolved_conflicts.clear();

        // Any global conflicts for which solutions were found in the
        // sub-transaction still have solutions in the parent.
        for (conf_i = global_conflicts_with_solutions.begin(); conf_i != global_conflicts_with_solutions.end(); conf_i++) {
            
            CdlConflict conf = *conf_i;
            CYG_LOOP_INVARIANT_CLASSC(conf);
            
            // It is not clear that this search is actually useful, especially
            // given that the solution is currently stored with the conflict
            // rather than with the transaction.
            conf_j = std::find(parent->global_conflicts_with_solutions.begin(),
                                parent->global_conflicts_with_solutions.end(),
                                conf);
            if (conf_j == parent->global_conflicts_with_solutions.end()) {
                parent->global_conflicts_with_solutions.push_back(conf);
                parent->dirty = true;
            }
        }
        global_conflicts_with_solutions.clear();
        
        // Now take care of deleted conflicts.
        for (conf_i2 = deleted_conflicts.begin(); conf_i2 != deleted_conflicts.end(); conf_i2++) {
            CdlConflict conf = *conf_i2;
            CYG_LOOP_INVARIANT_CLASSC(conf);
            // Possibilities to consider:
            // 1) the conflict may have been local to the parent transaction,
            //    in which case it can be deleted.
            // 2) the conflict may have been created in a higher-level
            //    transaction, in which case it has to be moved to the
            //    parent's deleted_conflicts vector.
            // 3) the conflict may also have been global, again it needs
            //    to be propagated into the parent's deleted_conflicts vector.
            //
            // But that is not the whole story. If this sub-transaction was
            // created specifically to resolve the conflict then the latter
            // should appear on the resolved vector and not be destroyed
            // immediately. This is true for both global and per-transaction
            // conflicts.
            //
            // For global conflicts it is also necessary to worry about
            // the global_conflicts_with_solutions list, that has to be
            // kept in synch with the rest of the world.
            conf_i = std::find(parent->new_conflicts.begin(), parent->new_conflicts.end(), conf);
            bool can_delete = false;
            if (conf_i != parent->new_conflicts.end()) {
                parent->new_conflicts.erase(conf_i);
                can_delete = true;
            } else {
                parent->deleted_conflicts.push_back(conf);
            }
            if (0 == conf->transaction) {
                conf_j = std::find(parent->global_conflicts_with_solutions.begin(),
                                   parent->global_conflicts_with_solutions.end(),
                                   conf);
                if (conf_j != parent->global_conflicts_with_solutions.end()) {
                    parent->global_conflicts_with_solutions.erase(conf_j);
                }
            }
            if (conf == this->conflict) {
                // The conflict may have been fortuitously resolved lower down,
                // in which case it will have appeared in this->resolved_conflicts()
                // and copied already.
                conf_j2 = std::find(parent->resolved_conflicts.begin(), parent->resolved_conflicts.end(), conf);
                if (conf_j2 == parent->resolved_conflicts.end()) {
                    parent->resolved_conflicts.push_back(conf);
                    parent->dirty = true;
                }
            } else if (can_delete) {
                delete conf;
            }
        }
        // Unnecessary, but let's keep things clean.
        deleted_conflicts.clear();

        // Deleted structural conflicts. For now the inference engine can do nothing with
        // these so they are a bit simpler.
        for (conf_i2 = deleted_structural_conflicts.begin(); conf_i2 != deleted_structural_conflicts.end(); conf_i2++) {
            
            CdlConflict conf = *conf_i2;
            CYG_LOOP_INVARIANT_CLASSC(conf);
            conf_i = std::find(parent->new_structural_conflicts.begin(), parent->new_structural_conflicts.end(), conf);
            if (conf_i != parent->new_structural_conflicts.end()) {
                parent->new_structural_conflicts.erase(conf_i);
                delete conf;
            } else {
                parent->deleted_structural_conflicts.push_back(conf);
            }
        }
        deleted_structural_conflicts.clear();
 
        // All value changes need to be propagated from the child to the parent.
        // Also, these value changes may invalidate existing solutions.
        for (map_i = changes.begin(); map_i != changes.end(); map_i++) {
            CYG_LOOP_INVARIANT_CLASSC(map_i->first);
            CYG_LOOP_INVARIANT_CLASSOC(map_i->second);
            parent->changes[map_i->first] = map_i->second;
            for (conf_i = parent->new_conflicts.begin(); conf_i != parent->new_conflicts.end(); conf_i++) {
                CYG_LOOP_INVARIANT_CLASSC(*conf_i);
                (*conf_i)->update_solution_validity(map_i->first);
            }
            for (conf_i = parent->global_conflicts_with_solutions.begin();
                 conf_i != parent->global_conflicts_with_solutions.end();
                 ) {

                conf_j = conf_i++;
                CYG_LOOP_INVARIANT_CLASSC(*conf_j);

                (*conf_j)->update_solution_validity(map_i->first);
                if (!(*conf_j)->has_known_solution()) {
                    parent->global_conflicts_with_solutions.erase(conf_j);
                }
            }
        }
        changes.clear();
        
        // Continue propagating the conflicts.New conflicts can just
        // be added.
        for (conf_i = new_conflicts.begin(); conf_i != new_conflicts.end(); conf_i++) {
            CYG_LOOP_INVARIANT_CLASSC(*conf_i);
            parent->new_conflicts.push_back(*conf_i);
            parent->dirty = true;
            (*conf_i)->transaction = parent;
        }
        for (conf_i = new_structural_conflicts.begin(); conf_i != new_structural_conflicts.end(); conf_i++) {
            CYG_LOOP_INVARIANT_CLASSC(*conf_i);
            parent->new_structural_conflicts.push_back(*conf_i);
            parent->dirty = true;
            (*conf_i)->transaction = parent;
        }
        // The cancel operation at the end will delete new conflicts, so the
        // containers had better be cleared here.
        new_conflicts.clear();
        new_structural_conflicts.clear();

        
        // Also keep track of nodes that have become active or inactive.
        set_j = parent->activated.begin();
        for (set_i = activated.begin(); set_i != activated.end(); set_i++) {
            set_j = parent->activated.insert(set_j, *set_i);
            set_k = parent->deactivated.find(*set_i);
            if (set_k != parent->deactivated.end()) {
                parent->deactivated.erase(set_k);
            }
        }
        
        set_j = parent->deactivated.begin();
        for (set_i = deactivated.begin(); set_i != deactivated.end(); set_i++) {
            set_j = parent->deactivated.insert(set_j, *set_i);
            set_k = parent->activated.find(*set_i);
            if (set_k != parent->activated.end()) {
                parent->activated.erase(set_k);
            }
        }
        activated.clear();
        deactivated.clear();

        // Keep track of other property changes.
        set_j2 = parent->legal_values_changes.begin();
        for (set_i2 = legal_values_changes.begin(); set_i2 != legal_values_changes.end(); set_i2++) {
            set_j2 = parent->legal_values_changes.insert(set_j2, *set_i2);
        }
        legal_values_changes.clear();

        // Any pending commit/cancel ops needs to be transferred to the parent
        parent->commit_cancel_ops.insert(parent->commit_cancel_ops.end(),
                                         this->commit_cancel_ops.begin(), this->commit_cancel_ops.end());
        this->commit_cancel_ops.clear();
        
    } else {
        
        CYG_ASSERT_CLASSC(toplevel);

        // If there is a registered callback function, it is necessary to fill
        // in the remaining fields of the all_changes callback structure. This
        // should happen before any conflicts get deleted. The actual callback
        // is invoked at the end, once all the changes have been moved to
        // the toplevel.
        CdlTransactionCallback all_changes(this);
        if (0 != callback_fn) {
            
            for (map_i = changes.begin(); map_i != changes.end(); map_i++) {
                if (0 == map_i->first->get_toplevel()) {
                    continue;
                }
                CdlValueFlavor flavor = map_i->second.get_flavor();
                const CdlValue& old_value = map_i->first->get_whole_value();
                CYG_LOOP_INVARIANTC(flavor == old_value.get_flavor());
                bool value_changed = false;
                
                if (old_value.get_source() != map_i->second.get_source()) {
                    all_changes.value_source_changes.push_back(map_i->first);
                }
                if ((CdlValueFlavor_Bool == flavor) || (CdlValueFlavor_BoolData == flavor)) {
                    if (old_value.is_enabled() != map_i->second.is_enabled()) {
                        value_changed = true;
                    }
                }
                if (!value_changed && ((CdlValueFlavor_BoolData == flavor) || (CdlValueFlavor_Data == flavor))) {
                    if (old_value.get_simple_value() != map_i->second.get_simple_value()) {
                        value_changed = true;
                    }
                }
                if (value_changed) {
                    all_changes.value_changes.push_back(map_i->first);
                }
            }
            for (conf_i = new_conflicts.begin(); conf_i != new_conflicts.end(); conf_i++) {
                all_changes.new_conflicts.push_back(*conf_i);
            }
            for (conf_i = new_structural_conflicts.begin(); conf_i != new_structural_conflicts.end(); conf_i++) {
                all_changes.new_structural_conflicts.push_back(*conf_i);
            }
            for (conf_i2 = deleted_conflicts.begin(); conf_i2 != deleted_conflicts.end(); conf_i2++) {
                CdlNode node = (*conf_i2)->get_node();
                CYG_LOOP_INVARIANT_CLASSC(node);

                all_changes.nodes_with_resolved_conflicts.push_back(node);
            }
            for (conf_i2 = deleted_structural_conflicts.begin(); conf_i2 != deleted_structural_conflicts.end(); conf_i2++) {
                CdlNode node = (*conf_i2)->get_node();
                CYG_LOOP_INVARIANT_CLASSC(node);

                all_changes.nodes_with_resolved_structural_conflicts.push_back(node);
            }
            for (set_i = activated.begin(); set_i != activated.end(); set_i++) {
                if (0 != (*set_i)->get_toplevel()) {
                    all_changes.active_changes.push_back(*set_i);
                }
            }
            for (set_i = deactivated.begin(); set_i != deactivated.end(); set_i++) {
                if (0 != (*set_i)->get_toplevel()) {
                    all_changes.active_changes.push_back(*set_i);
                }
            }
            for (set_i2 = legal_values_changes.begin(); set_i2 != legal_values_changes.end(); set_i2++) {
                if (0 != (*set_i)->get_toplevel()) {
                    all_changes.legal_values_changes.push_back(*set_i2);
                }
            }
            legal_values_changes.clear();
        }

        // All new values need to be installed in the appropriate valuable.
        for (map_i = changes.begin(); map_i != changes.end(); map_i++) {
            CYG_LOOP_INVARIANT_CLASSC(map_i->first);
            CYG_LOOP_INVARIANT_CLASSOC(map_i->second);
            map_i->first->value = map_i->second;
        }
        changes.clear();

        // Sort out the conflicts. New conflicts can just be added, although
        // care has to be taken to clear state - currently no attempt is
        // made to check solution validity for global conflicts.
        for (conf_i = new_conflicts.begin(); conf_i != new_conflicts.end(); conf_i++) {
            CYG_LOOP_INVARIANT_CLASSC(*conf_i);
            toplevel->conflicts.push_back(*conf_i);
            (*conf_i)->transaction = 0;
            (*conf_i)->clear_solution();
        }
        new_conflicts.clear();
        for (conf_i = new_structural_conflicts.begin(); conf_i != new_structural_conflicts.end(); conf_i++) {
            CYG_LOOP_INVARIANT_CLASSC(*conf_i);
            toplevel->structural_conflicts.push_back(*conf_i);
            (*conf_i)->transaction = 0;
            (*conf_i)->clear_solution();
        }
        new_structural_conflicts.clear();

        // Resolved conflicts can be either global or per-transaction
        // ones. If the former then the conflict will also be present
        // in deleted_conflicts and will get deleted shortly.
        for (conf_i2 = resolved_conflicts.begin(); conf_i2 != resolved_conflicts.end(); conf_i2++) {
            CYG_LOOP_INVARIANT_CLASSC(*conf_i2);
            if (0 != (*conf_i2)->transaction) {
                delete *conf_i2;
            }
        }
        resolved_conflicts.clear();

        // Now process conflicts that have gone away. These must actually
        // be deleted.
        for (conf_i2 = deleted_conflicts.begin(); conf_i2 != deleted_conflicts.end(); conf_i2++) {
            CYG_LOOP_INVARIANT_CLASSC(*conf_i2);
            std::list<CdlConflict>::iterator tmp;
            tmp = std::find(toplevel->conflicts.begin(), toplevel->conflicts.end(), *conf_i2);
            CYG_LOOP_INVARIANTC(tmp != toplevel->conflicts.end());
            toplevel->conflicts.erase(tmp);
            delete *conf_i2;
        }
        deleted_conflicts.clear();
        for (conf_i2 = deleted_structural_conflicts.begin(); conf_i2 != deleted_structural_conflicts.end(); conf_i2++) {
            CYG_LOOP_INVARIANT_CLASSC(*conf_i2);
            std::list<CdlConflict>::iterator tmp;
            tmp = std::find(toplevel->structural_conflicts.begin(), toplevel->structural_conflicts.end(), *conf_i2);
            CYG_LOOP_INVARIANTC(tmp != toplevel->structural_conflicts.end());
            toplevel->structural_conflicts.erase(tmp);
            delete *conf_i2;
        }
        deleted_structural_conflicts.clear();

        // Any global conflicts with solutions need to have their solutions cleared,
        // since currently no attempt is made to preserve their accuracy.
        for (conf_i = global_conflicts_with_solutions.begin(); conf_i != global_conflicts_with_solutions.end(); conf_i++) {
            CYG_LOOP_INVARIANT_CLASSC(*conf_i);
            (*conf_i)->clear_solution();
        }
        global_conflicts_with_solutions.clear();
        
        for (set_i = activated.begin(); set_i != activated.end(); set_i++) {
            (*set_i)->active = true;
        }
        for (set_i = deactivated.begin(); set_i != deactivated.end(); set_i++) {
            (*set_i)->active = false;
        }
        activated.clear();
        deactivated.clear();

        // Invoke all pending commit operations
        std::vector<CdlTransactionCommitCancelOp*>::iterator commit_i;
        for (commit_i = commit_cancel_ops.begin(); commit_i != commit_cancel_ops.end(); commit_i++) {
            (*commit_i)->commit(this);
            delete *commit_i;
            *commit_i = 0;
        }
        commit_cancel_ops.clear();

        // Finally take care of the callback.
        if (0 != callback_fn) {
            (*callback_fn)(all_changes);
        }
    }
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Solution support                         

// ----------------------------------------------------------------------------
// Saving a solution basically involves remembering what changes took place
// in the corresponding sub-transaction. There is no need to worry about
// other data in the sub-transaction such as conflicts, because there
// are no actual value changes.

void
CdlTransactionBody::save_solution()
{
    CYG_REPORT_FUNCNAME("CdlTransaction::save_solution");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    // Solutions should only be applicable to sub-transactions immediately
    // below the main transaction, since that is the only level at which
    // inference callbacks occur
    CYG_PRECONDITIONC((0 != parent) && (0 == parent->parent));

    CYG_PRECONDITION_CLASSC(conflict);
    CYG_PRECONDITIONC(0 == conflict->solution.size());
    
    std::map<CdlValuable, CdlValue>::const_iterator map_i;
    for (map_i = changes.begin(); map_i != changes.end(); map_i++) {
        // If the valuable was calculated or is otherwise non-modifiable,
        // there is no point in storing it with the solution since the
        // information is unlikely to be of interest to the user.
        CdlValuable valuable = map_i->first;
        if (valuable->is_modifiable()) {
            conflict->solution.push_back(*map_i);
        }
    }
    
    // save_solution() should operate like commit() or cancel(), i.e.
    // it leaves an empty sub-transaction. This sub-transaction cannot
    // actually be re-used at present because it still references a
    // conflict for which a solution is now already in place, but that
    // may get cleaned up in future.
    this->cancel();
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Can a solution be applied without e.g. overwriting a user value with
// an inferred value. There is a setting inference_override which controls
// this. Making a previously enabled option inactive also requires
// user confirmation, thus preventing the inference engine from disabling
// entire components.
bool
CdlTransactionBody::user_confirmation_required() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::user_confirmation_required", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(this->parent);

    bool result = false;
  
    std::map<CdlValuable, CdlValue>::const_iterator val_i;
    for (val_i = changes.begin(); val_i != changes.end(); val_i++) {
        const CdlValue& old_value = parent->get_whole_value(val_i->first);
        if (old_value.get_source() > CdlTransactionBody::inference_override) {
            result = true;
            break;
        }
    }
    std::set<CdlNode>::const_iterator val_j;
    for (val_j = deactivated.begin(); val_j != deactivated.end(); val_j++) {
        CdlValuable valuable = dynamic_cast<CdlValuable>(*val_j);
        if (0 != valuable) {
            const CdlValue& old_value = parent->get_whole_value(valuable);
            if ((old_value.get_source() > CdlTransactionBody::inference_override) && old_value.is_enabled()) {
                result = true;
                break;
            }
        }
    }
  
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// The inference engine is considering modifying a particular valuable. If
// the user has explicitly changed this valuable during the transaction then
// it would be inappropriate to suggest changing it again.
bool
CdlTransactionBody::changed_by_user(CdlValuable valuable) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::changed_by_user", "result %d");
    CYG_REPORT_FUNCARG2XV(this, valuable);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(valuable);
    
    bool result = false;
    std::map<CdlValuable, CdlValue>::const_iterator change_i = changes.find(valuable);
    if (change_i != changes.end()) {
        CdlValueSource source = change_i->second.get_source();
        if (CdlValueSource_User == source) {
            result = true;
        }
    }
    if (!result && (0 != parent)) {
        result = parent->changed_by_user(valuable);
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// A variant which takes into account the hierarchy: disabling a container
// when a sub-node has just been changed by the user is also a no-no.
bool
CdlTransactionBody::subnode_changed_by_user(CdlContainer container) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::subnode_changed_by_user", "result %d");
    CYG_REPORT_FUNCARG2XV(this, container);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(container);

    bool result = false;

    const std::vector<CdlNode>& contents = container->get_contents();
    std::vector<CdlNode>::const_iterator node_i;
    for (node_i = contents.begin(); node_i != contents.end(); node_i++) {
        CdlValuable valuable = dynamic_cast<CdlValuable>(*node_i);
        if ((0 != valuable) && this->changed_by_user(valuable)) {
            result = true;
            break;
        }
        CdlContainer container = dynamic_cast<CdlContainer>(*node_i);
        if ((0 != container) && this->subnode_changed_by_user(container)) {
            result = true;
            break;
        }
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Is one solution preferable to another? This code assumes that
// user_confirmation_required() and changed_by_user() have already
// been considered, so the only issue at stake here is the changes
// themselves.
//
// For now a simple metric of the number of changes is used. A more
// intelligent approach would take into account how much of the
// hierarchy is affected, e.g. how many other items would end
// up being disabled. Arguably the calling code should be able to
// supply an additional weighting.

bool
CdlTransactionBody::is_preferable_to(CdlTransaction other) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransactionBody::preferable_to", "result %d");
    CYG_REPORT_FUNCARG2XV(this, other);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(other);
    CYG_PRECONDITIONC(this != other);

    bool result = false;
    unsigned int this_changes  = this->changes.size()  + this->activated.size()  + this->deactivated.size();
    unsigned int other_changes = other->changes.size() + other->activated.size() + other->deactivated.size();
        
    if (this_changes <= other_changes) {
        result = true;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Applying solutions. Multiple solutions can be applied in one go. If there
// is any overlap, tough. Propagation needs to happen after solutions are
// applied.

void
CdlTransactionBody::apply_solution(CdlConflict conflict)
{
    CYG_REPORT_FUNCNAME("CdlTransaction::apply_solution");
    CYG_REPORT_FUNCARG2XV(this, conflict);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(conflict);

    // The solution can be for either a per-transaction conflict
    // or a global one. There are two lists to search.
    std::list<CdlConflict>::const_iterator conf_i;
    conf_i = std::find(this->new_conflicts.begin(), this->new_conflicts.end(), conflict);
    if (conf_i == this->new_conflicts.end()) {
        conf_i = std::find(this->global_conflicts_with_solutions.begin(),
                           this->global_conflicts_with_solutions.end(),
                           conflict);
        CYG_ASSERTC(conf_i != this->global_conflicts_with_solutions.end());
    }
    
    std::vector<std::pair<CdlValuable, CdlValue> >::const_iterator val_i;
    for (val_i = conflict->solution.begin(); val_i != conflict->solution.end(); val_i++) {
        CdlValuable valuable = val_i->first;
        CYG_LOOP_INVARIANT_CLASSC(valuable);

        const CdlValue& old_value = this->get_whole_value(valuable);
        this->set_whole_value(valuable, old_value, val_i->second);
    }

    CYG_REPORT_RETURN();
}

void
CdlTransactionBody::apply_solutions(const std::vector<CdlConflict>& solutions)
{
    CYG_REPORT_FUNCNAME("CdlTransaction::apply_solutions");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::vector<CdlConflict>::const_iterator conf_i;
    for (conf_i = solutions.begin(); conf_i != solutions.end(); conf_i++) {
        
        std::list<CdlConflict>::const_iterator conf_j;
        conf_j = std::find(this->new_conflicts.begin(), this->new_conflicts.end(), conflict);
        if (conf_j == this->new_conflicts.end()) {
            conf_j = std::find(this->global_conflicts_with_solutions.begin(),
                               this->global_conflicts_with_solutions.end(),
                               conflict);
            CYG_ASSERTC(conf_j != this->global_conflicts_with_solutions.end());
        }

        std::vector<std::pair<CdlValuable, CdlValue> >::const_iterator val_i;
        for (val_i = conflict->solution.begin(); val_i != conflict->solution.end(); val_i++) {
            CdlValuable valuable = val_i->first;
            CYG_LOOP_INVARIANT_CLASSC(valuable);
            const CdlValue& old_value = this->get_whole_value(valuable);
            this->set_whole_value(valuable, old_value, val_i->second);
        }
    }

    CYG_REPORT_RETURN();
}

void
CdlTransactionBody::apply_all_solutions()
{
    CYG_REPORT_FUNCNAME("CdlTransaction::apply_all_solutions");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::list<CdlConflict>::const_iterator conf_i;
    for (conf_i = this->new_conflicts.begin(); conf_i != this->new_conflicts.end(); conf_i++) {
        if ((*conf_i)->has_known_solution()) {

            std::vector<std::pair<CdlValuable, CdlValue> >::const_iterator val_i;
            for (val_i = conflict->solution.begin(); val_i != conflict->solution.end(); val_i++) {
                CdlValuable valuable = val_i->first;
                CYG_LOOP_INVARIANT_CLASSC(valuable);
                const CdlValue& old_value = this->get_whole_value(valuable);
                this->set_whole_value(valuable, old_value, val_i->second);
            }
        }
    }
    for (conf_i = this->global_conflicts_with_solutions.begin();
         conf_i != this->global_conflicts_with_solutions.end();
         conf_i++) {

        CYG_ASSERTC((*conf_i)->has_known_solution());
        
        std::vector<std::pair<CdlValuable, CdlValue> >::const_iterator val_i;
        for (val_i = conflict->solution.begin(); val_i != conflict->solution.end(); val_i++) {
            CdlValuable valuable = val_i->first;
            CYG_LOOP_INVARIANT_CLASSC(valuable);
            const CdlValue& old_value = this->get_whole_value(valuable);
            this->set_whole_value(valuable, old_value, val_i->second);
        }
    }

    CYG_REPORT_RETURN();
}

//}}}
//{{{  Inference                                

// ----------------------------------------------------------------------------
//{{{  resolve() - all per-transaction conflicts        

void
CdlTransactionBody::resolve(int level)
{
    CYG_REPORT_FUNCNAME("CdlTransaction::resolve");
    CYG_REPORT_FUNCARG2XV(this, level);
    CYG_PRECONDITION_THISC();

    while(1) {
        // Resolving one conflict may affect others, so iterating down the list
        // is not safe. Instead we need to loop as long as there are conflicts
        // to be considered.
        std::list<CdlConflict>::iterator conf_i;
        for (conf_i = new_conflicts.begin(); conf_i != new_conflicts.end(); conf_i++) {
            CYG_LOOP_INVARIANT_CLASSC(*conf_i);

            // Is there any point in attempt to resolve this conflict?
            if ((*conf_i)->has_known_solution() ||
                (*conf_i)->has_no_solution()    ||
                !(*conf_i)->resolution_implemented()) {
                continue;
            }
            this->resolve(*conf_i, level);
            break;
        }
        if (conf_i == new_conflicts.end()) {
            break;
        }
    }
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  resolve() - vector                               

// ----------------------------------------------------------------------------
void
CdlTransactionBody::resolve(const std::vector<CdlConflict>& conflicts, int level)
{
    CYG_REPORT_FUNCNAME("CdlTransaction::resolve");
    CYG_REPORT_FUNCARG2XV(this, level);
    CYG_PRECONDITION_THISC();

    std::vector<CdlConflict>::const_iterator conf_i;
    for (conf_i = conflicts.begin(); conf_i != conflicts.end(); conf_i++) {
        CYG_LOOP_INVARIANT_CLASSC(*conf_i);

        // Is there any point in attempt to resolve this conflict?
        if (!(*conf_i)->has_known_solution() &&
            !(*conf_i)->has_no_solution()    &&
            (*conf_i)->resolution_implemented()) {
            this->resolve(*conf_i, level);
        }
    }
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  resolve() - single conflict                      

// ----------------------------------------------------------------------------
// There is a conflict that may have a solution. The resolution
// attempt needs to happen in the context of a sub-transaction
//
// The conflict may have been created during this transaction,
// or it may be a global conflict left over from a previous
// transaction. This can be detected using the conflict's
// transaction field. The commit() code, amongst others, needs
// to handle global and per-transaction conflicts differently.

void
CdlTransactionBody::resolve(CdlConflict conflict, int level)
{
    CYG_REPORT_FUNCNAME("CdlTransaction::resolve");
    CYG_REPORT_FUNCARG3XV(this, conflict, level);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(conflict);
    CYG_PRECONDITIONC(0 == conflict->solution.size());
    CYG_PRECONDITIONC((0 <= level) && (level <= inference_recursion_limit));

    CdlTransaction sub_transaction = this->make(conflict);
    CYG_PRECONDITION_CLASSC(sub_transaction);
    if (!conflict->inner_resolve(sub_transaction, level)) {
        CYG_ASSERTC(0 == sub_transaction->changes.size());
        sub_transaction->cancel();
        delete sub_transaction;
        conflict->no_solution = true;
        CYG_REPORT_RETURN();
        return;
    }
    // Is the inference engine lying? The conflict should be resolved
    // in the sub-transaction.
    if (conflict->is_structural()) {
        if (std::find(sub_transaction->deleted_structural_conflicts.begin(),
                      sub_transaction->deleted_structural_conflicts.end(),
                      conflict) == sub_transaction->deleted_structural_conflicts.end()) {
 
            CYG_FAIL("The inference engine has proved too optimistic.");
            sub_transaction->cancel();
            delete sub_transaction;
            conflict->no_solution = true;
            CYG_REPORT_RETURN();
            return;
        }
    } else {
        if (std::find(sub_transaction->deleted_conflicts.begin(), sub_transaction->deleted_conflicts.end(), conflict)
            == sub_transaction->deleted_conflicts.end()) {
                    
            CYG_FAIL("The inference engine has proved too optimistic.");
            sub_transaction->cancel();
            delete sub_transaction;
            conflict->no_solution = true;
            CYG_REPORT_RETURN();
            return;
        }
    }
            
    // Even if there is a solution it cannot always be applied
    // automatically because that would affect existing user
    // values. Instead the solution needs to be saved so that
    // the user can inspect it later. This should only happen
    // at level 0. If we have recursed into the inference
    // engine then we should only worry about this right at
    // the end, not at every stage (although internally the
    // inference code may worry about this when choosing
    // between alternatives).
    if ((0 == level) && sub_transaction->user_confirmation_required()) {
        sub_transaction->save_solution();
        sub_transaction->cancel();
        delete sub_transaction;
        this->dirty = true;

        if (0 == conflict->transaction) {
            // This is a global conflict, not a per-transaction one.
            // There is a separate list of these conflicts.
            std::list<CdlConflict>::const_iterator conf_i;
            conf_i = std::find(this->global_conflicts_with_solutions.begin(),
                               this->global_conflicts_with_solutions.end(),
                               conflict);
            if (conf_i == this->global_conflicts_with_solutions.end()) {
                this->global_conflicts_with_solutions.push_back(conflict);
            }
        }
    } else {
            
        // This sub-transaction is safe, it can be applied
        // immediately. The commit code detects that the
        // solution being committed is for a particular
        // resolved conflict and will take care of moving that
        // conflict to the resolved list.
        conflict->solution_references.clear();     // No point in preserving this information
        conflict->no_solution = false;             // Redundant
        std::map<CdlValuable, CdlValue>::const_iterator soln_i;
        for (soln_i = sub_transaction->changes.begin(); soln_i != sub_transaction->changes.end(); soln_i++) {
            conflict->solution.push_back(*soln_i);
        }
        sub_transaction->commit();
        delete sub_transaction;
    }

    CYG_REPORT_RETURN();
}

//}}}
//{{{  resolve_recursion()                              

// ----------------------------------------------------------------------------
// resolve_recursion()
//
// The inference engine has tried one or more changes in the context of
// a sub-transaction. It is now necessary to check whether these changes
// are beneficial, i.e. whether or not any new problems are introduced
// that cannot be resolved.
bool
CdlTransactionBody::resolve_recursion(int level)
{
    CYG_REPORT_FUNCNAMETYPE("CdlTransaction::resolve_recursion", "result %d");
    CYG_REPORT_FUNCARG2XV(this, level);
    CYG_PRECONDITION_THISC();

    bool result = false;
    this->propagate();
    if (0 == new_conflicts.size()) {
        result = true;
        CYG_REPORT_RETVAL(result);
        return result;
    }
    if (level >= inference_recursion_limit) {
        result = false;
        CYG_REPORT_RETVAL(result);
        return result;
    }

    // There are new conflicts, but it may be possible to resolve them
    // by a recursive invocation of the inference engine.
    bool solutions_possible = false;
    do {
        this->resolve(level + 1);
        std::list<CdlConflict>::const_iterator conf_i;
        solutions_possible = false;
        for (conf_i = this->new_conflicts.begin(); conf_i != this->new_conflicts.end(); conf_i++) {
            if (!(*conf_i)->has_no_solution()) {
                solutions_possible = true;
            }
        }
    } while(solutions_possible);

    result = (0 == new_conflicts.size());
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}

//}}}
//{{{  Body                                     

// ----------------------------------------------------------------------------
// The majority of transactions involve the same set of steps. First one
// or more values are modified. Then there has to be propagation, inference,
// an inference callback, ... There may be a number of iterations. It is
// convenient to have a single transaction body function which takes care
// of all of that.
//
// If automatic inference is disabled then life is pretty simple, there
// should be one propagate() operation followed by a commit.
//
// If automatic inference is enabled but there is no inference callback
// then we need a loop consisting of propagation and inference, while
// progress is made. Progress can be detected by value changes.
//
// If there is an inference callback then life gets pretty complicated.
// The problem is figuring out exactly when the inference callback
// should be invoked:
//
// 1) any new conflicts should certainly result in a callback, to give
//    the user a chance to cancel the changes.
// 2) any new solutions that have been applied automatically need to
//    be shown to the user, again so that it is possible to cancel
//    the changes.
// 3) any existing conflicts with a new solution, albeit one that cannot
//    be applied automatically, should result in a callback. This is
//    somewhat problematical since the new solution may in fact be
//    identical to a previous one that the user has already decided
//    against committing.
//
// It is not easy to keep track of when new conflicts or solutions get
// added to a transaction. Simply counting the entries in the
// appropriate STL containers is insufficient, as conflicts come and
// go. Instead it is necessary to have a "dirty" flag. Unfortunately
// this too is not fool-proof: a new conflict may have been created,
// resulting in the dirty flag being set, and then the conflict may
// have disappeared.

void
CdlTransactionBody::body()
{
    CYG_REPORT_FUNCNAME("CdlTransaction::body");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    // The Body() member function can only be applied to a toplevel
    // transaction, it does not really make sense to apply it to
    // a sub-transaction (at least, not yet);
    CYG_PRECONDITIONC((0 == parent) && (0 != toplevel));

    if (!inference_enabled) {
        this->propagate();
        this->commit();
        CYG_REPORT_RETURN();
        return;
    }

    if (0 == inference_callback) {
        bool progress = false;
        do {
            progress = false;
            this->propagate();
            CYG_LOOP_INVARIANTC(0 == value_changes.size());
            this->resolve();
            if (0 != value_changes.size()) {
                progress = true;
            }
        } while(progress);

        this->commit();
        CYG_REPORT_RETURN();
        return;
    }

    bool cancel = false;
    
    unsigned int resolved_size = 0;
    unsigned int globals_with_solutions_size = 0;
    
    do {
        bool progress = false;
        do {
            progress = false;
            this->propagate();
            CYG_LOOP_INVARIANTC(0 == value_changes.size());
            this->resolve();
            if (0 != value_changes.size()) {
                progress = true;
            }
        } while(progress);

        // Sanity check: if there are no conflicts and no new entries in
        // the resolved vector, then stop here. The user has already seen
        // and taken care of everything of interest.
        if ((0 == new_conflicts.size()) &&
            (resolved_size == resolved_conflicts.size()) &&
            (globals_with_solutions_size == global_conflicts_with_solutions.size())) {
            cancel = false;
            break;
        }

        // Also, if no conflicts have been added, no new solutions
        // have been identified, and no new solutions have been applied,
        // then there is no point in asking for user feedback.
        if (!this->dirty) {
            cancel = false;
            break;
        }
        
        // Clear state before invoking the callback. If the user does not
        // change anything else then we should get out of the loop next
        // time around.
        this->dirty = false;
        resolved_size = resolved_conflicts.size();
        globals_with_solutions_size = global_conflicts_with_solutions.size();

        // Invoke the callback. If the result is cancel, do so. Otherwise
        // we need to spin while things are changing.
        if (CdlInferenceCallbackResult_Cancel == (*inference_callback)(this)) {
            cancel = true;
        }
    } while(!cancel);

    if (cancel) {
        this->cancel();
    } else {
        this->commit();
    }
    
    CYG_REPORT_RETURN();
}

//}}}

