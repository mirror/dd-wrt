//{{{  Banner                           

//============================================================================
//
//      conflict.cxx
//
//      The CdlConflict class
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
// Date:        1999/01/28
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
// the class definitions rely on these headers. It also brings
// in <tcl.h>
#include <cdlcore.hxx>

//}}}

//{{{  Statics                          

// ----------------------------------------------------------------------------
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlConflictBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlConflict_UnresolvedBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlConflict_IllegalValueBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlConflict_EvalExceptionBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlConflict_RequiresBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlConflict_DataBody);

//}}}
//{{{  CdlConflict                      

//{{{  Creation and destruction                 

// ----------------------------------------------------------------------------
// The basic conflicts. Conflicts are created in the context of a transaction.
// If the transaction gets committed then they are transferred to a toplevel.
// If the transaction gets cancelled then they just disappear.
//
// A conflict that is only part of a transaction may go away as that
// transaction proceeds, in which case it can be deleted immediately.
// A conflict that is already part of the toplevel cannot be
// destroyed, instead it gets marked in the transaction object. Only
// when the transaction is committed does the object get deleted.
// In addition within the context of a transaction old conflicts
// may hang around for a while.
//

CdlConflictBody::CdlConflictBody(CdlTransaction trans_arg, CdlNode node_arg, CdlProperty property_arg, bool structural_arg)
{
    CYG_REPORT_FUNCNAME("CdlConflict:: constructor");
    CYG_REPORT_FUNCARG4XV(this, trans_arg, node_arg, property_arg);
    CYG_PRECONDITION_CLASSC(trans_arg);
    CYG_PRECONDITION_CLASSC(node_arg);
    CYG_PRECONDITION_CLASSC(property_arg);

    node        = node_arg;
    property    = property_arg;
    transaction = trans_arg;
    structural  = structural_arg;
    no_solution = false;
    // The vectors take care of themselves
    enabled     = true;
    reason      = "";

    transaction->dirty = true;
    if (structural_arg) {
        transaction->new_structural_conflicts.push_back(this);
    } else {
        transaction->new_conflicts.push_back(this);
    }
    
    cdlconflictbody_cookie      = CdlConflictBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();

    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}


// ----------------------------------------------------------------------------
// The destructor can get invoked during a transaction commit, in the
// case of a global conflict, or from inside clear() for a per-transaction
// conflict. In all cases the calling code is responsible for removing
// the conflict from any STL containers.

CdlConflictBody::~CdlConflictBody()
{
    CYG_REPORT_FUNCNAME("CdlConflict:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdlconflictbody_cookie      = CdlConflictBody_Invalid;
    reason                      = "";
    enabled                     = false;
    no_solution                 = false;
    solution.clear();
    solution_references.clear();
    structural                  = false;
    transaction                 = 0;
    property                    = 0;
    node                        = 0;
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Solution support                         

// ----------------------------------------------------------------------------
bool
CdlConflictBody::has_known_solution() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlConflict::has_solution", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = (0 != solution.size());
    
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlConflictBody::has_no_solution() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlConflict::has_no_solution", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = no_solution;
    
    CYG_REPORT_RETVAL(result);
    return result;
}

const std::vector<std::pair<CdlValuable, CdlValue> >&
CdlConflictBody::get_solution() const
{
    CYG_REPORT_FUNCNAME("CdlConflict::get_solution");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return solution;
}

const std::set<CdlValuable>&
CdlConflictBody::get_solution_references() const
{
    CYG_REPORT_FUNCNAME("CdlConflict::get_solution_references");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return solution_references;
}

// ----------------------------------------------------------------------------
// Try to resolve a conflict. If the conflict was created in a transaction,
// use that transaction. More commonly the conflict will be global and
// a new transaction will have to be created specially for it. Either
// way the conflict may cease to exist.
void
CdlConflictBody::resolve()
{
    CYG_REPORT_FUNCNAME("CdlConflict::resolve");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC(0 == transaction);

    if (this->resolution_implemented()) {
        CdlTransaction transact = CdlTransactionBody::make(this->get_node()->get_toplevel());
        transact->resolve(this);
        transact->body();
        delete transact;
    }
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// A valuable has just been changed. If this value was relevant to the
// current solution (or lack thereof) then an update is necessary.
void
CdlConflictBody::update_solution_validity(CdlValuable valuable)
{
    CYG_REPORT_FUNCNAME("CdlConflict::update_solution_validity");
    CYG_REPORT_FUNCARG2XV(this, valuable);
    CYG_PRECONDITION_THISC();

    if (solution_references.find(valuable) != solution_references.end()) {
        no_solution = false;
        solution.clear();
        solution_references.clear();
    }

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Default implementations of the inference engine do not do a lot...
bool
CdlConflictBody::inner_resolve(CdlTransaction trans_arg, int level)
{
    CYG_REPORT_FUNCNAMETYPE("CdlConflict::inner_resolve", "result false");
    CYG_REPORT_FUNCARG3XV(this, trans_arg, level);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(trans_arg);

    // Setting the no_solution flag while keeping a clear
    // solution_accessed vector means that the no_solution flag should
    // always remain set, and hence no further inference attempts will be made.
    no_solution = true;
    
    CYG_REPORT_RETURN();
    return false;
}

bool
CdlConflictBody::resolution_implemented() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlConflict::resolution_implemented", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETVAL(false);
    return false;
}

// ----------------------------------------------------------------------------
// Clearing a solution. This is needed if the inference engine has
// failed to find a complete solution, because in attempting this the
// solution_references vector will have been filled in anyway. It may
// have some other uses as well.
void
CdlConflictBody::clear_solution()
{
    CYG_REPORT_FUNCNAME("CdlConflict::clear_solution");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    no_solution = false;
    solution.clear();
    solution_references.clear();

    CYG_REPORT_RETURN();
}

//}}}
//{{{  Basics                                   

// ----------------------------------------------------------------------------

CdlNode
CdlConflictBody::get_node() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlConflict::get_node", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlNode result = node;
    CYG_REPORT_RETVAL(result);
    return result;
}

CdlProperty
CdlConflictBody::get_property() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlConflict::get_property", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlProperty result  = property;
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlConflictBody::is_structural() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlConflict::is_structural", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = structural;
    CYG_REPORT_RETVAL(result);
    return result;
}

CdlTransaction
CdlConflictBody::get_transaction() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlConflict::get_transaction", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlTransaction result = transaction;
    CYG_REPORT_RETVAL(result);
    return result;
}

// FIXME: these are not currently implemented. It would be necessary
// to store the information in the savefile, which requires an
// unambiguous way of identifying a conflict that is likely to
// survice package version changes.

bool
CdlConflictBody::is_enabled() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlConflict::is_enabled", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = enabled;
    CYG_REPORT_RETVAL(result);
    return result;
}

std::string
CdlConflictBody::get_disabled_reason() const
{
    CYG_REPORT_FUNCNAME("CdlConflict::get_disabled_reason");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    // Possibly there should be a check that the conflict is currently
    // disabled, but it might be useful. 
    CYG_REPORT_RETURN();
    return reason;
}

void
CdlConflictBody::disable(std::string reason_arg)
{
    CYG_REPORT_FUNCNAME("CdlConflict::disable");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    reason      = reason_arg;
    enabled     = false;

    CYG_REPORT_RETURN();
}

void
CdlConflictBody::enable()
{
    CYG_REPORT_FUNCNAME("CdlConflict::enable");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    enabled     = true;
    // Leave "reason" alone, it may still be useful
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  check_this()                             

// ----------------------------------------------------------------------------
bool
CdlConflictBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlConflictBody_Magic != cdlconflictbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    if ((0 == node) || (0 == property)) {
        return false;
    }
    switch(zeal) {
      case cyg_system_test :
      case cyg_extreme :
      {
          if (!node->check_this(cyg_quick)) {
              return false;
          }
          // Accessing the properties would involve a function call.
          
          if (0 != transaction) {
              if (!transaction->check_this(cyg_quick)) {
                  return false;
              }
              if (structural) {
                  // The conflict should exist in the new_structural_conflicts vector
                  // deleted_structural_conflicts is for toplevel ones.
                  if (std::find(transaction->new_structural_conflicts.begin(),
                                transaction->new_structural_conflicts.end(), this) ==
                      transaction->new_structural_conflicts.end()) {
                      return false;
                  }
              } else {
                  // The conflict may appear on the new_conflicts list
                  // or in the resolved_conflicts vector.
                  if (std::find(transaction->new_conflicts.begin(), transaction->new_conflicts.end(), this) ==
                      transaction->new_conflicts.end()) {

                      if (std::find(transaction->resolved_conflicts.begin(), transaction->resolved_conflicts.end(), this) ==
                          transaction->resolved_conflicts.end()) {

                          return false;
                      }
                  }
              }
          }
          // Checking the toplevel lists would be good, but involves a
          // further function call and hence nested assertions.
      }
      case cyg_thorough :
      {
          if (!node->check_this(cyg_quick)) {
              return false;
          }
          if (!property->check_this(cyg_quick)) {
              return false;
          }
      }
      case cyg_quick :
      {
          if (no_solution && (0 != solution.size())) {
              return false;
          }
      }
      case cyg_trivial :
      case cyg_none :
      default:
          break;
    }
    
    return true;
}

//}}}

//}}}
//{{{  CdlConflict_Unresolved           

// ----------------------------------------------------------------------------
// Unresolved references. Usually unresolved references occur inside
// expressions, but other properties that may be affected are parent,
// dialog and wizard.
//
// It is possible for a single expression to have multiple unresolved
// references, each of which will result in a separate conflict
// object. It is also possible for an expression to refer to the same
// unknown entity twice or more, in which case there would also be
// separate conflict objects. Unresolved references may also result in
// eval exceptions and in failed requires statements. Trying to cope
// with the various combinations as a single conflict seems too
// error-prone.

void
CdlConflict_UnresolvedBody::make(CdlTransaction trans, CdlNode node_arg, CdlProperty prop_arg, std::string name_arg)
{
    CdlConflict_Unresolved tmp = new CdlConflict_UnresolvedBody(trans, node_arg, prop_arg, name_arg);
    CYG_UNUSED_PARAM(CdlConflict_Unresolved, tmp);
}

CdlConflict_UnresolvedBody::CdlConflict_UnresolvedBody(CdlTransaction trans_arg, CdlNode node_arg, CdlProperty prop_arg,
                                                       std::string target_name_arg)
    : CdlConflictBody(trans_arg, node_arg, prop_arg, true)
{
    CYG_REPORT_FUNCNAME("CdlConflict_Unresolved:: constructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITIONC("" != target_name_arg);

    target_name = target_name_arg;
    cdlconflict_unresolvedbody_cookie = CdlConflict_UnresolvedBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlConflict_UnresolvedBody::~CdlConflict_UnresolvedBody()
{
    CYG_REPORT_FUNCNAME("CdlConflict_Unresolved:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdlconflict_unresolvedbody_cookie = CdlConflict_UnresolvedBody_Invalid;
    target_name = "";
    CYGDBG_MEMLEAK_DESTRUCTOR();

    CYG_REPORT_RETURN();
}

std::string
CdlConflict_UnresolvedBody::get_target_name() const
{
    CYG_REPORT_FUNCNAME("CdlConflict_Unresolved::get_target_name");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return target_name;
}

// For now, just report the node and property name, a brief text, and the
// entity being referenced. 
//
// Eventually we can do clever things like looking up the name in a
// database.

std::string
CdlConflict_UnresolvedBody::get_explanation() const
{
    CYG_REPORT_FUNCNAME("CdlConflict_Unresolved::get_explanation");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::string result;
    result = node->get_name() + ", property " + property->get_property_name() +
        ":\nReference to unknown object " + target_name;

    CYG_REPORT_RETURN();
    return result;
}

bool
CdlConflict_UnresolvedBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlConflict_UnresolvedBody_Magic != cdlconflict_unresolvedbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();

    // There is not a lot of checking that can be done on the name.
    
    return CdlConflictBody::check_this(zeal);
}

bool
CdlConflict_UnresolvedBody::test(CdlConflict conf)
{
    CYG_REPORT_FUNCNAMETYPE("CdlConflict_Unresolved::test", "result %d");
    CYG_REPORT_FUNCARG1XV(conf);
    CYG_PRECONDITION_CLASSC(conf);

    bool result = false;
    CdlConflict_Unresolved tmp = dynamic_cast<CdlConflict_Unresolved>(conf);
    if (0 != tmp) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  CdlConflict_IllegalValue         

// ----------------------------------------------------------------------------
// A valuable object has an illegal value. This can happen because the
// current value is not within the legal_values list, or because of a
// failure of check_proc or entry_proc. In the latter two cases the
// Tcl code should supply an explanation as to why the value is illegal.
//
// Note: if future variants of CDL implement some concept of a value
// that does not match the CdlValuable class then that will need a
// separate CdlConflict derived class.

void
CdlConflict_IllegalValueBody::make(CdlTransaction trans, CdlNode node_arg, CdlProperty prop_arg)
{
    CdlConflict_IllegalValue tmp = new CdlConflict_IllegalValueBody(trans, node_arg, prop_arg);
    CYG_UNUSED_PARAM(CdlConflict_IllegalValue, tmp);
}

CdlConflict_IllegalValueBody::CdlConflict_IllegalValueBody(CdlTransaction trans, CdlNode node_arg, CdlProperty prop_arg)
    : CdlConflictBody(trans, node_arg, prop_arg, false)
{
    CYG_REPORT_FUNCNAME("CdlConflict_IllegalValue:: constructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITIONC(0 != dynamic_cast<CdlValuable>(node_arg));

    explanation = "";
    cdlconflict_illegalvaluebody_cookie = CdlConflict_IllegalValueBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();

    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlConflict_IllegalValueBody::~CdlConflict_IllegalValueBody()
{
    CYG_REPORT_FUNCNAME("CdlConflict_IllegalValue:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdlconflict_illegalvaluebody_cookie = CdlConflict_IllegalValueBody_Invalid;
    explanation = "";
    CYGDBG_MEMLEAK_DESTRUCTOR();

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// If the property is legal_values then it should be a list expression
// property. Display the current value and the original expression.
//
// If the property is check_proc or entry_proc, the Tcl code should
// have produced an explanation string and stored it in the conflict
// object.
std::string
CdlConflict_IllegalValueBody::get_explanation() const
{
    CYG_REPORT_FUNCNAME("CdlConflict_IllegalValue::get_explanation()");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlConstValuable valuable = dynamic_cast<CdlConstValuable>(node);
    CYG_ASSERTC(0 != valuable);

    std::string result = "";

    // FIXME: analyse the current value and flavor a bit more
    result += "Illegal current value " + valuable->get_value() + "\n";

    if (CdlPropertyId_LegalValues == property->get_property_name()) {
        
        CdlConstProperty_ListExpression expr = dynamic_cast<CdlConstProperty_ListExpression>(property);
        CYG_ASSERTC(0 != expr);
        result += "Legal values are: " + expr->get_original_string();
        
    } else if (explanation != "") {
        
        result += explanation;
    } else {

        CYG_FAIL("Inexplicable illegal value");
    }

    CYG_REPORT_RETURN();
    return result;
}

void
CdlConflict_IllegalValueBody::set_explanation(std::string explanation_arg)
{
    CYG_REPORT_FUNCNAME("CdlConflict_IllegalValue::set_explanation");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    explanation = explanation_arg;

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Inference is implemented. The actual inference code is in infer.cxx
bool
CdlConflict_IllegalValueBody::resolution_implemented() const
{
    CYG_REPORT_FUNCNAME("CdlConflict_IllegalValue::resolution_implemented");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return true;
}

// ----------------------------------------------------------------------------
bool
CdlConflict_IllegalValueBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlConflict_IllegalValueBody_Magic != cdlconflict_illegalvaluebody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();

    return CdlConflictBody::check_this(zeal);
}

bool
CdlConflict_IllegalValueBody::test(CdlConflict conf)
{
    CYG_REPORT_FUNCNAMETYPE("CdlConflict_IllegalValue::test", "result %d");
    CYG_REPORT_FUNCARG1XV(conf);
    CYG_PRECONDITION_CLASSC(conf);

    bool result = false;
    CdlConflict_IllegalValue tmp = dynamic_cast<CdlConflict_IllegalValue>(conf);
    if (0 != tmp) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  CdlConflict_EvalException        

// ----------------------------------------------------------------------------
// Run-time expression failures are possible because of division by
// zero, if for no other reason. The evaluation code should have
// stored a suitable diagnostic with the conflict. This is not part
// of the constructor, allowing the conflict object to be re-used
// if the detailed reason changes

void
CdlConflict_EvalExceptionBody::make(CdlTransaction trans, CdlNode node_arg, CdlProperty prop_arg, std::string msg_arg)
{
    CdlConflict_EvalException tmp = new CdlConflict_EvalExceptionBody(trans, node_arg, prop_arg, msg_arg);
    CYG_UNUSED_PARAM(CdlConflict_EvalException, tmp);
}

CdlConflict_EvalExceptionBody::CdlConflict_EvalExceptionBody(CdlTransaction trans, CdlNode node_arg, CdlProperty prop_arg,
                                                             std::string msg_arg)
    : CdlConflictBody(trans, node_arg, prop_arg, false)
{
    CYG_REPORT_FUNCNAME("CdlConflict_EvalException");
    CYG_REPORT_FUNCARG1XV(this);

    explanation = msg_arg;
    cdlconflict_evalexceptionbody_cookie = CdlConflict_EvalExceptionBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();

    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlConflict_EvalExceptionBody::~CdlConflict_EvalExceptionBody()
{
    CYG_REPORT_FUNCNAME("CdlConflict_EvalException");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdlconflict_evalexceptionbody_cookie = CdlConflict_EvalExceptionBody_Invalid;
    explanation = "";
    CYGDBG_MEMLEAK_DESTRUCTOR();

    CYG_REPORT_RETURN();
}

// If there has been an eval exception then the property must be an
// ordinary expression, a list expression, or a goal expression
std::string
CdlConflict_EvalExceptionBody::get_explanation() const
{
    CYG_REPORT_FUNCNAME("CdlConflict_EvalException::get_explanation");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::string result = node->get_name() + ", property " + property->get_property_name() + "\n";
    result += "Error while evaluation expression: ";
    if ("" != explanation) {
        result += explanation;
    }
    result += "\n";

    if (CdlConstProperty_Expression expr = dynamic_cast<CdlConstProperty_Expression>(property)) {
        
        result += "Expression: " + expr->get_original_string();
        
    } else if (CdlConstProperty_ListExpression lexpr = dynamic_cast<CdlConstProperty_ListExpression>(property)) {

        result += "List expression: " + lexpr->get_original_string();
        
    } else if (CdlConstProperty_GoalExpression gexpr = dynamic_cast<CdlConstProperty_GoalExpression>(property)) {

        result += "Goal expression: " + gexpr->get_original_string();
        
    } else {
        CYG_FAIL("Unknown expression type");
    }

    CYG_REPORT_RETURN();
    return result;
}

void
CdlConflict_EvalExceptionBody::set_explanation(std::string explanation_arg)
{
    CYG_REPORT_FUNCNAME("CdlConflict_EvalException::set_explanation");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    explanation = explanation_arg;

    CYG_REPORT_RETURN();
}

bool
CdlConflict_EvalExceptionBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlConflict_EvalExceptionBody_Magic != cdlconflict_evalexceptionbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return CdlConflictBody::check_this(zeal);
}

bool
CdlConflict_EvalExceptionBody::test(CdlConflict conf)
{
    CYG_REPORT_FUNCNAMETYPE("CdlConflict_EvalException::test", "result %d");
    CYG_REPORT_FUNCARG1XV(conf);
    CYG_PRECONDITION_CLASSC(conf);

    bool result = false;
    CdlConflict_EvalException tmp = dynamic_cast<CdlConflict_EvalException>(conf);
    if (0 != tmp) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  CdlConflict_Requires             

// ----------------------------------------------------------------------------
// A requires constraint is not satisfied. Producing a decent diagnostic
// here requires a detailed understanding of goal expressions. For now
// there is no extra data associated with goal expressions.

void
CdlConflict_RequiresBody::make(CdlTransaction trans, CdlNode node_arg, CdlProperty prop_arg)
{
    CdlConflict_Requires tmp = new CdlConflict_RequiresBody(trans, node_arg, prop_arg);
    CYG_UNUSED_PARAM(CdlConflict_Requires, tmp);
}

CdlConflict_RequiresBody::CdlConflict_RequiresBody(CdlTransaction trans, CdlNode node_arg, CdlProperty prop_arg)
    : CdlConflictBody(trans, node_arg, prop_arg, false)
{
    CYG_REPORT_FUNCNAME("CdlConflict_Requires:: constructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITIONC(0 != dynamic_cast<CdlProperty_GoalExpression>(prop_arg));

    cdlconflict_requiresbody_cookie = CdlConflict_RequiresBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();

    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlConflict_RequiresBody::~CdlConflict_RequiresBody()
{
    CYG_REPORT_FUNCNAME("CdlConflict_Requires:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdlconflict_requiresbody_cookie = CdlConflict_RequiresBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();

    CYG_REPORT_RETURN();
}

// FIXME: implement properly
std::string
CdlConflict_RequiresBody::get_explanation() const
{
    CYG_REPORT_FUNCNAME("CdlConflict::get_explanation");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlConstProperty_GoalExpression gexpr = dynamic_cast<CdlConstProperty_GoalExpression>(property);
    CYG_ASSERTC(0 != gexpr);
    
    std::string result = "";
    result += "\"requires\" constraint not satisfied: " + gexpr->get_original_string();

    CYG_REPORT_RETURN();
    return result;
}

// Inference is implemented, see infer.cxx
bool
CdlConflict_RequiresBody::resolution_implemented() const
{
    CYG_REPORT_FUNCNAME("CdlConflict_Requires");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return true;
}

bool
CdlConflict_RequiresBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlConflict_RequiresBody_Magic != cdlconflict_requiresbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return CdlConflictBody::check_this(zeal);
}

bool
CdlConflict_RequiresBody::test(CdlConflict conf)
{
    CYG_REPORT_FUNCNAMETYPE("CdlConflict_Requires::test", "result %d");
    CYG_REPORT_FUNCARG1XV(conf);
    CYG_PRECONDITION_CLASSC(conf);

    bool result = false;
    CdlConflict_Requires tmp = dynamic_cast<CdlConflict_Requires>(conf);
    if (0 != tmp) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  CdlConflict_Data                 

// ----------------------------------------------------------------------------
// There is some strange problem in the configuration data, for example
// a parent property that is resolved but the target is not a container.

void
CdlConflict_DataBody::make(CdlTransaction trans, CdlNode node_arg, CdlProperty prop_arg, std::string message_arg)
{
    CdlConflict_Data tmp = new CdlConflict_DataBody(trans, node_arg, prop_arg, message_arg);
    CYG_UNUSED_PARAM(CdlConflict_Data, tmp);
}

CdlConflict_DataBody::CdlConflict_DataBody(CdlTransaction trans, CdlNode node_arg, CdlProperty prop_arg,
                                           std::string message_arg)
    : CdlConflictBody(trans, node_arg, prop_arg, true)
{
    CYG_REPORT_FUNCNAME("CdlConflict_Data:: constructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITIONC("" != message_arg);

    message = message_arg;
    cdlconflict_databody_cookie = CdlConflict_DataBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();

    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlConflict_DataBody::~CdlConflict_DataBody()
{
    CYG_REPORT_FUNCNAME("CdlConflict_ata: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdlconflict_databody_cookie = CdlConflict_DataBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();

    CYG_REPORT_RETURN();
}

std::string
CdlConflict_DataBody::get_explanation() const
{
    CYG_REPORT_FUNCNAME("CdlConflict_Data::get_explanation");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::string result = message;

    CYG_REPORT_RETURN();
    return result;
}

bool
CdlConflict_DataBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlConflict_DataBody_Magic != cdlconflict_databody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return CdlConflictBody::check_this(zeal);
}

bool
CdlConflict_DataBody::test(CdlConflict conf)
{
    CYG_REPORT_FUNCNAMETYPE("CdlConflict_Data::test", "result %d");
    CYG_REPORT_FUNCARG1XV(conf);
    CYG_PRECONDITION_CLASSC(conf);

    bool result = false;
    CdlConflict_Data tmp = dynamic_cast<CdlConflict_Data>(conf);
    if (0 != tmp) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
